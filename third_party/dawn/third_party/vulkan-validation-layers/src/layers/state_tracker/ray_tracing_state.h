/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
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
#include "state_tracker/device_memory_state.h"
#include "state_tracker/buffer_state.h"
#include "generated/dispatch_functions.h"

namespace vvl {

class AccelerationStructureNV : public Bindable {
  public:
    AccelerationStructureNV(VkDevice device, VkAccelerationStructureNV handle,
                            const VkAccelerationStructureCreateInfoNV *pCreateInfo)
        : Bindable(handle, kVulkanObjectTypeAccelerationStructureNV, false, false, 0),
          safe_create_info(pCreateInfo),
          create_info(*safe_create_info.ptr()),
          memory_requirements(GetMemReqs(device, handle, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV)),
          build_scratch_memory_requirements(
              GetMemReqs(device, handle, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV)),
          update_scratch_memory_requirements(
              GetMemReqs(device, handle, VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_UPDATE_SCRATCH_NV)),
          tracker_(&memory_requirements) {
        Bindable::SetMemoryTracker(&tracker_);
    }
    AccelerationStructureNV(const AccelerationStructureNV &rh_obj) = delete;

    VkAccelerationStructureNV VkHandle() const { return handle_.Cast<VkAccelerationStructureNV>(); }

    void Build(const VkAccelerationStructureInfoNV *pInfo) {
        built = true;
        build_info.initialize(pInfo);
    };

    const vku::safe_VkAccelerationStructureCreateInfoNV safe_create_info;
    const VkAccelerationStructureCreateInfoNV &create_info;

    vku::safe_VkAccelerationStructureInfoNV build_info;
    const VkMemoryRequirements memory_requirements;
    const VkMemoryRequirements build_scratch_memory_requirements;
    const VkMemoryRequirements update_scratch_memory_requirements;
    uint64_t opaque_handle = 0;
    bool memory_requirements_checked = false;
    bool build_scratch_memory_requirements_checked = false;
    bool update_scratch_memory_requirements_checked = false;
    bool built = false;

  private:
    static VkMemoryRequirements GetMemReqs(VkDevice device, VkAccelerationStructureNV as,
                                           VkAccelerationStructureMemoryRequirementsTypeNV mem_type) {
        VkAccelerationStructureMemoryRequirementsInfoNV req_info = vku::InitStructHelper();
        req_info.type = mem_type;
        req_info.accelerationStructure = as;
        VkMemoryRequirements2 requirements = vku::InitStructHelper();
        DispatchGetAccelerationStructureMemoryRequirementsNV(device, &req_info, &requirements);
        return requirements.memoryRequirements;
    }
    BindableLinearMemoryTracker tracker_;
};

class AccelerationStructureKHR : public StateObject {
  public:
    AccelerationStructureKHR(VkAccelerationStructureKHR handle, const VkAccelerationStructureCreateInfoKHR *pCreateInfo,
                             std::shared_ptr<Buffer> &&buf_state)
        : StateObject(handle, kVulkanObjectTypeAccelerationStructureKHR),
          safe_create_info(pCreateInfo),
          create_info(*safe_create_info.ptr()),
          buffer_state(buf_state) {}
    AccelerationStructureKHR(const AccelerationStructureKHR &rh_obj) = delete;

    virtual ~AccelerationStructureKHR() {
        if (!Destroyed()) {
            Destroy();
        }
    }

    VkAccelerationStructureKHR VkHandle() const { return handle_.Cast<VkAccelerationStructureKHR>(); }

    void LinkChildNodes() override {
        // Connect child node(s), which cannot safely be done in the constructor.
        buffer_state->AddParent(this);
    }

    void Destroy() override {
        if (buffer_state) {
            buffer_state->RemoveParent(this);
            buffer_state = nullptr;
        }
        StateObject::Destroy();
    }

    void Build(const VkAccelerationStructureBuildGeometryInfoKHR *pInfo, const bool is_host,
               const VkAccelerationStructureBuildRangeInfoKHR *build_range_info) {
        is_built = true;
        if (!build_info_khr.has_value()) {
            build_info_khr = vku::safe_VkAccelerationStructureBuildGeometryInfoKHR();
        }
        build_info_khr->initialize(pInfo, is_host, build_range_info);
    };

    void UpdateBuildRangeInfos(const VkAccelerationStructureBuildRangeInfoKHR *p_build_range_infos, uint32_t geometry_count) {
        build_range_infos.resize(geometry_count);
        for (const auto [i, build_range] : vvl::enumerate(p_build_range_infos, geometry_count)) {
            build_range_infos[i] = build_range;
        }
    }

    const vku::safe_VkAccelerationStructureCreateInfoKHR safe_create_info;
    const VkAccelerationStructureCreateInfoKHR &create_info;

    uint64_t opaque_handle = 0;
    std::shared_ptr<vvl::Buffer> buffer_state{};
    std::optional<vku::safe_VkAccelerationStructureBuildGeometryInfoKHR> build_info_khr{};
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> build_range_infos{};
    // You can't have is_built == false and a build_info_khr, but you can have is_built == true and no build_info_khr,
    // if the acceleration structure was filled by a call to vkCmdCopyMemoryToAccelerationStructure
    bool is_built = false;
};

}  // namespace vvl
