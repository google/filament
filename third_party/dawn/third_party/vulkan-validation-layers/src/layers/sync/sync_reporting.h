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

#pragma once

#include "generated/error_location_helper.h"
#include "generated/sync_validation_types.h"

namespace vvl {
class CommandBuffer;
class StateObject;
class Image;
class Queue;
}  // namespace vvl
class DebugReport;
class SyncValidator;
struct DeviceFeatures;
struct DeviceExtensions;

struct SyncNodeFormatter {
    const DebugReport *debug_report;
    const vvl::StateObject *node;
    const char *label;

    SyncNodeFormatter(const SyncValidator &sync_state, const vvl::CommandBuffer *cb_state);
    SyncNodeFormatter(const SyncValidator &sync_state, const vvl::Image *image);
    SyncNodeFormatter(const SyncValidator &sync_state, const vvl::Queue *q_state);
    SyncNodeFormatter(const SyncValidator &sync_state, const vvl::StateObject *state_object, const char *label_ = nullptr);
};
std::string FormatStateObject(const SyncNodeFormatter &formatter);

struct ReportKeyValues {
    struct KeyValue {
        std::string key;
        std::string value;
    };
    std::vector<KeyValue> key_values;

    void Add(std::string_view key, std::string_view value);
    void Add(std::string_view key, uint64_t value);

    std::string GetExtraPropertiesSection(bool pretty_print) const;
    const std::string *FindProperty(const std::string &key) const;
};

struct ReportUsageInfo {
    vvl::Func command = vvl::Func::Empty;
};

std::vector<std::pair<VkPipelineStageFlags2, VkAccessFlags2>> ConvertSyncAccessesToCompactVkForm(
    const SyncAccessFlags &sync_accesses, VkQueueFlags allowed_queue_flags, const DeviceFeatures &features,
    const DeviceExtensions &device_extensions);

std::string FormatSyncAccesses(const SyncAccessFlags &sync_accesses, VkQueueFlags allowed_queue_flags,
                               const DeviceFeatures &features, const DeviceExtensions &device_extensions,
                               bool format_as_extra_property);

inline constexpr const char *kPropertyMessageType = "message_type";
inline constexpr const char *kPropertyHazardType = "hazard_type";
inline constexpr const char *kPropertyCommand = "command";
inline constexpr const char *kPropertyPriorCommand = "prior_command";
inline constexpr const char *kPropertyDebugRegion = "debug_region";
inline constexpr const char *kPropertyAccess = "access";
inline constexpr const char *kPropertyPriorAccess = "prior_access";
inline constexpr const char *kPropertyReadBarriers = "read_barriers";
inline constexpr const char *kPropertyWriteBarriers = "write_barriers";
inline constexpr const char *kPropertyCopyRegion = "copy_region";
inline constexpr const char *kPropertyLoadOp = "load_op";
inline constexpr const char *kPropertyStoreOp = "store_op";
inline constexpr const char *kPropertyResolveMode = "resolve_mode";
inline constexpr const char *kPropertyOldLayout = "old_layout";
inline constexpr const char *kPropertyNewLayout = "new_layout";
inline constexpr const char *kPropertyDescriptorType = "descriptor_type";
inline constexpr const char *kPropertyImageLayout = "image_layout";
inline constexpr const char *kPropertyImageAspect = "image_aspect";

// debug properties
inline constexpr const char *kPropertySeqNo = "seq_no";
inline constexpr const char *kPropertySubCmd = "subcmd";
inline constexpr const char *kPropertyResetNo = "reset_no";
inline constexpr const char *kPropertyBatchTag = "batch_tag";
