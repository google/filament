/* Copyright (c) 2025 Valve Corporation
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

#include "ray_tracing_utils.h"

#include "generated/dispatch_functions.h"
#include "containers/custom_containers.h"

#include <cassert>
#include <vector>
#include <vulkan/utility/vk_struct_helper.hpp>

namespace rt {

static VkAccelerationStructureBuildSizesInfoKHR ComputeBuildSizes(const VkDevice device,
                                                                  const VkAccelerationStructureBuildTypeKHR build_type,
                                                                  const VkAccelerationStructureBuildGeometryInfoKHR &build_info,
                                                                  const VkAccelerationStructureBuildRangeInfoKHR *range_infos) {
    std::vector<uint32_t> primitive_counts(build_info.geometryCount);
    for (const auto [i, build_range] : vvl::enumerate(range_infos, build_info.geometryCount)) {
        primitive_counts[i] = build_range.primitiveCount;
    }
    VkAccelerationStructureBuildSizesInfoKHR size_info = vku::InitStructHelper();
    DispatchGetAccelerationStructureBuildSizesKHR(device, build_type, &build_info, primitive_counts.data(), &size_info);

    return size_info;
}

VkDeviceSize ComputeScratchSize(BuildType build_type, const VkDevice device,
                                const VkAccelerationStructureBuildGeometryInfoKHR &build_info,
                                const VkAccelerationStructureBuildRangeInfoKHR *range_infos) {
    const VkAccelerationStructureBuildSizesInfoKHR size_info =
        ComputeBuildSizes(device,
                          build_type == BuildType::Device ? VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR
                                                          : VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR,
                          build_info, range_infos);
    switch (build_info.mode) {
        case VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR:
            return size_info.buildScratchSize;
        case VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR:
            return size_info.updateScratchSize;
        default:
            assert(false);
            return static_cast<VkDeviceSize>(0);
            break;
    }
}

VkDeviceSize ComputeAccelerationStructureSize(BuildType build_type, const VkDevice device,
                                              const VkAccelerationStructureBuildGeometryInfoKHR &build_info,
                                              const VkAccelerationStructureBuildRangeInfoKHR *range_infos) {
    const VkAccelerationStructureBuildSizesInfoKHR size_info =
        ComputeBuildSizes(device,
                          build_type == BuildType::Device ? VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR
                                                          : VK_ACCELERATION_STRUCTURE_BUILD_TYPE_HOST_KHR,
                          build_info, range_infos);
    return size_info.accelerationStructureSize;
}

}  // namespace rt
