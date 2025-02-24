/* Copyright (c) 2024 The Khronos Group Inc.
 * Copyright (c) 2024 Valve Corporation
 * Copyright (c) 2024 LunarG, Inc.
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

#include "utils/vk_struct_compare.h"
#include "utils/vk_layer_utils.h"
#include <vulkan/utility/vk_struct_helper.hpp>

static inline bool ComparePipelineSampleLocationsStateCreateInfo(const VkPipelineSampleLocationsStateCreateInfoEXT &a,
                                                                 const VkPipelineSampleLocationsStateCreateInfoEXT &b) {
    // Having VkSampleLocationEXT not confirmed to matter for the VU this is being used for, so just check the Count is good enough
    return (a.sampleLocationsEnable == b.sampleLocationsEnable) &&
           (a.sampleLocationsInfo.sampleLocationsPerPixel == b.sampleLocationsInfo.sampleLocationsPerPixel) &&
           (a.sampleLocationsInfo.sampleLocationGridSize.height == b.sampleLocationsInfo.sampleLocationGridSize.height) &&
           (a.sampleLocationsInfo.sampleLocationGridSize.width == b.sampleLocationsInfo.sampleLocationGridSize.width) &&
           (a.sampleLocationsInfo.sampleLocationsCount == b.sampleLocationsInfo.sampleLocationsCount);
}

bool ComparePipelineMultisampleStateCreateInfo(const VkPipelineMultisampleStateCreateInfo &a,
                                               const VkPipelineMultisampleStateCreateInfo &b) {
    bool valid_mask = true;
    if (a.pSampleMask && b.pSampleMask && (a.rasterizationSamples == b.rasterizationSamples)) {
        uint32_t length = (SampleCountSize(a.rasterizationSamples) + 31) / 32;
        for (uint32_t i = 0; i < length; i++) {
            if (a.pSampleMask[i] != b.pSampleMask[i]) {
                valid_mask = false;
                break;
            }
        }
    } else if (a.pSampleMask || b.pSampleMask) {
        valid_mask = false;  // one is not null
    }

    bool valid_pNext = true;
    if (a.pNext && b.pNext) {
        auto *a_sample_location = vku::FindStructInPNextChain<VkPipelineSampleLocationsStateCreateInfoEXT>(a.pNext);
        auto *b_sample_location = vku::FindStructInPNextChain<VkPipelineSampleLocationsStateCreateInfoEXT>(b.pNext);
        if (a_sample_location && b_sample_location) {
            if (!ComparePipelineSampleLocationsStateCreateInfo(*a_sample_location, *b_sample_location)) {
                valid_pNext = false;
            }
        } else if (a_sample_location != b_sample_location) {
            valid_pNext = false;  // both are not null
        }
    } else if (a.pNext != b.pNext) {
        valid_pNext = false;  // both are not null
    }

    return (a.sType == b.sType) && (valid_pNext) && (a.flags == b.flags) && (a.rasterizationSamples == b.rasterizationSamples) &&
           (a.sampleShadingEnable == b.sampleShadingEnable) && (a.minSampleShading == b.minSampleShading) && (valid_mask) &&
           (a.alphaToCoverageEnable == b.alphaToCoverageEnable) && (a.alphaToOneEnable == b.alphaToOneEnable);
}

bool CompareDescriptorSetLayoutBinding(const VkDescriptorSetLayoutBinding &a, const VkDescriptorSetLayoutBinding &b) {
    return (a.binding == b.binding) && (a.descriptorType == b.descriptorType) && (a.descriptorCount == b.descriptorCount) &&
           (a.stageFlags == b.stageFlags) && (a.pImmutableSamplers == b.pImmutableSamplers);
}

bool ComparePipelineColorBlendAttachmentState(const VkPipelineColorBlendAttachmentState &a,
                                              const VkPipelineColorBlendAttachmentState &b) {
    return (a.blendEnable == b.blendEnable) && (a.srcColorBlendFactor == b.srcColorBlendFactor) &&
           (a.dstColorBlendFactor == b.dstColorBlendFactor) && (a.colorBlendOp == b.colorBlendOp) &&
           (a.srcAlphaBlendFactor == b.srcAlphaBlendFactor) && (a.dstAlphaBlendFactor == b.dstAlphaBlendFactor) &&
           (a.alphaBlendOp == b.alphaBlendOp) && (a.colorWriteMask == b.colorWriteMask);
}

bool ComparePipelineFragmentShadingRateStateCreateInfo(const VkPipelineFragmentShadingRateStateCreateInfoKHR &a,
                                                       const VkPipelineFragmentShadingRateStateCreateInfoKHR &b) {
    // Since this is chained in a pnext, we don't want to check the pNext/sType
    return (a.fragmentSize.width == b.fragmentSize.width) && (a.fragmentSize.height == b.fragmentSize.height) &&
           (a.combinerOps[0] == b.combinerOps[0]) && (a.combinerOps[1] == b.combinerOps[1]);
}

static inline bool CompareSamplerYcbcrConversionInfo(const VkSamplerYcbcrConversionInfo &a, const VkSamplerYcbcrConversionInfo &b) {
    return a.conversion == b.conversion;
}
static inline bool CompareSamplerReductionModeCreateInfo(const VkSamplerReductionModeCreateInfo &a,
                                                         const VkSamplerReductionModeCreateInfo &b) {
    return a.reductionMode == b.reductionMode;
}

static inline bool CompareSamplerBorderColorComponentMappingCreateInfo(const VkSamplerBorderColorComponentMappingCreateInfoEXT &a,
                                                                       const VkSamplerBorderColorComponentMappingCreateInfoEXT &b) {
    return (a.components.r == b.components.r) && (a.components.g == b.components.g) && (a.components.b == b.components.b) &&
           (a.components.a == b.components.a) && (a.srgb == b.srgb);
}

static inline bool CompareSamplerCustomBorderColorCreateInfo(const VkSamplerCustomBorderColorCreateInfoEXT &a,
                                                             const VkSamplerCustomBorderColorCreateInfoEXT &b) {
    return (a.format == b.format);
}

bool CompareSamplerCreateInfo(const VkSamplerCreateInfo &a, const VkSamplerCreateInfo &b) {
    if (a.pNext && b.pNext) {
        auto *a_ycbcr_conversion = vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(a.pNext);
        auto *b_ycbcr_conversion = vku::FindStructInPNextChain<VkSamplerYcbcrConversionInfo>(b.pNext);
        if (a_ycbcr_conversion && b_ycbcr_conversion) {
            if (!CompareSamplerYcbcrConversionInfo(*a_ycbcr_conversion, *b_ycbcr_conversion)) {
                return false;
            }
        } else if (a_ycbcr_conversion != b_ycbcr_conversion) {
            return false;  // both are not null
        }

        auto *a_reduction_mode = vku::FindStructInPNextChain<VkSamplerReductionModeCreateInfo>(a.pNext);
        auto *b_reduction_mode = vku::FindStructInPNextChain<VkSamplerReductionModeCreateInfo>(b.pNext);
        if (a_reduction_mode && b_reduction_mode) {
            if (!CompareSamplerReductionModeCreateInfo(*a_reduction_mode, *b_reduction_mode)) {
                return false;
            }
        } else if (a_reduction_mode != b_reduction_mode) {
            return false;  // both are not null
        }

        auto *a_component_mapping = vku::FindStructInPNextChain<VkSamplerBorderColorComponentMappingCreateInfoEXT>(a.pNext);
        auto *b_component_mapping = vku::FindStructInPNextChain<VkSamplerBorderColorComponentMappingCreateInfoEXT>(b.pNext);
        if (a_component_mapping && b_component_mapping) {
            if (!CompareSamplerBorderColorComponentMappingCreateInfo(*a_component_mapping, *b_component_mapping)) {
                return false;
            }
        } else if (a_component_mapping != b_component_mapping) {
            return false;  // both are not null
        }

        auto *a_border_color = vku::FindStructInPNextChain<VkSamplerCustomBorderColorCreateInfoEXT>(a.pNext);
        auto *b_border_color = vku::FindStructInPNextChain<VkSamplerCustomBorderColorCreateInfoEXT>(b.pNext);
        if (a_border_color && b_border_color) {
            if (!CompareSamplerCustomBorderColorCreateInfo(*a_border_color, *b_border_color)) {
                return false;
            }
        } else if (a_border_color != b_border_color) {
            return false;  // both are not null
        }
    } else if (a.pNext != b.pNext) {
        return false;  // both are not null
    }

    return (a.flags == b.flags) && (a.magFilter == b.magFilter) && (a.minFilter == b.minFilter) && (a.mipmapMode == b.mipmapMode) &&
           (a.addressModeU == b.addressModeU) && (a.addressModeV == b.addressModeV) && (a.addressModeW == b.addressModeW) &&
           (a.mipLodBias == b.mipLodBias) && (a.anisotropyEnable == b.anisotropyEnable) && (a.maxAnisotropy == b.maxAnisotropy) &&
           (a.compareEnable == b.compareEnable) && (a.compareOp == b.compareOp) && (a.minLod == b.minLod) &&
           (a.maxLod == b.maxLod) && (a.borderColor == b.borderColor) && (a.unnormalizedCoordinates == b.unnormalizedCoordinates);
}