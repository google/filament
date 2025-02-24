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

#include "QuadRasterizer.hpp"

#include "Primitive.hpp"
#include "Renderer.hpp"
#include "Pipeline/Constants.hpp"
#include "System/Debug.hpp"
#include "System/Math.hpp"
#include "Vulkan/VkDevice.hpp"

namespace sw {

QuadRasterizer::QuadRasterizer(const PixelProcessor::State &state, const SpirvShader *spirvShader)
    : state(state)
    , spirvShader{ spirvShader }
{
}

QuadRasterizer::~QuadRasterizer()
{
}

void QuadRasterizer::generate()
{
	constants = device + OFFSET(vk::Device, constants);
	occlusion = 0;

	Do
	{
		Int yMin = *Pointer<Int>(primitive + OFFSET(Primitive, yMin));
		Int yMax = *Pointer<Int>(primitive + OFFSET(Primitive, yMax));

		Int cluster2 = cluster + cluster;
		yMin += clusterCount * 2 - 2 - cluster2;
		yMin &= -clusterCount * 2;
		yMin += cluster2;

		If(yMin < yMax)
		{
			rasterize(yMin, yMax);
		}

		primitive += sizeof(Primitive) * state.multiSampleCount;
		count--;
	}
	Until(count == 0);

	if(state.occlusionEnabled)
	{
		UInt clusterOcclusion = *Pointer<UInt>(data + OFFSET(DrawData, occlusion) + 4 * cluster);
		clusterOcclusion += occlusion;
		*Pointer<UInt>(data + OFFSET(DrawData, occlusion) + 4 * cluster) = clusterOcclusion;
	}

	Return();
}

void QuadRasterizer::rasterize(Int &yMin, Int &yMax)
{
	Pointer<Byte> cBuffer[MAX_COLOR_BUFFERS];
	Pointer<Byte> zBuffer;
	Pointer<Byte> sBuffer;

	Int clusterCountLog2 = 31 - Ctlz(UInt(clusterCount), false);

	for(int index = 0; index < MAX_COLOR_BUFFERS; index++)
	{
		if(state.colorWriteActive(index))
		{
			cBuffer[index] = *Pointer<Pointer<Byte>>(data + OFFSET(DrawData, colorBuffer[index])) + yMin * *Pointer<Int>(data + OFFSET(DrawData, colorPitchB[index]));
		}
	}

	if(state.depthTestActive || state.depthBoundsTestActive)
	{
		zBuffer = *Pointer<Pointer<Byte>>(data + OFFSET(DrawData, depthBuffer)) + yMin * *Pointer<Int>(data + OFFSET(DrawData, depthPitchB));
	}

	if(state.stencilActive)
	{
		sBuffer = *Pointer<Pointer<Byte>>(data + OFFSET(DrawData, stencilBuffer)) + yMin * *Pointer<Int>(data + OFFSET(DrawData, stencilPitchB));
	}

	Int y = yMin;

	Do
	{
		Int x0a = Int(*Pointer<Short>(primitive + OFFSET(Primitive, outline->left) + (y + 0) * sizeof(Primitive::Span)));
		Int x0b = Int(*Pointer<Short>(primitive + OFFSET(Primitive, outline->left) + (y + 1) * sizeof(Primitive::Span)));
		Int x0 = Min(x0a, x0b);

		for(unsigned int q = 1; q < state.multiSampleCount; q++)
		{
			x0a = Int(*Pointer<Short>(primitive + q * sizeof(Primitive) + OFFSET(Primitive, outline->left) + (y + 0) * sizeof(Primitive::Span)));
			x0b = Int(*Pointer<Short>(primitive + q * sizeof(Primitive) + OFFSET(Primitive, outline->left) + (y + 1) * sizeof(Primitive::Span)));
			x0 = Min(x0, Min(x0a, x0b));
		}

		x0 &= 0xFFFFFFFE;

		Int x1a = Int(*Pointer<Short>(primitive + OFFSET(Primitive, outline->right) + (y + 0) * sizeof(Primitive::Span)));
		Int x1b = Int(*Pointer<Short>(primitive + OFFSET(Primitive, outline->right) + (y + 1) * sizeof(Primitive::Span)));
		Int x1 = Max(x1a, x1b);

		for(unsigned int q = 1; q < state.multiSampleCount; q++)
		{
			x1a = Int(*Pointer<Short>(primitive + q * sizeof(Primitive) + OFFSET(Primitive, outline->right) + (y + 0) * sizeof(Primitive::Span)));
			x1b = Int(*Pointer<Short>(primitive + q * sizeof(Primitive) + OFFSET(Primitive, outline->right) + (y + 1) * sizeof(Primitive::Span)));
			x1 = Max(x1, Max(x1a, x1b));
		}

		// Compute the y coordinate of each fragment in the SIMD group.
		const auto yMorton = SIMD::Float([](int i) { return float(compactEvenBits(i >> 1)); });  // 0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 3, 3, 2, 2, 3, 3, ...
		yFragment = SIMD::Float(Float(y)) + yMorton - SIMD::Float(*Pointer<Float>(primitive + OFFSET(Primitive, y0)));

		if(interpolateZ())
		{
			for(unsigned int q = 0; q < state.multiSampleCount; q++)
			{
				SIMD::Float y = yFragment;

				if(state.enableMultiSampling)
				{
					y += SIMD::Float(*Pointer<Float>(constants + OFFSET(Constants, SampleLocationsY) + q * sizeof(float)));
				}

				Dz[q] = SIMD::Float(*Pointer<Float>(primitive + OFFSET(Primitive, z.C))) + y * SIMD::Float(*Pointer<Float>(primitive + OFFSET(Primitive, z.B)));
			}
		}

		If(x0 < x1)
		{
			if(interpolateW())
			{
				Dw = SIMD::Float(*Pointer<Float>(primitive + OFFSET(Primitive, w.C))) + yFragment * SIMD::Float(*Pointer<Float>(primitive + OFFSET(Primitive, w.B)));
			}

			if(spirvShader)
			{
				int packedInterpolant = 0;
				for(int interfaceInterpolant = 0; interfaceInterpolant < MAX_INTERFACE_COMPONENTS; interfaceInterpolant++)
				{
					if(spirvShader->inputs[interfaceInterpolant].Type != SpirvShader::ATTRIBTYPE_UNUSED)
					{
						Dv[interfaceInterpolant] = *Pointer<Float>(primitive + OFFSET(Primitive, V[packedInterpolant].C));
						if(!spirvShader->inputs[interfaceInterpolant].Flat)
						{
							Dv[interfaceInterpolant] +=
							    yFragment * SIMD::Float(*Pointer<Float>(primitive + OFFSET(Primitive, V[packedInterpolant].B)));
						}
						packedInterpolant++;
					}
				}

				for(unsigned int i = 0; i < state.numClipDistances; i++)
				{
					DclipDistance[i] = SIMD::Float(*Pointer<Float>(primitive + OFFSET(Primitive, clipDistance[i].C))) +
					                   yFragment * SIMD::Float(*Pointer<Float>(primitive + OFFSET(Primitive, clipDistance[i].B)));
				}

				for(unsigned int i = 0; i < state.numCullDistances; i++)
				{
					DcullDistance[i] = SIMD::Float(*Pointer<Float>(primitive + OFFSET(Primitive, cullDistance[i].C))) +
					                   yFragment * SIMD::Float(*Pointer<Float>(primitive + OFFSET(Primitive, cullDistance[i].B)));
				}
			}

			Short4 xLeft[4];
			Short4 xRight[4];

			for(unsigned int q = 0; q < state.multiSampleCount; q++)
			{
				xLeft[q] = *Pointer<Short4>(primitive + q * sizeof(Primitive) + OFFSET(Primitive, outline) + y * sizeof(Primitive::Span));
				xRight[q] = xLeft[q];

				xLeft[q] = Swizzle(xLeft[q], 0x0022) - Short4(1, 2, 1, 2);
				xRight[q] = Swizzle(xRight[q], 0x1133) - Short4(0, 1, 0, 1);
			}

			For(Int x = x0, x < x1, x += 2)
			{
				Short4 xxxx = Short4(x);
				Int cMask[4];

				for(unsigned int q = 0; q < state.multiSampleCount; q++)
				{
					if(state.multiSampleMask & (1 << q))
					{
						unsigned int i = state.enableMultiSampling ? q : 0;
						Short4 mask = CmpGT(xxxx, xLeft[i]) & CmpGT(xRight[i], xxxx);
						cMask[q] = SignMask(PackSigned(mask, mask)) & 0x0000000F;
					}
				}

				quad(cBuffer, zBuffer, sBuffer, cMask, x, y);
			}
		}

		for(int index = 0; index < MAX_COLOR_BUFFERS; index++)
		{
			if(state.colorWriteActive(index))
			{
				cBuffer[index] += *Pointer<Int>(data + OFFSET(DrawData, colorPitchB[index])) << (1 + clusterCountLog2);  // FIXME: Precompute
			}
		}

		if(state.depthTestActive || state.depthBoundsTestActive)
		{
			zBuffer += *Pointer<Int>(data + OFFSET(DrawData, depthPitchB)) << (1 + clusterCountLog2);  // FIXME: Precompute
		}

		if(state.stencilActive)
		{
			sBuffer += *Pointer<Int>(data + OFFSET(DrawData, stencilPitchB)) << (1 + clusterCountLog2);  // FIXME: Precompute
		}

		y += 2 * clusterCount;
	}
	Until(y >= yMax);
}

SIMD::Float QuadRasterizer::interpolate(SIMD::Float &x, SIMD::Float &D, SIMD::Float &rhw, Pointer<Byte> planeEquation, bool flat, bool perspective)
{
	if(flat)
	{
		return D;
	}

	SIMD::Float interpolant = mulAdd(x, SIMD::Float(*Pointer<Float>(planeEquation + OFFSET(PlaneEquation, A))), D);

	if(perspective)
	{
		interpolant *= rhw;
	}

	return interpolant;
}

bool QuadRasterizer::interpolateZ() const
{
	return state.depthTestActive || (spirvShader && spirvShader->hasBuiltinInput(spv::BuiltInFragCoord));
}

bool QuadRasterizer::interpolateW() const
{
	// Note: could optimize cases where there is a fragment shader but it has no
	// perspective-correct inputs, but that's vanishingly rare.
	return spirvShader != nullptr;
}

}  // namespace sw
