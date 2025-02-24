/* Copyright (c) 2015-2017, 2019-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2017, 2019-2025 Valve Corporation
 * Copyright (c) 2015-2017, 2019-2025 LunarG, Inc.
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

#include <algorithm>
#include <atomic>
#include <bitset>
#include <cassert>
#include <cctype>
#include <cmath>
#include <cstring>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <type_traits>
#include <vector>

#include <vulkan/utility/vk_format_utils.h>
#include <vulkan/utility/vk_concurrent_unordered_map.hpp>
#include <vulkan/utility/vk_struct_helper.hpp>
#include "vulkan/vk_layer.h"

#include "generated/vk_layer_dispatch_table.h"

#ifndef WIN32
#include <strings.h>  // For ffs()
#else
#include <intrin.h>  // For __lzcnt()
#endif

#define STRINGIFY(s) STRINGIFY_HELPER(s)
#define STRINGIFY_HELPER(s) #s

#if defined __PRETTY_FUNCTION__
#define VVL_PRETTY_FUNCTION __PRETTY_FUNCTION__
#else
// For MSVC
#if defined(__FUNCSIG__)
#define VVL_PRETTY_FUNCTION __FUNCSIG__
#else
#define VVL_PRETTY_FUNCTION __FILE__ ":" STRINGIFY(__LINE__)
#endif
#endif

// There are many times we want to assert, but also it is highly important to not crash for release builds.
// This Macro also makes it more obvious if we are returning early because of a known situation or if we are just guarding against
// something wrong actually happening.
#define ASSERT_AND_RETURN(cond) \
    do {                        \
        if (!(cond)) {          \
            assert(false);      \
            return;             \
        }                       \
    } while (0)

#define ASSERT_AND_RETURN_SKIP(cond) \
    do {                             \
        if (!(cond)) {               \
            assert(false);           \
            return skip;             \
        }                            \
    } while (0)

#define ASSERT_AND_CONTINUE(cond) \
    if (!(cond)) {                \
        assert(false);            \
        continue;                 \
    }

static inline VkExtent3D CastTo3D(const VkExtent2D &d2) {
    VkExtent3D d3 = {d2.width, d2.height, 1};
    return d3;
}

static inline VkOffset3D CastTo3D(const VkOffset2D &d2) {
    VkOffset3D d3 = {d2.x, d2.y, 0};
    return d3;
}

// It is very rare to have more than 3 stages (really only geo/tess) and better to save memory/time for the 99% use cases
static const uint32_t kCommonMaxGraphicsShaderStages = 3;

typedef void *dispatch_key;
static inline dispatch_key GetDispatchKey(const void *object) { return (dispatch_key) * (VkLayerDispatchTable **)object; }

VkLayerInstanceCreateInfo *GetChainInfo(const VkInstanceCreateInfo *pCreateInfo, VkLayerFunction func);
VkLayerDeviceCreateInfo *GetChainInfo(const VkDeviceCreateInfo *pCreateInfo, VkLayerFunction func);

template <typename T>
constexpr bool IsPowerOfTwo(T x) {
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "Unsigned integer required");
    return x && !(x & (x - 1));
}

template <typename T>
constexpr uint32_t GetBitSetCount(T value) {
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "Unsigned integer required");
    static_assert(sizeof(T) == 4 || sizeof(T) == 8, "32 or 64 bit value is expected");
    return static_cast<uint32_t>(std::bitset<sizeof(T) * 8>(value).count());
}

template <typename T>
constexpr bool IsSingleBitSet(T flags) {
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "Unsigned integer required");
    return IsPowerOfTwo(flags);
}

// Returns the 0-based index of the MSB, like the x86 bit scan reverse (bsr) instruction
// Note: an input mask of 0 yields -1
static inline int MostSignificantBit(uint32_t mask) {
#if defined __GNUC__
    return mask ? __builtin_clz(mask) ^ 31 : -1;
#elif defined _MSC_VER
    unsigned long bit_pos;
    return _BitScanReverse(&bit_pos, mask) ? int(bit_pos) : -1;
#else
    for (int k = 31; k >= 0; --k) {
        if (((mask >> k) & 1) != 0) {
            return k;
        }
    }
    return -1;
#endif
}

static inline int u_ffs(int val) {
#ifdef WIN32
    unsigned long bit_pos = 0;
    if (_BitScanForward(&bit_pos, val) != 0) {
        bit_pos += 1;
    }
    return bit_pos;
#else
    return ffs(val);
#endif
}

// Given p2 a power of two, returns smallest multiple of p2 greater than or equal to x
// Different than std::align in that it simply aligns an unsigned integer, when std::align aligns a virtual address and does the
// necessary bookkeeping to be able to correctly free memory at the new address
template <typename T>
constexpr T Align(T x, T p2) {
    static_assert(std::numeric_limits<T>::is_integer, "Unsigned integer required.");
    static_assert(std::is_unsigned<T>::value, "Unsigned integer required.");
    assert(IsPowerOfTwo(p2));
    return (x + p2 - 1) & ~(p2 - 1);
}

// Returns the 0-based index of the LSB. An input mask of 0 yields -1
static inline int LeastSignificantBit(uint32_t mask) { return u_ffs(static_cast<int>(mask)) - 1; }

template <typename FlagBits, typename Flags>
FlagBits LeastSignificantFlag(Flags flags) {
    const int bit_shift = LeastSignificantBit(flags);
    assert(bit_shift != -1);
    return static_cast<FlagBits>(1ull << bit_shift);
}

// Iterates over all set bits and calls the callback with a bit mask corresponding to each flag.
// FlagBits and Flags follow Vulkan naming convensions for flag types.
// An example of a more efficient implementation: https://lemire.me/blog/2018/02/21/iterating-over-set-bits-quickly/
template <typename FlagBits, typename Flags, typename Callback>
void IterateFlags(Flags flags, Callback callback) {
    uint32_t bit_shift = 0;
    while (flags) {
        if (flags & 1) {
            callback(static_cast<FlagBits>(1ull << bit_shift));
        }
        flags >>= 1;
        ++bit_shift;
    }
}

static inline uint32_t SampleCountSize(VkSampleCountFlagBits sample_count) {
    uint32_t size = 0;
    switch (sample_count) {
        case VK_SAMPLE_COUNT_1_BIT:
            size = 1;
            break;
        case VK_SAMPLE_COUNT_2_BIT:
            size = 2;
            break;
        case VK_SAMPLE_COUNT_4_BIT:
            size = 4;
            break;
        case VK_SAMPLE_COUNT_8_BIT:
            size = 8;
            break;
        case VK_SAMPLE_COUNT_16_BIT:
            size = 16;
            break;
        case VK_SAMPLE_COUNT_32_BIT:
            size = 32;
            break;
        case VK_SAMPLE_COUNT_64_BIT:
            size = 64;
            break;
        default:
            size = 0;
    }
    return size;
}

static inline bool IsImageLayoutReadOnly(VkImageLayout layout) {
    constexpr std::array read_only_layouts = {
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
    };
    return std::any_of(read_only_layouts.begin(), read_only_layouts.end(),
                       [layout](const VkImageLayout read_only_layout) { return layout == read_only_layout; });
}

static inline bool IsImageLayoutDepthOnly(VkImageLayout layout) {
    constexpr std::array depth_only_layouts = {VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL};
    return std::any_of(depth_only_layouts.begin(), depth_only_layouts.end(),
                       [layout](const VkImageLayout read_only_layout) { return layout == read_only_layout; });
}

static inline bool IsImageLayoutDepthReadOnly(VkImageLayout layout) {
    constexpr std::array read_only_layouts = {
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
    };
    return std::any_of(read_only_layouts.begin(), read_only_layouts.end(),
                       [layout](const VkImageLayout read_only_layout) { return layout == read_only_layout; });
}

static inline bool IsImageLayoutStencilOnly(VkImageLayout layout) {
    constexpr std::array depth_only_layouts = {VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL,
                                               VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL};
    return std::any_of(depth_only_layouts.begin(), depth_only_layouts.end(),
                       [layout](const VkImageLayout read_only_layout) { return layout == read_only_layout; });
}

static inline bool IsImageLayoutStencilReadOnly(VkImageLayout layout) {
    constexpr std::array read_only_layouts = {
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL,
        VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL,
    };
    return std::any_of(read_only_layouts.begin(), read_only_layouts.end(),
                       [layout](const VkImageLayout read_only_layout) { return layout == read_only_layout; });
}

static inline bool IsIdentitySwizzle(VkComponentMapping components) {
    // clang-format off
    return (
        ((components.r == VK_COMPONENT_SWIZZLE_IDENTITY) || (components.r == VK_COMPONENT_SWIZZLE_R)) &&
        ((components.g == VK_COMPONENT_SWIZZLE_IDENTITY) || (components.g == VK_COMPONENT_SWIZZLE_G)) &&
        ((components.b == VK_COMPONENT_SWIZZLE_IDENTITY) || (components.b == VK_COMPONENT_SWIZZLE_B)) &&
        ((components.a == VK_COMPONENT_SWIZZLE_IDENTITY) || (components.a == VK_COMPONENT_SWIZZLE_A))
    );
    // clang-format on
}

static inline uint32_t GetIndexAlignment(VkIndexType indexType) {
    switch (indexType) {
        case VK_INDEX_TYPE_UINT16:
            return 2;
        case VK_INDEX_TYPE_UINT32:
            return 4;
        case VK_INDEX_TYPE_UINT8:
            return 1;
        case VK_INDEX_TYPE_NONE_KHR:  // alias VK_INDEX_TYPE_NONE_NV
            return 0;
        default:
            // Not a real index type. Express no alignment requirement here; we expect upper layer
            // to have already picked up on the enum being nonsense.
            return 1;
    }
}

inline constexpr uint32_t GetIndexBitsSize(VkIndexType indexType) {
    switch (indexType) {
        case VK_INDEX_TYPE_UINT16:
            return 16;
        case VK_INDEX_TYPE_UINT32:
            return 32;
        case VK_INDEX_TYPE_NONE_KHR:
            return 0;
        case VK_INDEX_TYPE_UINT8_KHR:
            return 8;
        case VK_INDEX_TYPE_MAX_ENUM:
            return 0;
    }
    return 0;
}

// vkspec.html#formats-planes-image-aspect
static inline bool IsValidPlaneAspect(VkFormat format, VkImageAspectFlags aspect_mask) {
    const uint32_t planes = vkuFormatPlaneCount(format);
    constexpr VkImageAspectFlags valid_planes =
        VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT;

    if (((aspect_mask & valid_planes) == aspect_mask) && (aspect_mask != 0)) {
        if ((planes == 3) || ((planes == 2) && ((aspect_mask & VK_IMAGE_ASPECT_PLANE_2_BIT) == 0))) {
            return true;
        }
    }
    return false;  // Expects calls to make sure it is a multi-planar format
}

static inline bool IsOnlyOneValidPlaneAspect(VkFormat format, VkImageAspectFlags aspect_mask) {
    const bool multiple_bits = aspect_mask != 0 && !IsPowerOfTwo(aspect_mask);
    return !multiple_bits && IsValidPlaneAspect(format, aspect_mask);
}

static inline bool IsMultiplePlaneAspect(VkImageAspectFlags aspect_mask) {
    // If checking for multiple planes, there will already be another check if valid for plane count
    constexpr VkImageAspectFlags valid_planes =
        VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT;
    const VkImageAspectFlags planes = aspect_mask & valid_planes;
    return planes != 0 && !IsPowerOfTwo(planes);
}

static inline bool IsAnyPlaneAspect(VkImageAspectFlags aspect_mask) {
    constexpr VkImageAspectFlags valid_planes =
        VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT;
    return (aspect_mask & valid_planes) != 0;
}

static inline uint32_t GetVertexInputFormatSize(VkFormat format) {
    // Vertex input attributes use VkFormat, but only to make use of how they define sizes, things such as
    // depth/multi-plane/compressed will never be used here because they would mean nothing. So we can ensure these are "standard"
    // color formats being used. This function is a wrapper to make it more clear of the intent.
    return vkuFormatTexelBlockSize(format);
}

static inline uint32_t GetTexelBufferFormatSize(VkFormat format) {
    // The spec says "If format is a block-compressed format, then bufferFeatures must not support any features for the format"
    // For Texel Buffers, we can assume the texel blocks are a 1x1x1 extent
    // See https://gitlab.khronos.org/vulkan/vulkan/-/issues/4155 for more details
    return vkuFormatTexelBlockSize(format);
}

bool AreFormatsSizeCompatible(VkFormat a, VkFormat b);
std::string DescribeFormatsSizeCompatible(VkFormat a, VkFormat b);

static const VkShaderStageFlags kShaderStageAllGraphics =
    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT |
    VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_TASK_BIT_EXT | VK_SHADER_STAGE_MESH_BIT_EXT;

static const VkShaderStageFlags kShaderStageAllRayTracing =
    VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CALLABLE_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR |
    VK_SHADER_STAGE_INTERSECTION_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_RAYGEN_BIT_KHR;

static bool inline IsStageInPipelineBindPoint(VkShaderStageFlags stages, VkPipelineBindPoint bind_point) {
    switch (bind_point) {
        case VK_PIPELINE_BIND_POINT_GRAPHICS:
            return (stages & kShaderStageAllGraphics) != 0;
        case VK_PIPELINE_BIND_POINT_COMPUTE:
            return (stages & VK_SHADER_STAGE_COMPUTE_BIT) != 0;
        case VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR:
            return (stages & kShaderStageAllRayTracing) != 0;
        default:
            return false;
    }
}

// all "advanced blend operation" found in spec
static inline bool IsAdvanceBlendOperation(const VkBlendOp blend_op) {
    return (static_cast<int>(blend_op) >= VK_BLEND_OP_ZERO_EXT) && (static_cast<int>(blend_op) <= VK_BLEND_OP_BLUE_EXT);
}

// Helper for Dual-Source Blending
static inline bool IsSecondaryColorInputBlendFactor(VkBlendFactor blend_factor) {
    return (blend_factor == VK_BLEND_FACTOR_SRC1_COLOR || blend_factor == VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR ||
            blend_factor == VK_BLEND_FACTOR_SRC1_ALPHA || blend_factor == VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA);
}

// Check if size is in range
static inline bool IsBetweenInclusive(VkDeviceSize value, VkDeviceSize min, VkDeviceSize max) {
    return (value >= min) && (value <= max);
}

static inline bool IsBetweenInclusive(const VkExtent2D &value, const VkExtent2D &min, const VkExtent2D &max) {
    return IsBetweenInclusive(value.width, min.width, max.width) && IsBetweenInclusive(value.height, min.height, max.height);
}

static inline bool IsBetweenInclusive(float value, float min, float max) { return (value >= min) && (value <= max); }

// Check if value is integer multiple of granularity
static inline bool IsIntegerMultipleOf(VkDeviceSize value, VkDeviceSize granularity) {
    if (granularity == 0) {
        return value == 0;
    } else {
        return (value % granularity) == 0;
    }
}

static inline bool IsIntegerMultipleOf(const VkOffset2D &value, const VkOffset2D &granularity) {
    return IsIntegerMultipleOf(value.x, granularity.x) && IsIntegerMultipleOf(value.y, granularity.y);
}

// Perform a zero-tolerant modulo operation
static inline VkDeviceSize SafeModulo(VkDeviceSize dividend, VkDeviceSize divisor) {
    VkDeviceSize result = 0;
    if (divisor != 0) {
        result = dividend % divisor;
    }
    return result;
}

static inline VkDeviceSize SafeDivision(VkDeviceSize dividend, VkDeviceSize divisor) {
    VkDeviceSize result = 0;
    if (divisor != 0) {
        result = dividend / divisor;
    }
    return result;
}

static inline uint32_t FullMipChainLevels(VkExtent3D extent) {
    // uint cast applies floor()
    return 1u + static_cast<uint32_t>(log2(std::max({extent.height, extent.width, extent.depth})));
}

// Returns the effective extent of an image subresource, adjusted for mip level and array depth.
VkExtent3D GetEffectiveExtent(const VkImageCreateInfo &ci, const VkImageAspectFlags aspect_mask, const uint32_t mip_level);

// Used to get the VkExternalFormatANDROID without having to use ifdef in logic
// Result of zero is same of not having pNext struct
constexpr uint64_t GetExternalFormat(const void *pNext) {
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    if (pNext) {
        const auto *external_format = vku::FindStructInPNextChain<VkExternalFormatANDROID>(pNext);
        if (external_format) {
            return external_format->externalFormat;
        }
    }
#endif
    (void)pNext;
    return 0;
}

// Find whether or not an element is in list
// Two definitions, to be able to do the following calls:
// IsValueIn(1, {1, 2, 3});
// std::array arr {1, 2, 3};
// IsValueIn(1, arr);
template <typename T, typename RANGE>
bool IsValueIn(const T &v, const RANGE &range) {
    return std::find(std::begin(range), std::end(range), v) != std::end(range);
}

template <typename T>
bool IsValueIn(const T &v, const std::initializer_list<T> &list) {
    return IsValueIn<T, decltype(list)>(v, list);
}

#define VK_LAYER_API_VERSION VK_HEADER_VERSION_COMPLETE

typedef enum VkStringErrorFlagBits {
    VK_STRING_ERROR_NONE = 0x00000000,
    VK_STRING_ERROR_LENGTH = 0x00000001,
    VK_STRING_ERROR_BAD_DATA = 0x00000002,
} VkStringErrorFlagBits;
typedef VkFlags VkStringErrorFlags;

std::string GetTempFilePath();

// Aliases to avoid excessive typing. We can't easily auto these away because
// there are virtual methods in vvl::base::Device which return lock guards
// and those cannot use return type deduction.
typedef std::shared_lock<std::shared_mutex> ReadLockGuard;
typedef std::unique_lock<std::shared_mutex> WriteLockGuard;

// helper class for the very common case of getting and then locking a command buffer (or other state object)
template <typename T, typename Guard>
class LockedSharedPtr : public std::shared_ptr<T> {
  public:
    LockedSharedPtr(std::shared_ptr<T> &&ptr, Guard &&guard) : std::shared_ptr<T>(std::move(ptr)), guard_(std::move(guard)) {}
    LockedSharedPtr() : std::shared_ptr<T>(), guard_() {}

  private:
    Guard guard_;
};

static constexpr VkPipelineStageFlags2KHR kFramebufferStagePipelineStageFlags =
    (VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
     VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

static constexpr VkAccessFlags2 kShaderTileImageAllowedAccessFlags =
    VK_ACCESS_2_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
    VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

static constexpr bool HasNonFramebufferStagePipelineStageFlags(VkPipelineStageFlags2KHR inflags) {
    return (inflags & ~kFramebufferStagePipelineStageFlags) != 0;
}

static constexpr bool HasFramebufferStagePipelineStageFlags(VkPipelineStageFlags2KHR inflags) {
    return (inflags & kFramebufferStagePipelineStageFlags) != 0;
}

static constexpr bool HasNonShaderTileImageAccessFlags(VkAccessFlags2 in_flags) {
    return ((in_flags & ~kShaderTileImageAllowedAccessFlags) != 0);
}

bool RangesIntersect(int64_t x, uint64_t x_size, int64_t y, uint64_t y_size);

namespace vvl {

// The standard does not specify the value of data() for zero-sized contatiners as being null or non-null,
// only that it is not dereferenceable.
//
// Vulkan VUID's OTOH frequently require NULLs for zero-sized entries, or for option entries with non-zero counts
template <typename T>
const typename T::value_type *DataOrNull(const T &container) {
    if (!container.empty()) {
        return container.data();
    }
    return nullptr;
}

// Workaround for static_assert(false) before C++ 23 arrives
// https://en.cppreference.com/w/cpp/language/static_assert
// https://cplusplus.github.io/CWG/issues/2518.html
template <typename>
inline constexpr bool dependent_false_v = false;

// Until C++ 26 std::atomic<T>::fetch_max arrives
// https://en.cppreference.com/w/cpp/atomic/atomic/fetch_max
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2024/p0493r5.pdf
template <typename T>
inline T atomic_fetch_max(std::atomic<T> &current_max, const T &value) noexcept {
    T t = current_max.load();
    while (!current_max.compare_exchange_weak(t, std::max(t, value)))
        ;
    return t;
}

}  // namespace vvl
