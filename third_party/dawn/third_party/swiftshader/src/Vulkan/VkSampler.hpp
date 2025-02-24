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

#ifndef VK_SAMPLER_HPP_
#define VK_SAMPLER_HPP_

#include "VkImageView.hpp"  // For ResolveIdentityMapping()
#include "Device/Config.hpp"
#include "Device/Memset.hpp"
#include "System/Math.hpp"

#include <atomic>

namespace vk {

struct SamplerState : sw::Memset<SamplerState>
{
	SamplerState(const VkSamplerCreateInfo *pCreateInfo, const vk::SamplerYcbcrConversion *ycbcrConversion,
	             const VkClearColorValue &customBorderColor);

	// Prevents accessing mipmap levels out of range.
	static float ClampLod(float lod)
	{
		return sw::clamp(lod, 0.0f, (float)(sw::MAX_TEXTURE_LOD));
	}

	const VkFilter magFilter = VK_FILTER_NEAREST;
	const VkFilter minFilter = VK_FILTER_NEAREST;
	const VkSamplerMipmapMode mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	const VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	const VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	const VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	const float mipLodBias = 0.0f;
	const VkBool32 anisotropyEnable = VK_FALSE;
	const float maxAnisotropy = 0.0f;
	const VkBool32 compareEnable = VK_FALSE;
	const VkCompareOp compareOp = VK_COMPARE_OP_NEVER;
	const float minLod = 0.0f;
	const float maxLod = 0.0f;
	const VkBorderColor borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	const VkClearColorValue customBorderColor = {};
	const VkBool32 unnormalizedCoordinates = VK_FALSE;

	VkSamplerYcbcrModelConversion ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
	const bool highPrecisionFiltering = false;
	bool studioSwing = false;    // Narrow range
	bool swappedChroma = false;  // Cb/Cr components in reverse order
	VkFilter chromaFilter = VK_FILTER_NEAREST;
	VkChromaLocation chromaXOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
	VkChromaLocation chromaYOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
};

class Sampler : public Object<Sampler, VkSampler>, public SamplerState
{
public:
	Sampler(const VkSamplerCreateInfo *pCreateInfo, void *mem, const SamplerState &samplerState, uint32_t samplerID);

	static size_t ComputeRequiredAllocationSize(const VkSamplerCreateInfo *pCreateInfo)
	{
		return 0;
	}

	const uint32_t id = 0;
};

class SamplerYcbcrConversion : public Object<SamplerYcbcrConversion, VkSamplerYcbcrConversion>
{
public:
	SamplerYcbcrConversion(const VkSamplerYcbcrConversionCreateInfo *pCreateInfo, void *mem)
	    : format(pCreateInfo->format)
	    , ycbcrModel(pCreateInfo->ycbcrModel)
	    , ycbcrRange(pCreateInfo->ycbcrRange)
	    , components(ResolveIdentityMapping(pCreateInfo->components))
	    , xChromaOffset(pCreateInfo->xChromaOffset)
	    , yChromaOffset(pCreateInfo->yChromaOffset)
	    , chromaFilter(pCreateInfo->chromaFilter)
	    , forceExplicitReconstruction(pCreateInfo->forceExplicitReconstruction)
	{
	}

	~SamplerYcbcrConversion() = default;

	static size_t ComputeRequiredAllocationSize(const VkSamplerYcbcrConversionCreateInfo *pCreateInfo)
	{
		return 0;
	}

	const VkFormat format = VK_FORMAT_UNDEFINED;
	const VkSamplerYcbcrModelConversion ycbcrModel = VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY;
	const VkSamplerYcbcrRange ycbcrRange = VK_SAMPLER_YCBCR_RANGE_ITU_FULL;
	const VkComponentMapping components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	const VkChromaLocation xChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
	const VkChromaLocation yChromaOffset = VK_CHROMA_LOCATION_COSITED_EVEN;
	const VkFilter chromaFilter = VK_FILTER_NEAREST;
	const VkBool32 forceExplicitReconstruction = VK_FALSE;
};

static inline Sampler *Cast(VkSampler object)
{
	return Sampler::Cast(object);
}

static inline SamplerYcbcrConversion *Cast(VkSamplerYcbcrConversion object)
{
	return SamplerYcbcrConversion::Cast(object);
}

}  // namespace vk

#endif  // VK_SAMPLER_HPP_
