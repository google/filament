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

#ifndef sw_PixelRoutine_hpp
#define sw_PixelRoutine_hpp

#include "Device/QuadRasterizer.hpp"

#include <vector>

namespace sw {

class PixelShader;
class SamplerCore;

class PixelRoutine : public sw::QuadRasterizer
{
public:
	PixelRoutine(const PixelProcessor::State &state,
	             const vk::PipelineLayout *pipelineLayout,
	             const SpirvShader *spirvShader,
	             const vk::Attachments &attachments,
	             const vk::DescriptorSet::Bindings &descriptorSets);

	virtual ~PixelRoutine();

protected:
	using SampleSet = std::vector<int>;

	SIMD::Float z[4];  // Multisampled z
	SIMD::Float w;     // Used as is
	SIMD::Float rhw;   // Reciprocal w

	SpirvRoutine routine;
	const vk::Attachments &attachments;
	const vk::DescriptorSet::Bindings &descriptorSets;

	virtual void setBuiltins(Int &x, Int &y, SIMD::Float (&z)[4], SIMD::Float &w, Int cMask[4], const SampleSet &samples) = 0;
	virtual void executeShader(Int cMask[4], Int sMask[4], Int zMask[4], const SampleSet &samples) = 0;
	virtual Bool alphaTest(Int cMask[4], const SampleSet &samples) = 0;
	virtual void blendColor(Pointer<Byte> cBuffer[4], Int &x, Int sMask[4], Int zMask[4], Int cMask[4], const SampleSet &samples) = 0;

	void quad(Pointer<Byte> cBuffer[4], Pointer<Byte> &zBuffer, Pointer<Byte> &sBuffer, Int cMask[4], Int &x, Int &y) override;

	void alphaTest(Int &aMask, const Short4 &alpha);
	void alphaToCoverage(Int cMask[4], const SIMD::Float &alpha, const SampleSet &samples);

	void writeColor(int index, const Pointer<Byte> &cBuffer, const Int &x, Vector4f &color, const Int &sMask, const Int &zMask, const Int &cMask);
	SIMD::Float4 alphaBlend(int index, const Pointer<Byte> &cBuffer, const SIMD::Float4 &sourceColor, const Int &x);

	bool isSRGB(int index) const;

private:
	bool hasStencilReplaceRef() const;
	Byte8 stencilReplaceRef();
	void stencilTest(const Pointer<Byte> &sBuffer, const Int &x, Int sMask[4], const SampleSet &samples);
	void stencilTest(Byte8 &value, VkCompareOp stencilCompareMode, bool isBack);
	Byte8 stencilOperation(const Byte8 &bufferValue, const PixelProcessor::States::StencilOpState &ops, bool isBack, const Int &zMask, const Int &sMask);
	Byte8 stencilOperation(const Byte8 &bufferValue, VkStencilOp operation, bool isBack);
	SIMD::Float clampDepth(const SIMD::Float &z);
	Bool depthTest(const Pointer<Byte> &zBuffer, int q, const Int &x, const SIMD::Float &z, const Int &sMask, Int &zMask, const Int &cMask);
	void depthBoundsTest(const Pointer<Byte> &zBuffer, int q, const Int &x, Int &zMask, Int &cMask);

	void readPixel(int index, const Pointer<Byte> &cBuffer, const Int &x, Vector4s &pixel);
	enum BlendFactorModifier
	{
		None,
		OneMinus
	};
	Float blendConstant(vk::Format format, int component, BlendFactorModifier modifier = None);
	void blendFactorRGB(SIMD::Float4 &blendFactorRGB, const SIMD::Float4 &sourceColor, const SIMD::Float4 &destColor, VkBlendFactor colorBlendFactor, vk::Format format);
	void blendFactorAlpha(SIMD::Float &blendFactorAlpha, const SIMD::Float &sourceAlpha, const SIMD::Float &destAlpha, VkBlendFactor alphaBlendFactor, vk::Format format);

	bool blendFactorCanExceedFormatRange(VkBlendFactor blendFactor, vk::Format format);
	SIMD::Float4 computeAdvancedBlendMode(int index, const SIMD::Float4 &src, const SIMD::Float4 &dst, const SIMD::Float4 &srcFactor, const SIMD::Float4 &dstFactor);
	SIMD::Float blendOpOverlay(SIMD::Float &src, SIMD::Float &dst);
	SIMD::Float blendOpColorDodge(SIMD::Float &src, SIMD::Float &dst);
	SIMD::Float blendOpColorBurn(SIMD::Float &src, SIMD::Float &dst);
	SIMD::Float blendOpHardlight(SIMD::Float &src, SIMD::Float &dst);
	SIMD::Float blendOpSoftlight(SIMD::Float &src, SIMD::Float &dst);
	void setLumSat(SIMD::Float4 &cbase, SIMD::Float4 &csat, SIMD::Float4 &clum, SIMD::Float &x, SIMD::Float &y, SIMD::Float &z);
	void setLum(SIMD::Float4 &cbase, SIMD::Float4 &clum, SIMD::Float &x, SIMD::Float &y, SIMD::Float &z);
	SIMD::Float computeLum(SIMD::Float &color, SIMD::Float &lum, SIMD::Float &mincol, SIMD::Float &maxcol, SIMD::Int &negative, SIMD::Int &aboveOne);
	SIMD::Float maxRGB(SIMD::Float4 &c);
	SIMD::Float minRGB(SIMD::Float4 &c);
	SIMD::Float lumRGB(SIMD::Float4 &c);
	void premultiply(SIMD::Float4 &c);

	void writeStencil(Pointer<Byte> &sBuffer, const Int &x, const Int sMask[4], const Int zMask[4], const Int cMask[4], const SampleSet &samples);
	void writeDepth(Pointer<Byte> &zBuffer, const Int &x, const Int zMask[4], const SampleSet &samples);
	void occlusionSampleCount(const Int zMask[4], const Int sMask[4], const SampleSet &samples);

	SIMD::Float readDepth32F(const Pointer<Byte> &zBuffer, int q, const Int &x) const;
	SIMD::Float readDepth16(const Pointer<Byte> &zBuffer, int q, const Int &x) const;

	void writeDepth32F(Pointer<Byte> &zBuffer, int q, const Int &x, const Float4 &z, const Int &zMask);
	void writeDepth16(Pointer<Byte> &zBuffer, int q, const Int &x, const Float4 &z, const Int &zMask);

	Int4 depthBoundsTest32F(const Pointer<Byte> &zBuffer, int q, const Int &x);
	Int4 depthBoundsTest16(const Pointer<Byte> &zBuffer, int q, const Int &x);

	// Derived state parameters
	const bool shaderContainsInterpolation;  // TODO(b/194714095)
	const bool perSampleShading;
	const int invocationCount;

	SampleSet getSampleSet(int invocation) const;
};

}  // namespace sw

#endif  // sw_PixelRoutine_hpp
