/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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

#include "sync/sync_error_messages.h"
#include "sync/sync_commandbuffer.h"
#include "sync/sync_image.h"
#include "sync/sync_reporting.h"
#include "sync/sync_validation.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/descriptor_sets.h"
#include "utils/text_utils.h"
using text::Format;

#include <cassert>
#include <cinttypes>
#include <cstdarg>
#include <sstream>

static const char* string_SyncHazard(SyncHazard hazard) {
    switch (hazard) {
        case SyncHazard::NONE:
            return "NONE";
            break;
        case SyncHazard::READ_AFTER_WRITE:
            return "READ_AFTER_WRITE";
            break;
        case SyncHazard::WRITE_AFTER_READ:
            return "WRITE_AFTER_READ";
            break;
        case SyncHazard::WRITE_AFTER_WRITE:
            return "WRITE_AFTER_WRITE";
            break;
        case SyncHazard::READ_RACING_WRITE:
            return "READ_RACING_WRITE";
            break;
        case SyncHazard::WRITE_RACING_WRITE:
            return "WRITE_RACING_WRITE";
            break;
        case SyncHazard::WRITE_RACING_READ:
            return "WRITE_RACING_READ";
            break;
        case SyncHazard::READ_AFTER_PRESENT:
            return "READ_AFTER_PRESENT";
            break;
        case SyncHazard::WRITE_AFTER_PRESENT:
            return "WRITE_AFTER_PRESENT";
            break;
        case SyncHazard::PRESENT_AFTER_WRITE:
            return "PRESENT_AFTER_WRITE";
            break;
        case SyncHazard::PRESENT_AFTER_READ:
            return "PRESENT_AFTER_READ";
            break;
        default:
            assert(0);
    }
    return "INVALID HAZARD";
}

namespace syncval {

ErrorMessages::ErrorMessages(vvl::Device& validator)
    : validator_(validator),
      extra_properties_(validator_.syncval_settings.message_extra_properties),
      pretty_print_extra_(validator_.syncval_settings.message_extra_properties_pretty_print) {}

void ErrorMessages::AddCbContextExtraProperties(const CommandBufferAccessContext& cb_context, ResourceUsageTag tag,
                                                ReportKeyValues& key_values) const {
    if (validator_.syncval_settings.message_extra_properties) {
        cb_context.AddUsageRecordExtraProperties(tag, key_values);
    }
}

std::string ErrorMessages::Error(const HazardResult& hazard, const char* description, const CommandBufferAccessContext& cb_context,
                                 vvl::Func command) const {
    const auto format = "Hazard %s for %s. Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), description, access_info.c_str());
    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "GeneralError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::BufferError(const HazardResult& hazard, VkBuffer buffer, const char* buffer_description,
                                       const CommandBufferAccessContext& cb_context, vvl::Func command) const {
    const auto format = "Hazard %s for %s %s. Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), buffer_description,
                                 validator_.FormatHandle(buffer).c_str(), access_info.c_str());
    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "BufferError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

// Given that access is hazardous, we check if at least stage or access part of it is covered
// by the synchronization. If applied synchronization covers at least stage or access component
// then we can provide more precise message by focusing on the other component.
static std::pair<bool, bool> GetPartialProtectedInfo(const SyncAccessInfo& access, const SyncAccessFlags& write_barriers,
                                                     const CommandBufferAccessContext& cb_context) {
    const auto protected_stage_access_pairs =
        ConvertSyncAccessesToCompactVkForm(write_barriers, cb_context.GetQueueFlags(), cb_context.GetSyncState().enabled_features,
                                           cb_context.GetSyncState().extensions);
    bool is_stage_protected = false;
    bool is_access_protected = false;
    for (const auto& protected_stage_access : protected_stage_access_pairs) {
        if (protected_stage_access.first & access.stage_mask) {
            is_stage_protected = true;
        }
        if (protected_stage_access.second & access.access_mask) {
            is_access_protected = true;
        }
    }
    return std::make_pair(is_stage_protected, is_access_protected);
}

std::string ErrorMessages::BufferRegionError(const HazardResult& hazard, VkBuffer buffer, uint32_t region_index,
                                             ResourceAccessRange region_range, const CommandBufferAccessContext& cb_context,
                                             const vvl::Func command) const {
    const SyncHazard hazard_type = hazard.Hazard();

    const SyncAccessFlags write_barriers = hazard.State().access_state->GetWriteBarriers();
    const VkPipelineStageFlags2 read_barriers = hazard.State().access_state->GetReadBarriers(hazard.State().prior_access_index);

    const SyncAccessInfo& access = syncAccessInfoByAccessIndex()[hazard.State().access_index];
    const SyncAccessInfo& prior_access = syncAccessInfoByAccessIndex()[hazard.State().prior_access_index];

    const ReportUsageInfo usage_info = cb_context.GetReportUsageInfo(hazard.TagEx());
    const SyncHazardInfo hazard_info = GetSyncHazardInfo(hazard_type);

    const bool missing_synchronization = (hazard_info.IsPriorWrite() && write_barriers.none()) ||
                                         (hazard_info.IsPriorRead() && read_barriers == VK_PIPELINE_STAGE_2_NONE);

    // Add properties that describe this hazard
    ReportKeyValues key_values;
    cb_context.FormatHazard(hazard, key_values);
    key_values.Add(kPropertyMessageType, "BufferRegionError");
    key_values.Add(kPropertyHazardType, string_SyncHazard(hazard_type));
    key_values.Add(kPropertyCommand, vvl::String(command));
    key_values.Add(kPropertyCopyRegion, region_index);
    AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);

    // Brief description of what happened
    std::stringstream ss;
    ss << string_SyncHazard(hazard_type) << " hazard detected. ";
    ss << vvl::String(command);
    ss << (hazard_info.IsWrite() ? " writes to " : " reads ");
    ss << validator_.FormatHandle(buffer);
    ss << (hazard_info.IsRacingHazard() ? ", which is being " : ", which was previously ");
    ss << (hazard_info.IsPriorWrite() ? "written by " : "read by ");
    if (usage_info.command == command) {
        ss << "another " << vvl::String(usage_info.command) << " command";
    } else {
        ss << vvl::String(usage_info.command);
    }
    if (const auto* debug_region = key_values.FindProperty("debug_region")) {
        ss << " (debug region: " << *debug_region << ")";
    }
    ss << ". ";

    // Synchronization information
    if (missing_synchronization) {
        const char* access_type = hazard_info.IsWrite() ? "write" : "read";
        const char* prior_access_type = hazard_info.IsPriorWrite() ? "write" : "read";
        ss << "No sufficient synchronization is present to ensure that a " << access_type << " (";
        ss << string_VkAccessFlagBits2(access.access_mask) << ") at ";
        ss << string_VkPipelineStageFlagBits2(access.stage_mask) << " does not conflict with a prior ";
        ss << prior_access_type << " (" << string_VkAccessFlags2(prior_access.access_mask) << ") at ";
        if (prior_access.stage_mask == access.stage_mask) {
            ss << "the same stage.";
        } else {
            ss << string_VkPipelineStageFlagBits2(prior_access.stage_mask) << ".";
        }
    } else if (hazard_info.IsPriorWrite()) {  // RAW/WAW hazards
        ss << "The current synchronization allows ";
        ss << FormatSyncAccesses(write_barriers, cb_context.GetQueueFlags(), cb_context.GetSyncState().enabled_features,
                                 cb_context.GetSyncState().extensions, false);
        auto [is_stage_protected, is_access_protected] = GetPartialProtectedInfo(access, write_barriers, cb_context);
        if (is_access_protected) {
            ss << " but not at " << string_VkPipelineStageFlagBits2(access.stage_mask) << ".";
        } else {
            ss << ", but to prevent this hazard, it must allow ";
            ss << string_VkAccessFlagBits2(access.access_mask) << " accesses at ";
            ss << string_VkPipelineStageFlagBits2(access.stage_mask) << ".";
        }
    } else {  // WAR hazard
        ss << "The current synchronization waits at ";
        ss << string_VkPipelineStageFlags2(read_barriers);
        ss << ", but to prevent this hazard, it must wait at ";
        ss << string_VkPipelineStageFlagBits2(access.stage_mask) << ".";
    }

    // Copy region information
    ss << " Hazardous copy region: " << region_index << " (offset = " << region_range.begin;
    ss << ", size = " << region_range.end - region_range.begin << ").";

    // Give a hint for WAR hazard
    if (IsValueIn(hazard_type, {WRITE_AFTER_READ, WRITE_RACING_READ, PRESENT_AFTER_READ})) {
        ss << " An execution dependency is sufficient to prevent this hazard.";
    }

    std::string message = ss.str();
    if (extra_properties_) {
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::ImageRegionError(const HazardResult& hazard, VkImage image, bool is_src_image, uint32_t region_index,
                                            const CommandBufferAccessContext& cb_context, vvl::Func command) const {
    const auto format = "Hazard %s for %s %s, region %" PRIu32 ". Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    const char* resource_parameter = is_src_image ? "srcImage" : "dstImage";
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), resource_parameter,
                                 validator_.FormatHandle(image).c_str(), region_index, access_info.c_str());
    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "ImageRegionError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::ImageSubresourceRangeError(const HazardResult& hazard, VkImage image, uint32_t subresource_range_index,
                                                      const CommandBufferAccessContext& cb_context, vvl::Func command) const {
    const auto format = "Hazard %s for %s, range index %" PRIu32 ". Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), validator_.FormatHandle(image).c_str(),
                                 subresource_range_index, access_info.c_str());
    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "ImageSubresourceRangeError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::BeginRenderingError(const HazardResult& hazard,
                                               const syncval_state::DynamicRenderingInfo::Attachment& attachment,
                                               const CommandBufferAccessContext& cb_context, vvl::Func command) const {
    const auto format = "(%s), with loadOp %s. Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    const char* load_op_str = string_VkAttachmentLoadOp(attachment.info.loadOp);
    std::string message =
        Format(format, validator_.FormatHandle(attachment.view->Handle()).c_str(), load_op_str, access_info.c_str());
    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "BeginRenderingError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        key_values.Add(kPropertyLoadOp, load_op_str);
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::EndRenderingResolveError(const HazardResult& hazard, const VulkanTypedHandle& image_view_handle,
                                                    VkResolveModeFlagBits resolve_mode,
                                                    const CommandBufferAccessContext& cb_context, vvl::Func command) const {
    const auto format = "(%s), during resolve with resolveMode %s. Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    const char* resolve_mode_str = string_VkResolveModeFlagBits(resolve_mode);
    std::string message = Format(format, validator_.FormatHandle(image_view_handle).c_str(), resolve_mode_str, access_info.c_str());
    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "EndRenderingResolveError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        key_values.Add(kPropertyResolveMode, resolve_mode_str);
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::EndRenderingStoreError(const HazardResult& hazard, const VulkanTypedHandle& image_view_handle,
                                                  VkAttachmentStoreOp store_op, const CommandBufferAccessContext& cb_context,
                                                  vvl::Func command) const {
    const auto format = "(%s), during store with storeOp %s. Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    const char* store_op_str = string_VkAttachmentStoreOp(store_op);
    std::string message = Format(format, validator_.FormatHandle(image_view_handle).c_str(), store_op_str, access_info.c_str());
    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "EndRenderingStoreError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        key_values.Add(kPropertyStoreOp, store_op_str);
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::DrawDispatchImageError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                  const vvl::ImageView& image_view, const vvl::Pipeline& pipeline,
                                                  const vvl::DescriptorSet& descriptor_set, VkDescriptorType descriptor_type,
                                                  VkImageLayout image_layout, uint32_t descriptor_binding, uint32_t binding_index,
                                                  vvl::Func command) const {
    const auto format =
        "Hazard %s for %s, in %s, and %s, %s, type: %s, imageLayout: %s, binding #%" PRIu32 ", index %" PRIu32 ". Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    const char* descriptor_type_str = string_VkDescriptorType(descriptor_type);
    const char* image_layout_str = string_VkImageLayout(image_layout);
    std::string message =
        Format(format, string_SyncHazard(hazard.Hazard()), validator_.FormatHandle(image_view.Handle()).c_str(),
               validator_.FormatHandle(cb_context.Handle()).c_str(), validator_.FormatHandle(pipeline.Handle()).c_str(),
               validator_.FormatHandle(descriptor_set.Handle()).c_str(), descriptor_type_str, image_layout_str, descriptor_binding,
               binding_index, access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "DrawDispatchImageError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        key_values.Add(kPropertyDescriptorType, descriptor_type_str);
        key_values.Add(kPropertyImageLayout, image_layout_str);
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::DrawDispatchTexelBufferError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                        const vvl::BufferView& buffer_view, const vvl::Pipeline& pipeline,
                                                        const vvl::DescriptorSet& descriptor_set, VkDescriptorType descriptor_type,
                                                        uint32_t descriptor_binding, uint32_t binding_index,
                                                        vvl::Func command) const {
    const auto format = "Hazard %s for %s in %s, %s, and %s, type: %s, binding #%" PRIu32 " index %" PRIu32 ". Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    const char* descriptor_type_str = string_VkDescriptorType(descriptor_type);
    std::string message =
        Format(format, string_SyncHazard(hazard.Hazard()), validator_.FormatHandle(buffer_view.Handle()).c_str(),
               validator_.FormatHandle(cb_context.Handle()).c_str(), validator_.FormatHandle(pipeline.Handle()).c_str(),
               validator_.FormatHandle(descriptor_set.Handle()).c_str(), descriptor_type_str, descriptor_binding, binding_index,
               access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "DrawDispatchTexelBufferError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        key_values.Add(kPropertyDescriptorType, descriptor_type_str);
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::DrawDispatchBufferError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                   const vvl::Buffer& buffer, const vvl::Pipeline& pipeline,
                                                   const vvl::DescriptorSet& descriptor_set, VkDescriptorType descriptor_type,
                                                   uint32_t descriptor_binding, uint32_t binding_index, vvl::Func command) const {
    const auto format = "Hazard %s for %s in %s, %s, and %s, type: %s, binding #%" PRIu32 " index %" PRIu32 ". Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    const char* descriptor_type_str = string_VkDescriptorType(descriptor_type);
    std::string message =
        Format(format, string_SyncHazard(hazard.Hazard()), validator_.FormatHandle(buffer.Handle()).c_str(),
               validator_.FormatHandle(cb_context.Handle()).c_str(), validator_.FormatHandle(pipeline.Handle()).c_str(),
               validator_.FormatHandle(descriptor_set.Handle()).c_str(), descriptor_type_str, descriptor_binding, binding_index,
               access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "DrawDispatchBufferError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        key_values.Add(kPropertyDescriptorType, descriptor_type_str);
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::DrawVertexBufferError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                 const vvl::Buffer& vertex_buffer, vvl::Func command) const {
    const auto format = "Hazard %s for vertex %s in %s. Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    std::string message =
        Format(format, string_SyncHazard(hazard.Hazard()), validator_.FormatHandle(vertex_buffer.Handle()).c_str(),
               validator_.FormatHandle(cb_context.Handle()).c_str(), access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "DrawVertexBufferError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::DrawIndexBufferError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                const vvl::Buffer& index_buffer, vvl::Func command) const {
    const auto format = "Hazard %s for index %s in %s. Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), validator_.FormatHandle(index_buffer.Handle()).c_str(),
                                 validator_.FormatHandle(cb_context.Handle()).c_str(), access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "DrawIndexBufferError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::DrawAttachmentError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                               const vvl::ImageView& attachment_view, vvl::Func command) const {
    const auto format = "(%s). Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    std::string message = Format(format, validator_.FormatHandle(attachment_view.Handle()).c_str(), access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "DrawAttachmentError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::ClearColorAttachmentError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                     const std::string& subpass_attachment_info, vvl::Func command) const {
    const auto format = "Hazard %s while clearing color attachment%s. Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), subpass_attachment_info.c_str(), access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "ClearColorAttachmentError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::ClearDepthStencilAttachmentError(const HazardResult& hazard,
                                                            const CommandBufferAccessContext& cb_context,
                                                            const std::string& subpass_attachment_info,
                                                            VkImageAspectFlagBits aspect, vvl::Func command) const {
    const auto format = "Hazard %s when clearing %s aspect of depth-stencil attachment%s. Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    const char* image_aspect_str = string_VkImageAspectFlagBits(aspect);
    std::string message =
        Format(format, string_SyncHazard(hazard.Hazard()), image_aspect_str, subpass_attachment_info.c_str(), access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "ClearDepthStencilAttachmentError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        key_values.Add(kPropertyImageAspect, image_aspect_str);
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::PipelineBarrierError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                uint32_t image_barrier_index, const vvl::Image& image, vvl::Func command) const {
    const auto format = "Hazard %s for image barrier %" PRIu32 " %s. Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), image_barrier_index,
                                 validator_.FormatHandle(image.Handle()).c_str(), access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "PipelineBarrierError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::WaitEventsError(const HazardResult& hazard, const CommandExecutionContext& exec_context,
                                           uint32_t image_barrier_index, const vvl::Image& image, vvl::Func command) const {
    const auto format = "Hazard %s for image barrier %" PRIu32 " %s. Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = exec_context.FormatHazard(hazard, key_values);
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), image_barrier_index,
                                 validator_.FormatHandle(image.Handle()).c_str(), access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "WaitEventsError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        exec_context.AddUsageRecordExtraProperties(hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::FirstUseError(const HazardResult& hazard, const CommandExecutionContext& exec_context,
                                         const CommandBufferAccessContext& recorded_context, uint32_t command_buffer_index,
                                         VkCommandBuffer recorded_handle, vvl::Func command) const {
    const auto format = "Hazard %s for entry %" PRIu32 ", %s, %s access info %s. Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = exec_context.FormatHazard(hazard, key_values);
    std::string message =
        Format(format, string_SyncHazard(hazard.Hazard()), command_buffer_index, validator_.FormatHandle(recorded_handle).c_str(),
               exec_context.ExecutionTypeString(),
               recorded_context.FormatUsage(exec_context.ExecutionUsageString(), *hazard.RecordedAccess(), key_values).c_str(),
               access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "SubmitTimeError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        // TODO: ensure correct command is used here, currently it's always empty
        // key_values.Add(kPropertyCommand, vvl::String(command));
        exec_context.AddUsageRecordExtraProperties(hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::RenderPassResolveError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                  uint32_t subpass, const char* aspect_name, const char* attachment_name,
                                                  uint32_t src_attachment, uint32_t dst_attachment, vvl::Func command) const {
    const auto format = "Hazard %s in subpass %" PRIu32 "during %s %s, from attachment %" PRIu32 " to resolve attachment %" PRIu32
                        ". Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), subpass, aspect_name, attachment_name, src_attachment,
                                 dst_attachment, access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "RenderPassResolveError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::RenderPassLayoutTransitionVsStoreOrResolveError(const HazardResult& hazard, uint32_t subpass,
                                                                           uint32_t attachment, VkImageLayout old_layout,
                                                                           VkImageLayout new_layout, uint32_t store_resolve_subpass,
                                                                           vvl::Func command) const {
    const auto format =
        "Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32
        " image layout transition (old_layout: %s, new_layout: %s) after store/resolve operation in subpass %" PRIu32;

    const char* old_layout_str = string_VkImageLayout(old_layout);
    const char* new_layout_str = string_VkImageLayout(new_layout);
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), subpass, attachment, old_layout_str, new_layout_str,
                                 store_resolve_subpass);
    if (extra_properties_) {
        ReportKeyValues key_values;
        key_values.Add(kPropertyMessageType, "RenderPassLayoutTransitionVsStoreOrResolveError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        key_values.Add(kPropertyOldLayout, old_layout_str);
        key_values.Add(kPropertyNewLayout, new_layout_str);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::RenderPassLayoutTransitionError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                           uint32_t subpass, uint32_t attachment, VkImageLayout old_layout,
                                                           VkImageLayout new_layout, vvl::Func command) const {
    const auto format = "Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32
                        " image layout transition (old_layout: %s, new_layout: %s). Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    const char* old_layout_str = string_VkImageLayout(old_layout);
    const char* new_layout_str = string_VkImageLayout(new_layout);
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), subpass, attachment, old_layout_str, new_layout_str,
                                 access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "RenderPassLayoutTransitionError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        key_values.Add(kPropertyOldLayout, old_layout_str);
        key_values.Add(kPropertyNewLayout, new_layout_str);
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::RenderPassLoadOpVsLayoutTransitionError(const HazardResult& hazard, uint32_t subpass,
                                                                   uint32_t attachment, const char* aspect_name,
                                                                   VkAttachmentLoadOp load_op, vvl::Func command) const {
    const auto format =
        "Hazard %s vs. layout transition in subpass %" PRIu32 " for attachment %" PRIu32 " aspect %s during load with loadOp %s.";

    const char* load_op_str = string_VkAttachmentLoadOp(load_op);
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), subpass, attachment, aspect_name, load_op_str);

    if (extra_properties_) {
        ReportKeyValues key_values;
        key_values.Add(kPropertyMessageType, "RenderPassLoadOpVsLayoutTransitionError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        key_values.Add(kPropertyLoadOp, load_op_str);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::RenderPassLoadOpError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                 uint32_t subpass, uint32_t attachment, const char* aspect_name,
                                                 VkAttachmentLoadOp load_op, vvl::Func command) const {
    const auto format =
        "Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32 " aspect %s during load with loadOp %s. Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    const char* load_op_str = string_VkAttachmentLoadOp(load_op);
    std::string message =
        Format(format, string_SyncHazard(hazard.Hazard()), subpass, attachment, aspect_name, load_op_str, access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "RenderPassLoadOpError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        key_values.Add(kPropertyLoadOp, load_op_str);
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::RenderPassStoreOpError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                  uint32_t subpass, uint32_t attachment, const char* aspect_name,
                                                  const char* store_op_type_name, VkAttachmentStoreOp store_op,
                                                  vvl::Func command) const {
    const auto format =
        "Hazard %s in subpass %" PRIu32 " for attachment %" PRIu32 " %s aspect during store with %s %s. Access info %s";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    const char* store_op_str = string_VkAttachmentStoreOp(store_op);
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), subpass, attachment, aspect_name, store_op_type_name,
                                 store_op_str, access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "RenderPassStoreOpError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        key_values.Add(kPropertyStoreOp, store_op_str);
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::RenderPassColorAttachmentError(const HazardResult& hazard, const CommandBufferAccessContext& cb_context,
                                                          const vvl::ImageView& view, uint32_t attachment,
                                                          vvl::Func command) const {
    const auto format = "Hazard %s for %s in %s, Subpass #%d, and pColorAttachments #%d. Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), validator_.FormatHandle(view.Handle()).c_str(),
                                 validator_.FormatHandle(cb_context.GetCBState().Handle()).c_str(),
                                 cb_context.GetCBState().GetActiveSubpass(), attachment, access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "RenderPassColorAttachmentError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::RenderPassDepthStencilAttachmentError(const HazardResult& hazard,
                                                                 const CommandBufferAccessContext& cb_context,
                                                                 const vvl::ImageView& view, bool is_depth,
                                                                 vvl::Func command) const {
    const auto format = "Hazard %s for %s in %s, Subpass #%d, and %s part of pDepthStencilAttachment. Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), validator_.FormatHandle(view.Handle()).c_str(),
                                 validator_.FormatHandle(cb_context.GetCBState().Handle()).c_str(),
                                 cb_context.GetCBState().GetActiveSubpass(), is_depth ? "depth" : "stencil", access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "RenderPassDepthStencilAttachmentError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::RenderPassFinalLayoutTransitionVsStoreOrResolveError(const HazardResult& hazard,
                                                                                const CommandBufferAccessContext& cb_context,
                                                                                uint32_t subpass, uint32_t attachment,
                                                                                VkImageLayout old_layout, VkImageLayout new_layout,
                                                                                vvl::Func command) const {
    const auto format = "Hazard %s vs. store/resolve operations in subpass %" PRIu32 " for attachment %" PRIu32
                        " final image layout transition (old_layout: %s, new_layout: %s).";
    ReportKeyValues key_values;

    const char* old_layout_str = string_VkImageLayout(old_layout);
    const char* new_layout_str = string_VkImageLayout(new_layout);
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), subpass, attachment, old_layout_str, new_layout_str);

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "RenderPassFinalLayoutTransitionVsStoreOrResolveError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        key_values.Add(kPropertyOldLayout, old_layout_str);
        key_values.Add(kPropertyNewLayout, new_layout_str);
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::RenderPassFinalLayoutTransitionError(const HazardResult& hazard,
                                                                const CommandBufferAccessContext& cb_context, uint32_t subpass,
                                                                uint32_t attachment, VkImageLayout old_layout,
                                                                VkImageLayout new_layout, vvl::Func command) const {
    const auto format = "Hazard %s with last use subpass %" PRIu32 " for attachment %" PRIu32
                        " final image layout transition (old_layout: %s, new_layout: %s). Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    const char* old_layout_str = string_VkImageLayout(old_layout);
    const char* new_layout_str = string_VkImageLayout(new_layout);
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), subpass, attachment, old_layout_str, new_layout_str,
                                 access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "RenderPassFinalLayoutTransitionError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        key_values.Add(kPropertyOldLayout, old_layout_str);
        key_values.Add(kPropertyNewLayout, new_layout_str);
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::PresentError(const HazardResult& hazard, const QueueBatchContext& batch_context, uint32_t present_index,
                                        const VulkanTypedHandle& swapchain_handle, uint32_t image_index,
                                        const VulkanTypedHandle& image_handle, vvl::Func command) const {
    const auto format =
        "Hazard %s for present pSwapchains[%" PRIu32 "] , swapchain %s, image index %" PRIu32 " %s, Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = batch_context.FormatHazard(hazard, key_values);
    std::string message =
        Format(format, string_SyncHazard(hazard.Hazard()), present_index, validator_.FormatHandle(swapchain_handle).c_str(),
               image_index, validator_.FormatHandle(image_handle).c_str(), access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "PresentError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        batch_context.AddUsageRecordExtraProperties(hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

std::string ErrorMessages::VideoReferencePictureError(const HazardResult& hazard, uint32_t reference_picture_index,
                                                      const CommandBufferAccessContext& cb_context, vvl::Func command) const {
    const auto format = "Hazard %s for reference picture #%u. Access info %s.";
    ReportKeyValues key_values;

    const std::string access_info = cb_context.FormatHazard(hazard, key_values);
    std::string message = Format(format, string_SyncHazard(hazard.Hazard()), reference_picture_index, access_info.c_str());

    if (extra_properties_) {
        key_values.Add(kPropertyMessageType, "VideoReferencePictureError");
        key_values.Add(kPropertyHazardType, string_SyncHazard(hazard.Hazard()));
        key_values.Add(kPropertyCommand, vvl::String(command));
        AddCbContextExtraProperties(cb_context, hazard.Tag(), key_values);
        message += key_values.GetExtraPropertiesSection(pretty_print_extra_);
    }
    return message;
}

}  // namespace syncval
