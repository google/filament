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

#include "SpirvShader.hpp"

#include "SamplerCore.hpp"
#include "Device/Config.hpp"
#include "System/Debug.hpp"
#include "System/Math.hpp"
#include "Vulkan/VkDescriptorSetLayout.hpp"
#include "Vulkan/VkDevice.hpp"
#include "Vulkan/VkImageView.hpp"
#include "Vulkan/VkSampler.hpp"

#include <spirv/unified1/spirv.hpp>

#include <climits>
#include <mutex>

namespace sw {

SpirvEmitter::ImageSampler *SpirvEmitter::getImageSampler(const vk::Device *device, uint32_t signature, uint32_t samplerId, uint32_t imageViewId)
{
	ImageInstructionSignature instruction(signature);
	ASSERT(imageViewId != 0 && (samplerId != 0 || instruction.samplerMethod == Fetch || instruction.samplerMethod == Write));
	ASSERT(device);

	vk::Device::SamplingRoutineCache::Key key = { signature, samplerId, imageViewId };

	auto createSamplingRoutine = [device](const vk::Device::SamplingRoutineCache::Key &key) {
		ImageInstructionSignature instruction(key.instruction);
		const vk::Identifier::State imageViewState = vk::Identifier(key.imageView).getState();
		const vk::SamplerState *vkSamplerState = (key.sampler != 0) ? device->findSampler(key.sampler) : nullptr;

		auto type = imageViewState.imageViewType;
		auto samplerMethod = static_cast<SamplerMethod>(instruction.samplerMethod);

		Sampler samplerState = {};
		samplerState.textureType = type;
		ASSERT(instruction.coordinates >= samplerState.dimensionality());  // "It may be a vector larger than needed, but all unused components appear after all used components."
		samplerState.textureFormat = imageViewState.format;

		samplerState.addressingModeU = convertAddressingMode(0, vkSamplerState, type);
		samplerState.addressingModeV = convertAddressingMode(1, vkSamplerState, type);
		samplerState.addressingModeW = convertAddressingMode(2, vkSamplerState, type);

		samplerState.mipmapFilter = convertMipmapMode(vkSamplerState);
		samplerState.swizzle = imageViewState.mapping;
		samplerState.gatherComponent = instruction.gatherComponent;

		if(vkSamplerState)
		{
			samplerState.textureFilter = convertFilterMode(vkSamplerState, type, samplerMethod);
			samplerState.border = vkSamplerState->borderColor;
			samplerState.customBorder = vkSamplerState->customBorderColor;

			samplerState.mipmapFilter = convertMipmapMode(vkSamplerState);
			samplerState.highPrecisionFiltering = vkSamplerState->highPrecisionFiltering;

			samplerState.compareEnable = (vkSamplerState->compareEnable != VK_FALSE);
			samplerState.compareOp = vkSamplerState->compareOp;
			samplerState.unnormalizedCoordinates = (vkSamplerState->unnormalizedCoordinates != VK_FALSE);

			samplerState.ycbcrModel = vkSamplerState->ycbcrModel;
			samplerState.studioSwing = vkSamplerState->studioSwing;
			samplerState.swappedChroma = vkSamplerState->swappedChroma;
			samplerState.chromaFilter = vkSamplerState->chromaFilter == VK_FILTER_LINEAR ?  FILTER_LINEAR : FILTER_POINT;
			samplerState.chromaXOffset = vkSamplerState->chromaXOffset;
			samplerState.chromaYOffset = vkSamplerState->chromaYOffset;

			samplerState.mipLodBias = vkSamplerState->mipLodBias;
			samplerState.maxAnisotropy = vkSamplerState->maxAnisotropy;
			samplerState.minLod = vkSamplerState->minLod;
			samplerState.maxLod = vkSamplerState->maxLod;

			// If there's a single mip level and filtering doesn't depend on the LOD level,
			// the sampler will need to compute the LOD to produce the proper result.
			// Otherwise, it can be ignored.
			// We can skip the LOD computation for all modes, except LOD query,
			// where we have to return the proper value even if nothing else requires it.
			if(imageViewState.singleMipLevel &&
			   (samplerState.textureFilter != FILTER_MIN_POINT_MAG_LINEAR) &&
			   (samplerState.textureFilter != FILTER_MIN_LINEAR_MAG_POINT) &&
			   (samplerMethod != Query))
			{
				samplerState.minLod = 0.0f;
				samplerState.maxLod = 0.0f;
			}
		}
		else if(samplerMethod == Fetch)
		{
			// OpImageFetch does not take a sampler descriptor, but for VK_EXT_image_robustness
			// requires replacing invalid texels with zero.
			// TODO(b/162327166): Only perform bounds checks when VK_EXT_image_robustness is enabled.
			samplerState.border = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;

			// If there's a single mip level we can skip LOD computation.
			if(imageViewState.singleMipLevel)
			{
				samplerState.minLod = 0.0f;
				samplerState.maxLod = 0.0f;
			}
			// Otherwise make sure LOD is clamped for robustness
			else
			{
				samplerState.minLod = imageViewState.minLod;
				samplerState.maxLod = imageViewState.maxLod;
			}
		}
		else if(samplerMethod == Write)
		{
			return emitWriteRoutine(instruction, samplerState);
		}
		else
			ASSERT(false);

		return emitSamplerRoutine(instruction, samplerState);
	};

	vk::Device::SamplingRoutineCache *cache = device->getSamplingRoutineCache();
	auto routine = cache->getOrCreate(key, createSamplingRoutine);

	return (ImageSampler *)(routine->getEntry());
}

std::shared_ptr<rr::Routine> SpirvEmitter::emitWriteRoutine(ImageInstructionSignature instruction, const Sampler &samplerState)
{
	// TODO(b/129523279): Hold a separate mutex lock for the sampler being built.
	rr::Function<Void(Pointer<Byte>, Pointer<SIMD::Float>, Pointer<SIMD::Float>, Pointer<Byte>)> function;
	{
		Pointer<Byte> descriptor = function.Arg<0>();
		Pointer<SIMD::Float> coord = function.Arg<1>();
		Pointer<SIMD::Float> texelAndMask = function.Arg<2>();
		Pointer<Byte> constants = function.Arg<3>();

		WriteImage(instruction, descriptor, coord, texelAndMask, samplerState.textureFormat);
	}

	return function("sampler");
}

std::shared_ptr<rr::Routine> SpirvEmitter::emitSamplerRoutine(ImageInstructionSignature instruction, const Sampler &samplerState)
{
	// TODO(b/129523279): Hold a separate mutex lock for the sampler being built.
	rr::Function<Void(Pointer<Byte>, Pointer<SIMD::Float>, Pointer<SIMD::Float>, Pointer<Byte>)> function;
	{
		Pointer<Byte> texture = function.Arg<0>();
		Pointer<SIMD::Float> in = function.Arg<1>();
		Pointer<SIMD::Float> out = function.Arg<2>();
		Pointer<Byte> constants = function.Arg<3>();

		SIMD::Float uvwa[4];
		SIMD::Float dRef;
		SIMD::Float lodOrBias;  // Explicit level-of-detail, or bias added to the implicit level-of-detail (depending on samplerMethod).
		SIMD::Float dsx[4];
		SIMD::Float dsy[4];
		SIMD::Int offset[4];
		SIMD::Int sampleId;
		SamplerFunction samplerFunction = instruction.getSamplerFunction();

		uint32_t i = 0;
		for(; i < instruction.coordinates; i++)
		{
			uvwa[i] = in[i];
		}

		if(instruction.isDref())
		{
			dRef = in[i];
			i++;
		}

		if(instruction.samplerMethod == Lod || instruction.samplerMethod == Bias || instruction.samplerMethod == Fetch)
		{
			lodOrBias = in[i];
			i++;
		}
		else if(instruction.samplerMethod == Grad)
		{
			for(uint32_t j = 0; j < instruction.grad; j++, i++)
			{
				dsx[j] = in[i];
			}

			for(uint32_t j = 0; j < instruction.grad; j++, i++)
			{
				dsy[j] = in[i];
			}
		}

		for(uint32_t j = 0; j < instruction.offset; j++, i++)
		{
			offset[j] = As<SIMD::Int>(in[i]);
		}

		if(instruction.sample)
		{
			sampleId = As<SIMD::Int>(in[i]);
		}

		SamplerCore s(constants, samplerState, samplerFunction);

		// For explicit-lod instructions the LOD can be different per SIMD lane. SamplerCore currently assumes
		// a single LOD per four elements, so we sample the image again for each LOD separately.
		// TODO(b/133868964) Pass down 4 component lodOrBias, dsx, and dsy to sampleTexture
		if(samplerFunction.method == Lod || samplerFunction.method == Grad ||
		   samplerFunction.method == Bias || samplerFunction.method == Fetch)
		{
			// Only perform per-lane sampling if LOD diverges or we're doing Grad sampling.
			Bool perLaneSampling = (samplerFunction.method == Grad) || Divergent(As<SIMD::Int>(lodOrBias));
			auto lod = Pointer<Float>(&lodOrBias);
			Int i = 0;
			Do
			{
				SIMD::Float dPdx;
				SIMD::Float dPdy;
				dPdx.x = Pointer<Float>(&dsx[0])[i];
				dPdx.y = Pointer<Float>(&dsx[1])[i];
				dPdx.z = Pointer<Float>(&dsx[2])[i];

				dPdy.x = Pointer<Float>(&dsy[0])[i];
				dPdy.y = Pointer<Float>(&dsy[1])[i];
				dPdy.z = Pointer<Float>(&dsy[2])[i];

				SIMD::Float4 sample = s.sampleTexture(texture, uvwa, dRef, lod[i], dPdx, dPdy, offset, sampleId);

				If(perLaneSampling)
				{
					Pointer<Float> rgba = out;
					rgba[0 * SIMD::Width + i] = Pointer<Float>(&sample.x)[i];
					rgba[1 * SIMD::Width + i] = Pointer<Float>(&sample.y)[i];
					rgba[2 * SIMD::Width + i] = Pointer<Float>(&sample.z)[i];
					rgba[3 * SIMD::Width + i] = Pointer<Float>(&sample.w)[i];
					i++;
				}
				Else
				{
					Pointer<SIMD::Float> rgba = out;
					rgba[0] = sample.x;
					rgba[1] = sample.y;
					rgba[2] = sample.z;
					rgba[3] = sample.w;
					i = SIMD::Width;
				}
			}
			Until(i == SIMD::Width);
		}
		else
		{
			Float lod = Float(lodOrBias.x);
			SIMD::Float4 sample = s.sampleTexture(texture, uvwa, dRef, lod, (dsx[0]), (dsy[0]), offset, sampleId);

			Pointer<SIMD::Float> rgba = out;
			rgba[0] = sample.x;
			rgba[1] = sample.y;
			rgba[2] = sample.z;
			rgba[3] = sample.w;
		}
	}

	return function("sampler");
}

sw::FilterType SpirvEmitter::convertFilterMode(const vk::SamplerState *samplerState, VkImageViewType imageViewType, SamplerMethod samplerMethod)
{
	if(samplerMethod == Gather)
	{
		return FILTER_GATHER;
	}

	if(samplerMethod == Fetch)
	{
		return FILTER_POINT;
	}

	if(samplerState->anisotropyEnable != VK_FALSE)
	{
		if(imageViewType == VK_IMAGE_VIEW_TYPE_2D || imageViewType == VK_IMAGE_VIEW_TYPE_2D_ARRAY)
		{
			if(samplerMethod != Lod)  // TODO(b/162926129): Support anisotropic filtering with explicit LOD.
			{
				return FILTER_ANISOTROPIC;
			}
		}
	}

	switch(samplerState->magFilter)
	{
	case VK_FILTER_NEAREST:
		switch(samplerState->minFilter)
		{
		case VK_FILTER_NEAREST: return FILTER_POINT;
		case VK_FILTER_LINEAR: return FILTER_MIN_LINEAR_MAG_POINT;
		default:
			UNSUPPORTED("minFilter %d", samplerState->minFilter);
			return FILTER_POINT;
		}
		break;
	case VK_FILTER_LINEAR:
		switch(samplerState->minFilter)
		{
		case VK_FILTER_NEAREST: return FILTER_MIN_POINT_MAG_LINEAR;
		case VK_FILTER_LINEAR: return FILTER_LINEAR;
		default:
			UNSUPPORTED("minFilter %d", samplerState->minFilter);
			return FILTER_POINT;
		}
		break;
	default:
		break;
	}

	UNSUPPORTED("magFilter %d", samplerState->magFilter);
	return FILTER_POINT;
}

sw::MipmapType SpirvEmitter::convertMipmapMode(const vk::SamplerState *samplerState)
{
	if(!samplerState)
	{
		return MIPMAP_POINT;  // Samplerless operations (OpImageFetch) can take an integer Lod operand.
	}

	if(samplerState->ycbcrModel != VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY)
	{
		// TODO(b/151263485): Check image view level count instead.
		return MIPMAP_NONE;
	}

	switch(samplerState->mipmapMode)
	{
	case VK_SAMPLER_MIPMAP_MODE_NEAREST: return MIPMAP_POINT;
	case VK_SAMPLER_MIPMAP_MODE_LINEAR: return MIPMAP_LINEAR;
	default:
		UNSUPPORTED("mipmapMode %d", samplerState->mipmapMode);
		return MIPMAP_POINT;
	}
}

sw::AddressingMode SpirvEmitter::convertAddressingMode(int coordinateIndex, const vk::SamplerState *samplerState, VkImageViewType imageViewType)
{
	switch(imageViewType)
	{
	case VK_IMAGE_VIEW_TYPE_1D:
	case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
		if(coordinateIndex >= 1)
		{
			return ADDRESSING_UNUSED;
		}
		break;
	case VK_IMAGE_VIEW_TYPE_2D:
	case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
		if(coordinateIndex == 2)
		{
			return ADDRESSING_UNUSED;
		}
		break;

	case VK_IMAGE_VIEW_TYPE_3D:
		break;

	case VK_IMAGE_VIEW_TYPE_CUBE:
	case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
		if(coordinateIndex <= 1)  // Cube faces themselves are addressed as 2D images.
		{
			// Vulkan 1.1 spec:
			// "Cube images ignore the wrap modes specified in the sampler. Instead, if VK_FILTER_NEAREST is used within a mip level then
			//  VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE is used, and if VK_FILTER_LINEAR is used within a mip level then sampling at the edges
			//  is performed as described earlier in the Cube map edge handling section."
			// This corresponds with our 'SEAMLESS' addressing mode.
			return ADDRESSING_SEAMLESS;
		}
		else  // coordinateIndex == 2
		{
			// The cube face is an index into 2D array layers.
			return ADDRESSING_CUBEFACE;
		}
		break;

	default:
		UNSUPPORTED("imageViewType %d", imageViewType);
		return ADDRESSING_WRAP;
	}

	if(!samplerState)
	{
		// OpImageFetch does not take a sampler descriptor, but still needs a valid
		// addressing mode that prevents out-of-bounds accesses:
		// "The value returned by a read of an invalid texel is undefined, unless that
		//  read operation is from a buffer resource and the robustBufferAccess feature
		//  is enabled. In that case, an invalid texel is replaced as described by the
		//  robustBufferAccess feature." - Vulkan 1.1

		// VK_EXT_image_robustness requires nullifying out-of-bounds accesses.
		// ADDRESSING_BORDER causes texel replacement to be performed.
		// TODO(b/162327166): Only perform bounds checks when VK_EXT_image_robustness is enabled.
		return ADDRESSING_BORDER;
	}

	VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	switch(coordinateIndex)
	{
	case 0: addressMode = samplerState->addressModeU; break;
	case 1: addressMode = samplerState->addressModeV; break;
	case 2: addressMode = samplerState->addressModeW; break;
	default: UNSUPPORTED("coordinateIndex: %d", coordinateIndex);
	}

	switch(addressMode)
	{
	case VK_SAMPLER_ADDRESS_MODE_REPEAT: return ADDRESSING_WRAP;
	case VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT: return ADDRESSING_MIRROR;
	case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE: return ADDRESSING_CLAMP;
	case VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER: return ADDRESSING_BORDER;
	case VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE: return ADDRESSING_MIRRORONCE;
	default:
		UNSUPPORTED("addressMode %d", addressMode);
		return ADDRESSING_WRAP;
	}
}

}  // namespace sw
