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

#ifndef sw_SamplerCore_hpp
#define sw_SamplerCore_hpp

#include "ShaderCore.hpp"
#include "Device/Sampler.hpp"
#include "Reactor/Print.hpp"
#include "Reactor/Reactor.hpp"

namespace sw {

using namespace rr;

enum SamplerMethod : uint32_t
{
	Implicit,      // Compute gradients (pixel shader only).
	Bias,          // Compute gradients and add provided bias.
	Lod,           // Use provided LOD.
	Grad,          // Use provided gradients.
	Fetch,         // Use provided integer coordinates.
	Base,          // Sample base level.
	Query,         // Return implicit LOD.
	Gather,        // Return one channel of each texel in footprint.
	Read,          // Read a texel from an image without a sampler.
	Write,         // Write a texel to an image without a sampler.
	TexelPointer,  // Form a pointer to a texel of an image.
	SAMPLER_METHOD_LAST = TexelPointer,
};

// TODO(b/129523279): Eliminate and use SpirvShader::ImageInstruction instead.
struct SamplerFunction
{
	SamplerFunction(SamplerMethod method, bool offset = false, bool sample = false)
	    : method(method)
	    , offset(offset)
	    , sample(sample)
	{}

	operator SamplerMethod() const { return method; }

	const SamplerMethod method;
	const bool offset;
	const bool sample;
};

class SamplerCore
{
public:
	SamplerCore(Pointer<Byte> &constants, const Sampler &state, SamplerFunction function);

	SIMD::Float4 sampleTexture(Pointer<Byte> &texture, SIMD::Float uvwa[4], const SIMD::Float &dRef, const Float &lodOrBias, const SIMD::Float &dsx, const SIMD::Float &dsy, SIMD::Int offset[4], const SIMD::Int &sample);

private:
	Vector4f sampleTexture128(Pointer<Byte> &texture, Float4 uvwa[4], const Float4 &dRef, const Float &lodOrBias, const Float4 &dsx, const Float4 &dsy, Vector4i &offset, const Int4 &sample);

	Float4 applySwizzle(const Vector4f &c, VkComponentSwizzle swizzle, bool integer);
	Short4 offsetSample(Short4 &uvw, Pointer<Byte> &mipmap, int halfOffset, bool wrap, int count, Float &lod);
	Vector4s sampleFilter(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, const Float4 &a, Vector4i &offset, const Int4 &sample, Float &lod, Float &anisotropy, Float4 &uDelta, Float4 &vDelta);
	Vector4s sampleAniso(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, const Float4 &a, Vector4i &offset, const Int4 &sample, Float &lod, Float &anisotropy, Float4 &uDelta, Float4 &vDelta, bool secondLOD);
	Vector4s sampleQuad(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, const Float4 &a, Vector4i &offset, const Int4 &sample, Float &lod, bool secondLOD);
	Vector4s sampleQuad2D(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, const Float4 &a, Vector4i &offset, const Int4 &sample, Float &lod, bool secondLOD);
	Vector4s sample3D(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, Vector4i &offset, const Int4 &sample, Float &lod, bool secondLOD);
	Vector4f sampleFloatFilter(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, const Float4 &a, const Float4 &dRef, Vector4i &offset, const Int4 &sample, Float &lod, Float &anisotropy, Float4 &uDelta, Float4 &vDelta);
	Vector4f sampleFloatAniso(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, const Float4 &a, const Float4 &dRef, Vector4i &offset, const Int4 &sample, Float &lod, Float &anisotropy, Float4 &uDelta, Float4 &vDelta, bool secondLOD);
	Vector4f sampleFloat(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, const Float4 &a, const Float4 &dRef, Vector4i &offset, const Int4 &sample, Float &lod, bool secondLOD);
	Vector4f sampleFloat2D(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, const Float4 &a, const Float4 &dRef, Vector4i &offset, const Int4 &sample, Float &lod, bool secondLOD);
	Vector4f sampleFloat3D(Pointer<Byte> &texture, Float4 &u, Float4 &v, Float4 &w, const Float4 &dRef, Vector4i &offset, const Int4 &sample, Float &lod, bool secondLOD);
	void computeLod1D(Pointer<Byte> &texture, Float &lod, Float4 &u, const Float4 &dsx, const Float4 &dsy);
	void computeLod2D(Pointer<Byte> &texture, Float &lod, Float &anisotropy, Float4 &uDelta, Float4 &vDelta, Float4 &u, Float4 &v, const Float4 &dsx, const Float4 &dsy);
	void computeLodCube(Pointer<Byte> &texture, Float &lod, Float4 &u, Float4 &v, Float4 &w, const Float4 &dsx, const Float4 &dsy, Float4 &M);
	void computeLod3D(Pointer<Byte> &texture, Float &lod, Float4 &u, Float4 &v, Float4 &w, const Float4 &dsx, const Float4 &dsy);
	Int4 cubeFace(Float4 &U, Float4 &V, Float4 &x, Float4 &y, Float4 &z, Float4 &M);
	void applyOffset(Float4 &u, Float4 &v, Float4 &w, Vector4i &offset, Pointer<Byte> mipmap);
	void computeIndices(UInt index[4], Short4 uuuu, Short4 vvvv, Short4 wwww, const Short4 &cubeArrayLayer, const Int4 &sample, const Pointer<Byte> &mipmap);
	void computeIndices(UInt index[4], Int4 uuuu, Int4 vvvv, Int4 wwww, const Int4 &sample, Int4 valid, const Pointer<Byte> &mipmap);
	void bilinearInterpolateFloat(Vector4f &output, const Short4 &uuuu0, const Short4 &vvvv0, Vector4f &c00, Vector4f &c01, Vector4f &c10, Vector4f &c11, const Pointer<Byte> &mipmap, bool interpolateComponent0, bool interpolateComponent1, bool interpolateComponent2, bool interpolateComponent3);
	void bilinearInterpolate(Vector4s &output, const Short4 &uuuu0, const Short4 &vvvv0, Vector4s &c00, Vector4s &c01, Vector4s &c10, Vector4s &c11, const Pointer<Byte> &mipmap);
	void sampleLumaTexel(Vector4f& output, Short4 &u, Short4 &v, Short4 &w, const Short4 &cubeArrayLayer, const Int4 &sample, Pointer<Byte> &lumaMipmap, Pointer<Byte> lumaBuffer);
	void sampleChromaTexel(Vector4f& output, Short4 &u, Short4 &v, Short4 &w, const Short4 &cubeArrayLayer, const Int4 &sample, Pointer<Byte> &mipmapU, Pointer<Byte> bufferU, Pointer<Byte> &mipmapV, Pointer<Byte> bufferV);
	Vector4s sampleTexel(Short4 &u, Short4 &v, Short4 &w, const Short4 &cubeArrayLayer, const Int4 &sample, Pointer<Byte> &mipmap, Pointer<Byte> buffer);
	Vector4s sampleTexel(UInt index[4], Pointer<Byte> buffer);
	Vector4f sampleTexel(Int4 &u, Int4 &v, Int4 &w, const Float4 &dRef, const Int4 &sample, Pointer<Byte> &mipmap, Pointer<Byte> buffer);
	Vector4f replaceBorderTexel(const Vector4f &c, Int4 valid);
	Pointer<Byte> selectMipmap(const Pointer<Byte> &texture, const Float &lod, bool secondLOD);
	Short4 address(const Float4 &uvw, AddressingMode addressingMode);
	Short4 computeLayerIndex16(const Float4 &a, Pointer<Byte> &mipmap);
	void address(const Float4 &uvw, Int4 &xyz0, Int4 &xyz1, Float4 &f, Pointer<Byte> &mipmap, Int4 &filter, int whd, AddressingMode addressingMode);
	Int4 computeLayerIndex(const Float4 &a, Pointer<Byte> &mipmap);
	Int4 computeFilterOffset(Float &lod);
	void sRGBtoLinearFF00(Short4 &c);

	bool hasNormalizedFormat() const;
	bool hasFloatTexture() const;
	bool hasUnnormalizedIntegerTexture() const;
	bool hasUnsignedTextureComponent(int component) const;
	int textureComponentCount() const;
	bool has16bitPackedTextureFormat() const;
	bool has8bitTextureComponents() const;
	bool has16bitTextureComponents() const;
	bool has32bitIntegerTextureComponents() const;
	bool isYcbcrFormat() const;
	bool isRGBComponent(int component) const;
	bool borderModeActive() const;
	VkComponentSwizzle gatherSwizzle() const;
	sw::float4 getComponentScale() const;
	int getGatherComponent() const;

	Pointer<Byte> &constants;
	const Sampler &state;
	const SamplerFunction function;
};

}  // namespace sw

#ifdef ENABLE_RR_PRINT
namespace rr {

template<>
struct PrintValue::Ty<sw::SamplerFunction>
{
	static std::string fmt(const sw::SamplerFunction &v)
	{
		return std::string("SamplerFunction[") +
		       "method: " + std::to_string(v.method) +
		       ", offset: " + std::to_string(v.offset) +
		       ", sample: " + std::to_string(v.sample) +
		       "]";
	}

	static std::vector<rr::Value *> val(const sw::SamplerFunction &v) { return {}; }
};

}  // namespace rr
#endif  // ENABLE_RR_PRINT

#endif  // sw_SamplerCore_hpp
