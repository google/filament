// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef VK_CONFIG_HPP_
#define VK_CONFIG_HPP_

#include "Version.hpp"
#include "Device/Config.hpp"
#include "Vulkan/VulkanPlatform.hpp"
#include "spirv-tools/libspirv.h"

#ifndef SWIFTSHADER_LEGACY_PRECISION
#	define SWIFTSHADER_LEGACY_PRECISION false
#endif

#ifndef SWIFTSHADER_DEVICE_MEMORY_ALLOCATION_ALIGNMENT
#	define SWIFTSHADER_DEVICE_MEMORY_ALLOCATION_ALIGNMENT 256
#endif

namespace vk {

// Note: Constant array initialization requires a string literal.
//       constexpr char* or char[] does not work for that purpose.
#define SWIFTSHADER_DEVICE_NAME "SwiftShader Device"  // Max length: VK_MAX_PHYSICAL_DEVICE_NAME_SIZE
#define SWIFTSHADER_UUID "SwiftShaderUUID"            // Max length: VK_UUID_SIZE (16)

constexpr spv_target_env SPIRV_VERSION = SPV_ENV_VULKAN_1_3;

constexpr uint32_t API_VERSION = VK_API_VERSION_1_3;
constexpr uint32_t DRIVER_VERSION = VK_MAKE_VERSION(MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION);
constexpr uint32_t VENDOR_ID = 0x1AE0;  // Google, Inc.: https://pcisig.com/google-inc-1
constexpr uint32_t DEVICE_ID = 0xC0DE;  // SwiftShader (placeholder)

// "Allocations returned by vkAllocateMemory are guaranteed to meet any alignment requirement of the implementation."
constexpr VkDeviceSize DEVICE_MEMORY_ALLOCATION_ALIGNMENT = SWIFTSHADER_DEVICE_MEMORY_ALLOCATION_ALIGNMENT;

constexpr VkDeviceSize MIN_MEMORY_MAP_ALIGNMENT = 64;
static_assert(DEVICE_MEMORY_ALLOCATION_ALIGNMENT >= MIN_MEMORY_MAP_ALIGNMENT);

constexpr VkDeviceSize MIN_IMPORTED_HOST_POINTER_ALIGNMENT = 4096;
static_assert(MIN_IMPORTED_HOST_POINTER_ALIGNMENT >= DEVICE_MEMORY_ALLOCATION_ALIGNMENT);

// Vulkan 1.2 requires buffer offset alignment to be at most 256.
constexpr VkDeviceSize MIN_TEXEL_BUFFER_OFFSET_ALIGNMENT = 256;
constexpr VkDeviceSize MIN_UNIFORM_BUFFER_OFFSET_ALIGNMENT = 256;
constexpr VkDeviceSize MIN_STORAGE_BUFFER_OFFSET_ALIGNMENT = 256;
static_assert(DEVICE_MEMORY_ALLOCATION_ALIGNMENT >= MIN_TEXEL_BUFFER_OFFSET_ALIGNMENT);
static_assert(DEVICE_MEMORY_ALLOCATION_ALIGNMENT >= MIN_UNIFORM_BUFFER_OFFSET_ALIGNMENT);
static_assert(DEVICE_MEMORY_ALLOCATION_ALIGNMENT >= MIN_STORAGE_BUFFER_OFFSET_ALIGNMENT);

// Alignment of all other Vulkan resources.
constexpr VkDeviceSize MEMORY_REQUIREMENTS_OFFSET_ALIGNMENT = 16;  // 16 bytes for 128-bit vector types.
static_assert(DEVICE_MEMORY_ALLOCATION_ALIGNMENT >= MEMORY_REQUIREMENTS_OFFSET_ALIGNMENT);

constexpr VkDeviceSize HOST_MEMORY_ALLOCATION_ALIGNMENT = 16;  // 16 bytes for 128-bit vector types.

constexpr uint32_t MEMORY_TYPE_GENERIC_BIT = 0x1;  // Generic system memory.

constexpr uint32_t MAX_IMAGE_LEVELS_1D = 15;
constexpr uint32_t MAX_IMAGE_LEVELS_2D = 15;
constexpr uint32_t MAX_IMAGE_LEVELS_3D = 12;
constexpr uint32_t MAX_IMAGE_LEVELS_CUBE = 15;
constexpr uint32_t MAX_IMAGE_ARRAY_LAYERS = 2048;
constexpr float MAX_SAMPLER_LOD_BIAS = 15.0;

static_assert(MAX_IMAGE_LEVELS_1D <= sw::MIPMAP_LEVELS);
static_assert(MAX_IMAGE_LEVELS_2D <= sw::MIPMAP_LEVELS);
static_assert(MAX_IMAGE_LEVELS_3D <= sw::MIPMAP_LEVELS);
static_assert(MAX_IMAGE_LEVELS_CUBE <= sw::MIPMAP_LEVELS);

constexpr uint32_t MAX_BOUND_DESCRIPTOR_SETS = 4;
constexpr uint32_t MAX_VERTEX_INPUT_BINDINGS = 16;
constexpr uint32_t MAX_PUSH_CONSTANT_SIZE = 128;
constexpr uint32_t MAX_UPDATE_AFTER_BIND_DESCRIPTORS = 500000;

constexpr uint32_t MAX_DESCRIPTOR_SET_UNIFORM_BUFFERS_DYNAMIC = 8;
constexpr uint32_t MAX_DESCRIPTOR_SET_STORAGE_BUFFERS_DYNAMIC = 4;
constexpr uint32_t MAX_DESCRIPTOR_SET_COMBINED_BUFFERS_DYNAMIC =
    MAX_DESCRIPTOR_SET_UNIFORM_BUFFERS_DYNAMIC +
    MAX_DESCRIPTOR_SET_STORAGE_BUFFERS_DYNAMIC;

constexpr uint32_t MAX_COMPUTE_WORKGROUP_INVOCATIONS = 256;

constexpr size_t MAX_INLINE_UNIFORM_BLOCK_SIZE = 256;

constexpr float MAX_POINT_SIZE = 1023.0;

constexpr int MAX_SAMPLER_ALLOCATION_COUNT = 4000;

constexpr int SUBPIXEL_PRECISION_BITS = SWIFTSHADER_LEGACY_PRECISION ? 4 : 8;
constexpr float SUBPIXEL_PRECISION_FACTOR = static_cast<float>(1 << SUBPIXEL_PRECISION_BITS);
constexpr int SUBPIXEL_PRECISION_MASK = 0xFFFFFFFF >> (32 - SUBPIXEL_PRECISION_BITS);

constexpr int MAX_VIEWPORTS = 16;

// TODO: The heap size should be configured based on available RAM.
constexpr VkDeviceSize PHYSICAL_DEVICE_HEAP_SIZE = 0x80000000ull;   // 0x80000000 = 2 GiB
constexpr VkDeviceSize MAX_MEMORY_ALLOCATION_SIZE = 0x40000000ull;  // 0x40000000 = 1 GiB

// Memory offset calculations in 32-bit SIMD elements limit us to addressing at most 4 GiB.
// Signed arithmetic further restricts it to 2 GiB.
static_assert(MAX_MEMORY_ALLOCATION_SIZE <= 0x80000000ull, "maxMemoryAllocationSize must not exceed 2 GiB");

}  // namespace vk

#if defined(__linux__) && !defined(__ANDROID__)
#	define SWIFTSHADER_EXTERNAL_MEMORY_OPAQUE_FD 1
#	define SWIFTSHADER_EXTERNAL_SEMAPHORE_OPAQUE_FD 1
#elif defined(__ANDROID__)
#	define SWIFTSHADER_EXTERNAL_SEMAPHORE_OPAQUE_FD 1
#endif
#if defined(__APPLE__)
#	define SWIFTSHADER_EXTERNAL_MEMORY_OPAQUE_FD 1
#endif

#endif  // VK_CONFIG_HPP_
