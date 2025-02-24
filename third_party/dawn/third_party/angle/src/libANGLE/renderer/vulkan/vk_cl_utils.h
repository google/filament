//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_cl_utils:
//    Helper functions for the Vulkan Renderer in translation of vk state from/to cl state.
//

#ifndef LIBANGLE_RENDERER_VULKAN_CL_VK_UTILS_H_
#define LIBANGLE_RENDERER_VULKAN_CL_VK_UTILS_H_

#include "common/PackedCLEnums_autogen.h"

#include "libANGLE/CLBitField.h"
#include "libANGLE/cl_types.h"

#include "vulkan/vulkan_core.h"

namespace rx
{
namespace cl_vk
{
VkExtent3D GetExtent(const cl::Extents &extent);
VkOffset3D GetOffset(const cl::Offset &offset);
VkImageType GetImageType(cl::MemObjectType memObjectType);
VkImageViewType GetImageViewType(cl::MemObjectType memObjectType);
VkMemoryPropertyFlags GetMemoryPropertyFlags(cl::MemFlags memFlags);
VkBufferUsageFlags GetBufferUsageFlags(cl::MemFlags memFlags);

}  // namespace cl_vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_CL_VK_UTILS_H_
