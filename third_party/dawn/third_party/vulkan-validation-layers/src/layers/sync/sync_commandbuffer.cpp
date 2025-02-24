/*
 * Copyright (c) 2019-2025 Valve Corporation
 * Copyright (c) 2019-2025 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "sync/sync_commandbuffer.h"
#include "sync/sync_op.h"
#include "sync/sync_reporting.h"
#include "sync/sync_validation.h"
#include "sync/sync_image.h"
#include "state_tracker/descriptor_sets.h"
#include "state_tracker/image_state.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/render_pass_state.h"
#include "state_tracker/shader_module.h"
#include "utils/text_utils.h"

SyncAccessIndex GetSyncStageAccessIndexsByDescriptorSet(VkDescriptorType descriptor_type,
                                                        const spirv::ResourceInterfaceVariable &variable,
                                                        VkShaderStageFlagBits stage_flag) {
    if (!variable.IsAccessed()) {
        return SYNC_ACCESS_INDEX_NONE;
    }
    if (descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) {
        assert(stage_flag == VK_SHADER_STAGE_FRAGMENT_BIT);
        return SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ;
    }
    const auto stage_accesses = sync_utils::GetShaderStageAccesses(stage_flag);

    if (descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER || descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) {
        return stage_accesses.uniform_read;
    }

    // If the desriptorSet is writable, we don't need to care SHADER_READ. SHADER_WRITE is enough.
    // Because if write hazard happens, read hazard might or might not happen.
    // But if write hazard doesn't happen, read hazard is impossible to happen.
    if (variable.IsWrittenTo()) {
        return stage_accesses.storage_write;
    } else if (descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE ||
               descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
               descriptor_type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER) {
        return stage_accesses.sampled_read;
    } else {
        if (variable.IsImage() && !variable.IsImageReadFrom()) {
            // only image descriptor was accessed, not the image data
            return SYNC_ACCESS_INDEX_NONE;
        }
        return stage_accesses.storage_read;
    }
}

CommandExecutionContext::CommandExecutionContext(const SyncValidator &sync_validator, VkQueueFlags queue_flags)
    : sync_state_(sync_validator), error_messages_(sync_validator.error_messages_), queue_flags_(queue_flags) {}

bool CommandExecutionContext::ValidForSyncOps() const {
    const bool valid = GetCurrentEventsContext() && GetCurrentAccessContext();
    assert(valid);
    return valid;
}

CommandBufferAccessContext::CommandBufferAccessContext(const SyncValidator &sync_validator, VkQueueFlags queue_flags)
    : CommandExecutionContext(sync_validator, queue_flags),
      cb_state_(),
      access_log_(std::make_shared<AccessLog>()),
      cbs_referenced_(std::make_shared<CommandBufferSet>()),
      command_number_(0),
      subcommand_number_(0),
      reset_count_(0),
      cb_access_context_(),
      current_context_(&cb_access_context_),
      events_context_(),
      render_pass_contexts_(),
      current_renderpass_context_(),
      sync_ops_() {}

CommandBufferAccessContext::CommandBufferAccessContext(SyncValidator &sync_validator, vvl::CommandBuffer *cb_state)
    : CommandBufferAccessContext(sync_validator, cb_state->GetQueueFlags()) {
    cb_state_ = cb_state;
    sync_state_.stats.AddCommandBufferContext();
}

// NOTE: Make sure the proxy doesn't outlive from, as the proxy is pointing directly to access contexts owned by from.
CommandBufferAccessContext::CommandBufferAccessContext(const CommandBufferAccessContext &from, AsProxyContext dummy)
    : CommandBufferAccessContext(from.sync_state_, from.cb_state_->GetQueueFlags()) {
    // Copy only the needed fields out of from for a temporary, proxy command buffer context
    cb_state_ = from.cb_state_;
    access_log_ = std::make_shared<AccessLog>(*from.access_log_);  // potentially large, but no choice given tagging lookup.
    command_number_ = from.command_number_;
    subcommand_number_ = from.subcommand_number_;
    reset_count_ = from.reset_count_;

    handles_ = from.handles_;
    sync_state_.stats.AddHandleRecord((uint32_t)from.handles_.size());

    const auto *from_context = from.GetCurrentAccessContext();
    assert(from_context);

    // Construct a fully resolved single access context out of from
    cb_access_context_.ResolveFromContext(*from_context);
    // The proxy has flatten the current render pass context (if any), but the async contexts are needed for hazard detection
    cb_access_context_.ImportAsyncContexts(*from_context);

    events_context_ = from.events_context_;

    // We don't want to copy the full render_pass_context_ history just for the proxy.
    sync_state_.stats.AddCommandBufferContext();
}

CommandBufferAccessContext::~CommandBufferAccessContext() {
    sync_state_.stats.RemoveCommandBufferContext();
    sync_state_.stats.RemoveHandleRecord((uint32_t)handles_.size());
}

void CommandBufferAccessContext::Reset() {
    access_log_ = std::make_shared<AccessLog>();
    cbs_referenced_ = std::make_shared<CommandBufferSet>();
    if (cb_state_) {
        cbs_referenced_->push_back(cb_state_->shared_from_this());
    }
    sync_ops_.clear();
    command_number_ = 0;
    subcommand_number_ = 0;
    reset_count_++;

    sync_state_.stats.RemoveHandleRecord((uint32_t)handles_.size());
    handles_.clear();

    current_command_tag_ = vvl::kNoIndex32;
    cb_access_context_.Reset();
    render_pass_contexts_.clear();
    current_context_ = &cb_access_context_;
    current_renderpass_context_ = nullptr;
    events_context_.Clear();
    dynamic_rendering_info_.reset();
}

std::string CommandBufferAccessContext::FormatUsage(const char *usage_string, const ResourceFirstAccess &access,
                                                    ReportKeyValues &key_values) const {
    std::stringstream out;
    assert(access.usage_info);
    out << "(" << usage_string << ": " << access.usage_info->name;
    out << ", " << FormatUsage(access.TagEx(), key_values) << ")";
    return out.str();
}

bool CommandBufferAccessContext::ValidateBeginRendering(const ErrorObject &error_obj,
                                                        syncval_state::BeginRenderingCmdState &cmd_state) const {
    bool skip = false;
    const syncval_state::DynamicRenderingInfo &info = cmd_state.GetRenderingInfo();

    // Load operations do not happen when resuming
    if (info.info.flags & VK_RENDERING_RESUMING_BIT) return skip;

    // Need to hazard detect load operations vs. the attachment views
    const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
    for (uint32_t i = 0; i < attachment_count; i++) {
        const auto &attachment = info.attachments[i];
        const SyncAccessIndex load_index = attachment.GetLoadUsage();
        if (load_index == SYNC_ACCESS_INDEX_NONE) continue;

        const HazardResult hazard =
            GetCurrentAccessContext()->DetectHazard(attachment.view_gen, load_index, attachment.GetOrdering());
        if (hazard.IsHazard()) {
            LogObjectList objlist(cb_state_->Handle(), attachment.view->Handle());
            Location loc = attachment.GetLocation(error_obj.location, i);
            const auto error =
                sync_state_.error_messages_.BeginRenderingError(hazard, attachment, *this, error_obj.location.function);
            skip |= sync_state_.SyncError(hazard.Hazard(), objlist, loc.dot(vvl::Field::imageView), error);
            if (skip) break;
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordBeginRendering(syncval_state::BeginRenderingCmdState &cmd_state,
                                                      const RecordObject &record_obj) {
    using Attachment = syncval_state::DynamicRenderingInfo::Attachment;
    const syncval_state::DynamicRenderingInfo &info = cmd_state.GetRenderingInfo();
    const auto tag = NextCommandTag(record_obj.location.function);

    // Only load if not resuming
    if (0 == (info.info.flags & VK_RENDERING_RESUMING_BIT)) {
        const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
        for (uint32_t i = 0; i < attachment_count; i++) {
            const Attachment &attachment = info.attachments[i];
            const SyncAccessIndex load_index = attachment.GetLoadUsage();
            if (load_index == SYNC_ACCESS_INDEX_NONE) continue;

            GetCurrentAccessContext()->UpdateAccessState(attachment.view_gen, load_index, attachment.GetOrdering(),
                                                         ResourceUsageTagEx{tag});
        }
    }

    dynamic_rendering_info_ = std::move(cmd_state.info);
}

bool CommandBufferAccessContext::ValidateEndRendering(const ErrorObject &error_obj) const {
    bool skip = false;
    if (dynamic_rendering_info_ && (0 == (dynamic_rendering_info_->info.flags & VK_RENDERING_SUSPENDING_BIT))) {
        // Only validate resolve and store if not suspending (as specified by BeginRendering)
        const syncval_state::DynamicRenderingInfo &info = *dynamic_rendering_info_;
        const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
        const AccessContext *access_context = GetCurrentAccessContext();
        assert(access_context);
        auto report_resolve_hazard = [&error_obj, this](const HazardResult &hazard, const Location &loc,
                                                        const VulkanTypedHandle image_handle,
                                                        const VkResolveModeFlagBits resolve_mode) {
            LogObjectList objlist(cb_state_->Handle(), image_handle);
            const auto error = sync_state_.error_messages_.EndRenderingResolveError(hazard, image_handle, resolve_mode, *this,
                                                                                    error_obj.location.function);
            return sync_state_.SyncError(hazard.Hazard(), objlist, loc, error);
        };

        for (uint32_t i = 0; i < attachment_count && !skip; i++) {
            const auto &attachment = info.attachments[i];
            if (attachment.resolve_gen) {
                const bool is_color = attachment.type == syncval_state::AttachmentType::kColor;
                const SyncOrdering kResolveOrder = is_color ? kColorResolveOrder : kDepthStencilResolveOrder;
                // The logic about whether to resolve is embedded in the Attachment constructor
                assert(attachment.view);
                HazardResult hazard = access_context->DetectHazard(attachment.view_gen, kResolveRead, kResolveOrder);

                if (hazard.IsHazard()) {
                    Location loc = attachment.GetLocation(error_obj.location, i);
                    skip |= report_resolve_hazard(hazard, loc.dot(vvl::Field::imageView), attachment.view->Handle(),
                                                  attachment.info.resolveMode);
                }
                if (!skip) {
                    hazard = access_context->DetectHazard(*attachment.resolve_gen, kResolveWrite, kResolveOrder);
                    if (hazard.IsHazard()) {
                        Location loc = attachment.GetLocation(error_obj.location, i);
                        skip |= report_resolve_hazard(hazard, loc.dot(vvl::Field::resolveImageView),
                                                      attachment.resolve_view->Handle(), attachment.info.resolveMode);
                    }
                }
            }

            const auto store_usage = attachment.GetStoreUsage();
            if (store_usage != SYNC_ACCESS_INDEX_NONE) {
                HazardResult hazard = access_context->DetectHazard(attachment.view_gen, store_usage, kStoreOrder);
                if (hazard.IsHazard()) {
                    const VulkanTypedHandle image_handle = attachment.view->Handle();
                    LogObjectList objlist(cb_state_->Handle(), image_handle);
                    Location loc = attachment.GetLocation(error_obj.location, i);
                    const auto error = sync_state_.error_messages_.EndRenderingStoreError(
                        hazard, image_handle, attachment.info.storeOp, *this, error_obj.location.function);
                    skip |= sync_state_.SyncError(hazard.Hazard(), objlist, loc.dot(vvl::Field::imageView), error);
                }
            }
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordEndRendering(const RecordObject &record_obj) {
    if (dynamic_rendering_info_ && (0 == (dynamic_rendering_info_->info.flags & VK_RENDERING_SUSPENDING_BIT))) {
        auto store_tag = NextCommandTag(record_obj.location.function, ResourceUsageRecord::SubcommandType::kStoreOp);

        const syncval_state::DynamicRenderingInfo &info = *dynamic_rendering_info_;
        const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
        AccessContext *access_context = GetCurrentAccessContext();
        for (uint32_t i = 0; i < attachment_count; i++) {
            const auto &attachment = info.attachments[i];
            if (attachment.resolve_gen) {
                const bool is_color = attachment.type == syncval_state::AttachmentType::kColor;
                const SyncOrdering kResolveOrder = is_color ? kColorResolveOrder : kDepthStencilResolveOrder;
                access_context->UpdateAccessState(attachment.view_gen, kResolveRead, kResolveOrder, ResourceUsageTagEx{store_tag});
                access_context->UpdateAccessState(*attachment.resolve_gen, kResolveWrite, kResolveOrder,
                                                  ResourceUsageTagEx{store_tag});
            }

            const SyncAccessIndex store_index = attachment.GetStoreUsage();
            if (store_index == SYNC_ACCESS_INDEX_NONE) continue;
            access_context->UpdateAccessState(attachment.view_gen, store_index, kStoreOrder, ResourceUsageTagEx{store_tag});
        }
    }

    dynamic_rendering_info_.reset();
}

bool CommandBufferAccessContext::ValidateDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint,
                                                                   const Location &loc) const {
    bool skip = false;
    if (!sync_state_.syncval_settings.shader_accesses_heuristic) {
        return skip;
    }
    const vvl::Pipeline *pipe = nullptr;
    const std::vector<LastBound::DescriptorSetSlot> *ds_slots = nullptr;
    cb_state_->GetCurrentPipelineAndDesriptorSets(pipelineBindPoint, &pipe, &ds_slots);
    if (!pipe || !ds_slots) {
        return skip;
    }

    using DescriptorClass = vvl::DescriptorClass;
    using BufferDescriptor = vvl::BufferDescriptor;
    using ImageDescriptor = vvl::ImageDescriptor;
    using TexelDescriptor = vvl::TexelDescriptor;

    for (const auto &stage_state : pipe->stage_states) {
        if (stage_state.GetStage() == VK_SHADER_STAGE_FRAGMENT_BIT && pipe->RasterizationDisabled()) {
            continue;
        } else if (!stage_state.entrypoint) {
            continue;
        }
        for (const auto &variable : stage_state.entrypoint->resource_interface_variables) {
            if (variable.decorations.set >= ds_slots->size()) {
                // This should be caught by Core validation, but if core checks are disabled SyncVal should not crash.
                continue;
            }
            const auto &ds_slot = (*ds_slots)[variable.decorations.set];
            const auto *descriptor_set = ds_slot.ds_state.get();
            if (!descriptor_set) continue;
            auto binding = descriptor_set->GetBinding(variable.decorations.binding);
            const auto descriptor_type = binding->type;
            SyncAccessIndex sync_index = GetSyncStageAccessIndexsByDescriptorSet(descriptor_type, variable, stage_state.GetStage());

            // Currently, validation of memory accesses based on declared descriptors can produce false-positives.
            // The shader can decide not to do such accesses, it can perform accesses with more narrow scope
            // (e.g. read access, when both reads and writes are allowed) or for an array of descriptors, not all
            // elements are accessed in the general case.
            //
            // This workaround disables validation for the descriptor array case.
            if (binding->count > 1) {
                continue;
            }

            for (uint32_t index = 0; index < binding->count; index++) {
                const auto *descriptor = binding->GetDescriptor(index);
                switch (descriptor->GetClass()) {
                    case DescriptorClass::ImageSampler:
                    case DescriptorClass::Image: {
                        if (descriptor->Invalid()) {
                            continue;
                        }

                        // NOTE: ImageSamplerDescriptor inherits from ImageDescriptor, so this cast works for both types.
                        const auto *image_descriptor = static_cast<const ImageDescriptor *>(descriptor);
                        const auto *img_view_state =
                            static_cast<const syncval_state::ImageViewState *>(image_descriptor->GetImageViewState());
                        VkImageLayout image_layout = image_descriptor->GetImageLayout();

                        if (img_view_state->IsDepthSliced()) {
                            // NOTE: 2D ImageViews of VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT Images are not allowed in
                            // Descriptors, unless VK_EXT_image_2d_view_of_3d is supported, which it isn't at the moment.
                            // See: VUID 00343
                            continue;
                        }

                        HazardResult hazard;

                        if (sync_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ) {
                            const VkExtent3D extent = CastTo3D(cb_state_->render_area.extent);
                            const VkOffset3D offset = CastTo3D(cb_state_->render_area.offset);
                            // Input attachments are subject to raster ordering rules
                            hazard =
                                current_context_->DetectHazard(*img_view_state, offset, extent, sync_index, SyncOrdering::kRaster);
                        } else {
                            hazard = current_context_->DetectHazard(*img_view_state, sync_index);
                        }

                        if (hazard.IsHazard() && !sync_state_.SupressedBoundDescriptorWAW(hazard)) {
                            const auto error = error_messages_.DrawDispatchImageError(
                                hazard, *this, *img_view_state, *pipe, *descriptor_set, descriptor_type, image_layout,
                                variable.decorations.binding, index, loc.function);
                            skip |= sync_state_.SyncError(hazard.Hazard(), img_view_state->Handle(), loc, error);
                        }
                        break;
                    }
                    case DescriptorClass::TexelBuffer: {
                        const auto *texel_descriptor = static_cast<const TexelDescriptor *>(descriptor);
                        if (texel_descriptor->Invalid()) {
                            continue;
                        }
                        const auto *buf_view_state = texel_descriptor->GetBufferViewState();
                        const auto *buf_state = buf_view_state->buffer_state.get();
                        const ResourceAccessRange range = MakeRange(*buf_view_state);
                        auto hazard = current_context_->DetectHazard(*buf_state, sync_index, range);
                        if (hazard.IsHazard() && !sync_state_.SupressedBoundDescriptorWAW(hazard)) {
                            const auto error = error_messages_.DrawDispatchTexelBufferError(
                                hazard, *this, *buf_view_state, *pipe, *descriptor_set, descriptor_type,
                                variable.decorations.binding, index, loc.function);
                            skip |= sync_state_.SyncError(hazard.Hazard(), buf_view_state->Handle(), loc, error);
                        }
                        break;
                    }
                    case DescriptorClass::GeneralBuffer: {
                        const auto *buffer_descriptor = static_cast<const BufferDescriptor *>(descriptor);
                        if (buffer_descriptor->Invalid()) {
                            continue;
                        }
                        VkDeviceSize offset = buffer_descriptor->GetOffset();
                        if (vvl::IsDynamicDescriptor(descriptor_type)) {
                            const uint32_t dynamic_offset_index =
                                descriptor_set->GetDynamicOffsetIndexFromBinding(binding->binding);
                            if (dynamic_offset_index >= ds_slot.dynamic_offsets.size()) {
                                continue;  // core validation error
                            }
                            offset += ds_slot.dynamic_offsets[dynamic_offset_index];
                        }
                        const auto *buf_state = buffer_descriptor->GetBufferState();
                        const ResourceAccessRange range = MakeRange(*buf_state, offset, buffer_descriptor->GetRange());
                        auto hazard = current_context_->DetectHazard(*buf_state, sync_index, range);
                        if (hazard.IsHazard() && !sync_state_.SupressedBoundDescriptorWAW(hazard)) {
                            const auto error = error_messages_.DrawDispatchBufferError(
                                hazard, *this, *buf_state, *pipe, *descriptor_set, descriptor_type, variable.decorations.binding,
                                index, loc.function);
                            skip |= sync_state_.SyncError(hazard.Hazard(), buf_state->Handle(), loc, error);
                        }
                        break;
                    }
                    // TODO: INLINE_UNIFORM_BLOCK_EXT, ACCELERATION_STRUCTURE_KHR
                    default:
                        break;
                }
            }
        }
    }
    return skip;
}

// TODO: Record structure repeats Validate. Unify this code, it was the source of bugs few times already.
void CommandBufferAccessContext::RecordDispatchDrawDescriptorSet(VkPipelineBindPoint pipelineBindPoint,
                                                                 const ResourceUsageTag tag) {
    if (!sync_state_.syncval_settings.shader_accesses_heuristic) {
        return;
    }
    const vvl::Pipeline *pipe = nullptr;
    const std::vector<LastBound::DescriptorSetSlot> *ds_slots = nullptr;
    cb_state_->GetCurrentPipelineAndDesriptorSets(pipelineBindPoint, &pipe, &ds_slots);
    if (!pipe || !ds_slots) {
        return;
    }

    using DescriptorClass = vvl::DescriptorClass;
    using BufferDescriptor = vvl::BufferDescriptor;
    using ImageDescriptor = vvl::ImageDescriptor;
    using TexelDescriptor = vvl::TexelDescriptor;

    for (const auto &stage_state : pipe->stage_states) {
        if (stage_state.GetStage() == VK_SHADER_STAGE_FRAGMENT_BIT && pipe->RasterizationDisabled()) {
            continue;
        } else if (!stage_state.entrypoint) {
            continue;
        }
        for (const auto &variable : stage_state.entrypoint->resource_interface_variables) {
            if (variable.decorations.set >= ds_slots->size()) {
                // This should be caught by Core validation, but if core checks are disabled SyncVal should not crash.
                continue;
            }
            const auto &ds_slot = (*ds_slots)[variable.decorations.set];
            const auto *descriptor_set = ds_slot.ds_state.get();
            if (!descriptor_set) continue;
            auto binding = descriptor_set->GetBinding(variable.decorations.binding);
            const auto descriptor_type = binding->type;
            SyncAccessIndex sync_index = GetSyncStageAccessIndexsByDescriptorSet(descriptor_type, variable, stage_state.GetStage());

            // Do not update state for descriptor array (the same as in Validate function).
            if (binding->count > 1) {
                continue;
            }

            for (uint32_t i = 0; i < binding->count; i++) {
                const auto *descriptor = binding->GetDescriptor(i);
                switch (descriptor->GetClass()) {
                    case DescriptorClass::ImageSampler:
                    case DescriptorClass::Image: {
                        // NOTE: ImageSamplerDescriptor inherits from ImageDescriptor, so this cast works for both types.
                        const auto *image_descriptor = static_cast<const ImageDescriptor *>(descriptor);
                        if (image_descriptor->Invalid()) {
                            continue;
                        }
                        const auto *img_view_state =
                            static_cast<const syncval_state::ImageViewState *>(image_descriptor->GetImageViewState());
                        if (img_view_state->IsDepthSliced()) {
                            // NOTE: 2D ImageViews of VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT Images are not allowed in
                            // Descriptors, unless VK_EXT_image_2d_view_of_3d is supported, which it isn't at the moment.
                            // See: VUID 00343
                            continue;
                        }
                        const ResourceUsageTagEx tag_ex = AddCommandHandle(tag, img_view_state->GetImageState()->Handle());
                        if (sync_index == SYNC_FRAGMENT_SHADER_INPUT_ATTACHMENT_READ) {
                            const VkExtent3D extent = CastTo3D(cb_state_->render_area.extent);
                            const VkOffset3D offset = CastTo3D(cb_state_->render_area.offset);
                            current_context_->UpdateAccessState(*img_view_state, sync_index, SyncOrdering::kRaster, offset, extent,
                                                                tag_ex);
                        } else {
                            current_context_->UpdateAccessState(*img_view_state, sync_index, SyncOrdering::kNonAttachment, tag_ex);
                        }
                        break;
                    }
                    case DescriptorClass::TexelBuffer: {
                        const auto *texel_descriptor = static_cast<const TexelDescriptor *>(descriptor);
                        if (texel_descriptor->Invalid()) {
                            continue;
                        }
                        const auto *buf_view_state = texel_descriptor->GetBufferViewState();
                        const auto *buf_state = buf_view_state->buffer_state.get();
                        const ResourceAccessRange range = MakeRange(*buf_view_state);
                        const ResourceUsageTagEx tag_ex = AddCommandHandle(tag, buf_view_state->Handle());
                        current_context_->UpdateAccessState(*buf_state, sync_index, SyncOrdering::kNonAttachment, range, tag_ex);
                        break;
                    }
                    case DescriptorClass::GeneralBuffer: {
                        const auto *buffer_descriptor = static_cast<const BufferDescriptor *>(descriptor);
                        if (buffer_descriptor->Invalid()) {
                            continue;
                        }
                        VkDeviceSize offset = buffer_descriptor->GetOffset();
                        if (vvl::IsDynamicDescriptor(descriptor_type)) {
                            const uint32_t dynamic_offset_index =
                                descriptor_set->GetDynamicOffsetIndexFromBinding(binding->binding);
                            if (dynamic_offset_index >= ds_slot.dynamic_offsets.size()) {
                                continue;  // core validation error
                            }
                            offset += ds_slot.dynamic_offsets[dynamic_offset_index];
                        }
                        const auto *buf_state = buffer_descriptor->GetBufferState();
                        const ResourceAccessRange range = MakeRange(*buf_state, offset, buffer_descriptor->GetRange());
                        const ResourceUsageTagEx tag_ex = AddCommandHandle(tag, buf_state->Handle());
                        current_context_->UpdateAccessState(*buf_state, sync_index, SyncOrdering::kNonAttachment, range, tag_ex);
                        break;
                    }
                    // TODO: INLINE_UNIFORM_BLOCK_EXT, ACCELERATION_STRUCTURE_KHR
                    default:
                        break;
                }
            }
        }
    }
}

bool CommandBufferAccessContext::ValidateDrawVertex(std::optional<uint32_t> vertexCount, uint32_t firstVertex,
                                                    const Location &loc) const {
    bool skip = false;
    const auto *pipe = cb_state_->GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (!pipe) {
        return skip;
    }

    const auto &binding_buffers = cb_state_->current_vertex_buffer_binding_info;
    const auto &vertex_bindings = pipe->IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT)
                                      ? cb_state_->dynamic_state_value.vertex_bindings
                                      : pipe->vertex_input_state->bindings;

    for (const auto &[_, binding_state] : vertex_bindings) {
        const auto &binding_desc = binding_state.desc;
        if (binding_desc.inputRate != VK_VERTEX_INPUT_RATE_VERTEX) {
            // TODO: add support to determine range of instance level attributes
            continue;
        }
        if (const auto *vertex_buffer = vvl::Find(binding_buffers, binding_desc.binding)) {
            const auto buf_state = sync_state_.Get<vvl::Buffer>(vertex_buffer->buffer);
            if (!buf_state) continue;  // also skips if using nullDescriptor

            ResourceAccessRange range;
            if (vertexCount.has_value()) {  // the range is specified
                range = MakeRange(vertex_buffer->offset, firstVertex, *vertexCount, binding_desc.stride);
            } else {  // entire vertex buffer
                range = MakeRange(*vertex_buffer);
            }

            auto hazard = current_context_->DetectHazard(*buf_state, SYNC_VERTEX_ATTRIBUTE_INPUT_VERTEX_ATTRIBUTE_READ, range);
            if (hazard.IsHazard()) {
                const auto error = error_messages_.DrawVertexBufferError(hazard, *this, *buf_state, loc.function);
                skip |= sync_state_.SyncError(hazard.Hazard(), buf_state->Handle(), loc, error);
            }
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordDrawVertex(std::optional<uint32_t> vertexCount, uint32_t firstVertex,
                                                  const ResourceUsageTag tag) {
    const auto *pipe = cb_state_->GetCurrentPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS);
    if (!pipe) {
        return;
    }
    const auto &binding_buffers = cb_state_->current_vertex_buffer_binding_info;
    const auto &vertex_bindings = pipe->IsDynamic(CB_DYNAMIC_STATE_VERTEX_INPUT_EXT)
                                      ? cb_state_->dynamic_state_value.vertex_bindings
                                      : pipe->vertex_input_state->bindings;

    for (const auto &[_, binding_state] : vertex_bindings) {
        const auto &binding_desc = binding_state.desc;
        if (binding_desc.inputRate != VK_VERTEX_INPUT_RATE_VERTEX) {
            // TODO: add support to determine range of instance level attributes
            continue;
        }
        if (const auto *vertex_buffer = vvl::Find(binding_buffers, binding_desc.binding)) {
            const auto buf_state = sync_state_.Get<vvl::Buffer>(vertex_buffer->buffer);
            if (!buf_state) continue;  // also skips if using nullDescriptor

            ResourceAccessRange range;
            if (vertexCount.has_value()) {  // the range is specified
                range = MakeRange(vertex_buffer->offset, firstVertex, *vertexCount, binding_desc.stride);
            } else {  // entire vertex buffer
                range = MakeRange(*vertex_buffer);
            }

            const ResourceUsageTagEx tag_ex = AddCommandHandle(tag, buf_state->Handle());
            current_context_->UpdateAccessState(*buf_state, SYNC_VERTEX_ATTRIBUTE_INPUT_VERTEX_ATTRIBUTE_READ,
                                                SyncOrdering::kNonAttachment, range, tag_ex);
        }
    }
}

bool CommandBufferAccessContext::ValidateDrawVertexIndex(uint32_t index_count, uint32_t firstIndex, const Location &loc) const {
    bool skip = false;
    const auto &index_binding = cb_state_->index_buffer_binding;
    const auto index_buf_state = sync_state_.Get<vvl::Buffer>(index_binding.buffer);
    if (!index_buf_state) return skip;

    const auto index_size = GetIndexAlignment(index_binding.index_type);
    const ResourceAccessRange range = MakeRange(index_binding.offset, firstIndex, index_count, index_size);

    auto hazard = current_context_->DetectHazard(*index_buf_state, SYNC_INDEX_INPUT_INDEX_READ, range);
    if (hazard.IsHazard()) {
        const auto error = error_messages_.DrawIndexBufferError(hazard, *this, *index_buf_state, loc.function);
        skip |= sync_state_.SyncError(hazard.Hazard(), index_buf_state->Handle(), loc, error);
    }

    // TODO: Shader instrumentation support is needed to read index buffer content and determine more accurate range
    // of accessed versices (new syncval mode). Scanning index buffer for each draw can be impractical though.
    // More practical option can be to leave this as an optional heuristic that always tracks entire vertex buffer.
    skip |= ValidateDrawVertex(std::optional<uint32_t>(), 0, loc);
    return skip;
}

void CommandBufferAccessContext::RecordDrawVertexIndex(uint32_t indexCount, uint32_t firstIndex, const ResourceUsageTag tag) {
    const auto &index_binding = cb_state_->index_buffer_binding;
    const auto index_buf_state = sync_state_.Get<vvl::Buffer>(index_binding.buffer);
    if (!index_buf_state) return;

    const auto index_size = GetIndexAlignment(index_binding.index_type);
    const ResourceAccessRange range = MakeRange(index_binding.offset, firstIndex, indexCount, index_size);
    const ResourceUsageTagEx tag_ex = AddCommandHandle(tag, index_buf_state->Handle());
    current_context_->UpdateAccessState(*index_buf_state, SYNC_INDEX_INPUT_INDEX_READ, SyncOrdering::kNonAttachment, range, tag_ex);

    // TODO: Shader instrumentation support is needed to read index buffer content and determine more accurate range
    // of accessed versices (new syncval mode). Scanning index buffer for each draw can be impractical though.
    // More practical option can be to leave this as an optional heuristic that always tracks entire vertex buffer.
    RecordDrawVertex(std::optional<uint32_t>(), 0, tag);
}

bool CommandBufferAccessContext::ValidateDrawAttachment(const Location &loc) const {
    bool skip = false;
    if (current_renderpass_context_) {
        skip |= current_renderpass_context_->ValidateDrawSubpassAttachment(*this, loc.function);
    } else if (dynamic_rendering_info_) {
        skip |= ValidateDrawDynamicRenderingAttachment(loc);
    }
    return skip;
}

bool CommandBufferAccessContext::ValidateDrawDynamicRenderingAttachment(const Location &location) const {
    bool skip = false;
    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const auto &last_bound_state = cb_state_->lastBound[lv_bind_point];
    const auto *pipe = last_bound_state.pipeline_state;
    if (!pipe || pipe->RasterizationDisabled()) return skip;

    const auto &list = pipe->fragmentShader_writable_output_location_list;
    const auto &access_context = *GetCurrentAccessContext();

    const syncval_state::DynamicRenderingInfo &info = *dynamic_rendering_info_;
    for (const auto output_location : list) {
        if (output_location >= info.info.colorAttachmentCount) continue;
        const auto &attachment = info.attachments[output_location];
        if (!attachment.IsWriteable(last_bound_state)) continue;

        HazardResult hazard = access_context.DetectHazard(attachment.view_gen, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                                                          SyncOrdering::kColorAttachment);
        if (hazard.IsHazard()) {
            LogObjectList obj_list(cb_state_->Handle(), attachment.view->Handle());
            Location loc = attachment.GetLocation(location, output_location);
            const auto error = error_messages_.DrawAttachmentError(hazard, *this, *attachment.view, location.function);
            skip |= sync_state_.SyncError(hazard.Hazard(), obj_list, loc.dot(vvl::Field::imageView), error);
        }
    }

    // TODO -- fixup this and Subpass attachment to correct map the various depth stencil enables/reads vs. writes
    // PHASE1 TODO: Add layout based read/vs. write selection.
    // PHASE1 TODO: Read operations for both depth and stencil are possible in the future.
    // PHASE1 TODO: Add EARLY stage detection based on ExecutionMode.

    const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
    for (uint32_t i = info.info.colorAttachmentCount; i < attachment_count; i++) {
        const auto &attachment = info.attachments[i];
        bool writeable = attachment.IsWriteable(last_bound_state);

        if (writeable) {
            HazardResult hazard =
                access_context.DetectHazard(attachment.view_gen, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                            SyncOrdering::kDepthStencilAttachment);
            // Depth stencil Hazard check
            if (hazard.IsHazard()) {
                LogObjectList objlist(cb_state_->Handle(), attachment.view->Handle());
                Location loc = attachment.GetLocation(location);
                const auto error = error_messages_.DrawAttachmentError(hazard, *this, *attachment.view, location.function);
                skip |= sync_state_.SyncError(hazard.Hazard(), objlist, loc.dot(vvl::Field::imageView), error);
            }
        }
    }

    return skip;
}

void CommandBufferAccessContext::RecordDrawAttachment(const ResourceUsageTag tag) {
    if (current_renderpass_context_) {
        current_renderpass_context_->RecordDrawSubpassAttachment(*cb_state_, tag);
    } else if (dynamic_rendering_info_) {
        RecordDrawDynamicRenderingAttachment(tag);
    }
}

void CommandBufferAccessContext::RecordDrawDynamicRenderingAttachment(ResourceUsageTag tag) {
    const auto lv_bind_point = ConvertToLvlBindPoint(VK_PIPELINE_BIND_POINT_GRAPHICS);
    const auto &last_bound_state = cb_state_->lastBound[lv_bind_point];
    const auto *pipe = last_bound_state.pipeline_state;
    if (!pipe || pipe->RasterizationDisabled()) return;

    const auto &list = pipe->fragmentShader_writable_output_location_list;
    auto &access_context = *GetCurrentAccessContext();

    const syncval_state::DynamicRenderingInfo &info = *dynamic_rendering_info_;
    for (const auto output_location : list) {
        if (output_location >= info.info.colorAttachmentCount) continue;
        const auto &attachment = info.attachments[output_location];
        if (!attachment.IsWriteable(last_bound_state)) continue;

        access_context.UpdateAccessState(attachment.view_gen, SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                                         SyncOrdering::kColorAttachment, ResourceUsageTagEx{tag});
    }

    // TODO -- fixup this and Subpass attachment to correct map the various depth stencil enables/reads vs. writes
    // PHASE1 TODO: Add layout based read/vs. write selection.
    // PHASE1 TODO: Read operations for both depth and stencil are possible in the future.
    // PHASE1 TODO: Add EARLY stage detection based on ExecutionMode.

    const uint32_t attachment_count = static_cast<uint32_t>(info.attachments.size());
    for (uint32_t i = info.info.colorAttachmentCount; i < attachment_count; i++) {
        const auto &attachment = info.attachments[i];
        bool writeable = attachment.IsWriteable(last_bound_state);

        if (writeable) {
            access_context.UpdateAccessState(attachment.view_gen, SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                             SyncOrdering::kDepthStencilAttachment, ResourceUsageTagEx{tag});
        }
    }
}

ClearAttachmentInfo CommandBufferAccessContext::GetClearAttachmentInfo(const VkClearAttachment &clear_attachment,
                                                                       const VkClearRect &rect) const {
    // This is a NOOP if there's no renderpass nor dynamic rendering
    // Caller must used "IsValid" to determine if clear_info contains meaningful information.
    ClearAttachmentInfo clear_info;
    if (current_renderpass_context_) {
        clear_info = current_renderpass_context_->GetClearAttachmentInfo(clear_attachment, rect);
    } else if (dynamic_rendering_info_) {
        clear_info = dynamic_rendering_info_->GetClearAttachmentInfo(clear_attachment, rect);
    }

    return clear_info;
}

bool CommandBufferAccessContext::ValidateClearAttachment(const Location &loc, const VkClearAttachment &clear_attachment,
                                                         const VkClearRect &rect) const {
    bool skip = false;

    ClearAttachmentInfo clear_info = GetClearAttachmentInfo(clear_attachment, rect);
    if (clear_info.IsValid()) {
        skip |= ValidateClearAttachment(loc, clear_info);
    }

    return skip;
}

void CommandBufferAccessContext::RecordClearAttachment(ResourceUsageTag tag, const VkClearAttachment &clear_attachment,
                                                       const VkClearRect &rect) {
    ClearAttachmentInfo clear_info = GetClearAttachmentInfo(clear_attachment, rect);
    if (clear_info.IsValid()) {
        RecordClearAttachment(tag, clear_info);
    }
}

QueueId CommandBufferAccessContext::GetQueueId() const { return kQueueIdInvalid; }

ResourceUsageTag CommandBufferAccessContext::RecordBeginRenderPass(
    vvl::Func command, const vvl::RenderPass &rp_state, const VkRect2D &render_area,
    const std::vector<const syncval_state::ImageViewState *> &attachment_views) {
    // Create an access context the current renderpass.
    const auto barrier_tag = NextCommandTag(command, ResourceUsageRecord::SubcommandType::kSubpassTransition);
    AddCommandHandle(barrier_tag, rp_state.Handle());
    const auto load_tag = NextSubcommandTag(command, ResourceUsageRecord::SubcommandType::kLoadOp);
    render_pass_contexts_.emplace_back(
        std::make_unique<RenderPassAccessContext>(rp_state, render_area, GetQueueFlags(), attachment_views, &cb_access_context_));
    current_renderpass_context_ = render_pass_contexts_.back().get();
    current_renderpass_context_->RecordBeginRenderPass(barrier_tag, load_tag);
    current_context_ = &current_renderpass_context_->CurrentContext();
    return barrier_tag;
}

ResourceUsageTag CommandBufferAccessContext::RecordNextSubpass(vvl::Func command) {
    assert(current_renderpass_context_);
    if (!current_renderpass_context_) return NextCommandTag(command);

    auto store_tag = NextCommandTag(command, ResourceUsageRecord::SubcommandType::kStoreOp);
    AddCommandHandle(store_tag, current_renderpass_context_->GetRenderPassState()->Handle());

    auto barrier_tag = NextSubcommandTag(command, ResourceUsageRecord::SubcommandType::kSubpassTransition);
    auto load_tag = NextSubcommandTag(command, ResourceUsageRecord::SubcommandType::kLoadOp);

    current_renderpass_context_->RecordNextSubpass(store_tag, barrier_tag, load_tag);
    current_context_ = &current_renderpass_context_->CurrentContext();
    return barrier_tag;
}

ResourceUsageTag CommandBufferAccessContext::RecordEndRenderPass(vvl::Func command) {
    assert(current_renderpass_context_);
    if (!current_renderpass_context_) return NextCommandTag(command);

    auto store_tag = NextCommandTag(command, ResourceUsageRecord::SubcommandType::kStoreOp);
    AddCommandHandle(store_tag, current_renderpass_context_->GetRenderPassState()->Handle());

    auto barrier_tag = NextSubcommandTag(command, ResourceUsageRecord::SubcommandType::kSubpassTransition);

    current_renderpass_context_->RecordEndRenderPass(&cb_access_context_, store_tag, barrier_tag);
    current_context_ = &cb_access_context_;
    current_renderpass_context_ = nullptr;
    return barrier_tag;
}

void CommandBufferAccessContext::RecordDestroyEvent(vvl::Event *event_state) { GetCurrentEventsContext()->Destroy(event_state); }

void CommandBufferAccessContext::RecordExecutedCommandBuffer(const CommandBufferAccessContext &recorded_cb_context) {
    const AccessContext *recorded_context = recorded_cb_context.GetCurrentAccessContext();
    assert(recorded_context);

    // Just run through the barriers ignoring the usage from the recorded context, as Resolve will overwrite outdated state
    const ResourceUsageTag base_tag = GetTagCount();
    for (const auto &sync_op : recorded_cb_context.GetSyncOps()) {
        // we update the range to any include layout transition first use writes,
        // as they are stored along with the source scope (as effective barrier) when recorded
        sync_op.sync_op->ReplayRecord(*this, base_tag + sync_op.tag);
    }

    ImportRecordedAccessLog(recorded_cb_context);
    ResolveExecutedCommandBuffer(*recorded_context, base_tag);
}

void CommandBufferAccessContext::ResolveExecutedCommandBuffer(const AccessContext &recorded_context, ResourceUsageTag offset) {
    auto tag_offset = [offset](ResourceAccessState *access) { access->OffsetTag(offset); };
    GetCurrentAccessContext()->ResolveFromContext(tag_offset, recorded_context);
}

void CommandBufferAccessContext::ImportRecordedAccessLog(const CommandBufferAccessContext &recorded_context) {
    cbs_referenced_->emplace_back(recorded_context.GetCBStateShared());
    access_log_->insert(access_log_->end(), recorded_context.access_log_->cbegin(), recorded_context.access_log_->cend());

    // Adjust command indices for the log records added from recorded_context.
    const auto &recorded_label_commands = recorded_context.cb_state_->GetLabelCommands();
    const bool use_proxy = !proxy_label_commands_.empty();
    const auto &label_commands = use_proxy ? proxy_label_commands_ : cb_state_->GetLabelCommands();
    if (!label_commands.empty()) {
        assert(label_commands.size() >= recorded_label_commands.size());
        const uint32_t command_offset = static_cast<uint32_t>(label_commands.size() - recorded_label_commands.size());
        for (size_t i = 0; i < recorded_context.access_log_->size(); i++) {
            size_t index = (access_log_->size() - 1) - i;
            assert((*access_log_)[index].label_command_index != vvl::kU32Max);
            (*access_log_)[index].label_command_index += command_offset;
        }
    }
}

ResourceUsageTag CommandBufferAccessContext::NextCommandTag(vvl::Func command, ResourceUsageRecord::SubcommandType subcommand) {
    command_number_++;
    subcommand_number_ = 0;
    current_command_tag_ = access_log_->size();

    auto &record = access_log_->emplace_back(command, command_number_, subcommand, subcommand_number_, cb_state_, reset_count_);

    if (!cb_state_->GetLabelCommands().empty()) {
        record.label_command_index = static_cast<uint32_t>(cb_state_->GetLabelCommands().size() - 1);
    }
    CheckCommandTagDebugCheckpoint();
    return current_command_tag_;
}

ResourceUsageTag CommandBufferAccessContext::NextSubcommandTag(vvl::Func command, ResourceUsageRecord::SubcommandType subcommand) {
    subcommand_number_++;

    const ResourceUsageTag tag = access_log_->size();
    auto &record = access_log_->emplace_back(command, command_number_, subcommand, subcommand_number_, cb_state_, reset_count_);

    // By default copy handle range from the main command, but can be overwritten with AddSubcommandHandle.
    const auto &main_command_record = (*access_log_)[current_command_tag_];
    record.first_handle_index = main_command_record.first_handle_index;
    record.handle_count = main_command_record.handle_count;

    if (!cb_state_->GetLabelCommands().empty()) {
        record.label_command_index = static_cast<uint32_t>(cb_state_->GetLabelCommands().size() - 1);
    }
    return tag;
}

uint32_t CommandBufferAccessContext::AddHandle(const VulkanTypedHandle &typed_handle, uint32_t index) {
    const uint32_t handle_index = static_cast<uint32_t>(handles_.size());
    handles_.emplace_back(HandleRecord(typed_handle, index));
    sync_state_.stats.AddHandleRecord();
    return handle_index;
}

ResourceUsageTagEx CommandBufferAccessContext::AddCommandHandle(ResourceUsageTag tag, const VulkanTypedHandle &typed_handle,
                                                                uint32_t index) {
    assert(tag < access_log_->size());
    const uint32_t handle_index = AddHandle(typed_handle, index);
    // TODO: the following range check is not needed. Test and remove.
    if (tag < access_log_->size()) {
        auto &record = (*access_log_)[tag];
        if (record.first_handle_index == vvl::kNoIndex32) {
            record.first_handle_index = handle_index;
            record.handle_count = 1;
        } else {
            // assert that command handles occupy continuous range
            assert(handle_index - record.first_handle_index == record.handle_count);
            record.handle_count++;
        }
    }
    return {tag, handle_index};
}

void CommandBufferAccessContext::AddSubcommandHandle(ResourceUsageTag tag, const VulkanTypedHandle &typed_handle, uint32_t index) {
    assert(tag < access_log_->size());
    const uint32_t handle_index = AddHandle(typed_handle, index);
    // TODO: the following range check is not needed. Test and remove.
    if (tag < access_log_->size()) {
        auto &record = (*access_log_)[tag];
        const auto &main_command_record = (*access_log_)[current_command_tag_];
        if (record.first_handle_index == main_command_record.first_handle_index) {
            // override default behavior that subcommand references the same handles as the main command
            record.first_handle_index = handle_index;
            record.handle_count = 1;
        } else {
            // assert that command handles occupy continuous range
            assert(handle_index - record.first_handle_index == record.handle_count);
            record.handle_count++;
        }
    }
}

std::string CommandBufferAccessContext::GetDebugRegionName(const ResourceUsageRecord &record) const {
    const bool use_proxy = !proxy_label_commands_.empty();
    const auto &label_commands = use_proxy ? proxy_label_commands_ : cb_state_->GetLabelCommands();
    return vvl::CommandBuffer::GetDebugRegionName(label_commands, record.label_command_index);
}

void CommandBufferAccessContext::RecordSyncOp(SyncOpPointer &&sync_op) {
    auto tag = sync_op->Record(this);
    // As renderpass operations can have side effects on the command buffer access context,
    // update the sync operation to record these if any.
    sync_ops_.emplace_back(tag, std::move(sync_op));
}

bool CommandBufferAccessContext::ValidateClearAttachment(const Location &loc, const ClearAttachmentInfo &info) const {
    bool skip = false;
    VkImageSubresourceRange subresource_range = info.subresource_range;
    const AccessContext *access_context = GetCurrentAccessContext();
    assert(access_context);
    if (info.aspects_to_clear & kColorAspects) {
        assert(GetBitSetCount(info.aspects_to_clear) == 1);
        subresource_range.aspectMask = info.aspects_to_clear;

        HazardResult hazard = access_context->DetectHazard(
            *info.view->GetImageState(), subresource_range, info.offset, info.extent, info.view->IsDepthSliced(),
            SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE, SyncOrdering::kColorAttachment);
        if (hazard.IsHazard()) {
            const LogObjectList objlist(cb_state_->Handle(), info.view->Handle());
            const auto error =
                error_messages_.ClearColorAttachmentError(hazard, *this, info.GetSubpassAttachmentText(), loc.function);
            skip |= sync_state_.SyncError(hazard.Hazard(), objlist, loc, error);
        }
    }

    constexpr VkImageAspectFlagBits depth_stencil_aspects[2] = {VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_ASPECT_STENCIL_BIT};
    for (const auto aspect : depth_stencil_aspects) {
        if (info.aspects_to_clear & aspect) {
            // Original aspect mask can contain both stencil and depth but here we track each aspect separately
            subresource_range.aspectMask = aspect;

            // vkCmdClearAttachments depth/stencil writes are executed by the EARLY_FRAGMENT_TESTS_BIT and LATE_FRAGMENT_TESTS_BIT
            // stages. The implementation tracks the most recent access, which happens in the LATE_FRAGMENT_TESTS_BIT stage.
            HazardResult hazard = access_context->DetectHazard(
                *info.view->GetImageState(), info.subresource_range, info.offset, info.extent, info.view->IsDepthSliced(),
                SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE, SyncOrdering::kDepthStencilAttachment);

            if (hazard.IsHazard()) {
                const LogObjectList objlist(cb_state_->Handle(), info.view->Handle());
                const auto error = error_messages_.ClearDepthStencilAttachmentError(hazard, *this, info.GetSubpassAttachmentText(),
                                                                                    aspect, loc.function);
                skip |= sync_state_.SyncError(hazard.Hazard(), objlist, loc, error);
            }
        }
    }
    return skip;
}

void CommandBufferAccessContext::RecordClearAttachment(ResourceUsageTag tag, const ClearAttachmentInfo &clear_info) {
    auto subresource_range = clear_info.subresource_range;

    // Original subresource range can include aspects that are not cleared, they should not be tracked
    subresource_range.aspectMask = clear_info.aspects_to_clear;
    AccessContext *access_context = GetCurrentAccessContext();

    if (clear_info.aspects_to_clear & kColorAspects) {
        assert((clear_info.aspects_to_clear & kDepthStencilAspects) == 0);
        access_context->UpdateAccessState(*clear_info.view->GetImageState(), SYNC_COLOR_ATTACHMENT_OUTPUT_COLOR_ATTACHMENT_WRITE,
                                          SyncOrdering::kColorAttachment, subresource_range, clear_info.offset, clear_info.extent,
                                          ResourceUsageTagEx{tag});
    } else {
        assert((clear_info.aspects_to_clear & kColorAspects) == 0);
        access_context->UpdateAccessState(*clear_info.view->GetImageState(),
                                          SYNC_LATE_FRAGMENT_TESTS_DEPTH_STENCIL_ATTACHMENT_WRITE,
                                          SyncOrdering::kDepthStencilAttachment, subresource_range, clear_info.offset,
                                          clear_info.extent, ResourceUsageTagEx{tag});
    }
}

// NOTE: debug location reporting feature works only for reproducible application sessions
// (it uses command number/reset count from the error message from the previous session).
// It's considered experimental and can be replaced with a better way to report syncval debug locations.
//
// Logs informational message when vulkan command stream reaches a specific location.
// The message can be intercepted by the reporting routines. For example, the message handler can trigger a breakpoint.
// The location can be specified through environment variables.
// VK_SYNCVAL_DEBUG_COMMAND_NUMBER: the command number
// VK_SYNCVAL_DEBUG_RESET_COUNT: (optional, default value is 1) command buffer reset count
// VK_SYNCVAL_DEBUG_CMDBUF_PATTERN: (optional, empty string by default) pattern to match command buffer debug name
void CommandBufferAccessContext::CheckCommandTagDebugCheckpoint() {
    auto get_cmdbuf_name = [](const DebugReport &debug_report, uint64_t cmdbuf_handle) {
        std::unique_lock<std::mutex> lock(debug_report.debug_output_mutex);
        std::string object_name = debug_report.GetUtilsObjectNameNoLock(cmdbuf_handle);
        if (object_name.empty()) {
            object_name = debug_report.GetMarkerObjectNameNoLock(cmdbuf_handle);
        }
        text::ToLower(object_name);
        return object_name;
    };
    if (sync_state_.debug_command_number == command_number_ && sync_state_.debug_reset_count == reset_count_) {
        const auto cmdbuf_name = get_cmdbuf_name(*sync_state_.debug_report, cb_state_->Handle().handle);
        const auto &pattern = sync_state_.debug_cmdbuf_pattern;
        const bool cmdbuf_match = pattern.empty() || (cmdbuf_name.find(pattern) != std::string::npos);
        if (cmdbuf_match) {
            sync_state_.LogInfo("SYNCVAL_DEBUG_COMMAND", LogObjectList(), Location(access_log_->back().command),
                                "Command stream has reached command #%" PRIu32 " in command buffer %s with reset count #%" PRIu32,
                                sync_state_.debug_command_number, sync_state_.FormatHandle(cb_state_->Handle()).c_str(),
                                sync_state_.debug_reset_count);
        }
    }
}

syncval_state::CommandBuffer::CommandBuffer(SyncValidator &dev, VkCommandBuffer handle,
                                            const VkCommandBufferAllocateInfo *allocate_info, const vvl::CommandPool *pool)
    : vvl::CommandBuffer(dev, handle, allocate_info, pool), access_context(dev, this) {}

void syncval_state::CommandBuffer::Destroy() {
    access_context.Destroy();  // must be first to clean up self references correctly.
    vvl::CommandBuffer::Destroy();
}

void syncval_state::CommandBuffer::Reset(const Location &loc) {
    vvl::CommandBuffer::Reset(loc);
    access_context.Reset();
}

void syncval_state::CommandBuffer::NotifyInvalidate(const vvl::StateObject::NodeList &invalid_nodes, bool unlink) {
    for (auto &obj : invalid_nodes) {
        switch (obj->Type()) {
            case kVulkanObjectTypeEvent:
                access_context.RecordDestroyEvent(static_cast<vvl::Event *>(obj.get()));
                break;
            default:
                break;
        }
        vvl::CommandBuffer::NotifyInvalidate(invalid_nodes, unlink);
    }
}
