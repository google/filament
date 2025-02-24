/* Copyright (c) 2024-2025 The Khronos Group Inc.
 * Copyright (c) 2024-2025 Valve Corporation
 * Copyright (c) 2024-2025 LunarG, Inc.
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

#include "sync/sync_reporting.h"
#include "sync/sync_image.h"
#include "sync/sync_validation.h"

SyncNodeFormatter::SyncNodeFormatter(const SyncValidator &sync_state, const vvl::CommandBuffer *cb_state)
    : debug_report(sync_state.debug_report), node(cb_state), label("command_buffer") {}

SyncNodeFormatter::SyncNodeFormatter(const SyncValidator &sync_state, const vvl::Image *image)
    : debug_report(sync_state.debug_report), node(image), label("image") {}

SyncNodeFormatter::SyncNodeFormatter(const SyncValidator &sync_state, const vvl::Queue *q_state)
    : debug_report(sync_state.debug_report), node(q_state), label("queue") {}

SyncNodeFormatter::SyncNodeFormatter(const SyncValidator &sync_state, const vvl::StateObject *state_object, const char *label_)
    : debug_report(sync_state.debug_report), node(state_object), label(label_) {}

std::string FormatStateObject(const SyncNodeFormatter &formatter) {
    std::stringstream out;
    if (formatter.label) {
        out << formatter.label << ": ";
    }
    if (formatter.node) {
        out << formatter.debug_report->FormatHandle(*formatter.node).c_str();
        if (formatter.node->Destroyed()) {
            out << " (destroyed)";
        }
    } else {
        out << "null handle";
    }
    return out.str();
}

void ReportKeyValues::Add(std::string_view key, std::string_view value) {
    key_values.emplace_back(KeyValue{std::string(key), std::string(value)});
}

void ReportKeyValues::Add(std::string_view key, uint64_t value) {
    key_values.emplace_back(KeyValue{std::string(key), std::to_string(value)});
}

static auto SortKeyValues(const std::vector<ReportKeyValues::KeyValue> &key_values) {
    auto get_sort_order = [](const std::string &key) -> uint32_t {
        // message_type goes first
        if (key == kPropertyMessageType) {
            return 0;
        }
        // followed by hazard type
        if (key == kPropertyHazardType) {
            return 1;
        }
        // then some common properties
        const char *common_properties[] = {kPropertyAccess,       kPropertyPriorAccess,  kPropertyCommand,
                                           kPropertyPriorCommand, kPropertyReadBarriers, kPropertyWriteBarriers};
        if (IsValueIn(key, common_properties)) {
            return 2;
        }
        // debug properties are at the end
        const char *debug_properties[] = {kPropertySeqNo, kPropertySubCmd, kPropertyResetNo, kPropertyBatchTag};
        if (IsValueIn(key, debug_properties)) {
            return 4;
        }
        // everything else
        return 3;
    };
    auto sorted = key_values;
    std::stable_sort(sorted.begin(), sorted.end(), [&get_sort_order](const auto &a, const auto &b) {
        const uint32_t a_order = get_sort_order(a.key);
        const uint32_t b_order = get_sort_order(b.key);
        // Sort ordering groups
        if (a_order != b_order) {
            return a_order < b_order;
        }
        // Do not rearrange elements within a group. By returning false we indicate neither element
        // in the group is less than the other one. Stable sort will keep the original order.
        return false;
    });
    return sorted;
}

std::string ReportKeyValues::GetExtraPropertiesSection(bool pretty_print) const {
    if (key_values.empty()) {
        return {};
    }
    const auto sorted = SortKeyValues(key_values);
    std::stringstream ss;
    ss << "\n[Extra properties]\n";
    bool first = true;
    for (const auto &kv : sorted) {
        if (!first) {
            ss << "\n";
        }
        first = false;

        const uint32_t pretty_print_alignment = 18;
        uint32_t extra_space_count = 0;
        if (pretty_print && kv.key.length() < pretty_print_alignment) {
            extra_space_count = pretty_print_alignment - (uint32_t)kv.key.length();
        }

        ss << kv.key << std::string(extra_space_count, ' ') << " = " << kv.value;
    }
    return ss.str();
}

const std::string *ReportKeyValues::FindProperty(const std::string &key) const {
    for (const auto &property : key_values) {
        if (property.key == key) {
            return &property.value;
        }
    }
    return nullptr;
}

static std::string FormatHandleRecord(const HandleRecord::FormatterState &formatter) {
    std::stringstream out;
    const HandleRecord &handle = formatter.that;
    bool labeled = false;

    // Hardcode possible options in order not to store string per HandleRecord object.
    // If more general solution is needed the preference should be to store const char*
    // literal (8 bytes on 64 bit) instead of std::string which, even if empty,
    // can occupy 40 bytes, as was observed in one implementation. HandleRecord is memory
    // sensitive object (there can be a lot of instances).
    if (handle.type == kVulkanObjectTypeRenderPass) {
        out << "renderpass";
        labeled = true;
    } else if (handle.type == kVulkanObjectTypeCommandBuffer && handle.IsIndexed()) {
        out << "pCommandBuffers";
        labeled = true;
    }

    if (handle.IsIndexed()) {
        out << "[" << handle.index << "]";
        labeled = true;
    }
    if (labeled) {
        out << ": ";
    }
    out << formatter.state.FormatHandle(handle.TypedHandle());
    return out.str();
}

static std::string FormatResourceUsageRecord(const ResourceUsageRecord::FormatterState &formatter, ReportKeyValues &key_values) {
    std::stringstream out;
    const ResourceUsageRecord &record = formatter.record;
    if (record.alt_usage) {
        out << record.alt_usage.Formatter(formatter.sync_state);
    } else {
        out << "command: " << vvl::String(record.command);
        // Note: ex_cb_state set to null forces output of record.cb_state
        if (!formatter.ex_cb_state || (formatter.ex_cb_state != record.cb_state)) {
            out << ", " << FormatStateObject(SyncNodeFormatter(formatter.sync_state, record.cb_state));
        }

        // Associated resource
        if (formatter.handle_index != vvl::kNoIndex32) {
            auto cb_context = static_cast<const syncval_state::CommandBuffer *>(record.cb_state);
            const auto handle_records = cb_context->access_context.GetHandleRecords();

            // Command buffer can be in inconsistent state due to unhandled core validation error (core validation is disabled).
            // In this case the goal is not to crash, no guarantees that reported information (handle index) makes sense.
            const bool valid_handle_index = formatter.handle_index < handle_records.size();

            if (valid_handle_index) {
                out << ", resource: " << FormatHandleRecord(handle_records[formatter.handle_index].Formatter(formatter.sync_state));
            }
        }
        // Report debug region name. Empty name means that we are not inside any debug region.
        if (formatter.debug_name_provider) {
            const std::string debug_region_name = formatter.debug_name_provider->GetDebugRegionName(record);
            if (!debug_region_name.empty()) {
                out << ", debug_region: " << debug_region_name;
                key_values.Add(kPropertyDebugRegion, debug_region_name);
            }
        }
    }
    return out.str();
}

static bool IsHazardVsRead(SyncHazard hazard) {
    bool vs_read = false;
    switch (hazard) {
        case SyncHazard::WRITE_AFTER_READ:
            vs_read = true;
            break;
        case SyncHazard::WRITE_RACING_READ:
            vs_read = true;
            break;
        case SyncHazard::PRESENT_AFTER_READ:
            vs_read = true;
            break;
        default:
            break;
    }
    return vs_read;
}

static VkPipelineStageFlags2 GetAllowedStages(VkQueueFlags queue_flags, VkPipelineStageFlagBits2 disabled_stages) {
    VkPipelineStageFlags2 allowed_stages = 0;
    for (const auto &[queue_flag, stages] : syncAllCommandStagesByQueueFlags()) {
        if (queue_flag & queue_flags) {
            allowed_stages |= (stages & ~disabled_stages);
        }
    }
    return allowed_stages;
}

static SyncAccessFlags FilterSyncAccessesByAllowedVkStages(const SyncAccessFlags &accesses, VkPipelineStageFlags2 allowed_stages) {
    SyncAccessFlags filtered_accesses = accesses;
    const auto &access_infos = syncAccessInfoByAccessIndex();
    for (size_t i = 0; i < access_infos.size(); i++) {
        const SyncAccessInfo &access_info = access_infos[i];
        const bool is_stage_allowed = (access_info.stage_mask & allowed_stages) != 0;
        if (!is_stage_allowed) {
            filtered_accesses.reset(i);
        }
    }
    return filtered_accesses;
}

static SyncAccessFlags FilterSyncAccessesByAllowedVkAccesses(const SyncAccessFlags &accesses, VkAccessFlags2 allowed_vk_accesses) {
    SyncAccessFlags filtered_accesses = accesses;
    const auto &access_infos = syncAccessInfoByAccessIndex();
    for (size_t i = 0; i < access_infos.size(); i++) {
        const SyncAccessInfo &access_info = access_infos[i];
        if (filtered_accesses[i]) {
            const bool is_access_allowed = (access_info.access_mask & allowed_vk_accesses) != 0;
            if (!is_access_allowed) {
                filtered_accesses.reset(i);
            }
        }
    }
    return filtered_accesses;
}

std::vector<std::pair<VkPipelineStageFlags2, VkAccessFlags2>> ConvertSyncAccessesToCompactVkForm(
    const SyncAccessFlags &sync_accesses, VkQueueFlags allowed_queue_flags, const DeviceFeatures &features,
    const DeviceExtensions &device_extensions) {
    if (sync_accesses.none()) {
        return {};
    }

    const VkPipelineStageFlags2 disabled_stages = sync_utils::DisabledPipelineStages(features, device_extensions);
    const VkPipelineStageFlags2 all_transfer_expand_bits = kAllTransferExpandBits & ~disabled_stages;

    // Build stage -> accesses mapping. OR-merge accesses that happen on the same stage.
    // Also handle ALL_COMMANDS accesses.
    vvl::unordered_map<VkPipelineStageFlagBits2, VkAccessFlags2> stage_to_accesses;
    {
        const VkPipelineStageFlags2 allowed_stages = GetAllowedStages(allowed_queue_flags, disabled_stages);
        const SyncAccessFlags filtered_accesses = FilterSyncAccessesByAllowedVkStages(sync_accesses, allowed_stages);

        const SyncAccessFlags all_reads = FilterSyncAccessesByAllowedVkStages(syncAccessReadMask, allowed_stages);
        const SyncAccessFlags all_shader_reads = FilterSyncAccessesByAllowedVkAccesses(all_reads, kShaderReadExpandBits);

        const SyncAccessFlags all_writes = FilterSyncAccessesByAllowedVkStages(syncAccessWriteMask, allowed_stages);
        const SyncAccessFlags all_shader_writes = FilterSyncAccessesByAllowedVkAccesses(all_writes, kShaderWriteExpandBits);

        if (filtered_accesses == all_reads) {
            stage_to_accesses[VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT] = VK_ACCESS_2_MEMORY_READ_BIT;
        } else if (filtered_accesses == all_shader_reads) {
            stage_to_accesses[VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT] = VK_ACCESS_2_SHADER_READ_BIT;
        } else if (filtered_accesses == all_shader_writes) {
            stage_to_accesses[VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT] = VK_ACCESS_2_SHADER_WRITE_BIT;
        } else {
            for (size_t i = 0; i < filtered_accesses.size(); i++) {
                if (filtered_accesses[i]) {
                    const SyncAccessInfo &info = syncAccessInfoByAccessIndex()[i];
                    stage_to_accesses[info.stage_mask] |= info.access_mask;
                }
            }
        }
    }

    // Build accesses -> stages mapping. OR-merge stages that share the same accesses
    vvl::unordered_map<VkAccessFlags2, VkPipelineStageFlags2> accesses_to_stages;
    for (const auto [stage, accesses] : stage_to_accesses) {
        accesses_to_stages[accesses] |= stage;
    }

    // Replace sequences of stages/accesses with more compact equivalent meta values where possible
    std::vector<std::pair<VkPipelineStageFlags2, VkAccessFlags2>> result;
    VkPipelineStageFlags2 stages_with_all_supported_accesses = 0;
    VkAccessFlags2 all_accesses = 0;  // accesses for the above stages

    for (const auto &entry : accesses_to_stages) {
        VkAccessFlags2 accesses = entry.first;
        VkPipelineStageFlags2 stages = entry.second;

        // Detect if ALL allowed accesses for the given stage are used.
        // This is an opportunity to use a compact message form.
        {
            VkAccessFlags2 all_supported_accesses = sync_utils::CompatibleAccessMask(stages);
            // Remove meta stages.
            // TODO: revisit CompatibleAccessMask helper. SyncVal works with expanded representation.
            // Meta stages are needed for core checks in this case, update function so serve both purposes well.
            all_supported_accesses &= ~VK_ACCESS_2_SHADER_READ_BIT;
            all_supported_accesses &= ~VK_ACCESS_2_SHADER_WRITE_BIT;
            // Remove unsupported accesses, otherwise the access mask won't be detected as the one that covers ALL accesses
            // TODO: ideally this should be integrated into utilities logic (need to revisit all use cases)
            if (!IsExtEnabled(device_extensions.vk_ext_blend_operation_advanced)) {
                all_supported_accesses &= ~VK_ACCESS_2_COLOR_ATTACHMENT_READ_NONCOHERENT_BIT_EXT;
            }
            if (accesses == all_supported_accesses) {
                stages_with_all_supported_accesses |= stages;
                all_accesses |= all_supported_accesses;
                continue;
            }
        }

        sync_utils::ReplaceExpandBitsWithMetaMask(stages, all_transfer_expand_bits, VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT);
        sync_utils::ReplaceExpandBitsWithMetaMask(accesses, kShaderReadExpandBits, VK_ACCESS_2_SHADER_READ_BIT);
        result.emplace_back(stages, accesses);
    }
    if (stages_with_all_supported_accesses) {
        if (IsSingleBitSet(stages_with_all_supported_accesses) && GetBitSetCount(all_accesses) <= 2) {
            // For simple configurations (1 stage and at most 2 accesses) don't use ALL accesses shortcut
            result.emplace_back(stages_with_all_supported_accesses, all_accesses);
        } else {
            sync_utils::ReplaceExpandBitsWithMetaMask(stages_with_all_supported_accesses, all_transfer_expand_bits,
                                                      VK_PIPELINE_STAGE_2_ALL_TRANSFER_BIT);
            result.emplace_back(stages_with_all_supported_accesses, sync_utils::kAllAccesses);
        }
    }
    return result;
}

std::string FormatSyncAccesses(const SyncAccessFlags &sync_accesses, VkQueueFlags allowed_queue_flags,
                               const DeviceFeatures &features, const DeviceExtensions &device_extensions,
                               bool format_as_extra_property) {
    const auto report_accesses =
        ConvertSyncAccessesToCompactVkForm(sync_accesses, allowed_queue_flags, features, device_extensions);
    if (report_accesses.empty()) {
        return "0";
    }
    std::stringstream out;
    bool first = true;
    for (const auto &[stages, accesses] : report_accesses) {
        if (!first) {
            out << ":";
        }
        if (format_as_extra_property) {
            if (accesses == sync_utils::kAllAccesses) {
                out << string_VkPipelineStageFlags2(stages) << "(ALL_ACCESSES)";
            } else {
                out << string_VkPipelineStageFlags2(stages) << "(" << string_VkAccessFlags2(accesses) << ")";
            }
        } else {
            if (accesses == sync_utils::kAllAccesses) {
                out << "all accesses at " << string_VkPipelineStageFlags2(stages);
            } else {
                out << string_VkAccessFlags2(accesses) << " accesses at " << string_VkPipelineStageFlags2(stages);
            }
        }
        first = false;
    }
    return out.str();
}

static std::string FormatAccessProperty(const SyncAccessInfo &access) {
    constexpr std::array special_accesses = {SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_ACQUIRE_READ_SYNCVAL,
                                             SYNC_PRESENT_ENGINE_SYNCVAL_PRESENT_PRESENTED_SYNCVAL, SYNC_IMAGE_LAYOUT_TRANSITION,
                                             SYNC_QUEUE_FAMILY_OWNERSHIP_TRANSFER};
    if (IsValueIn(access.access_index, special_accesses)) {
        // Print internal name for accesses that don't have corresponding Vulkan constants
        return access.name;
    }
    return string_VkPipelineStageFlagBits2(access.stage_mask) + std::string("(") + string_VkAccessFlagBits2(access.access_mask) +
           ")";
}

static std::string FormatHazardState(const HazardResult::HazardState &hazard, VkQueueFlags queue_flags,
                                     const DeviceFeatures &features, const DeviceExtensions &device_extensions,
                                     ReportKeyValues &key_values) {
    std::stringstream out;
    assert(hazard.access_index < static_cast<SyncAccessIndex>(syncAccessInfoByAccessIndex().size()));
    assert(hazard.prior_access_index < static_cast<SyncAccessIndex>(syncAccessInfoByAccessIndex().size()));
    const auto &usage_info = syncAccessInfoByAccessIndex()[hazard.access_index];
    const auto &prior_usage_info = syncAccessInfoByAccessIndex()[hazard.prior_access_index];
    out << "(";
    if (!hazard.recorded_access.get()) {
        // if we have a recorded usage the usage is reported from the recorded contexts point of view
        out << "usage: " << usage_info.name << ", ";
        key_values.Add(kPropertyAccess, FormatAccessProperty(usage_info));
    }
    out << "prior_usage: " << prior_usage_info.name;
    key_values.Add(kPropertyPriorAccess, FormatAccessProperty(prior_usage_info));
    if (IsHazardVsRead(hazard.hazard)) {
        const VkPipelineStageFlags2 barriers = hazard.access_state->GetReadBarriers(hazard.prior_access_index);
        const std::string barriers_str = string_VkPipelineStageFlags2(barriers);
        out << ", read_barriers: " << barriers_str;
        key_values.Add(kPropertyReadBarriers, barriers ? barriers_str : "0");
    } else {
        const SyncAccessFlags barriers = hazard.access_state->GetWriteBarriers();

        const std::string message_barriers_str = FormatSyncAccesses(barriers, queue_flags, features, device_extensions, false);
        out << ", write_barriers: " << message_barriers_str;

        const std::string property_barriers_str = FormatSyncAccesses(barriers, queue_flags, features, device_extensions, true);
        key_values.Add(kPropertyWriteBarriers, property_barriers_str);
    }
    return out.str();
}

std::string CommandExecutionContext::FormatHazard(const HazardResult &hazard, ReportKeyValues &key_values) const {
    std::stringstream out;
    assert(hazard.IsHazard());
    out << FormatHazardState(hazard.State(), queue_flags_, sync_state_.enabled_features, sync_state_.extensions, key_values);
    out << ", " << FormatUsage(hazard.TagEx(), key_values) << ")";
    return out.str();
}

ReportUsageInfo CommandBufferAccessContext::GetReportUsageInfo(ResourceUsageTagEx tag_ex) const {
    const ResourceUsageRecord &record = (*access_log_)[tag_ex.tag];
    ReportUsageInfo info;
    if (record.alt_usage) {
        info.command = record.alt_usage.GetCommand();
    } else {
        info.command = record.command;
    }
    return info;
}

std::string CommandBufferAccessContext::FormatUsage(ResourceUsageTagEx tag_ex, ReportKeyValues &extra_properties) const {
    if (tag_ex.tag >= access_log_->size()) return std::string();

    std::stringstream out;
    assert(tag_ex.tag < access_log_->size());
    const auto &record = (*access_log_)[tag_ex.tag];
    const auto debug_name_provider = (record.label_command_index == vvl::kU32Max) ? nullptr : this;
    out << FormatResourceUsageRecord(record.Formatter(sync_state_, cb_state_, debug_name_provider, tag_ex.handle_index),
                                     extra_properties);
    return out.str();
}

void CommandBufferAccessContext::AddUsageRecordExtraProperties(ResourceUsageTag tag, ReportKeyValues &extra_properties) const {
    if (tag >= access_log_->size()) return;
    const ResourceUsageRecord &record = (*access_log_)[tag];
    extra_properties.Add(kPropertyPriorCommand, vvl::String(record.command));
    extra_properties.Add(kPropertySeqNo, record.seq_num);
    if (record.sub_command != 0) {
        extra_properties.Add(kPropertySubCmd, record.sub_command);
    }
    extra_properties.Add(kPropertyResetNo, record.reset_count);
}

std::string QueueBatchContext::FormatUsage(ResourceUsageTagEx tag_ex, ReportKeyValues &extra_properties) const {
    std::stringstream out;
    BatchAccessLog::AccessRecord access = batch_log_.GetAccessRecord(tag_ex.tag);
    if (access.IsValid()) {
        const BatchAccessLog::BatchRecord &batch = *access.batch;
        const ResourceUsageRecord &record = *access.record;
        if (batch.queue) {
            // Queue and Batch information (for enqueued operations)
            out << FormatStateObject(SyncNodeFormatter(sync_state_, batch.queue->GetQueueState()));
            out << ", submit: " << batch.submit_index << ", batch: " << batch.batch_index << ", ";
        }
        out << FormatResourceUsageRecord(record.Formatter(sync_state_, nullptr, access.debug_name_provider, tag_ex.handle_index),
                                         extra_properties);
    }
    return out.str();
}

void QueueBatchContext::AddUsageRecordExtraProperties(ResourceUsageTag tag, ReportKeyValues &extra_properties) const {
    BatchAccessLog::AccessRecord access = batch_log_.GetAccessRecord(tag);
    if (access.IsValid()) {
        extra_properties.Add(kPropertyBatchTag, access.batch->base_tag);
    }
}
