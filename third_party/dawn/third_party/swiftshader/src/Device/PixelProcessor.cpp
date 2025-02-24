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

#include "PixelProcessor.hpp"

#include "Primitive.hpp"
#include "Pipeline/Constants.hpp"
#include "Pipeline/PixelProgram.hpp"
#include "System/Debug.hpp"
#include "Vulkan/VkImageView.hpp"
#include "Vulkan/VkPipelineLayout.hpp"

#include <cstring>

namespace sw {

uint32_t PixelProcessor::States::computeHash()
{
	uint32_t *state = reinterpret_cast<uint32_t *>(this);
	uint32_t hash = 0;

	for(unsigned int i = 0; i < sizeof(States) / sizeof(uint32_t); i++)
	{
		hash ^= state[i];
	}

	return hash;
}

bool PixelProcessor::State::operator==(const State &state) const
{
	if(hash != state.hash)
	{
		return false;
	}

	return *static_cast<const States *>(this) == static_cast<const States &>(state);
}

PixelProcessor::PixelProcessor()
{
	setRoutineCacheSize(1024);
}

void PixelProcessor::setBlendConstant(const float4 &blendConstant)
{
	for(int i = 0; i < 4; i++)
	{
		factor.blendConstantF[i] = blendConstant[i];
		factor.invBlendConstantF[i] = 1.0f - blendConstant[i];
		factor.blendConstantU[i] = clamp(blendConstant[i], 0.0f, 1.0f);
		factor.invBlendConstantU[i] = 1.0f - clamp(blendConstant[i], 0.0f, 1.0f);
		factor.blendConstantS[i] = clamp(blendConstant[i], -1.0f, 1.0f);
		factor.invBlendConstantS[i] = 1.0f - clamp(blendConstant[i], -1.0f, 1.0f);
	}
}

void PixelProcessor::setRoutineCacheSize(int cacheSize)
{
	routineCache = std::make_unique<RoutineCacheType>(clamp(cacheSize, 1, 65536));
}

const PixelProcessor::State PixelProcessor::update(const vk::GraphicsState &pipelineState, const sw::SpirvShader *fragmentShader, const sw::SpirvShader *vertexShader, const vk::Attachments &attachments, bool occlusionEnabled) const
{
	const vk::VertexInputInterfaceState &vertexInputInterfaceState = pipelineState.getVertexInputInterfaceState();
	const vk::PreRasterizationState &preRasterizationState = pipelineState.getPreRasterizationState();
	const vk::FragmentState &fragmentState = pipelineState.getFragmentState();
	const vk::FragmentOutputInterfaceState &fragmentOutputInterfaceState = pipelineState.getFragmentOutputInterfaceState();

	State state;

	state.numClipDistances = vertexShader->getNumOutputClipDistances();
	state.numCullDistances = vertexShader->getNumOutputCullDistances();

	if(fragmentShader)
	{
		state.shaderID = fragmentShader->getIdentifier();
		state.pipelineLayoutIdentifier = fragmentState.getPipelineLayout()->identifier;
		state.robustBufferAccess = fragmentShader->getRobustBufferAccess();
	}
	else
	{
		state.shaderID = 0;
		state.pipelineLayoutIdentifier = 0;
		state.robustBufferAccess = false;
	}

	state.alphaToCoverage = fragmentOutputInterfaceState.hasAlphaToCoverage();
	state.depthWriteEnable = fragmentState.depthWriteActive(attachments);

	if(fragmentState.stencilActive(attachments))
	{
		state.stencilActive = true;
		state.frontStencil = fragmentState.getFrontStencil();
		state.backStencil = fragmentState.getBackStencil();
	}

	state.depthFormat = attachments.depthFormat();
	state.depthBoundsTestActive = fragmentState.depthBoundsTestActive(attachments);
	state.minDepthBounds = fragmentState.getMinDepthBounds();
	state.maxDepthBounds = fragmentState.getMaxDepthBounds();

	if(fragmentState.depthTestActive(attachments))
	{
		state.depthTestActive = true;
		state.depthCompareMode = fragmentState.getDepthCompareMode();

		state.depthBias = preRasterizationState.getConstantDepthBias() != 0.0f || preRasterizationState.getSlopeDepthBias() != 0.0f;

		bool pipelineDepthClamp = preRasterizationState.getDepthClampEnable();
		// "For fixed-point depth buffers, fragment depth values are always limited to the range [0,1] by clamping after depth bias addition is performed.
		//  Unless the VK_EXT_depth_range_unrestricted extension is enabled, fragment depth values are clamped even when the depth buffer uses a floating-point representation."
		state.depthClamp = pipelineDepthClamp || !state.depthFormat.isFloatFormat() || !preRasterizationState.hasDepthRangeUnrestricted();

		if(pipelineDepthClamp)
		{
			const VkViewport viewport = preRasterizationState.getViewport();
			state.minDepthClamp = min(viewport.minDepth, viewport.maxDepth);
			state.maxDepthClamp = max(viewport.minDepth, viewport.maxDepth);
		}
		else if(state.depthClamp)
		{
			state.minDepthClamp = 0.0f;
			state.maxDepthClamp = 1.0f;
		}
	}

	state.occlusionEnabled = occlusionEnabled;

	bool fragmentContainsDiscard = (fragmentShader && fragmentShader->getAnalysis().ContainsDiscard);
	for(uint32_t location = 0; location < MAX_COLOR_BUFFERS; location++)
	{
		state.colorFormat[location] = attachments.colorFormat(location);

		state.colorWriteMask |= fragmentOutputInterfaceState.colorWriteActive(location, attachments) << (4 * location);
		state.blendState[location] = fragmentOutputInterfaceState.getBlendState(location, attachments, fragmentContainsDiscard);
	}

	const bool isBresenhamLine = vertexInputInterfaceState.isDrawLine(true, preRasterizationState.getPolygonMode()) &&
	                             preRasterizationState.getLineRasterizationMode() == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT;

	state.multiSampleCount = static_cast<unsigned int>(fragmentOutputInterfaceState.getSampleCount());
	state.multiSampleMask = fragmentOutputInterfaceState.getMultiSampleMask();
	state.enableMultiSampling = state.multiSampleCount > 1 && !isBresenhamLine;

	// SampleId and SamplePosition require per-sample fragment shader invocations, so the Vulkan spec
	// requires turning on sample shading if either of them is present in the shader:
	// "If a fragment shader entry point's interface includes an input variable decorated with SampleId,
	//  Sample Shading is considered enabled with a minSampleShading value of 1.0."
	// "If a fragment shader entry point's interface includes an input variable decorated with SamplePosition,
	//  Sample Shading is considered enabled with a minSampleShading value of 1.0."
	bool shaderContainsSampleDecoration = fragmentShader && (fragmentShader->hasBuiltinInput(spv::BuiltInSampleId) ||
	                                                         fragmentShader->hasBuiltinInput(spv::BuiltInSamplePosition));

	if(shaderContainsSampleDecoration)
	{
		state.sampleShadingEnabled = true;
		state.minSampleShading = 1.0f;
	}
	else
	{
		state.sampleShadingEnabled = fragmentOutputInterfaceState.hasSampleShadingEnabled();
		state.minSampleShading = fragmentOutputInterfaceState.getMinSampleShading();
	}

	if(state.enableMultiSampling && fragmentShader)
	{
		state.centroid = fragmentShader->getAnalysis().NeedsCentroid;
	}

	state.frontFace = preRasterizationState.getFrontFace();

	state.hash = state.computeHash();

	return state;
}

PixelProcessor::RoutineType PixelProcessor::routine(const State &state,
                                                    const vk::PipelineLayout *pipelineLayout,
                                                    const SpirvShader *pixelShader,
                                                    const vk::Attachments &attachments,
                                                    const vk::DescriptorSet::Bindings &descriptorSets)
{
	auto routine = routineCache->lookup(state);

	if(!routine)
	{
		QuadRasterizer *generator = new PixelProgram(state, pipelineLayout, pixelShader, attachments, descriptorSets);
		generator->generate();
		routine = (*generator)("PixelRoutine_%0.8X", state.shaderID);
		delete generator;

		routineCache->add(state, routine);
	}

	return routine;
}

}  // namespace sw
