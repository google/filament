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

#include "VertexRoutine.hpp"

#include "Constants.hpp"
#include "SpirvShader.hpp"
#include "Device/Clipper.hpp"
#include "Device/Renderer.hpp"
#include "Device/Vertex.hpp"
#include "System/Debug.hpp"
#include "System/Half.hpp"
#include "Vulkan/VkDevice.hpp"

namespace sw {

VertexRoutine::VertexRoutine(
    const VertexProcessor::State &state,
    const vk::PipelineLayout *pipelineLayout,
    const SpirvShader *spirvShader)
    : routine(pipelineLayout)
    , state(state)
    , spirvShader(spirvShader)
{
	spirvShader->emitProlog(&routine);
}

VertexRoutine::~VertexRoutine()
{
}

void VertexRoutine::generate()
{
	Pointer<Byte> cache = task + OFFSET(VertexTask, vertexCache);
	Pointer<Byte> vertexCache = cache + OFFSET(VertexCache, vertex);
	Pointer<UInt> tagCache = Pointer<UInt>(cache + OFFSET(VertexCache, tag));

	UInt vertexCount = *Pointer<UInt>(task + OFFSET(VertexTask, vertexCount));

	constants = device + OFFSET(vk::Device, constants);

	// Check the cache one vertex index at a time. If a hit occurs, copy from the cache to the 'vertex' output buffer.
	// On a cache miss, process a SIMD width of consecutive indices from the input batch. They're written to the cache
	// in reverse order to guarantee that the first one doesn't get evicted and can be written out.

	Do
	{
		UInt index = *batch;
		UInt cacheIndex = index & VertexCache::TAG_MASK;

		If(tagCache[cacheIndex] != index)
		{
			readInput(batch);
			program(batch, vertexCount);
			computeClipFlags();
			computeCullMask();

			writeCache(vertexCache, tagCache, batch);
		}

		Pointer<Byte> cacheEntry = vertexCache + cacheIndex * UInt((int)sizeof(Vertex));

		// For points, vertexCount is 1 per primitive, so duplicate vertex for all 3 vertices of the primitive
		for(int i = 0; i < (state.isPoint ? 3 : 1); i++)
		{
			writeVertex(vertex, cacheEntry);
			vertex += sizeof(Vertex);
		}

		batch = Pointer<UInt>(Pointer<Byte>(batch) + sizeof(uint32_t));
		vertexCount--;
	}
	Until(vertexCount == 0);

	Return();
}

void VertexRoutine::readInput(Pointer<UInt> &batch)
{
	for(int i = 0; i < MAX_INTERFACE_COMPONENTS; i += 4)
	{
		if(spirvShader->inputs[i + 0].Type != Spirv::ATTRIBTYPE_UNUSED ||
		   spirvShader->inputs[i + 1].Type != Spirv::ATTRIBTYPE_UNUSED ||
		   spirvShader->inputs[i + 2].Type != Spirv::ATTRIBTYPE_UNUSED ||
		   spirvShader->inputs[i + 3].Type != Spirv::ATTRIBTYPE_UNUSED)
		{
			Pointer<Byte> input = *Pointer<Pointer<Byte>>(data + OFFSET(DrawData, input) + sizeof(void *) * (i / 4));
			UInt stride = *Pointer<UInt>(data + OFFSET(DrawData, stride) + sizeof(uint32_t) * (i / 4));
			Int baseVertex = *Pointer<Int>(data + OFFSET(DrawData, baseVertex));
			UInt robustnessSize(0);
			if(state.robustBufferAccess)
			{
				robustnessSize = *Pointer<UInt>(data + OFFSET(DrawData, robustnessSize) + sizeof(uint32_t) * (i / 4));
			}

			auto value = readStream(input, stride, state.input[i / 4], batch, state.robustBufferAccess, robustnessSize, baseVertex);
			routine.inputs[i + 0] = value.x;
			routine.inputs[i + 1] = value.y;
			routine.inputs[i + 2] = value.z;
			routine.inputs[i + 3] = value.w;
		}
	}
}

void VertexRoutine::computeClipFlags()
{
	auto it = spirvShader->outputBuiltins.find(spv::BuiltInPosition);
	if(it != spirvShader->outputBuiltins.end())
	{
		assert(it->second.SizeInComponents == 4);
		auto &pos = routine.getVariable(it->second.Id);
		auto posX = pos[it->second.FirstComponent + 0];
		auto posY = pos[it->second.FirstComponent + 1];
		auto posZ = pos[it->second.FirstComponent + 2];
		auto posW = pos[it->second.FirstComponent + 3];

		SIMD::Int maxX = CmpLT(posW, posX);
		SIMD::Int maxY = CmpLT(posW, posY);
		SIMD::Int minX = CmpNLE(-posW, posX);
		SIMD::Int minY = CmpNLE(-posW, posY);

		clipFlags = maxX & Clipper::CLIP_RIGHT;
		clipFlags |= maxY & Clipper::CLIP_TOP;
		clipFlags |= minX & Clipper::CLIP_LEFT;
		clipFlags |= minY & Clipper::CLIP_BOTTOM;
		if(state.depthClipEnable)
		{
			// If depthClipNegativeOneToOne is enabled, depth values are in [-1, 1] instead of [0, 1].
			SIMD::Int maxZ = CmpLT(posW, posZ);
			SIMD::Int minZ = CmpNLE(state.depthClipNegativeOneToOne ? -posW : 0.0f, posZ);
			clipFlags |= maxZ & Clipper::CLIP_FAR;
			clipFlags |= minZ & Clipper::CLIP_NEAR;
		}

		SIMD::Float maxPos = As<SIMD::Float>(SIMD::Int(0x7F7FFFFF));
		SIMD::Int finiteX = CmpLE(Abs(posX), maxPos);
		SIMD::Int finiteY = CmpLE(Abs(posY), maxPos);
		SIMD::Int finiteZ = CmpLE(Abs(posZ), maxPos);

		SIMD::Int finiteXYZ = finiteX & finiteY & finiteZ;
		clipFlags |= finiteXYZ & Clipper::CLIP_FINITE;
	}
}

void VertexRoutine::computeCullMask()
{
	cullMask = Int(15);

	auto it = spirvShader->outputBuiltins.find(spv::BuiltInCullDistance);
	if(it != spirvShader->outputBuiltins.end())
	{
		auto count = spirvShader->getNumOutputCullDistances();
		for(uint32_t i = 0; i < count; i++)
		{
			const auto &distance = routine.getVariable(it->second.Id)[it->second.FirstComponent + i];
			auto mask = SignMask(CmpGE(distance, SIMD::Float(0)));
			cullMask &= mask;
		}
	}
}

Vector4f VertexRoutine::readStream(Pointer<Byte> &buffer, UInt &stride, const Stream &stream, Pointer<UInt> &batch,
                                   bool robustBufferAccess, UInt &robustnessSize, Int baseVertex)
{
	Vector4f v;
	// Because of the following rule in the Vulkan spec, we do not care if a very large negative
	// baseVertex would overflow all the way back into a valid region of the index buffer:
	// "Out-of-bounds buffer loads will return any of the following values :
	//  - Values from anywhere within the memory range(s) bound to the buffer (possibly including
	//    bytes of memory past the end of the buffer, up to the end of the bound range)."
	UInt4 offsets = (*Pointer<UInt4>(As<Pointer<UInt4>>(batch)) + As<UInt4>(Int4(baseVertex))) * UInt4(stride);

	Pointer<Byte> source0 = buffer + offsets.x;
	Pointer<Byte> source1 = buffer + offsets.y;
	Pointer<Byte> source2 = buffer + offsets.z;
	Pointer<Byte> source3 = buffer + offsets.w;

	vk::Format format(stream.format);

	UInt4 zero(0);
	if(robustBufferAccess)
	{
		// Prevent integer overflow on the addition below.
		offsets = Min(offsets, UInt4(robustnessSize));

		// "vertex input attributes are considered out of bounds if the offset of the attribute
		//  in the bound vertex buffer range plus the size of the attribute is greater than ..."
		UInt4 limits = offsets + UInt4(format.bytes());

		Pointer<Byte> zeroSource = As<Pointer<Byte>>(&zero);
		// TODO(b/141124876): Optimize for wide-vector gather operations.
		source0 = IfThenElse(limits.x > robustnessSize, zeroSource, source0);
		source1 = IfThenElse(limits.y > robustnessSize, zeroSource, source1);
		source2 = IfThenElse(limits.z > robustnessSize, zeroSource, source2);
		source3 = IfThenElse(limits.w > robustnessSize, zeroSource, source3);
	}

	int componentCount = format.componentCount();
	bool normalized = !format.isUnnormalizedInteger();
	bool isNativeFloatAttrib = (stream.attribType == Spirv::ATTRIBTYPE_FLOAT) || normalized;
	bool bgra = false;

	switch(stream.format)
	{
	case VK_FORMAT_R32_SFLOAT:
	case VK_FORMAT_R32G32_SFLOAT:
	case VK_FORMAT_R32G32B32_SFLOAT:
	case VK_FORMAT_R32G32B32A32_SFLOAT:
		{
			if(componentCount == 0)
			{
				// Null stream, all default components
			}
			else
			{
				if(componentCount == 1)
				{
					v.x.x = *Pointer<Float>(source0);
					v.x.y = *Pointer<Float>(source1);
					v.x.z = *Pointer<Float>(source2);
					v.x.w = *Pointer<Float>(source3);
				}
				else
				{
					v.x = *Pointer<Float4>(source0);
					v.y = *Pointer<Float4>(source1);
					v.z = *Pointer<Float4>(source2);
					v.w = *Pointer<Float4>(source3);

					transpose4xN(v.x, v.y, v.z, v.w, componentCount);
				}
			}
		}
		break;
	case VK_FORMAT_B8G8R8A8_UNORM:
		bgra = true;
		// [[fallthrough]]
	case VK_FORMAT_R8_UNORM:
	case VK_FORMAT_R8G8_UNORM:
	case VK_FORMAT_R8G8B8A8_UNORM:
	case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
		v.x = Float4(*Pointer<Byte4>(source0));
		v.y = Float4(*Pointer<Byte4>(source1));
		v.z = Float4(*Pointer<Byte4>(source2));
		v.w = Float4(*Pointer<Byte4>(source3));

		transpose4xN(v.x, v.y, v.z, v.w, componentCount);

		if(componentCount >= 1) v.x *= (1.0f / 0xFF);
		if(componentCount >= 2) v.y *= (1.0f / 0xFF);
		if(componentCount >= 3) v.z *= (1.0f / 0xFF);
		if(componentCount >= 4) v.w *= (1.0f / 0xFF);
		break;
	case VK_FORMAT_R8_UINT:
	case VK_FORMAT_R8G8_UINT:
	case VK_FORMAT_R8G8B8A8_UINT:
	case VK_FORMAT_A8B8G8R8_UINT_PACK32:
		v.x = As<Float4>(Int4(*Pointer<Byte4>(source0)));
		v.y = As<Float4>(Int4(*Pointer<Byte4>(source1)));
		v.z = As<Float4>(Int4(*Pointer<Byte4>(source2)));
		v.w = As<Float4>(Int4(*Pointer<Byte4>(source3)));

		transpose4xN(v.x, v.y, v.z, v.w, componentCount);
		break;
	case VK_FORMAT_R8_SNORM:
	case VK_FORMAT_R8G8_SNORM:
	case VK_FORMAT_R8G8B8A8_SNORM:
	case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
		v.x = Float4(*Pointer<SByte4>(source0));
		v.y = Float4(*Pointer<SByte4>(source1));
		v.z = Float4(*Pointer<SByte4>(source2));
		v.w = Float4(*Pointer<SByte4>(source3));

		transpose4xN(v.x, v.y, v.z, v.w, componentCount);

		if(componentCount >= 1) v.x = Max(v.x * (1.0f / 0x7F), Float4(-1.0f));
		if(componentCount >= 2) v.y = Max(v.y * (1.0f / 0x7F), Float4(-1.0f));
		if(componentCount >= 3) v.z = Max(v.z * (1.0f / 0x7F), Float4(-1.0f));
		if(componentCount >= 4) v.w = Max(v.w * (1.0f / 0x7F), Float4(-1.0f));
		break;
	case VK_FORMAT_R8_USCALED:
	case VK_FORMAT_R8G8_USCALED:
	case VK_FORMAT_R8G8B8A8_USCALED:
	case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
		v.x = Float4(*Pointer<Byte4>(source0));
		v.y = Float4(*Pointer<Byte4>(source1));
		v.z = Float4(*Pointer<Byte4>(source2));
		v.w = Float4(*Pointer<Byte4>(source3));

		transpose4xN(v.x, v.y, v.z, v.w, componentCount);
		break;
	case VK_FORMAT_R8_SSCALED:
	case VK_FORMAT_R8G8_SSCALED:
	case VK_FORMAT_R8G8B8A8_SSCALED:
	case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
		v.x = Float4(*Pointer<SByte4>(source0));
		v.y = Float4(*Pointer<SByte4>(source1));
		v.z = Float4(*Pointer<SByte4>(source2));
		v.w = Float4(*Pointer<SByte4>(source3));

		transpose4xN(v.x, v.y, v.z, v.w, componentCount);
		break;
	case VK_FORMAT_R8_SINT:
	case VK_FORMAT_R8G8_SINT:
	case VK_FORMAT_R8G8B8A8_SINT:
	case VK_FORMAT_A8B8G8R8_SINT_PACK32:
		v.x = As<Float4>(Int4(*Pointer<SByte4>(source0)));
		v.y = As<Float4>(Int4(*Pointer<SByte4>(source1)));
		v.z = As<Float4>(Int4(*Pointer<SByte4>(source2)));
		v.w = As<Float4>(Int4(*Pointer<SByte4>(source3)));

		transpose4xN(v.x, v.y, v.z, v.w, componentCount);
		break;
	case VK_FORMAT_R16_UNORM:
	case VK_FORMAT_R16G16_UNORM:
	case VK_FORMAT_R16G16B16A16_UNORM:
		v.x = Float4(*Pointer<UShort4>(source0));
		v.y = Float4(*Pointer<UShort4>(source1));
		v.z = Float4(*Pointer<UShort4>(source2));
		v.w = Float4(*Pointer<UShort4>(source3));

		transpose4xN(v.x, v.y, v.z, v.w, componentCount);

		if(componentCount >= 1) v.x *= (1.0f / 0xFFFF);
		if(componentCount >= 2) v.y *= (1.0f / 0xFFFF);
		if(componentCount >= 3) v.z *= (1.0f / 0xFFFF);
		if(componentCount >= 4) v.w *= (1.0f / 0xFFFF);
		break;
	case VK_FORMAT_R16_SNORM:
	case VK_FORMAT_R16G16_SNORM:
	case VK_FORMAT_R16G16B16A16_SNORM:
		v.x = Float4(*Pointer<Short4>(source0));
		v.y = Float4(*Pointer<Short4>(source1));
		v.z = Float4(*Pointer<Short4>(source2));
		v.w = Float4(*Pointer<Short4>(source3));

		transpose4xN(v.x, v.y, v.z, v.w, componentCount);

		if(componentCount >= 1) v.x = Max(v.x * (1.0f / 0x7FFF), Float4(-1.0f));
		if(componentCount >= 2) v.y = Max(v.y * (1.0f / 0x7FFF), Float4(-1.0f));
		if(componentCount >= 3) v.z = Max(v.z * (1.0f / 0x7FFF), Float4(-1.0f));
		if(componentCount >= 4) v.w = Max(v.w * (1.0f / 0x7FFF), Float4(-1.0f));
		break;
	case VK_FORMAT_R16_USCALED:
	case VK_FORMAT_R16G16_USCALED:
	case VK_FORMAT_R16G16B16A16_USCALED:
		v.x = Float4(*Pointer<UShort4>(source0));
		v.y = Float4(*Pointer<UShort4>(source1));
		v.z = Float4(*Pointer<UShort4>(source2));
		v.w = Float4(*Pointer<UShort4>(source3));

		transpose4xN(v.x, v.y, v.z, v.w, componentCount);
		break;
	case VK_FORMAT_R16_SSCALED:
	case VK_FORMAT_R16G16_SSCALED:
	case VK_FORMAT_R16G16B16A16_SSCALED:
		v.x = Float4(*Pointer<Short4>(source0));
		v.y = Float4(*Pointer<Short4>(source1));
		v.z = Float4(*Pointer<Short4>(source2));
		v.w = Float4(*Pointer<Short4>(source3));

		transpose4xN(v.x, v.y, v.z, v.w, componentCount);
		break;
	case VK_FORMAT_R16_SINT:
	case VK_FORMAT_R16G16_SINT:
	case VK_FORMAT_R16G16B16A16_SINT:
		v.x = As<Float4>(Int4(*Pointer<Short4>(source0)));
		v.y = As<Float4>(Int4(*Pointer<Short4>(source1)));
		v.z = As<Float4>(Int4(*Pointer<Short4>(source2)));
		v.w = As<Float4>(Int4(*Pointer<Short4>(source3)));

		transpose4xN(v.x, v.y, v.z, v.w, componentCount);
		break;
	case VK_FORMAT_R16_UINT:
	case VK_FORMAT_R16G16_UINT:
	case VK_FORMAT_R16G16B16A16_UINT:
		v.x = As<Float4>(Int4(*Pointer<UShort4>(source0)));
		v.y = As<Float4>(Int4(*Pointer<UShort4>(source1)));
		v.z = As<Float4>(Int4(*Pointer<UShort4>(source2)));
		v.w = As<Float4>(Int4(*Pointer<UShort4>(source3)));

		transpose4xN(v.x, v.y, v.z, v.w, componentCount);
		break;
	case VK_FORMAT_R32_SINT:
	case VK_FORMAT_R32G32_SINT:
	case VK_FORMAT_R32G32B32_SINT:
	case VK_FORMAT_R32G32B32A32_SINT:
		v.x = *Pointer<Float4>(source0);
		v.y = *Pointer<Float4>(source1);
		v.z = *Pointer<Float4>(source2);
		v.w = *Pointer<Float4>(source3);

		transpose4xN(v.x, v.y, v.z, v.w, componentCount);
		break;
	case VK_FORMAT_R32_UINT:
	case VK_FORMAT_R32G32_UINT:
	case VK_FORMAT_R32G32B32_UINT:
	case VK_FORMAT_R32G32B32A32_UINT:
		v.x = *Pointer<Float4>(source0);
		v.y = *Pointer<Float4>(source1);
		v.z = *Pointer<Float4>(source2);
		v.w = *Pointer<Float4>(source3);

		transpose4xN(v.x, v.y, v.z, v.w, componentCount);
		break;
	case VK_FORMAT_R16_SFLOAT:
	case VK_FORMAT_R16G16_SFLOAT:
	case VK_FORMAT_R16G16B16A16_SFLOAT:
		{
			if(componentCount >= 1)
			{
				UShort x0 = *Pointer<UShort>(source0 + 0);
				UShort x1 = *Pointer<UShort>(source1 + 0);
				UShort x2 = *Pointer<UShort>(source2 + 0);
				UShort x3 = *Pointer<UShort>(source3 + 0);

				v.x.x = *Pointer<Float>(constants + OFFSET(Constants, half2float) + Int(x0) * 4);
				v.x.y = *Pointer<Float>(constants + OFFSET(Constants, half2float) + Int(x1) * 4);
				v.x.z = *Pointer<Float>(constants + OFFSET(Constants, half2float) + Int(x2) * 4);
				v.x.w = *Pointer<Float>(constants + OFFSET(Constants, half2float) + Int(x3) * 4);
			}

			if(componentCount >= 2)
			{
				UShort y0 = *Pointer<UShort>(source0 + 2);
				UShort y1 = *Pointer<UShort>(source1 + 2);
				UShort y2 = *Pointer<UShort>(source2 + 2);
				UShort y3 = *Pointer<UShort>(source3 + 2);

				v.y.x = *Pointer<Float>(constants + OFFSET(Constants, half2float) + Int(y0) * 4);
				v.y.y = *Pointer<Float>(constants + OFFSET(Constants, half2float) + Int(y1) * 4);
				v.y.z = *Pointer<Float>(constants + OFFSET(Constants, half2float) + Int(y2) * 4);
				v.y.w = *Pointer<Float>(constants + OFFSET(Constants, half2float) + Int(y3) * 4);
			}

			if(componentCount >= 3)
			{
				UShort z0 = *Pointer<UShort>(source0 + 4);
				UShort z1 = *Pointer<UShort>(source1 + 4);
				UShort z2 = *Pointer<UShort>(source2 + 4);
				UShort z3 = *Pointer<UShort>(source3 + 4);

				v.z.x = *Pointer<Float>(constants + OFFSET(Constants, half2float) + Int(z0) * 4);
				v.z.y = *Pointer<Float>(constants + OFFSET(Constants, half2float) + Int(z1) * 4);
				v.z.z = *Pointer<Float>(constants + OFFSET(Constants, half2float) + Int(z2) * 4);
				v.z.w = *Pointer<Float>(constants + OFFSET(Constants, half2float) + Int(z3) * 4);
			}

			if(componentCount >= 4)
			{
				UShort w0 = *Pointer<UShort>(source0 + 6);
				UShort w1 = *Pointer<UShort>(source1 + 6);
				UShort w2 = *Pointer<UShort>(source2 + 6);
				UShort w3 = *Pointer<UShort>(source3 + 6);

				v.w.x = *Pointer<Float>(constants + OFFSET(Constants, half2float) + Int(w0) * 4);
				v.w.y = *Pointer<Float>(constants + OFFSET(Constants, half2float) + Int(w1) * 4);
				v.w.z = *Pointer<Float>(constants + OFFSET(Constants, half2float) + Int(w2) * 4);
				v.w.w = *Pointer<Float>(constants + OFFSET(Constants, half2float) + Int(w3) * 4);
			}
		}
		break;
	case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
		bgra = true;
		// [[fallthrough]]
	case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
		{
			Int4 src;
			src = Insert(src, *Pointer<Int>(source0), 0);
			src = Insert(src, *Pointer<Int>(source1), 1);
			src = Insert(src, *Pointer<Int>(source2), 2);
			src = Insert(src, *Pointer<Int>(source3), 3);
			v.x = Float4((src << 22) >> 22);
			v.y = Float4((src << 12) >> 22);
			v.z = Float4((src << 02) >> 22);
			v.w = Float4(src >> 30);

			v.x = Max(v.x * Float4(1.0f / 0x1FF), Float4(-1.0f));
			v.y = Max(v.y * Float4(1.0f / 0x1FF), Float4(-1.0f));
			v.z = Max(v.z * Float4(1.0f / 0x1FF), Float4(-1.0f));
			v.w = Max(v.w, Float4(-1.0f));
		}
		break;
	case VK_FORMAT_A2R10G10B10_SINT_PACK32:
		bgra = true;
		// [[fallthrough]]
	case VK_FORMAT_A2B10G10R10_SINT_PACK32:
		{
			Int4 src;
			src = Insert(src, *Pointer<Int>(source0), 0);
			src = Insert(src, *Pointer<Int>(source1), 1);
			src = Insert(src, *Pointer<Int>(source2), 2);
			src = Insert(src, *Pointer<Int>(source3), 3);
			v.x = As<Float4>((src << 22) >> 22);
			v.y = As<Float4>((src << 12) >> 22);
			v.z = As<Float4>((src << 02) >> 22);
			v.w = As<Float4>(src >> 30);
		}
		break;
	case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
		bgra = true;
		// [[fallthrough]]
	case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
		{
			Int4 src;
			src = Insert(src, *Pointer<Int>(source0), 0);
			src = Insert(src, *Pointer<Int>(source1), 1);
			src = Insert(src, *Pointer<Int>(source2), 2);
			src = Insert(src, *Pointer<Int>(source3), 3);

			v.x = Float4(src & Int4(0x3FF));
			v.y = Float4((src >> 10) & Int4(0x3FF));
			v.z = Float4((src >> 20) & Int4(0x3FF));
			v.w = Float4((src >> 30) & Int4(0x3));

			v.x *= Float4(1.0f / 0x3FF);
			v.y *= Float4(1.0f / 0x3FF);
			v.z *= Float4(1.0f / 0x3FF);
			v.w *= Float4(1.0f / 0x3);
		}
		break;
	case VK_FORMAT_A2R10G10B10_UINT_PACK32:
		bgra = true;
		// [[fallthrough]]
	case VK_FORMAT_A2B10G10R10_UINT_PACK32:
		{
			Int4 src;
			src = Insert(src, *Pointer<Int>(source0), 0);
			src = Insert(src, *Pointer<Int>(source1), 1);
			src = Insert(src, *Pointer<Int>(source2), 2);
			src = Insert(src, *Pointer<Int>(source3), 3);

			v.x = As<Float4>(src & Int4(0x3FF));
			v.y = As<Float4>((src >> 10) & Int4(0x3FF));
			v.z = As<Float4>((src >> 20) & Int4(0x3FF));
			v.w = As<Float4>((src >> 30) & Int4(0x3));
		}
		break;
	default:
		UNSUPPORTED("stream.format %d", int(stream.format));
	}

	if(bgra)
	{
		// Swap red and blue
		Float4 t = v.x;
		v.x = v.z;
		v.z = t;
	}

	if(componentCount < 1) v.x = Float4(0.0f);
	if(componentCount < 2) v.y = Float4(0.0f);
	if(componentCount < 3) v.z = Float4(0.0f);
	if(componentCount < 4) v.w = isNativeFloatAttrib ? As<Float4>(Float4(1.0f)) : As<Float4>(Int4(1));

	return v;
}

void VertexRoutine::writeCache(Pointer<Byte> &vertexCache, Pointer<UInt> &tagCache, Pointer<UInt> &batch)
{
	ASSERT(SIMD::Width == 4);

	UInt index0 = batch[0];
	UInt index1 = batch[1];
	UInt index2 = batch[2];
	UInt index3 = batch[3];

	UInt cacheIndex0 = index0 & VertexCache::TAG_MASK;
	UInt cacheIndex1 = index1 & VertexCache::TAG_MASK;
	UInt cacheIndex2 = index2 & VertexCache::TAG_MASK;
	UInt cacheIndex3 = index3 & VertexCache::TAG_MASK;

	// We processed a SIMD group of vertices, with the first one being the one that missed the cache tag check.
	// Write them out in reverse order here and below to ensure the first one is now guaranteed to be in the cache.
	tagCache[cacheIndex3] = index3;
	tagCache[cacheIndex2] = index2;
	tagCache[cacheIndex1] = index1;
	tagCache[cacheIndex0] = index0;

	auto it = spirvShader->outputBuiltins.find(spv::BuiltInPosition);
	if(it != spirvShader->outputBuiltins.end())
	{
		assert(it->second.SizeInComponents == 4);
		auto &position = routine.getVariable(it->second.Id);

		SIMD::Float4 pos;
		pos.x = position[it->second.FirstComponent + 0];
		pos.y = position[it->second.FirstComponent + 1];
		pos.z = position[it->second.FirstComponent + 2];
		pos.w = position[it->second.FirstComponent + 3];

		// Projection and viewport transform.
		SIMD::Float w = As<SIMD::Float>(As<SIMD::Int>(pos.w) | (As<SIMD::Int>(CmpEQ(pos.w, 0.0f)) & As<SIMD::Int>(SIMD::Float(1.0f))));
		SIMD::Float rhw = 1.0f / w;

		SIMD::Float4 proj;
		proj.x = As<Float4>(RoundIntClamped(SIMD::Float(*Pointer<Float>(data + OFFSET(DrawData, X0xF))) + pos.x * rhw * SIMD::Float(*Pointer<Float>(data + OFFSET(DrawData, WxF)))));
		proj.y = As<Float4>(RoundIntClamped(SIMD::Float(*Pointer<Float>(data + OFFSET(DrawData, Y0xF))) + pos.y * rhw * SIMD::Float(*Pointer<Float>(data + OFFSET(DrawData, HxF)))));
		proj.z = pos.z * rhw;
		proj.w = rhw;

		Float4 pos_x = Extract128(pos.x, 0);
		Float4 pos_y = Extract128(pos.y, 0);
		Float4 pos_z = Extract128(pos.z, 0);
		Float4 pos_w = Extract128(pos.w, 0);
		transpose4x4(pos_x, pos_y, pos_z, pos_w);

		*Pointer<Float4>(vertexCache + sizeof(Vertex) * cacheIndex3 + OFFSET(Vertex, position), 16) = pos_w;
		*Pointer<Float4>(vertexCache + sizeof(Vertex) * cacheIndex2 + OFFSET(Vertex, position), 16) = pos_z;
		*Pointer<Float4>(vertexCache + sizeof(Vertex) * cacheIndex1 + OFFSET(Vertex, position), 16) = pos_y;
		*Pointer<Float4>(vertexCache + sizeof(Vertex) * cacheIndex0 + OFFSET(Vertex, position), 16) = pos_x;

		*Pointer<Int>(vertexCache + sizeof(Vertex) * cacheIndex3 + OFFSET(Vertex, clipFlags)) = Extract(clipFlags, 3);
		*Pointer<Int>(vertexCache + sizeof(Vertex) * cacheIndex2 + OFFSET(Vertex, clipFlags)) = Extract(clipFlags, 2);
		*Pointer<Int>(vertexCache + sizeof(Vertex) * cacheIndex1 + OFFSET(Vertex, clipFlags)) = Extract(clipFlags, 1);
		*Pointer<Int>(vertexCache + sizeof(Vertex) * cacheIndex0 + OFFSET(Vertex, clipFlags)) = Extract(clipFlags, 0);

		Float4 proj_x = Extract128(proj.x, 0);
		Float4 proj_y = Extract128(proj.y, 0);
		Float4 proj_z = Extract128(proj.z, 0);
		Float4 proj_w = Extract128(proj.w, 0);
		transpose4x4(proj_x, proj_y, proj_z, proj_w);

		*Pointer<Float4>(vertexCache + sizeof(Vertex) * cacheIndex3 + OFFSET(Vertex, projected), 16) = proj_w;
		*Pointer<Float4>(vertexCache + sizeof(Vertex) * cacheIndex2 + OFFSET(Vertex, projected), 16) = proj_z;
		*Pointer<Float4>(vertexCache + sizeof(Vertex) * cacheIndex1 + OFFSET(Vertex, projected), 16) = proj_y;
		*Pointer<Float4>(vertexCache + sizeof(Vertex) * cacheIndex0 + OFFSET(Vertex, projected), 16) = proj_x;
	}

	it = spirvShader->outputBuiltins.find(spv::BuiltInPointSize);
	if(it != spirvShader->outputBuiltins.end())
	{
		ASSERT(it->second.SizeInComponents == 1);
		auto psize = routine.getVariable(it->second.Id)[it->second.FirstComponent];

		*Pointer<Float>(vertexCache + sizeof(Vertex) * cacheIndex3 + OFFSET(Vertex, pointSize)) = Extract(psize, 3);
		*Pointer<Float>(vertexCache + sizeof(Vertex) * cacheIndex2 + OFFSET(Vertex, pointSize)) = Extract(psize, 2);
		*Pointer<Float>(vertexCache + sizeof(Vertex) * cacheIndex1 + OFFSET(Vertex, pointSize)) = Extract(psize, 1);
		*Pointer<Float>(vertexCache + sizeof(Vertex) * cacheIndex0 + OFFSET(Vertex, pointSize)) = Extract(psize, 0);
	}

	it = spirvShader->outputBuiltins.find(spv::BuiltInClipDistance);
	if(it != spirvShader->outputBuiltins.end())
	{
		auto count = spirvShader->getNumOutputClipDistances();
		for(unsigned int i = 0; i < count; i++)
		{
			auto dist = routine.getVariable(it->second.Id)[it->second.FirstComponent + i];
			*Pointer<Float>(vertexCache + sizeof(Vertex) * cacheIndex3 + OFFSET(Vertex, clipDistance[i])) = Extract(dist, 3);
			*Pointer<Float>(vertexCache + sizeof(Vertex) * cacheIndex2 + OFFSET(Vertex, clipDistance[i])) = Extract(dist, 2);
			*Pointer<Float>(vertexCache + sizeof(Vertex) * cacheIndex1 + OFFSET(Vertex, clipDistance[i])) = Extract(dist, 1);
			*Pointer<Float>(vertexCache + sizeof(Vertex) * cacheIndex0 + OFFSET(Vertex, clipDistance[i])) = Extract(dist, 0);
		}
	}

	it = spirvShader->outputBuiltins.find(spv::BuiltInCullDistance);
	if(it != spirvShader->outputBuiltins.end())
	{
		auto count = spirvShader->getNumOutputCullDistances();
		for(unsigned int i = 0; i < count; i++)
		{
			auto dist = routine.getVariable(it->second.Id)[it->second.FirstComponent + i];
			*Pointer<Float>(vertexCache + sizeof(Vertex) * cacheIndex3 + OFFSET(Vertex, cullDistance[i])) = Extract(dist, 3);
			*Pointer<Float>(vertexCache + sizeof(Vertex) * cacheIndex2 + OFFSET(Vertex, cullDistance[i])) = Extract(dist, 2);
			*Pointer<Float>(vertexCache + sizeof(Vertex) * cacheIndex1 + OFFSET(Vertex, cullDistance[i])) = Extract(dist, 1);
			*Pointer<Float>(vertexCache + sizeof(Vertex) * cacheIndex0 + OFFSET(Vertex, cullDistance[i])) = Extract(dist, 0);
		}
	}

	*Pointer<Int>(vertexCache + sizeof(Vertex) * cacheIndex3 + OFFSET(Vertex, cullMask)) = -((cullMask >> 3) & 1);
	*Pointer<Int>(vertexCache + sizeof(Vertex) * cacheIndex2 + OFFSET(Vertex, cullMask)) = -((cullMask >> 2) & 1);
	*Pointer<Int>(vertexCache + sizeof(Vertex) * cacheIndex1 + OFFSET(Vertex, cullMask)) = -((cullMask >> 1) & 1);
	*Pointer<Int>(vertexCache + sizeof(Vertex) * cacheIndex0 + OFFSET(Vertex, cullMask)) = -((cullMask >> 0) & 1);

	for(int i = 0; i < MAX_INTERFACE_COMPONENTS; i += 4)
	{
		if(spirvShader->outputs[i + 0].Type != Spirv::ATTRIBTYPE_UNUSED ||
		   spirvShader->outputs[i + 1].Type != Spirv::ATTRIBTYPE_UNUSED ||
		   spirvShader->outputs[i + 2].Type != Spirv::ATTRIBTYPE_UNUSED ||
		   spirvShader->outputs[i + 3].Type != Spirv::ATTRIBTYPE_UNUSED)
		{
			Vector4f v;
			v.x = Extract128(routine.outputs[i + 0], 0);
			v.y = Extract128(routine.outputs[i + 1], 0);
			v.z = Extract128(routine.outputs[i + 2], 0);
			v.w = Extract128(routine.outputs[i + 3], 0);

			transpose4x4(v.x, v.y, v.z, v.w);

			*Pointer<Float4>(vertexCache + sizeof(Vertex) * cacheIndex3 + OFFSET(Vertex, v[i]), 16) = v.w;
			*Pointer<Float4>(vertexCache + sizeof(Vertex) * cacheIndex2 + OFFSET(Vertex, v[i]), 16) = v.z;
			*Pointer<Float4>(vertexCache + sizeof(Vertex) * cacheIndex1 + OFFSET(Vertex, v[i]), 16) = v.y;
			*Pointer<Float4>(vertexCache + sizeof(Vertex) * cacheIndex0 + OFFSET(Vertex, v[i]), 16) = v.x;
		}
	}
}

void VertexRoutine::writeVertex(const Pointer<Byte> &vertex, Pointer<Byte> &cacheEntry)
{
	*Pointer<Int4>(vertex + OFFSET(Vertex, position)) = *Pointer<Int4>(cacheEntry + OFFSET(Vertex, position));
	*Pointer<Int>(vertex + OFFSET(Vertex, pointSize)) = *Pointer<Int>(cacheEntry + OFFSET(Vertex, pointSize));

	*Pointer<Int>(vertex + OFFSET(Vertex, clipFlags)) = *Pointer<Int>(cacheEntry + OFFSET(Vertex, clipFlags));
	*Pointer<Int>(vertex + OFFSET(Vertex, cullMask)) = *Pointer<Int>(cacheEntry + OFFSET(Vertex, cullMask));
	*Pointer<Int4>(vertex + OFFSET(Vertex, projected)) = *Pointer<Int4>(cacheEntry + OFFSET(Vertex, projected));

	for(int i = 0; i < MAX_INTERFACE_COMPONENTS; i++)
	{
		if(spirvShader->outputs[i].Type != Spirv::ATTRIBTYPE_UNUSED)
		{
			*Pointer<Int>(vertex + OFFSET(Vertex, v[i]), 4) = *Pointer<Int>(cacheEntry + OFFSET(Vertex, v[i]), 4);
		}
	}
	for(unsigned int i = 0; i < spirvShader->getNumOutputClipDistances(); i++)
	{
		*Pointer<Float>(vertex + OFFSET(Vertex, clipDistance[i]), 4) = *Pointer<Float>(cacheEntry + OFFSET(Vertex, clipDistance[i]), 4);
	}
	for(unsigned int i = 0; i < spirvShader->getNumOutputCullDistances(); i++)
	{
		*Pointer<Float>(vertex + OFFSET(Vertex, cullDistance[i]), 4) = *Pointer<Float>(cacheEntry + OFFSET(Vertex, cullDistance[i]), 4);
	}
}

}  // namespace sw
