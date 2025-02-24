/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020 Advanced Micro Devices, Inc. All rights reserved.
 * Modifications Copyright (C) 2022 RasterGrid Kft.
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

#include "vulkan/vulkan.h"

namespace vvl {
class Buffer;

// Data for a given binding, that can be updated by calls like vkCmdBindVertexBuffers and vkCmdSetVertexInputEXT
struct VertexBufferBinding {
    // buffer might be VK_NULL_HANDLE because it was set or by default, we need to actually know if the command buffer binds or not
    bool bound;

    VkBuffer buffer;  // VK_NULL_HANDLE is valid if using nullDescriptor
    // Binding valid size: 0 if buffer is not tracked, actual size if VK_WHOLE_SIZE was specified,
    // clamped up to 0 if specified size is greater than the size actually left in the buffer
    VkDeviceSize effective_size;
    VkDeviceSize offset;
    VkDeviceSize stride;

    VertexBufferBinding() : bound(false), buffer(VK_NULL_HANDLE), effective_size(0), offset(0), stride(0) {}

    void reset() { *this = VertexBufferBinding(); }
};

struct IndexBufferBinding {
    VkBuffer buffer;  // VK_NULL_HANDLE is valid if using nullDescriptor
    VkDeviceSize size;
    VkDeviceSize offset;
    VkIndexType index_type;

    IndexBufferBinding() : buffer(VK_NULL_HANDLE), size(0), offset(0), index_type(static_cast<VkIndexType>(0)) {}
    IndexBufferBinding(VkBuffer buffer_, VkDeviceSize size_, VkDeviceSize offset_, VkIndexType index_type_)
        : buffer(buffer_), size(size_), offset(offset_), index_type(index_type_) {}

    void reset() { *this = IndexBufferBinding(); }
};

}  // namespace vvl