/* Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
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

#include <vulkan/vulkan_core.h>

namespace rt {

enum class BuildType { Device, Host };

// Compute scratch buffer size the idiomatic way.
// Note: range_infos must be an array of build_info.geometryCount elements
VkDeviceSize ComputeScratchSize(BuildType build_type, const VkDevice device,
                                const VkAccelerationStructureBuildGeometryInfoKHR &build_info,
                                const VkAccelerationStructureBuildRangeInfoKHR *range_infos);

// Compute acceleration structure size the idiomatic way.
// Note: range_infos must be an array of build_info.geometryCount elements
VkDeviceSize ComputeAccelerationStructureSize(BuildType build_type, const VkDevice device,
                                              const VkAccelerationStructureBuildGeometryInfoKHR &build_info,
                                              const VkAccelerationStructureBuildRangeInfoKHR *range_infos);

inline const VkAccelerationStructureGeometryKHR &GetGeometry(const VkAccelerationStructureBuildGeometryInfoKHR &info,
                                                             uint32_t geometry_i) {
    return info.pGeometries ? info.pGeometries[geometry_i] : *info.ppGeometries[geometry_i];
}
}  // namespace rt
