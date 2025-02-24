// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
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

#ifndef sw_Sampler_hpp
#define sw_Sampler_hpp

#include "Device/Config.hpp"
#include "System/Types.hpp"
#include "Vulkan/VkFormat.hpp"

namespace sw {

struct Mipmap
{
	const void *buffer;

	ushort4 uHalf;
	ushort4 vHalf;
	ushort4 wHalf;
	uint4 width;
	uint4 height;
	uint4 depth;
	short4 onePitchP;
	uint4 pitchP;
	uint4 sliceP;
	uint4 samplePitchP;
	uint4 sampleMax;
};

struct Texture
{
	Mipmap mipmap[MIPMAP_LEVELS];

	float4 widthWidthHeightHeight;
	float4 width;
	float4 height;
	float4 depth;
};

enum FilterType ENUM_UNDERLYING_TYPE_UNSIGNED_INT
{
	FILTER_POINT,
	FILTER_GATHER,
	FILTER_MIN_POINT_MAG_LINEAR,
	FILTER_MIN_LINEAR_MAG_POINT,
	FILTER_LINEAR,
	FILTER_ANISOTROPIC,

	FILTER_LAST = FILTER_ANISOTROPIC
};

enum MipmapType ENUM_UNDERLYING_TYPE_UNSIGNED_INT
{
	MIPMAP_NONE,
	MIPMAP_POINT,
	MIPMAP_LINEAR,

	MIPMAP_LAST = MIPMAP_LINEAR
};

enum AddressingMode ENUM_UNDERLYING_TYPE_UNSIGNED_INT
{
	ADDRESSING_UNUSED,
	ADDRESSING_WRAP,
	ADDRESSING_CLAMP,
	ADDRESSING_MIRROR,
	ADDRESSING_MIRRORONCE,
	ADDRESSING_BORDER,    // Single color
	ADDRESSING_SEAMLESS,  // Border of pixels
	ADDRESSING_CUBEFACE,  // Cube face layer
	ADDRESSING_TEXELFETCH,

	ADDRESSING_LAST = ADDRESSING_TEXELFETCH
};

struct Sampler
{
	VkImageViewType textureType;
	vk::Format textureFormat;
	FilterType textureFilter;
	AddressingMode addressingModeU;
	AddressingMode addressingModeV;
	AddressingMode addressingModeW;
	MipmapType mipmapFilter;
	VkComponentMapping swizzle;
	int gatherComponent;
	bool highPrecisionFiltering;
	bool compareEnable;
	VkCompareOp compareOp;
	VkBorderColor border;
	VkClearColorValue customBorder;
	bool unnormalizedCoordinates;

	VkSamplerYcbcrModelConversion ycbcrModel;
	bool studioSwing;    // Narrow range
	bool swappedChroma;  // Cb/Cr components in reverse order
	FilterType chromaFilter;
	VkChromaLocation chromaXOffset;
	VkChromaLocation chromaYOffset;

	float mipLodBias = 0.0f;
	float maxAnisotropy = 0.0f;
	float minLod = -1000.0f;
	float maxLod = 1000.0f;

	bool is1D() const
	{
		switch(textureType)
		{
		case VK_IMAGE_VIEW_TYPE_1D:
		case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
			return true;
		case VK_IMAGE_VIEW_TYPE_2D:
		case VK_IMAGE_VIEW_TYPE_3D:
		case VK_IMAGE_VIEW_TYPE_CUBE:
		case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
		case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
			return false;
		default:
			UNSUPPORTED("VkImageViewType %d", (int)textureType);
			return false;
		}
	}

	bool is2D() const
	{
		switch(textureType)
		{
		case VK_IMAGE_VIEW_TYPE_2D:
		case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
			return true;
		case VK_IMAGE_VIEW_TYPE_1D:
		case VK_IMAGE_VIEW_TYPE_3D:
		case VK_IMAGE_VIEW_TYPE_CUBE:
		case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
		case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
			return false;
		default:
			UNSUPPORTED("VkImageViewType %d", (int)textureType);
			return false;
		}
	}

	bool is3D() const
	{
		switch(textureType)
		{
		case VK_IMAGE_VIEW_TYPE_3D:
			return true;
		case VK_IMAGE_VIEW_TYPE_1D:
		case VK_IMAGE_VIEW_TYPE_2D:
		case VK_IMAGE_VIEW_TYPE_CUBE:
		case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
		case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
		case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
			return false;
		default:
			UNSUPPORTED("VkImageViewType %d", (int)textureType);
			return false;
		}
	}

	bool isCube() const
	{
		switch(textureType)
		{
		case VK_IMAGE_VIEW_TYPE_CUBE:
		case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
			return true;
		case VK_IMAGE_VIEW_TYPE_1D:
		case VK_IMAGE_VIEW_TYPE_2D:
		case VK_IMAGE_VIEW_TYPE_3D:
		case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
		case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
			return false;
		default:
			UNSUPPORTED("VkImageViewType %d", (int)textureType);
			return false;
		}
	}

	bool isArrayed() const
	{
		switch(textureType)
		{
		case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
		case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
		case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
			return true;
		case VK_IMAGE_VIEW_TYPE_1D:
		case VK_IMAGE_VIEW_TYPE_2D:
		case VK_IMAGE_VIEW_TYPE_3D:
		case VK_IMAGE_VIEW_TYPE_CUBE:
			return false;
		default:
			UNSUPPORTED("VkImageViewType %d", (int)textureType);
			return false;
		}
	}

	// Returns the number of coordinates required to sample the image,
	// not including any array coordinate, which is indicated by isArrayed().
	unsigned int dimensionality() const
	{
		switch(textureType)
		{
		case VK_IMAGE_VIEW_TYPE_1D:
		case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
			return 1;
		case VK_IMAGE_VIEW_TYPE_2D:
		case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
			return 2;
		case VK_IMAGE_VIEW_TYPE_3D:
		case VK_IMAGE_VIEW_TYPE_CUBE:
		case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
			return 3;
		default:
			UNSUPPORTED("VkImageViewType %d", (int)textureType);
			return 0;
		}
	}
};

}  // namespace sw

#endif  // sw_Sampler_hpp
