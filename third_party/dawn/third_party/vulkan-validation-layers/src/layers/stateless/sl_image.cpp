/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2024 Google Inc.
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

#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/vulkan_core.h>
#include "error_message/error_strings.h"
#include "stateless/stateless_validation.h"
#include "generated/enum_flag_bits.h"
#include "utils/vk_layer_utils.h"

namespace stateless {

bool Device::manual_PreCallValidateCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo,
                                               const VkAllocationCallbacks *pAllocator, VkImage *pImage,
                                               const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (pCreateInfo == nullptr) {
        return skip;
    }
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    if (pCreateInfo->sharingMode == VK_SHARING_MODE_CONCURRENT) {
        // If sharingMode is VK_SHARING_MODE_CONCURRENT, queueFamilyIndexCount must be greater than 1
        auto const queue_family_index_count = pCreateInfo->queueFamilyIndexCount;
        if (queue_family_index_count <= 1) {
            skip |= LogError("VUID-VkImageCreateInfo-sharingMode-00942", device, create_info_loc.dot(Field::queueFamilyIndexCount),
                             "is %" PRIu32 ".", queue_family_index_count);
        }

        // If sharingMode is VK_SHARING_MODE_CONCURRENT, pQueueFamilyIndices must be a pointer to an array of
        // queueFamilyIndexCount uint32_t values
        if (pCreateInfo->pQueueFamilyIndices == nullptr) {
            skip |= LogError("VUID-VkImageCreateInfo-sharingMode-00941", device, create_info_loc.dot(Field::pQueueFamilyIndices),
                             "is NULL.");
        }
    }

    skip |= context.ValidateNotZero(pCreateInfo->extent.width == 0, "VUID-VkImageCreateInfo-extent-00944",
                                    create_info_loc.dot(Field::extent).dot(Field::width));
    skip |= context.ValidateNotZero(pCreateInfo->extent.height == 0, "VUID-VkImageCreateInfo-extent-00945",
                                    create_info_loc.dot(Field::extent).dot(Field::height));
    skip |= context.ValidateNotZero(pCreateInfo->extent.depth == 0, "VUID-VkImageCreateInfo-extent-00946",
                                    create_info_loc.dot(Field::extent).dot(Field::depth));

    skip |= context.ValidateNotZero(pCreateInfo->mipLevels == 0, "VUID-VkImageCreateInfo-mipLevels-00947",
                                    create_info_loc.dot(Field::mipLevels));
    skip |= context.ValidateNotZero(pCreateInfo->arrayLayers == 0, "VUID-VkImageCreateInfo-arrayLayers-00948",
                                    create_info_loc.dot(Field::arrayLayers));

    // InitialLayout must be PREINITIALIZED or UNDEFINED
    if ((pCreateInfo->initialLayout != VK_IMAGE_LAYOUT_UNDEFINED) &&
        (pCreateInfo->initialLayout != VK_IMAGE_LAYOUT_PREINITIALIZED)) {
        skip |= LogError("VUID-VkImageCreateInfo-initialLayout-00993", device, create_info_loc.dot(Field::initialLayout),
                         "is %s, but must be UNDEFINED or PREINITIALIZED.", string_VkImageLayout(pCreateInfo->initialLayout));
    }

    if ((pCreateInfo->imageType == VK_IMAGE_TYPE_1D) && ((pCreateInfo->extent.height != 1) || (pCreateInfo->extent.depth != 1))) {
        skip |= LogError("VUID-VkImageCreateInfo-imageType-00956", device, create_info_loc.dot(Field::imageType),
                         "is VK_IMAGE_TYPE_1D but extent.height (%" PRIu32 ") and extent.depth (%" PRIu32 ") must both be 1.",
                         pCreateInfo->extent.height, pCreateInfo->extent.depth);
    }

    if (pCreateInfo->flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) {
        if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-00949", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT but imageType (%s) is not VK_IMAGE_TYPE_2D.",
                             string_VkImageType(pCreateInfo->imageType));
        }

        if (pCreateInfo->extent.width != pCreateInfo->extent.height) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-08865", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT but extent.width (%" PRIu32
                             ") is not equal to extent.height (%" PRIu32 ").",
                             pCreateInfo->extent.width, pCreateInfo->extent.height);
        }

        if (pCreateInfo->arrayLayers < 6) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-08866", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT but arrayLayers (%" PRIu32 ") is less than 6.",
                             pCreateInfo->arrayLayers);
        }
    }

    if (pCreateInfo->imageType == VK_IMAGE_TYPE_2D) {
        if (pCreateInfo->extent.depth != 1) {
            skip |= LogError("VUID-VkImageCreateInfo-imageType-00957", device, create_info_loc.dot(Field::imageType),
                             "is VK_IMAGE_TYPE_2D but extent.depth (%" PRIu32 ") must be 1.", pCreateInfo->extent.depth);
        }
    }

    // 3D image may have only 1 layer
    if ((pCreateInfo->imageType == VK_IMAGE_TYPE_3D) && (pCreateInfo->arrayLayers != 1)) {
        skip |= LogError("VUID-VkImageCreateInfo-imageType-00961", device, create_info_loc.dot(Field::imageType),
                         "is VK_IMAGE_TYPE_3D but arrayLayers (%" PRIu32 ") must be 1.", pCreateInfo->arrayLayers);
    }

    if (0 != (pCreateInfo->usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT)) {
        VkImageUsageFlags legal_flags = (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                         VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
        // At least one of the legal attachment bits must be set
        if (0 == (pCreateInfo->usage & legal_flags)) {
            skip |= LogError("VUID-VkImageCreateInfo-usage-00966", device, create_info_loc.dot(Field::usage),
                             "(%s) includes VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT but is missing one of %s.",
                             string_VkImageUsageFlags(pCreateInfo->usage).c_str(), string_VkImageUsageFlags(legal_flags).c_str());
        }
        // No flags other than the legal attachment bits may be set
        legal_flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        if (0 != (pCreateInfo->usage & ~legal_flags)) {
            skip |= LogError("VUID-VkImageCreateInfo-usage-00963", device, create_info_loc,
                             "(%s) includes VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT so it can't include %s.",
                             string_VkImageUsageFlags(pCreateInfo->usage).c_str(),
                             string_VkImageUsageFlags(pCreateInfo->usage & ~legal_flags).c_str());
        }
    }

    const VkImageCreateFlags image_flags = pCreateInfo->flags;
    // mipLevels must be less than or equal to the number of levels in the complete mipmap chain
    uint32_t max_dim = std::max(std::max(pCreateInfo->extent.width, pCreateInfo->extent.height), pCreateInfo->extent.depth);
    // Max mip levels is different for corner-sampled images vs normal images.
    uint32_t max_mip_levels = (image_flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV)
                                  ? static_cast<uint32_t>(ceil(log2(max_dim)))
                                  : static_cast<uint32_t>(floor(log2(max_dim)) + 1);
    if (max_dim > 0 && pCreateInfo->mipLevels > max_mip_levels) {
        skip |= LogError("VUID-VkImageCreateInfo-mipLevels-00958", device, create_info_loc.dot(Field::mipLevels),
                         "(%" PRIu32 ") must be less than or equal to %" PRIu32 " (based on extent of %s)", pCreateInfo->mipLevels,
                         max_mip_levels, string_VkExtent3D(pCreateInfo->extent).c_str());
    }

    if ((image_flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT) && (pCreateInfo->imageType != VK_IMAGE_TYPE_3D)) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-00950", device, create_info_loc.dot(Field::flags),
                         "includes VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT but "
                         "imageType is %s.",
                         string_VkImageType(pCreateInfo->imageType));
    }

    if ((image_flags & VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT) && (pCreateInfo->imageType != VK_IMAGE_TYPE_3D)) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-07755", device, create_info_loc.dot(Field::flags),
                         "includes VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT but "
                         "imageType is %s.",
                         string_VkImageType(pCreateInfo->imageType));
    }

    skip |= ValidateCreateImageSparse(*pCreateInfo, create_info_loc);
    skip |= ValidateCreateImageFragmentShadingRate(*pCreateInfo, create_info_loc);
    skip |= ValidateCreateImageCornerSampled(*pCreateInfo, create_info_loc);
    skip |= ValidateCreateImageStencilUsage(*pCreateInfo, create_info_loc);
    skip |= ValidateCreateImageCompressionControl(context, *pCreateInfo, create_info_loc);
    skip |= ValidateCreateImageSwapchain(*pCreateInfo, create_info_loc);
    skip |= ValidateCreateImageFormatList(*pCreateInfo, create_info_loc);
    skip |= ValidateCreateImageMetalObject(*pCreateInfo, create_info_loc);

    std::vector<uint64_t> image_create_drm_format_modifiers;
    skip |= ValidateCreateImageDrmFormatModifiers(*pCreateInfo, create_info_loc, image_create_drm_format_modifiers);

    const VkFormat image_format = pCreateInfo->format;
    if (((image_flags & VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT) != 0) &&
        (vkuFormatHasDepth(image_format) == false)) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-01533", device, create_info_loc.dot(Field::flags),
                         "includes VK_IMAGE_CREATE_SAMPLE_LOCATIONS_COMPATIBLE_DEPTH_BIT_EXT but the "
                         "format (%s) must be a depth or depth/stencil format.",
                         string_VkFormat(image_format));
    }

    if ((!enabled_features.shaderStorageImageMultisample) && ((pCreateInfo->usage & VK_IMAGE_USAGE_STORAGE_BIT) != 0) &&
        (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT)) {
        skip |= LogError("VUID-VkImageCreateInfo-usage-00968", device, create_info_loc.dot(Field::usage),
                         "includes VK_IMAGE_USAGE_STORAGE_BIT and imageType is %s, but shaderStorageImageMultisample feature "
                         "was not enabled.",
                         string_VkSampleCountFlagBits(pCreateInfo->samples));
    }

    if (!enabled_features.hostImageCopy && (pCreateInfo->usage & VK_IMAGE_USAGE_HOST_TRANSFER_BIT) != 0) {
        skip |= LogError("VUID-VkImageCreateInfo-usage-10245", device, create_info_loc.dot(Field::usage),
                         "includes VK_IMAGE_USAGE_HOST_TRANSFER_BIT, but hostImageCopy feature was not enabled.");
    }

    static const uint64_t drm_format_mod_linear = 0;
    bool image_create_maybe_linear = false;
    if (pCreateInfo->tiling == VK_IMAGE_TILING_LINEAR) {
        image_create_maybe_linear = true;
    } else if (pCreateInfo->tiling == VK_IMAGE_TILING_OPTIMAL) {
        image_create_maybe_linear = false;
    } else if (pCreateInfo->tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        image_create_maybe_linear = (std::find(image_create_drm_format_modifiers.begin(), image_create_drm_format_modifiers.end(),
                                               drm_format_mod_linear) != image_create_drm_format_modifiers.end());
    }

    // If multi-sample, validate type, usage, tiling and mip levels.
    if ((pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) &&
        ((pCreateInfo->imageType != VK_IMAGE_TYPE_2D) || (image_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) ||
         (pCreateInfo->mipLevels != 1) || image_create_maybe_linear)) {
        skip |= LogError("VUID-VkImageCreateInfo-samples-02257", device, create_info_loc,
                         "image created with\n"
                         "samples = %s\n"
                         "imageType = %s\n"
                         "flags = %s (contains VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)\n"
                         "mipLevels = %" PRIu32
                         "\n"
                         "which is not valid.",
                         string_VkSampleCountFlagBits(pCreateInfo->samples), string_VkImageType(pCreateInfo->imageType),
                         string_VkImageCreateFlags(image_flags).c_str(), pCreateInfo->mipLevels);
    }

    if ((image_flags & VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT) &&
        ((pCreateInfo->mipLevels != 1) || (pCreateInfo->arrayLayers != 1) || (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) ||
         image_create_maybe_linear)) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-02259", device, create_info_loc,
                         "image created with\n"
                         "imageType = %s\n"
                         "flags = %s (contains VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT)\n"
                         "arrayLayers = %" PRIu32
                         "\n"
                         "mipLevels = %" PRIu32
                         "\n"
                         "which is not valid.",
                         string_VkImageType(pCreateInfo->imageType), string_VkImageCreateFlags(image_flags).c_str(),
                         pCreateInfo->arrayLayers, pCreateInfo->mipLevels);
    }

    if (pCreateInfo->usage & VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT) {
        if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-02557", device, create_info_loc.dot(Field::usage),
                             "includes VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT, but imageType is %s.",
                             string_VkImageType(pCreateInfo->imageType));
        }
        if (pCreateInfo->samples != VK_SAMPLE_COUNT_1_BIT) {
            skip |= LogError("VUID-VkImageCreateInfo-samples-02558", device, create_info_loc.dot(Field::usage),
                             "includes VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT, but samples is %s.",
                             string_VkSampleCountFlagBits(pCreateInfo->samples));
        }
    }
    if (image_flags & VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT) {
        if (pCreateInfo->tiling != VK_IMAGE_TILING_OPTIMAL) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-02565", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT, but tiling is %s.",
                             string_VkImageTiling(pCreateInfo->tiling));
        }
        if (pCreateInfo->imageType != VK_IMAGE_TYPE_2D) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-02566", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT, but imageType is %s.",
                             string_VkImageType(pCreateInfo->imageType));
        }
        if (image_flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-02567", device, create_info_loc.dot(Field::flags),
                             "(%s) contains both VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT and VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT.",
                             string_VkImageCreateFlags(image_flags).c_str());
        }
        if (pCreateInfo->mipLevels != 1) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-02568", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_SUBSAMPLED_BIT_EXT, but mipLevels is %" PRIu32 ".", pCreateInfo->mipLevels);
        }
    }

    // If Chroma subsampled format ( _420_ or _422_ )
    if (vkuFormatIsXChromaSubsampled(image_format) && (SafeModulo(pCreateInfo->extent.width, 2) != 0)) {
        skip |=
            LogError("VUID-VkImageCreateInfo-format-04712", device, create_info_loc.dot(Field::format),
                     "(%s) is X Chroma Subsampled (has _422 or _420 suffix) so the width (%" PRIu32 ") must be a multiple of 2.",
                     string_VkFormat(image_format), pCreateInfo->extent.width);
    }
    if (vkuFormatIsYChromaSubsampled(image_format) && (SafeModulo(pCreateInfo->extent.height, 2) != 0)) {
        skip |= LogError("VUID-VkImageCreateInfo-format-04713", device, create_info_loc.dot(Field::format),
                         "(%s) is Y Chroma Subsampled (has _420 suffix) so the height (%" PRIu32 ") must be a multiple of 2.",
                         string_VkFormat(image_format), pCreateInfo->extent.height);
    }

    return skip;
}

bool Device::ValidateCreateImageSparse(const VkImageCreateInfo &create_info, const Location &create_info_loc) const {
    bool skip = false;
    const VkImageCreateFlags image_flags = create_info.flags;
    const VkImageCreateFlags sparse_flags =
        VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_ALIASED_BIT;
    const bool has_sparse_flags = (image_flags & sparse_flags) != 0;

    if (has_sparse_flags) {
        if (create_info.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) {
            skip |= LogError("VUID-VkImageCreateInfo-None-01925", device, create_info_loc,
                             "images using sparse memory cannot have VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT set. (image flags %s)",
                             string_VkImageCreateFlags(image_flags).c_str());
        }
        if (image_flags & VK_IMAGE_CREATE_2D_VIEW_COMPATIBLE_BIT_EXT) {
            skip |= LogError("VUID-VkImageCreateInfo-imageType-10197", device, create_info_loc.dot(Field::flags), "is %s.",
                             string_VkImageCreateFlags(image_flags).c_str());
        }
        if (image_flags & VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT) {
            skip |= LogError("VUID-VkImageCreateInfo-flags-09403", device, create_info_loc.dot(Field::flags), "is %s.",
                             string_VkImageCreateFlags(image_flags).c_str());
        }
    }

    if ((image_flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) && (!enabled_features.sparseBinding)) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-00969", device, create_info_loc.dot(Field::flags),
                         "includes VK_IMAGE_CREATE_SPARSE_BINDING_BIT, but the "
                         "sparseBinding feature was not enabled.");
    }

    if ((image_flags & VK_IMAGE_CREATE_SPARSE_ALIASED_BIT) && (!enabled_features.sparseResidencyAliased)) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-01924", device, create_info_loc.dot(Field::flags),
                         "includes VK_IMAGE_CREATE_SPARSE_ALIASED_BIT but the sparseResidencyAliased feature was not enabled.");
    }

    // If flags contains VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT or VK_IMAGE_CREATE_SPARSE_ALIASED_BIT, it must also contain
    // VK_IMAGE_CREATE_SPARSE_BINDING_BIT
    if (((image_flags & (VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT | VK_IMAGE_CREATE_SPARSE_ALIASED_BIT)) != 0) &&
        ((image_flags & VK_IMAGE_CREATE_SPARSE_BINDING_BIT) != VK_IMAGE_CREATE_SPARSE_BINDING_BIT)) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-00987", device, create_info_loc.dot(Field::flags), "is %s.",
                         string_VkImageCreateFlags(image_flags).c_str());
    }

    // Check for combinations of attributes that are incompatible with having VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT set
    if ((image_flags & VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT) != 0) {
        // Linear tiling is unsupported
        if (VK_IMAGE_TILING_LINEAR == create_info.tiling) {
            skip |= LogError("VUID-VkImageCreateInfo-tiling-04121", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT but tiling is VK_IMAGE_TILING_LINEAR.");
        }

        // Sparse 1D image isn't valid
        if (VK_IMAGE_TYPE_1D == create_info.imageType) {
            skip |= LogError("VUID-VkImageCreateInfo-imageType-00970", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_SPARSE_BINDING_BIT but imageType is VK_IMAGE_TYPE_1D.");
        }

        // Sparse 2D image when device doesn't support it
        if ((!enabled_features.sparseResidencyImage2D) && (VK_IMAGE_TYPE_2D == create_info.imageType)) {
            skip |= LogError("VUID-VkImageCreateInfo-imageType-00971", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_SPARSE_BINDING_BIT and imageType is VK_IMAGE_TYPE_2D, but "
                             "sparseResidencyImage2D feature was not enabled.");
        }

        // Sparse 3D image when device doesn't support it
        if ((!enabled_features.sparseResidencyImage3D) && (VK_IMAGE_TYPE_3D == create_info.imageType)) {
            skip |= LogError("VUID-VkImageCreateInfo-imageType-00972", device, create_info_loc.dot(Field::flags),
                             "includes VK_IMAGE_CREATE_SPARSE_BINDING_BIT and imageType is VK_IMAGE_TYPE_3D, but "
                             "sparseResidencyImage3D feature was not enabled.");
        }

        // Multi-sample 2D image when device doesn't support it
        if (VK_IMAGE_TYPE_2D == create_info.imageType) {
            if ((!enabled_features.sparseResidency2Samples) && (VK_SAMPLE_COUNT_2_BIT == create_info.samples)) {
                skip |= LogError("VUID-VkImageCreateInfo-imageType-00973", device, create_info_loc.dot(Field::flags),
                                 "includes VK_IMAGE_CREATE_SPARSE_BINDING_BIT and imageType is VK_IMAGE_TYPE_2D and samples is "
                                 "VK_SAMPLE_COUNT_2_BIT, but sparseResidency2Samples feature was not enabled.");
            } else if ((!enabled_features.sparseResidency4Samples) && (VK_SAMPLE_COUNT_4_BIT == create_info.samples)) {
                skip |= LogError("VUID-VkImageCreateInfo-imageType-00974", device, create_info_loc.dot(Field::flags),
                                 "includes VK_IMAGE_CREATE_SPARSE_BINDING_BIT and imageType is VK_IMAGE_TYPE_2D and samples is "
                                 "VK_SAMPLE_COUNT_4_BIT, but sparseResidency4Samples feature was not enabled.");
            } else if ((!enabled_features.sparseResidency8Samples) && (VK_SAMPLE_COUNT_8_BIT == create_info.samples)) {
                skip |= LogError("VUID-VkImageCreateInfo-imageType-00975", device, create_info_loc.dot(Field::flags),
                                 "includes VK_IMAGE_CREATE_SPARSE_BINDING_BIT and imageType is VK_IMAGE_TYPE_2D and samples is "
                                 "VK_SAMPLE_COUNT_8_BIT, but sparseResidency8Samples feature was not enabled.");
            } else if ((!enabled_features.sparseResidency16Samples) && (VK_SAMPLE_COUNT_16_BIT == create_info.samples)) {
                skip |= LogError("VUID-VkImageCreateInfo-imageType-00976", device, create_info_loc.dot(Field::flags),
                                 "includes VK_IMAGE_CREATE_SPARSE_BINDING_BIT and imageType is VK_IMAGE_TYPE_2D and samples is "
                                 "VK_SAMPLE_COUNT_16_BIT, but sparseResidency16Samples feature was not enabled.");
            }
        }
    }
    return skip;
}

bool Device::ValidateCreateImageFragmentShadingRate(const VkImageCreateInfo &create_info, const Location &create_info_loc) const {
    bool skip = false;
    // alias VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV
    if ((create_info.usage & VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR) == 0) return skip;

    if (create_info.imageType != VK_IMAGE_TYPE_2D) {
        skip |= LogError("VUID-VkImageCreateInfo-imageType-02082", device, create_info_loc.dot(Field::usage),
                         "includes VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR (or the "
                         "alias VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV), but imageType is %s.",
                         string_VkImageType(create_info.imageType));
    }
    if (create_info.samples != VK_SAMPLE_COUNT_1_BIT) {
        skip |= LogError("VUID-VkImageCreateInfo-samples-02083", device, create_info_loc.dot(Field::usage),
                         "includes VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR (or the "
                         "alias VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV), but samples is %s.",
                         string_VkSampleCountFlagBits(create_info.samples));
    }
    if (enabled_features.shadingRateImage && create_info.tiling != VK_IMAGE_TILING_OPTIMAL) {
        // KHR flag can be non-optimal
        skip |= LogError("VUID-VkImageCreateInfo-shadingRateImage-07727", device, create_info_loc.dot(Field::usage),
                         "includes VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV, tiling must be "
                         "VK_IMAGE_TILING_OPTIMAL.");
    }
    return skip;
}

bool Device::ValidateCreateImageCornerSampled(const VkImageCreateInfo &create_info, const Location &create_info_loc) const {
    bool skip = false;
    if ((create_info.flags & VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV) == 0) return skip;

    if (create_info.imageType != VK_IMAGE_TYPE_2D && create_info.imageType != VK_IMAGE_TYPE_3D) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-02050", device, create_info_loc.dot(Field::flags),
                         "includes VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV, "
                         "but imageType is %s.",
                         string_VkImageType(create_info.imageType));
    }

    if ((create_info.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT) || vkuFormatIsDepthOrStencil(create_info.format)) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-02051", device, create_info_loc.dot(Field::flags),
                         "includes VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV, "
                         "it must not also contain VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT and format (%s) must not be a "
                         "depth/stencil format.",
                         string_VkFormat(create_info.format));
    }

    if (create_info.imageType == VK_IMAGE_TYPE_2D && (create_info.extent.width == 1 || create_info.extent.height == 1)) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-02052", device, create_info_loc.dot(Field::flags),
                         "includes VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV and "
                         "imageType is VK_IMAGE_TYPE_2D, extent.width and extent.height must be "
                         "greater than 1.");
    } else if (create_info.imageType == VK_IMAGE_TYPE_3D &&
               (create_info.extent.width == 1 || create_info.extent.height == 1 || create_info.extent.depth == 1)) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-02053", device, create_info_loc.dot(Field::flags),
                         "includes VK_IMAGE_CREATE_CORNER_SAMPLED_BIT_NV and "
                         "imageType is VK_IMAGE_TYPE_3D, extent.width, extent.height, and extent.depth "
                         "must be greater than 1.");
    }
    return skip;
}

bool Device::ValidateCreateImageStencilUsage(const VkImageCreateInfo &create_info, const Location &create_info_loc) const {
    bool skip = false;
    const auto image_stencil_struct = vku::FindStructInPNextChain<VkImageStencilUsageCreateInfo>(create_info.pNext);
    if (!image_stencil_struct) return skip;

    if ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0) {
        VkImageUsageFlags legal_flags = (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
        // No flags other than the legal attachment bits may be set
        legal_flags |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        if ((image_stencil_struct->stencilUsage & ~legal_flags) != 0) {
            skip |= LogError("VUID-VkImageStencilUsageCreateInfo-stencilUsage-02539", device,
                             create_info_loc.pNext(Struct::VkImageStencilUsageCreateInfo, Field::stencilUsage), "is %s.",
                             string_VkImageUsageFlags(image_stencil_struct->stencilUsage).c_str());
        }
    }

    if (vkuFormatIsDepthOrStencil(create_info.format)) {
        if ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT) != 0) {
            if (create_info.extent.width > device_limits.maxFramebufferWidth) {
                skip |= LogError("VUID-VkImageCreateInfo-Format-02536", device,
                                 create_info_loc.pNext(Struct::VkImageStencilUsageCreateInfo, Field::stencilUsage),
                                 "includes VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT and image width (%" PRIu32
                                 ") exceeds device "
                                 "maxFramebufferWidth (%" PRIu32 ")",
                                 create_info.extent.width, device_limits.maxFramebufferWidth);
            }

            if (create_info.extent.height > device_limits.maxFramebufferHeight) {
                skip |= LogError("VUID-VkImageCreateInfo-format-02537", device,
                                 create_info_loc.pNext(Struct::VkImageStencilUsageCreateInfo, Field::stencilUsage),
                                 "includes VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT and image height (%" PRIu32
                                 ") exceeds device "
                                 "maxFramebufferHeight (%" PRIu32 ")",
                                 create_info.extent.height, device_limits.maxFramebufferHeight);
            }
        }

        if (!enabled_features.shaderStorageImageMultisample &&
            ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_STORAGE_BIT) != 0) &&
            (create_info.samples != VK_SAMPLE_COUNT_1_BIT)) {
            skip |= LogError("VUID-VkImageCreateInfo-format-02538", device,
                             create_info_loc.pNext(Struct::VkImageStencilUsageCreateInfo, Field::stencilUsage),
                             "includes VK_IMAGE_USAGE_STORAGE_BIT and format is %s and samples is %s, but "
                             "shaderStorageImageMultisample feature was not enabled.",
                             string_VkFormat(create_info.format), string_VkSampleCountFlagBits(create_info.samples));
        }

        if (((create_info.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0) &&
            ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0)) {
            skip |= LogError("VUID-VkImageCreateInfo-format-02795", device, create_info_loc.dot(Field::usage),
                             "is (%s), format is %s, and %s is %s", string_VkImageUsageFlags(create_info.usage).c_str(),
                             string_VkFormat(create_info.format),
                             create_info_loc.pNext(Struct::VkImageStencilUsageCreateInfo, Field::stencilUsage).Fields().c_str(),
                             string_VkImageUsageFlags(image_stencil_struct->stencilUsage).c_str());
        } else if (((create_info.usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) == 0) &&
                   ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0)) {
            skip |= LogError("VUID-VkImageCreateInfo-format-02796", device, create_info_loc.dot(Field::usage),
                             "is (%s), format is %s, and %s is %s", string_VkImageUsageFlags(create_info.usage).c_str(),
                             string_VkFormat(create_info.format),
                             create_info_loc.pNext(Struct::VkImageStencilUsageCreateInfo, Field::stencilUsage).Fields().c_str(),
                             string_VkImageUsageFlags(image_stencil_struct->stencilUsage).c_str());
        }

        if (((create_info.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0) &&
            ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) == 0)) {
            skip |= LogError("VUID-VkImageCreateInfo-format-02797", device, create_info_loc.dot(Field::usage),
                             "is (%s), format is %s, and %s is %s", string_VkImageUsageFlags(create_info.usage).c_str(),
                             string_VkFormat(create_info.format),
                             create_info_loc.pNext(Struct::VkImageStencilUsageCreateInfo, Field::stencilUsage).Fields().c_str(),
                             string_VkImageUsageFlags(image_stencil_struct->stencilUsage).c_str());
        } else if (((create_info.usage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) == 0) &&
                   ((image_stencil_struct->stencilUsage & VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT) != 0)) {
            skip |= LogError("VUID-VkImageCreateInfo-format-02798", device, create_info_loc.dot(Field::usage),
                             "is (%s), format is %s, and %s is %s", string_VkImageUsageFlags(create_info.usage).c_str(),
                             string_VkFormat(create_info.format),
                             create_info_loc.pNext(Struct::VkImageStencilUsageCreateInfo, Field::stencilUsage).Fields().c_str(),
                             string_VkImageUsageFlags(image_stencil_struct->stencilUsage).c_str());
        }
    }

    return skip;
}

bool Device::ValidateCreateImageCompressionControl(const Context &context, const VkImageCreateInfo &create_info,
                                                   const Location &create_info_loc) const {
    bool skip = false;
    const auto image_compression_control = vku::FindStructInPNextChain<VkImageCompressionControlEXT>(create_info.pNext);
    if (!image_compression_control) return skip;

    skip |= context.ValidateFlags(create_info_loc.pNext(Struct::VkImageCompressionControlEXT, Field::flags),
                                  vvl::FlagBitmask::VkImageCompressionFlagBitsEXT, AllVkImageCompressionFlagBitsEXT,
                                  image_compression_control->flags, kOptionalSingleBit,
                                  "VUID-VkImageCompressionControlEXT-flags-06747");

    if (image_compression_control->flags == VK_IMAGE_COMPRESSION_FIXED_RATE_EXPLICIT_EXT &&
        !image_compression_control->pFixedRateFlags) {
        skip |= LogError("VUID-VkImageCompressionControlEXT-flags-06748", device,
                         create_info_loc.pNext(Struct::VkImageCompressionControlEXT, Field::flags),
                         "is %s, but pFixedRateFlags is NULL.",
                         string_VkImageCompressionFlagsEXT(image_compression_control->flags).c_str());
    }

    return skip;
}

bool Device::ValidateCreateImageSwapchain(const VkImageCreateInfo &create_info, const Location &create_info_loc) const {
    bool skip = false;
    const auto swapchain_create_info = vku::FindStructInPNextChain<VkImageSwapchainCreateInfoKHR>(create_info.pNext);
    if (!swapchain_create_info || swapchain_create_info->swapchain == VK_NULL_HANDLE) return skip;

    // All the following fall under the same VU that checks that the swapchain image uses parameters listed in the
    // #swapchain-wsi-image-create-info table. Breaking up into multiple checks allows for more useful information
    // to be returned when this error occurs. Check for matching Swapchain flags is done later in state tracking validation
    const char *vuid = "VUID-VkImageSwapchainCreateInfoKHR-swapchain-00995";
    const Location swapchain_loc = create_info_loc.pNext(Struct::VkImageSwapchainCreateInfoKHR, Field::swapchain);

    if (create_info.imageType != VK_IMAGE_TYPE_2D) {
        // also implicitly forces the check above that extent.depth is 1
        skip |= LogError(vuid, swapchain_create_info->swapchain, swapchain_loc,
                         "is not NULL, but imageType (%s) is not VK_IMAGE_TYPE_2D.", string_VkImageType(create_info.imageType));
    }
    if (create_info.mipLevels != 1) {
        skip |= LogError(vuid, swapchain_create_info->swapchain, swapchain_loc,
                         "is not NULL, but mipLevels (%" PRIu32 ") is not 1.", create_info.mipLevels);
    }
    if (create_info.samples != VK_SAMPLE_COUNT_1_BIT) {
        skip |= LogError(vuid, swapchain_create_info->swapchain, swapchain_loc,
                         "is not NULL, but samples (%s) is not VK_SAMPLE_COUNT_1_BIT.",
                         string_VkSampleCountFlagBits(create_info.samples));
    }
    if (create_info.tiling != VK_IMAGE_TILING_OPTIMAL) {
        skip |= LogError(vuid, swapchain_create_info->swapchain, swapchain_loc,
                         "is not NULL, but tiling (%s) is not VK_IMAGE_TILING_OPTIMAL.", string_VkImageTiling(create_info.tiling));
    }
    if (create_info.initialLayout != VK_IMAGE_LAYOUT_UNDEFINED) {
        skip |= LogError(vuid, swapchain_create_info->swapchain, swapchain_loc,
                         "is not NULL, but initialLayout (%s) is not VK_IMAGE_LAYOUT_UNDEFINED.",
                         string_VkImageLayout(create_info.initialLayout));
    }
    const VkImageCreateFlags valid_flags = (VK_IMAGE_CREATE_SPLIT_INSTANCE_BIND_REGIONS_BIT | VK_IMAGE_CREATE_PROTECTED_BIT |
                                            VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT | VK_IMAGE_CREATE_EXTENDED_USAGE_BIT);
    if ((create_info.flags & ~valid_flags) != 0) {
        skip |= LogError(vuid, swapchain_create_info->swapchain, swapchain_loc,
                         "is not NULL, but flags %s must only have valid flags (%s).",
                         string_VkImageCreateFlags(create_info.flags).c_str(), string_VkImageCreateFlags(valid_flags).c_str());
    }

    return skip;
}

bool Device::ValidateCreateImageFormatList(const VkImageCreateInfo &create_info, const Location &create_info_loc) const {
    bool skip = false;
    const auto format_list_info = vku::FindStructInPNextChain<VkImageFormatListCreateInfo>(create_info.pNext);
    if (!format_list_info) return skip;

    const VkImageCreateFlags image_flags = create_info.flags;
    const uint32_t view_format_count = format_list_info->viewFormatCount;
    const bool mutable_image = (image_flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) != 0;
    if (!mutable_image && view_format_count > 1) {
        skip |= LogError("VUID-VkImageCreateInfo-flags-04738", device,
                         create_info_loc.pNext(Struct::VkImageFormatListCreateInfo, Field::viewFormatCount),
                         "is %" PRIu32 " but flag (%s) does not include VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT.", view_format_count,
                         string_VkImageCreateFlags(image_flags).c_str());
    }

    // Check if viewFormatCount is not zero that it is all compatible
    const VkFormat image_format = create_info.format;
    const auto image_format_class = vkuFormatCompatibilityClass(image_format);
    for (uint32_t i = 0; i < view_format_count; i++) {
        const VkFormat view_format = format_list_info->pViewFormats[i];
        const Location format_loc = create_info_loc.pNext(Struct::VkImageFormatListCreateInfo, Field::pViewFormats, i);
        const auto view_format_class = vkuFormatCompatibilityClass(view_format);

        if (view_format == VK_FORMAT_UNDEFINED) {
            skip |=
                LogError("VUID-VkImageFormatListCreateInfo-viewFormatCount-09540", device, format_loc, "is VK_FORMAT_UNDEFINED.");
        } else if (vkuFormatIsMultiplane(image_format)) {
            if (vkuFormatIsMultiplane(view_format)) {
                // TODO - need VU to say these need to be the same
                // https://gitlab.khronos.org/vulkan/vulkan/-/merge_requests/6967
            } else if (mutable_image) {
                // Need to make sure it is compatible with any possible planes because we don't know the apsectMask yet
                const VkFormat plane_0_format = vkuFindMultiplaneCompatibleFormat(image_format, VK_IMAGE_ASPECT_PLANE_0_BIT);
                const VkFormat plane_1_format = vkuFindMultiplaneCompatibleFormat(image_format, VK_IMAGE_ASPECT_PLANE_1_BIT);
                const VkFormat plane_2_format = vkuFindMultiplaneCompatibleFormat(image_format, VK_IMAGE_ASPECT_PLANE_2_BIT);
                const uint32_t plane_count = vkuFormatPlaneCount(image_format);
                bool found_compatible = false;
                if (view_format_class == vkuFormatCompatibilityClass(plane_0_format)) {
                    found_compatible = true;
                } else if ((plane_count > 1) && view_format_class == vkuFormatCompatibilityClass(plane_1_format)) {
                    found_compatible = true;
                } else if ((plane_count > 2) && view_format_class == vkuFormatCompatibilityClass(plane_2_format)) {
                    found_compatible = true;
                }
                if (!found_compatible) {
                    std::stringstream ss;
                    ss << "Plane 0 " << string_VkFormat(plane_0_format);
                    if (plane_count > 1) {
                        ss << "\nPlane 1 " << string_VkFormat(plane_1_format);
                    }
                    if (plane_count > 2) {
                        ss << "\nPlane 2 " << string_VkFormat(plane_2_format);
                    }
                    skip |= LogError("VUID-VkImageCreateInfo-pNext-10062", device, format_loc,
                                     "(%s) is not compatible with any plane of VkImageCreateInfo::format (%s)\n%s.",
                                     string_VkFormat(view_format), string_VkFormat(image_format), ss.str().c_str());
                }
            }
        } else if (view_format_class != image_format_class) {
            if (image_flags & VK_IMAGE_CREATE_BLOCK_TEXEL_VIEW_COMPATIBLE_BIT) {
                if (!AreFormatsSizeCompatible(view_format, image_format)) {
                    skip |= LogError("VUID-VkImageCreateInfo-pNext-06722", device, format_loc,
                                     "(%s) and VkImageCreateInfo::format (%s) are not class compatible or size-compatible. %s",
                                     string_VkFormat(view_format), string_VkFormat(image_format),
                                     DescribeFormatsSizeCompatible(view_format, image_format).c_str());
                }
            } else {
                skip |= LogError("VUID-VkImageCreateInfo-pNext-06722", device, format_loc,
                                 "(%s) and VkImageCreateInfo::format (%s) are not class compatible.", string_VkFormat(view_format),
                                 string_VkFormat(image_format));
            }
        }
    }

    return skip;
}

bool Device::ValidateCreateImageMetalObject(const VkImageCreateInfo &create_info, const Location &create_info_loc) const {
    bool skip = false;
#ifdef VK_USE_PLATFORM_METAL_EXT
    auto export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(create_info.pNext);
    while (export_metal_object_info) {
        if ((export_metal_object_info->exportObjectType != VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT) &&
            (export_metal_object_info->exportObjectType != VK_EXPORT_METAL_OBJECT_TYPE_METAL_IOSURFACE_BIT_EXT)) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-06783", device,
                             create_info_loc.pNext(Struct::VkExportMetalObjectCreateInfoEXT, Field::exportObjectType),
                             "is %s, but only VkExportMetalObjectCreateInfoEXT structs with exportObjectType of "
                             "VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT or "
                             "VK_EXPORT_METAL_OBJECT_TYPE_METAL_IOSURFACE_BIT_EXT are allowed",
                             string_VkExportMetalObjectTypeFlagBitsEXT(export_metal_object_info->exportObjectType));
        }
        export_metal_object_info = vku::FindStructInPNextChain<VkExportMetalObjectCreateInfoEXT>(export_metal_object_info->pNext);
    }
    auto import_metal_texture_info = vku::FindStructInPNextChain<VkImportMetalTextureInfoEXT>(create_info.pNext);
    while (import_metal_texture_info) {
        const Location texture_info_loc = create_info_loc.pNext(Struct::VkImportMetalTextureInfoEXT, Field::plane);
        if ((import_metal_texture_info->plane != VK_IMAGE_ASPECT_PLANE_0_BIT) &&
            (import_metal_texture_info->plane != VK_IMAGE_ASPECT_PLANE_1_BIT) &&
            (import_metal_texture_info->plane != VK_IMAGE_ASPECT_PLANE_2_BIT)) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-06784", device, texture_info_loc,
                             "is %s, but only VK_IMAGE_ASPECT_PLANE_0_BIT, VK_IMAGE_ASPECT_PLANE_1_BIT, or "
                             "VK_IMAGE_ASPECT_PLANE_2_BIT are allowed",
                             string_VkImageAspectFlags(import_metal_texture_info->plane).c_str());
        }
        auto format_plane_count = vkuFormatPlaneCount(create_info.format);
        if ((format_plane_count <= 1) && (import_metal_texture_info->plane != VK_IMAGE_ASPECT_PLANE_0_BIT)) {
            skip |=
                LogError("VUID-VkImageCreateInfo-pNext-06785", device, texture_info_loc,
                         "is %s, but only VK_IMAGE_ASPECT_PLANE_0_BIT is allowed for an image created with format %s, "
                         "which is not multiplaner",
                         string_VkImageAspectFlags(import_metal_texture_info->plane).c_str(), string_VkFormat(create_info.format));
        }
        if ((format_plane_count == 2) && (import_metal_texture_info->plane == VK_IMAGE_ASPECT_PLANE_2_BIT)) {
            skip |= LogError("VUID-VkImageCreateInfo-pNext-06786", device, texture_info_loc,
                             "is VK_IMAGE_ASPECT_PLANE_2_BIT, which is not allowed for an image created with format %s, "
                             "which has only 2 planes",
                             string_VkFormat(create_info.format));
        }
        import_metal_texture_info = vku::FindStructInPNextChain<VkImportMetalTextureInfoEXT>(import_metal_texture_info->pNext);
    }
#endif  // VK_USE_PLATFORM_METAL_EXT
    return skip;
}

bool Device::ValidateCreateImageDrmFormatModifiers(const VkImageCreateInfo &create_info, const Location &create_info_loc,
                                                   std::vector<uint64_t> &image_create_drm_format_modifiers) const {
    bool skip = false;
    if (!IsExtEnabled(extensions.vk_ext_image_drm_format_modifier)) return skip;

    const auto drm_format_mod_list = vku::FindStructInPNextChain<VkImageDrmFormatModifierListCreateInfoEXT>(create_info.pNext);
    const auto drm_format_mod_explict =
        vku::FindStructInPNextChain<VkImageDrmFormatModifierExplicitCreateInfoEXT>(create_info.pNext);
    if (create_info.tiling == VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT) {
        if ((!drm_format_mod_list) && (!drm_format_mod_explict)) {
            skip |= LogError("VUID-VkImageCreateInfo-tiling-02261", device, create_info_loc.dot(Field::tiling),
                             "is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT but pNext is missing "
                             "VkImageDrmFormatModifierListCreateInfoEXT or "
                             "VkImageDrmFormatModifierExplicitCreateInfoEXT.");
        } else if ((drm_format_mod_list) && (drm_format_mod_explict)) {
            skip |= LogError("VUID-VkImageCreateInfo-tiling-02261", device, create_info_loc.dot(Field::tiling),
                             "is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT but pNext has both "
                             "VkImageDrmFormatModifierListCreateInfoEXT and "
                             "VkImageDrmFormatModifierExplicitCreateInfoEXT.");
        } else if (drm_format_mod_explict) {
            image_create_drm_format_modifiers.push_back(drm_format_mod_explict->drmFormatModifier);
        } else if (drm_format_mod_list) {
            for (uint32_t i = 0; i < drm_format_mod_list->drmFormatModifierCount; i++) {
                image_create_drm_format_modifiers.push_back(*drm_format_mod_list->pDrmFormatModifiers);
            }
        }

        if (create_info.flags & VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT) {
            const auto format_list_info = vku::FindStructInPNextChain<VkImageFormatListCreateInfo>(create_info.pNext);
            if (!format_list_info) {
                skip |= LogError("VUID-VkImageCreateInfo-tiling-02353", device, create_info_loc.dot(Field::tiling),
                                 "is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT, flags includes "
                                 "VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, but pNext is missing VkImageFormatListCreateInfo.");
            } else if (format_list_info->viewFormatCount == 0) {
                skip |= LogError("VUID-VkImageCreateInfo-tiling-02353", device, create_info_loc.dot(Field::tiling),
                                 "is VK_IMAGE_TILING_DRM_FORMAT_MODIFIER_EXT, flags includes VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT, "
                                 "but pNext<VkImageFormatListCreateInfo>.viewFormatCount is zero.");
            }
        }
    } else if (drm_format_mod_list) {
        skip |= LogError("VUID-VkImageCreateInfo-pNext-02262", device, create_info_loc.dot(Field::tiling),
                         "is %s, but there is a "
                         "VkImageDrmFormatModifierListCreateInfoEXT in the pNext chain",
                         string_VkImageTiling(create_info.tiling));
    } else if (drm_format_mod_explict) {
        skip |= LogError("VUID-VkImageCreateInfo-pNext-02262", device, create_info_loc.dot(Field::tiling),
                         "is %s, but there is a VkImageDrmFormatModifierExplicitCreateInfoEXT "
                         "in the pNext chain",
                         string_VkImageTiling(create_info.tiling));
    }

    if (drm_format_mod_explict && drm_format_mod_explict->pPlaneLayouts) {
        for (uint32_t i = 0; i < drm_format_mod_explict->drmFormatModifierPlaneCount; ++i) {
            const Location drm_loc =
                create_info_loc.pNext(Struct::VkImageDrmFormatModifierExplicitCreateInfoEXT, Field::pPlaneLayouts, i);
            if (drm_format_mod_explict->pPlaneLayouts[i].size != 0) {
                skip |= LogError("VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-size-02267", device, drm_loc.dot(Field::size),
                                 "is not zero (%" PRIu64 ").", drm_format_mod_explict->pPlaneLayouts[i].size);
            }
            if (create_info.arrayLayers == 1 && drm_format_mod_explict->pPlaneLayouts[i].arrayPitch != 0) {
                skip |= LogError("VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-arrayPitch-02268", device,
                                 drm_loc.dot(Field::arrayPitch), "is %" PRIu64 " and arrayLayers is 1.",
                                 drm_format_mod_explict->pPlaneLayouts[i].arrayPitch);
            }
            if (create_info.extent.depth == 1 && drm_format_mod_explict->pPlaneLayouts[i].depthPitch != 0) {
                skip |= LogError("VUID-VkImageDrmFormatModifierExplicitCreateInfoEXT-depthPitch-02269", device,
                                 drm_loc.dot(Field::depthPitch), "is %" PRIu64 " and extext.depth is 1.",
                                 drm_format_mod_explict->pPlaneLayouts[i].depthPitch);
            }
        }
    }

    const auto compression_control = vku::FindStructInPNextChain<VkImageCompressionControlEXT>(create_info.pNext);
    if (drm_format_mod_explict && compression_control) {
        skip |= LogError("VUID-VkImageCreateInfo-pNext-06746", device, create_info_loc.dot(Field::pNext),
                         "has both VkImageCompressionControlEXT and VkImageDrmFormatModifierExplicitCreateInfoEXT.");
    }

    return skip;
}

bool Device::manual_PreCallValidateCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkImageView *pView,
                                                   const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (pCreateInfo == nullptr) {
        return skip;
    }
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    // Validate feature set if using CUBE_ARRAY
    if ((pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) && (!enabled_features.imageCubeArray)) {
        skip |= LogError("VUID-VkImageViewCreateInfo-viewType-01004", pCreateInfo->image, create_info_loc.dot(Field::viewType),
                         "is VK_IMAGE_VIEW_TYPE_CUBE_ARRAY but the imageCubeArray feature is not enabled.");
    }

    if (pCreateInfo->subresourceRange.layerCount != VK_REMAINING_ARRAY_LAYERS) {
        if (pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_CUBE && pCreateInfo->subresourceRange.layerCount != 6) {
            skip |= LogError("VUID-VkImageViewCreateInfo-viewType-02960", pCreateInfo->image,
                             create_info_loc.dot(Field::subresourceRange).dot(Field::layerCount),
                             "(%" PRIu32 ") must be 6 or VK_REMAINING_ARRAY_LAYERS.", pCreateInfo->subresourceRange.layerCount);
        }
        if (pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY && (pCreateInfo->subresourceRange.layerCount % 6) != 0) {
            skip |= LogError("VUID-VkImageViewCreateInfo-viewType-02961", pCreateInfo->image,
                             create_info_loc.dot(Field::subresourceRange).dot(Field::layerCount),
                             "(%" PRIu32 ") must be a multiple of 6 or VK_REMAINING_ARRAY_LAYERS.",
                             pCreateInfo->subresourceRange.layerCount);
        }
    }

    auto astc_decode_mode = vku::FindStructInPNextChain<VkImageViewASTCDecodeModeEXT>(pCreateInfo->pNext);
    if (astc_decode_mode != nullptr) {
        if ((astc_decode_mode->decodeMode != VK_FORMAT_R16G16B16A16_SFLOAT) &&
            (astc_decode_mode->decodeMode != VK_FORMAT_R8G8B8A8_UNORM) &&
            (astc_decode_mode->decodeMode != VK_FORMAT_E5B9G9R9_UFLOAT_PACK32)) {
            skip |= LogError("VUID-VkImageViewASTCDecodeModeEXT-decodeMode-02230", pCreateInfo->image,
                             create_info_loc.pNext(Struct::VkImageViewASTCDecodeModeEXT, Field::decodeMode), "is %s.",
                             string_VkFormat(astc_decode_mode->decodeMode));
        }
        if ((vkuFormatIsCompressed_ASTC_LDR(pCreateInfo->format) == false) &&
            (vkuFormatIsCompressed_ASTC_HDR(pCreateInfo->format) == false)) {
            skip |=
                LogError("VUID-VkImageViewASTCDecodeModeEXT-format-04084", pCreateInfo->image, create_info_loc.dot(Field::format),
                         "%s is  not an ASTC format (because VkImageViewASTCDecodeModeEXT was passed in the pNext chain).",
                         string_VkFormat(pCreateInfo->format));
        }
    }

    auto ycbcr_conversion = vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(pCreateInfo->pNext);
    if (ycbcr_conversion != nullptr) {
        if (ycbcr_conversion->conversion != VK_NULL_HANDLE) {
            if (IsIdentitySwizzle(pCreateInfo->components) == false) {
                skip |= LogError("VUID-VkImageViewCreateInfo-pNext-01970", pCreateInfo->image, create_info_loc,
                                 "If there is a VkSamplerYcbcrConversion, the imageView must "
                                 "be created with the identity swizzle. Here are the actual swizzle values:\n%s",
                                 string_VkComponentMapping(pCreateInfo->components).c_str());
            }
        }
    }
#ifdef VK_USE_PLATFORM_METAL_EXT
    skip |=
        ExportMetalObjectsPNextUtil(VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT, "VUID-VkImageViewCreateInfo-pNext-06787",
                                    error_obj.location, "VK_EXPORT_METAL_OBJECT_TYPE_METAL_TEXTURE_BIT_EXT", pCreateInfo->pNext);
#endif  // VK_USE_PLATFORM_METAL_EXT
    return skip;
}

bool Device::manual_PreCallValidateGetDeviceImageSubresourceLayout(VkDevice device, const VkDeviceImageSubresourceInfo *pInfo,
                                                                   VkSubresourceLayout2 *pLayout, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    const Location info_loc = error_obj.location.dot(Field::pInfo);
    const Location create_info_loc = info_loc.dot(Field::pCreateInfo);
    const Location subresource_loc = info_loc.dot(Field::pSubresource);

    const VkImageCreateInfo &create_info = *pInfo->pCreateInfo;
    const VkImageSubresource &subresource = pInfo->pSubresource->imageSubresource;
    const VkImageAspectFlags aspect_mask = subresource.aspectMask;

    if (GetBitSetCount(aspect_mask) != 1) {
        skip |= LogError("VUID-VkDeviceImageSubresourceInfo-aspectMask-00997", device, subresource_loc.dot(Field::aspectMask),
                         "(%s) must have exactly 1 bit set.", string_VkImageAspectFlags(aspect_mask).c_str());
    }

    if (subresource.mipLevel >= create_info.mipLevels) {
        skip |= LogError("VUID-VkDeviceImageSubresourceInfo-mipLevel-01716", device, subresource_loc.dot(Field::mipLevel),
                         "(%" PRIu32 ") must be less than %s (%" PRIu32 ").", subresource.mipLevel,
                         create_info_loc.dot(Field::mipLevel).Fields().c_str(), create_info.mipLevels);
    }

    if (subresource.arrayLayer >= create_info.arrayLayers) {
        skip |= LogError("VUID-VkDeviceImageSubresourceInfo-arrayLayer-01717", device, subresource_loc.dot(Field::arrayLayer),
                         "(%" PRIu32 ") must be less than %s (%" PRIu32 ").", subresource.arrayLayer,
                         create_info_loc.dot(Field::arrayLayers).Fields().c_str(), create_info.arrayLayers);
    }

    const VkFormat image_format = create_info.format;
    const bool tiling_linear_optimal =
        create_info.tiling == VK_IMAGE_TILING_LINEAR || create_info.tiling == VK_IMAGE_TILING_OPTIMAL;
    if (vkuFormatIsColor(image_format) && !vkuFormatIsMultiplane(image_format) && (aspect_mask != VK_IMAGE_ASPECT_COLOR_BIT) &&
        tiling_linear_optimal) {
        skip |= LogError("VUID-VkDeviceImageSubresourceInfo-format-08886", device, subresource_loc.dot(Field::aspectMask),
                         "(%s) is invalid with %s (%s).", string_VkImageAspectFlags(aspect_mask).c_str(),
                         create_info_loc.dot(Field::format).Fields().c_str(), string_VkFormat(image_format));
    }

    if (vkuFormatHasDepth(image_format) && ((aspect_mask & VK_IMAGE_ASPECT_DEPTH_BIT) == 0)) {
        skip |= LogError("VUID-VkDeviceImageSubresourceInfo-format-04462", device, subresource_loc.dot(Field::aspectMask),
                         "(%s) is invalid with %s (%s).", string_VkImageAspectFlags(aspect_mask).c_str(),
                         create_info_loc.dot(Field::format).Fields().c_str(), string_VkFormat(image_format));
    }

    if (vkuFormatHasStencil(image_format) && ((aspect_mask & VK_IMAGE_ASPECT_STENCIL_BIT) == 0)) {
        skip |= LogError("VUID-VkDeviceImageSubresourceInfo-format-04463", device, subresource_loc.dot(Field::aspectMask),
                         "(%s) is invalid with %s (%s).", string_VkImageAspectFlags(aspect_mask).c_str(),
                         create_info_loc.dot(Field::format).Fields().c_str(), string_VkFormat(image_format));
    }

    if (!vkuFormatHasDepth(image_format) && !vkuFormatHasStencil(image_format)) {
        if ((aspect_mask & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0) {
            skip |= LogError("VUID-VkDeviceImageSubresourceInfo-format-04464", device, subresource_loc.dot(Field::aspectMask),
                             "(%s) is invalid with %s (%s).", string_VkImageAspectFlags(aspect_mask).c_str(),
                             create_info_loc.dot(Field::format).Fields().c_str(), string_VkFormat(image_format));
        }
    }

    // subresource's aspect must be compatible with image's format.
    if (create_info.tiling == VK_IMAGE_TILING_LINEAR) {
        if (vkuFormatIsMultiplane(image_format) && !IsOnlyOneValidPlaneAspect(image_format, aspect_mask)) {
            skip |= LogError("VUID-VkDeviceImageSubresourceInfo-tiling-08717", device, subresource_loc.dot(Field::aspectMask),
                             "(%s) is invalid for %s (%s).", string_VkImageAspectFlags(aspect_mask).c_str(),
                             create_info_loc.dot(Field::format).Fields().c_str(), string_VkFormat(image_format));
        }
    }

    return skip;
}
}  // namespace stateless
