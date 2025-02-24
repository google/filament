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

#ifndef sw_PixelProcessor_hpp
#define sw_PixelProcessor_hpp

#include "Context.hpp"
#include "Memset.hpp"
#include "RoutineCache.hpp"
#include "Vulkan/VkFormat.hpp"

#include <memory>

namespace sw {

struct DrawData;
struct Primitive;
class SpirvShader;

using RasterizerFunction = FunctionT<void(const vk::Device *device, const Primitive *primitive, int count, int cluster, int clusterCount, DrawData *draw)>;

class PixelProcessor
{
public:
	struct States : Memset<States>
	{
		// Same as VkStencilOpState, but with no reference, as it's not part of the state
		// (it doesn't require a different program to be generated)
		struct StencilOpState
		{
			VkStencilOp failOp;
			VkStencilOp passOp;
			VkStencilOp depthFailOp;
			VkCompareOp compareOp;
			bool useCompareMask;
			bool useWriteMask;
			bool writeEnabled;

			void operator=(const VkStencilOpState &rhs)
			{
				failOp = rhs.failOp;
				passOp = rhs.passOp;
				depthFailOp = rhs.depthFailOp;
				compareOp = rhs.compareOp;
				useCompareMask = (rhs.compareMask != 0xff);
				useWriteMask = ((rhs.writeMask & 0xFF) != 0xFF);
				writeEnabled = (rhs.writeMask != 0);
			}
		};

		States()
		    : Memset(this, 0)
		{}

		uint32_t computeHash();

		uint64_t shaderID;
		uint32_t pipelineLayoutIdentifier;

		unsigned int numClipDistances;
		unsigned int numCullDistances;

		VkCompareOp depthCompareMode;
		bool depthWriteEnable;

		bool robustBufferAccess;

		bool stencilActive;
		StencilOpState frontStencil;
		StencilOpState backStencil;

		bool depthTestActive;
		bool depthBoundsTestActive;
		bool occlusionEnabled;
		bool perspective;

		vk::BlendState blendState[MAX_COLOR_BUFFERS];

		unsigned int colorWriteMask;
		vk::Format colorFormat[MAX_COLOR_BUFFERS];
		unsigned int multiSampleCount;
		unsigned int multiSampleMask;
		bool enableMultiSampling;
		bool alphaToCoverage;
		bool centroid;
		bool sampleShadingEnabled;
		float minSampleShading;
		float minDepthBounds;
		float maxDepthBounds;
		VkFrontFace frontFace;
		vk::Format depthFormat;
		bool depthBias;
		bool depthClamp;

		float minDepthClamp;
		float maxDepthClamp;
	};

	struct State : States
	{
		bool operator==(const State &state) const;

		int colorWriteActive(int index) const
		{
			return (colorWriteMask >> (index * 4)) & 0xF;
		}

		uint32_t hash;
	};

	struct Stencil
	{
		int64_t testMaskQ;
		int64_t referenceMaskedQ;
		int64_t referenceMaskedSignedQ;
		int64_t writeMaskQ;
		int64_t invWriteMaskQ;
		int64_t referenceQ;

		void set(int reference, int testMask, int writeMask)
		{
			referenceQ = replicate(reference);
			testMaskQ = replicate(testMask);
			writeMaskQ = replicate(writeMask);
			invWriteMaskQ = ~writeMaskQ;
			referenceMaskedQ = referenceQ & testMaskQ;
			referenceMaskedSignedQ = replicate(((reference & testMask) + 0x80) & 0xFF);
		}

		static int64_t replicate(int b)
		{
			int64_t w = b & 0xFF;

			return (w << 0) | (w << 8) | (w << 16) | (w << 24) | (w << 32) | (w << 40) | (w << 48) | (w << 56);
		}
	};

	struct Factor
	{
		float4 blendConstantF;     // Unclamped for floating-point attachment formats.
		float4 invBlendConstantF;  // Unclamped for floating-point attachment formats.
		float4 blendConstantU;     // Clamped to [0,1] for unsigned fixed-point attachment formats.
		float4 invBlendConstantU;  // Clamped to [0,1] for unsigned fixed-point attachment formats.
		float4 blendConstantS;     // Clamped to [-1,1] for signed fixed-point attachment formats.
		float4 invBlendConstantS;  // Clamped to [-1,1] for signed fixed-point attachment formats.
	};

public:
	using RoutineType = RasterizerFunction::RoutineType;

	PixelProcessor();

	void setBlendConstant(const float4 &blendConstant);

	const State update(const vk::GraphicsState &pipelineState, const sw::SpirvShader *fragmentShader, const sw::SpirvShader *vertexShader, const vk::Attachments &attachments, bool occlusionEnabled) const;
	RoutineType routine(const State &state, const vk::PipelineLayout *pipelineLayout,
	                    const SpirvShader *pixelShader, const vk::Attachments &attachments, const vk::DescriptorSet::Bindings &descriptorSets);
	void setRoutineCacheSize(int routineCacheSize);

	// Other semi-constants
	Factor factor;

private:
	using RoutineCacheType = RoutineCache<State, RasterizerFunction::CFunctionType>;
	std::unique_ptr<RoutineCacheType> routineCache;
};

}  // namespace sw

namespace std {

template<>
struct hash<sw::PixelProcessor::State>
{
	uint64_t operator()(const sw::PixelProcessor::State &state) const
	{
		return state.hash;
	}
};

}  // namespace std

#endif  // sw_PixelProcessor_hpp
