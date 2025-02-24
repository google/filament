// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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

#include "VkSampler.hpp"

namespace vk {

SamplerState::SamplerState(const VkSamplerCreateInfo *pCreateInfo, const vk::SamplerYcbcrConversion *ycbcrConversion,
                           const VkClearColorValue &customBorderColor)
    : Memset(this, 0)
    , magFilter(pCreateInfo->magFilter)
    , minFilter(pCreateInfo->minFilter)
    , mipmapMode(pCreateInfo->mipmapMode)
    , addressModeU(pCreateInfo->addressModeU)
    , addressModeV(pCreateInfo->addressModeV)
    , addressModeW(pCreateInfo->addressModeW)
    , mipLodBias(pCreateInfo->mipLodBias)
    , anisotropyEnable(pCreateInfo->anisotropyEnable)
    , maxAnisotropy(pCreateInfo->maxAnisotropy)
    , compareEnable(pCreateInfo->compareEnable)
    , compareOp(pCreateInfo->compareOp)
    , minLod(ClampLod(pCreateInfo->minLod))
    , maxLod(ClampLod(pCreateInfo->maxLod))
    , borderColor(pCreateInfo->borderColor)
    , customBorderColor(customBorderColor)
    , unnormalizedCoordinates(pCreateInfo->unnormalizedCoordinates)
#ifdef SWIFTSHADER_HIGH_PRECISION_FILTERING
    , highPrecisionFiltering(true)
#endif
{
	if(ycbcrConversion)
	{
		ycbcrModel = ycbcrConversion->ycbcrModel;
		studioSwing = (ycbcrConversion->ycbcrRange == VK_SAMPLER_YCBCR_RANGE_ITU_NARROW);
		swappedChroma = (ycbcrConversion->components.r != VK_COMPONENT_SWIZZLE_R);
        chromaFilter = ycbcrConversion->chromaFilter;
        chromaXOffset = ycbcrConversion->xChromaOffset;
        chromaYOffset = ycbcrConversion->yChromaOffset;
	}
}

Sampler::Sampler(const VkSamplerCreateInfo *pCreateInfo, void *mem, const SamplerState &samplerState, uint32_t samplerID)
    : SamplerState(samplerState)
    , id(samplerID)
{
}

}  // namespace vk
