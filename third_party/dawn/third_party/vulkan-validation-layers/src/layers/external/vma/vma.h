/* Copyright (c) 2023-2025 The Khronos Group Inc.
 * Copyright (c) 2023-2025 Valve Corporation
 * Copyright (c) 2023-2025 LunarG, Inc.
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

#include <vulkan/vulkan.h>

#define VMA_STATIC_VULKAN_FUNCTIONS 0  // We will supply Vulkan function pointers at initialization
// TODO - Currently using 1.4 (or VK_KHR_maintenance4) will blow up in vmaFindMemoryTypeIndexForBufferInfo
#define VMA_VULKAN_VERSION 1001000  // Vulkan 1.1
#define VMA_KHR_MAINTENANCE4 0
// Currently fails compiler errors if on (and not currently using it for GPU-AV)
#define VMA_EXTERNAL_MEMORY_WIN32 0

#ifdef _MSVC_LANG

#pragma warning(push, 4)
#pragma warning(disable : 4127)  // conditional expression is constant
#pragma warning(disable : 4100)  // unreferenced formal parameter
#pragma warning(disable : 4189)  // local variable is initialized but not referenced
#pragma warning(disable : 4324)  // structure was padded due to alignment specifier
#pragma warning(disable : 4820)  // 'X': 'N' bytes padding added after data member 'X'

#endif  // #ifdef _MSVC_LANG

// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
// https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/
// https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator/blob/master/src/VmaUsage.h
#include "vma/vk_mem_alloc.h"

#ifdef _MSVC_LANG
#pragma warning(pop)
#endif
