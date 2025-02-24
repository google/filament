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

#include "SetupProcessor.hpp"

#include "Polygon.hpp"
#include "Primitive.hpp"
#include "Renderer.hpp"
#include "Pipeline/Constants.hpp"
#include "Pipeline/SetupRoutine.hpp"
#include "Pipeline/SpirvShader.hpp"
#include "System/Debug.hpp"
#include "Vulkan/VkImageView.hpp"

#include <cstring>

namespace sw {

uint32_t SetupProcessor::States::computeHash()
{
	uint32_t *state = reinterpret_cast<uint32_t *>(this);
	uint32_t hash = 0;

	for(unsigned int i = 0; i < sizeof(States) / sizeof(uint32_t); i++)
	{
		hash ^= state[i];
	}

	return hash;
}

bool SetupProcessor::State::operator==(const State &state) const
{
	if(hash != state.hash)
	{
		return false;
	}

	return *static_cast<const States *>(this) == static_cast<const States &>(state);
}

SetupProcessor::SetupProcessor()
{
	setRoutineCacheSize(1024);
}

SetupProcessor::State SetupProcessor::update(const vk::GraphicsState &pipelineState, const sw::SpirvShader *fragmentShader, const sw::SpirvShader *vertexShader, const vk::Attachments &attachments) const
{
	const vk::VertexInputInterfaceState &vertexInputInterfaceState = pipelineState.getVertexInputInterfaceState();
	const vk::PreRasterizationState &preRasterizationState = pipelineState.getPreRasterizationState();
	const vk::FragmentState &fragmentState = pipelineState.getFragmentState();
	const vk::FragmentOutputInterfaceState &fragmentOutputInterfaceState = pipelineState.getFragmentOutputInterfaceState();

	State state;

	bool vPosZW = (fragmentShader && fragmentShader->hasBuiltinInput(spv::BuiltInFragCoord));

	const VkPolygonMode polygonMode = preRasterizationState.getPolygonMode();

	state.isDrawPoint = vertexInputInterfaceState.isDrawPoint(true, polygonMode);
	state.isDrawLine = vertexInputInterfaceState.isDrawLine(true, polygonMode);
	state.isDrawTriangle = vertexInputInterfaceState.isDrawTriangle(true, polygonMode);
	state.fixedPointDepthBuffer = attachments.depthBuffer && !attachments.depthBuffer->getFormat(VK_IMAGE_ASPECT_DEPTH_BIT).isFloatFormat();
	state.applyConstantDepthBias = vertexInputInterfaceState.isDrawTriangle(false, polygonMode) && (preRasterizationState.getConstantDepthBias() != 0.0f);
	state.applySlopeDepthBias = vertexInputInterfaceState.isDrawTriangle(false, polygonMode) && (preRasterizationState.getSlopeDepthBias() != 0.0f);
	state.applyDepthBiasClamp = vertexInputInterfaceState.isDrawTriangle(false, polygonMode) && (preRasterizationState.getDepthBiasClamp() != 0.0f);
	state.interpolateZ = fragmentState.depthTestActive(attachments) || vPosZW;
	state.interpolateW = fragmentShader != nullptr;
	state.frontFace = preRasterizationState.getFrontFace();
	state.cullMode = preRasterizationState.getCullMode();

	const bool isBresenhamLine = vertexInputInterfaceState.isDrawLine(true, preRasterizationState.getPolygonMode()) &&
	                             preRasterizationState.getLineRasterizationMode() == VK_LINE_RASTERIZATION_MODE_BRESENHAM_EXT;

	state.multiSampleCount = fragmentOutputInterfaceState.getSampleCount();
	state.enableMultiSampling = state.multiSampleCount > 1 && !isBresenhamLine;

	state.numClipDistances = vertexShader->getNumOutputClipDistances();
	state.numCullDistances = vertexShader->getNumOutputCullDistances();

	if(fragmentShader)
	{
		for(int interpolant = 0; interpolant < MAX_INTERFACE_COMPONENTS; interpolant++)
		{
			state.gradient[interpolant] = fragmentShader->inputs[interpolant];
		}
	}

	state.hash = state.computeHash();

	return state;
}

SetupProcessor::RoutineType SetupProcessor::routine(const State &state)
{
	auto routine = routineCache->lookup(state);

	if(!routine)
	{
		SetupRoutine *generator = new SetupRoutine(state);
		generator->generate();
		routine = generator->getRoutine();
		delete generator;

		routineCache->add(state, routine);
	}

	return routine;
}

void SetupProcessor::setRoutineCacheSize(int cacheSize)
{
	routineCache = std::make_unique<RoutineCacheType>(clamp(cacheSize, 1, 65536));
}

}  // namespace sw
