/* Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (C) 2015-2023 Google Inc.
 * Modifications Copyright (C) 2020-2022 Advanced Micro Devices, Inc. All rights reserved.
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
 *
 */

#include "core_validation.h"

// There is a table in the Vulkan spec to list all formats that implicitly require YCbCr conversion,
// but some features/extensions can explicitly turn that restriction off
// The implicit check is done in format utils, while feature checks are done here in CoreChecks
bool CoreChecks::FormatRequiresYcbcrConversionExplicitly(const VkFormat format) const {
    // VK_EXT_rgba10x6_formats
    if (format == VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16 && enabled_features.formatRgba10x6WithoutYCbCrSampler) {
        return false;
    }
    return vkuFormatRequiresYcbcrConversion(format);
}

bool CoreChecks::PreCallValidateCreateSamplerYcbcrConversion(VkDevice device, const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator,
                                                             VkSamplerYcbcrConversion *pYcbcrConversion,
                                                             const ErrorObject &error_obj) const {
    bool skip = false;
    const VkFormat conversion_format = pCreateInfo->format;
    const Location create_info_loc = error_obj.location.dot(Field::pCreateInfo);

    // Need to check for external format conversion first as it allows for non-UNORM format
    const uint64_t external_format = GetExternalFormat(pCreateInfo->pNext);
    if (external_format != 0) {
        if (VK_FORMAT_UNDEFINED != conversion_format) {
            return LogError("VUID-VkSamplerYcbcrConversionCreateInfo-format-01904", device, create_info_loc.dot(Field::format),
                            "(%s) is not VK_FORMAT_UNDEFINED while "
                            "there is a chained VkExternalFormatANDROID struct with a non-zero externalFormat.",
                            string_VkFormat(conversion_format));
        }
    } else if (vkuFormatIsUNORM(conversion_format) == false) {
        skip |= LogError("VUID-VkSamplerYcbcrConversionCreateInfo-format-04061", device, create_info_loc.dot(Field::format),
                         "(%s) is not an UNORM format and there is no external format conversion being created.",
                         string_VkFormat(conversion_format));
    }

    // Gets VkFormatFeatureFlags according to Sampler Ycbcr Conversion Format Features
    // (vkspec.html#potential-format-features)
    VkFormatFeatureFlags2KHR format_features = ~0ULL;
    if (conversion_format == VK_FORMAT_UNDEFINED) {
        // only check for external format inside VK_FORMAT_UNDEFINED check to prevent unnecessary extra errors from no format
        // features being supported
        if (external_format != 0) {
            auto it = ahb_ext_formats_map.find(external_format);
            if (it != ahb_ext_formats_map.end()) {
                format_features = it->second;
            }
        }
    } else {
        format_features = GetPotentialFormatFeatures(conversion_format);
    }

    // Check all VUID that are based off of VkFormatFeatureFlags
    // These can't be in StatelessValidation due to needing possible External AHB state for feature support
    if (((format_features & VK_FORMAT_FEATURE_2_MIDPOINT_CHROMA_SAMPLES_BIT) == 0) &&
        ((format_features & VK_FORMAT_FEATURE_2_COSITED_CHROMA_SAMPLES_BIT) == 0)) {
        skip |= LogError("VUID-VkSamplerYcbcrConversionCreateInfo-format-01650", device, create_info_loc.dot(Field::format),
                         "(%s) does not support either VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT or "
                         "VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT",
                         string_VkFormat(conversion_format));
    }
    if ((format_features & VK_FORMAT_FEATURE_2_COSITED_CHROMA_SAMPLES_BIT) == 0) {
        if (vkuFormatIsXChromaSubsampled(conversion_format) && pCreateInfo->xChromaOffset == VK_CHROMA_LOCATION_COSITED_EVEN) {
            skip |=
                LogError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651", device, create_info_loc.dot(Field::format),
                         "(%s) does not support VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT so xChromaOffset can't "
                         "be VK_CHROMA_LOCATION_COSITED_EVEN",
                         string_VkFormat(conversion_format));
        }
        if (vkuFormatIsYChromaSubsampled(conversion_format) && pCreateInfo->yChromaOffset == VK_CHROMA_LOCATION_COSITED_EVEN) {
            skip |=
                LogError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01651", device, create_info_loc.dot(Field::format),
                         "(%s) does not support VK_FORMAT_FEATURE_COSITED_CHROMA_SAMPLES_BIT so yChromaOffset can't "
                         "be VK_CHROMA_LOCATION_COSITED_EVEN",
                         string_VkFormat(conversion_format));
        }
    }
    if ((format_features & VK_FORMAT_FEATURE_2_MIDPOINT_CHROMA_SAMPLES_BIT) == 0) {
        if (vkuFormatIsXChromaSubsampled(conversion_format) && pCreateInfo->xChromaOffset == VK_CHROMA_LOCATION_MIDPOINT) {
            skip |=
                LogError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01652", device, create_info_loc.dot(Field::format),
                         "(%s) does not support VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT so xChromaOffset can't "
                         "be VK_CHROMA_LOCATION_MIDPOINT",
                         string_VkFormat(conversion_format));
        }
        if (vkuFormatIsYChromaSubsampled(conversion_format) && pCreateInfo->yChromaOffset == VK_CHROMA_LOCATION_MIDPOINT) {
            skip |=
                LogError("VUID-VkSamplerYcbcrConversionCreateInfo-xChromaOffset-01652", device, create_info_loc.dot(Field::format),
                         "(%s) does not support VK_FORMAT_FEATURE_MIDPOINT_CHROMA_SAMPLES_BIT so yChromaOffset can't "
                         "be VK_CHROMA_LOCATION_MIDPOINT",
                         string_VkFormat(conversion_format));
        }
    }
    if (((format_features & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT) ==
         0) &&
        (pCreateInfo->forceExplicitReconstruction == VK_TRUE)) {
        skip |= LogError("VUID-VkSamplerYcbcrConversionCreateInfo-forceExplicitReconstruction-01656", device,
                         create_info_loc.dot(Field::format),
                         "(%s) does not support "
                         "VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_CHROMA_RECONSTRUCTION_EXPLICIT_FORCEABLE_BIT so "
                         "forceExplicitReconstruction must be VK_FALSE",
                         string_VkFormat(conversion_format));
    }
    if (((format_features & VK_FORMAT_FEATURE_2_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT) == 0) &&
        (pCreateInfo->chromaFilter == VK_FILTER_LINEAR)) {
        skip |= LogError("VUID-VkSamplerYcbcrConversionCreateInfo-chromaFilter-01657", device, create_info_loc.dot(Field::format),
                         "(%s) does not support VK_FORMAT_FEATURE_SAMPLED_IMAGE_YCBCR_CONVERSION_LINEAR_FILTER_BIT so "
                         "chromaFilter must not be VK_FILTER_LINEAR",
                         string_VkFormat(conversion_format));
    }

    return skip;
}

bool CoreChecks::PreCallValidateCreateSamplerYcbcrConversionKHR(VkDevice device,
                                                                const VkSamplerYcbcrConversionCreateInfo *pCreateInfo,
                                                                const VkAllocationCallbacks *pAllocator,
                                                                VkSamplerYcbcrConversion *pYcbcrConversion,
                                                                const ErrorObject &error_obj) const {
    return PreCallValidateCreateSamplerYcbcrConversion(device, pCreateInfo, pAllocator, pYcbcrConversion, error_obj);
}
