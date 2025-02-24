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

#include "SamplerCore.hpp"

#include "Constants.hpp"
#include "PixelRoutine.hpp"
#include "System/Debug.hpp"
#include "Vulkan/VkSampler.hpp"

namespace sw {

SamplerCore::SamplerCore(Pointer<Byte> &constants, const Sampler &state, SamplerFunction function)
    : constants(constants)
    , state(state)
    , function(function)
{
}

SIMD::Float4 SamplerCore::sampleTexture(Pointer<Byte> &texture, SIMD::Float uvwa[4], const SIMD::Float &dRef, const Float &lodOrBias, const SIMD::Float &dsx, const SIMD::Float &dsy, SIMD::Int offset[4], const SIMD::Int &sample)
{
	SIMD::Float4 c;

	for(int i = 0; i < SIMD::Width / 4; i++)
	{
		Float4 uvwa128[4];
		uvwa128[0] = Extract128(uvwa[0], i);
		uvwa128[1] = Extract128(uvwa[1], i);
		uvwa128[2] = Extract128(uvwa[2], i);
		uvwa128[3] = Extract128(uvwa[3], i);

		Vector4i offset128;
		offset128[0] = Extract128(offset[0], i);
		offset128[1] = Extract128(offset[1], i);
		offset128[2] = Extract128(offset[2], i);
		offset128[3] = Extract128(offset[3], i);

		Vector4f c128 = sampleTexture128(texture, uvwa128, Extract128(dRef, i), lodOrBias, Extract128(dsx, i), Extract128(dsy, i), offset128, Extract128(sample, i));
		c.x = Insert128(c.x, c128.x, i);
		c.y = Insert128(c.y, c128.y, i);
		c.z = Insert128(c.z, c128.z, i);
		c.w = Insert128(c.w, c128.w, i);
	}

	return c;
}

Vector4f SamplerCore::sampleTexture128(Pointer<Byte> &texture, Float4 uvwa[4], const Float4 &dRef, const Float &lodOrBias, const Float4 &dsx, const Float4 &dsy, Vector4i &offset, const Int4 &sample)
{
	Vector4f c;

	Float4 u = uvwa[0];
	Float4 v = uvwa[1];
	Float4 w = uvwa[2];
	Float4 a;  // Array layer coordinate
	switch(state.textureType)
	{
	case VK_IMAGE_VIEW_TYPE_1D_ARRAY: a = uvwa[1]; break;
	case VK_IMAGE_VIEW_TYPE_2D_ARRAY: a = uvwa[2]; break;
	case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY: a = uvwa[3]; break;
	default: break;
	}

	Float lod;
	Float anisotropy;
	Float4 uDelta;
	Float4 vDelta;
	Float4 M;  // Major axis

	if(state.isCube())
	{
		Int4 face = cubeFace(u, v, uvwa[0], uvwa[1], uvwa[2], M);
		w = As<Float4>(face);
	}

	// Determine if we can skip the LOD computation. This is the case when the mipmap has only one level, except for LOD query,
	// where we have to return the computed value. Anisotropic filtering requires computing the anisotropy factor even for a single mipmap level.
	bool singleMipLevel = (state.minLod == state.maxLod);
	bool requiresLodComputation = (function == Query) || (state.textureFilter == FILTER_ANISOTROPIC);
	bool skipLodComputation = singleMipLevel && !requiresLodComputation;

	if(skipLodComputation)
	{
		lod = state.minLod;
	}
	else if(function == Implicit || function == Bias || function == Grad || function == Query)
	{
		if(state.is1D())
		{
			computeLod1D(texture, lod, u, dsx, dsy);
		}
		else if(state.is2D())
		{
			computeLod2D(texture, lod, anisotropy, uDelta, vDelta, u, v, dsx, dsy);
		}
		else if(state.isCube())
		{
			computeLodCube(texture, lod, uvwa[0], uvwa[1], uvwa[2], dsx, dsy, M);
		}
		else
		{
			computeLod3D(texture, lod, u, v, w, dsx, dsy);
		}

		Float bias = state.mipLodBias;

		if(function == Bias)
		{
			// Add SPIR-V Bias operand to the sampler provided bias and clamp to maxSamplerLodBias limit.
			bias = Min(Max(bias + lodOrBias, -vk::MAX_SAMPLER_LOD_BIAS), vk::MAX_SAMPLER_LOD_BIAS);
		}

		lod += bias;
	}
	else if(function == Lod)
	{
		// Vulkan 1.1: "The absolute value of mipLodBias must be less than or equal to VkPhysicalDeviceLimits::maxSamplerLodBias"
		// Hence no explicit clamping to maxSamplerLodBias is required in this case.
		lod = lodOrBias + state.mipLodBias;
	}
	else if(function == Fetch)
	{
		// TODO: Eliminate int-float-int conversion.
		lod = Float(As<Int>(lodOrBias));
		lod = Max(lod, state.minLod);
		lod = Min(lod, state.maxLod);
	}
	else if(function == Base || function == Gather)
	{
		lod = Float(0);
	}
	else
		UNREACHABLE("Sampler function %d", int(function));

	if(function != Base && function != Fetch && function != Gather)
	{
		if(function == Query)
		{
			c.y = Float4(lod);  // Unclamped LOD.
		}

		if(!skipLodComputation)
		{
			lod = Max(lod, state.minLod);
			lod = Min(lod, state.maxLod);
		}

		if(function == Query)
		{
			if(state.mipmapFilter == MIPMAP_POINT)
			{
				lod = Round(lod);  // TODO: Preferred formula is ceil(lod + 0.5) - 1
			}

			c.x = lod;
			//	c.y contains unclamped LOD.

			return c;
		}
	}

	bool force32BitFiltering = state.highPrecisionFiltering && !isYcbcrFormat() && (state.textureFilter != FILTER_POINT);
	bool use32BitFiltering = hasFloatTexture() || hasUnnormalizedIntegerTexture() || force32BitFiltering ||
	                         state.isCube() || state.unnormalizedCoordinates || state.compareEnable ||
	                         borderModeActive() || (function == Gather) || (function == Fetch);
	int numComponents = (function == Gather) ? 4 : textureComponentCount();

	if(use32BitFiltering)
	{
		c = sampleFloatFilter(texture, u, v, w, a, dRef, offset, sample, lod, anisotropy, uDelta, vDelta);
	}
	else  // 16-bit filtering.
	{
		Vector4s cs = sampleFilter(texture, u, v, w, a, offset, sample, lod, anisotropy, uDelta, vDelta);

		for(int component = 0; component < numComponents; component++)
		{
			if(hasUnsignedTextureComponent(component))
			{
				c[component] = Float4(As<UShort4>(cs[component]));
			}
			else
			{
				c[component] = Float4(cs[component]);
			}
		}
	}

	if(hasNormalizedFormat() && !state.compareEnable)
	{
		sw::float4 scale = getComponentScale();

		for(int component = 0; component < numComponents; component++)
		{
			int texelComponent = (function == Gather) ? getGatherComponent() : component;
			c[component] *= Float4(1.0f / scale[texelComponent]);
		}
	}

	if(state.textureFormat.isSignedNormalized())
	{
		for(int component = 0; component < numComponents; component++)
		{
			c[component] = Max(c[component], Float4(-1.0f));
		}
	}

	if(state.textureFilter != FILTER_GATHER)
	{
		if((state.swizzle.r != VK_COMPONENT_SWIZZLE_R) ||
		   (state.swizzle.g != VK_COMPONENT_SWIZZLE_G) ||
		   (state.swizzle.b != VK_COMPONENT_SWIZZLE_B) ||
		   (state.swizzle.a != VK_COMPONENT_SWIZZLE_A))
		{
			const Vector4f col = c;
			bool integer = hasUnnormalizedIntegerTexture();
			c.x = applySwizzle(col, state.swizzle.r, integer);
			c.y = applySwizzle(col, state.swizzle.g, integer);
			c.z = applySwizzle(col, state.swizzle.b, integer);
			c.w = applySwizzle(col, state.swizzle.a, integer);
		}
	}
	else  // Gather
	{
		VkComponentSwizzle swizzle = gatherSwizzle();

		// R/G/B/A swizzles affect the component collected from each texel earlier.
		// Handle the ZERO and ONE cases here because we don't need to know the format.

		if(swizzle == VK_COMPONENT_SWIZZLE_ZERO)
		{
			c.x = c.y = c.z = c.w = Float4(0);
		}
		else if(swizzle == VK_COMPONENT_SWIZZLE_ONE)
		{
			bool integer = hasUnnormalizedIntegerTexture();
			c.x = c.y = c.z = c.w = integer ? As<Float4>(Int4(1)) : RValue<Float4>(Float4(1.0f));
		}
	}

	return c;
}

Float4 SamplerCore::applySwizzle(const Vector4f &c, VkComponentSwizzle swizzle, bool integer)
{
	switch(swizzle)
	{
	default: UNSUPPORTED("VkComponentSwizzle %d", (int)swizzle);
	case VK_COMPONENT_SWIZZLE_R: return c.x;
	case VK_COMPONENT_SWIZZLE_G: return c.y;
	case VK_COMPONENT_SWIZZLE_B: return c.z;
	case VK_COMPONENT_SWIZZLE_A: return c.w;
	case VK_COMPONENT_SWIZZLE_ZERO: return Float4(0.0f, 0.0f, 0.0f, 0.0f);
	case VK_COMPONENT_SWIZZLE_ONE:
		if(integer)
		{
			return Float4(As<Float4>(sw::Int4(1, 1, 1, 1)));
		}
		else
		{
			return Float4(1.0f, 1.0f, 1.0f, 1.0f);
		}
		break;
	}
};

Short4 SamplerCore::offsetSample(Short4 &uvw, Pointer<Byte> &mipmap, int halfOffset, bool wrap, int count, Float &lod)
{
	Short4 offset = *Pointer<UShort4>(mipmap + halfOffset);

	if(state.textureFilter == FILTER_MIN_LINEAR_MAG_POINT)
	{
		offset &= Short4(CmpNLE(Float4(lod), Float4(0.0f)));
	}
	else if(state.textureFilter == FILTER_MIN_POINT_MAG_LINEAR)
	{
		offset &= Short4(CmpLE(Float4(lod), Float4(0.0f)));
	}

	if(wrap)
	{
		switch(count)
		{
		case -1: return uvw - offset;
		case 0: return uvw;
		case +1: return uvw + offset;
		case 2: return uvw + offset + offset;
		}
	}
	else  // Clamp or mirror
	{
		switch(count)
		{
		case -1: return SubSat(As<UShort4>(uvw), As<UShort4>(offset));
		case 0: return uvw;
		case +1: return AddSat(As<UShort4>(uvw), As<UShort4>(offset));
		case 2: return AddSat(AddSat(As<UShort4>(uvw), As<UShort4>(offset)), As<UShort4>(offset));
		}
	}

	return uvw;
}

Vector4s SamplerCore::sampleFilter(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, const Float4 &a, Vector4i &offset, const Int4 &sample, Float &lod, Float &anisotropy, Float4 &uDelta, Float4 &vDelta)
{
	Vector4s c = sampleAniso(texture, u, v, w, a, offset, sample, lod, anisotropy, uDelta, vDelta, false);

	if(function == Fetch)
	{
		return c;
	}

	if(state.mipmapFilter == MIPMAP_LINEAR)
	{
		Vector4s cc = sampleAniso(texture, u, v, w, a, offset, sample, lod, anisotropy, uDelta, vDelta, true);

		lod *= Float(1 << 16);

		UShort4 utri = UShort4(Float4(lod));  // TODO: Optimize
		Short4 stri = utri >> 1;              // TODO: Optimize

		if(hasUnsignedTextureComponent(0))
			cc.x = MulHigh(As<UShort4>(cc.x), utri);
		else
			cc.x = MulHigh(cc.x, stri);
		if(hasUnsignedTextureComponent(1))
			cc.y = MulHigh(As<UShort4>(cc.y), utri);
		else
			cc.y = MulHigh(cc.y, stri);
		if(hasUnsignedTextureComponent(2))
			cc.z = MulHigh(As<UShort4>(cc.z), utri);
		else
			cc.z = MulHigh(cc.z, stri);
		if(hasUnsignedTextureComponent(3))
			cc.w = MulHigh(As<UShort4>(cc.w), utri);
		else
			cc.w = MulHigh(cc.w, stri);

		utri = ~utri;
		stri = Short4(0x7FFF) - stri;

		if(hasUnsignedTextureComponent(0))
			c.x = MulHigh(As<UShort4>(c.x), utri);
		else
			c.x = MulHigh(c.x, stri);
		if(hasUnsignedTextureComponent(1))
			c.y = MulHigh(As<UShort4>(c.y), utri);
		else
			c.y = MulHigh(c.y, stri);
		if(hasUnsignedTextureComponent(2))
			c.z = MulHigh(As<UShort4>(c.z), utri);
		else
			c.z = MulHigh(c.z, stri);
		if(hasUnsignedTextureComponent(3))
			c.w = MulHigh(As<UShort4>(c.w), utri);
		else
			c.w = MulHigh(c.w, stri);

		c.x += cc.x;
		c.y += cc.y;
		c.z += cc.z;
		c.w += cc.w;

		if(!hasUnsignedTextureComponent(0)) c.x += c.x;
		if(!hasUnsignedTextureComponent(1)) c.y += c.y;
		if(!hasUnsignedTextureComponent(2)) c.z += c.z;
		if(!hasUnsignedTextureComponent(3)) c.w += c.w;
	}

	return c;
}

Vector4s SamplerCore::sampleAniso(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, const Float4 &a, Vector4i &offset, const Int4 &sample, Float &lod, Float &anisotropy, Float4 &uDelta, Float4 &vDelta, bool secondLOD)
{
	Vector4s c;

	if(state.textureFilter != FILTER_ANISOTROPIC)
	{
		c = sampleQuad(texture, u, v, w, a, offset, sample, lod, secondLOD);
	}
	else
	{
		Int N = RoundInt(anisotropy);

		Vector4s cSum;

		cSum.x = Short4(0);
		cSum.y = Short4(0);
		cSum.z = Short4(0);
		cSum.w = Short4(0);

		Float4 A = *Pointer<Float4>(constants + OFFSET(Constants, uvWeight) + 16 * N);
		Float4 B = *Pointer<Float4>(constants + OFFSET(Constants, uvStart) + 16 * N);
		UShort4 cw = *Pointer<UShort4>(constants + OFFSET(Constants, cWeight) + 8 * N);
		Short4 sw = Short4(cw >> 1);

		Float4 du = uDelta;
		Float4 dv = vDelta;

		Float4 u0 = u + B * du;
		Float4 v0 = v + B * dv;

		du *= A;
		dv *= A;

		Int i = 0;

		Do
		{
			c = sampleQuad(texture, u0, v0, w, a, offset, sample, lod, secondLOD);

			u0 += du;
			v0 += dv;

			if(hasUnsignedTextureComponent(0))
				cSum.x += As<Short4>(MulHigh(As<UShort4>(c.x), cw));
			else
				cSum.x += MulHigh(c.x, sw);
			if(hasUnsignedTextureComponent(1))
				cSum.y += As<Short4>(MulHigh(As<UShort4>(c.y), cw));
			else
				cSum.y += MulHigh(c.y, sw);
			if(hasUnsignedTextureComponent(2))
				cSum.z += As<Short4>(MulHigh(As<UShort4>(c.z), cw));
			else
				cSum.z += MulHigh(c.z, sw);
			if(hasUnsignedTextureComponent(3))
				cSum.w += As<Short4>(MulHigh(As<UShort4>(c.w), cw));
			else
				cSum.w += MulHigh(c.w, sw);

			i++;
		}
		Until(i >= N);

		if(hasUnsignedTextureComponent(0))
			c.x = cSum.x;
		else
			c.x = AddSat(cSum.x, cSum.x);
		if(hasUnsignedTextureComponent(1))
			c.y = cSum.y;
		else
			c.y = AddSat(cSum.y, cSum.y);
		if(hasUnsignedTextureComponent(2))
			c.z = cSum.z;
		else
			c.z = AddSat(cSum.z, cSum.z);
		if(hasUnsignedTextureComponent(3))
			c.w = cSum.w;
		else
			c.w = AddSat(cSum.w, cSum.w);
	}

	return c;
}

Vector4s SamplerCore::sampleQuad(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, const Float4 &a, Vector4i &offset, const Int4 &sample, Float &lod, bool secondLOD)
{
	if(state.textureType != VK_IMAGE_VIEW_TYPE_3D)
	{
		return sampleQuad2D(texture, u, v, w, a, offset, sample, lod, secondLOD);
	}
	else
	{
		return sample3D(texture, u, v, w, offset, sample, lod, secondLOD);
	}
}

void SamplerCore::bilinearInterpolateFloat(Vector4f &output, const Short4 &uuuu0, const Short4 &vvvv0, Vector4f &c00, Vector4f &c01, Vector4f &c10, Vector4f &c11, const Pointer<Byte> &mipmap, bool interpolateComponent0, bool interpolateComponent1, bool interpolateComponent2, bool interpolateComponent3)
{
	int componentCount = textureComponentCount();

	Float4 unnormalizedUUUU0 = (Float4(uuuu0) / Float4(1 << 16)) * Float4(*Pointer<UInt4>(mipmap + OFFSET(Mipmap, width)));
	Float4 unnormalizedVVVV0 = (Float4(vvvv0) / Float4(1 << 16)) * Float4(*Pointer<UInt4>(mipmap + OFFSET(Mipmap, height)));

	Float4 frac0u = Frac(unnormalizedUUUU0);
	Float4 frac0v = Frac(unnormalizedVVVV0);

	if(interpolateComponent0 && componentCount >= 1)
	{
		c00.x = Mix(c00.x, c10.x, frac0u);
		c01.x = Mix(c01.x, c11.x, frac0u);
		output.x = Mix(c00.x, c01.x, frac0v);
	}
	if(interpolateComponent1 && componentCount >= 2)
	{
		c00.y = Mix(c00.y, c10.y, frac0u);
		c01.y = Mix(c01.y, c11.y, frac0u);
		output.y = Mix(c00.y, c01.y, frac0v);
	}
	if(interpolateComponent2 && componentCount >= 3)
	{
		c00.z = Mix(c00.z, c10.z, frac0u);
		c01.z = Mix(c01.z, c11.z, frac0u);
		output.z = Mix(c00.z, c01.z, frac0v);
	}
	if(interpolateComponent3 && componentCount >= 4)
	{
		c00.w = Mix(c00.w, c10.w, frac0u);
		c01.w = Mix(c01.w, c11.w, frac0u);
		output.w = Mix(c00.w, c01.w, frac0v);
	}
}

void SamplerCore::bilinearInterpolate(Vector4s &output, const Short4 &uuuu0, const Short4 &vvvv0, Vector4s &c00, Vector4s &c01, Vector4s &c10, Vector4s &c11, const Pointer<Byte> &mipmap)
{
	int componentCount = textureComponentCount();

	// Fractions
	UShort4 f0u = As<UShort4>(uuuu0) * UShort4(*Pointer<UInt4>(mipmap + OFFSET(Mipmap, width)));
	UShort4 f0v = As<UShort4>(vvvv0) * UShort4(*Pointer<UInt4>(mipmap + OFFSET(Mipmap, height)));

	UShort4 f1u = ~f0u;
	UShort4 f1v = ~f0v;

	UShort4 f0u0v = MulHigh(f0u, f0v);
	UShort4 f1u0v = MulHigh(f1u, f0v);
	UShort4 f0u1v = MulHigh(f0u, f1v);
	UShort4 f1u1v = MulHigh(f1u, f1v);

	// Signed fractions
	Short4 f1u1vs;
	Short4 f0u1vs;
	Short4 f1u0vs;
	Short4 f0u0vs;

	if(!hasUnsignedTextureComponent(0) || !hasUnsignedTextureComponent(1) || !hasUnsignedTextureComponent(2) || !hasUnsignedTextureComponent(3))
	{
		f1u1vs = f1u1v >> 1;
		f0u1vs = f0u1v >> 1;
		f1u0vs = f1u0v >> 1;
		f0u0vs = f0u0v >> 1;
	}

	// Bilinear interpolation
	if(componentCount >= 1)
	{
		if(has16bitTextureComponents() && hasUnsignedTextureComponent(0))
		{
			c00.x = As<UShort4>(c00.x) - MulHigh(As<UShort4>(c00.x), f0u) + MulHigh(As<UShort4>(c10.x), f0u);
			c01.x = As<UShort4>(c01.x) - MulHigh(As<UShort4>(c01.x), f0u) + MulHigh(As<UShort4>(c11.x), f0u);
			output.x = As<UShort4>(c00.x) - MulHigh(As<UShort4>(c00.x), f0v) + MulHigh(As<UShort4>(c01.x), f0v);
		}
		else
		{
			if(hasUnsignedTextureComponent(0))
			{
				c00.x = MulHigh(As<UShort4>(c00.x), f1u1v);
				c10.x = MulHigh(As<UShort4>(c10.x), f0u1v);
				c01.x = MulHigh(As<UShort4>(c01.x), f1u0v);
				c11.x = MulHigh(As<UShort4>(c11.x), f0u0v);
			}
			else
			{
				c00.x = MulHigh(c00.x, f1u1vs);
				c10.x = MulHigh(c10.x, f0u1vs);
				c01.x = MulHigh(c01.x, f1u0vs);
				c11.x = MulHigh(c11.x, f0u0vs);
			}

			output.x = (c00.x + c10.x) + (c01.x + c11.x);
			if(!hasUnsignedTextureComponent(0)) output.x = AddSat(output.x, output.x);  // Correct for signed fractions
		}
	}

	if(componentCount >= 2)
	{
		if(has16bitTextureComponents() && hasUnsignedTextureComponent(1))
		{
			c00.y = As<UShort4>(c00.y) - MulHigh(As<UShort4>(c00.y), f0u) + MulHigh(As<UShort4>(c10.y), f0u);
			c01.y = As<UShort4>(c01.y) - MulHigh(As<UShort4>(c01.y), f0u) + MulHigh(As<UShort4>(c11.y), f0u);
			output.y = As<UShort4>(c00.y) - MulHigh(As<UShort4>(c00.y), f0v) + MulHigh(As<UShort4>(c01.y), f0v);
		}
		else
		{
			if(hasUnsignedTextureComponent(1))
			{
				c00.y = MulHigh(As<UShort4>(c00.y), f1u1v);
				c10.y = MulHigh(As<UShort4>(c10.y), f0u1v);
				c01.y = MulHigh(As<UShort4>(c01.y), f1u0v);
				c11.y = MulHigh(As<UShort4>(c11.y), f0u0v);
			}
			else
			{
				c00.y = MulHigh(c00.y, f1u1vs);
				c10.y = MulHigh(c10.y, f0u1vs);
				c01.y = MulHigh(c01.y, f1u0vs);
				c11.y = MulHigh(c11.y, f0u0vs);
			}

			output.y = (c00.y + c10.y) + (c01.y + c11.y);
			if(!hasUnsignedTextureComponent(1)) output.y = AddSat(output.y, output.y);  // Correct for signed fractions
		}
	}

	if(componentCount >= 3)
	{
		if(has16bitTextureComponents() && hasUnsignedTextureComponent(2))
		{
			c00.z = As<UShort4>(c00.z) - MulHigh(As<UShort4>(c00.z), f0u) + MulHigh(As<UShort4>(c10.z), f0u);
			c01.z = As<UShort4>(c01.z) - MulHigh(As<UShort4>(c01.z), f0u) + MulHigh(As<UShort4>(c11.z), f0u);
			output.z = As<UShort4>(c00.z) - MulHigh(As<UShort4>(c00.z), f0v) + MulHigh(As<UShort4>(c01.z), f0v);
		}
		else
		{
			if(hasUnsignedTextureComponent(2))
			{
				c00.z = MulHigh(As<UShort4>(c00.z), f1u1v);
				c10.z = MulHigh(As<UShort4>(c10.z), f0u1v);
				c01.z = MulHigh(As<UShort4>(c01.z), f1u0v);
				c11.z = MulHigh(As<UShort4>(c11.z), f0u0v);
			}
			else
			{
				c00.z = MulHigh(c00.z, f1u1vs);
				c10.z = MulHigh(c10.z, f0u1vs);
				c01.z = MulHigh(c01.z, f1u0vs);
				c11.z = MulHigh(c11.z, f0u0vs);
			}

			output.z = (c00.z + c10.z) + (c01.z + c11.z);
			if(!hasUnsignedTextureComponent(2)) output.z = AddSat(output.z, output.z);  // Correct for signed fractions
		}
	}

	if(componentCount >= 4)
	{
		if(has16bitTextureComponents() && hasUnsignedTextureComponent(3))
		{
			c00.w = As<UShort4>(c00.w) - MulHigh(As<UShort4>(c00.w), f0u) + MulHigh(As<UShort4>(c10.w), f0u);
			c01.w = As<UShort4>(c01.w) - MulHigh(As<UShort4>(c01.w), f0u) + MulHigh(As<UShort4>(c11.w), f0u);
			output.w = As<UShort4>(c00.w) - MulHigh(As<UShort4>(c00.w), f0v) + MulHigh(As<UShort4>(c01.w), f0v);
		}
		else
		{
			if(hasUnsignedTextureComponent(3))
			{
				c00.w = MulHigh(As<UShort4>(c00.w), f1u1v);
				c10.w = MulHigh(As<UShort4>(c10.w), f0u1v);
				c01.w = MulHigh(As<UShort4>(c01.w), f1u0v);
				c11.w = MulHigh(As<UShort4>(c11.w), f0u0v);
			}
			else
			{
				c00.w = MulHigh(c00.w, f1u1vs);
				c10.w = MulHigh(c10.w, f0u1vs);
				c01.w = MulHigh(c01.w, f1u0vs);
				c11.w = MulHigh(c11.w, f0u0vs);
			}

			output.w = (c00.w + c10.w) + (c01.w + c11.w);
			if(!hasUnsignedTextureComponent(3)) output.w = AddSat(output.w, output.w);  // Correct for signed fractions
		}
	}
}

Vector4s SamplerCore::sampleQuad2D(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, const Float4 &a, Vector4i &offset, const Int4 &sample, Float &lod, bool secondLOD)
{
	Vector4s c;

	bool gather = (state.textureFilter == FILTER_GATHER);

	Pointer<Byte> mipmap = selectMipmap(texture, lod, secondLOD);
	Pointer<Byte> buffer = *Pointer<Pointer<Byte>>(mipmap + OFFSET(Mipmap, buffer));

	applyOffset(u, v, w, offset, mipmap);

	Short4 uuuu = address(u, state.addressingModeU);
	Short4 vvvv = address(v, state.addressingModeV);
	Short4 wwww = address(w, state.addressingModeW);
	Short4 layerIndex = computeLayerIndex16(a, mipmap);

	if(isYcbcrFormat())
	{
		uint8_t lumaBits = 8;
		uint8_t chromaBits = 8;
		switch(state.textureFormat)
		{
		case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
		case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
			lumaBits = 8;
			chromaBits = 8;
			break;
		case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
			lumaBits = 10;
			chromaBits = 10;
			break;
		default:
			UNSUPPORTED("state.textureFormat %d", (int)state.textureFormat);
			break;
		}

		// TODO: investigate apparent precision losses in dEQP-VK.ycbcr when sampling and interpolating with Short4.

		// Unnnormalized YUV values in [0, 255] for 8-bit formats, [0, 1023] for 10-bit formats.
		Vector4f yuv;
		Vector4f yuv00;
		Vector4f yuv10;
		Vector4f yuv01;
		Vector4f yuv11;

		if(state.textureFilter == FILTER_POINT)
		{
			sampleLumaTexel(yuv, uuuu, vvvv, wwww, layerIndex, sample, mipmap, buffer);
		}
		else
		{
			Short4 uuuu0 = offsetSample(uuuu, mipmap, OFFSET(Mipmap, uHalf), state.addressingModeU == ADDRESSING_WRAP, -1, lod);
			Short4 vvvv0 = offsetSample(vvvv, mipmap, OFFSET(Mipmap, vHalf), state.addressingModeV == ADDRESSING_WRAP, -1, lod);
			Short4 uuuu1 = offsetSample(uuuu, mipmap, OFFSET(Mipmap, uHalf), state.addressingModeU == ADDRESSING_WRAP, +1, lod);
			Short4 vvvv1 = offsetSample(vvvv, mipmap, OFFSET(Mipmap, vHalf), state.addressingModeV == ADDRESSING_WRAP, +1, lod);

			sampleLumaTexel(yuv00, uuuu0, vvvv0, wwww, layerIndex, sample, mipmap, buffer);
			sampleLumaTexel(yuv01, uuuu0, vvvv1, wwww, layerIndex, sample, mipmap, buffer);
			sampleLumaTexel(yuv10, uuuu1, vvvv0, wwww, layerIndex, sample, mipmap, buffer);
			sampleLumaTexel(yuv11, uuuu1, vvvv1, wwww, layerIndex, sample, mipmap, buffer);

			bilinearInterpolateFloat(yuv, uuuu0, vvvv0, yuv00, yuv01, yuv10, yuv11, mipmap, false, true, false, false);
		}

		// Pointers to the planes of YCbCr images are stored in consecutive mipmap levels.
		Pointer<Byte> mipmapU = Pointer<Byte>(mipmap + 1 * sizeof(Mipmap));
		Pointer<Byte> mipmapV = Pointer<Byte>(mipmap + 2 * sizeof(Mipmap));
		Pointer<Byte> bufferU = *Pointer<Pointer<Byte>>(mipmapU + OFFSET(Mipmap, buffer));  // U/V for 2-plane interleaved formats.
		Pointer<Byte> bufferV = *Pointer<Pointer<Byte>>(mipmapV + OFFSET(Mipmap, buffer));

		// https://registry.khronos.org/vulkan/specs/1.3-extensions/html/vkspec.html#textures-implict-reconstruction
		// but using normalized coordinates.
		Float4 chromaU = u;
		Float4 chromaV = v;
		if(state.chromaXOffset == VK_CHROMA_LOCATION_COSITED_EVEN)
		{
			chromaU += (Float4(0.25f) / Float4(*Pointer<UInt4>(mipmapU + OFFSET(Mipmap, width))));
		}
		if(state.chromaYOffset == VK_CHROMA_LOCATION_COSITED_EVEN)
		{
			chromaV += (Float4(0.25f) / Float4(*Pointer<UInt4>(mipmapU + OFFSET(Mipmap, height))));
		}

		Short4 chromaUUUU = address(chromaU, state.addressingModeU);
		Short4 chromaVVVV = address(chromaV, state.addressingModeV);

		if(state.chromaFilter == FILTER_POINT)
		{
			sampleChromaTexel(yuv, chromaUUUU, chromaVVVV, wwww, layerIndex, sample, mipmapU, bufferU, mipmapV, bufferV);
		}
		else
		{
			Short4 chromaUUUU0 = offsetSample(chromaUUUU, mipmapU, OFFSET(Mipmap, uHalf), state.addressingModeU == ADDRESSING_WRAP, -1, lod);
			Short4 chromaVVVV0 = offsetSample(chromaVVVV, mipmapU, OFFSET(Mipmap, vHalf), state.addressingModeV == ADDRESSING_WRAP, -1, lod);
			Short4 chromaUUUU1 = offsetSample(chromaUUUU, mipmapU, OFFSET(Mipmap, uHalf), state.addressingModeU == ADDRESSING_WRAP, +1, lod);
			Short4 chromaVVVV1 = offsetSample(chromaVVVV, mipmapU, OFFSET(Mipmap, vHalf), state.addressingModeV == ADDRESSING_WRAP, +1, lod);

			sampleChromaTexel(yuv00, chromaUUUU0, chromaVVVV0, wwww, layerIndex, sample, mipmapU, bufferU, mipmapV, bufferV);
			sampleChromaTexel(yuv01, chromaUUUU0, chromaVVVV1, wwww, layerIndex, sample, mipmapU, bufferU, mipmapV, bufferV);
			sampleChromaTexel(yuv10, chromaUUUU1, chromaVVVV0, wwww, layerIndex, sample, mipmapU, bufferU, mipmapV, bufferV);
			sampleChromaTexel(yuv11, chromaUUUU1, chromaVVVV1, wwww, layerIndex, sample, mipmapU, bufferU, mipmapV, bufferV);

			bilinearInterpolateFloat(yuv, chromaUUUU0, chromaVVVV0, yuv00, yuv01, yuv10, yuv11, mipmapU, true, false, true, false);
		}

		if(state.swappedChroma)
		{
			std::swap(yuv.x, yuv.z);
		}

		if(state.ycbcrModel == VK_SAMPLER_YCBCR_MODEL_CONVERSION_RGB_IDENTITY)
		{
			// Scale to the output 15-bit.
			c.x = UShort4(yuv.x) << (15 - chromaBits);
			c.y = UShort4(yuv.y) << (15 - lumaBits);
			c.z = UShort4(yuv.z) << (15 - chromaBits);
		}
		else
		{
			const float twoPowLumaBits = static_cast<float>(0x1u << lumaBits);
			const float twoPowLumaBitsMinus8 = static_cast<float>(0x1u << (lumaBits - 8));
			const float twoPowChromaBits = static_cast<float>(0x1u << chromaBits);
			const float twoPowChromaBitsMinus1 = static_cast<float>(0x1u << (chromaBits - 1));
			const float twoPowChromaBitsMinus8 = static_cast<float>(0x1u << (chromaBits - 8));

			Float4 y = Float4(yuv.y);
			Float4 u = Float4(yuv.z);
			Float4 v = Float4(yuv.x);

			if(state.studioSwing)
			{
				// See https://www.khronos.org/registry/DataFormat/specs/1.3/dataformat.1.3.html#QUANTIZATION_NARROW
				y = ((y / Float4(twoPowLumaBitsMinus8)) - Float4(16.0f)) / Float4(219.0f);
				u = ((u / Float4(twoPowChromaBitsMinus8)) - Float4(128.0f)) / Float4(224.0f);
				v = ((v / Float4(twoPowChromaBitsMinus8)) - Float4(128.0f)) / Float4(224.0f);
			}
			else
			{
				// See https://www.khronos.org/registry/DataFormat/specs/1.3/dataformat.1.3.html#QUANTIZATION_FULL
				y = y / Float4(twoPowLumaBits - 1.0f);
				u = (u - Float4(twoPowChromaBitsMinus1)) / Float4(twoPowChromaBits - 1.0f);
				v = (v - Float4(twoPowChromaBitsMinus1)) / Float4(twoPowChromaBits - 1.0f);
			}

			// Now, `y` is in [0, 1] and `u` and `v` are in [-0.5, 0.5].

			if(state.ycbcrModel == VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_IDENTITY)
			{
				c.x = Short4(v * static_cast<float>(0x7FFF));
				c.y = Short4(y * static_cast<float>(0x7FFF));
				c.z = Short4(u * static_cast<float>(0x7FFF));
			}
			else
			{
				// Generic YCbCr to RGB transformation:
				// R = Y                               +           2 * (1 - Kr) * Cr
				// G = Y - 2 * Kb * (1 - Kb) / Kg * Cb - 2 * Kr * (1 - Kr) / Kg * Cr
				// B = Y +           2 * (1 - Kb) * Cb

				float Kb = 0.114f;
				float Kr = 0.299f;

				switch(state.ycbcrModel)
				{
				case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_709:
					Kb = 0.0722f;
					Kr = 0.2126f;
					break;
				case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_601:
					Kb = 0.114f;
					Kr = 0.299f;
					break;
				case VK_SAMPLER_YCBCR_MODEL_CONVERSION_YCBCR_2020:
					Kb = 0.0593f;
					Kr = 0.2627f;
					break;
				default:
					UNSUPPORTED("ycbcrModel %d", int(state.ycbcrModel));
				}

				const float Kg = 1.0f - Kr - Kb;

				const float Rr = 2 * (1 - Kr);
				const float Gb = -2 * Kb * (1 - Kb) / Kg;
				const float Gr = -2 * Kr * (1 - Kr) / Kg;
				const float Bb = 2 * (1 - Kb);

				Float4 r = y + Float4(Rr) * v;
				Float4 g = y + Float4(Gb) * u + Float4(Gr) * v;
				Float4 b = y + Float4(Bb) * u;

				c.x = Short4(r * static_cast<float>(0x7FFF));
				c.y = Short4(g * static_cast<float>(0x7FFF));
				c.z = Short4(b * static_cast<float>(0x7FFF));
			}
		}
	}
	else  // !isYcbcrFormat()
	{
		if(state.textureFilter == FILTER_POINT)
		{
			c = sampleTexel(uuuu, vvvv, wwww, layerIndex, sample, mipmap, buffer);
		}
		else
		{
			Short4 uuuu0 = offsetSample(uuuu, mipmap, OFFSET(Mipmap, uHalf), state.addressingModeU == ADDRESSING_WRAP, -1, lod);
			Short4 vvvv0 = offsetSample(vvvv, mipmap, OFFSET(Mipmap, vHalf), state.addressingModeV == ADDRESSING_WRAP, -1, lod);
			Short4 uuuu1 = offsetSample(uuuu, mipmap, OFFSET(Mipmap, uHalf), state.addressingModeU == ADDRESSING_WRAP, +1, lod);
			Short4 vvvv1 = offsetSample(vvvv, mipmap, OFFSET(Mipmap, vHalf), state.addressingModeV == ADDRESSING_WRAP, +1, lod);

			Vector4s c00 = sampleTexel(uuuu0, vvvv0, wwww, layerIndex, sample, mipmap, buffer);
			Vector4s c10 = sampleTexel(uuuu1, vvvv0, wwww, layerIndex, sample, mipmap, buffer);
			Vector4s c01 = sampleTexel(uuuu0, vvvv1, wwww, layerIndex, sample, mipmap, buffer);
			Vector4s c11 = sampleTexel(uuuu1, vvvv1, wwww, layerIndex, sample, mipmap, buffer);

			if(!gather)  // Blend
			{
				bilinearInterpolate(c, uuuu0, vvvv0, c00, c01, c10, c11, mipmap);
			}
			else
			{
				VkComponentSwizzle swizzle = gatherSwizzle();
				switch(swizzle)
				{
				case VK_COMPONENT_SWIZZLE_ZERO:
				case VK_COMPONENT_SWIZZLE_ONE:
					// Handled at the final component swizzle.
					break;
				default:
					c.x = c01[swizzle - VK_COMPONENT_SWIZZLE_R];
					c.y = c11[swizzle - VK_COMPONENT_SWIZZLE_R];
					c.z = c10[swizzle - VK_COMPONENT_SWIZZLE_R];
					c.w = c00[swizzle - VK_COMPONENT_SWIZZLE_R];
					break;
				}
			}
		}
	}

	return c;
}

Vector4s SamplerCore::sample3D(Pointer<Byte> &texture, Float4 &u_, Float4 &v_, Float4 &w_, Vector4i &offset, const Int4 &sample, Float &lod, bool secondLOD)
{
	Vector4s c_;

	int componentCount = textureComponentCount();

	Pointer<Byte> mipmap = selectMipmap(texture, lod, secondLOD);
	Pointer<Byte> buffer = *Pointer<Pointer<Byte>>(mipmap + OFFSET(Mipmap, buffer));

	applyOffset(u_, v_, w_, offset, mipmap);

	Short4 uuuu = address(u_, state.addressingModeU);
	Short4 vvvv = address(v_, state.addressingModeV);
	Short4 wwww = address(w_, state.addressingModeW);

	if(state.textureFilter == FILTER_POINT)
	{
		c_ = sampleTexel(uuuu, vvvv, wwww, 0, sample, mipmap, buffer);
	}
	else
	{
		Vector4s c[2][2][2];

		Short4 u[2][2][2];
		Short4 v[2][2][2];
		Short4 s[2][2][2];

		for(int i = 0; i < 2; i++)
		{
			for(int j = 0; j < 2; j++)
			{
				for(int k = 0; k < 2; k++)
				{
					u[i][j][k] = offsetSample(uuuu, mipmap, OFFSET(Mipmap, uHalf), state.addressingModeU == ADDRESSING_WRAP, i * 2 - 1, lod);
					v[i][j][k] = offsetSample(vvvv, mipmap, OFFSET(Mipmap, vHalf), state.addressingModeV == ADDRESSING_WRAP, j * 2 - 1, lod);
					s[i][j][k] = offsetSample(wwww, mipmap, OFFSET(Mipmap, wHalf), state.addressingModeW == ADDRESSING_WRAP, k * 2 - 1, lod);
				}
			}
		}

		// Fractions
		UShort4 f0u = As<UShort4>(u[0][0][0]) * UShort4(*Pointer<UInt4>(mipmap + OFFSET(Mipmap, width)));
		UShort4 f0v = As<UShort4>(v[0][0][0]) * UShort4(*Pointer<UInt4>(mipmap + OFFSET(Mipmap, height)));
		UShort4 f0s = As<UShort4>(s[0][0][0]) * UShort4(*Pointer<UInt4>(mipmap + OFFSET(Mipmap, depth)));

		UShort4 f1u = ~f0u;
		UShort4 f1v = ~f0v;
		UShort4 f1s = ~f0s;

		UShort4 f[2][2][2];
		Short4 fs[2][2][2];

		f[1][1][1] = MulHigh(f1u, f1v);
		f[0][1][1] = MulHigh(f0u, f1v);
		f[1][0][1] = MulHigh(f1u, f0v);
		f[0][0][1] = MulHigh(f0u, f0v);
		f[1][1][0] = MulHigh(f1u, f1v);
		f[0][1][0] = MulHigh(f0u, f1v);
		f[1][0][0] = MulHigh(f1u, f0v);
		f[0][0][0] = MulHigh(f0u, f0v);

		f[1][1][1] = MulHigh(f[1][1][1], f1s);
		f[0][1][1] = MulHigh(f[0][1][1], f1s);
		f[1][0][1] = MulHigh(f[1][0][1], f1s);
		f[0][0][1] = MulHigh(f[0][0][1], f1s);
		f[1][1][0] = MulHigh(f[1][1][0], f0s);
		f[0][1][0] = MulHigh(f[0][1][0], f0s);
		f[1][0][0] = MulHigh(f[1][0][0], f0s);
		f[0][0][0] = MulHigh(f[0][0][0], f0s);

		// Signed fractions
		if(!hasUnsignedTextureComponent(0) || !hasUnsignedTextureComponent(1) || !hasUnsignedTextureComponent(2) || !hasUnsignedTextureComponent(3))
		{
			fs[0][0][0] = f[0][0][0] >> 1;
			fs[0][0][1] = f[0][0][1] >> 1;
			fs[0][1][0] = f[0][1][0] >> 1;
			fs[0][1][1] = f[0][1][1] >> 1;
			fs[1][0][0] = f[1][0][0] >> 1;
			fs[1][0][1] = f[1][0][1] >> 1;
			fs[1][1][0] = f[1][1][0] >> 1;
			fs[1][1][1] = f[1][1][1] >> 1;
		}

		for(int i = 0; i < 2; i++)
		{
			for(int j = 0; j < 2; j++)
			{
				for(int k = 0; k < 2; k++)
				{
					c[i][j][k] = sampleTexel(u[i][j][k], v[i][j][k], s[i][j][k], 0, sample, mipmap, buffer);

					if(componentCount >= 1)
					{
						if(hasUnsignedTextureComponent(0))
							c[i][j][k].x = MulHigh(As<UShort4>(c[i][j][k].x), f[1 - i][1 - j][1 - k]);
						else
							c[i][j][k].x = MulHigh(c[i][j][k].x, fs[1 - i][1 - j][1 - k]);
					}
					if(componentCount >= 2)
					{
						if(hasUnsignedTextureComponent(1))
							c[i][j][k].y = MulHigh(As<UShort4>(c[i][j][k].y), f[1 - i][1 - j][1 - k]);
						else
							c[i][j][k].y = MulHigh(c[i][j][k].y, fs[1 - i][1 - j][1 - k]);
					}
					if(componentCount >= 3)
					{
						if(hasUnsignedTextureComponent(2))
							c[i][j][k].z = MulHigh(As<UShort4>(c[i][j][k].z), f[1 - i][1 - j][1 - k]);
						else
							c[i][j][k].z = MulHigh(c[i][j][k].z, fs[1 - i][1 - j][1 - k]);
					}
					if(componentCount >= 4)
					{
						if(hasUnsignedTextureComponent(3))
							c[i][j][k].w = MulHigh(As<UShort4>(c[i][j][k].w), f[1 - i][1 - j][1 - k]);
						else
							c[i][j][k].w = MulHigh(c[i][j][k].w, fs[1 - i][1 - j][1 - k]);
					}

					if(i != 0 || j != 0 || k != 0)
					{
						if(componentCount >= 1) c[0][0][0].x += c[i][j][k].x;
						if(componentCount >= 2) c[0][0][0].y += c[i][j][k].y;
						if(componentCount >= 3) c[0][0][0].z += c[i][j][k].z;
						if(componentCount >= 4) c[0][0][0].w += c[i][j][k].w;
					}
				}
			}
		}

		if(componentCount >= 1) c_.x = c[0][0][0].x;
		if(componentCount >= 2) c_.y = c[0][0][0].y;
		if(componentCount >= 3) c_.z = c[0][0][0].z;
		if(componentCount >= 4) c_.w = c[0][0][0].w;

		// Correct for signed fractions
		if(componentCount >= 1)
			if(!hasUnsignedTextureComponent(0)) c_.x = AddSat(c_.x, c_.x);
		if(componentCount >= 2)
			if(!hasUnsignedTextureComponent(1)) c_.y = AddSat(c_.y, c_.y);
		if(componentCount >= 3)
			if(!hasUnsignedTextureComponent(2)) c_.z = AddSat(c_.z, c_.z);
		if(componentCount >= 4)
			if(!hasUnsignedTextureComponent(3)) c_.w = AddSat(c_.w, c_.w);
	}

	return c_;
}

Vector4f SamplerCore::sampleFloatFilter(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, const Float4 &a, const Float4 &dRef, Vector4i &offset, const Int4 &sample, Float &lod, Float &anisotropy, Float4 &uDelta, Float4 &vDelta)
{
	Vector4f c = sampleFloatAniso(texture, u, v, w, a, dRef, offset, sample, lod, anisotropy, uDelta, vDelta, false);

	if(function == Fetch)
	{
		return c;
	}

	if(state.mipmapFilter == MIPMAP_LINEAR)
	{
		Vector4f cc = sampleFloatAniso(texture, u, v, w, a, dRef, offset, sample, lod, anisotropy, uDelta, vDelta, true);

		Float4 lod4 = Float4(Frac(lod));

		c.x = (cc.x - c.x) * lod4 + c.x;
		c.y = (cc.y - c.y) * lod4 + c.y;
		c.z = (cc.z - c.z) * lod4 + c.z;
		c.w = (cc.w - c.w) * lod4 + c.w;
	}

	return c;
}

Vector4f SamplerCore::sampleFloatAniso(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, const Float4 &a, const Float4 &dRef, Vector4i &offset, const Int4 &sample, Float &lod, Float &anisotropy, Float4 &uDelta, Float4 &vDelta, bool secondLOD)
{
	Vector4f c;

	if(state.textureFilter != FILTER_ANISOTROPIC)
	{
		c = sampleFloat(texture, u, v, w, a, dRef, offset, sample, lod, secondLOD);
	}
	else
	{
		Int N = RoundInt(anisotropy);

		Vector4f cSum;

		cSum.x = Float4(0.0f);
		cSum.y = Float4(0.0f);
		cSum.z = Float4(0.0f);
		cSum.w = Float4(0.0f);

		Float4 A = *Pointer<Float4>(constants + OFFSET(Constants, uvWeight) + 16 * N);
		Float4 B = *Pointer<Float4>(constants + OFFSET(Constants, uvStart) + 16 * N);

		Float4 du = uDelta;
		Float4 dv = vDelta;

		Float4 u0 = u + B * du;
		Float4 v0 = v + B * dv;

		du *= A;
		dv *= A;

		Int i = 0;

		Do
		{
			c = sampleFloat(texture, u0, v0, w, a, dRef, offset, sample, lod, secondLOD);

			u0 += du;
			v0 += dv;

			cSum.x += c.x * A;
			cSum.y += c.y * A;
			cSum.z += c.z * A;
			cSum.w += c.w * A;

			i++;
		}
		Until(i >= N);

		c.x = cSum.x;
		c.y = cSum.y;
		c.z = cSum.z;
		c.w = cSum.w;
	}

	return c;
}

Vector4f SamplerCore::sampleFloat(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, const Float4 &a, const Float4 &dRef, Vector4i &offset, const Int4 &sample, Float &lod, bool secondLOD)
{
	if(state.textureType != VK_IMAGE_VIEW_TYPE_3D)
	{
		return sampleFloat2D(texture, u, v, w, a, dRef, offset, sample, lod, secondLOD);
	}
	else
	{
		return sampleFloat3D(texture, u, v, w, dRef, offset, sample, lod, secondLOD);
	}
}

Vector4f SamplerCore::sampleFloat2D(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, const Float4 &a, const Float4 &dRef, Vector4i &offset, const Int4 &sample, Float &lod, bool secondLOD)
{
	Vector4f c;

	int componentCount = textureComponentCount();
	bool gather = (state.textureFilter == FILTER_GATHER);

	Pointer<Byte> mipmap = selectMipmap(texture, lod, secondLOD);
	Pointer<Byte> buffer = *Pointer<Pointer<Byte>>(mipmap + OFFSET(Mipmap, buffer));

	applyOffset(u, v, w, offset, mipmap);

	Int4 x0, x1, y0, y1;
	Float4 fu, fv;
	Int4 filter = computeFilterOffset(lod);
	address(u, x0, x1, fu, mipmap, filter, OFFSET(Mipmap, width), state.addressingModeU);
	address(v, y0, y1, fv, mipmap, filter, OFFSET(Mipmap, height), state.addressingModeV);

	Int4 pitchP = As<Int4>(*Pointer<UInt4>(mipmap + OFFSET(Mipmap, pitchP), 16));
	y0 *= pitchP;

	Int4 z;
	if(state.isCube() || state.isArrayed())
	{
		Int4 face = As<Int4>(w);
		Int4 layerIndex = computeLayerIndex(a, mipmap);

		// For cube maps, the layer argument is per cube, each of which has 6 layers
		if(state.textureType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY)
		{
			layerIndex *= Int4(6);
		}

		z = state.isCube() ? face : layerIndex;

		if(state.textureType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY)
		{
			z += layerIndex;
		}

		z *= *Pointer<Int4>(mipmap + OFFSET(Mipmap, sliceP), 16);
	}

	if(state.textureFilter == FILTER_POINT || (function == Fetch))
	{
		c = sampleTexel(x0, y0, z, dRef, sample, mipmap, buffer);
	}
	else
	{
		y1 *= pitchP;

		Vector4f c00 = sampleTexel(x0, y0, z, dRef, sample, mipmap, buffer);
		Vector4f c10 = sampleTexel(x1, y0, z, dRef, sample, mipmap, buffer);
		Vector4f c01 = sampleTexel(x0, y1, z, dRef, sample, mipmap, buffer);
		Vector4f c11 = sampleTexel(x1, y1, z, dRef, sample, mipmap, buffer);

		if(!gather)  // Blend
		{
			if(componentCount >= 1) c00.x = c00.x + fu * (c10.x - c00.x);
			if(componentCount >= 2) c00.y = c00.y + fu * (c10.y - c00.y);
			if(componentCount >= 3) c00.z = c00.z + fu * (c10.z - c00.z);
			if(componentCount >= 4) c00.w = c00.w + fu * (c10.w - c00.w);

			if(componentCount >= 1) c01.x = c01.x + fu * (c11.x - c01.x);
			if(componentCount >= 2) c01.y = c01.y + fu * (c11.y - c01.y);
			if(componentCount >= 3) c01.z = c01.z + fu * (c11.z - c01.z);
			if(componentCount >= 4) c01.w = c01.w + fu * (c11.w - c01.w);

			if(componentCount >= 1) c.x = c00.x + fv * (c01.x - c00.x);
			if(componentCount >= 2) c.y = c00.y + fv * (c01.y - c00.y);
			if(componentCount >= 3) c.z = c00.z + fv * (c01.z - c00.z);
			if(componentCount >= 4) c.w = c00.w + fv * (c01.w - c00.w);
		}
		else  // Gather
		{
			VkComponentSwizzle swizzle = gatherSwizzle();
			switch(swizzle)
			{
			case VK_COMPONENT_SWIZZLE_ZERO:
			case VK_COMPONENT_SWIZZLE_ONE:
				// Handled at the final component swizzle.
				break;
			default:
				c.x = c01[swizzle - VK_COMPONENT_SWIZZLE_R];
				c.y = c11[swizzle - VK_COMPONENT_SWIZZLE_R];
				c.z = c10[swizzle - VK_COMPONENT_SWIZZLE_R];
				c.w = c00[swizzle - VK_COMPONENT_SWIZZLE_R];
				break;
			}
		}
	}

	return c;
}

Vector4f SamplerCore::sampleFloat3D(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, const Float4 &dRef, Vector4i &offset, const Int4 &sample, Float &lod, bool secondLOD)
{
	Vector4f c;

	int componentCount = textureComponentCount();

	Pointer<Byte> mipmap = selectMipmap(texture, lod, secondLOD);
	Pointer<Byte> buffer = *Pointer<Pointer<Byte>>(mipmap + OFFSET(Mipmap, buffer));

	applyOffset(u, v, w, offset, mipmap);

	Int4 x0, x1, y0, y1, z0, z1;
	Float4 fu, fv, fw;
	Int4 filter = computeFilterOffset(lod);
	address(u, x0, x1, fu, mipmap, filter, OFFSET(Mipmap, width), state.addressingModeU);
	address(v, y0, y1, fv, mipmap, filter, OFFSET(Mipmap, height), state.addressingModeV);
	address(w, z0, z1, fw, mipmap, filter, OFFSET(Mipmap, depth), state.addressingModeW);

	Int4 pitchP = As<Int4>(*Pointer<UInt4>(mipmap + OFFSET(Mipmap, pitchP), 16));
	Int4 sliceP = As<Int4>(*Pointer<UInt4>(mipmap + OFFSET(Mipmap, sliceP), 16));
	y0 *= pitchP;
	z0 *= sliceP;

	if(state.textureFilter == FILTER_POINT || (function == Fetch))
	{
		c = sampleTexel(x0, y0, z0, dRef, sample, mipmap, buffer);
	}
	else
	{
		y1 *= pitchP;
		z1 *= sliceP;

		Vector4f c000 = sampleTexel(x0, y0, z0, dRef, sample, mipmap, buffer);
		Vector4f c100 = sampleTexel(x1, y0, z0, dRef, sample, mipmap, buffer);
		Vector4f c010 = sampleTexel(x0, y1, z0, dRef, sample, mipmap, buffer);
		Vector4f c110 = sampleTexel(x1, y1, z0, dRef, sample, mipmap, buffer);
		Vector4f c001 = sampleTexel(x0, y0, z1, dRef, sample, mipmap, buffer);
		Vector4f c101 = sampleTexel(x1, y0, z1, dRef, sample, mipmap, buffer);
		Vector4f c011 = sampleTexel(x0, y1, z1, dRef, sample, mipmap, buffer);
		Vector4f c111 = sampleTexel(x1, y1, z1, dRef, sample, mipmap, buffer);

		// Blend first slice
		if(componentCount >= 1) c000.x = c000.x + fu * (c100.x - c000.x);
		if(componentCount >= 2) c000.y = c000.y + fu * (c100.y - c000.y);
		if(componentCount >= 3) c000.z = c000.z + fu * (c100.z - c000.z);
		if(componentCount >= 4) c000.w = c000.w + fu * (c100.w - c000.w);

		if(componentCount >= 1) c010.x = c010.x + fu * (c110.x - c010.x);
		if(componentCount >= 2) c010.y = c010.y + fu * (c110.y - c010.y);
		if(componentCount >= 3) c010.z = c010.z + fu * (c110.z - c010.z);
		if(componentCount >= 4) c010.w = c010.w + fu * (c110.w - c010.w);

		if(componentCount >= 1) c000.x = c000.x + fv * (c010.x - c000.x);
		if(componentCount >= 2) c000.y = c000.y + fv * (c010.y - c000.y);
		if(componentCount >= 3) c000.z = c000.z + fv * (c010.z - c000.z);
		if(componentCount >= 4) c000.w = c000.w + fv * (c010.w - c000.w);

		// Blend second slice
		if(componentCount >= 1) c001.x = c001.x + fu * (c101.x - c001.x);
		if(componentCount >= 2) c001.y = c001.y + fu * (c101.y - c001.y);
		if(componentCount >= 3) c001.z = c001.z + fu * (c101.z - c001.z);
		if(componentCount >= 4) c001.w = c001.w + fu * (c101.w - c001.w);

		if(componentCount >= 1) c011.x = c011.x + fu * (c111.x - c011.x);
		if(componentCount >= 2) c011.y = c011.y + fu * (c111.y - c011.y);
		if(componentCount >= 3) c011.z = c011.z + fu * (c111.z - c011.z);
		if(componentCount >= 4) c011.w = c011.w + fu * (c111.w - c011.w);

		if(componentCount >= 1) c001.x = c001.x + fv * (c011.x - c001.x);
		if(componentCount >= 2) c001.y = c001.y + fv * (c011.y - c001.y);
		if(componentCount >= 3) c001.z = c001.z + fv * (c011.z - c001.z);
		if(componentCount >= 4) c001.w = c001.w + fv * (c011.w - c001.w);

		// Blend slices
		if(componentCount >= 1) c.x = c000.x + fw * (c001.x - c000.x);
		if(componentCount >= 2) c.y = c000.y + fw * (c001.y - c000.y);
		if(componentCount >= 3) c.z = c000.z + fw * (c001.z - c000.z);
		if(componentCount >= 4) c.w = c000.w + fw * (c001.w - c000.w);
	}

	return c;
}

static Float log2sqrt(Float lod)
{
	// log2(sqrt(lod))                              // Equals 0.25 * log2(lod^2).
	lod *= lod;                                     // Squaring doubles the exponent and produces an extra bit of precision.
	lod = Float(As<Int>(lod)) - Float(0x3F800000);  // Interpret as integer and subtract the exponent bias.
	lod *= As<Float>(Int(0x33000000));              // Scale by 0.25 * 2^-23 (mantissa length).

	return lod;
}

static Float log2(Float lod)
{
	lod *= lod;                                     // Squaring doubles the exponent and produces an extra bit of precision.
	lod = Float(As<Int>(lod)) - Float(0x3F800000);  // Interpret as integer and subtract the exponent bias.
	lod *= As<Float>(Int(0x33800000));              // Scale by 0.5 * 2^-23 (mantissa length).

	return lod;
}

void SamplerCore::computeLod1D(Pointer<Byte> &texture, Float &lod, Float4 &uuuu, const Float4 &dsx, const Float4 &dsy)
{
	Float4 dudxy;

	if(function != Grad)  // Implicit
	{
		dudxy = uuuu.yz - uuuu.xx;
	}
	else
	{
		dudxy = UnpackLow(dsx, dsy);
	}

	// Scale by texture dimensions.
	Float4 dUdxy = dudxy * *Pointer<Float4>(texture + OFFSET(Texture, widthWidthHeightHeight));

	// Note we could take the absolute value here and omit the square root below,
	// but this is more consistent with the 2D calculation and still cheap.
	Float4 dU2dxy = dUdxy * dUdxy;

	lod = Max(Float(dU2dxy.x), Float(dU2dxy.y));
	lod = log2sqrt(lod);
}

void SamplerCore::computeLod2D(Pointer<Byte> &texture, Float &lod, Float &anisotropy, Float4 &uDelta, Float4 &vDelta, Float4 &uuuu, Float4 &vvvv, const Float4 &dsx, const Float4 &dsy)
{
	Float4 duvdxy;

	if(function != Grad)  // Implicit
	{
		duvdxy = Float4(uuuu.yz, vvvv.yz) - Float4(uuuu.xx, vvvv.xx);
	}
	else
	{
		Float4 dudxy = Float4(dsx.xx, dsy.xx);
		Float4 dvdxy = Float4(dsx.yy, dsy.yy);

		duvdxy = Float4(dudxy.xz, dvdxy.xz);
	}

	// Scale by texture dimensions.
	Float4 dUVdxy = duvdxy * *Pointer<Float4>(texture + OFFSET(Texture, widthWidthHeightHeight));

	Float4 dUV2dxy = dUVdxy * dUVdxy;
	Float4 dUV2 = dUV2dxy.xy + dUV2dxy.zw;

	lod = Max(Float(dUV2.x), Float(dUV2.y));  // Square length of major axis

	if(state.textureFilter == FILTER_ANISOTROPIC)
	{
		Float det = Abs(Float(dUVdxy.x) * Float(dUVdxy.w) - Float(dUVdxy.y) * Float(dUVdxy.z));

		Float4 dudx = duvdxy.xxxx;
		Float4 dudy = duvdxy.yyyy;
		Float4 dvdx = duvdxy.zzzz;
		Float4 dvdy = duvdxy.wwww;

		Int4 mask = As<Int4>(CmpNLT(dUV2.x, dUV2.y));
		uDelta = As<Float4>((As<Int4>(dudx) & mask) | ((As<Int4>(dudy) & ~mask)));
		vDelta = As<Float4>((As<Int4>(dvdx) & mask) | ((As<Int4>(dvdy) & ~mask)));

		anisotropy = lod * Rcp(det, true /* relaxedPrecision */);
		anisotropy = Min(anisotropy, state.maxAnisotropy);

		// TODO(b/151263485): While we always need `lod` above, when there's only
		// a single mipmap level the following calculations could be skipped.
		lod *= Rcp(anisotropy * anisotropy, true /* relaxedPrecision */);
	}

	lod = log2sqrt(lod);  // log2(sqrt(lod))
}

void SamplerCore::computeLodCube(Pointer<Byte> &texture, Float &lod, Float4 &u, Float4 &v, Float4 &w, const Float4 &dsx, const Float4 &dsy, Float4 &M)
{
	Float4 dudxy, dvdxy, dsdxy;

	if(function != Grad)  // Implicit
	{
		Float4 U = u * M;
		Float4 V = v * M;
		Float4 W = w * M;

		dudxy = Abs(U - U.xxxx);
		dvdxy = Abs(V - V.xxxx);
		dsdxy = Abs(W - W.xxxx);
	}
	else
	{
		dudxy = Float4(dsx.xx, dsy.xx);
		dvdxy = Float4(dsx.yy, dsy.yy);
		dsdxy = Float4(dsx.zz, dsy.zz);

		dudxy = Abs(dudxy * Float4(M.x));
		dvdxy = Abs(dvdxy * Float4(M.x));
		dsdxy = Abs(dsdxy * Float4(M.x));
	}

	// Compute the largest Manhattan distance in two dimensions.
	// This takes the footprint across adjacent faces into account.
	Float4 duvdxy = dudxy + dvdxy;
	Float4 dusdxy = dudxy + dsdxy;
	Float4 dvsdxy = dvdxy + dsdxy;

	dudxy = Max(Max(duvdxy, dusdxy), dvsdxy);

	lod = Max(Float(dudxy.y), Float(dudxy.z));  // TODO: Max(dudxy.y, dudxy.z);

	// Scale by texture dimension.
	lod *= *Pointer<Float>(texture + OFFSET(Texture, width));

	lod = log2(lod);
}

void SamplerCore::computeLod3D(Pointer<Byte> &texture, Float &lod, Float4 &uuuu, Float4 &vvvv, Float4 &wwww, const Float4 &dsx, const Float4 &dsy)
{
	Float4 dudxy, dvdxy, dsdxy;

	if(function != Grad)  // Implicit
	{
		dudxy = uuuu - uuuu.xxxx;
		dvdxy = vvvv - vvvv.xxxx;
		dsdxy = wwww - wwww.xxxx;
	}
	else
	{
		dudxy = Float4(dsx.xx, dsy.xx);
		dvdxy = Float4(dsx.yy, dsy.yy);
		dsdxy = Float4(dsx.zz, dsy.zz);
	}

	// Scale by texture dimensions.
	dudxy *= *Pointer<Float4>(texture + OFFSET(Texture, width));
	dvdxy *= *Pointer<Float4>(texture + OFFSET(Texture, height));
	dsdxy *= *Pointer<Float4>(texture + OFFSET(Texture, depth));

	dudxy *= dudxy;
	dvdxy *= dvdxy;
	dsdxy *= dsdxy;

	dudxy += dvdxy;
	dudxy += dsdxy;

	lod = Max(Float(dudxy.y), Float(dudxy.z));  // TODO: Max(dudxy.y, dudxy.z);

	lod = log2sqrt(lod);  // log2(sqrt(lod))
}

Int4 SamplerCore::cubeFace(Float4 &U, Float4 &V, Float4 &x, Float4 &y, Float4 &z, Float4 &M)
{
	// TODO: Comply with Vulkan recommendation:
	// Vulkan 1.1: "The rules should have as the first rule that rz wins over ry and rx, and the second rule that ry wins over rx."

	Int4 xn = CmpLT(x, 0.0f);  // x < 0
	Int4 yn = CmpLT(y, 0.0f);  // y < 0
	Int4 zn = CmpLT(z, 0.0f);  // z < 0

	Float4 absX = Abs(x);
	Float4 absY = Abs(y);
	Float4 absZ = Abs(z);

	Int4 xy = CmpNLE(absX, absY);  // abs(x) > abs(y)
	Int4 yz = CmpNLE(absY, absZ);  // abs(y) > abs(z)
	Int4 zx = CmpNLE(absZ, absX);  // abs(z) > abs(x)
	Int4 xMajor = xy & ~zx;        // abs(x) > abs(y) && abs(x) > abs(z)
	Int4 yMajor = yz & ~xy;        // abs(y) > abs(z) && abs(y) > abs(x)
	Int4 zMajor = zx & ~yz;        // abs(z) > abs(x) && abs(z) > abs(y)

	// FACE_POSITIVE_X = 000b
	// FACE_NEGATIVE_X = 001b
	// FACE_POSITIVE_Y = 010b
	// FACE_NEGATIVE_Y = 011b
	// FACE_POSITIVE_Z = 100b
	// FACE_NEGATIVE_Z = 101b

	Int yAxis = SignMask(yMajor);
	Int zAxis = SignMask(zMajor);

	Int4 n = ((xn & xMajor) | (yn & yMajor) | (zn & zMajor)) & Int4(0x80000000);
	Int negative = SignMask(n);

	Int faces = *Pointer<Int>(constants + OFFSET(Constants, transposeBit0) + negative * 4);
	faces |= *Pointer<Int>(constants + OFFSET(Constants, transposeBit1) + yAxis * 4);
	faces |= *Pointer<Int>(constants + OFFSET(Constants, transposeBit2) + zAxis * 4);

	Int4 face;
	face.x = faces & 0x7;
	face.y = (faces >> 4) & 0x7;
	face.z = (faces >> 8) & 0x7;
	face.w = (faces >> 12) & 0x7;

	M = Max(Max(absX, absY), absZ);

	// U = xMajor ? (neg ^ -z) : ((zMajor & neg) ^ x)
	U = As<Float4>((xMajor & (n ^ As<Int4>(-z))) | (~xMajor & ((zMajor & n) ^ As<Int4>(x))));

	// V = !yMajor ? -y : (n ^ z)
	V = As<Float4>((~yMajor & As<Int4>(-y)) | (yMajor & (n ^ As<Int4>(z))));

	M = reciprocal(M) * 0.5f;
	U = U * M + 0.5f;
	V = V * M + 0.5f;

	return face;
}

void SamplerCore::applyOffset(Float4 &u, Float4 &v, Float4 &w, Vector4i &offset, Pointer<Byte> mipmap)
{
	if(function.offset)
	{
		if(function == Fetch)
		{
			// Unnormalized coordinates
			u = As<Float4>(As<Int4>(u) + offset.x);
			if(state.is2D() || state.is3D() || state.isCube())
			{
				v = As<Float4>(As<Int4>(v) + offset.y);
				if(state.is3D())
				{
					w = As<Float4>(As<Int4>(w) + offset.z);
				}
			}
		}
		else
		{
			// Normalized coordinates
			UInt4 width = *Pointer<UInt4>(mipmap + OFFSET(Mipmap, width));
			u += Float4(offset.x) / Float4(width);
			if(state.is2D() || state.is3D() || state.isCube())
			{
				UInt4 height = *Pointer<UInt4>(mipmap + OFFSET(Mipmap, height));
				v += Float4(offset.y) / Float4(height);
				if(state.is3D())
				{
					UInt4 depth = *Pointer<UInt4>(mipmap + OFFSET(Mipmap, depth));
					w += Float4(offset.z) / Float4(depth);
				}
			}
		}
	}
}

void SamplerCore::computeIndices(UInt index[4], Short4 uuuu, Short4 vvvv, Short4 wwww, const Short4 &layerIndex, const Int4 &sample, const Pointer<Byte> &mipmap)
{
	uuuu = MulHigh(As<UShort4>(uuuu), UShort4(*Pointer<UInt4>(mipmap + OFFSET(Mipmap, width))));

	UInt4 indices = Int4(uuuu);

	if(state.is2D() || state.is3D() || state.isCube())
	{
		vvvv = MulHigh(As<UShort4>(vvvv), UShort4(*Pointer<UInt4>(mipmap + OFFSET(Mipmap, height))));

		Short4 uv0uv1 = As<Short4>(UnpackLow(uuuu, vvvv));
		Short4 uv2uv3 = As<Short4>(UnpackHigh(uuuu, vvvv));
		Int2 i01 = MulAdd(uv0uv1, *Pointer<Short4>(mipmap + OFFSET(Mipmap, onePitchP)));
		Int2 i23 = MulAdd(uv2uv3, *Pointer<Short4>(mipmap + OFFSET(Mipmap, onePitchP)));

		indices = UInt4(As<UInt2>(i01), As<UInt2>(i23));
	}

	if(state.is3D())
	{
		wwww = MulHigh(As<UShort4>(wwww), UShort4(*Pointer<Int4>(mipmap + OFFSET(Mipmap, depth))));

		indices += As<UInt4>(Int4(As<UShort4>(wwww))) * *Pointer<UInt4>(mipmap + OFFSET(Mipmap, sliceP));
	}

	if(state.isArrayed())
	{
		Int4 layer = Int4(As<UShort4>(layerIndex));

		if(state.textureType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY)
		{
			layer *= Int4(6);
		}

		UInt4 layerOffset = As<UInt4>(layer) * *Pointer<UInt4>(mipmap + OFFSET(Mipmap, sliceP));

		indices += layerOffset;
	}

	if(function.sample)
	{
		UInt4 sampleOffset = Min(As<UInt4>(sample), *Pointer<UInt4>(mipmap + OFFSET(Mipmap, sampleMax), 16)) *
		                     *Pointer<UInt4>(mipmap + OFFSET(Mipmap, samplePitchP), 16);
		indices += sampleOffset;
	}

	index[0] = Extract(indices, 0);
	index[1] = Extract(indices, 1);
	index[2] = Extract(indices, 2);
	index[3] = Extract(indices, 3);
}

void SamplerCore::computeIndices(UInt index[4], Int4 uuuu, Int4 vvvv, Int4 wwww, const Int4 &sample, Int4 valid, const Pointer<Byte> &mipmap)
{
	UInt4 indices = uuuu;

	if(state.is2D() || state.is3D() || state.isCube())
	{
		indices += As<UInt4>(vvvv);
	}

	if(state.is3D() || state.isCube() || state.isArrayed())
	{
		indices += As<UInt4>(wwww);
	}

	if(function.sample)
	{
		indices += Min(As<UInt4>(sample), *Pointer<UInt4>(mipmap + OFFSET(Mipmap, sampleMax), 16)) *
		           *Pointer<UInt4>(mipmap + OFFSET(Mipmap, samplePitchP), 16);
	}

	if(borderModeActive())
	{
		// Texels out of range are still sampled before being replaced
		// with the border color, so sample them at linear index 0.
		indices &= As<UInt4>(valid);
	}

	for(int i = 0; i < 4; i++)
	{
		index[i] = Extract(As<Int4>(indices), i);
	}
}

Vector4s SamplerCore::sampleTexel(UInt index[4], Pointer<Byte> buffer)
{
	Vector4s c;

	if(has16bitPackedTextureFormat())
	{
		c.x = Insert(c.x, Pointer<Short>(buffer)[index[0]], 0);
		c.x = Insert(c.x, Pointer<Short>(buffer)[index[1]], 1);
		c.x = Insert(c.x, Pointer<Short>(buffer)[index[2]], 2);
		c.x = Insert(c.x, Pointer<Short>(buffer)[index[3]], 3);

		switch(state.textureFormat)
		{
		case VK_FORMAT_R5G6B5_UNORM_PACK16:
			c.z = (c.x & Short4(0x001Fu)) << 11;
			c.y = (c.x & Short4(0x07E0u)) << 5;
			c.x = (c.x & Short4(0xF800u));
			break;
		case VK_FORMAT_B5G6R5_UNORM_PACK16:
			c.z = (c.x & Short4(0xF800u));
			c.y = (c.x & Short4(0x07E0u)) << 5;
			c.x = (c.x & Short4(0x001Fu)) << 11;
			break;
		case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
			c.w = (c.x << 12) & Short4(0xF000u);
			c.z = (c.x << 8) & Short4(0xF000u);
			c.y = (c.x << 4) & Short4(0xF000u);
			c.x = (c.x) & Short4(0xF000u);
			break;
		case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
			c.w = (c.x << 12) & Short4(0xF000u);
			c.z = (c.x) & Short4(0xF000u);
			c.y = (c.x << 4) & Short4(0xF000u);
			c.x = (c.x << 8) & Short4(0xF000u);
			break;
		case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
			c.w = (c.x) & Short4(0xF000u);
			c.z = (c.x << 12) & Short4(0xF000u);
			c.y = (c.x << 8) & Short4(0xF000u);
			c.x = (c.x << 4) & Short4(0xF000u);
			break;
		case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
			c.w = (c.x) & Short4(0xF000u);
			c.z = (c.x << 4) & Short4(0xF000u);
			c.y = (c.x << 8) & Short4(0xF000u);
			c.x = (c.x << 12) & Short4(0xF000u);
			break;
		case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
			c.w = (c.x << 15) & Short4(0x8000u);
			c.z = (c.x << 10) & Short4(0xF800u);
			c.y = (c.x << 5) & Short4(0xF800u);
			c.x = (c.x) & Short4(0xF800u);
			break;
		case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
			c.w = (c.x << 15) & Short4(0x8000u);
			c.z = (c.x) & Short4(0xF800u);
			c.y = (c.x << 5) & Short4(0xF800u);
			c.x = (c.x << 10) & Short4(0xF800u);
			break;
		case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
			c.w = (c.x) & Short4(0x8000u);
			c.z = (c.x << 11) & Short4(0xF800u);
			c.y = (c.x << 6) & Short4(0xF800u);
			c.x = (c.x << 1) & Short4(0xF800u);
			break;
		default:
			ASSERT(false);
		}
	}
	else if(has8bitTextureComponents())
	{
		switch(textureComponentCount())
		{
		case 4:
			{
				Byte4 c0 = Pointer<Byte4>(buffer)[index[0]];
				Byte4 c1 = Pointer<Byte4>(buffer)[index[1]];
				Byte4 c2 = Pointer<Byte4>(buffer)[index[2]];
				Byte4 c3 = Pointer<Byte4>(buffer)[index[3]];
				c.x = Unpack(c0, c1);
				c.y = Unpack(c2, c3);

				switch(state.textureFormat)
				{
				case VK_FORMAT_B8G8R8A8_UNORM:
				case VK_FORMAT_B8G8R8A8_SRGB:
					c.z = As<Short4>(UnpackLow(c.x, c.y));
					c.x = As<Short4>(UnpackHigh(c.x, c.y));
					c.y = c.z;
					c.w = c.x;
					c.z = UnpackLow(As<Byte8>(Short4(0)), As<Byte8>(c.z));
					c.y = UnpackHigh(As<Byte8>(Short4(0)), As<Byte8>(c.y));
					c.x = UnpackLow(As<Byte8>(Short4(0)), As<Byte8>(c.x));
					c.w = UnpackHigh(As<Byte8>(Short4(0)), As<Byte8>(c.w));
					break;
				case VK_FORMAT_R8G8B8A8_UNORM:
				case VK_FORMAT_R8G8B8A8_SNORM:
				case VK_FORMAT_R8G8B8A8_SINT:
				case VK_FORMAT_R8G8B8A8_SRGB:
				case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
				case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
				case VK_FORMAT_A8B8G8R8_SINT_PACK32:
				case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
					c.z = As<Short4>(UnpackHigh(c.x, c.y));
					c.x = As<Short4>(UnpackLow(c.x, c.y));
					c.y = c.x;
					c.w = c.z;
					c.x = UnpackLow(As<Byte8>(Short4(0)), As<Byte8>(c.x));
					c.y = UnpackHigh(As<Byte8>(Short4(0)), As<Byte8>(c.y));
					c.z = UnpackLow(As<Byte8>(Short4(0)), As<Byte8>(c.z));
					c.w = UnpackHigh(As<Byte8>(Short4(0)), As<Byte8>(c.w));
					// Propagate sign bit
					if(state.textureFormat == VK_FORMAT_R8G8B8A8_SINT ||
					   state.textureFormat == VK_FORMAT_A8B8G8R8_SINT_PACK32)
					{
						c.x >>= 8;
						c.y >>= 8;
						c.z >>= 8;
						c.w >>= 8;
					}
					break;
				case VK_FORMAT_R8G8B8A8_UINT:
				case VK_FORMAT_A8B8G8R8_UINT_PACK32:
					c.z = As<Short4>(UnpackHigh(c.x, c.y));
					c.x = As<Short4>(UnpackLow(c.x, c.y));
					c.y = c.x;
					c.w = c.z;
					c.x = UnpackLow(As<Byte8>(c.x), As<Byte8>(Short4(0)));
					c.y = UnpackHigh(As<Byte8>(c.y), As<Byte8>(Short4(0)));
					c.z = UnpackLow(As<Byte8>(c.z), As<Byte8>(Short4(0)));
					c.w = UnpackHigh(As<Byte8>(c.w), As<Byte8>(Short4(0)));
					break;
				default:
					ASSERT(false);
				}
			}
			break;
		case 2:
			c.x = Insert(c.x, Pointer<Short>(buffer)[index[0]], 0);
			c.x = Insert(c.x, Pointer<Short>(buffer)[index[1]], 1);
			c.x = Insert(c.x, Pointer<Short>(buffer)[index[2]], 2);
			c.x = Insert(c.x, Pointer<Short>(buffer)[index[3]], 3);

			switch(state.textureFormat)
			{
			case VK_FORMAT_R8G8_UNORM:
			case VK_FORMAT_R8G8_SNORM:
			case VK_FORMAT_R8G8_SRGB:
				c.y = (c.x & Short4(0xFF00u));
				c.x = (c.x << 8);
				break;
			case VK_FORMAT_R8G8_SINT:
				c.y = c.x >> 8;
				c.x = (c.x << 8) >> 8;  // Propagate sign bit
				break;
			case VK_FORMAT_R8G8_UINT:
				c.y = As<Short4>(As<UShort4>(c.x) >> 8);
				c.x &= Short4(0x00FFu);
				break;
			default:
				ASSERT(false);
			}
			break;
		case 1:
			{
				Int c0 = Int(*Pointer<Byte>(buffer + index[0]));
				Int c1 = Int(*Pointer<Byte>(buffer + index[1]));
				Int c2 = Int(*Pointer<Byte>(buffer + index[2]));
				Int c3 = Int(*Pointer<Byte>(buffer + index[3]));
				c0 = c0 | (c1 << 8) | (c2 << 16) | (c3 << 24);

				switch(state.textureFormat)
				{
				case VK_FORMAT_R8_SINT:
				case VK_FORMAT_R8_UINT:
				case VK_FORMAT_S8_UINT:
					{
						Int zero(0);
						c.x = Unpack(As<Byte4>(c0), As<Byte4>(zero));
						// Propagate sign bit
						if(state.textureFormat == VK_FORMAT_R8_SINT)
						{
							c.x = (c.x << 8) >> 8;
						}
					}
					break;
				case VK_FORMAT_R8_SNORM:
				case VK_FORMAT_R8_UNORM:
				case VK_FORMAT_R8_SRGB:
					// TODO: avoid populating the low bits at all.
					c.x = Unpack(As<Byte4>(c0));
					c.x &= Short4(0xFF00u);
					break;
				default:
					c.x = Unpack(As<Byte4>(c0));
					break;
				}
			}
			break;
		default:
			ASSERT(false);
		}
	}
	else if(has16bitTextureComponents())
	{
		switch(textureComponentCount())
		{
		case 4:
			c.x = Pointer<Short4>(buffer)[index[0]];
			c.y = Pointer<Short4>(buffer)[index[1]];
			c.z = Pointer<Short4>(buffer)[index[2]];
			c.w = Pointer<Short4>(buffer)[index[3]];
			transpose4x4(c.x, c.y, c.z, c.w);
			break;
		case 2:
			c.x = *Pointer<Short4>(buffer + 4 * index[0]);
			c.x = As<Short4>(UnpackLow(c.x, *Pointer<Short4>(buffer + 4 * index[1])));
			c.z = *Pointer<Short4>(buffer + 4 * index[2]);
			c.z = As<Short4>(UnpackLow(c.z, *Pointer<Short4>(buffer + 4 * index[3])));
			c.y = c.x;
			c.x = UnpackLow(As<Int2>(c.x), As<Int2>(c.z));
			c.y = UnpackHigh(As<Int2>(c.y), As<Int2>(c.z));
			break;
		case 1:
			c.x = Insert(c.x, Pointer<Short>(buffer)[index[0]], 0);
			c.x = Insert(c.x, Pointer<Short>(buffer)[index[1]], 1);
			c.x = Insert(c.x, Pointer<Short>(buffer)[index[2]], 2);
			c.x = Insert(c.x, Pointer<Short>(buffer)[index[3]], 3);
			break;
		default:
			ASSERT(false);
		}
	}
	else if(state.textureFormat == VK_FORMAT_A2B10G10R10_UNORM_PACK32)
	{
		Int4 cc;
		cc = Insert(cc, Pointer<Int>(buffer)[index[0]], 0);
		cc = Insert(cc, Pointer<Int>(buffer)[index[1]], 1);
		cc = Insert(cc, Pointer<Int>(buffer)[index[2]], 2);
		cc = Insert(cc, Pointer<Int>(buffer)[index[3]], 3);

		c.x = Short4(cc << 6) & Short4(0xFFC0u);
		c.y = Short4(cc >> 4) & Short4(0xFFC0u);
		c.z = Short4(cc >> 14) & Short4(0xFFC0u);
		c.w = Short4(cc >> 16) & Short4(0xC000u);
	}
	else if(state.textureFormat == VK_FORMAT_A2R10G10B10_UNORM_PACK32)
	{
		Int4 cc;
		cc = Insert(cc, Pointer<Int>(buffer)[index[0]], 0);
		cc = Insert(cc, Pointer<Int>(buffer)[index[1]], 1);
		cc = Insert(cc, Pointer<Int>(buffer)[index[2]], 2);
		cc = Insert(cc, Pointer<Int>(buffer)[index[3]], 3);

		c.x = Short4(cc >> 14) & Short4(0xFFC0u);
		c.y = Short4(cc >> 4) & Short4(0xFFC0u);
		c.z = Short4(cc << 6) & Short4(0xFFC0u);
		c.w = Short4(cc >> 16) & Short4(0xC000u);
	}
	else if(state.textureFormat == VK_FORMAT_A2B10G10R10_UINT_PACK32)
	{
		Int4 cc;
		cc = Insert(cc, Pointer<Int>(buffer)[index[0]], 0);
		cc = Insert(cc, Pointer<Int>(buffer)[index[1]], 1);
		cc = Insert(cc, Pointer<Int>(buffer)[index[2]], 2);
		cc = Insert(cc, Pointer<Int>(buffer)[index[3]], 3);

		c.x = Short4(cc & Int4(0x3FF));
		c.y = Short4((cc >> 10) & Int4(0x3FF));
		c.z = Short4((cc >> 20) & Int4(0x3FF));
		c.w = Short4((cc >> 30) & Int4(0x3));
	}
	else if(state.textureFormat == VK_FORMAT_A2R10G10B10_UINT_PACK32)
	{
		Int4 cc;
		cc = Insert(cc, Pointer<Int>(buffer)[index[0]], 0);
		cc = Insert(cc, Pointer<Int>(buffer)[index[1]], 1);
		cc = Insert(cc, Pointer<Int>(buffer)[index[2]], 2);
		cc = Insert(cc, Pointer<Int>(buffer)[index[3]], 3);

		c.z = Short4((cc & Int4(0x3FF)));
		c.y = Short4(((cc >> 10) & Int4(0x3FF)));
		c.x = Short4(((cc >> 20) & Int4(0x3FF)));
		c.w = Short4(((cc >> 30) & Int4(0x3)));
	}
	else
		ASSERT(false);

	if(state.textureFormat.isSRGBformat())
	{
		for(int i = 0; i < textureComponentCount(); i++)
		{
			if(isRGBComponent(i))
			{
				// The current table-based sRGB conversion requires 0xFF00 to represent 1.0.
				ASSERT(state.textureFormat.has8bitTextureComponents());

				sRGBtoLinearFF00(c[i]);
			}
		}
	}

	return c;
}

void SamplerCore::sampleLumaTexel(Vector4f &output, Short4 &uuuu, Short4 &vvvv, Short4 &wwww, const Short4 &layerIndex, const Int4 &sample, Pointer<Byte> &lumaMipmap, Pointer<Byte> lumaBuffer)
{
	ASSERT(isYcbcrFormat());

	UInt index[4];
	computeIndices(index, uuuu, vvvv, wwww, layerIndex, sample, lumaMipmap);

	// Luminance (either 8-bit or 10-bit in bottom bits).
	UShort4 Y;

	switch(state.textureFormat)
	{
	case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
	case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
		{
			Y = Insert(Y, UShort(lumaBuffer[index[0]]), 0);
			Y = Insert(Y, UShort(lumaBuffer[index[1]]), 1);
			Y = Insert(Y, UShort(lumaBuffer[index[2]]), 2);
			Y = Insert(Y, UShort(lumaBuffer[index[3]]), 3);
		}
		break;
	case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
		{
			Y = Insert(Y, Pointer<UShort>(lumaBuffer)[index[0]], 0);
			Y = Insert(Y, Pointer<UShort>(lumaBuffer)[index[1]], 1);
			Y = Insert(Y, Pointer<UShort>(lumaBuffer)[index[2]], 2);
			Y = Insert(Y, Pointer<UShort>(lumaBuffer)[index[3]], 3);
			// Top 10 bits of each 16 bits:
			Y = (Y & UShort4(0xFFC0u)) >> 6;
		}
		break;
	default:
		UNSUPPORTED("state.textureFormat %d", (int)state.textureFormat);
		break;
	}

	output.y = Float4(Y);
}

void SamplerCore::sampleChromaTexel(Vector4f &output, Short4 &uuuu, Short4 &vvvv, Short4 &wwww, const Short4 &layerIndex, const Int4 &sample, Pointer<Byte> &mipmapU, Pointer<Byte> bufferU, Pointer<Byte> &mipmapV, Pointer<Byte> bufferV)
{
	ASSERT(isYcbcrFormat());

	UInt index[4];

	// Chroma (either 8-bit or 10-bit in bottom bits).
	UShort4 U, V;
	computeIndices(index, uuuu, vvvv, wwww, layerIndex, sample, mipmapU);

	switch(state.textureFormat)
	{
	case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
		{
			U = Insert(U, UShort(bufferU[index[0]]), 0);
			U = Insert(U, UShort(bufferU[index[1]]), 1);
			U = Insert(U, UShort(bufferU[index[2]]), 2);
			U = Insert(U, UShort(bufferU[index[3]]), 3);

			V = Insert(V, UShort(bufferV[index[0]]), 0);
			V = Insert(V, UShort(bufferV[index[1]]), 1);
			V = Insert(V, UShort(bufferV[index[2]]), 2);
			V = Insert(V, UShort(bufferV[index[3]]), 3);
		}
		break;
	case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
		{
			UShort4 UV;
			UV = Insert(UV, Pointer<UShort>(bufferU)[index[0]], 0);
			UV = Insert(UV, Pointer<UShort>(bufferU)[index[1]], 1);
			UV = Insert(UV, Pointer<UShort>(bufferU)[index[2]], 2);
			UV = Insert(UV, Pointer<UShort>(bufferU)[index[3]], 3);

			U = (UV & UShort4(0x00FFu));
			V = (UV & UShort4(0xFF00u)) >> 8;
		}
		break;
	case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
		{
			UInt4 UV;
			UV = Insert(UV, Pointer<UInt>(bufferU)[index[0]], 0);
			UV = Insert(UV, Pointer<UInt>(bufferU)[index[1]], 1);
			UV = Insert(UV, Pointer<UInt>(bufferU)[index[2]], 2);
			UV = Insert(UV, Pointer<UInt>(bufferU)[index[3]], 3);
			// Top 10 bits of first 16-bits:
			U = UShort4((UV & UInt4(0x0000FFC0u)) >> 6);
			// Top 10 bits of second 16-bits:
			V = UShort4((UV & UInt4(0xFFC00000u)) >> 22);
		}
		break;
	default:
		UNSUPPORTED("state.textureFormat %d", (int)state.textureFormat);
		break;
	}

	output.x = Float4(V);
	output.z = Float4(U);
}

Vector4s SamplerCore::sampleTexel(Short4 &uuuu, Short4 &vvvv, Short4 &wwww, const Short4 &layerIndex, const Int4 &sample, Pointer<Byte> &mipmap, Pointer<Byte> buffer)
{
	ASSERT(!isYcbcrFormat());

	UInt index[4];
	computeIndices(index, uuuu, vvvv, wwww, layerIndex, sample, mipmap);

	return sampleTexel(index, buffer);
}

Vector4f SamplerCore::sampleTexel(Int4 &uuuu, Int4 &vvvv, Int4 &wwww, const Float4 &dRef, const Int4 &sample, Pointer<Byte> &mipmap, Pointer<Byte> buffer)
{
	Int4 valid;

	if(borderModeActive())
	{
		// Valid texels have positive coordinates.
		Int4 negative = uuuu;
		if(state.is2D() || state.is3D() || state.isCube()) negative |= vvvv;
		if(state.is3D() || state.isCube() || state.isArrayed()) negative |= wwww;
		valid = CmpNLT(negative, Int4(0));
	}

	UInt index[4];
	computeIndices(index, uuuu, vvvv, wwww, sample, valid, mipmap);

	Vector4f c;

	if(hasFloatTexture() || has32bitIntegerTextureComponents())
	{
		UInt4 t0, t1, t2, t3;

		switch(state.textureFormat)
		{
		case VK_FORMAT_R16_SFLOAT:
			t0 = Int4(*Pointer<UShort4>(buffer + index[0] * 2));
			t1 = Int4(*Pointer<UShort4>(buffer + index[1] * 2));
			t2 = Int4(*Pointer<UShort4>(buffer + index[2] * 2));
			t3 = Int4(*Pointer<UShort4>(buffer + index[3] * 2));

			c.x.x = Extract(As<Float4>(halfToFloatBits(t0)), 0);
			c.x.y = Extract(As<Float4>(halfToFloatBits(t1)), 0);
			c.x.z = Extract(As<Float4>(halfToFloatBits(t2)), 0);
			c.x.w = Extract(As<Float4>(halfToFloatBits(t3)), 0);
			break;
		case VK_FORMAT_R16G16_SFLOAT:
			t0 = Int4(*Pointer<UShort4>(buffer + index[0] * 4));
			t1 = Int4(*Pointer<UShort4>(buffer + index[1] * 4));
			t2 = Int4(*Pointer<UShort4>(buffer + index[2] * 4));
			t3 = Int4(*Pointer<UShort4>(buffer + index[3] * 4));

			// TODO: shuffles
			c.x = As<Float4>(halfToFloatBits(t0));
			c.y = As<Float4>(halfToFloatBits(t1));
			c.z = As<Float4>(halfToFloatBits(t2));
			c.w = As<Float4>(halfToFloatBits(t3));
			transpose4x4(c.x, c.y, c.z, c.w);
			break;
		case VK_FORMAT_R16G16B16A16_SFLOAT:
			t0 = Int4(*Pointer<UShort4>(buffer + index[0] * 8));
			t1 = Int4(*Pointer<UShort4>(buffer + index[1] * 8));
			t2 = Int4(*Pointer<UShort4>(buffer + index[2] * 8));
			t3 = Int4(*Pointer<UShort4>(buffer + index[3] * 8));

			c.x = As<Float4>(halfToFloatBits(t0));
			c.y = As<Float4>(halfToFloatBits(t1));
			c.z = As<Float4>(halfToFloatBits(t2));
			c.w = As<Float4>(halfToFloatBits(t3));
			transpose4x4(c.x, c.y, c.z, c.w);
			break;
		case VK_FORMAT_R32_SFLOAT:
		case VK_FORMAT_R32_SINT:
		case VK_FORMAT_R32_UINT:
		case VK_FORMAT_D32_SFLOAT:
			// TODO: Optimal shuffling?
			c.x.x = *Pointer<Float>(buffer + index[0] * 4);
			c.x.y = *Pointer<Float>(buffer + index[1] * 4);
			c.x.z = *Pointer<Float>(buffer + index[2] * 4);
			c.x.w = *Pointer<Float>(buffer + index[3] * 4);
			break;
		case VK_FORMAT_R32G32_SFLOAT:
		case VK_FORMAT_R32G32_SINT:
		case VK_FORMAT_R32G32_UINT:
			// TODO: Optimal shuffling?
			c.x.xy = *Pointer<Float4>(buffer + index[0] * 8);
			c.x.zw = *Pointer<Float4>(buffer + index[1] * 8 - 8);
			c.z.xy = *Pointer<Float4>(buffer + index[2] * 8);
			c.z.zw = *Pointer<Float4>(buffer + index[3] * 8 - 8);
			c.y = c.x;
			c.x = Float4(c.x.xz, c.z.xz);
			c.y = Float4(c.y.yw, c.z.yw);
			break;
		case VK_FORMAT_R32G32B32A32_SFLOAT:
		case VK_FORMAT_R32G32B32A32_SINT:
		case VK_FORMAT_R32G32B32A32_UINT:
			c.x = *Pointer<Float4>(buffer + index[0] * 16, 16);
			c.y = *Pointer<Float4>(buffer + index[1] * 16, 16);
			c.z = *Pointer<Float4>(buffer + index[2] * 16, 16);
			c.w = *Pointer<Float4>(buffer + index[3] * 16, 16);
			transpose4x4(c.x, c.y, c.z, c.w);
			break;
		case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
			{
				Float4 t;  // TODO: add Insert(UInt4, RValue<UInt>)
				t.x = *Pointer<Float>(buffer + index[0] * 4);
				t.y = *Pointer<Float>(buffer + index[1] * 4);
				t.z = *Pointer<Float>(buffer + index[2] * 4);
				t.w = *Pointer<Float>(buffer + index[3] * 4);
				t0 = As<UInt4>(t);
				c.w = Float4(UInt4(1) << ((t0 >> 27) & UInt4(0x1F))) * Float4(1.0f / (1 << 24));
				c.x = Float4(t0 & UInt4(0x1FF)) * c.w;
				c.y = Float4((t0 >> 9) & UInt4(0x1FF)) * c.w;
				c.z = Float4((t0 >> 18) & UInt4(0x1FF)) * c.w;
			}
			break;
		case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
			{
				Float4 t;  // TODO: add Insert(UInt4, RValue<UInt>)
				t.x = *Pointer<Float>(buffer + index[0] * 4);
				t.y = *Pointer<Float>(buffer + index[1] * 4);
				t.z = *Pointer<Float>(buffer + index[2] * 4);
				t.w = *Pointer<Float>(buffer + index[3] * 4);
				t0 = As<UInt4>(t);
				c.x = As<Float4>(halfToFloatBits((t0 << 4) & UInt4(0x7FF0)));
				c.y = As<Float4>(halfToFloatBits((t0 >> 7) & UInt4(0x7FF0)));
				c.z = As<Float4>(halfToFloatBits((t0 >> 17) & UInt4(0x7FE0)));
			}
			break;
		default:
			UNSUPPORTED("Format %d", VkFormat(state.textureFormat));
		}
	}
	else
	{
		ASSERT(!isYcbcrFormat());

		Vector4s cs = sampleTexel(index, buffer);

		bool isInteger = state.textureFormat.isUnnormalizedInteger();
		int componentCount = textureComponentCount();
		for(int n = 0; n < componentCount; n++)
		{
			if(hasUnsignedTextureComponent(n))
			{
				if(isInteger)
				{
					c[n] = As<Float4>(Int4(As<UShort4>(cs[n])));
				}
				else
				{
					c[n] = Float4(As<UShort4>(cs[n]));
				}
			}
			else
			{
				if(isInteger)
				{
					c[n] = As<Float4>(Int4(cs[n]));
				}
				else
				{
					c[n] = Float4(cs[n]);
				}
			}
		}
	}

	if(borderModeActive())
	{
		c = replaceBorderTexel(c, valid);
	}

	if(state.compareEnable)
	{
		Float4 ref = dRef;

		if(!hasFloatTexture())
		{
			// D16_UNORM: clamp reference, normalize texel value
			ref = Min(Max(ref, Float4(0.0f)), Float4(1.0f));
			c.x = c.x * Float4(1.0f / 0xFFFF);
		}

		Int4 boolean;

		switch(state.compareOp)
		{
		case VK_COMPARE_OP_LESS_OR_EQUAL: boolean = CmpLE(ref, c.x); break;
		case VK_COMPARE_OP_GREATER_OR_EQUAL: boolean = CmpNLT(ref, c.x); break;
		case VK_COMPARE_OP_LESS: boolean = CmpLT(ref, c.x); break;
		case VK_COMPARE_OP_GREATER: boolean = CmpNLE(ref, c.x); break;
		case VK_COMPARE_OP_EQUAL: boolean = CmpEQ(ref, c.x); break;
		case VK_COMPARE_OP_NOT_EQUAL: boolean = CmpNEQ(ref, c.x); break;
		case VK_COMPARE_OP_ALWAYS: boolean = Int4(-1); break;
		case VK_COMPARE_OP_NEVER: boolean = Int4(0); break;
		default: ASSERT(false);
		}

		c.x = As<Float4>(boolean & As<Int4>(Float4(1.0f)));
		c.y = Float4(0.0f);
		c.z = Float4(0.0f);
		c.w = Float4(1.0f);
	}

	return c;
}

Vector4f SamplerCore::replaceBorderTexel(const Vector4f &c, Int4 valid)
{
	Vector4i border;

	const bool scaled = hasNormalizedFormat();
	const sw::float4 scaleComp = scaled ? getComponentScale() : sw::float4(1.0f, 1.0f, 1.0f, 1.0f);

	switch(state.border)
	{
	case VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK:
	case VK_BORDER_COLOR_INT_TRANSPARENT_BLACK:
		border.x = Int4(0);
		border.y = Int4(0);
		border.z = Int4(0);
		border.w = Int4(0);
		break;
	case VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK:
		border.x = Int4(0);
		border.y = Int4(0);
		border.z = Int4(0);
		border.w = Int4(bit_cast<int>(scaleComp.w));
		break;
	case VK_BORDER_COLOR_INT_OPAQUE_BLACK:
		border.x = Int4(0);
		border.y = Int4(0);
		border.z = Int4(0);
		border.w = Int4(1);
		break;
	case VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE:
		border.x = Int4(bit_cast<int>(scaleComp.x));
		border.y = Int4(bit_cast<int>(scaleComp.y));
		border.z = Int4(bit_cast<int>(scaleComp.z));
		border.w = Int4(bit_cast<int>(scaleComp.w));
		break;
	case VK_BORDER_COLOR_INT_OPAQUE_WHITE:
		border.x = Int4(1);
		border.y = Int4(1);
		border.z = Int4(1);
		border.w = Int4(1);
		break;
	case VK_BORDER_COLOR_FLOAT_CUSTOM_EXT:
		// This bit-casts from float to int in C++ code instead of Reactor code
		// because Reactor does not guarantee preserving infinity (b/140302841).
		border.x = Int4(bit_cast<int>(scaleComp.x * state.customBorder.float32[0]));
		border.y = Int4(bit_cast<int>(scaleComp.y * state.customBorder.float32[1]));
		border.z = Int4(bit_cast<int>(scaleComp.z * state.customBorder.float32[2]));
		border.w = Int4(bit_cast<int>(scaleComp.w * state.customBorder.float32[3]));
		break;
	case VK_BORDER_COLOR_INT_CUSTOM_EXT:
		border.x = Int4(state.customBorder.int32[0]);
		border.y = Int4(state.customBorder.int32[1]);
		border.z = Int4(state.customBorder.int32[2]);
		border.w = Int4(state.customBorder.int32[3]);
		break;
	default:
		UNSUPPORTED("sint/uint/sfloat border: %u", state.border);
	}

	Vector4f out;
	out.x = As<Float4>((valid & As<Int4>(c.x)) | (~valid & border.x));  // TODO: IfThenElse()
	out.y = As<Float4>((valid & As<Int4>(c.y)) | (~valid & border.y));
	out.z = As<Float4>((valid & As<Int4>(c.z)) | (~valid & border.z));
	out.w = As<Float4>((valid & As<Int4>(c.w)) | (~valid & border.w));

	return out;
}

Pointer<Byte> SamplerCore::selectMipmap(const Pointer<Byte> &texture, const Float &lod, bool secondLOD)
{
	Pointer<Byte> mipmap0 = texture + OFFSET(Texture, mipmap[0]);

	if(state.mipmapFilter == MIPMAP_NONE)
	{
		return mipmap0;
	}

	Int ilod;

	if(state.mipmapFilter == MIPMAP_POINT)
	{
		// TODO: Preferred formula is ceil(lod + 0.5) - 1
		ilod = RoundInt(lod);
	}
	else  // MIPMAP_LINEAR
	{
		ilod = Int(lod);
	}

	return mipmap0 + ilod * sizeof(Mipmap) + secondLOD * sizeof(Mipmap);
}

Int4 SamplerCore::computeFilterOffset(Float &lod)
{
	if(state.textureFilter == FILTER_POINT)
	{
		return Int4(0);
	}
	else if(state.textureFilter == FILTER_MIN_LINEAR_MAG_POINT)
	{
		return CmpNLE(Float4(lod), Float4(0.0f));
	}
	else if(state.textureFilter == FILTER_MIN_POINT_MAG_LINEAR)
	{
		return CmpLE(Float4(lod), Float4(0.0f));
	}

	return Int4(~0);
}

Short4 SamplerCore::address(const Float4 &uw, AddressingMode addressingMode)
{
	if(addressingMode == ADDRESSING_UNUSED)
	{
		return Short4(0);  // TODO(b/134669567): Optimize for 1D filtering
	}
	else if(addressingMode == ADDRESSING_CLAMP || addressingMode == ADDRESSING_BORDER)
	{
		Float4 clamp = Min(Max(uw, Float4(0.0f)), Float4(65535.0f / 65536.0f));

		return Short4(Int4(clamp * Float4(1 << 16)));
	}
	else if(addressingMode == ADDRESSING_MIRROR)
	{
		Int4 convert = Int4(uw * Float4(1 << 16));
		Int4 mirror = (convert << 15) >> 31;

		convert ^= mirror;

		return Short4(convert);
	}
	else if(addressingMode == ADDRESSING_MIRRORONCE)
	{
		// Absolute value
		Int4 convert = Int4(Abs(uw * Float4(1 << 16)));

		// Clamp
		convert -= Int4(0x00008000, 0x00008000, 0x00008000, 0x00008000);
		convert = As<Int4>(PackSigned(convert, convert));

		return As<Short4>(Int2(convert)) + Short4(0x8000u);
	}
	else  // Wrap
	{
		return Short4(Int4(uw * Float4(1 << 16)));
	}
}

Short4 SamplerCore::computeLayerIndex16(const Float4 &a, Pointer<Byte> &mipmap)
{
	if(!state.isArrayed())
	{
		return {};
	}

	Int4 layers = *Pointer<Int4>(mipmap + OFFSET(Mipmap, depth));

	return Short4(Min(Max(RoundInt(a), Int4(0)), layers - Int4(1)));
}

// TODO: Eliminate when the gather + mirror addressing case is handled by mirroring the footprint.
static Int4 mirror(Int4 n)
{
	auto positive = CmpNLT(n, Int4(0));
	return (positive & n) | (~positive & (-(Int4(1) + n)));
}

static Int4 mod(Int4 n, Int4 d)
{
	auto x = n % d;
	auto positive = CmpNLT(x, Int4(0));
	return (positive & x) | (~positive & (x + d));
}

void SamplerCore::address(const Float4 &uvw, Int4 &xyz0, Int4 &xyz1, Float4 &f, Pointer<Byte> &mipmap, Int4 &filter, int whd, AddressingMode addressingMode)
{
	if(addressingMode == ADDRESSING_UNUSED)
	{
		f = Float4(0.0f);  // TODO(b/134669567): Optimize for 1D filtering
		return;
	}

	Int4 dim = As<Int4>(*Pointer<UInt4>(mipmap + whd, 16));
	Int4 maxXYZ = dim - Int4(1);

	if(function == Fetch)  // Unnormalized coordinates
	{
		Int4 xyz = As<Int4>(uvw);
		xyz0 = Min(Max(xyz, Int4(0)), maxXYZ);

		// VK_EXT_image_robustness requires checking for out-of-bounds accesses.
		// TODO(b/162327166): Only perform bounds checks when VK_EXT_image_robustness is enabled.
		// If the above clamping altered the result, the access is out-of-bounds.
		// In that case set the coordinate to -1 to perform texel replacement later.
		Int4 outOfBounds = CmpNEQ(xyz, xyz0);
		xyz0 |= outOfBounds;
	}
	else if(addressingMode == ADDRESSING_CUBEFACE)
	{
		xyz0 = As<Int4>(uvw);
	}
	else
	{
		const int oneBits = 0x3F7FFFFF;  // Value just under 1.0f

		Float4 coord = uvw;

		if(state.unnormalizedCoordinates)
		{
			switch(addressingMode)
			{
			case ADDRESSING_CLAMP:
				coord = Min(Max(coord, Float4(0.0f)), Float4(dim) * As<Float4>(Int4(oneBits)));
				break;
			case ADDRESSING_BORDER:
				// Don't map to a valid range here.
				break;
			default:
				// "If unnormalizedCoordinates is VK_TRUE, addressModeU and addressModeV must each be
				//  either VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE or VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER"
				UNREACHABLE("addressingMode %d", int(addressingMode));
				break;
			}
		}
		else if(state.textureFilter == FILTER_GATHER && addressingMode == ADDRESSING_MIRROR)
		{
			// Gather requires the 'footprint' of the texels from which a component is taken, to also mirror around.
			// Therefore we can't just compute one texel's location and find the other ones at +1 offsets from it.
			// Here we handle that case separately by doing the mirroring per texel coordinate.
			// TODO: Mirror the footprint by adjusting the sign of the 0.5f and 1 offsets.

			coord = coord * Float4(dim);
			coord -= Float4(0.5f);
			Float4 floor = Floor(coord);
			xyz0 = Int4(floor);
			xyz1 = xyz0 + Int4(1);

			xyz0 = (maxXYZ)-mirror(mod(xyz0, Int4(2) * dim) - dim);
			xyz1 = (maxXYZ)-mirror(mod(xyz1, Int4(2) * dim) - dim);

			return;
		}
		else
		{
			switch(addressingMode)
			{
			case ADDRESSING_CLAMP:
			case ADDRESSING_SEAMLESS:
				// While cube face coordinates are nominally already in the [0.0, 1.0] range
				// due to the projection, and numerical imprecision is tolerated due to the
				// border of pixels for seamless filtering, the projection doesn't cause
				// range normalization for Inf and NaN values. So we always clamp.
				{
					Float4 one = As<Float4>(Int4(oneBits));
					coord = Min(Max(coord, Float4(0.0f)), one);
				}
				break;
			case ADDRESSING_MIRROR:
				{
					Float4 one = As<Float4>(Int4(oneBits));
					coord = coord * Float4(0.5f);
					coord = Float4(2.0f) * Abs(coord - Round(coord));
					coord = Min(coord, one);
				}
				break;
			case ADDRESSING_MIRRORONCE:
				{
					Float4 one = As<Float4>(Int4(oneBits));
					coord = Min(Abs(coord), one);
				}
				break;
			case ADDRESSING_BORDER:
				// Don't map to a valid range here.
				break;
			default:  // Wrap
				coord = Frac(coord);
				break;
			}

			coord = coord * Float4(dim);
		}

		if(state.textureFilter == FILTER_POINT)
		{
			if(addressingMode == ADDRESSING_BORDER)
			{
				xyz0 = Int4(Floor(coord));
			}
			else  // Can't have negative coordinates, so floor() is redundant when casting to int.
			{
				xyz0 = Int4(coord);
			}
		}
		else
		{
			if(state.textureFilter == FILTER_MIN_POINT_MAG_LINEAR ||
			   state.textureFilter == FILTER_MIN_LINEAR_MAG_POINT)
			{
				coord -= As<Float4>(As<Int4>(Float4(0.5f)) & filter);
			}
			else
			{
				coord -= Float4(0.5f);
			}

			Float4 floor = Floor(coord);
			xyz0 = Int4(floor);
			f = coord - floor;
		}

		if(addressingMode == ADDRESSING_SEAMLESS)  // Adjust for border.
		{
			xyz0 += Int4(1);
		}

		xyz1 = xyz0 - filter;  // Increment

		if(addressingMode == ADDRESSING_BORDER)
		{
			// Replace the coordinates with -1 if they're out of range.
			Int4 border0 = CmpLT(xyz0, Int4(0)) | CmpNLT(xyz0, dim);
			Int4 border1 = CmpLT(xyz1, Int4(0)) | CmpNLT(xyz1, dim);
			xyz0 |= border0;
			xyz1 |= border1;
		}
		else if(state.textureFilter != FILTER_POINT)
		{
			switch(addressingMode)
			{
			case ADDRESSING_SEAMLESS:
				break;
			case ADDRESSING_MIRROR:
			case ADDRESSING_MIRRORONCE:
			case ADDRESSING_CLAMP:
				xyz0 = Max(xyz0, Int4(0));
				xyz1 = Min(xyz1, maxXYZ);
				break;
			default:  // Wrap
				{
					Int4 under = CmpLT(xyz0, Int4(0));
					xyz0 = (under & maxXYZ) | (~under & xyz0);  // xyz < 0 ? dim - 1 : xyz   // TODO: IfThenElse()

					Int4 nover = CmpLT(xyz1, dim);
					xyz1 = nover & xyz1;  // xyz >= dim ? 0 : xyz
				}
				break;
			}
		}
	}
}

Int4 SamplerCore::computeLayerIndex(const Float4 &a, Pointer<Byte> &mipmap)
{
	if(!state.isArrayed())
	{
		return {};
	}

	Int4 layers = *Pointer<Int4>(mipmap + OFFSET(Mipmap, depth), 16);
	Int4 maxLayer = layers - Int4(1);

	if(function == Fetch)  // Unnormalized coordinates
	{
		Int4 xyz = As<Int4>(a);
		Int4 xyz0 = Min(Max(xyz, Int4(0)), maxLayer);

		// VK_EXT_image_robustness requires checking for out-of-bounds accesses.
		// TODO(b/162327166): Only perform bounds checks when VK_EXT_image_robustness is enabled.
		// If the above clamping altered the result, the access is out-of-bounds.
		// In that case set the coordinate to -1 to perform texel replacement later.
		Int4 outOfBounds = CmpNEQ(xyz, xyz0);
		xyz0 |= outOfBounds;

		return xyz0;
	}
	else
	{
		return Min(Max(RoundInt(a), Int4(0)), maxLayer);
	}
}

void SamplerCore::sRGBtoLinearFF00(Short4 &c)
{
	c = As<UShort4>(c) >> 8;

	Pointer<Byte> LUT = Pointer<Byte>(constants + OFFSET(Constants, sRGBtoLinearFF_FF00));

	c = Insert(c, *Pointer<Short>(LUT + 2 * Int(Extract(c, 0))), 0);
	c = Insert(c, *Pointer<Short>(LUT + 2 * Int(Extract(c, 1))), 1);
	c = Insert(c, *Pointer<Short>(LUT + 2 * Int(Extract(c, 2))), 2);
	c = Insert(c, *Pointer<Short>(LUT + 2 * Int(Extract(c, 3))), 3);
}

bool SamplerCore::hasNormalizedFormat() const
{
	return state.textureFormat.isSignedNormalized() || state.textureFormat.isUnsignedNormalized();
}

bool SamplerCore::hasFloatTexture() const
{
	return state.textureFormat.isFloatFormat();
}

bool SamplerCore::hasUnnormalizedIntegerTexture() const
{
	return state.textureFormat.isUnnormalizedInteger();
}

bool SamplerCore::hasUnsignedTextureComponent(int component) const
{
	return state.textureFormat.isUnsignedComponent(component);
}

int SamplerCore::textureComponentCount() const
{
	return state.textureFormat.componentCount();
}

bool SamplerCore::has16bitPackedTextureFormat() const
{
	return state.textureFormat.has16bitPackedTextureFormat();
}

bool SamplerCore::has8bitTextureComponents() const
{
	return state.textureFormat.has8bitTextureComponents();
}

bool SamplerCore::has16bitTextureComponents() const
{
	return state.textureFormat.has16bitTextureComponents();
}

bool SamplerCore::has32bitIntegerTextureComponents() const
{
	return state.textureFormat.has32bitIntegerTextureComponents();
}

bool SamplerCore::isYcbcrFormat() const
{
	return state.textureFormat.isYcbcrFormat();
}

bool SamplerCore::isRGBComponent(int component) const
{
	return state.textureFormat.isRGBComponent(component);
}

bool SamplerCore::borderModeActive() const
{
	return state.addressingModeU == ADDRESSING_BORDER ||
	       state.addressingModeV == ADDRESSING_BORDER ||
	       state.addressingModeW == ADDRESSING_BORDER;
}

VkComponentSwizzle SamplerCore::gatherSwizzle() const
{
	switch(state.gatherComponent)
	{
	case 0: return state.swizzle.r;
	case 1: return state.swizzle.g;
	case 2: return state.swizzle.b;
	case 3: return state.swizzle.a;
	default:
		UNREACHABLE("Invalid component");
		return VK_COMPONENT_SWIZZLE_R;
	}
}

sw::float4 SamplerCore::getComponentScale() const
{
	// TODO(b/204709464): Unlike other formats, the fixed-point representation of the formats below are handled with bit extension.
	// This special handling of such formats should be removed later.
	switch(state.textureFormat)
	{
	case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
	case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
	case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
		return sw::float4(0x7FFF, 0x7FFF, 0x7FFF, 0x7FFF);
	default:
		break;
	};

	const sw::int4 bits = state.textureFormat.bitsPerComponent();
	const sw::int4 shift = sw::int4(16 - bits.x, 16 - bits.y, 16 - bits.z, 16 - bits.w);
	const uint16_t sign = state.textureFormat.isUnsigned() ? 0xFFFF : 0x7FFF;

	return sw::float4(static_cast<uint16_t>(0xFFFF << shift.x) & sign,
	                  static_cast<uint16_t>(0xFFFF << shift.y) & sign,
	                  static_cast<uint16_t>(0xFFFF << shift.z) & sign,
	                  static_cast<uint16_t>(0xFFFF << shift.w) & sign);
}

int SamplerCore::getGatherComponent() const
{
	VkComponentSwizzle swizzle = gatherSwizzle();

	switch(swizzle)
	{
	default: UNSUPPORTED("VkComponentSwizzle %d", (int)swizzle); return 0;
	case VK_COMPONENT_SWIZZLE_R:
	case VK_COMPONENT_SWIZZLE_G:
	case VK_COMPONENT_SWIZZLE_B:
	case VK_COMPONENT_SWIZZLE_A:
		// Normalize all components using the gather component scale.
		return swizzle - VK_COMPONENT_SWIZZLE_R;
	case VK_COMPONENT_SWIZZLE_ZERO:
	case VK_COMPONENT_SWIZZLE_ONE:
		// These cases are handled later.
		return 0;
	}

	return 0;
}

}  // namespace sw
