/* Copyright (c) 2015-2016, 2020-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2016, 2020-2025 Valve Corporation
 * Copyright (c) 2015-2016, 2020-2025 LunarG, Inc.
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

#include "vk_layer_utils.h"

#include <string.h>
#include <sys/stat.h>
#include <vulkan/vk_enum_string_helper.h>

#include "containers/range_vector.h"
#include "vulkan/vulkan_core.h"
#include "vk_layer_config.h"

VkLayerInstanceCreateInfo *GetChainInfo(const VkInstanceCreateInfo *pCreateInfo, VkLayerFunction func) {
    VkLayerInstanceCreateInfo *chain_info = (VkLayerInstanceCreateInfo *)pCreateInfo->pNext;
    while (chain_info && !(chain_info->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO && chain_info->function == func)) {
        chain_info = (VkLayerInstanceCreateInfo *)chain_info->pNext;
    }
    assert(chain_info != NULL);
    return chain_info;
}

VkLayerDeviceCreateInfo *GetChainInfo(const VkDeviceCreateInfo *pCreateInfo, VkLayerFunction func) {
    VkLayerDeviceCreateInfo *chain_info = (VkLayerDeviceCreateInfo *)pCreateInfo->pNext;
    while (chain_info && !(chain_info->sType == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO && chain_info->function == func)) {
        chain_info = (VkLayerDeviceCreateInfo *)chain_info->pNext;
    }
    assert(chain_info != NULL);
    return chain_info;
}

std::string GetTempFilePath() {
    auto tmp_path = GetEnvironment("XDG_CACHE_HOME");
    if (!tmp_path.size()) {
        auto cachepath = GetEnvironment("HOME") + "/.cache";
        struct stat info;
        if (stat(cachepath.c_str(), &info) == 0) {
            if ((info.st_mode & S_IFMT) == S_IFDIR) {
                tmp_path = cachepath;
            }
        }
    }
    if (!tmp_path.size()) tmp_path = GetEnvironment("TMPDIR");
    if (!tmp_path.size()) tmp_path = GetEnvironment("TMP");
    if (!tmp_path.size()) tmp_path = GetEnvironment("TEMP");
    if (!tmp_path.size()) tmp_path = "/tmp";
    return tmp_path;
}

// Returns the effective extent of an image subresource, adjusted for mip level and array depth.
VkExtent3D GetEffectiveExtent(const VkImageCreateInfo &ci, const VkImageAspectFlags aspect_mask, const uint32_t mip_level) {
    // Return zero extent if mip level doesn't exist
    if (mip_level >= ci.mipLevels) {
        return VkExtent3D{0, 0, 0};
    }

    VkExtent3D extent = ci.extent;

    // If multi-plane, adjust per-plane extent
    const VkFormat format = ci.format;
    if (vkuFormatIsMultiplane(format)) {
        VkExtent2D divisors = vkuFindMultiplaneExtentDivisors(format, static_cast<VkImageAspectFlagBits>(aspect_mask));
        extent.width /= divisors.width;
        extent.height /= divisors.height;
    }

    // Mip Maps
    {
        const uint32_t corner = (ci.flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV) ? 1 : 0;
        const uint32_t min_size = 1 + corner;
        const std::array dimensions = {&extent.width, &extent.height, &extent.depth};
        for (uint32_t *dim : dimensions) {
            // Don't allow mip adjustment to create 0 dim, but pass along a 0 if that's what subresource specified
            if (*dim == 0) {
                continue;
            }
            *dim >>= mip_level;
            *dim = std::max(min_size, *dim);
        }
    }

    // Image arrays have an effective z extent that isn't diminished by mip level
    if (VK_IMAGE_TYPE_3D != ci.imageType) {
        extent.depth = ci.arrayLayers;
    }

    return extent;
}

// Returns true if [x, x + x_size) and [y, y + y_size) overlap
bool RangesIntersect(int64_t x, uint64_t x_size, int64_t y, uint64_t y_size) {
    auto intersection = GetRangeIntersection(x, x_size, y, y_size);
    return intersection.non_empty();
}

// Implements the vkspec.html#formats-size-compatibility section of the spec
bool AreFormatsSizeCompatible(VkFormat a, VkFormat b) {
    const bool is_a_a8 = a == VK_FORMAT_A8_UNORM;
    const bool is_b_a8 = b == VK_FORMAT_A8_UNORM;
    if ((is_a_a8 && !is_b_a8) || (!is_a_a8 && is_b_a8)) {
        return false;
    }

    const bool is_a_depth_stencil = vkuFormatIsDepthOrStencil(a);
    const bool is_b_depth_stencil = vkuFormatIsDepthOrStencil(b);
    if (is_a_depth_stencil && !is_b_depth_stencil) {
        return vkuFormatIsDepthStencilWithColorSizeCompatible(b, a);
    } else if (!is_a_depth_stencil && is_b_depth_stencil) {
        return vkuFormatIsDepthStencilWithColorSizeCompatible(a, b);
    } else if (is_a_depth_stencil && is_b_depth_stencil) {
        return a == b;
    }

    // Color formats are considered compatible if their texel block size in bytes is the same
    return vkuFormatTexelBlockSize(a) == vkuFormatTexelBlockSize(b);
}

std::string DescribeFormatsSizeCompatible(VkFormat a, VkFormat b) {
    std::stringstream ss;
    const bool is_a_a8 = a == VK_FORMAT_A8_UNORM;
    const bool is_b_a8 = b == VK_FORMAT_A8_UNORM;
    if ((is_a_a8 && !is_b_a8) || (!is_a_a8 && is_b_a8)) {
        ss << string_VkFormat(a) << " and " << string_VkFormat(b)
           << " either both need to be VK_FORMAT_A8_UNORM or neither of them";
        return ss.str();
    }

    const bool is_a_depth_stencil = vkuFormatIsDepthOrStencil(a);
    const bool is_b_depth_stencil = vkuFormatIsDepthOrStencil(b);
    if (is_a_depth_stencil && is_b_depth_stencil) {
        ss << string_VkFormat(a) << " and " << string_VkFormat(b)
           << " are both depth/stencil, therefor they must be the exact same format";
    } else if (is_a_depth_stencil || is_b_depth_stencil) {
        if (is_a_depth_stencil && !is_b_depth_stencil) {
            ss << string_VkFormat(a) << " is a depth/stencil and " << string_VkFormat(b) << " is color";
        } else if (!is_a_depth_stencil && is_b_depth_stencil) {
            ss << string_VkFormat(a) << " is a color and " << string_VkFormat(b) << " is depth/stencil";
        }
        ss << " (this is only allowed with a certain set of formats during image copy with VK_KHR_maintenance8)";
    } else {
        ss << string_VkFormat(a) << " has a texel block size of " << vkuFormatTexelBlockSize(a) << " while " << string_VkFormat(b)
           << " has a texel block size of " << vkuFormatTexelBlockSize(b);
    }
    return ss.str();
}