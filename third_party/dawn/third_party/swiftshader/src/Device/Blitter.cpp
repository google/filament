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

#include "Blitter.hpp"

#include "Pipeline/ShaderCore.hpp"
#include "Reactor/Reactor.hpp"
#include "System/CPUID.hpp"
#include "System/Debug.hpp"
#include "System/Half.hpp"
#include "System/Memory.hpp"
#include "Vulkan/VkImage.hpp"
#include "Vulkan/VkImageView.hpp"

#include <utility>

#if defined(__i386__) || defined(__x86_64__)
#	include <xmmintrin.h>
#	include <emmintrin.h>
#endif

namespace sw {

static rr::RValue<rr::Int> PackFields(const rr::Int4 &ints, const sw::int4 shifts)
{
	return (rr::Int(ints.x) << shifts[0]) |
	       (rr::Int(ints.y) << shifts[1]) |
	       (rr::Int(ints.z) << shifts[2]) |
	       (rr::Int(ints.w) << shifts[3]);
}

Blitter::Blitter()
    : blitMutex()
    , blitCache(1024)
    , cornerUpdateMutex()
    , cornerUpdateCache(64)  // We only need one of these per format
{
}

Blitter::~Blitter()
{
}

void Blitter::clear(const void *pixel, vk::Format format, vk::Image *dest, const vk::Format &viewFormat, const VkImageSubresourceRange &subresourceRange, const VkRect2D *renderArea)
{
	VkImageAspectFlagBits aspect = static_cast<VkImageAspectFlagBits>(subresourceRange.aspectMask);
	vk::Format dstFormat = viewFormat.getAspectFormat(aspect);
	if(dstFormat == VK_FORMAT_UNDEFINED)
	{
		return;
	}

	VkClearValue clampedPixel;
	if(viewFormat.isSignedNormalized() || viewFormat.isUnsignedNormalized())
	{
		const float minValue = viewFormat.isSignedNormalized() ? -1.0f : 0.0f;

		if(aspect & VK_IMAGE_ASPECT_COLOR_BIT)
		{
			memcpy(clampedPixel.color.float32, pixel, sizeof(VkClearColorValue));
			clampedPixel.color.float32[0] = sw::clamp(clampedPixel.color.float32[0], minValue, 1.0f);
			clampedPixel.color.float32[1] = sw::clamp(clampedPixel.color.float32[1], minValue, 1.0f);
			clampedPixel.color.float32[2] = sw::clamp(clampedPixel.color.float32[2], minValue, 1.0f);
			clampedPixel.color.float32[3] = sw::clamp(clampedPixel.color.float32[3], minValue, 1.0f);
			pixel = clampedPixel.color.float32;
		}

		// Stencil never requires clamping, so we can check for Depth only
		if(aspect & VK_IMAGE_ASPECT_DEPTH_BIT)
		{
			memcpy(&(clampedPixel.depthStencil), pixel, sizeof(VkClearDepthStencilValue));
			clampedPixel.depthStencil.depth = sw::clamp(clampedPixel.depthStencil.depth, minValue, 1.0f);
			pixel = &(clampedPixel.depthStencil);
		}
	}

	if(fastClear(pixel, format, dest, dstFormat, subresourceRange, renderArea))
	{
		return;
	}

	State state(format, dstFormat, 1, dest->getSampleCount(), Options{ 0xF });
	auto blitRoutine = getBlitRoutine(state);
	if(!blitRoutine)
	{
		return;
	}

	VkImageSubresource subres = {
		subresourceRange.aspectMask,
		subresourceRange.baseMipLevel,
		subresourceRange.baseArrayLayer
	};

	uint32_t lastMipLevel = dest->getLastMipLevel(subresourceRange);
	uint32_t lastLayer = dest->getLastLayerIndex(subresourceRange);

	VkRect2D area = { { 0, 0 }, { 0, 0 } };
	if(renderArea)
	{
		ASSERT(subresourceRange.levelCount == 1);
		area = *renderArea;
	}

	for(; subres.mipLevel <= lastMipLevel; subres.mipLevel++)
	{
		VkExtent3D extent = dest->getMipLevelExtent(aspect, subres.mipLevel);
		if(!renderArea)
		{
			area.extent.width = extent.width;
			area.extent.height = extent.height;
		}

		BlitData data = {
			pixel, nullptr,  // source, dest

			assert_cast<uint32_t>(format.bytes()),                                  // sPitchB
			assert_cast<uint32_t>(dest->rowPitchBytes(aspect, subres.mipLevel)),    // dPitchB
			0,                                                                      // sSliceB (unused in clear operations)
			assert_cast<uint32_t>(dest->slicePitchBytes(aspect, subres.mipLevel)),  // dSliceB

			0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 0.0f,  // x0, y0, z0, w, h, d

			area.offset.x, static_cast<int>(area.offset.x + area.extent.width),   // x0d, x1d
			area.offset.y, static_cast<int>(area.offset.y + area.extent.height),  // y0d, y1d
			0, 1,                                                                 // z0d, z1d

			0, 0, 0,  // sWidth, sHeight, sDepth

			false,  // filter3D
		};

		if(renderArea && dest->is3DSlice())
		{
			// Reinterpret layers as depth slices
			subres.arrayLayer = 0;
			for(uint32_t depth = subresourceRange.baseArrayLayer; depth <= lastLayer; depth++)
			{
				data.dest = dest->getTexelPointer({ 0, 0, static_cast<int32_t>(depth) }, subres);
				blitRoutine(&data);
			}
		}
		else
		{
			for(subres.arrayLayer = subresourceRange.baseArrayLayer; subres.arrayLayer <= lastLayer; subres.arrayLayer++)
			{
				for(uint32_t depth = 0; depth < extent.depth; depth++)
				{
					data.dest = dest->getTexelPointer({ 0, 0, static_cast<int32_t>(depth) }, subres);

					blitRoutine(&data);
				}
			}
		}
	}
	dest->contentsChanged(subresourceRange);
}

bool Blitter::fastClear(const void *clearValue, vk::Format clearFormat, vk::Image *dest, const vk::Format &viewFormat, const VkImageSubresourceRange &subresourceRange, const VkRect2D *renderArea)
{
	if(clearFormat != VK_FORMAT_R32G32B32A32_SFLOAT &&
	   clearFormat != VK_FORMAT_D32_SFLOAT &&
	   clearFormat != VK_FORMAT_S8_UINT)
	{
		return false;
	}

	union ClearValue
	{
		struct
		{
			float r;
			float g;
			float b;
			float a;
		};

		float rgb[3];

		float d;
		uint32_t d_as_u32;

		uint32_t s;
	};

	const ClearValue &c = *reinterpret_cast<const ClearValue *>(clearValue);

	uint32_t packed = 0;

	VkImageAspectFlagBits aspect = static_cast<VkImageAspectFlagBits>(subresourceRange.aspectMask);
	switch(viewFormat)
	{
	case VK_FORMAT_R5G6B5_UNORM_PACK16:
		packed = ((uint16_t)(31 * c.b + 0.5f) << 0) |
		         ((uint16_t)(63 * c.g + 0.5f) << 5) |
		         ((uint16_t)(31 * c.r + 0.5f) << 11);
		break;
	case VK_FORMAT_B5G6R5_UNORM_PACK16:
		packed = ((uint16_t)(31 * c.r + 0.5f) << 0) |
		         ((uint16_t)(63 * c.g + 0.5f) << 5) |
		         ((uint16_t)(31 * c.b + 0.5f) << 11);
		break;
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
	case VK_FORMAT_R8G8B8A8_UNORM:
		packed = ((uint32_t)(255 * c.a + 0.5f) << 24) |
		         ((uint32_t)(255 * c.b + 0.5f) << 16) |
		         ((uint32_t)(255 * c.g + 0.5f) << 8) |
		         ((uint32_t)(255 * c.r + 0.5f) << 0);
		break;
	case VK_FORMAT_B8G8R8A8_UNORM:
		packed = ((uint32_t)(255 * c.a + 0.5f) << 24) |
		         ((uint32_t)(255 * c.r + 0.5f) << 16) |
		         ((uint32_t)(255 * c.g + 0.5f) << 8) |
		         ((uint32_t)(255 * c.b + 0.5f) << 0);
		break;
	case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
		packed = R11G11B10F(c.rgb);
		break;
	case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
		packed = RGB9E5(c.rgb);
		break;
	case VK_FORMAT_D32_SFLOAT:
		ASSERT(clearFormat == VK_FORMAT_D32_SFLOAT);
		packed = c.d_as_u32;  // float reinterpreted as uint32
		break;
	case VK_FORMAT_S8_UINT:
		ASSERT(clearFormat == VK_FORMAT_S8_UINT);
		packed = static_cast<uint8_t>(c.s);
		break;
	default:
		return false;
	}

	VkImageSubresource subres = {
		subresourceRange.aspectMask,
		subresourceRange.baseMipLevel,
		subresourceRange.baseArrayLayer
	};
	uint32_t lastMipLevel = dest->getLastMipLevel(subresourceRange);
	uint32_t lastLayer = dest->getLastLayerIndex(subresourceRange);

	VkRect2D area = { { 0, 0 }, { 0, 0 } };
	if(renderArea)
	{
		ASSERT(subresourceRange.levelCount == 1);
		area = *renderArea;
	}

	for(; subres.mipLevel <= lastMipLevel; subres.mipLevel++)
	{
		int rowPitchBytes = dest->rowPitchBytes(aspect, subres.mipLevel);
		int slicePitchBytes = dest->slicePitchBytes(aspect, subres.mipLevel);
		VkExtent3D extent = dest->getMipLevelExtent(aspect, subres.mipLevel);
		if(!renderArea)
		{
			area.extent.width = extent.width;
			area.extent.height = extent.height;
		}
		else if(dest->is3DSlice())
		{
			extent.depth = 1;  // The 3D image is instead interpreted as a 2D image with layers
		}

		for(subres.arrayLayer = subresourceRange.baseArrayLayer; subres.arrayLayer <= lastLayer; subres.arrayLayer++)
		{
			for(uint32_t depth = 0; depth < extent.depth; depth++)
			{
				uint8_t *slice = (uint8_t *)dest->getTexelPointer(
				    { area.offset.x, area.offset.y, static_cast<int32_t>(depth) }, subres);

				for(int j = 0; j < dest->getSampleCount(); j++)
				{
					uint8_t *d = slice;

					switch(viewFormat.bytes())
					{
					case 4:
						for(uint32_t i = 0; i < area.extent.height; i++)
						{
							ASSERT(d < dest->end());
							sw::clear((uint32_t *)d, packed, area.extent.width);
							d += rowPitchBytes;
						}
						break;
					case 2:
						for(uint32_t i = 0; i < area.extent.height; i++)
						{
							ASSERT(d < dest->end());
							sw::clear((uint16_t *)d, static_cast<uint16_t>(packed), area.extent.width);
							d += rowPitchBytes;
						}
						break;
					case 1:
						for(uint32_t i = 0; i < area.extent.height; i++)
						{
							ASSERT(d < dest->end());
							memset(d, packed, area.extent.width);
							d += rowPitchBytes;
						}
						break;
					default:
						assert(false);
					}

					slice += slicePitchBytes;
				}
			}
		}
	}
	dest->contentsChanged(subresourceRange);

	return true;
}

Float4 Blitter::readFloat4(Pointer<Byte> element, const State &state)
{
	Float4 c(0.0f, 0.0f, 0.0f, 1.0f);

	switch(state.sourceFormat)
	{
	case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
		c.w = Float(Int(*Pointer<Byte>(element)) & Int(0xF));
		c.x = Float((Int(*Pointer<Byte>(element)) >> 4) & Int(0xF));
		c.y = Float(Int(*Pointer<Byte>(element + 1)) & Int(0xF));
		c.z = Float((Int(*Pointer<Byte>(element + 1)) >> 4) & Int(0xF));
		break;
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_R8_SNORM:
		c.x = Float(Int(*Pointer<SByte>(element)));
		c.w = float(0x7F);
		break;
	case VK_FORMAT_R8_UNORM:
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_R8_SRGB:
		c.x = Float(Int(*Pointer<Byte>(element)));
		c.w = float(0xFF);
		break;
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_R16_SNORM:
		c.x = Float(Int(*Pointer<Short>(element)));
		c.w = float(0x7FFF);
		break;
	case VK_FORMAT_R16_UNORM:
	case VK_FORMAT_R16_UINT:
		c.x = Float(Int(*Pointer<UShort>(element)));
		c.w = float(0xFFFF);
		break;
	case VK_FORMAT_R32_SINT:
		c.x = Float(*Pointer<Int>(element));
		c.w = float(0x7FFFFFFF);
		break;
	case VK_FORMAT_R32_UINT:
		c.x = Float(*Pointer<UInt>(element));
		c.w = float(0xFFFFFFFF);
		break;
	case VK_FORMAT_B8G8R8A8_SRGB:
	case VK_FORMAT_B8G8R8A8_UNORM:
		c = Float4(*Pointer<Byte4>(element)).zyxw;
		break;
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
	case VK_FORMAT_R8G8B8A8_SNORM:
		c = Float4(*Pointer<SByte4>(element));
		break;
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_UINT:
	case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
	case VK_FORMAT_R8G8B8A8_SRGB:
		c = Float4(*Pointer<Byte4>(element));
		break;
	case VK_FORMAT_R16G16B16A16_SINT:
	case VK_FORMAT_R16G16B16A16_SNORM:
		c = Float4(*Pointer<Short4>(element));
		break;
	case VK_FORMAT_R16G16B16A16_UNORM:
	case VK_FORMAT_R16G16B16A16_UINT:
		c = Float4(*Pointer<UShort4>(element));
		break;
	case VK_FORMAT_R32G32B32A32_SINT:
		c = Float4(*Pointer<Int4>(element));
		break;
	case VK_FORMAT_R32G32B32A32_UINT:
		c = Float4(*Pointer<UInt4>(element));
		break;
	case VK_FORMAT_R8G8_SINT:
	case VK_FORMAT_R8G8_SNORM:
		c.x = Float(Int(*Pointer<SByte>(element + 0)));
		c.y = Float(Int(*Pointer<SByte>(element + 1)));
		c.w = float(0x7F);
		break;
	case VK_FORMAT_R8G8_UNORM:
	case VK_FORMAT_R8G8_UINT:
	case VK_FORMAT_R8G8_SRGB:
		c.x = Float(Int(*Pointer<Byte>(element + 0)));
		c.y = Float(Int(*Pointer<Byte>(element + 1)));
		c.w = float(0xFF);
		break;
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R16G16_SNORM:
		c.x = Float(Int(*Pointer<Short>(element + 0)));
		c.y = Float(Int(*Pointer<Short>(element + 2)));
		c.w = float(0x7FFF);
		break;
	case VK_FORMAT_R16G16_UNORM:
	case VK_FORMAT_R16G16_UINT:
		c.x = Float(Int(*Pointer<UShort>(element + 0)));
		c.y = Float(Int(*Pointer<UShort>(element + 2)));
		c.w = float(0xFFFF);
		break;
	case VK_FORMAT_R32G32_SINT:
		c.x = Float(*Pointer<Int>(element + 0));
		c.y = Float(*Pointer<Int>(element + 4));
		c.w = float(0x7FFFFFFF);
		break;
	case VK_FORMAT_R32G32_UINT:
		c.x = Float(*Pointer<UInt>(element + 0));
		c.y = Float(*Pointer<UInt>(element + 4));
		c.w = float(0xFFFFFFFF);
		break;
	case VK_FORMAT_R32G32B32A32_SFLOAT:
		c = *Pointer<Float4>(element);
		break;
	case VK_FORMAT_R32G32_SFLOAT:
		c.x = *Pointer<Float>(element + 0);
		c.y = *Pointer<Float>(element + 4);
		break;
	case VK_FORMAT_R32_SFLOAT:
		c.x = *Pointer<Float>(element);
		break;
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		c.w = Float(*Pointer<Half>(element + 6));
	case VK_FORMAT_R16G16B16_SFLOAT:
		c.z = Float(*Pointer<Half>(element + 4));
	case VK_FORMAT_R16G16_SFLOAT:
		c.y = Float(*Pointer<Half>(element + 2));
	case VK_FORMAT_R16_SFLOAT:
		c.x = Float(*Pointer<Half>(element));
		break;
	case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
		c = r11g11b10Unpack(*Pointer<UInt>(element));
		break;
	case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
		// This type contains a common 5 bit exponent (E) and a 9 bit the mantissa for R, G and B.
		c.x = Float(*Pointer<UInt>(element) & UInt(0x000001FF));          // R's mantissa (bits 0-8)
		c.y = Float((*Pointer<UInt>(element) & UInt(0x0003FE00)) >> 9);   // G's mantissa (bits 9-17)
		c.z = Float((*Pointer<UInt>(element) & UInt(0x07FC0000)) >> 18);  // B's mantissa (bits 18-26)
		c *= Float4(
		    // 2^E, using the exponent (bits 27-31) and treating it as an unsigned integer value
		    Float(UInt(1) << ((*Pointer<UInt>(element) & UInt(0xF8000000)) >> 27)) *
		    // Since the 9 bit mantissa values currently stored in RGB were converted straight
		    // from int to float (in the [0, 1<<9] range instead of the [0, 1] range), they
		    // are (1 << 9) times too high.
		    // Also, the exponent has 5 bits and we compute the exponent bias of floating point
		    // formats using "2^(k-1) - 1", so, in this case, the exponent bias is 2^(5-1)-1 = 15
		    // Exponent bias (15) + number of mantissa bits per component (9) = 24
		    Float(1.0f / (1 << 24)));
		c.w = 1.0f;
		break;
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
		c.x = Float(Int((*Pointer<UShort>(element) & UShort(0xF000)) >> UShort(12)));
		c.y = Float(Int((*Pointer<UShort>(element) & UShort(0x0F00)) >> UShort(8)));
		c.z = Float(Int((*Pointer<UShort>(element) & UShort(0x00F0)) >> UShort(4)));
		c.w = Float(Int(*Pointer<UShort>(element) & UShort(0x000F)));
		break;
	case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
		c.w = Float(Int((*Pointer<UShort>(element) & UShort(0xF000)) >> UShort(12)));
		c.z = Float(Int((*Pointer<UShort>(element) & UShort(0x0F00)) >> UShort(8)));
		c.y = Float(Int((*Pointer<UShort>(element) & UShort(0x00F0)) >> UShort(4)));
		c.x = Float(Int(*Pointer<UShort>(element) & UShort(0x000F)));
		break;
	case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
		c.w = Float(Int((*Pointer<UShort>(element) & UShort(0xF000)) >> UShort(12)));
		c.x = Float(Int((*Pointer<UShort>(element) & UShort(0x0F00)) >> UShort(8)));
		c.y = Float(Int((*Pointer<UShort>(element) & UShort(0x00F0)) >> UShort(4)));
		c.z = Float(Int(*Pointer<UShort>(element) & UShort(0x000F)));
		break;
	case VK_FORMAT_R5G6B5_UNORM_PACK16:
		c.x = Float(Int((*Pointer<UShort>(element) & UShort(0xF800)) >> UShort(11)));
		c.y = Float(Int((*Pointer<UShort>(element) & UShort(0x07E0)) >> UShort(5)));
		c.z = Float(Int(*Pointer<UShort>(element) & UShort(0x001F)));
		break;
	case VK_FORMAT_B5G6R5_UNORM_PACK16:
		c.z = Float(Int((*Pointer<UShort>(element) & UShort(0xF800)) >> UShort(11)));
		c.y = Float(Int((*Pointer<UShort>(element) & UShort(0x07E0)) >> UShort(5)));
		c.x = Float(Int(*Pointer<UShort>(element) & UShort(0x001F)));
		break;
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
		c.x = Float(Int((*Pointer<UShort>(element) & UShort(0xF800)) >> UShort(11)));
		c.y = Float(Int((*Pointer<UShort>(element) & UShort(0x07C0)) >> UShort(6)));
		c.z = Float(Int((*Pointer<UShort>(element) & UShort(0x003E)) >> UShort(1)));
		c.w = Float(Int(*Pointer<UShort>(element) & UShort(0x0001)));
		break;
	case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
		c.z = Float(Int((*Pointer<UShort>(element) & UShort(0xF800)) >> UShort(11)));
		c.y = Float(Int((*Pointer<UShort>(element) & UShort(0x07C0)) >> UShort(6)));
		c.x = Float(Int((*Pointer<UShort>(element) & UShort(0x003E)) >> UShort(1)));
		c.w = Float(Int(*Pointer<UShort>(element) & UShort(0x0001)));
		break;
	case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
		c.w = Float(Int((*Pointer<UShort>(element) & UShort(0x8000)) >> UShort(15)));
		c.x = Float(Int((*Pointer<UShort>(element) & UShort(0x7C00)) >> UShort(10)));
		c.y = Float(Int((*Pointer<UShort>(element) & UShort(0x03E0)) >> UShort(5)));
		c.z = Float(Int(*Pointer<UShort>(element) & UShort(0x001F)));
		break;
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:
		c.x = Float(Int((*Pointer<UInt>(element) & UInt(0x000003FF))));
		c.y = Float(Int((*Pointer<UInt>(element) & UInt(0x000FFC00)) >> 10));
		c.z = Float(Int((*Pointer<UInt>(element) & UInt(0x3FF00000)) >> 20));
		c.w = Float(Int((*Pointer<UInt>(element) & UInt(0xC0000000)) >> 30));
		break;
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
		c.z = Float(Int((*Pointer<UInt>(element) & UInt(0x000003FF))));
		c.y = Float(Int((*Pointer<UInt>(element) & UInt(0x000FFC00)) >> 10));
		c.x = Float(Int((*Pointer<UInt>(element) & UInt(0x3FF00000)) >> 20));
		c.w = Float(Int((*Pointer<UInt>(element) & UInt(0xC0000000)) >> 30));
		break;
	case VK_FORMAT_D16_UNORM:
		c.x = Float(Int((*Pointer<UShort>(element))));
		break;
	case VK_FORMAT_X8_D24_UNORM_PACK32:
		c.x = Float(Int((*Pointer<UInt>(element) & UInt(0xFFFFFF00)) >> 8));
		break;
	case VK_FORMAT_D32_SFLOAT:
		c.x = *Pointer<Float>(element);
		break;
	case VK_FORMAT_S8_UINT:
		c.x = Float(Int(*Pointer<Byte>(element)));
		break;
	default:
		UNSUPPORTED("Blitter source format %d", (int)state.sourceFormat);
	}

	return c;
}

void Blitter::write(Float4 &c, Pointer<Byte> element, const State &state)
{
	bool writeR = state.writeRed;
	bool writeG = state.writeGreen;
	bool writeB = state.writeBlue;
	bool writeA = state.writeAlpha;
	bool writeRGBA = writeR && writeG && writeB && writeA;

	switch(state.destFormat)
	{
	case VK_FORMAT_R4G4_UNORM_PACK8:
		if(writeR | writeG)
		{
			if(!writeR)
			{
				*Pointer<Byte>(element) = (Byte(RoundInt(Float(c.y))) & Byte(0xF)) |
				                          (*Pointer<Byte>(element) & Byte(0xF0));
			}
			else if(!writeG)
			{
				*Pointer<Byte>(element) = (*Pointer<Byte>(element) & Byte(0xF)) |
				                          (Byte(RoundInt(Float(c.x))) << Byte(4));
			}
			else
			{
				*Pointer<Byte>(element) = (Byte(RoundInt(Float(c.y))) & Byte(0xF)) |
				                          (Byte(RoundInt(Float(c.x))) << Byte(4));
			}
		}
		break;
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
		if(writeRGBA)
		{
			*Pointer<UShort>(element) = UShort(PackFields(RoundInt(c) & Int4(0xF), { 12, 8, 4, 0 }));
		}
		else
		{
			unsigned short mask = (writeA ? 0x000F : 0x0000) |
			                      (writeB ? 0x00F0 : 0x0000) |
			                      (writeG ? 0x0F00 : 0x0000) |
			                      (writeR ? 0xF000 : 0x0000);
			unsigned short unmask = ~mask;
			*Pointer<UShort>(element) = (*Pointer<UShort>(element) & UShort(unmask)) |
			                            (UShort(PackFields(RoundInt(c) & Int4(0xF), { 12, 8, 4, 0 })) & UShort(mask));
		}
		break;
	case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
		if(writeRGBA)
		{
			*Pointer<UShort>(element) = UShort(PackFields(RoundInt(c) & Int4(0xF), { 4, 8, 12, 0 }));
		}
		else
		{
			unsigned short mask = (writeA ? 0x000F : 0x0000) |
			                      (writeR ? 0x00F0 : 0x0000) |
			                      (writeG ? 0x0F00 : 0x0000) |
			                      (writeB ? 0xF000 : 0x0000);
			unsigned short unmask = ~mask;
			*Pointer<UShort>(element) = (*Pointer<UShort>(element) & UShort(unmask)) |
			                            (UShort(PackFields(RoundInt(c) & Int4(0xF), { 4, 8, 12, 0 })) & UShort(mask));
		}
		break;
	case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
		if(writeRGBA)
		{
			*Pointer<UShort>(element) = UShort(PackFields(RoundInt(c) & Int4(0xF), { 8, 4, 0, 12 }));
		}
		else
		{
			unsigned short mask = (writeB ? 0x000F : 0x0000) |
			                      (writeG ? 0x00F0 : 0x0000) |
			                      (writeR ? 0x0F00 : 0x0000) |
			                      (writeA ? 0xF000 : 0x0000);
			unsigned short unmask = ~mask;
			*Pointer<UShort>(element) = (*Pointer<UShort>(element) & UShort(unmask)) |
			                            (UShort(PackFields(RoundInt(c) & Int4(0xF), { 8, 4, 0, 12 })) & UShort(mask));
		}
		break;
	case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
		if(writeRGBA)
		{
			*Pointer<UShort>(element) = UShort(PackFields(RoundInt(c) & Int4(0xF), { 0, 4, 8, 12 }));
		}
		else
		{
			unsigned short mask = (writeR ? 0x000F : 0x0000) |
			                      (writeG ? 0x00F0 : 0x0000) |
			                      (writeB ? 0x0F00 : 0x0000) |
			                      (writeA ? 0xF000 : 0x0000);
			unsigned short unmask = ~mask;
			*Pointer<UShort>(element) = (*Pointer<UShort>(element) & UShort(unmask)) |
			                            (UShort(PackFields(RoundInt(c) & Int4(0xF), { 0, 4, 8, 12 })) & UShort(mask));
		}
		break;
	case VK_FORMAT_B8G8R8A8_SRGB:
	case VK_FORMAT_B8G8R8A8_UNORM:
		if(writeRGBA)
		{
			Short4 c0 = RoundShort4(c.zyxw);
			*Pointer<Byte4>(element) = Byte4(PackUnsigned(c0, c0));
		}
		else
		{
			if(writeB) { *Pointer<Byte>(element + 0) = Byte(RoundInt(Float(c.z))); }
			if(writeG) { *Pointer<Byte>(element + 1) = Byte(RoundInt(Float(c.y))); }
			if(writeR) { *Pointer<Byte>(element + 2) = Byte(RoundInt(Float(c.x))); }
			if(writeA) { *Pointer<Byte>(element + 3) = Byte(RoundInt(Float(c.w))); }
		}
		break;
	case VK_FORMAT_B8G8R8_SNORM:
		if(writeB) { *Pointer<SByte>(element + 0) = SByte(RoundInt(Float(c.z))); }
		if(writeG) { *Pointer<SByte>(element + 1) = SByte(RoundInt(Float(c.y))); }
		if(writeR) { *Pointer<SByte>(element + 2) = SByte(RoundInt(Float(c.x))); }
		break;
	case VK_FORMAT_B8G8R8_UNORM:
	case VK_FORMAT_B8G8R8_SRGB:
		if(writeB) { *Pointer<Byte>(element + 0) = Byte(RoundInt(Float(c.z))); }
		if(writeG) { *Pointer<Byte>(element + 1) = Byte(RoundInt(Float(c.y))); }
		if(writeR) { *Pointer<Byte>(element + 2) = Byte(RoundInt(Float(c.x))); }
		break;
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
	case VK_FORMAT_R8G8B8A8_SRGB:
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
	case VK_FORMAT_R8G8B8A8_UINT:
	case VK_FORMAT_R8G8B8A8_USCALED:
	case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
		if(writeRGBA)
		{
			Short4 c0 = RoundShort4(c);
			*Pointer<Byte4>(element) = Byte4(PackUnsigned(c0, c0));
		}
		else
		{
			if(writeR) { *Pointer<Byte>(element + 0) = Byte(RoundInt(Float(c.x))); }
			if(writeG) { *Pointer<Byte>(element + 1) = Byte(RoundInt(Float(c.y))); }
			if(writeB) { *Pointer<Byte>(element + 2) = Byte(RoundInt(Float(c.z))); }
			if(writeA) { *Pointer<Byte>(element + 3) = Byte(RoundInt(Float(c.w))); }
		}
		break;
	case VK_FORMAT_R32G32B32A32_SFLOAT:
		if(writeRGBA)
		{
			*Pointer<Float4>(element) = c;
		}
		else
		{
			if(writeR) { *Pointer<Float>(element) = c.x; }
			if(writeG) { *Pointer<Float>(element + 4) = c.y; }
			if(writeB) { *Pointer<Float>(element + 8) = c.z; }
			if(writeA) { *Pointer<Float>(element + 12) = c.w; }
		}
		break;
	case VK_FORMAT_R32G32B32_SFLOAT:
		if(writeR) { *Pointer<Float>(element) = c.x; }
		if(writeG) { *Pointer<Float>(element + 4) = c.y; }
		if(writeB) { *Pointer<Float>(element + 8) = c.z; }
		break;
	case VK_FORMAT_R32G32_SFLOAT:
		if(writeR && writeG)
		{
			*Pointer<Float2>(element) = Float2(c);
		}
		else
		{
			if(writeR) { *Pointer<Float>(element) = c.x; }
			if(writeG) { *Pointer<Float>(element + 4) = c.y; }
		}
		break;
	case VK_FORMAT_R32_SFLOAT:
		if(writeR) { *Pointer<Float>(element) = c.x; }
		break;
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		if(writeA) { *Pointer<Half>(element + 6) = Half(c.w); }
		// [[fallthrough]]
	case VK_FORMAT_R16G16B16_SFLOAT:
		if(writeB) { *Pointer<Half>(element + 4) = Half(c.z); }
		// [[fallthrough]]
	case VK_FORMAT_R16G16_SFLOAT:
		if(writeG) { *Pointer<Half>(element + 2) = Half(c.y); }
		// [[fallthrough]]
	case VK_FORMAT_R16_SFLOAT:
		if(writeR) { *Pointer<Half>(element) = Half(c.x); }
		break;
	case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
		{
			UInt rgb = r11g11b10Pack(c);

			UInt old = *Pointer<UInt>(element);

			unsigned int mask = (writeR ? 0x000007FF : 0) |
			                    (writeG ? 0x003FF800 : 0) |
			                    (writeB ? 0xFFC00000 : 0);

			*Pointer<UInt>(element) = (rgb & mask) | (old & ~mask);
		}
		break;
	case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
		{
			ASSERT(writeRGBA);  // Can't sensibly write just part of this format.

			// Vulkan 1.1.117 section 15.2.1 RGB to Shared Exponent Conversion

			constexpr int N = 9;       // number of mantissa bits per component
			constexpr int B = 15;      // exponent bias
			constexpr int E_max = 31;  // maximum possible biased exponent value

			// Maximum representable value.
			constexpr float sharedexp_max = ((static_cast<float>(1 << N) - 1) / static_cast<float>(1 << N)) * static_cast<float>(1 << (E_max - B));

			// Clamp components to valid range. NaN becomes 0.
			Float red_c = Min(IfThenElse(!(c.x > 0), Float(0), Float(c.x)), sharedexp_max);
			Float green_c = Min(IfThenElse(!(c.y > 0), Float(0), Float(c.y)), sharedexp_max);
			Float blue_c = Min(IfThenElse(!(c.z > 0), Float(0), Float(c.z)), sharedexp_max);

			// We're reducing the mantissa to 9 bits, so we must round up if the next
			// bit is 1. In other words add 0.5 to the new mantissa's position and
			// allow overflow into the exponent so we can scale correctly.
			constexpr int half = 1 << (23 - N);
			Float red_r = As<Float>(As<Int>(red_c) + half);
			Float green_r = As<Float>(As<Int>(green_c) + half);
			Float blue_r = As<Float>(As<Int>(blue_c) + half);

			// The largest component determines the shared exponent. It can't be lower
			// than 0 (after bias subtraction) so also limit to the mimimum representable.
			constexpr float min_s = 0.5f / (1 << B);
			Float max_s = Max(Max(red_r, green_r), Max(blue_r, min_s));

			// Obtain the reciprocal of the shared exponent by inverting the bits,
			// and scale by the new mantissa's size. Note that the IEEE-754 single-precision
			// format has an implicit leading 1, but this shared component format does not.
			Float scale = As<Float>((As<Int>(max_s) & 0x7F800000) ^ 0x7F800000) * (1 << (N - 2));

			UInt R9 = RoundInt(red_c * scale);
			UInt G9 = UInt(RoundInt(green_c * scale));
			UInt B9 = UInt(RoundInt(blue_c * scale));
			UInt E5 = (As<UInt>(max_s) >> 23) - 127 + 15 + 1;

			UInt E5B9G9R9 = (E5 << 27) | (B9 << 18) | (G9 << 9) | R9;

			*Pointer<UInt>(element) = E5B9G9R9;
		}
		break;
	case VK_FORMAT_B8G8R8A8_SNORM:
		if(writeB) { *Pointer<SByte>(element) = SByte(RoundInt(Float(c.z))); }
		if(writeG) { *Pointer<SByte>(element + 1) = SByte(RoundInt(Float(c.y))); }
		if(writeR) { *Pointer<SByte>(element + 2) = SByte(RoundInt(Float(c.x))); }
		if(writeA) { *Pointer<SByte>(element + 3) = SByte(RoundInt(Float(c.w))); }
		break;
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
	case VK_FORMAT_R8G8B8A8_SNORM:
	case VK_FORMAT_R8G8B8A8_SSCALED:
	case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
		if(writeA) { *Pointer<SByte>(element + 3) = SByte(RoundInt(Float(c.w))); }
		// [[fallthrough]]
	case VK_FORMAT_R8G8B8_SINT:
	case VK_FORMAT_R8G8B8_SNORM:
	case VK_FORMAT_R8G8B8_SSCALED:
		if(writeB) { *Pointer<SByte>(element + 2) = SByte(RoundInt(Float(c.z))); }
		// [[fallthrough]]
	case VK_FORMAT_R8G8_SINT:
	case VK_FORMAT_R8G8_SNORM:
	case VK_FORMAT_R8G8_SSCALED:
		if(writeG) { *Pointer<SByte>(element + 1) = SByte(RoundInt(Float(c.y))); }
		// [[fallthrough]]
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_R8_SNORM:
	case VK_FORMAT_R8_SSCALED:
		if(writeR) { *Pointer<SByte>(element) = SByte(RoundInt(Float(c.x))); }
		break;
	case VK_FORMAT_R8G8B8_UINT:
	case VK_FORMAT_R8G8B8_UNORM:
	case VK_FORMAT_R8G8B8_USCALED:
	case VK_FORMAT_R8G8B8_SRGB:
		if(writeB) { *Pointer<Byte>(element + 2) = Byte(RoundInt(Float(c.z))); }
		// [[fallthrough]]
	case VK_FORMAT_R8G8_UINT:
	case VK_FORMAT_R8G8_UNORM:
	case VK_FORMAT_R8G8_USCALED:
	case VK_FORMAT_R8G8_SRGB:
		if(writeG) { *Pointer<Byte>(element + 1) = Byte(RoundInt(Float(c.y))); }
		// [[fallthrough]]
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_R8_UNORM:
	case VK_FORMAT_R8_USCALED:
	case VK_FORMAT_R8_SRGB:
		if(writeR) { *Pointer<Byte>(element) = Byte(RoundInt(Float(c.x))); }
		break;
	case VK_FORMAT_R16G16B16A16_SINT:
	case VK_FORMAT_R16G16B16A16_SNORM:
	case VK_FORMAT_R16G16B16A16_SSCALED:
		if(writeRGBA)
		{
			*Pointer<Short4>(element) = Short4(RoundInt(c));
		}
		else
		{
			if(writeR) { *Pointer<Short>(element) = Short(RoundInt(Float(c.x))); }
			if(writeG) { *Pointer<Short>(element + 2) = Short(RoundInt(Float(c.y))); }
			if(writeB) { *Pointer<Short>(element + 4) = Short(RoundInt(Float(c.z))); }
			if(writeA) { *Pointer<Short>(element + 6) = Short(RoundInt(Float(c.w))); }
		}
		break;
	case VK_FORMAT_R16G16B16_SINT:
	case VK_FORMAT_R16G16B16_SNORM:
	case VK_FORMAT_R16G16B16_SSCALED:
		if(writeR) { *Pointer<Short>(element) = Short(RoundInt(Float(c.x))); }
		if(writeG) { *Pointer<Short>(element + 2) = Short(RoundInt(Float(c.y))); }
		if(writeB) { *Pointer<Short>(element + 4) = Short(RoundInt(Float(c.z))); }
		break;
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R16G16_SNORM:
	case VK_FORMAT_R16G16_SSCALED:
		if(writeR && writeG)
		{
			*Pointer<Short2>(element) = Short2(Short4(RoundInt(c)));
		}
		else
		{
			if(writeR) { *Pointer<Short>(element) = Short(RoundInt(Float(c.x))); }
			if(writeG) { *Pointer<Short>(element + 2) = Short(RoundInt(Float(c.y))); }
		}
		break;
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_R16_SNORM:
	case VK_FORMAT_R16_SSCALED:
		if(writeR) { *Pointer<Short>(element) = Short(RoundInt(Float(c.x))); }
		break;
	case VK_FORMAT_R16G16B16A16_UINT:
	case VK_FORMAT_R16G16B16A16_UNORM:
	case VK_FORMAT_R16G16B16A16_USCALED:
		if(writeRGBA)
		{
			*Pointer<UShort4>(element) = UShort4(RoundInt(c));
		}
		else
		{
			if(writeR) { *Pointer<UShort>(element) = UShort(RoundInt(Float(c.x))); }
			if(writeG) { *Pointer<UShort>(element + 2) = UShort(RoundInt(Float(c.y))); }
			if(writeB) { *Pointer<UShort>(element + 4) = UShort(RoundInt(Float(c.z))); }
			if(writeA) { *Pointer<UShort>(element + 6) = UShort(RoundInt(Float(c.w))); }
		}
		break;
	case VK_FORMAT_R16G16B16_UINT:
	case VK_FORMAT_R16G16B16_UNORM:
	case VK_FORMAT_R16G16B16_USCALED:
		if(writeR) { *Pointer<UShort>(element) = UShort(RoundInt(Float(c.x))); }
		if(writeG) { *Pointer<UShort>(element + 2) = UShort(RoundInt(Float(c.y))); }
		if(writeB) { *Pointer<UShort>(element + 4) = UShort(RoundInt(Float(c.z))); }
		break;
	case VK_FORMAT_R16G16_UINT:
	case VK_FORMAT_R16G16_UNORM:
	case VK_FORMAT_R16G16_USCALED:
		if(writeR && writeG)
		{
			*Pointer<UShort2>(element) = UShort2(UShort4(RoundInt(c)));
		}
		else
		{
			if(writeR) { *Pointer<UShort>(element) = UShort(RoundInt(Float(c.x))); }
			if(writeG) { *Pointer<UShort>(element + 2) = UShort(RoundInt(Float(c.y))); }
		}
		break;
	case VK_FORMAT_R16_UINT:
	case VK_FORMAT_R16_UNORM:
	case VK_FORMAT_R16_USCALED:
		if(writeR) { *Pointer<UShort>(element) = UShort(RoundInt(Float(c.x))); }
		break;
	case VK_FORMAT_R32G32B32A32_SINT:
		if(writeRGBA)
		{
			*Pointer<Int4>(element) = RoundInt(c);
		}
		else
		{
			if(writeR) { *Pointer<Int>(element) = RoundInt(Float(c.x)); }
			if(writeG) { *Pointer<Int>(element + 4) = RoundInt(Float(c.y)); }
			if(writeB) { *Pointer<Int>(element + 8) = RoundInt(Float(c.z)); }
			if(writeA) { *Pointer<Int>(element + 12) = RoundInt(Float(c.w)); }
		}
		break;
	case VK_FORMAT_R32G32B32_SINT:
		if(writeB) { *Pointer<Int>(element + 8) = RoundInt(Float(c.z)); }
		// [[fallthrough]]
	case VK_FORMAT_R32G32_SINT:
		if(writeG) { *Pointer<Int>(element + 4) = RoundInt(Float(c.y)); }
		// [[fallthrough]]
	case VK_FORMAT_R32_SINT:
		if(writeR) { *Pointer<Int>(element) = RoundInt(Float(c.x)); }
		break;
	case VK_FORMAT_R32G32B32A32_UINT:
		if(writeRGBA)
		{
			*Pointer<UInt4>(element) = UInt4(RoundInt(c));
		}
		else
		{
			if(writeR) { *Pointer<UInt>(element) = As<UInt>(RoundInt(Float(c.x))); }
			if(writeG) { *Pointer<UInt>(element + 4) = As<UInt>(RoundInt(Float(c.y))); }
			if(writeB) { *Pointer<UInt>(element + 8) = As<UInt>(RoundInt(Float(c.z))); }
			if(writeA) { *Pointer<UInt>(element + 12) = As<UInt>(RoundInt(Float(c.w))); }
		}
		break;
	case VK_FORMAT_R32G32B32_UINT:
		if(writeB) { *Pointer<UInt>(element + 8) = As<UInt>(RoundInt(Float(c.z))); }
		// [[fallthrough]]
	case VK_FORMAT_R32G32_UINT:
		if(writeG) { *Pointer<UInt>(element + 4) = As<UInt>(RoundInt(Float(c.y))); }
		// [[fallthrough]]
	case VK_FORMAT_R32_UINT:
		if(writeR) { *Pointer<UInt>(element) = As<UInt>(RoundInt(Float(c.x))); }
		break;
	case VK_FORMAT_R5G6B5_UNORM_PACK16:
		if(writeR && writeG && writeB)
		{
			*Pointer<UShort>(element) = UShort(PackFields(RoundInt(c.xyzz), { 11, 5, 0, 0 }));
		}
		else
		{
			unsigned short mask = (writeB ? 0x001F : 0x0000) | (writeG ? 0x07E0 : 0x0000) | (writeR ? 0xF800 : 0x0000);
			unsigned short unmask = ~mask;
			*Pointer<UShort>(element) = (*Pointer<UShort>(element) & UShort(unmask)) |
			                            (UShort(PackFields(RoundInt(c.xyzz), { 11, 5, 0, 0 })) &
			                             UShort(mask));
		}
		break;
	case VK_FORMAT_B5G6R5_UNORM_PACK16:
		if(writeR && writeG && writeB)
		{
			*Pointer<UShort>(element) = UShort(PackFields(RoundInt(c.zyxx), { 11, 5, 0, 0 }));
		}
		else
		{
			unsigned short mask = (writeR ? 0x001F : 0x0000) | (writeG ? 0x07E0 : 0x0000) | (writeB ? 0xF800 : 0x0000);
			unsigned short unmask = ~mask;
			*Pointer<UShort>(element) = (*Pointer<UShort>(element) & UShort(unmask)) |
			                            (UShort(PackFields(RoundInt(c.zyxx), { 11, 5, 0, 0 })) &
			                             UShort(mask));
		}
		break;
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
		if(writeRGBA)
		{
			*Pointer<UShort>(element) = UShort(PackFields(RoundInt(c), { 11, 6, 1, 0 }));
		}
		else
		{
			unsigned short mask = (writeA ? 0x8000 : 0x0000) |
			                      (writeR ? 0x7C00 : 0x0000) |
			                      (writeG ? 0x03E0 : 0x0000) |
			                      (writeB ? 0x001F : 0x0000);
			unsigned short unmask = ~mask;
			*Pointer<UShort>(element) = (*Pointer<UShort>(element) & UShort(unmask)) |
			                            (UShort(PackFields(RoundInt(c), { 11, 6, 1, 0 })) &
			                             UShort(mask));
		}
		break;
	case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
		if(writeRGBA)
		{
			*Pointer<UShort>(element) = UShort(PackFields(RoundInt(c), { 1, 6, 11, 0 }));
		}
		else
		{
			unsigned short mask = (writeA ? 0x8000 : 0x0000) |
			                      (writeR ? 0x7C00 : 0x0000) |
			                      (writeG ? 0x03E0 : 0x0000) |
			                      (writeB ? 0x001F : 0x0000);
			unsigned short unmask = ~mask;
			*Pointer<UShort>(element) = (*Pointer<UShort>(element) & UShort(unmask)) |
			                            (UShort(PackFields(RoundInt(c), { 1, 6, 11, 0 })) &
			                             UShort(mask));
		}
		break;
	case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
		if(writeRGBA)
		{
			*Pointer<UShort>(element) = UShort(PackFields(RoundInt(c), { 10, 5, 0, 15 }));
		}
		else
		{
			unsigned short mask = (writeA ? 0x8000 : 0x0000) |
			                      (writeR ? 0x7C00 : 0x0000) |
			                      (writeG ? 0x03E0 : 0x0000) |
			                      (writeB ? 0x001F : 0x0000);
			unsigned short unmask = ~mask;
			*Pointer<UShort>(element) = (*Pointer<UShort>(element) & UShort(unmask)) |
			                            (UShort(PackFields(RoundInt(c), { 10, 5, 0, 15 })) &
			                             UShort(mask));
		}
		break;
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:
	case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
		if(writeRGBA)
		{
			*Pointer<UInt>(element) = As<UInt>(PackFields(RoundInt(c), { 0, 10, 20, 30 }));
		}
		else
		{
			unsigned int mask = (writeA ? 0xC0000000 : 0x0000) |
			                    (writeB ? 0x3FF00000 : 0x0000) |
			                    (writeG ? 0x000FFC00 : 0x0000) |
			                    (writeR ? 0x000003FF : 0x0000);
			unsigned int unmask = ~mask;
			*Pointer<UInt>(element) = (*Pointer<UInt>(element) & UInt(unmask)) |
			                          (As<UInt>(PackFields(RoundInt(c), { 0, 10, 20, 30 })) &
			                           UInt(mask));
		}
		break;
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
	case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
		if(writeRGBA)
		{
			*Pointer<UInt>(element) = As<UInt>(PackFields(RoundInt(c), { 20, 10, 0, 30 }));
		}
		else
		{
			unsigned int mask = (writeA ? 0xC0000000 : 0x0000) |
			                    (writeR ? 0x3FF00000 : 0x0000) |
			                    (writeG ? 0x000FFC00 : 0x0000) |
			                    (writeB ? 0x000003FF : 0x0000);
			unsigned int unmask = ~mask;
			*Pointer<UInt>(element) = (*Pointer<UInt>(element) & UInt(unmask)) |
			                          (As<UInt>(PackFields(RoundInt(c), { 20, 10, 0, 30 })) &
			                           UInt(mask));
		}
		break;
	case VK_FORMAT_D16_UNORM:
		*Pointer<UShort>(element) = UShort(RoundInt(Float(c.x)));
		break;
	case VK_FORMAT_X8_D24_UNORM_PACK32:
		*Pointer<UInt>(element) = UInt(RoundInt(Float(c.x)) << 8);
		break;
	case VK_FORMAT_D32_SFLOAT:
		*Pointer<Float>(element) = c.x;
		break;
	case VK_FORMAT_S8_UINT:
		*Pointer<Byte>(element) = Byte(RoundInt(Float(c.x)));
		break;
	default:
		UNSUPPORTED("Blitter destination format %d", (int)state.destFormat);
		break;
	}
}

Int4 Blitter::readInt4(Pointer<Byte> element, const State &state)
{
	Int4 c(0, 0, 0, 1);

	switch(state.sourceFormat)
	{
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
	case VK_FORMAT_R8G8B8A8_SINT:
		c = Insert(c, Int(*Pointer<SByte>(element + 3)), 3);
		c = Insert(c, Int(*Pointer<SByte>(element + 2)), 2);
		// [[fallthrough]]
	case VK_FORMAT_R8G8_SINT:
		c = Insert(c, Int(*Pointer<SByte>(element + 1)), 1);
		// [[fallthrough]]
	case VK_FORMAT_R8_SINT:
		c = Insert(c, Int(*Pointer<SByte>(element)), 0);
		break;
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:
		c = Insert(c, Int((*Pointer<UInt>(element) & UInt(0x000003FF))), 0);
		c = Insert(c, Int((*Pointer<UInt>(element) & UInt(0x000FFC00)) >> 10), 1);
		c = Insert(c, Int((*Pointer<UInt>(element) & UInt(0x3FF00000)) >> 20), 2);
		c = Insert(c, Int((*Pointer<UInt>(element) & UInt(0xC0000000)) >> 30), 3);
		break;
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
		c = Insert(c, Int((*Pointer<UInt>(element) & UInt(0x000003FF))), 2);
		c = Insert(c, Int((*Pointer<UInt>(element) & UInt(0x000FFC00)) >> 10), 1);
		c = Insert(c, Int((*Pointer<UInt>(element) & UInt(0x3FF00000)) >> 20), 0);
		c = Insert(c, Int((*Pointer<UInt>(element) & UInt(0xC0000000)) >> 30), 3);
		break;
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
	case VK_FORMAT_R8G8B8A8_UINT:
		c = Insert(c, Int(*Pointer<Byte>(element + 3)), 3);
		c = Insert(c, Int(*Pointer<Byte>(element + 2)), 2);
		// [[fallthrough]]
	case VK_FORMAT_R8G8_UINT:
		c = Insert(c, Int(*Pointer<Byte>(element + 1)), 1);
		// [[fallthrough]]
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_S8_UINT:
		c = Insert(c, Int(*Pointer<Byte>(element)), 0);
		break;
	case VK_FORMAT_R16G16B16A16_SINT:
		c = Insert(c, Int(*Pointer<Short>(element + 6)), 3);
		c = Insert(c, Int(*Pointer<Short>(element + 4)), 2);
		// [[fallthrough]]
	case VK_FORMAT_R16G16_SINT:
		c = Insert(c, Int(*Pointer<Short>(element + 2)), 1);
		// [[fallthrough]]
	case VK_FORMAT_R16_SINT:
		c = Insert(c, Int(*Pointer<Short>(element)), 0);
		break;
	case VK_FORMAT_R16G16B16A16_UINT:
		c = Insert(c, Int(*Pointer<UShort>(element + 6)), 3);
		c = Insert(c, Int(*Pointer<UShort>(element + 4)), 2);
		// [[fallthrough]]
	case VK_FORMAT_R16G16_UINT:
		c = Insert(c, Int(*Pointer<UShort>(element + 2)), 1);
		// [[fallthrough]]
	case VK_FORMAT_R16_UINT:
		c = Insert(c, Int(*Pointer<UShort>(element)), 0);
		break;
	case VK_FORMAT_R32G32B32A32_SINT:
	case VK_FORMAT_R32G32B32A32_UINT:
		c = *Pointer<Int4>(element);
		break;
	case VK_FORMAT_R32G32_SINT:
	case VK_FORMAT_R32G32_UINT:
		c = Insert(c, *Pointer<Int>(element + 4), 1);
		// [[fallthrough]]
	case VK_FORMAT_R32_SINT:
	case VK_FORMAT_R32_UINT:
		c = Insert(c, *Pointer<Int>(element), 0);
		break;
	default:
		UNSUPPORTED("Blitter source format %d", (int)state.sourceFormat);
	}

	return c;
}

void Blitter::write(Int4 &c, Pointer<Byte> element, const State &state)
{
	bool writeR = state.writeRed;
	bool writeG = state.writeGreen;
	bool writeB = state.writeBlue;
	bool writeA = state.writeAlpha;
	bool writeRGBA = writeR && writeG && writeB && writeA;

	ASSERT(state.sourceFormat.isUnsigned() == state.destFormat.isUnsigned());

	switch(state.destFormat)
	{
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
		c = Min(As<UInt4>(c), UInt4(0x03FF, 0x03FF, 0x03FF, 0x0003));
		break;
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
	case VK_FORMAT_R8G8B8A8_UINT:
	case VK_FORMAT_R8G8B8_UINT:
	case VK_FORMAT_R8G8_UINT:
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_R8G8B8A8_USCALED:
	case VK_FORMAT_R8G8B8_USCALED:
	case VK_FORMAT_R8G8_USCALED:
	case VK_FORMAT_R8_USCALED:
	case VK_FORMAT_S8_UINT:
		c = Min(As<UInt4>(c), UInt4(0xFF));
		break;
	case VK_FORMAT_R16G16B16A16_UINT:
	case VK_FORMAT_R16G16B16_UINT:
	case VK_FORMAT_R16G16_UINT:
	case VK_FORMAT_R16_UINT:
	case VK_FORMAT_R16G16B16A16_USCALED:
	case VK_FORMAT_R16G16B16_USCALED:
	case VK_FORMAT_R16G16_USCALED:
	case VK_FORMAT_R16_USCALED:
		c = Min(As<UInt4>(c), UInt4(0xFFFF));
		break;
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_R8G8_SINT:
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_R8G8B8A8_SSCALED:
	case VK_FORMAT_R8G8B8_SSCALED:
	case VK_FORMAT_R8G8_SSCALED:
	case VK_FORMAT_R8_SSCALED:
		c = Min(Max(c, Int4(-0x80)), Int4(0x7F));
		break;
	case VK_FORMAT_R16G16B16A16_SINT:
	case VK_FORMAT_R16G16B16_SINT:
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_R16G16B16A16_SSCALED:
	case VK_FORMAT_R16G16B16_SSCALED:
	case VK_FORMAT_R16G16_SSCALED:
	case VK_FORMAT_R16_SSCALED:
		c = Min(Max(c, Int4(-0x8000)), Int4(0x7FFF));
		break;
	default:
		break;
	}

	switch(state.destFormat)
	{
	case VK_FORMAT_B8G8R8A8_SINT:
	case VK_FORMAT_B8G8R8A8_SSCALED:
		if(writeA) { *Pointer<SByte>(element + 3) = SByte(Extract(c, 3)); }
		// [[fallthrough]]
	case VK_FORMAT_B8G8R8_SINT:
	case VK_FORMAT_B8G8R8_SSCALED:
		if(writeB) { *Pointer<SByte>(element) = SByte(Extract(c, 2)); }
		if(writeG) { *Pointer<SByte>(element + 1) = SByte(Extract(c, 1)); }
		if(writeR) { *Pointer<SByte>(element + 2) = SByte(Extract(c, 0)); }
		break;
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_R8G8B8A8_SSCALED:
	case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
		if(writeA) { *Pointer<SByte>(element + 3) = SByte(Extract(c, 3)); }
		// [[fallthrough]]
	case VK_FORMAT_R8G8B8_SINT:
	case VK_FORMAT_R8G8B8_SSCALED:
		if(writeB) { *Pointer<SByte>(element + 2) = SByte(Extract(c, 2)); }
		// [[fallthrough]]
	case VK_FORMAT_R8G8_SINT:
	case VK_FORMAT_R8G8_SSCALED:
		if(writeG) { *Pointer<SByte>(element + 1) = SByte(Extract(c, 1)); }
		// [[fallthrough]]
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_R8_SSCALED:
		if(writeR) { *Pointer<SByte>(element) = SByte(Extract(c, 0)); }
		break;
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:
	case VK_FORMAT_A2B10G10R10_SINT_PACK32:
	case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
	case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
		if(writeRGBA)
		{
			*Pointer<UInt>(element) = As<UInt>(PackFields(c, { 0, 10, 20, 30 }));
		}
		else
		{
			unsigned int mask = (writeA ? 0xC0000000 : 0x0000) |
			                    (writeB ? 0x3FF00000 : 0x0000) |
			                    (writeG ? 0x000FFC00 : 0x0000) |
			                    (writeR ? 0x000003FF : 0x0000);
			unsigned int unmask = ~mask;
			*Pointer<UInt>(element) = (*Pointer<UInt>(element) & UInt(unmask)) |
			                          (As<UInt>(PackFields(c, { 0, 10, 20, 30 })) & UInt(mask));
		}
		break;
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
	case VK_FORMAT_A2R10G10B10_SINT_PACK32:
	case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
	case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
		if(writeRGBA)
		{
			*Pointer<UInt>(element) = As<UInt>(PackFields(c, { 20, 10, 0, 30 }));
		}
		else
		{
			unsigned int mask = (writeA ? 0xC0000000 : 0x0000) |
			                    (writeR ? 0x3FF00000 : 0x0000) |
			                    (writeG ? 0x000FFC00 : 0x0000) |
			                    (writeB ? 0x000003FF : 0x0000);
			unsigned int unmask = ~mask;
			*Pointer<UInt>(element) = (*Pointer<UInt>(element) & UInt(unmask)) |
			                          (As<UInt>(PackFields(c, { 20, 10, 0, 30 })) & UInt(mask));
		}
		break;
	case VK_FORMAT_B8G8R8A8_UINT:
	case VK_FORMAT_B8G8R8A8_USCALED:
		if(writeA) { *Pointer<Byte>(element + 3) = Byte(Extract(c, 3)); }
		// [[fallthrough]]
	case VK_FORMAT_B8G8R8_UINT:
	case VK_FORMAT_B8G8R8_USCALED:
	case VK_FORMAT_B8G8R8_SRGB:
		if(writeB) { *Pointer<Byte>(element) = Byte(Extract(c, 2)); }
		if(writeG) { *Pointer<Byte>(element + 1) = Byte(Extract(c, 1)); }
		if(writeR) { *Pointer<Byte>(element + 2) = Byte(Extract(c, 0)); }
		break;
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
	case VK_FORMAT_R8G8B8A8_UINT:
	case VK_FORMAT_R8G8B8A8_USCALED:
	case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
		if(writeA) { *Pointer<Byte>(element + 3) = Byte(Extract(c, 3)); }
		// [[fallthrough]]
	case VK_FORMAT_R8G8B8_UINT:
	case VK_FORMAT_R8G8B8_USCALED:
		if(writeB) { *Pointer<Byte>(element + 2) = Byte(Extract(c, 2)); }
		// [[fallthrough]]
	case VK_FORMAT_R8G8_UINT:
	case VK_FORMAT_R8G8_USCALED:
		if(writeG) { *Pointer<Byte>(element + 1) = Byte(Extract(c, 1)); }
		// [[fallthrough]]
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_R8_USCALED:
	case VK_FORMAT_S8_UINT:
		if(writeR) { *Pointer<Byte>(element) = Byte(Extract(c, 0)); }
		break;
	case VK_FORMAT_R16G16B16A16_SINT:
	case VK_FORMAT_R16G16B16A16_SSCALED:
		if(writeA) { *Pointer<Short>(element + 6) = Short(Extract(c, 3)); }
		// [[fallthrough]]
	case VK_FORMAT_R16G16B16_SINT:
	case VK_FORMAT_R16G16B16_SSCALED:
		if(writeB) { *Pointer<Short>(element + 4) = Short(Extract(c, 2)); }
		// [[fallthrough]]
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R16G16_SSCALED:
		if(writeG) { *Pointer<Short>(element + 2) = Short(Extract(c, 1)); }
		// [[fallthrough]]
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_R16_SSCALED:
		if(writeR) { *Pointer<Short>(element) = Short(Extract(c, 0)); }
		break;
	case VK_FORMAT_R16G16B16A16_UINT:
	case VK_FORMAT_R16G16B16A16_USCALED:
		if(writeA) { *Pointer<UShort>(element + 6) = UShort(Extract(c, 3)); }
		// [[fallthrough]]
	case VK_FORMAT_R16G16B16_UINT:
	case VK_FORMAT_R16G16B16_USCALED:
		if(writeB) { *Pointer<UShort>(element + 4) = UShort(Extract(c, 2)); }
		// [[fallthrough]]
	case VK_FORMAT_R16G16_UINT:
	case VK_FORMAT_R16G16_USCALED:
		if(writeG) { *Pointer<UShort>(element + 2) = UShort(Extract(c, 1)); }
		// [[fallthrough]]
	case VK_FORMAT_R16_UINT:
	case VK_FORMAT_R16_USCALED:
		if(writeR) { *Pointer<UShort>(element) = UShort(Extract(c, 0)); }
		break;
	case VK_FORMAT_R32G32B32A32_SINT:
		if(writeRGBA)
		{
			*Pointer<Int4>(element) = c;
		}
		else
		{
			if(writeR) { *Pointer<Int>(element) = Extract(c, 0); }
			if(writeG) { *Pointer<Int>(element + 4) = Extract(c, 1); }
			if(writeB) { *Pointer<Int>(element + 8) = Extract(c, 2); }
			if(writeA) { *Pointer<Int>(element + 12) = Extract(c, 3); }
		}
		break;
	case VK_FORMAT_R32G32B32_SINT:
		if(writeR) { *Pointer<Int>(element) = Extract(c, 0); }
		if(writeG) { *Pointer<Int>(element + 4) = Extract(c, 1); }
		if(writeB) { *Pointer<Int>(element + 8) = Extract(c, 2); }
		break;
	case VK_FORMAT_R32G32_SINT:
		if(writeR) { *Pointer<Int>(element) = Extract(c, 0); }
		if(writeG) { *Pointer<Int>(element + 4) = Extract(c, 1); }
		break;
	case VK_FORMAT_R32_SINT:
		if(writeR) { *Pointer<Int>(element) = Extract(c, 0); }
		break;
	case VK_FORMAT_R32G32B32A32_UINT:
		if(writeRGBA)
		{
			*Pointer<UInt4>(element) = As<UInt4>(c);
		}
		else
		{
			if(writeR) { *Pointer<UInt>(element) = As<UInt>(Extract(c, 0)); }
			if(writeG) { *Pointer<UInt>(element + 4) = As<UInt>(Extract(c, 1)); }
			if(writeB) { *Pointer<UInt>(element + 8) = As<UInt>(Extract(c, 2)); }
			if(writeA) { *Pointer<UInt>(element + 12) = As<UInt>(Extract(c, 3)); }
		}
		break;
	case VK_FORMAT_R32G32B32_UINT:
		if(writeB) { *Pointer<UInt>(element + 8) = As<UInt>(Extract(c, 2)); }
		// [[fallthrough]]
	case VK_FORMAT_R32G32_UINT:
		if(writeG) { *Pointer<UInt>(element + 4) = As<UInt>(Extract(c, 1)); }
		// [[fallthrough]]
	case VK_FORMAT_R32_UINT:
		if(writeR) { *Pointer<UInt>(element) = As<UInt>(Extract(c, 0)); }
		break;
	default:
		UNSUPPORTED("Blitter destination format %d", (int)state.destFormat);
	}
}

void Blitter::ApplyScaleAndClamp(Float4 &value, const State &state, bool preScaled)
{
	float4 scale{}, unscale{};

	if(state.clearOperation &&
	   state.sourceFormat.isUnnormalizedInteger() &&
	   !state.destFormat.isUnnormalizedInteger())
	{
		// If we're clearing a buffer from an int or uint color into a normalized color,
		// then the whole range of the int or uint color must be scaled between 0 and 1.
		switch(state.sourceFormat)
		{
		case VK_FORMAT_R32G32B32A32_SINT:
			unscale = float4(static_cast<float>(0x7FFFFFFF));
			break;
		case VK_FORMAT_R32G32B32A32_UINT:
			unscale = float4(static_cast<float>(0xFFFFFFFF));
			break;
		default:
			UNSUPPORTED("Blitter source format %d", (int)state.sourceFormat);
		}
	}
	else
	{
		unscale = state.sourceFormat.getScale();
	}

	scale = state.destFormat.getScale();

	bool srcSRGB = state.sourceFormat.isSRGBformat();
	bool dstSRGB = state.destFormat.isSRGBformat();

	if(state.allowSRGBConversion && ((srcSRGB && !preScaled) || dstSRGB))  // One of the formats is sRGB encoded.
	{
		value *= preScaled ? Float4(1.0f / scale.x, 1.0f / scale.y, 1.0f / scale.z, 1.0f / scale.w) :  // Unapply scale
		             Float4(1.0f / unscale.x, 1.0f / unscale.y, 1.0f / unscale.z, 1.0f / unscale.w);   // Apply unscale
		value.xyz = (srcSRGB && !preScaled) ? sRGBtoLinear(value) : linearToSRGB(value);
		value *= Float4(scale.x, scale.y, scale.z, scale.w);  // Apply scale
	}
	else if(unscale != scale)
	{
		value *= Float4(scale.x / unscale.x, scale.y / unscale.y, scale.z / unscale.z, scale.w / unscale.w);
	}

	if(state.sourceFormat.isFloatFormat() && !state.destFormat.isFloatFormat())
	{
		value = Min(value, Float4(scale.x, scale.y, scale.z, scale.w));

		value = Max(value, Float4(state.destFormat.isUnsignedComponent(0) ? 0.0f : -scale.x,
		                          state.destFormat.isUnsignedComponent(1) ? 0.0f : -scale.y,
		                          state.destFormat.isUnsignedComponent(2) ? 0.0f : -scale.z,
		                          state.destFormat.isUnsignedComponent(3) ? 0.0f : -scale.w));
	}

	if(!state.sourceFormat.isUnsigned() && state.destFormat.isUnsigned())
	{
		value = Max(value, Float4(0.0f));
	}
}

Int Blitter::ComputeOffset(Int &x, Int &y, Int &pitchB, int bytes)
{
	return y * pitchB + x * bytes;
}

Int Blitter::ComputeOffset(Int &x, Int &y, Int &z, Int &sliceB, Int &pitchB, int bytes)
{
	return z * sliceB + y * pitchB + x * bytes;
}

Float4 Blitter::sample(Pointer<Byte> &source, Float &x, Float &y, Float &z,
                       Int &sWidth, Int &sHeight, Int &sDepth,
                       Int &sSliceB, Int &sPitchB, const State &state)
{
	bool intSrc = state.sourceFormat.isUnnormalizedInteger();
	int srcBytes = state.sourceFormat.bytes();

	Float4 color;

	bool preScaled = false;
	if(!state.filter || intSrc)
	{
		Int X = Int(x);
		Int Y = Int(y);
		Int Z = Int(z);

		if(state.clampToEdge)
		{
			X = Clamp(X, 0, sWidth - 1);
			Y = Clamp(Y, 0, sHeight - 1);
			Z = Clamp(Z, 0, sDepth - 1);
		}

		Pointer<Byte> s = source + ComputeOffset(X, Y, Z, sSliceB, sPitchB, srcBytes);

		color = readFloat4(s, state);

		if(state.srcSamples > 1)  // Resolve multisampled source
		{
			if(state.allowSRGBConversion && state.sourceFormat.isSRGBformat())  // sRGB -> RGB
			{
				ApplyScaleAndClamp(color, state);
				preScaled = true;
			}
			Float4 accum = color;
			for(int sample = 1; sample < state.srcSamples; sample++)
			{
				s += sSliceB;
				color = readFloat4(s, state);

				if(state.allowSRGBConversion && state.sourceFormat.isSRGBformat())  // sRGB -> RGB
				{
					ApplyScaleAndClamp(color, state);
					preScaled = true;
				}
				accum += color;
			}
			color = accum * Float4(1.0f / static_cast<float>(state.srcSamples));
		}
	}
	else  // Bilinear filtering
	{
		Float X = x;
		Float Y = y;
		Float Z = z;

		if(state.clampToEdge)
		{
			X = Min(Max(x, 0.5f), Float(sWidth) - 0.5f);
			Y = Min(Max(y, 0.5f), Float(sHeight) - 0.5f);
			Z = Min(Max(z, 0.5f), Float(sDepth) - 0.5f);
		}

		Float x0 = X - 0.5f;
		Float y0 = Y - 0.5f;
		Float z0 = Z - 0.5f;

		Int X0 = Max(Int(x0), 0);
		Int Y0 = Max(Int(y0), 0);
		Int Z0 = Max(Int(z0), 0);

		Int X1 = X0 + 1;
		Int Y1 = Y0 + 1;
		X1 = IfThenElse(X1 >= sWidth, X0, X1);
		Y1 = IfThenElse(Y1 >= sHeight, Y0, Y1);

		if(state.filter3D)
		{
			Int Z1 = Z0 + 1;
			Z1 = IfThenElse(Z1 >= sHeight, Z0, Z1);

			Pointer<Byte> s000 = source + ComputeOffset(X0, Y0, Z0, sSliceB, sPitchB, srcBytes);
			Pointer<Byte> s010 = source + ComputeOffset(X1, Y0, Z0, sSliceB, sPitchB, srcBytes);
			Pointer<Byte> s100 = source + ComputeOffset(X0, Y1, Z0, sSliceB, sPitchB, srcBytes);
			Pointer<Byte> s110 = source + ComputeOffset(X1, Y1, Z0, sSliceB, sPitchB, srcBytes);
			Pointer<Byte> s001 = source + ComputeOffset(X0, Y0, Z1, sSliceB, sPitchB, srcBytes);
			Pointer<Byte> s011 = source + ComputeOffset(X1, Y0, Z1, sSliceB, sPitchB, srcBytes);
			Pointer<Byte> s101 = source + ComputeOffset(X0, Y1, Z1, sSliceB, sPitchB, srcBytes);
			Pointer<Byte> s111 = source + ComputeOffset(X1, Y1, Z1, sSliceB, sPitchB, srcBytes);

			Float4 c000 = readFloat4(s000, state);
			Float4 c010 = readFloat4(s010, state);
			Float4 c100 = readFloat4(s100, state);
			Float4 c110 = readFloat4(s110, state);
			Float4 c001 = readFloat4(s001, state);
			Float4 c011 = readFloat4(s011, state);
			Float4 c101 = readFloat4(s101, state);
			Float4 c111 = readFloat4(s111, state);

			if(state.allowSRGBConversion && state.sourceFormat.isSRGBformat())  // sRGB -> RGB
			{
				ApplyScaleAndClamp(c000, state);
				ApplyScaleAndClamp(c010, state);
				ApplyScaleAndClamp(c100, state);
				ApplyScaleAndClamp(c110, state);
				ApplyScaleAndClamp(c001, state);
				ApplyScaleAndClamp(c011, state);
				ApplyScaleAndClamp(c101, state);
				ApplyScaleAndClamp(c111, state);
				preScaled = true;
			}

			Float4 fx = Float4(x0 - Float(X0));
			Float4 fy = Float4(y0 - Float(Y0));
			Float4 fz = Float4(z0 - Float(Z0));
			Float4 ix = Float4(1.0f) - fx;
			Float4 iy = Float4(1.0f) - fy;
			Float4 iz = Float4(1.0f) - fz;

			color = ((c000 * ix + c010 * fx) * iy +
			         (c100 * ix + c110 * fx) * fy) *
			            iz +
			        ((c001 * ix + c011 * fx) * iy +
			         (c101 * ix + c111 * fx) * fy) *
			            fz;
		}
		else
		{
			Pointer<Byte> s00 = source + ComputeOffset(X0, Y0, Z0, sSliceB, sPitchB, srcBytes);
			Pointer<Byte> s01 = source + ComputeOffset(X1, Y0, Z0, sSliceB, sPitchB, srcBytes);
			Pointer<Byte> s10 = source + ComputeOffset(X0, Y1, Z0, sSliceB, sPitchB, srcBytes);
			Pointer<Byte> s11 = source + ComputeOffset(X1, Y1, Z0, sSliceB, sPitchB, srcBytes);

			Float4 c00 = readFloat4(s00, state);
			Float4 c01 = readFloat4(s01, state);
			Float4 c10 = readFloat4(s10, state);
			Float4 c11 = readFloat4(s11, state);

			if(state.allowSRGBConversion && state.sourceFormat.isSRGBformat())  // sRGB -> RGB
			{
				ApplyScaleAndClamp(c00, state);
				ApplyScaleAndClamp(c01, state);
				ApplyScaleAndClamp(c10, state);
				ApplyScaleAndClamp(c11, state);
				preScaled = true;
			}

			Float4 fx = Float4(x0 - Float(X0));
			Float4 fy = Float4(y0 - Float(Y0));
			Float4 ix = Float4(1.0f) - fx;
			Float4 iy = Float4(1.0f) - fy;

			color = (c00 * ix + c01 * fx) * iy +
			        (c10 * ix + c11 * fx) * fy;
		}
	}

	ApplyScaleAndClamp(color, state, preScaled);

	return color;
}

Blitter::BlitRoutineType Blitter::generate(const State &state)
{
	BlitFunction function;
	{
		Pointer<Byte> blit(function.Arg<0>());

		Pointer<Byte> source = *Pointer<Pointer<Byte>>(blit + OFFSET(BlitData, source));
		Pointer<Byte> dest = *Pointer<Pointer<Byte>>(blit + OFFSET(BlitData, dest));
		Int sPitchB = *Pointer<Int>(blit + OFFSET(BlitData, sPitchB));
		Int dPitchB = *Pointer<Int>(blit + OFFSET(BlitData, dPitchB));
		Int sSliceB = *Pointer<Int>(blit + OFFSET(BlitData, sSliceB));
		Int dSliceB = *Pointer<Int>(blit + OFFSET(BlitData, dSliceB));

		Float x0 = *Pointer<Float>(blit + OFFSET(BlitData, x0));
		Float y0 = *Pointer<Float>(blit + OFFSET(BlitData, y0));
		Float z0 = *Pointer<Float>(blit + OFFSET(BlitData, z0));
		Float w = *Pointer<Float>(blit + OFFSET(BlitData, w));
		Float h = *Pointer<Float>(blit + OFFSET(BlitData, h));
		Float d = *Pointer<Float>(blit + OFFSET(BlitData, d));

		Int x0d = *Pointer<Int>(blit + OFFSET(BlitData, x0d));
		Int x1d = *Pointer<Int>(blit + OFFSET(BlitData, x1d));
		Int y0d = *Pointer<Int>(blit + OFFSET(BlitData, y0d));
		Int y1d = *Pointer<Int>(blit + OFFSET(BlitData, y1d));
		Int z0d = *Pointer<Int>(blit + OFFSET(BlitData, z0d));
		Int z1d = *Pointer<Int>(blit + OFFSET(BlitData, z1d));

		Int sWidth = *Pointer<Int>(blit + OFFSET(BlitData, sWidth));
		Int sHeight = *Pointer<Int>(blit + OFFSET(BlitData, sHeight));
		Int sDepth = *Pointer<Int>(blit + OFFSET(BlitData, sDepth));

		bool intSrc = state.sourceFormat.isUnnormalizedInteger();
		bool intDst = state.destFormat.isUnnormalizedInteger();
		bool intBoth = intSrc && intDst;
		int srcBytes = state.sourceFormat.bytes();
		int dstBytes = state.destFormat.bytes();

		bool hasConstantColorI = false;
		Int4 constantColorI;
		bool hasConstantColorF = false;
		Float4 constantColorF;
		if(state.clearOperation)
		{
			if(intBoth)  // Integer types
			{
				constantColorI = readInt4(source, state);
				hasConstantColorI = true;
			}
			else
			{
				constantColorF = readFloat4(source, state);
				hasConstantColorF = true;

				ApplyScaleAndClamp(constantColorF, state);
			}
		}

		For(Int k = z0d, k < z1d, k++)
		{
			Float z = state.clearOperation ? RValue<Float>(z0) : z0 + Float(k) * d;
			Pointer<Byte> destSlice = dest + k * dSliceB;

			For(Int j = y0d, j < y1d, j++)
			{
				Float y = state.clearOperation ? RValue<Float>(y0) : y0 + Float(j) * h;
				Pointer<Byte> destLine = destSlice + j * dPitchB;

				For(Int i = x0d, i < x1d, i++)
				{
					Float x = state.clearOperation ? RValue<Float>(x0) : x0 + Float(i) * w;
					Pointer<Byte> d = destLine + i * dstBytes;

					if(hasConstantColorI)
					{
						for(int s = 0; s < state.destSamples; s++)
						{
							write(constantColorI, d, state);

							d += dSliceB;
						}
					}
					else if(hasConstantColorF)
					{
						for(int s = 0; s < state.destSamples; s++)
						{
							write(constantColorF, d, state);

							d += dSliceB;
						}
					}
					else if(intBoth)  // Integer types do not support filtering
					{
						Int X = Int(x);
						Int Y = Int(y);
						Int Z = Int(z);

						if(state.clampToEdge)
						{
							X = Clamp(X, 0, sWidth - 1);
							Y = Clamp(Y, 0, sHeight - 1);
							Z = Clamp(Z, 0, sDepth - 1);
						}

						Pointer<Byte> s = source + ComputeOffset(X, Y, Z, sSliceB, sPitchB, srcBytes);

						// When both formats are true integer types, we don't go to float to avoid losing precision
						Int4 color = readInt4(s, state);
						for(int s = 0; s < state.destSamples; s++)
						{
							write(color, d, state);

							d += dSliceB;
						}
					}
					else
					{
						Float4 color = sample(source, x, y, z, sWidth, sHeight, sDepth, sSliceB, sPitchB, state);

						for(int s = 0; s < state.destSamples; s++)
						{
							write(color, d, state);

							d += dSliceB;
						}
					}
				}
			}
		}
	}

	return function("BlitRoutine");
}

Blitter::BlitRoutineType Blitter::getBlitRoutine(const State &state)
{
	marl::lock lock(blitMutex);
	auto blitRoutine = blitCache.lookup(state);

	if(!blitRoutine)
	{
		blitRoutine = generate(state);
		blitCache.add(state, blitRoutine);
	}

	return blitRoutine;
}

Blitter::CornerUpdateRoutineType Blitter::getCornerUpdateRoutine(const State &state)
{
	marl::lock lock(cornerUpdateMutex);
	auto cornerUpdateRoutine = cornerUpdateCache.lookup(state);

	if(!cornerUpdateRoutine)
	{
		cornerUpdateRoutine = generateCornerUpdate(state);
		cornerUpdateCache.add(state, cornerUpdateRoutine);
	}

	return cornerUpdateRoutine;
}

void Blitter::blit(const vk::Image *src, vk::Image *dst, VkImageBlit2KHR region, VkFilter filter)
{
	ASSERT(src->getFormat() != VK_FORMAT_UNDEFINED);
	ASSERT(dst->getFormat() != VK_FORMAT_UNDEFINED);

	// Vulkan 1.2 section 18.5. Image Copies with Scaling:
	// "The layerCount member of srcSubresource and dstSubresource must match"
	// "The aspectMask member of srcSubresource and dstSubresource must match"
	ASSERT(region.srcSubresource.layerCount == region.dstSubresource.layerCount);
	ASSERT(region.srcSubresource.aspectMask == region.dstSubresource.aspectMask);

	if(region.dstOffsets[0].x > region.dstOffsets[1].x)
	{
		std::swap(region.srcOffsets[0].x, region.srcOffsets[1].x);
		std::swap(region.dstOffsets[0].x, region.dstOffsets[1].x);
	}

	if(region.dstOffsets[0].y > region.dstOffsets[1].y)
	{
		std::swap(region.srcOffsets[0].y, region.srcOffsets[1].y);
		std::swap(region.dstOffsets[0].y, region.dstOffsets[1].y);
	}

	if(region.dstOffsets[0].z > region.dstOffsets[1].z)
	{
		std::swap(region.srcOffsets[0].z, region.srcOffsets[1].z);
		std::swap(region.dstOffsets[0].z, region.dstOffsets[1].z);
	}

	VkImageAspectFlagBits srcAspect = static_cast<VkImageAspectFlagBits>(region.srcSubresource.aspectMask);
	VkImageAspectFlagBits dstAspect = static_cast<VkImageAspectFlagBits>(region.dstSubresource.aspectMask);
	VkExtent3D srcExtent = src->getMipLevelExtent(srcAspect, region.srcSubresource.mipLevel);

	float widthRatio = static_cast<float>(region.srcOffsets[1].x - region.srcOffsets[0].x) /
	                   static_cast<float>(region.dstOffsets[1].x - region.dstOffsets[0].x);
	float heightRatio = static_cast<float>(region.srcOffsets[1].y - region.srcOffsets[0].y) /
	                    static_cast<float>(region.dstOffsets[1].y - region.dstOffsets[0].y);
	float depthRatio = static_cast<float>(region.srcOffsets[1].z - region.srcOffsets[0].z) /
	                   static_cast<float>(region.dstOffsets[1].z - region.dstOffsets[0].z);
	float x0 = region.srcOffsets[0].x + (0.5f - region.dstOffsets[0].x) * widthRatio;
	float y0 = region.srcOffsets[0].y + (0.5f - region.dstOffsets[0].y) * heightRatio;
	float z0 = region.srcOffsets[0].z + (0.5f - region.dstOffsets[0].z) * depthRatio;

	auto srcFormat = src->getFormat(srcAspect);
	auto dstFormat = dst->getFormat(dstAspect);

	bool doFilter = (filter != VK_FILTER_NEAREST);
	bool allowSRGBConversion =
	    doFilter ||
	    (src->getSampleCount() > 1) ||
	    (srcFormat.isSRGBformat() != dstFormat.isSRGBformat());

	State state(srcFormat, dstFormat, src->getSampleCount(), dst->getSampleCount(),
	            Options{ doFilter, allowSRGBConversion });
	state.clampToEdge = (region.srcOffsets[0].x < 0) ||
	                    (region.srcOffsets[0].y < 0) ||
	                    (static_cast<uint32_t>(region.srcOffsets[1].x) > srcExtent.width) ||
	                    (static_cast<uint32_t>(region.srcOffsets[1].y) > srcExtent.height) ||
	                    (doFilter && ((x0 < 0.5f) || (y0 < 0.5f)));
	state.filter3D = (region.srcOffsets[1].z - region.srcOffsets[0].z) !=
	                 (region.dstOffsets[1].z - region.dstOffsets[0].z);

	auto blitRoutine = getBlitRoutine(state);
	if(!blitRoutine)
	{
		return;
	}

	BlitData data = {
		nullptr,                                                                                 // source
		nullptr,                                                                                 // dest
		assert_cast<uint32_t>(src->rowPitchBytes(srcAspect, region.srcSubresource.mipLevel)),    // sPitchB
		assert_cast<uint32_t>(dst->rowPitchBytes(dstAspect, region.dstSubresource.mipLevel)),    // dPitchB
		assert_cast<uint32_t>(src->slicePitchBytes(srcAspect, region.srcSubresource.mipLevel)),  // sSliceB
		assert_cast<uint32_t>(dst->slicePitchBytes(dstAspect, region.dstSubresource.mipLevel)),  // dSliceB

		x0,
		y0,
		z0,
		widthRatio,
		heightRatio,
		depthRatio,

		region.dstOffsets[0].x,  // x0d
		region.dstOffsets[1].x,  // x1d
		region.dstOffsets[0].y,  // y0d
		region.dstOffsets[1].y,  // y1d
		region.dstOffsets[0].z,  // z0d
		region.dstOffsets[1].z,  // z1d

		static_cast<int>(srcExtent.width),   // sWidth
		static_cast<int>(srcExtent.height),  // sHeight
		static_cast<int>(srcExtent.depth),   // sDepth

		false,  // filter3D
	};

	VkImageSubresource srcSubres = {
		region.srcSubresource.aspectMask,
		region.srcSubresource.mipLevel,
		region.srcSubresource.baseArrayLayer
	};

	VkImageSubresource dstSubres = {
		region.dstSubresource.aspectMask,
		region.dstSubresource.mipLevel,
		region.dstSubresource.baseArrayLayer
	};

	VkImageSubresourceRange dstSubresRange = {
		region.dstSubresource.aspectMask,
		region.dstSubresource.mipLevel,
		1,  // levelCount
		region.dstSubresource.baseArrayLayer,
		region.dstSubresource.layerCount
	};

	uint32_t lastLayer = src->getLastLayerIndex(dstSubresRange);

	for(; dstSubres.arrayLayer <= lastLayer; srcSubres.arrayLayer++, dstSubres.arrayLayer++)
	{
		data.source = src->getTexelPointer({ 0, 0, 0 }, srcSubres);
		data.dest = dst->getTexelPointer({ 0, 0, 0 }, dstSubres);

		ASSERT(data.source < src->end());
		ASSERT(data.dest < dst->end());

		blitRoutine(&data);
	}

	dst->contentsChanged(dstSubresRange);
}

static void resolveDepth(const vk::ImageView *src, vk::ImageView *dst, const VkResolveModeFlagBits depthResolveMode)
{
	if(depthResolveMode == VK_RESOLVE_MODE_NONE)
	{
		return;
	}

	vk::Format format = src->getFormat(VK_IMAGE_ASPECT_DEPTH_BIT);
	VkExtent2D extent = src->getMipLevelExtent(0, VK_IMAGE_ASPECT_DEPTH_BIT);
	int width = extent.width;
	int height = extent.height;
	int pitch = src->rowPitchBytes(VK_IMAGE_ASPECT_DEPTH_BIT, 0);

	// To support other resolve modes, get the slice bytes and get a pointer to each sample plane.
	// Then modify the loop below to include logic for handling each new mode.
	uint8_t *source = (uint8_t *)src->getOffsetPointer({ 0, 0, 0 }, VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0);
	uint8_t *dest = (uint8_t *)dst->getOffsetPointer({ 0, 0, 0 }, VK_IMAGE_ASPECT_DEPTH_BIT, 0, 0);

	size_t formatSize = format.bytes();
	// TODO(b/167558951) support other resolve modes.
	ASSERT(depthResolveMode == VK_RESOLVE_MODE_SAMPLE_ZERO_BIT);
	for(int y = 0; y < height; y++)
	{
		memcpy(dest, source, formatSize * width);

		source += pitch;
		dest += pitch;
	}

	dst->contentsChanged(vk::Image::DIRECT_MEMORY_ACCESS);
}

static void resolveStencil(const vk::ImageView *src, vk::ImageView *dst, const VkResolveModeFlagBits stencilResolveMode)
{
	if(stencilResolveMode == VK_RESOLVE_MODE_NONE)
	{
		return;
	}

	VkExtent2D extent = src->getMipLevelExtent(0, VK_IMAGE_ASPECT_STENCIL_BIT);
	int width = extent.width;
	int height = extent.height;
	int pitch = src->rowPitchBytes(VK_IMAGE_ASPECT_STENCIL_BIT, 0);

	// To support other resolve modes, use src->slicePitchBytes() and get a pointer to each sample's slice.
	// Then modify the loop below to include logic for handling each new mode.
	uint8_t *source = reinterpret_cast<uint8_t *>(src->getOffsetPointer({ 0, 0, 0 }, VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0));
	uint8_t *dest = reinterpret_cast<uint8_t *>(dst->getOffsetPointer({ 0, 0, 0 }, VK_IMAGE_ASPECT_STENCIL_BIT, 0, 0));

	// TODO(b/167558951) support other resolve modes.
	ASSERT(stencilResolveMode == VK_RESOLVE_MODE_SAMPLE_ZERO_BIT);
	for(int y = 0; y < height; y++)
	{
		// Stencil is always 8 bits, so the width of the resource we're resolving is
		// the number of bytes in each row we need to copy during for SAMPLE_ZERO
		memcpy(dest, source, width);

		source += pitch;
		dest += pitch;
	}

	dst->contentsChanged(vk::Image::DIRECT_MEMORY_ACCESS);
}

void Blitter::resolveDepthStencil(const vk::ImageView *src, vk::ImageView *dst, VkResolveModeFlagBits depthResolveMode, VkResolveModeFlagBits stencilResolveMode)
{
	VkImageSubresourceRange srcRange = src->getSubresourceRange();
	VkImageSubresourceRange dstRange = src->getSubresourceRange();
	ASSERT(src->getFormat() == dst->getFormat());
	ASSERT(srcRange.layerCount == 1 && dstRange.layerCount == 1);
	ASSERT(srcRange.aspectMask == dstRange.aspectMask);

	if(srcRange.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)
	{
		resolveDepth(src, dst, depthResolveMode);
	}
	if(srcRange.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)
	{
		resolveStencil(src, dst, stencilResolveMode);
	}
}

void Blitter::resolve(const vk::Image *src, vk::Image *dst, VkImageResolve2KHR region)
{
	// "The aspectMask member of srcSubresource and dstSubresource must only contain VK_IMAGE_ASPECT_COLOR_BIT"
	ASSERT(region.srcSubresource.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT);
	ASSERT(region.dstSubresource.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT);
	// "The layerCount member of srcSubresource and dstSubresource must match"
	ASSERT(region.srcSubresource.layerCount == region.dstSubresource.layerCount);

	// We use this method both for explicit resolves from vkCmdResolveImage, and implicit ones for resolve attachments.
	// - vkCmdResolveImage: "srcImage and dstImage must have been created with the same image format."
	// - VkSubpassDescription: "each resolve attachment that is not VK_ATTACHMENT_UNUSED must have the same VkFormat as its corresponding color attachment."
	ASSERT(src->getFormat() == dst->getFormat());

	if(fastResolve(src, dst, region))
	{
		return;
	}

	// Fall back to a generic blit which performs the resolve.
	VkImageBlit2KHR blitRegion;
	blitRegion.sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2_KHR;
	blitRegion.pNext = nullptr;

	blitRegion.srcOffsets[0] = blitRegion.srcOffsets[1] = region.srcOffset;
	blitRegion.srcOffsets[1].x += region.extent.width;
	blitRegion.srcOffsets[1].y += region.extent.height;
	blitRegion.srcOffsets[1].z += region.extent.depth;

	blitRegion.dstOffsets[0] = blitRegion.dstOffsets[1] = region.dstOffset;
	blitRegion.dstOffsets[1].x += region.extent.width;
	blitRegion.dstOffsets[1].y += region.extent.height;
	blitRegion.dstOffsets[1].z += region.extent.depth;

	blitRegion.srcSubresource = region.srcSubresource;
	blitRegion.dstSubresource = region.dstSubresource;

	blit(src, dst, blitRegion, VK_FILTER_NEAREST);
}

static inline uint32_t averageByte4(uint32_t x, uint32_t y)
{
	return (x & y) + (((x ^ y) >> 1) & 0x7F7F7F7F) + ((x ^ y) & 0x01010101);
}

bool Blitter::fastResolve(const vk::Image *src, vk::Image *dst, VkImageResolve2KHR region)
{
	if(region.dstOffset != VkOffset3D{ 0, 0, 0 })
	{
		return false;
	}

	if(region.srcOffset != VkOffset3D{ 0, 0, 0 })
	{
		return false;
	}

	if(region.srcSubresource.layerCount != 1)
	{
		return false;
	}

	if(region.extent != src->getExtent() ||
	   region.extent != dst->getExtent() ||
	   region.extent.depth != 1)
	{
		return false;
	}

	VkImageSubresource srcSubresource = {
		region.srcSubresource.aspectMask,
		region.srcSubresource.mipLevel,
		region.srcSubresource.baseArrayLayer
	};

	VkImageSubresource dstSubresource = {
		region.dstSubresource.aspectMask,
		region.dstSubresource.mipLevel,
		region.dstSubresource.baseArrayLayer
	};

	VkImageSubresourceRange dstSubresourceRange = {
		region.dstSubresource.aspectMask,
		region.dstSubresource.mipLevel,
		1,  // levelCount
		region.dstSubresource.baseArrayLayer,
		region.dstSubresource.layerCount
	};

	void *source = src->getTexelPointer({ 0, 0, 0 }, srcSubresource);
	uint8_t *dest = reinterpret_cast<uint8_t *>(dst->getTexelPointer({ 0, 0, 0 }, dstSubresource));

	auto format = src->getFormat();
	auto samples = src->getSampleCount();
	auto extent = src->getExtent();

	int width = extent.width;
	int height = extent.height;
	int pitch = src->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, region.srcSubresource.mipLevel);
	int slice = src->slicePitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, region.srcSubresource.mipLevel);

	uint8_t *source0 = (uint8_t *)source;
	uint8_t *source1 = source0 + slice;
	uint8_t *source2 = source1 + slice;
	uint8_t *source3 = source2 + slice;

	[[maybe_unused]] const bool SSE2 = CPUID::supportsSSE2();

	if(format == VK_FORMAT_R8G8B8A8_UNORM || format == VK_FORMAT_B8G8R8A8_UNORM || format == VK_FORMAT_A8B8G8R8_UNORM_PACK32)
	{
		if(samples == 4)
		{
			for(int y = 0; y < height; y++)
			{
				int x = 0;

#if defined(__i386__) || defined(__x86_64__)
				if(SSE2)
				{
					for(; (x + 3) < width; x += 4)
					{
						__m128i c0 = _mm_loadu_si128((__m128i *)(source0 + 4 * x));
						__m128i c1 = _mm_loadu_si128((__m128i *)(source1 + 4 * x));
						__m128i c2 = _mm_loadu_si128((__m128i *)(source2 + 4 * x));
						__m128i c3 = _mm_loadu_si128((__m128i *)(source3 + 4 * x));

						c0 = _mm_avg_epu8(c0, c1);
						c2 = _mm_avg_epu8(c2, c3);
						c0 = _mm_avg_epu8(c0, c2);

						_mm_storeu_si128((__m128i *)(dest + 4 * x), c0);
					}
				}
#endif

				for(; x < width; x++)
				{
					uint32_t c0 = *(uint32_t *)(source0 + 4 * x);
					uint32_t c1 = *(uint32_t *)(source1 + 4 * x);
					uint32_t c2 = *(uint32_t *)(source2 + 4 * x);
					uint32_t c3 = *(uint32_t *)(source3 + 4 * x);

					uint32_t c01 = averageByte4(c0, c1);
					uint32_t c23 = averageByte4(c2, c3);
					uint32_t c03 = averageByte4(c01, c23);

					*(uint32_t *)(dest + 4 * x) = c03;
				}

				source0 += pitch;
				source1 += pitch;
				source2 += pitch;
				source3 += pitch;
				dest += pitch;

				ASSERT(source0 < src->end());
				ASSERT(source3 < src->end());
				ASSERT(dest < dst->end());
			}
		}
		else
			UNSUPPORTED("Samples: %d", samples);
	}
	else
	{
		return false;
	}

	dst->contentsChanged(dstSubresourceRange);

	return true;
}

void Blitter::copy(const vk::Image *src, uint8_t *dst, unsigned int dstPitch)
{
	VkExtent3D extent = src->getExtent();
	size_t rowBytes = src->getFormat(VK_IMAGE_ASPECT_COLOR_BIT).bytes() * extent.width;
	unsigned int srcPitch = src->rowPitchBytes(VK_IMAGE_ASPECT_COLOR_BIT, 0);
	ASSERT(dstPitch >= rowBytes && srcPitch >= rowBytes && src->getMipLevelExtent(VK_IMAGE_ASPECT_COLOR_BIT, 0).height >= extent.height);

	const uint8_t *s = (uint8_t *)src->getTexelPointer({ 0, 0, 0 }, { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 });
	uint8_t *d = dst;

	for(uint32_t y = 0; y < extent.height; y++)
	{
		memcpy(d, s, rowBytes);

		s += srcPitch;
		d += dstPitch;
	}
}

void Blitter::computeCubeCorner(Pointer<Byte> &layer, Int &x0, Int &x1, Int &y0, Int &y1, Int &pitchB, const State &state)
{
	int bytes = state.sourceFormat.bytes();

	Float4 c = readFloat4(layer + ComputeOffset(x0, y1, pitchB, bytes), state) +
	           readFloat4(layer + ComputeOffset(x1, y0, pitchB, bytes), state) +
	           readFloat4(layer + ComputeOffset(x1, y1, pitchB, bytes), state);

	c *= Float4(1.0f / 3.0f);

	write(c, layer + ComputeOffset(x0, y0, pitchB, bytes), state);
}

Blitter::CornerUpdateRoutineType Blitter::generateCornerUpdate(const State &state)
{
	// Reading and writing from/to the same image
	ASSERT(state.sourceFormat == state.destFormat);
	ASSERT(state.srcSamples == state.destSamples);

	// Vulkan 1.2: "If samples is not VK_SAMPLE_COUNT_1_BIT, then imageType must be
	// VK_IMAGE_TYPE_2D, flags must not contain VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT"
	ASSERT(state.srcSamples == 1);

	CornerUpdateFunction function;
	{
		Pointer<Byte> blit(function.Arg<0>());

		Pointer<Byte> layers = *Pointer<Pointer<Byte>>(blit + OFFSET(CubeBorderData, layers));
		Int pitchB = *Pointer<Int>(blit + OFFSET(CubeBorderData, pitchB));
		UInt layerSize = *Pointer<Int>(blit + OFFSET(CubeBorderData, layerSize));
		UInt dim = *Pointer<Int>(blit + OFFSET(CubeBorderData, dim));

		// Low Border, Low Pixel, High Border, High Pixel
		Int LB(-1), LP(0), HB(dim), HP(dim - 1);

		for(int face = 0; face < 6; face++)
		{
			computeCubeCorner(layers, LB, LP, LB, LP, pitchB, state);
			computeCubeCorner(layers, LB, LP, HB, HP, pitchB, state);
			computeCubeCorner(layers, HB, HP, LB, LP, pitchB, state);
			computeCubeCorner(layers, HB, HP, HB, HP, pitchB, state);
			layers = layers + layerSize;
		}
	}

	return function("BlitRoutine");
}

void Blitter::updateBorders(const vk::Image *image, const VkImageSubresource &subresource)
{
	ASSERT(image->getArrayLayers() >= (subresource.arrayLayer + 6));

	// From Vulkan 1.1 spec, section 11.5. Image Views:
	// "For cube and cube array image views, the layers of the image view starting
	//  at baseArrayLayer correspond to faces in the order +X, -X, +Y, -Y, +Z, -Z."
	VkImageSubresource posX = subresource;
	VkImageSubresource negX = posX;
	negX.arrayLayer++;
	VkImageSubresource posY = negX;
	posY.arrayLayer++;
	VkImageSubresource negY = posY;
	negY.arrayLayer++;
	VkImageSubresource posZ = negY;
	posZ.arrayLayer++;
	VkImageSubresource negZ = posZ;
	negZ.arrayLayer++;

	// Copy top / bottom
	copyCubeEdge(image, posX, BOTTOM, negY, RIGHT);
	copyCubeEdge(image, posY, BOTTOM, posZ, TOP);
	copyCubeEdge(image, posZ, BOTTOM, negY, TOP);
	copyCubeEdge(image, negX, BOTTOM, negY, LEFT);
	copyCubeEdge(image, negY, BOTTOM, negZ, BOTTOM);
	copyCubeEdge(image, negZ, BOTTOM, negY, BOTTOM);

	copyCubeEdge(image, posX, TOP, posY, RIGHT);
	copyCubeEdge(image, posY, TOP, negZ, TOP);
	copyCubeEdge(image, posZ, TOP, posY, BOTTOM);
	copyCubeEdge(image, negX, TOP, posY, LEFT);
	copyCubeEdge(image, negY, TOP, posZ, BOTTOM);
	copyCubeEdge(image, negZ, TOP, posY, TOP);

	// Copy left / right
	copyCubeEdge(image, posX, RIGHT, negZ, LEFT);
	copyCubeEdge(image, posY, RIGHT, posX, TOP);
	copyCubeEdge(image, posZ, RIGHT, posX, LEFT);
	copyCubeEdge(image, negX, RIGHT, posZ, LEFT);
	copyCubeEdge(image, negY, RIGHT, posX, BOTTOM);
	copyCubeEdge(image, negZ, RIGHT, negX, LEFT);

	copyCubeEdge(image, posX, LEFT, posZ, RIGHT);
	copyCubeEdge(image, posY, LEFT, negX, TOP);
	copyCubeEdge(image, posZ, LEFT, negX, RIGHT);
	copyCubeEdge(image, negX, LEFT, negZ, RIGHT);
	copyCubeEdge(image, negY, LEFT, negX, BOTTOM);
	copyCubeEdge(image, negZ, LEFT, posX, RIGHT);

	// Compute corner colors
	VkImageAspectFlagBits aspect = static_cast<VkImageAspectFlagBits>(subresource.aspectMask);
	vk::Format format = image->getFormat(aspect);
	VkSampleCountFlagBits samples = image->getSampleCount();
	State state(format, format, samples, samples, Options{ 0xF });

	// Vulkan 1.2: "If samples is not VK_SAMPLE_COUNT_1_BIT, then imageType must be
	// VK_IMAGE_TYPE_2D, flags must not contain VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT"
	ASSERT(samples == VK_SAMPLE_COUNT_1_BIT);

	auto cornerUpdateRoutine = getCornerUpdateRoutine(state);
	if(!cornerUpdateRoutine)
	{
		return;
	}

	VkExtent3D extent = image->getMipLevelExtent(aspect, subresource.mipLevel);
	CubeBorderData data = {
		image->getTexelPointer({ 0, 0, 0 }, posX),
		assert_cast<uint32_t>(image->rowPitchBytes(aspect, subresource.mipLevel)),
		assert_cast<uint32_t>(image->getLayerSize(aspect)),
		extent.width
	};
	cornerUpdateRoutine(&data);
}

void Blitter::copyCubeEdge(const vk::Image *image,
                           const VkImageSubresource &dstSubresource, Edge dstEdge,
                           const VkImageSubresource &srcSubresource, Edge srcEdge)
{
	ASSERT(srcSubresource.aspectMask == dstSubresource.aspectMask);
	ASSERT(srcSubresource.mipLevel == dstSubresource.mipLevel);
	ASSERT(srcSubresource.arrayLayer != dstSubresource.arrayLayer);

	// Figure out if the edges to be copied in reverse order respectively from one another
	// The copy should be reversed whenever the same edges are contiguous or if we're
	// copying top <-> right or bottom <-> left. This is explained by the layout, which is:
	//
	//      | +y |
	// | -x | +z | +x | -z |
	//      | -y |

	bool reverse = (srcEdge == dstEdge) ||
	               ((srcEdge == TOP) && (dstEdge == RIGHT)) ||
	               ((srcEdge == RIGHT) && (dstEdge == TOP)) ||
	               ((srcEdge == BOTTOM) && (dstEdge == LEFT)) ||
	               ((srcEdge == LEFT) && (dstEdge == BOTTOM));

	VkImageAspectFlagBits aspect = static_cast<VkImageAspectFlagBits>(srcSubresource.aspectMask);
	int bytes = image->getFormat(aspect).bytes();
	int pitchB = image->rowPitchBytes(aspect, srcSubresource.mipLevel);

	VkExtent3D extent = image->getMipLevelExtent(aspect, srcSubresource.mipLevel);
	int w = extent.width;
	int h = extent.height;
	if(w != h)
	{
		UNSUPPORTED("Cube doesn't have square faces : (%d, %d)", w, h);
	}

	// Src is expressed in the regular [0, width-1], [0, height-1] space
	bool srcHorizontal = ((srcEdge == TOP) || (srcEdge == BOTTOM));
	int srcDelta = srcHorizontal ? bytes : pitchB;
	VkOffset3D srcOffset = { (srcEdge == RIGHT) ? (w - 1) : 0, (srcEdge == BOTTOM) ? (h - 1) : 0, 0 };

	// Dst contains borders, so it is expressed in the [-1, width], [-1, height] space
	bool dstHorizontal = ((dstEdge == TOP) || (dstEdge == BOTTOM));
	int dstDelta = (dstHorizontal ? bytes : pitchB) * (reverse ? -1 : 1);
	VkOffset3D dstOffset = { (dstEdge == RIGHT) ? w : -1, (dstEdge == BOTTOM) ? h : -1, 0 };

	// Don't write in the corners
	if(dstHorizontal)
	{
		dstOffset.x += reverse ? w : 1;
	}
	else
	{
		dstOffset.y += reverse ? h : 1;
	}

	const uint8_t *src = static_cast<const uint8_t *>(image->getTexelPointer(srcOffset, srcSubresource));
	uint8_t *dst = static_cast<uint8_t *>(image->getTexelPointer(dstOffset, dstSubresource));
	ASSERT((src < image->end()) && ((src + (w * srcDelta)) < image->end()));
	ASSERT((dst < image->end()) && ((dst + (w * dstDelta)) < image->end()));

	for(int i = 0; i < w; ++i, dst += dstDelta, src += srcDelta)
	{
		memcpy(dst, src, bytes);
	}
}

}  // namespace sw
