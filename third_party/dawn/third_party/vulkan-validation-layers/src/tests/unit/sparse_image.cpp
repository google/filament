/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "utils/vk_layer_utils.h"
#include "../framework/layer_validation_tests.h"
#include "../framework/external_memory_sync.h"

class NegativeSparseImage : public VkLayerTest {};

TEST_F(NegativeSparseImage, BindingImageBufferCreate) {
    TEST_DESCRIPTION("Create buffer/image with sparse attributes but without the sparse_binding bit set");
    AddRequiredFeature(vkt::Feature::sparseResidencyAliased);
    AddRequiredFeature(vkt::Feature::sparseResidencyBuffer);
    AddRequiredFeature(vkt::Feature::sparseResidencyImage2D);
    RETURN_IF_SKIP(Init());

    VkBufferCreateInfo buf_info = vku::InitStructHelper();
    buf_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buf_info.size = 2048;

    if (m_device->Physical().Features().sparseResidencyBuffer) {
        buf_info.flags = VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT;
        CreateBufferTest(*this, &buf_info, "VUID-VkBufferCreateInfo-flags-00918");
    } else {
        GTEST_SKIP() << "Test requires unsupported sparseResidencyBuffer feature";
    }

    if (m_device->Physical().Features().sparseResidencyAliased) {
        buf_info.flags = VK_BUFFER_CREATE_SPARSE_ALIASED_BIT;
        CreateBufferTest(*this, &buf_info, "VUID-VkBufferCreateInfo-flags-00918");
    } else {
        GTEST_SKIP() << "Test requires unsupported sparseResidencyAliased feature";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 512;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    if (m_device->Physical().Features().sparseResidencyImage2D) {
        image_create_info.flags = VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-00987");
    } else {
        GTEST_SKIP() << "Test requires unsupported sparseResidencyImage2D feature";
    }

    if (m_device->Physical().Features().sparseResidencyAliased) {
        image_create_info.flags = VK_IMAGE_CREATE_SPARSE_ALIASED_BIT;
        CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-flags-00987");
    } else {
        GTEST_SKIP() << "Test requires unsupported sparseResidencyAliased feature";
    }
}

TEST_F(NegativeSparseImage, ResidencyImageCreateUnsupportedTypes) {
    TEST_DESCRIPTION("Create images with sparse residency with unsupported types");

    AddRequiredFeature(vkt::Feature::sparseBinding);

    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_1D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 512;
    image_create_info.extent.height = 1;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_BINDING_BIT;

    // 1D image w/ sparse residency is an error
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageType-00970");

    // 2D image w/ sparse residency when feature isn't available
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.extent.height = 64;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageType-00971");

    // 3D image w/ sparse residency when feature isn't available
    image_create_info.imageType = VK_IMAGE_TYPE_3D;
    image_create_info.extent.depth = 8;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageType-00972");
}

TEST_F(NegativeSparseImage, ResidencyImageCreateUnsupportedSamples) {
    TEST_DESCRIPTION("Create images with sparse residency with unsupported tiling or sample counts");
    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::sparseResidencyImage2D);

    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_LINEAR;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_BINDING_BIT;

    // 2D image w/ sparse residency and linear tiling is an error
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-tiling-04121");
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;

    // Multi-sample image w/ sparse residency when feature isn't available (4 flavors)
    image_create_info.samples = VK_SAMPLE_COUNT_2_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageType-00973");

    image_create_info.samples = VK_SAMPLE_COUNT_4_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageType-00974");

    image_create_info.samples = VK_SAMPLE_COUNT_8_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageType-00975");

    image_create_info.samples = VK_SAMPLE_COUNT_16_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-imageType-00976");
}

TEST_F(NegativeSparseImage, ResidencyFlag) {
    TEST_DESCRIPTION("Try to use VkSparseImageMemoryBindInfo without sparse residency flag");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::sparseResidencyImage2D);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 512;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    VkSparseImageMemoryBind image_memory_bind = {};
    image_memory_bind.subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_memory_bind.extent = image_create_info.extent;

    VkSparseImageMemoryBindInfo image_memory_bind_info = {};
    image_memory_bind_info.image = image.handle();
    image_memory_bind_info.bindCount = 1;
    image_memory_bind_info.pBinds = &image_memory_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.imageBindCount = 1;
    bind_info.pImageBinds = &image_memory_bind_info;

    m_errorMonitor->SetDesiredError("VUID-VkSparseImageMemoryBindInfo-image-02901");
    vk::QueueBindSparse(m_device->QueuesWithSparseCapability()[0]->handle(), 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSparseImage, ImageUsageBits) {
    TEST_DESCRIPTION("Try to use VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT with sparse image");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent = {32, 32, 1};
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    CreateImageTest(*this, &image_create_info, "VUID-VkImageCreateInfo-None-01925");
}

TEST_F(NegativeSparseImage, MemoryBindOffset) {
    TEST_DESCRIPTION("Try to use VkSparseImageMemoryBind with offset not less than memory size");
    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::sparseResidencyBuffer);
    AddRequiredFeature(vkt::Feature::sparseResidencyImage2D);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    buffer_create_info.size = 0x10000;

    if (m_device->Physical().Features().sparseResidencyBuffer) {
        buffer_create_info.flags = VK_BUFFER_CREATE_SPARSE_RESIDENCY_BIT | VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    } else {
        GTEST_SKIP() << "Test requires unsupported sparseResidencyBuffer feature";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper(nullptr);
    image_create_info.flags = 0;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 8;
    image_create_info.extent.height = 8;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    if (m_device->Physical().Features().sparseResidencyImage2D) {
        image_create_info.flags = VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    } else {
        GTEST_SKIP() << "Test requires unsupported sparseResidencyImage2D feature";
    }

    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);
    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer, &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    buffer_mem_alloc.allocationSize =
        (buffer_mem_alloc.allocationSize + buffer_mem_reqs.alignment - 1) & ~(buffer_mem_reqs.alignment - 1);
    vkt::DeviceMemory buffer_mem(*m_device, buffer_mem_alloc);

    vkt::Image image(*m_device, image_create_info, vkt::no_mem);
    VkMemoryRequirements image_mem_reqs;
    vk::GetImageMemoryRequirements(device(), image, &image_mem_reqs);
    VkMemoryAllocateInfo image_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    image_mem_alloc.allocationSize =
        (image_mem_alloc.allocationSize + image_mem_reqs.alignment - 1) & ~(image_mem_reqs.alignment - 1);
    vkt::DeviceMemory image_mem(*m_device, image_mem_alloc);

    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.size = buffer_create_info.size;
    buffer_memory_bind.memory = buffer_mem;
    buffer_memory_bind.memoryOffset = buffer_mem_alloc.allocationSize;

    VkSparseImageMemoryBind image_memory_bind = {};
    image_memory_bind.subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_memory_bind.memoryOffset = image_mem_alloc.allocationSize;
    image_memory_bind.memory = image_mem;
    image_memory_bind.extent = image_create_info.extent;

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info = {};
    buffer_memory_bind_info.buffer = buffer;
    buffer_memory_bind_info.bindCount = 1;
    buffer_memory_bind_info.pBinds = &buffer_memory_bind;

    VkSparseMemoryBind image_opaque_memory_bind = {};
    image_opaque_memory_bind.size = 4 * image_create_info.extent.width * image_create_info.extent.height;
    image_opaque_memory_bind.memory = image_mem;
    image_opaque_memory_bind.memoryOffset = image_mem_alloc.allocationSize;

    VkSparseImageOpaqueMemoryBindInfo image_opaque_memory_bind_info = {};
    image_opaque_memory_bind_info.image = image;
    image_opaque_memory_bind_info.bindCount = 1;
    image_opaque_memory_bind_info.pBinds = &image_opaque_memory_bind;

    VkSparseImageMemoryBindInfo image_memory_bind_info = {};
    image_memory_bind_info.image = image;
    image_memory_bind_info.bindCount = 1;
    image_memory_bind_info.pBinds = &image_memory_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.bufferBindCount = 1;
    bind_info.pBufferBinds = &buffer_memory_bind_info;
    bind_info.imageOpaqueBindCount = 1;
    bind_info.pImageOpaqueBinds = &image_opaque_memory_bind_info;
    bind_info.imageBindCount = 1;
    bind_info.pImageBinds = &image_memory_bind_info;

    m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memoryOffset-01101", 3);
    m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-size-01102", 2);
    vk::QueueBindSparse(m_device->QueuesWithSparseCapability()[0]->handle(), 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSparseImage, QueueBindSparseMemoryType) {
    TEST_DESCRIPTION("Test QueueBindSparse with memory of a wrong type");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::sparseResidencyBuffer);
    AddRequiredFeature(vkt::Feature::sparseResidencyImage2D);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    const uint32_t mem_types_mask = (1u << m_device->Physical().memory_properties_.memoryTypeCount) - 1;

    /// Create buffer whose memory has an incompatible type
    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_create_info.size = 1024;

    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &buffer_mem_reqs);
    const bool buffer_supports_all_mem_types = (buffer_mem_reqs.memoryTypeBits & mem_types_mask) == mem_types_mask;
    VkMemoryAllocateInfo buffer_mem_alloc = vku::InitStructHelper();
    buffer_mem_alloc.allocationSize = buffer_mem_reqs.size;
    buffer_mem_alloc.memoryTypeIndex = vvl::kU32Max;
    // Try to pick incompatible memory type
    for (uint32_t memory_type_i = 0; memory_type_i < m_device->Physical().memory_properties_.memoryTypeCount; ++memory_type_i) {
        if (m_device->Physical().memory_properties_.memoryTypes[memory_type_i].propertyFlags &
            VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) {
            continue;
        }

        if (m_device->Physical().memory_properties_.memoryTypes[memory_type_i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) {
            continue;
        }

        if (!((1u << memory_type_i) & buffer_mem_reqs.memoryTypeBits)) {
            buffer_mem_alloc.memoryTypeIndex = memory_type_i;
            break;
        }
    }

    if (buffer_mem_alloc.memoryTypeIndex == vvl::kU32Max) {
        GTEST_SKIP() << "Could not find suitable memory type for buffer, skipping test";
    }

    const bool buffer_mem_lazy =
        m_device->Physical().memory_properties_.memoryTypes[buffer_mem_alloc.memoryTypeIndex].propertyFlags &
        VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;

    vkt::DeviceMemory buffer_mem(*m_device, buffer_mem_alloc);

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    /// Create image whose memory has an incompatible type
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    VkMemoryRequirements image_mem_reqs;
    vk::GetImageMemoryRequirements(device(), image.handle(), &image_mem_reqs);
    const bool image_supports_all_mem_types = (image_mem_reqs.memoryTypeBits & mem_types_mask) == mem_types_mask;
    VkMemoryAllocateInfo image_mem_alloc = vku::InitStructHelper();
    image_mem_alloc.allocationSize = image_mem_reqs.size;
    image_mem_alloc.memoryTypeIndex = vvl::kU32Max;
    // Try to pick incompatible memory type
    for (uint32_t memory_type_i = 0; memory_type_i < m_device->Physical().memory_properties_.memoryTypeCount; ++memory_type_i) {
        if (m_device->Physical().memory_properties_.memoryTypes[memory_type_i].propertyFlags &
            VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD) {
            continue;
        }

        if (m_device->Physical().memory_properties_.memoryTypes[memory_type_i].propertyFlags & VK_MEMORY_PROPERTY_PROTECTED_BIT) {
            continue;
        }

        if (!((1u << memory_type_i) & image_mem_reqs.memoryTypeBits)) {
            image_mem_alloc.memoryTypeIndex = memory_type_i;
            break;
        }
    }

    if (image_mem_alloc.memoryTypeIndex == vvl::kU32Max) {
        GTEST_SKIP() << "Could not find suitable memory type for image, skipping test";
    }

    const bool image_mem_lazy = m_device->Physical().memory_properties_.memoryTypes[image_mem_alloc.memoryTypeIndex].propertyFlags &
                                VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;

    vkt::DeviceMemory image_mem(*m_device, image_mem_alloc);

    /// Specify memory bindings
    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.size = buffer_mem_reqs.size;
    buffer_memory_bind.memory = buffer_mem.handle();

    VkSparseMemoryBind image_memory_bind = {};
    image_memory_bind.size = image_mem_reqs.size;
    image_memory_bind.memory = image_mem.handle();

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info = {};
    buffer_memory_bind_info.buffer = buffer.handle();
    buffer_memory_bind_info.bindCount = 1;
    buffer_memory_bind_info.pBinds = &buffer_memory_bind;

    VkSparseImageOpaqueMemoryBindInfo image_opaque_memory_bind_info = {};
    image_opaque_memory_bind_info.image = image.handle();
    image_opaque_memory_bind_info.bindCount = 1;
    image_opaque_memory_bind_info.pBinds = &image_memory_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.pBufferBinds = &buffer_memory_bind_info;
    bind_info.pImageOpaqueBinds = &image_opaque_memory_bind_info;

    // Validate only buffer
    if (!buffer_supports_all_mem_types) {
        bind_info.bufferBindCount = 1;
        bind_info.imageOpaqueBindCount = 0;
        m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01096");
        if (buffer_mem_lazy) {
            m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01097");
        }
        vk::QueueBindSparse(m_device->QueuesWithSparseCapability()[0]->handle(), 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    } else {
        printf("Could not find an invalid memory type for buffer, skipping part of test.\n");
    }

    // Validate only image
    if (!image_supports_all_mem_types) {
        bind_info.bufferBindCount = 0;
        bind_info.imageOpaqueBindCount = 1;
        m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01096");
        if (image_mem_lazy) {
            m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01097");
        }
        vk::QueueBindSparse(m_device->QueuesWithSparseCapability()[0]->handle(), 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    } else {
        printf("Could not find an invalid memory type for image, skipping part of test.\n");
    }

    // Validate both a buffer and image error occur
    {
        bind_info.bufferBindCount = 1;
        bind_info.imageOpaqueBindCount = 1;
        if (!buffer_supports_all_mem_types) {
            m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01096");
        }
        if (buffer_mem_lazy) {
            m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01097");
        }
        if (!image_supports_all_mem_types) {
            m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01096");
        }
        if (image_mem_lazy) {
            m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01097");
        }
        vk::QueueBindSparse(m_device->QueuesWithSparseCapability()[0]->handle(), 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeSparseImage, QueueBindSparseMemoryType2) {
    TEST_DESCRIPTION("Test QueueBindSparse with lazily allocated memory");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::sparseResidencyBuffer);
    AddRequiredFeature(vkt::Feature::sparseResidencyImage2D);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    uint32_t lazily_allocated_index =
        m_device->Physical().memory_properties_.memoryTypeCount;  // Set to an invalid value just in case
    for (uint32_t i = 0; i < m_device->Physical().memory_properties_.memoryTypeCount; ++i) {
        if ((m_device->Physical().memory_properties_.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT) != 0) {
            lazily_allocated_index = i;
            break;
        }
    }
    if (lazily_allocated_index == m_device->Physical().memory_properties_.memoryTypeCount) {
        GTEST_SKIP() << "Did not find memory with VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT";
    }

    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();
    buffer_create_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_create_info.size = 1024;

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    VkMemoryRequirements buffer_mem_reqs;
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &buffer_mem_reqs);
    VkMemoryAllocateInfo buffer_mem_alloc = vku::InitStructHelper();
    buffer_mem_alloc.allocationSize = buffer_mem_reqs.size;
    buffer_mem_alloc.memoryTypeIndex = lazily_allocated_index;

    VkMemoryRequirements image_mem_reqs;
    vk::GetImageMemoryRequirements(device(), image.handle(), &image_mem_reqs);
    VkMemoryAllocateInfo image_mem_alloc = vku::InitStructHelper();
    image_mem_alloc.allocationSize = image_mem_reqs.size;
    image_mem_alloc.memoryTypeIndex = lazily_allocated_index;

    vkt::DeviceMemory buffer_mem(*m_device, buffer_mem_alloc);

    vkt::DeviceMemory image_mem(*m_device, image_mem_alloc);

    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.size = buffer_mem_reqs.size;
    buffer_memory_bind.memory = buffer_mem.handle();

    VkSparseMemoryBind image_memory_bind = {};
    image_memory_bind.size = image_mem_reqs.size;
    image_memory_bind.memory = image_mem.handle();

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info = {};
    buffer_memory_bind_info.buffer = buffer.handle();
    buffer_memory_bind_info.bindCount = 1;
    buffer_memory_bind_info.pBinds = &buffer_memory_bind;

    VkSparseImageOpaqueMemoryBindInfo image_opaque_memory_bind_info = {};
    image_opaque_memory_bind_info.image = image.handle();
    image_opaque_memory_bind_info.bindCount = 1;
    image_opaque_memory_bind_info.pBinds = &image_memory_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.pBufferBinds = &buffer_memory_bind_info;
    bind_info.pImageOpaqueBinds = &image_opaque_memory_bind_info;

    // Validate only buffer
    {
        bind_info.bufferBindCount = 1;
        bind_info.imageOpaqueBindCount = 0;
        m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01097");
        if (!((1u << buffer_mem_alloc.memoryTypeIndex) & buffer_mem_reqs.memoryTypeBits)) {
            m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01096");
        }
        vk::QueueBindSparse(m_device->QueuesWithSparseCapability()[0]->handle(), 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Validate only image
    {
        bind_info.bufferBindCount = 0;
        bind_info.imageOpaqueBindCount = 1;
        m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01097");
        if (!((1u << image_mem_alloc.memoryTypeIndex) & image_mem_reqs.memoryTypeBits)) {
            m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01096");
        }
        vk::QueueBindSparse(m_device->QueuesWithSparseCapability()[0]->handle(), 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Validate both a buffer and image error occur
    {
        bind_info.bufferBindCount = 1;
        bind_info.imageOpaqueBindCount = 1;
        m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01097");
        m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01097");
        if (!((1u << buffer_mem_alloc.memoryTypeIndex) & buffer_mem_reqs.memoryTypeBits)) {
            m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01096");
        }
        if (!((1u << image_mem_alloc.memoryTypeIndex) & image_mem_reqs.memoryTypeBits)) {
            m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01096");
        }
        vk::QueueBindSparse(m_device->QueuesWithSparseCapability()[0]->handle(), 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeSparseImage, QueueBindSparseMemoryType3) {
    TEST_DESCRIPTION(
        "Test QueueBindSparse with memory having export external handle types that do not match those of the resource");

    SetTargetApiVersion(VK_API_VERSION_1_1);
    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::sparseResidencyBuffer);
    AddRequiredFeature(vkt::Feature::sparseResidencyImage2D);
    // Required to pass in various memory flags without querying for corresponding extensions.
    AddRequiredExtensions(VK_KHR_MAINTENANCE_5_EXTENSION_NAME);
    RETURN_IF_SKIP(Init());
    IgnoreHandleTypeError(m_errorMonitor);

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    /// Allocate buffer and buffer memory with an external handle type
    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper();  // Do not set any supported external handle type
    buffer_create_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    buffer_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    buffer_create_info.size = 1024;
    vkt::Buffer buffer(*m_device, buffer_create_info, vkt::no_mem);
    const auto buffer_exportable_types =
        FindSupportedExternalMemoryHandleTypes(Gpu(), buffer_create_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);
    if (!buffer_exportable_types) {
        GTEST_SKIP() << "Unable to find exportable handle type for buffer, skipping test";
    }
    const auto buffer_exportable_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(buffer_exportable_types);
    VkExportMemoryAllocateInfo buffer_export_mem_alloc_info = vku::InitStructHelper();
    buffer_export_mem_alloc_info.handleTypes = GetCompatibleHandleTypes(Gpu(), buffer_create_info, buffer_exportable_type);
    VkMemoryRequirements buffer_mem_reqs{};
    vk::GetBufferMemoryRequirements(device(), buffer.handle(), &buffer_mem_reqs);
    const VkMemoryAllocateInfo buffer_mem_alloc = vkt::DeviceMemory::GetResourceAllocInfo(
        *m_device, buffer_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &buffer_export_mem_alloc_info);
    vkt::DeviceMemory buffer_mem(*m_device, buffer_mem_alloc);

    /// Allocate image and image memory  with an external handle type
    VkImageCreateInfo image_create_info = vku::InitStructHelper();  // Do not set any supported external handle type
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    const auto image_exportable_types =
        FindSupportedExternalMemoryHandleTypes(Gpu(), image_create_info, VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT);
    if (!image_exportable_types) {
        GTEST_SKIP() << "Unable to find exportable handle type for image, skipping test";
    }
    const auto image_exportable_type = LeastSignificantFlag<VkExternalMemoryHandleTypeFlagBits>(image_exportable_types);

    VkExportMemoryAllocateInfo image_export_mem_alloc_info = vku::InitStructHelper();
    image_export_mem_alloc_info.handleTypes = GetCompatibleHandleTypes(Gpu(), image_create_info, image_exportable_type);
    VkMemoryRequirements image_mem_reqs;
    vk::GetImageMemoryRequirements(device(), image.handle(), &image_mem_reqs);
    const VkMemoryAllocateInfo image_mem_alloc = vkt::DeviceMemory::GetResourceAllocInfo(
        *m_device, image_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &image_export_mem_alloc_info);
    vkt::DeviceMemory image_mem(*m_device, image_mem_alloc);

    // Setup memory bindings
    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.size = buffer_mem_reqs.size;
    buffer_memory_bind.memory = buffer_mem.handle();

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info = {};
    buffer_memory_bind_info.buffer = buffer.handle();
    buffer_memory_bind_info.bindCount = 1;
    buffer_memory_bind_info.pBinds = &buffer_memory_bind;

    VkSparseMemoryBind image_memory_bind = {};
    image_memory_bind.size = image_mem_reqs.size;
    image_memory_bind.memory = image_mem.handle();

    VkSparseImageOpaqueMemoryBindInfo image_opaque_memory_bind_info = {};
    image_opaque_memory_bind_info.image = image.handle();
    image_opaque_memory_bind_info.bindCount = 1;
    image_opaque_memory_bind_info.pBinds = &image_memory_bind;

    VkSparseImageMemoryBind image_memory_bind_2 = {};
    image_memory_bind_2.extent = image_create_info.extent;
    image_memory_bind_2.memory = image_mem.handle();
    image_memory_bind_2.subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkSparseImageMemoryBindInfo image_memory_bind_info = {};
    image_memory_bind_info.image = image;
    image_memory_bind_info.bindCount = 1;
    image_memory_bind_info.pBinds = &image_memory_bind_2;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.pBufferBinds = &buffer_memory_bind_info;
    bind_info.pImageOpaqueBinds = &image_opaque_memory_bind_info;
    bind_info.pImageBinds = &image_memory_bind_info;

    // Validate only buffer
    {
        bind_info.bufferBindCount = 1;
        bind_info.imageOpaqueBindCount = 0;
        m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-02730");
        vk::QueueBindSparse(m_device->QueuesWithSparseCapability()[0]->handle(), 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Validate only image opaque bind
    {
        bind_info.bufferBindCount = 0;
        bind_info.imageOpaqueBindCount = 1;
        m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-02730");
        vk::QueueBindSparse(m_device->QueuesWithSparseCapability()[0]->handle(), 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Validate both a buffer and image error occur
    {
        bind_info.bufferBindCount = 1;
        bind_info.imageOpaqueBindCount = 1;
        m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-02730", 2);
        vk::QueueBindSparse(m_device->QueuesWithSparseCapability()[0]->handle(), 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Validate only image bind
    {
        bind_info.bufferBindCount = 0;
        bind_info.imageOpaqueBindCount = 0;
        bind_info.imageBindCount = 1;
        m_errorMonitor->SetDesiredError("VUID-VkSparseImageMemoryBind-memory-02732");
        vk::QueueBindSparse(m_device->QueuesWithSparseCapability()[0]->handle(), 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeSparseImage, QueueBindSparseMemoryType4) {
    TEST_DESCRIPTION(
        "Test QueueBindSparse with memory having import external handle types that do not match those of the resource");

    SetTargetApiVersion(VK_API_VERSION_1_1);
#ifdef _WIN32
    const auto ext_mem_extension_name = VK_KHR_EXTERNAL_MEMORY_WIN32_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_WIN32_BIT;
#else
    const auto ext_mem_extension_name = VK_KHR_EXTERNAL_MEMORY_FD_EXTENSION_NAME;
    const auto handle_type = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;
#endif

    AddRequiredExtensions(VK_KHR_EXTERNAL_MEMORY_EXTENSION_NAME);
    AddRequiredExtensions(VK_KHR_DEDICATED_ALLOCATION_EXTENSION_NAME);
    AddRequiredExtensions(ext_mem_extension_name);
    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::sparseResidencyBuffer);
    AddRequiredFeature(vkt::Feature::sparseResidencyImage2D);
    RETURN_IF_SKIP(Init());
    if (IsPlatformMockICD()) {
        GTEST_SKIP() << "External tests are not supported by MockICD, skipping tests";
    }

    // Check for import/export capability
    VkPhysicalDeviceExternalBufferInfoKHR external_buffer_info = {
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_BUFFER_INFO_KHR, nullptr, VK_BUFFER_CREATE_SPARSE_BINDING_BIT,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, handle_type};
    VkExternalBufferProperties external_buffer_props = {VK_STRUCTURE_TYPE_EXTERNAL_BUFFER_PROPERTIES_KHR, nullptr, {0, 0, 0}};
    vk::GetPhysicalDeviceExternalBufferProperties(Gpu(), &external_buffer_info, &external_buffer_props);
    if (!(external_buffer_props.externalMemoryProperties.compatibleHandleTypes & handle_type) ||
        !(external_buffer_props.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_EXPORTABLE_BIT) ||
        !(external_buffer_props.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_IMPORTABLE_BIT)) {
        GTEST_SKIP() << "External buffer does not support importing and exporting";
    }

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    // Check if dedicated allocation is required
    const bool dedicated_allocation =
        external_buffer_props.externalMemoryProperties.externalMemoryFeatures & VK_EXTERNAL_MEMORY_FEATURE_DEDICATED_ONLY_BIT;
    if (dedicated_allocation) {
        // VUID-VkMemoryDedicatedAllocateInfo-buffer-01436
        GTEST_SKIP() << "Dedicated allocation is required, which cannot be used with VK_BUFFER_CREATE_SPARSE_BINDING_BIT";
    }

    /// Allocate buffer and buffer memory with no supported external type
    VkExternalMemoryBufferCreateInfo external_memory_buffer_info = vku::InitStructHelper();
    external_memory_buffer_info.handleTypes = 0;
    VkBufferCreateInfo buffer_create_info = vku::InitStructHelper(&external_memory_buffer_info);
    buffer_create_info.flags = VK_BUFFER_CREATE_SPARSE_BINDING_BIT;
    buffer_create_info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    buffer_create_info.size = 1024;
    vkt::Buffer buffer1(*m_device, buffer_create_info, vkt::no_mem);
    buffer_create_info.size = 65536;
    vkt::Buffer buffer2(*m_device, buffer_create_info, vkt::no_mem);

    VkMemoryAllocateInfo buffer_mem_alloc1 = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer1.MemoryRequirements(), 0);
    VkMemoryAllocateInfo buffer_mem_alloc2 = vkt::DeviceMemory::GetResourceAllocInfo(*m_device, buffer2.MemoryRequirements(), 0);
    VkExportMemoryAllocateInfo export_info1 = vku::InitStructHelper();
    export_info1.handleTypes = handle_type;
    buffer_mem_alloc1.pNext = &export_info1;
    VkExportMemoryAllocateInfo export_info2 = vku::InitStructHelper();
    export_info2.handleTypes = handle_type;
    buffer_mem_alloc2.pNext = &export_info2;

    // Export memory
    vkt::DeviceMemory buffer_memory_export1(*m_device, buffer_mem_alloc1);
    vkt::DeviceMemory buffer_memory_export2(*m_device, buffer_mem_alloc2);

#ifdef _WIN32
    // Export memory to handle
    VkMemoryGetWin32HandleInfoKHR mghi = vku::InitStructHelper();
    mghi.memory = buffer_memory_export1;
    mghi.handleType = handle_type;

    HANDLE handle1;
    ASSERT_EQ(VK_SUCCESS, vk::GetMemoryWin32HandleKHR(device(), &mghi, &handle1));

    VkImportMemoryWin32HandleInfoKHR import_info1 = vku::InitStructHelper();
    import_info1.handleType = handle_type;
    import_info1.handle = handle1;

    mghi.memory = buffer_memory_export2;
    HANDLE handle2;
    ASSERT_EQ(VK_SUCCESS, vk::GetMemoryWin32HandleKHR(device(), &mghi, &handle2));

    VkImportMemoryWin32HandleInfoKHR import_info2 = vku::InitStructHelper();
    import_info2.handleType = handle_type;
    import_info2.handle = handle2;
#else
    // Export memory to fd
    VkMemoryGetFdInfoKHR mgfi = vku::InitStructHelper();
    mgfi.memory = buffer_memory_export1;
    mgfi.handleType = handle_type;

    int fd1 = 0;
    ASSERT_EQ(VK_SUCCESS, vk::GetMemoryFdKHR(device(), &mgfi, &fd1));

    VkImportMemoryFdInfoKHR import_info1 = vku::InitStructHelper();
    import_info1.handleType = handle_type;
    import_info1.fd = fd1;

    mgfi.memory = buffer_memory_export2;
    int fd2 = 0;
    ASSERT_EQ(VK_SUCCESS, vk::GetMemoryFdKHR(device(), &mgfi, &fd2));

    VkImportMemoryFdInfoKHR import_info2 = vku::InitStructHelper();
    import_info2.handleType = handle_type;
    import_info2.fd = fd2;
#endif

    buffer_mem_alloc1.pNext = &import_info1;
    vkt::DeviceMemory buffer_memory_imported(*m_device, buffer_mem_alloc1);

    /// Allocate image and image memory with an external handle type
    VkImageCreateInfo image_create_info = vku::InitStructHelper();  // Do not set any supported external handle type
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_R8G8B8A8_UNORM;
    image_create_info.extent.width = 64;
    image_create_info.extent.height = 64;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    VkMemoryRequirements image_mem_reqs;
    vk::GetImageMemoryRequirements(device(), image.handle(), &image_mem_reqs);
    const VkMemoryAllocateInfo image_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &import_info2);
    vkt::DeviceMemory image_mem(*m_device, image_mem_alloc);

    // Setup memory bindings
    VkSparseMemoryBind buffer_memory_bind = {};
    buffer_memory_bind.size = buffer1.MemoryRequirements().size;
    buffer_memory_bind.memory = buffer_memory_imported.handle();

    VkSparseBufferMemoryBindInfo buffer_memory_bind_info = {};
    buffer_memory_bind_info.buffer = buffer1.handle();
    buffer_memory_bind_info.bindCount = 1;
    buffer_memory_bind_info.pBinds = &buffer_memory_bind;

    VkSparseMemoryBind image_memory_bind = {};
    image_memory_bind.size = image_mem_reqs.size;
    image_memory_bind.memory = image_mem.handle();

    VkSparseImageOpaqueMemoryBindInfo image_opaque_memory_bind_info = {};
    image_opaque_memory_bind_info.image = image.handle();
    image_opaque_memory_bind_info.bindCount = 1;
    image_opaque_memory_bind_info.pBinds = &image_memory_bind;

    VkSparseImageMemoryBind image_memory_bind_2 = {};
    image_memory_bind_2.extent = image_create_info.extent;
    image_memory_bind_2.memory = image_mem.handle();
    image_memory_bind_2.subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkSparseImageMemoryBindInfo image_memory_bind_info = {};
    image_memory_bind_info.image = image;
    image_memory_bind_info.bindCount = 1;
    image_memory_bind_info.pBinds = &image_memory_bind_2;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.pBufferBinds = &buffer_memory_bind_info;
    bind_info.pImageOpaqueBinds = &image_opaque_memory_bind_info;
    bind_info.pImageBinds = &image_memory_bind_info;

    // Validate only buffer
    {
        bind_info.bufferBindCount = 1;
        bind_info.imageOpaqueBindCount = 0;
        m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-02731");
        vk::QueueBindSparse(m_device->QueuesWithSparseCapability()[0]->handle(), 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Validate only image opaque bind
    {
        bind_info.bufferBindCount = 0;
        bind_info.imageOpaqueBindCount = 1;
        m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-02731");
        vk::QueueBindSparse(m_device->QueuesWithSparseCapability()[0]->handle(), 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Validate both a buffer and image error occur
    {
        bind_info.bufferBindCount = 1;
        bind_info.imageOpaqueBindCount = 1;
        m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-02731", 2);
        vk::QueueBindSparse(m_device->QueuesWithSparseCapability()[0]->handle(), 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }

    // Validate only image bind
    {
        bind_info.bufferBindCount = 0;
        bind_info.imageOpaqueBindCount = 0;
        bind_info.imageBindCount = 1;
        m_errorMonitor->SetDesiredError("VUID-VkSparseImageMemoryBind-memory-02733");
        vk::QueueBindSparse(m_device->QueuesWithSparseCapability()[0]->handle(), 1, &bind_info, VK_NULL_HANDLE);
        m_errorMonitor->VerifyFound();
    }
}

TEST_F(NegativeSparseImage, ImageMemoryBind) {
    TEST_DESCRIPTION("Try to bind sparse resident image with invalid VkSparseImageMemoryBind");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::sparseResidencyImage3D);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    VkImageCreateInfo create_info = vkt::Image::CreateInfo();
    create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
    create_info.imageType = VK_IMAGE_TYPE_3D;
    create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    create_info.extent.width = 1024;
    create_info.extent.height = 1024;
    create_info.extent.depth = 1;
    create_info.arrayLayers = 1;
    vkt::Image image(*m_device, create_info, vkt::no_mem);

    VkMemoryRequirements image_mem_reqs;
    vk::GetImageMemoryRequirements(m_device->handle(), image.handle(), &image_mem_reqs);
    const auto image_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory image_mem;
    image_mem.init(*m_device, image_mem_alloc);

    uint32_t requirements_count = 0u;
    vk::GetImageSparseMemoryRequirements(m_device->handle(), image.handle(), &requirements_count, nullptr);

    if (requirements_count == 0u) {
        GTEST_SKIP() << "No sparse image requirements for image format VK_FORMAT_B8G8R8A8_UNORM";
    }

    std::vector<VkSparseImageMemoryRequirements> sparse_reqs(requirements_count);
    vk::GetImageSparseMemoryRequirements(m_device->handle(), image.handle(), &requirements_count, sparse_reqs.data());

    VkExtent3D granularity = sparse_reqs[0].formatProperties.imageGranularity;
    VkSparseImageMemoryBind image_bind{};
    image_bind.subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_bind.memory = image_mem.handle();
    image_bind.extent = granularity;

    VkSparseImageMemoryBindInfo image_bind_info{};
    image_bind_info.image = image.handle();
    image_bind_info.bindCount = 1u;
    image_bind_info.pBinds = &image_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.imageBindCount = 1u;
    bind_info.pImageBinds = &image_bind_info;

    VkQueue sparse_queue = m_device->QueuesWithSparseCapability()[0]->handle();

    // Force offset.x to invalid value
    image_bind.offset.x = granularity.width - 1;
    m_errorMonitor->SetDesiredError("VUID-VkSparseImageMemoryBind-offset-01107");
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    image_bind.offset.x = 0u;

    // Force offset.y to invalid value
    image_bind.offset.y = granularity.height - 1;
    m_errorMonitor->SetDesiredError("VUID-VkSparseImageMemoryBind-offset-01109");
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    image_bind.offset.y = 0u;

    // Force offset.y to invalid value
    image_bind.offset.z = granularity.depth - 1;
    m_errorMonitor->SetDesiredError("VUID-VkSparseImageMemoryBind-offset-01111");
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    image_bind.offset.z = 0u;

    // Force extent.width to invalid value
    image_bind.extent.width = granularity.width - 1;
    m_errorMonitor->SetDesiredError("VUID-VkSparseImageMemoryBind-extent-01108");
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    image_bind.extent.width = granularity.width;

    // Force extent.height to invalid value
    image_bind.extent.height = granularity.height - 1;
    m_errorMonitor->SetDesiredError("VUID-VkSparseImageMemoryBind-extent-01110");
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    image_bind.extent.height = granularity.height;

    // Force extent.depth to invalid value
    image_bind.extent.depth = granularity.depth - 1;
    m_errorMonitor->SetDesiredError("VUID-VkSparseImageMemoryBind-extent-01112");
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    image_bind.extent.depth = granularity.depth;

    // Force greater mip level
    image_bind.subresource.mipLevel = VK_REMAINING_MIP_LEVELS;
    m_errorMonitor->SetDesiredError("VUID-VkSparseImageMemoryBindInfo-subresource-01722");
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    image_bind.subresource.mipLevel = 0;

    // Force greater array layer
    image_bind.subresource.arrayLayer = VK_REMAINING_ARRAY_LAYERS;
    m_errorMonitor->SetDesiredError("VUID-VkSparseImageMemoryBindInfo-subresource-01723");
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    image_bind.subresource.arrayLayer = 0;

    // Force invalid aspect mask
    image_bind.subresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    m_errorMonitor->SetDesiredError("VUID-VkSparseImageMemoryBindInfo-subresource-01106");
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    image_bind.subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
}

TEST_F(NegativeSparseImage, ImageMemoryBindInvalidMemory) {
    TEST_DESCRIPTION("Try to bind sparse resident image with invalid VkSparseImageMemoryBind");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::sparseResidencyImage3D);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    VkImageCreateInfo create_info = vkt::Image::CreateInfo();
    create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
    create_info.imageType = VK_IMAGE_TYPE_3D;
    create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    create_info.extent.width = 1024;
    create_info.extent.height = 1024;
    create_info.extent.depth = 1;
    create_info.arrayLayers = 1;
    vkt::Image image(*m_device, create_info, vkt::no_mem);

    VkMemoryRequirements image_mem_reqs;
    vk::GetImageMemoryRequirements(m_device->handle(), image.handle(), &image_mem_reqs);
    const auto image_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory image_mem;
    image_mem.init(*m_device, image_mem_alloc);

    VkImageCreateInfo invalid_create_info = create_info;
    vkt::Image invalid_image(*m_device, invalid_create_info, vkt::no_mem);

    VkMemoryRequirements invalid_image_mem_reqs;
    vk::GetImageMemoryRequirements(m_device->handle(), invalid_image.handle(), &invalid_image_mem_reqs);

    // Make sure that the same memory type is not chosen.
    invalid_image_mem_reqs.memoryTypeBits = ~image_mem_reqs.memoryTypeBits;

    VkMemoryAllocateInfo invalid_image_mem_alloc = vku::InitStructHelper();
    invalid_image_mem_alloc.allocationSize = invalid_image_mem_reqs.size;
    if (!m_device->Physical().SetMemoryType(invalid_image_mem_reqs.memoryTypeBits, &invalid_image_mem_alloc,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                            VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_PROTECTED_BIT)) {
        GTEST_SKIP() << "Could not find required memory type";
    }

    vkt::DeviceMemory invalid_image_mem;
    invalid_image_mem.init(*m_device, invalid_image_mem_alloc);

    uint32_t requirements_count = 0u;
    vk::GetImageSparseMemoryRequirements(m_device->handle(), image.handle(), &requirements_count, nullptr);

    if (requirements_count == 0u) {
        GTEST_SKIP() << "No sparse image requirements for image format VK_FORMAT_B8G8R8A8_UNORM";
    }

    std::vector<VkSparseImageMemoryRequirements> sparse_reqs(requirements_count);
    vk::GetImageSparseMemoryRequirements(m_device->handle(), image.handle(), &requirements_count, sparse_reqs.data());

    VkExtent3D granularity = sparse_reqs[0].formatProperties.imageGranularity;
    VkSparseImageMemoryBind image_bind{};
    image_bind.subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_bind.memory = image_mem.handle();
    image_bind.memoryOffset = 0;
    image_bind.extent = granularity;

    VkSparseImageMemoryBindInfo image_bind_info{};
    image_bind_info.image = image.handle();
    image_bind_info.bindCount = 1u;
    image_bind_info.pBinds = &image_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.imageBindCount = 1u;
    bind_info.pImageBinds = &image_bind_info;

    VkQueue sparse_queue = m_device->QueuesWithSparseCapability()[0]->handle();

    // Force invalid device memory
    image_bind.memory = invalid_image_mem.handle();
    m_errorMonitor->SetDesiredError("VUID-VkSparseImageMemoryBind-memory-01105");
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSparseImage, ImageMemoryBindInvalidAlignment) {
    TEST_DESCRIPTION("Try to bind sparse resident image with invalid VkSparseImageMemoryBind");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::sparseResidencyImage3D);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    VkImageCreateInfo create_info = vkt::Image::CreateInfo();
    create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
    create_info.imageType = VK_IMAGE_TYPE_3D;
    create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    create_info.extent.width = 1024;
    create_info.extent.height = 1024;
    create_info.extent.depth = 1;
    create_info.arrayLayers = 1;
    vkt::Image image(*m_device, create_info, vkt::no_mem);

    VkMemoryRequirements image_mem_reqs;
    vk::GetImageMemoryRequirements(m_device->handle(), image.handle(), &image_mem_reqs);
    if (image_mem_reqs.alignment == 1) {
        GTEST_SKIP() << "Need image memory required alignment to be more than 1";
    }

    const auto image_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory image_mem;
    image_mem.init(*m_device, image_mem_alloc);

    VkImageCreateInfo invalid_create_info = create_info;
    vkt::Image invalid_image(*m_device, invalid_create_info, vkt::no_mem);

    VkMemoryRequirements invalid_image_mem_reqs;
    vk::GetImageMemoryRequirements(m_device->handle(), invalid_image.handle(), &invalid_image_mem_reqs);

    // Make sure that the same memory type is not chosen.
    invalid_image_mem_reqs.memoryTypeBits = ~image_mem_reqs.memoryTypeBits;

    VkMemoryAllocateInfo invalid_image_mem_alloc = vku::InitStructHelper();
    invalid_image_mem_alloc.allocationSize = invalid_image_mem_reqs.size;
    if (!m_device->Physical().SetMemoryType(invalid_image_mem_reqs.memoryTypeBits, &invalid_image_mem_alloc,
                                            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                                            VK_MEMORY_PROPERTY_DEVICE_COHERENT_BIT_AMD | VK_MEMORY_PROPERTY_PROTECTED_BIT)) {
        GTEST_SKIP() << "Could not find required memory type";
    }

    vkt::DeviceMemory invalid_image_mem;
    invalid_image_mem.init(*m_device, invalid_image_mem_alloc);

    uint32_t requirements_count = 0u;
    vk::GetImageSparseMemoryRequirements(m_device->handle(), image.handle(), &requirements_count, nullptr);

    if (requirements_count == 0u) {
        GTEST_SKIP() << "No sparse image requirements for image format VK_FORMAT_B8G8R8A8_UNORM";
    }

    std::vector<VkSparseImageMemoryRequirements> sparse_reqs(requirements_count);
    vk::GetImageSparseMemoryRequirements(m_device->handle(), image.handle(), &requirements_count, sparse_reqs.data());

    VkExtent3D granularity = sparse_reqs[0].formatProperties.imageGranularity;
    VkSparseImageMemoryBind image_bind{};
    image_bind.subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_bind.memory = image_mem.handle();
    image_bind.memoryOffset = 0;
    image_bind.extent = granularity;

    VkSparseImageMemoryBindInfo image_bind_info{};
    image_bind_info.image = image.handle();
    image_bind_info.bindCount = 1u;
    image_bind_info.pBinds = &image_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.imageBindCount = 1u;
    bind_info.pImageBinds = &image_bind_info;

    VkQueue sparse_queue = m_device->QueuesWithSparseCapability()[0]->handle();

    // Force memoryOffset to invalid value
    image_bind.memoryOffset = image_mem_reqs.alignment + 1;

    m_errorMonitor->SetDesiredError("VUID-VkSparseImageMemoryBind-memory-01105");
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
    image_bind.memoryOffset = 0;
}

TEST_F(NegativeSparseImage, ImageMemoryBindInvalidExtent) {
    TEST_DESCRIPTION("Try to bind sparse resident image with an extent having a null size on one of its dimension");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    AddRequiredFeature(vkt::Feature::sparseResidencyImage3D);
    RETURN_IF_SKIP(Init());

    if (m_device->QueuesWithSparseCapability().empty()) {
        GTEST_SKIP() << "Required SPARSE_BINDING queue families not present";
    }

    VkImageCreateInfo create_info = vkt::Image::CreateInfo();
    create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT;
    create_info.imageType = VK_IMAGE_TYPE_3D;
    create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    create_info.extent.width = 1024;
    create_info.extent.height = 1024;
    create_info.extent.depth = 1;
    create_info.arrayLayers = 1;
    vkt::Image image(*m_device, create_info, vkt::no_mem);

    VkMemoryRequirements image_mem_reqs;
    vk::GetImageMemoryRequirements(m_device->handle(), image.handle(), &image_mem_reqs);
    const auto image_mem_alloc =
        vkt::DeviceMemory::GetResourceAllocInfo(*m_device, image_mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkt::DeviceMemory image_mem;
    image_mem.init(*m_device, image_mem_alloc);

    uint32_t requirements_count = 0u;
    vk::GetImageSparseMemoryRequirements(m_device->handle(), image.handle(), &requirements_count, nullptr);

    if (requirements_count == 0u) {
        GTEST_SKIP() << "No sparse image requirements for image format VK_FORMAT_B8G8R8A8_UNORM";
    }

    std::vector<VkSparseImageMemoryRequirements> sparse_reqs(requirements_count);
    vk::GetImageSparseMemoryRequirements(m_device->handle(), image.handle(), &requirements_count, sparse_reqs.data());

    VkExtent3D granularity = sparse_reqs[0].formatProperties.imageGranularity;
    VkSparseImageMemoryBind image_bind{};
    image_bind.subresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    image_bind.memory = image_mem.handle();
    image_bind.extent = granularity;

    VkSparseImageMemoryBindInfo image_bind_info{};
    image_bind_info.image = image.handle();
    image_bind_info.bindCount = 1u;
    image_bind_info.pBinds = &image_bind;

    VkBindSparseInfo bind_info = vku::InitStructHelper();
    bind_info.imageBindCount = 1u;
    bind_info.pImageBinds = &image_bind_info;

    VkQueue sparse_queue = m_device->QueuesWithSparseCapability()[0]->handle();

    image_bind.extent.width = 0;
    m_errorMonitor->SetDesiredError("VUID-VkSparseImageMemoryBind-extent-09388");
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    image_bind.extent = granularity;

    image_bind.extent.height = 0;
    m_errorMonitor->SetDesiredError("VUID-VkSparseImageMemoryBind-extent-09389");
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    image_bind.extent = granularity;

    image_bind.extent.depth = 0;
    m_errorMonitor->SetDesiredError("VUID-VkSparseImageMemoryBind-extent-09390");
    vk::QueueBindSparse(sparse_queue, 1, &bind_info, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();
}

TEST_F(NegativeSparseImage, UnalignedBindOffsets) {
    TEST_DESCRIPTION("VkSparseMemoryBind have unaligned memory offset and resource offset");

    AddRequiredFeature(vkt::Feature::sparseBinding);
    RETURN_IF_SKIP(Init());

    auto index = m_device->graphics_queue_node_index_;
    if (!(m_device->Physical().queue_properties_[index].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)) {
        GTEST_SKIP() << "Graphics queue does not have sparse binding bit";
    }

    VkImageCreateInfo image_create_info = vku::InitStructHelper();
    image_create_info.imageType = VK_IMAGE_TYPE_2D;
    image_create_info.format = VK_FORMAT_B8G8R8A8_UNORM;
    image_create_info.extent.width = 1024;
    image_create_info.extent.height = 1024;
    image_create_info.extent.depth = 1;
    image_create_info.mipLevels = 1;
    image_create_info.arrayLayers = 1;
    image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.flags = VK_IMAGE_CREATE_SPARSE_BINDING_BIT;
    vkt::Image image(*m_device, image_create_info, vkt::no_mem);

    VkMemoryRequirements memory_reqs;
    vk::GetImageMemoryRequirements(device(), image, &memory_reqs);
    if (memory_reqs.alignment == 1) {
        GTEST_SKIP() << "Need image memory required alignment to be more than 1";
    }
    // Find an image big enough to allow sparse mapping of 2 memory regions
    // Increase the image size until it is at least twice the
    // size of the required alignment, to ensure we can bind both
    // allocated memory blocks to the image on aligned offsets.
    while (memory_reqs.size < (memory_reqs.alignment * 2)) {
        image.destroy();
        image_create_info.extent.width *= 2;
        image_create_info.extent.height *= 2;
        image.InitNoMemory(*m_device, image_create_info);
        vk::GetImageMemoryRequirements(device(), image, &memory_reqs);
    }
    // Allocate 2 memory regions of minimum alignment size, bind one at 0, the other
    // at the end of the first
    VkMemoryAllocateInfo memory_info = vku::InitStructHelper();
    memory_info.allocationSize = 2 * memory_reqs.alignment;
    bool pass = m_device->Physical().SetMemoryType(memory_reqs.memoryTypeBits, &memory_info, 0);
    ASSERT_TRUE(pass);
    vkt::DeviceMemory memory_one(*m_device, memory_info);
    vkt::DeviceMemory memory_two(*m_device, memory_info);

    std::array<VkSparseMemoryBind, 2> binds = {};
    binds[0].memory = memory_one;
    binds[0].memoryOffset = 1;
    binds[0].resourceOffset = memory_info.allocationSize / 2;
    binds[0].size = memory_info.allocationSize / 2;
    binds[1].memory = memory_two;
    binds[1].memoryOffset = 0;
    binds[1].resourceOffset = 1;
    binds[1].size = memory_info.allocationSize / 2;

    VkSparseImageOpaqueMemoryBindInfo opaqueBindInfo;
    opaqueBindInfo.image = image;
    opaqueBindInfo.bindCount = size32(binds);
    opaqueBindInfo.pBinds = binds.data();

    VkBindSparseInfo bindSparseInfo = vku::InitStructHelper();
    bindSparseInfo.imageOpaqueBindCount = 1;
    bindSparseInfo.pImageOpaqueBinds = &opaqueBindInfo;

    // Unaligned memory bind offset
    m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-memory-01096");
    // Unaligned memory bind offset, and resource offset
    m_errorMonitor->SetDesiredError("VUID-VkSparseMemoryBind-resourceOffset-09492", 2);
    vk::QueueBindSparse(m_default_queue->handle(), 1, &bindSparseInfo, VK_NULL_HANDLE);
    m_errorMonitor->VerifyFound();

    // Wait for operations to finish before destroying anything
    m_default_queue->Wait();
}
