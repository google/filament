// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <vector>

#include "dawn/common/vulkan_platform.h"
#include "dawn/native/vulkan/MemoryTypeSelector.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/webgpu_cpp_print.h"
#include "gtest/gtest.h"

namespace dawn::native::vulkan {

// Memory requirement that allows any memory type to be selected. Size and alignment aren't
// important. The corresponding bit in `memoryTypeBits` must be 1 for each memory type index.
constexpr VkMemoryRequirements kAnyType = {.size = 16,
                                           .alignment = 16,
                                           .memoryTypeBits = 0xFFFFFFFF};

TEST(MemoryTypeSelectorTests, Quatro_P100) {
    // Memory info from Nvidia Quatro P100 GPU.
    std::vector<VkMemoryType> memoryTypes = {
        {VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, /*heap=*/0},
        {VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, /*heap=*/1},
        {VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT |
             VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
         /*heap=*/1},
        {VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
         /*heap=*/2}};
    std::vector<VkMemoryHeap> memoryHeaps = {{4'294'967'296, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT},
                                             {151'015'474'176, 0},
                                             {257'949'696, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT}};
    MemoryTypeSelector selector(memoryTypes, memoryHeaps);

    // Device local buffers or textures.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear), 0);
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::DeviceLocal), 0);
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::DeviceLocal),
              0);

    // Write mappable buffers prefer HOST_COHERENT.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::WriteMappable),
              1);

    // Read mappable buffers prefer HOST_CACHED.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::ReadMappable),
              2);

    // Read+write mappable buffers prefer HOST_CACHED+HOST_COHERENT.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::ReadMappable |
                                                       MemoryKind::WriteMappable),
              2);

    // BufferMapExtendedUsages buffers are disabled on Nvidia GPUs.
}

TEST(MemoryTypeSelectorTests, Mali_G715) {
    // Memory info from Mali G715 GPU.
    std::vector<VkMemoryType> memoryTypes = {
        {VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
         /*heap=*/0},
        {VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
             VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
         /*heap=*/0},
        {VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT, /*heap=*/0},
        {VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_PROTECTED_BIT, /*heap=*/1}};
    std::vector<VkMemoryHeap> memoryHeaps = {{11'871'064'064, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT},
                                             {104'857'600, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT}};
    MemoryTypeSelector selector(memoryTypes, memoryHeaps);

    // Device local buffers or textures.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear), 0);
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::DeviceLocal), 0);
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::DeviceLocal),
              0);

    // Write mappable buffers prefer HOST_COHERENT.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::WriteMappable),
              0);

    // Read mappable buffers prefer HOST_CACHED over HOST_COHERENT.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::ReadMappable),
              1);

    // Read+write mappable buffers prefer HOST_CACHED since HOST_CACHED+HOST_COHERENT isn't
    // available.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::ReadMappable |
                                                       MemoryKind::WriteMappable),
              1);

    // Lazily allocated textures.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::LazilyAllocated), 2);

    // BufferMapExtendedUsages write mappable buffers prefer HOST_COHERENT.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::DeviceLocal |
                                                       MemoryKind::WriteMappable),
              0);

    // BufferMapExtendedUsages read mappable buffers prefer HOST_CACHED.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::DeviceLocal |
                                                       MemoryKind::ReadMappable),
              1);

    // BufferMapExtendedUsages read+write mappable buffers prefer HOST_CACHED since no
    // HOST_CACHED+HOST_COHERENT exists.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::DeviceLocal |
                                                       MemoryKind::ReadMappable |
                                                       MemoryKind::WriteMappable),
              1);
}

TEST(MemoryTypeSelectorTests, Mali_G72) {
    // Memory info from Mali G72 GPU.
    std::vector<VkMemoryType> memoryTypes = {
        {VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
         /*heap=*/0},
        {VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
         /*heap=*/0},
        {VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT,
         /*heap=*/0}};
    std::vector<VkMemoryHeap> memoryHeaps = {{5'931'765'760, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT}};
    MemoryTypeSelector selector(memoryTypes, memoryHeaps);

    // Device local buffers or textures.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear), 0);
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::DeviceLocal), 0);
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::DeviceLocal),
              0);

    // Write mappable buffers prefer HOST_COHERENT.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::WriteMappable),
              0);

    // Read mappable buffers prefer HOST_CACHED over HOST_COHERENT.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::ReadMappable),
              1);

    // Read+write mappable buffers prefer HOST_CACHED+HOST_COHERENT.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::ReadMappable |
                                                       MemoryKind::WriteMappable),
              1);

    // Lazily allocated textures.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::LazilyAllocated), 2);

    // BufferMapExtendedUsages write mappable buffers prefer HOST_COHERENT.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::DeviceLocal |
                                                       MemoryKind::WriteMappable),
              0);

    // BufferMapExtendedUsages read mappable buffers prefer HOST_CACHED.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::DeviceLocal |
                                                       MemoryKind::ReadMappable),
              1);

    // BufferMapExtendedUsages read+write mappable buffers prefer HOST_CACHED+HOST_COHERENT.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::DeviceLocal |
                                                       MemoryKind::ReadMappable |
                                                       MemoryKind::WriteMappable),
              1);
}

TEST(MemoryTypeSelectorTests, Adreno_620) {
    // Memory info from Adreno 620 GPU.
    std::vector<VkMemoryType> memoryTypes = {
        {VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         /*heap=*/0},
        {VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
         /*heap=*/0},
        {VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
             VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
         /*heap=*/0},
        {VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
             VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_CACHED_BIT,
         /*heap=*/0},
        {VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_PROTECTED_BIT,
         /*heap=*/1}};
    std::vector<VkMemoryHeap> memoryHeaps = {{5'727'592'448, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT},
                                             {268'435'456, VK_MEMORY_HEAP_DEVICE_LOCAL_BIT}};
    MemoryTypeSelector selector(memoryTypes, memoryHeaps);

    // Device local buffers or textures.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear), 0);
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::DeviceLocal), 0);
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::DeviceLocal),
              0);

    // Write mappable buffers prefer HOST_COHERENT.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::WriteMappable),
              1);

    // Read mappable buffers prefer HOST_CACHED over HOST_COHERENT.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::ReadMappable),
              2);

    // Read+write mappable buffers prefer HOST_CACHED+HOST_COHERENT.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::ReadMappable |
                                                       MemoryKind::WriteMappable),
              3);

    // BufferMapExtendedUsages write mappable buffers prefer HOST_COHERENT.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::DeviceLocal |
                                                       MemoryKind::WriteMappable),
              1);

    // BufferMapExtendedUsages read mappable buffers prefer HOST_CACHED.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::DeviceLocal |
                                                       MemoryKind::ReadMappable),
              2);

    // BufferMapExtendedUsages read+write mappable buffers prefer HOST_CACHED+HOST_COHERENT.
    EXPECT_EQ(selector.FindBestTypeIndex(kAnyType, MemoryKind::Linear | MemoryKind::DeviceLocal |
                                                       MemoryKind::ReadMappable |
                                                       MemoryKind::WriteMappable),
              3);
}

}  // namespace dawn::native::vulkan
