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

#include "PixelRoutine.hpp"

#include "Constants.hpp"
#include "SamplerCore.hpp"
#include "Device/Primitive.hpp"
#include "Device/QuadRasterizer.hpp"
#include "Device/Renderer.hpp"
#include "System/Debug.hpp"
#include "System/Math.hpp"
#include "Vulkan/VkPipelineLayout.hpp"
#include "Vulkan/VkStringify.hpp"

namespace sw {
namespace {

bool shouldUsePerSampleShading(const PixelProcessor::State &state, const SpirvShader *spirvShader)
{
	if(state.sampleShadingEnabled && (state.minSampleShading * state.multiSampleCount > 1.0f))
	{
		return true;
	}

	if(spirvShader)
	{
		if(spirvShader->getUsedCapabilities().InterpolationFunction)  // TODO(b/194714095)
		{
			return true;
		}

		if(spirvShader->getUsedCapabilities().SampleRateShading)
		{
			return true;
		}
	}

	return false;
}

}  // namespace

PixelRoutine::PixelRoutine(
    const PixelProcessor::State &state,
    const vk::PipelineLayout *pipelineLayout,
    const SpirvShader *spirvShader,
    const vk::Attachments &attachments,
    const vk::DescriptorSet::Bindings &descriptorSets)
    : QuadRasterizer(state, spirvShader)
    , routine(pipelineLayout)
    , attachments(attachments)
    , descriptorSets(descriptorSets)
    , shaderContainsInterpolation(spirvShader && spirvShader->getUsedCapabilities().InterpolationFunction)
    , perSampleShading(shouldUsePerSampleShading(state, spirvShader))
    , invocationCount(perSampleShading ? state.multiSampleCount : 1)
{
	if(spirvShader)
	{
		spirvShader->emitProlog(&routine);
	}
}

PixelRoutine::~PixelRoutine()
{
}

PixelRoutine::SampleSet PixelRoutine::getSampleSet(int invocation) const
{
	unsigned int sampleBegin = perSampleShading ? invocation : 0;
	unsigned int sampleEnd = perSampleShading ? (invocation + 1) : state.multiSampleCount;

	SampleSet samples;

	for(unsigned int q = sampleBegin; q < sampleEnd; q++)
	{
		if(state.multiSampleMask & (1 << q))
		{
			samples.push_back(q);
		}
	}

	return samples;
}

void PixelRoutine::quad(Pointer<Byte> cBuffer[MAX_COLOR_BUFFERS], Pointer<Byte> &zBuffer, Pointer<Byte> &sBuffer, Int cMask[4], Int &x, Int &y)
{
	const bool earlyFragmentTests = !spirvShader || spirvShader->getExecutionModes().EarlyFragmentTests;

	Int zMask[4];  // Depth mask
	Int sMask[4];  // Stencil mask
	SIMD::Float unclampedZ[4];

	for(int invocation = 0; invocation < invocationCount; invocation++)
	{
		SampleSet samples = getSampleSet(invocation);

		if(samples.empty())
		{
			continue;
		}

		for(unsigned int q : samples)
		{
			zMask[q] = cMask[q];
			sMask[q] = cMask[q];
		}

		stencilTest(sBuffer, x, sMask, samples);

		SIMD::Float rhwCentroid;

		// Compute the x coordinate of each fragment in the SIMD group.
		const auto xMorton = SIMD::Float([](int i) { return float(compactEvenBits(i)); });  // 0, 1, 0, 1, 2, 3, 2, 3, 0, 1, 0, 1, 2, 3, 2, 3, ...
		xFragment = SIMD::Float(Float(x)) + xMorton - SIMD::Float(*Pointer<Float>(primitive + OFFSET(Primitive, x0)));

		if(interpolateZ())
		{
			for(unsigned int q : samples)
			{
				SIMD::Float x = xFragment;

				if(state.enableMultiSampling)
				{
					x -= SIMD::Float(*Pointer<Float>(constants + OFFSET(Constants, SampleLocationsX) + q * sizeof(float)));
				}

				z[q] = interpolate(x, Dz[q], z[q], primitive + OFFSET(Primitive, z), false, false);

				if(state.depthBias)
				{
					z[q] += SIMD::Float(*Pointer<Float>(primitive + OFFSET(Primitive, zBias)));
				}

				unclampedZ[q] = z[q];
			}
		}

		Bool depthPass = false;

		if(earlyFragmentTests)
		{
			for(unsigned int q : samples)
			{
				z[q] = clampDepth(z[q]);
				depthPass = depthPass || depthTest(zBuffer, q, x, z[q], sMask[q], zMask[q], cMask[q]);
				depthBoundsTest(zBuffer, q, x, zMask[q], cMask[q]);
			}

			writeStencil(sBuffer, x, sMask, zMask, cMask, samples);
		}

		If(depthPass || !earlyFragmentTests)
		{
			if(earlyFragmentTests)
			{
				writeDepth(zBuffer, x, zMask, samples);
				occlusionSampleCount(zMask, sMask, samples);
			}

			// TODO(b/236162233): Use SIMD::Float2
			SIMD::Float xCentroid = 0.0f;
			SIMD::Float yCentroid = 0.0f;

			if(state.centroid || shaderContainsInterpolation)  // TODO(b/194714095)
			{
				SIMD::Float weight = 1.0e-9f;

				for(unsigned int q : samples)
				{
					ASSERT(SIMD::Width == 4);
					xCentroid += SIMD::Float(*Pointer<Float4>(constants + OFFSET(Constants, sampleX[q]) + 16 * cMask[q]));
					yCentroid += SIMD::Float(*Pointer<Float4>(constants + OFFSET(Constants, sampleY[q]) + 16 * cMask[q]));
					weight += SIMD::Float(*Pointer<Float4>(constants + OFFSET(Constants, weight) + 16 * cMask[q]));
				}

				weight = Rcp(weight, true /* relaxedPrecision */);
				xCentroid *= weight;
				yCentroid *= weight;

				xCentroid += xFragment;
				yCentroid += yFragment;
			}

			if(interpolateW())
			{
				w = interpolate(xFragment, Dw, rhw, primitive + OFFSET(Primitive, w), false, false);
				rhw = reciprocal(w, false, true);

				if(state.centroid || shaderContainsInterpolation)  // TODO(b/194714095)
				{
					rhwCentroid = reciprocal(SpirvRoutine::interpolateAtXY(xCentroid, yCentroid, rhwCentroid, primitive + OFFSET(Primitive, w), SpirvRoutine::Linear));
				}
			}

			if(spirvShader)
			{
				if(shaderContainsInterpolation)  // TODO(b/194714095)
				{
					routine.interpolationData.primitive = primitive;

					routine.interpolationData.x = xFragment;
					routine.interpolationData.y = yFragment;
					routine.interpolationData.rhw = rhw;

					routine.interpolationData.xCentroid = xCentroid;
					routine.interpolationData.yCentroid = yCentroid;
					routine.interpolationData.rhwCentroid = rhwCentroid;
				}

				SIMD::Float xSample = xFragment;
				SIMD::Float ySample = yFragment;

				if(perSampleShading && (state.multiSampleCount > 1))
				{
					xSample += SampleLocationsX[samples[0]];
					ySample += SampleLocationsY[samples[0]];
				}

				int packedInterpolant = 0;
				for(int interfaceInterpolant = 0; interfaceInterpolant < MAX_INTERFACE_COMPONENTS; interfaceInterpolant++)
				{
					const auto &input = spirvShader->inputs[interfaceInterpolant];
					if(input.Type != Spirv::ATTRIBTYPE_UNUSED)
					{
						routine.inputsInterpolation[packedInterpolant] = input.Flat ? SpirvRoutine::Flat : (input.NoPerspective ? SpirvRoutine::Linear : SpirvRoutine::Perspective);
						if(input.Centroid && state.enableMultiSampling)
						{
							routine.inputs[interfaceInterpolant] =
							    SpirvRoutine::interpolateAtXY(xCentroid, yCentroid, rhwCentroid,
							                                  primitive + OFFSET(Primitive, V[packedInterpolant]),
							                                  routine.inputsInterpolation[packedInterpolant]);
						}
						else if(perSampleShading)
						{
							routine.inputs[interfaceInterpolant] =
							    SpirvRoutine::interpolateAtXY(xSample, ySample, rhw,
							                                  primitive + OFFSET(Primitive, V[packedInterpolant]),
							                                  routine.inputsInterpolation[packedInterpolant]);
						}
						else
						{
							routine.inputs[interfaceInterpolant] =
							    interpolate(xFragment, Dv[interfaceInterpolant], rhw,
							                primitive + OFFSET(Primitive, V[packedInterpolant]),
							                input.Flat, !input.NoPerspective);
						}
						packedInterpolant++;
					}
				}

				setBuiltins(x, y, unclampedZ, w, cMask, samples);

				for(uint32_t i = 0; i < state.numClipDistances; i++)
				{
					auto distance = interpolate(xFragment, DclipDistance[i], rhw,
					                            primitive + OFFSET(Primitive, clipDistance[i]),
					                            false, true);

					auto clipMask = SignMask(CmpGE(distance, SIMD::Float(0)));
					for(unsigned int q : samples)
					{
						// FIXME(b/148105887): Fragments discarded by clipping do not exist at
						// all -- they should not be counted in queries or have their Z/S effects
						// performed when early fragment tests are enabled.
						cMask[q] &= clipMask;
					}

					if(spirvShader->getUsedCapabilities().ClipDistance)
					{
						auto it = spirvShader->inputBuiltins.find(spv::BuiltInClipDistance);
						if(it != spirvShader->inputBuiltins.end())
						{
							if(i < it->second.SizeInComponents)
							{
								routine.getVariable(it->second.Id)[it->second.FirstComponent + i] = distance;
							}
						}
					}
				}

				if(spirvShader->getUsedCapabilities().CullDistance)
				{
					auto it = spirvShader->inputBuiltins.find(spv::BuiltInCullDistance);
					if(it != spirvShader->inputBuiltins.end())
					{
						for(uint32_t i = 0; i < state.numCullDistances; i++)
						{
							if(i < it->second.SizeInComponents)
							{
								routine.getVariable(it->second.Id)[it->second.FirstComponent + i] =
								    interpolate(xFragment, DcullDistance[i], rhw,
								                primitive + OFFSET(Primitive, cullDistance[i]),
								                false, true);
							}
						}
					}
				}
			}

			if(spirvShader)
			{
				executeShader(cMask, earlyFragmentTests ? sMask : cMask, earlyFragmentTests ? zMask : cMask, samples);
			}

			Bool alphaPass = alphaTest(cMask, samples);

			if((spirvShader && spirvShader->coverageModified()) || state.alphaToCoverage)
			{
				for(unsigned int q : samples)
				{
					zMask[q] &= cMask[q];
					sMask[q] &= cMask[q];
				}
			}

			If(alphaPass)
			{
				if(!earlyFragmentTests)
				{
					for(unsigned int q : samples)
					{
						z[q] = clampDepth(z[q]);
						depthPass = depthPass || depthTest(zBuffer, q, x, z[q], sMask[q], zMask[q], cMask[q]);
						depthBoundsTest(zBuffer, q, x, zMask[q], cMask[q]);
					}
				}

				If(depthPass)
				{
					if(!earlyFragmentTests)
					{
						writeDepth(zBuffer, x, zMask, samples);
						occlusionSampleCount(zMask, sMask, samples);
					}

					blendColor(cBuffer, x, sMask, zMask, cMask, samples);
				}
			}
		}

		if(!earlyFragmentTests)
		{
			writeStencil(sBuffer, x, sMask, zMask, cMask, samples);
		}
	}
}

void PixelRoutine::stencilTest(const Pointer<Byte> &sBuffer, const Int &x, Int sMask[4], const SampleSet &samples)
{
	if(!state.stencilActive)
	{
		return;
	}

	for(unsigned int q : samples)
	{
		// (StencilRef & StencilMask) CompFunc (StencilBufferValue & StencilMask)

		Pointer<Byte> buffer = sBuffer + x;

		if(q > 0)
		{
			buffer += q * *Pointer<Int>(data + OFFSET(DrawData, stencilSliceB));
		}

		Int pitch = *Pointer<Int>(data + OFFSET(DrawData, stencilPitchB));
		Byte8 value = *Pointer<Byte8>(buffer) & Byte8(-1, -1, 0, 0, 0, 0, 0, 0);
		value = value | (*Pointer<Byte8>(buffer + pitch - 2) & Byte8(0, 0, -1, -1, 0, 0, 0, 0));
		Byte8 valueBack = value;

		if(state.frontStencil.useCompareMask)
		{
			value &= *Pointer<Byte8>(data + OFFSET(DrawData, stencil[0].testMaskQ));
		}

		stencilTest(value, state.frontStencil.compareOp, false);

		if(state.backStencil.useCompareMask)
		{
			valueBack &= *Pointer<Byte8>(data + OFFSET(DrawData, stencil[1].testMaskQ));
		}

		stencilTest(valueBack, state.backStencil.compareOp, true);

		value &= *Pointer<Byte8>(primitive + OFFSET(Primitive, clockwiseMask));
		valueBack &= *Pointer<Byte8>(primitive + OFFSET(Primitive, invClockwiseMask));
		value |= valueBack;

		sMask[q] &= SignMask(value);
	}
}

void PixelRoutine::stencilTest(Byte8 &value, VkCompareOp stencilCompareMode, bool isBack)
{
	Byte8 equal;

	switch(stencilCompareMode)
	{
	case VK_COMPARE_OP_ALWAYS:
		value = Byte8(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
		break;
	case VK_COMPARE_OP_NEVER:
		value = Byte8(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
		break;
	case VK_COMPARE_OP_LESS:  // a < b ~ b > a
		value += Byte8(0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80);
		value = CmpGT(As<SByte8>(value), *Pointer<SByte8>(data + OFFSET(DrawData, stencil[isBack].referenceMaskedSignedQ)));
		break;
	case VK_COMPARE_OP_EQUAL:
		value = CmpEQ(value, *Pointer<Byte8>(data + OFFSET(DrawData, stencil[isBack].referenceMaskedQ)));
		break;
	case VK_COMPARE_OP_NOT_EQUAL:  // a != b ~ !(a == b)
		value = CmpEQ(value, *Pointer<Byte8>(data + OFFSET(DrawData, stencil[isBack].referenceMaskedQ)));
		value ^= Byte8(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
		break;
	case VK_COMPARE_OP_LESS_OR_EQUAL:  // a <= b ~ (b > a) || (a == b)
		equal = value;
		equal = CmpEQ(equal, *Pointer<Byte8>(data + OFFSET(DrawData, stencil[isBack].referenceMaskedQ)));
		value += Byte8(0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80);
		value = CmpGT(As<SByte8>(value), *Pointer<SByte8>(data + OFFSET(DrawData, stencil[isBack].referenceMaskedSignedQ)));
		value |= equal;
		break;
	case VK_COMPARE_OP_GREATER:  // a > b
		equal = *Pointer<Byte8>(data + OFFSET(DrawData, stencil[isBack].referenceMaskedSignedQ));
		value += Byte8(0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80);
		equal = CmpGT(As<SByte8>(equal), As<SByte8>(value));
		value = equal;
		break;
	case VK_COMPARE_OP_GREATER_OR_EQUAL:  // a >= b ~ !(a < b) ~ !(b > a)
		value += Byte8(0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80);
		value = CmpGT(As<SByte8>(value), *Pointer<SByte8>(data + OFFSET(DrawData, stencil[isBack].referenceMaskedSignedQ)));
		value ^= Byte8(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
		break;
	default:
		UNSUPPORTED("VkCompareOp: %d", int(stencilCompareMode));
	}
}

SIMD::Float PixelRoutine::readDepth32F(const Pointer<Byte> &zBuffer, int q, const Int &x) const
{
	ASSERT(SIMD::Width == 4);
	Pointer<Byte> buffer = zBuffer + 4 * x;
	Int pitch = *Pointer<Int>(data + OFFSET(DrawData, depthPitchB));

	if(q > 0)
	{
		buffer += q * *Pointer<Int>(data + OFFSET(DrawData, depthSliceB));
	}

	Float4 zValue = Float4(*Pointer<Float2>(buffer), *Pointer<Float2>(buffer + pitch));
	return SIMD::Float(zValue);
}

SIMD::Float PixelRoutine::readDepth16(const Pointer<Byte> &zBuffer, int q, const Int &x) const
{
	ASSERT(SIMD::Width == 4);
	Pointer<Byte> buffer = zBuffer + 2 * x;
	Int pitch = *Pointer<Int>(data + OFFSET(DrawData, depthPitchB));

	if(q > 0)
	{
		buffer += q * *Pointer<Int>(data + OFFSET(DrawData, depthSliceB));
	}

	UShort4 zValue16;
	zValue16 = As<UShort4>(Insert(As<Int2>(zValue16), *Pointer<Int>(buffer), 0));
	zValue16 = As<UShort4>(Insert(As<Int2>(zValue16), *Pointer<Int>(buffer + pitch), 1));
	Float4 zValue = Float4(zValue16);
	return SIMD::Float(zValue);
}

SIMD::Float PixelRoutine::clampDepth(const SIMD::Float &z)
{
	if(!state.depthClamp)
	{
		return z;
	}

	return Min(Max(z, state.minDepthClamp), state.maxDepthClamp);
}

Bool PixelRoutine::depthTest(const Pointer<Byte> &zBuffer, int q, const Int &x, const SIMD::Float &z, const Int &sMask, Int &zMask, const Int &cMask)
{
	if(!state.depthTestActive)
	{
		return true;
	}

	SIMD::Float Z;
	SIMD::Float zValue;

	if(state.depthCompareMode != VK_COMPARE_OP_NEVER || (state.depthCompareMode != VK_COMPARE_OP_ALWAYS && !state.depthWriteEnable))
	{
		switch(state.depthFormat)
		{
		case VK_FORMAT_D16_UNORM:
			Z = Min(Max(Round(z * 0xFFFF), 0.0f), 0xFFFF);
			zValue = readDepth16(zBuffer, q, x);
			break;
		case VK_FORMAT_D32_SFLOAT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			Z = z;
			zValue = readDepth32F(zBuffer, q, x);
			break;
		default:
			UNSUPPORTED("Depth format: %d", int(state.depthFormat));
			return false;
		}
	}

	SIMD::Int zTest;

	switch(state.depthCompareMode)
	{
	case VK_COMPARE_OP_ALWAYS:
		// Optimized
		break;
	case VK_COMPARE_OP_NEVER:
		// Optimized
		break;
	case VK_COMPARE_OP_EQUAL:
		zTest = CmpEQ(zValue, Z);
		break;
	case VK_COMPARE_OP_NOT_EQUAL:
		zTest = CmpNEQ(zValue, Z);
		break;
	case VK_COMPARE_OP_LESS:
		zTest = CmpNLE(zValue, Z);
		break;
	case VK_COMPARE_OP_GREATER_OR_EQUAL:
		zTest = CmpLE(zValue, Z);
		break;
	case VK_COMPARE_OP_LESS_OR_EQUAL:
		zTest = CmpNLT(zValue, Z);
		break;
	case VK_COMPARE_OP_GREATER:
		zTest = CmpLT(zValue, Z);
		break;
	default:
		UNSUPPORTED("VkCompareOp: %d", int(state.depthCompareMode));
	}

	switch(state.depthCompareMode)
	{
	case VK_COMPARE_OP_ALWAYS:
		zMask = cMask;
		break;
	case VK_COMPARE_OP_NEVER:
		zMask = 0x0;
		break;
	default:
		zMask = SignMask(zTest) & cMask;
		break;
	}

	if(state.stencilActive)
	{
		zMask &= sMask;
	}

	return zMask != 0;
}

Int4 PixelRoutine::depthBoundsTest16(const Pointer<Byte> &zBuffer, int q, const Int &x)
{
	Pointer<Byte> buffer = zBuffer + 2 * x;
	Int pitch = *Pointer<Int>(data + OFFSET(DrawData, depthPitchB));

	if(q > 0)
	{
		buffer += q * *Pointer<Int>(data + OFFSET(DrawData, depthSliceB));
	}

	Float4 minDepthBound(state.minDepthBounds);
	Float4 maxDepthBound(state.maxDepthBounds);

	Int2 z;
	z = Insert(z, *Pointer<Int>(buffer), 0);
	z = Insert(z, *Pointer<Int>(buffer + pitch), 1);

	Float4 zValue = Float4(As<UShort4>(z)) * (1.0f / 0xFFFF);
	return Int4(CmpLE(minDepthBound, zValue) & CmpLE(zValue, maxDepthBound));
}

Int4 PixelRoutine::depthBoundsTest32F(const Pointer<Byte> &zBuffer, int q, const Int &x)
{
	Pointer<Byte> buffer = zBuffer + 4 * x;
	Int pitch = *Pointer<Int>(data + OFFSET(DrawData, depthPitchB));

	if(q > 0)
	{
		buffer += q * *Pointer<Int>(data + OFFSET(DrawData, depthSliceB));
	}

	Float4 zValue = Float4(*Pointer<Float2>(buffer), *Pointer<Float2>(buffer + pitch));
	return Int4(CmpLE(state.minDepthBounds, zValue) & CmpLE(zValue, state.maxDepthBounds));
}

void PixelRoutine::depthBoundsTest(const Pointer<Byte> &zBuffer, int q, const Int &x, Int &zMask, Int &cMask)
{
	if(!state.depthBoundsTestActive)
	{
		return;
	}

	Int4 zTest;
	switch(state.depthFormat)
	{
	case VK_FORMAT_D16_UNORM:
		zTest = depthBoundsTest16(zBuffer, q, x);
		break;
	case VK_FORMAT_D32_SFLOAT:
	case VK_FORMAT_D32_SFLOAT_S8_UINT:
		zTest = depthBoundsTest32F(zBuffer, q, x);
		break;
	default:
		UNSUPPORTED("Depth format: %d", int(state.depthFormat));
		break;
	}

	if(!state.depthTestActive)
	{
		cMask &= zMask & SignMask(zTest);
	}
	else
	{
		zMask &= cMask & SignMask(zTest);
	}
}

void PixelRoutine::alphaToCoverage(Int cMask[4], const SIMD::Float &alpha, const SampleSet &samples)
{
	static const int a2c[4] = {
		OFFSET(DrawData, a2c0),
		OFFSET(DrawData, a2c1),
		OFFSET(DrawData, a2c2),
		OFFSET(DrawData, a2c3),
	};

	for(unsigned int q : samples)
	{
		SIMD::Int coverage = CmpNLT(alpha, SIMD::Float(*Pointer<Float>(data + a2c[q])));
		Int aMask = SignMask(coverage);
		cMask[q] &= aMask;
	}
}

void PixelRoutine::writeDepth32F(Pointer<Byte> &zBuffer, int q, const Int &x, const Float4 &z, const Int &zMask)
{
	Float4 Z = z;

	Pointer<Byte> buffer = zBuffer + 4 * x;
	Int pitch = *Pointer<Int>(data + OFFSET(DrawData, depthPitchB));

	if(q > 0)
	{
		buffer += q * *Pointer<Int>(data + OFFSET(DrawData, depthSliceB));
	}

	Float4 zValue;

	if(state.depthCompareMode != VK_COMPARE_OP_NEVER || (state.depthCompareMode != VK_COMPARE_OP_ALWAYS && !state.depthWriteEnable))
	{
		zValue = Float4(*Pointer<Float2>(buffer), *Pointer<Float2>(buffer + pitch));
	}

	Z = As<Float4>(As<Int4>(Z) & *Pointer<Int4>(constants + OFFSET(Constants, maskD4X) + zMask * 16, 16));
	zValue = As<Float4>(As<Int4>(zValue) & *Pointer<Int4>(constants + OFFSET(Constants, invMaskD4X) + zMask * 16, 16));
	Z = As<Float4>(As<Int4>(Z) | As<Int4>(zValue));

	*Pointer<Float2>(buffer) = Float2(Z.xy);
	*Pointer<Float2>(buffer + pitch) = Float2(Z.zw);
}

void PixelRoutine::writeDepth16(Pointer<Byte> &zBuffer, int q, const Int &x, const Float4 &z, const Int &zMask)
{
	Short4 Z = UShort4(Round(z * 0xFFFF), true);

	Pointer<Byte> buffer = zBuffer + 2 * x;
	Int pitch = *Pointer<Int>(data + OFFSET(DrawData, depthPitchB));

	if(q > 0)
	{
		buffer += q * *Pointer<Int>(data + OFFSET(DrawData, depthSliceB));
	}

	Short4 zValue;

	if(state.depthCompareMode != VK_COMPARE_OP_NEVER || (state.depthCompareMode != VK_COMPARE_OP_ALWAYS && !state.depthWriteEnable))
	{
		zValue = As<Short4>(Insert(As<Int2>(zValue), *Pointer<Int>(buffer), 0));
		zValue = As<Short4>(Insert(As<Int2>(zValue), *Pointer<Int>(buffer + pitch), 1));
	}

	Z = Z & *Pointer<Short4>(constants + OFFSET(Constants, maskW4Q) + zMask * 8, 8);
	zValue = zValue & *Pointer<Short4>(constants + OFFSET(Constants, invMaskW4Q) + zMask * 8, 8);
	Z = Z | zValue;

	*Pointer<Int>(buffer) = Extract(As<Int2>(Z), 0);
	*Pointer<Int>(buffer + pitch) = Extract(As<Int2>(Z), 1);
}

void PixelRoutine::writeDepth(Pointer<Byte> &zBuffer, const Int &x, const Int zMask[4], const SampleSet &samples)
{
	if(!state.depthWriteEnable)
	{
		return;
	}

	for(unsigned int q : samples)
	{
		ASSERT(SIMD::Width == 4);
		switch(state.depthFormat)
		{
		case VK_FORMAT_D16_UNORM:
			writeDepth16(zBuffer, q, x, Extract128(z[q], 0), zMask[q]);
			break;
		case VK_FORMAT_D32_SFLOAT:
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			writeDepth32F(zBuffer, q, x, Extract128(z[q], 0), zMask[q]);
			break;
		default:
			UNSUPPORTED("Depth format: %d", int(state.depthFormat));
			break;
		}
	}
}

void PixelRoutine::occlusionSampleCount(const Int zMask[4], const Int sMask[4], const SampleSet &samples)
{
	if(!state.occlusionEnabled)
	{
		return;
	}

	for(unsigned int q : samples)
	{
		occlusion += *Pointer<UInt>(constants + OFFSET(Constants, occlusionCount) + 4 * (zMask[q] & sMask[q]));
	}
}

void PixelRoutine::writeStencil(Pointer<Byte> &sBuffer, const Int &x, const Int sMask[4], const Int zMask[4], const Int cMask[4], const SampleSet &samples)
{
	if(!state.stencilActive)
	{
		return;
	}

	if(state.frontStencil.passOp == VK_STENCIL_OP_KEEP && state.frontStencil.depthFailOp == VK_STENCIL_OP_KEEP && state.frontStencil.failOp == VK_STENCIL_OP_KEEP)
	{
		if(state.backStencil.passOp == VK_STENCIL_OP_KEEP && state.backStencil.depthFailOp == VK_STENCIL_OP_KEEP && state.backStencil.failOp == VK_STENCIL_OP_KEEP)
		{
			return;
		}
	}

	if(!state.frontStencil.writeEnabled && !state.backStencil.writeEnabled)
	{
		return;
	}

	for(unsigned int q : samples)
	{
		Pointer<Byte> buffer = sBuffer + x;

		if(q > 0)
		{
			buffer += q * *Pointer<Int>(data + OFFSET(DrawData, stencilSliceB));
		}

		Int pitch = *Pointer<Int>(data + OFFSET(DrawData, stencilPitchB));
		Byte8 bufferValue = *Pointer<Byte8>(buffer) & Byte8(-1, -1, 0, 0, 0, 0, 0, 0);
		bufferValue = bufferValue | (*Pointer<Byte8>(buffer + pitch - 2) & Byte8(0, 0, -1, -1, 0, 0, 0, 0));
		Byte8 newValue = stencilOperation(bufferValue, state.frontStencil, false, zMask[q], sMask[q]);

		if(state.frontStencil.useWriteMask)  // Assume 8-bit stencil buffer
		{
			Byte8 maskedValue = bufferValue;
			newValue &= *Pointer<Byte8>(data + OFFSET(DrawData, stencil[0].writeMaskQ));
			maskedValue &= *Pointer<Byte8>(data + OFFSET(DrawData, stencil[0].invWriteMaskQ));
			newValue |= maskedValue;
		}

		Byte8 newValueBack = stencilOperation(bufferValue, state.backStencil, true, zMask[q], sMask[q]);

		if(state.backStencil.useWriteMask)  // Assume 8-bit stencil buffer
		{
			Byte8 maskedValue = bufferValue;
			newValueBack &= *Pointer<Byte8>(data + OFFSET(DrawData, stencil[1].writeMaskQ));
			maskedValue &= *Pointer<Byte8>(data + OFFSET(DrawData, stencil[1].invWriteMaskQ));
			newValueBack |= maskedValue;
		}

		newValue &= *Pointer<Byte8>(primitive + OFFSET(Primitive, clockwiseMask));
		newValueBack &= *Pointer<Byte8>(primitive + OFFSET(Primitive, invClockwiseMask));
		newValue |= newValueBack;

		newValue &= *Pointer<Byte8>(constants + OFFSET(Constants, maskB4Q) + 8 * cMask[q]);
		bufferValue &= *Pointer<Byte8>(constants + OFFSET(Constants, invMaskB4Q) + 8 * cMask[q]);
		newValue |= bufferValue;

		*Pointer<Short>(buffer) = Extract(As<Short4>(newValue), 0);
		*Pointer<Short>(buffer + pitch) = Extract(As<Short4>(newValue), 1);
	}
}

Byte8 PixelRoutine::stencilOperation(const Byte8 &bufferValue, const PixelProcessor::States::StencilOpState &ops, bool isBack, const Int &zMask, const Int &sMask)
{
	Byte8 pass = stencilOperation(bufferValue, ops.passOp, isBack);

	if(state.depthTestActive && ops.depthFailOp != ops.passOp)  // zMask valid and values not the same
	{
		Byte8 zFail = stencilOperation(bufferValue, ops.depthFailOp, isBack);

		pass &= *Pointer<Byte8>(constants + OFFSET(Constants, maskB4Q) + 8 * zMask);
		zFail &= *Pointer<Byte8>(constants + OFFSET(Constants, invMaskB4Q) + 8 * zMask);
		pass |= zFail;
	}

	if(ops.failOp != ops.passOp || (state.depthTestActive && ops.failOp != ops.depthFailOp))
	{
		Byte8 fail = stencilOperation(bufferValue, ops.failOp, isBack);

		pass &= *Pointer<Byte8>(constants + OFFSET(Constants, maskB4Q) + 8 * sMask);
		fail &= *Pointer<Byte8>(constants + OFFSET(Constants, invMaskB4Q) + 8 * sMask);
		pass |= fail;
	}

	return pass;
}

bool PixelRoutine::hasStencilReplaceRef() const
{
	return spirvShader &&
	       (spirvShader->outputBuiltins.find(spv::BuiltInFragStencilRefEXT) !=
	        spirvShader->outputBuiltins.end());
}

Byte8 PixelRoutine::stencilReplaceRef()
{
	ASSERT(spirvShader);

	auto it = spirvShader->outputBuiltins.find(spv::BuiltInFragStencilRefEXT);
	ASSERT(it != spirvShader->outputBuiltins.end());

	UInt4 sRef = As<UInt4>(routine.getVariable(it->second.Id)[it->second.FirstComponent]) & UInt4(0xff);
	// TODO (b/148295813): Could be done with a single pshufb instruction. Optimize the
	//                     following line by either adding a rr::Shuffle() variant to do
	//                     it explicitly or adding a Byte4(Int4) constructor would work.
	sRef.x = rr::UInt(sRef.x) | (rr::UInt(sRef.y) << 8) | (rr::UInt(sRef.z) << 16) | (rr::UInt(sRef.w) << 24);

	UInt2 sRefDuplicated;
	sRefDuplicated = Insert(sRefDuplicated, sRef.x, 0);
	sRefDuplicated = Insert(sRefDuplicated, sRef.x, 1);
	return As<Byte8>(sRefDuplicated);
}

Byte8 PixelRoutine::stencilOperation(const Byte8 &bufferValue, VkStencilOp operation, bool isBack)
{
	if(hasStencilReplaceRef())
	{
		return stencilReplaceRef();
	}
	else
	{
		switch(operation)
		{
		case VK_STENCIL_OP_KEEP:
			return bufferValue;
		case VK_STENCIL_OP_ZERO:
			return Byte8(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
		case VK_STENCIL_OP_REPLACE:
			return *Pointer<Byte8>(data + OFFSET(DrawData, stencil[isBack].referenceQ));
		case VK_STENCIL_OP_INCREMENT_AND_CLAMP:
			return AddSat(bufferValue, Byte8(1, 1, 1, 1, 1, 1, 1, 1));
		case VK_STENCIL_OP_DECREMENT_AND_CLAMP:
			return SubSat(bufferValue, Byte8(1, 1, 1, 1, 1, 1, 1, 1));
		case VK_STENCIL_OP_INVERT:
			return bufferValue ^ Byte8(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
		case VK_STENCIL_OP_INCREMENT_AND_WRAP:
			return bufferValue + Byte8(1, 1, 1, 1, 1, 1, 1, 1);
		case VK_STENCIL_OP_DECREMENT_AND_WRAP:
			return bufferValue - Byte8(1, 1, 1, 1, 1, 1, 1, 1);
		default:
			UNSUPPORTED("VkStencilOp: %d", int(operation));
		}
	}

	return Byte8(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
}

bool PixelRoutine::isSRGB(int index) const
{
	return vk::Format(state.colorFormat[index]).isSRGBformat();
}

void PixelRoutine::readPixel(int index, const Pointer<Byte> &cBuffer, const Int &x, Vector4s &pixel)
{
	Short4 c01;
	Short4 c23;
	Pointer<Byte> buffer = cBuffer;
	Pointer<Byte> buffer2;

	Int pitchB = *Pointer<Int>(data + OFFSET(DrawData, colorPitchB[index]));

	vk::Format format = state.colorFormat[index];
	switch(format)
	{
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
		buffer += 2 * x;
		buffer2 = buffer + pitchB;
		c01 = As<Short4>(Int2(*Pointer<Int>(buffer), *Pointer<Int>(buffer2)));

		pixel.x = (c01 & Short4(0xF000u));
		pixel.y = (c01 & Short4(0x0F00u)) << 4;
		pixel.z = (c01 & Short4(0x00F0u)) << 8;
		pixel.w = (c01 & Short4(0x000Fu)) << 12;

		// Expand to 16 bit range
		pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 4);
		pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 8);
		pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 4);
		pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 8);
		pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 4);
		pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 8);
		pixel.w |= As<Short4>(As<UShort4>(pixel.w) >> 4);
		pixel.w |= As<Short4>(As<UShort4>(pixel.w) >> 8);
		break;
	case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
		buffer += 2 * x;
		buffer2 = buffer + pitchB;
		c01 = As<Short4>(Int2(*Pointer<Int>(buffer), *Pointer<Int>(buffer2)));

		pixel.z = (c01 & Short4(0xF000u));
		pixel.y = (c01 & Short4(0x0F00u)) << 4;
		pixel.x = (c01 & Short4(0x00F0u)) << 8;
		pixel.w = (c01 & Short4(0x000Fu)) << 12;

		// Expand to 16 bit range
		pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 4);
		pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 8);
		pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 4);
		pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 8);
		pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 4);
		pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 8);
		pixel.w |= As<Short4>(As<UShort4>(pixel.w) >> 4);
		pixel.w |= As<Short4>(As<UShort4>(pixel.w) >> 8);
		break;
	case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
		buffer += 2 * x;
		buffer2 = buffer + pitchB;
		c01 = As<Short4>(Int2(*Pointer<Int>(buffer), *Pointer<Int>(buffer2)));

		pixel.w = (c01 & Short4(0xF000u));
		pixel.z = (c01 & Short4(0x0F00u)) << 4;
		pixel.y = (c01 & Short4(0x00F0u)) << 8;
		pixel.x = (c01 & Short4(0x000Fu)) << 12;

		// Expand to 16 bit range
		pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 4);
		pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 8);
		pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 4);
		pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 8);
		pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 4);
		pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 8);
		pixel.w |= As<Short4>(As<UShort4>(pixel.w) >> 4);
		pixel.w |= As<Short4>(As<UShort4>(pixel.w) >> 8);
		break;
	case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
		buffer += 2 * x;
		buffer2 = buffer + pitchB;
		c01 = As<Short4>(Int2(*Pointer<Int>(buffer), *Pointer<Int>(buffer2)));

		pixel.w = (c01 & Short4(0xF000u));
		pixel.x = (c01 & Short4(0x0F00u)) << 4;
		pixel.y = (c01 & Short4(0x00F0u)) << 8;
		pixel.z = (c01 & Short4(0x000Fu)) << 12;

		// Expand to 16 bit range
		pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 4);
		pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 8);
		pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 4);
		pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 8);
		pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 4);
		pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 8);
		pixel.w |= As<Short4>(As<UShort4>(pixel.w) >> 4);
		pixel.w |= As<Short4>(As<UShort4>(pixel.w) >> 8);
		break;
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
		buffer += 2 * x;
		buffer2 = buffer + pitchB;
		c01 = As<Short4>(Int2(*Pointer<Int>(buffer), *Pointer<Int>(buffer2)));

		pixel.x = (c01 & Short4(0xF800u));
		pixel.y = (c01 & Short4(0x07C0u)) << 5;
		pixel.z = (c01 & Short4(0x003Eu)) << 10;
		pixel.w = ((c01 & Short4(0x0001u)) << 15) >> 15;

		// Expand to 16 bit range
		pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 5);
		pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 10);
		pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 5);
		pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 10);
		pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 5);
		pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 10);
		break;
	case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
		buffer += 2 * x;
		buffer2 = buffer + pitchB;
		c01 = As<Short4>(Int2(*Pointer<Int>(buffer), *Pointer<Int>(buffer2)));

		pixel.z = (c01 & Short4(0xF800u));
		pixel.y = (c01 & Short4(0x07C0u)) << 5;
		pixel.x = (c01 & Short4(0x003Eu)) << 10;
		pixel.w = ((c01 & Short4(0x0001u)) << 15) >> 15;

		// Expand to 16 bit range
		pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 5);
		pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 10);
		pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 5);
		pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 10);
		pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 5);
		pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 10);
		break;
	case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
		buffer += 2 * x;
		buffer2 = buffer + pitchB;
		c01 = As<Short4>(Int2(*Pointer<Int>(buffer), *Pointer<Int>(buffer2)));

		pixel.x = (c01 & Short4(0x7C00u)) << 1;
		pixel.y = (c01 & Short4(0x03E0u)) << 6;
		pixel.z = (c01 & Short4(0x001Fu)) << 11;
		pixel.w = (c01 & Short4(0x8000u)) >> 15;

		// Expand to 16 bit range
		pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 5);
		pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 10);
		pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 5);
		pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 10);
		pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 5);
		pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 10);
		break;
	case VK_FORMAT_R5G6B5_UNORM_PACK16:
		buffer += 2 * x;
		buffer2 = buffer + pitchB;
		c01 = As<Short4>(Int2(*Pointer<Int>(buffer), *Pointer<Int>(buffer2)));

		pixel.x = c01 & Short4(0xF800u);
		pixel.y = (c01 & Short4(0x07E0u)) << 5;
		pixel.z = (c01 & Short4(0x001Fu)) << 11;
		pixel.w = Short4(0xFFFFu);

		// Expand to 16 bit range
		pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 5);
		pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 10);
		pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 6);
		pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 12);
		pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 5);
		pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 10);
		break;
	case VK_FORMAT_B5G6R5_UNORM_PACK16:
		buffer += 2 * x;
		buffer2 = buffer + pitchB;
		c01 = As<Short4>(Int2(*Pointer<Int>(buffer), *Pointer<Int>(buffer2)));

		pixel.z = c01 & Short4(0xF800u);
		pixel.y = (c01 & Short4(0x07E0u)) << 5;
		pixel.x = (c01 & Short4(0x001Fu)) << 11;
		pixel.w = Short4(0xFFFFu);

		// Expand to 16 bit range
		pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 5);
		pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 10);
		pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 6);
		pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 12);
		pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 5);
		pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 10);
		break;
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_B8G8R8A8_SRGB:
		buffer += 4 * x;
		c01 = *Pointer<Short4>(buffer);
		buffer += pitchB;
		c23 = *Pointer<Short4>(buffer);
		pixel.z = c01;
		pixel.y = c01;
		pixel.z = UnpackLow(As<Byte8>(pixel.z), As<Byte8>(c23));
		pixel.y = UnpackHigh(As<Byte8>(pixel.y), As<Byte8>(c23));
		pixel.x = pixel.z;
		pixel.z = UnpackLow(As<Byte8>(pixel.z), As<Byte8>(pixel.y));
		pixel.x = UnpackHigh(As<Byte8>(pixel.x), As<Byte8>(pixel.y));
		pixel.y = pixel.z;
		pixel.w = pixel.x;
		pixel.x = UnpackLow(As<Byte8>(pixel.x), As<Byte8>(pixel.x));
		pixel.y = UnpackHigh(As<Byte8>(pixel.y), As<Byte8>(pixel.y));
		pixel.z = UnpackLow(As<Byte8>(pixel.z), As<Byte8>(pixel.z));
		pixel.w = UnpackHigh(As<Byte8>(pixel.w), As<Byte8>(pixel.w));
		break;
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SRGB:
		buffer += 4 * x;
		c01 = *Pointer<Short4>(buffer);
		buffer += pitchB;
		c23 = *Pointer<Short4>(buffer);
		pixel.z = c01;
		pixel.y = c01;
		pixel.z = UnpackLow(As<Byte8>(pixel.z), As<Byte8>(c23));
		pixel.y = UnpackHigh(As<Byte8>(pixel.y), As<Byte8>(c23));
		pixel.x = pixel.z;
		pixel.z = UnpackLow(As<Byte8>(pixel.z), As<Byte8>(pixel.y));
		pixel.x = UnpackHigh(As<Byte8>(pixel.x), As<Byte8>(pixel.y));
		pixel.y = pixel.z;
		pixel.w = pixel.x;
		pixel.x = UnpackLow(As<Byte8>(pixel.z), As<Byte8>(pixel.z));
		pixel.y = UnpackHigh(As<Byte8>(pixel.y), As<Byte8>(pixel.y));
		pixel.z = UnpackLow(As<Byte8>(pixel.w), As<Byte8>(pixel.w));
		pixel.w = UnpackHigh(As<Byte8>(pixel.w), As<Byte8>(pixel.w));
		break;
	case VK_FORMAT_R8_UNORM:
		buffer += 1 * x;
		pixel.x = Insert(pixel.x, *Pointer<Short>(buffer), 0);
		buffer += pitchB;
		pixel.x = Insert(pixel.x, *Pointer<Short>(buffer), 1);
		pixel.x = UnpackLow(As<Byte8>(pixel.x), As<Byte8>(pixel.x));
		pixel.y = Short4(0x0000);
		pixel.z = Short4(0x0000);
		pixel.w = Short4(0xFFFFu);
		break;
	case VK_FORMAT_R8G8_UNORM:
		buffer += 2 * x;
		c01 = As<Short4>(Insert(As<Int2>(c01), *Pointer<Int>(buffer), 0));
		buffer += pitchB;
		c01 = As<Short4>(Insert(As<Int2>(c01), *Pointer<Int>(buffer), 1));
		pixel.x = (c01 & Short4(0x00FFu)) | (c01 << 8);
		pixel.y = (c01 & Short4(0xFF00u)) | As<Short4>(As<UShort4>(c01) >> 8);
		pixel.z = Short4(0x0000u);
		pixel.w = Short4(0xFFFFu);
		break;
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
		{
			Int4 v = Int4(0);
			buffer += 4 * x;
			v = Insert(v, *Pointer<Int>(buffer + 0), 0);
			v = Insert(v, *Pointer<Int>(buffer + 4), 1);
			buffer += pitchB;
			v = Insert(v, *Pointer<Int>(buffer + 0), 2);
			v = Insert(v, *Pointer<Int>(buffer + 4), 3);

			pixel.x = Short4(v << 6) & Short4(0xFFC0u);
			pixel.y = Short4(v >> 4) & Short4(0xFFC0u);
			pixel.z = Short4(v >> 14) & Short4(0xFFC0u);
			pixel.w = Short4(v >> 16) & Short4(0xC000u);

			// Expand to 16 bit range
			pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 10);
			pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 10);
			pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 10);
			pixel.w |= As<Short4>(As<UShort4>(pixel.w) >> 2);
			pixel.w |= As<Short4>(As<UShort4>(pixel.w) >> 4);
			pixel.w |= As<Short4>(As<UShort4>(pixel.w) >> 8);
		}
		break;
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
		{
			Int4 v = Int4(0);
			v = Insert(v, *Pointer<Int>(buffer + 4 * x), 0);
			v = Insert(v, *Pointer<Int>(buffer + 4 * x + 4), 1);
			buffer += *Pointer<Int>(data + OFFSET(DrawData, colorPitchB[index]));
			v = Insert(v, *Pointer<Int>(buffer + 4 * x), 2);
			v = Insert(v, *Pointer<Int>(buffer + 4 * x + 4), 3);

			pixel.x = Short4(v >> 14) & Short4(0xFFC0u);
			pixel.y = Short4(v >> 4) & Short4(0xFFC0u);
			pixel.z = Short4(v << 6) & Short4(0xFFC0u);
			pixel.w = Short4(v >> 16) & Short4(0xC000u);

			// Expand to 16 bit range
			pixel.x |= As<Short4>(As<UShort4>(pixel.x) >> 10);
			pixel.y |= As<Short4>(As<UShort4>(pixel.y) >> 10);
			pixel.z |= As<Short4>(As<UShort4>(pixel.z) >> 10);
			pixel.w |= As<Short4>(As<UShort4>(pixel.w) >> 2);
			pixel.w |= As<Short4>(As<UShort4>(pixel.w) >> 4);
			pixel.w |= As<Short4>(As<UShort4>(pixel.w) >> 8);
		}
		break;
	default:
		UNSUPPORTED("VkFormat %d", int(format));
	}
}

Float PixelRoutine::blendConstant(vk::Format format, int component, BlendFactorModifier modifier)
{
	bool inverse = (modifier == OneMinus);

	if(format.isUnsignedNormalized())
	{
		return inverse ? *Pointer<Float>(data + OFFSET(DrawData, factor.invBlendConstantU.v[component]))
		               : *Pointer<Float>(data + OFFSET(DrawData, factor.blendConstantU.v[component]));
	}
	else if(format.isSignedNormalized())
	{
		return inverse ? *Pointer<Float>(data + OFFSET(DrawData, factor.invBlendConstantS.v[component]))
		               : *Pointer<Float>(data + OFFSET(DrawData, factor.blendConstantS.v[component]));
	}
	else  // Floating-point format
	{
		ASSERT(format.isFloatFormat());
		return inverse ? *Pointer<Float>(data + OFFSET(DrawData, factor.invBlendConstantF.v[component]))
		               : *Pointer<Float>(data + OFFSET(DrawData, factor.blendConstantF.v[component]));
	}
}

void PixelRoutine::blendFactorRGB(SIMD::Float4 &blendFactor, const SIMD::Float4 &sourceColor, const SIMD::Float4 &destColor, VkBlendFactor colorBlendFactor, vk::Format format)
{
	switch(colorBlendFactor)
	{
	case VK_BLEND_FACTOR_ZERO:
		blendFactor.x = 0.0f;
		blendFactor.y = 0.0f;
		blendFactor.z = 0.0f;
		break;
	case VK_BLEND_FACTOR_ONE:
		blendFactor.x = 1.0f;
		blendFactor.y = 1.0f;
		blendFactor.z = 1.0f;
		break;
	case VK_BLEND_FACTOR_SRC_COLOR:
		blendFactor.x = sourceColor.x;
		blendFactor.y = sourceColor.y;
		blendFactor.z = sourceColor.z;
		break;
	case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
		blendFactor.x = 1.0f - sourceColor.x;
		blendFactor.y = 1.0f - sourceColor.y;
		blendFactor.z = 1.0f - sourceColor.z;
		break;
	case VK_BLEND_FACTOR_DST_COLOR:
		blendFactor.x = destColor.x;
		blendFactor.y = destColor.y;
		blendFactor.z = destColor.z;
		break;
	case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
		blendFactor.x = 1.0f - destColor.x;
		blendFactor.y = 1.0f - destColor.y;
		blendFactor.z = 1.0f - destColor.z;
		break;
	case VK_BLEND_FACTOR_SRC_ALPHA:
		blendFactor.x = sourceColor.w;
		blendFactor.y = sourceColor.w;
		blendFactor.z = sourceColor.w;
		break;
	case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
		blendFactor.x = 1.0f - sourceColor.w;
		blendFactor.y = 1.0f - sourceColor.w;
		blendFactor.z = 1.0f - sourceColor.w;
		break;
	case VK_BLEND_FACTOR_DST_ALPHA:
		blendFactor.x = destColor.w;
		blendFactor.y = destColor.w;
		blendFactor.z = destColor.w;
		break;
	case VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
		blendFactor.x = 1.0f - destColor.w;
		blendFactor.y = 1.0f - destColor.w;
		blendFactor.z = 1.0f - destColor.w;
		break;
	case VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
		blendFactor.x = 1.0f - destColor.w;
		blendFactor.x = Min(blendFactor.x, sourceColor.w);
		blendFactor.y = blendFactor.x;
		blendFactor.z = blendFactor.x;
		break;
	case VK_BLEND_FACTOR_CONSTANT_COLOR:
		blendFactor.x = blendConstant(format, 0);
		blendFactor.y = blendConstant(format, 1);
		blendFactor.z = blendConstant(format, 2);
		break;
	case VK_BLEND_FACTOR_CONSTANT_ALPHA:
		blendFactor.x = blendConstant(format, 3);
		blendFactor.y = blendConstant(format, 3);
		blendFactor.z = blendConstant(format, 3);
		break;
	case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
		blendFactor.x = blendConstant(format, 0, OneMinus);
		blendFactor.y = blendConstant(format, 1, OneMinus);
		blendFactor.z = blendConstant(format, 2, OneMinus);
		break;
	case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
		blendFactor.x = blendConstant(format, 3, OneMinus);
		blendFactor.y = blendConstant(format, 3, OneMinus);
		blendFactor.z = blendConstant(format, 3, OneMinus);
		break;

	default:
		UNSUPPORTED("VkBlendFactor: %d", int(colorBlendFactor));
	}

	// "If the color attachment is fixed-point, the components of the source and destination values and blend factors are each clamped
	//  to [0,1] or [-1,1] respectively for an unsigned normalized or signed normalized color attachment prior to evaluating the blend
	//  operations. If the color attachment is floating-point, no clamping occurs."
	if(blendFactorCanExceedFormatRange(colorBlendFactor, format))
	{
		if(format.isUnsignedNormalized())
		{
			blendFactor.x = Min(Max(blendFactor.x, 0.0f), 1.0f);
			blendFactor.y = Min(Max(blendFactor.y, 0.0f), 1.0f);
			blendFactor.z = Min(Max(blendFactor.z, 0.0f), 1.0f);
		}
		else if(format.isSignedNormalized())
		{
			blendFactor.x = Min(Max(blendFactor.x, -1.0f), 1.0f);
			blendFactor.y = Min(Max(blendFactor.y, -1.0f), 1.0f);
			blendFactor.z = Min(Max(blendFactor.z, -1.0f), 1.0f);
		}
	}
}

void PixelRoutine::blendFactorAlpha(SIMD::Float &blendFactorAlpha, const SIMD::Float &sourceAlpha, const SIMD::Float &destAlpha, VkBlendFactor alphaBlendFactor, vk::Format format)
{
	switch(alphaBlendFactor)
	{
	case VK_BLEND_FACTOR_ZERO:
		blendFactorAlpha = 0.0f;
		break;
	case VK_BLEND_FACTOR_ONE:
		blendFactorAlpha = 1.0f;
		break;
	case VK_BLEND_FACTOR_SRC_COLOR:
		blendFactorAlpha = sourceAlpha;
		break;
	case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
		blendFactorAlpha = 1.0f - sourceAlpha;
		break;
	case VK_BLEND_FACTOR_DST_COLOR:
		blendFactorAlpha = destAlpha;
		break;
	case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
		blendFactorAlpha = 1.0f - destAlpha;
		break;
	case VK_BLEND_FACTOR_SRC_ALPHA:
		blendFactorAlpha = sourceAlpha;
		break;
	case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
		blendFactorAlpha = 1.0f - sourceAlpha;
		break;
	case VK_BLEND_FACTOR_DST_ALPHA:
		blendFactorAlpha = destAlpha;
		break;
	case VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
		blendFactorAlpha = 1.0f - destAlpha;
		break;
	case VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
		blendFactorAlpha = 1.0f;
		break;
	case VK_BLEND_FACTOR_CONSTANT_COLOR:
	case VK_BLEND_FACTOR_CONSTANT_ALPHA:
		blendFactorAlpha = blendConstant(format, 3);
		break;
	case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
	case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
		blendFactorAlpha = blendConstant(format, 3, OneMinus);
		break;
	default:
		UNSUPPORTED("VkBlendFactor: %d", int(alphaBlendFactor));
	}

	// "If the color attachment is fixed-point, the components of the source and destination values and blend factors are each clamped
	//  to [0,1] or [-1,1] respectively for an unsigned normalized or signed normalized color attachment prior to evaluating the blend
	//  operations. If the color attachment is floating-point, no clamping occurs."
	if(blendFactorCanExceedFormatRange(alphaBlendFactor, format))
	{
		if(format.isUnsignedNormalized())
		{
			blendFactorAlpha = Min(Max(blendFactorAlpha, 0.0f), 1.0f);
		}
		else if(format.isSignedNormalized())
		{
			blendFactorAlpha = Min(Max(blendFactorAlpha, -1.0f), 1.0f);
		}
	}
}

SIMD::Float PixelRoutine::blendOpOverlay(SIMD::Float &src, SIMD::Float &dst)
{
	SIMD::Int largeDst = CmpGT(dst, 0.5f);
	return As<SIMD::Float>(
	    (~largeDst & As<SIMD::Int>(2.0f * src * dst)) |
	    (largeDst & As<SIMD::Int>(1.0f - (2.0f * (1.0f - src) * (1.0f - dst)))));
}

SIMD::Float PixelRoutine::blendOpColorDodge(SIMD::Float &src, SIMD::Float &dst)
{
	SIMD::Int srcBelowOne = CmpLT(src, 1.0f);
	SIMD::Int positiveDst = CmpGT(dst, 0.0f);
	return As<SIMD::Float>(positiveDst & ((~srcBelowOne & As<SIMD::Int>(SIMD::Float(1.0f))) |
	                                      (srcBelowOne & As<SIMD::Int>(Min(1.0f, (dst / (1.0f - src)))))));
}

SIMD::Float PixelRoutine::blendOpColorBurn(SIMD::Float &src, SIMD::Float &dst)
{
	SIMD::Int dstBelowOne = CmpLT(dst, 1.0f);
	SIMD::Int positiveSrc = CmpGT(src, 0.0f);
	return As<SIMD::Float>(
	    (~dstBelowOne & As<SIMD::Int>(SIMD::Float(1.0f))) |
	    (dstBelowOne & positiveSrc & As<SIMD::Int>(1.0f - Min(1.0f, (1.0f - dst) / src))));
}

SIMD::Float PixelRoutine::blendOpHardlight(SIMD::Float &src, SIMD::Float &dst)
{
	SIMD::Int largeSrc = CmpGT(src, 0.5f);
	return As<SIMD::Float>(
	    (~largeSrc & As<SIMD::Int>(2.0f * src * dst)) |
	    (largeSrc & As<SIMD::Int>(1.0f - (2.0f * (1.0f - src) * (1.0f - dst)))));
}

SIMD::Float PixelRoutine::blendOpSoftlight(SIMD::Float &src, SIMD::Float &dst)
{
	SIMD::Int largeSrc = CmpGT(src, 0.5f);
	SIMD::Int largeDst = CmpGT(dst, 0.25f);

	return As<SIMD::Float>(
	    (~largeSrc & As<SIMD::Int>(dst - ((1.0f - (2.0f * src)) * dst * (1.0f - dst)))) |
	    (largeSrc & ((~largeDst & As<SIMD::Int>(dst + (((2.0f * src) - 1.0f) * dst * ((((16.0f * dst) - 12.0f) * dst) + 3.0f)))) |
	                 (largeDst & As<SIMD::Int>(dst + (((2.0f * src) - 1.0f) * (Sqrt<Mediump>(dst) - dst)))))));
}

SIMD::Float PixelRoutine::maxRGB(SIMD::Float4 &c)
{
	return Max(Max(c.x, c.y), c.z);
}

SIMD::Float PixelRoutine::minRGB(SIMD::Float4 &c)
{
	return Min(Min(c.x, c.y), c.z);
}

void PixelRoutine::setLumSat(SIMD::Float4 &cbase, SIMD::Float4 &csat, SIMD::Float4 &clum, SIMD::Float &x, SIMD::Float &y, SIMD::Float &z)
{
	SIMD::Float minbase = minRGB(cbase);
	SIMD::Float sbase = maxRGB(cbase) - minbase;
	SIMD::Float ssat = maxRGB(csat) - minRGB(csat);
	SIMD::Int isNonZero = CmpGT(sbase, 0.0f);
	SIMD::Float4 color;
	color.x = As<SIMD::Float>(isNonZero & As<SIMD::Int>((cbase.x - minbase) * ssat / sbase));
	color.y = As<SIMD::Float>(isNonZero & As<SIMD::Int>((cbase.y - minbase) * ssat / sbase));
	color.z = As<SIMD::Float>(isNonZero & As<SIMD::Int>((cbase.z - minbase) * ssat / sbase));
	setLum(color, clum, x, y, z);
}

SIMD::Float PixelRoutine::lumRGB(SIMD::Float4 &c)
{
	return c.x * 0.3f + c.y * 0.59f + c.z * 0.11f;
}

SIMD::Float PixelRoutine::computeLum(SIMD::Float &color, SIMD::Float &lum, SIMD::Float &mincol, SIMD::Float &maxcol, SIMD::Int &negative, SIMD::Int &aboveOne)
{
	return As<SIMD::Float>(
	    (negative & As<SIMD::Int>(lum + ((color - lum) * lum) / (lum - mincol))) |
	    (~negative & ((aboveOne & As<SIMD::Int>(lum + ((color - lum) * (1.0f - lum)) / (maxcol - lum))) |
	                  (~aboveOne & As<SIMD::Int>(color)))));
}

void PixelRoutine::setLum(SIMD::Float4 &cbase, SIMD::Float4 &clum, SIMD::Float &x, SIMD::Float &y, SIMD::Float &z)
{
	SIMD::Float lbase = lumRGB(cbase);
	SIMD::Float llum = lumRGB(clum);
	SIMD::Float ldiff = llum - lbase;

	SIMD::Float4 color;
	color.x = cbase.x + ldiff;
	color.y = cbase.y + ldiff;
	color.z = cbase.z + ldiff;

	SIMD::Float lum = lumRGB(color);
	SIMD::Float mincol = minRGB(color);
	SIMD::Float maxcol = maxRGB(color);

	SIMD::Int negative = CmpLT(mincol, 0.0f);
	SIMD::Int aboveOne = CmpGT(maxcol, 1.0f);

	x = computeLum(color.x, lum, mincol, maxcol, negative, aboveOne);
	y = computeLum(color.y, lum, mincol, maxcol, negative, aboveOne);
	z = computeLum(color.z, lum, mincol, maxcol, negative, aboveOne);
}

void PixelRoutine::premultiply(SIMD::Float4 &c)
{
	SIMD::Int nonZeroAlpha = CmpNEQ(c.w, 0.0f);
	c.x = As<SIMD::Float>(nonZeroAlpha & As<SIMD::Int>(c.x / c.w));
	c.y = As<SIMD::Float>(nonZeroAlpha & As<SIMD::Int>(c.y / c.w));
	c.z = As<SIMD::Float>(nonZeroAlpha & As<SIMD::Int>(c.z / c.w));
}

SIMD::Float4 PixelRoutine::computeAdvancedBlendMode(int index, const SIMD::Float4 &src, const SIMD::Float4 &dst, const SIMD::Float4 &srcFactor, const SIMD::Float4 &dstFactor)
{
	SIMD::Float4 srcColor = src;
	srcColor.x *= srcFactor.x;
	srcColor.y *= srcFactor.y;
	srcColor.z *= srcFactor.z;
	srcColor.w *= srcFactor.w;

	SIMD::Float4 dstColor = dst;
	dstColor.x *= dstFactor.x;
	dstColor.y *= dstFactor.y;
	dstColor.z *= dstFactor.z;
	dstColor.w *= dstFactor.w;

	premultiply(srcColor);
	premultiply(dstColor);

	SIMD::Float4 blendedColor;

	switch(state.blendState[index].blendOperation)
	{
	case VK_BLEND_OP_MULTIPLY_EXT:
		blendedColor.x = (srcColor.x * dstColor.x);
		blendedColor.y = (srcColor.y * dstColor.y);
		blendedColor.z = (srcColor.z * dstColor.z);
		break;
	case VK_BLEND_OP_SCREEN_EXT:
		blendedColor.x = srcColor.x + dstColor.x - (srcColor.x * dstColor.x);
		blendedColor.y = srcColor.y + dstColor.y - (srcColor.y * dstColor.y);
		blendedColor.z = srcColor.z + dstColor.z - (srcColor.z * dstColor.z);
		break;
	case VK_BLEND_OP_OVERLAY_EXT:
		blendedColor.x = blendOpOverlay(srcColor.x, dstColor.x);
		blendedColor.y = blendOpOverlay(srcColor.y, dstColor.y);
		blendedColor.z = blendOpOverlay(srcColor.z, dstColor.z);
		break;
	case VK_BLEND_OP_DARKEN_EXT:
		blendedColor.x = Min(srcColor.x, dstColor.x);
		blendedColor.y = Min(srcColor.y, dstColor.y);
		blendedColor.z = Min(srcColor.z, dstColor.z);
		break;
	case VK_BLEND_OP_LIGHTEN_EXT:
		blendedColor.x = Max(srcColor.x, dstColor.x);
		blendedColor.y = Max(srcColor.y, dstColor.y);
		blendedColor.z = Max(srcColor.z, dstColor.z);
		break;
	case VK_BLEND_OP_COLORDODGE_EXT:
		blendedColor.x = blendOpColorDodge(srcColor.x, dstColor.x);
		blendedColor.y = blendOpColorDodge(srcColor.y, dstColor.y);
		blendedColor.z = blendOpColorDodge(srcColor.z, dstColor.z);
		break;
	case VK_BLEND_OP_COLORBURN_EXT:
		blendedColor.x = blendOpColorBurn(srcColor.x, dstColor.x);
		blendedColor.y = blendOpColorBurn(srcColor.y, dstColor.y);
		blendedColor.z = blendOpColorBurn(srcColor.z, dstColor.z);
		break;
	case VK_BLEND_OP_HARDLIGHT_EXT:
		blendedColor.x = blendOpHardlight(srcColor.x, dstColor.x);
		blendedColor.y = blendOpHardlight(srcColor.y, dstColor.y);
		blendedColor.z = blendOpHardlight(srcColor.z, dstColor.z);
		break;
	case VK_BLEND_OP_SOFTLIGHT_EXT:
		blendedColor.x = blendOpSoftlight(srcColor.x, dstColor.x);
		blendedColor.y = blendOpSoftlight(srcColor.y, dstColor.y);
		blendedColor.z = blendOpSoftlight(srcColor.z, dstColor.z);
		break;
	case VK_BLEND_OP_DIFFERENCE_EXT:
		blendedColor.x = Abs(srcColor.x - dstColor.x);
		blendedColor.y = Abs(srcColor.y - dstColor.y);
		blendedColor.z = Abs(srcColor.z - dstColor.z);
		break;
	case VK_BLEND_OP_EXCLUSION_EXT:
		blendedColor.x = srcColor.x + dstColor.x - (srcColor.x * dstColor.x * 2.0f);
		blendedColor.y = srcColor.y + dstColor.y - (srcColor.y * dstColor.y * 2.0f);
		blendedColor.z = srcColor.z + dstColor.z - (srcColor.z * dstColor.z * 2.0f);
		break;
	case VK_BLEND_OP_HSL_HUE_EXT:
		setLumSat(srcColor, dstColor, dstColor, blendedColor.x, blendedColor.y, blendedColor.z);
		break;
	case VK_BLEND_OP_HSL_SATURATION_EXT:
		setLumSat(dstColor, srcColor, dstColor, blendedColor.x, blendedColor.y, blendedColor.z);
		break;
	case VK_BLEND_OP_HSL_COLOR_EXT:
		setLum(srcColor, dstColor, blendedColor.x, blendedColor.y, blendedColor.z);
		break;
	case VK_BLEND_OP_HSL_LUMINOSITY_EXT:
		setLum(dstColor, srcColor, blendedColor.x, blendedColor.y, blendedColor.z);
		break;
	default:
		UNSUPPORTED("Unsupported advanced VkBlendOp: %d", int(state.blendState[index].blendOperation));
		break;
	}

	SIMD::Float p = srcColor.w * dstColor.w;
	blendedColor.x *= p;
	blendedColor.y *= p;
	blendedColor.z *= p;

	p = srcColor.w * (1.0f - dstColor.w);
	blendedColor.x += srcColor.x * p;
	blendedColor.y += srcColor.y * p;
	blendedColor.z += srcColor.z * p;

	p = dstColor.w * (1.0f - srcColor.w);
	blendedColor.x += dstColor.x * p;
	blendedColor.y += dstColor.y * p;
	blendedColor.z += dstColor.z * p;

	return blendedColor;
}

bool PixelRoutine::blendFactorCanExceedFormatRange(VkBlendFactor blendFactor, vk::Format format)
{
	switch(blendFactor)
	{
	case VK_BLEND_FACTOR_ZERO:
	case VK_BLEND_FACTOR_ONE:
		return false;
	case VK_BLEND_FACTOR_SRC_COLOR:
	case VK_BLEND_FACTOR_SRC_ALPHA:
		// Source values have been clamped after fragment shader execution if the attachment format is normalized.
		return false;
	case VK_BLEND_FACTOR_DST_COLOR:
	case VK_BLEND_FACTOR_DST_ALPHA:
		// Dest values have a valid range due to being read from the attachment.
		return false;
	case VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR:
	case VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA:
	case VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR:
	case VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA:
		// For signed formats, negative values cause the result to exceed 1.0.
		return format.isSignedNormalized();
	case VK_BLEND_FACTOR_SRC_ALPHA_SATURATE:
		// min(As, 1 - Ad)
		return false;
	case VK_BLEND_FACTOR_CONSTANT_COLOR:
	case VK_BLEND_FACTOR_CONSTANT_ALPHA:
	case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR:
	case VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA:
		return false;

	default:
		UNSUPPORTED("VkBlendFactor: %d", int(blendFactor));
		return false;
	}
}

SIMD::Float4 PixelRoutine::alphaBlend(int index, const Pointer<Byte> &cBuffer, const SIMD::Float4 &sourceColor, const Int &x)
{
	if(!state.blendState[index].alphaBlendEnable)
	{
		return sourceColor;
	}

	vk::Format format = state.colorFormat[index];
	ASSERT(format.supportsColorAttachmentBlend());

	Pointer<Byte> buffer = cBuffer;
	Int pitchB = *Pointer<Int>(data + OFFSET(DrawData, colorPitchB[index]));

	// texelColor holds four texel color values.
	// Note: Despite the type being Vector4f, the colors may be stored as
	// integers. Half-floats are stored as full 32-bit floats.
	// Non-float and non-fixed point formats are not alpha blended.
	Vector4f texelColor;

	switch(format)
	{
	case VK_FORMAT_R32_SINT:
	case VK_FORMAT_R32_UINT:
	case VK_FORMAT_R32_SFLOAT:
		// FIXME: movlps
		buffer += 4 * x;
		texelColor.x.x = *Pointer<Float>(buffer + 0);
		texelColor.x.y = *Pointer<Float>(buffer + 4);
		buffer += pitchB;
		// FIXME: movhps
		texelColor.x.z = *Pointer<Float>(buffer + 0);
		texelColor.x.w = *Pointer<Float>(buffer + 4);
		texelColor.y = texelColor.z = texelColor.w = 1.0f;
		break;
	case VK_FORMAT_R32G32_SINT:
	case VK_FORMAT_R32G32_UINT:
	case VK_FORMAT_R32G32_SFLOAT:
		buffer += 8 * x;
		texelColor.x = *Pointer<Float4>(buffer, 16);
		buffer += pitchB;
		texelColor.y = *Pointer<Float4>(buffer, 16);
		texelColor.z = texelColor.x;
		texelColor.x = ShuffleLowHigh(texelColor.x, texelColor.y, 0x0202);
		texelColor.z = ShuffleLowHigh(texelColor.z, texelColor.y, 0x1313);
		texelColor.y = texelColor.z;
		texelColor.z = texelColor.w = 1.0f;
		break;
	case VK_FORMAT_R32G32B32A32_SFLOAT:
	case VK_FORMAT_R32G32B32A32_SINT:
	case VK_FORMAT_R32G32B32A32_UINT:
		buffer += 16 * x;
		texelColor.x = *Pointer<Float4>(buffer + 0, 16);
		texelColor.y = *Pointer<Float4>(buffer + 16, 16);
		buffer += pitchB;
		texelColor.z = *Pointer<Float4>(buffer + 0, 16);
		texelColor.w = *Pointer<Float4>(buffer + 16, 16);
		transpose4x4(texelColor.x, texelColor.y, texelColor.z, texelColor.w);
		break;
	case VK_FORMAT_R16_UNORM:
		buffer += 2 * x;
		texelColor.x.x = Float(Int(*Pointer<UShort>(buffer + 0)));
		texelColor.x.y = Float(Int(*Pointer<UShort>(buffer + 2)));
		buffer += pitchB;
		texelColor.x.z = Float(Int(*Pointer<UShort>(buffer + 0)));
		texelColor.x.w = Float(Int(*Pointer<UShort>(buffer + 2)));
		texelColor.x *= (1.0f / 0xFFFF);
		texelColor.y = texelColor.z = texelColor.w = 1.0f;
		break;
	case VK_FORMAT_R16_SFLOAT:
		buffer += 2 * x;
		texelColor.x.x = Float(*Pointer<Half>(buffer + 0));
		texelColor.x.y = Float(*Pointer<Half>(buffer + 2));
		buffer += pitchB;
		texelColor.x.z = Float(*Pointer<Half>(buffer + 0));
		texelColor.x.w = Float(*Pointer<Half>(buffer + 2));
		texelColor.y = texelColor.z = texelColor.w = 1.0f;
		break;
	case VK_FORMAT_R16G16_UNORM:
		buffer += 4 * x;
		texelColor.x.x = Float(Int(*Pointer<UShort>(buffer + 0)));
		texelColor.y.x = Float(Int(*Pointer<UShort>(buffer + 2)));
		texelColor.x.y = Float(Int(*Pointer<UShort>(buffer + 4)));
		texelColor.y.y = Float(Int(*Pointer<UShort>(buffer + 6)));
		buffer += pitchB;
		texelColor.x.z = Float(Int(*Pointer<UShort>(buffer + 0)));
		texelColor.y.z = Float(Int(*Pointer<UShort>(buffer + 2)));
		texelColor.x.w = Float(Int(*Pointer<UShort>(buffer + 4)));
		texelColor.y.w = Float(Int(*Pointer<UShort>(buffer + 6)));
		texelColor.x *= (1.0f / 0xFFFF);
		texelColor.y *= (1.0f / 0xFFFF);
		texelColor.z = texelColor.w = 1.0f;
		break;
	case VK_FORMAT_R16G16_SFLOAT:
		buffer += 4 * x;
		texelColor.x.x = Float(*Pointer<Half>(buffer + 0));
		texelColor.y.x = Float(*Pointer<Half>(buffer + 2));
		texelColor.x.y = Float(*Pointer<Half>(buffer + 4));
		texelColor.y.y = Float(*Pointer<Half>(buffer + 6));
		buffer += pitchB;
		texelColor.x.z = Float(*Pointer<Half>(buffer + 0));
		texelColor.y.z = Float(*Pointer<Half>(buffer + 2));
		texelColor.x.w = Float(*Pointer<Half>(buffer + 4));
		texelColor.y.w = Float(*Pointer<Half>(buffer + 6));
		texelColor.z = texelColor.w = 1.0f;
		break;
	case VK_FORMAT_R16G16B16A16_UNORM:
		buffer += 8 * x;
		texelColor.x.x = Float(Int(*Pointer<UShort>(buffer + 0x0)));
		texelColor.y.x = Float(Int(*Pointer<UShort>(buffer + 0x2)));
		texelColor.z.x = Float(Int(*Pointer<UShort>(buffer + 0x4)));
		texelColor.w.x = Float(Int(*Pointer<UShort>(buffer + 0x6)));
		texelColor.x.y = Float(Int(*Pointer<UShort>(buffer + 0x8)));
		texelColor.y.y = Float(Int(*Pointer<UShort>(buffer + 0xa)));
		texelColor.z.y = Float(Int(*Pointer<UShort>(buffer + 0xc)));
		texelColor.w.y = Float(Int(*Pointer<UShort>(buffer + 0xe)));
		buffer += pitchB;
		texelColor.x.z = Float(Int(*Pointer<UShort>(buffer + 0x0)));
		texelColor.y.z = Float(Int(*Pointer<UShort>(buffer + 0x2)));
		texelColor.z.z = Float(Int(*Pointer<UShort>(buffer + 0x4)));
		texelColor.w.z = Float(Int(*Pointer<UShort>(buffer + 0x6)));
		texelColor.x.w = Float(Int(*Pointer<UShort>(buffer + 0x8)));
		texelColor.y.w = Float(Int(*Pointer<UShort>(buffer + 0xa)));
		texelColor.z.w = Float(Int(*Pointer<UShort>(buffer + 0xc)));
		texelColor.w.w = Float(Int(*Pointer<UShort>(buffer + 0xe)));
		texelColor.x *= (1.0f / 0xFFFF);
		texelColor.y *= (1.0f / 0xFFFF);
		texelColor.z *= (1.0f / 0xFFFF);
		texelColor.w *= (1.0f / 0xFFFF);
		break;
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		buffer += 8 * x;
		texelColor.x.x = Float(*Pointer<Half>(buffer + 0x0));
		texelColor.y.x = Float(*Pointer<Half>(buffer + 0x2));
		texelColor.z.x = Float(*Pointer<Half>(buffer + 0x4));
		texelColor.w.x = Float(*Pointer<Half>(buffer + 0x6));
		texelColor.x.y = Float(*Pointer<Half>(buffer + 0x8));
		texelColor.y.y = Float(*Pointer<Half>(buffer + 0xa));
		texelColor.z.y = Float(*Pointer<Half>(buffer + 0xc));
		texelColor.w.y = Float(*Pointer<Half>(buffer + 0xe));
		buffer += pitchB;
		texelColor.x.z = Float(*Pointer<Half>(buffer + 0x0));
		texelColor.y.z = Float(*Pointer<Half>(buffer + 0x2));
		texelColor.z.z = Float(*Pointer<Half>(buffer + 0x4));
		texelColor.w.z = Float(*Pointer<Half>(buffer + 0x6));
		texelColor.x.w = Float(*Pointer<Half>(buffer + 0x8));
		texelColor.y.w = Float(*Pointer<Half>(buffer + 0xa));
		texelColor.z.w = Float(*Pointer<Half>(buffer + 0xc));
		texelColor.w.w = Float(*Pointer<Half>(buffer + 0xe));
		break;
	case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
		buffer += 4 * x;
		texelColor.x = r11g11b10Unpack(*Pointer<UInt>(buffer + 0));
		texelColor.y = r11g11b10Unpack(*Pointer<UInt>(buffer + 4));
		buffer += pitchB;
		texelColor.z = r11g11b10Unpack(*Pointer<UInt>(buffer + 0));
		texelColor.w = r11g11b10Unpack(*Pointer<UInt>(buffer + 4));
		transpose4x3(texelColor.x, texelColor.y, texelColor.z, texelColor.w);
		texelColor.w = 1.0f;
		break;
	default:
		{
			// Attempt to read an integer based format and convert it to float
			Vector4s color;
			readPixel(index, cBuffer, x, color);
			texelColor.x = Float4(As<UShort4>(color.x)) * (1.0f / 0xFFFF);
			texelColor.y = Float4(As<UShort4>(color.y)) * (1.0f / 0xFFFF);
			texelColor.z = Float4(As<UShort4>(color.z)) * (1.0f / 0xFFFF);
			texelColor.w = Float4(As<UShort4>(color.w)) * (1.0f / 0xFFFF);

			if(isSRGB(index))
			{
				texelColor.x = sRGBtoLinear(texelColor.x);
				texelColor.y = sRGBtoLinear(texelColor.y);
				texelColor.z = sRGBtoLinear(texelColor.z);
			}
		}
		break;
	}

	ASSERT(SIMD::Width == 4);
	SIMD::Float4 destColor;
	destColor.x = texelColor.x;
	destColor.y = texelColor.y;
	destColor.z = texelColor.z;
	destColor.w = texelColor.w;

	SIMD::Float4 sourceFactor;
	SIMD::Float4 destFactor;

	blendFactorRGB(sourceFactor, sourceColor, destColor, state.blendState[index].sourceBlendFactor, format);
	blendFactorRGB(destFactor, sourceColor, destColor, state.blendState[index].destBlendFactor, format);
	blendFactorAlpha(sourceFactor.w, sourceColor.w, destColor.w, state.blendState[index].sourceBlendFactorAlpha, format);
	blendFactorAlpha(destFactor.w, sourceColor.w, destColor.w, state.blendState[index].destBlendFactorAlpha, format);

	SIMD::Float4 blendedColor;

	switch(state.blendState[index].blendOperation)
	{
	case VK_BLEND_OP_ADD:
		blendedColor.x = sourceColor.x * sourceFactor.x + destColor.x * destFactor.x;
		blendedColor.y = sourceColor.y * sourceFactor.y + destColor.y * destFactor.y;
		blendedColor.z = sourceColor.z * sourceFactor.z + destColor.z * destFactor.z;
		break;
	case VK_BLEND_OP_SUBTRACT:
		blendedColor.x = sourceColor.x * sourceFactor.x - destColor.x * destFactor.x;
		blendedColor.y = sourceColor.y * sourceFactor.y - destColor.y * destFactor.y;
		blendedColor.z = sourceColor.z * sourceFactor.z - destColor.z * destFactor.z;
		break;
	case VK_BLEND_OP_REVERSE_SUBTRACT:
		blendedColor.x = destColor.x * destFactor.x - sourceColor.x * sourceFactor.x;
		blendedColor.y = destColor.y * destFactor.y - sourceColor.y * sourceFactor.y;
		blendedColor.z = destColor.z * destFactor.z - sourceColor.z * sourceFactor.z;
		break;
	case VK_BLEND_OP_MIN:
		blendedColor.x = Min(sourceColor.x, destColor.x);
		blendedColor.y = Min(sourceColor.y, destColor.y);
		blendedColor.z = Min(sourceColor.z, destColor.z);
		break;
	case VK_BLEND_OP_MAX:
		blendedColor.x = Max(sourceColor.x, destColor.x);
		blendedColor.y = Max(sourceColor.y, destColor.y);
		blendedColor.z = Max(sourceColor.z, destColor.z);
		break;
	case VK_BLEND_OP_SRC_EXT:
		blendedColor.x = sourceColor.x;
		blendedColor.y = sourceColor.y;
		blendedColor.z = sourceColor.z;
		break;
	case VK_BLEND_OP_DST_EXT:
		blendedColor.x = destColor.x;
		blendedColor.y = destColor.y;
		blendedColor.z = destColor.z;
		break;
	case VK_BLEND_OP_ZERO_EXT:
		blendedColor.x = 0.0f;
		blendedColor.y = 0.0f;
		blendedColor.z = 0.0f;
		break;
	case VK_BLEND_OP_MULTIPLY_EXT:
	case VK_BLEND_OP_SCREEN_EXT:
	case VK_BLEND_OP_OVERLAY_EXT:
	case VK_BLEND_OP_DARKEN_EXT:
	case VK_BLEND_OP_LIGHTEN_EXT:
	case VK_BLEND_OP_COLORDODGE_EXT:
	case VK_BLEND_OP_COLORBURN_EXT:
	case VK_BLEND_OP_HARDLIGHT_EXT:
	case VK_BLEND_OP_SOFTLIGHT_EXT:
	case VK_BLEND_OP_DIFFERENCE_EXT:
	case VK_BLEND_OP_EXCLUSION_EXT:
	case VK_BLEND_OP_HSL_HUE_EXT:
	case VK_BLEND_OP_HSL_SATURATION_EXT:
	case VK_BLEND_OP_HSL_COLOR_EXT:
	case VK_BLEND_OP_HSL_LUMINOSITY_EXT:
		blendedColor = computeAdvancedBlendMode(index, sourceColor, destColor, sourceFactor, destFactor);
		break;
	default:
		UNSUPPORTED("VkBlendOp: %d", int(state.blendState[index].blendOperation));
	}

	switch(state.blendState[index].blendOperationAlpha)
	{
	case VK_BLEND_OP_ADD:
		blendedColor.w = sourceColor.w * sourceFactor.w + destColor.w * destFactor.w;
		break;
	case VK_BLEND_OP_SUBTRACT:
		blendedColor.w = sourceColor.w * sourceFactor.w - destColor.w * destFactor.w;
		break;
	case VK_BLEND_OP_REVERSE_SUBTRACT:
		blendedColor.w = destColor.w * destFactor.w - sourceColor.w * sourceFactor.w;
		break;
	case VK_BLEND_OP_MIN:
		blendedColor.w = Min(sourceColor.w, destColor.w);
		break;
	case VK_BLEND_OP_MAX:
		blendedColor.w = Max(sourceColor.w, destColor.w);
		break;
	case VK_BLEND_OP_SRC_EXT:
		blendedColor.w = sourceColor.w;
		break;
	case VK_BLEND_OP_DST_EXT:
		blendedColor.w = destColor.w;
		break;
	case VK_BLEND_OP_ZERO_EXT:
		blendedColor.w = 0.0f;
		break;
	case VK_BLEND_OP_MULTIPLY_EXT:
	case VK_BLEND_OP_SCREEN_EXT:
	case VK_BLEND_OP_OVERLAY_EXT:
	case VK_BLEND_OP_DARKEN_EXT:
	case VK_BLEND_OP_LIGHTEN_EXT:
	case VK_BLEND_OP_COLORDODGE_EXT:
	case VK_BLEND_OP_COLORBURN_EXT:
	case VK_BLEND_OP_HARDLIGHT_EXT:
	case VK_BLEND_OP_SOFTLIGHT_EXT:
	case VK_BLEND_OP_DIFFERENCE_EXT:
	case VK_BLEND_OP_EXCLUSION_EXT:
	case VK_BLEND_OP_HSL_HUE_EXT:
	case VK_BLEND_OP_HSL_SATURATION_EXT:
	case VK_BLEND_OP_HSL_COLOR_EXT:
	case VK_BLEND_OP_HSL_LUMINOSITY_EXT:
		// All of the currently supported 'advanced blend modes' compute the alpha the same way.
		blendedColor.w = sourceColor.w + destColor.w - (sourceColor.w * destColor.w);
		break;
	default:
		UNSUPPORTED("VkBlendOp: %d", int(state.blendState[index].blendOperationAlpha));
	}

	return blendedColor;
}

void PixelRoutine::writeColor(int index, const Pointer<Byte> &cBuffer, const Int &x, Vector4f &color, const Int &sMask, const Int &zMask, const Int &cMask)
{
	if(isSRGB(index))
	{
		color.x = linearToSRGB(color.x);
		color.y = linearToSRGB(color.y);
		color.z = linearToSRGB(color.z);
	}

	vk::Format format = state.colorFormat[index];
	switch(format)
	{
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_B8G8R8A8_SRGB:
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SRGB:
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
		color.w = Min(Max(color.w, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.w = As<Float4>(RoundInt(color.w * 0xFF));
		color.z = Min(Max(color.z, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.z = As<Float4>(RoundInt(color.z * 0xFF));
		// [[fallthrough]]
	case VK_FORMAT_R8G8_UNORM:
		color.y = Min(Max(color.y, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.y = As<Float4>(RoundInt(color.y * 0xFF));
		//[[fallthrough]]
	case VK_FORMAT_R8_UNORM:
		color.x = Min(Max(color.x, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.x = As<Float4>(RoundInt(color.x * 0xFF));
		break;
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
	case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
	case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
	case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
		color.w = Min(Max(color.w, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.w = As<Float4>(RoundInt(color.w * 0xF));
		color.z = Min(Max(color.z, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.z = As<Float4>(RoundInt(color.z * 0xF));
		color.y = Min(Max(color.y, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.y = As<Float4>(RoundInt(color.y * 0xF));
		color.x = Min(Max(color.x, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.x = As<Float4>(RoundInt(color.x * 0xF));
		break;
	case VK_FORMAT_B5G6R5_UNORM_PACK16:
	case VK_FORMAT_R5G6B5_UNORM_PACK16:
		color.z = Min(Max(color.z, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.z = As<Float4>(RoundInt(color.z * 0x1F));
		color.y = Min(Max(color.y, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.y = As<Float4>(RoundInt(color.y * 0x3F));
		color.x = Min(Max(color.x, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.x = As<Float4>(RoundInt(color.x * 0x1F));
		break;
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
	case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
	case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
		color.w = Min(Max(color.w, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.w = As<Float4>(RoundInt(color.w));
		color.z = Min(Max(color.z, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.z = As<Float4>(RoundInt(color.z * 0x1F));
		color.y = Min(Max(color.y, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.y = As<Float4>(RoundInt(color.y * 0x1F));
		color.x = Min(Max(color.x, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.x = As<Float4>(RoundInt(color.x * 0x1F));
		break;
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
		color.w = Min(Max(color.w, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.w = As<Float4>(RoundInt(color.w * 0x3));
		color.z = Min(Max(color.z, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.z = As<Float4>(RoundInt(color.z * 0x3FF));
		color.y = Min(Max(color.y, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.y = As<Float4>(RoundInt(color.y * 0x3FF));
		color.x = Min(Max(color.x, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.x = As<Float4>(RoundInt(color.x * 0x3FF));
		break;
	case VK_FORMAT_R16G16B16A16_UNORM:
		color.w = Min(Max(color.w, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.w = As<Float4>(RoundInt(color.w * 0xFFFF));
		color.z = Min(Max(color.z, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.z = As<Float4>(RoundInt(color.z * 0xFFFF));
		// [[fallthrough]]
	case VK_FORMAT_R16G16_UNORM:
		color.y = Min(Max(color.y, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.y = As<Float4>(RoundInt(color.y * 0xFFFF));
		//[[fallthrough]]
	case VK_FORMAT_R16_UNORM:
		color.x = Min(Max(color.x, 0.0f), 1.0f);  // TODO(b/204560089): Omit clamp if redundant
		color.x = As<Float4>(RoundInt(color.x * 0xFFFF));
		break;
	default:
		// TODO(b/204560089): Omit clamp if redundant
		if(format.isUnsignedNormalized())
		{
			color.x = Min(Max(color.x, 0.0f), 1.0f);
			color.y = Min(Max(color.y, 0.0f), 1.0f);
			color.z = Min(Max(color.z, 0.0f), 1.0f);
			color.w = Min(Max(color.w, 0.0f), 1.0f);
		}
		else if(format.isSignedNormalized())
		{
			color.x = Min(Max(color.x, -1.0f), 1.0f);
			color.y = Min(Max(color.y, -1.0f), 1.0f);
			color.z = Min(Max(color.z, -1.0f), 1.0f);
			color.w = Min(Max(color.w, -1.0f), 1.0f);
		}
	}

	switch(format)
	{
	case VK_FORMAT_R16_SFLOAT:
	case VK_FORMAT_R32_SFLOAT:
	case VK_FORMAT_R32_SINT:
	case VK_FORMAT_R32_UINT:
	case VK_FORMAT_R16_UNORM:
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_R16_UINT:
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_R8_UNORM:
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
	case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
	case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
	case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
	case VK_FORMAT_B5G6R5_UNORM_PACK16:
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
	case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
	case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
	case VK_FORMAT_R5G6B5_UNORM_PACK16:
		break;
	case VK_FORMAT_R16G16_SFLOAT:
	case VK_FORMAT_R32G32_SFLOAT:
	case VK_FORMAT_R32G32_SINT:
	case VK_FORMAT_R32G32_UINT:
	case VK_FORMAT_R16G16_UNORM:
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R16G16_UINT:
	case VK_FORMAT_R8G8_SINT:
	case VK_FORMAT_R8G8_UINT:
	case VK_FORMAT_R8G8_UNORM:
		color.z = color.x;
		color.x = UnpackLow(color.x, color.y);
		color.z = UnpackHigh(color.z, color.y);
		color.y = color.z;
		break;
	case VK_FORMAT_R16G16B16A16_SFLOAT:
	case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
	case VK_FORMAT_R32G32B32A32_SFLOAT:
	case VK_FORMAT_R32G32B32A32_SINT:
	case VK_FORMAT_R32G32B32A32_UINT:
	case VK_FORMAT_R16G16B16A16_UNORM:
	case VK_FORMAT_R16G16B16A16_SINT:
	case VK_FORMAT_R16G16B16A16_UINT:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_R8G8B8A8_UINT:
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SRGB:
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
		transpose4x4(color.x, color.y, color.z, color.w);
		break;
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_B8G8R8A8_SRGB:
		transpose4x4zyxw(color.z, color.y, color.x, color.w);
		break;
	default:
		UNSUPPORTED("VkFormat: %d", int(format));
	}

	int writeMask = state.colorWriteActive(index);
	if(format.isBGRformat())
	{
		// For BGR formats, flip R and B channels in the channels mask
		writeMask = (writeMask & 0x0000000A) | (writeMask & 0x00000001) << 2 | (writeMask & 0x00000004) >> 2;
	}

	Int xMask;  // Combination of all masks

	if(state.depthTestActive)
	{
		xMask = zMask;
	}
	else
	{
		xMask = cMask;
	}

	if(state.stencilActive)
	{
		xMask &= sMask;
	}

	Pointer<Byte> buffer = cBuffer;
	Int pitchB = *Pointer<Int>(data + OFFSET(DrawData, colorPitchB[index]));
	Float4 value;

	switch(format)
	{
	case VK_FORMAT_R32_SFLOAT:
	case VK_FORMAT_R32_SINT:
	case VK_FORMAT_R32_UINT:
		if(writeMask & 0x00000001)
		{
			buffer += 4 * x;

			// FIXME: movlps
			value.x = *Pointer<Float>(buffer + 0);
			value.y = *Pointer<Float>(buffer + 4);

			buffer += pitchB;

			// FIXME: movhps
			value.z = *Pointer<Float>(buffer + 0);
			value.w = *Pointer<Float>(buffer + 4);

			color.x = As<Float4>(As<Int4>(color.x) & *Pointer<Int4>(constants + OFFSET(Constants, maskD4X) + xMask * 16, 16));
			value = As<Float4>(As<Int4>(value) & *Pointer<Int4>(constants + OFFSET(Constants, invMaskD4X) + xMask * 16, 16));
			color.x = As<Float4>(As<Int4>(color.x) | As<Int4>(value));

			// FIXME: movhps
			*Pointer<Float>(buffer + 0) = color.x.z;
			*Pointer<Float>(buffer + 4) = color.x.w;

			buffer -= pitchB;

			// FIXME: movlps
			*Pointer<Float>(buffer + 0) = color.x.x;
			*Pointer<Float>(buffer + 4) = color.x.y;
		}
		break;
	case VK_FORMAT_R16_SFLOAT:
		if(writeMask & 0x00000001)
		{
			buffer += 2 * x;

			value = Insert(value, Float(*Pointer<Half>(buffer + 0)), 0);
			value = Insert(value, Float(*Pointer<Half>(buffer + 2)), 1);

			buffer += pitchB;

			value = Insert(value, Float(*Pointer<Half>(buffer + 0)), 2);
			value = Insert(value, Float(*Pointer<Half>(buffer + 2)), 3);

			color.x = As<Float4>(As<Int4>(color.x) & *Pointer<Int4>(constants + OFFSET(Constants, maskD4X) + xMask * 16, 16));
			value = As<Float4>(As<Int4>(value) & *Pointer<Int4>(constants + OFFSET(Constants, invMaskD4X) + xMask * 16, 16));
			color.x = As<Float4>(As<Int4>(color.x) | As<Int4>(value));

			*Pointer<Half>(buffer + 0) = Half(color.x.z);
			*Pointer<Half>(buffer + 2) = Half(color.x.w);

			buffer -= pitchB;

			*Pointer<Half>(buffer + 0) = Half(color.x.x);
			*Pointer<Half>(buffer + 2) = Half(color.x.y);
		}
		break;
	case VK_FORMAT_R16_UNORM:
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_R16_UINT:
		if(writeMask & 0x00000001)
		{
			buffer += 2 * x;

			UShort4 xyzw;
			xyzw = As<UShort4>(Insert(As<Int2>(xyzw), *Pointer<Int>(buffer), 0));

			buffer += pitchB;

			xyzw = As<UShort4>(Insert(As<Int2>(xyzw), *Pointer<Int>(buffer), 1));
			value = As<Float4>(Int4(xyzw));

			color.x = As<Float4>(As<Int4>(color.x) & *Pointer<Int4>(constants + OFFSET(Constants, maskD4X) + xMask * 16, 16));
			value = As<Float4>(As<Int4>(value) & *Pointer<Int4>(constants + OFFSET(Constants, invMaskD4X) + xMask * 16, 16));
			color.x = As<Float4>(As<Int4>(color.x) | As<Int4>(value));

			Float component = color.x.z;
			*Pointer<UShort>(buffer + 0) = UShort(As<Int>(component));
			component = color.x.w;
			*Pointer<UShort>(buffer + 2) = UShort(As<Int>(component));

			buffer -= pitchB;

			component = color.x.x;
			*Pointer<UShort>(buffer + 0) = UShort(As<Int>(component));
			component = color.x.y;
			*Pointer<UShort>(buffer + 2) = UShort(As<Int>(component));
		}
		break;
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_R8_UNORM:
		if(writeMask & 0x00000001)
		{
			buffer += x;

			UInt xyzw, packedCol;

			xyzw = UInt(*Pointer<UShort>(buffer)) & 0xFFFFu;
			buffer += pitchB;
			xyzw |= UInt(*Pointer<UShort>(buffer)) << 16;

			Short4 tmpCol = Short4(As<Int4>(color.x));
			if(format == VK_FORMAT_R8_SINT)
			{
				tmpCol = As<Short4>(PackSigned(tmpCol, tmpCol));
			}
			else
			{
				tmpCol = As<Short4>(PackUnsigned(tmpCol, tmpCol));
			}
			packedCol = Extract(As<Int2>(tmpCol), 0);

			packedCol = (packedCol & *Pointer<UInt>(constants + OFFSET(Constants, maskB4Q) + 8 * xMask)) |
			            (xyzw & *Pointer<UInt>(constants + OFFSET(Constants, invMaskB4Q) + 8 * xMask));

			*Pointer<UShort>(buffer) = UShort(packedCol >> 16);
			buffer -= pitchB;
			*Pointer<UShort>(buffer) = UShort(packedCol);
		}
		break;
	case VK_FORMAT_R32G32_SFLOAT:
	case VK_FORMAT_R32G32_SINT:
	case VK_FORMAT_R32G32_UINT:
		buffer += 8 * x;

		value = *Pointer<Float4>(buffer);

		if((writeMask & 0x00000003) != 0x00000003)
		{
			Float4 masked = value;
			color.x = As<Float4>(As<Int4>(color.x) & *Pointer<Int4>(constants + OFFSET(Constants, maskD01X[writeMask & 0x3][0])));
			masked = As<Float4>(As<Int4>(masked) & *Pointer<Int4>(constants + OFFSET(Constants, maskD01X[~writeMask & 0x3][0])));
			color.x = As<Float4>(As<Int4>(color.x) | As<Int4>(masked));
		}

		color.x = As<Float4>(As<Int4>(color.x) & *Pointer<Int4>(constants + OFFSET(Constants, maskQ01X) + xMask * 16, 16));
		value = As<Float4>(As<Int4>(value) & *Pointer<Int4>(constants + OFFSET(Constants, invMaskQ01X) + xMask * 16, 16));
		color.x = As<Float4>(As<Int4>(color.x) | As<Int4>(value));
		*Pointer<Float4>(buffer) = color.x;

		buffer += pitchB;

		value = *Pointer<Float4>(buffer);

		if((writeMask & 0x00000003) != 0x00000003)
		{
			Float4 masked;

			masked = value;
			color.y = As<Float4>(As<Int4>(color.y) & *Pointer<Int4>(constants + OFFSET(Constants, maskD01X[writeMask & 0x3][0])));
			masked = As<Float4>(As<Int4>(masked) & *Pointer<Int4>(constants + OFFSET(Constants, maskD01X[~writeMask & 0x3][0])));
			color.y = As<Float4>(As<Int4>(color.y) | As<Int4>(masked));
		}

		color.y = As<Float4>(As<Int4>(color.y) & *Pointer<Int4>(constants + OFFSET(Constants, maskQ23X) + xMask * 16, 16));
		value = As<Float4>(As<Int4>(value) & *Pointer<Int4>(constants + OFFSET(Constants, invMaskQ23X) + xMask * 16, 16));
		color.y = As<Float4>(As<Int4>(color.y) | As<Int4>(value));
		*Pointer<Float4>(buffer) = color.y;
		break;
	case VK_FORMAT_R16G16_SFLOAT:
		if((writeMask & 0x00000003) != 0x0)
		{
			buffer += 4 * x;

			UInt2 rgbaMask;
			UInt2 packedCol;
			packedCol = Insert(packedCol, (UInt(As<UShort>(Half(color.x.y))) << 16) | UInt(As<UShort>(Half(color.x.x))), 0);
			packedCol = Insert(packedCol, (UInt(As<UShort>(Half(color.x.w))) << 16) | UInt(As<UShort>(Half(color.x.z))), 1);

			UShort4 value = *Pointer<UShort4>(buffer);
			UInt2 mergedMask = *Pointer<UInt2>(constants + OFFSET(Constants, maskD01Q) + xMask * 8);
			if((writeMask & 0x3) != 0x3)
			{
				Int tmpMask = *Pointer<Int>(constants + OFFSET(Constants, maskW4Q[writeMask & 0x3]));
				rgbaMask = As<UInt2>(Int2(tmpMask, tmpMask));
				mergedMask &= rgbaMask;
			}
			*Pointer<UInt2>(buffer) = (packedCol & mergedMask) | (As<UInt2>(value) & ~mergedMask);

			buffer += pitchB;

			packedCol = Insert(packedCol, (UInt(As<UShort>(Half(color.y.y))) << 16) | UInt(As<UShort>(Half(color.y.x))), 0);
			packedCol = Insert(packedCol, (UInt(As<UShort>(Half(color.y.w))) << 16) | UInt(As<UShort>(Half(color.y.z))), 1);
			value = *Pointer<UShort4>(buffer);
			mergedMask = *Pointer<UInt2>(constants + OFFSET(Constants, maskD23Q) + xMask * 8);
			if((writeMask & 0x3) != 0x3)
			{
				mergedMask &= rgbaMask;
			}
			*Pointer<UInt2>(buffer) = (packedCol & mergedMask) | (As<UInt2>(value) & ~mergedMask);
		}
		break;
	case VK_FORMAT_R16G16_UNORM:
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R16G16_UINT:
		if((writeMask & 0x00000003) != 0x0)
		{
			buffer += 4 * x;

			UInt2 rgbaMask;
			UShort4 packedCol = UShort4(As<Int4>(color.x));
			UShort4 value = *Pointer<UShort4>(buffer);
			UInt2 mergedMask = *Pointer<UInt2>(constants + OFFSET(Constants, maskD01Q) + xMask * 8);
			if((writeMask & 0x3) != 0x3)
			{
				Int tmpMask = *Pointer<Int>(constants + OFFSET(Constants, maskW4Q[writeMask & 0x3]));
				rgbaMask = As<UInt2>(Int2(tmpMask, tmpMask));
				mergedMask &= rgbaMask;
			}
			*Pointer<UInt2>(buffer) = (As<UInt2>(packedCol) & mergedMask) | (As<UInt2>(value) & ~mergedMask);

			buffer += pitchB;

			packedCol = UShort4(As<Int4>(color.y));
			value = *Pointer<UShort4>(buffer);
			mergedMask = *Pointer<UInt2>(constants + OFFSET(Constants, maskD23Q) + xMask * 8);
			if((writeMask & 0x3) != 0x3)
			{
				mergedMask &= rgbaMask;
			}
			*Pointer<UInt2>(buffer) = (As<UInt2>(packedCol) & mergedMask) | (As<UInt2>(value) & ~mergedMask);
		}
		break;
	case VK_FORMAT_R8G8_SINT:
	case VK_FORMAT_R8G8_UINT:
	case VK_FORMAT_R8G8_UNORM:
		if((writeMask & 0x00000003) != 0x0)
		{
			buffer += 2 * x;

			Int2 xyzw, packedCol;

			xyzw = Insert(xyzw, *Pointer<Int>(buffer), 0);
			buffer += pitchB;
			xyzw = Insert(xyzw, *Pointer<Int>(buffer), 1);

			if(format == VK_FORMAT_R8G8_SINT)
			{
				packedCol = As<Int2>(PackSigned(Short4(As<Int4>(color.x)), Short4(As<Int4>(color.y))));
			}
			else
			{
				packedCol = As<Int2>(PackUnsigned(Short4(As<Int4>(color.x)), Short4(As<Int4>(color.y))));
			}

			UInt2 mergedMask = *Pointer<UInt2>(constants + OFFSET(Constants, maskW4Q) + xMask * 8);
			if((writeMask & 0x3) != 0x3)
			{
				Int tmpMask = *Pointer<Int>(constants + OFFSET(Constants, maskB4Q[5 * (writeMask & 0x3)]));
				UInt2 rgbaMask = As<UInt2>(Int2(tmpMask, tmpMask));
				mergedMask &= rgbaMask;
			}

			packedCol = As<Int2>((As<UInt2>(packedCol) & mergedMask) | (As<UInt2>(xyzw) & ~mergedMask));

			*Pointer<UInt>(buffer) = As<UInt>(Extract(packedCol, 1));
			buffer -= pitchB;
			*Pointer<UInt>(buffer) = As<UInt>(Extract(packedCol, 0));
		}
		break;
	case VK_FORMAT_R32G32B32A32_SFLOAT:
	case VK_FORMAT_R32G32B32A32_SINT:
	case VK_FORMAT_R32G32B32A32_UINT:
		buffer += 16 * x;

		{
			value = *Pointer<Float4>(buffer, 16);

			if(writeMask != 0x0000000F)
			{
				Float4 masked = value;
				color.x = As<Float4>(As<Int4>(color.x) & *Pointer<Int4>(constants + OFFSET(Constants, maskD4X[writeMask])));
				masked = As<Float4>(As<Int4>(masked) & *Pointer<Int4>(constants + OFFSET(Constants, invMaskD4X[writeMask])));
				color.x = As<Float4>(As<Int4>(color.x) | As<Int4>(masked));
			}

			color.x = As<Float4>(As<Int4>(color.x) & *Pointer<Int4>(constants + OFFSET(Constants, maskX0X) + xMask * 16, 16));
			value = As<Float4>(As<Int4>(value) & *Pointer<Int4>(constants + OFFSET(Constants, invMaskX0X) + xMask * 16, 16));
			color.x = As<Float4>(As<Int4>(color.x) | As<Int4>(value));
			*Pointer<Float4>(buffer, 16) = color.x;
		}

		{
			value = *Pointer<Float4>(buffer + 16, 16);

			if(writeMask != 0x0000000F)
			{
				Float4 masked = value;
				color.y = As<Float4>(As<Int4>(color.y) & *Pointer<Int4>(constants + OFFSET(Constants, maskD4X[writeMask])));
				masked = As<Float4>(As<Int4>(masked) & *Pointer<Int4>(constants + OFFSET(Constants, invMaskD4X[writeMask])));
				color.y = As<Float4>(As<Int4>(color.y) | As<Int4>(masked));
			}

			color.y = As<Float4>(As<Int4>(color.y) & *Pointer<Int4>(constants + OFFSET(Constants, maskX1X) + xMask * 16, 16));
			value = As<Float4>(As<Int4>(value) & *Pointer<Int4>(constants + OFFSET(Constants, invMaskX1X) + xMask * 16, 16));
			color.y = As<Float4>(As<Int4>(color.y) | As<Int4>(value));
			*Pointer<Float4>(buffer + 16, 16) = color.y;
		}

		buffer += pitchB;

		{
			value = *Pointer<Float4>(buffer, 16);

			if(writeMask != 0x0000000F)
			{
				Float4 masked = value;
				color.z = As<Float4>(As<Int4>(color.z) & *Pointer<Int4>(constants + OFFSET(Constants, maskD4X[writeMask])));
				masked = As<Float4>(As<Int4>(masked) & *Pointer<Int4>(constants + OFFSET(Constants, invMaskD4X[writeMask])));
				color.z = As<Float4>(As<Int4>(color.z) | As<Int4>(masked));
			}

			color.z = As<Float4>(As<Int4>(color.z) & *Pointer<Int4>(constants + OFFSET(Constants, maskX2X) + xMask * 16, 16));
			value = As<Float4>(As<Int4>(value) & *Pointer<Int4>(constants + OFFSET(Constants, invMaskX2X) + xMask * 16, 16));
			color.z = As<Float4>(As<Int4>(color.z) | As<Int4>(value));
			*Pointer<Float4>(buffer, 16) = color.z;
		}

		{
			value = *Pointer<Float4>(buffer + 16, 16);

			if(writeMask != 0x0000000F)
			{
				Float4 masked = value;
				color.w = As<Float4>(As<Int4>(color.w) & *Pointer<Int4>(constants + OFFSET(Constants, maskD4X[writeMask])));
				masked = As<Float4>(As<Int4>(masked) & *Pointer<Int4>(constants + OFFSET(Constants, invMaskD4X[writeMask])));
				color.w = As<Float4>(As<Int4>(color.w) | As<Int4>(masked));
			}

			color.w = As<Float4>(As<Int4>(color.w) & *Pointer<Int4>(constants + OFFSET(Constants, maskX3X) + xMask * 16, 16));
			value = As<Float4>(As<Int4>(value) & *Pointer<Int4>(constants + OFFSET(Constants, invMaskX3X) + xMask * 16, 16));
			color.w = As<Float4>(As<Int4>(color.w) | As<Int4>(value));
			*Pointer<Float4>(buffer + 16, 16) = color.w;
		}
		break;
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		if((writeMask & 0x0000000F) != 0x0)
		{
			buffer += 8 * x;

			UInt4 rgbaMask;
			UInt4 value = *Pointer<UInt4>(buffer);
			UInt4 packedCol;
			packedCol = Insert(packedCol, (UInt(As<UShort>(Half(color.x.y))) << 16) | UInt(As<UShort>(Half(color.x.x))), 0);
			packedCol = Insert(packedCol, (UInt(As<UShort>(Half(color.x.w))) << 16) | UInt(As<UShort>(Half(color.x.z))), 1);
			packedCol = Insert(packedCol, (UInt(As<UShort>(Half(color.y.y))) << 16) | UInt(As<UShort>(Half(color.y.x))), 2);
			packedCol = Insert(packedCol, (UInt(As<UShort>(Half(color.y.w))) << 16) | UInt(As<UShort>(Half(color.y.z))), 3);
			UInt4 mergedMask = *Pointer<UInt4>(constants + OFFSET(Constants, maskQ01X) + xMask * 16);
			if((writeMask & 0xF) != 0xF)
			{
				UInt2 tmpMask = *Pointer<UInt2>(constants + OFFSET(Constants, maskW4Q[writeMask]));
				rgbaMask = UInt4(tmpMask, tmpMask);
				mergedMask &= rgbaMask;
			}
			*Pointer<UInt4>(buffer) = (packedCol & mergedMask) | (As<UInt4>(value) & ~mergedMask);

			buffer += pitchB;

			value = *Pointer<UInt4>(buffer);
			packedCol = Insert(packedCol, (UInt(As<UShort>(Half(color.z.y))) << 16) | UInt(As<UShort>(Half(color.z.x))), 0);
			packedCol = Insert(packedCol, (UInt(As<UShort>(Half(color.z.w))) << 16) | UInt(As<UShort>(Half(color.z.z))), 1);
			packedCol = Insert(packedCol, (UInt(As<UShort>(Half(color.w.y))) << 16) | UInt(As<UShort>(Half(color.w.x))), 2);
			packedCol = Insert(packedCol, (UInt(As<UShort>(Half(color.w.w))) << 16) | UInt(As<UShort>(Half(color.w.z))), 3);
			mergedMask = *Pointer<UInt4>(constants + OFFSET(Constants, maskQ23X) + xMask * 16);
			if((writeMask & 0xF) != 0xF)
			{
				mergedMask &= rgbaMask;
			}
			*Pointer<UInt4>(buffer) = (packedCol & mergedMask) | (As<UInt4>(value) & ~mergedMask);
		}
		break;
	case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
		if((writeMask & 0x7) != 0x0)
		{
			buffer += 4 * x;

			UInt4 packedCol;
			packedCol = Insert(packedCol, r11g11b10Pack(color.x), 0);
			packedCol = Insert(packedCol, r11g11b10Pack(color.y), 1);
			packedCol = Insert(packedCol, r11g11b10Pack(color.z), 2);
			packedCol = Insert(packedCol, r11g11b10Pack(color.w), 3);

			UInt4 value;
			value = Insert(value, *Pointer<UInt>(buffer + 0), 0);
			value = Insert(value, *Pointer<UInt>(buffer + 4), 1);
			buffer += pitchB;
			value = Insert(value, *Pointer<UInt>(buffer + 0), 2);
			value = Insert(value, *Pointer<UInt>(buffer + 4), 3);

			UInt4 mask = *Pointer<UInt4>(constants + OFFSET(Constants, maskD4X[0]) + xMask * 16, 16);
			if((writeMask & 0x7) != 0x7)
			{
				mask &= *Pointer<UInt4>(constants + OFFSET(Constants, mask11X[writeMask & 0x7]), 16);
			}
			value = (packedCol & mask) | (value & ~mask);

			*Pointer<UInt>(buffer + 0) = value.z;
			*Pointer<UInt>(buffer + 4) = value.w;
			buffer -= pitchB;
			*Pointer<UInt>(buffer + 0) = value.x;
			*Pointer<UInt>(buffer + 4) = value.y;
		}
		break;
	case VK_FORMAT_R16G16B16A16_UNORM:
	case VK_FORMAT_R16G16B16A16_SINT:
	case VK_FORMAT_R16G16B16A16_UINT:
		if((writeMask & 0x0000000F) != 0x0)
		{
			buffer += 8 * x;

			UInt4 rgbaMask;
			UShort8 value = *Pointer<UShort8>(buffer);
			UShort8 packedCol = UShort8(UShort4(As<Int4>(color.x)), UShort4(As<Int4>(color.y)));
			UInt4 mergedMask = *Pointer<UInt4>(constants + OFFSET(Constants, maskQ01X) + xMask * 16);
			if((writeMask & 0xF) != 0xF)
			{
				UInt2 tmpMask = *Pointer<UInt2>(constants + OFFSET(Constants, maskW4Q[writeMask]));
				rgbaMask = UInt4(tmpMask, tmpMask);
				mergedMask &= rgbaMask;
			}
			*Pointer<UInt4>(buffer) = (As<UInt4>(packedCol) & mergedMask) | (As<UInt4>(value) & ~mergedMask);

			buffer += pitchB;

			value = *Pointer<UShort8>(buffer);
			packedCol = UShort8(UShort4(As<Int4>(color.z)), UShort4(As<Int4>(color.w)));
			mergedMask = *Pointer<UInt4>(constants + OFFSET(Constants, maskQ23X) + xMask * 16);
			if((writeMask & 0xF) != 0xF)
			{
				mergedMask &= rgbaMask;
			}
			*Pointer<UInt4>(buffer) = (As<UInt4>(packedCol) & mergedMask) | (As<UInt4>(value) & ~mergedMask);
		}
		break;
	case VK_FORMAT_B8G8R8A8_UNORM:
	case VK_FORMAT_B8G8R8A8_SRGB:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_R8G8B8A8_UINT:
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_R8G8B8A8_SRGB:
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
	case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
		if((writeMask & 0x0000000F) != 0x0)
		{
			UInt2 value, packedCol, mergedMask;

			buffer += 4 * x;

			bool isSigned = !format.isUnsigned();

			if(isSigned)
			{
				packedCol = As<UInt2>(PackSigned(Short4(As<Int4>(color.x)), Short4(As<Int4>(color.y))));
			}
			else
			{
				packedCol = As<UInt2>(PackUnsigned(Short4(As<Int4>(color.x)), Short4(As<Int4>(color.y))));
			}
			value = *Pointer<UInt2>(buffer, 16);
			mergedMask = *Pointer<UInt2>(constants + OFFSET(Constants, maskD01Q) + xMask * 8);
			if(writeMask != 0xF)
			{
				mergedMask &= *Pointer<UInt2>(constants + OFFSET(Constants, maskB4Q[writeMask]));
			}
			*Pointer<UInt2>(buffer) = (packedCol & mergedMask) | (value & ~mergedMask);

			buffer += pitchB;

			if(isSigned)
			{
				packedCol = As<UInt2>(PackSigned(Short4(As<Int4>(color.z)), Short4(As<Int4>(color.w))));
			}
			else
			{
				packedCol = As<UInt2>(PackUnsigned(Short4(As<Int4>(color.z)), Short4(As<Int4>(color.w))));
			}
			value = *Pointer<UInt2>(buffer, 16);
			mergedMask = *Pointer<UInt2>(constants + OFFSET(Constants, maskD23Q) + xMask * 8);
			if(writeMask != 0xF)
			{
				mergedMask &= *Pointer<UInt2>(constants + OFFSET(Constants, maskB4Q[writeMask]));
			}
			*Pointer<UInt2>(buffer) = (packedCol & mergedMask) | (value & ~mergedMask);
		}
		break;
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
		if((writeMask & 0x0000000F) != 0x0)
		{
			Int2 mergedMask, packedCol, value;
			Int4 packed = ((As<Int4>(color.w) & Int4(0x3)) << 30) |
			              ((As<Int4>(color.z) & Int4(0x3ff)) << 20) |
			              ((As<Int4>(color.y) & Int4(0x3ff)) << 10) |
			              ((As<Int4>(color.x) & Int4(0x3ff)));

			buffer += 4 * x;
			value = *Pointer<Int2>(buffer, 16);
			mergedMask = *Pointer<Int2>(constants + OFFSET(Constants, maskD01Q) + xMask * 8);
			if(writeMask != 0xF)
			{
				mergedMask &= *Pointer<Int2>(constants + OFFSET(Constants, mask10Q[writeMask]));
			}
			*Pointer<Int2>(buffer) = (As<Int2>(packed) & mergedMask) | (value & ~mergedMask);

			buffer += pitchB;

			value = *Pointer<Int2>(buffer, 16);
			mergedMask = *Pointer<Int2>(constants + OFFSET(Constants, maskD23Q) + xMask * 8);
			if(writeMask != 0xF)
			{
				mergedMask &= *Pointer<Int2>(constants + OFFSET(Constants, mask10Q[writeMask]));
			}
			*Pointer<Int2>(buffer) = (As<Int2>(Int4(packed.zwww)) & mergedMask) | (value & ~mergedMask);
		}
		break;
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
		if((writeMask & 0x0000000F) != 0x0)
		{
			Int2 mergedMask, packedCol, value;
			Int4 packed = ((As<Int4>(color.w) & Int4(0x3)) << 30) |
			              ((As<Int4>(color.x) & Int4(0x3ff)) << 20) |
			              ((As<Int4>(color.y) & Int4(0x3ff)) << 10) |
			              ((As<Int4>(color.z) & Int4(0x3ff)));

			buffer += 4 * x;
			value = *Pointer<Int2>(buffer, 16);
			mergedMask = *Pointer<Int2>(constants + OFFSET(Constants, maskD01Q) + xMask * 8);
			if(writeMask != 0xF)
			{
				mergedMask &= *Pointer<Int2>(constants + OFFSET(Constants, mask10Q[writeMask]));
			}
			*Pointer<Int2>(buffer) = (As<Int2>(packed) & mergedMask) | (value & ~mergedMask);

			buffer += pitchB;

			value = *Pointer<Int2>(buffer, 16);
			mergedMask = *Pointer<Int2>(constants + OFFSET(Constants, maskD23Q) + xMask * 8);
			if(writeMask != 0xF)
			{
				mergedMask &= *Pointer<Int2>(constants + OFFSET(Constants, mask10Q[writeMask]));
			}
			*Pointer<Int2>(buffer) = (As<Int2>(Int4(packed.zwww)) & mergedMask) | (value & ~mergedMask);
		}
		break;
	case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
	case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
	case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
	case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
		{
			buffer += 2 * x;
			Int value = *Pointer<Int>(buffer);

			Int channelMask;
			Short4 current;
			switch(format)
			{
			case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
				channelMask = *Pointer<Int>(constants + OFFSET(Constants, mask4argbQ[writeMask][0]));
				current = (UShort4(As<Int4>(color.x)) & UShort4(0xF)) << 12 |
				          (UShort4(As<Int4>(color.y)) & UShort4(0xF)) << 8 |
				          (UShort4(As<Int4>(color.z)) & UShort4(0xF)) << 4 |
				          (UShort4(As<Int4>(color.w)) & UShort4(0xF));
				break;
			case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
				channelMask = *Pointer<Int>(constants + OFFSET(Constants, mask4argbQ[writeMask][0]));
				current = (UShort4(As<Int4>(color.z)) & UShort4(0xF)) << 12 |
				          (UShort4(As<Int4>(color.y)) & UShort4(0xF)) << 8 |
				          (UShort4(As<Int4>(color.x)) & UShort4(0xF)) << 4 |
				          (UShort4(As<Int4>(color.w)) & UShort4(0xF));
				break;
			case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
				channelMask = *Pointer<Int>(constants + OFFSET(Constants, mask4rgbaQ[writeMask][0]));
				current = (UShort4(As<Int4>(color.w)) & UShort4(0xF)) << 12 |
				          (UShort4(As<Int4>(color.x)) & UShort4(0xF)) << 8 |
				          (UShort4(As<Int4>(color.y)) & UShort4(0xF)) << 4 |
				          (UShort4(As<Int4>(color.z)) & UShort4(0xF));
				break;
			case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
				channelMask = *Pointer<Int>(constants + OFFSET(Constants, mask4rgbaQ[writeMask][0]));
				current = (UShort4(As<Int4>(color.w)) & UShort4(0xF)) << 12 |
				          (UShort4(As<Int4>(color.z)) & UShort4(0xF)) << 8 |
				          (UShort4(As<Int4>(color.y)) & UShort4(0xF)) << 4 |
				          (UShort4(As<Int4>(color.x)) & UShort4(0xF));
				break;
			default:
				UNREACHABLE("Format: %s", vk::Stringify(format).c_str());
			}

			Int c01 = Extract(As<Int2>(current), 0);
			Int mask01 = *Pointer<Int>(constants + OFFSET(Constants, maskW4Q[0][0]) + xMask * 8);
			if(writeMask != 0x0000000F)
			{
				mask01 &= channelMask;
			}
			*Pointer<Int>(buffer) = (c01 & mask01) | (value & ~mask01);

			buffer += pitchB;
			value = *Pointer<Int>(buffer);

			Int c23 = Extract(As<Int2>(current), 1);
			Int mask23 = *Pointer<Int>(constants + OFFSET(Constants, maskW4Q[0][2]) + xMask * 8);
			if(writeMask != 0x0000000F)
			{
				mask23 &= channelMask;
			}
			*Pointer<Int>(buffer) = (c23 & mask23) | (value & ~mask23);
		}
		break;
	case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
		{
			buffer += 2 * x;
			Int value = *Pointer<Int>(buffer);

			Int channelMask = *Pointer<Int>(constants + OFFSET(Constants, maskr5g5b5a1Q[writeMask][0]));
			Short4 current = (UShort4(As<Int4>(color.x)) & UShort4(0x1F)) << 11 |
			                 (UShort4(As<Int4>(color.y)) & UShort4(0x1F)) << 6 |
			                 (UShort4(As<Int4>(color.z)) & UShort4(0x1F)) << 1 |
			                 (UShort4(As<Int4>(color.w)) & UShort4(0x1));

			Int c01 = Extract(As<Int2>(current), 0);
			Int mask01 = *Pointer<Int>(constants + OFFSET(Constants, maskW4Q[0][0]) + xMask * 8);
			if(writeMask != 0x0000000F)
			{
				mask01 &= channelMask;
			}
			*Pointer<Int>(buffer) = (c01 & mask01) | (value & ~mask01);

			buffer += pitchB;
			value = *Pointer<Int>(buffer);

			Int c23 = Extract(As<Int2>(current), 1);
			Int mask23 = *Pointer<Int>(constants + OFFSET(Constants, maskW4Q[0][2]) + xMask * 8);
			if(writeMask != 0x0000000F)
			{
				mask23 &= channelMask;
			}
			*Pointer<Int>(buffer) = (c23 & mask23) | (value & ~mask23);
		}
		break;
	case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
		{
			buffer += 2 * x;
			Int value = *Pointer<Int>(buffer);

			Int channelMask = *Pointer<Int>(constants + OFFSET(Constants, maskb5g5r5a1Q[writeMask][0]));
			Short4 current = (UShort4(As<Int4>(color.z)) & UShort4(0x1F)) << 11 |
			                 (UShort4(As<Int4>(color.y)) & UShort4(0x1F)) << 6 |
			                 (UShort4(As<Int4>(color.x)) & UShort4(0x1F)) << 1 |
			                 (UShort4(As<Int4>(color.w)) & UShort4(0x1));

			Int c01 = Extract(As<Int2>(current), 0);
			Int mask01 = *Pointer<Int>(constants + OFFSET(Constants, maskW4Q[0][0]) + xMask * 8);
			if(writeMask != 0x0000000F)
			{
				mask01 &= channelMask;
			}
			*Pointer<Int>(buffer) = (c01 & mask01) | (value & ~mask01);

			buffer += pitchB;
			value = *Pointer<Int>(buffer);

			Int c23 = Extract(As<Int2>(current), 1);
			Int mask23 = *Pointer<Int>(constants + OFFSET(Constants, maskW4Q[0][2]) + xMask * 8);
			if(writeMask != 0x0000000F)
			{
				mask23 &= channelMask;
			}
			*Pointer<Int>(buffer) = (c23 & mask23) | (value & ~mask23);
		}
		break;
	case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
		{
			buffer += 2 * x;
			Int value = *Pointer<Int>(buffer);

			Int channelMask = *Pointer<Int>(constants + OFFSET(Constants, mask5551Q[writeMask][0]));
			Short4 current = (UShort4(As<Int4>(color.w)) & UShort4(0x1)) << 15 |
			                 (UShort4(As<Int4>(color.x)) & UShort4(0x1F)) << 10 |
			                 (UShort4(As<Int4>(color.y)) & UShort4(0x1F)) << 5 |
			                 (UShort4(As<Int4>(color.z)) & UShort4(0x1F));

			Int c01 = Extract(As<Int2>(current), 0);
			Int mask01 = *Pointer<Int>(constants + OFFSET(Constants, maskW4Q[0][0]) + xMask * 8);
			if(writeMask != 0x0000000F)
			{
				mask01 &= channelMask;
			}
			*Pointer<Int>(buffer) = (c01 & mask01) | (value & ~mask01);

			buffer += pitchB;
			value = *Pointer<Int>(buffer);

			Int c23 = Extract(As<Int2>(current), 1);
			Int mask23 = *Pointer<Int>(constants + OFFSET(Constants, maskW4Q[0][2]) + xMask * 8);
			if(writeMask != 0x0000000F)
			{
				mask23 &= channelMask;
			}
			*Pointer<Int>(buffer) = (c23 & mask23) | (value & ~mask23);
		}
		break;
	case VK_FORMAT_R5G6B5_UNORM_PACK16:
		{
			buffer += 2 * x;
			Int value = *Pointer<Int>(buffer);

			Int channelMask = *Pointer<Int>(constants + OFFSET(Constants, mask565Q[writeMask & 0x7][0]));
			Short4 current = (UShort4(As<Int4>(color.z)) & UShort4(0x1F)) |
			                 (UShort4(As<Int4>(color.y)) & UShort4(0x3F)) << 5 |
			                 (UShort4(As<Int4>(color.x)) & UShort4(0x1F)) << 11;

			Int c01 = Extract(As<Int2>(current), 0);
			Int mask01 = *Pointer<Int>(constants + OFFSET(Constants, maskW4Q[0][0]) + xMask * 8);
			if((writeMask & 0x00000007) != 0x00000007)
			{
				mask01 &= channelMask;
			}
			*Pointer<Int>(buffer) = (c01 & mask01) | (value & ~mask01);

			buffer += pitchB;
			value = *Pointer<Int>(buffer);

			Int c23 = Extract(As<Int2>(current), 1);
			Int mask23 = *Pointer<Int>(constants + OFFSET(Constants, maskW4Q[0][2]) + xMask * 8);
			if((writeMask & 0x00000007) != 0x00000007)
			{
				mask23 &= channelMask;
			}
			*Pointer<Int>(buffer) = (c23 & mask23) | (value & ~mask23);
		}
		break;
	case VK_FORMAT_B5G6R5_UNORM_PACK16:
		{
			buffer += 2 * x;
			Int value = *Pointer<Int>(buffer);

			Int channelMask = *Pointer<Int>(constants + OFFSET(Constants, mask565Q[writeMask & 0x7][0]));
			Short4 current = (UShort4(As<Int4>(color.x)) & UShort4(0x1F)) |
			                 (UShort4(As<Int4>(color.y)) & UShort4(0x3F)) << 5 |
			                 (UShort4(As<Int4>(color.z)) & UShort4(0x1F)) << 11;

			Int c01 = Extract(As<Int2>(current), 0);
			Int mask01 = *Pointer<Int>(constants + OFFSET(Constants, maskW4Q[0][0]) + xMask * 8);
			if((writeMask & 0x00000007) != 0x00000007)
			{
				mask01 &= channelMask;
			}
			*Pointer<Int>(buffer) = (c01 & mask01) | (value & ~mask01);

			buffer += pitchB;
			value = *Pointer<Int>(buffer);

			Int c23 = Extract(As<Int2>(current), 1);
			Int mask23 = *Pointer<Int>(constants + OFFSET(Constants, maskW4Q[0][2]) + xMask * 8);
			if((writeMask & 0x00000007) != 0x00000007)
			{
				mask23 &= channelMask;
			}
			*Pointer<Int>(buffer) = (c23 & mask23) | (value & ~mask23);
		}
		break;
	default:
		UNSUPPORTED("VkFormat: %d", int(format));
	}
}

}  // namespace sw
