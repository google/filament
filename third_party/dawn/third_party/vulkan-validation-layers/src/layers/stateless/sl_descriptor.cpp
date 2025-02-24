/* Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (C) 2015-2025 Google Inc.
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

#include "stateless/stateless_validation.h"
#include "generated/enum_flag_bits.h"
#include "error_message/error_strings.h"

namespace stateless {
bool Device::ValidateCoarseSampleOrderCustomNV(const VkCoarseSampleOrderCustomNV &order, const Location &order_loc) const {
    bool skip = false;

    struct SampleOrderInfo {
        VkShadingRatePaletteEntryNV shadingRate;
        uint32_t width;
        uint32_t height;
    };

    // All palette entries with more than one pixel per fragment
    constexpr std::array sample_order_infos = {
        SampleOrderInfo{VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_1X2_PIXELS_NV, 1, 2},
        SampleOrderInfo{VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X1_PIXELS_NV, 2, 1},
        SampleOrderInfo{VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X2_PIXELS_NV, 2, 2},
        SampleOrderInfo{VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X2_PIXELS_NV, 4, 2},
        SampleOrderInfo{VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_2X4_PIXELS_NV, 2, 4},
        SampleOrderInfo{VK_SHADING_RATE_PALETTE_ENTRY_1_INVOCATION_PER_4X4_PIXELS_NV, 4, 4},
    };

    const SampleOrderInfo *sample_order_info;
    uint32_t info_idx = 0;
    for (sample_order_info = nullptr; info_idx < sample_order_infos.size(); ++info_idx) {
        if (sample_order_infos[info_idx].shadingRate == order.shadingRate) {
            sample_order_info = &sample_order_infos[info_idx];
            break;
        }
    }

    if (sample_order_info == nullptr) {
        skip |= LogError("VUID-VkCoarseSampleOrderCustomNV-shadingRate-02073", device, order_loc,
                         "shadingRate must be a shading rate "
                         "that generates fragments with more than one pixel.");
        return skip;
    }

    if (order.sampleCount == 0 || (order.sampleCount & (order.sampleCount - 1)) ||
        !(order.sampleCount & device_limits.framebufferNoAttachmentsSampleCounts)) {
        skip |= LogError("VUID-VkCoarseSampleOrderCustomNV-sampleCount-02074", device, order_loc.dot(Field::sampleCount),
                         "(%" PRIu32
                         ") must correspond to a sample count enumerated in VkSampleCountFlags whose corresponding bit "
                         "is set in framebufferNoAttachmentsSampleCounts.",
                         order.sampleCount);
    }

    if (order.sampleLocationCount != order.sampleCount * sample_order_info->width * sample_order_info->height) {
        skip |= LogError("VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02075", device,
                         order_loc.dot(Field::sampleLocationCount),
                         "(%" PRIu32 ") must be equal to the product of sampleCount (%" PRIu32
                         "), the fragment width for shadingRate "
                         "(%" PRIu32 "), and the fragment height for shadingRate (%" PRIu32 ").",
                         order.sampleLocationCount, order.sampleCount, sample_order_info->width, sample_order_info->height);
    }

    if (order.sampleLocationCount > phys_dev_ext_props.shading_rate_image_props.shadingRateMaxCoarseSamples) {
        skip |= LogError("VUID-VkCoarseSampleOrderCustomNV-sampleLocationCount-02076", device,
                         order_loc.dot(Field::sampleLocationCount),
                         "(%" PRIu32 ") must be less than or equal to shadingRateMaxCoarseSamples (%" PRIu32 ").",
                         order.sampleLocationCount, phys_dev_ext_props.shading_rate_image_props.shadingRateMaxCoarseSamples);
    }

    // Accumulate a bitmask tracking which (x,y,sample) tuples are seen. Expect
    // the first width*height*sampleCount bits to all be set. Note: There is no
    // guarantee that 64 bits is enough, but practically it's unlikely for an
    // implementation to support more than 32 bits for samplemask.
    assert(phys_dev_ext_props.shading_rate_image_props.shadingRateMaxCoarseSamples <= 64);
    uint64_t sample_locations_mask = 0;
    for (uint32_t i = 0; i < order.sampleLocationCount; ++i) {
        const VkCoarseSampleLocationNV *sample_loc = &order.pSampleLocations[i];
        if (sample_loc->pixelX >= sample_order_info->width) {
            skip |= LogError("VUID-VkCoarseSampleLocationNV-pixelX-02078", device, order_loc,
                             "pixelX (%" PRIu32 ") must be less than the width (in pixels) of the fragment (%" PRIu32 ").",
                             sample_loc->pixelX, sample_order_info->width);
        }
        if (sample_loc->pixelY >= sample_order_info->height) {
            skip |= LogError("VUID-VkCoarseSampleLocationNV-pixelY-02079", device, order_loc,
                             "pixelY (%" PRIu32 ") must be less than the height (in pixels) of the fragment (%" PRIu32 ").",
                             sample_loc->pixelY, sample_order_info->height);
        }
        if (sample_loc->sample >= order.sampleCount) {
            skip |= LogError("VUID-VkCoarseSampleLocationNV-sample-02080", device, order_loc,
                             "sample (%" PRIu32
                             ") must be less than the number of coverage samples in each pixel belonging to the fragment (%" PRIu32
                             ").",
                             sample_loc->sample, order.sampleCount);
        }
        uint32_t idx =
            sample_loc->sample + order.sampleCount * (sample_loc->pixelX + sample_order_info->width * sample_loc->pixelY);
        sample_locations_mask |= 1ULL << idx;
    }

    uint64_t expected_mask = (order.sampleLocationCount == 64) ? ~0ULL : ((1ULL << order.sampleLocationCount) - 1);
    if (sample_locations_mask != expected_mask) {
        skip |= LogError(
            "VUID-VkCoarseSampleOrderCustomNV-pSampleLocations-02077", device, order_loc,
            "The array pSampleLocations must contain exactly one entry for "
            "every combination of valid values for pixelX, pixelY, and sample in the structure VkCoarseSampleOrderCustomNV.");
    }

    return skip;
}

// VK_EXT_sampler_filter_minmax
bool Device::ValidateSamplerFilterMinMax(const VkSamplerCreateInfo &create_info, const Location &create_info_loc) const {
    bool skip = false;
    const auto *sampler_reduction = vku::FindStructInPNextChain<VkSamplerReductionModeCreateInfo>(create_info.pNext);
    if (!sampler_reduction) return skip;

    if (sampler_reduction->reductionMode != VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE) {
        if ((api_version >= VK_API_VERSION_1_2) && !enabled_features.samplerFilterMinmax) {
            skip |= LogError("VUID-VkSamplerCreateInfo-pNext-06726", device,
                             create_info_loc.pNext(Struct::VkSamplerReductionModeCreateInfo, Field::reductionMode),
                             "is %s but samplerFilterMinmax feature was not enabled.",
                             string_VkSamplerReductionMode(sampler_reduction->reductionMode));
        } else if ((api_version < VK_API_VERSION_1_2) && !IsExtEnabled(extensions.vk_ext_sampler_filter_minmax)) {
            // NOTE: technically this VUID is only if the corresponding _feature_ is not enabled, and only if on api_version
            // >= 1.2, but there doesn't appear to be a similar VUID for when api_version < 1.2
            skip |=
                LogError("VUID-VkSamplerCreateInfo-pNext-06726", device,
                         create_info_loc.pNext(Struct::VkSamplerReductionModeCreateInfo, Field::reductionMode),
                         "is %s, but extension %s is not enabled.", string_VkSamplerReductionMode(sampler_reduction->reductionMode),
                         VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
        }

        if (vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(create_info.pNext)) {
            skip |= LogError("VUID-VkSamplerCreateInfo-None-01647", device,
                             create_info_loc.pNext(Struct::VkSamplerReductionModeCreateInfo, Field::reductionMode),
                             "is %s but a VkSamplerYcbcrConversionInfo structure is also linked in the pNext chain of the "
                             "VkSamplerCreateInfo structure.",
                             string_VkSamplerReductionMode(sampler_reduction->reductionMode));
        }

        if (create_info.compareEnable == VK_TRUE) {
            skip |=
                LogError("VUID-VkSamplerCreateInfo-compareEnable-01423", device,
                         create_info_loc.pNext(Struct::VkSamplerReductionModeCreateInfo, Field::reductionMode),
                         "is %s but compareEnable is VK_TRUE.", string_VkSamplerReductionMode(sampler_reduction->reductionMode));
        }

        // This VU is the one feature difference between the IMG and EXT version of the extension
        if (create_info.magFilter == VK_FILTER_CUBIC_IMG || create_info.minFilter == VK_FILTER_CUBIC_IMG) {
            if (!IsExtEnabled(extensions.vk_ext_filter_cubic)) {
                skip |= LogError("VUID-VkSamplerCreateInfo-magFilter-07911", device,
                                 create_info_loc.pNext(Struct::VkSamplerReductionModeCreateInfo, Field::reductionMode),
                                 "is %s, magFilter is %s and minFilter is %s, but "
                                 "extension %s is not enabled.",
                                 string_VkSamplerReductionMode(sampler_reduction->reductionMode),
                                 string_VkFilter(create_info.magFilter), string_VkFilter(create_info.minFilter),
                                 VK_EXT_FILTER_CUBIC_EXTENSION_NAME);
            }
        }
    }

    return skip;
}

// VK_EXT_custom_border_color
bool Device::ValidateSamplerCustomBoarderColor(const VkSamplerCreateInfo &create_info, const Location &create_info_loc) const {
    bool skip = false;

    if (create_info.borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT || create_info.borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT) {
        if (!enabled_features.customBorderColors) {
            skip |=
                LogError("VUID-VkSamplerCreateInfo-customBorderColors-04085", device, create_info_loc.dot(Field::borderColor),
                         "is %s but customBorderColors feature was not enabled.", string_VkBorderColor(create_info.borderColor));
        }

        auto custom_create_info = vku::FindStructInPNextChain<VkSamplerCustomBorderColorCreateInfoEXT>(create_info.pNext);
        if (!custom_create_info) {
            skip |= LogError("VUID-VkSamplerCreateInfo-borderColor-04011", device, create_info_loc.dot(Field::borderColor),
                             "is %s but there is no VkSamplerCustomBorderColorCreateInfoEXT "
                             "struct in pNext chain.",
                             string_VkBorderColor(create_info.borderColor));
        } else {
            if ((custom_create_info->format != VK_FORMAT_UNDEFINED) && !vkuFormatIsDepthAndStencil(custom_create_info->format) &&
                ((create_info.borderColor == VK_BORDER_COLOR_INT_CUSTOM_EXT &&
                  !vkuFormatIsSampledInt(custom_create_info->format)) ||
                 (create_info.borderColor == VK_BORDER_COLOR_FLOAT_CUSTOM_EXT &&
                  !vkuFormatIsSampledFloat(custom_create_info->format)))) {
                skip |= LogError("VUID-VkSamplerCustomBorderColorCreateInfoEXT-format-07605", device,
                                 create_info_loc.pNext(Struct::VkSamplerCustomBorderColorCreateInfoEXT, Field::format),
                                 "%s does not match borderColor (%s).", string_VkFormat(custom_create_info->format),
                                 string_VkBorderColor(create_info.borderColor));
            }

            if (custom_create_info->format == VK_FORMAT_UNDEFINED && !enabled_features.customBorderColorWithoutFormat) {
                skip |= LogError("VUID-VkSamplerCustomBorderColorCreateInfoEXT-format-04014", device,
                                 create_info_loc.pNext(Struct::VkSamplerCustomBorderColorCreateInfoEXT, Field::format),
                                 "is VK_FORMAT_UNDEFINED but the "
                                 "customBorderColorWithoutFormat feature was not enabled.");
            }
        }
    }

    return skip;
}

// VK_EXT_fragment_density_map
bool Device::ValidateSamplerSubsampled(const VkSamplerCreateInfo &create_info, const Location &create_info_loc) const {
    bool skip = false;
    if ((create_info.flags & VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT) == 0) return skip;

    if (create_info.minFilter != create_info.magFilter) {
        skip |= LogError("VUID-VkSamplerCreateInfo-flags-02574", device, create_info_loc.dot(Field::flags),
                         "includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, but "
                         "minFilter (%s) and magFilter (%s) are not equal.",
                         string_VkFilter(create_info.minFilter), string_VkFilter(create_info.magFilter));
    }
    if (create_info.mipmapMode != VK_SAMPLER_MIPMAP_MODE_NEAREST) {
        skip |= LogError("VUID-VkSamplerCreateInfo-flags-02575", device, create_info_loc.dot(Field::flags),
                         "includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, but "
                         "mipmapMode (%s) is not VK_SAMPLER_MIPMAP_MODE_NEAREST.",
                         string_VkSamplerMipmapMode(create_info.mipmapMode));
    }
    if (create_info.minLod != 0.0 || create_info.maxLod != 0.0) {
        skip |= LogError("VUID-VkSamplerCreateInfo-flags-02576", device, create_info_loc.dot(Field::flags),
                         "includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, but "
                         "minLod (%f) and maxLod (%f) are not zero.",
                         create_info.minLod, create_info.maxLod);
    }
    if (((create_info.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) &&
         (create_info.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) ||
        ((create_info.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) &&
         (create_info.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER))) {
        skip |=
            LogError("VUID-VkSamplerCreateInfo-flags-02577", device, create_info_loc.dot(Field::flags),
                     "includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, so "
                     "addressModeU (%s) and addressModeV (%s) must be "
                     "CLAMP_TO_EDGE or CLAMP_TO_BORDER",
                     string_VkSamplerAddressMode(create_info.addressModeU), string_VkSamplerAddressMode(create_info.addressModeV));
    }
    if (create_info.anisotropyEnable) {
        skip |= LogError("VUID-VkSamplerCreateInfo-flags-02578", device, create_info_loc.dot(Field::flags),
                         "includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, "
                         "but anisotropyEnable is VK_TRUE.");
    }
    if (create_info.compareEnable) {
        skip |= LogError("VUID-VkSamplerCreateInfo-flags-02579", device, create_info_loc.dot(Field::flags),
                         "includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, "
                         "but compareEnable is VK_TRUE.");
    }
    if (create_info.unnormalizedCoordinates) {
        skip |= LogError("VUID-VkSamplerCreateInfo-flags-02580", device, create_info_loc.dot(Field::flags),
                         "includes VK_SAMPLER_CREATE_SUBSAMPLED_BIT_EXT, "
                         "but unnormalizedCoordinates is VK_TRUE.");
    }

    return skip;
}

// VK_QCOM_image_processing
bool Device::ValidateSamplerImageProcessingQCOM(const VkSamplerCreateInfo &create_info, const Location &create_info_loc) const {
    bool skip = false;
    if ((create_info.flags & VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM) == 0) return skip;

    if ((create_info.minFilter != VK_FILTER_NEAREST) || (create_info.magFilter != VK_FILTER_NEAREST)) {
        skip |= LogError("VUID-VkSamplerCreateInfo-flags-06964", device, create_info_loc.dot(Field::flags),
                         "includes VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM, so "
                         "minFilter (%s) must be VK_FILTER_NEAREST and "
                         "magFilter (%s) must be VK_FILTER_NEAREST.",
                         string_VkFilter(create_info.minFilter), string_VkFilter(create_info.magFilter));
    }
    if (create_info.mipmapMode != VK_SAMPLER_MIPMAP_MODE_NEAREST) {
        skip |= LogError("VUID-VkSamplerCreateInfo-flags-06965", device, create_info_loc.dot(Field::flags),
                         "includes VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM, so "
                         "mipmapMode (%s) must be VK_SAMPLER_MIPMAP_MODE_NEAREST.",
                         string_VkSamplerMipmapMode(create_info.mipmapMode));
    }
    if ((create_info.minLod != 0) || (create_info.maxLod != 0)) {
        skip |= LogError("VUID-VkSamplerCreateInfo-flags-06966", device, create_info_loc.dot(Field::flags),
                         "includes VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM, so "
                         "minLod (%f) and maxLod (%f) must be 0.",
                         create_info.minLod, create_info.maxLod);
    }
    if (((create_info.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) &&
         (create_info.addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) ||
        ((create_info.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) &&
         (create_info.addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER))) {
        skip |=
            LogError("VUID-VkSamplerCreateInfo-flags-06967", device, create_info_loc.dot(Field::flags),
                     "includes VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM, so "
                     "addressModeU (%s) and addressModeV (%s) must be either "
                     "VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE or VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER.",
                     string_VkSamplerAddressMode(create_info.addressModeU), string_VkSamplerAddressMode(create_info.addressModeV));
    }
    if (((create_info.addressModeU == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) ||
         (create_info.addressModeV == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) &&
        (create_info.borderColor != VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK)) {
        skip |= LogError("VUID-VkSamplerCreateInfo-flags-06968", device, create_info_loc.dot(Field::flags),
                         "includes VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM, "
                         "so if either addressModeU (%s) or addressModeV (%s) is "
                         "VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, then"
                         "borderColor (%s) must be VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK.",
                         string_VkSamplerAddressMode(create_info.addressModeU),
                         string_VkSamplerAddressMode(create_info.addressModeV), string_VkBorderColor(create_info.borderColor));
    }
    if (create_info.anisotropyEnable) {
        skip |= LogError("VUID-VkSamplerCreateInfo-flags-06969", device, create_info_loc.dot(Field::flags),
                         "includes VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM, "
                         "but anisotropyEnable is VK_TRUE.");
    }
    if (create_info.compareEnable) {
        skip |= LogError("VUID-VkSamplerCreateInfo-flags-06970", device, create_info_loc.dot(Field::flags),
                         "includes VK_SAMPLER_CREATE_IMAGE_PROCESSING_BIT_QCOM, "
                         "but compareEnable is VK_TRUE.");
    }

    return skip;
}

bool Device::manual_PreCallValidateCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
                                                 const VkAllocationCallbacks *pAllocator, VkSampler *pSampler,
                                                 const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    const auto &limits = device_limits;

    if (pCreateInfo->anisotropyEnable == VK_TRUE) {
        if (!IsBetweenInclusive(pCreateInfo->maxAnisotropy, 1.0F, limits.maxSamplerAnisotropy)) {
            skip |= LogError("VUID-VkSamplerCreateInfo-anisotropyEnable-01071", device, create_info_loc.dot(Field::maxAnisotropy),
                             "is %f but must be in the range of [1.0, %f] (maxSamplerAnistropy).", pCreateInfo->maxAnisotropy,
                             limits.maxSamplerAnisotropy);
        }

        // Anistropy cannot be enabled in sampler unless enabled as a feature
        if (enabled_features.samplerAnisotropy == VK_FALSE) {
            skip |=
                LogError("VUID-VkSamplerCreateInfo-anisotropyEnable-01070", device, create_info_loc.dot(Field::anisotropyEnable),
                         "is VK_TRUE but the samplerAnisotropy feature was not enabled.");
        }
    }

    if (pCreateInfo->unnormalizedCoordinates == VK_TRUE) {
        if (pCreateInfo->minFilter != pCreateInfo->magFilter) {
            skip |= LogError("VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01072", device,
                             create_info_loc.dot(Field::unnormalizedCoordinates),
                             "is VK_TRUE, but minFilter (%s) is different then magFilter (%s).",
                             string_VkFilter(pCreateInfo->minFilter), string_VkFilter(pCreateInfo->magFilter));
        }
        if (pCreateInfo->mipmapMode != VK_SAMPLER_MIPMAP_MODE_NEAREST) {
            skip |= LogError("VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01073", device,
                             create_info_loc.dot(Field::unnormalizedCoordinates),
                             "is VK_TRUE, but mipmapMode (%s) must be VK_SAMPLER_MIPMAP_MODE_NEAREST.",
                             string_VkSamplerMipmapMode(pCreateInfo->mipmapMode));
        }
        if (pCreateInfo->minLod != 0.0f || pCreateInfo->maxLod != 0.0f) {
            skip |= LogError("VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01074", device,
                             create_info_loc.dot(Field::unnormalizedCoordinates),
                             "is VK_TRUE, but minLod (%f) and maxLod (%f) must both be zero.", pCreateInfo->minLod,
                             pCreateInfo->maxLod);
        }
        if ((pCreateInfo->addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE &&
             pCreateInfo->addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) ||
            (pCreateInfo->addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE &&
             pCreateInfo->addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) {
            skip |= LogError("VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01075", device,
                             create_info_loc.dot(Field::unnormalizedCoordinates),
                             "is VK_TRUE, but addressModeU (%s) and addressModeV (%s) must both be "
                             "CLAMP_TO_EDGE or CLAMP_TO_BORDER.",
                             string_VkSamplerAddressMode(pCreateInfo->addressModeU),
                             string_VkSamplerAddressMode(pCreateInfo->addressModeV));
        }
        if (pCreateInfo->anisotropyEnable == VK_TRUE) {
            skip |= LogError("VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01076", device, create_info_loc,
                             "anisotropyEnable and unnormalizedCoordinates are both VK_TRUE.");
        }
        if (pCreateInfo->compareEnable == VK_TRUE) {
            skip |= LogError("VUID-VkSamplerCreateInfo-unnormalizedCoordinates-01077", device, create_info_loc,
                             "compareEnable and unnormalizedCoordinates are both VK_TRUE.");
        }
    }

    if (pCreateInfo->compareEnable == VK_TRUE) {
        skip |= context.ValidateRangedEnum(create_info_loc.dot(Field::compareOp), vvl::Enum::VkCompareOp, pCreateInfo->compareOp,
                                           "VUID-VkSamplerCreateInfo-compareEnable-01080");
    }

    if ((pCreateInfo->addressModeU == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) ||
        (pCreateInfo->addressModeV == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER) ||
        (pCreateInfo->addressModeW == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)) {
        skip |= context.ValidateRangedEnum(create_info_loc.dot(Field::borderColor), vvl::Enum::VkBorderColor,
                                           pCreateInfo->borderColor, "VUID-VkSamplerCreateInfo-addressModeU-01078");
    }

    if (enabled_features.samplerMirrorClampToEdge == VK_FALSE) {
        // Only display at most one of address mode error otherwise it turns into spam
        if (pCreateInfo->addressModeU == VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE) {
            skip |= LogError("VUID-VkSamplerCreateInfo-addressModeU-01079", device, create_info_loc.dot(Field::addressModeU),
                             "is VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE but the "
                             "VK_KHR_sampler_mirror_clamp_to_edge extension or samplerMirrorClampToEdge feature was not enabled.");
        } else if (pCreateInfo->addressModeV == VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE) {
            skip |= LogError("VUID-VkSamplerCreateInfo-addressModeU-01079", device, create_info_loc.dot(Field::addressModeV),
                             "is VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE but the "
                             "VK_KHR_sampler_mirror_clamp_to_edge extension or samplerMirrorClampToEdge feature was not enabled.");
        } else if (pCreateInfo->addressModeW == VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE) {
            skip |= LogError("VUID-VkSamplerCreateInfo-addressModeU-01079", device, create_info_loc.dot(Field::addressModeW),
                             "is VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE but the "
                             "VK_KHR_sampler_mirror_clamp_to_edge extension or samplerMirrorClampToEdge feature was not enabled.");
        }
    }

    // Checks for the IMG cubic filtering extension
    if (IsExtEnabled(extensions.vk_img_filter_cubic)) {
        if ((pCreateInfo->anisotropyEnable == VK_TRUE) &&
            ((pCreateInfo->minFilter == VK_FILTER_CUBIC_IMG) || (pCreateInfo->magFilter == VK_FILTER_CUBIC_IMG))) {
            skip |= LogError("VUID-VkSamplerCreateInfo-magFilter-01081", device, create_info_loc.dot(Field::anisotropyEnable),
                             "is VK_TRUE, but minFilter (%s) and magFilter (%s) can't be cubic.",
                             string_VkFilter(pCreateInfo->minFilter), string_VkFilter(pCreateInfo->magFilter));
        }
    }

    // Check for valid Lod range
    if (pCreateInfo->minLod > pCreateInfo->maxLod) {
        skip |= LogError("VUID-VkSamplerCreateInfo-maxLod-01973", device, create_info_loc.dot(Field::minLod),
                         "(%f) is greater than maxLod (%f)", pCreateInfo->minLod, pCreateInfo->maxLod);
    }

    // Check mipLodBias to device limit
    if (pCreateInfo->mipLodBias > limits.maxSamplerLodBias) {
        skip |= LogError("VUID-VkSamplerCreateInfo-mipLodBias-01069", device, create_info_loc.dot(Field::mipLodBias),
                         "(%f) is greater than maxSamplerLodBias (%f)", pCreateInfo->mipLodBias, limits.maxSamplerLodBias);
    }

    if (vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(pCreateInfo->pNext)) {
        if ((pCreateInfo->addressModeU != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) ||
            (pCreateInfo->addressModeV != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE) ||
            (pCreateInfo->addressModeW != VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)) {
            skip |= LogError("VUID-VkSamplerCreateInfo-addressModeU-01646", device, create_info_loc.dot(Field::pNext),
                             "contains a VkSamplerYcbcrConversionInfo struct, but all address modes must be "
                             "VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE:\n"
                             "addressModeU (%s)\naddressModeV (%s)\naddressModeW (%s)\n",
                             string_VkSamplerAddressMode(pCreateInfo->addressModeU),
                             string_VkSamplerAddressMode(pCreateInfo->addressModeV),
                             string_VkSamplerAddressMode(pCreateInfo->addressModeW));
        } else if (pCreateInfo->anisotropyEnable) {
            skip |= LogError("VUID-VkSamplerCreateInfo-addressModeU-01646", device, create_info_loc.dot(Field::pNext),
                             "contains a VkSamplerYcbcrConversionInfo struct, but anisotropyEnable must be VK_FALSE.");
        } else if (pCreateInfo->unnormalizedCoordinates) {
            skip |= LogError("VUID-VkSamplerCreateInfo-addressModeU-01646", device, create_info_loc.dot(Field::pNext),
                             "contains a VkSamplerYcbcrConversionInfo struct, but unnormalizedCoordinates must be VK_FALSE.");
        }
    }

    if (vku::FindStructInPNextChain<VkSamplerBorderColorComponentMappingCreateInfoEXT>(pCreateInfo->pNext)) {
        if (!enabled_features.borderColorSwizzle) {
            skip |=
                LogError("VUID-VkSamplerBorderColorComponentMappingCreateInfoEXT-borderColorSwizzle-06437", device, create_info_loc,
                         "The borderColorSwizzle feature must be enabled to use "
                         "VkPhysicalDeviceBorderColorSwizzleFeaturesEXT");
        }
    }

    if ((pCreateInfo->flags & VK_SAMPLER_CREATE_NON_SEAMLESS_CUBE_MAP_BIT_EXT) && (!enabled_features.nonSeamlessCubeMap)) {
        skip |= LogError("VUID-VkSamplerCreateInfo-nonSeamlessCubeMap-06788", device, create_info_loc.dot(Field::flags),
                         "includes VK_SAMPLER_CREATE_NON_SEAMLESS_CUBE_MAP_BIT_EXT but the "
                         "nonSeamlessCubeMap feature was not enabled.");
    }

    if ((pCreateInfo->flags & VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT) &&
        !enabled_features.descriptorBufferCaptureReplay) {
        skip |= LogError("VUID-VkSamplerCreateInfo-flags-08110", device, create_info_loc.dot(Field::flags),
                         "includes VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT but descriptorBufferCaptureReplay "
                         "feature was not enabled.");
    }

    if (vku::FindStructInPNextChain<VkOpaqueCaptureDescriptorDataCreateInfoEXT>(pCreateInfo->pNext)) {
        if (!(pCreateInfo->flags & VK_SAMPLER_CREATE_DESCRIPTOR_BUFFER_CAPTURE_REPLAY_BIT_EXT)) {
            skip |= LogError("VUID-VkSamplerCreateInfo-pNext-08111", device, create_info_loc.dot(Field::flags),
                             "is %s but VkOpaqueCaptureDescriptorDataCreateInfoEXT is in pNext chain.",
                             string_VkSamplerCreateFlags(pCreateInfo->flags).c_str());
        }
    }

    skip |= ValidateSamplerFilterMinMax(*pCreateInfo, create_info_loc);
    skip |= ValidateSamplerCustomBoarderColor(*pCreateInfo, create_info_loc);
    skip |= ValidateSamplerSubsampled(*pCreateInfo, create_info_loc);
    skip |= ValidateSamplerImageProcessingQCOM(*pCreateInfo, create_info_loc);

    return skip;
}

bool Device::ValidateMutableDescriptorTypeCreateInfo(const VkDescriptorSetLayoutCreateInfo &create_info,
                                                     const VkMutableDescriptorTypeCreateInfoEXT &mutable_create_info,
                                                     const Location &create_info_loc) const {
    bool skip = false;

    for (uint32_t i = 0; i < create_info.bindingCount; ++i) {
        const Location binding_loc = create_info_loc.dot(Field::pBindings, i);
        uint32_t mutable_type_count = 0;
        if (mutable_create_info.mutableDescriptorTypeListCount > i) {
            mutable_type_count = mutable_create_info.pMutableDescriptorTypeLists[i].descriptorTypeCount;
        }
        if (create_info.pBindings[i].descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
            if (mutable_type_count == 0) {
                skip |= LogError(
                    "VUID-VkMutableDescriptorTypeListEXT-descriptorTypeCount-04597", device, binding_loc.dot(Field::descriptorType),
                    "is VK_DESCRIPTOR_TYPE_MUTABLE_EXT, but "
                    "VkMutableDescriptorTypeCreateInfoEXT::pMutableDescriptorTypeLists[%" PRIu32 "].descriptorTypeCount is 0.",
                    i);
            }
        } else {
            if (mutable_type_count > 0) {
                skip |= LogError(
                    "VUID-VkMutableDescriptorTypeListEXT-descriptorTypeCount-04599", device, binding_loc.dot(Field::descriptorType),
                    "is %s, but "
                    "VkMutableDescriptorTypeCreateInfoEXT::pMutableDescriptorTypeLists[%" PRIu32 "].descriptorTypeCount is not 0.",
                    string_VkDescriptorType(create_info.pBindings[i].descriptorType), i);
            }
        }
    }

    for (uint32_t j = 0; j < mutable_create_info.mutableDescriptorTypeListCount; ++j) {
        const Location mutable_loc =
            create_info_loc.pNext(Struct::VkMutableDescriptorTypeCreateInfoEXT, Field::pMutableDescriptorTypeLists, j);
        for (uint32_t k = 0; k < mutable_create_info.pMutableDescriptorTypeLists[j].descriptorTypeCount; ++k) {
            const Location type_loc = mutable_loc.dot(Field::pDescriptorTypes, k);
            switch (mutable_create_info.pMutableDescriptorTypeLists[j].pDescriptorTypes[k]) {
                case VK_DESCRIPTOR_TYPE_MUTABLE_EXT:
                    skip |= LogError("VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04600", device, type_loc,
                                     "is VK_DESCRIPTOR_TYPE_MUTABLE_EXT.");
                    break;
                case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
                    skip |= LogError("VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04601", device, type_loc,
                                     "is VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC.");
                    break;
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
                    skip |= LogError("VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04602", device, type_loc,
                                     "is VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC.");
                    break;
                case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
                    skip |= LogError("VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04603", device, type_loc,
                                     "is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK.");
                    break;
                default:
                    break;
            }
            for (uint32_t l = k + 1; l < mutable_create_info.pMutableDescriptorTypeLists[j].descriptorTypeCount; ++l) {
                if (mutable_create_info.pMutableDescriptorTypeLists[j].pDescriptorTypes[k] ==
                    mutable_create_info.pMutableDescriptorTypeLists[j].pDescriptorTypes[l]) {
                    skip |=
                        LogError("VUID-VkMutableDescriptorTypeListEXT-pDescriptorTypes-04598", device, type_loc,
                                 "and pDescriptorTypes[%" PRIu32 "] are both %s.", l,
                                 string_VkDescriptorType(mutable_create_info.pMutableDescriptorTypeLists[j].pDescriptorTypes[k]));
                }
            }
        }
    }

    return skip;
}

bool Device::ValidateDescriptorSetLayoutCreateInfo(const VkDescriptorSetLayoutCreateInfo &create_info,
                                                   const Location &create_info_loc) const {
    bool skip = false;

    const bool has_descriptor_buffer_flag = (create_info.flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT) != 0;
    const bool has_push_descriptor_flag = (create_info.flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT) != 0;
    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    if (create_info.pBindings != nullptr) {
        const auto *mutable_descriptor_type = vku::FindStructInPNextChain<VkMutableDescriptorTypeCreateInfoEXT>(create_info.pNext);
        for (const auto [i, binding] : vvl::enumerate(create_info.pBindings, create_info.bindingCount)) {
            if (binding.descriptorCount == 0) {
                continue;
            }

            const Location binding_loc = create_info_loc.dot(Field::pBindings, i);
            VkShaderStageFlags stage_flags = binding.stageFlags;
            if (stage_flags != 0) {
                if (stage_flags != VK_SHADER_STAGE_ALL && ((stage_flags & (~AllVkShaderStageFlagBitsExcludingStageAll)) != 0)) {
                    skip |= LogError(
                        "VUID-VkDescriptorSetLayoutBinding-descriptorCount-09465", device, binding_loc.dot(Field::descriptorCount),
                        "is %" PRIu32 " but stageFlags is invalid (0x%" PRIx32 ").", binding.descriptorCount, stage_flags);
                }

                if ((binding.descriptorType == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) &&
                    (stage_flags != VK_SHADER_STAGE_FRAGMENT_BIT)) {
                    skip |= LogError("VUID-VkDescriptorSetLayoutBinding-descriptorType-01510", device,
                                     binding_loc.dot(Field::stageFlags),
                                     "is %s but descriptorType is VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT.",
                                     string_VkShaderStageFlags(stage_flags).c_str());
                }
            }

            if (binding.descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
                if (mutable_descriptor_type) {
                    if (i >= mutable_descriptor_type->mutableDescriptorTypeListCount) {
                        skip |= LogError(
                            "VUID-VkDescriptorSetLayoutCreateInfo-pBindings-07303", device,
                            binding_loc.pNext(Struct::VkMutableDescriptorTypeCreateInfoEXT, Field::mutableDescriptorTypeListCount),
                            "(%" PRIu32 ") is less than %" PRIu32 " but descriptorType is VK_DESCRIPTOR_TYPE_MUTABLE_EXT", i,
                            mutable_descriptor_type->mutableDescriptorTypeListCount);
                    }
                } else {
                    skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-pBindings-07303", device,
                                     binding_loc.dot(Field::descriptorType),
                                     "is VK_DESCRIPTOR_TYPE_MUTABLE_EXT but VkMutableDescriptorTypeCreateInfoEXT is not "
                                     "included in the pNext chain.");
                }
                if (binding.pImmutableSamplers) {
                    skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-descriptorType-04594", device,
                                     binding_loc.dot(Field::descriptorType),
                                     "is VK_DESCRIPTOR_TYPE_MUTABLE_EXT but pImmutableSamplers is not NULL.");
                }
                if (!enabled_features.mutableDescriptorType) {
                    skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-mutableDescriptorType-04595", device,
                                     binding_loc.dot(Field::descriptorType),
                                     "is VK_DESCRIPTOR_TYPE_MUTABLE_EXT but "
                                     "mutableDescriptorType feature was not enabled.");
                }
            }

            if (has_push_descriptor_flag && binding.descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
                skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-04591", device, binding_loc.dot(Field::descriptorType),
                                 "is VK_DESCRIPTOR_TYPE_MUTABLE_EXT, but flags includes "
                                 "VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT.");
            }

            if (has_descriptor_buffer_flag && ((binding.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC) ||
                                               (binding.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC))) {
                skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-08000", device, binding_loc.dot(Field::descriptorType),
                                 "is %s, but flags includes VK_DESCRIPTOR_SET_LAYOUT_CREATE_DESCRIPTOR_BUFFER_BIT_EXT.",
                                 string_VkDescriptorType(binding.descriptorType));
            }
        }

        if (mutable_descriptor_type) {
            skip |= ValidateMutableDescriptorTypeCreateInfo(create_info, *mutable_descriptor_type, create_info_loc);
        }
    }

    const bool has_host_only_pool_flag = (create_info.flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_HOST_ONLY_POOL_BIT_EXT) != 0;
    const bool has_update_after_bind_flag = (create_info.flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT) != 0;
    const bool has_embedded_immutable_sampler_flag =
        (create_info.flags & VK_DESCRIPTOR_SET_LAYOUT_CREATE_EMBEDDED_IMMUTABLE_SAMPLERS_BIT_EXT) != 0;

    if (has_push_descriptor_flag && has_host_only_pool_flag) {
        skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-04590", device, create_info_loc.dot(Field::flags), "is %s.",
                         string_VkDescriptorSetLayoutCreateFlags(create_info.flags).c_str());
    }
    if (has_update_after_bind_flag && has_host_only_pool_flag) {
        skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-04592", device, create_info_loc.dot(Field::flags), "is %s.",
                         string_VkDescriptorSetLayoutCreateFlags(create_info.flags).c_str());
    }
    if (has_host_only_pool_flag && !enabled_features.mutableDescriptorType) {
        skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-04596", device, create_info_loc.dot(Field::flags),
                         "is %s, but mutableDescriptorType feature was not enabled.",
                         string_VkDescriptorSetLayoutCreateFlags(create_info.flags).c_str());
    }

    if (has_embedded_immutable_sampler_flag && !has_descriptor_buffer_flag) {
        skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-08001", device, create_info_loc.dot(Field::flags), "is %s.",
                         string_VkDescriptorSetLayoutCreateFlags(create_info.flags).c_str());
    }

    if (has_descriptor_buffer_flag && has_update_after_bind_flag) {
        skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-08002", device, create_info_loc.dot(Field::flags), "is %s.",
                         string_VkDescriptorSetLayoutCreateFlags(create_info.flags).c_str());
    }

    if (has_descriptor_buffer_flag && has_host_only_pool_flag) {
        skip |= LogError("VUID-VkDescriptorSetLayoutCreateInfo-flags-08003", device, create_info_loc.dot(Field::flags), "is %s.",
                         string_VkDescriptorSetLayoutCreateFlags(create_info.flags).c_str());
    }

    return skip;
}

bool Device::manual_PreCallValidateCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator,
                                                             VkDescriptorSetLayout *pSetLayout, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    skip |= ValidateDescriptorSetLayoutCreateInfo(*pCreateInfo, error_obj.location.dot(Field::pCreateInfo));
    return skip;
}

bool Device::manual_PreCallValidateGetDescriptorSetLayoutSupport(VkDevice device,
                                                                 const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
                                                                 VkDescriptorSetLayoutSupport *pSupport,
                                                                 const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    skip |= ValidateDescriptorSetLayoutCreateInfo(*pCreateInfo, error_obj.location.dot(Field::pCreateInfo));
    return skip;
}

bool Device::manual_PreCallValidateFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
                                                      const VkDescriptorSet *pDescriptorSets, const Context &context) const {
    const auto &error_obj = context.error_obj;
    // Validation for parameters excluded from the generated validation code due to a 'noautovalidity' tag in vk.xml
    // This is an array of handles, where the elements are allowed to be VK_NULL_HANDLE, and does not require any validation beyond
    // ValidateArray()
    return context.ValidateArray(error_obj.location.dot(Field::descriptorSetCount), error_obj.location.dot(Field::pDescriptorSets),
                                 descriptorSetCount, &pDescriptorSets, true, true, kVUIDUndefined,
                                 "VUID-vkFreeDescriptorSets-pDescriptorSets-00310");
}

bool Device::ValidateWriteDescriptorSet(const Context &context, const Location &loc, const uint32_t descriptorWriteCount,
                                        const VkWriteDescriptorSet *pDescriptorWrites) const {
    bool skip = false;
    if (!pDescriptorWrites) {
        return skip;
    }
    const bool is_push_descriptor = IsValueIn(loc.function, {Func::vkCmdPushDescriptorSet, Func::vkCmdPushDescriptorSet2,
                                                             Func::vkCmdPushDescriptorSetKHR, Func::vkCmdPushDescriptorSet2KHR});

    for (uint32_t i = 0; i < descriptorWriteCount; ++i) {
        const Location writes_loc = loc.dot(Field::pDescriptorWrites, i);
        const auto &descriptor_writes = pDescriptorWrites[i];

        // If called from vkCmdPushDescriptorSetKHR, the dstSet member is ignored.
        if (!is_push_descriptor) {
            // dstSet must be a valid VkDescriptorSet handle
            skip |= context.ValidateRequiredHandle(writes_loc.dot(Field::dstSet), descriptor_writes.dstSet);
        }

        const VkDescriptorType descriptor_type = descriptor_writes.descriptorType;
        if ((descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLER) || (descriptor_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) ||
            (descriptor_type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) || (descriptor_type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) ||
            (descriptor_type == VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)) {
            if (descriptor_writes.pImageInfo != nullptr &&  descriptor_type != VK_DESCRIPTOR_TYPE_SAMPLER) {
                for (uint32_t descriptor_index = 0; descriptor_index < descriptor_writes.descriptorCount; ++descriptor_index) {
                    skip |= context.ValidateRangedEnum(writes_loc.dot(Field::pImageInfo, descriptor_index).dot(Field::imageLayout),
                                                       vvl::Enum::VkImageLayout,
                                                       descriptor_writes.pImageInfo[descriptor_index].imageLayout, kVUIDUndefined);
                }
            }
        }
    }
    return skip;
}

bool Device::manual_PreCallValidateUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount,
                                                        const VkWriteDescriptorSet *pDescriptorWrites, uint32_t descriptorCopyCount,
                                                        const VkCopyDescriptorSet *pDescriptorCopies,
                                                        const Context &context) const {
    const auto &error_obj = context.error_obj;
    return ValidateWriteDescriptorSet(context, error_obj.location, descriptorWriteCount, pDescriptorWrites);
}

static bool MutableDescriptorTypePartialOverlap(const VkDescriptorPoolCreateInfo *pCreateInfo, uint32_t i, uint32_t j) {
    bool partial_overlap = false;

    constexpr std::array all_descriptor_types = {
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
        VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,
        VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
        VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV,
    };

    const auto *mutable_descriptor_type = vku::FindStructInPNextChain<VkMutableDescriptorTypeCreateInfoEXT>(pCreateInfo->pNext);
    if (mutable_descriptor_type) {
        vvl::span<const VkDescriptorType> first_types, second_types;

        if (mutable_descriptor_type->mutableDescriptorTypeListCount > i) {
            const uint32_t descriptor_type_count = mutable_descriptor_type->pMutableDescriptorTypeLists[i].descriptorTypeCount;
            auto *descriptor_types = mutable_descriptor_type->pMutableDescriptorTypeLists[i].pDescriptorTypes;
            first_types = vvl::make_span(descriptor_types, descriptor_type_count);
        } else {
            first_types = vvl::make_span(all_descriptor_types.data(), all_descriptor_types.size());
        }

        if (mutable_descriptor_type->mutableDescriptorTypeListCount > j) {
            const uint32_t descriptor_type_count = mutable_descriptor_type->pMutableDescriptorTypeLists[j].descriptorTypeCount;
            auto *descriptor_types = mutable_descriptor_type->pMutableDescriptorTypeLists[j].pDescriptorTypes;
            second_types = vvl::make_span(descriptor_types, descriptor_type_count);
        } else {
            second_types = vvl::make_span(all_descriptor_types.data(), all_descriptor_types.size());
        }

        bool complete_overlap = first_types.size() == second_types.size();
        bool disjoint = true;
        for (const auto first_type : first_types) {
            bool found = false;
            for (const auto second_type : second_types) {
                if (first_type == second_type) {
                    found = true;
                    break;
                }
            }
            if (found) {
                disjoint = false;
            } else {
                complete_overlap = false;
            }
            if (!disjoint && !complete_overlap) {
                partial_overlap = true;
                break;
            }
        }
    }

    return partial_overlap;
}

bool Device::manual_PreCallValidateCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
                                                        const VkAllocationCallbacks *pAllocator, VkDescriptorPool *pDescriptorPool,
                                                        const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);
    if (pCreateInfo->maxSets == 0 && ((pCreateInfo->flags & VK_DESCRIPTOR_POOL_CREATE_ALLOW_OVERALLOCATION_SETS_BIT_NV) == 0)) {
        skip |= LogError("VUID-VkDescriptorPoolCreateInfo-descriptorPoolOverallocation-09227", device,
                         create_info_loc.dot(Field::maxSets), "is zero.");
    }

    const auto *inline_uniform_info = vku::FindStructInPNextChain<VkDescriptorPoolInlineUniformBlockCreateInfo>(pCreateInfo->pNext);
    const bool non_zero_inline_uniform_count = inline_uniform_info && inline_uniform_info->maxInlineUniformBlockBindings != 0;

    if (pCreateInfo->pPoolSizes) {
        for (uint32_t i = 0; i < pCreateInfo->poolSizeCount; ++i) {
            const Location pool_loc = create_info_loc.dot(Field::pPoolSizes, i);
            if (pCreateInfo->pPoolSizes[i].descriptorCount <= 0) {
                skip |= LogError("VUID-VkDescriptorPoolSize-descriptorCount-00302", device, pool_loc.dot(Field::descriptorCount),
                                 "is zero.");
            }
            if (pCreateInfo->pPoolSizes[i].type == VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK) {
                if (SafeModulo(pCreateInfo->pPoolSizes[i].descriptorCount, 4) != 0) {
                    skip |= LogError("VUID-VkDescriptorPoolSize-type-02218", device, pool_loc.dot(Field::descriptorCount),
                                     "is %" PRIu32 " (not a multiple of 4), but type is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK.",
                                     pCreateInfo->pPoolSizes[i].descriptorCount);
                }
                if (!non_zero_inline_uniform_count) {
                    skip |=
                        LogError("VUID-VkDescriptorPoolCreateInfo-pPoolSizes-09424", device, pool_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK but no maxInlineUniformBlockBindings was provided.");
                }
            }
            if (pCreateInfo->pPoolSizes[i].type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT && !enabled_features.mutableDescriptorType) {
                skip |= LogError("VUID-VkDescriptorPoolCreateInfo-mutableDescriptorType-04608", device, pool_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_MUTABLE_EXT "
                                 ", but mutableDescriptorType feature was not enabled.");
            }
            if (pCreateInfo->pPoolSizes[i].type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
                for (uint32_t j = i + 1; j < pCreateInfo->poolSizeCount; ++j) {
                    if (pCreateInfo->pPoolSizes[j].type == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
                        if (MutableDescriptorTypePartialOverlap(pCreateInfo, i, j)) {
                            skip |= LogError("VUID-VkDescriptorPoolCreateInfo-pPoolSizes-04787", device, pool_loc.dot(Field::type),
                                             "and pPoolSizes[%" PRIu32
                                             "].type are both VK_DESCRIPTOR_TYPE_MUTABLE_EXT "
                                             " and have sets which partially overlap.",
                                             j);
                        }
                    }
                }
            }
        }
    }

    if (pCreateInfo->flags & VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT && !enabled_features.mutableDescriptorType) {
        skip |= LogError("VUID-VkDescriptorPoolCreateInfo-flags-04609", device, create_info_loc.dot(Field::flags),
                         "includes VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT, "
                         "but mutableDescriptorType feature was not enabled.");
    }
    if ((pCreateInfo->flags & VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT) &&
        (pCreateInfo->flags & VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT)) {
        skip |= LogError("VUID-VkDescriptorPoolCreateInfo-flags-04607", device, create_info_loc.dot(Field::flags),
                         "includes both "
                         "VK_DESCRIPTOR_POOL_CREATE_HOST_ONLY_BIT_EXT and VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT");
    }

    return skip;
}

bool Device::manual_PreCallValidateCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool,
                                                   const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    switch (pCreateInfo->queryType) {
        case VK_QUERY_TYPE_PIPELINE_STATISTICS: {
            if (!enabled_features.pipelineStatisticsQuery) {
                skip |= LogError("VUID-VkQueryPoolCreateInfo-queryType-00791", device, create_info_loc.dot(Field::queryType),
                                 "is VK_QUERY_TYPE_PIPELINE_STATISTICS but pipelineStatisticsQuery feature was not enabled.");
            } else if ((pCreateInfo->pipelineStatistics & (VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT |
                                                           VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT)) &&
                       !enabled_features.meshShaderQueries) {
                skip |= LogError("VUID-VkQueryPoolCreateInfo-meshShaderQueries-07069", device,
                                 create_info_loc.dot(Field::pipelineStatistics),
                                 "(%s) contains mesh/task shader bit, but "
                                 "meshShaderQueries feature was not enabled.",
                                 string_VkQueryPipelineStatisticFlags(pCreateInfo->pipelineStatistics).c_str());
            }

            if (pCreateInfo->pipelineStatistics == 0) {
                skip |= LogError("VUID-VkQueryPoolCreateInfo-queryType-09534", device, create_info_loc.dot(Field::queryType),
                                 "is VK_QUERY_TYPE_PIPELINE_STATISTICS, but pipelineStatistics is zero");
            } else if ((pCreateInfo->pipelineStatistics & (~AllVkQueryPipelineStatisticFlagBits)) != 0) {
                skip |= LogError("VUID-VkQueryPoolCreateInfo-queryType-00792", device, create_info_loc.dot(Field::queryType),
                                 "is VK_QUERY_TYPE_PIPELINE_STATISTICS, but "
                                 "pipelineStatistics must be a valid combination of VkQueryPipelineStatisticFlagBits "
                                 "values.");
            }
            break;
        }
        case VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR: {
            if (!enabled_features.performanceCounterQueryPools) {
                skip |=
                    LogError("VUID-VkQueryPoolPerformanceCreateInfoKHR-performanceCounterQueryPools-03237", device,
                             create_info_loc.dot(Field::queryType),
                             "is VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR but performanceCounterQueryPools feature was not enabled.");
            }
            if (!vku::FindStructInPNextChain<VkQueryPoolPerformanceCreateInfoKHR>(pCreateInfo->pNext)) {
                skip |= LogError("VUID-VkQueryPoolCreateInfo-queryType-03222", device, create_info_loc.dot(Field::queryType),
                                 "is VK_QUERY_TYPE_PERFORMANCE_QUERY_KHR, but the pNext chain does not contain an instance of "
                                 "VkQueryPoolPerformanceCreateInfoKHR.");
            }
            break;
        }
        case VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT: {
            if (!enabled_features.meshShaderQueries) {
                skip |=
                    LogError("VUID-VkQueryPoolCreateInfo-meshShaderQueries-07068", device, create_info_loc.dot(Field::queryType),
                             "is VK_QUERY_TYPE_MESH_PRIMITIVES_GENERATED_EXT but meshShaderQueries feature was not enabled.");
            }
            break;
        }
        default:
            break;
    }

    if (pCreateInfo->queryCount == 0) {
        skip |= LogError("VUID-VkQueryPoolCreateInfo-queryCount-02763", device, create_info_loc.dot(Field::queryCount), "is zero.");
    }
    return skip;
}

bool Device::manual_PreCallValidateCreateSamplerYcbcrConversion(VkDevice device,
                                                                const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkSamplerYcbcrConversion *pYcbcrConversion,
                                                                const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    // Check samplerYcbcrConversion feature is set
    if (!enabled_features.samplerYcbcrConversion) {
        skip |= LogError("VUID-vkCreateSamplerYcbcrConversion-None-01648", device, error_obj.location,
                         "samplerYcbcrConversion feature must be enabled.");
    }

    const VkFormat format = pCreateInfo->format;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    // If there is a VkExternalFormatANDROID with externalFormat != 0, the value of components is ignored.
    if (GetExternalFormat(pCreateInfo->pNext) != 0) {
        return skip;
    }
    const VkComponentMapping components = pCreateInfo->components;
    // XChroma Subsampled is same as "the format has a _422 or _420 suffix" from spec
    if (vkuFormatIsXChromaSubsampled(format) == true) {
        if ((components.g != VK_COMPONENT_SWIZZLE_G) && (components.g != VK_COMPONENT_SWIZZLE_IDENTITY)) {
            skip |= LogError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02581", device, create_info_loc,
                             "When using a XChroma subsampled format (%s) the components.g (%s) needs to be VK_COMPONENT_SWIZZLE_G "
                             "or VK_COMPONENT_SWIZZLE_IDENTITY.",
                             string_VkFormat(format), string_VkComponentSwizzle(components.g));
        }

        if ((components.a != VK_COMPONENT_SWIZZLE_A) && (components.a != VK_COMPONENT_SWIZZLE_IDENTITY) &&
            (components.a != VK_COMPONENT_SWIZZLE_ONE) && (components.a != VK_COMPONENT_SWIZZLE_ZERO)) {
            skip |=
                LogError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02582", device, create_info_loc,
                         " When using a XChroma subsampled format (%s) the components.a (%s) needs to be VK_COMPONENT_SWIZZLE_A or "
                         "VK_COMPONENT_SWIZZLE_IDENTITY or VK_COMPONENT_SWIZZLE_ONE or VK_COMPONENT_SWIZZLE_ZERO.",
                         string_VkFormat(format), string_VkComponentSwizzle(components.a));
        }

        if ((components.r != VK_COMPONENT_SWIZZLE_R) && (components.r != VK_COMPONENT_SWIZZLE_IDENTITY) &&
            (components.r != VK_COMPONENT_SWIZZLE_B)) {
            skip |= LogError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02583", device, create_info_loc,
                             "When using a XChroma subsampled format (%s) the components.r (%s) needs to be VK_COMPONENT_SWIZZLE_R "
                             "or VK_COMPONENT_SWIZZLE_IDENTITY or VK_COMPONENT_SWIZZLE_B.",
                             string_VkFormat(format), string_VkComponentSwizzle(components.r));
        }

        if ((components.b != VK_COMPONENT_SWIZZLE_B) && (components.b != VK_COMPONENT_SWIZZLE_IDENTITY) &&
            (components.b != VK_COMPONENT_SWIZZLE_R)) {
            skip |= LogError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02584", device, create_info_loc,
                             "When using a XChroma subsampled format (%s) the components.b (%s) needs to be VK_COMPONENT_SWIZZLE_B "
                             "or VK_COMPONENT_SWIZZLE_IDENTITY or VK_COMPONENT_SWIZZLE_R.",
                             string_VkFormat(format), string_VkComponentSwizzle(components.b));
        }

        // If one is identity, both need to be
        const bool r_identity = ((components.r == VK_COMPONENT_SWIZZLE_R) || (components.r == VK_COMPONENT_SWIZZLE_IDENTITY));
        const bool b_identity = ((components.b == VK_COMPONENT_SWIZZLE_B) || (components.b == VK_COMPONENT_SWIZZLE_IDENTITY));
        if ((r_identity != b_identity) && ((r_identity == true) || (b_identity == true))) {
            skip |=
                LogError("VUID-VkSamplerYcbcrConversionCreateInfo-components-02585", device, create_info_loc,
                         "When using a XChroma subsampled format (%s) if either the components.r (%s) or components.b (%s) "
                         "are an identity swizzle, then both need to be an identity swizzle.",
                         string_VkFormat(format), string_VkComponentSwizzle(components.r), string_VkComponentSwizzle(components.b));
        }
    }

    if (pCreateInfo->ycbcrModel != VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY) {
        // Checks same VU multiple ways in order to give a more useful error message
        const char *vuid = "VUID-VkSamplerYcbcrConversionCreateInfo-ycbcrModel-01655";
        if ((components.r == VK_COMPONENT_SWIZZLE_ONE) || (components.r == VK_COMPONENT_SWIZZLE_ZERO) ||
            (components.g == VK_COMPONENT_SWIZZLE_ONE) || (components.g == VK_COMPONENT_SWIZZLE_ZERO) ||
            (components.b == VK_COMPONENT_SWIZZLE_ONE) || (components.b == VK_COMPONENT_SWIZZLE_ZERO)) {
            skip |=
                LogError(vuid, device, create_info_loc,
                         "The ycbcrModel (%s) is not VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY so components.r (%s), "
                         "components.g (%s), nor components.b (%s) can't be VK_COMPONENT_SWIZZLE_ZERO or VK_COMPONENT_SWIZZLE_ONE.",
                         string_VkSamplerYcbcrModelConversion(pCreateInfo->ycbcrModel), string_VkComponentSwizzle(components.r),
                         string_VkComponentSwizzle(components.g), string_VkComponentSwizzle(components.b));
        }

        // "must not correspond to a component which contains zero or one as a consequence of conversion to RGBA"
        // 4 component format = no issue
        // 3 = no [a]
        // 2 = no [b,a]
        // 1 = no [g,b,a]
        // depth/stencil = no [g,b,a] (shouldn't ever occur, but no VU preventing it)
        const uint32_t component_count = (vkuFormatIsDepthOrStencil(format) == true) ? 1 : vkuFormatComponentCount(format);

        if ((component_count < 4) && ((components.r == VK_COMPONENT_SWIZZLE_A) || (components.g == VK_COMPONENT_SWIZZLE_A) ||
                                      (components.b == VK_COMPONENT_SWIZZLE_A))) {
            skip |= LogError(vuid, device, create_info_loc,
                             "The ycbcrModel (%s) is not VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY so components.r (%s), "
                             "components.g (%s), or components.b (%s) can't be VK_COMPONENT_SWIZZLE_A.",
                             string_VkSamplerYcbcrModelConversion(pCreateInfo->ycbcrModel), string_VkComponentSwizzle(components.r),
                             string_VkComponentSwizzle(components.g), string_VkComponentSwizzle(components.b));
        } else if ((component_count < 3) &&
                   ((components.r == VK_COMPONENT_SWIZZLE_B) || (components.g == VK_COMPONENT_SWIZZLE_B) ||
                    (components.b == VK_COMPONENT_SWIZZLE_B) || (components.b == VK_COMPONENT_SWIZZLE_IDENTITY))) {
            skip |= LogError(vuid, device, create_info_loc,
                             "The ycbcrModel (%s) is not VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY so components.r (%s), "
                             "components.g (%s), or components.b (%s) can't be VK_COMPONENT_SWIZZLE_B "
                             "(components.b also can't be VK_COMPONENT_SWIZZLE_IDENTITY).",
                             string_VkSamplerYcbcrModelConversion(pCreateInfo->ycbcrModel), string_VkComponentSwizzle(components.r),
                             string_VkComponentSwizzle(components.g), string_VkComponentSwizzle(components.b));
        } else if ((component_count < 2) &&
                   ((components.r == VK_COMPONENT_SWIZZLE_G) || (components.g == VK_COMPONENT_SWIZZLE_G) ||
                    (components.g == VK_COMPONENT_SWIZZLE_IDENTITY) || (components.b == VK_COMPONENT_SWIZZLE_G))) {
            skip |= LogError(vuid, device, create_info_loc,
                             "The ycbcrModel (%s) is not VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY so components.r (%s), "
                             "components.g (%s), or components.b (%s) can't be VK_COMPONENT_SWIZZLE_G "
                             "(components.g also can't be VK_COMPONENT_SWIZZLE_IDENTITY).",
                             string_VkSamplerYcbcrModelConversion(pCreateInfo->ycbcrModel), string_VkComponentSwizzle(components.r),
                             string_VkComponentSwizzle(components.g), string_VkComponentSwizzle(components.b));
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateGetDescriptorEXT(VkDevice device, const VkDescriptorGetInfoEXT *pDescriptorInfo, size_t dataSize,
                                                    void *pDescriptor, const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (!enabled_features.descriptorBuffer) {
        skip |=
            LogError("VUID-vkGetDescriptorEXT-None-08015", device, error_obj.location, "descriptorBuffer feature was not enabled.");
    }

    const Location descriptor_info_loc = error_obj.location.dot(Field::pDescriptorInfo);
    const Location data_loc = descriptor_info_loc.dot(Field::data);
    const VkDescriptorAddressInfoEXT *address_info = nullptr;
    Field data_field = Field::Empty;
    switch (pDescriptorInfo->type) {
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC:
        case VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK:
            skip |= LogError("VUID-VkDescriptorGetInfoEXT-type-08018", device, descriptor_info_loc.dot(Field::type), "is %s.",
                             string_VkDescriptorType(pDescriptorInfo->type));
            break;
        case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
            if (!pDescriptorInfo->data.pCombinedImageSampler) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-pCombinedImageSampler-parameter", device,
                                 descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, but pCombinedImageSampler is null.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT:
            if (!pDescriptorInfo->data.pInputAttachmentImage) {
                skip |= LogError("VUID-VkDescriptorGetInfoEXT-pInputAttachmentImage-parameter", device,
                                 descriptor_info_loc.dot(Field::type),
                                 "is VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, but pInputAttachmentImage is null.");
            }
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER:
            if (pDescriptorInfo->data.pUniformTexelBuffer) {
                address_info = pDescriptorInfo->data.pUniformTexelBuffer;
                data_field = Field::pUniformTexelBuffer;
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER:
            if (pDescriptorInfo->data.pStorageTexelBuffer) {
                address_info = pDescriptorInfo->data.pStorageTexelBuffer;
                data_field = Field::pStorageTexelBuffer;
            }
            break;
        case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
            if (pDescriptorInfo->data.pUniformBuffer) {
                address_info = pDescriptorInfo->data.pUniformBuffer;
                data_field = Field::pUniformBuffer;
            }
            break;
        case VK_DESCRIPTOR_TYPE_STORAGE_BUFFER:
            if (pDescriptorInfo->data.pStorageBuffer) {
                address_info = pDescriptorInfo->data.pStorageBuffer;
                data_field = Field::pStorageBuffer;
            }
            break;
        default:
            break;
    }

    if (address_info) {
        const Location address_loc = data_loc.dot(data_field);
        skip |= ValidateDescriptorAddressInfoEXT(context, *address_info, address_loc);

        if (address_info->address != 0) {
            if ((pDescriptorInfo->type == VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER ||
                 pDescriptorInfo->type == VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER) &&
                address_info->format == VK_FORMAT_UNDEFINED) {
                skip |= LogError("VUID-VkDescriptorAddressInfoEXT-None-09508", device, address_loc.dot(Field::format),
                                 "is VK_FORMAT_UNDEFINED.");
            }
        }
    }
    return skip;
}

bool Device::manual_PreCallValidateCmdSetDescriptorBufferOffsets2EXT(
    VkCommandBuffer commandBuffer, const VkSetDescriptorBufferOffsetsInfoEXT *pSetDescriptorBufferOffsetsInfo,
    const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (pSetDescriptorBufferOffsetsInfo->layout == VK_NULL_HANDLE) {
        if (!enabled_features.dynamicPipelineLayout) {
            skip |=
                LogError("VUID-VkSetDescriptorBufferOffsetsInfoEXT-None-09495", commandBuffer,
                         error_obj.location.dot(Field::pSetDescriptorBufferOffsetsInfo).dot(Field::layout), "is VK_NULL_HANDLE.");
        } else if (!vku::FindStructInPNextChain<VkPipelineLayoutCreateInfo>(pSetDescriptorBufferOffsetsInfo->pNext)) {
            skip |= LogError("VUID-VkSetDescriptorBufferOffsetsInfoEXT-layout-09496", commandBuffer,
                             error_obj.location.dot(Field::pSetDescriptorBufferOffsetsInfo).dot(Field::layout),
                             "is VK_NULL_HANDLE and pNext is missing VkPipelineLayoutCreateInfo.");
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdBindDescriptorBufferEmbeddedSamplers2EXT(
    VkCommandBuffer commandBuffer, const VkBindDescriptorBufferEmbeddedSamplersInfoEXT *pBindDescriptorBufferEmbeddedSamplersInfo,
    const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (pBindDescriptorBufferEmbeddedSamplersInfo->layout == VK_NULL_HANDLE) {
        if (!enabled_features.dynamicPipelineLayout) {
            skip |= LogError("VUID-VkBindDescriptorBufferEmbeddedSamplersInfoEXT-None-09495", commandBuffer,
                             error_obj.location.dot(Field::pBindDescriptorBufferEmbeddedSamplersInfo).dot(Field::layout),
                             "is VK_NULL_HANDLE.");
        } else if (!vku::FindStructInPNextChain<VkPipelineLayoutCreateInfo>(pBindDescriptorBufferEmbeddedSamplersInfo->pNext)) {
            skip |= LogError("VUID-VkBindDescriptorBufferEmbeddedSamplersInfoEXT-layout-09496", commandBuffer,
                             error_obj.location.dot(Field::pBindDescriptorBufferEmbeddedSamplersInfo).dot(Field::layout),
                             "is VK_NULL_HANDLE and pNext is missing VkPipelineLayoutCreateInfo.");
        }
    }

    return skip;
}

bool Device::manual_PreCallValidateCmdPushDescriptorSetWithTemplate2(
    VkCommandBuffer commandBuffer, const VkPushDescriptorSetWithTemplateInfo *pPushDescriptorSetWithTemplateInfo,
    const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;

    if (pPushDescriptorSetWithTemplateInfo->layout == VK_NULL_HANDLE) {
        if (!enabled_features.dynamicPipelineLayout) {
            skip |= LogError("VUID-VkPushDescriptorSetWithTemplateInfo-None-09495", commandBuffer,
                             error_obj.location.dot(Field::pPushDescriptorSetWithTemplateInfo).dot(Field::layout),
                             "is VK_NULL_HANDLE.");
        } else if (!vku::FindStructInPNextChain<VkPipelineLayoutCreateInfo>(pPushDescriptorSetWithTemplateInfo->pNext)) {
            skip |= LogError("VUID-VkPushDescriptorSetWithTemplateInfo-layout-09496", commandBuffer,
                             error_obj.location.dot(Field::pPushDescriptorSetWithTemplateInfo).dot(Field::layout),
                             "is VK_NULL_HANDLE and pNext is missing VkPipelineLayoutCreateInfo.");
        }
    }
    return skip;
}

bool Device::manual_PreCallValidateCmdBindDescriptorSets2(VkCommandBuffer commandBuffer,
                                                          const VkBindDescriptorSetsInfoKHR *pBindDescriptorSetsInfo,
                                                          const Context &context) const {
    bool skip = false;
    const auto &error_obj = context.error_obj;
    if (pBindDescriptorSetsInfo->layout == VK_NULL_HANDLE) {
        if (!enabled_features.dynamicPipelineLayout) {
            skip |= LogError("VUID-VkBindDescriptorSetsInfo-None-09495", commandBuffer,
                             error_obj.location.dot(Field::pBindDescriptorSetsInfo).dot(Field::layout), "is VK_NULL_HANDLE.");
        } else if (!vku::FindStructInPNextChain<VkPipelineLayoutCreateInfo>(pBindDescriptorSetsInfo->pNext)) {
            skip |= LogError("VUID-VkBindDescriptorSetsInfo-layout-09496", commandBuffer,
                             error_obj.location.dot(Field::pBindDescriptorSetsInfo).dot(Field::layout),
                             "is VK_NULL_HANDLE and pNext is missing VkPipelineLayoutCreateInfo.");
        }
    }

    return skip;
}
}  // namespace stateless
