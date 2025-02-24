/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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

#include <string>
#include <sstream>
#include <vector>

#include "core_validation.h"
#include "cc_vuid_maps.h"
#include "error_message/error_location.h"
#include "error_message/error_strings.h"
#include <vulkan/utility/vk_format_utils.h>
#include <vulkan/vk_enum_string_helper.h>
#include "state_tracker/image_state.h"
#include "state_tracker/buffer_state.h"
#include "state_tracker/device_state.h"
#include "generated/dispatch_functions.h"
#include "utils/vk_layer_utils.h"

struct ImageRegionIntersection {
    VkImageSubresourceLayers subresource = {};
    VkOffset3D offset = {0, 0, 0};
    VkExtent3D extent = {1, 1, 1};
    bool has_instersection = false;
    std::string String() const noexcept {
        std::stringstream ss;
        ss << "\nsubresource : { aspectMask: " << string_VkImageAspectFlags(subresource.aspectMask)
           << ", mipLevel: " << subresource.mipLevel << ", baseArrayLayer: " << subresource.baseArrayLayer
           << ", layerCount: " << subresource.layerCount << " },\noffset : {" << string_VkOffset3D(offset) << "},\nextent : {"
           << string_VkExtent3D(extent) << "}\n";
        return ss.str();
    }
};

// Returns true if source area of first vkImageCopy/vkImageCopy2KHR region intersects dest area of second region
// It is assumed that these are copy regions within a single image (otherwise no possibility of collision)
template <typename RegionType>
static ImageRegionIntersection GetRegionIntersection(const RegionType &region0, const RegionType &region1, VkImageType type,
                                                     bool is_multiplane) {
    ImageRegionIntersection result = {};

    // Separate planes within a multiplane image cannot intersect
    if (is_multiplane && (region0.srcSubresource.aspectMask != region1.dstSubresource.aspectMask)) {
        return result;
    }
    auto intersection = GetRangeIntersection(region0.srcSubresource.baseArrayLayer, region0.srcSubresource.layerCount,
                                             region1.dstSubresource.baseArrayLayer, region1.dstSubresource.layerCount);
    if ((region0.srcSubresource.mipLevel == region1.dstSubresource.mipLevel) && intersection.non_empty()) {
        result.subresource.aspectMask = region0.srcSubresource.aspectMask;
        result.subresource.baseArrayLayer = static_cast<uint32_t>(intersection.begin);
        result.subresource.layerCount = static_cast<uint32_t>(intersection.distance());
        result.subresource.mipLevel = region0.srcSubresource.mipLevel;
        result.has_instersection = true;
        switch (type) {
            case VK_IMAGE_TYPE_3D:
                intersection =
                    GetRangeIntersection(region0.srcOffset.z, region0.extent.depth, region1.dstOffset.z, region1.extent.depth);
                if (intersection.non_empty()) {
                    result.offset.z = static_cast<int32_t>(intersection.begin);
                    result.extent.depth = static_cast<uint32_t>(intersection.distance());
                } else {
                    result.has_instersection = false;
                    return result;
                }
                [[fallthrough]];
            case VK_IMAGE_TYPE_2D:
                intersection =
                    GetRangeIntersection(region0.srcOffset.y, region0.extent.height, region1.dstOffset.y, region1.extent.height);
                if (intersection.non_empty()) {
                    result.offset.y = static_cast<int32_t>(intersection.begin);
                    result.extent.height = static_cast<uint32_t>(intersection.distance());
                } else {
                    result.has_instersection = false;
                    return result;
                }
                [[fallthrough]];
            case VK_IMAGE_TYPE_1D:
                intersection =
                    GetRangeIntersection(region0.srcOffset.x, region0.extent.width, region1.dstOffset.x, region1.extent.width);
                if (intersection.non_empty()) {
                    result.offset.x = static_cast<int32_t>(intersection.begin);
                    result.extent.width = static_cast<uint32_t>(intersection.distance());
                } else {
                    result.has_instersection = false;
                    return result;
                }
                break;
            default:
                // Unrecognized or new IMAGE_TYPE enums will be caught in parameter_validation
                assert(false);
        }
    }
    return result;
}

// Returns true if source area of first vkImageCopy/vkImageCopy2KHR region intersects dest area of second region
// It is assumed that these are copy regions within a single image (otherwise no possibility of collision)
template <typename RegionType>
static bool RegionIntersects(const RegionType *region0, const RegionType *region1, VkImageType type, bool is_multiplane) {
    return GetRegionIntersection(region0, region1, type, is_multiplane).has_instersection;
}

template <typename RegionType>
static bool RegionIntersectsBlit(const RegionType *region0, const RegionType *region1, VkImageType type, bool is_multiplane) {
    bool result = false;

    // Separate planes within a multiplane image cannot intersect
    if (is_multiplane && (region0->srcSubresource.aspectMask != region1->dstSubresource.aspectMask)) {
        return result;
    }

    if ((region0->srcSubresource.mipLevel == region1->dstSubresource.mipLevel) &&
        (RangesIntersect(region0->srcSubresource.baseArrayLayer, region0->srcSubresource.layerCount,
                         region1->dstSubresource.baseArrayLayer, region1->dstSubresource.layerCount))) {
        result = true;
        switch (type) {
            case VK_IMAGE_TYPE_3D:
                result &= RangesIntersect(region0->srcOffsets[0].z, region0->srcOffsets[1].z - region0->srcOffsets[0].z,
                                          region1->dstOffsets[0].z, region1->dstOffsets[1].z - region1->dstOffsets[0].z);
                [[fallthrough]];
            case VK_IMAGE_TYPE_2D:
                result &= RangesIntersect(region0->srcOffsets[0].y, region0->srcOffsets[1].y - region0->srcOffsets[0].y,
                                          region1->dstOffsets[0].y, region1->dstOffsets[1].y - region1->dstOffsets[0].y);
                [[fallthrough]];
            case VK_IMAGE_TYPE_1D:
                result &= RangesIntersect(region0->srcOffsets[0].x, region0->srcOffsets[1].x - region0->srcOffsets[0].x,
                                          region1->dstOffsets[0].x, region1->dstOffsets[1].x - region1->dstOffsets[0].x);
                break;
            default:
                // Unrecognized or new IMAGE_TYPE enums will be caught in parameter_validation
                assert(false);
        }
    }
    return result;
}

// Test if the extent argument has all dimensions set to 0.
static inline bool IsExtentAllZeroes(const VkExtent3D &extent) {
    return ((extent.width == 0) && (extent.height == 0) && (extent.depth == 0));
}

// Returns the image transfer granularity for a specific image scaled by compressed block size if necessary.
VkExtent3D CoreChecks::GetScaledItg(const vvl::CommandBuffer &cb_state, const vvl::Image &image_state) const {
    // Default to (0, 0, 0) granularity in case we can't find the real granularity for the physical device.
    VkExtent3D granularity = {0, 0, 0};
    if (cb_state.command_pool) {
        granularity =
            physical_device_state->queue_family_properties[cb_state.command_pool->queueFamilyIndex].minImageTransferGranularity;
        const VkFormat image_format = image_state.create_info.format;
        if (vkuFormatIsBlockedImage(image_format)) {
            const VkExtent3D block_extent = vkuFormatTexelBlockExtent(image_format);
            granularity.width *= block_extent.width;
            granularity.height *= block_extent.height;
        }
    }
    return granularity;
}

// Test elements of a VkExtent3D structure against alignment constraints contained in another VkExtent3D structure
static inline bool IsExtentAligned(const VkExtent3D &extent, const VkExtent3D &granularity) {
    bool valid = true;
    if ((SafeModulo(extent.depth, granularity.depth) != 0) || (SafeModulo(extent.width, granularity.width) != 0) ||
        (SafeModulo(extent.height, granularity.height) != 0)) {
        valid = false;
    }
    return valid;
}

// Check elements of a VkOffset3D structure against a queue family's Image Transfer Granularity values
bool CoreChecks::ValidateTransferGranularityOffset(const LogObjectList &objlist, const VkOffset3D &offset,
                                                   const VkExtent3D &granularity, const Location &offset_loc,
                                                   const char *vuid) const {
    bool skip = false;
    VkExtent3D offset_extent = {};
    offset_extent.width = static_cast<uint32_t>(abs(offset.x));
    offset_extent.height = static_cast<uint32_t>(abs(offset.y));
    offset_extent.depth = static_cast<uint32_t>(abs(offset.z));
    if (IsExtentAllZeroes(granularity)) {
        // If the queue family image transfer granularity is (0, 0, 0), then the offset must always be (0, 0, 0)
        if (IsExtentAllZeroes(offset_extent) == false) {
            skip |= LogError(vuid, objlist, offset_loc,
                             "(%s) must be (x=0, y=0, z=0) when the command buffer's queue family "
                             "image transfer granularity is (w=0, h=0, d=0).",
                             string_VkOffset3D(offset).c_str());
        }
    } else if (!IsExtentAligned(offset_extent, granularity)) {
        // If the queue family image transfer granularity is not (0, 0, 0), then the offset dimensions must always be even
        // integer multiples of the image transfer granularity.
        skip |= LogError(vuid, objlist, offset_loc,
                         "(%s) dimensions must be even integer multiples of this command "
                         "buffer's queue family image transfer granularity (%s).",
                         string_VkOffset3D(offset).c_str(), string_VkExtent3D(granularity).c_str());
    }
    return skip;
}

// Test if two VkExtent3D structs are equivalent
static inline bool IsExtentEqual(const VkExtent3D &extent, const VkExtent3D &other_extent) {
    return (extent.width == other_extent.width) && (extent.height == other_extent.height) && (extent.depth == other_extent.depth);
}

// Check elements of a VkExtent3D structure against a queue family's Image Transfer Granularity values
bool CoreChecks::ValidateTransferGranularityExtent(const LogObjectList &objlist, const VkExtent3D &extent, const VkOffset3D &offset,
                                                   const VkExtent3D &granularity, const VkExtent3D &subresource_extent,
                                                   const VkImageType image_type, const Location &extent_loc,
                                                   const char *vuid) const {
    bool skip = false;
    if (IsExtentAllZeroes(granularity)) {
        // If the queue family image transfer granularity is (0, 0, 0), then the extent must always match the image
        // subresource extent.
        if (IsExtentEqual(extent, subresource_extent) == false) {
            skip |= LogError(vuid, objlist, extent_loc,
                             "(%s) must match the image subresource extents (%s) "
                             "when the command buffer's queue family image transfer granularity is (w=0, h=0, d=0).",
                             string_VkExtent3D(extent).c_str(), string_VkExtent3D(subresource_extent).c_str());
        }
    } else {
        // If the queue family image transfer granularity is not (0, 0, 0), then the extent dimensions must always be even
        // integer multiples of the image transfer granularity or the offset + extent dimensions must always match the image
        // subresource extent dimensions.
        VkExtent3D offset_extent_sum = {};
        offset_extent_sum.width = static_cast<uint32_t>(abs(offset.x)) + extent.width;
        offset_extent_sum.height = static_cast<uint32_t>(abs(offset.y)) + extent.height;
        offset_extent_sum.depth = static_cast<uint32_t>(abs(offset.z)) + extent.depth;
        bool x_ok = true;
        bool y_ok = true;
        bool z_ok = true;
        switch (image_type) {
            case VK_IMAGE_TYPE_3D:
                z_ok =
                    ((0 == SafeModulo(extent.depth, granularity.depth)) || (subresource_extent.depth == offset_extent_sum.depth));
                [[fallthrough]];
            case VK_IMAGE_TYPE_2D:
                y_ok = ((0 == SafeModulo(extent.height, granularity.height)) ||
                        (subresource_extent.height == offset_extent_sum.height));
                [[fallthrough]];
            case VK_IMAGE_TYPE_1D:
                x_ok =
                    ((0 == SafeModulo(extent.width, granularity.width)) || (subresource_extent.width == offset_extent_sum.width));
                break;
            default:
                // Unrecognized or new IMAGE_TYPE enums will be caught in parameter_validation
                assert(false);
        }
        if (!(x_ok && y_ok && z_ok)) {
            skip |= LogError(vuid, objlist, extent_loc,
                             "(%s) dimensions must be even integer multiples of this command "
                             "buffer's queue family image transfer granularity (%s) or offset (%s) + "
                             "extent (%s) must match the image subresource extents (%s).",
                             string_VkExtent3D(extent).c_str(), string_VkExtent3D(granularity).c_str(),
                             string_VkOffset3D(offset).c_str(), string_VkExtent3D(extent).c_str(),
                             string_VkExtent3D(subresource_extent).c_str());
        }
    }
    return skip;
}
template <typename HandleT>
bool CoreChecks::ValidateImageMipLevel(const HandleT handle, const vvl::Image &image_state, uint32_t mip_level,
                                       const Location &subresource_loc) const {
    bool skip = false;
    if (mip_level >= image_state.create_info.mipLevels) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(vvl::GetImageMipLevelVUID(subresource_loc), objlist, subresource_loc.dot(Field::mipLevel),
                         "is %" PRIu32 ", but provided %s has %" PRIu32 " mip levels.", mip_level,
                         FormatHandle(image_state).c_str(), image_state.create_info.mipLevels);
    }
    return skip;
}
template <typename HandleT>
bool CoreChecks::ValidateImageArrayLayerRange(const HandleT handle, const vvl::Image &image_state, const uint32_t base_layer,
                                              const uint32_t layer_count, const Location &subresource_loc) const {
    bool skip = false;
    if (base_layer >= image_state.create_info.arrayLayers || layer_count > image_state.create_info.arrayLayers ||
        (base_layer + layer_count) > image_state.create_info.arrayLayers) {
        if (layer_count != VK_REMAINING_ARRAY_LAYERS) {
            const LogObjectList objlist(handle, image_state.Handle());
            skip |= LogError(vvl::GetImageArrayLayerRangeVUID(subresource_loc), objlist, subresource_loc.dot(Field::baseArrayLayer),
                             "is %" PRIu32 " and layerCount is %" PRIu32 ", but provided %s has %" PRIu32 " array layers.",
                             base_layer, layer_count, FormatHandle(image_state).c_str(), image_state.create_info.arrayLayers);
        }
    }
    return skip;
}

bool IsValidAspectMaskForFormat(VkImageAspectFlags aspect_mask, VkFormat format) {
    if ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) != 0) {
        if (!(vkuFormatIsColor(format) || vkuFormatIsMultiplane(format))) return false;
    }
    if ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) != 0) {
        if (!vkuFormatHasDepth(format)) return false;
    }
    if ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
        if (!vkuFormatHasStencil(format)) return false;
    }
    if (0 != (aspect_mask & (VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT | VK_IMAGE_ASPECT_PLANE_2_BIT))) {
        if (vkuFormatPlaneCount(format) == 1) return false;
    }
    return true;
}

std::string DescribeValidAspectMaskForFormat(VkFormat format) {
    VkImageAspectFlags aspect_mask = 0;
    if (vkuFormatIsColor(format)) {
        aspect_mask |= VK_IMAGE_ASPECT_COLOR_BIT;
    }
    if (vkuFormatHasDepth(format)) {
        aspect_mask |= VK_IMAGE_ASPECT_DEPTH_BIT;
    }
    if (vkuFormatHasStencil(format)) {
        aspect_mask |= VK_IMAGE_ASPECT_STENCIL_BIT;
    }
    const uint32_t plane_count = vkuFormatPlaneCount(format);
    if (plane_count > 1) {
        // Color bit is "techically" valid, other VUs will warn if/when not allowed to use
        aspect_mask |= VK_IMAGE_ASPECT_COLOR_BIT | VK_IMAGE_ASPECT_PLANE_0_BIT | VK_IMAGE_ASPECT_PLANE_1_BIT;
    }
    if (plane_count > 2) {
        aspect_mask |= VK_IMAGE_ASPECT_PLANE_2_BIT;
    }

    std::stringstream ss;
    ss << "Valid VkImageAspectFlags are " << string_VkImageAspectFlags(aspect_mask);
    return ss.str();
}

template <typename T>
uint32_t GetRowLength(T data) {
    return data.bufferRowLength;
}
template <>
uint32_t GetRowLength<VkMemoryToImageCopyEXT>(VkMemoryToImageCopyEXT data) {
    return data.memoryRowLength;
}
template <>
uint32_t GetRowLength<VkImageToMemoryCopyEXT>(VkImageToMemoryCopyEXT data) {
    return data.memoryRowLength;
}
template <typename T>
uint32_t GetImageHeight(T data) {
    return data.bufferImageHeight;
}
template <>
uint32_t GetImageHeight<VkMemoryToImageCopyEXT>(VkMemoryToImageCopyEXT data) {
    return data.memoryImageHeight;
}
template <>
uint32_t GetImageHeight<VkImageToMemoryCopyEXT>(VkImageToMemoryCopyEXT data) {
    return data.memoryImageHeight;
}
template <typename HandleT, typename RegionType>
bool CoreChecks::ValidateHeterogeneousCopyData(const HandleT handle, const RegionType &region, const vvl::Image &image_state,
                                               const Location &region_loc) const {
    bool skip = false;
    const bool is_memory =
        IsValueIn(region_loc.function, {Func::vkCopyMemoryToImageEXT, Func::vkCopyImageToMemory, Func::vkCopyImageToMemoryEXT});

    const Location subresource_loc = region_loc.dot(Field::imageSubresource);
    if (image_state.create_info.imageType == VK_IMAGE_TYPE_1D) {
        if ((region.imageOffset.y != 0) || (region.imageExtent.height != 1)) {
            const LogObjectList objlist(handle, image_state.Handle());
            skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::Image1D_07979), objlist,
                             region_loc.dot(Field::imageOffset).dot(Field::y),
                             "is %" PRId32 " and imageExtent.height is %" PRIu32
                             ". For 1D images these must be 0 "
                             "and 1, respectively.",
                             region.imageOffset.y, region.imageExtent.height);
        }
    }

    if ((image_state.create_info.imageType == VK_IMAGE_TYPE_1D) || (image_state.create_info.imageType == VK_IMAGE_TYPE_2D)) {
        if ((region.imageOffset.z != 0) || (region.imageExtent.depth != 1)) {
            const LogObjectList objlist(handle, image_state.Handle());
            skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::Image1D_07980), objlist,
                             region_loc.dot(Field::imageOffset).dot(Field::z),
                             "is %" PRId32 " and imageExtent.depth is %" PRIu32
                             ". For 1D and 2D images these "
                             "must be 0 and 1, respectively.",
                             region.imageOffset.z, region.imageExtent.depth);
        }
    }

    if (image_state.create_info.imageType == VK_IMAGE_TYPE_3D) {
        if ((0 != region.imageSubresource.baseArrayLayer) || (1 != region.imageSubresource.layerCount)) {
            const LogObjectList objlist(handle, image_state.Handle());
            skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::Image3D_07983), objlist,
                             subresource_loc.dot(Field::baseArrayLayer),
                             "is %" PRIu32 " and layerCount is %" PRIu32 ". For 3D images these must be 0 and 1, respectively.",
                             region.imageSubresource.baseArrayLayer, region.imageSubresource.layerCount);
        }
    }

    // Make sure not a empty region
    if (region.imageExtent.width == 0) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::ImageExtentWidthZero_06659), objlist,
                         region_loc.dot(Field::imageExtent).dot(Field::width), "is zero (empty copies are not allowed).");
    }
    if (region.imageExtent.height == 0) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::ImageExtentHeightZero_06660), objlist,
                         region_loc.dot(Field::imageExtent).dot(Field::height), "is zero (empty copies are not allowed).");
    }
    if (region.imageExtent.depth == 0) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::ImageExtentDepthZero_06661), objlist,
                         region_loc.dot(Field::imageExtent).dot(Field::depth), "is zero (empty copies are not allowed).");
    }

    //  BufferRowLength must be 0, or greater than or equal to the width member of imageExtent
    uint32_t row_length = GetRowLength(region);
    if ((row_length != 0) && (row_length < region.imageExtent.width)) {
        const LogObjectList objlist(handle, image_state.Handle());
        Field field = is_memory ? Field::memoryRowLength : Field::bufferRowLength;
        skip |= LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::ImageExtentRowLength_09101), objlist,
                         region_loc.dot(field),
                         "(%" PRIu32 ") must be zero or greater-than-or-equal-to imageExtent.width (%" PRIu32 ").", row_length,
                         region.imageExtent.width);
    }

    //  BufferImageHeight must be 0, or greater than or equal to the height member of imageExtent
    uint32_t image_height = GetImageHeight(region);
    if ((image_height != 0) && (image_height < region.imageExtent.height)) {
        const LogObjectList objlist(handle, image_state.Handle());
        Field field = is_memory ? Field::memoryImageHeight : Field::bufferImageHeight;
        skip |= LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::ImageExtentImageHeight_09102), objlist,
                         region_loc.dot(field),
                         "(%" PRIu32 ") must be zero or greater-than-or-equal-to imageExtent.height (%" PRIu32 ").", image_height,
                         region.imageExtent.height);
    }

    // subresource aspectMask must have exactly 1 bit set
    const VkImageAspectFlags region_aspect_mask = region.imageSubresource.aspectMask;
    if (GetBitSetCount(region_aspect_mask) != 1) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::AspectMaskSingleBit_09103), objlist,
                         subresource_loc.dot(Field::aspectMask), "is %s (only one bit allowed).",
                         string_VkImageAspectFlags(region_aspect_mask).c_str());
    }

    // Calculate adjusted image extent, accounting for multiplane image factors
    VkExtent3D adjusted_image_extent = image_state.GetEffectiveSubresourceExtent(region.imageSubresource);
    // imageOffset.x and (imageExtent.width + imageOffset.x) must both be >= 0 and <= image subresource width
    if ((region.imageOffset.x < 0) || (region.imageOffset.x > static_cast<int32_t>(adjusted_image_extent.width)) ||
        ((region.imageOffset.x + static_cast<int32_t>(region.imageExtent.width)) >
         static_cast<int32_t>(adjusted_image_extent.width))) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::ImageOffest_07971), objlist,
                         region_loc.dot(Field::imageOffset).dot(Field::x),
                         "(%" PRId32 ") and (imageExtent.width + imageOffset.x) (%" PRIu32
                         ") must be >= "
                         "zero or <= image subresource width (%" PRIu32 ").",
                         region.imageOffset.x, (region.imageOffset.x + region.imageExtent.width), adjusted_image_extent.width);
    }

    // imageOffset.y and (imageExtent.height + imageOffset.y) must both be >= 0 and <= image subresource height
    if ((region.imageOffset.y < 0) || (region.imageOffset.y > static_cast<int32_t>(adjusted_image_extent.height)) ||
        ((region.imageOffset.y + static_cast<int32_t>(region.imageExtent.height)) >
         static_cast<int32_t>(adjusted_image_extent.height))) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::ImageOffest_07972), objlist,
                         region_loc.dot(Field::imageOffset).dot(Field::y),
                         "(%" PRId32 ") and (imageExtent.height + imageOffset.y) (%" PRIu32
                         ") must be >= "
                         "zero or <= image subresource height (%" PRIu32 ").",
                         region.imageOffset.y, (region.imageOffset.y + region.imageExtent.height), adjusted_image_extent.height);
    }

    // imageOffset.z and (imageExtent.depth + imageOffset.z) must both be >= 0 and <= image subresource depth
    if ((region.imageOffset.z < 0) || (region.imageOffset.z > static_cast<int32_t>(adjusted_image_extent.depth)) ||
        ((region.imageOffset.z + static_cast<int32_t>(region.imageExtent.depth)) >
         static_cast<int32_t>(adjusted_image_extent.depth))) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::ImageOffest_09104), objlist,
                         region_loc.dot(Field::imageOffset).dot(Field::z),
                         "(%" PRId32 ") and (imageExtent.depth + imageOffset.z) (%" PRIu32
                         ") must be >= "
                         "zero or <= image subresource depth (%" PRIu32 ").",
                         region.imageOffset.z, (region.imageOffset.z + region.imageExtent.depth), adjusted_image_extent.depth);
    }

    const VkFormat image_format = image_state.create_info.format;
    // image subresource aspect bit must match format
    if (!IsValidAspectMaskForFormat(region_aspect_mask, image_format)) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::AspectMask_09105), objlist,
                         subresource_loc.dot(Field::aspectMask), "(%s) is invalid for image format %s. (%s)",
                         string_VkImageAspectFlags(region_aspect_mask).c_str(), string_VkFormat(image_format),
                         DescribeValidAspectMaskForFormat(image_format).c_str());
    }

    const VkExtent3D block_extent = vkuFormatTexelBlockExtent(image_format);
    //  BufferRowLength must be a multiple of block width
    if (SafeModulo(row_length, block_extent.width) != 0) {
        const LogObjectList objlist(handle, image_state.Handle());
        Field field = is_memory ? Field::memoryRowLength : Field::bufferRowLength;
        skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::bufferRowLength_09106), objlist, region_loc.dot(field),
                         "(%" PRIu32 ") must be a multiple of the image (%s) texel block extent width (%" PRIu32 ").", row_length,
                         string_VkFormat(image_format), block_extent.width);
    }

    //  BufferRowHeight must be a multiple of block height
    if (SafeModulo(image_height, block_extent.height) != 0) {
        const LogObjectList objlist(handle, image_state.Handle());
        Field field = is_memory ? Field::memoryImageHeight : Field::bufferImageHeight;
        skip |=
            LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::bufferImageHeight_09107), objlist, region_loc.dot(field),
                     "(%" PRIu32 ") must be a multiple of the image (%s) texel block extent height (%" PRIu32 ").", image_height,
                     string_VkFormat(image_format), block_extent.height);
    }

    //  image offsets x must be multiple of block width
    if (SafeModulo(region.imageOffset.x, block_extent.width) != 0) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::TexelBlockExtentWidth_07274), objlist,
                         region_loc.dot(Field::imageOffset).dot(Field::x),
                         "(%" PRId32
                         ") must be a multiple of the image (%s) texel block extent "
                         "width (%" PRIu32 ").",
                         region.imageOffset.x, string_VkFormat(image_format), block_extent.width);
    }

    //  image offsets y must be multiple of block height
    if (SafeModulo(region.imageOffset.y, block_extent.height) != 0) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::TexelBlockExtentHeight_07275), objlist,
                         region_loc.dot(Field::imageOffset).dot(Field::y),
                         "(%" PRId32 ") must be a multiple of the image (%s) texel block height (%" PRIu32 ").",
                         region.imageOffset.y, string_VkFormat(image_format), block_extent.height);
    }

    //  image offsets z must be multiple of block depth
    if (SafeModulo(region.imageOffset.z, block_extent.depth) != 0) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::TexelBlockExtentDepth_07276), objlist,
                         region_loc.dot(Field::imageOffset).dot(Field::z),
                         "(%" PRId32 ") must be a multiple of the image (%s) texel block depth (%" PRIu32 ").",
                         region.imageOffset.z, string_VkFormat(image_format), block_extent.depth);
    }

    // imageExtent width must be a multiple of block width, or extent+offset width must equal subresource width
    VkExtent3D mip_extent = image_state.GetEffectiveSubresourceExtent(region.imageSubresource);
    if ((SafeModulo(region.imageExtent.width, block_extent.width) != 0) &&
        (region.imageExtent.width + region.imageOffset.x != mip_extent.width)) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(
            GetCopyBufferImageVUID(region_loc, vvl::CopyError::TexelBlockExtentWidth_00207), objlist,
            region_loc.dot(Field::imageExtent).dot(Field::width),
            "(%" PRIu32
            ") must be a multiple of the image (%s) texel block width "
            "(%" PRIu32 "), or when added to imageOffset.x (%" PRId32 ") must equal the image subresource width (%" PRIu32 ").",
            region.imageExtent.width, string_VkFormat(image_format), block_extent.width, region.imageOffset.x, mip_extent.width);
    }

    // imageExtent height must be a multiple of block height, or extent+offset height must equal subresource height
    if ((SafeModulo(region.imageExtent.height, block_extent.height) != 0) &&
        (region.imageExtent.height + region.imageOffset.y != mip_extent.height)) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(
            GetCopyBufferImageVUID(region_loc, vvl::CopyError::TexelBlockExtentHeight_00208), objlist,
            region_loc.dot(Field::imageExtent).dot(Field::height),
            "(%" PRIu32
            ") must be a multiple of the image (%s) texel block height "
            "(%" PRIu32 "), or when added to imageOffset.y (%" PRId32 ") must equal the image subresource height (%" PRIu32 ").",
            region.imageExtent.height, string_VkFormat(image_format), block_extent.height, region.imageOffset.y, mip_extent.height);
    }

    // imageExtent depth must be a multiple of block depth, or extent+offset depth must equal subresource depth
    if ((SafeModulo(region.imageExtent.depth, block_extent.depth) != 0) &&
        (region.imageExtent.depth + region.imageOffset.z != mip_extent.depth)) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(
            GetCopyBufferImageVUID(region_loc, vvl::CopyError::TexelBlockExtentDepth_00209), objlist,
            region_loc.dot(Field::imageExtent).dot(Field::depth),
            "(%" PRIu32
            ") must be a multiple of the image (%s) texel block depth "
            "(%" PRIu32 "), or when added to imageOffset.z (%" PRId32 ") must equal the image subresource depth (%" PRIu32 ").",
            region.imageExtent.depth, string_VkFormat(image_format), block_extent.depth, region.imageOffset.z, mip_extent.depth);
    }

    // *RowLength divided by the texel block extent width and then multiplied by the texel block size of the image must be
    // less than or equal to 2^31-1
    const uint32_t texel_block_size = vkuFormatTexelBlockSize(image_format);
    double test_value = row_length / block_extent.width;
    test_value = test_value * texel_block_size;
    const auto two_to_31_minus_1 = static_cast<double>((1u << 31) - 1);
    if (test_value > two_to_31_minus_1) {
        const LogObjectList objlist(handle, image_state.Handle());
        Field field = is_memory ? Field::memoryRowLength : Field::bufferRowLength;
        skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::bufferRowLength_09108), objlist, region_loc.dot(field),
                         "(%" PRIu32 ") divided by the  image (%s) texel block width (%" PRIu32
                         ") then multiplied by the "
                         "texel block size of image (%" PRIu32 ") is (%" PRIu64 ") which is greater than 2^31 - 1",
                         row_length, string_VkFormat(image_format), block_extent.width, texel_block_size,
                         static_cast<uint64_t>(test_value));
    }

    // Checks that apply only to multi-planar format images
    if (vkuFormatIsMultiplane(image_format) && !IsOnlyOneValidPlaneAspect(image_format, region_aspect_mask)) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(GetCopyBufferImageVUID(region_loc, vvl::CopyError::MultiPlaneAspectMask_07981), objlist,
                         subresource_loc.dot(Field::aspectMask), "(%s) is invalid for multi-planar format %s.",
                         string_VkImageAspectFlags(region_aspect_mask).c_str(), string_VkFormat(image_format));
    }

    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateBufferImageCopyData(const vvl::CommandBuffer &cb_state, const RegionType &region,
                                             const vvl::Image &image_state, const Location &region_loc) const {
    bool skip = false;
    skip |= ValidateHeterogeneousCopyData(cb_state.VkHandle(), region, image_state, region_loc);

    // bufferOffset must be a certain multiple depending if
    // - Depth Stencil format
    // - Multi-Planar format
    // - everything else
    const VkFormat image_format = image_state.create_info.format;
    if (vkuFormatIsDepthOrStencil(image_format)) {
        if (SafeModulo(region.bufferOffset, 4) != 0) {
            const LogObjectList objlist(cb_state.Handle(), image_state.Handle());
            skip |= LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::BufferOffset_07978), objlist,
                             region_loc.dot(Field::bufferOffset),
                             "(%" PRIu64 ") must be a multiple 4 if using a depth/stencil format (%s).", region.bufferOffset,
                             string_VkFormat(image_format));
        }
    } else if (vkuFormatIsMultiplane(image_format)) {
        // MultiPlaneAspectMask_07981 will validate if aspect mask is bad
        const VkImageAspectFlags region_aspect_mask = region.imageSubresource.aspectMask;
        if (IsAnyPlaneAspect(region_aspect_mask)) {
            const VkFormat compatible_format =
                vkuFindMultiplaneCompatibleFormat(image_format, static_cast<VkImageAspectFlagBits>(region_aspect_mask));
            const uint32_t texel_block_size = vkuFormatTexelBlockSize(compatible_format);
            if (SafeModulo(region.bufferOffset, texel_block_size) != 0) {
                const LogObjectList objlist(cb_state.Handle(), image_state.Handle());
                skip |= LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::MultiPlaneCompatible_07976), objlist,
                                 region_loc.dot(Field::bufferOffset),
                                 "(%" PRIu64 ") is not a multiple of texel block size (%" PRIu32
                                 ") for %s (which is the compatible format for plane %" PRIu32 " of %s).",
                                 region.bufferOffset, texel_block_size, string_VkFormat(compatible_format),
                                 vkuGetPlaneIndex(static_cast<VkImageAspectFlagBits>(region_aspect_mask)),
                                 string_VkFormat(image_format));
            }
        }
    } else {
        const uint32_t texel_block_size = vkuFormatTexelBlockSize(image_format);
        if (SafeModulo(region.bufferOffset, texel_block_size) != 0) {
            const LogObjectList objlist(cb_state.Handle(), image_state.Handle());
            skip |= LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::TexelBlockSize_07975), objlist,
                             region_loc.dot(Field::bufferOffset),
                             "(%" PRIu64 ") must be a multiple texel block size (%" PRIu32 ") for %s.", region.bufferOffset,
                             texel_block_size, string_VkFormat(image_format));
        }
    }

    if (SafeModulo(region.bufferOffset, 4) != 0) {
        const VkQueueFlags required_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
        if (!HasRequiredQueueFlags(cb_state, *physical_device_state, required_flags)) {
            const char *vuid = GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::BufferOffset_07737).c_str();
            const LogObjectList objlist(cb_state.Handle(), cb_state.command_pool->Handle());
            skip |=
                LogError(vuid, objlist, region_loc.dot(Field::bufferOffset), "(%" PRIu64 ") is not a multiple of 4, but is %s",
                         region.bufferOffset, DescribeRequiredQueueFlag(cb_state, *physical_device_state, required_flags).c_str());
        }
    }

    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyBufferBounds(VkCommandBuffer cb, const vvl::Buffer &src_buffer_state,
                                             const vvl::Buffer &dst_buffer_state, uint32_t regionCount, const RegionType *pRegions,
                                             const Location &loc) const {
    bool skip = false;
    const bool is_2 = loc.function == Func::vkCmdCopyBuffer2 || loc.function == Func::vkCmdCopyBuffer2KHR;

    VkDeviceSize src_buffer_size = src_buffer_state.create_info.size;
    VkDeviceSize dst_buffer_size = dst_buffer_state.create_info.size;
    const bool are_buffers_sparse = src_buffer_state.sparse || dst_buffer_state.sparse;

    const LogObjectList src_objlist(cb, src_buffer_state.Handle());
    const LogObjectList dst_objlist(cb, dst_buffer_state.Handle());

    const auto *src_binding = src_buffer_state.Binding();
    const auto *dst_binding = dst_buffer_state.Binding();

    const bool validate_no_memory_overlaps = !are_buffers_sparse && (regionCount > 0) && src_binding && dst_binding &&
                                             (src_binding->memory_state == dst_binding->memory_state);

    using MemoryRange = vvl::BindableMemoryTracker::BufferRange;

    std::vector<MemoryRange> src_memory_ranges;
    std::vector<MemoryRange> dst_memory_ranges;
    if (validate_no_memory_overlaps) {
        src_memory_ranges.reserve(regionCount);
        dst_memory_ranges.reserve(regionCount);
    }

    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const RegionType &region = pRegions[i];

        // The srcOffset member of each element of pRegions must be less than the size of srcBuffer
        if (region.srcOffset >= src_buffer_size) {
            const char *vuid = is_2 ? "VUID-VkCopyBufferInfo2-srcOffset-00113" : "VUID-vkCmdCopyBuffer-srcOffset-00113";
            skip |= LogError(vuid, src_objlist, region_loc.dot(Field::srcOffset),
                             "(%" PRIuLEAST64 ") is greater than size of srcBuffer (%" PRIuLEAST64 ").", region.srcOffset,
                             src_buffer_size);
        }

        // The dstOffset member of each element of pRegions must be less than the size of dstBuffer
        if (region.dstOffset >= dst_buffer_size) {
            const char *vuid = is_2 ? "VUID-VkCopyBufferInfo2-dstOffset-00114" : "VUID-vkCmdCopyBuffer-dstOffset-00114";
            skip |= LogError(vuid, dst_objlist, region_loc.dot(Field::dstOffset),
                             "(%" PRIuLEAST64 ") is greater than size of dstBuffer (%" PRIuLEAST64 ").", region.dstOffset,
                             dst_buffer_size);
        }

        // The size member of each element of pRegions must be less than or equal to the size of srcBuffer minus srcOffset
        if (region.size > (src_buffer_size - region.srcOffset)) {
            const char *vuid = is_2 ? "VUID-VkCopyBufferInfo2-size-00115" : "VUID-vkCmdCopyBuffer-size-00115";
            skip |= LogError(vuid, src_objlist, region_loc.dot(Field::size),
                             "(%" PRIuLEAST64 ") is greater than the source buffer size (%" PRIuLEAST64
                             ") minus srcOffset (%" PRIuLEAST64 ").",
                             region.size, src_buffer_size, region.srcOffset);
        }

        // The size member of each element of pRegions must be less than or equal to the size of dstBuffer minus dstOffset
        if (region.size > (dst_buffer_size - region.dstOffset)) {
            const char *vuid = is_2 ? "VUID-VkCopyBufferInfo2-size-00116" : "VUID-vkCmdCopyBuffer-size-00116";
            skip |= LogError(vuid, dst_objlist, region_loc.dot(Field::size),
                             "(%" PRIuLEAST64 ") is greater than the destination buffer size (%" PRIuLEAST64
                             ") minus dstOffset (%" PRIuLEAST64 ").",
                             region.size, dst_buffer_size, region.dstOffset);
        }

        // The union of the source regions, and the union of the destination regions, must not overlap in memory
        if (validate_no_memory_overlaps) {
            // Sort copy ranges
            {
                MemoryRange src_buffer_memory_range(src_binding->memory_offset + region.srcOffset,
                                                    src_binding->memory_offset + region.srcOffset + region.size);
                auto insert_pos = std::lower_bound(src_memory_ranges.begin(), src_memory_ranges.end(), src_buffer_memory_range);
                src_memory_ranges.insert(insert_pos, src_buffer_memory_range);
            }

            {
                MemoryRange dst_buffer_memory_range(dst_binding->memory_offset + region.dstOffset,
                                                    dst_binding->memory_offset + region.dstOffset + region.size);
                auto insert_pos = std::lower_bound(dst_memory_ranges.begin(), dst_memory_ranges.end(), dst_buffer_memory_range);
                dst_memory_ranges.insert(insert_pos, dst_buffer_memory_range);
            }
        }
    }

    if (validate_no_memory_overlaps) {
        // Memory ranges are sorted, so looking for overlaps can be done in linear time
        auto src_ranges_it = src_memory_ranges.cbegin();
        auto dst_ranges_it = dst_memory_ranges.cbegin();

        while (src_ranges_it != src_memory_ranges.cend() && dst_ranges_it != dst_memory_ranges.cend()) {
            if (src_ranges_it->intersects(*dst_ranges_it)) {
                auto memory_range_overlap = *src_ranges_it & *dst_ranges_it;

                const LogObjectList objlist(cb, src_binding->memory_state->Handle(), src_buffer_state.Handle(),
                                            dst_buffer_state.Handle());
                const char *vuid = is_2 ? "VUID-VkCopyBufferInfo2-pRegions-00117" : "VUID-vkCmdCopyBuffer-pRegions-00117";
                skip |= LogError(
                    vuid, objlist, loc,
                    "Copy source buffer range %s (from buffer %s) and destination buffer range %s (from buffer %s) are bound to "
                    "the same memory (%s), "
                    "and end up overlapping on memory range %s.",
                    sparse_container::string_range(*src_ranges_it).c_str(), FormatHandle(src_buffer_state.VkHandle()).c_str(),
                    sparse_container::string_range(*dst_ranges_it).c_str(), FormatHandle(dst_buffer_state.VkHandle()).c_str(),
                    FormatHandle(src_binding->memory_state->VkHandle()).c_str(),
                    sparse_container::string_range(memory_range_overlap).c_str());
            }

            if (*src_ranges_it < *dst_ranges_it) {
                ++src_ranges_it;
            } else {
                ++dst_ranges_it;
            }
        }
    }

    return skip;
}
template <typename RegionType>
bool CoreChecks::ValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                       const RegionType *pRegions, const Location &loc) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<vvl::CommandBuffer>(commandBuffer);
    auto src_buffer_state = Get<vvl::Buffer>(srcBuffer);
    auto dst_buffer_state = Get<vvl::Buffer>(dstBuffer);
    if (!cb_state_ptr || !src_buffer_state || !dst_buffer_state) {
        return skip;
    }
    const vvl::CommandBuffer &cb_state = *cb_state_ptr;

    const bool is_2 = loc.function == Func::vkCmdCopyBuffer2 || loc.function == Func::vkCmdCopyBuffer2KHR;
    const char *vuid;

    skip |= ValidateCmd(cb_state, loc);
    skip |= ValidateCmdCopyBufferBounds(commandBuffer, *src_buffer_state, *dst_buffer_state, regionCount, pRegions, loc);

    // src buffer
    {
        const Location src_buffer_loc = loc.dot(Field::srcBuffer);
        vuid = is_2 ? "VUID-VkCopyBufferInfo2-srcBuffer-00119" : "VUID-vkCmdCopyBuffer-srcBuffer-00119";
        skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *src_buffer_state, src_buffer_loc, vuid);

        vuid = is_2 ? "VUID-VkCopyBufferInfo2-srcBuffer-00118" : "VUID-vkCmdCopyBuffer-srcBuffer-00118";
        skip |= ValidateBufferUsageFlags(LogObjectList(commandBuffer, srcBuffer), *src_buffer_state,
                                         VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, vuid, src_buffer_loc);

        vuid = is_2 ? "VUID-vkCmdCopyBuffer2-commandBuffer-01822" : "VUID-vkCmdCopyBuffer-commandBuffer-01822";
        skip |= ValidateProtectedBuffer(cb_state, *src_buffer_state, src_buffer_loc, vuid);
    }

    // dst buffer
    {
        const Location dst_buffer_loc = loc.dot(Field::dstBuffer);
        vuid = is_2 ? "VUID-VkCopyBufferInfo2-dstBuffer-00121" : "VUID-vkCmdCopyBuffer-dstBuffer-00121";
        skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_buffer_state, dst_buffer_loc, vuid);

        vuid = is_2 ? "VUID-VkCopyBufferInfo2-dstBuffer-00120" : "VUID-vkCmdCopyBuffer-dstBuffer-00120";
        skip |= ValidateBufferUsageFlags(LogObjectList(commandBuffer, dstBuffer), *dst_buffer_state,
                                         VK_BUFFER_USAGE_TRANSFER_DST_BIT, true, vuid, dst_buffer_loc);

        vuid = is_2 ? "VUID-vkCmdCopyBuffer2-commandBuffer-01823" : "VUID-vkCmdCopyBuffer-commandBuffer-01823";
        skip |= ValidateProtectedBuffer(cb_state, *dst_buffer_state, dst_buffer_loc, vuid);
        vuid = is_2 ? "VUID-vkCmdCopyBuffer2-commandBuffer-01824" : "VUID-vkCmdCopyBuffer-commandBuffer-01824";
        skip |= ValidateUnprotectedBuffer(cb_state, *dst_buffer_state, dst_buffer_loc, vuid);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                              uint32_t regionCount, const VkBufferCopy *pRegions,
                                              const ErrorObject &error_obj) const {
    return ValidateCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions, error_obj.location);
}

bool CoreChecks::PreCallValidateCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo,
                                                  const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, error_obj);
}

bool CoreChecks::PreCallValidateCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                               const ErrorObject &error_obj) const {
    return ValidateCmdCopyBuffer(commandBuffer, pCopyBufferInfo->srcBuffer, pCopyBufferInfo->dstBuffer,
                                 pCopyBufferInfo->regionCount, pCopyBufferInfo->pRegions,
                                 error_obj.location.dot(Field::pCopyBufferInfo));
}

// Check valid usage Image Transfer Granularity requirements for elements of a VkBufferImageCopy/VkBufferImageCopy2 structure
template <typename RegionType>
bool CoreChecks::ValidateCopyBufferImageTransferGranularityRequirements(const vvl::CommandBuffer &cb_state,
                                                                        const vvl::Image &image_state, const RegionType &region,
                                                                        const Location &region_loc) const {
    bool skip = false;
    const LogObjectList objlist(cb_state.Handle(), image_state.Handle());
    std::string vuid = GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::TransferGranularity_07747);
    VkExtent3D granularity = GetScaledItg(cb_state, image_state);
    skip |= ValidateTransferGranularityOffset(objlist, region.imageOffset, granularity, region_loc.dot(Field::imageOffset),
                                              vuid.c_str());
    VkExtent3D subresource_extent = image_state.GetEffectiveSubresourceExtent(region.imageSubresource);
    skip |= ValidateTransferGranularityExtent(objlist, region.imageExtent, region.imageOffset, granularity, subresource_extent,
                                              image_state.create_info.imageType, region_loc.dot(Field::imageExtent), vuid.c_str());
    return skip;
}

template <typename HandleT>
bool CoreChecks::ValidateImageSubresourceLayers(HandleT handle, const VkImageSubresourceLayers &subresource_layers,
                                                const Location &subresource_loc) const {
    bool skip = false;
    if (subresource_layers.layerCount == VK_REMAINING_ARRAY_LAYERS) {
        if (!enabled_features.maintenance5) {
            skip |= LogError("VUID-VkImageSubresourceLayers-layerCount-09243", handle, subresource_loc.dot(Field::layerCount),
                             "is VK_REMAINING_ARRAY_LAYERS.");
        }
    } else if (subresource_layers.layerCount == 0) {
        skip |=
            LogError("VUID-VkImageSubresourceLayers-layerCount-01700", handle, subresource_loc.dot(Field::layerCount), "is zero.");
    }

    const VkImageAspectFlags aspect_mask = subresource_layers.aspectMask;
    // aspectMask must not contain VK_IMAGE_ASPECT_METADATA_BIT
    if (aspect_mask & VK_IMAGE_ASPECT_METADATA_BIT) {
        skip |= LogError("VUID-VkImageSubresourceLayers-aspectMask-00168", handle, subresource_loc.dot(Field::aspectMask), "is %s.",
                         string_VkImageAspectFlags(aspect_mask).c_str());
    }
    // if aspectMask contains COLOR, it must not contain either DEPTH or STENCIL
    if ((aspect_mask & VK_IMAGE_ASPECT_COLOR_BIT) && (aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT))) {
        skip |= LogError("VUID-VkImageSubresourceLayers-aspectMask-00167", handle, subresource_loc.dot(Field::aspectMask), "is %s.",
                         string_VkImageAspectFlags(aspect_mask).c_str());
    }
    // aspectMask must not contain VK_IMAGE_ASPECT_MEMORY_PLANE_i_BIT_EXT
    if (aspect_mask & (VK_IMAGE_ASPECT_MEMORY_PLANE_0_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_1_BIT_EXT |
                       VK_IMAGE_ASPECT_MEMORY_PLANE_2_BIT_EXT | VK_IMAGE_ASPECT_MEMORY_PLANE_3_BIT_EXT)) {
        skip |= LogError("VUID-VkImageSubresourceLayers-aspectMask-02247", handle, subresource_loc.dot(Field::aspectMask), "is %s.",
                         string_VkImageAspectFlags(aspect_mask).c_str());
    }
    return skip;
}

// For image copies between compressed/uncompressed formats, the extent is provided in source image texels
// Destination image texel extents must be adjusted by block size for the dest validation checks
static inline VkExtent3D GetAdjustedDstImageExtent(VkFormat src_format, VkFormat dst_format, VkExtent3D extent) {
    VkExtent3D adjusted_extent = extent;
    if (vkuFormatIsBlockedImage(src_format) && !vkuFormatIsBlockedImage(dst_format)) {
        const VkExtent3D block_extent = vkuFormatTexelBlockExtent(src_format);
        adjusted_extent.width /= block_extent.width;
        adjusted_extent.height /= block_extent.height;
        adjusted_extent.depth /= block_extent.depth;
    } else if (!vkuFormatIsBlockedImage(src_format) && vkuFormatIsBlockedImage(dst_format)) {
        const VkExtent3D block_extent = vkuFormatTexelBlockExtent(dst_format);
        adjusted_extent.width *= block_extent.width;
        adjusted_extent.height *= block_extent.height;
        adjusted_extent.depth *= block_extent.depth;
    }
    return adjusted_extent;
}

// Check valid usage Image Transfer Granularity requirements for elements of a VkImageCopy/VkImageCopy2KHR structure
template <typename RegionType>
bool CoreChecks::ValidateCopyImageTransferGranularityRequirements(const vvl::CommandBuffer &cb_state,
                                                                  const vvl::Image &src_image_state,
                                                                  const vvl::Image &dst_image_state, const RegionType &region,
                                                                  const Location &region_loc) const {
    bool skip = false;
    const bool is_2 = region_loc.function == Func::vkCmdCopyImage2 || region_loc.function == Func::vkCmdCopyImage2KHR;
    const char *vuid;

    const VkExtent3D extent = region.extent;
    {
        // Source image checks
        const LogObjectList objlist(cb_state.Handle(), src_image_state.Handle());
        const VkExtent3D granularity = GetScaledItg(cb_state, src_image_state);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcOffset-01783" : "VUID-vkCmdCopyImage-srcOffset-01783";
        skip |= ValidateTransferGranularityOffset(objlist, region.srcOffset, granularity, region_loc.dot(Field::srcOffset), vuid);
        const VkExtent3D subresource_extent = src_image_state.GetEffectiveSubresourceExtent(region.srcSubresource);
        skip |= ValidateTransferGranularityExtent(objlist, extent, region.srcOffset, granularity, subresource_extent,
                                                  src_image_state.create_info.imageType, region_loc.dot(Field::extent), vuid);
    }

    {
        // Destination image checks
        const LogObjectList objlist(cb_state.Handle(), dst_image_state.Handle());
        const VkExtent3D granularity = GetScaledItg(cb_state, dst_image_state);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-dstOffset-01784" : "VUID-vkCmdCopyImage-dstOffset-01784";
        skip |= ValidateTransferGranularityOffset(objlist, region.dstOffset, granularity, region_loc.dot(Field::dstOffset), vuid);
        // Adjust dest extent, if necessary
        const VkExtent3D dst_effective_extent =
            GetAdjustedDstImageExtent(src_image_state.create_info.format, dst_image_state.create_info.format, extent);
        const VkExtent3D subresource_extent = dst_image_state.GetEffectiveSubresourceExtent(region.dstSubresource);
        skip |= ValidateTransferGranularityExtent(objlist, dst_effective_extent, region.dstOffset, granularity, subresource_extent,
                                                  dst_image_state.create_info.imageType, region_loc.dot(Field::extent), vuid);
    }
    return skip;
}

// Validate contents of a VkImageCopy or VkImageCopy2KHR struct
template <typename HandleT, typename RegionType>
bool CoreChecks::ValidateImageCopyData(const HandleT handle, const RegionType &region, const vvl::Image &src_image_state,
                                       const vvl::Image &dst_image_state, bool is_host, const Location &region_loc) const {
    bool skip = false;
    const bool is_2 = region_loc.function == Func::vkCmdCopyImage2 || region_loc.function == Func::vkCmdCopyImage2KHR;
    const char *vuid;

    // For comp<->uncomp copies, the copy extent for the dest image must be adjusted
    const VkExtent3D src_copy_extent = region.extent;
    const VkExtent3D dst_copy_extent =
        GetAdjustedDstImageExtent(src_image_state.create_info.format, dst_image_state.create_info.format, region.extent);

    bool slice_override = false;
    uint32_t depth_slices = 0;

    // Special case for copying between a 1D/2D array and a 3D image
    // TBD: This seems like the only way to reconcile 3 mutually-exclusive VU checks for 2D/3D copies. Heads up.
    if ((VK_IMAGE_TYPE_3D == src_image_state.create_info.imageType) &&
        (VK_IMAGE_TYPE_3D != dst_image_state.create_info.imageType)) {
        depth_slices = region.dstSubresource.layerCount;  // Slice count from 2D subresource
        slice_override = (depth_slices != 1);
    } else if ((VK_IMAGE_TYPE_3D == dst_image_state.create_info.imageType) &&
               (VK_IMAGE_TYPE_3D != src_image_state.create_info.imageType)) {
        depth_slices = region.srcSubresource.layerCount;  // Slice count from 2D subresource
        slice_override = (depth_slices != 1);
    }

    // Do all checks on source image
    if (src_image_state.create_info.imageType == VK_IMAGE_TYPE_1D) {
        if ((0 != region.srcOffset.y) || (1 != src_copy_extent.height)) {
            const LogObjectList objlist(handle, src_image_state.Handle());
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcImage1D_00146), objlist,
                             region_loc.dot(Field::srcOffset).dot(Field::y),
                             "is %" PRId32 " and extent.height is %" PRIu32
                             ". For 1D images these must "
                             "be 0 and 1, respectively.",
                             region.srcOffset.y, src_copy_extent.height);
        }
    }

    if (((src_image_state.create_info.imageType == VK_IMAGE_TYPE_1D) ||
         ((src_image_state.create_info.imageType == VK_IMAGE_TYPE_2D) && is_host)) &&
        ((0 != region.srcOffset.z) || (1 != src_copy_extent.depth))) {
        const LogObjectList objlist(handle, src_image_state.Handle());
        const char *image_type = is_host ? "1D or 2D" : "1D";
        skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcImage1D_01785), objlist,
                         region_loc.dot(Field::srcOffset).dot(Field::z),
                         "is %" PRId32 " and extent.depth is %" PRIu32
                         ". For %s images "
                         "these must be 0 and 1, respectively.",
                         region.srcOffset.z, src_copy_extent.depth, image_type);
    }

    if ((src_image_state.create_info.imageType == VK_IMAGE_TYPE_2D) && (0 != region.srcOffset.z) && (!is_host)) {
        const LogObjectList objlist(handle, src_image_state.Handle());
        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01787" : "VUID-vkCmdCopyImage-srcImage-01787";
        skip |= LogError(vuid, objlist, region_loc.dot(Field::srcOffset).dot(Field::z),
                         "is %" PRId32 ". For 2D images the z-offset must be 0.", region.srcOffset.z);
    }

    {  // Used to be compressed checks, now apply to all
        const VkFormat src_image_format = src_image_state.create_info.format;
        const VkExtent3D block_extent = vkuFormatTexelBlockExtent(src_image_format);
        if (SafeModulo(region.srcOffset.x, block_extent.width) != 0) {
            const LogObjectList objlist(handle, src_image_state.Handle());
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_07278), objlist,
                             region_loc.dot(Field::srcOffset).dot(Field::x),
                             "(%" PRId32
                             ") must be a multiple of the image (%s) texel block "
                             "width (%" PRIu32 ").",
                             region.srcOffset.x, string_VkFormat(src_image_format), block_extent.width);
        }

        //  image offsets y must be multiple of block height
        if (SafeModulo(region.srcOffset.y, block_extent.height) != 0) {
            const LogObjectList objlist(handle, src_image_state.Handle());
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_07279), objlist,
                             region_loc.dot(Field::srcOffset).dot(Field::y),
                             "(%" PRId32
                             ") must be a multiple of the image (%s) texel block "
                             "height (%" PRIu32 ").",
                             region.srcOffset.y, string_VkFormat(src_image_format), block_extent.height);
        }

        //  image offsets z must be multiple of block depth
        if (SafeModulo(region.srcOffset.z, block_extent.depth) != 0) {
            const LogObjectList objlist(handle, src_image_state.Handle());
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_07280), objlist,
                             region_loc.dot(Field::srcOffset).dot(Field::z),
                             "(%" PRId32
                             ") must be a multiple of the image (%s) texel block "
                             "depth (%" PRIu32 ").",
                             region.srcOffset.z, string_VkFormat(src_image_format), block_extent.depth);
        }

        const VkExtent3D mip_extent = src_image_state.GetEffectiveSubresourceExtent(region.srcSubresource);
        if ((SafeModulo(src_copy_extent.width, block_extent.width) != 0) &&
            (src_copy_extent.width + region.srcOffset.x != mip_extent.width)) {
            const LogObjectList objlist(handle, src_image_state.Handle());
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_01728), objlist,
                             region_loc.dot(Field::extent).dot(Field::width),
                             "(%" PRIu32
                             ") must be a multiple of the image (%s) texel block "
                             "width (%" PRIu32 "), or when added to srcOffset.x (%" PRId32
                             ") must equal the image subresource width (%" PRIu32 ").",
                             src_copy_extent.width, string_VkFormat(src_image_format), block_extent.width, region.srcOffset.x,
                             mip_extent.width);
        }

        // Extent height must be a multiple of block height, or extent+offset height must equal subresource height
        if ((SafeModulo(src_copy_extent.height, block_extent.height) != 0) &&
            (src_copy_extent.height + region.srcOffset.y != mip_extent.height)) {
            const LogObjectList objlist(handle, src_image_state.Handle());
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_01729), objlist,
                             region_loc.dot(Field::extent).dot(Field::height),
                             "(%" PRIu32
                             ") must be a multiple of the image (%s) texel block "
                             "height (%" PRIu32 "), or when added to srcOffset.y (%" PRId32
                             ") must equal the image subresource height (%" PRIu32 ").",
                             src_copy_extent.height, string_VkFormat(src_image_format), block_extent.height, region.srcOffset.y,
                             mip_extent.height);
        }

        // Extent depth must be a multiple of block depth, or extent+offset depth must equal subresource depth
        uint32_t copy_depth = (slice_override ? depth_slices : src_copy_extent.depth);
        if ((SafeModulo(copy_depth, block_extent.depth) != 0) && (copy_depth + region.srcOffset.z != mip_extent.depth)) {
            const LogObjectList objlist(handle, src_image_state.Handle());
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_01730), objlist,
                             region_loc.dot(Field::extent).dot(Field::depth),
                             "(%" PRIu32
                             ") must be a multiple of the image (%s) texel block "
                             "depth (%" PRIu32 "), or when added to srcOffset.z (%" PRId32
                             ") must equal the image subresource depth (%" PRIu32 ").",
                             src_copy_extent.depth, string_VkFormat(src_image_format), block_extent.depth, region.srcOffset.z,
                             mip_extent.depth);
        }
    }

    // Do all checks on dest image
    if (dst_image_state.create_info.imageType == VK_IMAGE_TYPE_1D) {
        if ((0 != region.dstOffset.y) || (1 != dst_copy_extent.height)) {
            const LogObjectList objlist(handle, dst_image_state.Handle());
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstImage1D_00152), objlist,
                             region_loc.dot(Field::dstOffset).dot(Field::y),
                             "is %" PRId32 " and extent.height is %" PRIu32
                             ". For 1D images "
                             "these must be 0 and 1, respectively.",
                             region.dstOffset.y, dst_copy_extent.height);
        }
    }

    if (((dst_image_state.create_info.imageType == VK_IMAGE_TYPE_1D) ||
         ((dst_image_state.create_info.imageType == VK_IMAGE_TYPE_2D) && is_host)) &&
        ((0 != region.dstOffset.z) || (1 != dst_copy_extent.depth))) {
        const LogObjectList objlist(handle, dst_image_state.Handle());
        const char *image_type = is_host ? "1D or 2D" : "1D";
        skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstImage1D_01786), objlist,
                         region_loc.dot(Field::dstOffset).dot(Field::z),
                         "is %" PRId32 " and extent.depth is %" PRIu32
                         ". For %s images these must be 0 "
                         "and 1, respectively.",
                         region.dstOffset.z, dst_copy_extent.depth, image_type);
    }

    if ((dst_image_state.create_info.imageType == VK_IMAGE_TYPE_2D) && (0 != region.dstOffset.z) && !(is_host)) {
        const LogObjectList objlist(handle, dst_image_state.Handle());
        vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01788" : "VUID-vkCmdCopyImage-dstImage-01788";
        skip |= LogError(vuid, objlist, region_loc.dot(Field::dstOffset).dot(Field::z),
                         "is %" PRId32 ". For 2D images the z-offset must be 0.", region.dstOffset.z);
    }

    // Handle difference between Maintenance 1
    if (IsExtEnabled(extensions.vk_khr_maintenance1) || is_host) {
        if (src_image_state.create_info.imageType == VK_IMAGE_TYPE_3D) {
            const LogObjectList objlist(handle, src_image_state.Handle());
            if ((0 != region.srcSubresource.baseArrayLayer) || (1 != region.srcSubresource.layerCount)) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcImage3D_04443), objlist,
                                 region_loc.dot(Field::srcSubresource).dot(Field::baseArrayLayer),
                                 "is %" PRIu32 " and srcSubresource.layerCount is %" PRIu32
                                 ". For VK_IMAGE_TYPE_3D images these must be 0 and 1, respectively.",
                                 region.srcSubresource.baseArrayLayer, region.srcSubresource.layerCount);
            }
        }
        if (dst_image_state.create_info.imageType == VK_IMAGE_TYPE_3D) {
            const LogObjectList objlist(handle, dst_image_state.Handle());
            if ((0 != region.dstSubresource.baseArrayLayer) || (1 != region.dstSubresource.layerCount)) {
                skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstImage3D_04444), objlist,
                                 region_loc.dot(Field::dstSubresource).dot(Field::baseArrayLayer),
                                 "is %" PRIu32 " and dstSubresource.layerCount is %" PRIu32
                                 ". For VK_IMAGE_TYPE_3D images these must be 0 and 1, respectively.",
                                 region.dstSubresource.baseArrayLayer, region.dstSubresource.layerCount);
            }
        }
    } else {  // Pre maint 1
        if (src_image_state.create_info.imageType == VK_IMAGE_TYPE_3D ||
            dst_image_state.create_info.imageType == VK_IMAGE_TYPE_3D) {
            if ((0 != region.srcSubresource.baseArrayLayer) || (1 != region.srcSubresource.layerCount)) {
                const LogObjectList objlist(handle, src_image_state.Handle());
                vuid = is_2 ? "VUID-VkCopyImageInfo2-apiVersion-07932" : "VUID-vkCmdCopyImage-apiVersion-07932";
                skip |= LogError(vuid, objlist, region_loc.dot(Field::srcSubresource).dot(Field::baseArrayLayer),
                                 "is %" PRIu32 " and srcSubresource.layerCount is %" PRIu32
                                 ". For copies with either source or dest of type "
                                 "VK_IMAGE_TYPE_3D, these must be 0 and 1, respectively.",
                                 region.srcSubresource.baseArrayLayer, region.srcSubresource.layerCount);
            }
            if ((0 != region.dstSubresource.baseArrayLayer) || (1 != region.dstSubresource.layerCount)) {
                const LogObjectList objlist(handle, dst_image_state.Handle());
                vuid = is_2 ? "VUID-VkCopyImageInfo2-apiVersion-07932" : "VUID-vkCmdCopyImage-apiVersion-07932";
                skip |= LogError(vuid, objlist, region_loc.dot(Field::dstSubresource).dot(Field::baseArrayLayer),
                                 "is %" PRIu32 " and dstSubresource.layerCount is %" PRIu32
                                 ". For copies with either source or dest of type "
                                 "VK_IMAGE_TYPE_3D, these must be 0 and 1, respectively.",
                                 region.dstSubresource.baseArrayLayer, region.dstSubresource.layerCount);
            }
        }
    }

    {
        const VkFormat dst_image_format = dst_image_state.create_info.format;
        const VkExtent3D block_extent = vkuFormatTexelBlockExtent(dst_image_format);
        //  image offsets x must be multiple of block width
        if (SafeModulo(region.dstOffset.x, block_extent.width) != 0) {
            const LogObjectList objlist(handle, src_image_state.Handle());
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstOffset_07281), objlist,
                             region_loc.dot(Field::dstOffset).dot(Field::x),
                             "(%" PRId32 ") must be a multiple of the image (%s) texel block width (%" PRIu32 ").",
                             region.dstOffset.x, string_VkFormat(dst_image_format), block_extent.width);
        }

        //  image offsets y must be multiple of block height
        if (SafeModulo(region.dstOffset.y, block_extent.height) != 0) {
            const LogObjectList objlist(handle, src_image_state.Handle());
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstOffset_07282), objlist,
                             region_loc.dot(Field::dstOffset).dot(Field::y),
                             "(%" PRId32 ") must be a multiple of the image (%s) texel block height (%" PRIu32 ").",
                             region.dstOffset.y, string_VkFormat(dst_image_format), block_extent.height);
        }

        //  image offsets z must be multiple of block depth
        if (SafeModulo(region.dstOffset.z, block_extent.depth) != 0) {
            const LogObjectList objlist(handle, src_image_state.Handle());
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstOffset_07283), objlist,
                             region_loc.dot(Field::dstOffset).dot(Field::z),
                             "(%" PRId32 ") must be a multiple of the image (%s) texel block depth (%" PRIu32 ").",
                             region.dstOffset.z, string_VkFormat(dst_image_format), block_extent.depth);
        }

        const VkExtent3D mip_extent = dst_image_state.GetEffectiveSubresourceExtent(region.dstSubresource);
        if ((SafeModulo(dst_copy_extent.width, block_extent.width) != 0) &&
            (dst_copy_extent.width + region.dstOffset.x != mip_extent.width)) {
            const LogObjectList objlist(handle, dst_image_state.Handle());
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstOffset_01732), objlist,
                             region_loc.dot(Field::extent).dot(Field::width),
                             "(%" PRIu32 ") must be a multiple of the image (%s) texel block width (%" PRIu32
                             "), or when added to dstOffset.x (%" PRId32 ") must equal the image subresource width (%" PRIu32 ").",
                             dst_copy_extent.width, string_VkFormat(dst_image_format), block_extent.width, region.dstOffset.x,
                             mip_extent.width);
        }

        // Extent height must be a multiple of block height, or dst_copy_extent+offset height must equal subresource height
        if ((SafeModulo(dst_copy_extent.height, block_extent.height) != 0) &&
            (dst_copy_extent.height + region.dstOffset.y != mip_extent.height)) {
            const LogObjectList objlist(handle, dst_image_state.Handle());
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstOffset_01733), objlist,
                             region_loc.dot(Field::extent).dot(Field::height),
                             "(%" PRIu32 ") must be a multiple of the image (%s) texel block height (%" PRIu32
                             "), or when added to dstOffset.y (%" PRId32
                             ") must equal the image subresource "
                             "height (%" PRIu32 ").",
                             dst_copy_extent.height, string_VkFormat(dst_image_format), block_extent.height, region.dstOffset.y,
                             mip_extent.height);
        }

        // Extent depth must be a multiple of block depth, or dst_copy_extent+offset depth must equal subresource depth
        uint32_t copy_depth = (slice_override ? depth_slices : dst_copy_extent.depth);
        if ((SafeModulo(copy_depth, block_extent.depth) != 0) && (copy_depth + region.dstOffset.z != mip_extent.depth)) {
            const LogObjectList objlist(handle, dst_image_state.Handle());
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstOffset_01734), objlist,
                             region_loc.dot(Field::extent).dot(Field::depth),
                             "(%" PRIu32 ") must be a multiple of the image (%s) texel block depth (%" PRIu32
                             "), or when added to dstOffset.z (%" PRId32 ") must equal the image subresource depth (%" PRIu32 ").",
                             dst_copy_extent.depth, string_VkFormat(dst_image_format), block_extent.depth, region.dstOffset.z,
                             mip_extent.depth);
        }
    }

    return skip;
}

// Returns non-zero if offset and extent exceed image extents
static constexpr uint32_t kXBit = 1;
static constexpr uint32_t kYBit = 2;
static constexpr uint32_t kZBit = 4;
static uint32_t ExceedsBounds(const VkOffset3D *offset, const VkExtent3D *extent, const VkExtent3D *image_extent) {
    uint32_t result = 0;
    // Extents/depths cannot be negative but checks left in for clarity
    if ((offset->z + extent->depth > image_extent->depth) || (offset->z < 0) ||
        ((offset->z + static_cast<int32_t>(extent->depth)) < 0)) {
        result |= kZBit;
    }
    if ((offset->y + extent->height > image_extent->height) || (offset->y < 0) ||
        ((offset->y + static_cast<int32_t>(extent->height)) < 0)) {
        result |= kYBit;
    }
    if ((offset->x + extent->width > image_extent->width) || (offset->x < 0) ||
        ((offset->x + static_cast<int32_t>(extent->width)) < 0)) {
        result |= kXBit;
    }
    return result;
}

template <typename HandleT, typename RegionType>
bool CoreChecks::ValidateCopyImageRegionCommon(HandleT handle, const vvl::Image &src_image_state, const vvl::Image &dst_image_state,
                                               const RegionType &region, const Location &region_loc) const {
    bool skip = false;
    const Location src_subresource_loc = region_loc.dot(Field::srcSubresource);
    const Location dst_subresource_loc = region_loc.dot(Field::dstSubresource);
    const VkFormat src_format = src_image_state.create_info.format;
    const VkFormat dst_format = dst_image_state.create_info.format;

    // src
    {
        skip |= ValidateImageSubresourceLayers(handle, region.srcSubresource, src_subresource_loc);
        skip |= ValidateImageMipLevel(handle, src_image_state, region.srcSubresource.mipLevel, src_subresource_loc);
        skip |= ValidateImageArrayLayerRange(handle, src_image_state, region.srcSubresource.baseArrayLayer,
                                             region.srcSubresource.layerCount, src_subresource_loc);

        // For each region, the aspectMask member of srcSubresource must be present in the source image
        if (!IsValidAspectMaskForFormat(region.srcSubresource.aspectMask, src_format)) {
            const LogObjectList src_objlist(handle, src_image_state.VkHandle());
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcSubresource_00142), src_objlist,
                             src_subresource_loc.dot(Field::aspectMask), "(%s) is invalid for source image format %s. (%s)",
                             string_VkImageAspectFlags(region.srcSubresource.aspectMask).c_str(), string_VkFormat(src_format),
                             DescribeValidAspectMaskForFormat(src_format).c_str());
        }
    }

    // dst
    {
        skip |= ValidateImageSubresourceLayers(handle, region.dstSubresource, dst_subresource_loc);
        skip |= ValidateImageMipLevel(handle, dst_image_state, region.dstSubresource.mipLevel, dst_subresource_loc);
        skip |= ValidateImageArrayLayerRange(handle, dst_image_state, region.dstSubresource.baseArrayLayer,
                                             region.dstSubresource.layerCount, dst_subresource_loc);
        // For each region, the aspectMask member of dstSubresource must be present in the destination image
        if (!IsValidAspectMaskForFormat(region.dstSubresource.aspectMask, dst_format)) {
            const LogObjectList dst_objlist(handle, dst_image_state.VkHandle());
            skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstSubresource_00143), dst_objlist,
                             dst_subresource_loc.dot(Field::aspectMask), "(%s) is invalid for destination image format %s. (%s)",
                             string_VkImageAspectFlags(region.dstSubresource.aspectMask).c_str(), string_VkFormat(dst_format),
                             DescribeValidAspectMaskForFormat(dst_format).c_str());
        }
    }

    const bool is_2 = region_loc.function == Func::vkCmdCopyImage2 || region_loc.function == Func::vkCmdCopyImage2KHR;
    const bool is_host = region_loc.function == Func::vkCopyImageToImage || region_loc.function == Func::vkCopyImageToImageEXT;

    // For comp/uncomp copies, the copy extent for the dest image must be adjusted
    VkExtent3D src_copy_extent = region.extent;
    VkExtent3D dst_copy_extent = GetAdjustedDstImageExtent(src_format, dst_format, region.extent);

    bool slice_override = false;
    uint32_t depth_slices = 0;

    const bool src_is_3d = (VK_IMAGE_TYPE_3D == src_image_state.create_info.imageType);
    const bool dst_is_3d = (VK_IMAGE_TYPE_3D == dst_image_state.create_info.imageType);
    // Special case for copying between a 1D/2D array and a 3D image
    // TBD: This seems like the only way to reconcile 3 mutually-exclusive VU checks for 2D/3D copies. Heads up.
    if (src_is_3d && !dst_is_3d) {
        depth_slices = region.dstSubresource.layerCount;  // Slice count from 2D subresource
        slice_override = (depth_slices != 1);
    } else if (dst_is_3d && !src_is_3d) {
        depth_slices = region.srcSubresource.layerCount;  // Slice count from 2D subresource
        slice_override = (depth_slices != 1);
    }

    if (api_version < VK_API_VERSION_1_1) {
        if (!IsExtEnabled(extensions.vk_khr_maintenance1)) {
            // For each region the layerCount member of srcSubresource and dstSubresource must match
            if (region.srcSubresource.layerCount != region.dstSubresource.layerCount) {
                const LogObjectList objlist(handle, src_image_state.VkHandle(), dst_image_state.VkHandle());
                const char *vuid = (is_2 || is_host) ? "VUID-VkImageCopy2-apiVersion-07941" : "VUID-VkImageCopy-apiVersion-07941";
                skip |= LogError(vuid, objlist, src_subresource_loc.dot(Field::layerCount),
                                 "(%" PRIu32 ") does not match %s (%" PRIu32 ").", region.srcSubresource.layerCount,
                                 dst_subresource_loc.dot(Field::layerCount).Fields().c_str(), region.dstSubresource.layerCount);
            }
        }
        if (!IsExtEnabled(extensions.vk_khr_sampler_ycbcr_conversion)) {
            // For each region the aspectMask member of srcSubresource and dstSubresource must match
            if (region.srcSubresource.aspectMask != region.dstSubresource.aspectMask) {
                const LogObjectList objlist(handle, src_image_state.VkHandle(), dst_image_state.VkHandle());
                const char *vuid = (is_2 || is_host) ? "VUID-VkImageCopy2-apiVersion-07940" : "VUID-VkImageCopy-apiVersion-07940";
                skip |= LogError(vuid, objlist, src_subresource_loc.dot(Field::aspectMask), "(%s) does not match %s (%s).",
                                 string_VkImageAspectFlags(region.srcSubresource.aspectMask).c_str(),
                                 dst_subresource_loc.dot(Field::aspectMask).Fields().c_str(),
                                 string_VkImageAspectFlags(region.dstSubresource.aspectMask).c_str());
            }
        }
    }

    // Make sure not a empty region
    if (src_copy_extent.width == 0) {
        const char *vuid = (is_2 || is_host) ? "VUID-VkImageCopy2-extent-06668" : "VUID-VkImageCopy-extent-06668";
        const LogObjectList src_objlist(handle, src_image_state.VkHandle());
        skip |= LogError(vuid, src_objlist, region_loc.dot(Field::extent).dot(Field::width),
                         "is zero. (empty copies are not allowed).");
    }
    if (src_copy_extent.height == 0) {
        const char *vuid = (is_2 || is_host) ? "VUID-VkImageCopy2-extent-06669" : "VUID-VkImageCopy-extent-06669";
        const LogObjectList src_objlist(handle, src_image_state.VkHandle());
        skip |= LogError(vuid, src_objlist, region_loc.dot(Field::extent).dot(Field::height),
                         "is zero. (empty copies are not allowed).");
    }
    if (src_copy_extent.depth == 0) {
        const char *vuid = (is_2 || is_host) ? "VUID-VkImageCopy2-extent-06670" : "VUID-VkImageCopy-extent-06670";
        const LogObjectList src_objlist(handle, src_image_state.VkHandle());
        skip |= LogError(vuid, src_objlist, region_loc.dot(Field::extent).dot(Field::depth),
                         "is zero. (empty copies are not allowed).");
    }

    // Each dimension offset + extent limits must fall with image subresource extent
    VkExtent3D subresource_extent = src_image_state.GetEffectiveSubresourceExtent(region.srcSubresource);
    if (slice_override) src_copy_extent.depth = depth_slices;
    uint32_t extent_check = ExceedsBounds(&(region.srcOffset), &src_copy_extent, &subresource_extent);
    if (extent_check & kXBit) {
        const LogObjectList src_objlist(handle, src_image_state.VkHandle());
        skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_00144), src_objlist,
                         region_loc.dot(Field::srcOffset).dot(Field::x),
                         "(%" PRId32 ") + extent (%" PRIu32 ") exceeds miplevel %" PRIu32 " width (%" PRIu32 ").",
                         region.srcOffset.x, src_copy_extent.width, region.srcSubresource.mipLevel, subresource_extent.width);
    }

    if (extent_check & kYBit) {
        const LogObjectList src_objlist(handle, src_image_state.VkHandle());
        skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_00145), src_objlist,
                         region_loc.dot(Field::srcOffset).dot(Field::y),
                         "(%" PRId32 ") + extent (%" PRIu32 ") exceeds miplevel %" PRIu32 " height (%" PRIu32 ").",
                         region.srcOffset.y, src_copy_extent.height, region.srcSubresource.mipLevel, subresource_extent.height);
    }
    if (extent_check & kZBit) {
        const LogObjectList src_objlist(handle, src_image_state.VkHandle());
        skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::SrcOffset_00147), src_objlist,
                         region_loc.dot(Field::srcOffset).dot(Field::z),
                         "(%" PRId32 ") + extent (%" PRIu32 ") exceeds miplevel %" PRIu32 " depth (%" PRIu32 ").",
                         region.srcOffset.z, src_copy_extent.depth, region.srcSubresource.mipLevel, subresource_extent.depth);
    }

    // Adjust dest extent if necessary
    subresource_extent = dst_image_state.GetEffectiveSubresourceExtent(region.dstSubresource);
    if (slice_override) dst_copy_extent.depth = depth_slices;

    extent_check = ExceedsBounds(&(region.dstOffset), &dst_copy_extent, &subresource_extent);
    if (extent_check & kXBit) {
        const LogObjectList dst_objlist(handle, dst_image_state.VkHandle());
        skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstOffset_00150), dst_objlist,
                         region_loc.dot(Field::dstOffset).dot(Field::x),
                         "(%" PRId32 ") + extent (%" PRIu32 ") exceeds miplevel %" PRIu32 " width (%" PRIu32 ").",
                         region.dstOffset.x, dst_copy_extent.width, region.dstSubresource.mipLevel, subresource_extent.width);
    }
    if (extent_check & kYBit) {
        const LogObjectList dst_objlist(handle, dst_image_state.VkHandle());
        skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstOffset_00151), dst_objlist,
                         region_loc.dot(Field::dstOffset).dot(Field::y),
                         "(%" PRId32 ") + extent (%" PRIu32 ") exceeds miplevel %" PRIu32 " height (%" PRIu32 ").",
                         region.dstOffset.y, dst_copy_extent.height, region.dstSubresource.mipLevel, subresource_extent.height);
    }
    if (extent_check & kZBit) {
        const LogObjectList dst_objlist(handle, dst_image_state.VkHandle());
        skip |= LogError(GetCopyImageVUID(region_loc, vvl::CopyError::DstOffset_00153), dst_objlist,
                         region_loc.dot(Field::dstOffset).dot(Field::z),
                         "(%" PRId32 ") + extent (%" PRIu32 ") exceeds miplevel %" PRIu32 " depth (%" PRIu32 ").",
                         region.dstOffset.z, dst_copy_extent.depth, region.dstSubresource.mipLevel, subresource_extent.depth);
    }

    return skip;
}

template <typename HandleT>
bool CoreChecks::ValidateCopyImageCommon(HandleT handle, const vvl::Image &src_image_state, const vvl::Image &dst_image_state,
                                         const Location &loc) const {
    bool skip = false;

    // src
    {
        const LogObjectList src_objlist(handle, src_image_state.VkHandle());
        const Location src_image_loc = loc.dot(Field::srcImage);
        skip |= ValidateMemoryIsBoundToImage(src_objlist, src_image_state, src_image_loc,
                                             GetCopyImageVUID(loc, vvl::CopyError::SrcImageContiguous_07966).c_str());
        if (src_image_state.create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            skip |= LogError(GetCopyImageVUID(loc, vvl::CopyError::SrcImageSubsampled_07969), src_objlist, src_image_loc,
                             "was created with flags including VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT");
        }
    }

    // dst
    {
        const LogObjectList dst_objlist(handle, dst_image_state.VkHandle());
        const Location dst_image_loc = loc.dot(Field::dstImage);
        skip |= ValidateMemoryIsBoundToImage(dst_objlist, dst_image_state, dst_image_loc,
                                             GetCopyImageVUID(loc, vvl::CopyError::DstImageContiguous_07966).c_str());
        if (dst_image_state.create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            skip |= LogError(GetCopyImageVUID(loc, vvl::CopyError::DstImageSubsampled_07969), dst_objlist, dst_image_loc,
                             "was created with flags including VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT");
        }
    }

    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const RegionType *pRegions, const Location &loc) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<vvl::CommandBuffer>(commandBuffer);
    auto src_image_state = Get<vvl::Image>(srcImage);
    auto dst_image_state = Get<vvl::Image>(dstImage);
    ASSERT_AND_RETURN_SKIP(src_image_state && dst_image_state);

    const vvl::CommandBuffer &cb_state = *cb_state_ptr;
    const VkFormat src_format = src_image_state->create_info.format;
    const VkFormat dst_format = dst_image_state->create_info.format;
    const VkImageType src_image_type = src_image_state->create_info.imageType;
    const VkImageType dst_image_type = dst_image_state->create_info.imageType;
    const bool src_is_2d = (VK_IMAGE_TYPE_2D == src_image_type);
    const bool src_is_3d = (VK_IMAGE_TYPE_3D == src_image_type);
    const bool dst_is_2d = (VK_IMAGE_TYPE_2D == dst_image_type);
    const bool dst_is_3d = (VK_IMAGE_TYPE_3D == dst_image_type);
    const bool is_2 = loc.function == Func::vkCmdCopyImage2 || loc.function == Func::vkCmdCopyImage2KHR;

    const char *vuid;
    const Location src_image_loc = loc.dot(Field::srcImage);
    const Location dst_image_loc = loc.dot(Field::dstImage);

    const LogObjectList src_objlist(commandBuffer, srcImage);
    const LogObjectList dst_objlist(commandBuffer, dstImage);
    const LogObjectList all_objlist(commandBuffer, srcImage, dstImage);

    skip |= ValidateCopyImageCommon(commandBuffer, *src_image_state, *dst_image_state, loc);

    bool has_stencil_aspect = false;
    bool has_non_stencil_aspect = false;
    const bool same_image = (src_image_state == dst_image_state);
    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location src_subresource_loc = region_loc.dot(Field::srcSubresource);
        const Location dst_subresource_loc = region_loc.dot(Field::dstSubresource);
        const RegionType &region = pRegions[i];
        const VkImageSubresourceLayers &src_subresource = region.srcSubresource;
        const VkImageSubresourceLayers &dst_subresource = region.dstSubresource;

        // For comp/uncomp copies, the copy extent for the dest image must be adjusted
        VkExtent3D src_copy_extent = region.extent;
        VkExtent3D dst_copy_extent = GetAdjustedDstImageExtent(src_format, dst_format, region.extent);

        bool slice_override = false;
        uint32_t depth_slices = 0;

        // Special case for copying between a 1D/2D array and a 3D image
        // TBD: This seems like the only way to reconcile 3 mutually-exclusive VU checks for 2D/3D copies. Heads up.
        if (src_is_3d && !dst_is_3d) {
            depth_slices = dst_subresource.layerCount;  // Slice count from 2D subresource
            slice_override = (depth_slices != 1);
        } else if (dst_is_3d && !src_is_3d) {
            depth_slices = src_subresource.layerCount;  // Slice count from 2D subresource
            slice_override = (depth_slices != 1);
        }

        if (IsExtEnabled(extensions.vk_khr_maintenance1)) {
            // No chance of mismatch if we're overriding depth slice count
            if (!slice_override) {
                // The number of depth slices in srcSubresource and dstSubresource must match
                // Depth comes from layerCount for 1D,2D resources, from extent.depth for 3D
                uint32_t src_slices = (src_is_3d ? src_copy_extent.depth : src_subresource.layerCount);
                uint32_t dst_slices = (dst_is_3d ? dst_copy_extent.depth : dst_subresource.layerCount);
                if (src_slices == VK_REMAINING_ARRAY_LAYERS || dst_slices == VK_REMAINING_ARRAY_LAYERS) {
                    if (src_slices != VK_REMAINING_ARRAY_LAYERS) {
                        if (src_slices != (dst_image_state->create_info.arrayLayers - dst_subresource.baseArrayLayer)) {
                            vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-08794" : "VUID-vkCmdCopyImage-srcImage-08794";
                            skip |= LogError(vuid, dst_objlist, src_subresource_loc.dot(Field::layerCount),
                                             "(%" PRIu32 ") does not match dstImage arrayLayers (%" PRIu32
                                             ") minus baseArrayLayer (%" PRIu32 ").",
                                             src_slices, dst_image_state->create_info.arrayLayers, dst_subresource.baseArrayLayer);
                        }
                    } else if (dst_slices != VK_REMAINING_ARRAY_LAYERS) {
                        if (dst_slices != (src_image_state->create_info.arrayLayers - src_subresource.baseArrayLayer)) {
                            vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-08794" : "VUID-vkCmdCopyImage-srcImage-08794";
                            skip |= LogError(vuid, src_objlist, dst_subresource_loc.dot(Field::layerCount),
                                             "(%" PRIu32 ") does not match srcImage arrayLayers (%" PRIu32
                                             ") minus baseArrayLayer (%" PRIu32 ").",
                                             dst_slices, src_image_state->create_info.arrayLayers, src_subresource.baseArrayLayer);
                        }
                    }
                } else if (src_slices != dst_slices) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-08793" : "VUID-vkCmdCopyImage-srcImage-08793";
                    skip |= LogError(vuid, all_objlist, region_loc, "%s (%" PRIu32 ") is different from %s (%" PRIu32 ").",
                                     src_is_3d ? "extent.depth" : "srcSubresource.layerCount", src_slices,
                                     dst_is_3d ? "extent.depth" : "dstSubresource.layerCount", dst_slices);
                }
            }
            // Maintenance 1 requires both while prior only required one to be 2D
            if ((src_is_2d && dst_is_2d) && (src_copy_extent.depth != 1)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01790" : "VUID-vkCmdCopyImage-srcImage-01790";
                skip |= LogError(vuid, all_objlist, region_loc,
                                 "both srcImage and dstImage are 2D and extent.depth is %" PRIu32 " and has to be 1",
                                 src_copy_extent.depth);
            }

            if (src_image_type != dst_image_type) {
                // if different, one must be 3D and the other 2D
                const bool valid = (src_is_2d && dst_is_3d) || (src_is_3d && dst_is_2d) || enabled_features.maintenance5;
                if (!valid) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-07743" : "VUID-vkCmdCopyImage-srcImage-07743";
                    skip |=
                        LogError(vuid, all_objlist, region_loc,
                                 "srcImage type (%s) must be equal to dstImage type (%s) or else one must be 2D and the other 3D",
                                 string_VkImageType(src_image_type), string_VkImageType(dst_image_type));
                }
            }

            vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01995" : "VUID-vkCmdCopyImage-srcImage-01995";
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state, VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT,
                                                    src_image_loc, vuid);
            vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01996" : "VUID-vkCmdCopyImage-dstImage-01996";
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, *dst_image_state, VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT,
                                                    dst_image_loc, vuid);

            // Check if 2D with 3D and depth not equal to 2D layerCount
            if (src_is_2d && dst_is_3d && (src_copy_extent.depth != src_subresource.layerCount)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01791" : "VUID-vkCmdCopyImage-srcImage-01791";
                skip |= LogError(vuid, all_objlist, region_loc,
                                 "srcImage is 2D, dstImage is 3D and extent.depth is %" PRIu32
                                 " and has to be "
                                 "srcSubresource.layerCount (%" PRIu32 ")",
                                 src_copy_extent.depth, src_subresource.layerCount);
            } else if (src_is_3d && dst_is_2d && (src_copy_extent.depth != dst_subresource.layerCount)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01792" : "VUID-vkCmdCopyImage-dstImage-01792";
                skip |= LogError(vuid, all_objlist, region_loc,
                                 "srcImage is 3D, dstImage is 2D and extent.depth is %" PRIu32
                                 " and has to be "
                                 "dstSubresource.layerCount (%" PRIu32 ")",
                                 src_copy_extent.depth, dst_subresource.layerCount);
            }
        } else {  // !vk_khr_maintenance1
            if ((src_is_2d || dst_is_2d) && (src_copy_extent.depth != 1)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-apiVersion-08969" : "VUID-vkCmdCopyImage-apiVersion-08969";
                skip |= LogError(vuid, all_objlist, region_loc, "srcImage is %s is dstImage is %s but extent.depth is %" PRIu32 ".",
                                 string_VkImageType(src_image_type), string_VkImageType(dst_image_type), src_copy_extent.depth);
            }

            if (src_image_type != dst_image_type) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-apiVersion-07933" : "VUID-vkCmdCopyImage-apiVersion-07933";
                skip |= LogError(vuid, all_objlist, region_loc,
                                 "srcImage (%s) must be equal to dstImage (%s) without VK_KHR_maintenance1 enabled",
                                 string_VkImageType(src_image_type), string_VkImageType(dst_image_type));
            }
        }

        const VkImageAspectFlags src_aspect = src_subresource.aspectMask;
        const VkImageAspectFlags dst_aspect = dst_subresource.aspectMask;
        const bool is_src_multiplane = vkuFormatIsMultiplane(src_format);
        const bool is_dst_multiplane = vkuFormatIsMultiplane(dst_format);
        if (!is_src_multiplane && !is_dst_multiplane) {
            // If neither image is multi-plane the aspectMask member of src and dst must match
            if (src_aspect != dst_aspect && !enabled_features.maintenance8) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01551" : "VUID-vkCmdCopyImage-srcImage-01551";
                skip |= LogError(
                    vuid, all_objlist, src_subresource_loc.dot(Field::aspectMask),
                    "(%s) does not match %s (%s). (This can be allowed in some cases if maintenance8 feature is enabled)",
                    string_VkImageAspectFlags(src_aspect).c_str(), dst_subresource_loc.dot(Field::aspectMask).Fields().c_str(),
                    string_VkImageAspectFlags(dst_aspect).c_str());
            }

            if (!AreFormatsSizeCompatible(dst_format, src_format)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01548" : "VUID-vkCmdCopyImage-srcImage-01548";
                skip |=
                    LogError(vuid, all_objlist, loc, "srcImage format (%s) is not size-compatible with dstImage format (%s). %s",
                             string_VkFormat(src_format), string_VkFormat(dst_format),
                             DescribeFormatsSizeCompatible(src_format, dst_format).c_str());
            }
        } else {
            // Here we might be copying between 2 multi-planar formats, or color to/from multi-planar

            // Source image multiplane checks
            if (is_src_multiplane && !IsOnlyOneValidPlaneAspect(src_format, src_aspect)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-08713" : "VUID-vkCmdCopyImage-srcImage-08713";
                skip |= LogError(vuid, src_objlist, src_subresource_loc.dot(Field::aspectMask),
                                 "(%s) is invalid for multi-planar format %s.", string_VkImageAspectFlags(src_aspect).c_str(),
                                 string_VkFormat(src_format));
            }
            // Single-plane to multi-plane
            if (!is_src_multiplane && is_dst_multiplane && VK_IMAGE_ASPECT_COLOR_BIT != src_aspect) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-01557" : "VUID-vkCmdCopyImage-dstImage-01557";
                skip |= LogError(vuid, all_objlist, src_subresource_loc.dot(Field::aspectMask),
                                 "(%s) needs VK_IMAGE_ASPECT_COLOR_BIT\nsrcImage format %s\ndstImage format %s\n.",
                                 string_VkImageAspectFlags(src_aspect).c_str(), string_VkFormat(src_format),
                                 string_VkFormat(dst_format));
            }

            // Dest image multiplane checks
            if (is_dst_multiplane && !IsOnlyOneValidPlaneAspect(dst_format, dst_aspect)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImage-08714" : "VUID-vkCmdCopyImage-dstImage-08714";
                skip |= LogError(vuid, dst_objlist, dst_subresource_loc.dot(Field::aspectMask),
                                 "(%s) is invalid for multi-planar format %s.", string_VkImageAspectFlags(dst_aspect).c_str(),
                                 string_VkFormat(dst_format));
            }
            // Multi-plane to single-plane
            if (is_src_multiplane && !is_dst_multiplane && VK_IMAGE_ASPECT_COLOR_BIT != dst_aspect) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-01556" : "VUID-vkCmdCopyImage-srcImage-01556";
                skip |= LogError(vuid, all_objlist, dst_subresource_loc.dot(Field::aspectMask),
                                 "(%s) needs VK_IMAGE_ASPECT_COLOR_BIT\nsrcImage format %s\ndstImage format %s\n.",
                                 string_VkImageAspectFlags(dst_aspect).c_str(), string_VkFormat(src_format),
                                 string_VkFormat(dst_format));
            }

            const VkFormat src_plane_format =
                is_src_multiplane ? vkuFindMultiplaneCompatibleFormat(src_format, static_cast<VkImageAspectFlagBits>(src_aspect))
                                  : src_format;
            const VkFormat dst_plane_format =
                is_dst_multiplane ? vkuFindMultiplaneCompatibleFormat(dst_format, static_cast<VkImageAspectFlagBits>(dst_aspect))
                                  : dst_format;
            const uint32_t src_format_size = vkuFormatTexelBlockSize(src_plane_format);
            const uint32_t dst_format_size = vkuFormatTexelBlockSize(dst_plane_format);

            // If size is still zero, then format is invalid and will be caught in another VU
            if ((src_format_size != dst_format_size) && (src_format_size != 0) && (dst_format_size != 0)) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-None-01549" : "VUID-vkCmdCopyImage-None-01549";
                std::stringstream ss;
                ss << "srcImage format " << string_VkFormat(src_plane_format);
                if (is_src_multiplane) {
                    ss << " (which is the compatible format for plane "
                       << vkuGetPlaneIndex(static_cast<VkImageAspectFlagBits>(src_aspect)) << " of " << string_VkFormat(src_format)
                       << ")";
                }
                ss << " has texel block size of " << src_format_size << " which is different than the dstImage format "
                   << string_VkFormat(dst_plane_format);
                if (is_dst_multiplane) {
                    ss << " (which is the compatible format for plane "
                       << vkuGetPlaneIndex(static_cast<VkImageAspectFlagBits>(dst_aspect)) << " of " << string_VkFormat(dst_format)
                       << ")";
                }
                ss << " which has texel block size of " << dst_format_size;
                skip |= LogError(vuid, all_objlist, region_loc, "%s", ss.str().c_str());
            }
        }

        if (enabled_features.maintenance8) {
            const VkImageAspectFlags both_depth_and_stencil = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
            if (src_aspect == VK_IMAGE_ASPECT_COLOR_BIT) {
                if ((dst_aspect & both_depth_and_stencil) == both_depth_and_stencil) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-srcSubresource-10214" : "VUID-vkCmdCopyImage-srcSubresource-10214";
                    skip |=
                        LogError(vuid, all_objlist, src_subresource_loc.dot(Field::aspectMask),
                                 "is VK_IMAGE_ASPECT_COLOR_BIT but dstSubresource.aspectMask contains both Depth and Stencil (%s).",
                                 string_VkImageAspectFlags(dst_aspect).c_str());
                } else if (dst_aspect == VK_IMAGE_ASPECT_DEPTH_BIT || dst_aspect == VK_IMAGE_ASPECT_STENCIL_BIT) {
                    if (!vkuFormatIsDepthStencilWithColorSizeCompatible(src_format, dst_format)) {
                        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcSubresource-10211" : "VUID-vkCmdCopyImage-srcSubresource-10211";
                        skip |= LogError(vuid, all_objlist, src_subresource_loc.dot(Field::aspectMask),
                                         "is VK_IMAGE_ASPECT_COLOR_BIT and dstSubresource.aspectMask is %s, but the src color "
                                         "format (%s) is not "
                                         "compatible with the dst depth/stencil format (%s).",
                                         string_VkImageAspectFlags(dst_aspect).c_str(), string_VkFormat(src_format),
                                         string_VkFormat(dst_format));
                    }
                }
            }

            if (dst_aspect == VK_IMAGE_ASPECT_COLOR_BIT) {
                if ((src_aspect & both_depth_and_stencil) == both_depth_and_stencil) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-dstSubresource-10215" : "VUID-vkCmdCopyImage-dstSubresource-10215";
                    skip |=
                        LogError(vuid, all_objlist, dst_subresource_loc.dot(Field::aspectMask),
                                 "is VK_IMAGE_ASPECT_COLOR_BIT but srcSubresource.aspectMask contains both Depth and Stencil (%s).",
                                 string_VkImageAspectFlags(src_aspect).c_str());
                } else if (src_aspect == VK_IMAGE_ASPECT_DEPTH_BIT || src_aspect == VK_IMAGE_ASPECT_STENCIL_BIT) {
                    if (!vkuFormatIsDepthStencilWithColorSizeCompatible(dst_format, src_format)) {
                        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcSubresource-10212" : "VUID-vkCmdCopyImage-srcSubresource-10212";
                        skip |= LogError(vuid, all_objlist, dst_subresource_loc.dot(Field::aspectMask),
                                         "is VK_IMAGE_ASPECT_COLOR_BIT and srcSubresource.aspectMask is (%s), but the src "
                                         "depth/stencil format (%s) is not "
                                         "compatible with the dst color format (%s).",
                                         string_VkImageAspectFlags(dst_aspect).c_str(), string_VkFormat(src_format),
                                         string_VkFormat(dst_format));
                    }
                }
            }
        }

        // The union of all source regions, and the union of all destination regions, specified by the elements of regions,
        // must not overlap in memory
        // Validation is only performed when source image is the same as destination image.
        // In the general case, the mapping between an image and its underlying memory is undefined,
        // so checking for memory overlaps is not possible.
        if (srcImage == dstImage) {
            for (uint32_t j = 0; j < regionCount; j++) {
                if (auto intersection = GetRegionIntersection(region, pRegions[j], src_image_type, is_src_multiplane);
                    intersection.has_instersection) {
                    vuid = is_2 ? "VUID-VkCopyImageInfo2-pRegions-00124" : "VUID-vkCmdCopyImage-pRegions-00124";
                    skip |= LogError(vuid, all_objlist, loc,
                                     "pRegion[%" PRIu32 "] copy source overlaps with pRegions[%" PRIu32
                                     "] copy destination. Overlap info, with respect to image (%s):%s",
                                     i, j, FormatHandle(srcImage).c_str(), intersection.String().c_str());
                }
            }
        }

        // track aspect mask in loop through regions
        if ((src_aspect & VK_IMAGE_ASPECT_STENCIL_BIT) != 0) {
            has_stencil_aspect = true;
        }
        if ((src_aspect & (~VK_IMAGE_ASPECT_STENCIL_BIT)) != 0) {
            has_non_stencil_aspect = true;
        }

        // When performing copy from and to same subresource, VK_IMAGE_LAYOUT_GENERAL is the only option
        const bool same_subresource = (same_image && (src_subresource.mipLevel == dst_subresource.mipLevel) &&
                                       RangesIntersect(src_subresource.baseArrayLayer, src_subresource.layerCount,
                                                       dst_subresource.baseArrayLayer, dst_subresource.layerCount));
        if (same_subresource) {
            if (!IsValueIn(srcImageLayout, {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_GENERAL}) ||
                !IsValueIn(dstImageLayout, {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_GENERAL})) {
                vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-09460" : "VUID-vkCmdCopyImage-srcImage-09460";
                skip |= LogError(vuid, src_objlist, loc,
                                 "copying to same VkImage (miplevel = %u, srcLayer[%u,%u), dstLayer[%u,%u)), but srcImageLayout is "
                                 "%s and dstImageLayout is %s",
                                 src_subresource.mipLevel, src_subresource.baseArrayLayer,
                                 src_subresource.baseArrayLayer + src_subresource.layerCount, dst_subresource.baseArrayLayer,
                                 dst_subresource.baseArrayLayer + dst_subresource.layerCount, string_VkImageLayout(srcImageLayout),
                                 string_VkImageLayout(dstImageLayout));
            }
        }

        skip |= ValidateImageCopyData(commandBuffer, region, *src_image_state, *dst_image_state, false, region_loc);

        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImageLayout-00128" : "VUID-vkCmdCopyImage-srcImageLayout-00128";
        skip |= VerifyImageLayoutSubresource(cb_state, *src_image_state, src_subresource, srcImageLayout, src_image_loc, vuid);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImageLayout-00133" : "VUID-vkCmdCopyImage-dstImageLayout-00133";
        skip |= VerifyImageLayoutSubresource(cb_state, *dst_image_state, dst_subresource, dstImageLayout, dst_image_loc, vuid);
        skip |= ValidateCopyImageTransferGranularityRequirements(cb_state, *src_image_state, *dst_image_state, region, region_loc);
        skip |= ValidateCopyImageRegionCommon(commandBuffer, *src_image_state, *dst_image_state, region, region_loc);
        vuid = is_2 ? "VUID-vkCmdCopyImage2-commandBuffer-10217" : "VUID-vkCmdCopyImage-commandBuffer-10217";
        skip |= ValidateQueueFamilySupport(cb_state, *physical_device_state, src_aspect, *src_image_state,
                                           region_loc.dot(Field::srcSubresource).dot(Field::aspectMask), vuid);
        vuid = is_2 ? "VUID-vkCmdCopyImage2-commandBuffer-10218" : "VUID-vkCmdCopyImage-commandBuffer-10218";
        skip |= ValidateQueueFamilySupport(cb_state, *physical_device_state, dst_aspect, *dst_image_state,
                                           region_loc.dot(Field::dstSubresource).dot(Field::aspectMask), vuid);
    }

    if (vkuFormatIsCompressed(src_format) && vkuFormatIsCompressed(dst_format)) {
        const VkExtent3D src_block_extent = vkuFormatTexelBlockExtent(src_format);
        const VkExtent3D dst_block_extent = vkuFormatTexelBlockExtent(dst_format);
        if (src_block_extent.width != dst_block_extent.width || src_block_extent.height != dst_block_extent.height ||
            src_block_extent.depth != dst_block_extent.depth) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-09247" : "VUID-vkCmdCopyImage-srcImage-09247";
            skip |= LogError(vuid, all_objlist, loc,
                             "srcImage format %s has texel block extent (%s) and dstImage format %s has texel block extent (%s).",
                             string_VkFormat(src_format), string_VkExtent3D(src_block_extent).c_str(), string_VkFormat(dst_format),
                             string_VkExtent3D(dst_block_extent).c_str());
        }
    }

    // Validate that SRC & DST images have correct usage flags set
    if (!IsExtEnabled(extensions.vk_ext_separate_stencil_usage)) {
        vuid = is_2 ? "VUID-VkCopyImageInfo2-aspect-06662" : "VUID-vkCmdCopyImage-aspect-06662";
        skip |=
            ValidateImageUsageFlags(commandBuffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, false, vuid, src_image_loc);
        vuid = is_2 ? "VUID-VkCopyImageInfo2-aspect-06663" : "VUID-vkCmdCopyImage-aspect-06663";
        skip |=
            ValidateImageUsageFlags(commandBuffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, false, vuid, dst_image_loc);
    } else {
        auto src_separate_stencil = vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(src_image_state->create_info.pNext);
        if (src_separate_stencil && has_stencil_aspect &&
            ((src_separate_stencil->stencilUsage & VK_IMAGE_USAGE_TRANSFER_SRC_BIT) == 0)) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-aspect-06664" : "VUID-vkCmdCopyImage-aspect-06664";
            skip =
                LogError(vuid, src_objlist, src_image_loc, "(%s) was created with %s but requires VK_IMAGE_USAGE_TRANSFER_SRC_BIT.",
                         FormatHandle(src_image_state->Handle()).c_str(),
                         string_VkImageUsageFlags(src_separate_stencil->stencilUsage).c_str());
        }
        if (!src_separate_stencil || has_non_stencil_aspect) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-aspect-06662" : "VUID-vkCmdCopyImage-aspect-06662";
            skip |= ValidateImageUsageFlags(commandBuffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, false, vuid,
                                            src_image_loc);
        }

        auto dst_separate_stencil = vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(dst_image_state->create_info.pNext);
        if (dst_separate_stencil && has_stencil_aspect &&
            ((dst_separate_stencil->stencilUsage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0)) {
            vuid = is_2 ? "VUID-VkCopyImageInfo2-aspect-06665" : "VUID-vkCmdCopyImage-aspect-06665";
            skip =
                LogError(vuid, dst_objlist, dst_image_loc, "(%s) was created with %s but requires VK_IMAGE_USAGE_TRANSFER_DST_BIT.",
                         FormatHandle(dst_image_state->Handle()).c_str(),
                         string_VkImageUsageFlags(dst_separate_stencil->stencilUsage).c_str());
        }
        if (!dst_separate_stencil || has_non_stencil_aspect) {
            vuid = is_2 ? "VUID-vkCmdCopyImage-aspect-06663" : "VUID-vkCmdCopyImage-aspect-06663";
            skip |= ValidateImageUsageFlags(commandBuffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, false, vuid,
                                            dst_image_loc);
        }
    }

    // Source and dest image sample counts must match
    if (src_image_state->create_info.samples != dst_image_state->create_info.samples) {
        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImage-00136" : "VUID-vkCmdCopyImage-srcImage-00136";
        skip |= LogError(vuid, all_objlist, src_image_loc, "was created with (%s) but the dstImage was created with (%s).",
                         string_VkSampleCountFlagBits(src_image_state->create_info.samples),
                         string_VkSampleCountFlagBits(dst_image_state->create_info.samples));
    }

    vuid = is_2 ? "VUID-vkCmdCopyImage2-commandBuffer-01825" : "VUID-vkCmdCopyImage-commandBuffer-01825";
    skip |= ValidateProtectedImage(cb_state, *src_image_state, src_image_loc, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyImage2-commandBuffer-01826" : "VUID-vkCmdCopyImage-commandBuffer-01826";
    skip |= ValidateProtectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);
    vuid = is_2 ? "VUID-vkCmdCopyImage2-commandBuffer-01827" : "VUID-vkCmdCopyImage-commandBuffer-01827";
    skip |= ValidateUnprotectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);

    skip |= ValidateCmd(cb_state, loc);

    if (!IsValueIn(srcImageLayout,
                   {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL})) {
        vuid = is_2 ? "VUID-VkCopyImageInfo2-srcImageLayout-01917" : "VUID-vkCmdCopyImage-srcImageLayout-01917";
        skip |= LogError(vuid, src_objlist, loc.dot(Field::srcImageLayout), "is %s.", string_VkImageLayout(srcImageLayout));
    }

    if (!IsValueIn(dstImageLayout,
                   {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL})) {
        vuid = is_2 ? "VUID-VkCopyImageInfo2-dstImageLayout-01395" : "VUID-vkCmdCopyImage-dstImageLayout-01395";
        skip |= LogError(vuid, dst_objlist, loc.dot(Field::dstImageLayout), "is %s.", string_VkImageLayout(dstImageLayout));
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                             const VkImageCopy *pRegions, const ErrorObject &error_obj) const {
    return ValidateCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions,
                                error_obj.location);
}

bool CoreChecks::PreCallValidateCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo,
                                                 const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyImage2(commandBuffer, pCopyImageInfo, error_obj);
}

bool CoreChecks::PreCallValidateCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo,
                                              const ErrorObject &error_obj) const {
    return ValidateCmdCopyImage(commandBuffer, pCopyImageInfo->srcImage, pCopyImageInfo->srcImageLayout, pCopyImageInfo->dstImage,
                                pCopyImageInfo->dstImageLayout, pCopyImageInfo->regionCount, pCopyImageInfo->pRegions,
                                error_obj.location.dot(Field::pCopyImageInfo));
}

void CoreChecks::PostCallRecordCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                            VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                            const VkImageCopy *pRegions, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                             pRegions, record_obj);
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    auto src_image_state = Get<vvl::Image>(srcImage);
    auto dst_image_state = Get<vvl::Image>(dstImage);
    ASSERT_AND_RETURN(src_image_state && dst_image_state);

    // Make sure that all image slices are updated to correct layout
    for (uint32_t i = 0; i < regionCount; ++i) {
        cb_state->SetImageInitialLayout(*src_image_state, pRegions[i].srcSubresource, srcImageLayout);
        cb_state->SetImageInitialLayout(*dst_image_state, pRegions[i].dstSubresource, dstImageLayout);
    }
}

void CoreChecks::PostCallRecordCmdCopyImage2KHR(VkCommandBuffer commandBuffer, const VkCopyImageInfo2KHR *pCopyImageInfo,
                                                const RecordObject &record_obj) {
    PostCallRecordCmdCopyImage2(commandBuffer, pCopyImageInfo, record_obj);
}

void CoreChecks::PostCallRecordCmdCopyImage2(VkCommandBuffer commandBuffer, const VkCopyImageInfo2 *pCopyImageInfo,
                                             const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdCopyImage2(commandBuffer, pCopyImageInfo, record_obj);
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    auto src_image_state = Get<vvl::Image>(pCopyImageInfo->srcImage);
    auto dst_image_state = Get<vvl::Image>(pCopyImageInfo->dstImage);
    ASSERT_AND_RETURN(src_image_state && dst_image_state);

    // Make sure that all image slices are updated to correct layout
    for (uint32_t i = 0; i < pCopyImageInfo->regionCount; ++i) {
        cb_state->SetImageInitialLayout(*src_image_state, pCopyImageInfo->pRegions[i].srcSubresource,
                                        pCopyImageInfo->srcImageLayout);
        cb_state->SetImageInitialLayout(*dst_image_state, pCopyImageInfo->pRegions[i].dstSubresource,
                                        pCopyImageInfo->dstImageLayout);
    }
}

template <typename RegionType>
void CoreChecks::RecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t regionCount,
                                     const RegionType *pRegions, const Location &loc) {
    const bool is_2 = loc.function == Func::vkCmdCopyBuffer2 || loc.function == Func::vkCmdCopyBuffer2KHR;
    const char *vuid = is_2 ? "VUID-VkCopyBufferInfo2-pRegions-00117" : "VUID-vkCmdCopyBuffer-pRegions-00117";

    auto src_buffer_state = Get<vvl::Buffer>(srcBuffer);
    auto dst_buffer_state = Get<vvl::Buffer>(dstBuffer);
    ASSERT_AND_RETURN(src_buffer_state && dst_buffer_state);

    if (regionCount > 0 && (src_buffer_state->sparse || dst_buffer_state->sparse)) {
        auto cb_state = Get<vvl::CommandBuffer>(commandBuffer);

        using BufferRange = vvl::BindableMemoryTracker::BufferRange;

        std::vector<BufferRange> src_ranges(regionCount);
        std::vector<BufferRange> dst_ranges(regionCount);
        BufferRange src_ranges_bounds(pRegions[0].srcOffset, pRegions[0].srcOffset + pRegions[0].size);
        BufferRange dst_ranges_bounds(pRegions[0].dstOffset, pRegions[0].dstOffset + pRegions[0].size);

        for (uint32_t i = 0; i < regionCount; ++i) {
            const RegionType &region = pRegions[i];
            src_ranges[i] = sparse_container::range<VkDeviceSize>{region.srcOffset, region.srcOffset + region.size};
            dst_ranges[i] = sparse_container::range<VkDeviceSize>{region.dstOffset, region.dstOffset + region.size};

            src_ranges_bounds.begin = std::min(src_ranges_bounds.begin, region.srcOffset);
            src_ranges_bounds.end = std::max(src_ranges_bounds.end, region.srcOffset + region.size);

            dst_ranges_bounds.begin = std::min(dst_ranges_bounds.begin, region.dstOffset);
            dst_ranges_bounds.end = std::max(dst_ranges_bounds.end, region.dstOffset + region.size);
        }

        auto queue_submit_validation = [this, commandBuffer, src_buffer_state, dst_buffer_state, src_ranges = std::move(src_ranges),
                                        dst_ranges = std::move(dst_ranges), src_ranges_bounds, dst_ranges_bounds, loc,
                                        vuid](const vvl::Device &device_data, const class vvl::Queue &queue_state,
                                              const vvl::CommandBuffer &cb_state) -> bool {
            bool skip = false;

            auto src_vk_memory_to_ranges_map = src_buffer_state->GetBoundRanges(src_ranges_bounds, src_ranges);
            auto dst_vk_memory_to_ranges_map = dst_buffer_state->GetBoundRanges(dst_ranges_bounds, dst_ranges);

            for (const auto &[vk_memory, src_ranges] : src_vk_memory_to_ranges_map) {
                if (const auto find_mem_it = dst_vk_memory_to_ranges_map.find(vk_memory);
                    find_mem_it != dst_vk_memory_to_ranges_map.end()) {
                    // Some source and destination ranges are bound to the same VkDeviceMemory, look for overlaps.
                    // Memory ranges are sorted, so looking for overlaps can be done in linear time

                    auto &dst_ranges_vec = find_mem_it->second;
                    auto src_ranges_it = src_ranges.cbegin();
                    auto dst_ranges_it = dst_ranges_vec.cbegin();

                    while (src_ranges_it != src_ranges.cend() && dst_ranges_it != dst_ranges_vec.cend()) {
                        if (src_ranges_it->first.intersects(dst_ranges_it->first)) {
                            auto memory_range_overlap = src_ranges_it->first & dst_ranges_it->first;

                            const LogObjectList objlist(commandBuffer, src_buffer_state->Handle(), dst_buffer_state->Handle(),
                                                        vk_memory);
                            skip |= this->LogError(
                                vuid, objlist, loc,
                                "Copy source buffer range %s (from buffer %s) and destination buffer range %s (from buffer %s) are "
                                "bound to the same memory (%s), "
                                "and end up overlapping on memory range %s.",
                                sparse_container::string_range(src_ranges_it->second).c_str(),
                                FormatHandle(src_buffer_state->VkHandle()).c_str(),
                                sparse_container::string_range(dst_ranges_it->second).c_str(),
                                FormatHandle(dst_buffer_state->VkHandle()).c_str(), FormatHandle(vk_memory).c_str(),
                                sparse_container::string_range(memory_range_overlap).c_str());
                        }

                        if (src_ranges_it->first < dst_ranges_it->first) {
                            ++src_ranges_it;
                        } else {
                            ++dst_ranges_it;
                        }
                    }
                }
            }
            return skip;
        };

        cb_state->queue_submit_functions.emplace_back(queue_submit_validation);
    }
}

void CoreChecks::PostCallRecordCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
                                             uint32_t regionCount, const VkBufferCopy *pRegions, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions, record_obj);
    RecordCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions, record_obj.location);
}

void CoreChecks::PostCallRecordCmdCopyBuffer2KHR(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2KHR *pCopyBufferInfo,
                                                 const RecordObject &record_obj) {
    return PostCallRecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, record_obj);
}

void CoreChecks::PostCallRecordCmdCopyBuffer2(VkCommandBuffer commandBuffer, const VkCopyBufferInfo2 *pCopyBufferInfo,
                                              const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdCopyBuffer2(commandBuffer, pCopyBufferInfo, record_obj);
    RecordCmdCopyBuffer(commandBuffer, pCopyBufferInfo->srcBuffer, pCopyBufferInfo->dstBuffer, pCopyBufferInfo->regionCount,
                        pCopyBufferInfo->pRegions, record_obj.location);
}

template <typename T>
VkImageSubresourceLayers GetImageSubresource(T data, bool is_src) {
    return data.imageSubresource;
}
template <>
VkImageSubresourceLayers GetImageSubresource<VkImageCopy2>(VkImageCopy2 data, bool is_src) {
    return is_src ? data.srcSubresource : data.dstSubresource;
}
template <typename T>
VkOffset3D GetOffset(T data, bool is_src) {
    return data.imageOffset;
}
template <>
VkOffset3D GetOffset<VkImageCopy2>(VkImageCopy2 data, bool is_src) {
    return is_src ? data.srcOffset : data.dstOffset;
}
template <typename T>
VkExtent3D GetExtent(T data) {
    return data.imageExtent;
}
template <>
VkExtent3D GetExtent<VkImageCopy2>(VkImageCopy2 data) {
    return data.extent;
}
template <typename HandleT, typename RegionType>
bool CoreChecks::ValidateImageBounds(const HandleT handle, const vvl::Image &image_state, const RegionType &region,
                                     const Location &region_loc, const char *vuid, bool is_src) const {
    bool skip = false;

    VkExtent3D extent = GetExtent(region);
    VkOffset3D offset = GetOffset(region, is_src);
    VkImageSubresourceLayers subresource_layout = GetImageSubresource(region, is_src);

    VkExtent3D image_extent = image_state.GetEffectiveSubresourceExtent(subresource_layout);

    // If we're using a blocked image format, valid extent is rounded up to multiple of block size (per
    // vkspec.html#_common_operation)
    if (vkuFormatIsBlockedImage(image_state.create_info.format)) {
        const VkExtent3D block_extent = vkuFormatTexelBlockExtent(image_state.create_info.format);
        if (image_extent.width % block_extent.width) {
            image_extent.width += (block_extent.width - (image_extent.width % block_extent.width));
        }
        if (image_extent.height % block_extent.height) {
            image_extent.height += (block_extent.height - (image_extent.height % block_extent.height));
        }
        if (image_extent.depth % block_extent.depth) {
            image_extent.depth += (block_extent.depth - (image_extent.depth % block_extent.depth));
        }
    }

    if (0 != ExceedsBounds(&offset, &extent, &image_extent)) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |=
            LogError(vuid, objlist, region_loc,
                     "exceeds image bounds\n"
                     "region extent (%s)\n"
                     "region offset (%s)\n"
                     "image extent (%s)\n",
                     string_VkExtent3D(extent).c_str(), string_VkOffset3D(offset).c_str(), string_VkExtent3D(image_extent).c_str());
    }

    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateBufferBounds(VkCommandBuffer cb, const vvl::Image &image_state, const vvl::Buffer &buffer_state,
                                      const RegionType &region, const Location &region_loc) const {
    bool skip = false;

    const VkDeviceSize buffer_copy_size =
        vvl::GetBufferSizeFromCopyImage(region, image_state.create_info.format, image_state.create_info.arrayLayers);
    // This blocks against invalid VkBufferCopyImage that already have been caught elsewhere
    if (buffer_copy_size != 0) {
        const VkDeviceSize max_buffer_copy = buffer_copy_size + region.bufferOffset;
        if (buffer_state.create_info.size < max_buffer_copy) {
            const LogObjectList objlist(cb, buffer_state.Handle());
            skip |=
                LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::ExceedBufferBounds_00171), objlist, region_loc,
                         "is trying to copy %" PRIu64 " bytes + %" PRIu64
                         " offset to/from the (%s) which exceeds the VkBuffer total size of %" PRIu64 " bytes.",
                         buffer_copy_size, region.bufferOffset, FormatHandle(buffer_state).c_str(), buffer_state.create_info.size);
        }
    }

    return skip;
}

template <typename HandleT>
// Validate that an image's sampleCount matches the requirement for a specific API call
bool CoreChecks::ValidateImageSampleCount(const HandleT handle, const vvl::Image &image_state, VkSampleCountFlagBits sample_count,
                                          const Location &loc, const std::string &vuid) const {
    bool skip = false;
    if (image_state.create_info.samples != sample_count) {
        const LogObjectList objlist(handle, image_state.Handle());
        skip |= LogError(vuid, objlist, loc, "%s was created with a sample count of %s but must be %s.",
                         FormatHandle(image_state).c_str(), string_VkSampleCountFlagBits(image_state.create_info.samples),
                         string_VkSampleCountFlagBits(sample_count));
    }
    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateImageBufferCopyMemoryOverlap(const vvl::CommandBuffer &cb_state, const RegionType &region,
                                                      const vvl::Image &image_state, const vvl::Buffer &buffer_state,
                                                      const Location &region_loc) const {
    bool skip = false;

    if (vkuFormatIsCompressed(image_state.create_info.format)) {
        // TODO https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/6898
        return skip;
    }

    if (buffer_state.sparse || image_state.sparse) {
        // TODO - Will require a lot more logic to detect sparse copies
        return skip;
    }

    auto texel_size = vkuFormatTexelSizeWithAspect(image_state.create_info.format,
                                                   static_cast<VkImageAspectFlagBits>(region.imageSubresource.aspectMask));
    VkDeviceSize image_offset;
    if (image_state.create_info.tiling == VK_IMAGE_TILING_LINEAR) {
        // Can only know actual offset for linearly tiled images
        VkImageSubresource isr = {};
        isr.arrayLayer = region.imageSubresource.baseArrayLayer;
        isr.aspectMask = region.imageSubresource.aspectMask;
        isr.mipLevel = region.imageSubresource.mipLevel;
        VkSubresourceLayout srl = {};
        DispatchGetImageSubresourceLayout(device, image_state.VkHandle(), &isr, &srl);
        if (image_state.create_info.arrayLayers == 1) srl.arrayPitch = 0;
        if (image_state.create_info.imageType != VK_IMAGE_TYPE_3D) srl.depthPitch = 0;
        image_offset = (region.imageSubresource.baseArrayLayer * srl.arrayPitch) +
                       static_cast<VkDeviceSize>((region.imageOffset.x * texel_size)) + (region.imageOffset.y * srl.rowPitch) +
                       (region.imageOffset.z * srl.depthPitch) + srl.offset;
    } else {
        image_offset = static_cast<VkDeviceSize>(region.imageOffset.x * region.imageOffset.y * region.imageOffset.z * texel_size);
    }
    VkDeviceSize copy_size;
    if (region.bufferRowLength != 0 && region.bufferImageHeight != 0) {
        copy_size = static_cast<VkDeviceSize>(((region.bufferRowLength * region.bufferImageHeight) * texel_size));
    } else {
        copy_size = static_cast<VkDeviceSize>(
            ((region.imageExtent.width * region.imageExtent.height * region.imageExtent.depth) * texel_size));
    }
    auto image_region = sparse_container::range<VkDeviceSize>{image_offset, image_offset + copy_size};
    auto buffer_region = sparse_container::range<VkDeviceSize>{region.bufferOffset, region.bufferOffset + copy_size};
    if (image_state.DoesResourceMemoryOverlap(image_region, &buffer_state, buffer_region)) {
        const LogObjectList objlist(cb_state.Handle(), image_state.Handle());
        skip |= LogError(GetCopyBufferImageDeviceVUID(region_loc, vvl::CopyError::MemoryOverlap_00173), objlist, region_loc,
                         "Detected overlap between source and dest regions in memory.");
    }

    return skip;
}

bool CoreChecks::ValidateQueueFamilySupport(const vvl::CommandBuffer &cb_state, const vvl::PhysicalDevice &physical_device_state,
                                            VkImageAspectFlags aspectMask, const vvl::Image &image_state,
                                            const Location &aspect_mask_loc, const char *vuid) const {
    bool skip = false;

    if (!HasRequiredQueueFlags(cb_state, physical_device_state, VK_QUEUE_GRAPHICS_BIT) &&
        ((aspectMask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0)) {
        const LogObjectList objlist(cb_state.Handle(), image_state.Handle());
        skip |= LogError(vuid, objlist, aspect_mask_loc, "is %s, but is %s", string_VkImageAspectFlags(aspectMask).c_str(),
                         DescribeRequiredQueueFlag(cb_state, physical_device_state, VK_QUEUE_GRAPHICS_BIT).c_str());
    }

    return skip;
}

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                              VkBuffer dstBuffer, uint32_t regionCount, const RegionType *pRegions,
                                              const Location &loc) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<vvl::CommandBuffer>(commandBuffer);
    auto src_image_state = Get<vvl::Image>(srcImage);
    auto dst_buffer_state = Get<vvl::Buffer>(dstBuffer);
    ASSERT_AND_RETURN_SKIP(src_image_state && dst_buffer_state);

    const vvl::CommandBuffer &cb_state = *cb_state_ptr;

    const bool is_2 = loc.function == Func::vkCmdCopyImageToBuffer2 || loc.function == Func::vkCmdCopyImageToBuffer2KHR;
    const char *vuid;
    const Location src_image_loc = loc.dot(Field::srcImage);
    const Location dst_buffer_loc = loc.dot(Field::dstBuffer);
    const LogObjectList src_objlist(commandBuffer, srcImage);
    const LogObjectList dst_objlist(commandBuffer, dstBuffer);

    skip |= ValidateCmd(cb_state, loc);

    // dst buffer
    {
        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-dstBuffer-00192" : "VUID-vkCmdCopyImageToBuffer-dstBuffer-00192";
        skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *dst_buffer_state, dst_buffer_loc, vuid);

        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-dstBuffer-00191" : "VUID-vkCmdCopyImageToBuffer-dstBuffer-00191";
        skip |=
            ValidateBufferUsageFlags(dst_objlist, *dst_buffer_state, VK_BUFFER_USAGE_TRANSFER_DST_BIT, true, vuid, dst_buffer_loc);

        vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer2-commandBuffer-01832" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-01832";
        skip |= ValidateProtectedBuffer(cb_state, *dst_buffer_state, dst_buffer_loc, vuid);

        vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer2-commandBuffer-01833" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-01833";
        skip |= ValidateUnprotectedBuffer(cb_state, *dst_buffer_state, dst_buffer_loc, vuid);
    }

    // src image
    {
        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-07973" : "VUID-vkCmdCopyImageToBuffer-srcImage-07973";
        skip |= ValidateImageSampleCount(commandBuffer, *src_image_state, VK_SAMPLE_COUNT_1_BIT, src_image_loc, vuid);

        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-07966" : "VUID-vkCmdCopyImageToBuffer-srcImage-07966";
        skip |= ValidateMemoryIsBoundToImage(src_objlist, *src_image_state, src_image_loc, vuid);

        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-00186" : "VUID-vkCmdCopyImageToBuffer-srcImage-00186";
        skip |=
            ValidateImageUsageFlags(commandBuffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true, vuid, src_image_loc);

        vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer2-commandBuffer-01831" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-01831";
        skip |= ValidateProtectedImage(cb_state, *src_image_state, src_image_loc, vuid);

        // Validation for VK_EXT_fragment_density_map
        if (src_image_state->create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            const LogObjectList objlist(commandBuffer, srcImage, dstBuffer);
            vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-07969" : "VUID-vkCmdCopyImageToBuffer-srcImage-07969";
            skip |= LogError(vuid, objlist, src_image_loc, "was created with VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT.");
        }

        if (IsExtEnabled(extensions.vk_khr_maintenance1)) {
            vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImage-01998" : "VUID-vkCmdCopyImageToBuffer-srcImage-01998";
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state, VK_FORMAT_FEATURE_2_TRANSFER_SRC_BIT,
                                                    src_image_loc, vuid);
        }

        if (!IsValueIn(srcImageLayout,
                       {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL})) {
            vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImageLayout-01397" : "VUID-vkCmdCopyImageToBuffer-srcImageLayout-01397";
            skip |= LogError(vuid, src_objlist, loc.dot(Field::srcImageLayout), "is %s.", string_VkImageLayout(srcImageLayout));
        }
    }

    for (uint32_t i = 0; i < regionCount; ++i) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location subresource_loc = region_loc.dot(Field::imageSubresource);
        const RegionType region = pRegions[i];

        skip |= ValidateBufferImageCopyData(cb_state, region, *src_image_state, region_loc);
        skip |= ValidateImageSubresourceLayers(commandBuffer, region.imageSubresource, subresource_loc);
        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-srcImageLayout-00189" : "VUID-vkCmdCopyImageToBuffer-srcImageLayout-00189";
        skip |=
            VerifyImageLayoutSubresource(cb_state, *src_image_state, region.imageSubresource, srcImageLayout, src_image_loc, vuid);
        skip |= ValidateCopyBufferImageTransferGranularityRequirements(cb_state, *src_image_state, region, region_loc);
        skip |= ValidateImageMipLevel(commandBuffer, *src_image_state, region.imageSubresource.mipLevel, subresource_loc);
        skip |= ValidateImageArrayLayerRange(commandBuffer, *src_image_state, region.imageSubresource.baseArrayLayer,
                                             region.imageSubresource.layerCount, subresource_loc);

        vuid = is_2 ? "VUID-VkCopyImageToBufferInfo2-pRegions-04566" : "VUID-vkCmdCopyImageToBuffer-imageSubresource-07970";
        skip |= ValidateImageBounds(commandBuffer, *src_image_state, region, region_loc, vuid, true);
        skip |= ValidateBufferBounds(commandBuffer, *src_image_state, *dst_buffer_state, region, region_loc);

        skip |= ValidateImageBufferCopyMemoryOverlap(cb_state, region, *src_image_state, *dst_buffer_state, region_loc);
        vuid = is_2 ? "VUID-vkCmdCopyImageToBuffer2-commandBuffer-10216" : "VUID-vkCmdCopyImageToBuffer-commandBuffer-10216";
        skip |= ValidateQueueFamilySupport(cb_state, *physical_device_state, region.imageSubresource.aspectMask, *src_image_state,
                                           subresource_loc.dot(Field::aspectMask), vuid);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                     VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions,
                                                     const ErrorObject &error_obj) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions,
                                        error_obj.location);
}

bool CoreChecks::PreCallValidateCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                         const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo,
                                                         const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, error_obj);
}

bool CoreChecks::PreCallValidateCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                      const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo,
                                                      const ErrorObject &error_obj) const {
    return ValidateCmdCopyImageToBuffer(commandBuffer, pCopyImageToBufferInfo->srcImage, pCopyImageToBufferInfo->srcImageLayout,
                                        pCopyImageToBufferInfo->dstBuffer, pCopyImageToBufferInfo->regionCount,
                                        pCopyImageToBufferInfo->pRegions, error_obj.location.dot(Field::pCopyImageToBufferInfo));
}

void CoreChecks::PostCallRecordCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                    VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions,
                                                    const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions,
                                                     record_obj);

    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    auto src_image_state = Get<vvl::Image>(srcImage);
    ASSERT_AND_RETURN(src_image_state);

    // Make sure that all image slices record referenced layout
    for (uint32_t i = 0; i < regionCount; ++i) {
        cb_state->SetImageInitialLayout(*src_image_state, pRegions[i].imageSubresource, srcImageLayout);
    }
}

void CoreChecks::PostCallRecordCmdCopyImageToBuffer2KHR(VkCommandBuffer commandBuffer,
                                                        const VkCopyImageToBufferInfo2KHR *pCopyImageToBufferInfo,
                                                        const RecordObject &record_obj) {
    PostCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, record_obj);
}

void CoreChecks::PostCallRecordCmdCopyImageToBuffer2(VkCommandBuffer commandBuffer,
                                                     const VkCopyImageToBufferInfo2 *pCopyImageToBufferInfo,
                                                     const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdCopyImageToBuffer2(commandBuffer, pCopyImageToBufferInfo, record_obj);

    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    auto src_image_state = Get<vvl::Image>(pCopyImageToBufferInfo->srcImage);
    ASSERT_AND_RETURN(src_image_state);

    // Make sure that all image slices record referenced layout
    for (uint32_t i = 0; i < pCopyImageToBufferInfo->regionCount; ++i) {
        cb_state->SetImageInitialLayout(*src_image_state, pCopyImageToBufferInfo->pRegions[i].imageSubresource,
                                        pCopyImageToBufferInfo->srcImageLayout);
    }
}

template <typename RegionType>
bool CoreChecks::ValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                              VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                              const Location &loc) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<vvl::CommandBuffer>(commandBuffer);
    auto src_buffer_state = Get<vvl::Buffer>(srcBuffer);
    auto dst_image_state = Get<vvl::Image>(dstImage);
    ASSERT_AND_RETURN_SKIP(src_buffer_state && dst_image_state);

    const vvl::CommandBuffer &cb_state = *cb_state_ptr;

    const bool is_2 = loc.function == Func::vkCmdCopyBufferToImage2 || loc.function == Func::vkCmdCopyBufferToImage2KHR;
    const char *vuid;
    const Location src_buffer_loc = loc.dot(Field::srcBuffer);
    const Location dst_image_loc = loc.dot(Field::dstImage);
    const LogObjectList src_objlist(commandBuffer, srcBuffer);
    const LogObjectList dst_objlist(commandBuffer, dstImage);

    skip |= ValidateCmd(cb_state, loc);

    // src buffer
    {
        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-srcBuffer-00176" : "VUID-vkCmdCopyBufferToImage-srcBuffer-00176";
        skip |= ValidateMemoryIsBoundToBuffer(commandBuffer, *src_buffer_state, src_buffer_loc, vuid);

        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-srcBuffer-00174" : "VUID-vkCmdCopyBufferToImage-srcBuffer-00174";
        skip |=
            ValidateBufferUsageFlags(src_objlist, *src_buffer_state, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, true, vuid, src_buffer_loc);

        vuid = is_2 ? "VUID-vkCmdCopyBufferToImage2-commandBuffer-01828" : "VUID-vkCmdCopyBufferToImage-commandBuffer-01828";
        skip |= ValidateProtectedBuffer(cb_state, *src_buffer_state, src_buffer_loc, vuid);
    }

    // dst image
    {
        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-07973" : "VUID-vkCmdCopyBufferToImage-dstImage-07973";
        skip |= ValidateImageSampleCount(commandBuffer, *dst_image_state, VK_SAMPLE_COUNT_1_BIT, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-07966" : "VUID-vkCmdCopyBufferToImage-dstImage-07966";
        skip |= ValidateMemoryIsBoundToImage(dst_objlist, *dst_image_state, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-00177" : "VUID-vkCmdCopyBufferToImage-dstImage-00177";
        skip |=
            ValidateImageUsageFlags(commandBuffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, true, vuid, dst_image_loc);

        vuid = is_2 ? "VUID-vkCmdCopyBufferToImage2-commandBuffer-01829" : "VUID-vkCmdCopyBufferToImage-commandBuffer-01829";
        skip |= ValidateProtectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-vkCmdCopyBufferToImage-commandBuffer-01830" : "VUID-vkCmdCopyBufferToImage-commandBuffer-01830";
        skip |= ValidateUnprotectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);

        // Validation for VK_EXT_fragment_density_map
        if (dst_image_state->create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-07969" : "VUID-vkCmdCopyBufferToImage-dstImage-07969";
            skip |= LogError(vuid, dst_objlist, dst_image_loc, "was created with VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT.");
        }

        if (IsExtEnabled(extensions.vk_khr_maintenance1)) {
            vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImage-01997" : "VUID-vkCmdCopyBufferToImage-dstImage-01997";
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, *dst_image_state, VK_FORMAT_FEATURE_2_TRANSFER_DST_BIT,
                                                    dst_image_loc, vuid);
        }

        if (!IsValueIn(dstImageLayout,
                       {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL})) {
            vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImageLayout-01396" : "VUID-vkCmdCopyBufferToImage-dstImageLayout-01396";
            skip |= LogError(vuid, dst_objlist, loc.dot(Field::dstImageLayout), "is %s.", string_VkImageLayout(dstImageLayout));
        }
    }

    for (uint32_t i = 0; i < regionCount; ++i) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location subresource_loc = region_loc.dot(Field::imageSubresource);
        const RegionType region = pRegions[i];

        skip |= ValidateBufferImageCopyData(cb_state, region, *dst_image_state, region_loc);
        skip |= ValidateImageSubresourceLayers(commandBuffer, region.imageSubresource, subresource_loc);
        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-dstImageLayout-00180" : "VUID-vkCmdCopyBufferToImage-dstImageLayout-00180";
        skip |=
            VerifyImageLayoutSubresource(cb_state, *dst_image_state, region.imageSubresource, dstImageLayout, dst_image_loc, vuid);
        skip |= ValidateCopyBufferImageTransferGranularityRequirements(cb_state, *dst_image_state, region, region_loc);
        skip |= ValidateImageMipLevel(commandBuffer, *dst_image_state, region.imageSubresource.mipLevel, subresource_loc);
        skip |= ValidateImageArrayLayerRange(commandBuffer, *dst_image_state, region.imageSubresource.baseArrayLayer,
                                             region.imageSubresource.layerCount, subresource_loc);

        vuid = is_2 ? "VUID-VkCopyBufferToImageInfo2-pRegions-04565" : "VUID-vkCmdCopyBufferToImage-imageSubresource-07970";
        skip |= ValidateImageBounds(commandBuffer, *dst_image_state, region, region_loc, vuid, false);
        skip |= ValidateBufferBounds(commandBuffer, *dst_image_state, *src_buffer_state, region, region_loc);

        vuid = is_2 ? "VUID-vkCmdCopyBufferToImage2-commandBuffer-07739" : "VUID-vkCmdCopyBufferToImage-commandBuffer-07739";
        skip |= ValidateQueueFamilySupport(cb_state, *physical_device_state, region.imageSubresource.aspectMask, *dst_image_state,
                                           subresource_loc.dot(Field::aspectMask), vuid);

        skip |= ValidateImageBufferCopyMemoryOverlap(cb_state, region, *dst_image_state, *src_buffer_state, region_loc);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                     VkImageLayout dstImageLayout, uint32_t regionCount,
                                                     const VkBufferImageCopy *pRegions, const ErrorObject &error_obj) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions,
                                        error_obj.location);
}

bool CoreChecks::PreCallValidateCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                         const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo,
                                                         const ErrorObject &error_obj) const {
    return PreCallValidateCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo, error_obj);
}

bool CoreChecks::PreCallValidateCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                      const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo,
                                                      const ErrorObject &error_obj) const {
    return ValidateCmdCopyBufferToImage(commandBuffer, pCopyBufferToImageInfo->srcBuffer, pCopyBufferToImageInfo->dstImage,
                                        pCopyBufferToImageInfo->dstImageLayout, pCopyBufferToImageInfo->regionCount,
                                        pCopyBufferToImageInfo->pRegions, error_obj.location.dot(Field::pCopyBufferToImageInfo));
}

void CoreChecks::PostCallRecordCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
                                                    VkImageLayout dstImageLayout, uint32_t regionCount,
                                                    const VkBufferImageCopy *pRegions, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions,
                                                     record_obj);

    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    auto dst_image_state = Get<vvl::Image>(dstImage);
    ASSERT_AND_RETURN(dst_image_state);

    // Make sure that all image slices are record referenced layout
    for (uint32_t i = 0; i < regionCount; ++i) {
        cb_state->SetImageInitialLayout(*dst_image_state, pRegions[i].imageSubresource, dstImageLayout);
    }
}

void CoreChecks::PostCallRecordCmdCopyBufferToImage2KHR(VkCommandBuffer commandBuffer,
                                                        const VkCopyBufferToImageInfo2KHR *pCopyBufferToImageInfo2KHR,
                                                        const RecordObject &record_obj) {
    PostCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo2KHR, record_obj);
}

void CoreChecks::PostCallRecordCmdCopyBufferToImage2(VkCommandBuffer commandBuffer,
                                                     const VkCopyBufferToImageInfo2 *pCopyBufferToImageInfo,
                                                     const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdCopyBufferToImage2(commandBuffer, pCopyBufferToImageInfo, record_obj);

    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    auto dst_image_state = Get<vvl::Image>(pCopyBufferToImageInfo->dstImage);
    ASSERT_AND_RETURN(dst_image_state);

    // Make sure that all image slices are record referenced layout
    for (uint32_t i = 0; i < pCopyBufferToImageInfo->regionCount; ++i) {
        cb_state->SetImageInitialLayout(*dst_image_state, pCopyBufferToImageInfo->pRegions[i].imageSubresource,
                                        pCopyBufferToImageInfo->dstImageLayout);
    }
}

bool CoreChecks::UsageHostTransferCheck(const vvl::Image &image_state, const VkImageAspectFlags aspect_mask, const char *vuid_09111,
                                        const char *vuid_09112, const char *vuid_09113, const Location &subresource_loc) const {
    bool skip = false;
    const bool has_stencil = (aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT);
    const bool has_non_stencil = (aspect_mask & ~VK_IMAGE_ASPECT_STENCIL_BIT);

    if (has_stencil) {
        if (const auto image_stencil_struct =
                vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(image_state.create_info.pNext)) {
            if ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_HOST_TRANSFER_BIT) == 0) {
                skip |= LogError(vuid_09112, image_state.Handle(), subresource_loc.dot(Field::aspectMask),
                                 "(%s) includes VK_IMAGE_ASPECT_STENCIL_BIT and the image was created with "
                                 "VkImageStencilUsageCreateInfo, but VK_IMAGE_USAGE_HOST_TRANSFER_BIT was not included in "
                                 "VkImageStencilUsageCreateInfo::stencilUsage (%s) used to create image",
                                 string_VkImageAspectFlags(aspect_mask).c_str(),
                                 string_VkImageUsageFlags(image_stencil_struct->stencilUsage).c_str());
            }
        } else {
            if ((image_state.create_info.usage & VK_IMAGE_USAGE_HOST_TRANSFER_BIT) == 0) {
                skip |= LogError(vuid_09111, image_state.Handle(), subresource_loc.dot(Field::aspectMask),
                                 "(%s) includes VK_IMAGE_ASPECT_STENCIL_BIT and the image was not created with "
                                 "VkImageStencilUsageCreateInfo, but VK_IMAGE_USAGE_HOST_TRANSFER_BIT was not included in "
                                 "VkImageCreateInfo::usage (%s) used to create image",
                                 string_VkImageAspectFlags(aspect_mask).c_str(),
                                 string_VkImageUsageFlags(image_state.create_info.usage).c_str());
            }
        }
    }
    if (has_non_stencil) {
        if ((image_state.create_info.usage & VK_IMAGE_USAGE_HOST_TRANSFER_BIT) == 0) {
            skip |= LogError(vuid_09113, image_state.Handle(), subresource_loc.dot(Field::aspectMask),
                             "(%s) includes aspects other than VK_IMAGE_ASPECT_STENCIL_BIT, but "
                             "VK_IMAGE_USAGE_HOST_TRANSFER_BIT was not included "
                             "in VkImageCreateInfo::usage (%s) used to create image",
                             string_VkImageAspectFlags(aspect_mask).c_str(),
                             string_VkImageUsageFlags(image_state.create_info.usage).c_str());
        }
    }
    return skip;
}

VkImageLayout GetImageLayout(VkCopyMemoryToImageInfo data) { return data.dstImageLayout; }
VkImageLayout GetImageLayout(VkCopyImageToMemoryInfo data) { return data.srcImageLayout; }

VkImage GetImage(VkCopyMemoryToImageInfo data) { return data.dstImage; }
VkImage GetImage(VkCopyImageToMemoryInfo data) { return data.srcImage; }

template <typename InfoPointer>
bool CoreChecks::ValidateMemoryImageCopyCommon(InfoPointer info_ptr, const Location &loc) const {
    bool skip = false;
    VkImage image = GetImage(*info_ptr);
    auto image_state = Get<vvl::Image>(image);
    ASSERT_AND_RETURN_SKIP(image_state);
    const VkImageLayout image_layout = GetImageLayout(*info_ptr);
    const uint32_t regionCount = info_ptr->regionCount;
    const bool from_image = loc.function == Func::vkCopyImageToMemory || loc.function == Func::vkCopyImageToMemoryEXT;
    const Location image_loc = loc.dot(from_image ? Field::srcImage : Field::dstImage);
    const Field info_type = from_image ? Field::pCopyImageToMemoryInfo : Field::pCopyMemoryToImageInfo;
    const Field image_layout_field = from_image ? Field::srcImageLayout : Field::dstImageLayout;

    skip |= ValidateMemoryIsBoundToImage(
        image, *image_state, image_loc,
        from_image ? "VUID-VkCopyImageToMemoryInfo-srcImage-07966" : "VUID-VkCopyMemoryToImageInfo-dstImage-07966");

    if (image_state->sparse && (!image_state->HasFullRangeBound())) {
        const char *vuid =
            from_image ? "VUID-VkCopyImageToMemoryInfo-srcImage-09109" : "VUID-VkCopyMemoryToImageInfo-dstImage-09109";
        skip |= LogError(vuid, image_state->Handle(), image_loc, "is a sparse image with no memory bound");
    }

    if (image_state->create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        const char *vuid =
            from_image ? "VUID-VkCopyImageToMemoryInfo-srcImage-07969" : "VUID-VkCopyMemoryToImageInfo-dstImage-07969";
        skip |= LogError(vuid, image, image_loc,
                         "must not have been created with flags containing "
                         "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT");
    }

    skip |= ValidateImageSampleCount(
        device, *image_state, VK_SAMPLE_COUNT_1_BIT, image_loc,
        from_image ? "VUID-VkCopyImageToMemoryInfo-srcImage-07973" : "VUID-VkCopyMemoryToImageInfo-dstImage-07973");

    bool check_memcpy = (info_ptr->flags & VK_HOST_IMAGE_COPY_MEMCPY);
    const char *vuid_09111 =
        from_image ? "VUID-VkCopyImageToMemoryInfo-srcImage-09111" : "VUID-VkCopyMemoryToImageInfo-dstImage-09111";
    const char *vuid_09112 =
        from_image ? "VUID-VkCopyImageToMemoryInfo-srcImage-09112" : "VUID-VkCopyMemoryToImageInfo-dstImage-09112";
    const char *vuid_09113 =
        from_image ? "VUID-VkCopyImageToMemoryInfo-srcImage-09113" : "VUID-VkCopyMemoryToImageInfo-dstImage-09113";
    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location subresource_loc = region_loc.dot(Field::imageSubresource);
        const auto region = info_ptr->pRegions[i];

        skip |= ValidateHeterogeneousCopyData(device, region, *image_state, region_loc);
        skip |= ValidateImageMipLevel(device, *image_state, region.imageSubresource.mipLevel, subresource_loc);
        skip |= ValidateImageArrayLayerRange(device, *image_state, region.imageSubresource.baseArrayLayer,
                                             region.imageSubresource.layerCount, subresource_loc);
        skip |= ValidateImageSubresourceLayers(device, region.imageSubresource, subresource_loc);
        skip |= UsageHostTransferCheck(*image_state, region.imageSubresource.aspectMask, vuid_09111, vuid_09112, vuid_09113,
                                       subresource_loc);

        if (check_memcpy) {
            if (region.imageOffset.x != 0 || region.imageOffset.y != 0 || region.imageOffset.z != 0) {
                const char *vuid = from_image ? "VUID-VkCopyImageToMemoryInfo-imageOffset-09114"
                                              : "VUID-VkCopyMemoryToImageInfo-imageOffset-09114";
                skip |= LogError(vuid, device, loc.dot(info_type).dot(Field::flags),
                                 "contains VK_HOST_IMAGE_COPY_MEMCPY which "
                                 "means that pRegions[%" PRIu32 "].imageOffset x(%" PRIu32 "), y(%" PRIu32 ") and z(%" PRIu32
                                 ") must all be zero",
                                 i, region.imageOffset.x, region.imageOffset.y, region.imageOffset.z);
            }
            const VkExtent3D subresource_extent = image_state->GetEffectiveSubresourceExtent(region.imageSubresource);
            if (!IsExtentEqual(region.imageExtent, subresource_extent)) {
                const char *vuid =
                    from_image ? "VUID-VkCopyImageToMemoryInfo-srcImage-09115" : "VUID-VkCopyMemoryToImageInfo-dstImage-09115";
                skip |=
                    LogError(vuid, image_state->Handle(), region_loc.dot(Field::imageExtent),
                             "(%s) must match the image's subresource extents (%s) %s->flags contains VK_HOST_IMAGE_COPY_MEMCPY",
                             string_VkExtent3D(region.imageExtent).c_str(), string_VkExtent3D(subresource_extent).c_str(),
                             String(info_type));
            }
            if ((region.memoryRowLength != 0) || (region.memoryImageHeight != 0)) {
                const char *vuid =
                    from_image ? "VUID-VkCopyImageToMemoryInfo-flags-09394" : "VUID-VkCopyMemoryToImageInfo-flags-09393";
                skip |= LogError(vuid, image_state->Handle(), region_loc.dot(Field::memoryRowLength),
                                 "(%" PRIu32 "), and memoryImageHeight (%" PRIu32
                                 ") must both be zero if %s->flags contains VK_HOST_IMAGE_COPY_MEMCPY",
                                 region.memoryRowLength, region.memoryImageHeight, String(info_type));
            }
        }

        skip |= ValidateImageBounds(device, *image_state, region, region_loc,
                                    from_image ? "VUID-VkCopyImageToMemoryInfo-imageSubresource-07970"
                                               : "VUID-VkCopyMemoryToImageInfo-imageSubresource-07970",
                                    from_image);

        skip |=
            ValidateHostCopyCurrentLayout(image_layout, region.imageSubresource, *image_state, region_loc.dot(image_layout_field));
    }

    if (vkuFormatIsCompressed(image_state->create_info.format)) {
        // TODO - https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/8946
        // Discussion at https://gitlab.khronos.org/vulkan/vulkan/-/issues/4109
        return skip;
    }

    const auto &memory_states = image_state->GetBoundMemoryStates();
    for (const auto &state : memory_states) {
        // Image and host memory can't overlap unless the image memory is mapped
        if (state->mapped_range.size == 0) {
            continue;
        }
        const uint64_t mapped_size = (state->mapped_range.size == VK_WHOLE_SIZE)
                                         ? state->allocate_info.allocationSize
                                         : (state->mapped_range.offset + state->mapped_range.size);
        const void *mapped_start = state->p_driver_data;
        const void *mapped_end = static_cast<const char *>(mapped_start) + mapped_size;
        for (uint32_t i = 0; i < regionCount; i++) {
            const auto region = info_ptr->pRegions[i];
            const uint32_t texel_block_size = vkuFormatTexelBlockSize(image_state->create_info.format);
            uint64_t copy_size;
            if (region.memoryRowLength != 0 && region.memoryImageHeight != 0) {
                copy_size = ((region.memoryRowLength * region.memoryImageHeight) * texel_block_size);
            } else {
                copy_size = ((region.imageExtent.width * region.imageExtent.height * region.imageExtent.depth) * texel_block_size);
            }
            const void *copy_start = region.pHostPointer;
            const void *copy_end = static_cast<const char *>(copy_start) + copy_size;

            if ((copy_start >= mapped_start && copy_start < mapped_end) || (copy_end >= mapped_start && copy_end < mapped_end) ||
                (copy_start <= mapped_start && copy_end > mapped_end)) {
                const char *vuid =
                    from_image ? "VUID-VkImageToMemoryCopy-pRegions-09067" : "VUID-VkMemoryToImageCopy-pRegions-09062";
                skip |= LogError(vuid, image_state->Handle(), loc.dot(Field::pRegions, i).dot(Field::pHostPointer),
                                 "points to %" PRIu64 " bytes in host memory spanning\n[%p : %p]\nwhich overlaps with %" PRIu64
                                 " bytes of the image mapped memory at\n[%p : %p]\n",
                                 copy_size, copy_start, copy_end, mapped_size, mapped_start, mapped_end);
            }
        }
    }

    return skip;
}

bool CoreChecks::ValidateHostCopyImageCreateInfos(const vvl::Image &src_image_state, const vvl::Image &dst_image_state,
                                                  const Location &loc) const {
    bool skip = false;
    std::stringstream mismatch_stream{};
    const VkImageCreateInfo &src_info = src_image_state.create_info;
    const VkImageCreateInfo &dst_info = dst_image_state.create_info;

    if (src_info.flags != dst_info.flags) {
        mismatch_stream << "srcImage flags = " << string_VkImageCreateFlags(src_info.flags)
                        << " and dstImage flags = " << string_VkImageCreateFlags(dst_info.flags) << '\n';
    }
    if (src_info.imageType != dst_info.imageType) {
        mismatch_stream << "srcImage imageType = " << string_VkImageType(src_info.imageType)
                        << " and dstImage imageType = " << string_VkImageType(dst_info.imageType) << '\n';
    }
    if (src_info.format != dst_info.format) {
        mismatch_stream << "srcImage format = " << string_VkFormat(src_info.format)
                        << " and dstImage format = " << string_VkFormat(dst_info.format) << '\n';
    }
    if ((src_info.extent.width != dst_info.extent.width) || (src_info.extent.height != dst_info.extent.height) ||
        (src_info.extent.depth != dst_info.extent.depth)) {
        mismatch_stream << "srcImage extent = (" << string_VkExtent3D(src_info.extent) << ") but dstImage exten = ("
                        << string_VkExtent3D(dst_info.extent) << ")\n";
    }
    if (src_info.mipLevels != dst_info.mipLevels) {
        mismatch_stream << "srcImage mipLevels = " << src_info.mipLevels << "and dstImage mipLevels = " << dst_info.mipLevels
                        << '\n';
    }
    if (src_info.arrayLayers != dst_info.arrayLayers) {
        mismatch_stream << "srcImage arrayLayers = " << src_info.arrayLayers
                        << " and dstImage arrayLayers = " << dst_info.arrayLayers << '\n';
    }
    if (src_info.samples != dst_info.samples) {
        mismatch_stream << "srcImage samples = " << string_VkSampleCountFlagBits(src_info.samples)
                        << " and dstImage samples = " << string_VkSampleCountFlagBits(dst_info.samples) << '\n';
    }
    if (src_info.tiling != dst_info.tiling) {
        mismatch_stream << "srcImage tiling = " << string_VkImageTiling(src_info.tiling)
                        << " and dstImage tiling = " << string_VkImageTiling(dst_info.tiling) << '\n';
    }
    if (src_info.usage != dst_info.usage) {
        mismatch_stream << "srcImage usage = " << string_VkImageUsageFlags(src_info.usage)
                        << " and dstImage usage = " << string_VkImageUsageFlags(dst_info.usage) << '\n';
    }
    if (src_info.sharingMode != dst_info.sharingMode) {
        mismatch_stream << "srcImage sharingMode = " << string_VkSharingMode(src_info.sharingMode)
                        << " and dstImage sharingMode = " << string_VkSharingMode(dst_info.sharingMode) << '\n';
    }
    if (src_info.initialLayout != dst_info.initialLayout) {
        mismatch_stream << "srcImage initialLayout = " << string_VkImageLayout(src_info.initialLayout)
                        << " and dstImage initialLayout = " << string_VkImageLayout(dst_info.initialLayout) << '\n';
    }

    if (mismatch_stream.str().length() > 0) {
        std::stringstream ss;
        ss << "The creation parameters for srcImage and dstImage differ:\n" << mismatch_stream.str();
        LogObjectList objlist(src_image_state.Handle(), dst_image_state.Handle());
        skip |= LogError("VUID-VkCopyImageToImageInfo-srcImage-09069", objlist, loc, "%s.", ss.str().c_str());
    }
    return skip;
}

bool CoreChecks::ValidateHostCopyImageLayout(const VkImage image, const uint32_t layout_count,
                                             const VkImageLayout *supported_image_layouts, const VkImageLayout image_layout,
                                             const Location &loc, vvl::Field supported_name, const char *vuid) const {
    bool skip = false;
    for (uint32_t i = 0; i < layout_count; ++i) {
        if (supported_image_layouts[i] == image_layout) {
            return false;
        }
    }

    skip |= LogError(vuid, image, loc,
                     "is %s which is not one of the layouts returned in "
                     "VkPhysicalDeviceHostImageCopyPropertiesEXT::%s",
                     string_VkImageLayout(image_layout), String(supported_name));

    return skip;
}

bool CoreChecks::PreCallValidateCopyMemoryToImage(VkDevice device, const VkCopyMemoryToImageInfo *pCopyMemoryToImageInfo,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const Location copy_loc = error_obj.location.dot(Field::pCopyMemoryToImageInfo);
    auto dst_image = pCopyMemoryToImageInfo->dstImage;

    skip |= ValidateMemoryImageCopyCommon(pCopyMemoryToImageInfo, copy_loc);
    const auto &props = phys_dev_props_core14;
    skip |= ValidateHostCopyImageLayout(dst_image, props.copyDstLayoutCount, props.pCopyDstLayouts,
                                        pCopyMemoryToImageInfo->dstImageLayout, copy_loc.dot(Field::dstImageLayout),
                                        Field::pCopyDstLayouts, "VUID-VkCopyMemoryToImageInfo-dstImageLayout-09060");
    return skip;
}

bool CoreChecks::PreCallValidateCopyMemoryToImageEXT(VkDevice device, const VkCopyMemoryToImageInfoEXT *pCopyMemoryToImageInfo,
                                                     const ErrorObject &error_obj) const {
    return PreCallValidateCopyMemoryToImage(device, pCopyMemoryToImageInfo, error_obj);
}

bool CoreChecks::PreCallValidateCopyImageToMemory(VkDevice device, const VkCopyImageToMemoryInfo *pCopyImageToMemoryInfo,
                                                  const ErrorObject &error_obj) const {
    bool skip = false;
    const Location copy_loc = error_obj.location.dot(Field::pCopyImageToMemoryInfo);
    auto src_image = pCopyImageToMemoryInfo->srcImage;

    skip |= ValidateMemoryImageCopyCommon(pCopyImageToMemoryInfo, copy_loc);
    const auto &props = phys_dev_props_core14;
    skip |= ValidateHostCopyImageLayout(src_image, props.copySrcLayoutCount, props.pCopySrcLayouts,
                                        pCopyImageToMemoryInfo->srcImageLayout, copy_loc.dot(Field::srcImageLayout),
                                        Field::pCopySrcLayouts, "VUID-VkCopyImageToMemoryInfo-srcImageLayout-09065");
    return skip;
}

bool CoreChecks::PreCallValidateCopyImageToMemoryEXT(VkDevice device, const VkCopyImageToMemoryInfoEXT *pCopyImageToMemoryInfo,
                                                     const ErrorObject &error_obj) const {
    return PreCallValidateCopyImageToMemory(device, pCopyImageToMemoryInfo, error_obj);
}

bool CoreChecks::ValidateMemcpyExtents(const VkImageCopy2 &region, const vvl::Image &src_image_state,
                                       const vvl::Image &dst_image_state, const Location &region_loc) const {
    bool skip = false;
    if (region.srcOffset.x != 0 || region.srcOffset.y != 0 || region.srcOffset.z != 0) {
        skip |= LogError("VUID-VkCopyImageToImageInfo-srcOffset-09114", device, region_loc.dot(Field::srcOffset),
                         "is (%s) but flags contains VK_HOST_IMAGE_COPY_MEMCPY.", string_VkOffset3D(region.srcOffset).c_str());
    }
    if (!IsExtentEqual(region.extent, src_image_state.create_info.extent)) {
        skip |= LogError("VUID-VkCopyImageToImageInfo-srcImage-09115", src_image_state.Handle(), region_loc.dot(Field::imageExtent),
                         "(%s) must match the image's subresource "
                         "extents (%s) when VkCopyImageToImageInfo->flags contains VK_HOST_IMAGE_COPY_MEMCPY",
                         string_VkExtent3D(region.extent).c_str(), string_VkExtent3D(src_image_state.create_info.extent).c_str());
    }

    if (region.dstOffset.x != 0 || region.dstOffset.y != 0 || region.dstOffset.z != 0) {
        skip |= LogError("VUID-VkCopyImageToImageInfo-dstOffset-09114", device, region_loc.dot(Field::dstOffset),
                         "is (%s) but flags contains VK_HOST_IMAGE_COPY_MEMCPY.", string_VkOffset3D(region.dstOffset).c_str());
    }
    if (!IsExtentEqual(region.extent, dst_image_state.create_info.extent)) {
        skip |= LogError("VUID-VkCopyImageToImageInfo-dstImage-09115", dst_image_state.Handle(), region_loc.dot(Field::imageExtent),
                         "(%s) must match the image's subresource "
                         "extents (%s) when VkCopyImageToImageInfo->flags contains VK_HOST_IMAGE_COPY_MEMCPY",
                         string_VkExtent3D(region.extent).c_str(), string_VkExtent3D(dst_image_state.create_info.extent).c_str());
    }
    return skip;
}

bool CoreChecks::ValidateHostCopyMultiplane(const VkImageCopy2 &region, const vvl::Image &src_image_state,
                                            const vvl::Image &dst_image_state, const Location &region_loc) const {
    bool skip = false;
    const VkImageAspectFlags src_aspect_mask = region.srcSubresource.aspectMask;
    if (vkuFormatPlaneCount(src_image_state.create_info.format) == 2 &&
        (src_aspect_mask != VK_IMAGE_ASPECT_PLANE_0_BIT && src_aspect_mask != VK_IMAGE_ASPECT_PLANE_1_BIT)) {
        skip |= LogError("VUID-VkCopyImageToImageInfo-srcImage-07981", src_image_state.Handle(),
                         region_loc.dot(Field::srcSubresource), "is %s but srcImage has 2-plane format (%s).",
                         string_VkImageAspectFlags(src_aspect_mask).c_str(), string_VkFormat(src_image_state.create_info.format));
    }
    if (vkuFormatPlaneCount(src_image_state.create_info.format) == 3 &&
        (src_aspect_mask != VK_IMAGE_ASPECT_PLANE_0_BIT && src_aspect_mask != VK_IMAGE_ASPECT_PLANE_1_BIT &&
         src_aspect_mask != VK_IMAGE_ASPECT_PLANE_2_BIT)) {
        skip |= LogError("VUID-VkCopyImageToImageInfo-srcImage-07981", src_image_state.Handle(),
                         region_loc.dot(Field::srcSubresource), "is %s but srcImage has 3-plane format (%s).",
                         string_VkImageAspectFlags(src_aspect_mask).c_str(), string_VkFormat(src_image_state.create_info.format));
    }

    const VkImageAspectFlags dst_aspect_mask = region.dstSubresource.aspectMask;
    if (vkuFormatPlaneCount(dst_image_state.create_info.format) == 2 &&
        (dst_aspect_mask != VK_IMAGE_ASPECT_PLANE_0_BIT && dst_aspect_mask != VK_IMAGE_ASPECT_PLANE_1_BIT)) {
        skip |= LogError("VUID-VkCopyImageToImageInfo-dstImage-07981", dst_image_state.Handle(),
                         region_loc.dot(Field::dstSubresource), "is %s but dstImage has 2-plane format (%s).",
                         string_VkImageAspectFlags(dst_aspect_mask).c_str(), string_VkFormat(dst_image_state.create_info.format));
    }
    if (vkuFormatPlaneCount(dst_image_state.create_info.format) == 3 &&
        (dst_aspect_mask != VK_IMAGE_ASPECT_PLANE_0_BIT && dst_aspect_mask != VK_IMAGE_ASPECT_PLANE_1_BIT &&
         dst_aspect_mask != VK_IMAGE_ASPECT_PLANE_2_BIT)) {
        skip |= LogError("VUID-VkCopyImageToImageInfo-dstImage-07981", dst_image_state.Handle(),
                         region_loc.dot(Field::dstSubresource), "is %s but dstImage has 3-plane format (%s).",
                         string_VkImageAspectFlags(dst_aspect_mask).c_str(), string_VkFormat(dst_image_state.create_info.format));
    }
    return skip;
}

bool CoreChecks::PreCallValidateCopyImageToImage(VkDevice device, const VkCopyImageToImageInfo *pCopyImageToImageInfo,
                                                 const ErrorObject &error_obj) const {
    bool skip = false;
    auto info_ptr = pCopyImageToImageInfo;
    const Location loc = error_obj.location.dot(Field::pCopyImageToImageInfo);
    auto src_image_state = Get<vvl::Image>(info_ptr->srcImage);
    auto dst_image_state = Get<vvl::Image>(info_ptr->dstImage);
    ASSERT_AND_RETURN_SKIP(src_image_state && dst_image_state);

    // Formats are required to match, but check each image anyway
    const uint32_t src_plane_count = vkuFormatPlaneCount(src_image_state->create_info.format);
    const uint32_t dst_plane_count = vkuFormatPlaneCount(dst_image_state->create_info.format);
    bool check_multiplane = ((src_plane_count == 2 || src_plane_count == 3) || (dst_plane_count == 2 || dst_plane_count == 3));
    bool check_memcpy = (info_ptr->flags & VK_HOST_IMAGE_COPY_MEMCPY);

    skip |= ValidateHostCopyImageCreateInfos(*src_image_state, *dst_image_state, error_obj.location);
    skip |= ValidateCopyImageCommon(device, *src_image_state, *dst_image_state, error_obj.location);
    const auto &props = phys_dev_props_core14;
    skip |= ValidateHostCopyImageLayout(info_ptr->srcImage, props.copySrcLayoutCount, props.pCopySrcLayouts,
                                        info_ptr->srcImageLayout, loc.dot(Field::srcImageLayout), Field::pCopySrcLayouts,
                                        "VUID-VkCopyImageToImageInfo-srcImageLayout-09072");
    skip |= ValidateHostCopyImageLayout(info_ptr->dstImage, props.copyDstLayoutCount, props.pCopyDstLayouts,
                                        info_ptr->dstImageLayout, loc.dot(Field::dstImageLayout), Field::pCopyDstLayouts,
                                        "VUID-VkCopyImageToImageInfo-dstImageLayout-09073");

    if (src_image_state->sparse && (!src_image_state->HasFullRangeBound())) {
        skip |= LogError("VUID-VkCopyImageToImageInfo-srcImage-09109", src_image_state->Handle(), loc.dot(Field::srcImage),
                         "is a sparse image with no memory bound");
    }
    if (dst_image_state->sparse && (!dst_image_state->HasFullRangeBound())) {
        skip |= LogError("VUID-VkCopyImageToImageInfo-dstImage-09109", dst_image_state->Handle(), loc.dot(Field::dstImage),
                         "is a sparse image with no memory bound");
    }

    const uint32_t region_count = info_ptr->regionCount;
    for (uint32_t i = 0; i < region_count; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const auto &region = info_ptr->pRegions[i];

        skip |= ValidateImageCopyData(device, region, *src_image_state, *dst_image_state, true, region_loc);

        if (check_memcpy) {
            skip |= ValidateMemcpyExtents(region, *src_image_state, *dst_image_state, region_loc);
        }
        if (check_multiplane) {
            skip |= ValidateHostCopyMultiplane(region, *src_image_state, *dst_image_state, region_loc);
        }

        skip |= UsageHostTransferCheck(*src_image_state, region.srcSubresource.aspectMask,
                                       "VUID-VkCopyImageToImageInfo-srcImage-09111", "VUID-VkCopyImageToImageInfo-srcImage-09112",
                                       "VUID-VkCopyImageToImageInfo-srcImage-09113", region_loc.dot(Field::srcSubresource));
        skip |= UsageHostTransferCheck(*dst_image_state, region.dstSubresource.aspectMask,
                                       "VUID-VkCopyImageToImageInfo-dstImage-09111", "VUID-VkCopyImageToImageInfo-dstImage-09112",
                                       "VUID-VkCopyImageToImageInfo-dstImage-09113", region_loc.dot(Field::dstSubresource));

        skip |= ValidateHostCopyCurrentLayout(info_ptr->srcImageLayout, region.srcSubresource, *src_image_state,
                                              region_loc.dot(Field::srcImageLayout));
        skip |= ValidateHostCopyCurrentLayout(info_ptr->dstImageLayout, region.dstSubresource, *dst_image_state,
                                              region_loc.dot(Field::dstImageLayout));

        skip |= ValidateImageBounds(device, *src_image_state, region, region_loc,
                                    "VUID-VkCopyImageToImageInfo-srcSubresource-07970", true);
        skip |= ValidateImageBounds(device, *dst_image_state, region, region_loc,
                                    "VUID-VkCopyImageToImageInfo-dstSubresource-07970", false);

        skip |= ValidateCopyImageRegionCommon(device, *src_image_state, *dst_image_state, region, region_loc);
    }

    return skip;
}

bool CoreChecks::PreCallValidateCopyImageToImageEXT(VkDevice device, const VkCopyImageToImageInfoEXT *pCopyImageToImageInfo,
                                                    const ErrorObject &error_obj) const {
    return PreCallValidateCopyImageToImage(device, pCopyImageToImageInfo, error_obj);
}

template <typename RegionType>
bool CoreChecks::ValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                      const RegionType *pRegions, VkFilter filter, const Location &loc) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<vvl::CommandBuffer>(commandBuffer);
    auto src_image_state = Get<vvl::Image>(srcImage);
    auto dst_image_state = Get<vvl::Image>(dstImage);
    ASSERT_AND_RETURN_SKIP(src_image_state && dst_image_state);

    const bool is_2 = loc.function == Func::vkCmdBlitImage2 || loc.function == Func::vkCmdBlitImage2KHR;
    const Location src_image_loc = loc.dot(Field::srcImage);
    const Location dst_image_loc = loc.dot(Field::dstImage);

    const LogObjectList src_objlist(commandBuffer, srcImage);
    const LogObjectList dst_objlist(commandBuffer, dstImage);
    const LogObjectList all_objlist(commandBuffer, srcImage, dstImage);

    const vvl::CommandBuffer &cb_state = *cb_state_ptr;
    skip |= ValidateCmd(cb_state, loc);

    const char *vuid;

    // src image
    const VkFormat src_format = src_image_state->create_info.format;
    const VkImageType src_type = src_image_state->create_info.imageType;
    {
        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00233" : "VUID-vkCmdBlitImage-srcImage-00233";
        skip |= ValidateImageSampleCount(commandBuffer, *src_image_state, VK_SAMPLE_COUNT_1_BIT, src_image_loc, vuid);

        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00220" : "VUID-vkCmdBlitImage-srcImage-00220";
        skip |= ValidateMemoryIsBoundToImage(src_objlist, *src_image_state, src_image_loc, vuid);

        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00219" : "VUID-vkCmdBlitImage-srcImage-00219";
        skip |=
            ValidateImageUsageFlags(commandBuffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true, vuid, src_image_loc);

        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-01999" : "VUID-vkCmdBlitImage-srcImage-01999";
        skip |=
            ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state, VK_FORMAT_FEATURE_2_BLIT_SRC_BIT, src_image_loc, vuid);

        vuid = is_2 ? "VUID-vkCmdBlitImage2-commandBuffer-01834" : "VUID-vkCmdBlitImage-commandBuffer-01834";
        skip |= ValidateProtectedImage(cb_state, *src_image_state, src_image_loc, vuid);

        if (src_image_state->create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-02545" : "VUID-vkCmdBlitImage-dstImage-02545";
            skip |= LogError(vuid, src_objlist, src_image_loc, "was created with VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT.");
        }

        if (VK_FILTER_LINEAR == filter) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-filter-02001" : "VUID-vkCmdBlitImage-filter-02001";
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state,
                                                    VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_LINEAR_BIT, src_image_loc, vuid);
        } else if (VK_FILTER_CUBIC_IMG == filter) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-filter-02002" : "VUID-vkCmdBlitImage-filter-02002";
            skip |= ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state,
                                                    VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_FILTER_CUBIC_BIT, src_image_loc, vuid);
        }

        if (FormatRequiresYcbcrConversionExplicitly(src_format)) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-06421" : "VUID-vkCmdBlitImage-srcImage-06421";
            skip |= LogError(vuid, src_objlist, src_image_loc,
                             "format (%s) must not be one of the formats requiring sampler YCBCR "
                             "conversion for VK_IMAGE_ASPECT_COLOR_BIT image views",
                             string_VkFormat(src_format));
        }

        if ((VK_FILTER_CUBIC_IMG == filter) && (VK_IMAGE_TYPE_2D != src_type)) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-filter-00237" : "VUID-vkCmdBlitImage-filter-00237";
            skip |= LogError(vuid, src_objlist, loc.dot(Field::filter), "is VK_FILTER_CUBIC_IMG but srcImage was created with %s.",
                             string_VkImageType(src_type));
        }

        // Validate filter for Depth/Stencil formats
        if (vkuFormatIsDepthOrStencil(src_format) && (filter != VK_FILTER_NEAREST)) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00232" : "VUID-vkCmdBlitImage-srcImage-00232";
            skip |= LogError(vuid, src_objlist, src_image_loc, "has depth-stencil format %s but filter is %s.",
                             string_VkFormat(src_format), string_VkFilter(filter));
        }

        if (!IsValueIn(srcImageLayout,
                       {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL})) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImageLayout-01398" : "VUID-vkCmdBlitImage-srcImageLayout-01398";
            skip |= LogError(vuid, src_objlist, loc.dot(Field::srcImageLayout), "is %s.", string_VkImageLayout(srcImageLayout));
        }
    }

    // dst image
    const VkFormat dst_format = dst_image_state->create_info.format;
    const VkImageType dst_type = dst_image_state->create_info.imageType;
    {
        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00234" : "VUID-vkCmdBlitImage-dstImage-00234";
        skip |= ValidateImageSampleCount(commandBuffer, *dst_image_state, VK_SAMPLE_COUNT_1_BIT, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00225" : "VUID-vkCmdBlitImage-dstImage-00225";
        skip |= ValidateMemoryIsBoundToImage(dst_objlist, *dst_image_state, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00224" : "VUID-vkCmdBlitImage-dstImage-00224";
        skip |=
            ValidateImageUsageFlags(commandBuffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, true, vuid, dst_image_loc);

        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-02000" : "VUID-vkCmdBlitImage-dstImage-02000";
        skip |=
            ValidateImageFormatFeatureFlags(commandBuffer, *dst_image_state, VK_FORMAT_FEATURE_2_BLIT_DST_BIT, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-vkCmdBlitImage2-commandBuffer-01835" : "VUID-vkCmdBlitImage-commandBuffer-01835";
        skip |= ValidateProtectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-vkCmdBlitImage2-commandBuffer-01836" : "VUID-vkCmdBlitImage-commandBuffer-01836";
        skip |= ValidateUnprotectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);

        if (dst_image_state->create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-02545" : "VUID-vkCmdBlitImage-dstImage-02545";
            skip |= LogError(vuid, dst_objlist, dst_image_loc, "was created with VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT.");
        }

        if (FormatRequiresYcbcrConversionExplicitly(dst_format)) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-06422" : "VUID-vkCmdBlitImage-dstImage-06422";
            skip |= LogError(vuid, dst_objlist, dst_image_loc,
                             "format (%s) must not be one of the formats requiring sampler YCBCR "
                             "conversion for VK_IMAGE_ASPECT_COLOR_BIT image views",
                             string_VkFormat(dst_format));
        }

        if (!IsValueIn(dstImageLayout,
                       {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL})) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImageLayout-01399" : "VUID-vkCmdBlitImage-dstImageLayout-01399";
            skip |= LogError(vuid, dst_objlist, loc.dot(Field::dstImageLayout), "is %s.", string_VkImageLayout(dstImageLayout));
        }
    }

    // TODO: Need to validate image layouts, which will include layout validation for shared presentable images

    // Validate consistency for unsigned formats
    if (vkuFormatIsUINT(src_format) != vkuFormatIsUINT(dst_format)) {
        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00230" : "VUID-vkCmdBlitImage-srcImage-00230";
        skip |= LogError(vuid, all_objlist, loc, "srcImage format %s is different than dstImage format %s.",
                         string_VkFormat(src_format), string_VkFormat(dst_format));
    }

    // Validate consistency for signed formats
    if (vkuFormatIsSINT(src_format) != vkuFormatIsSINT(dst_format)) {
        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00229" : "VUID-vkCmdBlitImage-srcImage-00229";
        skip |= LogError(vuid, all_objlist, loc, "srcImage format %s is different than dstImage format %s.",
                         string_VkFormat(src_format), string_VkFormat(dst_format));
    }

    // Validate aspect bits and formats for depth/stencil images
    if (vkuFormatIsDepthOrStencil(src_format) || vkuFormatIsDepthOrStencil(dst_format)) {
        if (src_format != dst_format) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00231" : "VUID-vkCmdBlitImage-srcImage-00231";
            skip |= LogError(vuid, all_objlist, loc, "srcImage format %s is different than dstImage format %s.",
                             string_VkFormat(src_format), string_VkFormat(dst_format));
        }
    }

    const bool same_image = (src_image_state == dst_image_state);
    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location src_subresource_loc = region_loc.dot(Field::srcSubresource);
        const Location dst_subresource_loc = region_loc.dot(Field::dstSubresource);
        const RegionType region = pRegions[i];

        // When performing blit from and to same subresource, VK_IMAGE_LAYOUT_GENERAL is the only option
        const VkImageSubresourceLayers &src_subresource = region.srcSubresource;
        const VkImageSubresourceLayers &dst_subresource = region.dstSubresource;

        const bool same_subresource = (same_image && (src_subresource.mipLevel == dst_subresource.mipLevel) &&
                                       RangesIntersect(src_subresource.baseArrayLayer, src_subresource.layerCount,
                                                       dst_subresource.baseArrayLayer, dst_subresource.layerCount));
        if (same_subresource) {
            if (!IsValueIn(srcImageLayout, {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_GENERAL}) ||
                !IsValueIn(dstImageLayout, {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_GENERAL})) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-09459" : "VUID-vkCmdBlitImage-srcImage-09459";
                skip |= LogError(vuid, src_objlist, loc,
                                 "blitting to same same VkImage (miplevel = %u, srcLayer[%u,%u), dstLayer[%u,%u)), but "
                                 "srcImageLayout is %s and dstImageLayout is %s",
                                 src_subresource.mipLevel, src_subresource.baseArrayLayer,
                                 src_subresource.baseArrayLayer + src_subresource.layerCount, dst_subresource.baseArrayLayer,
                                 dst_subresource.baseArrayLayer + dst_subresource.layerCount, string_VkImageLayout(srcImageLayout),
                                 string_VkImageLayout(dstImageLayout));
            }
        }

        vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImageLayout-00221" : "VUID-vkCmdBlitImage-srcImageLayout-00221";
        skip |= VerifyImageLayoutSubresource(cb_state, *src_image_state, src_subresource, srcImageLayout, src_image_loc, vuid);
        vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImageLayout-00226" : "VUID-vkCmdBlitImage-dstImageLayout-00226";
        skip |= VerifyImageLayoutSubresource(cb_state, *dst_image_state, dst_subresource, dstImageLayout, dst_image_loc, vuid);
        skip |= ValidateImageSubresourceLayers(commandBuffer, src_subresource, src_subresource_loc);
        skip |= ValidateImageSubresourceLayers(commandBuffer, dst_subresource, dst_subresource_loc);
        skip |= ValidateImageMipLevel(commandBuffer, *src_image_state, src_subresource.mipLevel, src_subresource_loc);
        skip |= ValidateImageMipLevel(commandBuffer, *dst_image_state, dst_subresource.mipLevel, dst_subresource_loc);
        skip |= ValidateImageArrayLayerRange(commandBuffer, *src_image_state, src_subresource.baseArrayLayer,
                                             src_subresource.layerCount, src_subresource_loc);
        skip |= ValidateImageArrayLayerRange(commandBuffer, *dst_image_state, dst_subresource.baseArrayLayer,
                                             dst_subresource.layerCount, dst_subresource_loc);

        if (src_subresource.layerCount == VK_REMAINING_ARRAY_LAYERS || dst_subresource.layerCount == VK_REMAINING_ARRAY_LAYERS) {
            if (src_subresource.layerCount != VK_REMAINING_ARRAY_LAYERS) {
                if (src_subresource.layerCount != (dst_image_state->create_info.arrayLayers - dst_subresource.baseArrayLayer)) {
                    vuid = is_2 ? "VUID-VkImageBlit2-layerCount-08801" : "VUID-VkImageBlit-layerCount-08801";
                    skip |= LogError(
                        vuid, dst_objlist, src_subresource_loc.dot(Field::layerCount),
                        "(%" PRIu32 ") does not match dstImage arrayLayers (%" PRIu32 ") minus baseArrayLayer (%" PRIu32 ").",
                        src_subresource.layerCount, dst_image_state->create_info.arrayLayers, dst_subresource.baseArrayLayer);
                }
            } else if (dst_subresource.layerCount != VK_REMAINING_ARRAY_LAYERS) {
                if (dst_subresource.layerCount != (src_image_state->create_info.arrayLayers - src_subresource.baseArrayLayer)) {
                    vuid = is_2 ? "VUID-VkImageBlit2-layerCount-08801" : "VUID-VkImageBlit-layerCount-08801";
                    skip |= LogError(
                        vuid, src_objlist, dst_subresource_loc.dot(Field::layerCount),
                        "(%" PRIu32 ") does not match srcImage arrayLayers (%" PRIu32 ") minus baseArrayLayer (%" PRIu32 ").",
                        dst_subresource.layerCount, src_image_state->create_info.arrayLayers, src_subresource.baseArrayLayer);
                }
            }
        } else if (src_subresource.layerCount != dst_subresource.layerCount) {
            vuid = is_2 ? "VUID-VkImageBlit2-layerCount-08800" : "VUID-VkImageBlit-layerCount-08800";
            skip |= LogError(vuid, all_objlist, src_subresource_loc.dot(Field::layerCount),
                             "(%" PRIu32 ") does not match %s (%" PRIu32 ").", src_subresource.layerCount,
                             dst_subresource_loc.dot(Field::layerCount).Fields().c_str(), dst_subresource.layerCount);
        }

        const VkImageAspectFlags src_aspect = src_subresource.aspectMask;
        const VkImageAspectFlags dst_aspect = dst_subresource.aspectMask;
        if (src_aspect != dst_aspect) {
            vuid = is_2 ? "VUID-VkImageBlit2-aspectMask-00238" : "VUID-VkImageBlit-aspectMask-00238";
            skip |=
                LogError(vuid, all_objlist, src_subresource_loc.dot(Field::aspectMask), "(%s) does not match %s (%s).",
                         string_VkImageAspectFlags(src_aspect).c_str(), dst_subresource_loc.dot(Field::aspectMask).Fields().c_str(),
                         string_VkImageAspectFlags(dst_aspect).c_str());
        }

        if (!IsValidAspectMaskForFormat(src_aspect, src_format)) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-aspectMask-00241" : "VUID-vkCmdBlitImage-aspectMask-00241";
            skip |= LogError(vuid, src_objlist, src_subresource_loc.dot(Field::aspectMask),
                             "(%s) is invalid for source image format %s. (%s)", string_VkImageAspectFlags(src_aspect).c_str(),
                             string_VkFormat(src_format), DescribeValidAspectMaskForFormat(src_format).c_str());
        }

        if (!IsValidAspectMaskForFormat(dst_aspect, dst_format)) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-aspectMask-00242" : "VUID-vkCmdBlitImage-aspectMask-00242";
            skip |= LogError(vuid, dst_objlist, dst_subresource_loc.dot(Field::aspectMask),
                             "(%s) is invalid for destination image format %s. (%s)", string_VkImageAspectFlags(src_aspect).c_str(),
                             string_VkFormat(src_format), DescribeValidAspectMaskForFormat(dst_format).c_str());
        }

        // Validate source image offsets
        VkExtent3D src_extent = src_image_state->GetEffectiveSubresourceExtent(src_subresource);
        if (VK_IMAGE_TYPE_1D == src_type) {
            if ((0 != region.srcOffsets[0].y) || (1 != region.srcOffsets[1].y)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00245" : "VUID-vkCmdBlitImage-srcImage-00245";
                skip |=
                    LogError(vuid, src_objlist, region_loc,
                             "srcOffsets[0].y is %" PRId32 " and srcOffsets[1].y is %" PRId32 " but srcImage is VK_IMAGE_TYPE_1D.",
                             region.srcOffsets[0].y, region.srcOffsets[1].y);
            }
        }

        if ((VK_IMAGE_TYPE_1D == src_type) || (VK_IMAGE_TYPE_2D == src_type)) {
            if ((0 != region.srcOffsets[0].z) || (1 != region.srcOffsets[1].z)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00247" : "VUID-vkCmdBlitImage-srcImage-00247";
                skip |= LogError(vuid, src_objlist, region_loc,
                                 "srcOffsets[0].z is %" PRId32 " and srcOffsets[1].z is %" PRId32 " but srcImage is %s.",
                                 region.srcOffsets[0].z, region.srcOffsets[1].z, string_VkImageType(src_type));
            }
        }

        bool oob = false;
        if ((region.srcOffsets[0].x < 0) || (region.srcOffsets[0].x > static_cast<int32_t>(src_extent.width)) ||
            (region.srcOffsets[1].x < 0) || (region.srcOffsets[1].x > static_cast<int32_t>(src_extent.width))) {
            oob = true;
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcOffset-00243" : "VUID-vkCmdBlitImage-srcOffset-00243";
            skip |= LogError(vuid, src_objlist, region_loc,
                             "srcOffsets[0].x is %" PRId32 " and srcOffsets[1].x is %" PRId32
                             " which exceed srcSubresource width extent (%" PRIu32 ").",
                             region.srcOffsets[0].x, region.srcOffsets[1].x, src_extent.width);
        }
        if ((region.srcOffsets[0].y < 0) || (region.srcOffsets[0].y > static_cast<int32_t>(src_extent.height)) ||
            (region.srcOffsets[1].y < 0) || (region.srcOffsets[1].y > static_cast<int32_t>(src_extent.height))) {
            oob = true;
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcOffset-00244" : "VUID-vkCmdBlitImage-srcOffset-00244";
            skip |= LogError(vuid, src_objlist, region_loc,
                             "srcOffsets[0].y is %" PRId32 " and srcOffsets[1].y is %" PRId32
                             " which exceed srcSubresource height extent (%" PRIu32 ").",
                             region.srcOffsets[0].y, region.srcOffsets[1].y, src_extent.height);
        }
        if ((region.srcOffsets[0].z < 0) || (region.srcOffsets[0].z > static_cast<int32_t>(src_extent.depth)) ||
            (region.srcOffsets[1].z < 0) || (region.srcOffsets[1].z > static_cast<int32_t>(src_extent.depth))) {
            oob = true;
            vuid = is_2 ? "VUID-VkBlitImageInfo2-srcOffset-00246" : "VUID-vkCmdBlitImage-srcOffset-00246";
            skip |= LogError(vuid, src_objlist, region_loc,
                             "srcOffsets[0].z is %" PRId32 " and srcOffsets[1].z is %" PRId32
                             " which exceed srcSubresource depth extent (%" PRIu32 ").",
                             region.srcOffsets[0].z, region.srcOffsets[1].z, src_extent.depth);
        }
        if (oob) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-pRegions-00215" : "VUID-vkCmdBlitImage-pRegions-00215";
            skip |= LogError(vuid, src_objlist, region_loc, "source image blit region exceeds image dimensions.");
        }

        // Validate dest image offsets
        VkExtent3D dst_extent = dst_image_state->GetEffectiveSubresourceExtent(dst_subresource);
        if (VK_IMAGE_TYPE_1D == dst_type) {
            if ((0 != region.dstOffsets[0].y) || (1 != region.dstOffsets[1].y)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00250" : "VUID-vkCmdBlitImage-dstImage-00250";
                skip |=
                    LogError(vuid, dst_objlist, region_loc,
                             "dstOffsets[0].y is %" PRId32 " and dstOffsets[1].y is %" PRId32 " but dstImage is VK_IMAGE_TYPE_1D.",
                             region.dstOffsets[0].y, region.dstOffsets[1].y);
            }
        }

        if ((VK_IMAGE_TYPE_1D == dst_type) || (VK_IMAGE_TYPE_2D == dst_type)) {
            if ((0 != region.dstOffsets[0].z) || (1 != region.dstOffsets[1].z)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-dstImage-00252" : "VUID-vkCmdBlitImage-dstImage-00252";
                skip |= LogError(vuid, dst_objlist, region_loc,
                                 "dstOffsets[0].z is %" PRId32 " and dstOffsets[1].z is %" PRId32 " but dstImage is %s.",
                                 region.dstOffsets[0].z, region.dstOffsets[1].z, string_VkImageType(dst_type));
            }
        }

        oob = false;
        if ((region.dstOffsets[0].x < 0) || (region.dstOffsets[0].x > static_cast<int32_t>(dst_extent.width)) ||
            (region.dstOffsets[1].x < 0) || (region.dstOffsets[1].x > static_cast<int32_t>(dst_extent.width))) {
            oob = true;
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstOffset-00248" : "VUID-vkCmdBlitImage-dstOffset-00248";
            skip |= LogError(vuid, dst_objlist, region_loc,
                             "dstOffsets[0].x is %" PRId32 " and dstOffsets[1].x is %" PRId32
                             " which exceed dstSubresource width extent (%" PRIu32 ").",
                             region.dstOffsets[0].x, region.dstOffsets[1].x, dst_extent.width);
        }
        if ((region.dstOffsets[0].y < 0) || (region.dstOffsets[0].y > static_cast<int32_t>(dst_extent.height)) ||
            (region.dstOffsets[1].y < 0) || (region.dstOffsets[1].y > static_cast<int32_t>(dst_extent.height))) {
            oob = true;
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstOffset-00249" : "VUID-vkCmdBlitImage-dstOffset-00249";
            skip |= LogError(vuid, dst_objlist, region_loc,
                             "dstOffsets[0].y is %" PRId32 " and dstOffsets[1].y is %" PRId32
                             " which exceed dstSubresource height extent (%" PRIu32 ").",
                             region.dstOffsets[0].x, region.dstOffsets[1].x, dst_extent.height);
        }
        if ((region.dstOffsets[0].z < 0) || (region.dstOffsets[0].z > static_cast<int32_t>(dst_extent.depth)) ||
            (region.dstOffsets[1].z < 0) || (region.dstOffsets[1].z > static_cast<int32_t>(dst_extent.depth))) {
            oob = true;
            vuid = is_2 ? "VUID-VkBlitImageInfo2-dstOffset-00251" : "VUID-vkCmdBlitImage-dstOffset-00251";
            skip |= LogError(vuid, dst_objlist, region_loc,
                             "dstOffsets[0].z is %" PRId32 " and dstOffsets[1].z is %" PRId32
                             " which exceed dstSubresource depth extent (%" PRIu32 ").",
                             region.dstOffsets[0].z, region.dstOffsets[1].z, dst_extent.depth);
        }
        if (oob) {
            vuid = is_2 ? "VUID-VkBlitImageInfo2-pRegions-00216" : "VUID-vkCmdBlitImage-pRegions-00216";
            skip |= LogError(vuid, dst_objlist, region_loc, "destination image blit region exceeds image dimensions.");
        }

        // pre-maintenance8 both src/dst had to both match, after we only validate them independently
        if (!enabled_features.maintenance8 && (src_type == VK_IMAGE_TYPE_3D || dst_type == VK_IMAGE_TYPE_3D)) {
            if ((src_subresource.baseArrayLayer != 0) || (src_subresource.layerCount != 1) ||
                (dst_subresource.baseArrayLayer != 0) || (dst_subresource.layerCount != 1)) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-srcImage-00240" : "VUID-vkCmdBlitImage-srcImage-00240";
                skip |= LogError(vuid, all_objlist, region_loc,
                                 "srcImage %s\n"
                                 "dstImage %s\n"
                                 "srcSubresource (baseArrayLayer = %" PRIu32 ", layerCount = %" PRIu32
                                 ")\n"
                                 "dstSubresource (baseArrayLayer = %" PRIu32 ", layerCount = %" PRIu32 ")\n",
                                 string_VkImageType(src_type), string_VkImageType(dst_type), src_subresource.baseArrayLayer,
                                 src_subresource.layerCount, dst_subresource.baseArrayLayer, dst_subresource.layerCount);
            }
        } else if (enabled_features.maintenance8) {
            if (src_type == VK_IMAGE_TYPE_3D) {
                if (src_subresource.baseArrayLayer != 0 || src_subresource.layerCount != 1 || dst_subresource.layerCount != 1) {
                    vuid = is_2 ? "VUID-VkBlitImageInfo2-maintenance8-10207" : "VUID-vkCmdBlitImage-maintenance8-10207";
                    skip |= LogError(vuid, all_objlist, src_subresource_loc,
                                     "(src baseArrayLayer = %" PRIu32 ",  src layerCount = %" PRIu32 ", dst layerCount = %" PRIu32
                                     ") but srcImage is VK_IMAGE_TYPE_3D",
                                     src_subresource.baseArrayLayer, src_subresource.layerCount, dst_subresource.layerCount);
                }
            } else if (uint32_t diff = static_cast<uint32_t>(abs(region.dstOffsets[0].z - region.dstOffsets[1].z));
                       diff != src_subresource.layerCount) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-maintenance8-10579" : "VUID-vkCmdBlitImage-maintenance8-10579";
                skip |= LogError(vuid, all_objlist, region_loc,
                                 "has the absolute difference of dstOffsets[0].z (%" PRIu32 ") and dstOffsets[1].z (%" PRIu32
                                 ") = %" PRIu32 ", which is not equal to srcSubresource.layerCount (%" PRIu32 ")",
                                 region.dstOffsets[0].z, region.dstOffsets[1].z, diff, src_subresource.layerCount);
            }
            if (dst_type == VK_IMAGE_TYPE_3D) {
                if (dst_subresource.baseArrayLayer != 0 || dst_subresource.layerCount != 1 || src_subresource.layerCount != 1) {
                    vuid = is_2 ? "VUID-VkBlitImageInfo2-maintenance8-10208" : "VUID-vkCmdBlitImage-maintenance8-10208";
                    skip |= LogError(vuid, all_objlist, dst_subresource_loc,
                                     "(dst baseArrayLayer = %" PRIu32 ", dst layerCount = %" PRIu32 ", src layerCount = %" PRIu32
                                     ") but dstImage is VK_IMAGE_TYPE_3D",
                                     dst_subresource.baseArrayLayer, dst_subresource.layerCount, src_subresource.layerCount);
                }
            } else if (uint32_t diff = static_cast<uint32_t>(abs(region.srcOffsets[0].z - region.srcOffsets[1].z));
                       diff != dst_subresource.layerCount) {
                vuid = is_2 ? "VUID-VkBlitImageInfo2-maintenance8-10580" : "VUID-vkCmdBlitImage-maintenance8-10580";
                skip |= LogError(vuid, all_objlist, region_loc,
                                 "has the absolute difference of srcOffsets[0].z (%" PRIu32 ") and srcOffsets[1].z (%" PRIu32
                                 ") = %" PRIu32 ", which is not equal to dstSubresource.layerCount (%" PRIu32 ")",
                                 region.srcOffsets[0].z, region.srcOffsets[1].z, diff, dst_subresource.layerCount);
            }
        }

        // The union of all source regions, and the union of all destination regions, specified by the elements of regions,
        // must not overlap in memory
        if (srcImage == dstImage) {
            for (uint32_t j = 0; j < regionCount; j++) {
                if (RegionIntersectsBlit(&region, &pRegions[j], src_image_state->create_info.imageType,
                                         vkuFormatIsMultiplane(src_format))) {
                    vuid = is_2 ? "VUID-VkBlitImageInfo2-pRegions-00217" : "VUID-vkCmdBlitImage-pRegions-00217";
                    skip |=
                        LogError(vuid, all_objlist, loc, "pRegion[%" PRIu32 "] src overlaps with pRegions[%" PRIu32 "] dst.", i, j);
                }
            }
        }
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                             VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                             const VkImageBlit *pRegions, VkFilter filter, const ErrorObject &error_obj) const {
    return ValidateCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter,
                                error_obj.location);
}

bool CoreChecks::PreCallValidateCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo,
                                                 const ErrorObject &error_obj) const {
    return PreCallValidateCmdBlitImage2(commandBuffer, pBlitImageInfo, error_obj);
}

bool CoreChecks::PreCallValidateCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2 *pBlitImageInfo,
                                              const ErrorObject &error_obj) const {
    return ValidateCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                                pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                                pBlitImageInfo->filter, error_obj.location.dot(Field::pBlitImageInfo));
}

template <typename RegionType>
void CoreChecks::RecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout, VkImage dstImage,
                                    VkImageLayout dstImageLayout, uint32_t regionCount, const RegionType *pRegions,
                                    VkFilter filter) {
    auto cb_state = GetWrite<vvl::CommandBuffer>(commandBuffer);
    auto src_image_state = Get<vvl::Image>(srcImage);
    auto dst_image_state = Get<vvl::Image>(dstImage);
    ASSERT_AND_RETURN(src_image_state && dst_image_state);

    // Make sure that all image slices are updated to correct layout
    for (uint32_t i = 0; i < regionCount; ++i) {
        cb_state->SetImageInitialLayout(*src_image_state, pRegions[i].srcSubresource, srcImageLayout);
        cb_state->SetImageInitialLayout(*dst_image_state, pRegions[i].dstSubresource, dstImageLayout);
    }
}

void CoreChecks::PostCallRecordCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                            VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                            const VkImageBlit *pRegions, VkFilter filter, const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount,
                                             pRegions, filter, record_obj);
    RecordCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

void CoreChecks::PostCallRecordCmdBlitImage2KHR(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo,
                                                const RecordObject &record_obj) {
    PostCallRecordCmdBlitImage2(commandBuffer, pBlitImageInfo, record_obj);
}

void CoreChecks::PostCallRecordCmdBlitImage2(VkCommandBuffer commandBuffer, const VkBlitImageInfo2KHR *pBlitImageInfo,
                                             const RecordObject &record_obj) {
    BaseClass::PostCallRecordCmdBlitImage2(commandBuffer, pBlitImageInfo, record_obj);
    RecordCmdBlitImage(commandBuffer, pBlitImageInfo->srcImage, pBlitImageInfo->srcImageLayout, pBlitImageInfo->dstImage,
                       pBlitImageInfo->dstImageLayout, pBlitImageInfo->regionCount, pBlitImageInfo->pRegions,
                       pBlitImageInfo->filter);
}

template <typename RegionType>
bool CoreChecks::ValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                         VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                         const RegionType *pRegions, const Location &loc) const {
    bool skip = false;
    auto cb_state_ptr = GetRead<vvl::CommandBuffer>(commandBuffer);
    auto src_image_state = Get<vvl::Image>(srcImage);
    auto dst_image_state = Get<vvl::Image>(dstImage);
    ASSERT_AND_RETURN_SKIP(src_image_state && dst_image_state);

    const bool is_2 = loc.function == Func::vkCmdResolveImage2 || loc.function == Func::vkCmdResolveImage2KHR;
    const char *vuid;
    const Location src_image_loc = loc.dot(Field::srcImage);
    const Location dst_image_loc = loc.dot(Field::dstImage);

    const LogObjectList src_objlist(commandBuffer, srcImage);
    const LogObjectList dst_objlist(commandBuffer, dstImage);
    const LogObjectList all_objlist(commandBuffer, srcImage, dstImage);
    const vvl::CommandBuffer &cb_state = *cb_state_ptr;
    skip |= ValidateCmd(cb_state, loc);

    // src image
    {
        vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-00256" : "VUID-vkCmdResolveImage-srcImage-00256";
        skip |= ValidateMemoryIsBoundToImage(src_objlist, *src_image_state, src_image_loc, vuid);

        vuid = is_2 ? "VUID-vkCmdResolveImage2-commandBuffer-01837" : "VUID-vkCmdResolveImage-commandBuffer-01837";
        skip |= ValidateProtectedImage(cb_state, *src_image_state, src_image_loc, vuid);

        vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-06762" : "VUID-vkCmdResolveImage-srcImage-06762";
        skip |=
            ValidateImageUsageFlags(commandBuffer, *src_image_state, VK_IMAGE_USAGE_TRANSFER_SRC_BIT, true, vuid, src_image_loc);

        vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-06763" : "VUID-vkCmdResolveImage-srcImage-06763";
        skip |= ValidateImageFormatFeatureFlags(commandBuffer, *src_image_state, VK_FORMAT_FEATURE_TRANSFER_SRC_BIT, src_image_loc,
                                                vuid);

        if (src_image_state->create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-02546" : "VUID-vkCmdResolveImage-dstImage-02546";
            skip |= LogError(vuid, src_objlist, src_image_loc,
                             "must not have been created with flags containing "
                             "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT");
        }

        if (!IsValueIn(srcImageLayout,
                       {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL})) {
            vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImageLayout-01400" : "VUID-vkCmdResolveImage-srcImageLayout-01400";
            const LogObjectList objlist(cb_state.Handle(), src_image_state->Handle());
            skip |= LogError(vuid, objlist, loc.dot(Field::srcImageLayout), "is %s.", string_VkImageLayout(srcImageLayout));
        }

        if (src_image_state->create_info.samples == VK_SAMPLE_COUNT_1_BIT) {
            vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-00257" : "VUID-vkCmdResolveImage-srcImage-00257";
            skip |= LogError(vuid, src_objlist, src_image_loc, "was created with sample count VK_SAMPLE_COUNT_1_BIT.");
        }
    }

    // dst image
    {
        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-00258" : "VUID-vkCmdResolveImage-dstImage-00258";
        skip |= ValidateMemoryIsBoundToImage(dst_objlist, *dst_image_state, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-02003" : "VUID-vkCmdResolveImage-dstImage-02003";
        skip |= ValidateImageFormatFeatureFlags(commandBuffer, *dst_image_state, VK_FORMAT_FEATURE_2_COLOR_ATTACHMENT_BIT,
                                                dst_image_loc, vuid);
        vuid = is_2 ? "VUID-vkCmdResolveImage2-commandBuffer-01838" : "VUID-vkCmdResolveImage-commandBuffer-01838";
        skip |= ValidateProtectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-vkCmdResolveImage2-commandBuffer-01839" : "VUID-vkCmdResolveImage-commandBuffer-01839";
        skip |= ValidateUnprotectedImage(cb_state, *dst_image_state, dst_image_loc, vuid);

        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-06764" : "VUID-vkCmdResolveImage-dstImage-06764";
        skip |=
            ValidateImageUsageFlags(commandBuffer, *dst_image_state, VK_IMAGE_USAGE_TRANSFER_DST_BIT, true, vuid, dst_image_loc);

        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-06765" : "VUID-vkCmdResolveImage-dstImage-06765";
        skip |= ValidateImageFormatFeatureFlags(commandBuffer, *dst_image_state, VK_FORMAT_FEATURE_TRANSFER_DST_BIT, dst_image_loc,
                                                vuid);

        if (dst_image_state->create_info.flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
            vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-02546" : "VUID-vkCmdResolveImage-dstImage-02546";
            skip |= LogError(vuid, dst_objlist, dst_image_loc,
                             "must not have been created with flags containing "
                             "VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT");
        }

        if (!IsValueIn(dstImageLayout,
                       {VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL})) {
            vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImageLayout-01401" : "VUID-vkCmdResolveImage-dstImageLayout-01401";
            const LogObjectList objlist(cb_state.Handle(), dst_image_state->Handle());
            skip |= LogError(vuid, objlist, loc.dot(Field::dstImageLayout), "is %s.", string_VkImageLayout(dstImageLayout));
        }

        if (dst_image_state->create_info.samples != VK_SAMPLE_COUNT_1_BIT) {
            vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-00259" : "VUID-vkCmdResolveImage-dstImage-00259";
            skip |= LogError(vuid, dst_objlist, dst_image_loc, "was created with sample count (%s) (not VK_SAMPLE_COUNT_1_BIT).",
                             string_VkSampleCountFlagBits(dst_image_state->create_info.samples));
        }
    }

    // For each region, the number of layers in the image subresource should not be zero
    // For each region, src and dest image aspect must be color only
    for (uint32_t i = 0; i < regionCount; i++) {
        const Location region_loc = loc.dot(Field::pRegions, i);
        const Location src_subresource_loc = region_loc.dot(Field::srcSubresource);
        const Location dst_subresource_loc = region_loc.dot(Field::dstSubresource);
        const RegionType region = pRegions[i];
        const VkImageSubresourceLayers &src_subresource = region.srcSubresource;
        const VkImageSubresourceLayers &dst_subresource = region.dstSubresource;

        skip |= ValidateImageSubresourceLayers(commandBuffer, src_subresource, src_subresource_loc);
        skip |= ValidateImageSubresourceLayers(commandBuffer, dst_subresource, dst_subresource_loc);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImageLayout-00260" : "VUID-vkCmdResolveImage-srcImageLayout-00260";
        skip |= VerifyImageLayoutSubresource(cb_state, *src_image_state, src_subresource, srcImageLayout, src_image_loc, vuid);
        vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImageLayout-00262" : "VUID-vkCmdResolveImage-dstImageLayout-00262";
        skip |= VerifyImageLayoutSubresource(cb_state, *dst_image_state, dst_subresource, dstImageLayout, dst_image_loc, vuid);
        skip |= ValidateImageMipLevel(commandBuffer, *src_image_state, src_subresource.mipLevel, src_subresource_loc);
        skip |= ValidateImageMipLevel(commandBuffer, *dst_image_state, dst_subresource.mipLevel, dst_subresource_loc);
        skip |= ValidateImageArrayLayerRange(commandBuffer, *src_image_state, src_subresource.baseArrayLayer,
                                             src_subresource.layerCount, src_subresource_loc);
        skip |= ValidateImageArrayLayerRange(commandBuffer, *dst_image_state, dst_subresource.baseArrayLayer,
                                             dst_subresource.layerCount, dst_subresource_loc);

        if (src_subresource.layerCount == VK_REMAINING_ARRAY_LAYERS || dst_subresource.layerCount == VK_REMAINING_ARRAY_LAYERS) {
            if (src_subresource.layerCount != VK_REMAINING_ARRAY_LAYERS) {
                if (src_subresource.layerCount != (dst_image_state->create_info.arrayLayers - dst_subresource.baseArrayLayer)) {
                    vuid = is_2 ? "VUID-VkImageResolve2-layerCount-08804" : "VUID-VkImageResolve-layerCount-08804";
                    skip |= LogError(
                        vuid, dst_objlist, src_subresource_loc.dot(Field::layerCount),
                        "(%" PRIu32 ") does not match dstImage arrayLayers (%" PRIu32 ") minus baseArrayLayer (%" PRIu32 ").",
                        src_subresource.layerCount, dst_image_state->create_info.arrayLayers, dst_subresource.baseArrayLayer);
                }
            } else if (dst_subresource.layerCount != VK_REMAINING_ARRAY_LAYERS) {
                if (dst_subresource.layerCount != (src_image_state->create_info.arrayLayers - src_subresource.baseArrayLayer)) {
                    vuid = is_2 ? "VUID-VkImageResolve2-layerCount-08804" : "VUID-VkImageResolve-layerCount-08804";
                    skip |= LogError(
                        vuid, src_objlist, dst_subresource_loc.dot(Field::layerCount),
                        "(%" PRIu32 ") does not match srcImage arrayLayers (%" PRIu32 ") minus baseArrayLayer (%" PRIu32 ").",
                        dst_subresource.layerCount, src_image_state->create_info.arrayLayers, src_subresource.baseArrayLayer);
                }
            }
        } else if (src_subresource.layerCount != dst_subresource.layerCount) {
            vuid = is_2 ? "VUID-VkImageResolve2-layerCount-08803" : "VUID-VkImageResolve-layerCount-08803";
            skip |= LogError(vuid, all_objlist, src_subresource_loc.dot(Field::layerCount),
                             "(%" PRIu32 ") does not match %s (%" PRIu32 ").", region.srcSubresource.layerCount,
                             dst_subresource_loc.dot(Field::layerCount).Fields().c_str(), region.dstSubresource.layerCount);
        }
        // For each region, src and dest image aspect must be color only
        const VkImageAspectFlags src_aspect = src_subresource.aspectMask;
        const VkImageAspectFlags dst_aspect = dst_subresource.aspectMask;
        if ((src_aspect != VK_IMAGE_ASPECT_COLOR_BIT) || (dst_aspect != VK_IMAGE_ASPECT_COLOR_BIT)) {
            vuid = is_2 ? "VUID-VkImageResolve2-aspectMask-00266" : "VUID-VkImageResolve-aspectMask-00266";
            skip |= LogError(vuid, all_objlist, src_subresource_loc.dot(Field::aspectMask),
                             "(%s) and dstSubresource.aspectMask (%s) must only be VK_IMAGE_ASPECT_COLOR_BIT.",
                             string_VkImageAspectFlags(src_aspect).c_str(), string_VkImageAspectFlags(dst_aspect).c_str());
        }

        const VkImageType src_image_type = src_image_state->create_info.imageType;
        const VkImageType dst_image_type = dst_image_state->create_info.imageType;

        if (VK_IMAGE_TYPE_3D == dst_image_type) {
            if (src_subresource.layerCount != 1) {
                vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-04446" : "VUID-vkCmdResolveImage-srcImage-04446";
                skip |= LogError(vuid, src_objlist, src_subresource_loc.dot(Field::layerCount),
                                 "is %" PRIu32 " but dstImage is 3D.", src_subresource.layerCount);
            }
            if ((dst_subresource.baseArrayLayer != 0) || (dst_subresource.layerCount != 1)) {
                vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-04447" : "VUID-vkCmdResolveImage-srcImage-04447";
                skip |= LogError(vuid, dst_objlist, dst_subresource_loc.dot(Field::baseArrayLayer),
                                 "is %" PRIu32 " and layerCount is %" PRIu32 " but dstImage 3D.", dst_subresource.baseArrayLayer,
                                 dst_subresource.layerCount);
            }
        }

        if (VK_IMAGE_TYPE_1D == src_image_type) {
            if ((region.srcOffset.y != 0) || (region.extent.height != 1)) {
                vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-00271" : "VUID-vkCmdResolveImage-srcImage-00271";
                skip |= LogError(vuid, src_objlist, region_loc,
                                 "srcOffset.y is %" PRId32 ", extent.height is %" PRIu32 ", but srcImage (%s) is 1D.",
                                 region.srcOffset.y, region.extent.height, FormatHandle(src_image_state->Handle()).c_str());
            }
        }
        if ((VK_IMAGE_TYPE_1D == src_image_type) || (VK_IMAGE_TYPE_2D == src_image_type)) {
            if ((region.srcOffset.z != 0) || (region.extent.depth != 1)) {
                vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-00273" : "VUID-vkCmdResolveImage-srcImage-00273";
                skip |= LogError(vuid, src_objlist, region_loc,
                                 "srcOffset.z is %" PRId32 ", extent.depth is %" PRIu32 ", but srcImage (%s) is 2D.",
                                 region.srcOffset.z, region.extent.depth, FormatHandle(src_image_state->Handle()).c_str());
            }
        }

        if (VK_IMAGE_TYPE_1D == dst_image_type) {
            if ((region.dstOffset.y != 0) || (region.extent.height != 1)) {
                vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-00276" : "VUID-vkCmdResolveImage-dstImage-00276";
                skip |= LogError(vuid, dst_objlist, region_loc,
                                 "dstOffset.y is %" PRId32 ", extent.height is %" PRIu32 ", but dstImage (%s) is 1D.",
                                 region.dstOffset.y, region.extent.height, FormatHandle(dst_image_state->Handle()).c_str());
            }
        }
        if ((VK_IMAGE_TYPE_1D == dst_image_type) || (VK_IMAGE_TYPE_2D == dst_image_type)) {
            if ((region.dstOffset.z != 0) || (region.extent.depth != 1)) {
                vuid = is_2 ? "VUID-VkResolveImageInfo2-dstImage-00278" : "VUID-vkCmdResolveImage-dstImage-00278";
                skip |= LogError(vuid, dst_objlist, region_loc,
                                 "dstOffset.z is %" PRId32 ", extent.depth is %" PRIu32 ", but dstImage (%s) is 2D.",
                                 region.dstOffset.z, region.extent.depth, FormatHandle(dst_image_state->Handle()).c_str());
            }
        }

        // Each srcImage dimension offset + extent limits must fall with image subresource extent
        VkExtent3D subresource_extent = src_image_state->GetEffectiveSubresourceExtent(src_subresource);
        // MipLevel bound is checked already and adding extra errors with a "subresource extent of zero" is confusing to
        // developer
        if (src_subresource.mipLevel < src_image_state->create_info.mipLevels) {
            uint32_t extent_check = ExceedsBounds(&(region.srcOffset), &(region.extent), &subresource_extent);
            if ((extent_check & kXBit) != 0) {
                vuid = is_2 ? "VUID-VkResolveImageInfo2-srcOffset-00269" : "VUID-vkCmdResolveImage-srcOffset-00269";
                skip |= LogError(vuid, src_objlist, region_loc,
                                 "srcOffset.x (%" PRId32 ") + extent.width (%" PRIu32
                                 ") exceeds srcSubresource.extent.width (%" PRIu32 ").",
                                 region.srcOffset.x, region.extent.width, subresource_extent.width);
            }

            if ((extent_check & kYBit) != 0) {
                vuid = is_2 ? "VUID-VkResolveImageInfo2-srcOffset-00270" : "VUID-vkCmdResolveImage-srcOffset-00270";
                skip |= LogError(vuid, src_objlist, region_loc,
                                 "srcOffset.x (%" PRId32 ") + extent.height (%" PRIu32
                                 ") exceeds srcSubresource.extent.height (%" PRIu32 ").",
                                 region.srcOffset.y, region.extent.height, subresource_extent.height);
            }

            if ((extent_check & kZBit) != 0) {
                vuid = is_2 ? "VUID-VkResolveImageInfo2-srcOffset-00272" : "VUID-vkCmdResolveImage-srcOffset-00272";
                skip |= LogError(vuid, src_objlist, region_loc,
                                 "srcOffset.x (%" PRId32 ") + extent.depth (%" PRIu32
                                 ") exceeds srcSubresource.extent.depth (%" PRIu32 ").",
                                 region.srcOffset.z, region.extent.depth, subresource_extent.depth);
            }
        }

        // Each dstImage dimension offset + extent limits must fall with image subresource extent
        subresource_extent = dst_image_state->GetEffectiveSubresourceExtent(dst_subresource);
        // MipLevel bound is checked already and adding extra errors with a "subresource extent of zero" is confusing to
        // developer
        if (dst_subresource.mipLevel < dst_image_state->create_info.mipLevels) {
            uint32_t extent_check = ExceedsBounds(&(region.dstOffset), &(region.extent), &subresource_extent);
            if ((extent_check & kXBit) != 0) {
                vuid = is_2 ? "VUID-VkResolveImageInfo2-dstOffset-00274" : "VUID-vkCmdResolveImage-dstOffset-00274";
                skip |= LogError(vuid, dst_objlist, region_loc,
                                 "dstOffset.x (%" PRId32 ") + extent.width (%" PRIu32
                                 ") exceeds dstSubresource.extent.width (%" PRIu32 ").",
                                 region.dstOffset.x, region.extent.width, subresource_extent.width);
            }

            if ((extent_check & kYBit) != 0) {
                vuid = is_2 ? "VUID-VkResolveImageInfo2-dstOffset-00275" : "VUID-vkCmdResolveImage-dstOffset-00275";
                skip |= LogError(vuid, dst_objlist, region_loc,
                                 "dstOffset.x (%" PRId32 ") + extent.height (%" PRIu32
                                 ") exceeds dstSubresource.extent.height (%" PRIu32 ").",
                                 region.dstOffset.x, region.extent.height, subresource_extent.height);
            }

            if ((extent_check & kZBit) != 0) {
                vuid = is_2 ? "VUID-VkResolveImageInfo2-dstOffset-00277" : "VUID-vkCmdResolveImage-dstOffset-00277";
                skip |= LogError(vuid, dst_objlist, region_loc,
                                 "dstOffset.x (%" PRId32 ") + extent.depth (%" PRIu32
                                 ") exceeds dstSubresource.extent.depth (%" PRIu32 ").",
                                 region.dstOffset.x, region.extent.depth, subresource_extent.depth);
            }
        }
    }

    if (src_image_state->create_info.format != dst_image_state->create_info.format) {
        vuid = is_2 ? "VUID-VkResolveImageInfo2-srcImage-01386" : "VUID-vkCmdResolveImage-srcImage-01386";
        skip |=
            LogError(vuid, all_objlist, src_image_loc, "was created with format %s but dstImage format is %s.",
                     string_VkFormat(src_image_state->create_info.format), string_VkFormat(dst_image_state->create_info.format));
    }
    return skip;
}

bool CoreChecks::PreCallValidateCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
                                                VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount,
                                                const VkImageResolve *pRegions, const ErrorObject &error_obj) const {
    return ValidateCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions,
                                   error_obj.location);
}

bool CoreChecks::PreCallValidateCmdResolveImage2KHR(VkCommandBuffer commandBuffer, const VkResolveImageInfo2KHR *pResolveImageInfo,
                                                    const ErrorObject &error_obj) const {
    return PreCallValidateCmdResolveImage2(commandBuffer, pResolveImageInfo, error_obj);
}

bool CoreChecks::PreCallValidateCmdResolveImage2(VkCommandBuffer commandBuffer, const VkResolveImageInfo2 *pResolveImageInfo,
                                                 const ErrorObject &error_obj) const {
    return ValidateCmdResolveImage(commandBuffer, pResolveImageInfo->srcImage, pResolveImageInfo->srcImageLayout,
                                   pResolveImageInfo->dstImage, pResolveImageInfo->dstImageLayout, pResolveImageInfo->regionCount,
                                   pResolveImageInfo->pRegions, error_obj.location.dot(Field::pResolveImageInfo));
}
