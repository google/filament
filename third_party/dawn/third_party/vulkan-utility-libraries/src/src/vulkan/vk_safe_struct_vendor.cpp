// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See safe_struct_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 ****************************************************************************/

// NOLINTBEGIN

#include <vulkan/utility/vk_safe_struct.hpp>
#include <vulkan/utility/vk_struct_helper.hpp>

#include <cstddef>
#include <cstring>

namespace vku {

safe_VkPipelineRasterizationStateRasterizationOrderAMD::safe_VkPipelineRasterizationStateRasterizationOrderAMD(
    const VkPipelineRasterizationStateRasterizationOrderAMD* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), rasterizationOrder(in_struct->rasterizationOrder) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineRasterizationStateRasterizationOrderAMD::safe_VkPipelineRasterizationStateRasterizationOrderAMD()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_RASTERIZATION_ORDER_AMD), pNext(nullptr), rasterizationOrder() {}

safe_VkPipelineRasterizationStateRasterizationOrderAMD::safe_VkPipelineRasterizationStateRasterizationOrderAMD(
    const safe_VkPipelineRasterizationStateRasterizationOrderAMD& copy_src) {
    sType = copy_src.sType;
    rasterizationOrder = copy_src.rasterizationOrder;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineRasterizationStateRasterizationOrderAMD& safe_VkPipelineRasterizationStateRasterizationOrderAMD::operator=(
    const safe_VkPipelineRasterizationStateRasterizationOrderAMD& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    rasterizationOrder = copy_src.rasterizationOrder;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineRasterizationStateRasterizationOrderAMD::~safe_VkPipelineRasterizationStateRasterizationOrderAMD() {
    FreePnextChain(pNext);
}

void safe_VkPipelineRasterizationStateRasterizationOrderAMD::initialize(
    const VkPipelineRasterizationStateRasterizationOrderAMD* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    rasterizationOrder = in_struct->rasterizationOrder;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineRasterizationStateRasterizationOrderAMD::initialize(
    const safe_VkPipelineRasterizationStateRasterizationOrderAMD* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    rasterizationOrder = copy_src->rasterizationOrder;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDedicatedAllocationImageCreateInfoNV::safe_VkDedicatedAllocationImageCreateInfoNV(
    const VkDedicatedAllocationImageCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), dedicatedAllocation(in_struct->dedicatedAllocation) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDedicatedAllocationImageCreateInfoNV::safe_VkDedicatedAllocationImageCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_IMAGE_CREATE_INFO_NV), pNext(nullptr), dedicatedAllocation() {}

safe_VkDedicatedAllocationImageCreateInfoNV::safe_VkDedicatedAllocationImageCreateInfoNV(
    const safe_VkDedicatedAllocationImageCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    dedicatedAllocation = copy_src.dedicatedAllocation;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDedicatedAllocationImageCreateInfoNV& safe_VkDedicatedAllocationImageCreateInfoNV::operator=(
    const safe_VkDedicatedAllocationImageCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    dedicatedAllocation = copy_src.dedicatedAllocation;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDedicatedAllocationImageCreateInfoNV::~safe_VkDedicatedAllocationImageCreateInfoNV() { FreePnextChain(pNext); }

void safe_VkDedicatedAllocationImageCreateInfoNV::initialize(const VkDedicatedAllocationImageCreateInfoNV* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    dedicatedAllocation = in_struct->dedicatedAllocation;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDedicatedAllocationImageCreateInfoNV::initialize(const safe_VkDedicatedAllocationImageCreateInfoNV* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    dedicatedAllocation = copy_src->dedicatedAllocation;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDedicatedAllocationBufferCreateInfoNV::safe_VkDedicatedAllocationBufferCreateInfoNV(
    const VkDedicatedAllocationBufferCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), dedicatedAllocation(in_struct->dedicatedAllocation) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDedicatedAllocationBufferCreateInfoNV::safe_VkDedicatedAllocationBufferCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_BUFFER_CREATE_INFO_NV), pNext(nullptr), dedicatedAllocation() {}

safe_VkDedicatedAllocationBufferCreateInfoNV::safe_VkDedicatedAllocationBufferCreateInfoNV(
    const safe_VkDedicatedAllocationBufferCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    dedicatedAllocation = copy_src.dedicatedAllocation;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDedicatedAllocationBufferCreateInfoNV& safe_VkDedicatedAllocationBufferCreateInfoNV::operator=(
    const safe_VkDedicatedAllocationBufferCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    dedicatedAllocation = copy_src.dedicatedAllocation;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDedicatedAllocationBufferCreateInfoNV::~safe_VkDedicatedAllocationBufferCreateInfoNV() { FreePnextChain(pNext); }

void safe_VkDedicatedAllocationBufferCreateInfoNV::initialize(const VkDedicatedAllocationBufferCreateInfoNV* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    dedicatedAllocation = in_struct->dedicatedAllocation;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDedicatedAllocationBufferCreateInfoNV::initialize(const safe_VkDedicatedAllocationBufferCreateInfoNV* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    dedicatedAllocation = copy_src->dedicatedAllocation;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDedicatedAllocationMemoryAllocateInfoNV::safe_VkDedicatedAllocationMemoryAllocateInfoNV(
    const VkDedicatedAllocationMemoryAllocateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), image(in_struct->image), buffer(in_struct->buffer) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDedicatedAllocationMemoryAllocateInfoNV::safe_VkDedicatedAllocationMemoryAllocateInfoNV()
    : sType(VK_STRUCTURE_TYPE_DEDICATED_ALLOCATION_MEMORY_ALLOCATE_INFO_NV), pNext(nullptr), image(), buffer() {}

safe_VkDedicatedAllocationMemoryAllocateInfoNV::safe_VkDedicatedAllocationMemoryAllocateInfoNV(
    const safe_VkDedicatedAllocationMemoryAllocateInfoNV& copy_src) {
    sType = copy_src.sType;
    image = copy_src.image;
    buffer = copy_src.buffer;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDedicatedAllocationMemoryAllocateInfoNV& safe_VkDedicatedAllocationMemoryAllocateInfoNV::operator=(
    const safe_VkDedicatedAllocationMemoryAllocateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    image = copy_src.image;
    buffer = copy_src.buffer;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDedicatedAllocationMemoryAllocateInfoNV::~safe_VkDedicatedAllocationMemoryAllocateInfoNV() { FreePnextChain(pNext); }

void safe_VkDedicatedAllocationMemoryAllocateInfoNV::initialize(const VkDedicatedAllocationMemoryAllocateInfoNV* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    image = in_struct->image;
    buffer = in_struct->buffer;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDedicatedAllocationMemoryAllocateInfoNV::initialize(const safe_VkDedicatedAllocationMemoryAllocateInfoNV* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    image = copy_src->image;
    buffer = copy_src->buffer;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCuModuleCreateInfoNVX::safe_VkCuModuleCreateInfoNVX(const VkCuModuleCreateInfoNVX* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), dataSize(in_struct->dataSize), pData(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pData != nullptr) {
        auto temp = new std::byte[in_struct->dataSize];
        std::memcpy(temp, in_struct->pData, in_struct->dataSize);
        pData = temp;
    }
}

safe_VkCuModuleCreateInfoNVX::safe_VkCuModuleCreateInfoNVX()
    : sType(VK_STRUCTURE_TYPE_CU_MODULE_CREATE_INFO_NVX), pNext(nullptr), dataSize(), pData(nullptr) {}

safe_VkCuModuleCreateInfoNVX::safe_VkCuModuleCreateInfoNVX(const safe_VkCuModuleCreateInfoNVX& copy_src) {
    sType = copy_src.sType;
    dataSize = copy_src.dataSize;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pData != nullptr) {
        auto temp = new std::byte[copy_src.dataSize];
        std::memcpy(temp, copy_src.pData, copy_src.dataSize);
        pData = temp;
    }
}

safe_VkCuModuleCreateInfoNVX& safe_VkCuModuleCreateInfoNVX::operator=(const safe_VkCuModuleCreateInfoNVX& copy_src) {
    if (&copy_src == this) return *this;

    if (pData != nullptr) {
        auto temp = reinterpret_cast<const std::byte*>(pData);
        delete[] temp;
    }
    FreePnextChain(pNext);

    sType = copy_src.sType;
    dataSize = copy_src.dataSize;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pData != nullptr) {
        auto temp = new std::byte[copy_src.dataSize];
        std::memcpy(temp, copy_src.pData, copy_src.dataSize);
        pData = temp;
    }

    return *this;
}

safe_VkCuModuleCreateInfoNVX::~safe_VkCuModuleCreateInfoNVX() {
    if (pData != nullptr) {
        auto temp = reinterpret_cast<const std::byte*>(pData);
        delete[] temp;
    }
    FreePnextChain(pNext);
}

void safe_VkCuModuleCreateInfoNVX::initialize(const VkCuModuleCreateInfoNVX* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    if (pData != nullptr) {
        auto temp = reinterpret_cast<const std::byte*>(pData);
        delete[] temp;
    }
    FreePnextChain(pNext);
    sType = in_struct->sType;
    dataSize = in_struct->dataSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pData != nullptr) {
        auto temp = new std::byte[in_struct->dataSize];
        std::memcpy(temp, in_struct->pData, in_struct->dataSize);
        pData = temp;
    }
}

void safe_VkCuModuleCreateInfoNVX::initialize(const safe_VkCuModuleCreateInfoNVX* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    dataSize = copy_src->dataSize;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pData != nullptr) {
        auto temp = new std::byte[copy_src->dataSize];
        std::memcpy(temp, copy_src->pData, copy_src->dataSize);
        pData = temp;
    }
}

safe_VkCuModuleTexturingModeCreateInfoNVX::safe_VkCuModuleTexturingModeCreateInfoNVX(
    const VkCuModuleTexturingModeCreateInfoNVX* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), use64bitTexturing(in_struct->use64bitTexturing) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCuModuleTexturingModeCreateInfoNVX::safe_VkCuModuleTexturingModeCreateInfoNVX()
    : sType(VK_STRUCTURE_TYPE_CU_MODULE_TEXTURING_MODE_CREATE_INFO_NVX), pNext(nullptr), use64bitTexturing() {}

safe_VkCuModuleTexturingModeCreateInfoNVX::safe_VkCuModuleTexturingModeCreateInfoNVX(
    const safe_VkCuModuleTexturingModeCreateInfoNVX& copy_src) {
    sType = copy_src.sType;
    use64bitTexturing = copy_src.use64bitTexturing;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCuModuleTexturingModeCreateInfoNVX& safe_VkCuModuleTexturingModeCreateInfoNVX::operator=(
    const safe_VkCuModuleTexturingModeCreateInfoNVX& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    use64bitTexturing = copy_src.use64bitTexturing;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCuModuleTexturingModeCreateInfoNVX::~safe_VkCuModuleTexturingModeCreateInfoNVX() { FreePnextChain(pNext); }

void safe_VkCuModuleTexturingModeCreateInfoNVX::initialize(const VkCuModuleTexturingModeCreateInfoNVX* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    use64bitTexturing = in_struct->use64bitTexturing;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCuModuleTexturingModeCreateInfoNVX::initialize(const safe_VkCuModuleTexturingModeCreateInfoNVX* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    use64bitTexturing = copy_src->use64bitTexturing;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCuFunctionCreateInfoNVX::safe_VkCuFunctionCreateInfoNVX(const VkCuFunctionCreateInfoNVX* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), module(in_struct->module) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    pName = SafeStringCopy(in_struct->pName);
}

safe_VkCuFunctionCreateInfoNVX::safe_VkCuFunctionCreateInfoNVX()
    : sType(VK_STRUCTURE_TYPE_CU_FUNCTION_CREATE_INFO_NVX), pNext(nullptr), module(), pName(nullptr) {}

safe_VkCuFunctionCreateInfoNVX::safe_VkCuFunctionCreateInfoNVX(const safe_VkCuFunctionCreateInfoNVX& copy_src) {
    sType = copy_src.sType;
    module = copy_src.module;
    pNext = SafePnextCopy(copy_src.pNext);
    pName = SafeStringCopy(copy_src.pName);
}

safe_VkCuFunctionCreateInfoNVX& safe_VkCuFunctionCreateInfoNVX::operator=(const safe_VkCuFunctionCreateInfoNVX& copy_src) {
    if (&copy_src == this) return *this;

    if (pName) delete[] pName;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    module = copy_src.module;
    pNext = SafePnextCopy(copy_src.pNext);
    pName = SafeStringCopy(copy_src.pName);

    return *this;
}

safe_VkCuFunctionCreateInfoNVX::~safe_VkCuFunctionCreateInfoNVX() {
    if (pName) delete[] pName;
    FreePnextChain(pNext);
}

void safe_VkCuFunctionCreateInfoNVX::initialize(const VkCuFunctionCreateInfoNVX* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    if (pName) delete[] pName;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    module = in_struct->module;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    pName = SafeStringCopy(in_struct->pName);
}

void safe_VkCuFunctionCreateInfoNVX::initialize(const safe_VkCuFunctionCreateInfoNVX* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    module = copy_src->module;
    pNext = SafePnextCopy(copy_src->pNext);
    pName = SafeStringCopy(copy_src->pName);
}

safe_VkCuLaunchInfoNVX::safe_VkCuLaunchInfoNVX(const VkCuLaunchInfoNVX* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
                                               bool copy_pnext)
    : sType(in_struct->sType),
      function(in_struct->function),
      gridDimX(in_struct->gridDimX),
      gridDimY(in_struct->gridDimY),
      gridDimZ(in_struct->gridDimZ),
      blockDimX(in_struct->blockDimX),
      blockDimY(in_struct->blockDimY),
      blockDimZ(in_struct->blockDimZ),
      sharedMemBytes(in_struct->sharedMemBytes),
      paramCount(in_struct->paramCount),
      pParams(in_struct->pParams),
      extraCount(in_struct->extraCount),
      pExtras(in_struct->pExtras) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCuLaunchInfoNVX::safe_VkCuLaunchInfoNVX()
    : sType(VK_STRUCTURE_TYPE_CU_LAUNCH_INFO_NVX),
      pNext(nullptr),
      function(),
      gridDimX(),
      gridDimY(),
      gridDimZ(),
      blockDimX(),
      blockDimY(),
      blockDimZ(),
      sharedMemBytes(),
      paramCount(),
      pParams(nullptr),
      extraCount(),
      pExtras(nullptr) {}

safe_VkCuLaunchInfoNVX::safe_VkCuLaunchInfoNVX(const safe_VkCuLaunchInfoNVX& copy_src) {
    sType = copy_src.sType;
    function = copy_src.function;
    gridDimX = copy_src.gridDimX;
    gridDimY = copy_src.gridDimY;
    gridDimZ = copy_src.gridDimZ;
    blockDimX = copy_src.blockDimX;
    blockDimY = copy_src.blockDimY;
    blockDimZ = copy_src.blockDimZ;
    sharedMemBytes = copy_src.sharedMemBytes;
    paramCount = copy_src.paramCount;
    pParams = copy_src.pParams;
    extraCount = copy_src.extraCount;
    pExtras = copy_src.pExtras;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCuLaunchInfoNVX& safe_VkCuLaunchInfoNVX::operator=(const safe_VkCuLaunchInfoNVX& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    function = copy_src.function;
    gridDimX = copy_src.gridDimX;
    gridDimY = copy_src.gridDimY;
    gridDimZ = copy_src.gridDimZ;
    blockDimX = copy_src.blockDimX;
    blockDimY = copy_src.blockDimY;
    blockDimZ = copy_src.blockDimZ;
    sharedMemBytes = copy_src.sharedMemBytes;
    paramCount = copy_src.paramCount;
    pParams = copy_src.pParams;
    extraCount = copy_src.extraCount;
    pExtras = copy_src.pExtras;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCuLaunchInfoNVX::~safe_VkCuLaunchInfoNVX() { FreePnextChain(pNext); }

void safe_VkCuLaunchInfoNVX::initialize(const VkCuLaunchInfoNVX* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    function = in_struct->function;
    gridDimX = in_struct->gridDimX;
    gridDimY = in_struct->gridDimY;
    gridDimZ = in_struct->gridDimZ;
    blockDimX = in_struct->blockDimX;
    blockDimY = in_struct->blockDimY;
    blockDimZ = in_struct->blockDimZ;
    sharedMemBytes = in_struct->sharedMemBytes;
    paramCount = in_struct->paramCount;
    pParams = in_struct->pParams;
    extraCount = in_struct->extraCount;
    pExtras = in_struct->pExtras;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCuLaunchInfoNVX::initialize(const safe_VkCuLaunchInfoNVX* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    function = copy_src->function;
    gridDimX = copy_src->gridDimX;
    gridDimY = copy_src->gridDimY;
    gridDimZ = copy_src->gridDimZ;
    blockDimX = copy_src->blockDimX;
    blockDimY = copy_src->blockDimY;
    blockDimZ = copy_src->blockDimZ;
    sharedMemBytes = copy_src->sharedMemBytes;
    paramCount = copy_src->paramCount;
    pParams = copy_src->pParams;
    extraCount = copy_src->extraCount;
    pExtras = copy_src->pExtras;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImageViewHandleInfoNVX::safe_VkImageViewHandleInfoNVX(const VkImageViewHandleInfoNVX* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      imageView(in_struct->imageView),
      descriptorType(in_struct->descriptorType),
      sampler(in_struct->sampler) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImageViewHandleInfoNVX::safe_VkImageViewHandleInfoNVX()
    : sType(VK_STRUCTURE_TYPE_IMAGE_VIEW_HANDLE_INFO_NVX), pNext(nullptr), imageView(), descriptorType(), sampler() {}

safe_VkImageViewHandleInfoNVX::safe_VkImageViewHandleInfoNVX(const safe_VkImageViewHandleInfoNVX& copy_src) {
    sType = copy_src.sType;
    imageView = copy_src.imageView;
    descriptorType = copy_src.descriptorType;
    sampler = copy_src.sampler;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImageViewHandleInfoNVX& safe_VkImageViewHandleInfoNVX::operator=(const safe_VkImageViewHandleInfoNVX& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    imageView = copy_src.imageView;
    descriptorType = copy_src.descriptorType;
    sampler = copy_src.sampler;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImageViewHandleInfoNVX::~safe_VkImageViewHandleInfoNVX() { FreePnextChain(pNext); }

void safe_VkImageViewHandleInfoNVX::initialize(const VkImageViewHandleInfoNVX* in_struct,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    imageView = in_struct->imageView;
    descriptorType = in_struct->descriptorType;
    sampler = in_struct->sampler;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImageViewHandleInfoNVX::initialize(const safe_VkImageViewHandleInfoNVX* copy_src,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    imageView = copy_src->imageView;
    descriptorType = copy_src->descriptorType;
    sampler = copy_src->sampler;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImageViewAddressPropertiesNVX::safe_VkImageViewAddressPropertiesNVX(const VkImageViewAddressPropertiesNVX* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), deviceAddress(in_struct->deviceAddress), size(in_struct->size) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImageViewAddressPropertiesNVX::safe_VkImageViewAddressPropertiesNVX()
    : sType(VK_STRUCTURE_TYPE_IMAGE_VIEW_ADDRESS_PROPERTIES_NVX), pNext(nullptr), deviceAddress(), size() {}

safe_VkImageViewAddressPropertiesNVX::safe_VkImageViewAddressPropertiesNVX(const safe_VkImageViewAddressPropertiesNVX& copy_src) {
    sType = copy_src.sType;
    deviceAddress = copy_src.deviceAddress;
    size = copy_src.size;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImageViewAddressPropertiesNVX& safe_VkImageViewAddressPropertiesNVX::operator=(
    const safe_VkImageViewAddressPropertiesNVX& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    deviceAddress = copy_src.deviceAddress;
    size = copy_src.size;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImageViewAddressPropertiesNVX::~safe_VkImageViewAddressPropertiesNVX() { FreePnextChain(pNext); }

void safe_VkImageViewAddressPropertiesNVX::initialize(const VkImageViewAddressPropertiesNVX* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    deviceAddress = in_struct->deviceAddress;
    size = in_struct->size;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImageViewAddressPropertiesNVX::initialize(const safe_VkImageViewAddressPropertiesNVX* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    deviceAddress = copy_src->deviceAddress;
    size = copy_src->size;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkTextureLODGatherFormatPropertiesAMD::safe_VkTextureLODGatherFormatPropertiesAMD(
    const VkTextureLODGatherFormatPropertiesAMD* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), supportsTextureGatherLODBiasAMD(in_struct->supportsTextureGatherLODBiasAMD) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkTextureLODGatherFormatPropertiesAMD::safe_VkTextureLODGatherFormatPropertiesAMD()
    : sType(VK_STRUCTURE_TYPE_TEXTURE_LOD_GATHER_FORMAT_PROPERTIES_AMD), pNext(nullptr), supportsTextureGatherLODBiasAMD() {}

safe_VkTextureLODGatherFormatPropertiesAMD::safe_VkTextureLODGatherFormatPropertiesAMD(
    const safe_VkTextureLODGatherFormatPropertiesAMD& copy_src) {
    sType = copy_src.sType;
    supportsTextureGatherLODBiasAMD = copy_src.supportsTextureGatherLODBiasAMD;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkTextureLODGatherFormatPropertiesAMD& safe_VkTextureLODGatherFormatPropertiesAMD::operator=(
    const safe_VkTextureLODGatherFormatPropertiesAMD& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    supportsTextureGatherLODBiasAMD = copy_src.supportsTextureGatherLODBiasAMD;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkTextureLODGatherFormatPropertiesAMD::~safe_VkTextureLODGatherFormatPropertiesAMD() { FreePnextChain(pNext); }

void safe_VkTextureLODGatherFormatPropertiesAMD::initialize(const VkTextureLODGatherFormatPropertiesAMD* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    supportsTextureGatherLODBiasAMD = in_struct->supportsTextureGatherLODBiasAMD;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkTextureLODGatherFormatPropertiesAMD::initialize(const safe_VkTextureLODGatherFormatPropertiesAMD* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    supportsTextureGatherLODBiasAMD = copy_src->supportsTextureGatherLODBiasAMD;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_USE_PLATFORM_GGP

safe_VkStreamDescriptorSurfaceCreateInfoGGP::safe_VkStreamDescriptorSurfaceCreateInfoGGP(
    const VkStreamDescriptorSurfaceCreateInfoGGP* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags), streamDescriptor(in_struct->streamDescriptor) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkStreamDescriptorSurfaceCreateInfoGGP::safe_VkStreamDescriptorSurfaceCreateInfoGGP()
    : sType(VK_STRUCTURE_TYPE_STREAM_DESCRIPTOR_SURFACE_CREATE_INFO_GGP), pNext(nullptr), flags(), streamDescriptor() {}

safe_VkStreamDescriptorSurfaceCreateInfoGGP::safe_VkStreamDescriptorSurfaceCreateInfoGGP(
    const safe_VkStreamDescriptorSurfaceCreateInfoGGP& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    streamDescriptor = copy_src.streamDescriptor;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkStreamDescriptorSurfaceCreateInfoGGP& safe_VkStreamDescriptorSurfaceCreateInfoGGP::operator=(
    const safe_VkStreamDescriptorSurfaceCreateInfoGGP& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    streamDescriptor = copy_src.streamDescriptor;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkStreamDescriptorSurfaceCreateInfoGGP::~safe_VkStreamDescriptorSurfaceCreateInfoGGP() { FreePnextChain(pNext); }

void safe_VkStreamDescriptorSurfaceCreateInfoGGP::initialize(const VkStreamDescriptorSurfaceCreateInfoGGP* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    streamDescriptor = in_struct->streamDescriptor;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkStreamDescriptorSurfaceCreateInfoGGP::initialize(const safe_VkStreamDescriptorSurfaceCreateInfoGGP* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    streamDescriptor = copy_src->streamDescriptor;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_GGP

safe_VkPhysicalDeviceCornerSampledImageFeaturesNV::safe_VkPhysicalDeviceCornerSampledImageFeaturesNV(
    const VkPhysicalDeviceCornerSampledImageFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), cornerSampledImage(in_struct->cornerSampledImage) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCornerSampledImageFeaturesNV::safe_VkPhysicalDeviceCornerSampledImageFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CORNER_SAMPLED_IMAGE_FEATURES_NV), pNext(nullptr), cornerSampledImage() {}

safe_VkPhysicalDeviceCornerSampledImageFeaturesNV::safe_VkPhysicalDeviceCornerSampledImageFeaturesNV(
    const safe_VkPhysicalDeviceCornerSampledImageFeaturesNV& copy_src) {
    sType = copy_src.sType;
    cornerSampledImage = copy_src.cornerSampledImage;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCornerSampledImageFeaturesNV& safe_VkPhysicalDeviceCornerSampledImageFeaturesNV::operator=(
    const safe_VkPhysicalDeviceCornerSampledImageFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    cornerSampledImage = copy_src.cornerSampledImage;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCornerSampledImageFeaturesNV::~safe_VkPhysicalDeviceCornerSampledImageFeaturesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceCornerSampledImageFeaturesNV::initialize(const VkPhysicalDeviceCornerSampledImageFeaturesNV* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    cornerSampledImage = in_struct->cornerSampledImage;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCornerSampledImageFeaturesNV::initialize(
    const safe_VkPhysicalDeviceCornerSampledImageFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    cornerSampledImage = copy_src->cornerSampledImage;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkExternalMemoryImageCreateInfoNV::safe_VkExternalMemoryImageCreateInfoNV(const VkExternalMemoryImageCreateInfoNV* in_struct,
                                                                               [[maybe_unused]] PNextCopyState* copy_state,
                                                                               bool copy_pnext)
    : sType(in_struct->sType), handleTypes(in_struct->handleTypes) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkExternalMemoryImageCreateInfoNV::safe_VkExternalMemoryImageCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_EXTERNAL_MEMORY_IMAGE_CREATE_INFO_NV), pNext(nullptr), handleTypes() {}

safe_VkExternalMemoryImageCreateInfoNV::safe_VkExternalMemoryImageCreateInfoNV(
    const safe_VkExternalMemoryImageCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    handleTypes = copy_src.handleTypes;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkExternalMemoryImageCreateInfoNV& safe_VkExternalMemoryImageCreateInfoNV::operator=(
    const safe_VkExternalMemoryImageCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    handleTypes = copy_src.handleTypes;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkExternalMemoryImageCreateInfoNV::~safe_VkExternalMemoryImageCreateInfoNV() { FreePnextChain(pNext); }

void safe_VkExternalMemoryImageCreateInfoNV::initialize(const VkExternalMemoryImageCreateInfoNV* in_struct,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    handleTypes = in_struct->handleTypes;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkExternalMemoryImageCreateInfoNV::initialize(const safe_VkExternalMemoryImageCreateInfoNV* copy_src,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    handleTypes = copy_src->handleTypes;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkExportMemoryAllocateInfoNV::safe_VkExportMemoryAllocateInfoNV(const VkExportMemoryAllocateInfoNV* in_struct,
                                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), handleTypes(in_struct->handleTypes) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkExportMemoryAllocateInfoNV::safe_VkExportMemoryAllocateInfoNV()
    : sType(VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO_NV), pNext(nullptr), handleTypes() {}

safe_VkExportMemoryAllocateInfoNV::safe_VkExportMemoryAllocateInfoNV(const safe_VkExportMemoryAllocateInfoNV& copy_src) {
    sType = copy_src.sType;
    handleTypes = copy_src.handleTypes;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkExportMemoryAllocateInfoNV& safe_VkExportMemoryAllocateInfoNV::operator=(const safe_VkExportMemoryAllocateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    handleTypes = copy_src.handleTypes;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkExportMemoryAllocateInfoNV::~safe_VkExportMemoryAllocateInfoNV() { FreePnextChain(pNext); }

void safe_VkExportMemoryAllocateInfoNV::initialize(const VkExportMemoryAllocateInfoNV* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    handleTypes = in_struct->handleTypes;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkExportMemoryAllocateInfoNV::initialize(const safe_VkExportMemoryAllocateInfoNV* copy_src,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    handleTypes = copy_src->handleTypes;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_USE_PLATFORM_WIN32_KHR

safe_VkImportMemoryWin32HandleInfoNV::safe_VkImportMemoryWin32HandleInfoNV(const VkImportMemoryWin32HandleInfoNV* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), handleType(in_struct->handleType), handle(in_struct->handle) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportMemoryWin32HandleInfoNV::safe_VkImportMemoryWin32HandleInfoNV()
    : sType(VK_STRUCTURE_TYPE_IMPORT_MEMORY_WIN32_HANDLE_INFO_NV), pNext(nullptr), handleType(), handle() {}

safe_VkImportMemoryWin32HandleInfoNV::safe_VkImportMemoryWin32HandleInfoNV(const safe_VkImportMemoryWin32HandleInfoNV& copy_src) {
    sType = copy_src.sType;
    handleType = copy_src.handleType;
    handle = copy_src.handle;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportMemoryWin32HandleInfoNV& safe_VkImportMemoryWin32HandleInfoNV::operator=(
    const safe_VkImportMemoryWin32HandleInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    handleType = copy_src.handleType;
    handle = copy_src.handle;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportMemoryWin32HandleInfoNV::~safe_VkImportMemoryWin32HandleInfoNV() { FreePnextChain(pNext); }

void safe_VkImportMemoryWin32HandleInfoNV::initialize(const VkImportMemoryWin32HandleInfoNV* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    handleType = in_struct->handleType;
    handle = in_struct->handle;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportMemoryWin32HandleInfoNV::initialize(const safe_VkImportMemoryWin32HandleInfoNV* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    handleType = copy_src->handleType;
    handle = copy_src->handle;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkExportMemoryWin32HandleInfoNV::safe_VkExportMemoryWin32HandleInfoNV(const VkExportMemoryWin32HandleInfoNV* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), pAttributes(nullptr), dwAccess(in_struct->dwAccess) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

safe_VkExportMemoryWin32HandleInfoNV::safe_VkExportMemoryWin32HandleInfoNV()
    : sType(VK_STRUCTURE_TYPE_EXPORT_MEMORY_WIN32_HANDLE_INFO_NV), pNext(nullptr), pAttributes(nullptr), dwAccess() {}

safe_VkExportMemoryWin32HandleInfoNV::safe_VkExportMemoryWin32HandleInfoNV(const safe_VkExportMemoryWin32HandleInfoNV& copy_src) {
    sType = copy_src.sType;
    pAttributes = nullptr;
    dwAccess = copy_src.dwAccess;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src.pAttributes);
    }
}

safe_VkExportMemoryWin32HandleInfoNV& safe_VkExportMemoryWin32HandleInfoNV::operator=(
    const safe_VkExportMemoryWin32HandleInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pAttributes = nullptr;
    dwAccess = copy_src.dwAccess;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src.pAttributes);
    }

    return *this;
}

safe_VkExportMemoryWin32HandleInfoNV::~safe_VkExportMemoryWin32HandleInfoNV() {
    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);
}

void safe_VkExportMemoryWin32HandleInfoNV::initialize(const VkExportMemoryWin32HandleInfoNV* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pAttributes) delete pAttributes;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pAttributes = nullptr;
    dwAccess = in_struct->dwAccess;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*in_struct->pAttributes);
    }
}

void safe_VkExportMemoryWin32HandleInfoNV::initialize(const safe_VkExportMemoryWin32HandleInfoNV* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pAttributes = nullptr;
    dwAccess = copy_src->dwAccess;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pAttributes) {
        pAttributes = new SECURITY_ATTRIBUTES(*copy_src->pAttributes);
    }
}

safe_VkWin32KeyedMutexAcquireReleaseInfoNV::safe_VkWin32KeyedMutexAcquireReleaseInfoNV(
    const VkWin32KeyedMutexAcquireReleaseInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      acquireCount(in_struct->acquireCount),
      pAcquireSyncs(nullptr),
      pAcquireKeys(nullptr),
      pAcquireTimeoutMilliseconds(nullptr),
      releaseCount(in_struct->releaseCount),
      pReleaseSyncs(nullptr),
      pReleaseKeys(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (acquireCount && in_struct->pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i = 0; i < acquireCount; ++i) {
            pAcquireSyncs[i] = in_struct->pAcquireSyncs[i];
        }
    }

    if (in_struct->pAcquireKeys) {
        pAcquireKeys = new uint64_t[in_struct->acquireCount];
        memcpy((void*)pAcquireKeys, (void*)in_struct->pAcquireKeys, sizeof(uint64_t) * in_struct->acquireCount);
    }

    if (in_struct->pAcquireTimeoutMilliseconds) {
        pAcquireTimeoutMilliseconds = new uint32_t[in_struct->acquireCount];
        memcpy((void*)pAcquireTimeoutMilliseconds, (void*)in_struct->pAcquireTimeoutMilliseconds,
               sizeof(uint32_t) * in_struct->acquireCount);
    }
    if (releaseCount && in_struct->pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i = 0; i < releaseCount; ++i) {
            pReleaseSyncs[i] = in_struct->pReleaseSyncs[i];
        }
    }

    if (in_struct->pReleaseKeys) {
        pReleaseKeys = new uint64_t[in_struct->releaseCount];
        memcpy((void*)pReleaseKeys, (void*)in_struct->pReleaseKeys, sizeof(uint64_t) * in_struct->releaseCount);
    }
}

safe_VkWin32KeyedMutexAcquireReleaseInfoNV::safe_VkWin32KeyedMutexAcquireReleaseInfoNV()
    : sType(VK_STRUCTURE_TYPE_WIN32_KEYED_MUTEX_ACQUIRE_RELEASE_INFO_NV),
      pNext(nullptr),
      acquireCount(),
      pAcquireSyncs(nullptr),
      pAcquireKeys(nullptr),
      pAcquireTimeoutMilliseconds(nullptr),
      releaseCount(),
      pReleaseSyncs(nullptr),
      pReleaseKeys(nullptr) {}

safe_VkWin32KeyedMutexAcquireReleaseInfoNV::safe_VkWin32KeyedMutexAcquireReleaseInfoNV(
    const safe_VkWin32KeyedMutexAcquireReleaseInfoNV& copy_src) {
    sType = copy_src.sType;
    acquireCount = copy_src.acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeoutMilliseconds = nullptr;
    releaseCount = copy_src.releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (acquireCount && copy_src.pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i = 0; i < acquireCount; ++i) {
            pAcquireSyncs[i] = copy_src.pAcquireSyncs[i];
        }
    }

    if (copy_src.pAcquireKeys) {
        pAcquireKeys = new uint64_t[copy_src.acquireCount];
        memcpy((void*)pAcquireKeys, (void*)copy_src.pAcquireKeys, sizeof(uint64_t) * copy_src.acquireCount);
    }

    if (copy_src.pAcquireTimeoutMilliseconds) {
        pAcquireTimeoutMilliseconds = new uint32_t[copy_src.acquireCount];
        memcpy((void*)pAcquireTimeoutMilliseconds, (void*)copy_src.pAcquireTimeoutMilliseconds,
               sizeof(uint32_t) * copy_src.acquireCount);
    }
    if (releaseCount && copy_src.pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i = 0; i < releaseCount; ++i) {
            pReleaseSyncs[i] = copy_src.pReleaseSyncs[i];
        }
    }

    if (copy_src.pReleaseKeys) {
        pReleaseKeys = new uint64_t[copy_src.releaseCount];
        memcpy((void*)pReleaseKeys, (void*)copy_src.pReleaseKeys, sizeof(uint64_t) * copy_src.releaseCount);
    }
}

safe_VkWin32KeyedMutexAcquireReleaseInfoNV& safe_VkWin32KeyedMutexAcquireReleaseInfoNV::operator=(
    const safe_VkWin32KeyedMutexAcquireReleaseInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pAcquireSyncs) delete[] pAcquireSyncs;
    if (pAcquireKeys) delete[] pAcquireKeys;
    if (pAcquireTimeoutMilliseconds) delete[] pAcquireTimeoutMilliseconds;
    if (pReleaseSyncs) delete[] pReleaseSyncs;
    if (pReleaseKeys) delete[] pReleaseKeys;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    acquireCount = copy_src.acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeoutMilliseconds = nullptr;
    releaseCount = copy_src.releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (acquireCount && copy_src.pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i = 0; i < acquireCount; ++i) {
            pAcquireSyncs[i] = copy_src.pAcquireSyncs[i];
        }
    }

    if (copy_src.pAcquireKeys) {
        pAcquireKeys = new uint64_t[copy_src.acquireCount];
        memcpy((void*)pAcquireKeys, (void*)copy_src.pAcquireKeys, sizeof(uint64_t) * copy_src.acquireCount);
    }

    if (copy_src.pAcquireTimeoutMilliseconds) {
        pAcquireTimeoutMilliseconds = new uint32_t[copy_src.acquireCount];
        memcpy((void*)pAcquireTimeoutMilliseconds, (void*)copy_src.pAcquireTimeoutMilliseconds,
               sizeof(uint32_t) * copy_src.acquireCount);
    }
    if (releaseCount && copy_src.pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i = 0; i < releaseCount; ++i) {
            pReleaseSyncs[i] = copy_src.pReleaseSyncs[i];
        }
    }

    if (copy_src.pReleaseKeys) {
        pReleaseKeys = new uint64_t[copy_src.releaseCount];
        memcpy((void*)pReleaseKeys, (void*)copy_src.pReleaseKeys, sizeof(uint64_t) * copy_src.releaseCount);
    }

    return *this;
}

safe_VkWin32KeyedMutexAcquireReleaseInfoNV::~safe_VkWin32KeyedMutexAcquireReleaseInfoNV() {
    if (pAcquireSyncs) delete[] pAcquireSyncs;
    if (pAcquireKeys) delete[] pAcquireKeys;
    if (pAcquireTimeoutMilliseconds) delete[] pAcquireTimeoutMilliseconds;
    if (pReleaseSyncs) delete[] pReleaseSyncs;
    if (pReleaseKeys) delete[] pReleaseKeys;
    FreePnextChain(pNext);
}

void safe_VkWin32KeyedMutexAcquireReleaseInfoNV::initialize(const VkWin32KeyedMutexAcquireReleaseInfoNV* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    if (pAcquireSyncs) delete[] pAcquireSyncs;
    if (pAcquireKeys) delete[] pAcquireKeys;
    if (pAcquireTimeoutMilliseconds) delete[] pAcquireTimeoutMilliseconds;
    if (pReleaseSyncs) delete[] pReleaseSyncs;
    if (pReleaseKeys) delete[] pReleaseKeys;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    acquireCount = in_struct->acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeoutMilliseconds = nullptr;
    releaseCount = in_struct->releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (acquireCount && in_struct->pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i = 0; i < acquireCount; ++i) {
            pAcquireSyncs[i] = in_struct->pAcquireSyncs[i];
        }
    }

    if (in_struct->pAcquireKeys) {
        pAcquireKeys = new uint64_t[in_struct->acquireCount];
        memcpy((void*)pAcquireKeys, (void*)in_struct->pAcquireKeys, sizeof(uint64_t) * in_struct->acquireCount);
    }

    if (in_struct->pAcquireTimeoutMilliseconds) {
        pAcquireTimeoutMilliseconds = new uint32_t[in_struct->acquireCount];
        memcpy((void*)pAcquireTimeoutMilliseconds, (void*)in_struct->pAcquireTimeoutMilliseconds,
               sizeof(uint32_t) * in_struct->acquireCount);
    }
    if (releaseCount && in_struct->pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i = 0; i < releaseCount; ++i) {
            pReleaseSyncs[i] = in_struct->pReleaseSyncs[i];
        }
    }

    if (in_struct->pReleaseKeys) {
        pReleaseKeys = new uint64_t[in_struct->releaseCount];
        memcpy((void*)pReleaseKeys, (void*)in_struct->pReleaseKeys, sizeof(uint64_t) * in_struct->releaseCount);
    }
}

void safe_VkWin32KeyedMutexAcquireReleaseInfoNV::initialize(const safe_VkWin32KeyedMutexAcquireReleaseInfoNV* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    acquireCount = copy_src->acquireCount;
    pAcquireSyncs = nullptr;
    pAcquireKeys = nullptr;
    pAcquireTimeoutMilliseconds = nullptr;
    releaseCount = copy_src->releaseCount;
    pReleaseSyncs = nullptr;
    pReleaseKeys = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (acquireCount && copy_src->pAcquireSyncs) {
        pAcquireSyncs = new VkDeviceMemory[acquireCount];
        for (uint32_t i = 0; i < acquireCount; ++i) {
            pAcquireSyncs[i] = copy_src->pAcquireSyncs[i];
        }
    }

    if (copy_src->pAcquireKeys) {
        pAcquireKeys = new uint64_t[copy_src->acquireCount];
        memcpy((void*)pAcquireKeys, (void*)copy_src->pAcquireKeys, sizeof(uint64_t) * copy_src->acquireCount);
    }

    if (copy_src->pAcquireTimeoutMilliseconds) {
        pAcquireTimeoutMilliseconds = new uint32_t[copy_src->acquireCount];
        memcpy((void*)pAcquireTimeoutMilliseconds, (void*)copy_src->pAcquireTimeoutMilliseconds,
               sizeof(uint32_t) * copy_src->acquireCount);
    }
    if (releaseCount && copy_src->pReleaseSyncs) {
        pReleaseSyncs = new VkDeviceMemory[releaseCount];
        for (uint32_t i = 0; i < releaseCount; ++i) {
            pReleaseSyncs[i] = copy_src->pReleaseSyncs[i];
        }
    }

    if (copy_src->pReleaseKeys) {
        pReleaseKeys = new uint64_t[copy_src->releaseCount];
        memcpy((void*)pReleaseKeys, (void*)copy_src->pReleaseKeys, sizeof(uint64_t) * copy_src->releaseCount);
    }
}
#endif  // VK_USE_PLATFORM_WIN32_KHR
#ifdef VK_USE_PLATFORM_VI_NN

safe_VkViSurfaceCreateInfoNN::safe_VkViSurfaceCreateInfoNN(const VkViSurfaceCreateInfoNN* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags), window(in_struct->window) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkViSurfaceCreateInfoNN::safe_VkViSurfaceCreateInfoNN()
    : sType(VK_STRUCTURE_TYPE_VI_SURFACE_CREATE_INFO_NN), pNext(nullptr), flags(), window(nullptr) {}

safe_VkViSurfaceCreateInfoNN::safe_VkViSurfaceCreateInfoNN(const safe_VkViSurfaceCreateInfoNN& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    window = copy_src.window;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkViSurfaceCreateInfoNN& safe_VkViSurfaceCreateInfoNN::operator=(const safe_VkViSurfaceCreateInfoNN& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    window = copy_src.window;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkViSurfaceCreateInfoNN::~safe_VkViSurfaceCreateInfoNN() { FreePnextChain(pNext); }

void safe_VkViSurfaceCreateInfoNN::initialize(const VkViSurfaceCreateInfoNN* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    window = in_struct->window;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkViSurfaceCreateInfoNN::initialize(const safe_VkViSurfaceCreateInfoNN* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    window = copy_src->window;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_VI_NN

safe_VkPipelineViewportWScalingStateCreateInfoNV::safe_VkPipelineViewportWScalingStateCreateInfoNV(
    const VkPipelineViewportWScalingStateCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      viewportWScalingEnable(in_struct->viewportWScalingEnable),
      viewportCount(in_struct->viewportCount),
      pViewportWScalings(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pViewportWScalings) {
        pViewportWScalings = new VkViewportWScalingNV[in_struct->viewportCount];
        memcpy((void*)pViewportWScalings, (void*)in_struct->pViewportWScalings,
               sizeof(VkViewportWScalingNV) * in_struct->viewportCount);
    }
}

safe_VkPipelineViewportWScalingStateCreateInfoNV::safe_VkPipelineViewportWScalingStateCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_W_SCALING_STATE_CREATE_INFO_NV),
      pNext(nullptr),
      viewportWScalingEnable(),
      viewportCount(),
      pViewportWScalings(nullptr) {}

safe_VkPipelineViewportWScalingStateCreateInfoNV::safe_VkPipelineViewportWScalingStateCreateInfoNV(
    const safe_VkPipelineViewportWScalingStateCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    viewportWScalingEnable = copy_src.viewportWScalingEnable;
    viewportCount = copy_src.viewportCount;
    pViewportWScalings = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pViewportWScalings) {
        pViewportWScalings = new VkViewportWScalingNV[copy_src.viewportCount];
        memcpy((void*)pViewportWScalings, (void*)copy_src.pViewportWScalings,
               sizeof(VkViewportWScalingNV) * copy_src.viewportCount);
    }
}

safe_VkPipelineViewportWScalingStateCreateInfoNV& safe_VkPipelineViewportWScalingStateCreateInfoNV::operator=(
    const safe_VkPipelineViewportWScalingStateCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pViewportWScalings) delete[] pViewportWScalings;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    viewportWScalingEnable = copy_src.viewportWScalingEnable;
    viewportCount = copy_src.viewportCount;
    pViewportWScalings = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pViewportWScalings) {
        pViewportWScalings = new VkViewportWScalingNV[copy_src.viewportCount];
        memcpy((void*)pViewportWScalings, (void*)copy_src.pViewportWScalings,
               sizeof(VkViewportWScalingNV) * copy_src.viewportCount);
    }

    return *this;
}

safe_VkPipelineViewportWScalingStateCreateInfoNV::~safe_VkPipelineViewportWScalingStateCreateInfoNV() {
    if (pViewportWScalings) delete[] pViewportWScalings;
    FreePnextChain(pNext);
}

void safe_VkPipelineViewportWScalingStateCreateInfoNV::initialize(const VkPipelineViewportWScalingStateCreateInfoNV* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    if (pViewportWScalings) delete[] pViewportWScalings;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    viewportWScalingEnable = in_struct->viewportWScalingEnable;
    viewportCount = in_struct->viewportCount;
    pViewportWScalings = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pViewportWScalings) {
        pViewportWScalings = new VkViewportWScalingNV[in_struct->viewportCount];
        memcpy((void*)pViewportWScalings, (void*)in_struct->pViewportWScalings,
               sizeof(VkViewportWScalingNV) * in_struct->viewportCount);
    }
}

void safe_VkPipelineViewportWScalingStateCreateInfoNV::initialize(const safe_VkPipelineViewportWScalingStateCreateInfoNV* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    viewportWScalingEnable = copy_src->viewportWScalingEnable;
    viewportCount = copy_src->viewportCount;
    pViewportWScalings = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pViewportWScalings) {
        pViewportWScalings = new VkViewportWScalingNV[copy_src->viewportCount];
        memcpy((void*)pViewportWScalings, (void*)copy_src->pViewportWScalings,
               sizeof(VkViewportWScalingNV) * copy_src->viewportCount);
    }
}

safe_VkPresentTimesInfoGOOGLE::safe_VkPresentTimesInfoGOOGLE(const VkPresentTimesInfoGOOGLE* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), swapchainCount(in_struct->swapchainCount), pTimes(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pTimes) {
        pTimes = new VkPresentTimeGOOGLE[in_struct->swapchainCount];
        memcpy((void*)pTimes, (void*)in_struct->pTimes, sizeof(VkPresentTimeGOOGLE) * in_struct->swapchainCount);
    }
}

safe_VkPresentTimesInfoGOOGLE::safe_VkPresentTimesInfoGOOGLE()
    : sType(VK_STRUCTURE_TYPE_PRESENT_TIMES_INFO_GOOGLE), pNext(nullptr), swapchainCount(), pTimes(nullptr) {}

safe_VkPresentTimesInfoGOOGLE::safe_VkPresentTimesInfoGOOGLE(const safe_VkPresentTimesInfoGOOGLE& copy_src) {
    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pTimes = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pTimes) {
        pTimes = new VkPresentTimeGOOGLE[copy_src.swapchainCount];
        memcpy((void*)pTimes, (void*)copy_src.pTimes, sizeof(VkPresentTimeGOOGLE) * copy_src.swapchainCount);
    }
}

safe_VkPresentTimesInfoGOOGLE& safe_VkPresentTimesInfoGOOGLE::operator=(const safe_VkPresentTimesInfoGOOGLE& copy_src) {
    if (&copy_src == this) return *this;

    if (pTimes) delete[] pTimes;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    swapchainCount = copy_src.swapchainCount;
    pTimes = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pTimes) {
        pTimes = new VkPresentTimeGOOGLE[copy_src.swapchainCount];
        memcpy((void*)pTimes, (void*)copy_src.pTimes, sizeof(VkPresentTimeGOOGLE) * copy_src.swapchainCount);
    }

    return *this;
}

safe_VkPresentTimesInfoGOOGLE::~safe_VkPresentTimesInfoGOOGLE() {
    if (pTimes) delete[] pTimes;
    FreePnextChain(pNext);
}

void safe_VkPresentTimesInfoGOOGLE::initialize(const VkPresentTimesInfoGOOGLE* in_struct,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    if (pTimes) delete[] pTimes;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    swapchainCount = in_struct->swapchainCount;
    pTimes = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pTimes) {
        pTimes = new VkPresentTimeGOOGLE[in_struct->swapchainCount];
        memcpy((void*)pTimes, (void*)in_struct->pTimes, sizeof(VkPresentTimeGOOGLE) * in_struct->swapchainCount);
    }
}

void safe_VkPresentTimesInfoGOOGLE::initialize(const safe_VkPresentTimesInfoGOOGLE* copy_src,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    swapchainCount = copy_src->swapchainCount;
    pTimes = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pTimes) {
        pTimes = new VkPresentTimeGOOGLE[copy_src->swapchainCount];
        memcpy((void*)pTimes, (void*)copy_src->pTimes, sizeof(VkPresentTimeGOOGLE) * copy_src->swapchainCount);
    }
}

safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX::safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX(
    const VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), perViewPositionAllComponents(in_struct->perViewPositionAllComponents) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX::safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_ATTRIBUTES_PROPERTIES_NVX),
      pNext(nullptr),
      perViewPositionAllComponents() {}

safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX::safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX(
    const safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX& copy_src) {
    sType = copy_src.sType;
    perViewPositionAllComponents = copy_src.perViewPositionAllComponents;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX&
safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX::operator=(
    const safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    perViewPositionAllComponents = copy_src.perViewPositionAllComponents;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX::~safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX::initialize(
    const VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    perViewPositionAllComponents = in_struct->perViewPositionAllComponents;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX::initialize(
    const safe_VkPhysicalDeviceMultiviewPerViewAttributesPropertiesNVX* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    perViewPositionAllComponents = copy_src->perViewPositionAllComponents;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMultiviewPerViewAttributesInfoNVX::safe_VkMultiviewPerViewAttributesInfoNVX(
    const VkMultiviewPerViewAttributesInfoNVX* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      perViewAttributes(in_struct->perViewAttributes),
      perViewAttributesPositionXOnly(in_struct->perViewAttributesPositionXOnly) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMultiviewPerViewAttributesInfoNVX::safe_VkMultiviewPerViewAttributesInfoNVX()
    : sType(VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_ATTRIBUTES_INFO_NVX),
      pNext(nullptr),
      perViewAttributes(),
      perViewAttributesPositionXOnly() {}

safe_VkMultiviewPerViewAttributesInfoNVX::safe_VkMultiviewPerViewAttributesInfoNVX(
    const safe_VkMultiviewPerViewAttributesInfoNVX& copy_src) {
    sType = copy_src.sType;
    perViewAttributes = copy_src.perViewAttributes;
    perViewAttributesPositionXOnly = copy_src.perViewAttributesPositionXOnly;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMultiviewPerViewAttributesInfoNVX& safe_VkMultiviewPerViewAttributesInfoNVX::operator=(
    const safe_VkMultiviewPerViewAttributesInfoNVX& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    perViewAttributes = copy_src.perViewAttributes;
    perViewAttributesPositionXOnly = copy_src.perViewAttributesPositionXOnly;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMultiviewPerViewAttributesInfoNVX::~safe_VkMultiviewPerViewAttributesInfoNVX() { FreePnextChain(pNext); }

void safe_VkMultiviewPerViewAttributesInfoNVX::initialize(const VkMultiviewPerViewAttributesInfoNVX* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    perViewAttributes = in_struct->perViewAttributes;
    perViewAttributesPositionXOnly = in_struct->perViewAttributesPositionXOnly;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMultiviewPerViewAttributesInfoNVX::initialize(const safe_VkMultiviewPerViewAttributesInfoNVX* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    perViewAttributes = copy_src->perViewAttributes;
    perViewAttributesPositionXOnly = copy_src->perViewAttributesPositionXOnly;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineViewportSwizzleStateCreateInfoNV::safe_VkPipelineViewportSwizzleStateCreateInfoNV(
    const VkPipelineViewportSwizzleStateCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags), viewportCount(in_struct->viewportCount), pViewportSwizzles(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pViewportSwizzles) {
        pViewportSwizzles = new VkViewportSwizzleNV[in_struct->viewportCount];
        memcpy((void*)pViewportSwizzles, (void*)in_struct->pViewportSwizzles,
               sizeof(VkViewportSwizzleNV) * in_struct->viewportCount);
    }
}

safe_VkPipelineViewportSwizzleStateCreateInfoNV::safe_VkPipelineViewportSwizzleStateCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SWIZZLE_STATE_CREATE_INFO_NV),
      pNext(nullptr),
      flags(),
      viewportCount(),
      pViewportSwizzles(nullptr) {}

safe_VkPipelineViewportSwizzleStateCreateInfoNV::safe_VkPipelineViewportSwizzleStateCreateInfoNV(
    const safe_VkPipelineViewportSwizzleStateCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    viewportCount = copy_src.viewportCount;
    pViewportSwizzles = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pViewportSwizzles) {
        pViewportSwizzles = new VkViewportSwizzleNV[copy_src.viewportCount];
        memcpy((void*)pViewportSwizzles, (void*)copy_src.pViewportSwizzles, sizeof(VkViewportSwizzleNV) * copy_src.viewportCount);
    }
}

safe_VkPipelineViewportSwizzleStateCreateInfoNV& safe_VkPipelineViewportSwizzleStateCreateInfoNV::operator=(
    const safe_VkPipelineViewportSwizzleStateCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pViewportSwizzles) delete[] pViewportSwizzles;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    viewportCount = copy_src.viewportCount;
    pViewportSwizzles = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pViewportSwizzles) {
        pViewportSwizzles = new VkViewportSwizzleNV[copy_src.viewportCount];
        memcpy((void*)pViewportSwizzles, (void*)copy_src.pViewportSwizzles, sizeof(VkViewportSwizzleNV) * copy_src.viewportCount);
    }

    return *this;
}

safe_VkPipelineViewportSwizzleStateCreateInfoNV::~safe_VkPipelineViewportSwizzleStateCreateInfoNV() {
    if (pViewportSwizzles) delete[] pViewportSwizzles;
    FreePnextChain(pNext);
}

void safe_VkPipelineViewportSwizzleStateCreateInfoNV::initialize(const VkPipelineViewportSwizzleStateCreateInfoNV* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    if (pViewportSwizzles) delete[] pViewportSwizzles;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    viewportCount = in_struct->viewportCount;
    pViewportSwizzles = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pViewportSwizzles) {
        pViewportSwizzles = new VkViewportSwizzleNV[in_struct->viewportCount];
        memcpy((void*)pViewportSwizzles, (void*)in_struct->pViewportSwizzles,
               sizeof(VkViewportSwizzleNV) * in_struct->viewportCount);
    }
}

void safe_VkPipelineViewportSwizzleStateCreateInfoNV::initialize(const safe_VkPipelineViewportSwizzleStateCreateInfoNV* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    viewportCount = copy_src->viewportCount;
    pViewportSwizzles = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pViewportSwizzles) {
        pViewportSwizzles = new VkViewportSwizzleNV[copy_src->viewportCount];
        memcpy((void*)pViewportSwizzles, (void*)copy_src->pViewportSwizzles, sizeof(VkViewportSwizzleNV) * copy_src->viewportCount);
    }
}

safe_VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG::safe_VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG(
    const VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), relaxedLineRasterization(in_struct->relaxedLineRasterization) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG::safe_VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RELAXED_LINE_RASTERIZATION_FEATURES_IMG),
      pNext(nullptr),
      relaxedLineRasterization() {}

safe_VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG::safe_VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG(
    const safe_VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG& copy_src) {
    sType = copy_src.sType;
    relaxedLineRasterization = copy_src.relaxedLineRasterization;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG& safe_VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG::operator=(
    const safe_VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    relaxedLineRasterization = copy_src.relaxedLineRasterization;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG::~safe_VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG::initialize(
    const VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    relaxedLineRasterization = in_struct->relaxedLineRasterization;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG::initialize(
    const safe_VkPhysicalDeviceRelaxedLineRasterizationFeaturesIMG* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    relaxedLineRasterization = copy_src->relaxedLineRasterization;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_USE_PLATFORM_ANDROID_KHR

safe_VkAndroidHardwareBufferUsageANDROID::safe_VkAndroidHardwareBufferUsageANDROID(
    const VkAndroidHardwareBufferUsageANDROID* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), androidHardwareBufferUsage(in_struct->androidHardwareBufferUsage) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAndroidHardwareBufferUsageANDROID::safe_VkAndroidHardwareBufferUsageANDROID()
    : sType(VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_USAGE_ANDROID), pNext(nullptr), androidHardwareBufferUsage() {}

safe_VkAndroidHardwareBufferUsageANDROID::safe_VkAndroidHardwareBufferUsageANDROID(
    const safe_VkAndroidHardwareBufferUsageANDROID& copy_src) {
    sType = copy_src.sType;
    androidHardwareBufferUsage = copy_src.androidHardwareBufferUsage;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAndroidHardwareBufferUsageANDROID& safe_VkAndroidHardwareBufferUsageANDROID::operator=(
    const safe_VkAndroidHardwareBufferUsageANDROID& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    androidHardwareBufferUsage = copy_src.androidHardwareBufferUsage;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAndroidHardwareBufferUsageANDROID::~safe_VkAndroidHardwareBufferUsageANDROID() { FreePnextChain(pNext); }

void safe_VkAndroidHardwareBufferUsageANDROID::initialize(const VkAndroidHardwareBufferUsageANDROID* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    androidHardwareBufferUsage = in_struct->androidHardwareBufferUsage;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAndroidHardwareBufferUsageANDROID::initialize(const safe_VkAndroidHardwareBufferUsageANDROID* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    androidHardwareBufferUsage = copy_src->androidHardwareBufferUsage;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAndroidHardwareBufferPropertiesANDROID::safe_VkAndroidHardwareBufferPropertiesANDROID(
    const VkAndroidHardwareBufferPropertiesANDROID* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), allocationSize(in_struct->allocationSize), memoryTypeBits(in_struct->memoryTypeBits) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAndroidHardwareBufferPropertiesANDROID::safe_VkAndroidHardwareBufferPropertiesANDROID()
    : sType(VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_PROPERTIES_ANDROID), pNext(nullptr), allocationSize(), memoryTypeBits() {}

safe_VkAndroidHardwareBufferPropertiesANDROID::safe_VkAndroidHardwareBufferPropertiesANDROID(
    const safe_VkAndroidHardwareBufferPropertiesANDROID& copy_src) {
    sType = copy_src.sType;
    allocationSize = copy_src.allocationSize;
    memoryTypeBits = copy_src.memoryTypeBits;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAndroidHardwareBufferPropertiesANDROID& safe_VkAndroidHardwareBufferPropertiesANDROID::operator=(
    const safe_VkAndroidHardwareBufferPropertiesANDROID& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    allocationSize = copy_src.allocationSize;
    memoryTypeBits = copy_src.memoryTypeBits;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAndroidHardwareBufferPropertiesANDROID::~safe_VkAndroidHardwareBufferPropertiesANDROID() { FreePnextChain(pNext); }

void safe_VkAndroidHardwareBufferPropertiesANDROID::initialize(const VkAndroidHardwareBufferPropertiesANDROID* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    allocationSize = in_struct->allocationSize;
    memoryTypeBits = in_struct->memoryTypeBits;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAndroidHardwareBufferPropertiesANDROID::initialize(const safe_VkAndroidHardwareBufferPropertiesANDROID* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    allocationSize = copy_src->allocationSize;
    memoryTypeBits = copy_src->memoryTypeBits;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAndroidHardwareBufferFormatPropertiesANDROID::safe_VkAndroidHardwareBufferFormatPropertiesANDROID(
    const VkAndroidHardwareBufferFormatPropertiesANDROID* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      format(in_struct->format),
      externalFormat(in_struct->externalFormat),
      formatFeatures(in_struct->formatFeatures),
      samplerYcbcrConversionComponents(in_struct->samplerYcbcrConversionComponents),
      suggestedYcbcrModel(in_struct->suggestedYcbcrModel),
      suggestedYcbcrRange(in_struct->suggestedYcbcrRange),
      suggestedXChromaOffset(in_struct->suggestedXChromaOffset),
      suggestedYChromaOffset(in_struct->suggestedYChromaOffset) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAndroidHardwareBufferFormatPropertiesANDROID::safe_VkAndroidHardwareBufferFormatPropertiesANDROID()
    : sType(VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_ANDROID),
      pNext(nullptr),
      format(),
      externalFormat(),
      formatFeatures(),
      samplerYcbcrConversionComponents(),
      suggestedYcbcrModel(),
      suggestedYcbcrRange(),
      suggestedXChromaOffset(),
      suggestedYChromaOffset() {}

safe_VkAndroidHardwareBufferFormatPropertiesANDROID::safe_VkAndroidHardwareBufferFormatPropertiesANDROID(
    const safe_VkAndroidHardwareBufferFormatPropertiesANDROID& copy_src) {
    sType = copy_src.sType;
    format = copy_src.format;
    externalFormat = copy_src.externalFormat;
    formatFeatures = copy_src.formatFeatures;
    samplerYcbcrConversionComponents = copy_src.samplerYcbcrConversionComponents;
    suggestedYcbcrModel = copy_src.suggestedYcbcrModel;
    suggestedYcbcrRange = copy_src.suggestedYcbcrRange;
    suggestedXChromaOffset = copy_src.suggestedXChromaOffset;
    suggestedYChromaOffset = copy_src.suggestedYChromaOffset;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAndroidHardwareBufferFormatPropertiesANDROID& safe_VkAndroidHardwareBufferFormatPropertiesANDROID::operator=(
    const safe_VkAndroidHardwareBufferFormatPropertiesANDROID& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    format = copy_src.format;
    externalFormat = copy_src.externalFormat;
    formatFeatures = copy_src.formatFeatures;
    samplerYcbcrConversionComponents = copy_src.samplerYcbcrConversionComponents;
    suggestedYcbcrModel = copy_src.suggestedYcbcrModel;
    suggestedYcbcrRange = copy_src.suggestedYcbcrRange;
    suggestedXChromaOffset = copy_src.suggestedXChromaOffset;
    suggestedYChromaOffset = copy_src.suggestedYChromaOffset;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAndroidHardwareBufferFormatPropertiesANDROID::~safe_VkAndroidHardwareBufferFormatPropertiesANDROID() {
    FreePnextChain(pNext);
}

void safe_VkAndroidHardwareBufferFormatPropertiesANDROID::initialize(
    const VkAndroidHardwareBufferFormatPropertiesANDROID* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    format = in_struct->format;
    externalFormat = in_struct->externalFormat;
    formatFeatures = in_struct->formatFeatures;
    samplerYcbcrConversionComponents = in_struct->samplerYcbcrConversionComponents;
    suggestedYcbcrModel = in_struct->suggestedYcbcrModel;
    suggestedYcbcrRange = in_struct->suggestedYcbcrRange;
    suggestedXChromaOffset = in_struct->suggestedXChromaOffset;
    suggestedYChromaOffset = in_struct->suggestedYChromaOffset;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAndroidHardwareBufferFormatPropertiesANDROID::initialize(
    const safe_VkAndroidHardwareBufferFormatPropertiesANDROID* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    format = copy_src->format;
    externalFormat = copy_src->externalFormat;
    formatFeatures = copy_src->formatFeatures;
    samplerYcbcrConversionComponents = copy_src->samplerYcbcrConversionComponents;
    suggestedYcbcrModel = copy_src->suggestedYcbcrModel;
    suggestedYcbcrRange = copy_src->suggestedYcbcrRange;
    suggestedXChromaOffset = copy_src->suggestedXChromaOffset;
    suggestedYChromaOffset = copy_src->suggestedYChromaOffset;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImportAndroidHardwareBufferInfoANDROID::safe_VkImportAndroidHardwareBufferInfoANDROID(
    const VkImportAndroidHardwareBufferInfoANDROID* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), buffer(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    buffer = in_struct->buffer;
}

safe_VkImportAndroidHardwareBufferInfoANDROID::safe_VkImportAndroidHardwareBufferInfoANDROID()
    : sType(VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID), pNext(nullptr), buffer(nullptr) {}

safe_VkImportAndroidHardwareBufferInfoANDROID::safe_VkImportAndroidHardwareBufferInfoANDROID(
    const safe_VkImportAndroidHardwareBufferInfoANDROID& copy_src) {
    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);
    buffer = copy_src.buffer;
}

safe_VkImportAndroidHardwareBufferInfoANDROID& safe_VkImportAndroidHardwareBufferInfoANDROID::operator=(
    const safe_VkImportAndroidHardwareBufferInfoANDROID& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);
    buffer = copy_src.buffer;

    return *this;
}

safe_VkImportAndroidHardwareBufferInfoANDROID::~safe_VkImportAndroidHardwareBufferInfoANDROID() { FreePnextChain(pNext); }

void safe_VkImportAndroidHardwareBufferInfoANDROID::initialize(const VkImportAndroidHardwareBufferInfoANDROID* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    buffer = in_struct->buffer;
}

void safe_VkImportAndroidHardwareBufferInfoANDROID::initialize(const safe_VkImportAndroidHardwareBufferInfoANDROID* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pNext = SafePnextCopy(copy_src->pNext);
    buffer = copy_src->buffer;
}

safe_VkMemoryGetAndroidHardwareBufferInfoANDROID::safe_VkMemoryGetAndroidHardwareBufferInfoANDROID(
    const VkMemoryGetAndroidHardwareBufferInfoANDROID* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), memory(in_struct->memory) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryGetAndroidHardwareBufferInfoANDROID::safe_VkMemoryGetAndroidHardwareBufferInfoANDROID()
    : sType(VK_STRUCTURE_TYPE_MEMORY_GET_ANDROID_HARDWARE_BUFFER_INFO_ANDROID), pNext(nullptr), memory() {}

safe_VkMemoryGetAndroidHardwareBufferInfoANDROID::safe_VkMemoryGetAndroidHardwareBufferInfoANDROID(
    const safe_VkMemoryGetAndroidHardwareBufferInfoANDROID& copy_src) {
    sType = copy_src.sType;
    memory = copy_src.memory;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryGetAndroidHardwareBufferInfoANDROID& safe_VkMemoryGetAndroidHardwareBufferInfoANDROID::operator=(
    const safe_VkMemoryGetAndroidHardwareBufferInfoANDROID& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memory = copy_src.memory;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryGetAndroidHardwareBufferInfoANDROID::~safe_VkMemoryGetAndroidHardwareBufferInfoANDROID() { FreePnextChain(pNext); }

void safe_VkMemoryGetAndroidHardwareBufferInfoANDROID::initialize(const VkMemoryGetAndroidHardwareBufferInfoANDROID* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memory = in_struct->memory;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryGetAndroidHardwareBufferInfoANDROID::initialize(const safe_VkMemoryGetAndroidHardwareBufferInfoANDROID* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memory = copy_src->memory;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkExternalFormatANDROID::safe_VkExternalFormatANDROID(const VkExternalFormatANDROID* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), externalFormat(in_struct->externalFormat) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkExternalFormatANDROID::safe_VkExternalFormatANDROID()
    : sType(VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_ANDROID), pNext(nullptr), externalFormat() {}

safe_VkExternalFormatANDROID::safe_VkExternalFormatANDROID(const safe_VkExternalFormatANDROID& copy_src) {
    sType = copy_src.sType;
    externalFormat = copy_src.externalFormat;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkExternalFormatANDROID& safe_VkExternalFormatANDROID::operator=(const safe_VkExternalFormatANDROID& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    externalFormat = copy_src.externalFormat;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkExternalFormatANDROID::~safe_VkExternalFormatANDROID() { FreePnextChain(pNext); }

void safe_VkExternalFormatANDROID::initialize(const VkExternalFormatANDROID* in_struct,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    externalFormat = in_struct->externalFormat;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkExternalFormatANDROID::initialize(const safe_VkExternalFormatANDROID* copy_src,
                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    externalFormat = copy_src->externalFormat;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAndroidHardwareBufferFormatProperties2ANDROID::safe_VkAndroidHardwareBufferFormatProperties2ANDROID(
    const VkAndroidHardwareBufferFormatProperties2ANDROID* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      format(in_struct->format),
      externalFormat(in_struct->externalFormat),
      formatFeatures(in_struct->formatFeatures),
      samplerYcbcrConversionComponents(in_struct->samplerYcbcrConversionComponents),
      suggestedYcbcrModel(in_struct->suggestedYcbcrModel),
      suggestedYcbcrRange(in_struct->suggestedYcbcrRange),
      suggestedXChromaOffset(in_struct->suggestedXChromaOffset),
      suggestedYChromaOffset(in_struct->suggestedYChromaOffset) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAndroidHardwareBufferFormatProperties2ANDROID::safe_VkAndroidHardwareBufferFormatProperties2ANDROID()
    : sType(VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_PROPERTIES_2_ANDROID),
      pNext(nullptr),
      format(),
      externalFormat(),
      formatFeatures(),
      samplerYcbcrConversionComponents(),
      suggestedYcbcrModel(),
      suggestedYcbcrRange(),
      suggestedXChromaOffset(),
      suggestedYChromaOffset() {}

safe_VkAndroidHardwareBufferFormatProperties2ANDROID::safe_VkAndroidHardwareBufferFormatProperties2ANDROID(
    const safe_VkAndroidHardwareBufferFormatProperties2ANDROID& copy_src) {
    sType = copy_src.sType;
    format = copy_src.format;
    externalFormat = copy_src.externalFormat;
    formatFeatures = copy_src.formatFeatures;
    samplerYcbcrConversionComponents = copy_src.samplerYcbcrConversionComponents;
    suggestedYcbcrModel = copy_src.suggestedYcbcrModel;
    suggestedYcbcrRange = copy_src.suggestedYcbcrRange;
    suggestedXChromaOffset = copy_src.suggestedXChromaOffset;
    suggestedYChromaOffset = copy_src.suggestedYChromaOffset;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAndroidHardwareBufferFormatProperties2ANDROID& safe_VkAndroidHardwareBufferFormatProperties2ANDROID::operator=(
    const safe_VkAndroidHardwareBufferFormatProperties2ANDROID& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    format = copy_src.format;
    externalFormat = copy_src.externalFormat;
    formatFeatures = copy_src.formatFeatures;
    samplerYcbcrConversionComponents = copy_src.samplerYcbcrConversionComponents;
    suggestedYcbcrModel = copy_src.suggestedYcbcrModel;
    suggestedYcbcrRange = copy_src.suggestedYcbcrRange;
    suggestedXChromaOffset = copy_src.suggestedXChromaOffset;
    suggestedYChromaOffset = copy_src.suggestedYChromaOffset;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAndroidHardwareBufferFormatProperties2ANDROID::~safe_VkAndroidHardwareBufferFormatProperties2ANDROID() {
    FreePnextChain(pNext);
}

void safe_VkAndroidHardwareBufferFormatProperties2ANDROID::initialize(
    const VkAndroidHardwareBufferFormatProperties2ANDROID* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    format = in_struct->format;
    externalFormat = in_struct->externalFormat;
    formatFeatures = in_struct->formatFeatures;
    samplerYcbcrConversionComponents = in_struct->samplerYcbcrConversionComponents;
    suggestedYcbcrModel = in_struct->suggestedYcbcrModel;
    suggestedYcbcrRange = in_struct->suggestedYcbcrRange;
    suggestedXChromaOffset = in_struct->suggestedXChromaOffset;
    suggestedYChromaOffset = in_struct->suggestedYChromaOffset;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAndroidHardwareBufferFormatProperties2ANDROID::initialize(
    const safe_VkAndroidHardwareBufferFormatProperties2ANDROID* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    format = copy_src->format;
    externalFormat = copy_src->externalFormat;
    formatFeatures = copy_src->formatFeatures;
    samplerYcbcrConversionComponents = copy_src->samplerYcbcrConversionComponents;
    suggestedYcbcrModel = copy_src->suggestedYcbcrModel;
    suggestedYcbcrRange = copy_src->suggestedYcbcrRange;
    suggestedXChromaOffset = copy_src->suggestedXChromaOffset;
    suggestedYChromaOffset = copy_src->suggestedYChromaOffset;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR
#ifdef VK_ENABLE_BETA_EXTENSIONS

safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX::safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX(
    const VkPhysicalDeviceShaderEnqueueFeaturesAMDX* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), shaderEnqueue(in_struct->shaderEnqueue), shaderMeshEnqueue(in_struct->shaderMeshEnqueue) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX::safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_FEATURES_AMDX), pNext(nullptr), shaderEnqueue(), shaderMeshEnqueue() {}

safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX::safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX(
    const safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX& copy_src) {
    sType = copy_src.sType;
    shaderEnqueue = copy_src.shaderEnqueue;
    shaderMeshEnqueue = copy_src.shaderMeshEnqueue;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX& safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX::operator=(
    const safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderEnqueue = copy_src.shaderEnqueue;
    shaderMeshEnqueue = copy_src.shaderMeshEnqueue;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX::~safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX::initialize(const VkPhysicalDeviceShaderEnqueueFeaturesAMDX* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderEnqueue = in_struct->shaderEnqueue;
    shaderMeshEnqueue = in_struct->shaderMeshEnqueue;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX::initialize(const safe_VkPhysicalDeviceShaderEnqueueFeaturesAMDX* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderEnqueue = copy_src->shaderEnqueue;
    shaderMeshEnqueue = copy_src->shaderMeshEnqueue;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX::safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX(
    const VkPhysicalDeviceShaderEnqueuePropertiesAMDX* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      maxExecutionGraphDepth(in_struct->maxExecutionGraphDepth),
      maxExecutionGraphShaderOutputNodes(in_struct->maxExecutionGraphShaderOutputNodes),
      maxExecutionGraphShaderPayloadSize(in_struct->maxExecutionGraphShaderPayloadSize),
      maxExecutionGraphShaderPayloadCount(in_struct->maxExecutionGraphShaderPayloadCount),
      executionGraphDispatchAddressAlignment(in_struct->executionGraphDispatchAddressAlignment),
      maxExecutionGraphWorkgroups(in_struct->maxExecutionGraphWorkgroups) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < 3; ++i) {
        maxExecutionGraphWorkgroupCount[i] = in_struct->maxExecutionGraphWorkgroupCount[i];
    }
}

safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX::safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ENQUEUE_PROPERTIES_AMDX),
      pNext(nullptr),
      maxExecutionGraphDepth(),
      maxExecutionGraphShaderOutputNodes(),
      maxExecutionGraphShaderPayloadSize(),
      maxExecutionGraphShaderPayloadCount(),
      executionGraphDispatchAddressAlignment(),
      maxExecutionGraphWorkgroups() {}

safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX::safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX(
    const safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX& copy_src) {
    sType = copy_src.sType;
    maxExecutionGraphDepth = copy_src.maxExecutionGraphDepth;
    maxExecutionGraphShaderOutputNodes = copy_src.maxExecutionGraphShaderOutputNodes;
    maxExecutionGraphShaderPayloadSize = copy_src.maxExecutionGraphShaderPayloadSize;
    maxExecutionGraphShaderPayloadCount = copy_src.maxExecutionGraphShaderPayloadCount;
    executionGraphDispatchAddressAlignment = copy_src.executionGraphDispatchAddressAlignment;
    maxExecutionGraphWorkgroups = copy_src.maxExecutionGraphWorkgroups;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < 3; ++i) {
        maxExecutionGraphWorkgroupCount[i] = copy_src.maxExecutionGraphWorkgroupCount[i];
    }
}

safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX& safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX::operator=(
    const safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxExecutionGraphDepth = copy_src.maxExecutionGraphDepth;
    maxExecutionGraphShaderOutputNodes = copy_src.maxExecutionGraphShaderOutputNodes;
    maxExecutionGraphShaderPayloadSize = copy_src.maxExecutionGraphShaderPayloadSize;
    maxExecutionGraphShaderPayloadCount = copy_src.maxExecutionGraphShaderPayloadCount;
    executionGraphDispatchAddressAlignment = copy_src.executionGraphDispatchAddressAlignment;
    maxExecutionGraphWorkgroups = copy_src.maxExecutionGraphWorkgroups;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < 3; ++i) {
        maxExecutionGraphWorkgroupCount[i] = copy_src.maxExecutionGraphWorkgroupCount[i];
    }

    return *this;
}

safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX::~safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX::initialize(const VkPhysicalDeviceShaderEnqueuePropertiesAMDX* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxExecutionGraphDepth = in_struct->maxExecutionGraphDepth;
    maxExecutionGraphShaderOutputNodes = in_struct->maxExecutionGraphShaderOutputNodes;
    maxExecutionGraphShaderPayloadSize = in_struct->maxExecutionGraphShaderPayloadSize;
    maxExecutionGraphShaderPayloadCount = in_struct->maxExecutionGraphShaderPayloadCount;
    executionGraphDispatchAddressAlignment = in_struct->executionGraphDispatchAddressAlignment;
    maxExecutionGraphWorkgroups = in_struct->maxExecutionGraphWorkgroups;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < 3; ++i) {
        maxExecutionGraphWorkgroupCount[i] = in_struct->maxExecutionGraphWorkgroupCount[i];
    }
}

void safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX::initialize(const safe_VkPhysicalDeviceShaderEnqueuePropertiesAMDX* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxExecutionGraphDepth = copy_src->maxExecutionGraphDepth;
    maxExecutionGraphShaderOutputNodes = copy_src->maxExecutionGraphShaderOutputNodes;
    maxExecutionGraphShaderPayloadSize = copy_src->maxExecutionGraphShaderPayloadSize;
    maxExecutionGraphShaderPayloadCount = copy_src->maxExecutionGraphShaderPayloadCount;
    executionGraphDispatchAddressAlignment = copy_src->executionGraphDispatchAddressAlignment;
    maxExecutionGraphWorkgroups = copy_src->maxExecutionGraphWorkgroups;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < 3; ++i) {
        maxExecutionGraphWorkgroupCount[i] = copy_src->maxExecutionGraphWorkgroupCount[i];
    }
}

safe_VkExecutionGraphPipelineScratchSizeAMDX::safe_VkExecutionGraphPipelineScratchSizeAMDX(
    const VkExecutionGraphPipelineScratchSizeAMDX* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      minSize(in_struct->minSize),
      maxSize(in_struct->maxSize),
      sizeGranularity(in_struct->sizeGranularity) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkExecutionGraphPipelineScratchSizeAMDX::safe_VkExecutionGraphPipelineScratchSizeAMDX()
    : sType(VK_STRUCTURE_TYPE_EXECUTION_GRAPH_PIPELINE_SCRATCH_SIZE_AMDX),
      pNext(nullptr),
      minSize(),
      maxSize(),
      sizeGranularity() {}

safe_VkExecutionGraphPipelineScratchSizeAMDX::safe_VkExecutionGraphPipelineScratchSizeAMDX(
    const safe_VkExecutionGraphPipelineScratchSizeAMDX& copy_src) {
    sType = copy_src.sType;
    minSize = copy_src.minSize;
    maxSize = copy_src.maxSize;
    sizeGranularity = copy_src.sizeGranularity;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkExecutionGraphPipelineScratchSizeAMDX& safe_VkExecutionGraphPipelineScratchSizeAMDX::operator=(
    const safe_VkExecutionGraphPipelineScratchSizeAMDX& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    minSize = copy_src.minSize;
    maxSize = copy_src.maxSize;
    sizeGranularity = copy_src.sizeGranularity;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkExecutionGraphPipelineScratchSizeAMDX::~safe_VkExecutionGraphPipelineScratchSizeAMDX() { FreePnextChain(pNext); }

void safe_VkExecutionGraphPipelineScratchSizeAMDX::initialize(const VkExecutionGraphPipelineScratchSizeAMDX* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    minSize = in_struct->minSize;
    maxSize = in_struct->maxSize;
    sizeGranularity = in_struct->sizeGranularity;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkExecutionGraphPipelineScratchSizeAMDX::initialize(const safe_VkExecutionGraphPipelineScratchSizeAMDX* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    minSize = copy_src->minSize;
    maxSize = copy_src->maxSize;
    sizeGranularity = copy_src->sizeGranularity;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkExecutionGraphPipelineCreateInfoAMDX::safe_VkExecutionGraphPipelineCreateInfoAMDX(
    const VkExecutionGraphPipelineCreateInfoAMDX* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      stageCount(in_struct->stageCount),
      pStages(nullptr),
      pLibraryInfo(nullptr),
      layout(in_struct->layout),
      basePipelineHandle(in_struct->basePipelineHandle),
      basePipelineIndex(in_struct->basePipelineIndex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (stageCount && in_struct->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&in_struct->pStages[i]);
        }
    }
    if (in_struct->pLibraryInfo) pLibraryInfo = new safe_VkPipelineLibraryCreateInfoKHR(in_struct->pLibraryInfo);
}

safe_VkExecutionGraphPipelineCreateInfoAMDX::safe_VkExecutionGraphPipelineCreateInfoAMDX()
    : sType(VK_STRUCTURE_TYPE_EXECUTION_GRAPH_PIPELINE_CREATE_INFO_AMDX),
      pNext(nullptr),
      flags(),
      stageCount(),
      pStages(nullptr),
      pLibraryInfo(nullptr),
      layout(),
      basePipelineHandle(),
      basePipelineIndex() {}

safe_VkExecutionGraphPipelineCreateInfoAMDX::safe_VkExecutionGraphPipelineCreateInfoAMDX(
    const safe_VkExecutionGraphPipelineCreateInfoAMDX& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    stageCount = copy_src.stageCount;
    pStages = nullptr;
    pLibraryInfo = nullptr;
    layout = copy_src.layout;
    basePipelineHandle = copy_src.basePipelineHandle;
    basePipelineIndex = copy_src.basePipelineIndex;
    pNext = SafePnextCopy(copy_src.pNext);
    if (stageCount && copy_src.pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&copy_src.pStages[i]);
        }
    }
    if (copy_src.pLibraryInfo) pLibraryInfo = new safe_VkPipelineLibraryCreateInfoKHR(*copy_src.pLibraryInfo);
}

safe_VkExecutionGraphPipelineCreateInfoAMDX& safe_VkExecutionGraphPipelineCreateInfoAMDX::operator=(
    const safe_VkExecutionGraphPipelineCreateInfoAMDX& copy_src) {
    if (&copy_src == this) return *this;

    if (pStages) delete[] pStages;
    if (pLibraryInfo) delete pLibraryInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    stageCount = copy_src.stageCount;
    pStages = nullptr;
    pLibraryInfo = nullptr;
    layout = copy_src.layout;
    basePipelineHandle = copy_src.basePipelineHandle;
    basePipelineIndex = copy_src.basePipelineIndex;
    pNext = SafePnextCopy(copy_src.pNext);
    if (stageCount && copy_src.pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&copy_src.pStages[i]);
        }
    }
    if (copy_src.pLibraryInfo) pLibraryInfo = new safe_VkPipelineLibraryCreateInfoKHR(*copy_src.pLibraryInfo);

    return *this;
}

safe_VkExecutionGraphPipelineCreateInfoAMDX::~safe_VkExecutionGraphPipelineCreateInfoAMDX() {
    if (pStages) delete[] pStages;
    if (pLibraryInfo) delete pLibraryInfo;
    FreePnextChain(pNext);
}

void safe_VkExecutionGraphPipelineCreateInfoAMDX::initialize(const VkExecutionGraphPipelineCreateInfoAMDX* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStages) delete[] pStages;
    if (pLibraryInfo) delete pLibraryInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    stageCount = in_struct->stageCount;
    pStages = nullptr;
    pLibraryInfo = nullptr;
    layout = in_struct->layout;
    basePipelineHandle = in_struct->basePipelineHandle;
    basePipelineIndex = in_struct->basePipelineIndex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (stageCount && in_struct->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&in_struct->pStages[i]);
        }
    }
    if (in_struct->pLibraryInfo) pLibraryInfo = new safe_VkPipelineLibraryCreateInfoKHR(in_struct->pLibraryInfo);
}

void safe_VkExecutionGraphPipelineCreateInfoAMDX::initialize(const safe_VkExecutionGraphPipelineCreateInfoAMDX* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    stageCount = copy_src->stageCount;
    pStages = nullptr;
    pLibraryInfo = nullptr;
    layout = copy_src->layout;
    basePipelineHandle = copy_src->basePipelineHandle;
    basePipelineIndex = copy_src->basePipelineIndex;
    pNext = SafePnextCopy(copy_src->pNext);
    if (stageCount && copy_src->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&copy_src->pStages[i]);
        }
    }
    if (copy_src->pLibraryInfo) pLibraryInfo = new safe_VkPipelineLibraryCreateInfoKHR(*copy_src->pLibraryInfo);
}

safe_VkDeviceOrHostAddressConstAMDX::safe_VkDeviceOrHostAddressConstAMDX(const VkDeviceOrHostAddressConstAMDX* in_struct,
                                                                         PNextCopyState*) {
    initialize(in_struct);
}

safe_VkDeviceOrHostAddressConstAMDX::safe_VkDeviceOrHostAddressConstAMDX() : hostAddress(nullptr) {}

safe_VkDeviceOrHostAddressConstAMDX::safe_VkDeviceOrHostAddressConstAMDX(const safe_VkDeviceOrHostAddressConstAMDX& copy_src) {
    deviceAddress = copy_src.deviceAddress;
    hostAddress = copy_src.hostAddress;
}

safe_VkDeviceOrHostAddressConstAMDX& safe_VkDeviceOrHostAddressConstAMDX::operator=(
    const safe_VkDeviceOrHostAddressConstAMDX& copy_src) {
    if (&copy_src == this) return *this;

    deviceAddress = copy_src.deviceAddress;
    hostAddress = copy_src.hostAddress;

    return *this;
}

safe_VkDeviceOrHostAddressConstAMDX::~safe_VkDeviceOrHostAddressConstAMDX() {}

void safe_VkDeviceOrHostAddressConstAMDX::initialize(const VkDeviceOrHostAddressConstAMDX* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    deviceAddress = in_struct->deviceAddress;
    hostAddress = in_struct->hostAddress;
}

void safe_VkDeviceOrHostAddressConstAMDX::initialize(const safe_VkDeviceOrHostAddressConstAMDX* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    deviceAddress = copy_src->deviceAddress;
    hostAddress = copy_src->hostAddress;
}

safe_VkPipelineShaderStageNodeCreateInfoAMDX::safe_VkPipelineShaderStageNodeCreateInfoAMDX(
    const VkPipelineShaderStageNodeCreateInfoAMDX* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), index(in_struct->index) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    pName = SafeStringCopy(in_struct->pName);
}

safe_VkPipelineShaderStageNodeCreateInfoAMDX::safe_VkPipelineShaderStageNodeCreateInfoAMDX()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_NODE_CREATE_INFO_AMDX), pNext(nullptr), pName(nullptr), index() {}

safe_VkPipelineShaderStageNodeCreateInfoAMDX::safe_VkPipelineShaderStageNodeCreateInfoAMDX(
    const safe_VkPipelineShaderStageNodeCreateInfoAMDX& copy_src) {
    sType = copy_src.sType;
    index = copy_src.index;
    pNext = SafePnextCopy(copy_src.pNext);
    pName = SafeStringCopy(copy_src.pName);
}

safe_VkPipelineShaderStageNodeCreateInfoAMDX& safe_VkPipelineShaderStageNodeCreateInfoAMDX::operator=(
    const safe_VkPipelineShaderStageNodeCreateInfoAMDX& copy_src) {
    if (&copy_src == this) return *this;

    if (pName) delete[] pName;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    index = copy_src.index;
    pNext = SafePnextCopy(copy_src.pNext);
    pName = SafeStringCopy(copy_src.pName);

    return *this;
}

safe_VkPipelineShaderStageNodeCreateInfoAMDX::~safe_VkPipelineShaderStageNodeCreateInfoAMDX() {
    if (pName) delete[] pName;
    FreePnextChain(pNext);
}

void safe_VkPipelineShaderStageNodeCreateInfoAMDX::initialize(const VkPipelineShaderStageNodeCreateInfoAMDX* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    if (pName) delete[] pName;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    index = in_struct->index;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    pName = SafeStringCopy(in_struct->pName);
}

void safe_VkPipelineShaderStageNodeCreateInfoAMDX::initialize(const safe_VkPipelineShaderStageNodeCreateInfoAMDX* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    index = copy_src->index;
    pNext = SafePnextCopy(copy_src->pNext);
    pName = SafeStringCopy(copy_src->pName);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS

safe_VkAttachmentSampleCountInfoAMD::safe_VkAttachmentSampleCountInfoAMD(const VkAttachmentSampleCountInfoAMD* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType),
      colorAttachmentCount(in_struct->colorAttachmentCount),
      pColorAttachmentSamples(nullptr),
      depthStencilAttachmentSamples(in_struct->depthStencilAttachmentSamples) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pColorAttachmentSamples) {
        pColorAttachmentSamples = new VkSampleCountFlagBits[in_struct->colorAttachmentCount];
        memcpy((void*)pColorAttachmentSamples, (void*)in_struct->pColorAttachmentSamples,
               sizeof(VkSampleCountFlagBits) * in_struct->colorAttachmentCount);
    }
}

safe_VkAttachmentSampleCountInfoAMD::safe_VkAttachmentSampleCountInfoAMD()
    : sType(VK_STRUCTURE_TYPE_ATTACHMENT_SAMPLE_COUNT_INFO_AMD),
      pNext(nullptr),
      colorAttachmentCount(),
      pColorAttachmentSamples(nullptr),
      depthStencilAttachmentSamples() {}

safe_VkAttachmentSampleCountInfoAMD::safe_VkAttachmentSampleCountInfoAMD(const safe_VkAttachmentSampleCountInfoAMD& copy_src) {
    sType = copy_src.sType;
    colorAttachmentCount = copy_src.colorAttachmentCount;
    pColorAttachmentSamples = nullptr;
    depthStencilAttachmentSamples = copy_src.depthStencilAttachmentSamples;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pColorAttachmentSamples) {
        pColorAttachmentSamples = new VkSampleCountFlagBits[copy_src.colorAttachmentCount];
        memcpy((void*)pColorAttachmentSamples, (void*)copy_src.pColorAttachmentSamples,
               sizeof(VkSampleCountFlagBits) * copy_src.colorAttachmentCount);
    }
}

safe_VkAttachmentSampleCountInfoAMD& safe_VkAttachmentSampleCountInfoAMD::operator=(
    const safe_VkAttachmentSampleCountInfoAMD& copy_src) {
    if (&copy_src == this) return *this;

    if (pColorAttachmentSamples) delete[] pColorAttachmentSamples;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    colorAttachmentCount = copy_src.colorAttachmentCount;
    pColorAttachmentSamples = nullptr;
    depthStencilAttachmentSamples = copy_src.depthStencilAttachmentSamples;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pColorAttachmentSamples) {
        pColorAttachmentSamples = new VkSampleCountFlagBits[copy_src.colorAttachmentCount];
        memcpy((void*)pColorAttachmentSamples, (void*)copy_src.pColorAttachmentSamples,
               sizeof(VkSampleCountFlagBits) * copy_src.colorAttachmentCount);
    }

    return *this;
}

safe_VkAttachmentSampleCountInfoAMD::~safe_VkAttachmentSampleCountInfoAMD() {
    if (pColorAttachmentSamples) delete[] pColorAttachmentSamples;
    FreePnextChain(pNext);
}

void safe_VkAttachmentSampleCountInfoAMD::initialize(const VkAttachmentSampleCountInfoAMD* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    if (pColorAttachmentSamples) delete[] pColorAttachmentSamples;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    colorAttachmentCount = in_struct->colorAttachmentCount;
    pColorAttachmentSamples = nullptr;
    depthStencilAttachmentSamples = in_struct->depthStencilAttachmentSamples;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pColorAttachmentSamples) {
        pColorAttachmentSamples = new VkSampleCountFlagBits[in_struct->colorAttachmentCount];
        memcpy((void*)pColorAttachmentSamples, (void*)in_struct->pColorAttachmentSamples,
               sizeof(VkSampleCountFlagBits) * in_struct->colorAttachmentCount);
    }
}

void safe_VkAttachmentSampleCountInfoAMD::initialize(const safe_VkAttachmentSampleCountInfoAMD* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    colorAttachmentCount = copy_src->colorAttachmentCount;
    pColorAttachmentSamples = nullptr;
    depthStencilAttachmentSamples = copy_src->depthStencilAttachmentSamples;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pColorAttachmentSamples) {
        pColorAttachmentSamples = new VkSampleCountFlagBits[copy_src->colorAttachmentCount];
        memcpy((void*)pColorAttachmentSamples, (void*)copy_src->pColorAttachmentSamples,
               sizeof(VkSampleCountFlagBits) * copy_src->colorAttachmentCount);
    }
}

safe_VkPipelineCoverageToColorStateCreateInfoNV::safe_VkPipelineCoverageToColorStateCreateInfoNV(
    const VkPipelineCoverageToColorStateCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      coverageToColorEnable(in_struct->coverageToColorEnable),
      coverageToColorLocation(in_struct->coverageToColorLocation) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineCoverageToColorStateCreateInfoNV::safe_VkPipelineCoverageToColorStateCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_TO_COLOR_STATE_CREATE_INFO_NV),
      pNext(nullptr),
      flags(),
      coverageToColorEnable(),
      coverageToColorLocation() {}

safe_VkPipelineCoverageToColorStateCreateInfoNV::safe_VkPipelineCoverageToColorStateCreateInfoNV(
    const safe_VkPipelineCoverageToColorStateCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    coverageToColorEnable = copy_src.coverageToColorEnable;
    coverageToColorLocation = copy_src.coverageToColorLocation;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineCoverageToColorStateCreateInfoNV& safe_VkPipelineCoverageToColorStateCreateInfoNV::operator=(
    const safe_VkPipelineCoverageToColorStateCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    coverageToColorEnable = copy_src.coverageToColorEnable;
    coverageToColorLocation = copy_src.coverageToColorLocation;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineCoverageToColorStateCreateInfoNV::~safe_VkPipelineCoverageToColorStateCreateInfoNV() { FreePnextChain(pNext); }

void safe_VkPipelineCoverageToColorStateCreateInfoNV::initialize(const VkPipelineCoverageToColorStateCreateInfoNV* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    coverageToColorEnable = in_struct->coverageToColorEnable;
    coverageToColorLocation = in_struct->coverageToColorLocation;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineCoverageToColorStateCreateInfoNV::initialize(const safe_VkPipelineCoverageToColorStateCreateInfoNV* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    coverageToColorEnable = copy_src->coverageToColorEnable;
    coverageToColorLocation = copy_src->coverageToColorLocation;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineCoverageModulationStateCreateInfoNV::safe_VkPipelineCoverageModulationStateCreateInfoNV(
    const VkPipelineCoverageModulationStateCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      coverageModulationMode(in_struct->coverageModulationMode),
      coverageModulationTableEnable(in_struct->coverageModulationTableEnable),
      coverageModulationTableCount(in_struct->coverageModulationTableCount),
      pCoverageModulationTable(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pCoverageModulationTable) {
        pCoverageModulationTable = new float[in_struct->coverageModulationTableCount];
        memcpy((void*)pCoverageModulationTable, (void*)in_struct->pCoverageModulationTable,
               sizeof(float) * in_struct->coverageModulationTableCount);
    }
}

safe_VkPipelineCoverageModulationStateCreateInfoNV::safe_VkPipelineCoverageModulationStateCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_MODULATION_STATE_CREATE_INFO_NV),
      pNext(nullptr),
      flags(),
      coverageModulationMode(),
      coverageModulationTableEnable(),
      coverageModulationTableCount(),
      pCoverageModulationTable(nullptr) {}

safe_VkPipelineCoverageModulationStateCreateInfoNV::safe_VkPipelineCoverageModulationStateCreateInfoNV(
    const safe_VkPipelineCoverageModulationStateCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    coverageModulationMode = copy_src.coverageModulationMode;
    coverageModulationTableEnable = copy_src.coverageModulationTableEnable;
    coverageModulationTableCount = copy_src.coverageModulationTableCount;
    pCoverageModulationTable = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pCoverageModulationTable) {
        pCoverageModulationTable = new float[copy_src.coverageModulationTableCount];
        memcpy((void*)pCoverageModulationTable, (void*)copy_src.pCoverageModulationTable,
               sizeof(float) * copy_src.coverageModulationTableCount);
    }
}

safe_VkPipelineCoverageModulationStateCreateInfoNV& safe_VkPipelineCoverageModulationStateCreateInfoNV::operator=(
    const safe_VkPipelineCoverageModulationStateCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pCoverageModulationTable) delete[] pCoverageModulationTable;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    coverageModulationMode = copy_src.coverageModulationMode;
    coverageModulationTableEnable = copy_src.coverageModulationTableEnable;
    coverageModulationTableCount = copy_src.coverageModulationTableCount;
    pCoverageModulationTable = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pCoverageModulationTable) {
        pCoverageModulationTable = new float[copy_src.coverageModulationTableCount];
        memcpy((void*)pCoverageModulationTable, (void*)copy_src.pCoverageModulationTable,
               sizeof(float) * copy_src.coverageModulationTableCount);
    }

    return *this;
}

safe_VkPipelineCoverageModulationStateCreateInfoNV::~safe_VkPipelineCoverageModulationStateCreateInfoNV() {
    if (pCoverageModulationTable) delete[] pCoverageModulationTable;
    FreePnextChain(pNext);
}

void safe_VkPipelineCoverageModulationStateCreateInfoNV::initialize(const VkPipelineCoverageModulationStateCreateInfoNV* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    if (pCoverageModulationTable) delete[] pCoverageModulationTable;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    coverageModulationMode = in_struct->coverageModulationMode;
    coverageModulationTableEnable = in_struct->coverageModulationTableEnable;
    coverageModulationTableCount = in_struct->coverageModulationTableCount;
    pCoverageModulationTable = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pCoverageModulationTable) {
        pCoverageModulationTable = new float[in_struct->coverageModulationTableCount];
        memcpy((void*)pCoverageModulationTable, (void*)in_struct->pCoverageModulationTable,
               sizeof(float) * in_struct->coverageModulationTableCount);
    }
}

void safe_VkPipelineCoverageModulationStateCreateInfoNV::initialize(
    const safe_VkPipelineCoverageModulationStateCreateInfoNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    coverageModulationMode = copy_src->coverageModulationMode;
    coverageModulationTableEnable = copy_src->coverageModulationTableEnable;
    coverageModulationTableCount = copy_src->coverageModulationTableCount;
    pCoverageModulationTable = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pCoverageModulationTable) {
        pCoverageModulationTable = new float[copy_src->coverageModulationTableCount];
        memcpy((void*)pCoverageModulationTable, (void*)copy_src->pCoverageModulationTable,
               sizeof(float) * copy_src->coverageModulationTableCount);
    }
}

safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV::safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV(
    const VkPhysicalDeviceShaderSMBuiltinsPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), shaderSMCount(in_struct->shaderSMCount), shaderWarpsPerSM(in_struct->shaderWarpsPerSM) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV::safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_PROPERTIES_NV),
      pNext(nullptr),
      shaderSMCount(),
      shaderWarpsPerSM() {}

safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV::safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV(
    const safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV& copy_src) {
    sType = copy_src.sType;
    shaderSMCount = copy_src.shaderSMCount;
    shaderWarpsPerSM = copy_src.shaderWarpsPerSM;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV& safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV::operator=(
    const safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderSMCount = copy_src.shaderSMCount;
    shaderWarpsPerSM = copy_src.shaderWarpsPerSM;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV::~safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV::initialize(const VkPhysicalDeviceShaderSMBuiltinsPropertiesNV* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderSMCount = in_struct->shaderSMCount;
    shaderWarpsPerSM = in_struct->shaderWarpsPerSM;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV::initialize(
    const safe_VkPhysicalDeviceShaderSMBuiltinsPropertiesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderSMCount = copy_src->shaderSMCount;
    shaderWarpsPerSM = copy_src->shaderWarpsPerSM;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV(
    const VkPhysicalDeviceShaderSMBuiltinsFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), shaderSMBuiltins(in_struct->shaderSMBuiltins) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SM_BUILTINS_FEATURES_NV), pNext(nullptr), shaderSMBuiltins() {}

safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV(
    const safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV& copy_src) {
    sType = copy_src.sType;
    shaderSMBuiltins = copy_src.shaderSMBuiltins;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV& safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::operator=(
    const safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderSMBuiltins = copy_src.shaderSMBuiltins;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::~safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::initialize(const VkPhysicalDeviceShaderSMBuiltinsFeaturesNV* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderSMBuiltins = in_struct->shaderSMBuiltins;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV::initialize(const safe_VkPhysicalDeviceShaderSMBuiltinsFeaturesNV* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderSMBuiltins = copy_src->shaderSMBuiltins;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkShadingRatePaletteNV::safe_VkShadingRatePaletteNV(const VkShadingRatePaletteNV* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state)
    : shadingRatePaletteEntryCount(in_struct->shadingRatePaletteEntryCount), pShadingRatePaletteEntries(nullptr) {
    if (in_struct->pShadingRatePaletteEntries) {
        pShadingRatePaletteEntries = new VkShadingRatePaletteEntryNV[in_struct->shadingRatePaletteEntryCount];
        memcpy((void*)pShadingRatePaletteEntries, (void*)in_struct->pShadingRatePaletteEntries,
               sizeof(VkShadingRatePaletteEntryNV) * in_struct->shadingRatePaletteEntryCount);
    }
}

safe_VkShadingRatePaletteNV::safe_VkShadingRatePaletteNV() : shadingRatePaletteEntryCount(), pShadingRatePaletteEntries(nullptr) {}

safe_VkShadingRatePaletteNV::safe_VkShadingRatePaletteNV(const safe_VkShadingRatePaletteNV& copy_src) {
    shadingRatePaletteEntryCount = copy_src.shadingRatePaletteEntryCount;
    pShadingRatePaletteEntries = nullptr;

    if (copy_src.pShadingRatePaletteEntries) {
        pShadingRatePaletteEntries = new VkShadingRatePaletteEntryNV[copy_src.shadingRatePaletteEntryCount];
        memcpy((void*)pShadingRatePaletteEntries, (void*)copy_src.pShadingRatePaletteEntries,
               sizeof(VkShadingRatePaletteEntryNV) * copy_src.shadingRatePaletteEntryCount);
    }
}

safe_VkShadingRatePaletteNV& safe_VkShadingRatePaletteNV::operator=(const safe_VkShadingRatePaletteNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pShadingRatePaletteEntries) delete[] pShadingRatePaletteEntries;

    shadingRatePaletteEntryCount = copy_src.shadingRatePaletteEntryCount;
    pShadingRatePaletteEntries = nullptr;

    if (copy_src.pShadingRatePaletteEntries) {
        pShadingRatePaletteEntries = new VkShadingRatePaletteEntryNV[copy_src.shadingRatePaletteEntryCount];
        memcpy((void*)pShadingRatePaletteEntries, (void*)copy_src.pShadingRatePaletteEntries,
               sizeof(VkShadingRatePaletteEntryNV) * copy_src.shadingRatePaletteEntryCount);
    }

    return *this;
}

safe_VkShadingRatePaletteNV::~safe_VkShadingRatePaletteNV() {
    if (pShadingRatePaletteEntries) delete[] pShadingRatePaletteEntries;
}

void safe_VkShadingRatePaletteNV::initialize(const VkShadingRatePaletteNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pShadingRatePaletteEntries) delete[] pShadingRatePaletteEntries;
    shadingRatePaletteEntryCount = in_struct->shadingRatePaletteEntryCount;
    pShadingRatePaletteEntries = nullptr;

    if (in_struct->pShadingRatePaletteEntries) {
        pShadingRatePaletteEntries = new VkShadingRatePaletteEntryNV[in_struct->shadingRatePaletteEntryCount];
        memcpy((void*)pShadingRatePaletteEntries, (void*)in_struct->pShadingRatePaletteEntries,
               sizeof(VkShadingRatePaletteEntryNV) * in_struct->shadingRatePaletteEntryCount);
    }
}

void safe_VkShadingRatePaletteNV::initialize(const safe_VkShadingRatePaletteNV* copy_src,
                                             [[maybe_unused]] PNextCopyState* copy_state) {
    shadingRatePaletteEntryCount = copy_src->shadingRatePaletteEntryCount;
    pShadingRatePaletteEntries = nullptr;

    if (copy_src->pShadingRatePaletteEntries) {
        pShadingRatePaletteEntries = new VkShadingRatePaletteEntryNV[copy_src->shadingRatePaletteEntryCount];
        memcpy((void*)pShadingRatePaletteEntries, (void*)copy_src->pShadingRatePaletteEntries,
               sizeof(VkShadingRatePaletteEntryNV) * copy_src->shadingRatePaletteEntryCount);
    }
}

safe_VkPipelineViewportShadingRateImageStateCreateInfoNV::safe_VkPipelineViewportShadingRateImageStateCreateInfoNV(
    const VkPipelineViewportShadingRateImageStateCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      shadingRateImageEnable(in_struct->shadingRateImageEnable),
      viewportCount(in_struct->viewportCount),
      pShadingRatePalettes(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (viewportCount && in_struct->pShadingRatePalettes) {
        pShadingRatePalettes = new safe_VkShadingRatePaletteNV[viewportCount];
        for (uint32_t i = 0; i < viewportCount; ++i) {
            pShadingRatePalettes[i].initialize(&in_struct->pShadingRatePalettes[i]);
        }
    }
}

safe_VkPipelineViewportShadingRateImageStateCreateInfoNV::safe_VkPipelineViewportShadingRateImageStateCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_SHADING_RATE_IMAGE_STATE_CREATE_INFO_NV),
      pNext(nullptr),
      shadingRateImageEnable(),
      viewportCount(),
      pShadingRatePalettes(nullptr) {}

safe_VkPipelineViewportShadingRateImageStateCreateInfoNV::safe_VkPipelineViewportShadingRateImageStateCreateInfoNV(
    const safe_VkPipelineViewportShadingRateImageStateCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    shadingRateImageEnable = copy_src.shadingRateImageEnable;
    viewportCount = copy_src.viewportCount;
    pShadingRatePalettes = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (viewportCount && copy_src.pShadingRatePalettes) {
        pShadingRatePalettes = new safe_VkShadingRatePaletteNV[viewportCount];
        for (uint32_t i = 0; i < viewportCount; ++i) {
            pShadingRatePalettes[i].initialize(&copy_src.pShadingRatePalettes[i]);
        }
    }
}

safe_VkPipelineViewportShadingRateImageStateCreateInfoNV& safe_VkPipelineViewportShadingRateImageStateCreateInfoNV::operator=(
    const safe_VkPipelineViewportShadingRateImageStateCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pShadingRatePalettes) delete[] pShadingRatePalettes;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    shadingRateImageEnable = copy_src.shadingRateImageEnable;
    viewportCount = copy_src.viewportCount;
    pShadingRatePalettes = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (viewportCount && copy_src.pShadingRatePalettes) {
        pShadingRatePalettes = new safe_VkShadingRatePaletteNV[viewportCount];
        for (uint32_t i = 0; i < viewportCount; ++i) {
            pShadingRatePalettes[i].initialize(&copy_src.pShadingRatePalettes[i]);
        }
    }

    return *this;
}

safe_VkPipelineViewportShadingRateImageStateCreateInfoNV::~safe_VkPipelineViewportShadingRateImageStateCreateInfoNV() {
    if (pShadingRatePalettes) delete[] pShadingRatePalettes;
    FreePnextChain(pNext);
}

void safe_VkPipelineViewportShadingRateImageStateCreateInfoNV::initialize(
    const VkPipelineViewportShadingRateImageStateCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pShadingRatePalettes) delete[] pShadingRatePalettes;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shadingRateImageEnable = in_struct->shadingRateImageEnable;
    viewportCount = in_struct->viewportCount;
    pShadingRatePalettes = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (viewportCount && in_struct->pShadingRatePalettes) {
        pShadingRatePalettes = new safe_VkShadingRatePaletteNV[viewportCount];
        for (uint32_t i = 0; i < viewportCount; ++i) {
            pShadingRatePalettes[i].initialize(&in_struct->pShadingRatePalettes[i]);
        }
    }
}

void safe_VkPipelineViewportShadingRateImageStateCreateInfoNV::initialize(
    const safe_VkPipelineViewportShadingRateImageStateCreateInfoNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shadingRateImageEnable = copy_src->shadingRateImageEnable;
    viewportCount = copy_src->viewportCount;
    pShadingRatePalettes = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (viewportCount && copy_src->pShadingRatePalettes) {
        pShadingRatePalettes = new safe_VkShadingRatePaletteNV[viewportCount];
        for (uint32_t i = 0; i < viewportCount; ++i) {
            pShadingRatePalettes[i].initialize(&copy_src->pShadingRatePalettes[i]);
        }
    }
}

safe_VkPhysicalDeviceShadingRateImageFeaturesNV::safe_VkPhysicalDeviceShadingRateImageFeaturesNV(
    const VkPhysicalDeviceShadingRateImageFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      shadingRateImage(in_struct->shadingRateImage),
      shadingRateCoarseSampleOrder(in_struct->shadingRateCoarseSampleOrder) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShadingRateImageFeaturesNV::safe_VkPhysicalDeviceShadingRateImageFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_FEATURES_NV),
      pNext(nullptr),
      shadingRateImage(),
      shadingRateCoarseSampleOrder() {}

safe_VkPhysicalDeviceShadingRateImageFeaturesNV::safe_VkPhysicalDeviceShadingRateImageFeaturesNV(
    const safe_VkPhysicalDeviceShadingRateImageFeaturesNV& copy_src) {
    sType = copy_src.sType;
    shadingRateImage = copy_src.shadingRateImage;
    shadingRateCoarseSampleOrder = copy_src.shadingRateCoarseSampleOrder;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShadingRateImageFeaturesNV& safe_VkPhysicalDeviceShadingRateImageFeaturesNV::operator=(
    const safe_VkPhysicalDeviceShadingRateImageFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shadingRateImage = copy_src.shadingRateImage;
    shadingRateCoarseSampleOrder = copy_src.shadingRateCoarseSampleOrder;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShadingRateImageFeaturesNV::~safe_VkPhysicalDeviceShadingRateImageFeaturesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShadingRateImageFeaturesNV::initialize(const VkPhysicalDeviceShadingRateImageFeaturesNV* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shadingRateImage = in_struct->shadingRateImage;
    shadingRateCoarseSampleOrder = in_struct->shadingRateCoarseSampleOrder;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShadingRateImageFeaturesNV::initialize(const safe_VkPhysicalDeviceShadingRateImageFeaturesNV* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shadingRateImage = copy_src->shadingRateImage;
    shadingRateCoarseSampleOrder = copy_src->shadingRateCoarseSampleOrder;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShadingRateImagePropertiesNV::safe_VkPhysicalDeviceShadingRateImagePropertiesNV(
    const VkPhysicalDeviceShadingRateImagePropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      shadingRateTexelSize(in_struct->shadingRateTexelSize),
      shadingRatePaletteSize(in_struct->shadingRatePaletteSize),
      shadingRateMaxCoarseSamples(in_struct->shadingRateMaxCoarseSamples) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShadingRateImagePropertiesNV::safe_VkPhysicalDeviceShadingRateImagePropertiesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADING_RATE_IMAGE_PROPERTIES_NV),
      pNext(nullptr),
      shadingRateTexelSize(),
      shadingRatePaletteSize(),
      shadingRateMaxCoarseSamples() {}

safe_VkPhysicalDeviceShadingRateImagePropertiesNV::safe_VkPhysicalDeviceShadingRateImagePropertiesNV(
    const safe_VkPhysicalDeviceShadingRateImagePropertiesNV& copy_src) {
    sType = copy_src.sType;
    shadingRateTexelSize = copy_src.shadingRateTexelSize;
    shadingRatePaletteSize = copy_src.shadingRatePaletteSize;
    shadingRateMaxCoarseSamples = copy_src.shadingRateMaxCoarseSamples;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShadingRateImagePropertiesNV& safe_VkPhysicalDeviceShadingRateImagePropertiesNV::operator=(
    const safe_VkPhysicalDeviceShadingRateImagePropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shadingRateTexelSize = copy_src.shadingRateTexelSize;
    shadingRatePaletteSize = copy_src.shadingRatePaletteSize;
    shadingRateMaxCoarseSamples = copy_src.shadingRateMaxCoarseSamples;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShadingRateImagePropertiesNV::~safe_VkPhysicalDeviceShadingRateImagePropertiesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShadingRateImagePropertiesNV::initialize(const VkPhysicalDeviceShadingRateImagePropertiesNV* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shadingRateTexelSize = in_struct->shadingRateTexelSize;
    shadingRatePaletteSize = in_struct->shadingRatePaletteSize;
    shadingRateMaxCoarseSamples = in_struct->shadingRateMaxCoarseSamples;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShadingRateImagePropertiesNV::initialize(
    const safe_VkPhysicalDeviceShadingRateImagePropertiesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shadingRateTexelSize = copy_src->shadingRateTexelSize;
    shadingRatePaletteSize = copy_src->shadingRatePaletteSize;
    shadingRateMaxCoarseSamples = copy_src->shadingRateMaxCoarseSamples;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCoarseSampleOrderCustomNV::safe_VkCoarseSampleOrderCustomNV(const VkCoarseSampleOrderCustomNV* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state)
    : shadingRate(in_struct->shadingRate),
      sampleCount(in_struct->sampleCount),
      sampleLocationCount(in_struct->sampleLocationCount),
      pSampleLocations(nullptr) {
    if (in_struct->pSampleLocations) {
        pSampleLocations = new VkCoarseSampleLocationNV[in_struct->sampleLocationCount];
        memcpy((void*)pSampleLocations, (void*)in_struct->pSampleLocations,
               sizeof(VkCoarseSampleLocationNV) * in_struct->sampleLocationCount);
    }
}

safe_VkCoarseSampleOrderCustomNV::safe_VkCoarseSampleOrderCustomNV()
    : shadingRate(), sampleCount(), sampleLocationCount(), pSampleLocations(nullptr) {}

safe_VkCoarseSampleOrderCustomNV::safe_VkCoarseSampleOrderCustomNV(const safe_VkCoarseSampleOrderCustomNV& copy_src) {
    shadingRate = copy_src.shadingRate;
    sampleCount = copy_src.sampleCount;
    sampleLocationCount = copy_src.sampleLocationCount;
    pSampleLocations = nullptr;

    if (copy_src.pSampleLocations) {
        pSampleLocations = new VkCoarseSampleLocationNV[copy_src.sampleLocationCount];
        memcpy((void*)pSampleLocations, (void*)copy_src.pSampleLocations,
               sizeof(VkCoarseSampleLocationNV) * copy_src.sampleLocationCount);
    }
}

safe_VkCoarseSampleOrderCustomNV& safe_VkCoarseSampleOrderCustomNV::operator=(const safe_VkCoarseSampleOrderCustomNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pSampleLocations) delete[] pSampleLocations;

    shadingRate = copy_src.shadingRate;
    sampleCount = copy_src.sampleCount;
    sampleLocationCount = copy_src.sampleLocationCount;
    pSampleLocations = nullptr;

    if (copy_src.pSampleLocations) {
        pSampleLocations = new VkCoarseSampleLocationNV[copy_src.sampleLocationCount];
        memcpy((void*)pSampleLocations, (void*)copy_src.pSampleLocations,
               sizeof(VkCoarseSampleLocationNV) * copy_src.sampleLocationCount);
    }

    return *this;
}

safe_VkCoarseSampleOrderCustomNV::~safe_VkCoarseSampleOrderCustomNV() {
    if (pSampleLocations) delete[] pSampleLocations;
}

void safe_VkCoarseSampleOrderCustomNV::initialize(const VkCoarseSampleOrderCustomNV* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    if (pSampleLocations) delete[] pSampleLocations;
    shadingRate = in_struct->shadingRate;
    sampleCount = in_struct->sampleCount;
    sampleLocationCount = in_struct->sampleLocationCount;
    pSampleLocations = nullptr;

    if (in_struct->pSampleLocations) {
        pSampleLocations = new VkCoarseSampleLocationNV[in_struct->sampleLocationCount];
        memcpy((void*)pSampleLocations, (void*)in_struct->pSampleLocations,
               sizeof(VkCoarseSampleLocationNV) * in_struct->sampleLocationCount);
    }
}

void safe_VkCoarseSampleOrderCustomNV::initialize(const safe_VkCoarseSampleOrderCustomNV* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    shadingRate = copy_src->shadingRate;
    sampleCount = copy_src->sampleCount;
    sampleLocationCount = copy_src->sampleLocationCount;
    pSampleLocations = nullptr;

    if (copy_src->pSampleLocations) {
        pSampleLocations = new VkCoarseSampleLocationNV[copy_src->sampleLocationCount];
        memcpy((void*)pSampleLocations, (void*)copy_src->pSampleLocations,
               sizeof(VkCoarseSampleLocationNV) * copy_src->sampleLocationCount);
    }
}

safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV::safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV(
    const VkPipelineViewportCoarseSampleOrderStateCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      sampleOrderType(in_struct->sampleOrderType),
      customSampleOrderCount(in_struct->customSampleOrderCount),
      pCustomSampleOrders(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (customSampleOrderCount && in_struct->pCustomSampleOrders) {
        pCustomSampleOrders = new safe_VkCoarseSampleOrderCustomNV[customSampleOrderCount];
        for (uint32_t i = 0; i < customSampleOrderCount; ++i) {
            pCustomSampleOrders[i].initialize(&in_struct->pCustomSampleOrders[i]);
        }
    }
}

safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV::safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_COARSE_SAMPLE_ORDER_STATE_CREATE_INFO_NV),
      pNext(nullptr),
      sampleOrderType(),
      customSampleOrderCount(),
      pCustomSampleOrders(nullptr) {}

safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV::safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV(
    const safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    sampleOrderType = copy_src.sampleOrderType;
    customSampleOrderCount = copy_src.customSampleOrderCount;
    pCustomSampleOrders = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (customSampleOrderCount && copy_src.pCustomSampleOrders) {
        pCustomSampleOrders = new safe_VkCoarseSampleOrderCustomNV[customSampleOrderCount];
        for (uint32_t i = 0; i < customSampleOrderCount; ++i) {
            pCustomSampleOrders[i].initialize(&copy_src.pCustomSampleOrders[i]);
        }
    }
}

safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV& safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV::operator=(
    const safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pCustomSampleOrders) delete[] pCustomSampleOrders;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    sampleOrderType = copy_src.sampleOrderType;
    customSampleOrderCount = copy_src.customSampleOrderCount;
    pCustomSampleOrders = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (customSampleOrderCount && copy_src.pCustomSampleOrders) {
        pCustomSampleOrders = new safe_VkCoarseSampleOrderCustomNV[customSampleOrderCount];
        for (uint32_t i = 0; i < customSampleOrderCount; ++i) {
            pCustomSampleOrders[i].initialize(&copy_src.pCustomSampleOrders[i]);
        }
    }

    return *this;
}

safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV::~safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV() {
    if (pCustomSampleOrders) delete[] pCustomSampleOrders;
    FreePnextChain(pNext);
}

void safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV::initialize(
    const VkPipelineViewportCoarseSampleOrderStateCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pCustomSampleOrders) delete[] pCustomSampleOrders;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    sampleOrderType = in_struct->sampleOrderType;
    customSampleOrderCount = in_struct->customSampleOrderCount;
    pCustomSampleOrders = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (customSampleOrderCount && in_struct->pCustomSampleOrders) {
        pCustomSampleOrders = new safe_VkCoarseSampleOrderCustomNV[customSampleOrderCount];
        for (uint32_t i = 0; i < customSampleOrderCount; ++i) {
            pCustomSampleOrders[i].initialize(&in_struct->pCustomSampleOrders[i]);
        }
    }
}

void safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV::initialize(
    const safe_VkPipelineViewportCoarseSampleOrderStateCreateInfoNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    sampleOrderType = copy_src->sampleOrderType;
    customSampleOrderCount = copy_src->customSampleOrderCount;
    pCustomSampleOrders = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (customSampleOrderCount && copy_src->pCustomSampleOrders) {
        pCustomSampleOrders = new safe_VkCoarseSampleOrderCustomNV[customSampleOrderCount];
        for (uint32_t i = 0; i < customSampleOrderCount; ++i) {
            pCustomSampleOrders[i].initialize(&copy_src->pCustomSampleOrders[i]);
        }
    }
}

safe_VkRayTracingShaderGroupCreateInfoNV::safe_VkRayTracingShaderGroupCreateInfoNV(
    const VkRayTracingShaderGroupCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      type(in_struct->type),
      generalShader(in_struct->generalShader),
      closestHitShader(in_struct->closestHitShader),
      anyHitShader(in_struct->anyHitShader),
      intersectionShader(in_struct->intersectionShader) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkRayTracingShaderGroupCreateInfoNV::safe_VkRayTracingShaderGroupCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_NV),
      pNext(nullptr),
      type(),
      generalShader(),
      closestHitShader(),
      anyHitShader(),
      intersectionShader() {}

safe_VkRayTracingShaderGroupCreateInfoNV::safe_VkRayTracingShaderGroupCreateInfoNV(
    const safe_VkRayTracingShaderGroupCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    type = copy_src.type;
    generalShader = copy_src.generalShader;
    closestHitShader = copy_src.closestHitShader;
    anyHitShader = copy_src.anyHitShader;
    intersectionShader = copy_src.intersectionShader;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkRayTracingShaderGroupCreateInfoNV& safe_VkRayTracingShaderGroupCreateInfoNV::operator=(
    const safe_VkRayTracingShaderGroupCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    type = copy_src.type;
    generalShader = copy_src.generalShader;
    closestHitShader = copy_src.closestHitShader;
    anyHitShader = copy_src.anyHitShader;
    intersectionShader = copy_src.intersectionShader;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkRayTracingShaderGroupCreateInfoNV::~safe_VkRayTracingShaderGroupCreateInfoNV() { FreePnextChain(pNext); }

void safe_VkRayTracingShaderGroupCreateInfoNV::initialize(const VkRayTracingShaderGroupCreateInfoNV* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    type = in_struct->type;
    generalShader = in_struct->generalShader;
    closestHitShader = in_struct->closestHitShader;
    anyHitShader = in_struct->anyHitShader;
    intersectionShader = in_struct->intersectionShader;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkRayTracingShaderGroupCreateInfoNV::initialize(const safe_VkRayTracingShaderGroupCreateInfoNV* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    type = copy_src->type;
    generalShader = copy_src->generalShader;
    closestHitShader = copy_src->closestHitShader;
    anyHitShader = copy_src->anyHitShader;
    intersectionShader = copy_src->intersectionShader;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkRayTracingPipelineCreateInfoNV::safe_VkRayTracingPipelineCreateInfoNV(const VkRayTracingPipelineCreateInfoNV* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      stageCount(in_struct->stageCount),
      pStages(nullptr),
      groupCount(in_struct->groupCount),
      pGroups(nullptr),
      maxRecursionDepth(in_struct->maxRecursionDepth),
      layout(in_struct->layout),
      basePipelineHandle(in_struct->basePipelineHandle),
      basePipelineIndex(in_struct->basePipelineIndex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (stageCount && in_struct->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&in_struct->pStages[i]);
        }
    }
    if (groupCount && in_struct->pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoNV[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&in_struct->pGroups[i]);
        }
    }
}

safe_VkRayTracingPipelineCreateInfoNV::safe_VkRayTracingPipelineCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV),
      pNext(nullptr),
      flags(),
      stageCount(),
      pStages(nullptr),
      groupCount(),
      pGroups(nullptr),
      maxRecursionDepth(),
      layout(),
      basePipelineHandle(),
      basePipelineIndex() {}

safe_VkRayTracingPipelineCreateInfoNV::safe_VkRayTracingPipelineCreateInfoNV(
    const safe_VkRayTracingPipelineCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    stageCount = copy_src.stageCount;
    pStages = nullptr;
    groupCount = copy_src.groupCount;
    pGroups = nullptr;
    maxRecursionDepth = copy_src.maxRecursionDepth;
    layout = copy_src.layout;
    basePipelineHandle = copy_src.basePipelineHandle;
    basePipelineIndex = copy_src.basePipelineIndex;
    pNext = SafePnextCopy(copy_src.pNext);
    if (stageCount && copy_src.pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&copy_src.pStages[i]);
        }
    }
    if (groupCount && copy_src.pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoNV[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&copy_src.pGroups[i]);
        }
    }
}

safe_VkRayTracingPipelineCreateInfoNV& safe_VkRayTracingPipelineCreateInfoNV::operator=(
    const safe_VkRayTracingPipelineCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pStages) delete[] pStages;
    if (pGroups) delete[] pGroups;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    stageCount = copy_src.stageCount;
    pStages = nullptr;
    groupCount = copy_src.groupCount;
    pGroups = nullptr;
    maxRecursionDepth = copy_src.maxRecursionDepth;
    layout = copy_src.layout;
    basePipelineHandle = copy_src.basePipelineHandle;
    basePipelineIndex = copy_src.basePipelineIndex;
    pNext = SafePnextCopy(copy_src.pNext);
    if (stageCount && copy_src.pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&copy_src.pStages[i]);
        }
    }
    if (groupCount && copy_src.pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoNV[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&copy_src.pGroups[i]);
        }
    }

    return *this;
}

safe_VkRayTracingPipelineCreateInfoNV::~safe_VkRayTracingPipelineCreateInfoNV() {
    if (pStages) delete[] pStages;
    if (pGroups) delete[] pGroups;
    FreePnextChain(pNext);
}

void safe_VkRayTracingPipelineCreateInfoNV::initialize(const VkRayTracingPipelineCreateInfoNV* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStages) delete[] pStages;
    if (pGroups) delete[] pGroups;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    stageCount = in_struct->stageCount;
    pStages = nullptr;
    groupCount = in_struct->groupCount;
    pGroups = nullptr;
    maxRecursionDepth = in_struct->maxRecursionDepth;
    layout = in_struct->layout;
    basePipelineHandle = in_struct->basePipelineHandle;
    basePipelineIndex = in_struct->basePipelineIndex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (stageCount && in_struct->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&in_struct->pStages[i]);
        }
    }
    if (groupCount && in_struct->pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoNV[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&in_struct->pGroups[i]);
        }
    }
}

void safe_VkRayTracingPipelineCreateInfoNV::initialize(const safe_VkRayTracingPipelineCreateInfoNV* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    stageCount = copy_src->stageCount;
    pStages = nullptr;
    groupCount = copy_src->groupCount;
    pGroups = nullptr;
    maxRecursionDepth = copy_src->maxRecursionDepth;
    layout = copy_src->layout;
    basePipelineHandle = copy_src->basePipelineHandle;
    basePipelineIndex = copy_src->basePipelineIndex;
    pNext = SafePnextCopy(copy_src->pNext);
    if (stageCount && copy_src->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&copy_src->pStages[i]);
        }
    }
    if (groupCount && copy_src->pGroups) {
        pGroups = new safe_VkRayTracingShaderGroupCreateInfoNV[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&copy_src->pGroups[i]);
        }
    }
}

safe_VkGeometryTrianglesNV::safe_VkGeometryTrianglesNV(const VkGeometryTrianglesNV* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      vertexData(in_struct->vertexData),
      vertexOffset(in_struct->vertexOffset),
      vertexCount(in_struct->vertexCount),
      vertexStride(in_struct->vertexStride),
      vertexFormat(in_struct->vertexFormat),
      indexData(in_struct->indexData),
      indexOffset(in_struct->indexOffset),
      indexCount(in_struct->indexCount),
      indexType(in_struct->indexType),
      transformData(in_struct->transformData),
      transformOffset(in_struct->transformOffset) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkGeometryTrianglesNV::safe_VkGeometryTrianglesNV()
    : sType(VK_STRUCTURE_TYPE_GEOMETRY_TRIANGLES_NV),
      pNext(nullptr),
      vertexData(),
      vertexOffset(),
      vertexCount(),
      vertexStride(),
      vertexFormat(),
      indexData(),
      indexOffset(),
      indexCount(),
      indexType(),
      transformData(),
      transformOffset() {}

safe_VkGeometryTrianglesNV::safe_VkGeometryTrianglesNV(const safe_VkGeometryTrianglesNV& copy_src) {
    sType = copy_src.sType;
    vertexData = copy_src.vertexData;
    vertexOffset = copy_src.vertexOffset;
    vertexCount = copy_src.vertexCount;
    vertexStride = copy_src.vertexStride;
    vertexFormat = copy_src.vertexFormat;
    indexData = copy_src.indexData;
    indexOffset = copy_src.indexOffset;
    indexCount = copy_src.indexCount;
    indexType = copy_src.indexType;
    transformData = copy_src.transformData;
    transformOffset = copy_src.transformOffset;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkGeometryTrianglesNV& safe_VkGeometryTrianglesNV::operator=(const safe_VkGeometryTrianglesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    vertexData = copy_src.vertexData;
    vertexOffset = copy_src.vertexOffset;
    vertexCount = copy_src.vertexCount;
    vertexStride = copy_src.vertexStride;
    vertexFormat = copy_src.vertexFormat;
    indexData = copy_src.indexData;
    indexOffset = copy_src.indexOffset;
    indexCount = copy_src.indexCount;
    indexType = copy_src.indexType;
    transformData = copy_src.transformData;
    transformOffset = copy_src.transformOffset;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkGeometryTrianglesNV::~safe_VkGeometryTrianglesNV() { FreePnextChain(pNext); }

void safe_VkGeometryTrianglesNV::initialize(const VkGeometryTrianglesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    vertexData = in_struct->vertexData;
    vertexOffset = in_struct->vertexOffset;
    vertexCount = in_struct->vertexCount;
    vertexStride = in_struct->vertexStride;
    vertexFormat = in_struct->vertexFormat;
    indexData = in_struct->indexData;
    indexOffset = in_struct->indexOffset;
    indexCount = in_struct->indexCount;
    indexType = in_struct->indexType;
    transformData = in_struct->transformData;
    transformOffset = in_struct->transformOffset;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkGeometryTrianglesNV::initialize(const safe_VkGeometryTrianglesNV* copy_src,
                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    vertexData = copy_src->vertexData;
    vertexOffset = copy_src->vertexOffset;
    vertexCount = copy_src->vertexCount;
    vertexStride = copy_src->vertexStride;
    vertexFormat = copy_src->vertexFormat;
    indexData = copy_src->indexData;
    indexOffset = copy_src->indexOffset;
    indexCount = copy_src->indexCount;
    indexType = copy_src->indexType;
    transformData = copy_src->transformData;
    transformOffset = copy_src->transformOffset;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkGeometryAABBNV::safe_VkGeometryAABBNV(const VkGeometryAABBNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
                                             bool copy_pnext)
    : sType(in_struct->sType),
      aabbData(in_struct->aabbData),
      numAABBs(in_struct->numAABBs),
      stride(in_struct->stride),
      offset(in_struct->offset) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkGeometryAABBNV::safe_VkGeometryAABBNV()
    : sType(VK_STRUCTURE_TYPE_GEOMETRY_AABB_NV), pNext(nullptr), aabbData(), numAABBs(), stride(), offset() {}

safe_VkGeometryAABBNV::safe_VkGeometryAABBNV(const safe_VkGeometryAABBNV& copy_src) {
    sType = copy_src.sType;
    aabbData = copy_src.aabbData;
    numAABBs = copy_src.numAABBs;
    stride = copy_src.stride;
    offset = copy_src.offset;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkGeometryAABBNV& safe_VkGeometryAABBNV::operator=(const safe_VkGeometryAABBNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    aabbData = copy_src.aabbData;
    numAABBs = copy_src.numAABBs;
    stride = copy_src.stride;
    offset = copy_src.offset;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkGeometryAABBNV::~safe_VkGeometryAABBNV() { FreePnextChain(pNext); }

void safe_VkGeometryAABBNV::initialize(const VkGeometryAABBNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    aabbData = in_struct->aabbData;
    numAABBs = in_struct->numAABBs;
    stride = in_struct->stride;
    offset = in_struct->offset;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkGeometryAABBNV::initialize(const safe_VkGeometryAABBNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    aabbData = copy_src->aabbData;
    numAABBs = copy_src->numAABBs;
    stride = copy_src->stride;
    offset = copy_src->offset;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkGeometryNV::safe_VkGeometryNV(const VkGeometryNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), geometryType(in_struct->geometryType), geometry(in_struct->geometry), flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkGeometryNV::safe_VkGeometryNV()
    : sType(VK_STRUCTURE_TYPE_GEOMETRY_NV), pNext(nullptr), geometryType(), geometry(), flags() {}

safe_VkGeometryNV::safe_VkGeometryNV(const safe_VkGeometryNV& copy_src) {
    sType = copy_src.sType;
    geometryType = copy_src.geometryType;
    geometry = copy_src.geometry;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkGeometryNV& safe_VkGeometryNV::operator=(const safe_VkGeometryNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    geometryType = copy_src.geometryType;
    geometry = copy_src.geometry;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkGeometryNV::~safe_VkGeometryNV() { FreePnextChain(pNext); }

void safe_VkGeometryNV::initialize(const VkGeometryNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    geometryType = in_struct->geometryType;
    geometry = in_struct->geometry;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkGeometryNV::initialize(const safe_VkGeometryNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    geometryType = copy_src->geometryType;
    geometry = copy_src->geometry;
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAccelerationStructureInfoNV::safe_VkAccelerationStructureInfoNV(const VkAccelerationStructureInfoNV* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      type(in_struct->type),
      flags(in_struct->flags),
      instanceCount(in_struct->instanceCount),
      geometryCount(in_struct->geometryCount),
      pGeometries(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (geometryCount && in_struct->pGeometries) {
        pGeometries = new safe_VkGeometryNV[geometryCount];
        for (uint32_t i = 0; i < geometryCount; ++i) {
            pGeometries[i].initialize(&in_struct->pGeometries[i]);
        }
    }
}

safe_VkAccelerationStructureInfoNV::safe_VkAccelerationStructureInfoNV()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV),
      pNext(nullptr),
      type(),
      flags(),
      instanceCount(),
      geometryCount(),
      pGeometries(nullptr) {}

safe_VkAccelerationStructureInfoNV::safe_VkAccelerationStructureInfoNV(const safe_VkAccelerationStructureInfoNV& copy_src) {
    sType = copy_src.sType;
    type = copy_src.type;
    flags = copy_src.flags;
    instanceCount = copy_src.instanceCount;
    geometryCount = copy_src.geometryCount;
    pGeometries = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (geometryCount && copy_src.pGeometries) {
        pGeometries = new safe_VkGeometryNV[geometryCount];
        for (uint32_t i = 0; i < geometryCount; ++i) {
            pGeometries[i].initialize(&copy_src.pGeometries[i]);
        }
    }
}

safe_VkAccelerationStructureInfoNV& safe_VkAccelerationStructureInfoNV::operator=(
    const safe_VkAccelerationStructureInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pGeometries) delete[] pGeometries;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    type = copy_src.type;
    flags = copy_src.flags;
    instanceCount = copy_src.instanceCount;
    geometryCount = copy_src.geometryCount;
    pGeometries = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (geometryCount && copy_src.pGeometries) {
        pGeometries = new safe_VkGeometryNV[geometryCount];
        for (uint32_t i = 0; i < geometryCount; ++i) {
            pGeometries[i].initialize(&copy_src.pGeometries[i]);
        }
    }

    return *this;
}

safe_VkAccelerationStructureInfoNV::~safe_VkAccelerationStructureInfoNV() {
    if (pGeometries) delete[] pGeometries;
    FreePnextChain(pNext);
}

void safe_VkAccelerationStructureInfoNV::initialize(const VkAccelerationStructureInfoNV* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    if (pGeometries) delete[] pGeometries;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    type = in_struct->type;
    flags = in_struct->flags;
    instanceCount = in_struct->instanceCount;
    geometryCount = in_struct->geometryCount;
    pGeometries = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (geometryCount && in_struct->pGeometries) {
        pGeometries = new safe_VkGeometryNV[geometryCount];
        for (uint32_t i = 0; i < geometryCount; ++i) {
            pGeometries[i].initialize(&in_struct->pGeometries[i]);
        }
    }
}

void safe_VkAccelerationStructureInfoNV::initialize(const safe_VkAccelerationStructureInfoNV* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    type = copy_src->type;
    flags = copy_src->flags;
    instanceCount = copy_src->instanceCount;
    geometryCount = copy_src->geometryCount;
    pGeometries = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (geometryCount && copy_src->pGeometries) {
        pGeometries = new safe_VkGeometryNV[geometryCount];
        for (uint32_t i = 0; i < geometryCount; ++i) {
            pGeometries[i].initialize(&copy_src->pGeometries[i]);
        }
    }
}

safe_VkAccelerationStructureCreateInfoNV::safe_VkAccelerationStructureCreateInfoNV(
    const VkAccelerationStructureCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), compactedSize(in_struct->compactedSize), info(&in_struct->info) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAccelerationStructureCreateInfoNV::safe_VkAccelerationStructureCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV), pNext(nullptr), compactedSize() {}

safe_VkAccelerationStructureCreateInfoNV::safe_VkAccelerationStructureCreateInfoNV(
    const safe_VkAccelerationStructureCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    compactedSize = copy_src.compactedSize;
    info.initialize(&copy_src.info);
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureCreateInfoNV& safe_VkAccelerationStructureCreateInfoNV::operator=(
    const safe_VkAccelerationStructureCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    compactedSize = copy_src.compactedSize;
    info.initialize(&copy_src.info);
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAccelerationStructureCreateInfoNV::~safe_VkAccelerationStructureCreateInfoNV() { FreePnextChain(pNext); }

void safe_VkAccelerationStructureCreateInfoNV::initialize(const VkAccelerationStructureCreateInfoNV* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    compactedSize = in_struct->compactedSize;
    info.initialize(&in_struct->info);
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAccelerationStructureCreateInfoNV::initialize(const safe_VkAccelerationStructureCreateInfoNV* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    compactedSize = copy_src->compactedSize;
    info.initialize(&copy_src->info);
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkBindAccelerationStructureMemoryInfoNV::safe_VkBindAccelerationStructureMemoryInfoNV(
    const VkBindAccelerationStructureMemoryInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      accelerationStructure(in_struct->accelerationStructure),
      memory(in_struct->memory),
      memoryOffset(in_struct->memoryOffset),
      deviceIndexCount(in_struct->deviceIndexCount),
      pDeviceIndices(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pDeviceIndices) {
        pDeviceIndices = new uint32_t[in_struct->deviceIndexCount];
        memcpy((void*)pDeviceIndices, (void*)in_struct->pDeviceIndices, sizeof(uint32_t) * in_struct->deviceIndexCount);
    }
}

safe_VkBindAccelerationStructureMemoryInfoNV::safe_VkBindAccelerationStructureMemoryInfoNV()
    : sType(VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV),
      pNext(nullptr),
      accelerationStructure(),
      memory(),
      memoryOffset(),
      deviceIndexCount(),
      pDeviceIndices(nullptr) {}

safe_VkBindAccelerationStructureMemoryInfoNV::safe_VkBindAccelerationStructureMemoryInfoNV(
    const safe_VkBindAccelerationStructureMemoryInfoNV& copy_src) {
    sType = copy_src.sType;
    accelerationStructure = copy_src.accelerationStructure;
    memory = copy_src.memory;
    memoryOffset = copy_src.memoryOffset;
    deviceIndexCount = copy_src.deviceIndexCount;
    pDeviceIndices = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDeviceIndices) {
        pDeviceIndices = new uint32_t[copy_src.deviceIndexCount];
        memcpy((void*)pDeviceIndices, (void*)copy_src.pDeviceIndices, sizeof(uint32_t) * copy_src.deviceIndexCount);
    }
}

safe_VkBindAccelerationStructureMemoryInfoNV& safe_VkBindAccelerationStructureMemoryInfoNV::operator=(
    const safe_VkBindAccelerationStructureMemoryInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pDeviceIndices) delete[] pDeviceIndices;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    accelerationStructure = copy_src.accelerationStructure;
    memory = copy_src.memory;
    memoryOffset = copy_src.memoryOffset;
    deviceIndexCount = copy_src.deviceIndexCount;
    pDeviceIndices = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDeviceIndices) {
        pDeviceIndices = new uint32_t[copy_src.deviceIndexCount];
        memcpy((void*)pDeviceIndices, (void*)copy_src.pDeviceIndices, sizeof(uint32_t) * copy_src.deviceIndexCount);
    }

    return *this;
}

safe_VkBindAccelerationStructureMemoryInfoNV::~safe_VkBindAccelerationStructureMemoryInfoNV() {
    if (pDeviceIndices) delete[] pDeviceIndices;
    FreePnextChain(pNext);
}

void safe_VkBindAccelerationStructureMemoryInfoNV::initialize(const VkBindAccelerationStructureMemoryInfoNV* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    if (pDeviceIndices) delete[] pDeviceIndices;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    accelerationStructure = in_struct->accelerationStructure;
    memory = in_struct->memory;
    memoryOffset = in_struct->memoryOffset;
    deviceIndexCount = in_struct->deviceIndexCount;
    pDeviceIndices = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pDeviceIndices) {
        pDeviceIndices = new uint32_t[in_struct->deviceIndexCount];
        memcpy((void*)pDeviceIndices, (void*)in_struct->pDeviceIndices, sizeof(uint32_t) * in_struct->deviceIndexCount);
    }
}

void safe_VkBindAccelerationStructureMemoryInfoNV::initialize(const safe_VkBindAccelerationStructureMemoryInfoNV* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    accelerationStructure = copy_src->accelerationStructure;
    memory = copy_src->memory;
    memoryOffset = copy_src->memoryOffset;
    deviceIndexCount = copy_src->deviceIndexCount;
    pDeviceIndices = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pDeviceIndices) {
        pDeviceIndices = new uint32_t[copy_src->deviceIndexCount];
        memcpy((void*)pDeviceIndices, (void*)copy_src->pDeviceIndices, sizeof(uint32_t) * copy_src->deviceIndexCount);
    }
}

safe_VkWriteDescriptorSetAccelerationStructureNV::safe_VkWriteDescriptorSetAccelerationStructureNV(
    const VkWriteDescriptorSetAccelerationStructureNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), accelerationStructureCount(in_struct->accelerationStructureCount), pAccelerationStructures(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (accelerationStructureCount && in_struct->pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureNV[accelerationStructureCount];
        for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = in_struct->pAccelerationStructures[i];
        }
    }
}

safe_VkWriteDescriptorSetAccelerationStructureNV::safe_VkWriteDescriptorSetAccelerationStructureNV()
    : sType(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV),
      pNext(nullptr),
      accelerationStructureCount(),
      pAccelerationStructures(nullptr) {}

safe_VkWriteDescriptorSetAccelerationStructureNV::safe_VkWriteDescriptorSetAccelerationStructureNV(
    const safe_VkWriteDescriptorSetAccelerationStructureNV& copy_src) {
    sType = copy_src.sType;
    accelerationStructureCount = copy_src.accelerationStructureCount;
    pAccelerationStructures = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (accelerationStructureCount && copy_src.pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureNV[accelerationStructureCount];
        for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = copy_src.pAccelerationStructures[i];
        }
    }
}

safe_VkWriteDescriptorSetAccelerationStructureNV& safe_VkWriteDescriptorSetAccelerationStructureNV::operator=(
    const safe_VkWriteDescriptorSetAccelerationStructureNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pAccelerationStructures) delete[] pAccelerationStructures;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    accelerationStructureCount = copy_src.accelerationStructureCount;
    pAccelerationStructures = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (accelerationStructureCount && copy_src.pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureNV[accelerationStructureCount];
        for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = copy_src.pAccelerationStructures[i];
        }
    }

    return *this;
}

safe_VkWriteDescriptorSetAccelerationStructureNV::~safe_VkWriteDescriptorSetAccelerationStructureNV() {
    if (pAccelerationStructures) delete[] pAccelerationStructures;
    FreePnextChain(pNext);
}

void safe_VkWriteDescriptorSetAccelerationStructureNV::initialize(const VkWriteDescriptorSetAccelerationStructureNV* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    if (pAccelerationStructures) delete[] pAccelerationStructures;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    accelerationStructureCount = in_struct->accelerationStructureCount;
    pAccelerationStructures = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (accelerationStructureCount && in_struct->pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureNV[accelerationStructureCount];
        for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = in_struct->pAccelerationStructures[i];
        }
    }
}

void safe_VkWriteDescriptorSetAccelerationStructureNV::initialize(const safe_VkWriteDescriptorSetAccelerationStructureNV* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    accelerationStructureCount = copy_src->accelerationStructureCount;
    pAccelerationStructures = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (accelerationStructureCount && copy_src->pAccelerationStructures) {
        pAccelerationStructures = new VkAccelerationStructureNV[accelerationStructureCount];
        for (uint32_t i = 0; i < accelerationStructureCount; ++i) {
            pAccelerationStructures[i] = copy_src->pAccelerationStructures[i];
        }
    }
}

safe_VkAccelerationStructureMemoryRequirementsInfoNV::safe_VkAccelerationStructureMemoryRequirementsInfoNV(
    const VkAccelerationStructureMemoryRequirementsInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), type(in_struct->type), accelerationStructure(in_struct->accelerationStructure) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAccelerationStructureMemoryRequirementsInfoNV::safe_VkAccelerationStructureMemoryRequirementsInfoNV()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV),
      pNext(nullptr),
      type(),
      accelerationStructure() {}

safe_VkAccelerationStructureMemoryRequirementsInfoNV::safe_VkAccelerationStructureMemoryRequirementsInfoNV(
    const safe_VkAccelerationStructureMemoryRequirementsInfoNV& copy_src) {
    sType = copy_src.sType;
    type = copy_src.type;
    accelerationStructure = copy_src.accelerationStructure;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureMemoryRequirementsInfoNV& safe_VkAccelerationStructureMemoryRequirementsInfoNV::operator=(
    const safe_VkAccelerationStructureMemoryRequirementsInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    type = copy_src.type;
    accelerationStructure = copy_src.accelerationStructure;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAccelerationStructureMemoryRequirementsInfoNV::~safe_VkAccelerationStructureMemoryRequirementsInfoNV() {
    FreePnextChain(pNext);
}

void safe_VkAccelerationStructureMemoryRequirementsInfoNV::initialize(
    const VkAccelerationStructureMemoryRequirementsInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    type = in_struct->type;
    accelerationStructure = in_struct->accelerationStructure;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAccelerationStructureMemoryRequirementsInfoNV::initialize(
    const safe_VkAccelerationStructureMemoryRequirementsInfoNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    type = copy_src->type;
    accelerationStructure = copy_src->accelerationStructure;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRayTracingPropertiesNV::safe_VkPhysicalDeviceRayTracingPropertiesNV(
    const VkPhysicalDeviceRayTracingPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      shaderGroupHandleSize(in_struct->shaderGroupHandleSize),
      maxRecursionDepth(in_struct->maxRecursionDepth),
      maxShaderGroupStride(in_struct->maxShaderGroupStride),
      shaderGroupBaseAlignment(in_struct->shaderGroupBaseAlignment),
      maxGeometryCount(in_struct->maxGeometryCount),
      maxInstanceCount(in_struct->maxInstanceCount),
      maxTriangleCount(in_struct->maxTriangleCount),
      maxDescriptorSetAccelerationStructures(in_struct->maxDescriptorSetAccelerationStructures) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRayTracingPropertiesNV::safe_VkPhysicalDeviceRayTracingPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PROPERTIES_NV),
      pNext(nullptr),
      shaderGroupHandleSize(),
      maxRecursionDepth(),
      maxShaderGroupStride(),
      shaderGroupBaseAlignment(),
      maxGeometryCount(),
      maxInstanceCount(),
      maxTriangleCount(),
      maxDescriptorSetAccelerationStructures() {}

safe_VkPhysicalDeviceRayTracingPropertiesNV::safe_VkPhysicalDeviceRayTracingPropertiesNV(
    const safe_VkPhysicalDeviceRayTracingPropertiesNV& copy_src) {
    sType = copy_src.sType;
    shaderGroupHandleSize = copy_src.shaderGroupHandleSize;
    maxRecursionDepth = copy_src.maxRecursionDepth;
    maxShaderGroupStride = copy_src.maxShaderGroupStride;
    shaderGroupBaseAlignment = copy_src.shaderGroupBaseAlignment;
    maxGeometryCount = copy_src.maxGeometryCount;
    maxInstanceCount = copy_src.maxInstanceCount;
    maxTriangleCount = copy_src.maxTriangleCount;
    maxDescriptorSetAccelerationStructures = copy_src.maxDescriptorSetAccelerationStructures;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRayTracingPropertiesNV& safe_VkPhysicalDeviceRayTracingPropertiesNV::operator=(
    const safe_VkPhysicalDeviceRayTracingPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderGroupHandleSize = copy_src.shaderGroupHandleSize;
    maxRecursionDepth = copy_src.maxRecursionDepth;
    maxShaderGroupStride = copy_src.maxShaderGroupStride;
    shaderGroupBaseAlignment = copy_src.shaderGroupBaseAlignment;
    maxGeometryCount = copy_src.maxGeometryCount;
    maxInstanceCount = copy_src.maxInstanceCount;
    maxTriangleCount = copy_src.maxTriangleCount;
    maxDescriptorSetAccelerationStructures = copy_src.maxDescriptorSetAccelerationStructures;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRayTracingPropertiesNV::~safe_VkPhysicalDeviceRayTracingPropertiesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceRayTracingPropertiesNV::initialize(const VkPhysicalDeviceRayTracingPropertiesNV* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderGroupHandleSize = in_struct->shaderGroupHandleSize;
    maxRecursionDepth = in_struct->maxRecursionDepth;
    maxShaderGroupStride = in_struct->maxShaderGroupStride;
    shaderGroupBaseAlignment = in_struct->shaderGroupBaseAlignment;
    maxGeometryCount = in_struct->maxGeometryCount;
    maxInstanceCount = in_struct->maxInstanceCount;
    maxTriangleCount = in_struct->maxTriangleCount;
    maxDescriptorSetAccelerationStructures = in_struct->maxDescriptorSetAccelerationStructures;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRayTracingPropertiesNV::initialize(const safe_VkPhysicalDeviceRayTracingPropertiesNV* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderGroupHandleSize = copy_src->shaderGroupHandleSize;
    maxRecursionDepth = copy_src->maxRecursionDepth;
    maxShaderGroupStride = copy_src->maxShaderGroupStride;
    shaderGroupBaseAlignment = copy_src->shaderGroupBaseAlignment;
    maxGeometryCount = copy_src->maxGeometryCount;
    maxInstanceCount = copy_src->maxInstanceCount;
    maxTriangleCount = copy_src->maxTriangleCount;
    maxDescriptorSetAccelerationStructures = copy_src->maxDescriptorSetAccelerationStructures;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV::safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV(
    const VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), representativeFragmentTest(in_struct->representativeFragmentTest) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV::safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_REPRESENTATIVE_FRAGMENT_TEST_FEATURES_NV),
      pNext(nullptr),
      representativeFragmentTest() {}

safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV::safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV(
    const safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV& copy_src) {
    sType = copy_src.sType;
    representativeFragmentTest = copy_src.representativeFragmentTest;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV& safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV::operator=(
    const safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    representativeFragmentTest = copy_src.representativeFragmentTest;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV::~safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV::initialize(
    const VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    representativeFragmentTest = in_struct->representativeFragmentTest;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV::initialize(
    const safe_VkPhysicalDeviceRepresentativeFragmentTestFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    representativeFragmentTest = copy_src->representativeFragmentTest;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV::safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV(
    const VkPipelineRepresentativeFragmentTestStateCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), representativeFragmentTestEnable(in_struct->representativeFragmentTestEnable) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV::safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_REPRESENTATIVE_FRAGMENT_TEST_STATE_CREATE_INFO_NV),
      pNext(nullptr),
      representativeFragmentTestEnable() {}

safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV::safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV(
    const safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    representativeFragmentTestEnable = copy_src.representativeFragmentTestEnable;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV& safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV::operator=(
    const safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    representativeFragmentTestEnable = copy_src.representativeFragmentTestEnable;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV::~safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV() {
    FreePnextChain(pNext);
}

void safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV::initialize(
    const VkPipelineRepresentativeFragmentTestStateCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    representativeFragmentTestEnable = in_struct->representativeFragmentTestEnable;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV::initialize(
    const safe_VkPipelineRepresentativeFragmentTestStateCreateInfoNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    representativeFragmentTestEnable = copy_src->representativeFragmentTestEnable;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineCompilerControlCreateInfoAMD::safe_VkPipelineCompilerControlCreateInfoAMD(
    const VkPipelineCompilerControlCreateInfoAMD* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), compilerControlFlags(in_struct->compilerControlFlags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineCompilerControlCreateInfoAMD::safe_VkPipelineCompilerControlCreateInfoAMD()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_COMPILER_CONTROL_CREATE_INFO_AMD), pNext(nullptr), compilerControlFlags() {}

safe_VkPipelineCompilerControlCreateInfoAMD::safe_VkPipelineCompilerControlCreateInfoAMD(
    const safe_VkPipelineCompilerControlCreateInfoAMD& copy_src) {
    sType = copy_src.sType;
    compilerControlFlags = copy_src.compilerControlFlags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineCompilerControlCreateInfoAMD& safe_VkPipelineCompilerControlCreateInfoAMD::operator=(
    const safe_VkPipelineCompilerControlCreateInfoAMD& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    compilerControlFlags = copy_src.compilerControlFlags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineCompilerControlCreateInfoAMD::~safe_VkPipelineCompilerControlCreateInfoAMD() { FreePnextChain(pNext); }

void safe_VkPipelineCompilerControlCreateInfoAMD::initialize(const VkPipelineCompilerControlCreateInfoAMD* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    compilerControlFlags = in_struct->compilerControlFlags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineCompilerControlCreateInfoAMD::initialize(const safe_VkPipelineCompilerControlCreateInfoAMD* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    compilerControlFlags = copy_src->compilerControlFlags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderCorePropertiesAMD::safe_VkPhysicalDeviceShaderCorePropertiesAMD(
    const VkPhysicalDeviceShaderCorePropertiesAMD* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      shaderEngineCount(in_struct->shaderEngineCount),
      shaderArraysPerEngineCount(in_struct->shaderArraysPerEngineCount),
      computeUnitsPerShaderArray(in_struct->computeUnitsPerShaderArray),
      simdPerComputeUnit(in_struct->simdPerComputeUnit),
      wavefrontsPerSimd(in_struct->wavefrontsPerSimd),
      wavefrontSize(in_struct->wavefrontSize),
      sgprsPerSimd(in_struct->sgprsPerSimd),
      minSgprAllocation(in_struct->minSgprAllocation),
      maxSgprAllocation(in_struct->maxSgprAllocation),
      sgprAllocationGranularity(in_struct->sgprAllocationGranularity),
      vgprsPerSimd(in_struct->vgprsPerSimd),
      minVgprAllocation(in_struct->minVgprAllocation),
      maxVgprAllocation(in_struct->maxVgprAllocation),
      vgprAllocationGranularity(in_struct->vgprAllocationGranularity) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderCorePropertiesAMD::safe_VkPhysicalDeviceShaderCorePropertiesAMD()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_AMD),
      pNext(nullptr),
      shaderEngineCount(),
      shaderArraysPerEngineCount(),
      computeUnitsPerShaderArray(),
      simdPerComputeUnit(),
      wavefrontsPerSimd(),
      wavefrontSize(),
      sgprsPerSimd(),
      minSgprAllocation(),
      maxSgprAllocation(),
      sgprAllocationGranularity(),
      vgprsPerSimd(),
      minVgprAllocation(),
      maxVgprAllocation(),
      vgprAllocationGranularity() {}

safe_VkPhysicalDeviceShaderCorePropertiesAMD::safe_VkPhysicalDeviceShaderCorePropertiesAMD(
    const safe_VkPhysicalDeviceShaderCorePropertiesAMD& copy_src) {
    sType = copy_src.sType;
    shaderEngineCount = copy_src.shaderEngineCount;
    shaderArraysPerEngineCount = copy_src.shaderArraysPerEngineCount;
    computeUnitsPerShaderArray = copy_src.computeUnitsPerShaderArray;
    simdPerComputeUnit = copy_src.simdPerComputeUnit;
    wavefrontsPerSimd = copy_src.wavefrontsPerSimd;
    wavefrontSize = copy_src.wavefrontSize;
    sgprsPerSimd = copy_src.sgprsPerSimd;
    minSgprAllocation = copy_src.minSgprAllocation;
    maxSgprAllocation = copy_src.maxSgprAllocation;
    sgprAllocationGranularity = copy_src.sgprAllocationGranularity;
    vgprsPerSimd = copy_src.vgprsPerSimd;
    minVgprAllocation = copy_src.minVgprAllocation;
    maxVgprAllocation = copy_src.maxVgprAllocation;
    vgprAllocationGranularity = copy_src.vgprAllocationGranularity;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderCorePropertiesAMD& safe_VkPhysicalDeviceShaderCorePropertiesAMD::operator=(
    const safe_VkPhysicalDeviceShaderCorePropertiesAMD& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderEngineCount = copy_src.shaderEngineCount;
    shaderArraysPerEngineCount = copy_src.shaderArraysPerEngineCount;
    computeUnitsPerShaderArray = copy_src.computeUnitsPerShaderArray;
    simdPerComputeUnit = copy_src.simdPerComputeUnit;
    wavefrontsPerSimd = copy_src.wavefrontsPerSimd;
    wavefrontSize = copy_src.wavefrontSize;
    sgprsPerSimd = copy_src.sgprsPerSimd;
    minSgprAllocation = copy_src.minSgprAllocation;
    maxSgprAllocation = copy_src.maxSgprAllocation;
    sgprAllocationGranularity = copy_src.sgprAllocationGranularity;
    vgprsPerSimd = copy_src.vgprsPerSimd;
    minVgprAllocation = copy_src.minVgprAllocation;
    maxVgprAllocation = copy_src.maxVgprAllocation;
    vgprAllocationGranularity = copy_src.vgprAllocationGranularity;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderCorePropertiesAMD::~safe_VkPhysicalDeviceShaderCorePropertiesAMD() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderCorePropertiesAMD::initialize(const VkPhysicalDeviceShaderCorePropertiesAMD* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderEngineCount = in_struct->shaderEngineCount;
    shaderArraysPerEngineCount = in_struct->shaderArraysPerEngineCount;
    computeUnitsPerShaderArray = in_struct->computeUnitsPerShaderArray;
    simdPerComputeUnit = in_struct->simdPerComputeUnit;
    wavefrontsPerSimd = in_struct->wavefrontsPerSimd;
    wavefrontSize = in_struct->wavefrontSize;
    sgprsPerSimd = in_struct->sgprsPerSimd;
    minSgprAllocation = in_struct->minSgprAllocation;
    maxSgprAllocation = in_struct->maxSgprAllocation;
    sgprAllocationGranularity = in_struct->sgprAllocationGranularity;
    vgprsPerSimd = in_struct->vgprsPerSimd;
    minVgprAllocation = in_struct->minVgprAllocation;
    maxVgprAllocation = in_struct->maxVgprAllocation;
    vgprAllocationGranularity = in_struct->vgprAllocationGranularity;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderCorePropertiesAMD::initialize(const safe_VkPhysicalDeviceShaderCorePropertiesAMD* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderEngineCount = copy_src->shaderEngineCount;
    shaderArraysPerEngineCount = copy_src->shaderArraysPerEngineCount;
    computeUnitsPerShaderArray = copy_src->computeUnitsPerShaderArray;
    simdPerComputeUnit = copy_src->simdPerComputeUnit;
    wavefrontsPerSimd = copy_src->wavefrontsPerSimd;
    wavefrontSize = copy_src->wavefrontSize;
    sgprsPerSimd = copy_src->sgprsPerSimd;
    minSgprAllocation = copy_src->minSgprAllocation;
    maxSgprAllocation = copy_src->maxSgprAllocation;
    sgprAllocationGranularity = copy_src->sgprAllocationGranularity;
    vgprsPerSimd = copy_src->vgprsPerSimd;
    minVgprAllocation = copy_src->minVgprAllocation;
    maxVgprAllocation = copy_src->maxVgprAllocation;
    vgprAllocationGranularity = copy_src->vgprAllocationGranularity;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDeviceMemoryOverallocationCreateInfoAMD::safe_VkDeviceMemoryOverallocationCreateInfoAMD(
    const VkDeviceMemoryOverallocationCreateInfoAMD* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), overallocationBehavior(in_struct->overallocationBehavior) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDeviceMemoryOverallocationCreateInfoAMD::safe_VkDeviceMemoryOverallocationCreateInfoAMD()
    : sType(VK_STRUCTURE_TYPE_DEVICE_MEMORY_OVERALLOCATION_CREATE_INFO_AMD), pNext(nullptr), overallocationBehavior() {}

safe_VkDeviceMemoryOverallocationCreateInfoAMD::safe_VkDeviceMemoryOverallocationCreateInfoAMD(
    const safe_VkDeviceMemoryOverallocationCreateInfoAMD& copy_src) {
    sType = copy_src.sType;
    overallocationBehavior = copy_src.overallocationBehavior;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDeviceMemoryOverallocationCreateInfoAMD& safe_VkDeviceMemoryOverallocationCreateInfoAMD::operator=(
    const safe_VkDeviceMemoryOverallocationCreateInfoAMD& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    overallocationBehavior = copy_src.overallocationBehavior;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDeviceMemoryOverallocationCreateInfoAMD::~safe_VkDeviceMemoryOverallocationCreateInfoAMD() { FreePnextChain(pNext); }

void safe_VkDeviceMemoryOverallocationCreateInfoAMD::initialize(const VkDeviceMemoryOverallocationCreateInfoAMD* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    overallocationBehavior = in_struct->overallocationBehavior;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDeviceMemoryOverallocationCreateInfoAMD::initialize(const safe_VkDeviceMemoryOverallocationCreateInfoAMD* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    overallocationBehavior = copy_src->overallocationBehavior;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_USE_PLATFORM_GGP

safe_VkPresentFrameTokenGGP::safe_VkPresentFrameTokenGGP(const VkPresentFrameTokenGGP* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), frameToken(in_struct->frameToken) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPresentFrameTokenGGP::safe_VkPresentFrameTokenGGP()
    : sType(VK_STRUCTURE_TYPE_PRESENT_FRAME_TOKEN_GGP), pNext(nullptr), frameToken() {}

safe_VkPresentFrameTokenGGP::safe_VkPresentFrameTokenGGP(const safe_VkPresentFrameTokenGGP& copy_src) {
    sType = copy_src.sType;
    frameToken = copy_src.frameToken;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPresentFrameTokenGGP& safe_VkPresentFrameTokenGGP::operator=(const safe_VkPresentFrameTokenGGP& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    frameToken = copy_src.frameToken;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPresentFrameTokenGGP::~safe_VkPresentFrameTokenGGP() { FreePnextChain(pNext); }

void safe_VkPresentFrameTokenGGP::initialize(const VkPresentFrameTokenGGP* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    frameToken = in_struct->frameToken;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPresentFrameTokenGGP::initialize(const safe_VkPresentFrameTokenGGP* copy_src,
                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    frameToken = copy_src->frameToken;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_GGP

safe_VkPhysicalDeviceMeshShaderFeaturesNV::safe_VkPhysicalDeviceMeshShaderFeaturesNV(
    const VkPhysicalDeviceMeshShaderFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), taskShader(in_struct->taskShader), meshShader(in_struct->meshShader) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMeshShaderFeaturesNV::safe_VkPhysicalDeviceMeshShaderFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV), pNext(nullptr), taskShader(), meshShader() {}

safe_VkPhysicalDeviceMeshShaderFeaturesNV::safe_VkPhysicalDeviceMeshShaderFeaturesNV(
    const safe_VkPhysicalDeviceMeshShaderFeaturesNV& copy_src) {
    sType = copy_src.sType;
    taskShader = copy_src.taskShader;
    meshShader = copy_src.meshShader;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMeshShaderFeaturesNV& safe_VkPhysicalDeviceMeshShaderFeaturesNV::operator=(
    const safe_VkPhysicalDeviceMeshShaderFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    taskShader = copy_src.taskShader;
    meshShader = copy_src.meshShader;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMeshShaderFeaturesNV::~safe_VkPhysicalDeviceMeshShaderFeaturesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceMeshShaderFeaturesNV::initialize(const VkPhysicalDeviceMeshShaderFeaturesNV* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    taskShader = in_struct->taskShader;
    meshShader = in_struct->meshShader;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMeshShaderFeaturesNV::initialize(const safe_VkPhysicalDeviceMeshShaderFeaturesNV* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    taskShader = copy_src->taskShader;
    meshShader = copy_src->meshShader;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceMeshShaderPropertiesNV::safe_VkPhysicalDeviceMeshShaderPropertiesNV(
    const VkPhysicalDeviceMeshShaderPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      maxDrawMeshTasksCount(in_struct->maxDrawMeshTasksCount),
      maxTaskWorkGroupInvocations(in_struct->maxTaskWorkGroupInvocations),
      maxTaskTotalMemorySize(in_struct->maxTaskTotalMemorySize),
      maxTaskOutputCount(in_struct->maxTaskOutputCount),
      maxMeshWorkGroupInvocations(in_struct->maxMeshWorkGroupInvocations),
      maxMeshTotalMemorySize(in_struct->maxMeshTotalMemorySize),
      maxMeshOutputVertices(in_struct->maxMeshOutputVertices),
      maxMeshOutputPrimitives(in_struct->maxMeshOutputPrimitives),
      maxMeshMultiviewViewCount(in_struct->maxMeshMultiviewViewCount),
      meshOutputPerVertexGranularity(in_struct->meshOutputPerVertexGranularity),
      meshOutputPerPrimitiveGranularity(in_struct->meshOutputPerPrimitiveGranularity) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < 3; ++i) {
        maxTaskWorkGroupSize[i] = in_struct->maxTaskWorkGroupSize[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxMeshWorkGroupSize[i] = in_struct->maxMeshWorkGroupSize[i];
    }
}

safe_VkPhysicalDeviceMeshShaderPropertiesNV::safe_VkPhysicalDeviceMeshShaderPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV),
      pNext(nullptr),
      maxDrawMeshTasksCount(),
      maxTaskWorkGroupInvocations(),
      maxTaskTotalMemorySize(),
      maxTaskOutputCount(),
      maxMeshWorkGroupInvocations(),
      maxMeshTotalMemorySize(),
      maxMeshOutputVertices(),
      maxMeshOutputPrimitives(),
      maxMeshMultiviewViewCount(),
      meshOutputPerVertexGranularity(),
      meshOutputPerPrimitiveGranularity() {}

safe_VkPhysicalDeviceMeshShaderPropertiesNV::safe_VkPhysicalDeviceMeshShaderPropertiesNV(
    const safe_VkPhysicalDeviceMeshShaderPropertiesNV& copy_src) {
    sType = copy_src.sType;
    maxDrawMeshTasksCount = copy_src.maxDrawMeshTasksCount;
    maxTaskWorkGroupInvocations = copy_src.maxTaskWorkGroupInvocations;
    maxTaskTotalMemorySize = copy_src.maxTaskTotalMemorySize;
    maxTaskOutputCount = copy_src.maxTaskOutputCount;
    maxMeshWorkGroupInvocations = copy_src.maxMeshWorkGroupInvocations;
    maxMeshTotalMemorySize = copy_src.maxMeshTotalMemorySize;
    maxMeshOutputVertices = copy_src.maxMeshOutputVertices;
    maxMeshOutputPrimitives = copy_src.maxMeshOutputPrimitives;
    maxMeshMultiviewViewCount = copy_src.maxMeshMultiviewViewCount;
    meshOutputPerVertexGranularity = copy_src.meshOutputPerVertexGranularity;
    meshOutputPerPrimitiveGranularity = copy_src.meshOutputPerPrimitiveGranularity;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < 3; ++i) {
        maxTaskWorkGroupSize[i] = copy_src.maxTaskWorkGroupSize[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxMeshWorkGroupSize[i] = copy_src.maxMeshWorkGroupSize[i];
    }
}

safe_VkPhysicalDeviceMeshShaderPropertiesNV& safe_VkPhysicalDeviceMeshShaderPropertiesNV::operator=(
    const safe_VkPhysicalDeviceMeshShaderPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxDrawMeshTasksCount = copy_src.maxDrawMeshTasksCount;
    maxTaskWorkGroupInvocations = copy_src.maxTaskWorkGroupInvocations;
    maxTaskTotalMemorySize = copy_src.maxTaskTotalMemorySize;
    maxTaskOutputCount = copy_src.maxTaskOutputCount;
    maxMeshWorkGroupInvocations = copy_src.maxMeshWorkGroupInvocations;
    maxMeshTotalMemorySize = copy_src.maxMeshTotalMemorySize;
    maxMeshOutputVertices = copy_src.maxMeshOutputVertices;
    maxMeshOutputPrimitives = copy_src.maxMeshOutputPrimitives;
    maxMeshMultiviewViewCount = copy_src.maxMeshMultiviewViewCount;
    meshOutputPerVertexGranularity = copy_src.meshOutputPerVertexGranularity;
    meshOutputPerPrimitiveGranularity = copy_src.meshOutputPerPrimitiveGranularity;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < 3; ++i) {
        maxTaskWorkGroupSize[i] = copy_src.maxTaskWorkGroupSize[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxMeshWorkGroupSize[i] = copy_src.maxMeshWorkGroupSize[i];
    }

    return *this;
}

safe_VkPhysicalDeviceMeshShaderPropertiesNV::~safe_VkPhysicalDeviceMeshShaderPropertiesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceMeshShaderPropertiesNV::initialize(const VkPhysicalDeviceMeshShaderPropertiesNV* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxDrawMeshTasksCount = in_struct->maxDrawMeshTasksCount;
    maxTaskWorkGroupInvocations = in_struct->maxTaskWorkGroupInvocations;
    maxTaskTotalMemorySize = in_struct->maxTaskTotalMemorySize;
    maxTaskOutputCount = in_struct->maxTaskOutputCount;
    maxMeshWorkGroupInvocations = in_struct->maxMeshWorkGroupInvocations;
    maxMeshTotalMemorySize = in_struct->maxMeshTotalMemorySize;
    maxMeshOutputVertices = in_struct->maxMeshOutputVertices;
    maxMeshOutputPrimitives = in_struct->maxMeshOutputPrimitives;
    maxMeshMultiviewViewCount = in_struct->maxMeshMultiviewViewCount;
    meshOutputPerVertexGranularity = in_struct->meshOutputPerVertexGranularity;
    meshOutputPerPrimitiveGranularity = in_struct->meshOutputPerPrimitiveGranularity;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < 3; ++i) {
        maxTaskWorkGroupSize[i] = in_struct->maxTaskWorkGroupSize[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxMeshWorkGroupSize[i] = in_struct->maxMeshWorkGroupSize[i];
    }
}

void safe_VkPhysicalDeviceMeshShaderPropertiesNV::initialize(const safe_VkPhysicalDeviceMeshShaderPropertiesNV* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxDrawMeshTasksCount = copy_src->maxDrawMeshTasksCount;
    maxTaskWorkGroupInvocations = copy_src->maxTaskWorkGroupInvocations;
    maxTaskTotalMemorySize = copy_src->maxTaskTotalMemorySize;
    maxTaskOutputCount = copy_src->maxTaskOutputCount;
    maxMeshWorkGroupInvocations = copy_src->maxMeshWorkGroupInvocations;
    maxMeshTotalMemorySize = copy_src->maxMeshTotalMemorySize;
    maxMeshOutputVertices = copy_src->maxMeshOutputVertices;
    maxMeshOutputPrimitives = copy_src->maxMeshOutputPrimitives;
    maxMeshMultiviewViewCount = copy_src->maxMeshMultiviewViewCount;
    meshOutputPerVertexGranularity = copy_src->meshOutputPerVertexGranularity;
    meshOutputPerPrimitiveGranularity = copy_src->meshOutputPerPrimitiveGranularity;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < 3; ++i) {
        maxTaskWorkGroupSize[i] = copy_src->maxTaskWorkGroupSize[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxMeshWorkGroupSize[i] = copy_src->maxMeshWorkGroupSize[i];
    }
}

safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV::safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV(
    const VkPhysicalDeviceShaderImageFootprintFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), imageFootprint(in_struct->imageFootprint) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV::safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_IMAGE_FOOTPRINT_FEATURES_NV), pNext(nullptr), imageFootprint() {}

safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV::safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV(
    const safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV& copy_src) {
    sType = copy_src.sType;
    imageFootprint = copy_src.imageFootprint;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV& safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV::operator=(
    const safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    imageFootprint = copy_src.imageFootprint;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV::~safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV::initialize(
    const VkPhysicalDeviceShaderImageFootprintFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    imageFootprint = in_struct->imageFootprint;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV::initialize(
    const safe_VkPhysicalDeviceShaderImageFootprintFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    imageFootprint = copy_src->imageFootprint;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV::safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV(
    const VkPipelineViewportExclusiveScissorStateCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), exclusiveScissorCount(in_struct->exclusiveScissorCount), pExclusiveScissors(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pExclusiveScissors) {
        pExclusiveScissors = new VkRect2D[in_struct->exclusiveScissorCount];
        memcpy((void*)pExclusiveScissors, (void*)in_struct->pExclusiveScissors,
               sizeof(VkRect2D) * in_struct->exclusiveScissorCount);
    }
}

safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV::safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_EXCLUSIVE_SCISSOR_STATE_CREATE_INFO_NV),
      pNext(nullptr),
      exclusiveScissorCount(),
      pExclusiveScissors(nullptr) {}

safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV::safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV(
    const safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    exclusiveScissorCount = copy_src.exclusiveScissorCount;
    pExclusiveScissors = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pExclusiveScissors) {
        pExclusiveScissors = new VkRect2D[copy_src.exclusiveScissorCount];
        memcpy((void*)pExclusiveScissors, (void*)copy_src.pExclusiveScissors, sizeof(VkRect2D) * copy_src.exclusiveScissorCount);
    }
}

safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV& safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV::operator=(
    const safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pExclusiveScissors) delete[] pExclusiveScissors;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    exclusiveScissorCount = copy_src.exclusiveScissorCount;
    pExclusiveScissors = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pExclusiveScissors) {
        pExclusiveScissors = new VkRect2D[copy_src.exclusiveScissorCount];
        memcpy((void*)pExclusiveScissors, (void*)copy_src.pExclusiveScissors, sizeof(VkRect2D) * copy_src.exclusiveScissorCount);
    }

    return *this;
}

safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV::~safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV() {
    if (pExclusiveScissors) delete[] pExclusiveScissors;
    FreePnextChain(pNext);
}

void safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV::initialize(
    const VkPipelineViewportExclusiveScissorStateCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pExclusiveScissors) delete[] pExclusiveScissors;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    exclusiveScissorCount = in_struct->exclusiveScissorCount;
    pExclusiveScissors = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pExclusiveScissors) {
        pExclusiveScissors = new VkRect2D[in_struct->exclusiveScissorCount];
        memcpy((void*)pExclusiveScissors, (void*)in_struct->pExclusiveScissors,
               sizeof(VkRect2D) * in_struct->exclusiveScissorCount);
    }
}

void safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV::initialize(
    const safe_VkPipelineViewportExclusiveScissorStateCreateInfoNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    exclusiveScissorCount = copy_src->exclusiveScissorCount;
    pExclusiveScissors = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pExclusiveScissors) {
        pExclusiveScissors = new VkRect2D[copy_src->exclusiveScissorCount];
        memcpy((void*)pExclusiveScissors, (void*)copy_src->pExclusiveScissors, sizeof(VkRect2D) * copy_src->exclusiveScissorCount);
    }
}

safe_VkPhysicalDeviceExclusiveScissorFeaturesNV::safe_VkPhysicalDeviceExclusiveScissorFeaturesNV(
    const VkPhysicalDeviceExclusiveScissorFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), exclusiveScissor(in_struct->exclusiveScissor) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceExclusiveScissorFeaturesNV::safe_VkPhysicalDeviceExclusiveScissorFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXCLUSIVE_SCISSOR_FEATURES_NV), pNext(nullptr), exclusiveScissor() {}

safe_VkPhysicalDeviceExclusiveScissorFeaturesNV::safe_VkPhysicalDeviceExclusiveScissorFeaturesNV(
    const safe_VkPhysicalDeviceExclusiveScissorFeaturesNV& copy_src) {
    sType = copy_src.sType;
    exclusiveScissor = copy_src.exclusiveScissor;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceExclusiveScissorFeaturesNV& safe_VkPhysicalDeviceExclusiveScissorFeaturesNV::operator=(
    const safe_VkPhysicalDeviceExclusiveScissorFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    exclusiveScissor = copy_src.exclusiveScissor;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceExclusiveScissorFeaturesNV::~safe_VkPhysicalDeviceExclusiveScissorFeaturesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceExclusiveScissorFeaturesNV::initialize(const VkPhysicalDeviceExclusiveScissorFeaturesNV* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    exclusiveScissor = in_struct->exclusiveScissor;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceExclusiveScissorFeaturesNV::initialize(const safe_VkPhysicalDeviceExclusiveScissorFeaturesNV* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    exclusiveScissor = copy_src->exclusiveScissor;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkQueueFamilyCheckpointPropertiesNV::safe_VkQueueFamilyCheckpointPropertiesNV(
    const VkQueueFamilyCheckpointPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), checkpointExecutionStageMask(in_struct->checkpointExecutionStageMask) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkQueueFamilyCheckpointPropertiesNV::safe_VkQueueFamilyCheckpointPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_NV), pNext(nullptr), checkpointExecutionStageMask() {}

safe_VkQueueFamilyCheckpointPropertiesNV::safe_VkQueueFamilyCheckpointPropertiesNV(
    const safe_VkQueueFamilyCheckpointPropertiesNV& copy_src) {
    sType = copy_src.sType;
    checkpointExecutionStageMask = copy_src.checkpointExecutionStageMask;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkQueueFamilyCheckpointPropertiesNV& safe_VkQueueFamilyCheckpointPropertiesNV::operator=(
    const safe_VkQueueFamilyCheckpointPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    checkpointExecutionStageMask = copy_src.checkpointExecutionStageMask;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkQueueFamilyCheckpointPropertiesNV::~safe_VkQueueFamilyCheckpointPropertiesNV() { FreePnextChain(pNext); }

void safe_VkQueueFamilyCheckpointPropertiesNV::initialize(const VkQueueFamilyCheckpointPropertiesNV* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    checkpointExecutionStageMask = in_struct->checkpointExecutionStageMask;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkQueueFamilyCheckpointPropertiesNV::initialize(const safe_VkQueueFamilyCheckpointPropertiesNV* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    checkpointExecutionStageMask = copy_src->checkpointExecutionStageMask;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCheckpointDataNV::safe_VkCheckpointDataNV(const VkCheckpointDataNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
                                                 bool copy_pnext)
    : sType(in_struct->sType), stage(in_struct->stage), pCheckpointMarker(in_struct->pCheckpointMarker) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCheckpointDataNV::safe_VkCheckpointDataNV()
    : sType(VK_STRUCTURE_TYPE_CHECKPOINT_DATA_NV), pNext(nullptr), stage(), pCheckpointMarker(nullptr) {}

safe_VkCheckpointDataNV::safe_VkCheckpointDataNV(const safe_VkCheckpointDataNV& copy_src) {
    sType = copy_src.sType;
    stage = copy_src.stage;
    pCheckpointMarker = copy_src.pCheckpointMarker;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCheckpointDataNV& safe_VkCheckpointDataNV::operator=(const safe_VkCheckpointDataNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    stage = copy_src.stage;
    pCheckpointMarker = copy_src.pCheckpointMarker;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCheckpointDataNV::~safe_VkCheckpointDataNV() { FreePnextChain(pNext); }

void safe_VkCheckpointDataNV::initialize(const VkCheckpointDataNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stage = in_struct->stage;
    pCheckpointMarker = in_struct->pCheckpointMarker;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCheckpointDataNV::initialize(const safe_VkCheckpointDataNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stage = copy_src->stage;
    pCheckpointMarker = copy_src->pCheckpointMarker;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkQueueFamilyCheckpointProperties2NV::safe_VkQueueFamilyCheckpointProperties2NV(
    const VkQueueFamilyCheckpointProperties2NV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), checkpointExecutionStageMask(in_struct->checkpointExecutionStageMask) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkQueueFamilyCheckpointProperties2NV::safe_VkQueueFamilyCheckpointProperties2NV()
    : sType(VK_STRUCTURE_TYPE_QUEUE_FAMILY_CHECKPOINT_PROPERTIES_2_NV), pNext(nullptr), checkpointExecutionStageMask() {}

safe_VkQueueFamilyCheckpointProperties2NV::safe_VkQueueFamilyCheckpointProperties2NV(
    const safe_VkQueueFamilyCheckpointProperties2NV& copy_src) {
    sType = copy_src.sType;
    checkpointExecutionStageMask = copy_src.checkpointExecutionStageMask;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkQueueFamilyCheckpointProperties2NV& safe_VkQueueFamilyCheckpointProperties2NV::operator=(
    const safe_VkQueueFamilyCheckpointProperties2NV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    checkpointExecutionStageMask = copy_src.checkpointExecutionStageMask;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkQueueFamilyCheckpointProperties2NV::~safe_VkQueueFamilyCheckpointProperties2NV() { FreePnextChain(pNext); }

void safe_VkQueueFamilyCheckpointProperties2NV::initialize(const VkQueueFamilyCheckpointProperties2NV* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    checkpointExecutionStageMask = in_struct->checkpointExecutionStageMask;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkQueueFamilyCheckpointProperties2NV::initialize(const safe_VkQueueFamilyCheckpointProperties2NV* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    checkpointExecutionStageMask = copy_src->checkpointExecutionStageMask;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCheckpointData2NV::safe_VkCheckpointData2NV(const VkCheckpointData2NV* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), stage(in_struct->stage), pCheckpointMarker(in_struct->pCheckpointMarker) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCheckpointData2NV::safe_VkCheckpointData2NV()
    : sType(VK_STRUCTURE_TYPE_CHECKPOINT_DATA_2_NV), pNext(nullptr), stage(), pCheckpointMarker(nullptr) {}

safe_VkCheckpointData2NV::safe_VkCheckpointData2NV(const safe_VkCheckpointData2NV& copy_src) {
    sType = copy_src.sType;
    stage = copy_src.stage;
    pCheckpointMarker = copy_src.pCheckpointMarker;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCheckpointData2NV& safe_VkCheckpointData2NV::operator=(const safe_VkCheckpointData2NV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    stage = copy_src.stage;
    pCheckpointMarker = copy_src.pCheckpointMarker;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCheckpointData2NV::~safe_VkCheckpointData2NV() { FreePnextChain(pNext); }

void safe_VkCheckpointData2NV::initialize(const VkCheckpointData2NV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stage = in_struct->stage;
    pCheckpointMarker = in_struct->pCheckpointMarker;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCheckpointData2NV::initialize(const safe_VkCheckpointData2NV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stage = copy_src->stage;
    pCheckpointMarker = copy_src->pCheckpointMarker;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL::safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL(
    const VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), shaderIntegerFunctions2(in_struct->shaderIntegerFunctions2) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL::safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_FUNCTIONS_2_FEATURES_INTEL),
      pNext(nullptr),
      shaderIntegerFunctions2() {}

safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL::safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL(
    const safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL& copy_src) {
    sType = copy_src.sType;
    shaderIntegerFunctions2 = copy_src.shaderIntegerFunctions2;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL& safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL::operator=(
    const safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderIntegerFunctions2 = copy_src.shaderIntegerFunctions2;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL::~safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL::initialize(
    const VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderIntegerFunctions2 = in_struct->shaderIntegerFunctions2;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL::initialize(
    const safe_VkPhysicalDeviceShaderIntegerFunctions2FeaturesINTEL* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderIntegerFunctions2 = copy_src->shaderIntegerFunctions2;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPerformanceValueDataINTEL::safe_VkPerformanceValueDataINTEL(const VkPerformanceValueDataINTEL* in_struct, PNextCopyState*) {
    initialize(in_struct);
}

safe_VkPerformanceValueDataINTEL::safe_VkPerformanceValueDataINTEL() : valueString(nullptr) {}

safe_VkPerformanceValueDataINTEL::safe_VkPerformanceValueDataINTEL(const safe_VkPerformanceValueDataINTEL& copy_src) {
    value32 = copy_src.value32;
    value64 = copy_src.value64;
    valueFloat = copy_src.valueFloat;
    valueBool = copy_src.valueBool;
    valueString = SafeStringCopy(copy_src.valueString);
}

safe_VkPerformanceValueDataINTEL& safe_VkPerformanceValueDataINTEL::operator=(const safe_VkPerformanceValueDataINTEL& copy_src) {
    if (&copy_src == this) return *this;

    if (valueString) delete[] valueString;

    value32 = copy_src.value32;
    value64 = copy_src.value64;
    valueFloat = copy_src.valueFloat;
    valueBool = copy_src.valueBool;
    valueString = SafeStringCopy(copy_src.valueString);

    return *this;
}

safe_VkPerformanceValueDataINTEL::~safe_VkPerformanceValueDataINTEL() {
    if (valueString) delete[] valueString;
}

void safe_VkPerformanceValueDataINTEL::initialize(const VkPerformanceValueDataINTEL* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    if (valueString) delete[] valueString;
    value32 = in_struct->value32;
    value64 = in_struct->value64;
    valueFloat = in_struct->valueFloat;
    valueBool = in_struct->valueBool;
    valueString = SafeStringCopy(in_struct->valueString);
}

void safe_VkPerformanceValueDataINTEL::initialize(const safe_VkPerformanceValueDataINTEL* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    value32 = copy_src->value32;
    value64 = copy_src->value64;
    valueFloat = copy_src->valueFloat;
    valueBool = copy_src->valueBool;
    valueString = SafeStringCopy(copy_src->valueString);
}

safe_VkInitializePerformanceApiInfoINTEL::safe_VkInitializePerformanceApiInfoINTEL(
    const VkInitializePerformanceApiInfoINTEL* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pUserData(in_struct->pUserData) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkInitializePerformanceApiInfoINTEL::safe_VkInitializePerformanceApiInfoINTEL()
    : sType(VK_STRUCTURE_TYPE_INITIALIZE_PERFORMANCE_API_INFO_INTEL), pNext(nullptr), pUserData(nullptr) {}

safe_VkInitializePerformanceApiInfoINTEL::safe_VkInitializePerformanceApiInfoINTEL(
    const safe_VkInitializePerformanceApiInfoINTEL& copy_src) {
    sType = copy_src.sType;
    pUserData = copy_src.pUserData;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkInitializePerformanceApiInfoINTEL& safe_VkInitializePerformanceApiInfoINTEL::operator=(
    const safe_VkInitializePerformanceApiInfoINTEL& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pUserData = copy_src.pUserData;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkInitializePerformanceApiInfoINTEL::~safe_VkInitializePerformanceApiInfoINTEL() { FreePnextChain(pNext); }

void safe_VkInitializePerformanceApiInfoINTEL::initialize(const VkInitializePerformanceApiInfoINTEL* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pUserData = in_struct->pUserData;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkInitializePerformanceApiInfoINTEL::initialize(const safe_VkInitializePerformanceApiInfoINTEL* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pUserData = copy_src->pUserData;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkQueryPoolPerformanceQueryCreateInfoINTEL::safe_VkQueryPoolPerformanceQueryCreateInfoINTEL(
    const VkQueryPoolPerformanceQueryCreateInfoINTEL* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), performanceCountersSampling(in_struct->performanceCountersSampling) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkQueryPoolPerformanceQueryCreateInfoINTEL::safe_VkQueryPoolPerformanceQueryCreateInfoINTEL()
    : sType(VK_STRUCTURE_TYPE_QUERY_POOL_PERFORMANCE_QUERY_CREATE_INFO_INTEL), pNext(nullptr), performanceCountersSampling() {}

safe_VkQueryPoolPerformanceQueryCreateInfoINTEL::safe_VkQueryPoolPerformanceQueryCreateInfoINTEL(
    const safe_VkQueryPoolPerformanceQueryCreateInfoINTEL& copy_src) {
    sType = copy_src.sType;
    performanceCountersSampling = copy_src.performanceCountersSampling;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkQueryPoolPerformanceQueryCreateInfoINTEL& safe_VkQueryPoolPerformanceQueryCreateInfoINTEL::operator=(
    const safe_VkQueryPoolPerformanceQueryCreateInfoINTEL& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    performanceCountersSampling = copy_src.performanceCountersSampling;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkQueryPoolPerformanceQueryCreateInfoINTEL::~safe_VkQueryPoolPerformanceQueryCreateInfoINTEL() { FreePnextChain(pNext); }

void safe_VkQueryPoolPerformanceQueryCreateInfoINTEL::initialize(const VkQueryPoolPerformanceQueryCreateInfoINTEL* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    performanceCountersSampling = in_struct->performanceCountersSampling;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkQueryPoolPerformanceQueryCreateInfoINTEL::initialize(const safe_VkQueryPoolPerformanceQueryCreateInfoINTEL* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    performanceCountersSampling = copy_src->performanceCountersSampling;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPerformanceMarkerInfoINTEL::safe_VkPerformanceMarkerInfoINTEL(const VkPerformanceMarkerInfoINTEL* in_struct,
                                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), marker(in_struct->marker) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPerformanceMarkerInfoINTEL::safe_VkPerformanceMarkerInfoINTEL()
    : sType(VK_STRUCTURE_TYPE_PERFORMANCE_MARKER_INFO_INTEL), pNext(nullptr), marker() {}

safe_VkPerformanceMarkerInfoINTEL::safe_VkPerformanceMarkerInfoINTEL(const safe_VkPerformanceMarkerInfoINTEL& copy_src) {
    sType = copy_src.sType;
    marker = copy_src.marker;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPerformanceMarkerInfoINTEL& safe_VkPerformanceMarkerInfoINTEL::operator=(const safe_VkPerformanceMarkerInfoINTEL& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    marker = copy_src.marker;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPerformanceMarkerInfoINTEL::~safe_VkPerformanceMarkerInfoINTEL() { FreePnextChain(pNext); }

void safe_VkPerformanceMarkerInfoINTEL::initialize(const VkPerformanceMarkerInfoINTEL* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    marker = in_struct->marker;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPerformanceMarkerInfoINTEL::initialize(const safe_VkPerformanceMarkerInfoINTEL* copy_src,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    marker = copy_src->marker;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPerformanceStreamMarkerInfoINTEL::safe_VkPerformanceStreamMarkerInfoINTEL(
    const VkPerformanceStreamMarkerInfoINTEL* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), marker(in_struct->marker) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPerformanceStreamMarkerInfoINTEL::safe_VkPerformanceStreamMarkerInfoINTEL()
    : sType(VK_STRUCTURE_TYPE_PERFORMANCE_STREAM_MARKER_INFO_INTEL), pNext(nullptr), marker() {}

safe_VkPerformanceStreamMarkerInfoINTEL::safe_VkPerformanceStreamMarkerInfoINTEL(
    const safe_VkPerformanceStreamMarkerInfoINTEL& copy_src) {
    sType = copy_src.sType;
    marker = copy_src.marker;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPerformanceStreamMarkerInfoINTEL& safe_VkPerformanceStreamMarkerInfoINTEL::operator=(
    const safe_VkPerformanceStreamMarkerInfoINTEL& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    marker = copy_src.marker;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPerformanceStreamMarkerInfoINTEL::~safe_VkPerformanceStreamMarkerInfoINTEL() { FreePnextChain(pNext); }

void safe_VkPerformanceStreamMarkerInfoINTEL::initialize(const VkPerformanceStreamMarkerInfoINTEL* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    marker = in_struct->marker;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPerformanceStreamMarkerInfoINTEL::initialize(const safe_VkPerformanceStreamMarkerInfoINTEL* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    marker = copy_src->marker;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPerformanceOverrideInfoINTEL::safe_VkPerformanceOverrideInfoINTEL(const VkPerformanceOverrideInfoINTEL* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), type(in_struct->type), enable(in_struct->enable), parameter(in_struct->parameter) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPerformanceOverrideInfoINTEL::safe_VkPerformanceOverrideInfoINTEL()
    : sType(VK_STRUCTURE_TYPE_PERFORMANCE_OVERRIDE_INFO_INTEL), pNext(nullptr), type(), enable(), parameter() {}

safe_VkPerformanceOverrideInfoINTEL::safe_VkPerformanceOverrideInfoINTEL(const safe_VkPerformanceOverrideInfoINTEL& copy_src) {
    sType = copy_src.sType;
    type = copy_src.type;
    enable = copy_src.enable;
    parameter = copy_src.parameter;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPerformanceOverrideInfoINTEL& safe_VkPerformanceOverrideInfoINTEL::operator=(
    const safe_VkPerformanceOverrideInfoINTEL& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    type = copy_src.type;
    enable = copy_src.enable;
    parameter = copy_src.parameter;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPerformanceOverrideInfoINTEL::~safe_VkPerformanceOverrideInfoINTEL() { FreePnextChain(pNext); }

void safe_VkPerformanceOverrideInfoINTEL::initialize(const VkPerformanceOverrideInfoINTEL* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    type = in_struct->type;
    enable = in_struct->enable;
    parameter = in_struct->parameter;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPerformanceOverrideInfoINTEL::initialize(const safe_VkPerformanceOverrideInfoINTEL* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    type = copy_src->type;
    enable = copy_src->enable;
    parameter = copy_src->parameter;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPerformanceConfigurationAcquireInfoINTEL::safe_VkPerformanceConfigurationAcquireInfoINTEL(
    const VkPerformanceConfigurationAcquireInfoINTEL* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), type(in_struct->type) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPerformanceConfigurationAcquireInfoINTEL::safe_VkPerformanceConfigurationAcquireInfoINTEL()
    : sType(VK_STRUCTURE_TYPE_PERFORMANCE_CONFIGURATION_ACQUIRE_INFO_INTEL), pNext(nullptr), type() {}

safe_VkPerformanceConfigurationAcquireInfoINTEL::safe_VkPerformanceConfigurationAcquireInfoINTEL(
    const safe_VkPerformanceConfigurationAcquireInfoINTEL& copy_src) {
    sType = copy_src.sType;
    type = copy_src.type;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPerformanceConfigurationAcquireInfoINTEL& safe_VkPerformanceConfigurationAcquireInfoINTEL::operator=(
    const safe_VkPerformanceConfigurationAcquireInfoINTEL& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    type = copy_src.type;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPerformanceConfigurationAcquireInfoINTEL::~safe_VkPerformanceConfigurationAcquireInfoINTEL() { FreePnextChain(pNext); }

void safe_VkPerformanceConfigurationAcquireInfoINTEL::initialize(const VkPerformanceConfigurationAcquireInfoINTEL* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    type = in_struct->type;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPerformanceConfigurationAcquireInfoINTEL::initialize(const safe_VkPerformanceConfigurationAcquireInfoINTEL* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    type = copy_src->type;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD::safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD(
    const VkDisplayNativeHdrSurfaceCapabilitiesAMD* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), localDimmingSupport(in_struct->localDimmingSupport) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD::safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_NATIVE_HDR_SURFACE_CAPABILITIES_AMD), pNext(nullptr), localDimmingSupport() {}

safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD::safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD(
    const safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD& copy_src) {
    sType = copy_src.sType;
    localDimmingSupport = copy_src.localDimmingSupport;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD& safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD::operator=(
    const safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    localDimmingSupport = copy_src.localDimmingSupport;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD::~safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD() { FreePnextChain(pNext); }

void safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD::initialize(const VkDisplayNativeHdrSurfaceCapabilitiesAMD* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    localDimmingSupport = in_struct->localDimmingSupport;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD::initialize(const safe_VkDisplayNativeHdrSurfaceCapabilitiesAMD* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    localDimmingSupport = copy_src->localDimmingSupport;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSwapchainDisplayNativeHdrCreateInfoAMD::safe_VkSwapchainDisplayNativeHdrCreateInfoAMD(
    const VkSwapchainDisplayNativeHdrCreateInfoAMD* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), localDimmingEnable(in_struct->localDimmingEnable) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSwapchainDisplayNativeHdrCreateInfoAMD::safe_VkSwapchainDisplayNativeHdrCreateInfoAMD()
    : sType(VK_STRUCTURE_TYPE_SWAPCHAIN_DISPLAY_NATIVE_HDR_CREATE_INFO_AMD), pNext(nullptr), localDimmingEnable() {}

safe_VkSwapchainDisplayNativeHdrCreateInfoAMD::safe_VkSwapchainDisplayNativeHdrCreateInfoAMD(
    const safe_VkSwapchainDisplayNativeHdrCreateInfoAMD& copy_src) {
    sType = copy_src.sType;
    localDimmingEnable = copy_src.localDimmingEnable;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSwapchainDisplayNativeHdrCreateInfoAMD& safe_VkSwapchainDisplayNativeHdrCreateInfoAMD::operator=(
    const safe_VkSwapchainDisplayNativeHdrCreateInfoAMD& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    localDimmingEnable = copy_src.localDimmingEnable;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSwapchainDisplayNativeHdrCreateInfoAMD::~safe_VkSwapchainDisplayNativeHdrCreateInfoAMD() { FreePnextChain(pNext); }

void safe_VkSwapchainDisplayNativeHdrCreateInfoAMD::initialize(const VkSwapchainDisplayNativeHdrCreateInfoAMD* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    localDimmingEnable = in_struct->localDimmingEnable;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSwapchainDisplayNativeHdrCreateInfoAMD::initialize(const safe_VkSwapchainDisplayNativeHdrCreateInfoAMD* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    localDimmingEnable = copy_src->localDimmingEnable;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_USE_PLATFORM_FUCHSIA

safe_VkImagePipeSurfaceCreateInfoFUCHSIA::safe_VkImagePipeSurfaceCreateInfoFUCHSIA(
    const VkImagePipeSurfaceCreateInfoFUCHSIA* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags), imagePipeHandle(in_struct->imagePipeHandle) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImagePipeSurfaceCreateInfoFUCHSIA::safe_VkImagePipeSurfaceCreateInfoFUCHSIA()
    : sType(VK_STRUCTURE_TYPE_IMAGEPIPE_SURFACE_CREATE_INFO_FUCHSIA), pNext(nullptr), flags(), imagePipeHandle() {}

safe_VkImagePipeSurfaceCreateInfoFUCHSIA::safe_VkImagePipeSurfaceCreateInfoFUCHSIA(
    const safe_VkImagePipeSurfaceCreateInfoFUCHSIA& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    imagePipeHandle = copy_src.imagePipeHandle;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImagePipeSurfaceCreateInfoFUCHSIA& safe_VkImagePipeSurfaceCreateInfoFUCHSIA::operator=(
    const safe_VkImagePipeSurfaceCreateInfoFUCHSIA& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    imagePipeHandle = copy_src.imagePipeHandle;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImagePipeSurfaceCreateInfoFUCHSIA::~safe_VkImagePipeSurfaceCreateInfoFUCHSIA() { FreePnextChain(pNext); }

void safe_VkImagePipeSurfaceCreateInfoFUCHSIA::initialize(const VkImagePipeSurfaceCreateInfoFUCHSIA* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    imagePipeHandle = in_struct->imagePipeHandle;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImagePipeSurfaceCreateInfoFUCHSIA::initialize(const safe_VkImagePipeSurfaceCreateInfoFUCHSIA* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    imagePipeHandle = copy_src->imagePipeHandle;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_FUCHSIA

safe_VkPhysicalDeviceShaderCoreProperties2AMD::safe_VkPhysicalDeviceShaderCoreProperties2AMD(
    const VkPhysicalDeviceShaderCoreProperties2AMD* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      shaderCoreFeatures(in_struct->shaderCoreFeatures),
      activeComputeUnitCount(in_struct->activeComputeUnitCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderCoreProperties2AMD::safe_VkPhysicalDeviceShaderCoreProperties2AMD()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_2_AMD),
      pNext(nullptr),
      shaderCoreFeatures(),
      activeComputeUnitCount() {}

safe_VkPhysicalDeviceShaderCoreProperties2AMD::safe_VkPhysicalDeviceShaderCoreProperties2AMD(
    const safe_VkPhysicalDeviceShaderCoreProperties2AMD& copy_src) {
    sType = copy_src.sType;
    shaderCoreFeatures = copy_src.shaderCoreFeatures;
    activeComputeUnitCount = copy_src.activeComputeUnitCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderCoreProperties2AMD& safe_VkPhysicalDeviceShaderCoreProperties2AMD::operator=(
    const safe_VkPhysicalDeviceShaderCoreProperties2AMD& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderCoreFeatures = copy_src.shaderCoreFeatures;
    activeComputeUnitCount = copy_src.activeComputeUnitCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderCoreProperties2AMD::~safe_VkPhysicalDeviceShaderCoreProperties2AMD() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderCoreProperties2AMD::initialize(const VkPhysicalDeviceShaderCoreProperties2AMD* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderCoreFeatures = in_struct->shaderCoreFeatures;
    activeComputeUnitCount = in_struct->activeComputeUnitCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderCoreProperties2AMD::initialize(const safe_VkPhysicalDeviceShaderCoreProperties2AMD* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderCoreFeatures = copy_src->shaderCoreFeatures;
    activeComputeUnitCount = copy_src->activeComputeUnitCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD::safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD(
    const VkPhysicalDeviceCoherentMemoryFeaturesAMD* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), deviceCoherentMemory(in_struct->deviceCoherentMemory) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD::safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COHERENT_MEMORY_FEATURES_AMD), pNext(nullptr), deviceCoherentMemory() {}

safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD::safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD(
    const safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD& copy_src) {
    sType = copy_src.sType;
    deviceCoherentMemory = copy_src.deviceCoherentMemory;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD& safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD::operator=(
    const safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    deviceCoherentMemory = copy_src.deviceCoherentMemory;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD::~safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD::initialize(const VkPhysicalDeviceCoherentMemoryFeaturesAMD* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    deviceCoherentMemory = in_struct->deviceCoherentMemory;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD::initialize(const safe_VkPhysicalDeviceCoherentMemoryFeaturesAMD* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    deviceCoherentMemory = copy_src->deviceCoherentMemory;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV::safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV(
    const VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), dedicatedAllocationImageAliasing(in_struct->dedicatedAllocationImageAliasing) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV::safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEDICATED_ALLOCATION_IMAGE_ALIASING_FEATURES_NV),
      pNext(nullptr),
      dedicatedAllocationImageAliasing() {}

safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV::safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV(
    const safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV& copy_src) {
    sType = copy_src.sType;
    dedicatedAllocationImageAliasing = copy_src.dedicatedAllocationImageAliasing;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV&
safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV::operator=(
    const safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    dedicatedAllocationImageAliasing = copy_src.dedicatedAllocationImageAliasing;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV::
    ~safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV::initialize(
    const VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    dedicatedAllocationImageAliasing = in_struct->dedicatedAllocationImageAliasing;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV::initialize(
    const safe_VkPhysicalDeviceDedicatedAllocationImageAliasingFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    dedicatedAllocationImageAliasing = copy_src->dedicatedAllocationImageAliasing;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCooperativeMatrixPropertiesNV::safe_VkCooperativeMatrixPropertiesNV(const VkCooperativeMatrixPropertiesNV* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType),
      MSize(in_struct->MSize),
      NSize(in_struct->NSize),
      KSize(in_struct->KSize),
      AType(in_struct->AType),
      BType(in_struct->BType),
      CType(in_struct->CType),
      DType(in_struct->DType),
      scope(in_struct->scope) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCooperativeMatrixPropertiesNV::safe_VkCooperativeMatrixPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_PROPERTIES_NV),
      pNext(nullptr),
      MSize(),
      NSize(),
      KSize(),
      AType(),
      BType(),
      CType(),
      DType(),
      scope() {}

safe_VkCooperativeMatrixPropertiesNV::safe_VkCooperativeMatrixPropertiesNV(const safe_VkCooperativeMatrixPropertiesNV& copy_src) {
    sType = copy_src.sType;
    MSize = copy_src.MSize;
    NSize = copy_src.NSize;
    KSize = copy_src.KSize;
    AType = copy_src.AType;
    BType = copy_src.BType;
    CType = copy_src.CType;
    DType = copy_src.DType;
    scope = copy_src.scope;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCooperativeMatrixPropertiesNV& safe_VkCooperativeMatrixPropertiesNV::operator=(
    const safe_VkCooperativeMatrixPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    MSize = copy_src.MSize;
    NSize = copy_src.NSize;
    KSize = copy_src.KSize;
    AType = copy_src.AType;
    BType = copy_src.BType;
    CType = copy_src.CType;
    DType = copy_src.DType;
    scope = copy_src.scope;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCooperativeMatrixPropertiesNV::~safe_VkCooperativeMatrixPropertiesNV() { FreePnextChain(pNext); }

void safe_VkCooperativeMatrixPropertiesNV::initialize(const VkCooperativeMatrixPropertiesNV* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    MSize = in_struct->MSize;
    NSize = in_struct->NSize;
    KSize = in_struct->KSize;
    AType = in_struct->AType;
    BType = in_struct->BType;
    CType = in_struct->CType;
    DType = in_struct->DType;
    scope = in_struct->scope;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCooperativeMatrixPropertiesNV::initialize(const safe_VkCooperativeMatrixPropertiesNV* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    MSize = copy_src->MSize;
    NSize = copy_src->NSize;
    KSize = copy_src->KSize;
    AType = copy_src->AType;
    BType = copy_src->BType;
    CType = copy_src->CType;
    DType = copy_src->DType;
    scope = copy_src->scope;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV::safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV(
    const VkPhysicalDeviceCooperativeMatrixFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      cooperativeMatrix(in_struct->cooperativeMatrix),
      cooperativeMatrixRobustBufferAccess(in_struct->cooperativeMatrixRobustBufferAccess) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV::safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_FEATURES_NV),
      pNext(nullptr),
      cooperativeMatrix(),
      cooperativeMatrixRobustBufferAccess() {}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV::safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV(
    const safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV& copy_src) {
    sType = copy_src.sType;
    cooperativeMatrix = copy_src.cooperativeMatrix;
    cooperativeMatrixRobustBufferAccess = copy_src.cooperativeMatrixRobustBufferAccess;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV& safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV::operator=(
    const safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    cooperativeMatrix = copy_src.cooperativeMatrix;
    cooperativeMatrixRobustBufferAccess = copy_src.cooperativeMatrixRobustBufferAccess;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV::~safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV::initialize(const VkPhysicalDeviceCooperativeMatrixFeaturesNV* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    cooperativeMatrix = in_struct->cooperativeMatrix;
    cooperativeMatrixRobustBufferAccess = in_struct->cooperativeMatrixRobustBufferAccess;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV::initialize(const safe_VkPhysicalDeviceCooperativeMatrixFeaturesNV* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    cooperativeMatrix = copy_src->cooperativeMatrix;
    cooperativeMatrixRobustBufferAccess = copy_src->cooperativeMatrixRobustBufferAccess;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV::safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV(
    const VkPhysicalDeviceCooperativeMatrixPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), cooperativeMatrixSupportedStages(in_struct->cooperativeMatrixSupportedStages) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV::safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_PROPERTIES_NV),
      pNext(nullptr),
      cooperativeMatrixSupportedStages() {}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV::safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV(
    const safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV& copy_src) {
    sType = copy_src.sType;
    cooperativeMatrixSupportedStages = copy_src.cooperativeMatrixSupportedStages;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV& safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV::operator=(
    const safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    cooperativeMatrixSupportedStages = copy_src.cooperativeMatrixSupportedStages;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV::~safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV::initialize(const VkPhysicalDeviceCooperativeMatrixPropertiesNV* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    cooperativeMatrixSupportedStages = in_struct->cooperativeMatrixSupportedStages;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV::initialize(
    const safe_VkPhysicalDeviceCooperativeMatrixPropertiesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    cooperativeMatrixSupportedStages = copy_src->cooperativeMatrixSupportedStages;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV::safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV(
    const VkPhysicalDeviceCoverageReductionModeFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), coverageReductionMode(in_struct->coverageReductionMode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV::safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COVERAGE_REDUCTION_MODE_FEATURES_NV), pNext(nullptr), coverageReductionMode() {}

safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV::safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV(
    const safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV& copy_src) {
    sType = copy_src.sType;
    coverageReductionMode = copy_src.coverageReductionMode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV& safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV::operator=(
    const safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    coverageReductionMode = copy_src.coverageReductionMode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV::~safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV::initialize(
    const VkPhysicalDeviceCoverageReductionModeFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    coverageReductionMode = in_struct->coverageReductionMode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV::initialize(
    const safe_VkPhysicalDeviceCoverageReductionModeFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    coverageReductionMode = copy_src->coverageReductionMode;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineCoverageReductionStateCreateInfoNV::safe_VkPipelineCoverageReductionStateCreateInfoNV(
    const VkPipelineCoverageReductionStateCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags), coverageReductionMode(in_struct->coverageReductionMode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineCoverageReductionStateCreateInfoNV::safe_VkPipelineCoverageReductionStateCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_COVERAGE_REDUCTION_STATE_CREATE_INFO_NV), pNext(nullptr), flags(), coverageReductionMode() {}

safe_VkPipelineCoverageReductionStateCreateInfoNV::safe_VkPipelineCoverageReductionStateCreateInfoNV(
    const safe_VkPipelineCoverageReductionStateCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    coverageReductionMode = copy_src.coverageReductionMode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineCoverageReductionStateCreateInfoNV& safe_VkPipelineCoverageReductionStateCreateInfoNV::operator=(
    const safe_VkPipelineCoverageReductionStateCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    coverageReductionMode = copy_src.coverageReductionMode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineCoverageReductionStateCreateInfoNV::~safe_VkPipelineCoverageReductionStateCreateInfoNV() { FreePnextChain(pNext); }

void safe_VkPipelineCoverageReductionStateCreateInfoNV::initialize(const VkPipelineCoverageReductionStateCreateInfoNV* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    coverageReductionMode = in_struct->coverageReductionMode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineCoverageReductionStateCreateInfoNV::initialize(
    const safe_VkPipelineCoverageReductionStateCreateInfoNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    coverageReductionMode = copy_src->coverageReductionMode;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkFramebufferMixedSamplesCombinationNV::safe_VkFramebufferMixedSamplesCombinationNV(
    const VkFramebufferMixedSamplesCombinationNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      coverageReductionMode(in_struct->coverageReductionMode),
      rasterizationSamples(in_struct->rasterizationSamples),
      depthStencilSamples(in_struct->depthStencilSamples),
      colorSamples(in_struct->colorSamples) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkFramebufferMixedSamplesCombinationNV::safe_VkFramebufferMixedSamplesCombinationNV()
    : sType(VK_STRUCTURE_TYPE_FRAMEBUFFER_MIXED_SAMPLES_COMBINATION_NV),
      pNext(nullptr),
      coverageReductionMode(),
      rasterizationSamples(),
      depthStencilSamples(),
      colorSamples() {}

safe_VkFramebufferMixedSamplesCombinationNV::safe_VkFramebufferMixedSamplesCombinationNV(
    const safe_VkFramebufferMixedSamplesCombinationNV& copy_src) {
    sType = copy_src.sType;
    coverageReductionMode = copy_src.coverageReductionMode;
    rasterizationSamples = copy_src.rasterizationSamples;
    depthStencilSamples = copy_src.depthStencilSamples;
    colorSamples = copy_src.colorSamples;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkFramebufferMixedSamplesCombinationNV& safe_VkFramebufferMixedSamplesCombinationNV::operator=(
    const safe_VkFramebufferMixedSamplesCombinationNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    coverageReductionMode = copy_src.coverageReductionMode;
    rasterizationSamples = copy_src.rasterizationSamples;
    depthStencilSamples = copy_src.depthStencilSamples;
    colorSamples = copy_src.colorSamples;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkFramebufferMixedSamplesCombinationNV::~safe_VkFramebufferMixedSamplesCombinationNV() { FreePnextChain(pNext); }

void safe_VkFramebufferMixedSamplesCombinationNV::initialize(const VkFramebufferMixedSamplesCombinationNV* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    coverageReductionMode = in_struct->coverageReductionMode;
    rasterizationSamples = in_struct->rasterizationSamples;
    depthStencilSamples = in_struct->depthStencilSamples;
    colorSamples = in_struct->colorSamples;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkFramebufferMixedSamplesCombinationNV::initialize(const safe_VkFramebufferMixedSamplesCombinationNV* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    coverageReductionMode = copy_src->coverageReductionMode;
    rasterizationSamples = copy_src->rasterizationSamples;
    depthStencilSamples = copy_src->depthStencilSamples;
    colorSamples = copy_src->colorSamples;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV::safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV(
    const VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      maxGraphicsShaderGroupCount(in_struct->maxGraphicsShaderGroupCount),
      maxIndirectSequenceCount(in_struct->maxIndirectSequenceCount),
      maxIndirectCommandsTokenCount(in_struct->maxIndirectCommandsTokenCount),
      maxIndirectCommandsStreamCount(in_struct->maxIndirectCommandsStreamCount),
      maxIndirectCommandsTokenOffset(in_struct->maxIndirectCommandsTokenOffset),
      maxIndirectCommandsStreamStride(in_struct->maxIndirectCommandsStreamStride),
      minSequencesCountBufferOffsetAlignment(in_struct->minSequencesCountBufferOffsetAlignment),
      minSequencesIndexBufferOffsetAlignment(in_struct->minSequencesIndexBufferOffsetAlignment),
      minIndirectCommandsBufferOffsetAlignment(in_struct->minIndirectCommandsBufferOffsetAlignment) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV::safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_PROPERTIES_NV),
      pNext(nullptr),
      maxGraphicsShaderGroupCount(),
      maxIndirectSequenceCount(),
      maxIndirectCommandsTokenCount(),
      maxIndirectCommandsStreamCount(),
      maxIndirectCommandsTokenOffset(),
      maxIndirectCommandsStreamStride(),
      minSequencesCountBufferOffsetAlignment(),
      minSequencesIndexBufferOffsetAlignment(),
      minIndirectCommandsBufferOffsetAlignment() {}

safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV::safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV(
    const safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV& copy_src) {
    sType = copy_src.sType;
    maxGraphicsShaderGroupCount = copy_src.maxGraphicsShaderGroupCount;
    maxIndirectSequenceCount = copy_src.maxIndirectSequenceCount;
    maxIndirectCommandsTokenCount = copy_src.maxIndirectCommandsTokenCount;
    maxIndirectCommandsStreamCount = copy_src.maxIndirectCommandsStreamCount;
    maxIndirectCommandsTokenOffset = copy_src.maxIndirectCommandsTokenOffset;
    maxIndirectCommandsStreamStride = copy_src.maxIndirectCommandsStreamStride;
    minSequencesCountBufferOffsetAlignment = copy_src.minSequencesCountBufferOffsetAlignment;
    minSequencesIndexBufferOffsetAlignment = copy_src.minSequencesIndexBufferOffsetAlignment;
    minIndirectCommandsBufferOffsetAlignment = copy_src.minIndirectCommandsBufferOffsetAlignment;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV& safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV::operator=(
    const safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxGraphicsShaderGroupCount = copy_src.maxGraphicsShaderGroupCount;
    maxIndirectSequenceCount = copy_src.maxIndirectSequenceCount;
    maxIndirectCommandsTokenCount = copy_src.maxIndirectCommandsTokenCount;
    maxIndirectCommandsStreamCount = copy_src.maxIndirectCommandsStreamCount;
    maxIndirectCommandsTokenOffset = copy_src.maxIndirectCommandsTokenOffset;
    maxIndirectCommandsStreamStride = copy_src.maxIndirectCommandsStreamStride;
    minSequencesCountBufferOffsetAlignment = copy_src.minSequencesCountBufferOffsetAlignment;
    minSequencesIndexBufferOffsetAlignment = copy_src.minSequencesIndexBufferOffsetAlignment;
    minIndirectCommandsBufferOffsetAlignment = copy_src.minIndirectCommandsBufferOffsetAlignment;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV::~safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV::initialize(
    const VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxGraphicsShaderGroupCount = in_struct->maxGraphicsShaderGroupCount;
    maxIndirectSequenceCount = in_struct->maxIndirectSequenceCount;
    maxIndirectCommandsTokenCount = in_struct->maxIndirectCommandsTokenCount;
    maxIndirectCommandsStreamCount = in_struct->maxIndirectCommandsStreamCount;
    maxIndirectCommandsTokenOffset = in_struct->maxIndirectCommandsTokenOffset;
    maxIndirectCommandsStreamStride = in_struct->maxIndirectCommandsStreamStride;
    minSequencesCountBufferOffsetAlignment = in_struct->minSequencesCountBufferOffsetAlignment;
    minSequencesIndexBufferOffsetAlignment = in_struct->minSequencesIndexBufferOffsetAlignment;
    minIndirectCommandsBufferOffsetAlignment = in_struct->minIndirectCommandsBufferOffsetAlignment;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV::initialize(
    const safe_VkPhysicalDeviceDeviceGeneratedCommandsPropertiesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxGraphicsShaderGroupCount = copy_src->maxGraphicsShaderGroupCount;
    maxIndirectSequenceCount = copy_src->maxIndirectSequenceCount;
    maxIndirectCommandsTokenCount = copy_src->maxIndirectCommandsTokenCount;
    maxIndirectCommandsStreamCount = copy_src->maxIndirectCommandsStreamCount;
    maxIndirectCommandsTokenOffset = copy_src->maxIndirectCommandsTokenOffset;
    maxIndirectCommandsStreamStride = copy_src->maxIndirectCommandsStreamStride;
    minSequencesCountBufferOffsetAlignment = copy_src->minSequencesCountBufferOffsetAlignment;
    minSequencesIndexBufferOffsetAlignment = copy_src->minSequencesIndexBufferOffsetAlignment;
    minIndirectCommandsBufferOffsetAlignment = copy_src->minIndirectCommandsBufferOffsetAlignment;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV::safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV(
    const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), deviceGeneratedCommands(in_struct->deviceGeneratedCommands) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV::safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_FEATURES_NV), pNext(nullptr), deviceGeneratedCommands() {}

safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV::safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV(
    const safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV& copy_src) {
    sType = copy_src.sType;
    deviceGeneratedCommands = copy_src.deviceGeneratedCommands;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV& safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV::operator=(
    const safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    deviceGeneratedCommands = copy_src.deviceGeneratedCommands;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV::~safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV::initialize(
    const VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    deviceGeneratedCommands = in_struct->deviceGeneratedCommands;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV::initialize(
    const safe_VkPhysicalDeviceDeviceGeneratedCommandsFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    deviceGeneratedCommands = copy_src->deviceGeneratedCommands;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkGraphicsShaderGroupCreateInfoNV::safe_VkGraphicsShaderGroupCreateInfoNV(const VkGraphicsShaderGroupCreateInfoNV* in_struct,
                                                                               [[maybe_unused]] PNextCopyState* copy_state,
                                                                               bool copy_pnext)
    : sType(in_struct->sType),
      stageCount(in_struct->stageCount),
      pStages(nullptr),
      pVertexInputState(nullptr),
      pTessellationState(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (stageCount && in_struct->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&in_struct->pStages[i]);
        }
    }
    if (in_struct->pVertexInputState)
        pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(in_struct->pVertexInputState);
    if (in_struct->pTessellationState)
        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(in_struct->pTessellationState);
}

safe_VkGraphicsShaderGroupCreateInfoNV::safe_VkGraphicsShaderGroupCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_GRAPHICS_SHADER_GROUP_CREATE_INFO_NV),
      pNext(nullptr),
      stageCount(),
      pStages(nullptr),
      pVertexInputState(nullptr),
      pTessellationState(nullptr) {}

safe_VkGraphicsShaderGroupCreateInfoNV::safe_VkGraphicsShaderGroupCreateInfoNV(
    const safe_VkGraphicsShaderGroupCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    stageCount = copy_src.stageCount;
    pStages = nullptr;
    pVertexInputState = nullptr;
    pTessellationState = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (stageCount && copy_src.pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&copy_src.pStages[i]);
        }
    }
    if (copy_src.pVertexInputState) pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(*copy_src.pVertexInputState);
    if (copy_src.pTessellationState)
        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(*copy_src.pTessellationState);
}

safe_VkGraphicsShaderGroupCreateInfoNV& safe_VkGraphicsShaderGroupCreateInfoNV::operator=(
    const safe_VkGraphicsShaderGroupCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pStages) delete[] pStages;
    if (pVertexInputState) delete pVertexInputState;
    if (pTessellationState) delete pTessellationState;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    stageCount = copy_src.stageCount;
    pStages = nullptr;
    pVertexInputState = nullptr;
    pTessellationState = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (stageCount && copy_src.pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&copy_src.pStages[i]);
        }
    }
    if (copy_src.pVertexInputState) pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(*copy_src.pVertexInputState);
    if (copy_src.pTessellationState)
        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(*copy_src.pTessellationState);

    return *this;
}

safe_VkGraphicsShaderGroupCreateInfoNV::~safe_VkGraphicsShaderGroupCreateInfoNV() {
    if (pStages) delete[] pStages;
    if (pVertexInputState) delete pVertexInputState;
    if (pTessellationState) delete pTessellationState;
    FreePnextChain(pNext);
}

void safe_VkGraphicsShaderGroupCreateInfoNV::initialize(const VkGraphicsShaderGroupCreateInfoNV* in_struct,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStages) delete[] pStages;
    if (pVertexInputState) delete pVertexInputState;
    if (pTessellationState) delete pTessellationState;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stageCount = in_struct->stageCount;
    pStages = nullptr;
    pVertexInputState = nullptr;
    pTessellationState = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (stageCount && in_struct->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&in_struct->pStages[i]);
        }
    }
    if (in_struct->pVertexInputState)
        pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(in_struct->pVertexInputState);
    if (in_struct->pTessellationState)
        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(in_struct->pTessellationState);
}

void safe_VkGraphicsShaderGroupCreateInfoNV::initialize(const safe_VkGraphicsShaderGroupCreateInfoNV* copy_src,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stageCount = copy_src->stageCount;
    pStages = nullptr;
    pVertexInputState = nullptr;
    pTessellationState = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (stageCount && copy_src->pStages) {
        pStages = new safe_VkPipelineShaderStageCreateInfo[stageCount];
        for (uint32_t i = 0; i < stageCount; ++i) {
            pStages[i].initialize(&copy_src->pStages[i]);
        }
    }
    if (copy_src->pVertexInputState)
        pVertexInputState = new safe_VkPipelineVertexInputStateCreateInfo(*copy_src->pVertexInputState);
    if (copy_src->pTessellationState)
        pTessellationState = new safe_VkPipelineTessellationStateCreateInfo(*copy_src->pTessellationState);
}

safe_VkGraphicsPipelineShaderGroupsCreateInfoNV::safe_VkGraphicsPipelineShaderGroupsCreateInfoNV(
    const VkGraphicsPipelineShaderGroupsCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      groupCount(in_struct->groupCount),
      pGroups(nullptr),
      pipelineCount(in_struct->pipelineCount),
      pPipelines(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (groupCount && in_struct->pGroups) {
        pGroups = new safe_VkGraphicsShaderGroupCreateInfoNV[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&in_struct->pGroups[i]);
        }
    }
    if (pipelineCount && in_struct->pPipelines) {
        pPipelines = new VkPipeline[pipelineCount];
        for (uint32_t i = 0; i < pipelineCount; ++i) {
            pPipelines[i] = in_struct->pPipelines[i];
        }
    }
}

safe_VkGraphicsPipelineShaderGroupsCreateInfoNV::safe_VkGraphicsPipelineShaderGroupsCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_SHADER_GROUPS_CREATE_INFO_NV),
      pNext(nullptr),
      groupCount(),
      pGroups(nullptr),
      pipelineCount(),
      pPipelines(nullptr) {}

safe_VkGraphicsPipelineShaderGroupsCreateInfoNV::safe_VkGraphicsPipelineShaderGroupsCreateInfoNV(
    const safe_VkGraphicsPipelineShaderGroupsCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    groupCount = copy_src.groupCount;
    pGroups = nullptr;
    pipelineCount = copy_src.pipelineCount;
    pPipelines = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (groupCount && copy_src.pGroups) {
        pGroups = new safe_VkGraphicsShaderGroupCreateInfoNV[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&copy_src.pGroups[i]);
        }
    }
    if (pipelineCount && copy_src.pPipelines) {
        pPipelines = new VkPipeline[pipelineCount];
        for (uint32_t i = 0; i < pipelineCount; ++i) {
            pPipelines[i] = copy_src.pPipelines[i];
        }
    }
}

safe_VkGraphicsPipelineShaderGroupsCreateInfoNV& safe_VkGraphicsPipelineShaderGroupsCreateInfoNV::operator=(
    const safe_VkGraphicsPipelineShaderGroupsCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pGroups) delete[] pGroups;
    if (pPipelines) delete[] pPipelines;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    groupCount = copy_src.groupCount;
    pGroups = nullptr;
    pipelineCount = copy_src.pipelineCount;
    pPipelines = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (groupCount && copy_src.pGroups) {
        pGroups = new safe_VkGraphicsShaderGroupCreateInfoNV[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&copy_src.pGroups[i]);
        }
    }
    if (pipelineCount && copy_src.pPipelines) {
        pPipelines = new VkPipeline[pipelineCount];
        for (uint32_t i = 0; i < pipelineCount; ++i) {
            pPipelines[i] = copy_src.pPipelines[i];
        }
    }

    return *this;
}

safe_VkGraphicsPipelineShaderGroupsCreateInfoNV::~safe_VkGraphicsPipelineShaderGroupsCreateInfoNV() {
    if (pGroups) delete[] pGroups;
    if (pPipelines) delete[] pPipelines;
    FreePnextChain(pNext);
}

void safe_VkGraphicsPipelineShaderGroupsCreateInfoNV::initialize(const VkGraphicsPipelineShaderGroupsCreateInfoNV* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    if (pGroups) delete[] pGroups;
    if (pPipelines) delete[] pPipelines;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    groupCount = in_struct->groupCount;
    pGroups = nullptr;
    pipelineCount = in_struct->pipelineCount;
    pPipelines = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (groupCount && in_struct->pGroups) {
        pGroups = new safe_VkGraphicsShaderGroupCreateInfoNV[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&in_struct->pGroups[i]);
        }
    }
    if (pipelineCount && in_struct->pPipelines) {
        pPipelines = new VkPipeline[pipelineCount];
        for (uint32_t i = 0; i < pipelineCount; ++i) {
            pPipelines[i] = in_struct->pPipelines[i];
        }
    }
}

void safe_VkGraphicsPipelineShaderGroupsCreateInfoNV::initialize(const safe_VkGraphicsPipelineShaderGroupsCreateInfoNV* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    groupCount = copy_src->groupCount;
    pGroups = nullptr;
    pipelineCount = copy_src->pipelineCount;
    pPipelines = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (groupCount && copy_src->pGroups) {
        pGroups = new safe_VkGraphicsShaderGroupCreateInfoNV[groupCount];
        for (uint32_t i = 0; i < groupCount; ++i) {
            pGroups[i].initialize(&copy_src->pGroups[i]);
        }
    }
    if (pipelineCount && copy_src->pPipelines) {
        pPipelines = new VkPipeline[pipelineCount];
        for (uint32_t i = 0; i < pipelineCount; ++i) {
            pPipelines[i] = copy_src->pPipelines[i];
        }
    }
}

safe_VkIndirectCommandsLayoutTokenNV::safe_VkIndirectCommandsLayoutTokenNV(const VkIndirectCommandsLayoutTokenNV* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType),
      tokenType(in_struct->tokenType),
      stream(in_struct->stream),
      offset(in_struct->offset),
      vertexBindingUnit(in_struct->vertexBindingUnit),
      vertexDynamicStride(in_struct->vertexDynamicStride),
      pushconstantPipelineLayout(in_struct->pushconstantPipelineLayout),
      pushconstantShaderStageFlags(in_struct->pushconstantShaderStageFlags),
      pushconstantOffset(in_struct->pushconstantOffset),
      pushconstantSize(in_struct->pushconstantSize),
      indirectStateFlags(in_struct->indirectStateFlags),
      indexTypeCount(in_struct->indexTypeCount),
      pIndexTypes(nullptr),
      pIndexTypeValues(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pIndexTypes) {
        pIndexTypes = new VkIndexType[in_struct->indexTypeCount];
        memcpy((void*)pIndexTypes, (void*)in_struct->pIndexTypes, sizeof(VkIndexType) * in_struct->indexTypeCount);
    }

    if (in_struct->pIndexTypeValues) {
        pIndexTypeValues = new uint32_t[in_struct->indexTypeCount];
        memcpy((void*)pIndexTypeValues, (void*)in_struct->pIndexTypeValues, sizeof(uint32_t) * in_struct->indexTypeCount);
    }
}

safe_VkIndirectCommandsLayoutTokenNV::safe_VkIndirectCommandsLayoutTokenNV()
    : sType(VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_TOKEN_NV),
      pNext(nullptr),
      tokenType(),
      stream(),
      offset(),
      vertexBindingUnit(),
      vertexDynamicStride(),
      pushconstantPipelineLayout(),
      pushconstantShaderStageFlags(),
      pushconstantOffset(),
      pushconstantSize(),
      indirectStateFlags(),
      indexTypeCount(),
      pIndexTypes(nullptr),
      pIndexTypeValues(nullptr) {}

safe_VkIndirectCommandsLayoutTokenNV::safe_VkIndirectCommandsLayoutTokenNV(const safe_VkIndirectCommandsLayoutTokenNV& copy_src) {
    sType = copy_src.sType;
    tokenType = copy_src.tokenType;
    stream = copy_src.stream;
    offset = copy_src.offset;
    vertexBindingUnit = copy_src.vertexBindingUnit;
    vertexDynamicStride = copy_src.vertexDynamicStride;
    pushconstantPipelineLayout = copy_src.pushconstantPipelineLayout;
    pushconstantShaderStageFlags = copy_src.pushconstantShaderStageFlags;
    pushconstantOffset = copy_src.pushconstantOffset;
    pushconstantSize = copy_src.pushconstantSize;
    indirectStateFlags = copy_src.indirectStateFlags;
    indexTypeCount = copy_src.indexTypeCount;
    pIndexTypes = nullptr;
    pIndexTypeValues = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pIndexTypes) {
        pIndexTypes = new VkIndexType[copy_src.indexTypeCount];
        memcpy((void*)pIndexTypes, (void*)copy_src.pIndexTypes, sizeof(VkIndexType) * copy_src.indexTypeCount);
    }

    if (copy_src.pIndexTypeValues) {
        pIndexTypeValues = new uint32_t[copy_src.indexTypeCount];
        memcpy((void*)pIndexTypeValues, (void*)copy_src.pIndexTypeValues, sizeof(uint32_t) * copy_src.indexTypeCount);
    }
}

safe_VkIndirectCommandsLayoutTokenNV& safe_VkIndirectCommandsLayoutTokenNV::operator=(
    const safe_VkIndirectCommandsLayoutTokenNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pIndexTypes) delete[] pIndexTypes;
    if (pIndexTypeValues) delete[] pIndexTypeValues;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    tokenType = copy_src.tokenType;
    stream = copy_src.stream;
    offset = copy_src.offset;
    vertexBindingUnit = copy_src.vertexBindingUnit;
    vertexDynamicStride = copy_src.vertexDynamicStride;
    pushconstantPipelineLayout = copy_src.pushconstantPipelineLayout;
    pushconstantShaderStageFlags = copy_src.pushconstantShaderStageFlags;
    pushconstantOffset = copy_src.pushconstantOffset;
    pushconstantSize = copy_src.pushconstantSize;
    indirectStateFlags = copy_src.indirectStateFlags;
    indexTypeCount = copy_src.indexTypeCount;
    pIndexTypes = nullptr;
    pIndexTypeValues = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pIndexTypes) {
        pIndexTypes = new VkIndexType[copy_src.indexTypeCount];
        memcpy((void*)pIndexTypes, (void*)copy_src.pIndexTypes, sizeof(VkIndexType) * copy_src.indexTypeCount);
    }

    if (copy_src.pIndexTypeValues) {
        pIndexTypeValues = new uint32_t[copy_src.indexTypeCount];
        memcpy((void*)pIndexTypeValues, (void*)copy_src.pIndexTypeValues, sizeof(uint32_t) * copy_src.indexTypeCount);
    }

    return *this;
}

safe_VkIndirectCommandsLayoutTokenNV::~safe_VkIndirectCommandsLayoutTokenNV() {
    if (pIndexTypes) delete[] pIndexTypes;
    if (pIndexTypeValues) delete[] pIndexTypeValues;
    FreePnextChain(pNext);
}

void safe_VkIndirectCommandsLayoutTokenNV::initialize(const VkIndirectCommandsLayoutTokenNV* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pIndexTypes) delete[] pIndexTypes;
    if (pIndexTypeValues) delete[] pIndexTypeValues;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    tokenType = in_struct->tokenType;
    stream = in_struct->stream;
    offset = in_struct->offset;
    vertexBindingUnit = in_struct->vertexBindingUnit;
    vertexDynamicStride = in_struct->vertexDynamicStride;
    pushconstantPipelineLayout = in_struct->pushconstantPipelineLayout;
    pushconstantShaderStageFlags = in_struct->pushconstantShaderStageFlags;
    pushconstantOffset = in_struct->pushconstantOffset;
    pushconstantSize = in_struct->pushconstantSize;
    indirectStateFlags = in_struct->indirectStateFlags;
    indexTypeCount = in_struct->indexTypeCount;
    pIndexTypes = nullptr;
    pIndexTypeValues = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pIndexTypes) {
        pIndexTypes = new VkIndexType[in_struct->indexTypeCount];
        memcpy((void*)pIndexTypes, (void*)in_struct->pIndexTypes, sizeof(VkIndexType) * in_struct->indexTypeCount);
    }

    if (in_struct->pIndexTypeValues) {
        pIndexTypeValues = new uint32_t[in_struct->indexTypeCount];
        memcpy((void*)pIndexTypeValues, (void*)in_struct->pIndexTypeValues, sizeof(uint32_t) * in_struct->indexTypeCount);
    }
}

void safe_VkIndirectCommandsLayoutTokenNV::initialize(const safe_VkIndirectCommandsLayoutTokenNV* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    tokenType = copy_src->tokenType;
    stream = copy_src->stream;
    offset = copy_src->offset;
    vertexBindingUnit = copy_src->vertexBindingUnit;
    vertexDynamicStride = copy_src->vertexDynamicStride;
    pushconstantPipelineLayout = copy_src->pushconstantPipelineLayout;
    pushconstantShaderStageFlags = copy_src->pushconstantShaderStageFlags;
    pushconstantOffset = copy_src->pushconstantOffset;
    pushconstantSize = copy_src->pushconstantSize;
    indirectStateFlags = copy_src->indirectStateFlags;
    indexTypeCount = copy_src->indexTypeCount;
    pIndexTypes = nullptr;
    pIndexTypeValues = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pIndexTypes) {
        pIndexTypes = new VkIndexType[copy_src->indexTypeCount];
        memcpy((void*)pIndexTypes, (void*)copy_src->pIndexTypes, sizeof(VkIndexType) * copy_src->indexTypeCount);
    }

    if (copy_src->pIndexTypeValues) {
        pIndexTypeValues = new uint32_t[copy_src->indexTypeCount];
        memcpy((void*)pIndexTypeValues, (void*)copy_src->pIndexTypeValues, sizeof(uint32_t) * copy_src->indexTypeCount);
    }
}

safe_VkIndirectCommandsLayoutCreateInfoNV::safe_VkIndirectCommandsLayoutCreateInfoNV(
    const VkIndirectCommandsLayoutCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      pipelineBindPoint(in_struct->pipelineBindPoint),
      tokenCount(in_struct->tokenCount),
      pTokens(nullptr),
      streamCount(in_struct->streamCount),
      pStreamStrides(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (tokenCount && in_struct->pTokens) {
        pTokens = new safe_VkIndirectCommandsLayoutTokenNV[tokenCount];
        for (uint32_t i = 0; i < tokenCount; ++i) {
            pTokens[i].initialize(&in_struct->pTokens[i]);
        }
    }

    if (in_struct->pStreamStrides) {
        pStreamStrides = new uint32_t[in_struct->streamCount];
        memcpy((void*)pStreamStrides, (void*)in_struct->pStreamStrides, sizeof(uint32_t) * in_struct->streamCount);
    }
}

safe_VkIndirectCommandsLayoutCreateInfoNV::safe_VkIndirectCommandsLayoutCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_INDIRECT_COMMANDS_LAYOUT_CREATE_INFO_NV),
      pNext(nullptr),
      flags(),
      pipelineBindPoint(),
      tokenCount(),
      pTokens(nullptr),
      streamCount(),
      pStreamStrides(nullptr) {}

safe_VkIndirectCommandsLayoutCreateInfoNV::safe_VkIndirectCommandsLayoutCreateInfoNV(
    const safe_VkIndirectCommandsLayoutCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    pipelineBindPoint = copy_src.pipelineBindPoint;
    tokenCount = copy_src.tokenCount;
    pTokens = nullptr;
    streamCount = copy_src.streamCount;
    pStreamStrides = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (tokenCount && copy_src.pTokens) {
        pTokens = new safe_VkIndirectCommandsLayoutTokenNV[tokenCount];
        for (uint32_t i = 0; i < tokenCount; ++i) {
            pTokens[i].initialize(&copy_src.pTokens[i]);
        }
    }

    if (copy_src.pStreamStrides) {
        pStreamStrides = new uint32_t[copy_src.streamCount];
        memcpy((void*)pStreamStrides, (void*)copy_src.pStreamStrides, sizeof(uint32_t) * copy_src.streamCount);
    }
}

safe_VkIndirectCommandsLayoutCreateInfoNV& safe_VkIndirectCommandsLayoutCreateInfoNV::operator=(
    const safe_VkIndirectCommandsLayoutCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pTokens) delete[] pTokens;
    if (pStreamStrides) delete[] pStreamStrides;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    pipelineBindPoint = copy_src.pipelineBindPoint;
    tokenCount = copy_src.tokenCount;
    pTokens = nullptr;
    streamCount = copy_src.streamCount;
    pStreamStrides = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (tokenCount && copy_src.pTokens) {
        pTokens = new safe_VkIndirectCommandsLayoutTokenNV[tokenCount];
        for (uint32_t i = 0; i < tokenCount; ++i) {
            pTokens[i].initialize(&copy_src.pTokens[i]);
        }
    }

    if (copy_src.pStreamStrides) {
        pStreamStrides = new uint32_t[copy_src.streamCount];
        memcpy((void*)pStreamStrides, (void*)copy_src.pStreamStrides, sizeof(uint32_t) * copy_src.streamCount);
    }

    return *this;
}

safe_VkIndirectCommandsLayoutCreateInfoNV::~safe_VkIndirectCommandsLayoutCreateInfoNV() {
    if (pTokens) delete[] pTokens;
    if (pStreamStrides) delete[] pStreamStrides;
    FreePnextChain(pNext);
}

void safe_VkIndirectCommandsLayoutCreateInfoNV::initialize(const VkIndirectCommandsLayoutCreateInfoNV* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    if (pTokens) delete[] pTokens;
    if (pStreamStrides) delete[] pStreamStrides;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    pipelineBindPoint = in_struct->pipelineBindPoint;
    tokenCount = in_struct->tokenCount;
    pTokens = nullptr;
    streamCount = in_struct->streamCount;
    pStreamStrides = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (tokenCount && in_struct->pTokens) {
        pTokens = new safe_VkIndirectCommandsLayoutTokenNV[tokenCount];
        for (uint32_t i = 0; i < tokenCount; ++i) {
            pTokens[i].initialize(&in_struct->pTokens[i]);
        }
    }

    if (in_struct->pStreamStrides) {
        pStreamStrides = new uint32_t[in_struct->streamCount];
        memcpy((void*)pStreamStrides, (void*)in_struct->pStreamStrides, sizeof(uint32_t) * in_struct->streamCount);
    }
}

void safe_VkIndirectCommandsLayoutCreateInfoNV::initialize(const safe_VkIndirectCommandsLayoutCreateInfoNV* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    pipelineBindPoint = copy_src->pipelineBindPoint;
    tokenCount = copy_src->tokenCount;
    pTokens = nullptr;
    streamCount = copy_src->streamCount;
    pStreamStrides = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (tokenCount && copy_src->pTokens) {
        pTokens = new safe_VkIndirectCommandsLayoutTokenNV[tokenCount];
        for (uint32_t i = 0; i < tokenCount; ++i) {
            pTokens[i].initialize(&copy_src->pTokens[i]);
        }
    }

    if (copy_src->pStreamStrides) {
        pStreamStrides = new uint32_t[copy_src->streamCount];
        memcpy((void*)pStreamStrides, (void*)copy_src->pStreamStrides, sizeof(uint32_t) * copy_src->streamCount);
    }
}

safe_VkGeneratedCommandsInfoNV::safe_VkGeneratedCommandsInfoNV(const VkGeneratedCommandsInfoNV* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      pipelineBindPoint(in_struct->pipelineBindPoint),
      pipeline(in_struct->pipeline),
      indirectCommandsLayout(in_struct->indirectCommandsLayout),
      streamCount(in_struct->streamCount),
      pStreams(nullptr),
      sequencesCount(in_struct->sequencesCount),
      preprocessBuffer(in_struct->preprocessBuffer),
      preprocessOffset(in_struct->preprocessOffset),
      preprocessSize(in_struct->preprocessSize),
      sequencesCountBuffer(in_struct->sequencesCountBuffer),
      sequencesCountOffset(in_struct->sequencesCountOffset),
      sequencesIndexBuffer(in_struct->sequencesIndexBuffer),
      sequencesIndexOffset(in_struct->sequencesIndexOffset) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (streamCount && in_struct->pStreams) {
        pStreams = new VkIndirectCommandsStreamNV[streamCount];
        for (uint32_t i = 0; i < streamCount; ++i) {
            pStreams[i] = in_struct->pStreams[i];
        }
    }
}

safe_VkGeneratedCommandsInfoNV::safe_VkGeneratedCommandsInfoNV()
    : sType(VK_STRUCTURE_TYPE_GENERATED_COMMANDS_INFO_NV),
      pNext(nullptr),
      pipelineBindPoint(),
      pipeline(),
      indirectCommandsLayout(),
      streamCount(),
      pStreams(nullptr),
      sequencesCount(),
      preprocessBuffer(),
      preprocessOffset(),
      preprocessSize(),
      sequencesCountBuffer(),
      sequencesCountOffset(),
      sequencesIndexBuffer(),
      sequencesIndexOffset() {}

safe_VkGeneratedCommandsInfoNV::safe_VkGeneratedCommandsInfoNV(const safe_VkGeneratedCommandsInfoNV& copy_src) {
    sType = copy_src.sType;
    pipelineBindPoint = copy_src.pipelineBindPoint;
    pipeline = copy_src.pipeline;
    indirectCommandsLayout = copy_src.indirectCommandsLayout;
    streamCount = copy_src.streamCount;
    pStreams = nullptr;
    sequencesCount = copy_src.sequencesCount;
    preprocessBuffer = copy_src.preprocessBuffer;
    preprocessOffset = copy_src.preprocessOffset;
    preprocessSize = copy_src.preprocessSize;
    sequencesCountBuffer = copy_src.sequencesCountBuffer;
    sequencesCountOffset = copy_src.sequencesCountOffset;
    sequencesIndexBuffer = copy_src.sequencesIndexBuffer;
    sequencesIndexOffset = copy_src.sequencesIndexOffset;
    pNext = SafePnextCopy(copy_src.pNext);
    if (streamCount && copy_src.pStreams) {
        pStreams = new VkIndirectCommandsStreamNV[streamCount];
        for (uint32_t i = 0; i < streamCount; ++i) {
            pStreams[i] = copy_src.pStreams[i];
        }
    }
}

safe_VkGeneratedCommandsInfoNV& safe_VkGeneratedCommandsInfoNV::operator=(const safe_VkGeneratedCommandsInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pStreams) delete[] pStreams;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipelineBindPoint = copy_src.pipelineBindPoint;
    pipeline = copy_src.pipeline;
    indirectCommandsLayout = copy_src.indirectCommandsLayout;
    streamCount = copy_src.streamCount;
    pStreams = nullptr;
    sequencesCount = copy_src.sequencesCount;
    preprocessBuffer = copy_src.preprocessBuffer;
    preprocessOffset = copy_src.preprocessOffset;
    preprocessSize = copy_src.preprocessSize;
    sequencesCountBuffer = copy_src.sequencesCountBuffer;
    sequencesCountOffset = copy_src.sequencesCountOffset;
    sequencesIndexBuffer = copy_src.sequencesIndexBuffer;
    sequencesIndexOffset = copy_src.sequencesIndexOffset;
    pNext = SafePnextCopy(copy_src.pNext);
    if (streamCount && copy_src.pStreams) {
        pStreams = new VkIndirectCommandsStreamNV[streamCount];
        for (uint32_t i = 0; i < streamCount; ++i) {
            pStreams[i] = copy_src.pStreams[i];
        }
    }

    return *this;
}

safe_VkGeneratedCommandsInfoNV::~safe_VkGeneratedCommandsInfoNV() {
    if (pStreams) delete[] pStreams;
    FreePnextChain(pNext);
}

void safe_VkGeneratedCommandsInfoNV::initialize(const VkGeneratedCommandsInfoNV* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStreams) delete[] pStreams;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipelineBindPoint = in_struct->pipelineBindPoint;
    pipeline = in_struct->pipeline;
    indirectCommandsLayout = in_struct->indirectCommandsLayout;
    streamCount = in_struct->streamCount;
    pStreams = nullptr;
    sequencesCount = in_struct->sequencesCount;
    preprocessBuffer = in_struct->preprocessBuffer;
    preprocessOffset = in_struct->preprocessOffset;
    preprocessSize = in_struct->preprocessSize;
    sequencesCountBuffer = in_struct->sequencesCountBuffer;
    sequencesCountOffset = in_struct->sequencesCountOffset;
    sequencesIndexBuffer = in_struct->sequencesIndexBuffer;
    sequencesIndexOffset = in_struct->sequencesIndexOffset;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (streamCount && in_struct->pStreams) {
        pStreams = new VkIndirectCommandsStreamNV[streamCount];
        for (uint32_t i = 0; i < streamCount; ++i) {
            pStreams[i] = in_struct->pStreams[i];
        }
    }
}

void safe_VkGeneratedCommandsInfoNV::initialize(const safe_VkGeneratedCommandsInfoNV* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipelineBindPoint = copy_src->pipelineBindPoint;
    pipeline = copy_src->pipeline;
    indirectCommandsLayout = copy_src->indirectCommandsLayout;
    streamCount = copy_src->streamCount;
    pStreams = nullptr;
    sequencesCount = copy_src->sequencesCount;
    preprocessBuffer = copy_src->preprocessBuffer;
    preprocessOffset = copy_src->preprocessOffset;
    preprocessSize = copy_src->preprocessSize;
    sequencesCountBuffer = copy_src->sequencesCountBuffer;
    sequencesCountOffset = copy_src->sequencesCountOffset;
    sequencesIndexBuffer = copy_src->sequencesIndexBuffer;
    sequencesIndexOffset = copy_src->sequencesIndexOffset;
    pNext = SafePnextCopy(copy_src->pNext);
    if (streamCount && copy_src->pStreams) {
        pStreams = new VkIndirectCommandsStreamNV[streamCount];
        for (uint32_t i = 0; i < streamCount; ++i) {
            pStreams[i] = copy_src->pStreams[i];
        }
    }
}

safe_VkGeneratedCommandsMemoryRequirementsInfoNV::safe_VkGeneratedCommandsMemoryRequirementsInfoNV(
    const VkGeneratedCommandsMemoryRequirementsInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      pipelineBindPoint(in_struct->pipelineBindPoint),
      pipeline(in_struct->pipeline),
      indirectCommandsLayout(in_struct->indirectCommandsLayout),
      maxSequencesCount(in_struct->maxSequencesCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkGeneratedCommandsMemoryRequirementsInfoNV::safe_VkGeneratedCommandsMemoryRequirementsInfoNV()
    : sType(VK_STRUCTURE_TYPE_GENERATED_COMMANDS_MEMORY_REQUIREMENTS_INFO_NV),
      pNext(nullptr),
      pipelineBindPoint(),
      pipeline(),
      indirectCommandsLayout(),
      maxSequencesCount() {}

safe_VkGeneratedCommandsMemoryRequirementsInfoNV::safe_VkGeneratedCommandsMemoryRequirementsInfoNV(
    const safe_VkGeneratedCommandsMemoryRequirementsInfoNV& copy_src) {
    sType = copy_src.sType;
    pipelineBindPoint = copy_src.pipelineBindPoint;
    pipeline = copy_src.pipeline;
    indirectCommandsLayout = copy_src.indirectCommandsLayout;
    maxSequencesCount = copy_src.maxSequencesCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkGeneratedCommandsMemoryRequirementsInfoNV& safe_VkGeneratedCommandsMemoryRequirementsInfoNV::operator=(
    const safe_VkGeneratedCommandsMemoryRequirementsInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipelineBindPoint = copy_src.pipelineBindPoint;
    pipeline = copy_src.pipeline;
    indirectCommandsLayout = copy_src.indirectCommandsLayout;
    maxSequencesCount = copy_src.maxSequencesCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkGeneratedCommandsMemoryRequirementsInfoNV::~safe_VkGeneratedCommandsMemoryRequirementsInfoNV() { FreePnextChain(pNext); }

void safe_VkGeneratedCommandsMemoryRequirementsInfoNV::initialize(const VkGeneratedCommandsMemoryRequirementsInfoNV* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipelineBindPoint = in_struct->pipelineBindPoint;
    pipeline = in_struct->pipeline;
    indirectCommandsLayout = in_struct->indirectCommandsLayout;
    maxSequencesCount = in_struct->maxSequencesCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkGeneratedCommandsMemoryRequirementsInfoNV::initialize(const safe_VkGeneratedCommandsMemoryRequirementsInfoNV* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipelineBindPoint = copy_src->pipelineBindPoint;
    pipeline = copy_src->pipeline;
    indirectCommandsLayout = copy_src->indirectCommandsLayout;
    maxSequencesCount = copy_src->maxSequencesCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV::safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV(
    const VkPhysicalDeviceInheritedViewportScissorFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), inheritedViewportScissor2D(in_struct->inheritedViewportScissor2D) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV::safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INHERITED_VIEWPORT_SCISSOR_FEATURES_NV),
      pNext(nullptr),
      inheritedViewportScissor2D() {}

safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV::safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV(
    const safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV& copy_src) {
    sType = copy_src.sType;
    inheritedViewportScissor2D = copy_src.inheritedViewportScissor2D;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV& safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV::operator=(
    const safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    inheritedViewportScissor2D = copy_src.inheritedViewportScissor2D;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV::~safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV::initialize(
    const VkPhysicalDeviceInheritedViewportScissorFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    inheritedViewportScissor2D = in_struct->inheritedViewportScissor2D;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV::initialize(
    const safe_VkPhysicalDeviceInheritedViewportScissorFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    inheritedViewportScissor2D = copy_src->inheritedViewportScissor2D;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCommandBufferInheritanceViewportScissorInfoNV::safe_VkCommandBufferInheritanceViewportScissorInfoNV(
    const VkCommandBufferInheritanceViewportScissorInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      viewportScissor2D(in_struct->viewportScissor2D),
      viewportDepthCount(in_struct->viewportDepthCount),
      pViewportDepths(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pViewportDepths) {
        pViewportDepths = new VkViewport(*in_struct->pViewportDepths);
    }
}

safe_VkCommandBufferInheritanceViewportScissorInfoNV::safe_VkCommandBufferInheritanceViewportScissorInfoNV()
    : sType(VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_VIEWPORT_SCISSOR_INFO_NV),
      pNext(nullptr),
      viewportScissor2D(),
      viewportDepthCount(),
      pViewportDepths(nullptr) {}

safe_VkCommandBufferInheritanceViewportScissorInfoNV::safe_VkCommandBufferInheritanceViewportScissorInfoNV(
    const safe_VkCommandBufferInheritanceViewportScissorInfoNV& copy_src) {
    sType = copy_src.sType;
    viewportScissor2D = copy_src.viewportScissor2D;
    viewportDepthCount = copy_src.viewportDepthCount;
    pViewportDepths = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pViewportDepths) {
        pViewportDepths = new VkViewport(*copy_src.pViewportDepths);
    }
}

safe_VkCommandBufferInheritanceViewportScissorInfoNV& safe_VkCommandBufferInheritanceViewportScissorInfoNV::operator=(
    const safe_VkCommandBufferInheritanceViewportScissorInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pViewportDepths) delete pViewportDepths;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    viewportScissor2D = copy_src.viewportScissor2D;
    viewportDepthCount = copy_src.viewportDepthCount;
    pViewportDepths = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pViewportDepths) {
        pViewportDepths = new VkViewport(*copy_src.pViewportDepths);
    }

    return *this;
}

safe_VkCommandBufferInheritanceViewportScissorInfoNV::~safe_VkCommandBufferInheritanceViewportScissorInfoNV() {
    if (pViewportDepths) delete pViewportDepths;
    FreePnextChain(pNext);
}

void safe_VkCommandBufferInheritanceViewportScissorInfoNV::initialize(
    const VkCommandBufferInheritanceViewportScissorInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pViewportDepths) delete pViewportDepths;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    viewportScissor2D = in_struct->viewportScissor2D;
    viewportDepthCount = in_struct->viewportDepthCount;
    pViewportDepths = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pViewportDepths) {
        pViewportDepths = new VkViewport(*in_struct->pViewportDepths);
    }
}

void safe_VkCommandBufferInheritanceViewportScissorInfoNV::initialize(
    const safe_VkCommandBufferInheritanceViewportScissorInfoNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    viewportScissor2D = copy_src->viewportScissor2D;
    viewportDepthCount = copy_src->viewportDepthCount;
    pViewportDepths = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pViewportDepths) {
        pViewportDepths = new VkViewport(*copy_src->pViewportDepths);
    }
}

safe_VkRenderPassTransformBeginInfoQCOM::safe_VkRenderPassTransformBeginInfoQCOM(
    const VkRenderPassTransformBeginInfoQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), transform(in_struct->transform) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkRenderPassTransformBeginInfoQCOM::safe_VkRenderPassTransformBeginInfoQCOM()
    : sType(VK_STRUCTURE_TYPE_RENDER_PASS_TRANSFORM_BEGIN_INFO_QCOM), pNext(nullptr), transform() {}

safe_VkRenderPassTransformBeginInfoQCOM::safe_VkRenderPassTransformBeginInfoQCOM(
    const safe_VkRenderPassTransformBeginInfoQCOM& copy_src) {
    sType = copy_src.sType;
    transform = copy_src.transform;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkRenderPassTransformBeginInfoQCOM& safe_VkRenderPassTransformBeginInfoQCOM::operator=(
    const safe_VkRenderPassTransformBeginInfoQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    transform = copy_src.transform;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkRenderPassTransformBeginInfoQCOM::~safe_VkRenderPassTransformBeginInfoQCOM() { FreePnextChain(pNext); }

void safe_VkRenderPassTransformBeginInfoQCOM::initialize(const VkRenderPassTransformBeginInfoQCOM* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    transform = in_struct->transform;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkRenderPassTransformBeginInfoQCOM::initialize(const safe_VkRenderPassTransformBeginInfoQCOM* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    transform = copy_src->transform;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM::safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM(
    const VkCommandBufferInheritanceRenderPassTransformInfoQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), transform(in_struct->transform), renderArea(in_struct->renderArea) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM::safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM()
    : sType(VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_RENDER_PASS_TRANSFORM_INFO_QCOM),
      pNext(nullptr),
      transform(),
      renderArea() {}

safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM::safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM(
    const safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM& copy_src) {
    sType = copy_src.sType;
    transform = copy_src.transform;
    renderArea = copy_src.renderArea;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM& safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM::operator=(
    const safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    transform = copy_src.transform;
    renderArea = copy_src.renderArea;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM::~safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM() {
    FreePnextChain(pNext);
}

void safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM::initialize(
    const VkCommandBufferInheritanceRenderPassTransformInfoQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    transform = in_struct->transform;
    renderArea = in_struct->renderArea;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM::initialize(
    const safe_VkCommandBufferInheritanceRenderPassTransformInfoQCOM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    transform = copy_src->transform;
    renderArea = copy_src->renderArea;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePresentBarrierFeaturesNV::safe_VkPhysicalDevicePresentBarrierFeaturesNV(
    const VkPhysicalDevicePresentBarrierFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), presentBarrier(in_struct->presentBarrier) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePresentBarrierFeaturesNV::safe_VkPhysicalDevicePresentBarrierFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_BARRIER_FEATURES_NV), pNext(nullptr), presentBarrier() {}

safe_VkPhysicalDevicePresentBarrierFeaturesNV::safe_VkPhysicalDevicePresentBarrierFeaturesNV(
    const safe_VkPhysicalDevicePresentBarrierFeaturesNV& copy_src) {
    sType = copy_src.sType;
    presentBarrier = copy_src.presentBarrier;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePresentBarrierFeaturesNV& safe_VkPhysicalDevicePresentBarrierFeaturesNV::operator=(
    const safe_VkPhysicalDevicePresentBarrierFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentBarrier = copy_src.presentBarrier;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePresentBarrierFeaturesNV::~safe_VkPhysicalDevicePresentBarrierFeaturesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDevicePresentBarrierFeaturesNV::initialize(const VkPhysicalDevicePresentBarrierFeaturesNV* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentBarrier = in_struct->presentBarrier;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePresentBarrierFeaturesNV::initialize(const safe_VkPhysicalDevicePresentBarrierFeaturesNV* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentBarrier = copy_src->presentBarrier;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSurfaceCapabilitiesPresentBarrierNV::safe_VkSurfaceCapabilitiesPresentBarrierNV(
    const VkSurfaceCapabilitiesPresentBarrierNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), presentBarrierSupported(in_struct->presentBarrierSupported) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSurfaceCapabilitiesPresentBarrierNV::safe_VkSurfaceCapabilitiesPresentBarrierNV()
    : sType(VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_PRESENT_BARRIER_NV), pNext(nullptr), presentBarrierSupported() {}

safe_VkSurfaceCapabilitiesPresentBarrierNV::safe_VkSurfaceCapabilitiesPresentBarrierNV(
    const safe_VkSurfaceCapabilitiesPresentBarrierNV& copy_src) {
    sType = copy_src.sType;
    presentBarrierSupported = copy_src.presentBarrierSupported;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSurfaceCapabilitiesPresentBarrierNV& safe_VkSurfaceCapabilitiesPresentBarrierNV::operator=(
    const safe_VkSurfaceCapabilitiesPresentBarrierNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentBarrierSupported = copy_src.presentBarrierSupported;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSurfaceCapabilitiesPresentBarrierNV::~safe_VkSurfaceCapabilitiesPresentBarrierNV() { FreePnextChain(pNext); }

void safe_VkSurfaceCapabilitiesPresentBarrierNV::initialize(const VkSurfaceCapabilitiesPresentBarrierNV* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentBarrierSupported = in_struct->presentBarrierSupported;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSurfaceCapabilitiesPresentBarrierNV::initialize(const safe_VkSurfaceCapabilitiesPresentBarrierNV* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentBarrierSupported = copy_src->presentBarrierSupported;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSwapchainPresentBarrierCreateInfoNV::safe_VkSwapchainPresentBarrierCreateInfoNV(
    const VkSwapchainPresentBarrierCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), presentBarrierEnable(in_struct->presentBarrierEnable) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSwapchainPresentBarrierCreateInfoNV::safe_VkSwapchainPresentBarrierCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_SWAPCHAIN_PRESENT_BARRIER_CREATE_INFO_NV), pNext(nullptr), presentBarrierEnable() {}

safe_VkSwapchainPresentBarrierCreateInfoNV::safe_VkSwapchainPresentBarrierCreateInfoNV(
    const safe_VkSwapchainPresentBarrierCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    presentBarrierEnable = copy_src.presentBarrierEnable;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSwapchainPresentBarrierCreateInfoNV& safe_VkSwapchainPresentBarrierCreateInfoNV::operator=(
    const safe_VkSwapchainPresentBarrierCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentBarrierEnable = copy_src.presentBarrierEnable;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSwapchainPresentBarrierCreateInfoNV::~safe_VkSwapchainPresentBarrierCreateInfoNV() { FreePnextChain(pNext); }

void safe_VkSwapchainPresentBarrierCreateInfoNV::initialize(const VkSwapchainPresentBarrierCreateInfoNV* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentBarrierEnable = in_struct->presentBarrierEnable;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSwapchainPresentBarrierCreateInfoNV::initialize(const safe_VkSwapchainPresentBarrierCreateInfoNV* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentBarrierEnable = copy_src->presentBarrierEnable;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV::safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV(
    const VkPhysicalDeviceDiagnosticsConfigFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), diagnosticsConfig(in_struct->diagnosticsConfig) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV::safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DIAGNOSTICS_CONFIG_FEATURES_NV), pNext(nullptr), diagnosticsConfig() {}

safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV::safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV(
    const safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV& copy_src) {
    sType = copy_src.sType;
    diagnosticsConfig = copy_src.diagnosticsConfig;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV& safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV::operator=(
    const safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    diagnosticsConfig = copy_src.diagnosticsConfig;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV::~safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV::initialize(const VkPhysicalDeviceDiagnosticsConfigFeaturesNV* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    diagnosticsConfig = in_struct->diagnosticsConfig;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV::initialize(const safe_VkPhysicalDeviceDiagnosticsConfigFeaturesNV* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    diagnosticsConfig = copy_src->diagnosticsConfig;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDeviceDiagnosticsConfigCreateInfoNV::safe_VkDeviceDiagnosticsConfigCreateInfoNV(
    const VkDeviceDiagnosticsConfigCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDeviceDiagnosticsConfigCreateInfoNV::safe_VkDeviceDiagnosticsConfigCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_DEVICE_DIAGNOSTICS_CONFIG_CREATE_INFO_NV), pNext(nullptr), flags() {}

safe_VkDeviceDiagnosticsConfigCreateInfoNV::safe_VkDeviceDiagnosticsConfigCreateInfoNV(
    const safe_VkDeviceDiagnosticsConfigCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDeviceDiagnosticsConfigCreateInfoNV& safe_VkDeviceDiagnosticsConfigCreateInfoNV::operator=(
    const safe_VkDeviceDiagnosticsConfigCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDeviceDiagnosticsConfigCreateInfoNV::~safe_VkDeviceDiagnosticsConfigCreateInfoNV() { FreePnextChain(pNext); }

void safe_VkDeviceDiagnosticsConfigCreateInfoNV::initialize(const VkDeviceDiagnosticsConfigCreateInfoNV* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDeviceDiagnosticsConfigCreateInfoNV::initialize(const safe_VkDeviceDiagnosticsConfigCreateInfoNV* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCudaModuleCreateInfoNV::safe_VkCudaModuleCreateInfoNV(const VkCudaModuleCreateInfoNV* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), dataSize(in_struct->dataSize), pData(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pData != nullptr) {
        auto temp = new std::byte[in_struct->dataSize];
        std::memcpy(temp, in_struct->pData, in_struct->dataSize);
        pData = temp;
    }
}

safe_VkCudaModuleCreateInfoNV::safe_VkCudaModuleCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_CUDA_MODULE_CREATE_INFO_NV), pNext(nullptr), dataSize(), pData(nullptr) {}

safe_VkCudaModuleCreateInfoNV::safe_VkCudaModuleCreateInfoNV(const safe_VkCudaModuleCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    dataSize = copy_src.dataSize;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pData != nullptr) {
        auto temp = new std::byte[copy_src.dataSize];
        std::memcpy(temp, copy_src.pData, copy_src.dataSize);
        pData = temp;
    }
}

safe_VkCudaModuleCreateInfoNV& safe_VkCudaModuleCreateInfoNV::operator=(const safe_VkCudaModuleCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pData != nullptr) {
        auto temp = reinterpret_cast<const std::byte*>(pData);
        delete[] temp;
    }
    FreePnextChain(pNext);

    sType = copy_src.sType;
    dataSize = copy_src.dataSize;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pData != nullptr) {
        auto temp = new std::byte[copy_src.dataSize];
        std::memcpy(temp, copy_src.pData, copy_src.dataSize);
        pData = temp;
    }

    return *this;
}

safe_VkCudaModuleCreateInfoNV::~safe_VkCudaModuleCreateInfoNV() {
    if (pData != nullptr) {
        auto temp = reinterpret_cast<const std::byte*>(pData);
        delete[] temp;
    }
    FreePnextChain(pNext);
}

void safe_VkCudaModuleCreateInfoNV::initialize(const VkCudaModuleCreateInfoNV* in_struct,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    if (pData != nullptr) {
        auto temp = reinterpret_cast<const std::byte*>(pData);
        delete[] temp;
    }
    FreePnextChain(pNext);
    sType = in_struct->sType;
    dataSize = in_struct->dataSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pData != nullptr) {
        auto temp = new std::byte[in_struct->dataSize];
        std::memcpy(temp, in_struct->pData, in_struct->dataSize);
        pData = temp;
    }
}

void safe_VkCudaModuleCreateInfoNV::initialize(const safe_VkCudaModuleCreateInfoNV* copy_src,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    dataSize = copy_src->dataSize;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pData != nullptr) {
        auto temp = new std::byte[copy_src->dataSize];
        std::memcpy(temp, copy_src->pData, copy_src->dataSize);
        pData = temp;
    }
}

safe_VkCudaFunctionCreateInfoNV::safe_VkCudaFunctionCreateInfoNV(const VkCudaFunctionCreateInfoNV* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), module(in_struct->module) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    pName = SafeStringCopy(in_struct->pName);
}

safe_VkCudaFunctionCreateInfoNV::safe_VkCudaFunctionCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_CUDA_FUNCTION_CREATE_INFO_NV), pNext(nullptr), module(), pName(nullptr) {}

safe_VkCudaFunctionCreateInfoNV::safe_VkCudaFunctionCreateInfoNV(const safe_VkCudaFunctionCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    module = copy_src.module;
    pNext = SafePnextCopy(copy_src.pNext);
    pName = SafeStringCopy(copy_src.pName);
}

safe_VkCudaFunctionCreateInfoNV& safe_VkCudaFunctionCreateInfoNV::operator=(const safe_VkCudaFunctionCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pName) delete[] pName;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    module = copy_src.module;
    pNext = SafePnextCopy(copy_src.pNext);
    pName = SafeStringCopy(copy_src.pName);

    return *this;
}

safe_VkCudaFunctionCreateInfoNV::~safe_VkCudaFunctionCreateInfoNV() {
    if (pName) delete[] pName;
    FreePnextChain(pNext);
}

void safe_VkCudaFunctionCreateInfoNV::initialize(const VkCudaFunctionCreateInfoNV* in_struct,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    if (pName) delete[] pName;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    module = in_struct->module;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    pName = SafeStringCopy(in_struct->pName);
}

void safe_VkCudaFunctionCreateInfoNV::initialize(const safe_VkCudaFunctionCreateInfoNV* copy_src,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    module = copy_src->module;
    pNext = SafePnextCopy(copy_src->pNext);
    pName = SafeStringCopy(copy_src->pName);
}

safe_VkCudaLaunchInfoNV::safe_VkCudaLaunchInfoNV(const VkCudaLaunchInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
                                                 bool copy_pnext)
    : sType(in_struct->sType),
      function(in_struct->function),
      gridDimX(in_struct->gridDimX),
      gridDimY(in_struct->gridDimY),
      gridDimZ(in_struct->gridDimZ),
      blockDimX(in_struct->blockDimX),
      blockDimY(in_struct->blockDimY),
      blockDimZ(in_struct->blockDimZ),
      sharedMemBytes(in_struct->sharedMemBytes),
      paramCount(in_struct->paramCount),
      pParams(in_struct->pParams),
      extraCount(in_struct->extraCount),
      pExtras(in_struct->pExtras) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCudaLaunchInfoNV::safe_VkCudaLaunchInfoNV()
    : sType(VK_STRUCTURE_TYPE_CUDA_LAUNCH_INFO_NV),
      pNext(nullptr),
      function(),
      gridDimX(),
      gridDimY(),
      gridDimZ(),
      blockDimX(),
      blockDimY(),
      blockDimZ(),
      sharedMemBytes(),
      paramCount(),
      pParams(nullptr),
      extraCount(),
      pExtras(nullptr) {}

safe_VkCudaLaunchInfoNV::safe_VkCudaLaunchInfoNV(const safe_VkCudaLaunchInfoNV& copy_src) {
    sType = copy_src.sType;
    function = copy_src.function;
    gridDimX = copy_src.gridDimX;
    gridDimY = copy_src.gridDimY;
    gridDimZ = copy_src.gridDimZ;
    blockDimX = copy_src.blockDimX;
    blockDimY = copy_src.blockDimY;
    blockDimZ = copy_src.blockDimZ;
    sharedMemBytes = copy_src.sharedMemBytes;
    paramCount = copy_src.paramCount;
    pParams = copy_src.pParams;
    extraCount = copy_src.extraCount;
    pExtras = copy_src.pExtras;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCudaLaunchInfoNV& safe_VkCudaLaunchInfoNV::operator=(const safe_VkCudaLaunchInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    function = copy_src.function;
    gridDimX = copy_src.gridDimX;
    gridDimY = copy_src.gridDimY;
    gridDimZ = copy_src.gridDimZ;
    blockDimX = copy_src.blockDimX;
    blockDimY = copy_src.blockDimY;
    blockDimZ = copy_src.blockDimZ;
    sharedMemBytes = copy_src.sharedMemBytes;
    paramCount = copy_src.paramCount;
    pParams = copy_src.pParams;
    extraCount = copy_src.extraCount;
    pExtras = copy_src.pExtras;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCudaLaunchInfoNV::~safe_VkCudaLaunchInfoNV() { FreePnextChain(pNext); }

void safe_VkCudaLaunchInfoNV::initialize(const VkCudaLaunchInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    function = in_struct->function;
    gridDimX = in_struct->gridDimX;
    gridDimY = in_struct->gridDimY;
    gridDimZ = in_struct->gridDimZ;
    blockDimX = in_struct->blockDimX;
    blockDimY = in_struct->blockDimY;
    blockDimZ = in_struct->blockDimZ;
    sharedMemBytes = in_struct->sharedMemBytes;
    paramCount = in_struct->paramCount;
    pParams = in_struct->pParams;
    extraCount = in_struct->extraCount;
    pExtras = in_struct->pExtras;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCudaLaunchInfoNV::initialize(const safe_VkCudaLaunchInfoNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    function = copy_src->function;
    gridDimX = copy_src->gridDimX;
    gridDimY = copy_src->gridDimY;
    gridDimZ = copy_src->gridDimZ;
    blockDimX = copy_src->blockDimX;
    blockDimY = copy_src->blockDimY;
    blockDimZ = copy_src->blockDimZ;
    sharedMemBytes = copy_src->sharedMemBytes;
    paramCount = copy_src->paramCount;
    pParams = copy_src->pParams;
    extraCount = copy_src->extraCount;
    pExtras = copy_src->pExtras;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCudaKernelLaunchFeaturesNV::safe_VkPhysicalDeviceCudaKernelLaunchFeaturesNV(
    const VkPhysicalDeviceCudaKernelLaunchFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), cudaKernelLaunchFeatures(in_struct->cudaKernelLaunchFeatures) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCudaKernelLaunchFeaturesNV::safe_VkPhysicalDeviceCudaKernelLaunchFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_FEATURES_NV), pNext(nullptr), cudaKernelLaunchFeatures() {}

safe_VkPhysicalDeviceCudaKernelLaunchFeaturesNV::safe_VkPhysicalDeviceCudaKernelLaunchFeaturesNV(
    const safe_VkPhysicalDeviceCudaKernelLaunchFeaturesNV& copy_src) {
    sType = copy_src.sType;
    cudaKernelLaunchFeatures = copy_src.cudaKernelLaunchFeatures;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCudaKernelLaunchFeaturesNV& safe_VkPhysicalDeviceCudaKernelLaunchFeaturesNV::operator=(
    const safe_VkPhysicalDeviceCudaKernelLaunchFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    cudaKernelLaunchFeatures = copy_src.cudaKernelLaunchFeatures;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCudaKernelLaunchFeaturesNV::~safe_VkPhysicalDeviceCudaKernelLaunchFeaturesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceCudaKernelLaunchFeaturesNV::initialize(const VkPhysicalDeviceCudaKernelLaunchFeaturesNV* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    cudaKernelLaunchFeatures = in_struct->cudaKernelLaunchFeatures;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCudaKernelLaunchFeaturesNV::initialize(const safe_VkPhysicalDeviceCudaKernelLaunchFeaturesNV* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    cudaKernelLaunchFeatures = copy_src->cudaKernelLaunchFeatures;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCudaKernelLaunchPropertiesNV::safe_VkPhysicalDeviceCudaKernelLaunchPropertiesNV(
    const VkPhysicalDeviceCudaKernelLaunchPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      computeCapabilityMinor(in_struct->computeCapabilityMinor),
      computeCapabilityMajor(in_struct->computeCapabilityMajor) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCudaKernelLaunchPropertiesNV::safe_VkPhysicalDeviceCudaKernelLaunchPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUDA_KERNEL_LAUNCH_PROPERTIES_NV),
      pNext(nullptr),
      computeCapabilityMinor(),
      computeCapabilityMajor() {}

safe_VkPhysicalDeviceCudaKernelLaunchPropertiesNV::safe_VkPhysicalDeviceCudaKernelLaunchPropertiesNV(
    const safe_VkPhysicalDeviceCudaKernelLaunchPropertiesNV& copy_src) {
    sType = copy_src.sType;
    computeCapabilityMinor = copy_src.computeCapabilityMinor;
    computeCapabilityMajor = copy_src.computeCapabilityMajor;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCudaKernelLaunchPropertiesNV& safe_VkPhysicalDeviceCudaKernelLaunchPropertiesNV::operator=(
    const safe_VkPhysicalDeviceCudaKernelLaunchPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    computeCapabilityMinor = copy_src.computeCapabilityMinor;
    computeCapabilityMajor = copy_src.computeCapabilityMajor;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCudaKernelLaunchPropertiesNV::~safe_VkPhysicalDeviceCudaKernelLaunchPropertiesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceCudaKernelLaunchPropertiesNV::initialize(const VkPhysicalDeviceCudaKernelLaunchPropertiesNV* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    computeCapabilityMinor = in_struct->computeCapabilityMinor;
    computeCapabilityMajor = in_struct->computeCapabilityMajor;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCudaKernelLaunchPropertiesNV::initialize(
    const safe_VkPhysicalDeviceCudaKernelLaunchPropertiesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    computeCapabilityMinor = copy_src->computeCapabilityMinor;
    computeCapabilityMajor = copy_src->computeCapabilityMajor;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkQueryLowLatencySupportNV::safe_VkQueryLowLatencySupportNV(const VkQueryLowLatencySupportNV* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pQueriedLowLatencyData(in_struct->pQueriedLowLatencyData) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkQueryLowLatencySupportNV::safe_VkQueryLowLatencySupportNV()
    : sType(VK_STRUCTURE_TYPE_QUERY_LOW_LATENCY_SUPPORT_NV), pNext(nullptr), pQueriedLowLatencyData(nullptr) {}

safe_VkQueryLowLatencySupportNV::safe_VkQueryLowLatencySupportNV(const safe_VkQueryLowLatencySupportNV& copy_src) {
    sType = copy_src.sType;
    pQueriedLowLatencyData = copy_src.pQueriedLowLatencyData;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkQueryLowLatencySupportNV& safe_VkQueryLowLatencySupportNV::operator=(const safe_VkQueryLowLatencySupportNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pQueriedLowLatencyData = copy_src.pQueriedLowLatencyData;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkQueryLowLatencySupportNV::~safe_VkQueryLowLatencySupportNV() { FreePnextChain(pNext); }

void safe_VkQueryLowLatencySupportNV::initialize(const VkQueryLowLatencySupportNV* in_struct,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pQueriedLowLatencyData = in_struct->pQueriedLowLatencyData;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkQueryLowLatencySupportNV::initialize(const safe_VkQueryLowLatencySupportNV* copy_src,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pQueriedLowLatencyData = copy_src->pQueriedLowLatencyData;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD::safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD(
    const VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), shaderEarlyAndLateFragmentTests(in_struct->shaderEarlyAndLateFragmentTests) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD::safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_EARLY_AND_LATE_FRAGMENT_TESTS_FEATURES_AMD),
      pNext(nullptr),
      shaderEarlyAndLateFragmentTests() {}

safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD::safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD(
    const safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD& copy_src) {
    sType = copy_src.sType;
    shaderEarlyAndLateFragmentTests = copy_src.shaderEarlyAndLateFragmentTests;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD&
safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD::operator=(
    const safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderEarlyAndLateFragmentTests = copy_src.shaderEarlyAndLateFragmentTests;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD::
    ~safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD::initialize(
    const VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderEarlyAndLateFragmentTests = in_struct->shaderEarlyAndLateFragmentTests;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD::initialize(
    const safe_VkPhysicalDeviceShaderEarlyAndLateFragmentTestsFeaturesAMD* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderEarlyAndLateFragmentTests = copy_src->shaderEarlyAndLateFragmentTests;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV::safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV(
    const VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      fragmentShadingRateEnums(in_struct->fragmentShadingRateEnums),
      supersampleFragmentShadingRates(in_struct->supersampleFragmentShadingRates),
      noInvocationFragmentShadingRates(in_struct->noInvocationFragmentShadingRates) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV::safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_FEATURES_NV),
      pNext(nullptr),
      fragmentShadingRateEnums(),
      supersampleFragmentShadingRates(),
      noInvocationFragmentShadingRates() {}

safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV::safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV(
    const safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV& copy_src) {
    sType = copy_src.sType;
    fragmentShadingRateEnums = copy_src.fragmentShadingRateEnums;
    supersampleFragmentShadingRates = copy_src.supersampleFragmentShadingRates;
    noInvocationFragmentShadingRates = copy_src.noInvocationFragmentShadingRates;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV& safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV::operator=(
    const safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fragmentShadingRateEnums = copy_src.fragmentShadingRateEnums;
    supersampleFragmentShadingRates = copy_src.supersampleFragmentShadingRates;
    noInvocationFragmentShadingRates = copy_src.noInvocationFragmentShadingRates;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV::~safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV::initialize(
    const VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fragmentShadingRateEnums = in_struct->fragmentShadingRateEnums;
    supersampleFragmentShadingRates = in_struct->supersampleFragmentShadingRates;
    noInvocationFragmentShadingRates = in_struct->noInvocationFragmentShadingRates;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV::initialize(
    const safe_VkPhysicalDeviceFragmentShadingRateEnumsFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fragmentShadingRateEnums = copy_src->fragmentShadingRateEnums;
    supersampleFragmentShadingRates = copy_src->supersampleFragmentShadingRates;
    noInvocationFragmentShadingRates = copy_src->noInvocationFragmentShadingRates;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV::safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV(
    const VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), maxFragmentShadingRateInvocationCount(in_struct->maxFragmentShadingRateInvocationCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV::safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_SHADING_RATE_ENUMS_PROPERTIES_NV),
      pNext(nullptr),
      maxFragmentShadingRateInvocationCount() {}

safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV::safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV(
    const safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV& copy_src) {
    sType = copy_src.sType;
    maxFragmentShadingRateInvocationCount = copy_src.maxFragmentShadingRateInvocationCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV& safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV::operator=(
    const safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxFragmentShadingRateInvocationCount = copy_src.maxFragmentShadingRateInvocationCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV::~safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV::initialize(
    const VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxFragmentShadingRateInvocationCount = in_struct->maxFragmentShadingRateInvocationCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV::initialize(
    const safe_VkPhysicalDeviceFragmentShadingRateEnumsPropertiesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxFragmentShadingRateInvocationCount = copy_src->maxFragmentShadingRateInvocationCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV::safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV(
    const VkPipelineFragmentShadingRateEnumStateCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), shadingRateType(in_struct->shadingRateType), shadingRate(in_struct->shadingRate) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < 2; ++i) {
        combinerOps[i] = in_struct->combinerOps[i];
    }
}

safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV::safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_FRAGMENT_SHADING_RATE_ENUM_STATE_CREATE_INFO_NV),
      pNext(nullptr),
      shadingRateType(),
      shadingRate() {}

safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV::safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV(
    const safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    shadingRateType = copy_src.shadingRateType;
    shadingRate = copy_src.shadingRate;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < 2; ++i) {
        combinerOps[i] = copy_src.combinerOps[i];
    }
}

safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV& safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV::operator=(
    const safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shadingRateType = copy_src.shadingRateType;
    shadingRate = copy_src.shadingRate;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < 2; ++i) {
        combinerOps[i] = copy_src.combinerOps[i];
    }

    return *this;
}

safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV::~safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV() {
    FreePnextChain(pNext);
}

void safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV::initialize(
    const VkPipelineFragmentShadingRateEnumStateCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shadingRateType = in_struct->shadingRateType;
    shadingRate = in_struct->shadingRate;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < 2; ++i) {
        combinerOps[i] = in_struct->combinerOps[i];
    }
}

void safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV::initialize(
    const safe_VkPipelineFragmentShadingRateEnumStateCreateInfoNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shadingRateType = copy_src->shadingRateType;
    shadingRate = copy_src->shadingRate;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < 2; ++i) {
        combinerOps[i] = copy_src->combinerOps[i];
    }
}

safe_VkAccelerationStructureGeometryMotionTrianglesDataNV::safe_VkAccelerationStructureGeometryMotionTrianglesDataNV(
    const VkAccelerationStructureGeometryMotionTrianglesDataNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), vertexData(&in_struct->vertexData) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAccelerationStructureGeometryMotionTrianglesDataNV::safe_VkAccelerationStructureGeometryMotionTrianglesDataNV()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_MOTION_TRIANGLES_DATA_NV), pNext(nullptr) {}

safe_VkAccelerationStructureGeometryMotionTrianglesDataNV::safe_VkAccelerationStructureGeometryMotionTrianglesDataNV(
    const safe_VkAccelerationStructureGeometryMotionTrianglesDataNV& copy_src) {
    sType = copy_src.sType;
    vertexData.initialize(&copy_src.vertexData);
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureGeometryMotionTrianglesDataNV& safe_VkAccelerationStructureGeometryMotionTrianglesDataNV::operator=(
    const safe_VkAccelerationStructureGeometryMotionTrianglesDataNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    vertexData.initialize(&copy_src.vertexData);
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAccelerationStructureGeometryMotionTrianglesDataNV::~safe_VkAccelerationStructureGeometryMotionTrianglesDataNV() {
    FreePnextChain(pNext);
}

void safe_VkAccelerationStructureGeometryMotionTrianglesDataNV::initialize(
    const VkAccelerationStructureGeometryMotionTrianglesDataNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    vertexData.initialize(&in_struct->vertexData);
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAccelerationStructureGeometryMotionTrianglesDataNV::initialize(
    const safe_VkAccelerationStructureGeometryMotionTrianglesDataNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    vertexData.initialize(&copy_src->vertexData);
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAccelerationStructureMotionInfoNV::safe_VkAccelerationStructureMotionInfoNV(
    const VkAccelerationStructureMotionInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), maxInstances(in_struct->maxInstances), flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAccelerationStructureMotionInfoNV::safe_VkAccelerationStructureMotionInfoNV()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MOTION_INFO_NV), pNext(nullptr), maxInstances(), flags() {}

safe_VkAccelerationStructureMotionInfoNV::safe_VkAccelerationStructureMotionInfoNV(
    const safe_VkAccelerationStructureMotionInfoNV& copy_src) {
    sType = copy_src.sType;
    maxInstances = copy_src.maxInstances;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureMotionInfoNV& safe_VkAccelerationStructureMotionInfoNV::operator=(
    const safe_VkAccelerationStructureMotionInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxInstances = copy_src.maxInstances;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAccelerationStructureMotionInfoNV::~safe_VkAccelerationStructureMotionInfoNV() { FreePnextChain(pNext); }

void safe_VkAccelerationStructureMotionInfoNV::initialize(const VkAccelerationStructureMotionInfoNV* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxInstances = in_struct->maxInstances;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAccelerationStructureMotionInfoNV::initialize(const safe_VkAccelerationStructureMotionInfoNV* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxInstances = copy_src->maxInstances;
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV::safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV(
    const VkPhysicalDeviceRayTracingMotionBlurFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      rayTracingMotionBlur(in_struct->rayTracingMotionBlur),
      rayTracingMotionBlurPipelineTraceRaysIndirect(in_struct->rayTracingMotionBlurPipelineTraceRaysIndirect) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV::safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_MOTION_BLUR_FEATURES_NV),
      pNext(nullptr),
      rayTracingMotionBlur(),
      rayTracingMotionBlurPipelineTraceRaysIndirect() {}

safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV::safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV(
    const safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV& copy_src) {
    sType = copy_src.sType;
    rayTracingMotionBlur = copy_src.rayTracingMotionBlur;
    rayTracingMotionBlurPipelineTraceRaysIndirect = copy_src.rayTracingMotionBlurPipelineTraceRaysIndirect;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV& safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV::operator=(
    const safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    rayTracingMotionBlur = copy_src.rayTracingMotionBlur;
    rayTracingMotionBlurPipelineTraceRaysIndirect = copy_src.rayTracingMotionBlurPipelineTraceRaysIndirect;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV::~safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV::initialize(
    const VkPhysicalDeviceRayTracingMotionBlurFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    rayTracingMotionBlur = in_struct->rayTracingMotionBlur;
    rayTracingMotionBlurPipelineTraceRaysIndirect = in_struct->rayTracingMotionBlurPipelineTraceRaysIndirect;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV::initialize(
    const safe_VkPhysicalDeviceRayTracingMotionBlurFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    rayTracingMotionBlur = copy_src->rayTracingMotionBlur;
    rayTracingMotionBlurPipelineTraceRaysIndirect = copy_src->rayTracingMotionBlurPipelineTraceRaysIndirect;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCopyCommandTransformInfoQCOM::safe_VkCopyCommandTransformInfoQCOM(const VkCopyCommandTransformInfoQCOM* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), transform(in_struct->transform) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCopyCommandTransformInfoQCOM::safe_VkCopyCommandTransformInfoQCOM()
    : sType(VK_STRUCTURE_TYPE_COPY_COMMAND_TRANSFORM_INFO_QCOM), pNext(nullptr), transform() {}

safe_VkCopyCommandTransformInfoQCOM::safe_VkCopyCommandTransformInfoQCOM(const safe_VkCopyCommandTransformInfoQCOM& copy_src) {
    sType = copy_src.sType;
    transform = copy_src.transform;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCopyCommandTransformInfoQCOM& safe_VkCopyCommandTransformInfoQCOM::operator=(
    const safe_VkCopyCommandTransformInfoQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    transform = copy_src.transform;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCopyCommandTransformInfoQCOM::~safe_VkCopyCommandTransformInfoQCOM() { FreePnextChain(pNext); }

void safe_VkCopyCommandTransformInfoQCOM::initialize(const VkCopyCommandTransformInfoQCOM* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    transform = in_struct->transform;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCopyCommandTransformInfoQCOM::initialize(const safe_VkCopyCommandTransformInfoQCOM* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    transform = copy_src->transform;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_USE_PLATFORM_FUCHSIA

safe_VkImportMemoryZirconHandleInfoFUCHSIA::safe_VkImportMemoryZirconHandleInfoFUCHSIA(
    const VkImportMemoryZirconHandleInfoFUCHSIA* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), handleType(in_struct->handleType), handle(in_struct->handle) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportMemoryZirconHandleInfoFUCHSIA::safe_VkImportMemoryZirconHandleInfoFUCHSIA()
    : sType(VK_STRUCTURE_TYPE_IMPORT_MEMORY_ZIRCON_HANDLE_INFO_FUCHSIA), pNext(nullptr), handleType(), handle() {}

safe_VkImportMemoryZirconHandleInfoFUCHSIA::safe_VkImportMemoryZirconHandleInfoFUCHSIA(
    const safe_VkImportMemoryZirconHandleInfoFUCHSIA& copy_src) {
    sType = copy_src.sType;
    handleType = copy_src.handleType;
    handle = copy_src.handle;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportMemoryZirconHandleInfoFUCHSIA& safe_VkImportMemoryZirconHandleInfoFUCHSIA::operator=(
    const safe_VkImportMemoryZirconHandleInfoFUCHSIA& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    handleType = copy_src.handleType;
    handle = copy_src.handle;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportMemoryZirconHandleInfoFUCHSIA::~safe_VkImportMemoryZirconHandleInfoFUCHSIA() { FreePnextChain(pNext); }

void safe_VkImportMemoryZirconHandleInfoFUCHSIA::initialize(const VkImportMemoryZirconHandleInfoFUCHSIA* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    handleType = in_struct->handleType;
    handle = in_struct->handle;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportMemoryZirconHandleInfoFUCHSIA::initialize(const safe_VkImportMemoryZirconHandleInfoFUCHSIA* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    handleType = copy_src->handleType;
    handle = copy_src->handle;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMemoryZirconHandlePropertiesFUCHSIA::safe_VkMemoryZirconHandlePropertiesFUCHSIA(
    const VkMemoryZirconHandlePropertiesFUCHSIA* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), memoryTypeBits(in_struct->memoryTypeBits) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryZirconHandlePropertiesFUCHSIA::safe_VkMemoryZirconHandlePropertiesFUCHSIA()
    : sType(VK_STRUCTURE_TYPE_MEMORY_ZIRCON_HANDLE_PROPERTIES_FUCHSIA), pNext(nullptr), memoryTypeBits() {}

safe_VkMemoryZirconHandlePropertiesFUCHSIA::safe_VkMemoryZirconHandlePropertiesFUCHSIA(
    const safe_VkMemoryZirconHandlePropertiesFUCHSIA& copy_src) {
    sType = copy_src.sType;
    memoryTypeBits = copy_src.memoryTypeBits;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryZirconHandlePropertiesFUCHSIA& safe_VkMemoryZirconHandlePropertiesFUCHSIA::operator=(
    const safe_VkMemoryZirconHandlePropertiesFUCHSIA& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memoryTypeBits = copy_src.memoryTypeBits;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryZirconHandlePropertiesFUCHSIA::~safe_VkMemoryZirconHandlePropertiesFUCHSIA() { FreePnextChain(pNext); }

void safe_VkMemoryZirconHandlePropertiesFUCHSIA::initialize(const VkMemoryZirconHandlePropertiesFUCHSIA* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memoryTypeBits = in_struct->memoryTypeBits;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryZirconHandlePropertiesFUCHSIA::initialize(const safe_VkMemoryZirconHandlePropertiesFUCHSIA* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memoryTypeBits = copy_src->memoryTypeBits;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMemoryGetZirconHandleInfoFUCHSIA::safe_VkMemoryGetZirconHandleInfoFUCHSIA(
    const VkMemoryGetZirconHandleInfoFUCHSIA* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), memory(in_struct->memory), handleType(in_struct->handleType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryGetZirconHandleInfoFUCHSIA::safe_VkMemoryGetZirconHandleInfoFUCHSIA()
    : sType(VK_STRUCTURE_TYPE_MEMORY_GET_ZIRCON_HANDLE_INFO_FUCHSIA), pNext(nullptr), memory(), handleType() {}

safe_VkMemoryGetZirconHandleInfoFUCHSIA::safe_VkMemoryGetZirconHandleInfoFUCHSIA(
    const safe_VkMemoryGetZirconHandleInfoFUCHSIA& copy_src) {
    sType = copy_src.sType;
    memory = copy_src.memory;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryGetZirconHandleInfoFUCHSIA& safe_VkMemoryGetZirconHandleInfoFUCHSIA::operator=(
    const safe_VkMemoryGetZirconHandleInfoFUCHSIA& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memory = copy_src.memory;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryGetZirconHandleInfoFUCHSIA::~safe_VkMemoryGetZirconHandleInfoFUCHSIA() { FreePnextChain(pNext); }

void safe_VkMemoryGetZirconHandleInfoFUCHSIA::initialize(const VkMemoryGetZirconHandleInfoFUCHSIA* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memory = in_struct->memory;
    handleType = in_struct->handleType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryGetZirconHandleInfoFUCHSIA::initialize(const safe_VkMemoryGetZirconHandleInfoFUCHSIA* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memory = copy_src->memory;
    handleType = copy_src->handleType;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImportSemaphoreZirconHandleInfoFUCHSIA::safe_VkImportSemaphoreZirconHandleInfoFUCHSIA(
    const VkImportSemaphoreZirconHandleInfoFUCHSIA* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      semaphore(in_struct->semaphore),
      flags(in_struct->flags),
      handleType(in_struct->handleType),
      zirconHandle(in_struct->zirconHandle) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportSemaphoreZirconHandleInfoFUCHSIA::safe_VkImportSemaphoreZirconHandleInfoFUCHSIA()
    : sType(VK_STRUCTURE_TYPE_IMPORT_SEMAPHORE_ZIRCON_HANDLE_INFO_FUCHSIA),
      pNext(nullptr),
      semaphore(),
      flags(),
      handleType(),
      zirconHandle() {}

safe_VkImportSemaphoreZirconHandleInfoFUCHSIA::safe_VkImportSemaphoreZirconHandleInfoFUCHSIA(
    const safe_VkImportSemaphoreZirconHandleInfoFUCHSIA& copy_src) {
    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    flags = copy_src.flags;
    handleType = copy_src.handleType;
    zirconHandle = copy_src.zirconHandle;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportSemaphoreZirconHandleInfoFUCHSIA& safe_VkImportSemaphoreZirconHandleInfoFUCHSIA::operator=(
    const safe_VkImportSemaphoreZirconHandleInfoFUCHSIA& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    flags = copy_src.flags;
    handleType = copy_src.handleType;
    zirconHandle = copy_src.zirconHandle;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportSemaphoreZirconHandleInfoFUCHSIA::~safe_VkImportSemaphoreZirconHandleInfoFUCHSIA() { FreePnextChain(pNext); }

void safe_VkImportSemaphoreZirconHandleInfoFUCHSIA::initialize(const VkImportSemaphoreZirconHandleInfoFUCHSIA* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    semaphore = in_struct->semaphore;
    flags = in_struct->flags;
    handleType = in_struct->handleType;
    zirconHandle = in_struct->zirconHandle;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportSemaphoreZirconHandleInfoFUCHSIA::initialize(const safe_VkImportSemaphoreZirconHandleInfoFUCHSIA* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    semaphore = copy_src->semaphore;
    flags = copy_src->flags;
    handleType = copy_src->handleType;
    zirconHandle = copy_src->zirconHandle;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSemaphoreGetZirconHandleInfoFUCHSIA::safe_VkSemaphoreGetZirconHandleInfoFUCHSIA(
    const VkSemaphoreGetZirconHandleInfoFUCHSIA* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), semaphore(in_struct->semaphore), handleType(in_struct->handleType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSemaphoreGetZirconHandleInfoFUCHSIA::safe_VkSemaphoreGetZirconHandleInfoFUCHSIA()
    : sType(VK_STRUCTURE_TYPE_SEMAPHORE_GET_ZIRCON_HANDLE_INFO_FUCHSIA), pNext(nullptr), semaphore(), handleType() {}

safe_VkSemaphoreGetZirconHandleInfoFUCHSIA::safe_VkSemaphoreGetZirconHandleInfoFUCHSIA(
    const safe_VkSemaphoreGetZirconHandleInfoFUCHSIA& copy_src) {
    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSemaphoreGetZirconHandleInfoFUCHSIA& safe_VkSemaphoreGetZirconHandleInfoFUCHSIA::operator=(
    const safe_VkSemaphoreGetZirconHandleInfoFUCHSIA& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    semaphore = copy_src.semaphore;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSemaphoreGetZirconHandleInfoFUCHSIA::~safe_VkSemaphoreGetZirconHandleInfoFUCHSIA() { FreePnextChain(pNext); }

void safe_VkSemaphoreGetZirconHandleInfoFUCHSIA::initialize(const VkSemaphoreGetZirconHandleInfoFUCHSIA* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    semaphore = in_struct->semaphore;
    handleType = in_struct->handleType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSemaphoreGetZirconHandleInfoFUCHSIA::initialize(const safe_VkSemaphoreGetZirconHandleInfoFUCHSIA* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    semaphore = copy_src->semaphore;
    handleType = copy_src->handleType;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkBufferCollectionCreateInfoFUCHSIA::safe_VkBufferCollectionCreateInfoFUCHSIA(
    const VkBufferCollectionCreateInfoFUCHSIA* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), collectionToken(in_struct->collectionToken) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkBufferCollectionCreateInfoFUCHSIA::safe_VkBufferCollectionCreateInfoFUCHSIA()
    : sType(VK_STRUCTURE_TYPE_BUFFER_COLLECTION_CREATE_INFO_FUCHSIA), pNext(nullptr), collectionToken() {}

safe_VkBufferCollectionCreateInfoFUCHSIA::safe_VkBufferCollectionCreateInfoFUCHSIA(
    const safe_VkBufferCollectionCreateInfoFUCHSIA& copy_src) {
    sType = copy_src.sType;
    collectionToken = copy_src.collectionToken;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkBufferCollectionCreateInfoFUCHSIA& safe_VkBufferCollectionCreateInfoFUCHSIA::operator=(
    const safe_VkBufferCollectionCreateInfoFUCHSIA& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    collectionToken = copy_src.collectionToken;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkBufferCollectionCreateInfoFUCHSIA::~safe_VkBufferCollectionCreateInfoFUCHSIA() { FreePnextChain(pNext); }

void safe_VkBufferCollectionCreateInfoFUCHSIA::initialize(const VkBufferCollectionCreateInfoFUCHSIA* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    collectionToken = in_struct->collectionToken;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkBufferCollectionCreateInfoFUCHSIA::initialize(const safe_VkBufferCollectionCreateInfoFUCHSIA* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    collectionToken = copy_src->collectionToken;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImportMemoryBufferCollectionFUCHSIA::safe_VkImportMemoryBufferCollectionFUCHSIA(
    const VkImportMemoryBufferCollectionFUCHSIA* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), collection(in_struct->collection), index(in_struct->index) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImportMemoryBufferCollectionFUCHSIA::safe_VkImportMemoryBufferCollectionFUCHSIA()
    : sType(VK_STRUCTURE_TYPE_IMPORT_MEMORY_BUFFER_COLLECTION_FUCHSIA), pNext(nullptr), collection(), index() {}

safe_VkImportMemoryBufferCollectionFUCHSIA::safe_VkImportMemoryBufferCollectionFUCHSIA(
    const safe_VkImportMemoryBufferCollectionFUCHSIA& copy_src) {
    sType = copy_src.sType;
    collection = copy_src.collection;
    index = copy_src.index;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImportMemoryBufferCollectionFUCHSIA& safe_VkImportMemoryBufferCollectionFUCHSIA::operator=(
    const safe_VkImportMemoryBufferCollectionFUCHSIA& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    collection = copy_src.collection;
    index = copy_src.index;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImportMemoryBufferCollectionFUCHSIA::~safe_VkImportMemoryBufferCollectionFUCHSIA() { FreePnextChain(pNext); }

void safe_VkImportMemoryBufferCollectionFUCHSIA::initialize(const VkImportMemoryBufferCollectionFUCHSIA* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    collection = in_struct->collection;
    index = in_struct->index;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImportMemoryBufferCollectionFUCHSIA::initialize(const safe_VkImportMemoryBufferCollectionFUCHSIA* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    collection = copy_src->collection;
    index = copy_src->index;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkBufferCollectionImageCreateInfoFUCHSIA::safe_VkBufferCollectionImageCreateInfoFUCHSIA(
    const VkBufferCollectionImageCreateInfoFUCHSIA* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), collection(in_struct->collection), index(in_struct->index) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkBufferCollectionImageCreateInfoFUCHSIA::safe_VkBufferCollectionImageCreateInfoFUCHSIA()
    : sType(VK_STRUCTURE_TYPE_BUFFER_COLLECTION_IMAGE_CREATE_INFO_FUCHSIA), pNext(nullptr), collection(), index() {}

safe_VkBufferCollectionImageCreateInfoFUCHSIA::safe_VkBufferCollectionImageCreateInfoFUCHSIA(
    const safe_VkBufferCollectionImageCreateInfoFUCHSIA& copy_src) {
    sType = copy_src.sType;
    collection = copy_src.collection;
    index = copy_src.index;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkBufferCollectionImageCreateInfoFUCHSIA& safe_VkBufferCollectionImageCreateInfoFUCHSIA::operator=(
    const safe_VkBufferCollectionImageCreateInfoFUCHSIA& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    collection = copy_src.collection;
    index = copy_src.index;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkBufferCollectionImageCreateInfoFUCHSIA::~safe_VkBufferCollectionImageCreateInfoFUCHSIA() { FreePnextChain(pNext); }

void safe_VkBufferCollectionImageCreateInfoFUCHSIA::initialize(const VkBufferCollectionImageCreateInfoFUCHSIA* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    collection = in_struct->collection;
    index = in_struct->index;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkBufferCollectionImageCreateInfoFUCHSIA::initialize(const safe_VkBufferCollectionImageCreateInfoFUCHSIA* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    collection = copy_src->collection;
    index = copy_src->index;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkBufferCollectionConstraintsInfoFUCHSIA::safe_VkBufferCollectionConstraintsInfoFUCHSIA(
    const VkBufferCollectionConstraintsInfoFUCHSIA* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      minBufferCount(in_struct->minBufferCount),
      maxBufferCount(in_struct->maxBufferCount),
      minBufferCountForCamping(in_struct->minBufferCountForCamping),
      minBufferCountForDedicatedSlack(in_struct->minBufferCountForDedicatedSlack),
      minBufferCountForSharedSlack(in_struct->minBufferCountForSharedSlack) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkBufferCollectionConstraintsInfoFUCHSIA::safe_VkBufferCollectionConstraintsInfoFUCHSIA()
    : sType(VK_STRUCTURE_TYPE_BUFFER_COLLECTION_CONSTRAINTS_INFO_FUCHSIA),
      pNext(nullptr),
      minBufferCount(),
      maxBufferCount(),
      minBufferCountForCamping(),
      minBufferCountForDedicatedSlack(),
      minBufferCountForSharedSlack() {}

safe_VkBufferCollectionConstraintsInfoFUCHSIA::safe_VkBufferCollectionConstraintsInfoFUCHSIA(
    const safe_VkBufferCollectionConstraintsInfoFUCHSIA& copy_src) {
    sType = copy_src.sType;
    minBufferCount = copy_src.minBufferCount;
    maxBufferCount = copy_src.maxBufferCount;
    minBufferCountForCamping = copy_src.minBufferCountForCamping;
    minBufferCountForDedicatedSlack = copy_src.minBufferCountForDedicatedSlack;
    minBufferCountForSharedSlack = copy_src.minBufferCountForSharedSlack;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkBufferCollectionConstraintsInfoFUCHSIA& safe_VkBufferCollectionConstraintsInfoFUCHSIA::operator=(
    const safe_VkBufferCollectionConstraintsInfoFUCHSIA& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    minBufferCount = copy_src.minBufferCount;
    maxBufferCount = copy_src.maxBufferCount;
    minBufferCountForCamping = copy_src.minBufferCountForCamping;
    minBufferCountForDedicatedSlack = copy_src.minBufferCountForDedicatedSlack;
    minBufferCountForSharedSlack = copy_src.minBufferCountForSharedSlack;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkBufferCollectionConstraintsInfoFUCHSIA::~safe_VkBufferCollectionConstraintsInfoFUCHSIA() { FreePnextChain(pNext); }

void safe_VkBufferCollectionConstraintsInfoFUCHSIA::initialize(const VkBufferCollectionConstraintsInfoFUCHSIA* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    minBufferCount = in_struct->minBufferCount;
    maxBufferCount = in_struct->maxBufferCount;
    minBufferCountForCamping = in_struct->minBufferCountForCamping;
    minBufferCountForDedicatedSlack = in_struct->minBufferCountForDedicatedSlack;
    minBufferCountForSharedSlack = in_struct->minBufferCountForSharedSlack;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkBufferCollectionConstraintsInfoFUCHSIA::initialize(const safe_VkBufferCollectionConstraintsInfoFUCHSIA* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    minBufferCount = copy_src->minBufferCount;
    maxBufferCount = copy_src->maxBufferCount;
    minBufferCountForCamping = copy_src->minBufferCountForCamping;
    minBufferCountForDedicatedSlack = copy_src->minBufferCountForDedicatedSlack;
    minBufferCountForSharedSlack = copy_src->minBufferCountForSharedSlack;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkBufferConstraintsInfoFUCHSIA::safe_VkBufferConstraintsInfoFUCHSIA(const VkBufferConstraintsInfoFUCHSIA* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType),
      createInfo(&in_struct->createInfo),
      requiredFormatFeatures(in_struct->requiredFormatFeatures),
      bufferCollectionConstraints(&in_struct->bufferCollectionConstraints) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkBufferConstraintsInfoFUCHSIA::safe_VkBufferConstraintsInfoFUCHSIA()
    : sType(VK_STRUCTURE_TYPE_BUFFER_CONSTRAINTS_INFO_FUCHSIA), pNext(nullptr), requiredFormatFeatures() {}

safe_VkBufferConstraintsInfoFUCHSIA::safe_VkBufferConstraintsInfoFUCHSIA(const safe_VkBufferConstraintsInfoFUCHSIA& copy_src) {
    sType = copy_src.sType;
    createInfo.initialize(&copy_src.createInfo);
    requiredFormatFeatures = copy_src.requiredFormatFeatures;
    bufferCollectionConstraints.initialize(&copy_src.bufferCollectionConstraints);
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkBufferConstraintsInfoFUCHSIA& safe_VkBufferConstraintsInfoFUCHSIA::operator=(
    const safe_VkBufferConstraintsInfoFUCHSIA& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    createInfo.initialize(&copy_src.createInfo);
    requiredFormatFeatures = copy_src.requiredFormatFeatures;
    bufferCollectionConstraints.initialize(&copy_src.bufferCollectionConstraints);
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkBufferConstraintsInfoFUCHSIA::~safe_VkBufferConstraintsInfoFUCHSIA() { FreePnextChain(pNext); }

void safe_VkBufferConstraintsInfoFUCHSIA::initialize(const VkBufferConstraintsInfoFUCHSIA* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    createInfo.initialize(&in_struct->createInfo);
    requiredFormatFeatures = in_struct->requiredFormatFeatures;
    bufferCollectionConstraints.initialize(&in_struct->bufferCollectionConstraints);
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkBufferConstraintsInfoFUCHSIA::initialize(const safe_VkBufferConstraintsInfoFUCHSIA* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    createInfo.initialize(&copy_src->createInfo);
    requiredFormatFeatures = copy_src->requiredFormatFeatures;
    bufferCollectionConstraints.initialize(&copy_src->bufferCollectionConstraints);
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkBufferCollectionBufferCreateInfoFUCHSIA::safe_VkBufferCollectionBufferCreateInfoFUCHSIA(
    const VkBufferCollectionBufferCreateInfoFUCHSIA* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), collection(in_struct->collection), index(in_struct->index) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkBufferCollectionBufferCreateInfoFUCHSIA::safe_VkBufferCollectionBufferCreateInfoFUCHSIA()
    : sType(VK_STRUCTURE_TYPE_BUFFER_COLLECTION_BUFFER_CREATE_INFO_FUCHSIA), pNext(nullptr), collection(), index() {}

safe_VkBufferCollectionBufferCreateInfoFUCHSIA::safe_VkBufferCollectionBufferCreateInfoFUCHSIA(
    const safe_VkBufferCollectionBufferCreateInfoFUCHSIA& copy_src) {
    sType = copy_src.sType;
    collection = copy_src.collection;
    index = copy_src.index;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkBufferCollectionBufferCreateInfoFUCHSIA& safe_VkBufferCollectionBufferCreateInfoFUCHSIA::operator=(
    const safe_VkBufferCollectionBufferCreateInfoFUCHSIA& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    collection = copy_src.collection;
    index = copy_src.index;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkBufferCollectionBufferCreateInfoFUCHSIA::~safe_VkBufferCollectionBufferCreateInfoFUCHSIA() { FreePnextChain(pNext); }

void safe_VkBufferCollectionBufferCreateInfoFUCHSIA::initialize(const VkBufferCollectionBufferCreateInfoFUCHSIA* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    collection = in_struct->collection;
    index = in_struct->index;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkBufferCollectionBufferCreateInfoFUCHSIA::initialize(const safe_VkBufferCollectionBufferCreateInfoFUCHSIA* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    collection = copy_src->collection;
    index = copy_src->index;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSysmemColorSpaceFUCHSIA::safe_VkSysmemColorSpaceFUCHSIA(const VkSysmemColorSpaceFUCHSIA* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), colorSpace(in_struct->colorSpace) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSysmemColorSpaceFUCHSIA::safe_VkSysmemColorSpaceFUCHSIA()
    : sType(VK_STRUCTURE_TYPE_SYSMEM_COLOR_SPACE_FUCHSIA), pNext(nullptr), colorSpace() {}

safe_VkSysmemColorSpaceFUCHSIA::safe_VkSysmemColorSpaceFUCHSIA(const safe_VkSysmemColorSpaceFUCHSIA& copy_src) {
    sType = copy_src.sType;
    colorSpace = copy_src.colorSpace;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSysmemColorSpaceFUCHSIA& safe_VkSysmemColorSpaceFUCHSIA::operator=(const safe_VkSysmemColorSpaceFUCHSIA& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    colorSpace = copy_src.colorSpace;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSysmemColorSpaceFUCHSIA::~safe_VkSysmemColorSpaceFUCHSIA() { FreePnextChain(pNext); }

void safe_VkSysmemColorSpaceFUCHSIA::initialize(const VkSysmemColorSpaceFUCHSIA* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    colorSpace = in_struct->colorSpace;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSysmemColorSpaceFUCHSIA::initialize(const safe_VkSysmemColorSpaceFUCHSIA* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    colorSpace = copy_src->colorSpace;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkBufferCollectionPropertiesFUCHSIA::safe_VkBufferCollectionPropertiesFUCHSIA(
    const VkBufferCollectionPropertiesFUCHSIA* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      memoryTypeBits(in_struct->memoryTypeBits),
      bufferCount(in_struct->bufferCount),
      createInfoIndex(in_struct->createInfoIndex),
      sysmemPixelFormat(in_struct->sysmemPixelFormat),
      formatFeatures(in_struct->formatFeatures),
      sysmemColorSpaceIndex(&in_struct->sysmemColorSpaceIndex),
      samplerYcbcrConversionComponents(in_struct->samplerYcbcrConversionComponents),
      suggestedYcbcrModel(in_struct->suggestedYcbcrModel),
      suggestedYcbcrRange(in_struct->suggestedYcbcrRange),
      suggestedXChromaOffset(in_struct->suggestedXChromaOffset),
      suggestedYChromaOffset(in_struct->suggestedYChromaOffset) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkBufferCollectionPropertiesFUCHSIA::safe_VkBufferCollectionPropertiesFUCHSIA()
    : sType(VK_STRUCTURE_TYPE_BUFFER_COLLECTION_PROPERTIES_FUCHSIA),
      pNext(nullptr),
      memoryTypeBits(),
      bufferCount(),
      createInfoIndex(),
      sysmemPixelFormat(),
      formatFeatures(),
      samplerYcbcrConversionComponents(),
      suggestedYcbcrModel(),
      suggestedYcbcrRange(),
      suggestedXChromaOffset(),
      suggestedYChromaOffset() {}

safe_VkBufferCollectionPropertiesFUCHSIA::safe_VkBufferCollectionPropertiesFUCHSIA(
    const safe_VkBufferCollectionPropertiesFUCHSIA& copy_src) {
    sType = copy_src.sType;
    memoryTypeBits = copy_src.memoryTypeBits;
    bufferCount = copy_src.bufferCount;
    createInfoIndex = copy_src.createInfoIndex;
    sysmemPixelFormat = copy_src.sysmemPixelFormat;
    formatFeatures = copy_src.formatFeatures;
    sysmemColorSpaceIndex.initialize(&copy_src.sysmemColorSpaceIndex);
    samplerYcbcrConversionComponents = copy_src.samplerYcbcrConversionComponents;
    suggestedYcbcrModel = copy_src.suggestedYcbcrModel;
    suggestedYcbcrRange = copy_src.suggestedYcbcrRange;
    suggestedXChromaOffset = copy_src.suggestedXChromaOffset;
    suggestedYChromaOffset = copy_src.suggestedYChromaOffset;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkBufferCollectionPropertiesFUCHSIA& safe_VkBufferCollectionPropertiesFUCHSIA::operator=(
    const safe_VkBufferCollectionPropertiesFUCHSIA& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memoryTypeBits = copy_src.memoryTypeBits;
    bufferCount = copy_src.bufferCount;
    createInfoIndex = copy_src.createInfoIndex;
    sysmemPixelFormat = copy_src.sysmemPixelFormat;
    formatFeatures = copy_src.formatFeatures;
    sysmemColorSpaceIndex.initialize(&copy_src.sysmemColorSpaceIndex);
    samplerYcbcrConversionComponents = copy_src.samplerYcbcrConversionComponents;
    suggestedYcbcrModel = copy_src.suggestedYcbcrModel;
    suggestedYcbcrRange = copy_src.suggestedYcbcrRange;
    suggestedXChromaOffset = copy_src.suggestedXChromaOffset;
    suggestedYChromaOffset = copy_src.suggestedYChromaOffset;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkBufferCollectionPropertiesFUCHSIA::~safe_VkBufferCollectionPropertiesFUCHSIA() { FreePnextChain(pNext); }

void safe_VkBufferCollectionPropertiesFUCHSIA::initialize(const VkBufferCollectionPropertiesFUCHSIA* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memoryTypeBits = in_struct->memoryTypeBits;
    bufferCount = in_struct->bufferCount;
    createInfoIndex = in_struct->createInfoIndex;
    sysmemPixelFormat = in_struct->sysmemPixelFormat;
    formatFeatures = in_struct->formatFeatures;
    sysmemColorSpaceIndex.initialize(&in_struct->sysmemColorSpaceIndex);
    samplerYcbcrConversionComponents = in_struct->samplerYcbcrConversionComponents;
    suggestedYcbcrModel = in_struct->suggestedYcbcrModel;
    suggestedYcbcrRange = in_struct->suggestedYcbcrRange;
    suggestedXChromaOffset = in_struct->suggestedXChromaOffset;
    suggestedYChromaOffset = in_struct->suggestedYChromaOffset;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkBufferCollectionPropertiesFUCHSIA::initialize(const safe_VkBufferCollectionPropertiesFUCHSIA* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memoryTypeBits = copy_src->memoryTypeBits;
    bufferCount = copy_src->bufferCount;
    createInfoIndex = copy_src->createInfoIndex;
    sysmemPixelFormat = copy_src->sysmemPixelFormat;
    formatFeatures = copy_src->formatFeatures;
    sysmemColorSpaceIndex.initialize(&copy_src->sysmemColorSpaceIndex);
    samplerYcbcrConversionComponents = copy_src->samplerYcbcrConversionComponents;
    suggestedYcbcrModel = copy_src->suggestedYcbcrModel;
    suggestedYcbcrRange = copy_src->suggestedYcbcrRange;
    suggestedXChromaOffset = copy_src->suggestedXChromaOffset;
    suggestedYChromaOffset = copy_src->suggestedYChromaOffset;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImageFormatConstraintsInfoFUCHSIA::safe_VkImageFormatConstraintsInfoFUCHSIA(
    const VkImageFormatConstraintsInfoFUCHSIA* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      imageCreateInfo(&in_struct->imageCreateInfo),
      requiredFormatFeatures(in_struct->requiredFormatFeatures),
      flags(in_struct->flags),
      sysmemPixelFormat(in_struct->sysmemPixelFormat),
      colorSpaceCount(in_struct->colorSpaceCount),
      pColorSpaces(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (colorSpaceCount && in_struct->pColorSpaces) {
        pColorSpaces = new safe_VkSysmemColorSpaceFUCHSIA[colorSpaceCount];
        for (uint32_t i = 0; i < colorSpaceCount; ++i) {
            pColorSpaces[i].initialize(&in_struct->pColorSpaces[i]);
        }
    }
}

safe_VkImageFormatConstraintsInfoFUCHSIA::safe_VkImageFormatConstraintsInfoFUCHSIA()
    : sType(VK_STRUCTURE_TYPE_IMAGE_FORMAT_CONSTRAINTS_INFO_FUCHSIA),
      pNext(nullptr),
      requiredFormatFeatures(),
      flags(),
      sysmemPixelFormat(),
      colorSpaceCount(),
      pColorSpaces(nullptr) {}

safe_VkImageFormatConstraintsInfoFUCHSIA::safe_VkImageFormatConstraintsInfoFUCHSIA(
    const safe_VkImageFormatConstraintsInfoFUCHSIA& copy_src) {
    sType = copy_src.sType;
    imageCreateInfo.initialize(&copy_src.imageCreateInfo);
    requiredFormatFeatures = copy_src.requiredFormatFeatures;
    flags = copy_src.flags;
    sysmemPixelFormat = copy_src.sysmemPixelFormat;
    colorSpaceCount = copy_src.colorSpaceCount;
    pColorSpaces = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (colorSpaceCount && copy_src.pColorSpaces) {
        pColorSpaces = new safe_VkSysmemColorSpaceFUCHSIA[colorSpaceCount];
        for (uint32_t i = 0; i < colorSpaceCount; ++i) {
            pColorSpaces[i].initialize(&copy_src.pColorSpaces[i]);
        }
    }
}

safe_VkImageFormatConstraintsInfoFUCHSIA& safe_VkImageFormatConstraintsInfoFUCHSIA::operator=(
    const safe_VkImageFormatConstraintsInfoFUCHSIA& copy_src) {
    if (&copy_src == this) return *this;

    if (pColorSpaces) delete[] pColorSpaces;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    imageCreateInfo.initialize(&copy_src.imageCreateInfo);
    requiredFormatFeatures = copy_src.requiredFormatFeatures;
    flags = copy_src.flags;
    sysmemPixelFormat = copy_src.sysmemPixelFormat;
    colorSpaceCount = copy_src.colorSpaceCount;
    pColorSpaces = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (colorSpaceCount && copy_src.pColorSpaces) {
        pColorSpaces = new safe_VkSysmemColorSpaceFUCHSIA[colorSpaceCount];
        for (uint32_t i = 0; i < colorSpaceCount; ++i) {
            pColorSpaces[i].initialize(&copy_src.pColorSpaces[i]);
        }
    }

    return *this;
}

safe_VkImageFormatConstraintsInfoFUCHSIA::~safe_VkImageFormatConstraintsInfoFUCHSIA() {
    if (pColorSpaces) delete[] pColorSpaces;
    FreePnextChain(pNext);
}

void safe_VkImageFormatConstraintsInfoFUCHSIA::initialize(const VkImageFormatConstraintsInfoFUCHSIA* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    if (pColorSpaces) delete[] pColorSpaces;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    imageCreateInfo.initialize(&in_struct->imageCreateInfo);
    requiredFormatFeatures = in_struct->requiredFormatFeatures;
    flags = in_struct->flags;
    sysmemPixelFormat = in_struct->sysmemPixelFormat;
    colorSpaceCount = in_struct->colorSpaceCount;
    pColorSpaces = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (colorSpaceCount && in_struct->pColorSpaces) {
        pColorSpaces = new safe_VkSysmemColorSpaceFUCHSIA[colorSpaceCount];
        for (uint32_t i = 0; i < colorSpaceCount; ++i) {
            pColorSpaces[i].initialize(&in_struct->pColorSpaces[i]);
        }
    }
}

void safe_VkImageFormatConstraintsInfoFUCHSIA::initialize(const safe_VkImageFormatConstraintsInfoFUCHSIA* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    imageCreateInfo.initialize(&copy_src->imageCreateInfo);
    requiredFormatFeatures = copy_src->requiredFormatFeatures;
    flags = copy_src->flags;
    sysmemPixelFormat = copy_src->sysmemPixelFormat;
    colorSpaceCount = copy_src->colorSpaceCount;
    pColorSpaces = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (colorSpaceCount && copy_src->pColorSpaces) {
        pColorSpaces = new safe_VkSysmemColorSpaceFUCHSIA[colorSpaceCount];
        for (uint32_t i = 0; i < colorSpaceCount; ++i) {
            pColorSpaces[i].initialize(&copy_src->pColorSpaces[i]);
        }
    }
}

safe_VkImageConstraintsInfoFUCHSIA::safe_VkImageConstraintsInfoFUCHSIA(const VkImageConstraintsInfoFUCHSIA* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      formatConstraintsCount(in_struct->formatConstraintsCount),
      pFormatConstraints(nullptr),
      bufferCollectionConstraints(&in_struct->bufferCollectionConstraints),
      flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (formatConstraintsCount && in_struct->pFormatConstraints) {
        pFormatConstraints = new safe_VkImageFormatConstraintsInfoFUCHSIA[formatConstraintsCount];
        for (uint32_t i = 0; i < formatConstraintsCount; ++i) {
            pFormatConstraints[i].initialize(&in_struct->pFormatConstraints[i]);
        }
    }
}

safe_VkImageConstraintsInfoFUCHSIA::safe_VkImageConstraintsInfoFUCHSIA()
    : sType(VK_STRUCTURE_TYPE_IMAGE_CONSTRAINTS_INFO_FUCHSIA),
      pNext(nullptr),
      formatConstraintsCount(),
      pFormatConstraints(nullptr),
      flags() {}

safe_VkImageConstraintsInfoFUCHSIA::safe_VkImageConstraintsInfoFUCHSIA(const safe_VkImageConstraintsInfoFUCHSIA& copy_src) {
    sType = copy_src.sType;
    formatConstraintsCount = copy_src.formatConstraintsCount;
    pFormatConstraints = nullptr;
    bufferCollectionConstraints.initialize(&copy_src.bufferCollectionConstraints);
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
    if (formatConstraintsCount && copy_src.pFormatConstraints) {
        pFormatConstraints = new safe_VkImageFormatConstraintsInfoFUCHSIA[formatConstraintsCount];
        for (uint32_t i = 0; i < formatConstraintsCount; ++i) {
            pFormatConstraints[i].initialize(&copy_src.pFormatConstraints[i]);
        }
    }
}

safe_VkImageConstraintsInfoFUCHSIA& safe_VkImageConstraintsInfoFUCHSIA::operator=(
    const safe_VkImageConstraintsInfoFUCHSIA& copy_src) {
    if (&copy_src == this) return *this;

    if (pFormatConstraints) delete[] pFormatConstraints;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    formatConstraintsCount = copy_src.formatConstraintsCount;
    pFormatConstraints = nullptr;
    bufferCollectionConstraints.initialize(&copy_src.bufferCollectionConstraints);
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
    if (formatConstraintsCount && copy_src.pFormatConstraints) {
        pFormatConstraints = new safe_VkImageFormatConstraintsInfoFUCHSIA[formatConstraintsCount];
        for (uint32_t i = 0; i < formatConstraintsCount; ++i) {
            pFormatConstraints[i].initialize(&copy_src.pFormatConstraints[i]);
        }
    }

    return *this;
}

safe_VkImageConstraintsInfoFUCHSIA::~safe_VkImageConstraintsInfoFUCHSIA() {
    if (pFormatConstraints) delete[] pFormatConstraints;
    FreePnextChain(pNext);
}

void safe_VkImageConstraintsInfoFUCHSIA::initialize(const VkImageConstraintsInfoFUCHSIA* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    if (pFormatConstraints) delete[] pFormatConstraints;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    formatConstraintsCount = in_struct->formatConstraintsCount;
    pFormatConstraints = nullptr;
    bufferCollectionConstraints.initialize(&in_struct->bufferCollectionConstraints);
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (formatConstraintsCount && in_struct->pFormatConstraints) {
        pFormatConstraints = new safe_VkImageFormatConstraintsInfoFUCHSIA[formatConstraintsCount];
        for (uint32_t i = 0; i < formatConstraintsCount; ++i) {
            pFormatConstraints[i].initialize(&in_struct->pFormatConstraints[i]);
        }
    }
}

void safe_VkImageConstraintsInfoFUCHSIA::initialize(const safe_VkImageConstraintsInfoFUCHSIA* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    formatConstraintsCount = copy_src->formatConstraintsCount;
    pFormatConstraints = nullptr;
    bufferCollectionConstraints.initialize(&copy_src->bufferCollectionConstraints);
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);
    if (formatConstraintsCount && copy_src->pFormatConstraints) {
        pFormatConstraints = new safe_VkImageFormatConstraintsInfoFUCHSIA[formatConstraintsCount];
        for (uint32_t i = 0; i < formatConstraintsCount; ++i) {
            pFormatConstraints[i].initialize(&copy_src->pFormatConstraints[i]);
        }
    }
}
#endif  // VK_USE_PLATFORM_FUCHSIA

safe_VkSubpassShadingPipelineCreateInfoHUAWEI::safe_VkSubpassShadingPipelineCreateInfoHUAWEI(
    const VkSubpassShadingPipelineCreateInfoHUAWEI* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), renderPass(in_struct->renderPass), subpass(in_struct->subpass) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSubpassShadingPipelineCreateInfoHUAWEI::safe_VkSubpassShadingPipelineCreateInfoHUAWEI()
    : sType(VK_STRUCTURE_TYPE_SUBPASS_SHADING_PIPELINE_CREATE_INFO_HUAWEI), pNext(nullptr), renderPass(), subpass() {}

safe_VkSubpassShadingPipelineCreateInfoHUAWEI::safe_VkSubpassShadingPipelineCreateInfoHUAWEI(
    const safe_VkSubpassShadingPipelineCreateInfoHUAWEI& copy_src) {
    sType = copy_src.sType;
    renderPass = copy_src.renderPass;
    subpass = copy_src.subpass;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSubpassShadingPipelineCreateInfoHUAWEI& safe_VkSubpassShadingPipelineCreateInfoHUAWEI::operator=(
    const safe_VkSubpassShadingPipelineCreateInfoHUAWEI& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    renderPass = copy_src.renderPass;
    subpass = copy_src.subpass;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSubpassShadingPipelineCreateInfoHUAWEI::~safe_VkSubpassShadingPipelineCreateInfoHUAWEI() { FreePnextChain(pNext); }

void safe_VkSubpassShadingPipelineCreateInfoHUAWEI::initialize(const VkSubpassShadingPipelineCreateInfoHUAWEI* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    renderPass = in_struct->renderPass;
    subpass = in_struct->subpass;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSubpassShadingPipelineCreateInfoHUAWEI::initialize(const safe_VkSubpassShadingPipelineCreateInfoHUAWEI* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    renderPass = copy_src->renderPass;
    subpass = copy_src->subpass;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI::safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI(
    const VkPhysicalDeviceSubpassShadingFeaturesHUAWEI* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), subpassShading(in_struct->subpassShading) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI::safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_FEATURES_HUAWEI), pNext(nullptr), subpassShading() {}

safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI::safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI(
    const safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI& copy_src) {
    sType = copy_src.sType;
    subpassShading = copy_src.subpassShading;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI& safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI::operator=(
    const safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    subpassShading = copy_src.subpassShading;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI::~safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI::initialize(const VkPhysicalDeviceSubpassShadingFeaturesHUAWEI* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    subpassShading = in_struct->subpassShading;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI::initialize(
    const safe_VkPhysicalDeviceSubpassShadingFeaturesHUAWEI* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    subpassShading = copy_src->subpassShading;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI::safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI(
    const VkPhysicalDeviceSubpassShadingPropertiesHUAWEI* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), maxSubpassShadingWorkgroupSizeAspectRatio(in_struct->maxSubpassShadingWorkgroupSizeAspectRatio) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI::safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBPASS_SHADING_PROPERTIES_HUAWEI),
      pNext(nullptr),
      maxSubpassShadingWorkgroupSizeAspectRatio() {}

safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI::safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI(
    const safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI& copy_src) {
    sType = copy_src.sType;
    maxSubpassShadingWorkgroupSizeAspectRatio = copy_src.maxSubpassShadingWorkgroupSizeAspectRatio;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI& safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI::operator=(
    const safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxSubpassShadingWorkgroupSizeAspectRatio = copy_src.maxSubpassShadingWorkgroupSizeAspectRatio;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI::~safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI::initialize(
    const VkPhysicalDeviceSubpassShadingPropertiesHUAWEI* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxSubpassShadingWorkgroupSizeAspectRatio = in_struct->maxSubpassShadingWorkgroupSizeAspectRatio;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI::initialize(
    const safe_VkPhysicalDeviceSubpassShadingPropertiesHUAWEI* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxSubpassShadingWorkgroupSizeAspectRatio = copy_src->maxSubpassShadingWorkgroupSizeAspectRatio;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI::safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI(
    const VkPhysicalDeviceInvocationMaskFeaturesHUAWEI* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), invocationMask(in_struct->invocationMask) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI::safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INVOCATION_MASK_FEATURES_HUAWEI), pNext(nullptr), invocationMask() {}

safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI::safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI(
    const safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI& copy_src) {
    sType = copy_src.sType;
    invocationMask = copy_src.invocationMask;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI& safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI::operator=(
    const safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    invocationMask = copy_src.invocationMask;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI::~safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI::initialize(const VkPhysicalDeviceInvocationMaskFeaturesHUAWEI* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    invocationMask = in_struct->invocationMask;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI::initialize(
    const safe_VkPhysicalDeviceInvocationMaskFeaturesHUAWEI* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    invocationMask = copy_src->invocationMask;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMemoryGetRemoteAddressInfoNV::safe_VkMemoryGetRemoteAddressInfoNV(const VkMemoryGetRemoteAddressInfoNV* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), memory(in_struct->memory), handleType(in_struct->handleType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkMemoryGetRemoteAddressInfoNV::safe_VkMemoryGetRemoteAddressInfoNV()
    : sType(VK_STRUCTURE_TYPE_MEMORY_GET_REMOTE_ADDRESS_INFO_NV), pNext(nullptr), memory(), handleType() {}

safe_VkMemoryGetRemoteAddressInfoNV::safe_VkMemoryGetRemoteAddressInfoNV(const safe_VkMemoryGetRemoteAddressInfoNV& copy_src) {
    sType = copy_src.sType;
    memory = copy_src.memory;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkMemoryGetRemoteAddressInfoNV& safe_VkMemoryGetRemoteAddressInfoNV::operator=(
    const safe_VkMemoryGetRemoteAddressInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memory = copy_src.memory;
    handleType = copy_src.handleType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkMemoryGetRemoteAddressInfoNV::~safe_VkMemoryGetRemoteAddressInfoNV() { FreePnextChain(pNext); }

void safe_VkMemoryGetRemoteAddressInfoNV::initialize(const VkMemoryGetRemoteAddressInfoNV* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memory = in_struct->memory;
    handleType = in_struct->handleType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkMemoryGetRemoteAddressInfoNV::initialize(const safe_VkMemoryGetRemoteAddressInfoNV* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memory = copy_src->memory;
    handleType = copy_src->handleType;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV::safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV(
    const VkPhysicalDeviceExternalMemoryRDMAFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), externalMemoryRDMA(in_struct->externalMemoryRDMA) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV::safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_RDMA_FEATURES_NV), pNext(nullptr), externalMemoryRDMA() {}

safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV::safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV(
    const safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV& copy_src) {
    sType = copy_src.sType;
    externalMemoryRDMA = copy_src.externalMemoryRDMA;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV& safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV::operator=(
    const safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    externalMemoryRDMA = copy_src.externalMemoryRDMA;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV::~safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV::initialize(const VkPhysicalDeviceExternalMemoryRDMAFeaturesNV* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    externalMemoryRDMA = in_struct->externalMemoryRDMA;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV::initialize(
    const safe_VkPhysicalDeviceExternalMemoryRDMAFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    externalMemoryRDMA = copy_src->externalMemoryRDMA;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_USE_PLATFORM_SCREEN_QNX

safe_VkScreenSurfaceCreateInfoQNX::safe_VkScreenSurfaceCreateInfoQNX(const VkScreenSurfaceCreateInfoQNX* in_struct,
                                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags), context(nullptr), window(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    context = in_struct->context;
    window = in_struct->window;
}

safe_VkScreenSurfaceCreateInfoQNX::safe_VkScreenSurfaceCreateInfoQNX()
    : sType(VK_STRUCTURE_TYPE_SCREEN_SURFACE_CREATE_INFO_QNX), pNext(nullptr), flags(), context(nullptr), window(nullptr) {}

safe_VkScreenSurfaceCreateInfoQNX::safe_VkScreenSurfaceCreateInfoQNX(const safe_VkScreenSurfaceCreateInfoQNX& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
    context = copy_src.context;
    window = copy_src.window;
}

safe_VkScreenSurfaceCreateInfoQNX& safe_VkScreenSurfaceCreateInfoQNX::operator=(const safe_VkScreenSurfaceCreateInfoQNX& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
    context = copy_src.context;
    window = copy_src.window;

    return *this;
}

safe_VkScreenSurfaceCreateInfoQNX::~safe_VkScreenSurfaceCreateInfoQNX() { FreePnextChain(pNext); }

void safe_VkScreenSurfaceCreateInfoQNX::initialize(const VkScreenSurfaceCreateInfoQNX* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    context = in_struct->context;
    window = in_struct->window;
}

void safe_VkScreenSurfaceCreateInfoQNX::initialize(const safe_VkScreenSurfaceCreateInfoQNX* copy_src,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);
    context = copy_src->context;
    window = copy_src->window;
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX
#ifdef VK_ENABLE_BETA_EXTENSIONS

safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV::safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV(
    const VkPhysicalDeviceDisplacementMicromapFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), displacementMicromap(in_struct->displacementMicromap) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV::safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_FEATURES_NV), pNext(nullptr), displacementMicromap() {}

safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV::safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV(
    const safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV& copy_src) {
    sType = copy_src.sType;
    displacementMicromap = copy_src.displacementMicromap;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV& safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV::operator=(
    const safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    displacementMicromap = copy_src.displacementMicromap;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV::~safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV::initialize(
    const VkPhysicalDeviceDisplacementMicromapFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    displacementMicromap = in_struct->displacementMicromap;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV::initialize(
    const safe_VkPhysicalDeviceDisplacementMicromapFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    displacementMicromap = copy_src->displacementMicromap;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV::safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV(
    const VkPhysicalDeviceDisplacementMicromapPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), maxDisplacementMicromapSubdivisionLevel(in_struct->maxDisplacementMicromapSubdivisionLevel) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV::safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DISPLACEMENT_MICROMAP_PROPERTIES_NV),
      pNext(nullptr),
      maxDisplacementMicromapSubdivisionLevel() {}

safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV::safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV(
    const safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV& copy_src) {
    sType = copy_src.sType;
    maxDisplacementMicromapSubdivisionLevel = copy_src.maxDisplacementMicromapSubdivisionLevel;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV& safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV::operator=(
    const safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxDisplacementMicromapSubdivisionLevel = copy_src.maxDisplacementMicromapSubdivisionLevel;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV::~safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV::initialize(
    const VkPhysicalDeviceDisplacementMicromapPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxDisplacementMicromapSubdivisionLevel = in_struct->maxDisplacementMicromapSubdivisionLevel;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV::initialize(
    const safe_VkPhysicalDeviceDisplacementMicromapPropertiesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxDisplacementMicromapSubdivisionLevel = copy_src->maxDisplacementMicromapSubdivisionLevel;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_ENABLE_BETA_EXTENSIONS

safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI::safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI(
    const VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      clustercullingShader(in_struct->clustercullingShader),
      multiviewClusterCullingShader(in_struct->multiviewClusterCullingShader) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI::safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_FEATURES_HUAWEI),
      pNext(nullptr),
      clustercullingShader(),
      multiviewClusterCullingShader() {}

safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI::safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI(
    const safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI& copy_src) {
    sType = copy_src.sType;
    clustercullingShader = copy_src.clustercullingShader;
    multiviewClusterCullingShader = copy_src.multiviewClusterCullingShader;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI& safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI::operator=(
    const safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    clustercullingShader = copy_src.clustercullingShader;
    multiviewClusterCullingShader = copy_src.multiviewClusterCullingShader;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI::~safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI::initialize(
    const VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    clustercullingShader = in_struct->clustercullingShader;
    multiviewClusterCullingShader = in_struct->multiviewClusterCullingShader;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI::initialize(
    const safe_VkPhysicalDeviceClusterCullingShaderFeaturesHUAWEI* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    clustercullingShader = copy_src->clustercullingShader;
    multiviewClusterCullingShader = copy_src->multiviewClusterCullingShader;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI::safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI(
    const VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      maxOutputClusterCount(in_struct->maxOutputClusterCount),
      indirectBufferOffsetAlignment(in_struct->indirectBufferOffsetAlignment) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    for (uint32_t i = 0; i < 3; ++i) {
        maxWorkGroupCount[i] = in_struct->maxWorkGroupCount[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxWorkGroupSize[i] = in_struct->maxWorkGroupSize[i];
    }
}

safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI::safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_PROPERTIES_HUAWEI),
      pNext(nullptr),
      maxOutputClusterCount(),
      indirectBufferOffsetAlignment() {}

safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI::safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI(
    const safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI& copy_src) {
    sType = copy_src.sType;
    maxOutputClusterCount = copy_src.maxOutputClusterCount;
    indirectBufferOffsetAlignment = copy_src.indirectBufferOffsetAlignment;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < 3; ++i) {
        maxWorkGroupCount[i] = copy_src.maxWorkGroupCount[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxWorkGroupSize[i] = copy_src.maxWorkGroupSize[i];
    }
}

safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI& safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI::operator=(
    const safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxOutputClusterCount = copy_src.maxOutputClusterCount;
    indirectBufferOffsetAlignment = copy_src.indirectBufferOffsetAlignment;
    pNext = SafePnextCopy(copy_src.pNext);

    for (uint32_t i = 0; i < 3; ++i) {
        maxWorkGroupCount[i] = copy_src.maxWorkGroupCount[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxWorkGroupSize[i] = copy_src.maxWorkGroupSize[i];
    }

    return *this;
}

safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI::~safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI::initialize(
    const VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxOutputClusterCount = in_struct->maxOutputClusterCount;
    indirectBufferOffsetAlignment = in_struct->indirectBufferOffsetAlignment;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    for (uint32_t i = 0; i < 3; ++i) {
        maxWorkGroupCount[i] = in_struct->maxWorkGroupCount[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxWorkGroupSize[i] = in_struct->maxWorkGroupSize[i];
    }
}

void safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI::initialize(
    const safe_VkPhysicalDeviceClusterCullingShaderPropertiesHUAWEI* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxOutputClusterCount = copy_src->maxOutputClusterCount;
    indirectBufferOffsetAlignment = copy_src->indirectBufferOffsetAlignment;
    pNext = SafePnextCopy(copy_src->pNext);

    for (uint32_t i = 0; i < 3; ++i) {
        maxWorkGroupCount[i] = copy_src->maxWorkGroupCount[i];
    }

    for (uint32_t i = 0; i < 3; ++i) {
        maxWorkGroupSize[i] = copy_src->maxWorkGroupSize[i];
    }
}

safe_VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI::safe_VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI(
    const VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), clusterShadingRate(in_struct->clusterShadingRate) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI::safe_VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_CULLING_SHADER_VRS_FEATURES_HUAWEI), pNext(nullptr), clusterShadingRate() {}

safe_VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI::safe_VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI(
    const safe_VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI& copy_src) {
    sType = copy_src.sType;
    clusterShadingRate = copy_src.clusterShadingRate;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI& safe_VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI::operator=(
    const safe_VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    clusterShadingRate = copy_src.clusterShadingRate;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI::~safe_VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI::initialize(
    const VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    clusterShadingRate = in_struct->clusterShadingRate;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI::initialize(
    const safe_VkPhysicalDeviceClusterCullingShaderVrsFeaturesHUAWEI* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    clusterShadingRate = copy_src->clusterShadingRate;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderCorePropertiesARM::safe_VkPhysicalDeviceShaderCorePropertiesARM(
    const VkPhysicalDeviceShaderCorePropertiesARM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pixelRate(in_struct->pixelRate), texelRate(in_struct->texelRate), fmaRate(in_struct->fmaRate) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderCorePropertiesARM::safe_VkPhysicalDeviceShaderCorePropertiesARM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_PROPERTIES_ARM), pNext(nullptr), pixelRate(), texelRate(), fmaRate() {}

safe_VkPhysicalDeviceShaderCorePropertiesARM::safe_VkPhysicalDeviceShaderCorePropertiesARM(
    const safe_VkPhysicalDeviceShaderCorePropertiesARM& copy_src) {
    sType = copy_src.sType;
    pixelRate = copy_src.pixelRate;
    texelRate = copy_src.texelRate;
    fmaRate = copy_src.fmaRate;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderCorePropertiesARM& safe_VkPhysicalDeviceShaderCorePropertiesARM::operator=(
    const safe_VkPhysicalDeviceShaderCorePropertiesARM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pixelRate = copy_src.pixelRate;
    texelRate = copy_src.texelRate;
    fmaRate = copy_src.fmaRate;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderCorePropertiesARM::~safe_VkPhysicalDeviceShaderCorePropertiesARM() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderCorePropertiesARM::initialize(const VkPhysicalDeviceShaderCorePropertiesARM* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pixelRate = in_struct->pixelRate;
    texelRate = in_struct->texelRate;
    fmaRate = in_struct->fmaRate;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderCorePropertiesARM::initialize(const safe_VkPhysicalDeviceShaderCorePropertiesARM* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pixelRate = copy_src->pixelRate;
    texelRate = copy_src->texelRate;
    fmaRate = copy_src->fmaRate;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDeviceQueueShaderCoreControlCreateInfoARM::safe_VkDeviceQueueShaderCoreControlCreateInfoARM(
    const VkDeviceQueueShaderCoreControlCreateInfoARM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), shaderCoreCount(in_struct->shaderCoreCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDeviceQueueShaderCoreControlCreateInfoARM::safe_VkDeviceQueueShaderCoreControlCreateInfoARM()
    : sType(VK_STRUCTURE_TYPE_DEVICE_QUEUE_SHADER_CORE_CONTROL_CREATE_INFO_ARM), pNext(nullptr), shaderCoreCount() {}

safe_VkDeviceQueueShaderCoreControlCreateInfoARM::safe_VkDeviceQueueShaderCoreControlCreateInfoARM(
    const safe_VkDeviceQueueShaderCoreControlCreateInfoARM& copy_src) {
    sType = copy_src.sType;
    shaderCoreCount = copy_src.shaderCoreCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDeviceQueueShaderCoreControlCreateInfoARM& safe_VkDeviceQueueShaderCoreControlCreateInfoARM::operator=(
    const safe_VkDeviceQueueShaderCoreControlCreateInfoARM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderCoreCount = copy_src.shaderCoreCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDeviceQueueShaderCoreControlCreateInfoARM::~safe_VkDeviceQueueShaderCoreControlCreateInfoARM() { FreePnextChain(pNext); }

void safe_VkDeviceQueueShaderCoreControlCreateInfoARM::initialize(const VkDeviceQueueShaderCoreControlCreateInfoARM* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderCoreCount = in_struct->shaderCoreCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDeviceQueueShaderCoreControlCreateInfoARM::initialize(const safe_VkDeviceQueueShaderCoreControlCreateInfoARM* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderCoreCount = copy_src->shaderCoreCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceSchedulingControlsFeaturesARM::safe_VkPhysicalDeviceSchedulingControlsFeaturesARM(
    const VkPhysicalDeviceSchedulingControlsFeaturesARM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), schedulingControls(in_struct->schedulingControls) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceSchedulingControlsFeaturesARM::safe_VkPhysicalDeviceSchedulingControlsFeaturesARM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_FEATURES_ARM), pNext(nullptr), schedulingControls() {}

safe_VkPhysicalDeviceSchedulingControlsFeaturesARM::safe_VkPhysicalDeviceSchedulingControlsFeaturesARM(
    const safe_VkPhysicalDeviceSchedulingControlsFeaturesARM& copy_src) {
    sType = copy_src.sType;
    schedulingControls = copy_src.schedulingControls;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceSchedulingControlsFeaturesARM& safe_VkPhysicalDeviceSchedulingControlsFeaturesARM::operator=(
    const safe_VkPhysicalDeviceSchedulingControlsFeaturesARM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    schedulingControls = copy_src.schedulingControls;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceSchedulingControlsFeaturesARM::~safe_VkPhysicalDeviceSchedulingControlsFeaturesARM() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceSchedulingControlsFeaturesARM::initialize(const VkPhysicalDeviceSchedulingControlsFeaturesARM* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    schedulingControls = in_struct->schedulingControls;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceSchedulingControlsFeaturesARM::initialize(
    const safe_VkPhysicalDeviceSchedulingControlsFeaturesARM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    schedulingControls = copy_src->schedulingControls;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceSchedulingControlsPropertiesARM::safe_VkPhysicalDeviceSchedulingControlsPropertiesARM(
    const VkPhysicalDeviceSchedulingControlsPropertiesARM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), schedulingControlsFlags(in_struct->schedulingControlsFlags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceSchedulingControlsPropertiesARM::safe_VkPhysicalDeviceSchedulingControlsPropertiesARM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCHEDULING_CONTROLS_PROPERTIES_ARM), pNext(nullptr), schedulingControlsFlags() {}

safe_VkPhysicalDeviceSchedulingControlsPropertiesARM::safe_VkPhysicalDeviceSchedulingControlsPropertiesARM(
    const safe_VkPhysicalDeviceSchedulingControlsPropertiesARM& copy_src) {
    sType = copy_src.sType;
    schedulingControlsFlags = copy_src.schedulingControlsFlags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceSchedulingControlsPropertiesARM& safe_VkPhysicalDeviceSchedulingControlsPropertiesARM::operator=(
    const safe_VkPhysicalDeviceSchedulingControlsPropertiesARM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    schedulingControlsFlags = copy_src.schedulingControlsFlags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceSchedulingControlsPropertiesARM::~safe_VkPhysicalDeviceSchedulingControlsPropertiesARM() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceSchedulingControlsPropertiesARM::initialize(
    const VkPhysicalDeviceSchedulingControlsPropertiesARM* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    schedulingControlsFlags = in_struct->schedulingControlsFlags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceSchedulingControlsPropertiesARM::initialize(
    const safe_VkPhysicalDeviceSchedulingControlsPropertiesARM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    schedulingControlsFlags = copy_src->schedulingControlsFlags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE::safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE(
    const VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), descriptorSetHostMapping(in_struct->descriptorSetHostMapping) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE::safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_SET_HOST_MAPPING_FEATURES_VALVE),
      pNext(nullptr),
      descriptorSetHostMapping() {}

safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE::safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE(
    const safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE& copy_src) {
    sType = copy_src.sType;
    descriptorSetHostMapping = copy_src.descriptorSetHostMapping;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE& safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE::operator=(
    const safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    descriptorSetHostMapping = copy_src.descriptorSetHostMapping;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE::~safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE::initialize(
    const VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    descriptorSetHostMapping = in_struct->descriptorSetHostMapping;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE::initialize(
    const safe_VkPhysicalDeviceDescriptorSetHostMappingFeaturesVALVE* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    descriptorSetHostMapping = copy_src->descriptorSetHostMapping;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDescriptorSetBindingReferenceVALVE::safe_VkDescriptorSetBindingReferenceVALVE(
    const VkDescriptorSetBindingReferenceVALVE* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), descriptorSetLayout(in_struct->descriptorSetLayout), binding(in_struct->binding) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDescriptorSetBindingReferenceVALVE::safe_VkDescriptorSetBindingReferenceVALVE()
    : sType(VK_STRUCTURE_TYPE_DESCRIPTOR_SET_BINDING_REFERENCE_VALVE), pNext(nullptr), descriptorSetLayout(), binding() {}

safe_VkDescriptorSetBindingReferenceVALVE::safe_VkDescriptorSetBindingReferenceVALVE(
    const safe_VkDescriptorSetBindingReferenceVALVE& copy_src) {
    sType = copy_src.sType;
    descriptorSetLayout = copy_src.descriptorSetLayout;
    binding = copy_src.binding;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDescriptorSetBindingReferenceVALVE& safe_VkDescriptorSetBindingReferenceVALVE::operator=(
    const safe_VkDescriptorSetBindingReferenceVALVE& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    descriptorSetLayout = copy_src.descriptorSetLayout;
    binding = copy_src.binding;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDescriptorSetBindingReferenceVALVE::~safe_VkDescriptorSetBindingReferenceVALVE() { FreePnextChain(pNext); }

void safe_VkDescriptorSetBindingReferenceVALVE::initialize(const VkDescriptorSetBindingReferenceVALVE* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    descriptorSetLayout = in_struct->descriptorSetLayout;
    binding = in_struct->binding;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDescriptorSetBindingReferenceVALVE::initialize(const safe_VkDescriptorSetBindingReferenceVALVE* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    descriptorSetLayout = copy_src->descriptorSetLayout;
    binding = copy_src->binding;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDescriptorSetLayoutHostMappingInfoVALVE::safe_VkDescriptorSetLayoutHostMappingInfoVALVE(
    const VkDescriptorSetLayoutHostMappingInfoVALVE* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), descriptorOffset(in_struct->descriptorOffset), descriptorSize(in_struct->descriptorSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDescriptorSetLayoutHostMappingInfoVALVE::safe_VkDescriptorSetLayoutHostMappingInfoVALVE()
    : sType(VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_HOST_MAPPING_INFO_VALVE),
      pNext(nullptr),
      descriptorOffset(),
      descriptorSize() {}

safe_VkDescriptorSetLayoutHostMappingInfoVALVE::safe_VkDescriptorSetLayoutHostMappingInfoVALVE(
    const safe_VkDescriptorSetLayoutHostMappingInfoVALVE& copy_src) {
    sType = copy_src.sType;
    descriptorOffset = copy_src.descriptorOffset;
    descriptorSize = copy_src.descriptorSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDescriptorSetLayoutHostMappingInfoVALVE& safe_VkDescriptorSetLayoutHostMappingInfoVALVE::operator=(
    const safe_VkDescriptorSetLayoutHostMappingInfoVALVE& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    descriptorOffset = copy_src.descriptorOffset;
    descriptorSize = copy_src.descriptorSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDescriptorSetLayoutHostMappingInfoVALVE::~safe_VkDescriptorSetLayoutHostMappingInfoVALVE() { FreePnextChain(pNext); }

void safe_VkDescriptorSetLayoutHostMappingInfoVALVE::initialize(const VkDescriptorSetLayoutHostMappingInfoVALVE* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    descriptorOffset = in_struct->descriptorOffset;
    descriptorSize = in_struct->descriptorSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDescriptorSetLayoutHostMappingInfoVALVE::initialize(const safe_VkDescriptorSetLayoutHostMappingInfoVALVE* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    descriptorOffset = copy_src->descriptorOffset;
    descriptorSize = copy_src->descriptorSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRenderPassStripedFeaturesARM::safe_VkPhysicalDeviceRenderPassStripedFeaturesARM(
    const VkPhysicalDeviceRenderPassStripedFeaturesARM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), renderPassStriped(in_struct->renderPassStriped) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRenderPassStripedFeaturesARM::safe_VkPhysicalDeviceRenderPassStripedFeaturesARM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_FEATURES_ARM), pNext(nullptr), renderPassStriped() {}

safe_VkPhysicalDeviceRenderPassStripedFeaturesARM::safe_VkPhysicalDeviceRenderPassStripedFeaturesARM(
    const safe_VkPhysicalDeviceRenderPassStripedFeaturesARM& copy_src) {
    sType = copy_src.sType;
    renderPassStriped = copy_src.renderPassStriped;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRenderPassStripedFeaturesARM& safe_VkPhysicalDeviceRenderPassStripedFeaturesARM::operator=(
    const safe_VkPhysicalDeviceRenderPassStripedFeaturesARM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    renderPassStriped = copy_src.renderPassStriped;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRenderPassStripedFeaturesARM::~safe_VkPhysicalDeviceRenderPassStripedFeaturesARM() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceRenderPassStripedFeaturesARM::initialize(const VkPhysicalDeviceRenderPassStripedFeaturesARM* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    renderPassStriped = in_struct->renderPassStriped;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRenderPassStripedFeaturesARM::initialize(
    const safe_VkPhysicalDeviceRenderPassStripedFeaturesARM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    renderPassStriped = copy_src->renderPassStriped;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRenderPassStripedPropertiesARM::safe_VkPhysicalDeviceRenderPassStripedPropertiesARM(
    const VkPhysicalDeviceRenderPassStripedPropertiesARM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      renderPassStripeGranularity(in_struct->renderPassStripeGranularity),
      maxRenderPassStripes(in_struct->maxRenderPassStripes) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRenderPassStripedPropertiesARM::safe_VkPhysicalDeviceRenderPassStripedPropertiesARM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RENDER_PASS_STRIPED_PROPERTIES_ARM),
      pNext(nullptr),
      renderPassStripeGranularity(),
      maxRenderPassStripes() {}

safe_VkPhysicalDeviceRenderPassStripedPropertiesARM::safe_VkPhysicalDeviceRenderPassStripedPropertiesARM(
    const safe_VkPhysicalDeviceRenderPassStripedPropertiesARM& copy_src) {
    sType = copy_src.sType;
    renderPassStripeGranularity = copy_src.renderPassStripeGranularity;
    maxRenderPassStripes = copy_src.maxRenderPassStripes;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRenderPassStripedPropertiesARM& safe_VkPhysicalDeviceRenderPassStripedPropertiesARM::operator=(
    const safe_VkPhysicalDeviceRenderPassStripedPropertiesARM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    renderPassStripeGranularity = copy_src.renderPassStripeGranularity;
    maxRenderPassStripes = copy_src.maxRenderPassStripes;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRenderPassStripedPropertiesARM::~safe_VkPhysicalDeviceRenderPassStripedPropertiesARM() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceRenderPassStripedPropertiesARM::initialize(
    const VkPhysicalDeviceRenderPassStripedPropertiesARM* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    renderPassStripeGranularity = in_struct->renderPassStripeGranularity;
    maxRenderPassStripes = in_struct->maxRenderPassStripes;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRenderPassStripedPropertiesARM::initialize(
    const safe_VkPhysicalDeviceRenderPassStripedPropertiesARM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    renderPassStripeGranularity = copy_src->renderPassStripeGranularity;
    maxRenderPassStripes = copy_src->maxRenderPassStripes;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkRenderPassStripeInfoARM::safe_VkRenderPassStripeInfoARM(const VkRenderPassStripeInfoARM* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), stripeArea(in_struct->stripeArea) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkRenderPassStripeInfoARM::safe_VkRenderPassStripeInfoARM()
    : sType(VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_INFO_ARM), pNext(nullptr), stripeArea() {}

safe_VkRenderPassStripeInfoARM::safe_VkRenderPassStripeInfoARM(const safe_VkRenderPassStripeInfoARM& copy_src) {
    sType = copy_src.sType;
    stripeArea = copy_src.stripeArea;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkRenderPassStripeInfoARM& safe_VkRenderPassStripeInfoARM::operator=(const safe_VkRenderPassStripeInfoARM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    stripeArea = copy_src.stripeArea;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkRenderPassStripeInfoARM::~safe_VkRenderPassStripeInfoARM() { FreePnextChain(pNext); }

void safe_VkRenderPassStripeInfoARM::initialize(const VkRenderPassStripeInfoARM* in_struct,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stripeArea = in_struct->stripeArea;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkRenderPassStripeInfoARM::initialize(const safe_VkRenderPassStripeInfoARM* copy_src,
                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stripeArea = copy_src->stripeArea;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkRenderPassStripeBeginInfoARM::safe_VkRenderPassStripeBeginInfoARM(const VkRenderPassStripeBeginInfoARM* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), stripeInfoCount(in_struct->stripeInfoCount), pStripeInfos(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (stripeInfoCount && in_struct->pStripeInfos) {
        pStripeInfos = new safe_VkRenderPassStripeInfoARM[stripeInfoCount];
        for (uint32_t i = 0; i < stripeInfoCount; ++i) {
            pStripeInfos[i].initialize(&in_struct->pStripeInfos[i]);
        }
    }
}

safe_VkRenderPassStripeBeginInfoARM::safe_VkRenderPassStripeBeginInfoARM()
    : sType(VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_BEGIN_INFO_ARM), pNext(nullptr), stripeInfoCount(), pStripeInfos(nullptr) {}

safe_VkRenderPassStripeBeginInfoARM::safe_VkRenderPassStripeBeginInfoARM(const safe_VkRenderPassStripeBeginInfoARM& copy_src) {
    sType = copy_src.sType;
    stripeInfoCount = copy_src.stripeInfoCount;
    pStripeInfos = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (stripeInfoCount && copy_src.pStripeInfos) {
        pStripeInfos = new safe_VkRenderPassStripeInfoARM[stripeInfoCount];
        for (uint32_t i = 0; i < stripeInfoCount; ++i) {
            pStripeInfos[i].initialize(&copy_src.pStripeInfos[i]);
        }
    }
}

safe_VkRenderPassStripeBeginInfoARM& safe_VkRenderPassStripeBeginInfoARM::operator=(
    const safe_VkRenderPassStripeBeginInfoARM& copy_src) {
    if (&copy_src == this) return *this;

    if (pStripeInfos) delete[] pStripeInfos;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    stripeInfoCount = copy_src.stripeInfoCount;
    pStripeInfos = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (stripeInfoCount && copy_src.pStripeInfos) {
        pStripeInfos = new safe_VkRenderPassStripeInfoARM[stripeInfoCount];
        for (uint32_t i = 0; i < stripeInfoCount; ++i) {
            pStripeInfos[i].initialize(&copy_src.pStripeInfos[i]);
        }
    }

    return *this;
}

safe_VkRenderPassStripeBeginInfoARM::~safe_VkRenderPassStripeBeginInfoARM() {
    if (pStripeInfos) delete[] pStripeInfos;
    FreePnextChain(pNext);
}

void safe_VkRenderPassStripeBeginInfoARM::initialize(const VkRenderPassStripeBeginInfoARM* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStripeInfos) delete[] pStripeInfos;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stripeInfoCount = in_struct->stripeInfoCount;
    pStripeInfos = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (stripeInfoCount && in_struct->pStripeInfos) {
        pStripeInfos = new safe_VkRenderPassStripeInfoARM[stripeInfoCount];
        for (uint32_t i = 0; i < stripeInfoCount; ++i) {
            pStripeInfos[i].initialize(&in_struct->pStripeInfos[i]);
        }
    }
}

void safe_VkRenderPassStripeBeginInfoARM::initialize(const safe_VkRenderPassStripeBeginInfoARM* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stripeInfoCount = copy_src->stripeInfoCount;
    pStripeInfos = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (stripeInfoCount && copy_src->pStripeInfos) {
        pStripeInfos = new safe_VkRenderPassStripeInfoARM[stripeInfoCount];
        for (uint32_t i = 0; i < stripeInfoCount; ++i) {
            pStripeInfos[i].initialize(&copy_src->pStripeInfos[i]);
        }
    }
}

safe_VkRenderPassStripeSubmitInfoARM::safe_VkRenderPassStripeSubmitInfoARM(const VkRenderPassStripeSubmitInfoARM* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), stripeSemaphoreInfoCount(in_struct->stripeSemaphoreInfoCount), pStripeSemaphoreInfos(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (stripeSemaphoreInfoCount && in_struct->pStripeSemaphoreInfos) {
        pStripeSemaphoreInfos = new safe_VkSemaphoreSubmitInfo[stripeSemaphoreInfoCount];
        for (uint32_t i = 0; i < stripeSemaphoreInfoCount; ++i) {
            pStripeSemaphoreInfos[i].initialize(&in_struct->pStripeSemaphoreInfos[i]);
        }
    }
}

safe_VkRenderPassStripeSubmitInfoARM::safe_VkRenderPassStripeSubmitInfoARM()
    : sType(VK_STRUCTURE_TYPE_RENDER_PASS_STRIPE_SUBMIT_INFO_ARM),
      pNext(nullptr),
      stripeSemaphoreInfoCount(),
      pStripeSemaphoreInfos(nullptr) {}

safe_VkRenderPassStripeSubmitInfoARM::safe_VkRenderPassStripeSubmitInfoARM(const safe_VkRenderPassStripeSubmitInfoARM& copy_src) {
    sType = copy_src.sType;
    stripeSemaphoreInfoCount = copy_src.stripeSemaphoreInfoCount;
    pStripeSemaphoreInfos = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (stripeSemaphoreInfoCount && copy_src.pStripeSemaphoreInfos) {
        pStripeSemaphoreInfos = new safe_VkSemaphoreSubmitInfo[stripeSemaphoreInfoCount];
        for (uint32_t i = 0; i < stripeSemaphoreInfoCount; ++i) {
            pStripeSemaphoreInfos[i].initialize(&copy_src.pStripeSemaphoreInfos[i]);
        }
    }
}

safe_VkRenderPassStripeSubmitInfoARM& safe_VkRenderPassStripeSubmitInfoARM::operator=(
    const safe_VkRenderPassStripeSubmitInfoARM& copy_src) {
    if (&copy_src == this) return *this;

    if (pStripeSemaphoreInfos) delete[] pStripeSemaphoreInfos;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    stripeSemaphoreInfoCount = copy_src.stripeSemaphoreInfoCount;
    pStripeSemaphoreInfos = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (stripeSemaphoreInfoCount && copy_src.pStripeSemaphoreInfos) {
        pStripeSemaphoreInfos = new safe_VkSemaphoreSubmitInfo[stripeSemaphoreInfoCount];
        for (uint32_t i = 0; i < stripeSemaphoreInfoCount; ++i) {
            pStripeSemaphoreInfos[i].initialize(&copy_src.pStripeSemaphoreInfos[i]);
        }
    }

    return *this;
}

safe_VkRenderPassStripeSubmitInfoARM::~safe_VkRenderPassStripeSubmitInfoARM() {
    if (pStripeSemaphoreInfos) delete[] pStripeSemaphoreInfos;
    FreePnextChain(pNext);
}

void safe_VkRenderPassStripeSubmitInfoARM::initialize(const VkRenderPassStripeSubmitInfoARM* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pStripeSemaphoreInfos) delete[] pStripeSemaphoreInfos;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stripeSemaphoreInfoCount = in_struct->stripeSemaphoreInfoCount;
    pStripeSemaphoreInfos = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (stripeSemaphoreInfoCount && in_struct->pStripeSemaphoreInfos) {
        pStripeSemaphoreInfos = new safe_VkSemaphoreSubmitInfo[stripeSemaphoreInfoCount];
        for (uint32_t i = 0; i < stripeSemaphoreInfoCount; ++i) {
            pStripeSemaphoreInfos[i].initialize(&in_struct->pStripeSemaphoreInfos[i]);
        }
    }
}

void safe_VkRenderPassStripeSubmitInfoARM::initialize(const safe_VkRenderPassStripeSubmitInfoARM* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stripeSemaphoreInfoCount = copy_src->stripeSemaphoreInfoCount;
    pStripeSemaphoreInfos = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (stripeSemaphoreInfoCount && copy_src->pStripeSemaphoreInfos) {
        pStripeSemaphoreInfos = new safe_VkSemaphoreSubmitInfo[stripeSemaphoreInfoCount];
        for (uint32_t i = 0; i < stripeSemaphoreInfoCount; ++i) {
            pStripeSemaphoreInfos[i].initialize(&copy_src->pStripeSemaphoreInfos[i]);
        }
    }
}

safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM::safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM(
    const VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), fragmentDensityMapOffset(in_struct->fragmentDensityMapOffset) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM::safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_FEATURES_QCOM),
      pNext(nullptr),
      fragmentDensityMapOffset() {}

safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM::safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM(
    const safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM& copy_src) {
    sType = copy_src.sType;
    fragmentDensityMapOffset = copy_src.fragmentDensityMapOffset;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM& safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM::operator=(
    const safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fragmentDensityMapOffset = copy_src.fragmentDensityMapOffset;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM::~safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM::initialize(
    const VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fragmentDensityMapOffset = in_struct->fragmentDensityMapOffset;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM::initialize(
    const safe_VkPhysicalDeviceFragmentDensityMapOffsetFeaturesQCOM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fragmentDensityMapOffset = copy_src->fragmentDensityMapOffset;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM::safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM(
    const VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), fragmentDensityOffsetGranularity(in_struct->fragmentDensityOffsetGranularity) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM::safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FRAGMENT_DENSITY_MAP_OFFSET_PROPERTIES_QCOM),
      pNext(nullptr),
      fragmentDensityOffsetGranularity() {}

safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM::safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM(
    const safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM& copy_src) {
    sType = copy_src.sType;
    fragmentDensityOffsetGranularity = copy_src.fragmentDensityOffsetGranularity;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM& safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM::operator=(
    const safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    fragmentDensityOffsetGranularity = copy_src.fragmentDensityOffsetGranularity;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM::~safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM::initialize(
    const VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fragmentDensityOffsetGranularity = in_struct->fragmentDensityOffsetGranularity;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM::initialize(
    const safe_VkPhysicalDeviceFragmentDensityMapOffsetPropertiesQCOM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fragmentDensityOffsetGranularity = copy_src->fragmentDensityOffsetGranularity;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSubpassFragmentDensityMapOffsetEndInfoQCOM::safe_VkSubpassFragmentDensityMapOffsetEndInfoQCOM(
    const VkSubpassFragmentDensityMapOffsetEndInfoQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), fragmentDensityOffsetCount(in_struct->fragmentDensityOffsetCount), pFragmentDensityOffsets(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pFragmentDensityOffsets) {
        pFragmentDensityOffsets = new VkOffset2D[in_struct->fragmentDensityOffsetCount];
        memcpy((void*)pFragmentDensityOffsets, (void*)in_struct->pFragmentDensityOffsets,
               sizeof(VkOffset2D) * in_struct->fragmentDensityOffsetCount);
    }
}

safe_VkSubpassFragmentDensityMapOffsetEndInfoQCOM::safe_VkSubpassFragmentDensityMapOffsetEndInfoQCOM()
    : sType(VK_STRUCTURE_TYPE_SUBPASS_FRAGMENT_DENSITY_MAP_OFFSET_END_INFO_QCOM),
      pNext(nullptr),
      fragmentDensityOffsetCount(),
      pFragmentDensityOffsets(nullptr) {}

safe_VkSubpassFragmentDensityMapOffsetEndInfoQCOM::safe_VkSubpassFragmentDensityMapOffsetEndInfoQCOM(
    const safe_VkSubpassFragmentDensityMapOffsetEndInfoQCOM& copy_src) {
    sType = copy_src.sType;
    fragmentDensityOffsetCount = copy_src.fragmentDensityOffsetCount;
    pFragmentDensityOffsets = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pFragmentDensityOffsets) {
        pFragmentDensityOffsets = new VkOffset2D[copy_src.fragmentDensityOffsetCount];
        memcpy((void*)pFragmentDensityOffsets, (void*)copy_src.pFragmentDensityOffsets,
               sizeof(VkOffset2D) * copy_src.fragmentDensityOffsetCount);
    }
}

safe_VkSubpassFragmentDensityMapOffsetEndInfoQCOM& safe_VkSubpassFragmentDensityMapOffsetEndInfoQCOM::operator=(
    const safe_VkSubpassFragmentDensityMapOffsetEndInfoQCOM& copy_src) {
    if (&copy_src == this) return *this;

    if (pFragmentDensityOffsets) delete[] pFragmentDensityOffsets;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    fragmentDensityOffsetCount = copy_src.fragmentDensityOffsetCount;
    pFragmentDensityOffsets = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pFragmentDensityOffsets) {
        pFragmentDensityOffsets = new VkOffset2D[copy_src.fragmentDensityOffsetCount];
        memcpy((void*)pFragmentDensityOffsets, (void*)copy_src.pFragmentDensityOffsets,
               sizeof(VkOffset2D) * copy_src.fragmentDensityOffsetCount);
    }

    return *this;
}

safe_VkSubpassFragmentDensityMapOffsetEndInfoQCOM::~safe_VkSubpassFragmentDensityMapOffsetEndInfoQCOM() {
    if (pFragmentDensityOffsets) delete[] pFragmentDensityOffsets;
    FreePnextChain(pNext);
}

void safe_VkSubpassFragmentDensityMapOffsetEndInfoQCOM::initialize(const VkSubpassFragmentDensityMapOffsetEndInfoQCOM* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    if (pFragmentDensityOffsets) delete[] pFragmentDensityOffsets;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    fragmentDensityOffsetCount = in_struct->fragmentDensityOffsetCount;
    pFragmentDensityOffsets = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pFragmentDensityOffsets) {
        pFragmentDensityOffsets = new VkOffset2D[in_struct->fragmentDensityOffsetCount];
        memcpy((void*)pFragmentDensityOffsets, (void*)in_struct->pFragmentDensityOffsets,
               sizeof(VkOffset2D) * in_struct->fragmentDensityOffsetCount);
    }
}

void safe_VkSubpassFragmentDensityMapOffsetEndInfoQCOM::initialize(
    const safe_VkSubpassFragmentDensityMapOffsetEndInfoQCOM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    fragmentDensityOffsetCount = copy_src->fragmentDensityOffsetCount;
    pFragmentDensityOffsets = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pFragmentDensityOffsets) {
        pFragmentDensityOffsets = new VkOffset2D[copy_src->fragmentDensityOffsetCount];
        memcpy((void*)pFragmentDensityOffsets, (void*)copy_src->pFragmentDensityOffsets,
               sizeof(VkOffset2D) * copy_src->fragmentDensityOffsetCount);
    }
}

safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV::safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV(
    const VkPhysicalDeviceCopyMemoryIndirectFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), indirectCopy(in_struct->indirectCopy) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV::safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_FEATURES_NV), pNext(nullptr), indirectCopy() {}

safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV::safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV(
    const safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV& copy_src) {
    sType = copy_src.sType;
    indirectCopy = copy_src.indirectCopy;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV& safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV::operator=(
    const safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    indirectCopy = copy_src.indirectCopy;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV::~safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV::initialize(const VkPhysicalDeviceCopyMemoryIndirectFeaturesNV* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    indirectCopy = in_struct->indirectCopy;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV::initialize(
    const safe_VkPhysicalDeviceCopyMemoryIndirectFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    indirectCopy = copy_src->indirectCopy;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV::safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV(
    const VkPhysicalDeviceCopyMemoryIndirectPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), supportedQueues(in_struct->supportedQueues) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV::safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COPY_MEMORY_INDIRECT_PROPERTIES_NV), pNext(nullptr), supportedQueues() {}

safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV::safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV(
    const safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV& copy_src) {
    sType = copy_src.sType;
    supportedQueues = copy_src.supportedQueues;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV& safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV::operator=(
    const safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    supportedQueues = copy_src.supportedQueues;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV::~safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV::initialize(
    const VkPhysicalDeviceCopyMemoryIndirectPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    supportedQueues = in_struct->supportedQueues;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV::initialize(
    const safe_VkPhysicalDeviceCopyMemoryIndirectPropertiesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    supportedQueues = copy_src->supportedQueues;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV::safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV(
    const VkPhysicalDeviceMemoryDecompressionFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), memoryDecompression(in_struct->memoryDecompression) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV::safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_FEATURES_NV), pNext(nullptr), memoryDecompression() {}

safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV::safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV(
    const safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV& copy_src) {
    sType = copy_src.sType;
    memoryDecompression = copy_src.memoryDecompression;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV& safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV::operator=(
    const safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    memoryDecompression = copy_src.memoryDecompression;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV::~safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV::initialize(const VkPhysicalDeviceMemoryDecompressionFeaturesNV* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    memoryDecompression = in_struct->memoryDecompression;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV::initialize(
    const safe_VkPhysicalDeviceMemoryDecompressionFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    memoryDecompression = copy_src->memoryDecompression;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV::safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV(
    const VkPhysicalDeviceMemoryDecompressionPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      decompressionMethods(in_struct->decompressionMethods),
      maxDecompressionIndirectCount(in_struct->maxDecompressionIndirectCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV::safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_DECOMPRESSION_PROPERTIES_NV),
      pNext(nullptr),
      decompressionMethods(),
      maxDecompressionIndirectCount() {}

safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV::safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV(
    const safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV& copy_src) {
    sType = copy_src.sType;
    decompressionMethods = copy_src.decompressionMethods;
    maxDecompressionIndirectCount = copy_src.maxDecompressionIndirectCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV& safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV::operator=(
    const safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    decompressionMethods = copy_src.decompressionMethods;
    maxDecompressionIndirectCount = copy_src.maxDecompressionIndirectCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV::~safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV::initialize(
    const VkPhysicalDeviceMemoryDecompressionPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    decompressionMethods = in_struct->decompressionMethods;
    maxDecompressionIndirectCount = in_struct->maxDecompressionIndirectCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV::initialize(
    const safe_VkPhysicalDeviceMemoryDecompressionPropertiesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    decompressionMethods = copy_src->decompressionMethods;
    maxDecompressionIndirectCount = copy_src->maxDecompressionIndirectCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV::safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV(
    const VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      deviceGeneratedCompute(in_struct->deviceGeneratedCompute),
      deviceGeneratedComputePipelines(in_struct->deviceGeneratedComputePipelines),
      deviceGeneratedComputeCaptureReplay(in_struct->deviceGeneratedComputeCaptureReplay) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV::safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DEVICE_GENERATED_COMMANDS_COMPUTE_FEATURES_NV),
      pNext(nullptr),
      deviceGeneratedCompute(),
      deviceGeneratedComputePipelines(),
      deviceGeneratedComputeCaptureReplay() {}

safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV::safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV(
    const safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV& copy_src) {
    sType = copy_src.sType;
    deviceGeneratedCompute = copy_src.deviceGeneratedCompute;
    deviceGeneratedComputePipelines = copy_src.deviceGeneratedComputePipelines;
    deviceGeneratedComputeCaptureReplay = copy_src.deviceGeneratedComputeCaptureReplay;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV&
safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV::operator=(
    const safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    deviceGeneratedCompute = copy_src.deviceGeneratedCompute;
    deviceGeneratedComputePipelines = copy_src.deviceGeneratedComputePipelines;
    deviceGeneratedComputeCaptureReplay = copy_src.deviceGeneratedComputeCaptureReplay;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV::~safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV::initialize(
    const VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    deviceGeneratedCompute = in_struct->deviceGeneratedCompute;
    deviceGeneratedComputePipelines = in_struct->deviceGeneratedComputePipelines;
    deviceGeneratedComputeCaptureReplay = in_struct->deviceGeneratedComputeCaptureReplay;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV::initialize(
    const safe_VkPhysicalDeviceDeviceGeneratedCommandsComputeFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    deviceGeneratedCompute = copy_src->deviceGeneratedCompute;
    deviceGeneratedComputePipelines = copy_src->deviceGeneratedComputePipelines;
    deviceGeneratedComputeCaptureReplay = copy_src->deviceGeneratedComputeCaptureReplay;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkComputePipelineIndirectBufferInfoNV::safe_VkComputePipelineIndirectBufferInfoNV(
    const VkComputePipelineIndirectBufferInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      deviceAddress(in_struct->deviceAddress),
      size(in_struct->size),
      pipelineDeviceAddressCaptureReplay(in_struct->pipelineDeviceAddressCaptureReplay) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkComputePipelineIndirectBufferInfoNV::safe_VkComputePipelineIndirectBufferInfoNV()
    : sType(VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_INDIRECT_BUFFER_INFO_NV),
      pNext(nullptr),
      deviceAddress(),
      size(),
      pipelineDeviceAddressCaptureReplay() {}

safe_VkComputePipelineIndirectBufferInfoNV::safe_VkComputePipelineIndirectBufferInfoNV(
    const safe_VkComputePipelineIndirectBufferInfoNV& copy_src) {
    sType = copy_src.sType;
    deviceAddress = copy_src.deviceAddress;
    size = copy_src.size;
    pipelineDeviceAddressCaptureReplay = copy_src.pipelineDeviceAddressCaptureReplay;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkComputePipelineIndirectBufferInfoNV& safe_VkComputePipelineIndirectBufferInfoNV::operator=(
    const safe_VkComputePipelineIndirectBufferInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    deviceAddress = copy_src.deviceAddress;
    size = copy_src.size;
    pipelineDeviceAddressCaptureReplay = copy_src.pipelineDeviceAddressCaptureReplay;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkComputePipelineIndirectBufferInfoNV::~safe_VkComputePipelineIndirectBufferInfoNV() { FreePnextChain(pNext); }

void safe_VkComputePipelineIndirectBufferInfoNV::initialize(const VkComputePipelineIndirectBufferInfoNV* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    deviceAddress = in_struct->deviceAddress;
    size = in_struct->size;
    pipelineDeviceAddressCaptureReplay = in_struct->pipelineDeviceAddressCaptureReplay;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkComputePipelineIndirectBufferInfoNV::initialize(const safe_VkComputePipelineIndirectBufferInfoNV* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    deviceAddress = copy_src->deviceAddress;
    size = copy_src->size;
    pipelineDeviceAddressCaptureReplay = copy_src->pipelineDeviceAddressCaptureReplay;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPipelineIndirectDeviceAddressInfoNV::safe_VkPipelineIndirectDeviceAddressInfoNV(
    const VkPipelineIndirectDeviceAddressInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), pipelineBindPoint(in_struct->pipelineBindPoint), pipeline(in_struct->pipeline) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPipelineIndirectDeviceAddressInfoNV::safe_VkPipelineIndirectDeviceAddressInfoNV()
    : sType(VK_STRUCTURE_TYPE_PIPELINE_INDIRECT_DEVICE_ADDRESS_INFO_NV), pNext(nullptr), pipelineBindPoint(), pipeline() {}

safe_VkPipelineIndirectDeviceAddressInfoNV::safe_VkPipelineIndirectDeviceAddressInfoNV(
    const safe_VkPipelineIndirectDeviceAddressInfoNV& copy_src) {
    sType = copy_src.sType;
    pipelineBindPoint = copy_src.pipelineBindPoint;
    pipeline = copy_src.pipeline;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPipelineIndirectDeviceAddressInfoNV& safe_VkPipelineIndirectDeviceAddressInfoNV::operator=(
    const safe_VkPipelineIndirectDeviceAddressInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipelineBindPoint = copy_src.pipelineBindPoint;
    pipeline = copy_src.pipeline;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPipelineIndirectDeviceAddressInfoNV::~safe_VkPipelineIndirectDeviceAddressInfoNV() { FreePnextChain(pNext); }

void safe_VkPipelineIndirectDeviceAddressInfoNV::initialize(const VkPipelineIndirectDeviceAddressInfoNV* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipelineBindPoint = in_struct->pipelineBindPoint;
    pipeline = in_struct->pipeline;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPipelineIndirectDeviceAddressInfoNV::initialize(const safe_VkPipelineIndirectDeviceAddressInfoNV* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipelineBindPoint = copy_src->pipelineBindPoint;
    pipeline = copy_src->pipeline;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV::safe_VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV(
    const VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), spheres(in_struct->spheres), linearSweptSpheres(in_struct->linearSweptSpheres) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV::safe_VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_LINEAR_SWEPT_SPHERES_FEATURES_NV),
      pNext(nullptr),
      spheres(),
      linearSweptSpheres() {}

safe_VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV::safe_VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV(
    const safe_VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV& copy_src) {
    sType = copy_src.sType;
    spheres = copy_src.spheres;
    linearSweptSpheres = copy_src.linearSweptSpheres;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV& safe_VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV::operator=(
    const safe_VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    spheres = copy_src.spheres;
    linearSweptSpheres = copy_src.linearSweptSpheres;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV::~safe_VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV::initialize(
    const VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    spheres = in_struct->spheres;
    linearSweptSpheres = in_struct->linearSweptSpheres;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV::initialize(
    const safe_VkPhysicalDeviceRayTracingLinearSweptSpheresFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    spheres = copy_src->spheres;
    linearSweptSpheres = copy_src->linearSweptSpheres;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAccelerationStructureGeometryLinearSweptSpheresDataNV::safe_VkAccelerationStructureGeometryLinearSweptSpheresDataNV(
    const VkAccelerationStructureGeometryLinearSweptSpheresDataNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      vertexFormat(in_struct->vertexFormat),
      vertexData(&in_struct->vertexData),
      vertexStride(in_struct->vertexStride),
      radiusFormat(in_struct->radiusFormat),
      radiusData(&in_struct->radiusData),
      radiusStride(in_struct->radiusStride),
      indexType(in_struct->indexType),
      indexData(&in_struct->indexData),
      indexStride(in_struct->indexStride),
      indexingMode(in_struct->indexingMode),
      endCapsMode(in_struct->endCapsMode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAccelerationStructureGeometryLinearSweptSpheresDataNV::safe_VkAccelerationStructureGeometryLinearSweptSpheresDataNV()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_LINEAR_SWEPT_SPHERES_DATA_NV),
      pNext(nullptr),
      vertexFormat(),
      vertexStride(),
      radiusFormat(),
      radiusStride(),
      indexType(),
      indexStride(),
      indexingMode(),
      endCapsMode() {}

safe_VkAccelerationStructureGeometryLinearSweptSpheresDataNV::safe_VkAccelerationStructureGeometryLinearSweptSpheresDataNV(
    const safe_VkAccelerationStructureGeometryLinearSweptSpheresDataNV& copy_src) {
    sType = copy_src.sType;
    vertexFormat = copy_src.vertexFormat;
    vertexData.initialize(&copy_src.vertexData);
    vertexStride = copy_src.vertexStride;
    radiusFormat = copy_src.radiusFormat;
    radiusData.initialize(&copy_src.radiusData);
    radiusStride = copy_src.radiusStride;
    indexType = copy_src.indexType;
    indexData.initialize(&copy_src.indexData);
    indexStride = copy_src.indexStride;
    indexingMode = copy_src.indexingMode;
    endCapsMode = copy_src.endCapsMode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureGeometryLinearSweptSpheresDataNV&
safe_VkAccelerationStructureGeometryLinearSweptSpheresDataNV::operator=(
    const safe_VkAccelerationStructureGeometryLinearSweptSpheresDataNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    vertexFormat = copy_src.vertexFormat;
    vertexData.initialize(&copy_src.vertexData);
    vertexStride = copy_src.vertexStride;
    radiusFormat = copy_src.radiusFormat;
    radiusData.initialize(&copy_src.radiusData);
    radiusStride = copy_src.radiusStride;
    indexType = copy_src.indexType;
    indexData.initialize(&copy_src.indexData);
    indexStride = copy_src.indexStride;
    indexingMode = copy_src.indexingMode;
    endCapsMode = copy_src.endCapsMode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAccelerationStructureGeometryLinearSweptSpheresDataNV::~safe_VkAccelerationStructureGeometryLinearSweptSpheresDataNV() {
    FreePnextChain(pNext);
}

void safe_VkAccelerationStructureGeometryLinearSweptSpheresDataNV::initialize(
    const VkAccelerationStructureGeometryLinearSweptSpheresDataNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    vertexFormat = in_struct->vertexFormat;
    vertexData.initialize(&in_struct->vertexData);
    vertexStride = in_struct->vertexStride;
    radiusFormat = in_struct->radiusFormat;
    radiusData.initialize(&in_struct->radiusData);
    radiusStride = in_struct->radiusStride;
    indexType = in_struct->indexType;
    indexData.initialize(&in_struct->indexData);
    indexStride = in_struct->indexStride;
    indexingMode = in_struct->indexingMode;
    endCapsMode = in_struct->endCapsMode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAccelerationStructureGeometryLinearSweptSpheresDataNV::initialize(
    const safe_VkAccelerationStructureGeometryLinearSweptSpheresDataNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    vertexFormat = copy_src->vertexFormat;
    vertexData.initialize(&copy_src->vertexData);
    vertexStride = copy_src->vertexStride;
    radiusFormat = copy_src->radiusFormat;
    radiusData.initialize(&copy_src->radiusData);
    radiusStride = copy_src->radiusStride;
    indexType = copy_src->indexType;
    indexData.initialize(&copy_src->indexData);
    indexStride = copy_src->indexStride;
    indexingMode = copy_src->indexingMode;
    endCapsMode = copy_src->endCapsMode;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAccelerationStructureGeometrySpheresDataNV::safe_VkAccelerationStructureGeometrySpheresDataNV(
    const VkAccelerationStructureGeometrySpheresDataNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      vertexFormat(in_struct->vertexFormat),
      vertexData(&in_struct->vertexData),
      vertexStride(in_struct->vertexStride),
      radiusFormat(in_struct->radiusFormat),
      radiusData(&in_struct->radiusData),
      radiusStride(in_struct->radiusStride),
      indexType(in_struct->indexType),
      indexData(&in_struct->indexData),
      indexStride(in_struct->indexStride) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAccelerationStructureGeometrySpheresDataNV::safe_VkAccelerationStructureGeometrySpheresDataNV()
    : sType(VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_SPHERES_DATA_NV),
      pNext(nullptr),
      vertexFormat(),
      vertexStride(),
      radiusFormat(),
      radiusStride(),
      indexType(),
      indexStride() {}

safe_VkAccelerationStructureGeometrySpheresDataNV::safe_VkAccelerationStructureGeometrySpheresDataNV(
    const safe_VkAccelerationStructureGeometrySpheresDataNV& copy_src) {
    sType = copy_src.sType;
    vertexFormat = copy_src.vertexFormat;
    vertexData.initialize(&copy_src.vertexData);
    vertexStride = copy_src.vertexStride;
    radiusFormat = copy_src.radiusFormat;
    radiusData.initialize(&copy_src.radiusData);
    radiusStride = copy_src.radiusStride;
    indexType = copy_src.indexType;
    indexData.initialize(&copy_src.indexData);
    indexStride = copy_src.indexStride;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAccelerationStructureGeometrySpheresDataNV& safe_VkAccelerationStructureGeometrySpheresDataNV::operator=(
    const safe_VkAccelerationStructureGeometrySpheresDataNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    vertexFormat = copy_src.vertexFormat;
    vertexData.initialize(&copy_src.vertexData);
    vertexStride = copy_src.vertexStride;
    radiusFormat = copy_src.radiusFormat;
    radiusData.initialize(&copy_src.radiusData);
    radiusStride = copy_src.radiusStride;
    indexType = copy_src.indexType;
    indexData.initialize(&copy_src.indexData);
    indexStride = copy_src.indexStride;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAccelerationStructureGeometrySpheresDataNV::~safe_VkAccelerationStructureGeometrySpheresDataNV() { FreePnextChain(pNext); }

void safe_VkAccelerationStructureGeometrySpheresDataNV::initialize(const VkAccelerationStructureGeometrySpheresDataNV* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    vertexFormat = in_struct->vertexFormat;
    vertexData.initialize(&in_struct->vertexData);
    vertexStride = in_struct->vertexStride;
    radiusFormat = in_struct->radiusFormat;
    radiusData.initialize(&in_struct->radiusData);
    radiusStride = in_struct->radiusStride;
    indexType = in_struct->indexType;
    indexData.initialize(&in_struct->indexData);
    indexStride = in_struct->indexStride;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAccelerationStructureGeometrySpheresDataNV::initialize(
    const safe_VkAccelerationStructureGeometrySpheresDataNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    vertexFormat = copy_src->vertexFormat;
    vertexData.initialize(&copy_src->vertexData);
    vertexStride = copy_src->vertexStride;
    radiusFormat = copy_src->radiusFormat;
    radiusData.initialize(&copy_src->radiusData);
    radiusStride = copy_src->radiusStride;
    indexType = copy_src->indexType;
    indexData.initialize(&copy_src->indexData);
    indexStride = copy_src->indexStride;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV::safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV(
    const VkPhysicalDeviceLinearColorAttachmentFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), linearColorAttachment(in_struct->linearColorAttachment) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV::safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LINEAR_COLOR_ATTACHMENT_FEATURES_NV), pNext(nullptr), linearColorAttachment() {}

safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV::safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV(
    const safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV& copy_src) {
    sType = copy_src.sType;
    linearColorAttachment = copy_src.linearColorAttachment;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV& safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV::operator=(
    const safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    linearColorAttachment = copy_src.linearColorAttachment;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV::~safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV::initialize(
    const VkPhysicalDeviceLinearColorAttachmentFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    linearColorAttachment = in_struct->linearColorAttachment;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV::initialize(
    const safe_VkPhysicalDeviceLinearColorAttachmentFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    linearColorAttachment = copy_src->linearColorAttachment;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImageViewSampleWeightCreateInfoQCOM::safe_VkImageViewSampleWeightCreateInfoQCOM(
    const VkImageViewSampleWeightCreateInfoQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      filterCenter(in_struct->filterCenter),
      filterSize(in_struct->filterSize),
      numPhases(in_struct->numPhases) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImageViewSampleWeightCreateInfoQCOM::safe_VkImageViewSampleWeightCreateInfoQCOM()
    : sType(VK_STRUCTURE_TYPE_IMAGE_VIEW_SAMPLE_WEIGHT_CREATE_INFO_QCOM),
      pNext(nullptr),
      filterCenter(),
      filterSize(),
      numPhases() {}

safe_VkImageViewSampleWeightCreateInfoQCOM::safe_VkImageViewSampleWeightCreateInfoQCOM(
    const safe_VkImageViewSampleWeightCreateInfoQCOM& copy_src) {
    sType = copy_src.sType;
    filterCenter = copy_src.filterCenter;
    filterSize = copy_src.filterSize;
    numPhases = copy_src.numPhases;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImageViewSampleWeightCreateInfoQCOM& safe_VkImageViewSampleWeightCreateInfoQCOM::operator=(
    const safe_VkImageViewSampleWeightCreateInfoQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    filterCenter = copy_src.filterCenter;
    filterSize = copy_src.filterSize;
    numPhases = copy_src.numPhases;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImageViewSampleWeightCreateInfoQCOM::~safe_VkImageViewSampleWeightCreateInfoQCOM() { FreePnextChain(pNext); }

void safe_VkImageViewSampleWeightCreateInfoQCOM::initialize(const VkImageViewSampleWeightCreateInfoQCOM* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    filterCenter = in_struct->filterCenter;
    filterSize = in_struct->filterSize;
    numPhases = in_struct->numPhases;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImageViewSampleWeightCreateInfoQCOM::initialize(const safe_VkImageViewSampleWeightCreateInfoQCOM* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    filterCenter = copy_src->filterCenter;
    filterSize = copy_src->filterSize;
    numPhases = copy_src->numPhases;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceImageProcessingFeaturesQCOM::safe_VkPhysicalDeviceImageProcessingFeaturesQCOM(
    const VkPhysicalDeviceImageProcessingFeaturesQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      textureSampleWeighted(in_struct->textureSampleWeighted),
      textureBoxFilter(in_struct->textureBoxFilter),
      textureBlockMatch(in_struct->textureBlockMatch) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceImageProcessingFeaturesQCOM::safe_VkPhysicalDeviceImageProcessingFeaturesQCOM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_FEATURES_QCOM),
      pNext(nullptr),
      textureSampleWeighted(),
      textureBoxFilter(),
      textureBlockMatch() {}

safe_VkPhysicalDeviceImageProcessingFeaturesQCOM::safe_VkPhysicalDeviceImageProcessingFeaturesQCOM(
    const safe_VkPhysicalDeviceImageProcessingFeaturesQCOM& copy_src) {
    sType = copy_src.sType;
    textureSampleWeighted = copy_src.textureSampleWeighted;
    textureBoxFilter = copy_src.textureBoxFilter;
    textureBlockMatch = copy_src.textureBlockMatch;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceImageProcessingFeaturesQCOM& safe_VkPhysicalDeviceImageProcessingFeaturesQCOM::operator=(
    const safe_VkPhysicalDeviceImageProcessingFeaturesQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    textureSampleWeighted = copy_src.textureSampleWeighted;
    textureBoxFilter = copy_src.textureBoxFilter;
    textureBlockMatch = copy_src.textureBlockMatch;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceImageProcessingFeaturesQCOM::~safe_VkPhysicalDeviceImageProcessingFeaturesQCOM() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceImageProcessingFeaturesQCOM::initialize(const VkPhysicalDeviceImageProcessingFeaturesQCOM* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    textureSampleWeighted = in_struct->textureSampleWeighted;
    textureBoxFilter = in_struct->textureBoxFilter;
    textureBlockMatch = in_struct->textureBlockMatch;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceImageProcessingFeaturesQCOM::initialize(const safe_VkPhysicalDeviceImageProcessingFeaturesQCOM* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    textureSampleWeighted = copy_src->textureSampleWeighted;
    textureBoxFilter = copy_src->textureBoxFilter;
    textureBlockMatch = copy_src->textureBlockMatch;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceImageProcessingPropertiesQCOM::safe_VkPhysicalDeviceImageProcessingPropertiesQCOM(
    const VkPhysicalDeviceImageProcessingPropertiesQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      maxWeightFilterPhases(in_struct->maxWeightFilterPhases),
      maxWeightFilterDimension(in_struct->maxWeightFilterDimension),
      maxBlockMatchRegion(in_struct->maxBlockMatchRegion),
      maxBoxFilterBlockSize(in_struct->maxBoxFilterBlockSize) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceImageProcessingPropertiesQCOM::safe_VkPhysicalDeviceImageProcessingPropertiesQCOM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_PROPERTIES_QCOM),
      pNext(nullptr),
      maxWeightFilterPhases(),
      maxWeightFilterDimension(),
      maxBlockMatchRegion(),
      maxBoxFilterBlockSize() {}

safe_VkPhysicalDeviceImageProcessingPropertiesQCOM::safe_VkPhysicalDeviceImageProcessingPropertiesQCOM(
    const safe_VkPhysicalDeviceImageProcessingPropertiesQCOM& copy_src) {
    sType = copy_src.sType;
    maxWeightFilterPhases = copy_src.maxWeightFilterPhases;
    maxWeightFilterDimension = copy_src.maxWeightFilterDimension;
    maxBlockMatchRegion = copy_src.maxBlockMatchRegion;
    maxBoxFilterBlockSize = copy_src.maxBoxFilterBlockSize;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceImageProcessingPropertiesQCOM& safe_VkPhysicalDeviceImageProcessingPropertiesQCOM::operator=(
    const safe_VkPhysicalDeviceImageProcessingPropertiesQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxWeightFilterPhases = copy_src.maxWeightFilterPhases;
    maxWeightFilterDimension = copy_src.maxWeightFilterDimension;
    maxBlockMatchRegion = copy_src.maxBlockMatchRegion;
    maxBoxFilterBlockSize = copy_src.maxBoxFilterBlockSize;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceImageProcessingPropertiesQCOM::~safe_VkPhysicalDeviceImageProcessingPropertiesQCOM() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceImageProcessingPropertiesQCOM::initialize(const VkPhysicalDeviceImageProcessingPropertiesQCOM* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxWeightFilterPhases = in_struct->maxWeightFilterPhases;
    maxWeightFilterDimension = in_struct->maxWeightFilterDimension;
    maxBlockMatchRegion = in_struct->maxBlockMatchRegion;
    maxBoxFilterBlockSize = in_struct->maxBoxFilterBlockSize;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceImageProcessingPropertiesQCOM::initialize(
    const safe_VkPhysicalDeviceImageProcessingPropertiesQCOM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxWeightFilterPhases = copy_src->maxWeightFilterPhases;
    maxWeightFilterDimension = copy_src->maxWeightFilterDimension;
    maxBlockMatchRegion = copy_src->maxBlockMatchRegion;
    maxBoxFilterBlockSize = copy_src->maxBoxFilterBlockSize;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDirectDriverLoadingInfoLUNARG::safe_VkDirectDriverLoadingInfoLUNARG(const VkDirectDriverLoadingInfoLUNARG* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags), pfnGetInstanceProcAddr(in_struct->pfnGetInstanceProcAddr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDirectDriverLoadingInfoLUNARG::safe_VkDirectDriverLoadingInfoLUNARG()
    : sType(VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_INFO_LUNARG), pNext(nullptr), flags(), pfnGetInstanceProcAddr() {}

safe_VkDirectDriverLoadingInfoLUNARG::safe_VkDirectDriverLoadingInfoLUNARG(const safe_VkDirectDriverLoadingInfoLUNARG& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    pfnGetInstanceProcAddr = copy_src.pfnGetInstanceProcAddr;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDirectDriverLoadingInfoLUNARG& safe_VkDirectDriverLoadingInfoLUNARG::operator=(
    const safe_VkDirectDriverLoadingInfoLUNARG& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    pfnGetInstanceProcAddr = copy_src.pfnGetInstanceProcAddr;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDirectDriverLoadingInfoLUNARG::~safe_VkDirectDriverLoadingInfoLUNARG() { FreePnextChain(pNext); }

void safe_VkDirectDriverLoadingInfoLUNARG::initialize(const VkDirectDriverLoadingInfoLUNARG* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    pfnGetInstanceProcAddr = in_struct->pfnGetInstanceProcAddr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDirectDriverLoadingInfoLUNARG::initialize(const safe_VkDirectDriverLoadingInfoLUNARG* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    pfnGetInstanceProcAddr = copy_src->pfnGetInstanceProcAddr;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDirectDriverLoadingListLUNARG::safe_VkDirectDriverLoadingListLUNARG(const VkDirectDriverLoadingListLUNARG* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), mode(in_struct->mode), driverCount(in_struct->driverCount), pDrivers(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (driverCount && in_struct->pDrivers) {
        pDrivers = new safe_VkDirectDriverLoadingInfoLUNARG[driverCount];
        for (uint32_t i = 0; i < driverCount; ++i) {
            pDrivers[i].initialize(&in_struct->pDrivers[i]);
        }
    }
}

safe_VkDirectDriverLoadingListLUNARG::safe_VkDirectDriverLoadingListLUNARG()
    : sType(VK_STRUCTURE_TYPE_DIRECT_DRIVER_LOADING_LIST_LUNARG), pNext(nullptr), mode(), driverCount(), pDrivers(nullptr) {}

safe_VkDirectDriverLoadingListLUNARG::safe_VkDirectDriverLoadingListLUNARG(const safe_VkDirectDriverLoadingListLUNARG& copy_src) {
    sType = copy_src.sType;
    mode = copy_src.mode;
    driverCount = copy_src.driverCount;
    pDrivers = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (driverCount && copy_src.pDrivers) {
        pDrivers = new safe_VkDirectDriverLoadingInfoLUNARG[driverCount];
        for (uint32_t i = 0; i < driverCount; ++i) {
            pDrivers[i].initialize(&copy_src.pDrivers[i]);
        }
    }
}

safe_VkDirectDriverLoadingListLUNARG& safe_VkDirectDriverLoadingListLUNARG::operator=(
    const safe_VkDirectDriverLoadingListLUNARG& copy_src) {
    if (&copy_src == this) return *this;

    if (pDrivers) delete[] pDrivers;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    mode = copy_src.mode;
    driverCount = copy_src.driverCount;
    pDrivers = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (driverCount && copy_src.pDrivers) {
        pDrivers = new safe_VkDirectDriverLoadingInfoLUNARG[driverCount];
        for (uint32_t i = 0; i < driverCount; ++i) {
            pDrivers[i].initialize(&copy_src.pDrivers[i]);
        }
    }

    return *this;
}

safe_VkDirectDriverLoadingListLUNARG::~safe_VkDirectDriverLoadingListLUNARG() {
    if (pDrivers) delete[] pDrivers;
    FreePnextChain(pNext);
}

void safe_VkDirectDriverLoadingListLUNARG::initialize(const VkDirectDriverLoadingListLUNARG* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    if (pDrivers) delete[] pDrivers;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    mode = in_struct->mode;
    driverCount = in_struct->driverCount;
    pDrivers = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (driverCount && in_struct->pDrivers) {
        pDrivers = new safe_VkDirectDriverLoadingInfoLUNARG[driverCount];
        for (uint32_t i = 0; i < driverCount; ++i) {
            pDrivers[i].initialize(&in_struct->pDrivers[i]);
        }
    }
}

void safe_VkDirectDriverLoadingListLUNARG::initialize(const safe_VkDirectDriverLoadingListLUNARG* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    mode = copy_src->mode;
    driverCount = copy_src->driverCount;
    pDrivers = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (driverCount && copy_src->pDrivers) {
        pDrivers = new safe_VkDirectDriverLoadingInfoLUNARG[driverCount];
        for (uint32_t i = 0; i < driverCount; ++i) {
            pDrivers[i].initialize(&copy_src->pDrivers[i]);
        }
    }
}

safe_VkPhysicalDeviceOpticalFlowFeaturesNV::safe_VkPhysicalDeviceOpticalFlowFeaturesNV(
    const VkPhysicalDeviceOpticalFlowFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), opticalFlow(in_struct->opticalFlow) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceOpticalFlowFeaturesNV::safe_VkPhysicalDeviceOpticalFlowFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_FEATURES_NV), pNext(nullptr), opticalFlow() {}

safe_VkPhysicalDeviceOpticalFlowFeaturesNV::safe_VkPhysicalDeviceOpticalFlowFeaturesNV(
    const safe_VkPhysicalDeviceOpticalFlowFeaturesNV& copy_src) {
    sType = copy_src.sType;
    opticalFlow = copy_src.opticalFlow;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceOpticalFlowFeaturesNV& safe_VkPhysicalDeviceOpticalFlowFeaturesNV::operator=(
    const safe_VkPhysicalDeviceOpticalFlowFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    opticalFlow = copy_src.opticalFlow;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceOpticalFlowFeaturesNV::~safe_VkPhysicalDeviceOpticalFlowFeaturesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceOpticalFlowFeaturesNV::initialize(const VkPhysicalDeviceOpticalFlowFeaturesNV* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    opticalFlow = in_struct->opticalFlow;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceOpticalFlowFeaturesNV::initialize(const safe_VkPhysicalDeviceOpticalFlowFeaturesNV* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    opticalFlow = copy_src->opticalFlow;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceOpticalFlowPropertiesNV::safe_VkPhysicalDeviceOpticalFlowPropertiesNV(
    const VkPhysicalDeviceOpticalFlowPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      supportedOutputGridSizes(in_struct->supportedOutputGridSizes),
      supportedHintGridSizes(in_struct->supportedHintGridSizes),
      hintSupported(in_struct->hintSupported),
      costSupported(in_struct->costSupported),
      bidirectionalFlowSupported(in_struct->bidirectionalFlowSupported),
      globalFlowSupported(in_struct->globalFlowSupported),
      minWidth(in_struct->minWidth),
      minHeight(in_struct->minHeight),
      maxWidth(in_struct->maxWidth),
      maxHeight(in_struct->maxHeight),
      maxNumRegionsOfInterest(in_struct->maxNumRegionsOfInterest) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceOpticalFlowPropertiesNV::safe_VkPhysicalDeviceOpticalFlowPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_OPTICAL_FLOW_PROPERTIES_NV),
      pNext(nullptr),
      supportedOutputGridSizes(),
      supportedHintGridSizes(),
      hintSupported(),
      costSupported(),
      bidirectionalFlowSupported(),
      globalFlowSupported(),
      minWidth(),
      minHeight(),
      maxWidth(),
      maxHeight(),
      maxNumRegionsOfInterest() {}

safe_VkPhysicalDeviceOpticalFlowPropertiesNV::safe_VkPhysicalDeviceOpticalFlowPropertiesNV(
    const safe_VkPhysicalDeviceOpticalFlowPropertiesNV& copy_src) {
    sType = copy_src.sType;
    supportedOutputGridSizes = copy_src.supportedOutputGridSizes;
    supportedHintGridSizes = copy_src.supportedHintGridSizes;
    hintSupported = copy_src.hintSupported;
    costSupported = copy_src.costSupported;
    bidirectionalFlowSupported = copy_src.bidirectionalFlowSupported;
    globalFlowSupported = copy_src.globalFlowSupported;
    minWidth = copy_src.minWidth;
    minHeight = copy_src.minHeight;
    maxWidth = copy_src.maxWidth;
    maxHeight = copy_src.maxHeight;
    maxNumRegionsOfInterest = copy_src.maxNumRegionsOfInterest;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceOpticalFlowPropertiesNV& safe_VkPhysicalDeviceOpticalFlowPropertiesNV::operator=(
    const safe_VkPhysicalDeviceOpticalFlowPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    supportedOutputGridSizes = copy_src.supportedOutputGridSizes;
    supportedHintGridSizes = copy_src.supportedHintGridSizes;
    hintSupported = copy_src.hintSupported;
    costSupported = copy_src.costSupported;
    bidirectionalFlowSupported = copy_src.bidirectionalFlowSupported;
    globalFlowSupported = copy_src.globalFlowSupported;
    minWidth = copy_src.minWidth;
    minHeight = copy_src.minHeight;
    maxWidth = copy_src.maxWidth;
    maxHeight = copy_src.maxHeight;
    maxNumRegionsOfInterest = copy_src.maxNumRegionsOfInterest;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceOpticalFlowPropertiesNV::~safe_VkPhysicalDeviceOpticalFlowPropertiesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceOpticalFlowPropertiesNV::initialize(const VkPhysicalDeviceOpticalFlowPropertiesNV* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    supportedOutputGridSizes = in_struct->supportedOutputGridSizes;
    supportedHintGridSizes = in_struct->supportedHintGridSizes;
    hintSupported = in_struct->hintSupported;
    costSupported = in_struct->costSupported;
    bidirectionalFlowSupported = in_struct->bidirectionalFlowSupported;
    globalFlowSupported = in_struct->globalFlowSupported;
    minWidth = in_struct->minWidth;
    minHeight = in_struct->minHeight;
    maxWidth = in_struct->maxWidth;
    maxHeight = in_struct->maxHeight;
    maxNumRegionsOfInterest = in_struct->maxNumRegionsOfInterest;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceOpticalFlowPropertiesNV::initialize(const safe_VkPhysicalDeviceOpticalFlowPropertiesNV* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    supportedOutputGridSizes = copy_src->supportedOutputGridSizes;
    supportedHintGridSizes = copy_src->supportedHintGridSizes;
    hintSupported = copy_src->hintSupported;
    costSupported = copy_src->costSupported;
    bidirectionalFlowSupported = copy_src->bidirectionalFlowSupported;
    globalFlowSupported = copy_src->globalFlowSupported;
    minWidth = copy_src->minWidth;
    minHeight = copy_src->minHeight;
    maxWidth = copy_src->maxWidth;
    maxHeight = copy_src->maxHeight;
    maxNumRegionsOfInterest = copy_src->maxNumRegionsOfInterest;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkOpticalFlowImageFormatInfoNV::safe_VkOpticalFlowImageFormatInfoNV(const VkOpticalFlowImageFormatInfoNV* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), usage(in_struct->usage) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkOpticalFlowImageFormatInfoNV::safe_VkOpticalFlowImageFormatInfoNV()
    : sType(VK_STRUCTURE_TYPE_OPTICAL_FLOW_IMAGE_FORMAT_INFO_NV), pNext(nullptr), usage() {}

safe_VkOpticalFlowImageFormatInfoNV::safe_VkOpticalFlowImageFormatInfoNV(const safe_VkOpticalFlowImageFormatInfoNV& copy_src) {
    sType = copy_src.sType;
    usage = copy_src.usage;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkOpticalFlowImageFormatInfoNV& safe_VkOpticalFlowImageFormatInfoNV::operator=(
    const safe_VkOpticalFlowImageFormatInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    usage = copy_src.usage;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkOpticalFlowImageFormatInfoNV::~safe_VkOpticalFlowImageFormatInfoNV() { FreePnextChain(pNext); }

void safe_VkOpticalFlowImageFormatInfoNV::initialize(const VkOpticalFlowImageFormatInfoNV* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    usage = in_struct->usage;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkOpticalFlowImageFormatInfoNV::initialize(const safe_VkOpticalFlowImageFormatInfoNV* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    usage = copy_src->usage;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkOpticalFlowImageFormatPropertiesNV::safe_VkOpticalFlowImageFormatPropertiesNV(
    const VkOpticalFlowImageFormatPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), format(in_struct->format) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkOpticalFlowImageFormatPropertiesNV::safe_VkOpticalFlowImageFormatPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_OPTICAL_FLOW_IMAGE_FORMAT_PROPERTIES_NV), pNext(nullptr), format() {}

safe_VkOpticalFlowImageFormatPropertiesNV::safe_VkOpticalFlowImageFormatPropertiesNV(
    const safe_VkOpticalFlowImageFormatPropertiesNV& copy_src) {
    sType = copy_src.sType;
    format = copy_src.format;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkOpticalFlowImageFormatPropertiesNV& safe_VkOpticalFlowImageFormatPropertiesNV::operator=(
    const safe_VkOpticalFlowImageFormatPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    format = copy_src.format;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkOpticalFlowImageFormatPropertiesNV::~safe_VkOpticalFlowImageFormatPropertiesNV() { FreePnextChain(pNext); }

void safe_VkOpticalFlowImageFormatPropertiesNV::initialize(const VkOpticalFlowImageFormatPropertiesNV* in_struct,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    format = in_struct->format;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkOpticalFlowImageFormatPropertiesNV::initialize(const safe_VkOpticalFlowImageFormatPropertiesNV* copy_src,
                                                           [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    format = copy_src->format;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkOpticalFlowSessionCreateInfoNV::safe_VkOpticalFlowSessionCreateInfoNV(const VkOpticalFlowSessionCreateInfoNV* in_struct,
                                                                             [[maybe_unused]] PNextCopyState* copy_state,
                                                                             bool copy_pnext)
    : sType(in_struct->sType),
      width(in_struct->width),
      height(in_struct->height),
      imageFormat(in_struct->imageFormat),
      flowVectorFormat(in_struct->flowVectorFormat),
      costFormat(in_struct->costFormat),
      outputGridSize(in_struct->outputGridSize),
      hintGridSize(in_struct->hintGridSize),
      performanceLevel(in_struct->performanceLevel),
      flags(in_struct->flags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkOpticalFlowSessionCreateInfoNV::safe_VkOpticalFlowSessionCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_INFO_NV),
      pNext(nullptr),
      width(),
      height(),
      imageFormat(),
      flowVectorFormat(),
      costFormat(),
      outputGridSize(),
      hintGridSize(),
      performanceLevel(),
      flags() {}

safe_VkOpticalFlowSessionCreateInfoNV::safe_VkOpticalFlowSessionCreateInfoNV(
    const safe_VkOpticalFlowSessionCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    width = copy_src.width;
    height = copy_src.height;
    imageFormat = copy_src.imageFormat;
    flowVectorFormat = copy_src.flowVectorFormat;
    costFormat = copy_src.costFormat;
    outputGridSize = copy_src.outputGridSize;
    hintGridSize = copy_src.hintGridSize;
    performanceLevel = copy_src.performanceLevel;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkOpticalFlowSessionCreateInfoNV& safe_VkOpticalFlowSessionCreateInfoNV::operator=(
    const safe_VkOpticalFlowSessionCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    width = copy_src.width;
    height = copy_src.height;
    imageFormat = copy_src.imageFormat;
    flowVectorFormat = copy_src.flowVectorFormat;
    costFormat = copy_src.costFormat;
    outputGridSize = copy_src.outputGridSize;
    hintGridSize = copy_src.hintGridSize;
    performanceLevel = copy_src.performanceLevel;
    flags = copy_src.flags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkOpticalFlowSessionCreateInfoNV::~safe_VkOpticalFlowSessionCreateInfoNV() { FreePnextChain(pNext); }

void safe_VkOpticalFlowSessionCreateInfoNV::initialize(const VkOpticalFlowSessionCreateInfoNV* in_struct,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    width = in_struct->width;
    height = in_struct->height;
    imageFormat = in_struct->imageFormat;
    flowVectorFormat = in_struct->flowVectorFormat;
    costFormat = in_struct->costFormat;
    outputGridSize = in_struct->outputGridSize;
    hintGridSize = in_struct->hintGridSize;
    performanceLevel = in_struct->performanceLevel;
    flags = in_struct->flags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkOpticalFlowSessionCreateInfoNV::initialize(const safe_VkOpticalFlowSessionCreateInfoNV* copy_src,
                                                       [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    width = copy_src->width;
    height = copy_src->height;
    imageFormat = copy_src->imageFormat;
    flowVectorFormat = copy_src->flowVectorFormat;
    costFormat = copy_src->costFormat;
    outputGridSize = copy_src->outputGridSize;
    hintGridSize = copy_src->hintGridSize;
    performanceLevel = copy_src->performanceLevel;
    flags = copy_src->flags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkOpticalFlowSessionCreatePrivateDataInfoNV::safe_VkOpticalFlowSessionCreatePrivateDataInfoNV(
    const VkOpticalFlowSessionCreatePrivateDataInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), id(in_struct->id), size(in_struct->size), pPrivateData(in_struct->pPrivateData) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkOpticalFlowSessionCreatePrivateDataInfoNV::safe_VkOpticalFlowSessionCreatePrivateDataInfoNV()
    : sType(VK_STRUCTURE_TYPE_OPTICAL_FLOW_SESSION_CREATE_PRIVATE_DATA_INFO_NV),
      pNext(nullptr),
      id(),
      size(),
      pPrivateData(nullptr) {}

safe_VkOpticalFlowSessionCreatePrivateDataInfoNV::safe_VkOpticalFlowSessionCreatePrivateDataInfoNV(
    const safe_VkOpticalFlowSessionCreatePrivateDataInfoNV& copy_src) {
    sType = copy_src.sType;
    id = copy_src.id;
    size = copy_src.size;
    pPrivateData = copy_src.pPrivateData;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkOpticalFlowSessionCreatePrivateDataInfoNV& safe_VkOpticalFlowSessionCreatePrivateDataInfoNV::operator=(
    const safe_VkOpticalFlowSessionCreatePrivateDataInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    id = copy_src.id;
    size = copy_src.size;
    pPrivateData = copy_src.pPrivateData;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkOpticalFlowSessionCreatePrivateDataInfoNV::~safe_VkOpticalFlowSessionCreatePrivateDataInfoNV() { FreePnextChain(pNext); }

void safe_VkOpticalFlowSessionCreatePrivateDataInfoNV::initialize(const VkOpticalFlowSessionCreatePrivateDataInfoNV* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    id = in_struct->id;
    size = in_struct->size;
    pPrivateData = in_struct->pPrivateData;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkOpticalFlowSessionCreatePrivateDataInfoNV::initialize(const safe_VkOpticalFlowSessionCreatePrivateDataInfoNV* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    id = copy_src->id;
    size = copy_src->size;
    pPrivateData = copy_src->pPrivateData;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkOpticalFlowExecuteInfoNV::safe_VkOpticalFlowExecuteInfoNV(const VkOpticalFlowExecuteInfoNV* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), flags(in_struct->flags), regionCount(in_struct->regionCount), pRegions(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pRegions) {
        pRegions = new VkRect2D[in_struct->regionCount];
        memcpy((void*)pRegions, (void*)in_struct->pRegions, sizeof(VkRect2D) * in_struct->regionCount);
    }
}

safe_VkOpticalFlowExecuteInfoNV::safe_VkOpticalFlowExecuteInfoNV()
    : sType(VK_STRUCTURE_TYPE_OPTICAL_FLOW_EXECUTE_INFO_NV), pNext(nullptr), flags(), regionCount(), pRegions(nullptr) {}

safe_VkOpticalFlowExecuteInfoNV::safe_VkOpticalFlowExecuteInfoNV(const safe_VkOpticalFlowExecuteInfoNV& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    regionCount = copy_src.regionCount;
    pRegions = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pRegions) {
        pRegions = new VkRect2D[copy_src.regionCount];
        memcpy((void*)pRegions, (void*)copy_src.pRegions, sizeof(VkRect2D) * copy_src.regionCount);
    }
}

safe_VkOpticalFlowExecuteInfoNV& safe_VkOpticalFlowExecuteInfoNV::operator=(const safe_VkOpticalFlowExecuteInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pRegions) delete[] pRegions;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    regionCount = copy_src.regionCount;
    pRegions = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pRegions) {
        pRegions = new VkRect2D[copy_src.regionCount];
        memcpy((void*)pRegions, (void*)copy_src.pRegions, sizeof(VkRect2D) * copy_src.regionCount);
    }

    return *this;
}

safe_VkOpticalFlowExecuteInfoNV::~safe_VkOpticalFlowExecuteInfoNV() {
    if (pRegions) delete[] pRegions;
    FreePnextChain(pNext);
}

void safe_VkOpticalFlowExecuteInfoNV::initialize(const VkOpticalFlowExecuteInfoNV* in_struct,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    if (pRegions) delete[] pRegions;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    regionCount = in_struct->regionCount;
    pRegions = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pRegions) {
        pRegions = new VkRect2D[in_struct->regionCount];
        memcpy((void*)pRegions, (void*)in_struct->pRegions, sizeof(VkRect2D) * in_struct->regionCount);
    }
}

void safe_VkOpticalFlowExecuteInfoNV::initialize(const safe_VkOpticalFlowExecuteInfoNV* copy_src,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    regionCount = copy_src->regionCount;
    pRegions = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pRegions) {
        pRegions = new VkRect2D[copy_src->regionCount];
        memcpy((void*)pRegions, (void*)copy_src->pRegions, sizeof(VkRect2D) * copy_src->regionCount);
    }
}
#ifdef VK_USE_PLATFORM_ANDROID_KHR

safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID::safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID(
    const VkPhysicalDeviceExternalFormatResolveFeaturesANDROID* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), externalFormatResolve(in_struct->externalFormatResolve) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID::safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_FEATURES_ANDROID), pNext(nullptr), externalFormatResolve() {}

safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID::safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID(
    const safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID& copy_src) {
    sType = copy_src.sType;
    externalFormatResolve = copy_src.externalFormatResolve;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID& safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID::operator=(
    const safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    externalFormatResolve = copy_src.externalFormatResolve;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID::~safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID::initialize(
    const VkPhysicalDeviceExternalFormatResolveFeaturesANDROID* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    externalFormatResolve = in_struct->externalFormatResolve;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID::initialize(
    const safe_VkPhysicalDeviceExternalFormatResolveFeaturesANDROID* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    externalFormatResolve = copy_src->externalFormatResolve;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID::safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID(
    const VkPhysicalDeviceExternalFormatResolvePropertiesANDROID* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      nullColorAttachmentWithExternalFormatResolve(in_struct->nullColorAttachmentWithExternalFormatResolve),
      externalFormatResolveChromaOffsetX(in_struct->externalFormatResolveChromaOffsetX),
      externalFormatResolveChromaOffsetY(in_struct->externalFormatResolveChromaOffsetY) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID::safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_FORMAT_RESOLVE_PROPERTIES_ANDROID),
      pNext(nullptr),
      nullColorAttachmentWithExternalFormatResolve(),
      externalFormatResolveChromaOffsetX(),
      externalFormatResolveChromaOffsetY() {}

safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID::safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID(
    const safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID& copy_src) {
    sType = copy_src.sType;
    nullColorAttachmentWithExternalFormatResolve = copy_src.nullColorAttachmentWithExternalFormatResolve;
    externalFormatResolveChromaOffsetX = copy_src.externalFormatResolveChromaOffsetX;
    externalFormatResolveChromaOffsetY = copy_src.externalFormatResolveChromaOffsetY;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID& safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID::operator=(
    const safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    nullColorAttachmentWithExternalFormatResolve = copy_src.nullColorAttachmentWithExternalFormatResolve;
    externalFormatResolveChromaOffsetX = copy_src.externalFormatResolveChromaOffsetX;
    externalFormatResolveChromaOffsetY = copy_src.externalFormatResolveChromaOffsetY;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID::~safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID::initialize(
    const VkPhysicalDeviceExternalFormatResolvePropertiesANDROID* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    nullColorAttachmentWithExternalFormatResolve = in_struct->nullColorAttachmentWithExternalFormatResolve;
    externalFormatResolveChromaOffsetX = in_struct->externalFormatResolveChromaOffsetX;
    externalFormatResolveChromaOffsetY = in_struct->externalFormatResolveChromaOffsetY;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID::initialize(
    const safe_VkPhysicalDeviceExternalFormatResolvePropertiesANDROID* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    nullColorAttachmentWithExternalFormatResolve = copy_src->nullColorAttachmentWithExternalFormatResolve;
    externalFormatResolveChromaOffsetX = copy_src->externalFormatResolveChromaOffsetX;
    externalFormatResolveChromaOffsetY = copy_src->externalFormatResolveChromaOffsetY;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID::safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID(
    const VkAndroidHardwareBufferFormatResolvePropertiesANDROID* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), colorAttachmentFormat(in_struct->colorAttachmentFormat) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID::safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID()
    : sType(VK_STRUCTURE_TYPE_ANDROID_HARDWARE_BUFFER_FORMAT_RESOLVE_PROPERTIES_ANDROID), pNext(nullptr), colorAttachmentFormat() {}

safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID::safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID(
    const safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID& copy_src) {
    sType = copy_src.sType;
    colorAttachmentFormat = copy_src.colorAttachmentFormat;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID& safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID::operator=(
    const safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    colorAttachmentFormat = copy_src.colorAttachmentFormat;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID::~safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID() {
    FreePnextChain(pNext);
}

void safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID::initialize(
    const VkAndroidHardwareBufferFormatResolvePropertiesANDROID* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    colorAttachmentFormat = in_struct->colorAttachmentFormat;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID::initialize(
    const safe_VkAndroidHardwareBufferFormatResolvePropertiesANDROID* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    colorAttachmentFormat = copy_src->colorAttachmentFormat;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_ANDROID_KHR

safe_VkPhysicalDeviceAntiLagFeaturesAMD::safe_VkPhysicalDeviceAntiLagFeaturesAMD(
    const VkPhysicalDeviceAntiLagFeaturesAMD* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), antiLag(in_struct->antiLag) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceAntiLagFeaturesAMD::safe_VkPhysicalDeviceAntiLagFeaturesAMD()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ANTI_LAG_FEATURES_AMD), pNext(nullptr), antiLag() {}

safe_VkPhysicalDeviceAntiLagFeaturesAMD::safe_VkPhysicalDeviceAntiLagFeaturesAMD(
    const safe_VkPhysicalDeviceAntiLagFeaturesAMD& copy_src) {
    sType = copy_src.sType;
    antiLag = copy_src.antiLag;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceAntiLagFeaturesAMD& safe_VkPhysicalDeviceAntiLagFeaturesAMD::operator=(
    const safe_VkPhysicalDeviceAntiLagFeaturesAMD& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    antiLag = copy_src.antiLag;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceAntiLagFeaturesAMD::~safe_VkPhysicalDeviceAntiLagFeaturesAMD() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceAntiLagFeaturesAMD::initialize(const VkPhysicalDeviceAntiLagFeaturesAMD* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    antiLag = in_struct->antiLag;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceAntiLagFeaturesAMD::initialize(const safe_VkPhysicalDeviceAntiLagFeaturesAMD* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    antiLag = copy_src->antiLag;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAntiLagPresentationInfoAMD::safe_VkAntiLagPresentationInfoAMD(const VkAntiLagPresentationInfoAMD* in_struct,
                                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), stage(in_struct->stage), frameIndex(in_struct->frameIndex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAntiLagPresentationInfoAMD::safe_VkAntiLagPresentationInfoAMD()
    : sType(VK_STRUCTURE_TYPE_ANTI_LAG_PRESENTATION_INFO_AMD), pNext(nullptr), stage(), frameIndex() {}

safe_VkAntiLagPresentationInfoAMD::safe_VkAntiLagPresentationInfoAMD(const safe_VkAntiLagPresentationInfoAMD& copy_src) {
    sType = copy_src.sType;
    stage = copy_src.stage;
    frameIndex = copy_src.frameIndex;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAntiLagPresentationInfoAMD& safe_VkAntiLagPresentationInfoAMD::operator=(const safe_VkAntiLagPresentationInfoAMD& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    stage = copy_src.stage;
    frameIndex = copy_src.frameIndex;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAntiLagPresentationInfoAMD::~safe_VkAntiLagPresentationInfoAMD() { FreePnextChain(pNext); }

void safe_VkAntiLagPresentationInfoAMD::initialize(const VkAntiLagPresentationInfoAMD* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stage = in_struct->stage;
    frameIndex = in_struct->frameIndex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAntiLagPresentationInfoAMD::initialize(const safe_VkAntiLagPresentationInfoAMD* copy_src,
                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stage = copy_src->stage;
    frameIndex = copy_src->frameIndex;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAntiLagDataAMD::safe_VkAntiLagDataAMD(const VkAntiLagDataAMD* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
                                             bool copy_pnext)
    : sType(in_struct->sType), mode(in_struct->mode), maxFPS(in_struct->maxFPS), pPresentationInfo(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pPresentationInfo) pPresentationInfo = new safe_VkAntiLagPresentationInfoAMD(in_struct->pPresentationInfo);
}

safe_VkAntiLagDataAMD::safe_VkAntiLagDataAMD()
    : sType(VK_STRUCTURE_TYPE_ANTI_LAG_DATA_AMD), pNext(nullptr), mode(), maxFPS(), pPresentationInfo(nullptr) {}

safe_VkAntiLagDataAMD::safe_VkAntiLagDataAMD(const safe_VkAntiLagDataAMD& copy_src) {
    sType = copy_src.sType;
    mode = copy_src.mode;
    maxFPS = copy_src.maxFPS;
    pPresentationInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pPresentationInfo) pPresentationInfo = new safe_VkAntiLagPresentationInfoAMD(*copy_src.pPresentationInfo);
}

safe_VkAntiLagDataAMD& safe_VkAntiLagDataAMD::operator=(const safe_VkAntiLagDataAMD& copy_src) {
    if (&copy_src == this) return *this;

    if (pPresentationInfo) delete pPresentationInfo;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    mode = copy_src.mode;
    maxFPS = copy_src.maxFPS;
    pPresentationInfo = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (copy_src.pPresentationInfo) pPresentationInfo = new safe_VkAntiLagPresentationInfoAMD(*copy_src.pPresentationInfo);

    return *this;
}

safe_VkAntiLagDataAMD::~safe_VkAntiLagDataAMD() {
    if (pPresentationInfo) delete pPresentationInfo;
    FreePnextChain(pNext);
}

void safe_VkAntiLagDataAMD::initialize(const VkAntiLagDataAMD* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pPresentationInfo) delete pPresentationInfo;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    mode = in_struct->mode;
    maxFPS = in_struct->maxFPS;
    pPresentationInfo = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (in_struct->pPresentationInfo) pPresentationInfo = new safe_VkAntiLagPresentationInfoAMD(in_struct->pPresentationInfo);
}

void safe_VkAntiLagDataAMD::initialize(const safe_VkAntiLagDataAMD* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    mode = copy_src->mode;
    maxFPS = copy_src->maxFPS;
    pPresentationInfo = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (copy_src->pPresentationInfo) pPresentationInfo = new safe_VkAntiLagPresentationInfoAMD(*copy_src->pPresentationInfo);
}

safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM::safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM(
    const VkPhysicalDeviceTilePropertiesFeaturesQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), tileProperties(in_struct->tileProperties) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM::safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TILE_PROPERTIES_FEATURES_QCOM), pNext(nullptr), tileProperties() {}

safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM::safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM(
    const safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM& copy_src) {
    sType = copy_src.sType;
    tileProperties = copy_src.tileProperties;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM& safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM::operator=(
    const safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    tileProperties = copy_src.tileProperties;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM::~safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM::initialize(const VkPhysicalDeviceTilePropertiesFeaturesQCOM* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    tileProperties = in_struct->tileProperties;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM::initialize(const safe_VkPhysicalDeviceTilePropertiesFeaturesQCOM* copy_src,
                                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    tileProperties = copy_src->tileProperties;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkTilePropertiesQCOM::safe_VkTilePropertiesQCOM(const VkTilePropertiesQCOM* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), tileSize(in_struct->tileSize), apronSize(in_struct->apronSize), origin(in_struct->origin) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkTilePropertiesQCOM::safe_VkTilePropertiesQCOM()
    : sType(VK_STRUCTURE_TYPE_TILE_PROPERTIES_QCOM), pNext(nullptr), tileSize(), apronSize(), origin() {}

safe_VkTilePropertiesQCOM::safe_VkTilePropertiesQCOM(const safe_VkTilePropertiesQCOM& copy_src) {
    sType = copy_src.sType;
    tileSize = copy_src.tileSize;
    apronSize = copy_src.apronSize;
    origin = copy_src.origin;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkTilePropertiesQCOM& safe_VkTilePropertiesQCOM::operator=(const safe_VkTilePropertiesQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    tileSize = copy_src.tileSize;
    apronSize = copy_src.apronSize;
    origin = copy_src.origin;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkTilePropertiesQCOM::~safe_VkTilePropertiesQCOM() { FreePnextChain(pNext); }

void safe_VkTilePropertiesQCOM::initialize(const VkTilePropertiesQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    tileSize = in_struct->tileSize;
    apronSize = in_struct->apronSize;
    origin = in_struct->origin;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkTilePropertiesQCOM::initialize(const safe_VkTilePropertiesQCOM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    tileSize = copy_src->tileSize;
    apronSize = copy_src->apronSize;
    origin = copy_src->origin;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC::safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC(
    const VkPhysicalDeviceAmigoProfilingFeaturesSEC* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), amigoProfiling(in_struct->amigoProfiling) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC::safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_AMIGO_PROFILING_FEATURES_SEC), pNext(nullptr), amigoProfiling() {}

safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC::safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC(
    const safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC& copy_src) {
    sType = copy_src.sType;
    amigoProfiling = copy_src.amigoProfiling;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC& safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC::operator=(
    const safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    amigoProfiling = copy_src.amigoProfiling;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC::~safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC::initialize(const VkPhysicalDeviceAmigoProfilingFeaturesSEC* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    amigoProfiling = in_struct->amigoProfiling;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC::initialize(const safe_VkPhysicalDeviceAmigoProfilingFeaturesSEC* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    amigoProfiling = copy_src->amigoProfiling;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkAmigoProfilingSubmitInfoSEC::safe_VkAmigoProfilingSubmitInfoSEC(const VkAmigoProfilingSubmitInfoSEC* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      firstDrawTimestamp(in_struct->firstDrawTimestamp),
      swapBufferTimestamp(in_struct->swapBufferTimestamp) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkAmigoProfilingSubmitInfoSEC::safe_VkAmigoProfilingSubmitInfoSEC()
    : sType(VK_STRUCTURE_TYPE_AMIGO_PROFILING_SUBMIT_INFO_SEC), pNext(nullptr), firstDrawTimestamp(), swapBufferTimestamp() {}

safe_VkAmigoProfilingSubmitInfoSEC::safe_VkAmigoProfilingSubmitInfoSEC(const safe_VkAmigoProfilingSubmitInfoSEC& copy_src) {
    sType = copy_src.sType;
    firstDrawTimestamp = copy_src.firstDrawTimestamp;
    swapBufferTimestamp = copy_src.swapBufferTimestamp;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkAmigoProfilingSubmitInfoSEC& safe_VkAmigoProfilingSubmitInfoSEC::operator=(
    const safe_VkAmigoProfilingSubmitInfoSEC& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    firstDrawTimestamp = copy_src.firstDrawTimestamp;
    swapBufferTimestamp = copy_src.swapBufferTimestamp;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkAmigoProfilingSubmitInfoSEC::~safe_VkAmigoProfilingSubmitInfoSEC() { FreePnextChain(pNext); }

void safe_VkAmigoProfilingSubmitInfoSEC::initialize(const VkAmigoProfilingSubmitInfoSEC* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    firstDrawTimestamp = in_struct->firstDrawTimestamp;
    swapBufferTimestamp = in_struct->swapBufferTimestamp;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkAmigoProfilingSubmitInfoSEC::initialize(const safe_VkAmigoProfilingSubmitInfoSEC* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    firstDrawTimestamp = copy_src->firstDrawTimestamp;
    swapBufferTimestamp = copy_src->swapBufferTimestamp;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM::safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM(
    const VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), multiviewPerViewViewports(in_struct->multiviewPerViewViewports) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM::safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_VIEWPORTS_FEATURES_QCOM),
      pNext(nullptr),
      multiviewPerViewViewports() {}

safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM::safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM(
    const safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM& copy_src) {
    sType = copy_src.sType;
    multiviewPerViewViewports = copy_src.multiviewPerViewViewports;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM& safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM::operator=(
    const safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    multiviewPerViewViewports = copy_src.multiviewPerViewViewports;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM::~safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM::initialize(
    const VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    multiviewPerViewViewports = in_struct->multiviewPerViewViewports;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM::initialize(
    const safe_VkPhysicalDeviceMultiviewPerViewViewportsFeaturesQCOM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    multiviewPerViewViewports = copy_src->multiviewPerViewViewports;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV::safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV(
    const VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), rayTracingInvocationReorderReorderingHint(in_struct->rayTracingInvocationReorderReorderingHint) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV::safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_PROPERTIES_NV),
      pNext(nullptr),
      rayTracingInvocationReorderReorderingHint() {}

safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV::safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV(
    const safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV& copy_src) {
    sType = copy_src.sType;
    rayTracingInvocationReorderReorderingHint = copy_src.rayTracingInvocationReorderReorderingHint;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV&
safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV::operator=(
    const safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    rayTracingInvocationReorderReorderingHint = copy_src.rayTracingInvocationReorderReorderingHint;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV::~safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV::initialize(
    const VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    rayTracingInvocationReorderReorderingHint = in_struct->rayTracingInvocationReorderReorderingHint;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV::initialize(
    const safe_VkPhysicalDeviceRayTracingInvocationReorderPropertiesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    rayTracingInvocationReorderReorderingHint = copy_src->rayTracingInvocationReorderReorderingHint;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV::safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV(
    const VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), rayTracingInvocationReorder(in_struct->rayTracingInvocationReorder) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV::safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_INVOCATION_REORDER_FEATURES_NV),
      pNext(nullptr),
      rayTracingInvocationReorder() {}

safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV::safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV(
    const safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV& copy_src) {
    sType = copy_src.sType;
    rayTracingInvocationReorder = copy_src.rayTracingInvocationReorder;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV& safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV::operator=(
    const safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    rayTracingInvocationReorder = copy_src.rayTracingInvocationReorder;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV::~safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV::initialize(
    const VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    rayTracingInvocationReorder = in_struct->rayTracingInvocationReorder;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV::initialize(
    const safe_VkPhysicalDeviceRayTracingInvocationReorderFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    rayTracingInvocationReorder = copy_src->rayTracingInvocationReorder;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCooperativeVectorPropertiesNV::safe_VkPhysicalDeviceCooperativeVectorPropertiesNV(
    const VkPhysicalDeviceCooperativeVectorPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      cooperativeVectorSupportedStages(in_struct->cooperativeVectorSupportedStages),
      cooperativeVectorTrainingFloat16Accumulation(in_struct->cooperativeVectorTrainingFloat16Accumulation),
      cooperativeVectorTrainingFloat32Accumulation(in_struct->cooperativeVectorTrainingFloat32Accumulation),
      maxCooperativeVectorComponents(in_struct->maxCooperativeVectorComponents) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCooperativeVectorPropertiesNV::safe_VkPhysicalDeviceCooperativeVectorPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_PROPERTIES_NV),
      pNext(nullptr),
      cooperativeVectorSupportedStages(),
      cooperativeVectorTrainingFloat16Accumulation(),
      cooperativeVectorTrainingFloat32Accumulation(),
      maxCooperativeVectorComponents() {}

safe_VkPhysicalDeviceCooperativeVectorPropertiesNV::safe_VkPhysicalDeviceCooperativeVectorPropertiesNV(
    const safe_VkPhysicalDeviceCooperativeVectorPropertiesNV& copy_src) {
    sType = copy_src.sType;
    cooperativeVectorSupportedStages = copy_src.cooperativeVectorSupportedStages;
    cooperativeVectorTrainingFloat16Accumulation = copy_src.cooperativeVectorTrainingFloat16Accumulation;
    cooperativeVectorTrainingFloat32Accumulation = copy_src.cooperativeVectorTrainingFloat32Accumulation;
    maxCooperativeVectorComponents = copy_src.maxCooperativeVectorComponents;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCooperativeVectorPropertiesNV& safe_VkPhysicalDeviceCooperativeVectorPropertiesNV::operator=(
    const safe_VkPhysicalDeviceCooperativeVectorPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    cooperativeVectorSupportedStages = copy_src.cooperativeVectorSupportedStages;
    cooperativeVectorTrainingFloat16Accumulation = copy_src.cooperativeVectorTrainingFloat16Accumulation;
    cooperativeVectorTrainingFloat32Accumulation = copy_src.cooperativeVectorTrainingFloat32Accumulation;
    maxCooperativeVectorComponents = copy_src.maxCooperativeVectorComponents;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCooperativeVectorPropertiesNV::~safe_VkPhysicalDeviceCooperativeVectorPropertiesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceCooperativeVectorPropertiesNV::initialize(const VkPhysicalDeviceCooperativeVectorPropertiesNV* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    cooperativeVectorSupportedStages = in_struct->cooperativeVectorSupportedStages;
    cooperativeVectorTrainingFloat16Accumulation = in_struct->cooperativeVectorTrainingFloat16Accumulation;
    cooperativeVectorTrainingFloat32Accumulation = in_struct->cooperativeVectorTrainingFloat32Accumulation;
    maxCooperativeVectorComponents = in_struct->maxCooperativeVectorComponents;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCooperativeVectorPropertiesNV::initialize(
    const safe_VkPhysicalDeviceCooperativeVectorPropertiesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    cooperativeVectorSupportedStages = copy_src->cooperativeVectorSupportedStages;
    cooperativeVectorTrainingFloat16Accumulation = copy_src->cooperativeVectorTrainingFloat16Accumulation;
    cooperativeVectorTrainingFloat32Accumulation = copy_src->cooperativeVectorTrainingFloat32Accumulation;
    maxCooperativeVectorComponents = copy_src->maxCooperativeVectorComponents;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCooperativeVectorFeaturesNV::safe_VkPhysicalDeviceCooperativeVectorFeaturesNV(
    const VkPhysicalDeviceCooperativeVectorFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      cooperativeVector(in_struct->cooperativeVector),
      cooperativeVectorTraining(in_struct->cooperativeVectorTraining) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCooperativeVectorFeaturesNV::safe_VkPhysicalDeviceCooperativeVectorFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_VECTOR_FEATURES_NV),
      pNext(nullptr),
      cooperativeVector(),
      cooperativeVectorTraining() {}

safe_VkPhysicalDeviceCooperativeVectorFeaturesNV::safe_VkPhysicalDeviceCooperativeVectorFeaturesNV(
    const safe_VkPhysicalDeviceCooperativeVectorFeaturesNV& copy_src) {
    sType = copy_src.sType;
    cooperativeVector = copy_src.cooperativeVector;
    cooperativeVectorTraining = copy_src.cooperativeVectorTraining;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCooperativeVectorFeaturesNV& safe_VkPhysicalDeviceCooperativeVectorFeaturesNV::operator=(
    const safe_VkPhysicalDeviceCooperativeVectorFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    cooperativeVector = copy_src.cooperativeVector;
    cooperativeVectorTraining = copy_src.cooperativeVectorTraining;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCooperativeVectorFeaturesNV::~safe_VkPhysicalDeviceCooperativeVectorFeaturesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceCooperativeVectorFeaturesNV::initialize(const VkPhysicalDeviceCooperativeVectorFeaturesNV* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    cooperativeVector = in_struct->cooperativeVector;
    cooperativeVectorTraining = in_struct->cooperativeVectorTraining;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCooperativeVectorFeaturesNV::initialize(const safe_VkPhysicalDeviceCooperativeVectorFeaturesNV* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    cooperativeVector = copy_src->cooperativeVector;
    cooperativeVectorTraining = copy_src->cooperativeVectorTraining;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCooperativeVectorPropertiesNV::safe_VkCooperativeVectorPropertiesNV(const VkCooperativeVectorPropertiesNV* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType),
      inputType(in_struct->inputType),
      inputInterpretation(in_struct->inputInterpretation),
      matrixInterpretation(in_struct->matrixInterpretation),
      biasInterpretation(in_struct->biasInterpretation),
      resultType(in_struct->resultType),
      transpose(in_struct->transpose) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCooperativeVectorPropertiesNV::safe_VkCooperativeVectorPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_COOPERATIVE_VECTOR_PROPERTIES_NV),
      pNext(nullptr),
      inputType(),
      inputInterpretation(),
      matrixInterpretation(),
      biasInterpretation(),
      resultType(),
      transpose() {}

safe_VkCooperativeVectorPropertiesNV::safe_VkCooperativeVectorPropertiesNV(const safe_VkCooperativeVectorPropertiesNV& copy_src) {
    sType = copy_src.sType;
    inputType = copy_src.inputType;
    inputInterpretation = copy_src.inputInterpretation;
    matrixInterpretation = copy_src.matrixInterpretation;
    biasInterpretation = copy_src.biasInterpretation;
    resultType = copy_src.resultType;
    transpose = copy_src.transpose;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCooperativeVectorPropertiesNV& safe_VkCooperativeVectorPropertiesNV::operator=(
    const safe_VkCooperativeVectorPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    inputType = copy_src.inputType;
    inputInterpretation = copy_src.inputInterpretation;
    matrixInterpretation = copy_src.matrixInterpretation;
    biasInterpretation = copy_src.biasInterpretation;
    resultType = copy_src.resultType;
    transpose = copy_src.transpose;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCooperativeVectorPropertiesNV::~safe_VkCooperativeVectorPropertiesNV() { FreePnextChain(pNext); }

void safe_VkCooperativeVectorPropertiesNV::initialize(const VkCooperativeVectorPropertiesNV* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    inputType = in_struct->inputType;
    inputInterpretation = in_struct->inputInterpretation;
    matrixInterpretation = in_struct->matrixInterpretation;
    biasInterpretation = in_struct->biasInterpretation;
    resultType = in_struct->resultType;
    transpose = in_struct->transpose;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCooperativeVectorPropertiesNV::initialize(const safe_VkCooperativeVectorPropertiesNV* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    inputType = copy_src->inputType;
    inputInterpretation = copy_src->inputInterpretation;
    matrixInterpretation = copy_src->matrixInterpretation;
    biasInterpretation = copy_src->biasInterpretation;
    resultType = copy_src->resultType;
    transpose = copy_src->transpose;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkConvertCooperativeVectorMatrixInfoNV::safe_VkConvertCooperativeVectorMatrixInfoNV(
    const VkConvertCooperativeVectorMatrixInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      srcSize(in_struct->srcSize),
      srcData(&in_struct->srcData),
      pDstSize(nullptr),
      dstData(&in_struct->dstData),
      srcComponentType(in_struct->srcComponentType),
      dstComponentType(in_struct->dstComponentType),
      numRows(in_struct->numRows),
      numColumns(in_struct->numColumns),
      srcLayout(in_struct->srcLayout),
      srcStride(in_struct->srcStride),
      dstLayout(in_struct->dstLayout),
      dstStride(in_struct->dstStride) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pDstSize) {
        pDstSize = new size_t(*in_struct->pDstSize);
    }
}

safe_VkConvertCooperativeVectorMatrixInfoNV::safe_VkConvertCooperativeVectorMatrixInfoNV()
    : sType(VK_STRUCTURE_TYPE_CONVERT_COOPERATIVE_VECTOR_MATRIX_INFO_NV),
      pNext(nullptr),
      srcSize(),
      pDstSize(nullptr),
      srcComponentType(),
      dstComponentType(),
      numRows(),
      numColumns(),
      srcLayout(),
      srcStride(),
      dstLayout(),
      dstStride() {}

safe_VkConvertCooperativeVectorMatrixInfoNV::safe_VkConvertCooperativeVectorMatrixInfoNV(
    const safe_VkConvertCooperativeVectorMatrixInfoNV& copy_src) {
    sType = copy_src.sType;
    srcSize = copy_src.srcSize;
    srcData.initialize(&copy_src.srcData);
    pDstSize = nullptr;
    dstData.initialize(&copy_src.dstData);
    srcComponentType = copy_src.srcComponentType;
    dstComponentType = copy_src.dstComponentType;
    numRows = copy_src.numRows;
    numColumns = copy_src.numColumns;
    srcLayout = copy_src.srcLayout;
    srcStride = copy_src.srcStride;
    dstLayout = copy_src.dstLayout;
    dstStride = copy_src.dstStride;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDstSize) {
        pDstSize = new size_t(*copy_src.pDstSize);
    }
}

safe_VkConvertCooperativeVectorMatrixInfoNV& safe_VkConvertCooperativeVectorMatrixInfoNV::operator=(
    const safe_VkConvertCooperativeVectorMatrixInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pDstSize) delete pDstSize;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    srcSize = copy_src.srcSize;
    srcData.initialize(&copy_src.srcData);
    pDstSize = nullptr;
    dstData.initialize(&copy_src.dstData);
    srcComponentType = copy_src.srcComponentType;
    dstComponentType = copy_src.dstComponentType;
    numRows = copy_src.numRows;
    numColumns = copy_src.numColumns;
    srcLayout = copy_src.srcLayout;
    srcStride = copy_src.srcStride;
    dstLayout = copy_src.dstLayout;
    dstStride = copy_src.dstStride;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pDstSize) {
        pDstSize = new size_t(*copy_src.pDstSize);
    }

    return *this;
}

safe_VkConvertCooperativeVectorMatrixInfoNV::~safe_VkConvertCooperativeVectorMatrixInfoNV() {
    if (pDstSize) delete pDstSize;
    FreePnextChain(pNext);
}

void safe_VkConvertCooperativeVectorMatrixInfoNV::initialize(const VkConvertCooperativeVectorMatrixInfoNV* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    if (pDstSize) delete pDstSize;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    srcSize = in_struct->srcSize;
    srcData.initialize(&in_struct->srcData);
    pDstSize = nullptr;
    dstData.initialize(&in_struct->dstData);
    srcComponentType = in_struct->srcComponentType;
    dstComponentType = in_struct->dstComponentType;
    numRows = in_struct->numRows;
    numColumns = in_struct->numColumns;
    srcLayout = in_struct->srcLayout;
    srcStride = in_struct->srcStride;
    dstLayout = in_struct->dstLayout;
    dstStride = in_struct->dstStride;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pDstSize) {
        pDstSize = new size_t(*in_struct->pDstSize);
    }
}

void safe_VkConvertCooperativeVectorMatrixInfoNV::initialize(const safe_VkConvertCooperativeVectorMatrixInfoNV* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    srcSize = copy_src->srcSize;
    srcData.initialize(&copy_src->srcData);
    pDstSize = nullptr;
    dstData.initialize(&copy_src->dstData);
    srcComponentType = copy_src->srcComponentType;
    dstComponentType = copy_src->dstComponentType;
    numRows = copy_src->numRows;
    numColumns = copy_src->numColumns;
    srcLayout = copy_src->srcLayout;
    srcStride = copy_src->srcStride;
    dstLayout = copy_src->dstLayout;
    dstStride = copy_src->dstStride;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pDstSize) {
        pDstSize = new size_t(*copy_src->pDstSize);
    }
}

safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV::safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV(
    const VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), extendedSparseAddressSpace(in_struct->extendedSparseAddressSpace) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV::safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_FEATURES_NV),
      pNext(nullptr),
      extendedSparseAddressSpace() {}

safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV::safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV(
    const safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV& copy_src) {
    sType = copy_src.sType;
    extendedSparseAddressSpace = copy_src.extendedSparseAddressSpace;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV& safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV::operator=(
    const safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    extendedSparseAddressSpace = copy_src.extendedSparseAddressSpace;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV::~safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV::initialize(
    const VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    extendedSparseAddressSpace = in_struct->extendedSparseAddressSpace;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV::initialize(
    const safe_VkPhysicalDeviceExtendedSparseAddressSpaceFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    extendedSparseAddressSpace = copy_src->extendedSparseAddressSpace;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV::safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV(
    const VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      extendedSparseAddressSpaceSize(in_struct->extendedSparseAddressSpaceSize),
      extendedSparseImageUsageFlags(in_struct->extendedSparseImageUsageFlags),
      extendedSparseBufferUsageFlags(in_struct->extendedSparseBufferUsageFlags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV::safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_SPARSE_ADDRESS_SPACE_PROPERTIES_NV),
      pNext(nullptr),
      extendedSparseAddressSpaceSize(),
      extendedSparseImageUsageFlags(),
      extendedSparseBufferUsageFlags() {}

safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV::safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV(
    const safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV& copy_src) {
    sType = copy_src.sType;
    extendedSparseAddressSpaceSize = copy_src.extendedSparseAddressSpaceSize;
    extendedSparseImageUsageFlags = copy_src.extendedSparseImageUsageFlags;
    extendedSparseBufferUsageFlags = copy_src.extendedSparseBufferUsageFlags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV& safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV::operator=(
    const safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    extendedSparseAddressSpaceSize = copy_src.extendedSparseAddressSpaceSize;
    extendedSparseImageUsageFlags = copy_src.extendedSparseImageUsageFlags;
    extendedSparseBufferUsageFlags = copy_src.extendedSparseBufferUsageFlags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV::~safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV::initialize(
    const VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    extendedSparseAddressSpaceSize = in_struct->extendedSparseAddressSpaceSize;
    extendedSparseImageUsageFlags = in_struct->extendedSparseImageUsageFlags;
    extendedSparseBufferUsageFlags = in_struct->extendedSparseBufferUsageFlags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV::initialize(
    const safe_VkPhysicalDeviceExtendedSparseAddressSpacePropertiesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    extendedSparseAddressSpaceSize = copy_src->extendedSparseAddressSpaceSize;
    extendedSparseImageUsageFlags = copy_src->extendedSparseImageUsageFlags;
    extendedSparseBufferUsageFlags = copy_src->extendedSparseBufferUsageFlags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM::safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM(
    const VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), shaderCoreBuiltins(in_struct->shaderCoreBuiltins) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM::safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_FEATURES_ARM), pNext(nullptr), shaderCoreBuiltins() {}

safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM::safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM(
    const safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM& copy_src) {
    sType = copy_src.sType;
    shaderCoreBuiltins = copy_src.shaderCoreBuiltins;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM& safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM::operator=(
    const safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderCoreBuiltins = copy_src.shaderCoreBuiltins;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM::~safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM::initialize(const VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderCoreBuiltins = in_struct->shaderCoreBuiltins;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM::initialize(
    const safe_VkPhysicalDeviceShaderCoreBuiltinsFeaturesARM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderCoreBuiltins = copy_src->shaderCoreBuiltins;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM::safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM(
    const VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      shaderCoreMask(in_struct->shaderCoreMask),
      shaderCoreCount(in_struct->shaderCoreCount),
      shaderWarpsPerCore(in_struct->shaderWarpsPerCore) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM::safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_CORE_BUILTINS_PROPERTIES_ARM),
      pNext(nullptr),
      shaderCoreMask(),
      shaderCoreCount(),
      shaderWarpsPerCore() {}

safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM::safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM(
    const safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM& copy_src) {
    sType = copy_src.sType;
    shaderCoreMask = copy_src.shaderCoreMask;
    shaderCoreCount = copy_src.shaderCoreCount;
    shaderWarpsPerCore = copy_src.shaderWarpsPerCore;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM& safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM::operator=(
    const safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderCoreMask = copy_src.shaderCoreMask;
    shaderCoreCount = copy_src.shaderCoreCount;
    shaderWarpsPerCore = copy_src.shaderWarpsPerCore;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM::~safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM::initialize(
    const VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderCoreMask = in_struct->shaderCoreMask;
    shaderCoreCount = in_struct->shaderCoreCount;
    shaderWarpsPerCore = in_struct->shaderWarpsPerCore;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM::initialize(
    const safe_VkPhysicalDeviceShaderCoreBuiltinsPropertiesARM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderCoreMask = copy_src->shaderCoreMask;
    shaderCoreCount = copy_src->shaderCoreCount;
    shaderWarpsPerCore = copy_src->shaderWarpsPerCore;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkLatencySleepModeInfoNV::safe_VkLatencySleepModeInfoNV(const VkLatencySleepModeInfoNV* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      lowLatencyMode(in_struct->lowLatencyMode),
      lowLatencyBoost(in_struct->lowLatencyBoost),
      minimumIntervalUs(in_struct->minimumIntervalUs) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkLatencySleepModeInfoNV::safe_VkLatencySleepModeInfoNV()
    : sType(VK_STRUCTURE_TYPE_LATENCY_SLEEP_MODE_INFO_NV),
      pNext(nullptr),
      lowLatencyMode(),
      lowLatencyBoost(),
      minimumIntervalUs() {}

safe_VkLatencySleepModeInfoNV::safe_VkLatencySleepModeInfoNV(const safe_VkLatencySleepModeInfoNV& copy_src) {
    sType = copy_src.sType;
    lowLatencyMode = copy_src.lowLatencyMode;
    lowLatencyBoost = copy_src.lowLatencyBoost;
    minimumIntervalUs = copy_src.minimumIntervalUs;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkLatencySleepModeInfoNV& safe_VkLatencySleepModeInfoNV::operator=(const safe_VkLatencySleepModeInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    lowLatencyMode = copy_src.lowLatencyMode;
    lowLatencyBoost = copy_src.lowLatencyBoost;
    minimumIntervalUs = copy_src.minimumIntervalUs;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkLatencySleepModeInfoNV::~safe_VkLatencySleepModeInfoNV() { FreePnextChain(pNext); }

void safe_VkLatencySleepModeInfoNV::initialize(const VkLatencySleepModeInfoNV* in_struct,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    lowLatencyMode = in_struct->lowLatencyMode;
    lowLatencyBoost = in_struct->lowLatencyBoost;
    minimumIntervalUs = in_struct->minimumIntervalUs;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkLatencySleepModeInfoNV::initialize(const safe_VkLatencySleepModeInfoNV* copy_src,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    lowLatencyMode = copy_src->lowLatencyMode;
    lowLatencyBoost = copy_src->lowLatencyBoost;
    minimumIntervalUs = copy_src->minimumIntervalUs;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkLatencySleepInfoNV::safe_VkLatencySleepInfoNV(const VkLatencySleepInfoNV* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), signalSemaphore(in_struct->signalSemaphore), value(in_struct->value) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkLatencySleepInfoNV::safe_VkLatencySleepInfoNV()
    : sType(VK_STRUCTURE_TYPE_LATENCY_SLEEP_INFO_NV), pNext(nullptr), signalSemaphore(), value() {}

safe_VkLatencySleepInfoNV::safe_VkLatencySleepInfoNV(const safe_VkLatencySleepInfoNV& copy_src) {
    sType = copy_src.sType;
    signalSemaphore = copy_src.signalSemaphore;
    value = copy_src.value;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkLatencySleepInfoNV& safe_VkLatencySleepInfoNV::operator=(const safe_VkLatencySleepInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    signalSemaphore = copy_src.signalSemaphore;
    value = copy_src.value;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkLatencySleepInfoNV::~safe_VkLatencySleepInfoNV() { FreePnextChain(pNext); }

void safe_VkLatencySleepInfoNV::initialize(const VkLatencySleepInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    signalSemaphore = in_struct->signalSemaphore;
    value = in_struct->value;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkLatencySleepInfoNV::initialize(const safe_VkLatencySleepInfoNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    signalSemaphore = copy_src->signalSemaphore;
    value = copy_src->value;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSetLatencyMarkerInfoNV::safe_VkSetLatencyMarkerInfoNV(const VkSetLatencyMarkerInfoNV* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), presentID(in_struct->presentID), marker(in_struct->marker) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSetLatencyMarkerInfoNV::safe_VkSetLatencyMarkerInfoNV()
    : sType(VK_STRUCTURE_TYPE_SET_LATENCY_MARKER_INFO_NV), pNext(nullptr), presentID(), marker() {}

safe_VkSetLatencyMarkerInfoNV::safe_VkSetLatencyMarkerInfoNV(const safe_VkSetLatencyMarkerInfoNV& copy_src) {
    sType = copy_src.sType;
    presentID = copy_src.presentID;
    marker = copy_src.marker;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSetLatencyMarkerInfoNV& safe_VkSetLatencyMarkerInfoNV::operator=(const safe_VkSetLatencyMarkerInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentID = copy_src.presentID;
    marker = copy_src.marker;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSetLatencyMarkerInfoNV::~safe_VkSetLatencyMarkerInfoNV() { FreePnextChain(pNext); }

void safe_VkSetLatencyMarkerInfoNV::initialize(const VkSetLatencyMarkerInfoNV* in_struct,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentID = in_struct->presentID;
    marker = in_struct->marker;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSetLatencyMarkerInfoNV::initialize(const safe_VkSetLatencyMarkerInfoNV* copy_src,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentID = copy_src->presentID;
    marker = copy_src->marker;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkLatencyTimingsFrameReportNV::safe_VkLatencyTimingsFrameReportNV(const VkLatencyTimingsFrameReportNV* in_struct,
                                                                       [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      presentID(in_struct->presentID),
      inputSampleTimeUs(in_struct->inputSampleTimeUs),
      simStartTimeUs(in_struct->simStartTimeUs),
      simEndTimeUs(in_struct->simEndTimeUs),
      renderSubmitStartTimeUs(in_struct->renderSubmitStartTimeUs),
      renderSubmitEndTimeUs(in_struct->renderSubmitEndTimeUs),
      presentStartTimeUs(in_struct->presentStartTimeUs),
      presentEndTimeUs(in_struct->presentEndTimeUs),
      driverStartTimeUs(in_struct->driverStartTimeUs),
      driverEndTimeUs(in_struct->driverEndTimeUs),
      osRenderQueueStartTimeUs(in_struct->osRenderQueueStartTimeUs),
      osRenderQueueEndTimeUs(in_struct->osRenderQueueEndTimeUs),
      gpuRenderStartTimeUs(in_struct->gpuRenderStartTimeUs),
      gpuRenderEndTimeUs(in_struct->gpuRenderEndTimeUs) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkLatencyTimingsFrameReportNV::safe_VkLatencyTimingsFrameReportNV()
    : sType(VK_STRUCTURE_TYPE_LATENCY_TIMINGS_FRAME_REPORT_NV),
      pNext(nullptr),
      presentID(),
      inputSampleTimeUs(),
      simStartTimeUs(),
      simEndTimeUs(),
      renderSubmitStartTimeUs(),
      renderSubmitEndTimeUs(),
      presentStartTimeUs(),
      presentEndTimeUs(),
      driverStartTimeUs(),
      driverEndTimeUs(),
      osRenderQueueStartTimeUs(),
      osRenderQueueEndTimeUs(),
      gpuRenderStartTimeUs(),
      gpuRenderEndTimeUs() {}

safe_VkLatencyTimingsFrameReportNV::safe_VkLatencyTimingsFrameReportNV(const safe_VkLatencyTimingsFrameReportNV& copy_src) {
    sType = copy_src.sType;
    presentID = copy_src.presentID;
    inputSampleTimeUs = copy_src.inputSampleTimeUs;
    simStartTimeUs = copy_src.simStartTimeUs;
    simEndTimeUs = copy_src.simEndTimeUs;
    renderSubmitStartTimeUs = copy_src.renderSubmitStartTimeUs;
    renderSubmitEndTimeUs = copy_src.renderSubmitEndTimeUs;
    presentStartTimeUs = copy_src.presentStartTimeUs;
    presentEndTimeUs = copy_src.presentEndTimeUs;
    driverStartTimeUs = copy_src.driverStartTimeUs;
    driverEndTimeUs = copy_src.driverEndTimeUs;
    osRenderQueueStartTimeUs = copy_src.osRenderQueueStartTimeUs;
    osRenderQueueEndTimeUs = copy_src.osRenderQueueEndTimeUs;
    gpuRenderStartTimeUs = copy_src.gpuRenderStartTimeUs;
    gpuRenderEndTimeUs = copy_src.gpuRenderEndTimeUs;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkLatencyTimingsFrameReportNV& safe_VkLatencyTimingsFrameReportNV::operator=(
    const safe_VkLatencyTimingsFrameReportNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentID = copy_src.presentID;
    inputSampleTimeUs = copy_src.inputSampleTimeUs;
    simStartTimeUs = copy_src.simStartTimeUs;
    simEndTimeUs = copy_src.simEndTimeUs;
    renderSubmitStartTimeUs = copy_src.renderSubmitStartTimeUs;
    renderSubmitEndTimeUs = copy_src.renderSubmitEndTimeUs;
    presentStartTimeUs = copy_src.presentStartTimeUs;
    presentEndTimeUs = copy_src.presentEndTimeUs;
    driverStartTimeUs = copy_src.driverStartTimeUs;
    driverEndTimeUs = copy_src.driverEndTimeUs;
    osRenderQueueStartTimeUs = copy_src.osRenderQueueStartTimeUs;
    osRenderQueueEndTimeUs = copy_src.osRenderQueueEndTimeUs;
    gpuRenderStartTimeUs = copy_src.gpuRenderStartTimeUs;
    gpuRenderEndTimeUs = copy_src.gpuRenderEndTimeUs;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkLatencyTimingsFrameReportNV::~safe_VkLatencyTimingsFrameReportNV() { FreePnextChain(pNext); }

void safe_VkLatencyTimingsFrameReportNV::initialize(const VkLatencyTimingsFrameReportNV* in_struct,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentID = in_struct->presentID;
    inputSampleTimeUs = in_struct->inputSampleTimeUs;
    simStartTimeUs = in_struct->simStartTimeUs;
    simEndTimeUs = in_struct->simEndTimeUs;
    renderSubmitStartTimeUs = in_struct->renderSubmitStartTimeUs;
    renderSubmitEndTimeUs = in_struct->renderSubmitEndTimeUs;
    presentStartTimeUs = in_struct->presentStartTimeUs;
    presentEndTimeUs = in_struct->presentEndTimeUs;
    driverStartTimeUs = in_struct->driverStartTimeUs;
    driverEndTimeUs = in_struct->driverEndTimeUs;
    osRenderQueueStartTimeUs = in_struct->osRenderQueueStartTimeUs;
    osRenderQueueEndTimeUs = in_struct->osRenderQueueEndTimeUs;
    gpuRenderStartTimeUs = in_struct->gpuRenderStartTimeUs;
    gpuRenderEndTimeUs = in_struct->gpuRenderEndTimeUs;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkLatencyTimingsFrameReportNV::initialize(const safe_VkLatencyTimingsFrameReportNV* copy_src,
                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentID = copy_src->presentID;
    inputSampleTimeUs = copy_src->inputSampleTimeUs;
    simStartTimeUs = copy_src->simStartTimeUs;
    simEndTimeUs = copy_src->simEndTimeUs;
    renderSubmitStartTimeUs = copy_src->renderSubmitStartTimeUs;
    renderSubmitEndTimeUs = copy_src->renderSubmitEndTimeUs;
    presentStartTimeUs = copy_src->presentStartTimeUs;
    presentEndTimeUs = copy_src->presentEndTimeUs;
    driverStartTimeUs = copy_src->driverStartTimeUs;
    driverEndTimeUs = copy_src->driverEndTimeUs;
    osRenderQueueStartTimeUs = copy_src->osRenderQueueStartTimeUs;
    osRenderQueueEndTimeUs = copy_src->osRenderQueueEndTimeUs;
    gpuRenderStartTimeUs = copy_src->gpuRenderStartTimeUs;
    gpuRenderEndTimeUs = copy_src->gpuRenderEndTimeUs;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkGetLatencyMarkerInfoNV::safe_VkGetLatencyMarkerInfoNV(const VkGetLatencyMarkerInfoNV* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), timingCount(in_struct->timingCount), pTimings(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (timingCount && in_struct->pTimings) {
        pTimings = new safe_VkLatencyTimingsFrameReportNV[timingCount];
        for (uint32_t i = 0; i < timingCount; ++i) {
            pTimings[i].initialize(&in_struct->pTimings[i]);
        }
    }
}

safe_VkGetLatencyMarkerInfoNV::safe_VkGetLatencyMarkerInfoNV()
    : sType(VK_STRUCTURE_TYPE_GET_LATENCY_MARKER_INFO_NV), pNext(nullptr), timingCount(), pTimings(nullptr) {}

safe_VkGetLatencyMarkerInfoNV::safe_VkGetLatencyMarkerInfoNV(const safe_VkGetLatencyMarkerInfoNV& copy_src) {
    sType = copy_src.sType;
    timingCount = copy_src.timingCount;
    pTimings = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (timingCount && copy_src.pTimings) {
        pTimings = new safe_VkLatencyTimingsFrameReportNV[timingCount];
        for (uint32_t i = 0; i < timingCount; ++i) {
            pTimings[i].initialize(&copy_src.pTimings[i]);
        }
    }
}

safe_VkGetLatencyMarkerInfoNV& safe_VkGetLatencyMarkerInfoNV::operator=(const safe_VkGetLatencyMarkerInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pTimings) delete[] pTimings;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    timingCount = copy_src.timingCount;
    pTimings = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);
    if (timingCount && copy_src.pTimings) {
        pTimings = new safe_VkLatencyTimingsFrameReportNV[timingCount];
        for (uint32_t i = 0; i < timingCount; ++i) {
            pTimings[i].initialize(&copy_src.pTimings[i]);
        }
    }

    return *this;
}

safe_VkGetLatencyMarkerInfoNV::~safe_VkGetLatencyMarkerInfoNV() {
    if (pTimings) delete[] pTimings;
    FreePnextChain(pNext);
}

void safe_VkGetLatencyMarkerInfoNV::initialize(const VkGetLatencyMarkerInfoNV* in_struct,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    if (pTimings) delete[] pTimings;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    timingCount = in_struct->timingCount;
    pTimings = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    if (timingCount && in_struct->pTimings) {
        pTimings = new safe_VkLatencyTimingsFrameReportNV[timingCount];
        for (uint32_t i = 0; i < timingCount; ++i) {
            pTimings[i].initialize(&in_struct->pTimings[i]);
        }
    }
}

void safe_VkGetLatencyMarkerInfoNV::initialize(const safe_VkGetLatencyMarkerInfoNV* copy_src,
                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    timingCount = copy_src->timingCount;
    pTimings = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);
    if (timingCount && copy_src->pTimings) {
        pTimings = new safe_VkLatencyTimingsFrameReportNV[timingCount];
        for (uint32_t i = 0; i < timingCount; ++i) {
            pTimings[i].initialize(&copy_src->pTimings[i]);
        }
    }
}

safe_VkLatencySubmissionPresentIdNV::safe_VkLatencySubmissionPresentIdNV(const VkLatencySubmissionPresentIdNV* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), presentID(in_struct->presentID) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkLatencySubmissionPresentIdNV::safe_VkLatencySubmissionPresentIdNV()
    : sType(VK_STRUCTURE_TYPE_LATENCY_SUBMISSION_PRESENT_ID_NV), pNext(nullptr), presentID() {}

safe_VkLatencySubmissionPresentIdNV::safe_VkLatencySubmissionPresentIdNV(const safe_VkLatencySubmissionPresentIdNV& copy_src) {
    sType = copy_src.sType;
    presentID = copy_src.presentID;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkLatencySubmissionPresentIdNV& safe_VkLatencySubmissionPresentIdNV::operator=(
    const safe_VkLatencySubmissionPresentIdNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentID = copy_src.presentID;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkLatencySubmissionPresentIdNV::~safe_VkLatencySubmissionPresentIdNV() { FreePnextChain(pNext); }

void safe_VkLatencySubmissionPresentIdNV::initialize(const VkLatencySubmissionPresentIdNV* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentID = in_struct->presentID;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkLatencySubmissionPresentIdNV::initialize(const safe_VkLatencySubmissionPresentIdNV* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentID = copy_src->presentID;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSwapchainLatencyCreateInfoNV::safe_VkSwapchainLatencyCreateInfoNV(const VkSwapchainLatencyCreateInfoNV* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), latencyModeEnable(in_struct->latencyModeEnable) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSwapchainLatencyCreateInfoNV::safe_VkSwapchainLatencyCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_SWAPCHAIN_LATENCY_CREATE_INFO_NV), pNext(nullptr), latencyModeEnable() {}

safe_VkSwapchainLatencyCreateInfoNV::safe_VkSwapchainLatencyCreateInfoNV(const safe_VkSwapchainLatencyCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    latencyModeEnable = copy_src.latencyModeEnable;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSwapchainLatencyCreateInfoNV& safe_VkSwapchainLatencyCreateInfoNV::operator=(
    const safe_VkSwapchainLatencyCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    latencyModeEnable = copy_src.latencyModeEnable;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSwapchainLatencyCreateInfoNV::~safe_VkSwapchainLatencyCreateInfoNV() { FreePnextChain(pNext); }

void safe_VkSwapchainLatencyCreateInfoNV::initialize(const VkSwapchainLatencyCreateInfoNV* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    latencyModeEnable = in_struct->latencyModeEnable;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSwapchainLatencyCreateInfoNV::initialize(const safe_VkSwapchainLatencyCreateInfoNV* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    latencyModeEnable = copy_src->latencyModeEnable;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkOutOfBandQueueTypeInfoNV::safe_VkOutOfBandQueueTypeInfoNV(const VkOutOfBandQueueTypeInfoNV* in_struct,
                                                                 [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), queueType(in_struct->queueType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkOutOfBandQueueTypeInfoNV::safe_VkOutOfBandQueueTypeInfoNV()
    : sType(VK_STRUCTURE_TYPE_OUT_OF_BAND_QUEUE_TYPE_INFO_NV), pNext(nullptr), queueType() {}

safe_VkOutOfBandQueueTypeInfoNV::safe_VkOutOfBandQueueTypeInfoNV(const safe_VkOutOfBandQueueTypeInfoNV& copy_src) {
    sType = copy_src.sType;
    queueType = copy_src.queueType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkOutOfBandQueueTypeInfoNV& safe_VkOutOfBandQueueTypeInfoNV::operator=(const safe_VkOutOfBandQueueTypeInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    queueType = copy_src.queueType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkOutOfBandQueueTypeInfoNV::~safe_VkOutOfBandQueueTypeInfoNV() { FreePnextChain(pNext); }

void safe_VkOutOfBandQueueTypeInfoNV::initialize(const VkOutOfBandQueueTypeInfoNV* in_struct,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    queueType = in_struct->queueType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkOutOfBandQueueTypeInfoNV::initialize(const safe_VkOutOfBandQueueTypeInfoNV* copy_src,
                                                 [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    queueType = copy_src->queueType;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkLatencySurfaceCapabilitiesNV::safe_VkLatencySurfaceCapabilitiesNV(const VkLatencySurfaceCapabilitiesNV* in_struct,
                                                                         [[maybe_unused]] PNextCopyState* copy_state,
                                                                         bool copy_pnext)
    : sType(in_struct->sType), presentModeCount(in_struct->presentModeCount), pPresentModes(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pPresentModes) {
        pPresentModes = new VkPresentModeKHR[in_struct->presentModeCount];
        memcpy((void*)pPresentModes, (void*)in_struct->pPresentModes, sizeof(VkPresentModeKHR) * in_struct->presentModeCount);
    }
}

safe_VkLatencySurfaceCapabilitiesNV::safe_VkLatencySurfaceCapabilitiesNV()
    : sType(VK_STRUCTURE_TYPE_LATENCY_SURFACE_CAPABILITIES_NV), pNext(nullptr), presentModeCount(), pPresentModes(nullptr) {}

safe_VkLatencySurfaceCapabilitiesNV::safe_VkLatencySurfaceCapabilitiesNV(const safe_VkLatencySurfaceCapabilitiesNV& copy_src) {
    sType = copy_src.sType;
    presentModeCount = copy_src.presentModeCount;
    pPresentModes = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pPresentModes) {
        pPresentModes = new VkPresentModeKHR[copy_src.presentModeCount];
        memcpy((void*)pPresentModes, (void*)copy_src.pPresentModes, sizeof(VkPresentModeKHR) * copy_src.presentModeCount);
    }
}

safe_VkLatencySurfaceCapabilitiesNV& safe_VkLatencySurfaceCapabilitiesNV::operator=(
    const safe_VkLatencySurfaceCapabilitiesNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pPresentModes) delete[] pPresentModes;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    presentModeCount = copy_src.presentModeCount;
    pPresentModes = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pPresentModes) {
        pPresentModes = new VkPresentModeKHR[copy_src.presentModeCount];
        memcpy((void*)pPresentModes, (void*)copy_src.pPresentModes, sizeof(VkPresentModeKHR) * copy_src.presentModeCount);
    }

    return *this;
}

safe_VkLatencySurfaceCapabilitiesNV::~safe_VkLatencySurfaceCapabilitiesNV() {
    if (pPresentModes) delete[] pPresentModes;
    FreePnextChain(pNext);
}

void safe_VkLatencySurfaceCapabilitiesNV::initialize(const VkLatencySurfaceCapabilitiesNV* in_struct,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    if (pPresentModes) delete[] pPresentModes;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    presentModeCount = in_struct->presentModeCount;
    pPresentModes = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pPresentModes) {
        pPresentModes = new VkPresentModeKHR[in_struct->presentModeCount];
        memcpy((void*)pPresentModes, (void*)in_struct->pPresentModes, sizeof(VkPresentModeKHR) * in_struct->presentModeCount);
    }
}

void safe_VkLatencySurfaceCapabilitiesNV::initialize(const safe_VkLatencySurfaceCapabilitiesNV* copy_src,
                                                     [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    presentModeCount = copy_src->presentModeCount;
    pPresentModes = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pPresentModes) {
        pPresentModes = new VkPresentModeKHR[copy_src->presentModeCount];
        memcpy((void*)pPresentModes, (void*)copy_src->pPresentModes, sizeof(VkPresentModeKHR) * copy_src->presentModeCount);
    }
}

safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM::safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM(
    const VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), multiviewPerViewRenderAreas(in_struct->multiviewPerViewRenderAreas) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM::safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MULTIVIEW_PER_VIEW_RENDER_AREAS_FEATURES_QCOM),
      pNext(nullptr),
      multiviewPerViewRenderAreas() {}

safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM::safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM(
    const safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM& copy_src) {
    sType = copy_src.sType;
    multiviewPerViewRenderAreas = copy_src.multiviewPerViewRenderAreas;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM&
safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM::operator=(
    const safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    multiviewPerViewRenderAreas = copy_src.multiviewPerViewRenderAreas;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM::~safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM::initialize(
    const VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    multiviewPerViewRenderAreas = in_struct->multiviewPerViewRenderAreas;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM::initialize(
    const safe_VkPhysicalDeviceMultiviewPerViewRenderAreasFeaturesQCOM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    multiviewPerViewRenderAreas = copy_src->multiviewPerViewRenderAreas;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM::safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM(
    const VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), perViewRenderAreaCount(in_struct->perViewRenderAreaCount), pPerViewRenderAreas(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pPerViewRenderAreas) {
        pPerViewRenderAreas = new VkRect2D[in_struct->perViewRenderAreaCount];
        memcpy((void*)pPerViewRenderAreas, (void*)in_struct->pPerViewRenderAreas,
               sizeof(VkRect2D) * in_struct->perViewRenderAreaCount);
    }
}

safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM::safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM()
    : sType(VK_STRUCTURE_TYPE_MULTIVIEW_PER_VIEW_RENDER_AREAS_RENDER_PASS_BEGIN_INFO_QCOM),
      pNext(nullptr),
      perViewRenderAreaCount(),
      pPerViewRenderAreas(nullptr) {}

safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM::safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM(
    const safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM& copy_src) {
    sType = copy_src.sType;
    perViewRenderAreaCount = copy_src.perViewRenderAreaCount;
    pPerViewRenderAreas = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pPerViewRenderAreas) {
        pPerViewRenderAreas = new VkRect2D[copy_src.perViewRenderAreaCount];
        memcpy((void*)pPerViewRenderAreas, (void*)copy_src.pPerViewRenderAreas, sizeof(VkRect2D) * copy_src.perViewRenderAreaCount);
    }
}

safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM& safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM::operator=(
    const safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM& copy_src) {
    if (&copy_src == this) return *this;

    if (pPerViewRenderAreas) delete[] pPerViewRenderAreas;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    perViewRenderAreaCount = copy_src.perViewRenderAreaCount;
    pPerViewRenderAreas = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pPerViewRenderAreas) {
        pPerViewRenderAreas = new VkRect2D[copy_src.perViewRenderAreaCount];
        memcpy((void*)pPerViewRenderAreas, (void*)copy_src.pPerViewRenderAreas, sizeof(VkRect2D) * copy_src.perViewRenderAreaCount);
    }

    return *this;
}

safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM::~safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM() {
    if (pPerViewRenderAreas) delete[] pPerViewRenderAreas;
    FreePnextChain(pNext);
}

void safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM::initialize(
    const VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pPerViewRenderAreas) delete[] pPerViewRenderAreas;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    perViewRenderAreaCount = in_struct->perViewRenderAreaCount;
    pPerViewRenderAreas = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pPerViewRenderAreas) {
        pPerViewRenderAreas = new VkRect2D[in_struct->perViewRenderAreaCount];
        memcpy((void*)pPerViewRenderAreas, (void*)in_struct->pPerViewRenderAreas,
               sizeof(VkRect2D) * in_struct->perViewRenderAreaCount);
    }
}

void safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM::initialize(
    const safe_VkMultiviewPerViewRenderAreasRenderPassBeginInfoQCOM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    perViewRenderAreaCount = copy_src->perViewRenderAreaCount;
    pPerViewRenderAreas = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pPerViewRenderAreas) {
        pPerViewRenderAreas = new VkRect2D[copy_src->perViewRenderAreaCount];
        memcpy((void*)pPerViewRenderAreas, (void*)copy_src->pPerViewRenderAreas,
               sizeof(VkRect2D) * copy_src->perViewRenderAreaCount);
    }
}

safe_VkPhysicalDevicePerStageDescriptorSetFeaturesNV::safe_VkPhysicalDevicePerStageDescriptorSetFeaturesNV(
    const VkPhysicalDevicePerStageDescriptorSetFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      perStageDescriptorSet(in_struct->perStageDescriptorSet),
      dynamicPipelineLayout(in_struct->dynamicPipelineLayout) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePerStageDescriptorSetFeaturesNV::safe_VkPhysicalDevicePerStageDescriptorSetFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PER_STAGE_DESCRIPTOR_SET_FEATURES_NV),
      pNext(nullptr),
      perStageDescriptorSet(),
      dynamicPipelineLayout() {}

safe_VkPhysicalDevicePerStageDescriptorSetFeaturesNV::safe_VkPhysicalDevicePerStageDescriptorSetFeaturesNV(
    const safe_VkPhysicalDevicePerStageDescriptorSetFeaturesNV& copy_src) {
    sType = copy_src.sType;
    perStageDescriptorSet = copy_src.perStageDescriptorSet;
    dynamicPipelineLayout = copy_src.dynamicPipelineLayout;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePerStageDescriptorSetFeaturesNV& safe_VkPhysicalDevicePerStageDescriptorSetFeaturesNV::operator=(
    const safe_VkPhysicalDevicePerStageDescriptorSetFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    perStageDescriptorSet = copy_src.perStageDescriptorSet;
    dynamicPipelineLayout = copy_src.dynamicPipelineLayout;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePerStageDescriptorSetFeaturesNV::~safe_VkPhysicalDevicePerStageDescriptorSetFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDevicePerStageDescriptorSetFeaturesNV::initialize(
    const VkPhysicalDevicePerStageDescriptorSetFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    perStageDescriptorSet = in_struct->perStageDescriptorSet;
    dynamicPipelineLayout = in_struct->dynamicPipelineLayout;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePerStageDescriptorSetFeaturesNV::initialize(
    const safe_VkPhysicalDevicePerStageDescriptorSetFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    perStageDescriptorSet = copy_src->perStageDescriptorSet;
    dynamicPipelineLayout = copy_src->dynamicPipelineLayout;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM::safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM(
    const VkPhysicalDeviceImageProcessing2FeaturesQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), textureBlockMatch2(in_struct->textureBlockMatch2) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM::safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_FEATURES_QCOM), pNext(nullptr), textureBlockMatch2() {}

safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM::safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM(
    const safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM& copy_src) {
    sType = copy_src.sType;
    textureBlockMatch2 = copy_src.textureBlockMatch2;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM& safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM::operator=(
    const safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    textureBlockMatch2 = copy_src.textureBlockMatch2;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM::~safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM::initialize(const VkPhysicalDeviceImageProcessing2FeaturesQCOM* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    textureBlockMatch2 = in_struct->textureBlockMatch2;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM::initialize(
    const safe_VkPhysicalDeviceImageProcessing2FeaturesQCOM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    textureBlockMatch2 = copy_src->textureBlockMatch2;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM::safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM(
    const VkPhysicalDeviceImageProcessing2PropertiesQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), maxBlockMatchWindow(in_struct->maxBlockMatchWindow) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM::safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_PROCESSING_2_PROPERTIES_QCOM), pNext(nullptr), maxBlockMatchWindow() {}

safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM::safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM(
    const safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM& copy_src) {
    sType = copy_src.sType;
    maxBlockMatchWindow = copy_src.maxBlockMatchWindow;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM& safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM::operator=(
    const safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxBlockMatchWindow = copy_src.maxBlockMatchWindow;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM::~safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM::initialize(
    const VkPhysicalDeviceImageProcessing2PropertiesQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxBlockMatchWindow = in_struct->maxBlockMatchWindow;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM::initialize(
    const safe_VkPhysicalDeviceImageProcessing2PropertiesQCOM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxBlockMatchWindow = copy_src->maxBlockMatchWindow;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSamplerBlockMatchWindowCreateInfoQCOM::safe_VkSamplerBlockMatchWindowCreateInfoQCOM(
    const VkSamplerBlockMatchWindowCreateInfoQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), windowExtent(in_struct->windowExtent), windowCompareMode(in_struct->windowCompareMode) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSamplerBlockMatchWindowCreateInfoQCOM::safe_VkSamplerBlockMatchWindowCreateInfoQCOM()
    : sType(VK_STRUCTURE_TYPE_SAMPLER_BLOCK_MATCH_WINDOW_CREATE_INFO_QCOM), pNext(nullptr), windowExtent(), windowCompareMode() {}

safe_VkSamplerBlockMatchWindowCreateInfoQCOM::safe_VkSamplerBlockMatchWindowCreateInfoQCOM(
    const safe_VkSamplerBlockMatchWindowCreateInfoQCOM& copy_src) {
    sType = copy_src.sType;
    windowExtent = copy_src.windowExtent;
    windowCompareMode = copy_src.windowCompareMode;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSamplerBlockMatchWindowCreateInfoQCOM& safe_VkSamplerBlockMatchWindowCreateInfoQCOM::operator=(
    const safe_VkSamplerBlockMatchWindowCreateInfoQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    windowExtent = copy_src.windowExtent;
    windowCompareMode = copy_src.windowCompareMode;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSamplerBlockMatchWindowCreateInfoQCOM::~safe_VkSamplerBlockMatchWindowCreateInfoQCOM() { FreePnextChain(pNext); }

void safe_VkSamplerBlockMatchWindowCreateInfoQCOM::initialize(const VkSamplerBlockMatchWindowCreateInfoQCOM* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    windowExtent = in_struct->windowExtent;
    windowCompareMode = in_struct->windowCompareMode;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSamplerBlockMatchWindowCreateInfoQCOM::initialize(const safe_VkSamplerBlockMatchWindowCreateInfoQCOM* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    windowExtent = copy_src->windowExtent;
    windowCompareMode = copy_src->windowCompareMode;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM::safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM(
    const VkPhysicalDeviceCubicWeightsFeaturesQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), selectableCubicWeights(in_struct->selectableCubicWeights) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM::safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_WEIGHTS_FEATURES_QCOM), pNext(nullptr), selectableCubicWeights() {}

safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM::safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM(
    const safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM& copy_src) {
    sType = copy_src.sType;
    selectableCubicWeights = copy_src.selectableCubicWeights;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM& safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM::operator=(
    const safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    selectableCubicWeights = copy_src.selectableCubicWeights;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM::~safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM::initialize(const VkPhysicalDeviceCubicWeightsFeaturesQCOM* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    selectableCubicWeights = in_struct->selectableCubicWeights;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM::initialize(const safe_VkPhysicalDeviceCubicWeightsFeaturesQCOM* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    selectableCubicWeights = copy_src->selectableCubicWeights;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSamplerCubicWeightsCreateInfoQCOM::safe_VkSamplerCubicWeightsCreateInfoQCOM(
    const VkSamplerCubicWeightsCreateInfoQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), cubicWeights(in_struct->cubicWeights) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSamplerCubicWeightsCreateInfoQCOM::safe_VkSamplerCubicWeightsCreateInfoQCOM()
    : sType(VK_STRUCTURE_TYPE_SAMPLER_CUBIC_WEIGHTS_CREATE_INFO_QCOM), pNext(nullptr), cubicWeights() {}

safe_VkSamplerCubicWeightsCreateInfoQCOM::safe_VkSamplerCubicWeightsCreateInfoQCOM(
    const safe_VkSamplerCubicWeightsCreateInfoQCOM& copy_src) {
    sType = copy_src.sType;
    cubicWeights = copy_src.cubicWeights;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSamplerCubicWeightsCreateInfoQCOM& safe_VkSamplerCubicWeightsCreateInfoQCOM::operator=(
    const safe_VkSamplerCubicWeightsCreateInfoQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    cubicWeights = copy_src.cubicWeights;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSamplerCubicWeightsCreateInfoQCOM::~safe_VkSamplerCubicWeightsCreateInfoQCOM() { FreePnextChain(pNext); }

void safe_VkSamplerCubicWeightsCreateInfoQCOM::initialize(const VkSamplerCubicWeightsCreateInfoQCOM* in_struct,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    cubicWeights = in_struct->cubicWeights;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSamplerCubicWeightsCreateInfoQCOM::initialize(const safe_VkSamplerCubicWeightsCreateInfoQCOM* copy_src,
                                                          [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    cubicWeights = copy_src->cubicWeights;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkBlitImageCubicWeightsInfoQCOM::safe_VkBlitImageCubicWeightsInfoQCOM(const VkBlitImageCubicWeightsInfoQCOM* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), cubicWeights(in_struct->cubicWeights) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkBlitImageCubicWeightsInfoQCOM::safe_VkBlitImageCubicWeightsInfoQCOM()
    : sType(VK_STRUCTURE_TYPE_BLIT_IMAGE_CUBIC_WEIGHTS_INFO_QCOM), pNext(nullptr), cubicWeights() {}

safe_VkBlitImageCubicWeightsInfoQCOM::safe_VkBlitImageCubicWeightsInfoQCOM(const safe_VkBlitImageCubicWeightsInfoQCOM& copy_src) {
    sType = copy_src.sType;
    cubicWeights = copy_src.cubicWeights;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkBlitImageCubicWeightsInfoQCOM& safe_VkBlitImageCubicWeightsInfoQCOM::operator=(
    const safe_VkBlitImageCubicWeightsInfoQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    cubicWeights = copy_src.cubicWeights;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkBlitImageCubicWeightsInfoQCOM::~safe_VkBlitImageCubicWeightsInfoQCOM() { FreePnextChain(pNext); }

void safe_VkBlitImageCubicWeightsInfoQCOM::initialize(const VkBlitImageCubicWeightsInfoQCOM* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    cubicWeights = in_struct->cubicWeights;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkBlitImageCubicWeightsInfoQCOM::initialize(const safe_VkBlitImageCubicWeightsInfoQCOM* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    cubicWeights = copy_src->cubicWeights;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM::safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM(
    const VkPhysicalDeviceYcbcrDegammaFeaturesQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), ycbcrDegamma(in_struct->ycbcrDegamma) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM::safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_DEGAMMA_FEATURES_QCOM), pNext(nullptr), ycbcrDegamma() {}

safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM::safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM(
    const safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM& copy_src) {
    sType = copy_src.sType;
    ycbcrDegamma = copy_src.ycbcrDegamma;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM& safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM::operator=(
    const safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    ycbcrDegamma = copy_src.ycbcrDegamma;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM::~safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM::initialize(const VkPhysicalDeviceYcbcrDegammaFeaturesQCOM* in_struct,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    ycbcrDegamma = in_struct->ycbcrDegamma;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM::initialize(const safe_VkPhysicalDeviceYcbcrDegammaFeaturesQCOM* copy_src,
                                                               [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    ycbcrDegamma = copy_src->ycbcrDegamma;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM::safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM(
    const VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), enableYDegamma(in_struct->enableYDegamma), enableCbCrDegamma(in_struct->enableCbCrDegamma) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM::safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM()
    : sType(VK_STRUCTURE_TYPE_SAMPLER_YCBCR_CONVERSION_YCBCR_DEGAMMA_CREATE_INFO_QCOM),
      pNext(nullptr),
      enableYDegamma(),
      enableCbCrDegamma() {}

safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM::safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM(
    const safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM& copy_src) {
    sType = copy_src.sType;
    enableYDegamma = copy_src.enableYDegamma;
    enableCbCrDegamma = copy_src.enableCbCrDegamma;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM& safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM::operator=(
    const safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    enableYDegamma = copy_src.enableYDegamma;
    enableCbCrDegamma = copy_src.enableCbCrDegamma;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM::~safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM() {
    FreePnextChain(pNext);
}

void safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM::initialize(
    const VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    enableYDegamma = in_struct->enableYDegamma;
    enableCbCrDegamma = in_struct->enableCbCrDegamma;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM::initialize(
    const safe_VkSamplerYcbcrConversionYcbcrDegammaCreateInfoQCOM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    enableYDegamma = copy_src->enableYDegamma;
    enableCbCrDegamma = copy_src->enableCbCrDegamma;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCubicClampFeaturesQCOM::safe_VkPhysicalDeviceCubicClampFeaturesQCOM(
    const VkPhysicalDeviceCubicClampFeaturesQCOM* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), cubicRangeClamp(in_struct->cubicRangeClamp) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCubicClampFeaturesQCOM::safe_VkPhysicalDeviceCubicClampFeaturesQCOM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CUBIC_CLAMP_FEATURES_QCOM), pNext(nullptr), cubicRangeClamp() {}

safe_VkPhysicalDeviceCubicClampFeaturesQCOM::safe_VkPhysicalDeviceCubicClampFeaturesQCOM(
    const safe_VkPhysicalDeviceCubicClampFeaturesQCOM& copy_src) {
    sType = copy_src.sType;
    cubicRangeClamp = copy_src.cubicRangeClamp;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCubicClampFeaturesQCOM& safe_VkPhysicalDeviceCubicClampFeaturesQCOM::operator=(
    const safe_VkPhysicalDeviceCubicClampFeaturesQCOM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    cubicRangeClamp = copy_src.cubicRangeClamp;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCubicClampFeaturesQCOM::~safe_VkPhysicalDeviceCubicClampFeaturesQCOM() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceCubicClampFeaturesQCOM::initialize(const VkPhysicalDeviceCubicClampFeaturesQCOM* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    cubicRangeClamp = in_struct->cubicRangeClamp;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCubicClampFeaturesQCOM::initialize(const safe_VkPhysicalDeviceCubicClampFeaturesQCOM* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    cubicRangeClamp = copy_src->cubicRangeClamp;
    pNext = SafePnextCopy(copy_src->pNext);
}
#ifdef VK_USE_PLATFORM_SCREEN_QNX

safe_VkScreenBufferPropertiesQNX::safe_VkScreenBufferPropertiesQNX(const VkScreenBufferPropertiesQNX* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), allocationSize(in_struct->allocationSize), memoryTypeBits(in_struct->memoryTypeBits) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkScreenBufferPropertiesQNX::safe_VkScreenBufferPropertiesQNX()
    : sType(VK_STRUCTURE_TYPE_SCREEN_BUFFER_PROPERTIES_QNX), pNext(nullptr), allocationSize(), memoryTypeBits() {}

safe_VkScreenBufferPropertiesQNX::safe_VkScreenBufferPropertiesQNX(const safe_VkScreenBufferPropertiesQNX& copy_src) {
    sType = copy_src.sType;
    allocationSize = copy_src.allocationSize;
    memoryTypeBits = copy_src.memoryTypeBits;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkScreenBufferPropertiesQNX& safe_VkScreenBufferPropertiesQNX::operator=(const safe_VkScreenBufferPropertiesQNX& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    allocationSize = copy_src.allocationSize;
    memoryTypeBits = copy_src.memoryTypeBits;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkScreenBufferPropertiesQNX::~safe_VkScreenBufferPropertiesQNX() { FreePnextChain(pNext); }

void safe_VkScreenBufferPropertiesQNX::initialize(const VkScreenBufferPropertiesQNX* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    allocationSize = in_struct->allocationSize;
    memoryTypeBits = in_struct->memoryTypeBits;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkScreenBufferPropertiesQNX::initialize(const safe_VkScreenBufferPropertiesQNX* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    allocationSize = copy_src->allocationSize;
    memoryTypeBits = copy_src->memoryTypeBits;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkScreenBufferFormatPropertiesQNX::safe_VkScreenBufferFormatPropertiesQNX(const VkScreenBufferFormatPropertiesQNX* in_struct,
                                                                               [[maybe_unused]] PNextCopyState* copy_state,
                                                                               bool copy_pnext)
    : sType(in_struct->sType),
      format(in_struct->format),
      externalFormat(in_struct->externalFormat),
      screenUsage(in_struct->screenUsage),
      formatFeatures(in_struct->formatFeatures),
      samplerYcbcrConversionComponents(in_struct->samplerYcbcrConversionComponents),
      suggestedYcbcrModel(in_struct->suggestedYcbcrModel),
      suggestedYcbcrRange(in_struct->suggestedYcbcrRange),
      suggestedXChromaOffset(in_struct->suggestedXChromaOffset),
      suggestedYChromaOffset(in_struct->suggestedYChromaOffset) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkScreenBufferFormatPropertiesQNX::safe_VkScreenBufferFormatPropertiesQNX()
    : sType(VK_STRUCTURE_TYPE_SCREEN_BUFFER_FORMAT_PROPERTIES_QNX),
      pNext(nullptr),
      format(),
      externalFormat(),
      screenUsage(),
      formatFeatures(),
      samplerYcbcrConversionComponents(),
      suggestedYcbcrModel(),
      suggestedYcbcrRange(),
      suggestedXChromaOffset(),
      suggestedYChromaOffset() {}

safe_VkScreenBufferFormatPropertiesQNX::safe_VkScreenBufferFormatPropertiesQNX(
    const safe_VkScreenBufferFormatPropertiesQNX& copy_src) {
    sType = copy_src.sType;
    format = copy_src.format;
    externalFormat = copy_src.externalFormat;
    screenUsage = copy_src.screenUsage;
    formatFeatures = copy_src.formatFeatures;
    samplerYcbcrConversionComponents = copy_src.samplerYcbcrConversionComponents;
    suggestedYcbcrModel = copy_src.suggestedYcbcrModel;
    suggestedYcbcrRange = copy_src.suggestedYcbcrRange;
    suggestedXChromaOffset = copy_src.suggestedXChromaOffset;
    suggestedYChromaOffset = copy_src.suggestedYChromaOffset;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkScreenBufferFormatPropertiesQNX& safe_VkScreenBufferFormatPropertiesQNX::operator=(
    const safe_VkScreenBufferFormatPropertiesQNX& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    format = copy_src.format;
    externalFormat = copy_src.externalFormat;
    screenUsage = copy_src.screenUsage;
    formatFeatures = copy_src.formatFeatures;
    samplerYcbcrConversionComponents = copy_src.samplerYcbcrConversionComponents;
    suggestedYcbcrModel = copy_src.suggestedYcbcrModel;
    suggestedYcbcrRange = copy_src.suggestedYcbcrRange;
    suggestedXChromaOffset = copy_src.suggestedXChromaOffset;
    suggestedYChromaOffset = copy_src.suggestedYChromaOffset;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkScreenBufferFormatPropertiesQNX::~safe_VkScreenBufferFormatPropertiesQNX() { FreePnextChain(pNext); }

void safe_VkScreenBufferFormatPropertiesQNX::initialize(const VkScreenBufferFormatPropertiesQNX* in_struct,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    format = in_struct->format;
    externalFormat = in_struct->externalFormat;
    screenUsage = in_struct->screenUsage;
    formatFeatures = in_struct->formatFeatures;
    samplerYcbcrConversionComponents = in_struct->samplerYcbcrConversionComponents;
    suggestedYcbcrModel = in_struct->suggestedYcbcrModel;
    suggestedYcbcrRange = in_struct->suggestedYcbcrRange;
    suggestedXChromaOffset = in_struct->suggestedXChromaOffset;
    suggestedYChromaOffset = in_struct->suggestedYChromaOffset;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkScreenBufferFormatPropertiesQNX::initialize(const safe_VkScreenBufferFormatPropertiesQNX* copy_src,
                                                        [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    format = copy_src->format;
    externalFormat = copy_src->externalFormat;
    screenUsage = copy_src->screenUsage;
    formatFeatures = copy_src->formatFeatures;
    samplerYcbcrConversionComponents = copy_src->samplerYcbcrConversionComponents;
    suggestedYcbcrModel = copy_src->suggestedYcbcrModel;
    suggestedYcbcrRange = copy_src->suggestedYcbcrRange;
    suggestedXChromaOffset = copy_src->suggestedXChromaOffset;
    suggestedYChromaOffset = copy_src->suggestedYChromaOffset;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImportScreenBufferInfoQNX::safe_VkImportScreenBufferInfoQNX(const VkImportScreenBufferInfoQNX* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), buffer(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    buffer = in_struct->buffer;
}

safe_VkImportScreenBufferInfoQNX::safe_VkImportScreenBufferInfoQNX()
    : sType(VK_STRUCTURE_TYPE_IMPORT_SCREEN_BUFFER_INFO_QNX), pNext(nullptr), buffer(nullptr) {}

safe_VkImportScreenBufferInfoQNX::safe_VkImportScreenBufferInfoQNX(const safe_VkImportScreenBufferInfoQNX& copy_src) {
    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);
    buffer = copy_src.buffer;
}

safe_VkImportScreenBufferInfoQNX& safe_VkImportScreenBufferInfoQNX::operator=(const safe_VkImportScreenBufferInfoQNX& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pNext = SafePnextCopy(copy_src.pNext);
    buffer = copy_src.buffer;

    return *this;
}

safe_VkImportScreenBufferInfoQNX::~safe_VkImportScreenBufferInfoQNX() { FreePnextChain(pNext); }

void safe_VkImportScreenBufferInfoQNX::initialize(const VkImportScreenBufferInfoQNX* in_struct,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
    buffer = in_struct->buffer;
}

void safe_VkImportScreenBufferInfoQNX::initialize(const safe_VkImportScreenBufferInfoQNX* copy_src,
                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pNext = SafePnextCopy(copy_src->pNext);
    buffer = copy_src->buffer;
}

safe_VkExternalFormatQNX::safe_VkExternalFormatQNX(const VkExternalFormatQNX* in_struct,
                                                   [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), externalFormat(in_struct->externalFormat) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkExternalFormatQNX::safe_VkExternalFormatQNX()
    : sType(VK_STRUCTURE_TYPE_EXTERNAL_FORMAT_QNX), pNext(nullptr), externalFormat() {}

safe_VkExternalFormatQNX::safe_VkExternalFormatQNX(const safe_VkExternalFormatQNX& copy_src) {
    sType = copy_src.sType;
    externalFormat = copy_src.externalFormat;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkExternalFormatQNX& safe_VkExternalFormatQNX::operator=(const safe_VkExternalFormatQNX& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    externalFormat = copy_src.externalFormat;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkExternalFormatQNX::~safe_VkExternalFormatQNX() { FreePnextChain(pNext); }

void safe_VkExternalFormatQNX::initialize(const VkExternalFormatQNX* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    externalFormat = in_struct->externalFormat;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkExternalFormatQNX::initialize(const safe_VkExternalFormatQNX* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    externalFormat = copy_src->externalFormat;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX::safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX(
    const VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), screenBufferImport(in_struct->screenBufferImport) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX::safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_SCREEN_BUFFER_FEATURES_QNX), pNext(nullptr), screenBufferImport() {}

safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX::safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX(
    const safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX& copy_src) {
    sType = copy_src.sType;
    screenBufferImport = copy_src.screenBufferImport;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX& safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX::operator=(
    const safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    screenBufferImport = copy_src.screenBufferImport;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX::~safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX::initialize(
    const VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    screenBufferImport = in_struct->screenBufferImport;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX::initialize(
    const safe_VkPhysicalDeviceExternalMemoryScreenBufferFeaturesQNX* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    screenBufferImport = copy_src->screenBufferImport;
    pNext = SafePnextCopy(copy_src->pNext);
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT::safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT(
    const VkPhysicalDeviceLayeredDriverPropertiesMSFT* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), underlyingAPI(in_struct->underlyingAPI) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT::safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_LAYERED_DRIVER_PROPERTIES_MSFT), pNext(nullptr), underlyingAPI() {}

safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT::safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT(
    const safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT& copy_src) {
    sType = copy_src.sType;
    underlyingAPI = copy_src.underlyingAPI;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT& safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT::operator=(
    const safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    underlyingAPI = copy_src.underlyingAPI;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT::~safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT::initialize(const VkPhysicalDeviceLayeredDriverPropertiesMSFT* in_struct,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    underlyingAPI = in_struct->underlyingAPI;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT::initialize(const safe_VkPhysicalDeviceLayeredDriverPropertiesMSFT* copy_src,
                                                                  [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    underlyingAPI = copy_src->underlyingAPI;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV::safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV(
    const VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), descriptorPoolOverallocation(in_struct->descriptorPoolOverallocation) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV::safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_POOL_OVERALLOCATION_FEATURES_NV),
      pNext(nullptr),
      descriptorPoolOverallocation() {}

safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV::safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV(
    const safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV& copy_src) {
    sType = copy_src.sType;
    descriptorPoolOverallocation = copy_src.descriptorPoolOverallocation;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV& safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV::operator=(
    const safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    descriptorPoolOverallocation = copy_src.descriptorPoolOverallocation;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV::~safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV::initialize(
    const VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    descriptorPoolOverallocation = in_struct->descriptorPoolOverallocation;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV::initialize(
    const safe_VkPhysicalDeviceDescriptorPoolOverallocationFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    descriptorPoolOverallocation = copy_src->descriptorPoolOverallocation;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplaySurfaceStereoCreateInfoNV::safe_VkDisplaySurfaceStereoCreateInfoNV(
    const VkDisplaySurfaceStereoCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), stereoType(in_struct->stereoType) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplaySurfaceStereoCreateInfoNV::safe_VkDisplaySurfaceStereoCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_SURFACE_STEREO_CREATE_INFO_NV), pNext(nullptr), stereoType() {}

safe_VkDisplaySurfaceStereoCreateInfoNV::safe_VkDisplaySurfaceStereoCreateInfoNV(
    const safe_VkDisplaySurfaceStereoCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    stereoType = copy_src.stereoType;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplaySurfaceStereoCreateInfoNV& safe_VkDisplaySurfaceStereoCreateInfoNV::operator=(
    const safe_VkDisplaySurfaceStereoCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    stereoType = copy_src.stereoType;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplaySurfaceStereoCreateInfoNV::~safe_VkDisplaySurfaceStereoCreateInfoNV() { FreePnextChain(pNext); }

void safe_VkDisplaySurfaceStereoCreateInfoNV::initialize(const VkDisplaySurfaceStereoCreateInfoNV* in_struct,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    stereoType = in_struct->stereoType;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplaySurfaceStereoCreateInfoNV::initialize(const safe_VkDisplaySurfaceStereoCreateInfoNV* copy_src,
                                                         [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    stereoType = copy_src->stereoType;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkDisplayModeStereoPropertiesNV::safe_VkDisplayModeStereoPropertiesNV(const VkDisplayModeStereoPropertiesNV* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), hdmi3DSupported(in_struct->hdmi3DSupported) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkDisplayModeStereoPropertiesNV::safe_VkDisplayModeStereoPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_DISPLAY_MODE_STEREO_PROPERTIES_NV), pNext(nullptr), hdmi3DSupported() {}

safe_VkDisplayModeStereoPropertiesNV::safe_VkDisplayModeStereoPropertiesNV(const safe_VkDisplayModeStereoPropertiesNV& copy_src) {
    sType = copy_src.sType;
    hdmi3DSupported = copy_src.hdmi3DSupported;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkDisplayModeStereoPropertiesNV& safe_VkDisplayModeStereoPropertiesNV::operator=(
    const safe_VkDisplayModeStereoPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    hdmi3DSupported = copy_src.hdmi3DSupported;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkDisplayModeStereoPropertiesNV::~safe_VkDisplayModeStereoPropertiesNV() { FreePnextChain(pNext); }

void safe_VkDisplayModeStereoPropertiesNV::initialize(const VkDisplayModeStereoPropertiesNV* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    hdmi3DSupported = in_struct->hdmi3DSupported;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkDisplayModeStereoPropertiesNV::initialize(const safe_VkDisplayModeStereoPropertiesNV* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    hdmi3DSupported = copy_src->hdmi3DSupported;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRawAccessChainsFeaturesNV::safe_VkPhysicalDeviceRawAccessChainsFeaturesNV(
    const VkPhysicalDeviceRawAccessChainsFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), shaderRawAccessChains(in_struct->shaderRawAccessChains) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRawAccessChainsFeaturesNV::safe_VkPhysicalDeviceRawAccessChainsFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAW_ACCESS_CHAINS_FEATURES_NV), pNext(nullptr), shaderRawAccessChains() {}

safe_VkPhysicalDeviceRawAccessChainsFeaturesNV::safe_VkPhysicalDeviceRawAccessChainsFeaturesNV(
    const safe_VkPhysicalDeviceRawAccessChainsFeaturesNV& copy_src) {
    sType = copy_src.sType;
    shaderRawAccessChains = copy_src.shaderRawAccessChains;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRawAccessChainsFeaturesNV& safe_VkPhysicalDeviceRawAccessChainsFeaturesNV::operator=(
    const safe_VkPhysicalDeviceRawAccessChainsFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderRawAccessChains = copy_src.shaderRawAccessChains;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRawAccessChainsFeaturesNV::~safe_VkPhysicalDeviceRawAccessChainsFeaturesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceRawAccessChainsFeaturesNV::initialize(const VkPhysicalDeviceRawAccessChainsFeaturesNV* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderRawAccessChains = in_struct->shaderRawAccessChains;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRawAccessChainsFeaturesNV::initialize(const safe_VkPhysicalDeviceRawAccessChainsFeaturesNV* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderRawAccessChains = copy_src->shaderRawAccessChains;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCommandBufferInheritanceFeaturesNV::safe_VkPhysicalDeviceCommandBufferInheritanceFeaturesNV(
    const VkPhysicalDeviceCommandBufferInheritanceFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), commandBufferInheritance(in_struct->commandBufferInheritance) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCommandBufferInheritanceFeaturesNV::safe_VkPhysicalDeviceCommandBufferInheritanceFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COMMAND_BUFFER_INHERITANCE_FEATURES_NV), pNext(nullptr), commandBufferInheritance() {}

safe_VkPhysicalDeviceCommandBufferInheritanceFeaturesNV::safe_VkPhysicalDeviceCommandBufferInheritanceFeaturesNV(
    const safe_VkPhysicalDeviceCommandBufferInheritanceFeaturesNV& copy_src) {
    sType = copy_src.sType;
    commandBufferInheritance = copy_src.commandBufferInheritance;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCommandBufferInheritanceFeaturesNV& safe_VkPhysicalDeviceCommandBufferInheritanceFeaturesNV::operator=(
    const safe_VkPhysicalDeviceCommandBufferInheritanceFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    commandBufferInheritance = copy_src.commandBufferInheritance;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCommandBufferInheritanceFeaturesNV::~safe_VkPhysicalDeviceCommandBufferInheritanceFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceCommandBufferInheritanceFeaturesNV::initialize(
    const VkPhysicalDeviceCommandBufferInheritanceFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    commandBufferInheritance = in_struct->commandBufferInheritance;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCommandBufferInheritanceFeaturesNV::initialize(
    const safe_VkPhysicalDeviceCommandBufferInheritanceFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    commandBufferInheritance = copy_src->commandBufferInheritance;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV::safe_VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV(
    const VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), shaderFloat16VectorAtomics(in_struct->shaderFloat16VectorAtomics) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV::safe_VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT16_VECTOR_FEATURES_NV),
      pNext(nullptr),
      shaderFloat16VectorAtomics() {}

safe_VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV::safe_VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV(
    const safe_VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV& copy_src) {
    sType = copy_src.sType;
    shaderFloat16VectorAtomics = copy_src.shaderFloat16VectorAtomics;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV& safe_VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV::operator=(
    const safe_VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    shaderFloat16VectorAtomics = copy_src.shaderFloat16VectorAtomics;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV::~safe_VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV::initialize(
    const VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    shaderFloat16VectorAtomics = in_struct->shaderFloat16VectorAtomics;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV::initialize(
    const safe_VkPhysicalDeviceShaderAtomicFloat16VectorFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    shaderFloat16VectorAtomics = copy_src->shaderFloat16VectorAtomics;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceRayTracingValidationFeaturesNV::safe_VkPhysicalDeviceRayTracingValidationFeaturesNV(
    const VkPhysicalDeviceRayTracingValidationFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), rayTracingValidation(in_struct->rayTracingValidation) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceRayTracingValidationFeaturesNV::safe_VkPhysicalDeviceRayTracingValidationFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_VALIDATION_FEATURES_NV), pNext(nullptr), rayTracingValidation() {}

safe_VkPhysicalDeviceRayTracingValidationFeaturesNV::safe_VkPhysicalDeviceRayTracingValidationFeaturesNV(
    const safe_VkPhysicalDeviceRayTracingValidationFeaturesNV& copy_src) {
    sType = copy_src.sType;
    rayTracingValidation = copy_src.rayTracingValidation;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceRayTracingValidationFeaturesNV& safe_VkPhysicalDeviceRayTracingValidationFeaturesNV::operator=(
    const safe_VkPhysicalDeviceRayTracingValidationFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    rayTracingValidation = copy_src.rayTracingValidation;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceRayTracingValidationFeaturesNV::~safe_VkPhysicalDeviceRayTracingValidationFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceRayTracingValidationFeaturesNV::initialize(
    const VkPhysicalDeviceRayTracingValidationFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    rayTracingValidation = in_struct->rayTracingValidation;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceRayTracingValidationFeaturesNV::initialize(
    const safe_VkPhysicalDeviceRayTracingValidationFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    rayTracingValidation = copy_src->rayTracingValidation;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceClusterAccelerationStructureFeaturesNV::safe_VkPhysicalDeviceClusterAccelerationStructureFeaturesNV(
    const VkPhysicalDeviceClusterAccelerationStructureFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), clusterAccelerationStructure(in_struct->clusterAccelerationStructure) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceClusterAccelerationStructureFeaturesNV::safe_VkPhysicalDeviceClusterAccelerationStructureFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_FEATURES_NV),
      pNext(nullptr),
      clusterAccelerationStructure() {}

safe_VkPhysicalDeviceClusterAccelerationStructureFeaturesNV::safe_VkPhysicalDeviceClusterAccelerationStructureFeaturesNV(
    const safe_VkPhysicalDeviceClusterAccelerationStructureFeaturesNV& copy_src) {
    sType = copy_src.sType;
    clusterAccelerationStructure = copy_src.clusterAccelerationStructure;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceClusterAccelerationStructureFeaturesNV& safe_VkPhysicalDeviceClusterAccelerationStructureFeaturesNV::operator=(
    const safe_VkPhysicalDeviceClusterAccelerationStructureFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    clusterAccelerationStructure = copy_src.clusterAccelerationStructure;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceClusterAccelerationStructureFeaturesNV::~safe_VkPhysicalDeviceClusterAccelerationStructureFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceClusterAccelerationStructureFeaturesNV::initialize(
    const VkPhysicalDeviceClusterAccelerationStructureFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    clusterAccelerationStructure = in_struct->clusterAccelerationStructure;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceClusterAccelerationStructureFeaturesNV::initialize(
    const safe_VkPhysicalDeviceClusterAccelerationStructureFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    clusterAccelerationStructure = copy_src->clusterAccelerationStructure;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceClusterAccelerationStructurePropertiesNV::safe_VkPhysicalDeviceClusterAccelerationStructurePropertiesNV(
    const VkPhysicalDeviceClusterAccelerationStructurePropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      maxVerticesPerCluster(in_struct->maxVerticesPerCluster),
      maxTrianglesPerCluster(in_struct->maxTrianglesPerCluster),
      clusterScratchByteAlignment(in_struct->clusterScratchByteAlignment),
      clusterByteAlignment(in_struct->clusterByteAlignment),
      clusterTemplateByteAlignment(in_struct->clusterTemplateByteAlignment),
      clusterBottomLevelByteAlignment(in_struct->clusterBottomLevelByteAlignment),
      clusterTemplateBoundsByteAlignment(in_struct->clusterTemplateBoundsByteAlignment),
      maxClusterGeometryIndex(in_struct->maxClusterGeometryIndex) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceClusterAccelerationStructurePropertiesNV::safe_VkPhysicalDeviceClusterAccelerationStructurePropertiesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_CLUSTER_ACCELERATION_STRUCTURE_PROPERTIES_NV),
      pNext(nullptr),
      maxVerticesPerCluster(),
      maxTrianglesPerCluster(),
      clusterScratchByteAlignment(),
      clusterByteAlignment(),
      clusterTemplateByteAlignment(),
      clusterBottomLevelByteAlignment(),
      clusterTemplateBoundsByteAlignment(),
      maxClusterGeometryIndex() {}

safe_VkPhysicalDeviceClusterAccelerationStructurePropertiesNV::safe_VkPhysicalDeviceClusterAccelerationStructurePropertiesNV(
    const safe_VkPhysicalDeviceClusterAccelerationStructurePropertiesNV& copy_src) {
    sType = copy_src.sType;
    maxVerticesPerCluster = copy_src.maxVerticesPerCluster;
    maxTrianglesPerCluster = copy_src.maxTrianglesPerCluster;
    clusterScratchByteAlignment = copy_src.clusterScratchByteAlignment;
    clusterByteAlignment = copy_src.clusterByteAlignment;
    clusterTemplateByteAlignment = copy_src.clusterTemplateByteAlignment;
    clusterBottomLevelByteAlignment = copy_src.clusterBottomLevelByteAlignment;
    clusterTemplateBoundsByteAlignment = copy_src.clusterTemplateBoundsByteAlignment;
    maxClusterGeometryIndex = copy_src.maxClusterGeometryIndex;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceClusterAccelerationStructurePropertiesNV&
safe_VkPhysicalDeviceClusterAccelerationStructurePropertiesNV::operator=(
    const safe_VkPhysicalDeviceClusterAccelerationStructurePropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxVerticesPerCluster = copy_src.maxVerticesPerCluster;
    maxTrianglesPerCluster = copy_src.maxTrianglesPerCluster;
    clusterScratchByteAlignment = copy_src.clusterScratchByteAlignment;
    clusterByteAlignment = copy_src.clusterByteAlignment;
    clusterTemplateByteAlignment = copy_src.clusterTemplateByteAlignment;
    clusterBottomLevelByteAlignment = copy_src.clusterBottomLevelByteAlignment;
    clusterTemplateBoundsByteAlignment = copy_src.clusterTemplateBoundsByteAlignment;
    maxClusterGeometryIndex = copy_src.maxClusterGeometryIndex;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceClusterAccelerationStructurePropertiesNV::~safe_VkPhysicalDeviceClusterAccelerationStructurePropertiesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceClusterAccelerationStructurePropertiesNV::initialize(
    const VkPhysicalDeviceClusterAccelerationStructurePropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxVerticesPerCluster = in_struct->maxVerticesPerCluster;
    maxTrianglesPerCluster = in_struct->maxTrianglesPerCluster;
    clusterScratchByteAlignment = in_struct->clusterScratchByteAlignment;
    clusterByteAlignment = in_struct->clusterByteAlignment;
    clusterTemplateByteAlignment = in_struct->clusterTemplateByteAlignment;
    clusterBottomLevelByteAlignment = in_struct->clusterBottomLevelByteAlignment;
    clusterTemplateBoundsByteAlignment = in_struct->clusterTemplateBoundsByteAlignment;
    maxClusterGeometryIndex = in_struct->maxClusterGeometryIndex;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceClusterAccelerationStructurePropertiesNV::initialize(
    const safe_VkPhysicalDeviceClusterAccelerationStructurePropertiesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxVerticesPerCluster = copy_src->maxVerticesPerCluster;
    maxTrianglesPerCluster = copy_src->maxTrianglesPerCluster;
    clusterScratchByteAlignment = copy_src->clusterScratchByteAlignment;
    clusterByteAlignment = copy_src->clusterByteAlignment;
    clusterTemplateByteAlignment = copy_src->clusterTemplateByteAlignment;
    clusterBottomLevelByteAlignment = copy_src->clusterBottomLevelByteAlignment;
    clusterTemplateBoundsByteAlignment = copy_src->clusterTemplateBoundsByteAlignment;
    maxClusterGeometryIndex = copy_src->maxClusterGeometryIndex;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkClusterAccelerationStructureClustersBottomLevelInputNV::safe_VkClusterAccelerationStructureClustersBottomLevelInputNV(
    const VkClusterAccelerationStructureClustersBottomLevelInputNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      maxTotalClusterCount(in_struct->maxTotalClusterCount),
      maxClusterCountPerAccelerationStructure(in_struct->maxClusterCountPerAccelerationStructure) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkClusterAccelerationStructureClustersBottomLevelInputNV::safe_VkClusterAccelerationStructureClustersBottomLevelInputNV()
    : sType(VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_CLUSTERS_BOTTOM_LEVEL_INPUT_NV),
      pNext(nullptr),
      maxTotalClusterCount(),
      maxClusterCountPerAccelerationStructure() {}

safe_VkClusterAccelerationStructureClustersBottomLevelInputNV::safe_VkClusterAccelerationStructureClustersBottomLevelInputNV(
    const safe_VkClusterAccelerationStructureClustersBottomLevelInputNV& copy_src) {
    sType = copy_src.sType;
    maxTotalClusterCount = copy_src.maxTotalClusterCount;
    maxClusterCountPerAccelerationStructure = copy_src.maxClusterCountPerAccelerationStructure;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkClusterAccelerationStructureClustersBottomLevelInputNV&
safe_VkClusterAccelerationStructureClustersBottomLevelInputNV::operator=(
    const safe_VkClusterAccelerationStructureClustersBottomLevelInputNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxTotalClusterCount = copy_src.maxTotalClusterCount;
    maxClusterCountPerAccelerationStructure = copy_src.maxClusterCountPerAccelerationStructure;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkClusterAccelerationStructureClustersBottomLevelInputNV::~safe_VkClusterAccelerationStructureClustersBottomLevelInputNV() {
    FreePnextChain(pNext);
}

void safe_VkClusterAccelerationStructureClustersBottomLevelInputNV::initialize(
    const VkClusterAccelerationStructureClustersBottomLevelInputNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxTotalClusterCount = in_struct->maxTotalClusterCount;
    maxClusterCountPerAccelerationStructure = in_struct->maxClusterCountPerAccelerationStructure;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkClusterAccelerationStructureClustersBottomLevelInputNV::initialize(
    const safe_VkClusterAccelerationStructureClustersBottomLevelInputNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxTotalClusterCount = copy_src->maxTotalClusterCount;
    maxClusterCountPerAccelerationStructure = copy_src->maxClusterCountPerAccelerationStructure;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkClusterAccelerationStructureTriangleClusterInputNV::safe_VkClusterAccelerationStructureTriangleClusterInputNV(
    const VkClusterAccelerationStructureTriangleClusterInputNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      vertexFormat(in_struct->vertexFormat),
      maxGeometryIndexValue(in_struct->maxGeometryIndexValue),
      maxClusterUniqueGeometryCount(in_struct->maxClusterUniqueGeometryCount),
      maxClusterTriangleCount(in_struct->maxClusterTriangleCount),
      maxClusterVertexCount(in_struct->maxClusterVertexCount),
      maxTotalTriangleCount(in_struct->maxTotalTriangleCount),
      maxTotalVertexCount(in_struct->maxTotalVertexCount),
      minPositionTruncateBitCount(in_struct->minPositionTruncateBitCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkClusterAccelerationStructureTriangleClusterInputNV::safe_VkClusterAccelerationStructureTriangleClusterInputNV()
    : sType(VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_TRIANGLE_CLUSTER_INPUT_NV),
      pNext(nullptr),
      vertexFormat(),
      maxGeometryIndexValue(),
      maxClusterUniqueGeometryCount(),
      maxClusterTriangleCount(),
      maxClusterVertexCount(),
      maxTotalTriangleCount(),
      maxTotalVertexCount(),
      minPositionTruncateBitCount() {}

safe_VkClusterAccelerationStructureTriangleClusterInputNV::safe_VkClusterAccelerationStructureTriangleClusterInputNV(
    const safe_VkClusterAccelerationStructureTriangleClusterInputNV& copy_src) {
    sType = copy_src.sType;
    vertexFormat = copy_src.vertexFormat;
    maxGeometryIndexValue = copy_src.maxGeometryIndexValue;
    maxClusterUniqueGeometryCount = copy_src.maxClusterUniqueGeometryCount;
    maxClusterTriangleCount = copy_src.maxClusterTriangleCount;
    maxClusterVertexCount = copy_src.maxClusterVertexCount;
    maxTotalTriangleCount = copy_src.maxTotalTriangleCount;
    maxTotalVertexCount = copy_src.maxTotalVertexCount;
    minPositionTruncateBitCount = copy_src.minPositionTruncateBitCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkClusterAccelerationStructureTriangleClusterInputNV& safe_VkClusterAccelerationStructureTriangleClusterInputNV::operator=(
    const safe_VkClusterAccelerationStructureTriangleClusterInputNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    vertexFormat = copy_src.vertexFormat;
    maxGeometryIndexValue = copy_src.maxGeometryIndexValue;
    maxClusterUniqueGeometryCount = copy_src.maxClusterUniqueGeometryCount;
    maxClusterTriangleCount = copy_src.maxClusterTriangleCount;
    maxClusterVertexCount = copy_src.maxClusterVertexCount;
    maxTotalTriangleCount = copy_src.maxTotalTriangleCount;
    maxTotalVertexCount = copy_src.maxTotalVertexCount;
    minPositionTruncateBitCount = copy_src.minPositionTruncateBitCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkClusterAccelerationStructureTriangleClusterInputNV::~safe_VkClusterAccelerationStructureTriangleClusterInputNV() {
    FreePnextChain(pNext);
}

void safe_VkClusterAccelerationStructureTriangleClusterInputNV::initialize(
    const VkClusterAccelerationStructureTriangleClusterInputNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    vertexFormat = in_struct->vertexFormat;
    maxGeometryIndexValue = in_struct->maxGeometryIndexValue;
    maxClusterUniqueGeometryCount = in_struct->maxClusterUniqueGeometryCount;
    maxClusterTriangleCount = in_struct->maxClusterTriangleCount;
    maxClusterVertexCount = in_struct->maxClusterVertexCount;
    maxTotalTriangleCount = in_struct->maxTotalTriangleCount;
    maxTotalVertexCount = in_struct->maxTotalVertexCount;
    minPositionTruncateBitCount = in_struct->minPositionTruncateBitCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkClusterAccelerationStructureTriangleClusterInputNV::initialize(
    const safe_VkClusterAccelerationStructureTriangleClusterInputNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    vertexFormat = copy_src->vertexFormat;
    maxGeometryIndexValue = copy_src->maxGeometryIndexValue;
    maxClusterUniqueGeometryCount = copy_src->maxClusterUniqueGeometryCount;
    maxClusterTriangleCount = copy_src->maxClusterTriangleCount;
    maxClusterVertexCount = copy_src->maxClusterVertexCount;
    maxTotalTriangleCount = copy_src->maxTotalTriangleCount;
    maxTotalVertexCount = copy_src->maxTotalVertexCount;
    minPositionTruncateBitCount = copy_src->minPositionTruncateBitCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkClusterAccelerationStructureMoveObjectsInputNV::safe_VkClusterAccelerationStructureMoveObjectsInputNV(
    const VkClusterAccelerationStructureMoveObjectsInputNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      type(in_struct->type),
      noMoveOverlap(in_struct->noMoveOverlap),
      maxMovedBytes(in_struct->maxMovedBytes) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkClusterAccelerationStructureMoveObjectsInputNV::safe_VkClusterAccelerationStructureMoveObjectsInputNV()
    : sType(VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_MOVE_OBJECTS_INPUT_NV),
      pNext(nullptr),
      type(),
      noMoveOverlap(),
      maxMovedBytes() {}

safe_VkClusterAccelerationStructureMoveObjectsInputNV::safe_VkClusterAccelerationStructureMoveObjectsInputNV(
    const safe_VkClusterAccelerationStructureMoveObjectsInputNV& copy_src) {
    sType = copy_src.sType;
    type = copy_src.type;
    noMoveOverlap = copy_src.noMoveOverlap;
    maxMovedBytes = copy_src.maxMovedBytes;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkClusterAccelerationStructureMoveObjectsInputNV& safe_VkClusterAccelerationStructureMoveObjectsInputNV::operator=(
    const safe_VkClusterAccelerationStructureMoveObjectsInputNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    type = copy_src.type;
    noMoveOverlap = copy_src.noMoveOverlap;
    maxMovedBytes = copy_src.maxMovedBytes;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkClusterAccelerationStructureMoveObjectsInputNV::~safe_VkClusterAccelerationStructureMoveObjectsInputNV() {
    FreePnextChain(pNext);
}

void safe_VkClusterAccelerationStructureMoveObjectsInputNV::initialize(
    const VkClusterAccelerationStructureMoveObjectsInputNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    type = in_struct->type;
    noMoveOverlap = in_struct->noMoveOverlap;
    maxMovedBytes = in_struct->maxMovedBytes;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkClusterAccelerationStructureMoveObjectsInputNV::initialize(
    const safe_VkClusterAccelerationStructureMoveObjectsInputNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    type = copy_src->type;
    noMoveOverlap = copy_src->noMoveOverlap;
    maxMovedBytes = copy_src->maxMovedBytes;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkClusterAccelerationStructureOpInputNV::safe_VkClusterAccelerationStructureOpInputNV(
    const VkClusterAccelerationStructureOpInputNV* in_struct, PNextCopyState*) {
    initialize(in_struct);
}

safe_VkClusterAccelerationStructureOpInputNV::safe_VkClusterAccelerationStructureOpInputNV() : pClustersBottomLevel(nullptr) {}

safe_VkClusterAccelerationStructureOpInputNV::safe_VkClusterAccelerationStructureOpInputNV(
    const safe_VkClusterAccelerationStructureOpInputNV& copy_src) {
    pClustersBottomLevel = nullptr;
    pTriangleClusters = nullptr;
    pMoveObjects = nullptr;
    if (copy_src.pClustersBottomLevel)
        pClustersBottomLevel = new safe_VkClusterAccelerationStructureClustersBottomLevelInputNV(*copy_src.pClustersBottomLevel);
    if (copy_src.pTriangleClusters)
        pTriangleClusters = new safe_VkClusterAccelerationStructureTriangleClusterInputNV(*copy_src.pTriangleClusters);
    if (copy_src.pMoveObjects) pMoveObjects = new safe_VkClusterAccelerationStructureMoveObjectsInputNV(*copy_src.pMoveObjects);
}

safe_VkClusterAccelerationStructureOpInputNV& safe_VkClusterAccelerationStructureOpInputNV::operator=(
    const safe_VkClusterAccelerationStructureOpInputNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pClustersBottomLevel) delete pClustersBottomLevel;
    if (pTriangleClusters) delete pTriangleClusters;
    if (pMoveObjects) delete pMoveObjects;

    pClustersBottomLevel = nullptr;
    pTriangleClusters = nullptr;
    pMoveObjects = nullptr;
    if (copy_src.pClustersBottomLevel)
        pClustersBottomLevel = new safe_VkClusterAccelerationStructureClustersBottomLevelInputNV(*copy_src.pClustersBottomLevel);
    if (copy_src.pTriangleClusters)
        pTriangleClusters = new safe_VkClusterAccelerationStructureTriangleClusterInputNV(*copy_src.pTriangleClusters);
    if (copy_src.pMoveObjects) pMoveObjects = new safe_VkClusterAccelerationStructureMoveObjectsInputNV(*copy_src.pMoveObjects);

    return *this;
}

safe_VkClusterAccelerationStructureOpInputNV::~safe_VkClusterAccelerationStructureOpInputNV() {
    if (pClustersBottomLevel) delete pClustersBottomLevel;
    if (pTriangleClusters) delete pTriangleClusters;
    if (pMoveObjects) delete pMoveObjects;
}

void safe_VkClusterAccelerationStructureOpInputNV::initialize(const VkClusterAccelerationStructureOpInputNV* in_struct,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    if (pClustersBottomLevel) delete pClustersBottomLevel;
    if (pTriangleClusters) delete pTriangleClusters;
    if (pMoveObjects) delete pMoveObjects;
    pClustersBottomLevel = nullptr;
    pTriangleClusters = nullptr;
    pMoveObjects = nullptr;
    if (in_struct->pClustersBottomLevel)
        pClustersBottomLevel = new safe_VkClusterAccelerationStructureClustersBottomLevelInputNV(in_struct->pClustersBottomLevel);
    if (in_struct->pTriangleClusters)
        pTriangleClusters = new safe_VkClusterAccelerationStructureTriangleClusterInputNV(in_struct->pTriangleClusters);
    if (in_struct->pMoveObjects) pMoveObjects = new safe_VkClusterAccelerationStructureMoveObjectsInputNV(in_struct->pMoveObjects);
}

void safe_VkClusterAccelerationStructureOpInputNV::initialize(const safe_VkClusterAccelerationStructureOpInputNV* copy_src,
                                                              [[maybe_unused]] PNextCopyState* copy_state) {
    pClustersBottomLevel = nullptr;
    pTriangleClusters = nullptr;
    pMoveObjects = nullptr;
    if (copy_src->pClustersBottomLevel)
        pClustersBottomLevel = new safe_VkClusterAccelerationStructureClustersBottomLevelInputNV(*copy_src->pClustersBottomLevel);
    if (copy_src->pTriangleClusters)
        pTriangleClusters = new safe_VkClusterAccelerationStructureTriangleClusterInputNV(*copy_src->pTriangleClusters);
    if (copy_src->pMoveObjects) pMoveObjects = new safe_VkClusterAccelerationStructureMoveObjectsInputNV(*copy_src->pMoveObjects);
}

safe_VkClusterAccelerationStructureInputInfoNV::safe_VkClusterAccelerationStructureInputInfoNV(
    const VkClusterAccelerationStructureInputInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      maxAccelerationStructureCount(in_struct->maxAccelerationStructureCount),
      flags(in_struct->flags),
      opType(in_struct->opType),
      opMode(in_struct->opMode),
      opInput(&in_struct->opInput) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkClusterAccelerationStructureInputInfoNV::safe_VkClusterAccelerationStructureInputInfoNV()
    : sType(VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_INPUT_INFO_NV),
      pNext(nullptr),
      maxAccelerationStructureCount(),
      flags(),
      opType(),
      opMode() {}

safe_VkClusterAccelerationStructureInputInfoNV::safe_VkClusterAccelerationStructureInputInfoNV(
    const safe_VkClusterAccelerationStructureInputInfoNV& copy_src) {
    sType = copy_src.sType;
    maxAccelerationStructureCount = copy_src.maxAccelerationStructureCount;
    flags = copy_src.flags;
    opType = copy_src.opType;
    opMode = copy_src.opMode;
    opInput.initialize(&copy_src.opInput);
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkClusterAccelerationStructureInputInfoNV& safe_VkClusterAccelerationStructureInputInfoNV::operator=(
    const safe_VkClusterAccelerationStructureInputInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxAccelerationStructureCount = copy_src.maxAccelerationStructureCount;
    flags = copy_src.flags;
    opType = copy_src.opType;
    opMode = copy_src.opMode;
    opInput.initialize(&copy_src.opInput);
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkClusterAccelerationStructureInputInfoNV::~safe_VkClusterAccelerationStructureInputInfoNV() { FreePnextChain(pNext); }

void safe_VkClusterAccelerationStructureInputInfoNV::initialize(const VkClusterAccelerationStructureInputInfoNV* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxAccelerationStructureCount = in_struct->maxAccelerationStructureCount;
    flags = in_struct->flags;
    opType = in_struct->opType;
    opMode = in_struct->opMode;
    opInput.initialize(&in_struct->opInput);
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkClusterAccelerationStructureInputInfoNV::initialize(const safe_VkClusterAccelerationStructureInputInfoNV* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxAccelerationStructureCount = copy_src->maxAccelerationStructureCount;
    flags = copy_src->flags;
    opType = copy_src->opType;
    opMode = copy_src->opMode;
    opInput.initialize(&copy_src->opInput);
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkClusterAccelerationStructureCommandsInfoNV::safe_VkClusterAccelerationStructureCommandsInfoNV(
    const VkClusterAccelerationStructureCommandsInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      input(&in_struct->input),
      dstImplicitData(in_struct->dstImplicitData),
      scratchData(in_struct->scratchData),
      dstAddressesArray(in_struct->dstAddressesArray),
      dstSizesArray(in_struct->dstSizesArray),
      srcInfosArray(in_struct->srcInfosArray),
      srcInfosCount(in_struct->srcInfosCount),
      addressResolutionFlags(in_struct->addressResolutionFlags) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkClusterAccelerationStructureCommandsInfoNV::safe_VkClusterAccelerationStructureCommandsInfoNV()
    : sType(VK_STRUCTURE_TYPE_CLUSTER_ACCELERATION_STRUCTURE_COMMANDS_INFO_NV),
      pNext(nullptr),
      dstImplicitData(),
      scratchData(),
      dstAddressesArray(),
      dstSizesArray(),
      srcInfosArray(),
      srcInfosCount(),
      addressResolutionFlags() {}

safe_VkClusterAccelerationStructureCommandsInfoNV::safe_VkClusterAccelerationStructureCommandsInfoNV(
    const safe_VkClusterAccelerationStructureCommandsInfoNV& copy_src) {
    sType = copy_src.sType;
    input.initialize(&copy_src.input);
    dstImplicitData = copy_src.dstImplicitData;
    scratchData = copy_src.scratchData;
    dstAddressesArray = copy_src.dstAddressesArray;
    dstSizesArray = copy_src.dstSizesArray;
    srcInfosArray = copy_src.srcInfosArray;
    srcInfosCount = copy_src.srcInfosCount;
    addressResolutionFlags = copy_src.addressResolutionFlags;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkClusterAccelerationStructureCommandsInfoNV& safe_VkClusterAccelerationStructureCommandsInfoNV::operator=(
    const safe_VkClusterAccelerationStructureCommandsInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    input.initialize(&copy_src.input);
    dstImplicitData = copy_src.dstImplicitData;
    scratchData = copy_src.scratchData;
    dstAddressesArray = copy_src.dstAddressesArray;
    dstSizesArray = copy_src.dstSizesArray;
    srcInfosArray = copy_src.srcInfosArray;
    srcInfosCount = copy_src.srcInfosCount;
    addressResolutionFlags = copy_src.addressResolutionFlags;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkClusterAccelerationStructureCommandsInfoNV::~safe_VkClusterAccelerationStructureCommandsInfoNV() { FreePnextChain(pNext); }

void safe_VkClusterAccelerationStructureCommandsInfoNV::initialize(const VkClusterAccelerationStructureCommandsInfoNV* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    input.initialize(&in_struct->input);
    dstImplicitData = in_struct->dstImplicitData;
    scratchData = in_struct->scratchData;
    dstAddressesArray = in_struct->dstAddressesArray;
    dstSizesArray = in_struct->dstSizesArray;
    srcInfosArray = in_struct->srcInfosArray;
    srcInfosCount = in_struct->srcInfosCount;
    addressResolutionFlags = in_struct->addressResolutionFlags;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkClusterAccelerationStructureCommandsInfoNV::initialize(
    const safe_VkClusterAccelerationStructureCommandsInfoNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    input.initialize(&copy_src->input);
    dstImplicitData = copy_src->dstImplicitData;
    scratchData = copy_src->scratchData;
    dstAddressesArray = copy_src->dstAddressesArray;
    dstSizesArray = copy_src->dstSizesArray;
    srcInfosArray = copy_src->srcInfosArray;
    srcInfosCount = copy_src->srcInfosCount;
    addressResolutionFlags = copy_src->addressResolutionFlags;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV::
    safe_VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV(
        const VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
        bool copy_pnext)
    : sType(in_struct->sType), allowClusterAccelerationStructure(in_struct->allowClusterAccelerationStructure) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV::
    safe_VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV()
    : sType(VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CLUSTER_ACCELERATION_STRUCTURE_CREATE_INFO_NV),
      pNext(nullptr),
      allowClusterAccelerationStructure() {}

safe_VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV::
    safe_VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV(
        const safe_VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV& copy_src) {
    sType = copy_src.sType;
    allowClusterAccelerationStructure = copy_src.allowClusterAccelerationStructure;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV&
safe_VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV::operator=(
    const safe_VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    allowClusterAccelerationStructure = copy_src.allowClusterAccelerationStructure;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV::
    ~safe_VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV() {
    FreePnextChain(pNext);
}

void safe_VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV::initialize(
    const VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    allowClusterAccelerationStructure = in_struct->allowClusterAccelerationStructure;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV::initialize(
    const safe_VkRayTracingPipelineClusterAccelerationStructureCreateInfoNV* copy_src,
    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    allowClusterAccelerationStructure = copy_src->allowClusterAccelerationStructure;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV::safe_VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV(
    const VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), partitionedAccelerationStructure(in_struct->partitionedAccelerationStructure) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV::safe_VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_FEATURES_NV),
      pNext(nullptr),
      partitionedAccelerationStructure() {}

safe_VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV::safe_VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV(
    const safe_VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV& copy_src) {
    sType = copy_src.sType;
    partitionedAccelerationStructure = copy_src.partitionedAccelerationStructure;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV&
safe_VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV::operator=(
    const safe_VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    partitionedAccelerationStructure = copy_src.partitionedAccelerationStructure;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV::
    ~safe_VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV::initialize(
    const VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    partitionedAccelerationStructure = in_struct->partitionedAccelerationStructure;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV::initialize(
    const safe_VkPhysicalDevicePartitionedAccelerationStructureFeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    partitionedAccelerationStructure = copy_src->partitionedAccelerationStructure;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV::
    safe_VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV(
        const VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
        bool copy_pnext)
    : sType(in_struct->sType), maxPartitionCount(in_struct->maxPartitionCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV::
    safe_VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PARTITIONED_ACCELERATION_STRUCTURE_PROPERTIES_NV),
      pNext(nullptr),
      maxPartitionCount() {}

safe_VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV::
    safe_VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV(
        const safe_VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV& copy_src) {
    sType = copy_src.sType;
    maxPartitionCount = copy_src.maxPartitionCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV&
safe_VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV::operator=(
    const safe_VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maxPartitionCount = copy_src.maxPartitionCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV::
    ~safe_VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV::initialize(
    const VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maxPartitionCount = in_struct->maxPartitionCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV::initialize(
    const safe_VkPhysicalDevicePartitionedAccelerationStructurePropertiesNV* copy_src,
    [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maxPartitionCount = copy_src->maxPartitionCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPartitionedAccelerationStructureFlagsNV::safe_VkPartitionedAccelerationStructureFlagsNV(
    const VkPartitionedAccelerationStructureFlagsNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), enablePartitionTranslation(in_struct->enablePartitionTranslation) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPartitionedAccelerationStructureFlagsNV::safe_VkPartitionedAccelerationStructureFlagsNV()
    : sType(VK_STRUCTURE_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_FLAGS_NV), pNext(nullptr), enablePartitionTranslation() {}

safe_VkPartitionedAccelerationStructureFlagsNV::safe_VkPartitionedAccelerationStructureFlagsNV(
    const safe_VkPartitionedAccelerationStructureFlagsNV& copy_src) {
    sType = copy_src.sType;
    enablePartitionTranslation = copy_src.enablePartitionTranslation;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPartitionedAccelerationStructureFlagsNV& safe_VkPartitionedAccelerationStructureFlagsNV::operator=(
    const safe_VkPartitionedAccelerationStructureFlagsNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    enablePartitionTranslation = copy_src.enablePartitionTranslation;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPartitionedAccelerationStructureFlagsNV::~safe_VkPartitionedAccelerationStructureFlagsNV() { FreePnextChain(pNext); }

void safe_VkPartitionedAccelerationStructureFlagsNV::initialize(const VkPartitionedAccelerationStructureFlagsNV* in_struct,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    enablePartitionTranslation = in_struct->enablePartitionTranslation;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPartitionedAccelerationStructureFlagsNV::initialize(const safe_VkPartitionedAccelerationStructureFlagsNV* copy_src,
                                                                [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    enablePartitionTranslation = copy_src->enablePartitionTranslation;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkWriteDescriptorSetPartitionedAccelerationStructureNV::safe_VkWriteDescriptorSetPartitionedAccelerationStructureNV(
    const VkWriteDescriptorSetPartitionedAccelerationStructureNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), accelerationStructureCount(in_struct->accelerationStructureCount), pAccelerationStructures(nullptr) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
    if (in_struct->pAccelerationStructures) {
        pAccelerationStructures = new VkDeviceAddress[in_struct->accelerationStructureCount];
        memcpy((void*)pAccelerationStructures, (void*)in_struct->pAccelerationStructures,
               sizeof(VkDeviceAddress) * in_struct->accelerationStructureCount);
    }
}

safe_VkWriteDescriptorSetPartitionedAccelerationStructureNV::safe_VkWriteDescriptorSetPartitionedAccelerationStructureNV()
    : sType(VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_PARTITIONED_ACCELERATION_STRUCTURE_NV),
      pNext(nullptr),
      accelerationStructureCount(),
      pAccelerationStructures(nullptr) {}

safe_VkWriteDescriptorSetPartitionedAccelerationStructureNV::safe_VkWriteDescriptorSetPartitionedAccelerationStructureNV(
    const safe_VkWriteDescriptorSetPartitionedAccelerationStructureNV& copy_src) {
    sType = copy_src.sType;
    accelerationStructureCount = copy_src.accelerationStructureCount;
    pAccelerationStructures = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pAccelerationStructures) {
        pAccelerationStructures = new VkDeviceAddress[copy_src.accelerationStructureCount];
        memcpy((void*)pAccelerationStructures, (void*)copy_src.pAccelerationStructures,
               sizeof(VkDeviceAddress) * copy_src.accelerationStructureCount);
    }
}

safe_VkWriteDescriptorSetPartitionedAccelerationStructureNV& safe_VkWriteDescriptorSetPartitionedAccelerationStructureNV::operator=(
    const safe_VkWriteDescriptorSetPartitionedAccelerationStructureNV& copy_src) {
    if (&copy_src == this) return *this;

    if (pAccelerationStructures) delete[] pAccelerationStructures;
    FreePnextChain(pNext);

    sType = copy_src.sType;
    accelerationStructureCount = copy_src.accelerationStructureCount;
    pAccelerationStructures = nullptr;
    pNext = SafePnextCopy(copy_src.pNext);

    if (copy_src.pAccelerationStructures) {
        pAccelerationStructures = new VkDeviceAddress[copy_src.accelerationStructureCount];
        memcpy((void*)pAccelerationStructures, (void*)copy_src.pAccelerationStructures,
               sizeof(VkDeviceAddress) * copy_src.accelerationStructureCount);
    }

    return *this;
}

safe_VkWriteDescriptorSetPartitionedAccelerationStructureNV::~safe_VkWriteDescriptorSetPartitionedAccelerationStructureNV() {
    if (pAccelerationStructures) delete[] pAccelerationStructures;
    FreePnextChain(pNext);
}

void safe_VkWriteDescriptorSetPartitionedAccelerationStructureNV::initialize(
    const VkWriteDescriptorSetPartitionedAccelerationStructureNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    if (pAccelerationStructures) delete[] pAccelerationStructures;
    FreePnextChain(pNext);
    sType = in_struct->sType;
    accelerationStructureCount = in_struct->accelerationStructureCount;
    pAccelerationStructures = nullptr;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);

    if (in_struct->pAccelerationStructures) {
        pAccelerationStructures = new VkDeviceAddress[in_struct->accelerationStructureCount];
        memcpy((void*)pAccelerationStructures, (void*)in_struct->pAccelerationStructures,
               sizeof(VkDeviceAddress) * in_struct->accelerationStructureCount);
    }
}

void safe_VkWriteDescriptorSetPartitionedAccelerationStructureNV::initialize(
    const safe_VkWriteDescriptorSetPartitionedAccelerationStructureNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    accelerationStructureCount = copy_src->accelerationStructureCount;
    pAccelerationStructures = nullptr;
    pNext = SafePnextCopy(copy_src->pNext);

    if (copy_src->pAccelerationStructures) {
        pAccelerationStructures = new VkDeviceAddress[copy_src->accelerationStructureCount];
        memcpy((void*)pAccelerationStructures, (void*)copy_src->pAccelerationStructures,
               sizeof(VkDeviceAddress) * copy_src->accelerationStructureCount);
    }
}

safe_VkPartitionedAccelerationStructureInstancesInputNV::safe_VkPartitionedAccelerationStructureInstancesInputNV(
    const VkPartitionedAccelerationStructureInstancesInputNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      flags(in_struct->flags),
      instanceCount(in_struct->instanceCount),
      maxInstancePerPartitionCount(in_struct->maxInstancePerPartitionCount),
      partitionCount(in_struct->partitionCount),
      maxInstanceInGlobalPartitionCount(in_struct->maxInstanceInGlobalPartitionCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPartitionedAccelerationStructureInstancesInputNV::safe_VkPartitionedAccelerationStructureInstancesInputNV()
    : sType(VK_STRUCTURE_TYPE_PARTITIONED_ACCELERATION_STRUCTURE_INSTANCES_INPUT_NV),
      pNext(nullptr),
      flags(),
      instanceCount(),
      maxInstancePerPartitionCount(),
      partitionCount(),
      maxInstanceInGlobalPartitionCount() {}

safe_VkPartitionedAccelerationStructureInstancesInputNV::safe_VkPartitionedAccelerationStructureInstancesInputNV(
    const safe_VkPartitionedAccelerationStructureInstancesInputNV& copy_src) {
    sType = copy_src.sType;
    flags = copy_src.flags;
    instanceCount = copy_src.instanceCount;
    maxInstancePerPartitionCount = copy_src.maxInstancePerPartitionCount;
    partitionCount = copy_src.partitionCount;
    maxInstanceInGlobalPartitionCount = copy_src.maxInstanceInGlobalPartitionCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPartitionedAccelerationStructureInstancesInputNV& safe_VkPartitionedAccelerationStructureInstancesInputNV::operator=(
    const safe_VkPartitionedAccelerationStructureInstancesInputNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    flags = copy_src.flags;
    instanceCount = copy_src.instanceCount;
    maxInstancePerPartitionCount = copy_src.maxInstancePerPartitionCount;
    partitionCount = copy_src.partitionCount;
    maxInstanceInGlobalPartitionCount = copy_src.maxInstanceInGlobalPartitionCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPartitionedAccelerationStructureInstancesInputNV::~safe_VkPartitionedAccelerationStructureInstancesInputNV() {
    FreePnextChain(pNext);
}

void safe_VkPartitionedAccelerationStructureInstancesInputNV::initialize(
    const VkPartitionedAccelerationStructureInstancesInputNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    flags = in_struct->flags;
    instanceCount = in_struct->instanceCount;
    maxInstancePerPartitionCount = in_struct->maxInstancePerPartitionCount;
    partitionCount = in_struct->partitionCount;
    maxInstanceInGlobalPartitionCount = in_struct->maxInstanceInGlobalPartitionCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPartitionedAccelerationStructureInstancesInputNV::initialize(
    const safe_VkPartitionedAccelerationStructureInstancesInputNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    flags = copy_src->flags;
    instanceCount = copy_src->instanceCount;
    maxInstancePerPartitionCount = copy_src->maxInstancePerPartitionCount;
    partitionCount = copy_src->partitionCount;
    maxInstanceInGlobalPartitionCount = copy_src->maxInstanceInGlobalPartitionCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkBuildPartitionedAccelerationStructureInfoNV::safe_VkBuildPartitionedAccelerationStructureInfoNV(
    const VkBuildPartitionedAccelerationStructureInfoNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      input(&in_struct->input),
      srcAccelerationStructureData(in_struct->srcAccelerationStructureData),
      dstAccelerationStructureData(in_struct->dstAccelerationStructureData),
      scratchData(in_struct->scratchData),
      srcInfos(in_struct->srcInfos),
      srcInfosCount(in_struct->srcInfosCount) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkBuildPartitionedAccelerationStructureInfoNV::safe_VkBuildPartitionedAccelerationStructureInfoNV()
    : sType(VK_STRUCTURE_TYPE_BUILD_PARTITIONED_ACCELERATION_STRUCTURE_INFO_NV),
      pNext(nullptr),
      srcAccelerationStructureData(),
      dstAccelerationStructureData(),
      scratchData(),
      srcInfos(),
      srcInfosCount() {}

safe_VkBuildPartitionedAccelerationStructureInfoNV::safe_VkBuildPartitionedAccelerationStructureInfoNV(
    const safe_VkBuildPartitionedAccelerationStructureInfoNV& copy_src) {
    sType = copy_src.sType;
    input.initialize(&copy_src.input);
    srcAccelerationStructureData = copy_src.srcAccelerationStructureData;
    dstAccelerationStructureData = copy_src.dstAccelerationStructureData;
    scratchData = copy_src.scratchData;
    srcInfos = copy_src.srcInfos;
    srcInfosCount = copy_src.srcInfosCount;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkBuildPartitionedAccelerationStructureInfoNV& safe_VkBuildPartitionedAccelerationStructureInfoNV::operator=(
    const safe_VkBuildPartitionedAccelerationStructureInfoNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    input.initialize(&copy_src.input);
    srcAccelerationStructureData = copy_src.srcAccelerationStructureData;
    dstAccelerationStructureData = copy_src.dstAccelerationStructureData;
    scratchData = copy_src.scratchData;
    srcInfos = copy_src.srcInfos;
    srcInfosCount = copy_src.srcInfosCount;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkBuildPartitionedAccelerationStructureInfoNV::~safe_VkBuildPartitionedAccelerationStructureInfoNV() { FreePnextChain(pNext); }

void safe_VkBuildPartitionedAccelerationStructureInfoNV::initialize(const VkBuildPartitionedAccelerationStructureInfoNV* in_struct,
                                                                    [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    input.initialize(&in_struct->input);
    srcAccelerationStructureData = in_struct->srcAccelerationStructureData;
    dstAccelerationStructureData = in_struct->dstAccelerationStructureData;
    scratchData = in_struct->scratchData;
    srcInfos = in_struct->srcInfos;
    srcInfosCount = in_struct->srcInfosCount;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkBuildPartitionedAccelerationStructureInfoNV::initialize(
    const safe_VkBuildPartitionedAccelerationStructureInfoNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    input.initialize(&copy_src->input);
    srcAccelerationStructureData = copy_src->srcAccelerationStructureData;
    dstAccelerationStructureData = copy_src->dstAccelerationStructureData;
    scratchData = copy_src->scratchData;
    srcInfos = copy_src->srcInfos;
    srcInfosCount = copy_src->srcInfosCount;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceImageAlignmentControlFeaturesMESA::safe_VkPhysicalDeviceImageAlignmentControlFeaturesMESA(
    const VkPhysicalDeviceImageAlignmentControlFeaturesMESA* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), imageAlignmentControl(in_struct->imageAlignmentControl) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceImageAlignmentControlFeaturesMESA::safe_VkPhysicalDeviceImageAlignmentControlFeaturesMESA()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_FEATURES_MESA), pNext(nullptr), imageAlignmentControl() {}

safe_VkPhysicalDeviceImageAlignmentControlFeaturesMESA::safe_VkPhysicalDeviceImageAlignmentControlFeaturesMESA(
    const safe_VkPhysicalDeviceImageAlignmentControlFeaturesMESA& copy_src) {
    sType = copy_src.sType;
    imageAlignmentControl = copy_src.imageAlignmentControl;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceImageAlignmentControlFeaturesMESA& safe_VkPhysicalDeviceImageAlignmentControlFeaturesMESA::operator=(
    const safe_VkPhysicalDeviceImageAlignmentControlFeaturesMESA& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    imageAlignmentControl = copy_src.imageAlignmentControl;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceImageAlignmentControlFeaturesMESA::~safe_VkPhysicalDeviceImageAlignmentControlFeaturesMESA() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceImageAlignmentControlFeaturesMESA::initialize(
    const VkPhysicalDeviceImageAlignmentControlFeaturesMESA* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    imageAlignmentControl = in_struct->imageAlignmentControl;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceImageAlignmentControlFeaturesMESA::initialize(
    const safe_VkPhysicalDeviceImageAlignmentControlFeaturesMESA* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    imageAlignmentControl = copy_src->imageAlignmentControl;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceImageAlignmentControlPropertiesMESA::safe_VkPhysicalDeviceImageAlignmentControlPropertiesMESA(
    const VkPhysicalDeviceImageAlignmentControlPropertiesMESA* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), supportedImageAlignmentMask(in_struct->supportedImageAlignmentMask) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceImageAlignmentControlPropertiesMESA::safe_VkPhysicalDeviceImageAlignmentControlPropertiesMESA()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ALIGNMENT_CONTROL_PROPERTIES_MESA),
      pNext(nullptr),
      supportedImageAlignmentMask() {}

safe_VkPhysicalDeviceImageAlignmentControlPropertiesMESA::safe_VkPhysicalDeviceImageAlignmentControlPropertiesMESA(
    const safe_VkPhysicalDeviceImageAlignmentControlPropertiesMESA& copy_src) {
    sType = copy_src.sType;
    supportedImageAlignmentMask = copy_src.supportedImageAlignmentMask;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceImageAlignmentControlPropertiesMESA& safe_VkPhysicalDeviceImageAlignmentControlPropertiesMESA::operator=(
    const safe_VkPhysicalDeviceImageAlignmentControlPropertiesMESA& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    supportedImageAlignmentMask = copy_src.supportedImageAlignmentMask;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceImageAlignmentControlPropertiesMESA::~safe_VkPhysicalDeviceImageAlignmentControlPropertiesMESA() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceImageAlignmentControlPropertiesMESA::initialize(
    const VkPhysicalDeviceImageAlignmentControlPropertiesMESA* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    supportedImageAlignmentMask = in_struct->supportedImageAlignmentMask;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceImageAlignmentControlPropertiesMESA::initialize(
    const safe_VkPhysicalDeviceImageAlignmentControlPropertiesMESA* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    supportedImageAlignmentMask = copy_src->supportedImageAlignmentMask;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkImageAlignmentControlCreateInfoMESA::safe_VkImageAlignmentControlCreateInfoMESA(
    const VkImageAlignmentControlCreateInfoMESA* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), maximumRequestedAlignment(in_struct->maximumRequestedAlignment) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkImageAlignmentControlCreateInfoMESA::safe_VkImageAlignmentControlCreateInfoMESA()
    : sType(VK_STRUCTURE_TYPE_IMAGE_ALIGNMENT_CONTROL_CREATE_INFO_MESA), pNext(nullptr), maximumRequestedAlignment() {}

safe_VkImageAlignmentControlCreateInfoMESA::safe_VkImageAlignmentControlCreateInfoMESA(
    const safe_VkImageAlignmentControlCreateInfoMESA& copy_src) {
    sType = copy_src.sType;
    maximumRequestedAlignment = copy_src.maximumRequestedAlignment;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkImageAlignmentControlCreateInfoMESA& safe_VkImageAlignmentControlCreateInfoMESA::operator=(
    const safe_VkImageAlignmentControlCreateInfoMESA& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    maximumRequestedAlignment = copy_src.maximumRequestedAlignment;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkImageAlignmentControlCreateInfoMESA::~safe_VkImageAlignmentControlCreateInfoMESA() { FreePnextChain(pNext); }

void safe_VkImageAlignmentControlCreateInfoMESA::initialize(const VkImageAlignmentControlCreateInfoMESA* in_struct,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    maximumRequestedAlignment = in_struct->maximumRequestedAlignment;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkImageAlignmentControlCreateInfoMESA::initialize(const safe_VkImageAlignmentControlCreateInfoMESA* copy_src,
                                                            [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    maximumRequestedAlignment = copy_src->maximumRequestedAlignment;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceHdrVividFeaturesHUAWEI::safe_VkPhysicalDeviceHdrVividFeaturesHUAWEI(
    const VkPhysicalDeviceHdrVividFeaturesHUAWEI* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType), hdrVivid(in_struct->hdrVivid) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceHdrVividFeaturesHUAWEI::safe_VkPhysicalDeviceHdrVividFeaturesHUAWEI()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HDR_VIVID_FEATURES_HUAWEI), pNext(nullptr), hdrVivid() {}

safe_VkPhysicalDeviceHdrVividFeaturesHUAWEI::safe_VkPhysicalDeviceHdrVividFeaturesHUAWEI(
    const safe_VkPhysicalDeviceHdrVividFeaturesHUAWEI& copy_src) {
    sType = copy_src.sType;
    hdrVivid = copy_src.hdrVivid;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceHdrVividFeaturesHUAWEI& safe_VkPhysicalDeviceHdrVividFeaturesHUAWEI::operator=(
    const safe_VkPhysicalDeviceHdrVividFeaturesHUAWEI& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    hdrVivid = copy_src.hdrVivid;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceHdrVividFeaturesHUAWEI::~safe_VkPhysicalDeviceHdrVividFeaturesHUAWEI() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceHdrVividFeaturesHUAWEI::initialize(const VkPhysicalDeviceHdrVividFeaturesHUAWEI* in_struct,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    hdrVivid = in_struct->hdrVivid;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceHdrVividFeaturesHUAWEI::initialize(const safe_VkPhysicalDeviceHdrVividFeaturesHUAWEI* copy_src,
                                                             [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    hdrVivid = copy_src->hdrVivid;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkHdrVividDynamicMetadataHUAWEI::safe_VkHdrVividDynamicMetadataHUAWEI(const VkHdrVividDynamicMetadataHUAWEI* in_struct,
                                                                           [[maybe_unused]] PNextCopyState* copy_state,
                                                                           bool copy_pnext)
    : sType(in_struct->sType), dynamicMetadataSize(in_struct->dynamicMetadataSize), pDynamicMetadata(in_struct->pDynamicMetadata) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkHdrVividDynamicMetadataHUAWEI::safe_VkHdrVividDynamicMetadataHUAWEI()
    : sType(VK_STRUCTURE_TYPE_HDR_VIVID_DYNAMIC_METADATA_HUAWEI),
      pNext(nullptr),
      dynamicMetadataSize(),
      pDynamicMetadata(nullptr) {}

safe_VkHdrVividDynamicMetadataHUAWEI::safe_VkHdrVividDynamicMetadataHUAWEI(const safe_VkHdrVividDynamicMetadataHUAWEI& copy_src) {
    sType = copy_src.sType;
    dynamicMetadataSize = copy_src.dynamicMetadataSize;
    pDynamicMetadata = copy_src.pDynamicMetadata;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkHdrVividDynamicMetadataHUAWEI& safe_VkHdrVividDynamicMetadataHUAWEI::operator=(
    const safe_VkHdrVividDynamicMetadataHUAWEI& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    dynamicMetadataSize = copy_src.dynamicMetadataSize;
    pDynamicMetadata = copy_src.pDynamicMetadata;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkHdrVividDynamicMetadataHUAWEI::~safe_VkHdrVividDynamicMetadataHUAWEI() { FreePnextChain(pNext); }

void safe_VkHdrVividDynamicMetadataHUAWEI::initialize(const VkHdrVividDynamicMetadataHUAWEI* in_struct,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    dynamicMetadataSize = in_struct->dynamicMetadataSize;
    pDynamicMetadata = in_struct->pDynamicMetadata;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkHdrVividDynamicMetadataHUAWEI::initialize(const safe_VkHdrVividDynamicMetadataHUAWEI* copy_src,
                                                      [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    dynamicMetadataSize = copy_src->dynamicMetadataSize;
    pDynamicMetadata = copy_src->pDynamicMetadata;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkCooperativeMatrixFlexibleDimensionsPropertiesNV::safe_VkCooperativeMatrixFlexibleDimensionsPropertiesNV(
    const VkCooperativeMatrixFlexibleDimensionsPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType),
      MGranularity(in_struct->MGranularity),
      NGranularity(in_struct->NGranularity),
      KGranularity(in_struct->KGranularity),
      AType(in_struct->AType),
      BType(in_struct->BType),
      CType(in_struct->CType),
      ResultType(in_struct->ResultType),
      saturatingAccumulation(in_struct->saturatingAccumulation),
      scope(in_struct->scope),
      workgroupInvocations(in_struct->workgroupInvocations) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkCooperativeMatrixFlexibleDimensionsPropertiesNV::safe_VkCooperativeMatrixFlexibleDimensionsPropertiesNV()
    : sType(VK_STRUCTURE_TYPE_COOPERATIVE_MATRIX_FLEXIBLE_DIMENSIONS_PROPERTIES_NV),
      pNext(nullptr),
      MGranularity(),
      NGranularity(),
      KGranularity(),
      AType(),
      BType(),
      CType(),
      ResultType(),
      saturatingAccumulation(),
      scope(),
      workgroupInvocations() {}

safe_VkCooperativeMatrixFlexibleDimensionsPropertiesNV::safe_VkCooperativeMatrixFlexibleDimensionsPropertiesNV(
    const safe_VkCooperativeMatrixFlexibleDimensionsPropertiesNV& copy_src) {
    sType = copy_src.sType;
    MGranularity = copy_src.MGranularity;
    NGranularity = copy_src.NGranularity;
    KGranularity = copy_src.KGranularity;
    AType = copy_src.AType;
    BType = copy_src.BType;
    CType = copy_src.CType;
    ResultType = copy_src.ResultType;
    saturatingAccumulation = copy_src.saturatingAccumulation;
    scope = copy_src.scope;
    workgroupInvocations = copy_src.workgroupInvocations;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkCooperativeMatrixFlexibleDimensionsPropertiesNV& safe_VkCooperativeMatrixFlexibleDimensionsPropertiesNV::operator=(
    const safe_VkCooperativeMatrixFlexibleDimensionsPropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    MGranularity = copy_src.MGranularity;
    NGranularity = copy_src.NGranularity;
    KGranularity = copy_src.KGranularity;
    AType = copy_src.AType;
    BType = copy_src.BType;
    CType = copy_src.CType;
    ResultType = copy_src.ResultType;
    saturatingAccumulation = copy_src.saturatingAccumulation;
    scope = copy_src.scope;
    workgroupInvocations = copy_src.workgroupInvocations;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkCooperativeMatrixFlexibleDimensionsPropertiesNV::~safe_VkCooperativeMatrixFlexibleDimensionsPropertiesNV() {
    FreePnextChain(pNext);
}

void safe_VkCooperativeMatrixFlexibleDimensionsPropertiesNV::initialize(
    const VkCooperativeMatrixFlexibleDimensionsPropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    MGranularity = in_struct->MGranularity;
    NGranularity = in_struct->NGranularity;
    KGranularity = in_struct->KGranularity;
    AType = in_struct->AType;
    BType = in_struct->BType;
    CType = in_struct->CType;
    ResultType = in_struct->ResultType;
    saturatingAccumulation = in_struct->saturatingAccumulation;
    scope = in_struct->scope;
    workgroupInvocations = in_struct->workgroupInvocations;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkCooperativeMatrixFlexibleDimensionsPropertiesNV::initialize(
    const safe_VkCooperativeMatrixFlexibleDimensionsPropertiesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    MGranularity = copy_src->MGranularity;
    NGranularity = copy_src->NGranularity;
    KGranularity = copy_src->KGranularity;
    AType = copy_src->AType;
    BType = copy_src->BType;
    CType = copy_src->CType;
    ResultType = copy_src->ResultType;
    saturatingAccumulation = copy_src->saturatingAccumulation;
    scope = copy_src->scope;
    workgroupInvocations = copy_src->workgroupInvocations;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCooperativeMatrix2FeaturesNV::safe_VkPhysicalDeviceCooperativeMatrix2FeaturesNV(
    const VkPhysicalDeviceCooperativeMatrix2FeaturesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      cooperativeMatrixWorkgroupScope(in_struct->cooperativeMatrixWorkgroupScope),
      cooperativeMatrixFlexibleDimensions(in_struct->cooperativeMatrixFlexibleDimensions),
      cooperativeMatrixReductions(in_struct->cooperativeMatrixReductions),
      cooperativeMatrixConversions(in_struct->cooperativeMatrixConversions),
      cooperativeMatrixPerElementOperations(in_struct->cooperativeMatrixPerElementOperations),
      cooperativeMatrixTensorAddressing(in_struct->cooperativeMatrixTensorAddressing),
      cooperativeMatrixBlockLoads(in_struct->cooperativeMatrixBlockLoads) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCooperativeMatrix2FeaturesNV::safe_VkPhysicalDeviceCooperativeMatrix2FeaturesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_FEATURES_NV),
      pNext(nullptr),
      cooperativeMatrixWorkgroupScope(),
      cooperativeMatrixFlexibleDimensions(),
      cooperativeMatrixReductions(),
      cooperativeMatrixConversions(),
      cooperativeMatrixPerElementOperations(),
      cooperativeMatrixTensorAddressing(),
      cooperativeMatrixBlockLoads() {}

safe_VkPhysicalDeviceCooperativeMatrix2FeaturesNV::safe_VkPhysicalDeviceCooperativeMatrix2FeaturesNV(
    const safe_VkPhysicalDeviceCooperativeMatrix2FeaturesNV& copy_src) {
    sType = copy_src.sType;
    cooperativeMatrixWorkgroupScope = copy_src.cooperativeMatrixWorkgroupScope;
    cooperativeMatrixFlexibleDimensions = copy_src.cooperativeMatrixFlexibleDimensions;
    cooperativeMatrixReductions = copy_src.cooperativeMatrixReductions;
    cooperativeMatrixConversions = copy_src.cooperativeMatrixConversions;
    cooperativeMatrixPerElementOperations = copy_src.cooperativeMatrixPerElementOperations;
    cooperativeMatrixTensorAddressing = copy_src.cooperativeMatrixTensorAddressing;
    cooperativeMatrixBlockLoads = copy_src.cooperativeMatrixBlockLoads;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCooperativeMatrix2FeaturesNV& safe_VkPhysicalDeviceCooperativeMatrix2FeaturesNV::operator=(
    const safe_VkPhysicalDeviceCooperativeMatrix2FeaturesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    cooperativeMatrixWorkgroupScope = copy_src.cooperativeMatrixWorkgroupScope;
    cooperativeMatrixFlexibleDimensions = copy_src.cooperativeMatrixFlexibleDimensions;
    cooperativeMatrixReductions = copy_src.cooperativeMatrixReductions;
    cooperativeMatrixConversions = copy_src.cooperativeMatrixConversions;
    cooperativeMatrixPerElementOperations = copy_src.cooperativeMatrixPerElementOperations;
    cooperativeMatrixTensorAddressing = copy_src.cooperativeMatrixTensorAddressing;
    cooperativeMatrixBlockLoads = copy_src.cooperativeMatrixBlockLoads;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCooperativeMatrix2FeaturesNV::~safe_VkPhysicalDeviceCooperativeMatrix2FeaturesNV() { FreePnextChain(pNext); }

void safe_VkPhysicalDeviceCooperativeMatrix2FeaturesNV::initialize(const VkPhysicalDeviceCooperativeMatrix2FeaturesNV* in_struct,
                                                                   [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    cooperativeMatrixWorkgroupScope = in_struct->cooperativeMatrixWorkgroupScope;
    cooperativeMatrixFlexibleDimensions = in_struct->cooperativeMatrixFlexibleDimensions;
    cooperativeMatrixReductions = in_struct->cooperativeMatrixReductions;
    cooperativeMatrixConversions = in_struct->cooperativeMatrixConversions;
    cooperativeMatrixPerElementOperations = in_struct->cooperativeMatrixPerElementOperations;
    cooperativeMatrixTensorAddressing = in_struct->cooperativeMatrixTensorAddressing;
    cooperativeMatrixBlockLoads = in_struct->cooperativeMatrixBlockLoads;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCooperativeMatrix2FeaturesNV::initialize(
    const safe_VkPhysicalDeviceCooperativeMatrix2FeaturesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    cooperativeMatrixWorkgroupScope = copy_src->cooperativeMatrixWorkgroupScope;
    cooperativeMatrixFlexibleDimensions = copy_src->cooperativeMatrixFlexibleDimensions;
    cooperativeMatrixReductions = copy_src->cooperativeMatrixReductions;
    cooperativeMatrixConversions = copy_src->cooperativeMatrixConversions;
    cooperativeMatrixPerElementOperations = copy_src->cooperativeMatrixPerElementOperations;
    cooperativeMatrixTensorAddressing = copy_src->cooperativeMatrixTensorAddressing;
    cooperativeMatrixBlockLoads = copy_src->cooperativeMatrixBlockLoads;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDeviceCooperativeMatrix2PropertiesNV::safe_VkPhysicalDeviceCooperativeMatrix2PropertiesNV(
    const VkPhysicalDeviceCooperativeMatrix2PropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state, bool copy_pnext)
    : sType(in_struct->sType),
      cooperativeMatrixWorkgroupScopeMaxWorkgroupSize(in_struct->cooperativeMatrixWorkgroupScopeMaxWorkgroupSize),
      cooperativeMatrixFlexibleDimensionsMaxDimension(in_struct->cooperativeMatrixFlexibleDimensionsMaxDimension),
      cooperativeMatrixWorkgroupScopeReservedSharedMemory(in_struct->cooperativeMatrixWorkgroupScopeReservedSharedMemory) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDeviceCooperativeMatrix2PropertiesNV::safe_VkPhysicalDeviceCooperativeMatrix2PropertiesNV()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_COOPERATIVE_MATRIX_2_PROPERTIES_NV),
      pNext(nullptr),
      cooperativeMatrixWorkgroupScopeMaxWorkgroupSize(),
      cooperativeMatrixFlexibleDimensionsMaxDimension(),
      cooperativeMatrixWorkgroupScopeReservedSharedMemory() {}

safe_VkPhysicalDeviceCooperativeMatrix2PropertiesNV::safe_VkPhysicalDeviceCooperativeMatrix2PropertiesNV(
    const safe_VkPhysicalDeviceCooperativeMatrix2PropertiesNV& copy_src) {
    sType = copy_src.sType;
    cooperativeMatrixWorkgroupScopeMaxWorkgroupSize = copy_src.cooperativeMatrixWorkgroupScopeMaxWorkgroupSize;
    cooperativeMatrixFlexibleDimensionsMaxDimension = copy_src.cooperativeMatrixFlexibleDimensionsMaxDimension;
    cooperativeMatrixWorkgroupScopeReservedSharedMemory = copy_src.cooperativeMatrixWorkgroupScopeReservedSharedMemory;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDeviceCooperativeMatrix2PropertiesNV& safe_VkPhysicalDeviceCooperativeMatrix2PropertiesNV::operator=(
    const safe_VkPhysicalDeviceCooperativeMatrix2PropertiesNV& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    cooperativeMatrixWorkgroupScopeMaxWorkgroupSize = copy_src.cooperativeMatrixWorkgroupScopeMaxWorkgroupSize;
    cooperativeMatrixFlexibleDimensionsMaxDimension = copy_src.cooperativeMatrixFlexibleDimensionsMaxDimension;
    cooperativeMatrixWorkgroupScopeReservedSharedMemory = copy_src.cooperativeMatrixWorkgroupScopeReservedSharedMemory;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDeviceCooperativeMatrix2PropertiesNV::~safe_VkPhysicalDeviceCooperativeMatrix2PropertiesNV() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDeviceCooperativeMatrix2PropertiesNV::initialize(
    const VkPhysicalDeviceCooperativeMatrix2PropertiesNV* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    cooperativeMatrixWorkgroupScopeMaxWorkgroupSize = in_struct->cooperativeMatrixWorkgroupScopeMaxWorkgroupSize;
    cooperativeMatrixFlexibleDimensionsMaxDimension = in_struct->cooperativeMatrixFlexibleDimensionsMaxDimension;
    cooperativeMatrixWorkgroupScopeReservedSharedMemory = in_struct->cooperativeMatrixWorkgroupScopeReservedSharedMemory;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDeviceCooperativeMatrix2PropertiesNV::initialize(
    const safe_VkPhysicalDeviceCooperativeMatrix2PropertiesNV* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    cooperativeMatrixWorkgroupScopeMaxWorkgroupSize = copy_src->cooperativeMatrixWorkgroupScopeMaxWorkgroupSize;
    cooperativeMatrixFlexibleDimensionsMaxDimension = copy_src->cooperativeMatrixFlexibleDimensionsMaxDimension;
    cooperativeMatrixWorkgroupScopeReservedSharedMemory = copy_src->cooperativeMatrixWorkgroupScopeReservedSharedMemory;
    pNext = SafePnextCopy(copy_src->pNext);
}

safe_VkPhysicalDevicePipelineOpacityMicromapFeaturesARM::safe_VkPhysicalDevicePipelineOpacityMicromapFeaturesARM(
    const VkPhysicalDevicePipelineOpacityMicromapFeaturesARM* in_struct, [[maybe_unused]] PNextCopyState* copy_state,
    bool copy_pnext)
    : sType(in_struct->sType), pipelineOpacityMicromap(in_struct->pipelineOpacityMicromap) {
    if (copy_pnext) {
        pNext = SafePnextCopy(in_struct->pNext, copy_state);
    }
}

safe_VkPhysicalDevicePipelineOpacityMicromapFeaturesARM::safe_VkPhysicalDevicePipelineOpacityMicromapFeaturesARM()
    : sType(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_OPACITY_MICROMAP_FEATURES_ARM), pNext(nullptr), pipelineOpacityMicromap() {}

safe_VkPhysicalDevicePipelineOpacityMicromapFeaturesARM::safe_VkPhysicalDevicePipelineOpacityMicromapFeaturesARM(
    const safe_VkPhysicalDevicePipelineOpacityMicromapFeaturesARM& copy_src) {
    sType = copy_src.sType;
    pipelineOpacityMicromap = copy_src.pipelineOpacityMicromap;
    pNext = SafePnextCopy(copy_src.pNext);
}

safe_VkPhysicalDevicePipelineOpacityMicromapFeaturesARM& safe_VkPhysicalDevicePipelineOpacityMicromapFeaturesARM::operator=(
    const safe_VkPhysicalDevicePipelineOpacityMicromapFeaturesARM& copy_src) {
    if (&copy_src == this) return *this;

    FreePnextChain(pNext);

    sType = copy_src.sType;
    pipelineOpacityMicromap = copy_src.pipelineOpacityMicromap;
    pNext = SafePnextCopy(copy_src.pNext);

    return *this;
}

safe_VkPhysicalDevicePipelineOpacityMicromapFeaturesARM::~safe_VkPhysicalDevicePipelineOpacityMicromapFeaturesARM() {
    FreePnextChain(pNext);
}

void safe_VkPhysicalDevicePipelineOpacityMicromapFeaturesARM::initialize(
    const VkPhysicalDevicePipelineOpacityMicromapFeaturesARM* in_struct, [[maybe_unused]] PNextCopyState* copy_state) {
    FreePnextChain(pNext);
    sType = in_struct->sType;
    pipelineOpacityMicromap = in_struct->pipelineOpacityMicromap;
    pNext = SafePnextCopy(in_struct->pNext, copy_state);
}

void safe_VkPhysicalDevicePipelineOpacityMicromapFeaturesARM::initialize(
    const safe_VkPhysicalDevicePipelineOpacityMicromapFeaturesARM* copy_src, [[maybe_unused]] PNextCopyState* copy_state) {
    sType = copy_src->sType;
    pipelineOpacityMicromap = copy_src->pipelineOpacityMicromap;
    pNext = SafePnextCopy(copy_src->pNext);
}

}  // namespace vku

// NOLINTEND
