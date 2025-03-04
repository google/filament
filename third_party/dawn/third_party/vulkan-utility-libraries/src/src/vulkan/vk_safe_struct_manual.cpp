/***************************************************************************
 *
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

#include <vulkan/utility/vk_safe_struct.hpp>
#include <vulkan/utility/vk_struct_helper.hpp>
#include <vulkan/utility/vk_concurrent_unordered_map.hpp>

#include <cassert>
#include <cstring>

namespace vku {

std::vector<std::pair<uint32_t, uint32_t>>& GetCustomStypeInfo() {
    static std::vector<std::pair<uint32_t, uint32_t>> custom_stype_info{};
    return custom_stype_info;
}

struct ASGeomKHRExtraData {
    ASGeomKHRExtraData(uint8_t* alloc, uint32_t primOffset, uint32_t primCount)
        : ptr(alloc), primitiveOffset(primOffset), primitiveCount(primCount) {}
    ~ASGeomKHRExtraData() {
        if (ptr) delete[] ptr;
    }
    uint8_t* ptr;
    uint32_t primitiveOffset;
    uint32_t primitiveCount;
};

vku::concurrent::unordered_map<const safe_VkAccelerationStructureGeometryKHR*, ASGeomKHRExtraData*, 4>&
GetAccelStructGeomHostAllocMap() {
    static vku::concurrent::unordered_map<const safe_VkAccelerationStructureGeometryKHR*, ASGeomKHRExtraData*, 4>
        as_geom_khr_host_alloc;
    return as_geom_khr_host_alloc;
}

safe_VkAccelerationStructureGeometryKHR::safe_VkAccelerationStructureGeometryKHR(
    const VkAccelerationStructureGeometryKHR* in_struct, const bool is_host,
    const VkAccelerationStructureBuildRangeInfoKHR* build_range_info, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), geometryType(in_struct->geometryType), geometry(in_struct->geometry), flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
        if (geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
            geometry.instances.pNext = SafePnextCopy(in_struct->geometry.instances.pNext, copy_state);
        } else if (geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
            geometry.triangles.pNext = SafePnextCopy(in_struct->geometry.triangles.pNext, copy_state);
        } else if (geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
            geometry.aabbs.pNext = SafePnextCopy(in_struct->geometry.aabbs.pNext, copy_state);
        }
    }
    if (is_host && geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
        if (geometry.instances.arrayOfPointers) {
            size_t pp_array_size = build_range_info->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR*);
            size_t p_array_size = build_range_info->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);
            size_t array_size = build_range_info->primitiveOffset + pp_array_size + p_array_size;
            uint8_t* allocation = new uint8_t[array_size];
            VkAccelerationStructureInstanceKHR** ppInstances =
                reinterpret_cast<VkAccelerationStructureInstanceKHR**>(allocation + build_range_info->primitiveOffset);
            VkAccelerationStructureInstanceKHR* pInstances = reinterpret_cast<VkAccelerationStructureInstanceKHR*>(
                allocation + build_range_info->primitiveOffset + pp_array_size);
            for (uint32_t i = 0; i < build_range_info->primitiveCount; ++i) {
                const uint8_t* byte_ptr = reinterpret_cast<const uint8_t*>(in_struct->geometry.instances.data.hostAddress);
                pInstances[i] = *(
                    reinterpret_cast<VkAccelerationStructureInstanceKHR* const*>(byte_ptr + build_range_info->primitiveOffset)[i]);
                ppInstances[i] = &pInstances[i];
            }
            geometry.instances.data.hostAddress = allocation;
            GetAccelStructGeomHostAllocMap().insert(
                this, new ASGeomKHRExtraData(allocation, build_range_info->primitiveOffset, build_range_info->primitiveCount));
        } else {
            const auto primitive_offset = build_range_info->primitiveOffset;
            const auto primitive_count = build_range_info->primitiveCount;
            size_t array_size = primitive_offset + primitive_count * sizeof(VkAccelerationStructureInstanceKHR);
            uint8_t* allocation = new uint8_t[array_size];
            auto host_address = static_cast<const uint8_t*>(in_struct->geometry.instances.data.hostAddress);
            memcpy(allocation + primitive_offset, host_address + primitive_offset,
                   primitive_count * sizeof(VkAccelerationStructureInstanceKHR));
            geometry.instances.data.hostAddress = allocation;
            GetAccelStructGeomHostAllocMap().insert(
                this, new ASGeomKHRExtraData(allocation, build_range_info->primitiveOffset, build_range_info->primitiveCount));
        }
    }
}

safe_VkAccelerationStructureGeometryKHR::safe_VkAccelerationStructureGeometryKHR()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR), pNext(nullptr), geometryType(), geometry(), flags() {}

safe_VkAccelerationStructureGeometryKHR::safe_VkAccelerationStructureGeometryKHR(
    const safe_VkAccelerationStructureGeometryKHR& copy_src) {
    sType = copy_src.sType;
    geometryType = copy_src.geometryType;
    geometry = copy_src.geometry;
    flags = copy_src.flags;

    pNext = SafePnextCopy(copy_src.pNext);
    if (geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
        geometry.instances.pNext = SafePnextCopy(copy_src.geometry.instances.pNext);
    } else if (geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
        geometry.triangles.pNext = SafePnextCopy(copy_src.geometry.triangles.pNext);
    } else if (geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
        geometry.aabbs.pNext = SafePnextCopy(copy_src.geometry.aabbs.pNext);
    }
    auto src_iter = GetAccelStructGeomHostAllocMap().find(&copy_src);
    if (src_iter != GetAccelStructGeomHostAllocMap().end()) {
        auto& src_alloc = src_iter->second;
        if (geometry.instances.arrayOfPointers) {
            size_t pp_array_size = src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR*);
            size_t p_array_size = src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);
            size_t array_size = src_alloc->primitiveOffset + pp_array_size + p_array_size;
            uint8_t* allocation = new uint8_t[array_size];
            VkAccelerationStructureInstanceKHR** ppInstances =
                reinterpret_cast<VkAccelerationStructureInstanceKHR**>(allocation + src_alloc->primitiveOffset);
            VkAccelerationStructureInstanceKHR* pInstances =
                reinterpret_cast<VkAccelerationStructureInstanceKHR*>(allocation + src_alloc->primitiveOffset + pp_array_size);
            for (uint32_t i = 0; i < src_alloc->primitiveCount; ++i) {
                pInstances[i] =
                    *(reinterpret_cast<VkAccelerationStructureInstanceKHR* const*>(src_alloc->ptr + src_alloc->primitiveOffset)[i]);
                ppInstances[i] = &pInstances[i];
            }
            geometry.instances.data.hostAddress = allocation;
            GetAccelStructGeomHostAllocMap().insert(
                this, new ASGeomKHRExtraData(allocation, src_alloc->primitiveOffset, src_alloc->primitiveCount));
        } else {
            size_t array_size = src_alloc->primitiveOffset + src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);
            uint8_t* allocation = new uint8_t[array_size];
            memcpy(allocation, src_alloc->ptr, array_size);
            geometry.instances.data.hostAddress = allocation;
            GetAccelStructGeomHostAllocMap().insert(
                this, new ASGeomKHRExtraData(allocation, src_alloc->primitiveOffset, src_alloc->primitiveCount));
        }
    }
}

safe_VkAccelerationStructureGeometryKHR& safe_VkAccelerationStructureGeometryKHR::operator=(
    const safe_VkAccelerationStructureGeometryKHR& copy_src) {
    if (&copy_src == this) return *this;

    auto iter = GetAccelStructGeomHostAllocMap().pop(this);
    if (iter != GetAccelStructGeomHostAllocMap().end()) {
        delete iter->second;
    }
    FreePnextChain(pNext);
    if (geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
        FreePnextChain(geometry.instances.pNext);
    } else if (geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
        FreePnextChain(geometry.triangles.pNext);
    } else if (geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
        FreePnextChain(geometry.aabbs.pNext);
    }

    sType = copy_src.sType;
    geometryType = copy_src.geometryType;
    geometry = copy_src.geometry;
    flags = copy_src.flags;

    pNext = SafePnextCopy(copy_src.pNext);
    if (geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
        geometry.instances.pNext = SafePnextCopy(copy_src.geometry.instances.pNext);
    } else if (geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
        geometry.triangles.pNext = SafePnextCopy(copy_src.geometry.triangles.pNext);
    } else if (geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
        geometry.aabbs.pNext = SafePnextCopy(copy_src.geometry.aabbs.pNext);
    }
    auto src_iter = GetAccelStructGeomHostAllocMap().find(&copy_src);
    if (src_iter != GetAccelStructGeomHostAllocMap().end()) {
        auto& src_alloc = src_iter->second;
        if (geometry.instances.arrayOfPointers) {
            size_t pp_array_size = src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR*);
            size_t p_array_size = src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);
            size_t array_size = src_alloc->primitiveOffset + pp_array_size + p_array_size;
            uint8_t* allocation = new uint8_t[array_size];
            VkAccelerationStructureInstanceKHR** ppInstances =
                reinterpret_cast<VkAccelerationStructureInstanceKHR**>(allocation + src_alloc->primitiveOffset);
            VkAccelerationStructureInstanceKHR* pInstances =
                reinterpret_cast<VkAccelerationStructureInstanceKHR*>(allocation + src_alloc->primitiveOffset + pp_array_size);
            for (uint32_t i = 0; i < src_alloc->primitiveCount; ++i) {
                pInstances[i] =
                    *(reinterpret_cast<VkAccelerationStructureInstanceKHR* const*>(src_alloc->ptr + src_alloc->primitiveOffset)[i]);
                ppInstances[i] = &pInstances[i];
            }
            geometry.instances.data.hostAddress = allocation;
            GetAccelStructGeomHostAllocMap().insert(
                this, new ASGeomKHRExtraData(allocation, src_alloc->primitiveOffset, src_alloc->primitiveCount));
        } else {
            size_t array_size = src_alloc->primitiveOffset + src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);
            uint8_t* allocation = new uint8_t[array_size];
            memcpy(allocation, src_alloc->ptr, array_size);
            geometry.instances.data.hostAddress = allocation;
            GetAccelStructGeomHostAllocMap().insert(
                this, new ASGeomKHRExtraData(allocation, src_alloc->primitiveOffset, src_alloc->primitiveCount));
        }
    }

    return *this;
}

safe_VkAccelerationStructureGeometryKHR::~safe_VkAccelerationStructureGeometryKHR() {
    auto iter = GetAccelStructGeomHostAllocMap().pop(this);
    if (iter != GetAccelStructGeomHostAllocMap().end()) {
        delete iter->second;
    }
    FreePnextChain(pNext);
    if (geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
        FreePnextChain(geometry.instances.pNext);
    } else if (geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
        FreePnextChain(geometry.triangles.pNext);
    } else if (geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
        FreePnextChain(geometry.aabbs.pNext);
    }
}

void safe_VkAccelerationStructureGeometryKHR::initialize(const VkAccelerationStructureGeometryKHR* in_struct, const bool is_host,
                                                         const VkAccelerationStructureBuildRangeInfoKHR* build_range_info,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    auto iter = GetAccelStructGeomHostAllocMap().pop(this);
    if (iter != GetAccelStructGeomHostAllocMap().end()) {
        delete iter->second;
    }
    FreePnextChain(pNext);
    if (geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
        FreePnextChain(geometry.instances.pNext);
    } else if (geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
        FreePnextChain(geometry.triangles.pNext);
    } else if (geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
        FreePnextChain(geometry.aabbs.pNext);
    }
    sType = in_struct->sType;
    geometryType = in_struct->geometryType;
    geometry = in_struct->geometry;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (is_host && geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
        if (geometry.instances.arrayOfPointers) {
            size_t pp_array_size = build_range_info->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR*);
            size_t p_array_size = build_range_info->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);
            size_t array_size = build_range_info->primitiveOffset + pp_array_size + p_array_size;
            uint8_t* allocation = new uint8_t[array_size];
            VkAccelerationStructureInstanceKHR** ppInstances =
                reinterpret_cast<VkAccelerationStructureInstanceKHR**>(allocation + build_range_info->primitiveOffset);
            VkAccelerationStructureInstanceKHR* pInstances = reinterpret_cast<VkAccelerationStructureInstanceKHR*>(
                allocation + build_range_info->primitiveOffset + pp_array_size);
            for (uint32_t i = 0; i < build_range_info->primitiveCount; ++i) {
                const uint8_t* byte_ptr = reinterpret_cast<const uint8_t*>(in_struct->geometry.instances.data.hostAddress);
                pInstances[i] = *(
                    reinterpret_cast<VkAccelerationStructureInstanceKHR* const*>(byte_ptr + build_range_info->primitiveOffset)[i]);
                ppInstances[i] = &pInstances[i];
            }
            geometry.instances.data.hostAddress = allocation;
            GetAccelStructGeomHostAllocMap().insert(
                this, new ASGeomKHRExtraData(allocation, build_range_info->primitiveOffset, build_range_info->primitiveCount));
        } else {
            const auto primitive_offset = build_range_info->primitiveOffset;
            const auto primitive_count = build_range_info->primitiveCount;
            size_t array_size = primitive_offset + primitive_count * sizeof(VkAccelerationStructureInstanceKHR);
            uint8_t* allocation = new uint8_t[array_size];
            auto host_address = static_cast<const uint8_t*>(in_struct->geometry.instances.data.hostAddress);
            memcpy(allocation + primitive_offset, host_address + primitive_offset,
                   primitive_count * sizeof(VkAccelerationStructureInstanceKHR));
            geometry.instances.data.hostAddress = allocation;
            GetAccelStructGeomHostAllocMap().insert(
                this, new ASGeomKHRExtraData(allocation, build_range_info->primitiveOffset, build_range_info->primitiveCount));
        }
    }
}

void safe_VkAccelerationStructureGeometryKHR::initialize(const safe_VkAccelerationStructureGeometryKHR* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    geometryType = copy_src->geometryType;
    geometry = copy_src->geometry;
    flags = copy_src->flags;

    pNext = SafePnextCopy(copy_src->pNext);
    if (geometryType == VK_GEOMETRY_TYPE_INSTANCES_KHR) {
        geometry.instances.pNext = SafePnextCopy(copy_src->geometry.instances.pNext, copy_state);
    } else if (geometryType == VK_GEOMETRY_TYPE_TRIANGLES_KHR) {
        geometry.triangles.pNext = SafePnextCopy(copy_src->geometry.triangles.pNext, copy_state);
    } else if (geometryType == VK_GEOMETRY_TYPE_AABBS_KHR) {
        geometry.aabbs.pNext = SafePnextCopy(copy_src->geometry.aabbs.pNext, copy_state);
    }
    auto src_iter = GetAccelStructGeomHostAllocMap().find(copy_src);
    if (src_iter != GetAccelStructGeomHostAllocMap().end()) {
        auto& src_alloc = src_iter->second;
        if (geometry.instances.arrayOfPointers) {
            size_t pp_array_size = src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR*);
            size_t p_array_size = src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);
            size_t array_size = src_alloc->primitiveOffset + pp_array_size + p_array_size;
            uint8_t* allocation = new uint8_t[array_size];
            VkAccelerationStructureInstanceKHR** ppInstances =
                reinterpret_cast<VkAccelerationStructureInstanceKHR**>(allocation + src_alloc->primitiveOffset);
            VkAccelerationStructureInstanceKHR* pInstances =
                reinterpret_cast<VkAccelerationStructureInstanceKHR*>(allocation + src_alloc->primitiveOffset + pp_array_size);
            for (uint32_t i = 0; i < src_alloc->primitiveCount; ++i) {
                pInstances[i] =
                    *(reinterpret_cast<VkAccelerationStructureInstanceKHR* const*>(src_alloc->ptr + src_alloc->primitiveOffset)[i]);
                ppInstances[i] = &pInstances[i];
            }
            geometry.instances.data.hostAddress = allocation;
            GetAccelStructGeomHostAllocMap().insert(
                this, new ASGeomKHRExtraData(allocation, src_alloc->primitiveOffset, src_alloc->primitiveCount));
        } else {
            size_t array_size = src_alloc->primitiveOffset + src_alloc->primitiveCount * sizeof(VkAccelerationStructureInstanceKHR);
            uint8_t* allocation = new uint8_t[array_size];
            memcpy(allocation, src_alloc->ptr, array_size);
            geometry.instances.data.hostAddress = allocation;
            GetAccelStructGeomHostAllocMap().insert(
                this, new ASGeomKHRExtraData(allocation, src_alloc->primitiveOffset, src_alloc->primitiveCount));
        }
    }
}

void safe_VkRayTracingPipelineCreateInfoCommon::initialize(const VkRayTracingPipelineCreateInfoNV* pCreateInfo) {
    safe_VkRayTracingPipelineCreateInfoNV nvStruct;
    nvStruct.initialize(pCreateInfo);

    sType = nvStruct.sType;

    // Take ownership of the pointer and null it out in nvStruct
    pNext = nvStruct.pNext;
    nvStruct.pNext = nullptr;

    flags = nvStruct.flags;
    stageCount = nvStruct.stageCount;

    pStages = nvStruct.pStages;
    nvStruct.pStages = nullptr;

    groupCount = nvStruct.groupCount;
    maxRecursionDepth = nvStruct.maxRecursionDepth;
    layout = nvStruct.layout;
    basePipelineHandle = nvStruct.basePipelineHandle;
    basePipelineIndex = nvStruct.basePipelineIndex;

    assert(pGroups == nullptr);
    if (nvStruct.groupCount && nvStruct.pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoKHR[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].sType = nvStruct.pGroups[i].sType;
            pGroups[i].pNext = nvStruct.pGroups[i].pNext;
            pGroups[i].type = nvStruct.pGroups[i].type;
            pGroups[i].generalShader = nvStruct.pGroups[i].generalShader;
            pGroups[i].closestHitShader = nvStruct.pGroups[i].closestHitShader;
            pGroups[i].anyHitShader = nvStruct.pGroups[i].anyHitShader;
            pGroups[i].intersectionShader = nvStruct.pGroups[i].intersectionShader;
            pGroups[i].intersectionShader = nvStruct.pGroups[i].intersectionShader;
            pGroups[i].pShaderGroupCaptureReplayHandle = nullptr;
        }
    }
}

void safe_VkRayTracingPipelineCreateInfoCommon::initialize(const VkRayTracingPipelineCreateInfoKHR* pCreateInfo) {
    safe_VkRayTracingPipelineCreateInfoKHR::initialize(pCreateInfo);
}

safe_VkGraphicsPipelineCreateInfo::safe_VkGraphicsPipelineCreateInfo(const VkGraphicsPipelineCreateInfo* in_struct,
                                                                     const bool uses_color_attachment,
                                                                     const bool uses_depthstencil_attachment,
                                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      stageCount(in_struct->stageCount),
      pStages(nullptr),
      pVertexInputState(nullptr),
      pInputAssemblyState(nullptr),
      pTessellationState(nullptr),
      pViewportState(nullptr),
      pRasterizationState(nullptr),
      pMultisampleState(nullptr),
      pDepthStencilState(nullptr),
      pColorBlendState(nullptr),
      pDynamicState(nullptr),
      layout(in_struct->layout),
      renderPass(in_struct->renderPass),
      subpass(in_struct->subpass),
      basePipelineHandle(in_struct->basePipelineHandle),
      basePipelineIndex(in_struct->basePipelineIndex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    const bool is_graphics_library =
        vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(in_struct->pNext) != nullptr;
    if (stageCount && in_struct->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&in_struct->pStages[i]);
        }
    }
    if (in_struct->pVertexInputState)
        pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(in_struct->pVertexInputState);
    else
        pVertexInputState = nullptr;
    if (in_struct->pInputAssemblyState)
        pInputAssemblyState = new safe_VkPipelineInputAssemblyStateCreateInfo(in_struct->pInputAssemblyState);
    else
        pInputAssemblyState = nullptr;
    bool has_tessellation_stage = false;
    if (stageCount && pStages) {
        for (uint32_t i = 0; i < stageCount; ++i) {
            if (pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
                pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
                has_tessellation_stage = true;
        }
    }
    if (in_struct->pTessellationState && has_tessellation_stage)
        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(in_struct->pTessellationState);
    else
        pTessellationState = nullptr;  // original pTessellationState pointer ignored
    bool is_dynamic_has_rasterization = false;
    if (in_struct->pDynamicState && in_struct->pDynamicState->pDynamicStates) {
        for (uint32_t i = 0; i < in_struct->pDynamicState->dynamicStateCount && !is_dynamic_has_rasterization; ++i)
            if (in_struct->pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT)
                is_dynamic_has_rasterization = true;
    }
    // No pRasterizationState is same as dynamic, we assume rasterizerDiscardEnable is false
    const bool has_rasterization = is_dynamic_has_rasterization ||
                                   (!in_struct->pRasterizationState || !in_struct->pRasterizationState->rasterizerDiscardEnable);
    if (in_struct->pViewportState && (has_rasterization || is_graphics_library)) {
        bool is_dynamic_viewports = false;
        bool is_dynamic_scissors = false;
        if (in_struct->pDynamicState && in_struct->pDynamicState->pDynamicStates) {
            for (uint32_t i = 0; i < in_struct->pDynamicState->dynamicStateCount && !is_dynamic_viewports; ++i)
                if (in_struct->pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_VIEWPORT) is_dynamic_viewports = true;
            for (uint32_t i = 0; i < in_struct->pDynamicState->dynamicStateCount && !is_dynamic_scissors; ++i)
                if (in_struct->pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_SCISSOR) is_dynamic_scissors = true;
        }
        pViewportState =
            new safe_VkPipelineViewportStateCreateInfo(in_struct->pViewportState, is_dynamic_viewports, is_dynamic_scissors);
    } else
        pViewportState = nullptr;  // original pViewportState pointer ignored
    if (in_struct->pRasterizationState)
        pRasterizationState = new safe_VkPipelineRasterizationStateCreateInfo(in_struct->pRasterizationState);
    else
        pRasterizationState = nullptr;
    if (in_struct->pMultisampleState && (has_rasterization || is_graphics_library))
        pMultisampleState = new safe_VkPipelineMultisampleStateCreateInfo(in_struct->pMultisampleState);
    else
        pMultisampleState = nullptr;  // original pMultisampleState pointer ignored
    // needs a tracked subpass state uses_depthstencil_attachment
    if (in_struct->pDepthStencilState && ((has_rasterization && uses_depthstencil_attachment) || is_graphics_library))
        pDepthStencilState = new safe_VkPipelineDepthStencilStateCreateInfo(in_struct->pDepthStencilState);
    else
        pDepthStencilState = nullptr;  // original pDepthStencilState pointer ignored
    // needs a tracked subpass state usesColorAttachment
    if (in_struct->pColorBlendState && ((has_rasterization && uses_color_attachment) || is_graphics_library))
        pColorBlendState = new safe_VkPipelineColorBlendStateCreateInfo(in_struct->pColorBlendState);
    else
        pColorBlendState = nullptr;  // original pColorBlendState pointer ignored
    if (in_struct->pDynamicState)
        pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(in_struct->pDynamicState);
    else
        pDynamicState = nullptr;
}

safe_VkGraphicsPipelineCreateInfo::safe_VkGraphicsPipelineCreateInfo()
    : sType(VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO),
      pNext(nullptr),
      flags(),
      stageCount(),
      pStages(nullptr),
      pVertexInputState(nullptr),
      pInputAssemblyState(nullptr),
      pTessellationState(nullptr),
      pViewportState(nullptr),
      pRasterizationState(nullptr),
      pMultisampleState(nullptr),
      pDepthStencilState(nullptr),
      pColorBlendState(nullptr),
      pDynamicState(nullptr),
      layout(),
      renderPass(),
      subpass(),
      basePipelineHandle(),
      basePipelineIndex() {}

safe_VkGraphicsPipelineCreateInfo::safe_VkGraphicsPipelineCreateInfo(const safe_VkGraphicsPipelineCreateInfo& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    stageCount = copy_src.stageCount;
    pStages = nullptr;
    pVertexInputState = nullptr;
    pInputAssemblyState = nullptr;
    pTessellationState = nullptr;
    pViewportState = nullptr;
    pRasterizationState = nullptr;
    pMultisampleState = nullptr;
    pDepthStencilState = nullptr;
    pColorBlendState = nullptr;
    pDynamicState = nullptr;
    layout = copy_src.layout;
    renderPass = copy_src.renderPass;
    subpass = copy_src.subpass;
    basePipelineHandle = copy_src.basePipelineHandle;
    basePipelineIndex = copy_src.basePipelineIndex;

    pNext = SafePnextCopy(copy_src.pNext);
    const bool is_graphics_library = vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(copy_src.pNext);
    if (stageCount && copy_src.pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&copy_src.pStages[i]);
        }
    }
    if (copy_src.pVertexInputState)
        pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(*copy_src.pVertexInputState);
    else
        pVertexInputState = nullptr;
    if (copy_src.pInputAssemblyState)
        pInputAssemblyState = new safe_VkPipelineInputAssemblyStateCreateInfo(*copy_src.pInputAssemblyState);
    else
        pInputAssemblyState = nullptr;
    bool has_tessellation_stage = false;
    if (stageCount && pStages)
        for (uint32_t i = 0; i < stageCount && !has_tessellation_stage; ++i)
            if (pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
                pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
                has_tessellation_stage = true;
    if (copy_src.pTessellationState && has_tessellation_stage)
        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(*copy_src.pTessellationState);
    else
        pTessellationState = nullptr;  // original pTessellationState pointer ignored
    bool is_dynamic_has_rasterization = false;
    if (copy_src.pDynamicState && copy_src.pDynamicState->pDynamicStates) {
        for (uint32_t i = 0; i < copy_src.pDynamicState->dynamicStateCount && !is_dynamic_has_rasterization; ++i)
            if (copy_src.pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT)
                is_dynamic_has_rasterization = true;
    }
    // No pRasterizationState is same as dynamic, we assume rasterizerDiscardEnable is false
    const bool has_rasterization =
        is_dynamic_has_rasterization || (!copy_src.pRasterizationState || !copy_src.pRasterizationState->rasterizerDiscardEnable);
    if (copy_src.pViewportState && (has_rasterization || is_graphics_library)) {
        pViewportState = new safe_VkPipelineViewportStateCreateInfo(*copy_src.pViewportState);
    } else
        pViewportState = nullptr;  // original pViewportState pointer ignored
    if (copy_src.pRasterizationState)
        pRasterizationState = new safe_VkPipelineRasterizationStateCreateInfo(*copy_src.pRasterizationState);
    else
        pRasterizationState = nullptr;
    if (copy_src.pMultisampleState && (has_rasterization || is_graphics_library))
        pMultisampleState = new safe_VkPipelineMultisampleStateCreateInfo(*copy_src.pMultisampleState);
    else
        pMultisampleState = nullptr;  // original pMultisampleState pointer ignored
    if (copy_src.pDepthStencilState && (has_rasterization || is_graphics_library))
        pDepthStencilState = new safe_VkPipelineDepthStencilStateCreateInfo(*copy_src.pDepthStencilState);
    else
        pDepthStencilState = nullptr;  // original pDepthStencilState pointer ignored
    if (copy_src.pColorBlendState && (has_rasterization || is_graphics_library))
        pColorBlendState = new safe_VkPipelineColorBlendStateCreateInfo(*copy_src.pColorBlendState);
    else
        pColorBlendState = nullptr;  // original pColorBlendState pointer ignored
    if (copy_src.pDynamicState)
        pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(*copy_src.pDynamicState);
    else
        pDynamicState = nullptr;
}

safe_VkGraphicsPipelineCreateInfo& safe_VkGraphicsPipelineCreateInfo::operator=(const safe_VkGraphicsPipelineCreateInfo& copy_src) {
    if (&copy_src == this) return *this;

    if (pStages) delete[] pStages;
    if (pVertexInputState) delete pVertexInputState;
    if (pInputAssemblyState) delete pInputAssemblyState;
    if (pTessellationState) delete pTessellationState;
    if (pViewportState) delete pViewportState;
    if (pRasterizationState) delete pRasterizationState;
    if (pMultisampleState) delete pMultisampleState;
    if (pDepthStencilState) delete pDepthStencilState;
    if (pColorBlendState) delete pColorBlendState;
    if (pDynamicState) delete pDynamicState;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    stageCount = copy_src.stageCount;
    pStages = nullptr;
    pVertexInputState = nullptr;
    pInputAssemblyState = nullptr;
    pTessellationState = nullptr;
    pViewportState = nullptr;
    pRasterizationState = nullptr;
    pMultisampleState = nullptr;
    pDepthStencilState = nullptr;
    pColorBlendState = nullptr;
    pDynamicState = nullptr;
    layout = copy_src.layout;
    renderPass = copy_src.renderPass;
    subpass = copy_src.subpass;
    basePipelineHandle = copy_src.basePipelineHandle;
    basePipelineIndex = copy_src.basePipelineIndex;

    pNext = SafePnextCopy(copy_src.pNext);
    const bool is_graphics_library = vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(copy_src.pNext);
    if (stageCount && copy_src.pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&copy_src.pStages[i]);
        }
    }
    if (copy_src.pVertexInputState)
        pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(*copy_src.pVertexInputState);
    else
        pVertexInputState = nullptr;
    if (copy_src.pInputAssemblyState)
        pInputAssemblyState = new safe_VkPipelineInputAssemblyStateCreateInfo(*copy_src.pInputAssemblyState);
    else
        pInputAssemblyState = nullptr;
    bool has_tessellation_stage = false;
    if (stageCount && pStages)
        for (uint32_t i = 0; i < stageCount && !has_tessellation_stage; ++i)
            if (pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
                pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
                has_tessellation_stage = true;
    if (copy_src.pTessellationState && has_tessellation_stage)
        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(*copy_src.pTessellationState);
    else
        pTessellationState = nullptr;  // original pTessellationState pointer ignored
    bool is_dynamic_has_rasterization = false;
    if (copy_src.pDynamicState && copy_src.pDynamicState->pDynamicStates) {
        for (uint32_t i = 0; i < copy_src.pDynamicState->dynamicStateCount && !is_dynamic_has_rasterization; ++i)
            if (copy_src.pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT)
                is_dynamic_has_rasterization = true;
    }
    // No pRasterizationState is same as dynamic, we assume rasterizerDiscardEnable is false
    const bool has_rasterization =
        is_dynamic_has_rasterization || (!copy_src.pRasterizationState || !copy_src.pRasterizationState->rasterizerDiscardEnable);
    if (copy_src.pViewportState && (has_rasterization || is_graphics_library)) {
        pViewportState = new safe_VkPipelineViewportStateCreateInfo(*copy_src.pViewportState);
    } else
        pViewportState = nullptr;  // original pViewportState pointer ignored
    if (copy_src.pRasterizationState)
        pRasterizationState = new safe_VkPipelineRasterizationStateCreateInfo(*copy_src.pRasterizationState);
    else
        pRasterizationState = nullptr;
    if (copy_src.pMultisampleState && (has_rasterization || is_graphics_library))
        pMultisampleState = new safe_VkPipelineMultisampleStateCreateInfo(*copy_src.pMultisampleState);
    else
        pMultisampleState = nullptr;  // original pMultisampleState pointer ignored
    if (copy_src.pDepthStencilState && (has_rasterization || is_graphics_library))
        pDepthStencilState = new safe_VkPipelineDepthStencilStateCreateInfo(*copy_src.pDepthStencilState);
    else
        pDepthStencilState = nullptr;  // original pDepthStencilState pointer ignored
    if (copy_src.pColorBlendState && (has_rasterization || is_graphics_library))
        pColorBlendState = new safe_VkPipelineColorBlendStateCreateInfo(*copy_src.pColorBlendState);
    else
        pColorBlendState = nullptr;  // original pColorBlendState pointer ignored
    if (copy_src.pDynamicState)
        pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(*copy_src.pDynamicState);
    else
        pDynamicState = nullptr;

    return *this;
}

safe_VkGraphicsPipelineCreateInfo::~safe_VkGraphicsPipelineCreateInfo() {
    if (pStages) delete[] pStages;
    if (pVertexInputState) delete pVertexInputState;
    if (pInputAssemblyState) delete pInputAssemblyState;
    if (pTessellationState) delete pTessellationState;
    if (pViewportState) delete pViewportState;
    if (pRasterizationState) delete pRasterizationState;
    if (pMultisampleState) delete pMultisampleState;
    if (pDepthStencilState) delete pDepthStencilState;
    if (pColorBlendState) delete pColorBlendState;
    if (pDynamicState) delete pDynamicState;
    FreePnextChain(pNext);
}

void safe_VkGraphicsPipelineCreateInfo::initialize(const VkGraphicsPipelineCreateInfo* in_struct, const bool uses_color_attachment,
                                                   const bool uses_depthstencil_attachment,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStages) delete[] pStages;
    if (pVertexInputState) delete pVertexInputState;
    if (pInputAssemblyState) delete pInputAssemblyState;
    if (pTessellationState) delete pTessellationState;
    if (pViewportState) delete pViewportState;
    if (pRasterizationState) delete pRasterizationState;
    if (pMultisampleState) delete pMultisampleState;
    if (pDepthStencilState) delete pDepthStencilState;
    if (pColorBlendState) delete pColorBlendState;
    if (pDynamicState) delete pDynamicState;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    stageCount = in_struct->stageCount;
    pStages = nullptr;
    pVertexInputState = nullptr;
    pInputAssemblyState = nullptr;
    pTessellationState = nullptr;
    pViewportState = nullptr;
    pRasterizationState = nullptr;
    pMultisampleState = nullptr;
    pDepthStencilState = nullptr;
    pColorBlendState = nullptr;
    pDynamicState = nullptr;
    layout = in_struct->layout;
    renderPass = in_struct->renderPass;
    subpass = in_struct->subpass;
    basePipelineHandle = in_struct->basePipelineHandle;
    basePipelineIndex = in_struct->basePipelineIndex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    const bool is_graphics_library =
        vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(in_struct->pNext) != nullptr;
    if (stageCount && in_struct->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&in_struct->pStages[i]);
        }
    }
    if (in_struct->pVertexInputState)
        pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(in_struct->pVertexInputState);
    else
        pVertexInputState = nullptr;
    if (in_struct->pInputAssemblyState)
        pInputAssemblyState = new safe_VkPipelineInputAssemblyStateCreateInfo(in_struct->pInputAssemblyState);
    else
        pInputAssemblyState = nullptr;
    bool has_tessellation_stage = false;
    if (stageCount && pStages) {
        for (uint32_t i = 0; i < stageCount; ++i) {
            if (pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
                pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
                has_tessellation_stage = true;
        }
    }
    if (in_struct->pTessellationState && has_tessellation_stage)
        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(in_struct->pTessellationState);
    else
        pTessellationState = nullptr;  // original pTessellationState pointer ignored
    bool is_dynamic_has_rasterization = false;
    if (in_struct->pDynamicState && in_struct->pDynamicState->pDynamicStates) {
        for (uint32_t i = 0; i < in_struct->pDynamicState->dynamicStateCount && !is_dynamic_has_rasterization; ++i)
            if (in_struct->pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT)
                is_dynamic_has_rasterization = true;
    }
    // No pRasterizationState is same as dynamic, we assume rasterizerDiscardEnable is false
    const bool has_rasterization = is_dynamic_has_rasterization ||
                                   (!in_struct->pRasterizationState || !in_struct->pRasterizationState->rasterizerDiscardEnable);
    if (in_struct->pViewportState && (has_rasterization || is_graphics_library)) {
        bool is_dynamic_viewports = false;
        bool is_dynamic_scissors = false;
        if (in_struct->pDynamicState && in_struct->pDynamicState->pDynamicStates) {
            for (uint32_t i = 0; i < in_struct->pDynamicState->dynamicStateCount && !is_dynamic_viewports; ++i)
                if (in_struct->pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_VIEWPORT) is_dynamic_viewports = true;
            for (uint32_t i = 0; i < in_struct->pDynamicState->dynamicStateCount && !is_dynamic_scissors; ++i)
                if (in_struct->pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_SCISSOR) is_dynamic_scissors = true;
        }
        pViewportState =
            new safe_VkPipelineViewportStateCreateInfo(in_struct->pViewportState, is_dynamic_viewports, is_dynamic_scissors);
    } else
        pViewportState = nullptr;  // original pViewportState pointer ignored
    if (in_struct->pRasterizationState)
        pRasterizationState = new safe_VkPipelineRasterizationStateCreateInfo(in_struct->pRasterizationState);
    else
        pRasterizationState = nullptr;
    if (in_struct->pMultisampleState && (has_rasterization || is_graphics_library))
        pMultisampleState = new safe_VkPipelineMultisampleStateCreateInfo(in_struct->pMultisampleState);
    else
        pMultisampleState = nullptr;  // original pMultisampleState pointer ignored
    // needs a tracked subpass state uses_depthstencil_attachment
    if (in_struct->pDepthStencilState && ((has_rasterization && uses_depthstencil_attachment) || is_graphics_library))
        pDepthStencilState = new safe_VkPipelineDepthStencilStateCreateInfo(in_struct->pDepthStencilState);
    else
        pDepthStencilState = nullptr;  // original pDepthStencilState pointer ignored
    // needs a tracked subpass state usesColorAttachment
    if (in_struct->pColorBlendState && ((has_rasterization && uses_color_attachment) || is_graphics_library))
        pColorBlendState = new safe_VkPipelineColorBlendStateCreateInfo(in_struct->pColorBlendState);
    else
        pColorBlendState = nullptr;  // original pColorBlendState pointer ignored
    if (in_struct->pDynamicState)
        pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(in_struct->pDynamicState);
    else
        pDynamicState = nullptr;
}

void safe_VkGraphicsPipelineCreateInfo::initialize(const safe_VkGraphicsPipelineCreateInfo* copy_src,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    stageCount = copy_src->stageCount;
    pStages = nullptr;
    pVertexInputState = nullptr;
    pInputAssemblyState = nullptr;
    pTessellationState = nullptr;
    pViewportState = nullptr;
    pRasterizationState = nullptr;
    pMultisampleState = nullptr;
    pDepthStencilState = nullptr;
    pColorBlendState = nullptr;
    pDynamicState = nullptr;
    layout = copy_src->layout;
    renderPass = copy_src->renderPass;
    subpass = copy_src->subpass;
    basePipelineHandle = copy_src->basePipelineHandle;
    basePipelineIndex = copy_src->basePipelineIndex;

    pNext = SafePnextCopy(copy_src->pNext);
    const bool is_graphics_library = vku::FindStructInPNextChain<VkGraphicsPipelineLibraryCreateInfoEXT>(copy_src->pNext);
    if (stageCount && copy_src->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&copy_src->pStages[i]);
        }
    }
    if (copy_src->pVertexInputState)
        pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(*copy_src->pVertexInputState);
    else
        pVertexInputState = nullptr;
    if (copy_src->pInputAssemblyState)
        pInputAssemblyState = new safe_VkPipelineInputAssemblyStateCreateInfo(*copy_src->pInputAssemblyState);
    else
        pInputAssemblyState = nullptr;
    bool has_tessellation_stage = false;
    if (stageCount && pStages)
        for (uint32_t i = 0; i < stageCount && !has_tessellation_stage; ++i)
            if (pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT ||
                pStages[i].stage == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
                has_tessellation_stage = true;
    if (copy_src->pTessellationState && has_tessellation_stage)
        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(*copy_src->pTessellationState);
    else
        pTessellationState = nullptr;  // original pTessellationState pointer ignored
    bool is_dynamic_has_rasterization = false;
    if (copy_src->pDynamicState && copy_src->pDynamicState->pDynamicStates) {
        for (uint32_t i = 0; i < copy_src->pDynamicState->dynamicStateCount && !is_dynamic_has_rasterization; ++i)
            if (copy_src->pDynamicState->pDynamicStates[i] == VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE_EXT)
                is_dynamic_has_rasterization = true;
    }
    // No pRasterizationState is same as dynamic, we assume rasterizerDiscardEnable is false
    const bool has_rasterization =
        is_dynamic_has_rasterization || (!copy_src->pRasterizationState || !copy_src->pRasterizationState->rasterizerDiscardEnable);
    if (copy_src->pViewportState && (has_rasterization || is_graphics_library)) {
        pViewportState = new safe_VkPipelineViewportStateCreateInfo(*copy_src->pViewportState);
    } else
        pViewportState = nullptr;  // original pViewportState pointer ignored
    if (copy_src->pRasterizationState)
        pRasterizationState = new safe_VkPipelineRasterizationStateCreateInfo(*copy_src->pRasterizationState);
    else
        pRasterizationState = nullptr;
    if (copy_src->pMultisampleState && (has_rasterization || is_graphics_library))
        pMultisampleState = new safe_VkPipelineMultisampleStateCreateInfo(*copy_src->pMultisampleState);
    else
        pMultisampleState = nullptr;  // original pMultisampleState pointer ignored
    if (copy_src->pDepthStencilState && (has_rasterization || is_graphics_library))
        pDepthStencilState = new safe_VkPipelineDepthStencilStateCreateInfo(*copy_src->pDepthStencilState);
    else
        pDepthStencilState = nullptr;  // original pDepthStencilState pointer ignored
    if (copy_src->pColorBlendState && (has_rasterization || is_graphics_library))
        pColorBlendState = new safe_VkPipelineColorBlendStateCreateInfo(*copy_src->pColorBlendState);
    else
        pColorBlendState = nullptr;  // original pColorBlendState pointer ignored
    if (copy_src->pDynamicState)
        pDynamicState = new safe_VkPipelineDynamicStateCreateInfo(*copy_src->pDynamicState);
    else
        pDynamicState = nullptr;
}

safe_VkPipelineViewportStateCreateInfo::safe_VkPipelineViewportStateCreateInfo(const VkPipelineViewportStateCreateInfo* in_struct,
                                                                               const bool is_dynamic_viewports,
                                                                               const bool is_dynamic_scissors,
                                                                               [[maybe_unused]] PNextCopyState* copy_state,
                                                                               bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      viewportCount(in_struct->viewportCount),
      pViewports(nullptr),
      scissorCount(in_struct->scissorCount),
      pScissors(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pViewports && !is_dynamic_viewports) {
        pViewports = new VkViewport[in_struct->viewportCount];
        memcpy((void*)pViewports, (void*)in_struct->pViewports, sizeof(VkViewport) * in_struct->viewportCount);
    } else
        pViewports = nullptr;
    if (in_struct->pScissors && !is_dynamic_scissors) {
        pScissors = new VkRect2D[in_struct->scissorCount];
        memcpy((void*)pScissors, (void*)in_struct->pScissors, sizeof(VkRect2D) * in_struct->scissorCount);
    } else
        pScissors = nullptr;
}

safe_VkPipelineViewportStateCreateInfo::safe_VkPipelineViewportStateCreateInfo()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO),
      pNext(nullptr),
      flags(),
      viewportCount(),
      pViewports(nullptr),
      scissorCount(),
      pScissors(nullptr) {}

safe_VkPipelineViewportStateCreateInfo::safe_VkPipelineViewportStateCreateInfo(
    const safe_VkPipelineViewportStateCreateInfo& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    viewportCount = copy_src.viewportCount;
    pViewports = nullptr;
    scissorCount = copy_src.scissorCount;
    pScissors = nullptr;

    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pViewports) {
        pViewports = new VkViewport[copy_src.viewportCount];
        memcpy((void*)pViewports, (void*)copy_src.pViewports, sizeof(VkViewport) * copy_src.viewportCount);
    } else
        pViewports = nullptr;
    if (copy_src.pScissors) {
        pScissors = new VkRect2D[copy_src.scissorCount];
        memcpy((void*)pScissors, (void*)copy_src.pScissors, sizeof(VkRect2D) * copy_src.scissorCount);
    } else
        pScissors = nullptr;
}

safe_VkPipelineViewportStateCreateInfo& safe_VkPipelineViewportStateCreateInfo::operator=(
    const safe_VkPipelineViewportStateCreateInfo& copy_src) {
    if (&copy_src == this) return *this;

    if (pViewports) delete[] pViewports;
    if (pScissors) delete[] pScissors;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    viewportCount = copy_src.viewportCount;
    pViewports = nullptr;
    scissorCount = copy_src.scissorCount;
    pScissors = nullptr;

    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pViewports) {
        pViewports = new VkViewport[copy_src.viewportCount];
        memcpy((void*)pViewports, (void*)copy_src.pViewports, sizeof(VkViewport) * copy_src.viewportCount);
    } else
        pViewports = nullptr;
    if (copy_src.pScissors) {
        pScissors = new VkRect2D[copy_src.scissorCount];
        memcpy((void*)pScissors, (void*)copy_src.pScissors, sizeof(VkRect2D) * copy_src.scissorCount);
    } else
        pScissors = nullptr;

    return *this;
}

safe_VkPipelineViewportStateCreateInfo::~safe_VkPipelineViewportStateCreateInfo() {
    if (pViewports) delete[] pViewports;
    if (pScissors) delete[] pScissors;
    FreePnextChain(pNext);
}

void safe_VkPipelineViewportStateCreateInfo::initialize(const VkPipelineViewportStateCreateInfo* in_struct,
                                                        const bool is_dynamic_viewports, const bool is_dynamic_scissors,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    if (pViewports) delete[] pViewports;
    if (pScissors) delete[] pScissors;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    viewportCount = in_struct->viewportCount;
    pViewports = nullptr;
    scissorCount = in_struct->scissorCount;
    pScissors = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pViewports && !is_dynamic_viewports) {
        pViewports = new VkViewport[in_struct->viewportCount];
        memcpy((void*)pViewports, (void*)in_struct->pViewports, sizeof(VkViewport) * in_struct->viewportCount);
    } else
        pViewports = nullptr;
    if (in_struct->pScissors && !is_dynamic_scissors) {
        pScissors = new VkRect2D[in_struct->scissorCount];
        memcpy((void*)pScissors, (void*)in_struct->pScissors, sizeof(VkRect2D) * in_struct->scissorCount);
    } else
        pScissors = nullptr;
}

void safe_VkPipelineViewportStateCreateInfo::initialize(const safe_VkPipelineViewportStateCreateInfo* copy_src,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    viewportCount = copy_src->viewportCount;
    pViewports = nullptr;
    scissorCount = copy_src->scissorCount;
    pScissors = nullptr;

    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pViewports) {
        pViewports = new VkViewport[copy_src->viewportCount];
        memcpy((void*)pViewports, (void*)copy_src->pViewports, sizeof(VkViewport) * copy_src->viewportCount);
    } else
        pViewports = nullptr;
    if (copy_src->pScissors) {
        pScissors = new VkRect2D[copy_src->scissorCount];
        memcpy((void*)pScissors, (void*)copy_src->pScissors, sizeof(VkRect2D) * copy_src->scissorCount);
    } else
        pScissors = nullptr;
}

safe_VkAccelerationStructureBuildGeometryInfoKHR::safe_VkAccelerationStructureBuildGeometryInfoKHR(
    const VkAccelerationStructureBuildGeometryInfoKHR* in_struct, const bool is_host,
    const VkAccelerationStructureBuildRangeInfoKHR* build_range_infos, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      type(in_struct->type),
      flags(in_struct->flags),
      mode(in_struct->mode),
      srcAccelerationStructure(in_struct->srcAccelerationStructure),
      dstAccelerationStructure(in_struct->dstAccelerationStructure),
      geometryCount(in_struct->geometryCount),
      pGeometries(nullptr),
      ppGeometries(nullptr),
      scratchData(&in_struct->scratchData) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (geometryCount) {
        if (in_struct->ppGeometries) {
            ppGeometries = new safe_VkAccelerationStructureGeometryKHR*[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                ppGeometries[i] =
                    new safe_VkAccelerationStructureGeometryKHR(in_struct->ppGeometries[i], is_host, &build_range_infos[i]);
            }
        } else {
            pGeometries = new safe_VkAccelerationStructureGeometryKHR[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                (pGeometries)[i] =
                    safe_VkAccelerationStructureGeometryKHR(&(in_struct->pGeometries)[i], is_host, &build_range_infos[i]);
            }
        }
    }
}

safe_VkAccelerationStructureBuildGeometryInfoKHR::safe_VkAccelerationStructureBuildGeometryInfoKHR()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR),
      pNext(nullptr),
      type(),
      flags(),
      mode(),
      srcAccelerationStructure(),
      dstAccelerationStructure(),
      geometryCount(),
      pGeometries(nullptr),
      ppGeometries(nullptr) {}

safe_VkAccelerationStructureBuildGeometryInfoKHR::safe_VkAccelerationStructureBuildGeometryInfoKHR(
    const safe_VkAccelerationStructureBuildGeometryInfoKHR& copy_src) {
    sType = copy_src.sType;
    type = copy_src.type;
    flags = copy_src.flags;
    mode = copy_src.mode;
    srcAccelerationStructure = copy_src.srcAccelerationStructure;
    dstAccelerationStructure = copy_src.dstAccelerationStructure;
    geometryCount = copy_src.geometryCount;
    pGeometries = nullptr;
    ppGeometries = nullptr;
    scratchData.initialize(&copy_src.scratchData);

    if (geometryCount) {
        if (copy_src.ppGeometries) {
            ppGeometries = new safe_VkAccelerationStructureGeometryKHR*[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                ppGeometries[i] = new safe_VkAccelerationStructureGeometryKHR(*copy_src.ppGeometries[i]);
            }
        } else {
            pGeometries = new safe_VkAccelerationStructureGeometryKHR[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                pGeometries[i] = safe_VkAccelerationStructureGeometryKHR(copy_src.pGeometries[i]);
            }
        }
    }
}

safe_VkAccelerationStructureBuildGeometryInfoKHR& safe_VkAccelerationStructureBuildGeometryInfoKHR::operator=(
    const safe_VkAccelerationStructureBuildGeometryInfoKHR& copy_src) {
    if (&copy_src == this) return *this;

    if (ppGeometries) {
        for (uint32_t i = 0; i < geometryCount; ++i) {
            delete ppGeometries[i];
        }
        delete[] ppGeometries;
    } else if (pGeometries) {
        delete[] pGeometries;
    }
    FreePnextChain(pNext);

    sType = copy_src.sType;
    type = copy_src.type;
    flags = copy_src.flags;
    mode = copy_src.mode;
    srcAccelerationStructure = copy_src.srcAccelerationStructure;
    dstAccelerationStructure = copy_src.dstAccelerationStructure;
    geometryCount = copy_src.geometryCount;
    pGeometries = nullptr;
    ppGeometries = nullptr;
    scratchData.initialize(&copy_src.scratchData);

    if (geometryCount) {
        if (copy_src.ppGeometries) {
            ppGeometries = new safe_VkAccelerationStructureGeometryKHR*[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                ppGeometries[i] = new safe_VkAccelerationStructureGeometryKHR(*copy_src.ppGeometries[i]);
            }
        } else {
            pGeometries = new safe_VkAccelerationStructureGeometryKHR[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                pGeometries[i] = safe_VkAccelerationStructureGeometryKHR(copy_src.pGeometries[i]);
            }
        }
    }

    return *this;
}

safe_VkAccelerationStructureBuildGeometryInfoKHR::~safe_VkAccelerationStructureBuildGeometryInfoKHR() {
    if (ppGeometries) {
        for (uint32_t i = 0; i < geometryCount; ++i) {
            delete ppGeometries[i];
        }
        delete[] ppGeometries;
    } else if (pGeometries) {
        delete[] pGeometries;
    }
    FreePnextChain(pNext);
}

void safe_VkAccelerationStructureBuildGeometryInfoKHR::initialize(const VkAccelerationStructureBuildGeometryInfoKHR* in_struct,
                                                                  const bool is_host,
                                                                  const VkAccelerationStructureBuildRangeInfoKHR* build_range_infos,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    if (ppGeometries) {
        for (uint32_t i = 0; i < geometryCount; ++i) {
            delete ppGeometries[i];
        }
        delete[] ppGeometries;
    } else if (pGeometries) {
        delete[] pGeometries;
    }
    FreePnextChain(pNext);
    sType = in_struct->sType;
    type = in_struct->type;
    flags = in_struct->flags;
    mode = in_struct->mode;
    srcAccelerationStructure = in_struct->srcAccelerationStructure;
    dstAccelerationStructure = in_struct->dstAccelerationStructure;
    geometryCount = in_struct->geometryCount;
    pGeometries = nullptr;
    ppGeometries = nullptr;
    scratchData.initialize(&in_struct->scratchData);
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (geometryCount) {
        if (in_struct->ppGeometries) {
            ppGeometries = new safe_VkAccelerationStructureGeometryKHR*[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                ppGeometries[i] =
                    new safe_VkAccelerationStructureGeometryKHR(in_struct->ppGeometries[i], is_host, &build_range_infos[i]);
            }
        } else {
            pGeometries = new safe_VkAccelerationStructureGeometryKHR[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                (pGeometries)[i] =
                    safe_VkAccelerationStructureGeometryKHR(&(in_struct->pGeometries)[i], is_host, &build_range_infos[i]);
            }
        }
    }
}

void safe_VkAccelerationStructureBuildGeometryInfoKHR::initialize(const safe_VkAccelerationStructureBuildGeometryInfoKHR* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    type = copy_src->type;
    flags = copy_src->flags;
    mode = copy_src->mode;
    srcAccelerationStructure = copy_src->srcAccelerationStructure;
    dstAccelerationStructure = copy_src->dstAccelerationStructure;
    geometryCount = copy_src->geometryCount;
    pGeometries = nullptr;
    ppGeometries = nullptr;
    scratchData.initialize(&copy_src->scratchData);

    if (geometryCount) {
        if (copy_src->ppGeometries) {
            ppGeometries = new safe_VkAccelerationStructureGeometryKHR*[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                ppGeometries[i] = new safe_VkAccelerationStructureGeometryKHR(*copy_src->ppGeometries[i]);
            }
        } else {
            pGeometries = new safe_VkAccelerationStructureGeometryKHR[geometryCount];
            for (uint32_t i = 0; i < geometryCount; ++i) {
                pGeometries[i] = safe_VkAccelerationStructureGeometryKHR(copy_src->pGeometries[i]);
            }
        }
    }
}

safe_VkMicromapBuildInfoEXT::safe_VkMicromapBuildInfoEXT(const VkMicromapBuildInfoEXT* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      type(in_struct->type),
      flags(in_struct->flags),
      mode(in_struct->mode),
      dstMicromap(in_struct->dstMicromap),
      usageCountsCount(in_struct->usageCountsCount),
      pUsageCounts(nullptr),
      ppUsageCounts(nullptr),
      data(&in_struct->data),
      scratchData(&in_struct->scratchData),
      triangleArray(&in_struct->triangleArray),
      triangleArrayStride(in_struct->triangleArrayStride) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pUsageCounts) {
        pUsageCounts = new VkMicromapUsageEXT[in_struct->usageCountsCount];
        memcpy((void*)pUsageCounts, (void*)in_struct->pUsageCounts, sizeof(VkMicromapUsageEXT) * in_struct->usageCountsCount);
    }
    if (in_struct->ppUsageCounts) {
        VkMicromapUsageEXT** pointer_array = new VkMicromapUsageEXT*[in_struct->usageCountsCount];
        for (uint32_t i = 0; i < in_struct->usageCountsCount; ++i) {
            pointer_array[i] = new VkMicromapUsageEXT(*in_struct->ppUsageCounts[i]);
        }
        ppUsageCounts = pointer_array;
    }
}

safe_VkMicromapBuildInfoEXT::safe_VkMicromapBuildInfoEXT()
    : sType(VK_STRUCTURE_TYPE_MICROMAP_BUILD_INFO_EXT),
      pNext(nullptr),
      type(),
      flags(),
      mode(),
      dstMicromap(),
      usageCountsCount(),
      pUsageCounts(nullptr),
      ppUsageCounts(nullptr),
      triangleArrayStride() {}

safe_VkMicromapBuildInfoEXT::safe_VkMicromapBuildInfoEXT(const safe_VkMicromapBuildInfoEXT& copy_src) {
    sType = copy_src.sType;
    type = copy_src.type;
    flags = copy_src.flags;
    mode = copy_src.mode;
    dstMicromap = copy_src.dstMicromap;
    usageCountsCount = copy_src.usageCountsCount;
    pUsageCounts = nullptr;
    ppUsageCounts = nullptr;
    data.initialize(&copy_src.data);
    scratchData.initialize(&copy_src.scratchData);
    triangleArray.initialize(&copy_src.triangleArray);
    triangleArrayStride = copy_src.triangleArrayStride;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pUsageCounts) {
        pUsageCounts = new VkMicromapUsageEXT[copy_src.usageCountsCount];
        memcpy((void*)pUsageCounts, (void*)copy_src.pUsageCounts, sizeof(VkMicromapUsageEXT) * copy_src.usageCountsCount);
    }
    if (copy_src.ppUsageCounts) {
        VkMicromapUsageEXT** pointer_array = new VkMicromapUsageEXT*[copy_src.usageCountsCount];
        for (uint32_t i = 0; i < copy_src.usageCountsCount; ++i) {
            pointer_array[i] = new VkMicromapUsageEXT(*copy_src.ppUsageCounts[i]);
        }
        ppUsageCounts = pointer_array;
    }
}

safe_VkMicromapBuildInfoEXT& safe_VkMicromapBuildInfoEXT::operator=(const safe_VkMicromapBuildInfoEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pUsageCounts) delete[] pUsageCounts;
    if (ppUsageCounts) {
        for (uint32_t i = 0; i < usageCountsCount; ++i) {
            delete ppUsageCounts[i];
        }
        delete[] ppUsageCounts;
    }
    FreePnextChain(pNext);

    sType = copy_src.sType;
    type = copy_src.type;
    flags = copy_src.flags;
    mode = copy_src.mode;
    dstMicromap = copy_src.dstMicromap;
    usageCountsCount = copy_src.usageCountsCount;
    pUsageCounts = nullptr;
    ppUsageCounts = nullptr;
    data.initialize(&copy_src.data);
    scratchData.initialize(&copy_src.scratchData);
    triangleArray.initialize(&copy_src.triangleArray);
    triangleArrayStride = copy_src.triangleArrayStride;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pUsageCounts) {
        pUsageCounts = new VkMicromapUsageEXT[copy_src.usageCountsCount];
        memcpy((void*)pUsageCounts, (void*)copy_src.pUsageCounts, sizeof(VkMicromapUsageEXT) * copy_src.usageCountsCount);
    }
    if (copy_src.ppUsageCounts) {
        VkMicromapUsageEXT** pointer_array = new VkMicromapUsageEXT*[copy_src.usageCountsCount];
        for (uint32_t i = 0; i < copy_src.usageCountsCount; ++i) {
            pointer_array[i] = new VkMicromapUsageEXT(*copy_src.ppUsageCounts[i]);
        }
        ppUsageCounts = pointer_array;
    }

    return *this;
}

safe_VkMicromapBuildInfoEXT::~safe_VkMicromapBuildInfoEXT() {
    if (pUsageCounts) delete[] pUsageCounts;
    if (ppUsageCounts) {
        for (uint32_t i = 0; i < usageCountsCount; ++i) {
            delete ppUsageCounts[i];
        }
        delete[] ppUsageCounts;
    }
    FreePnextChain(pNext);
}

void safe_VkMicromapBuildInfoEXT::initialize(const VkMicromapBuildInfoEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pUsageCounts) delete[] pUsageCounts;
    if (ppUsageCounts) {
        for (uint32_t i = 0; i < usageCountsCount; ++i) {
            delete ppUsageCounts[i];
        }
        delete[] ppUsageCounts;
    }
    FreePnextChain(pNext);
    sType = in_struct->sType;
    type = in_struct->type;
    flags = in_struct->flags;
    mode = in_struct->mode;
    dstMicromap = in_struct->dstMicromap;
    usageCountsCount = in_struct->usageCountsCount;
    pUsageCounts = nullptr;
    ppUsageCounts = nullptr;
    data.initialize(&in_struct->data);
    scratchData.initialize(&in_struct->scratchData);
    triangleArray.initialize(&in_struct->triangleArray);
    triangleArrayStride = in_struct->triangleArrayStride;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pUsageCounts) {
        pUsageCounts = new VkMicromapUsageEXT[in_struct->usageCountsCount];
        memcpy((void*)pUsageCounts, (void*)in_struct->pUsageCounts, sizeof(VkMicromapUsageEXT) * in_struct->usageCountsCount);
    }
    if (in_struct->ppUsageCounts) {
        VkMicromapUsageEXT** pointer_array = new VkMicromapUsageEXT*[in_struct->usageCountsCount];
        for (uint32_t i = 0; i < in_struct->usageCountsCount; ++i) {
            pointer_array[i] = new VkMicromapUsageEXT(*in_struct->ppUsageCounts[i]);
        }
        ppUsageCounts = pointer_array;
    }
}

void safe_VkMicromapBuildInfoEXT::initialize(const safe_VkMicromapBuildInfoEXT* copy_src,
                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    type = copy_src->type;
    flags = copy_src->flags;
    mode = copy_src->mode;
    dstMicromap = copy_src->dstMicromap;
    usageCountsCount = copy_src->usageCountsCount;
    pUsageCounts = nullptr;
    ppUsageCounts = nullptr;
    data.initialize(&copy_src->data);
    scratchData.initialize(&copy_src->scratchData);
    triangleArray.initialize(&copy_src->triangleArray);
    triangleArrayStride = copy_src->triangleArrayStride;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pUsageCounts) {
        pUsageCounts = new VkMicromapUsageEXT[copy_src->usageCountsCount];
        memcpy((void*)pUsageCounts, (void*)copy_src->pUsageCounts, sizeof(VkMicromapUsageEXT) * copy_src->usageCountsCount);
    }
    if (copy_src->ppUsageCounts) {
        VkMicromapUsageEXT** pointer_array = new VkMicromapUsageEXT*[copy_src->usageCountsCount];
        for (uint32_t i = 0; i < copy_src->usageCountsCount; ++i) {
            pointer_array[i] = new VkMicromapUsageEXT(*copy_src->ppUsageCounts[i]);
        }
        ppUsageCounts = pointer_array;
    }
}

safe_VkAccelerationStructureTrianglesOpacityMicromapEXT::safe_VkAccelerationStructureTrianglesOpacityMicromapEXT(
    const VkAccelerationStructureTrianglesOpacityMicromapEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      indexType(in_struct->indexType),
      indexBuffer(&in_struct->indexBuffer),
      indexStride(in_struct->indexStride),
      baseTriangle(in_struct->baseTriangle),
      usageCountsCount(in_struct->usageCountsCount),
      pUsageCounts(nullptr),
      ppUsageCounts(nullptr),
      micromap(in_struct->micromap) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pUsageCounts) {
        pUsageCounts = new VkMicromapUsageEXT[in_struct->usageCountsCount];
        memcpy((void*)pUsageCounts, (void*)in_struct->pUsageCounts, sizeof(VkMicromapUsageEXT) * in_struct->usageCountsCount);
    }
    if (in_struct->ppUsageCounts) {
        VkMicromapUsageEXT** pointer_array = new VkMicromapUsageEXT*[in_struct->usageCountsCount];
        for (uint32_t i = 0; i < in_struct->usageCountsCount; ++i) {
            pointer_array[i] = new VkMicromapUsageEXT(*in_struct->ppUsageCounts[i]);
        }
        ppUsageCounts = pointer_array;
    }
}

safe_VkAccelerationStructureTrianglesOpacityMicromapEXT::safe_VkAccelerationStructureTrianglesOpacityMicromapEXT()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_OPACITY_MICROMAP_EXT),
      pNext(nullptr),
      indexType(),
      indexStride(),
      baseTriangle(),
      usageCountsCount(),
      pUsageCounts(nullptr),
      ppUsageCounts(nullptr),
      micromap() {}

safe_VkAccelerationStructureTrianglesOpacityMicromapEXT::safe_VkAccelerationStructureTrianglesOpacityMicromapEXT(
    const safe_VkAccelerationStructureTrianglesOpacityMicromapEXT& copy_src) {
    sType = copy_src.sType;
    indexType = copy_src.indexType;
    indexBuffer.initialize(&copy_src.indexBuffer);
    indexStride = copy_src.indexStride;
    baseTriangle = copy_src.baseTriangle;
    usageCountsCount = copy_src.usageCountsCount;
    pUsageCounts = nullptr;
    ppUsageCounts = nullptr;
    micromap = copy_src.micromap;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pUsageCounts) {
        pUsageCounts = new VkMicromapUsageEXT[copy_src.usageCountsCount];
        memcpy((void*)pUsageCounts, (void*)copy_src.pUsageCounts, sizeof(VkMicromapUsageEXT) * copy_src.usageCountsCount);
    }
    if (copy_src.ppUsageCounts) {
        VkMicromapUsageEXT** pointer_array = new VkMicromapUsageEXT*[copy_src.usageCountsCount];
        for (uint32_t i = 0; i < copy_src.usageCountsCount; ++i) {
            pointer_array[i] = new VkMicromapUsageEXT(*copy_src.ppUsageCounts[i]);
        }
        ppUsageCounts = pointer_array;
    }
}

safe_VkAccelerationStructureTrianglesOpacityMicromapEXT& safe_VkAccelerationStructureTrianglesOpacityMicromapEXT::operator=(
    const safe_VkAccelerationStructureTrianglesOpacityMicromapEXT& copy_src) {
    if (&copy_src == this) return *this;

    if (pUsageCounts) delete[] pUsageCounts;
    if (ppUsageCounts) {
        for (uint32_t i = 0; i < usageCountsCount; ++i) {
            delete ppUsageCounts[i];
        }
        delete[] ppUsageCounts;
    }
    FreePnextChain(pNext);

    sType = copy_src.sType;
    indexType = copy_src.indexType;
    indexBuffer.initialize(&copy_src.indexBuffer);
    indexStride = copy_src.indexStride;
    baseTriangle = copy_src.baseTriangle;
    usageCountsCount = copy_src.usageCountsCount;
    pUsageCounts = nullptr;
    ppUsageCounts = nullptr;
    micromap = copy_src.micromap;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pUsageCounts) {
        pUsageCounts = new VkMicromapUsageEXT[copy_src.usageCountsCount];
        memcpy((void*)pUsageCounts, (void*)copy_src.pUsageCounts, sizeof(VkMicromapUsageEXT) * copy_src.usageCountsCount);
    }
    if (copy_src.ppUsageCounts) {
        VkMicromapUsageEXT** pointer_array = new VkMicromapUsageEXT*[copy_src.usageCountsCount];
        for (uint32_t i = 0; i < copy_src.usageCountsCount; ++i) {
            pointer_array[i] = new VkMicromapUsageEXT(*copy_src.ppUsageCounts[i]);
        }
        ppUsageCounts = pointer_array;
    }

    return *this;
}

safe_VkAccelerationStructureTrianglesOpacityMicromapEXT::~safe_VkAccelerationStructureTrianglesOpacityMicromapEXT() {
    if (pUsageCounts) delete[] pUsageCounts;
    if (ppUsageCounts) {
        for (uint32_t i = 0; i < usageCountsCount; ++i) {
            delete ppUsageCounts[i];
        }
        delete[] ppUsageCounts;
    }
    FreePnextChain(pNext);
}

void safe_VkAccelerationStructureTrianglesOpacityMicromapEXT::initialize(
    const VkAccelerationStructureTrianglesOpacityMicromapEXT* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pUsageCounts) delete[] pUsageCounts;
    if (ppUsageCounts) {
        for (uint32_t i = 0; i < usageCountsCount; ++i) {
            delete ppUsageCounts[i];
        }
        delete[] ppUsageCounts;
    }
    FreePnextChain(pNext);
    sType = in_struct->sType;
    indexType = in_struct->indexType;
    indexBuffer.initialize(&in_struct->indexBuffer);
    indexStride = in_struct->indexStride;
    baseTriangle = in_struct->baseTriangle;
    usageCountsCount = in_struct->usageCountsCount;
    pUsageCounts = nullptr;
    ppUsageCounts = nullptr;
    micromap = in_struct->micromap;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pUsageCounts) {
        pUsageCounts = new VkMicromapUsageEXT[in_struct->usageCountsCount];
        memcpy((void*)pUsageCounts, (void*)in_struct->pUsageCounts, sizeof(VkMicromapUsageEXT) * in_struct->usageCountsCount);
    }
    if (in_struct->ppUsageCounts) {
        VkMicromapUsageEXT** pointer_array = new VkMicromapUsageEXT*[in_struct->usageCountsCount];
        for (uint32_t i = 0; i < in_struct->usageCountsCount; ++i) {
            pointer_array[i] = new VkMicromapUsageEXT(*in_struct->ppUsageCounts[i]);
        }
        ppUsageCounts = pointer_array;
    }
}

void safe_VkAccelerationStructureTrianglesOpacityMicromapEXT::initialize(
    const safe_VkAccelerationStructureTrianglesOpacityMicromapEXT* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    indexType = copy_src->indexType;
    indexBuffer.initialize(&copy_src->indexBuffer);
    indexStride = copy_src->indexStride;
    baseTriangle = copy_src->baseTriangle;
    usageCountsCount = copy_src->usageCountsCount;
    pUsageCounts = nullptr;
    ppUsageCounts = nullptr;
    micromap = copy_src->micromap;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pUsageCounts) {
        pUsageCounts = new VkMicromapUsageEXT[copy_src->usageCountsCount];
        memcpy((void*)pUsageCounts, (void*)copy_src->pUsageCounts, sizeof(VkMicromapUsageEXT) * copy_src->usageCountsCount);
    }
    if (copy_src->ppUsageCounts) {
        VkMicromapUsageEXT** pointer_array = new VkMicromapUsageEXT*[copy_src->usageCountsCount];
        for (uint32_t i = 0; i < copy_src->usageCountsCount; ++i) {
            pointer_array[i] = new VkMicromapUsageEXT(*copy_src->ppUsageCounts[i]);
        }
        ppUsageCounts = pointer_array;
    }
}

#ifdef VK_ENABLE_BETA_EXTENSIONS
safe_VkAccelerationStructureTrianglesDisplacementMicromapNV::safe_VkAccelerationStructureTrianglesDisplacementMicromapNV(
    const VkAccelerationStructureTrianglesDisplacementMicromapNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      displacementBiasAndScaleFormat(in_struct->displacementBiasAndScaleFormat),
      displacementVectorFormat(in_struct->displacementVectorFormat),
      displacementBiasAndScaleBuffer(&in_struct->displacementBiasAndScaleBuffer),
      displacementBiasAndScaleStride(in_struct->displacementBiasAndScaleStride),
      displacementVectorBuffer(&in_struct->displacementVectorBuffer),
      displacementVectorStride(in_struct->displacementVectorStride),
      displacedMicromapPrimitiveFlags(&in_struct->displacedMicromapPrimitiveFlags),
      displacedMicromapPrimitiveFlagsStride(in_struct->displacedMicromapPrimitiveFlagsStride),
      indexType(in_struct->indexType),
      indexBuffer(&in_struct->indexBuffer),
      indexStride(in_struct->indexStride),
      baseTriangle(in_struct->baseTriangle),
      usageCountsCount(in_struct->usageCountsCount),
      pUsageCounts(nullptr),
      ppUsageCounts(nullptr),
      micromap(in_struct->micromap) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pUsageCounts) {
        pUsageCounts = new VkMicromapUsageEXT[in_struct->usageCountsCount];
        memcpy((void*)pUsageCounts, (void*)in_struct->pUsageCounts, sizeof(VkMicromapUsageEXT) * in_struct->usageCountsCount);
    }
    if (in_struct->ppUsageCounts) {
        VkMicromapUsageEXT** pointer_array = new VkMicromapUsageEXT*[in_struct->usageCountsCount];
        for (uint32_t i = 0; i < in_struct->usageCountsCount; ++i) {
            pointer_array[i] = new VkMicromapUsageEXT(*in_struct->ppUsageCounts[i]);
        }
        ppUsageCounts = pointer_array;
    }
}

safe_VkAccelerationStructureTrianglesDisplacementMicromapNV::safe_VkAccelerationStructureTrianglesDisplacementMicromapNV()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_TRIANGLES_DISPLACEMENT_MICROMAP_NV),
      pNext(nullptr),
      displacementBiasAndScaleFormat(),
      displacementVectorFormat(),
      displacementBiasAndScaleStride(),
      displacementVectorStride(),
      displacedMicromapPrimitiveFlagsStride(),
      indexType(),
      indexStride(),
      baseTriangle(),
      usageCountsCount(),
      pUsageCounts(nullptr),
      ppUsageCounts(nullptr),
      micromap() {}

safe_VkAccelerationStructureTrianglesDisplacementMicromapNV::safe_VkAccelerationStructureTrianglesDisplacementMicromapNV(
    const safe_VkAccelerationStructureTrianglesDisplacementMicromapNV& copy_src) {
    sType = copy_src.sType;
    displacementBiasAndScaleFormat = copy_src.displacementBiasAndScaleFormat;
    displacementVectorFormat = copy_src.displacementVectorFormat;
    displacementBiasAndScaleBuffer.initialize(&copy_src.displacementBiasAndScaleBuffer);
    displacementBiasAndScaleStride = copy_src.displacementBiasAndScaleStride;
    displacementVectorBuffer.initialize(&copy_src.displacementVectorBuffer);
    displacementVectorStride = copy_src.displacementVectorStride;
    displacedMicromapPrimitiveFlags.initialize(&copy_src.displacedMicromapPrimitiveFlags);
    displacedMicromapPrimitiveFlagsStride = copy_src.displacedMicromapPrimitiveFlagsStride;
    indexType = copy_src.indexType;
    indexBuffer.initialize(&copy_src.indexBuffer);
    indexStride = copy_src.indexStride;
    baseTriangle = copy_src.baseTriangle;
    usageCountsCount = copy_src.usageCountsCount;
    pUsageCounts = nullptr;
    ppUsageCounts = nullptr;
    micromap = copy_src.micromap;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pUsageCounts) {
        pUsageCounts = new VkMicromapUsageEXT[copy_src.usageCountsCount];
        memcpy((void*)pUsageCounts, (void*)copy_src.pUsageCounts, sizeof(VkMicromapUsageEXT) * copy_src.usageCountsCount);
    }
    if (copy_src.ppUsageCounts) {
        VkMicromapUsageEXT** pointer_array = new VkMicromapUsageEXT*[copy_src.usageCountsCount];
        for (uint32_t i = 0; i < copy_src.usageCountsCount; ++i) {
            pointer_array[i] = new VkMicromapUsageEXT(*copy_src.ppUsageCounts[i]);
        }
        ppUsageCounts = pointer_array;
    }
}

safe_VkAccelerationStructureTrianglesDisplacementMicromapNV& safe_VkAccelerationStructureTrianglesDisplacementMicromapNV::operator=(
    const safe_VkAccelerationStructureTrianglesDisplacementMicromapNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pUsageCounts) delete[] pUsageCounts;
    if (ppUsageCounts) {
        for (uint32_t i = 0; i < usageCountsCount; ++i) {
            delete ppUsageCounts[i];
        }
        delete[] ppUsageCounts;
    }
    FreePnextChain(pNext);

    sType = copy_src.sType;
    displacementBiasAndScaleFormat = copy_src.displacementBiasAndScaleFormat;
    displacementVectorFormat = copy_src.displacementVectorFormat;
    displacementBiasAndScaleBuffer.initialize(&copy_src.displacementBiasAndScaleBuffer);
    displacementBiasAndScaleStride = copy_src.displacementBiasAndScaleStride;
    displacementVectorBuffer.initialize(&copy_src.displacementVectorBuffer);
    displacementVectorStride = copy_src.displacementVectorStride;
    displacedMicromapPrimitiveFlags.initialize(&copy_src.displacedMicromapPrimitiveFlags);
    displacedMicromapPrimitiveFlagsStride = copy_src.displacedMicromapPrimitiveFlagsStride;
    indexType = copy_src.indexType;
    indexBuffer.initialize(&copy_src.indexBuffer);
    indexStride = copy_src.indexStride;
    baseTriangle = copy_src.baseTriangle;
    usageCountsCount = copy_src.usageCountsCount;
    pUsageCounts = nullptr;
    ppUsageCounts = nullptr;
    micromap = copy_src.micromap;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pUsageCounts) {
        pUsageCounts = new VkMicromapUsageEXT[copy_src.usageCountsCount];
        memcpy((void*)pUsageCounts, (void*)copy_src.pUsageCounts, sizeof(VkMicromapUsageEXT) * copy_src.usageCountsCount);
    }
    if (copy_src.ppUsageCounts) {
        VkMicromapUsageEXT** pointer_array = new VkMicromapUsageEXT*[copy_src.usageCountsCount];
        for (uint32_t i = 0; i < copy_src.usageCountsCount; ++i) {
            pointer_array[i] = new VkMicromapUsageEXT(*copy_src.ppUsageCounts[i]);
        }
        ppUsageCounts = pointer_array;
    }

    return *this;
}

safe_VkAccelerationStructureTrianglesDisplacementMicromapNV::~safe_VkAccelerationStructureTrianglesDisplacementMicromapNV() {
    if (pUsageCounts) delete[] pUsageCounts;
    if (ppUsageCounts) {
        for (uint32_t i = 0; i < usageCountsCount; ++i) {
            delete ppUsageCounts[i];
        }
        delete[] ppUsageCounts;
    }
    FreePnextChain(pNext);
}

void safe_VkAccelerationStructureTrianglesDisplacementMicromapNV::initialize(
    const VkAccelerationStructureTrianglesDisplacementMicromapNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pUsageCounts) delete[] pUsageCounts;
    if (ppUsageCounts) {
        for (uint32_t i = 0; i < usageCountsCount; ++i) {
            delete ppUsageCounts[i];
        }
        delete[] ppUsageCounts;
    }
    FreePnextChain(pNext);
    sType = in_struct->sType;
    displacementBiasAndScaleFormat = in_struct->displacementBiasAndScaleFormat;
    displacementVectorFormat = in_struct->displacementVectorFormat;
    displacementBiasAndScaleBuffer.initialize(&in_struct->displacementBiasAndScaleBuffer);
    displacementBiasAndScaleStride = in_struct->displacementBiasAndScaleStride;
    displacementVectorBuffer.initialize(&in_struct->displacementVectorBuffer);
    displacementVectorStride = in_struct->displacementVectorStride;
    displacedMicromapPrimitiveFlags.initialize(&in_struct->displacedMicromapPrimitiveFlags);
    displacedMicromapPrimitiveFlagsStride = in_struct->displacedMicromapPrimitiveFlagsStride;
    indexType = in_struct->indexType;
    indexBuffer.initialize(&in_struct->indexBuffer);
    indexStride = in_struct->indexStride;
    baseTriangle = in_struct->baseTriangle;
    usageCountsCount = in_struct->usageCountsCount;
    pUsageCounts = nullptr;
    ppUsageCounts = nullptr;
    micromap = in_struct->micromap;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pUsageCounts) {
        pUsageCounts = new VkMicromapUsageEXT[in_struct->usageCountsCount];
        memcpy((void*)pUsageCounts, (void*)in_struct->pUsageCounts, sizeof(VkMicromapUsageEXT) * in_struct->usageCountsCount);
    }
    if (in_struct->ppUsageCounts) {
        VkMicromapUsageEXT** pointer_array = new VkMicromapUsageEXT*[in_struct->usageCountsCount];
        for (uint32_t i = 0; i < in_struct->usageCountsCount; ++i) {
            pointer_array[i] = new VkMicromapUsageEXT(*in_struct->ppUsageCounts[i]);
        }
        ppUsageCounts = pointer_array;
    }
}

void safe_VkAccelerationStructureTrianglesDisplacementMicromapNV::initialize(
    const safe_VkAccelerationStructureTrianglesDisplacementMicromapNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    displacementBiasAndScaleFormat = copy_src->displacementBiasAndScaleFormat;
    displacementVectorFormat = copy_src->displacementVectorFormat;
    displacementBiasAndScaleBuffer.initialize(&copy_src->displacementBiasAndScaleBuffer);
    displacementBiasAndScaleStride = copy_src->displacementBiasAndScaleStride;
    displacementVectorBuffer.initialize(&copy_src->displacementVectorBuffer);
    displacementVectorStride = copy_src->displacementVectorStride;
    displacedMicromapPrimitiveFlags.initialize(&copy_src->displacedMicromapPrimitiveFlags);
    displacedMicromapPrimitiveFlagsStride = copy_src->displacedMicromapPrimitiveFlagsStride;
    indexType = copy_src->indexType;
    indexBuffer.initialize(&copy_src->indexBuffer);
    indexStride = copy_src->indexStride;
    baseTriangle = copy_src->baseTriangle;
    usageCountsCount = copy_src->usageCountsCount;
    pUsageCounts = nullptr;
    ppUsageCounts = nullptr;
    micromap = copy_src->micromap;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pUsageCounts) {
        pUsageCounts = new VkMicromapUsageEXT[copy_src->usageCountsCount];
        memcpy((void*)pUsageCounts, (void*)copy_src->pUsageCounts, sizeof(VkMicromapUsageEXT) * copy_src->usageCountsCount);
    }
    if (copy_src->ppUsageCounts) {
        VkMicromapUsageEXT** pointer_array = new VkMicromapUsageEXT*[copy_src->usageCountsCount];
        for (uint32_t i = 0; i < copy_src->usageCountsCount; ++i) {
            pointer_array[i] = new VkMicromapUsageEXT(*copy_src->ppUsageCounts[i]);
        }
        ppUsageCounts = pointer_array;
    }
}
#endif  // VK_ENABLE_BETA_EXTENSIONS
}  // namespace vku
