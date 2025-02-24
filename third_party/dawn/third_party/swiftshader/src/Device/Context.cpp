// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
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

#include "Context.hpp"

#include "Vulkan/VkBuffer.hpp"
#include "Vulkan/VkDevice.hpp"
#include "Vulkan/VkImageView.hpp"
#include "Vulkan/VkPipeline.hpp"
#include "Vulkan/VkRenderPass.hpp"
#include "Vulkan/VkStringify.hpp"

namespace {

uint32_t ComputePrimitiveCount(VkPrimitiveTopology topology, uint32_t vertexCount)
{
	switch(topology)
	{
	case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
		return vertexCount;
	case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
		return vertexCount / 2;
	case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
		return std::max<uint32_t>(vertexCount, 1) - 1;
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
		return vertexCount / 3;
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
		return std::max<uint32_t>(vertexCount, 2) - 2;
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
		return std::max<uint32_t>(vertexCount, 2) - 2;
	default:
		UNSUPPORTED("VkPrimitiveTopology %d", int(topology));
	}

	return 0;
}

template<typename T>
void ProcessPrimitiveRestart(T *indexBuffer,
                             VkPrimitiveTopology topology,
                             uint32_t count,
                             std::vector<std::pair<uint32_t, void *>> *indexBuffers)
{
	static const T RestartIndex = static_cast<T>(-1);
	T *indexBufferStart = indexBuffer;
	uint32_t vertexCount = 0;
	for(uint32_t i = 0; i < count; i++)
	{
		if(indexBuffer[i] == RestartIndex)
		{
			// Record previous segment
			if(vertexCount > 0)
			{
				uint32_t primitiveCount = ComputePrimitiveCount(topology, vertexCount);
				if(primitiveCount > 0)
				{
					indexBuffers->push_back({ primitiveCount, indexBufferStart });
				}
			}
			vertexCount = 0;
		}
		else
		{
			if(vertexCount == 0)
			{
				indexBufferStart = indexBuffer + i;
			}
			vertexCount++;
		}
	}

	// Record last segment
	if(vertexCount > 0)
	{
		uint32_t primitiveCount = ComputePrimitiveCount(topology, vertexCount);
		if(primitiveCount > 0)
		{
			indexBuffers->push_back({ primitiveCount, indexBufferStart });
		}
	}
}

vk::InputsDynamicStateFlags ParseInputsDynamicStateFlags(const VkPipelineDynamicStateCreateInfo *dynamicStateCreateInfo)
{
	vk::InputsDynamicStateFlags dynamicStateFlags = {};

	if(dynamicStateCreateInfo == nullptr)
	{
		return dynamicStateFlags;
	}

	for(uint32_t i = 0; i < dynamicStateCreateInfo->dynamicStateCount; i++)
	{
		VkDynamicState dynamicState = dynamicStateCreateInfo->pDynamicStates[i];
		switch(dynamicState)
		{
		case VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE:
			dynamicStateFlags.dynamicVertexInputBindingStride = true;
			break;
		case VK_DYNAMIC_STATE_VERTEX_INPUT_EXT:
			dynamicStateFlags.dynamicVertexInput = true;
			dynamicStateFlags.dynamicVertexInputBindingStride = true;
			break;

		default:
			// The rest of the dynamic state is handled by ParseDynamicStateFlags.
			break;
		}
	}

	return dynamicStateFlags;
}

vk::DynamicStateFlags ParseDynamicStateFlags(const VkPipelineDynamicStateCreateInfo *dynamicStateCreateInfo)
{
	vk::DynamicStateFlags dynamicStateFlags = {};

	if(dynamicStateCreateInfo == nullptr)
	{
		return dynamicStateFlags;
	}

	if(dynamicStateCreateInfo->flags != 0)
	{
		// Vulkan 1.3: "flags is reserved for future use." "flags must be 0"
		UNSUPPORTED("dynamicStateCreateInfo->flags 0x%08X", int(dynamicStateCreateInfo->flags));
	}

	for(uint32_t i = 0; i < dynamicStateCreateInfo->dynamicStateCount; i++)
	{
		VkDynamicState dynamicState = dynamicStateCreateInfo->pDynamicStates[i];
		switch(dynamicState)
		{
		// Vertex input interface:
		case VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE:
			dynamicStateFlags.vertexInputInterface.dynamicPrimitiveRestartEnable = true;
			break;
		case VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY:
			dynamicStateFlags.vertexInputInterface.dynamicPrimitiveTopology = true;
			break;
		case VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE:
		case VK_DYNAMIC_STATE_VERTEX_INPUT_EXT:
			// Handled by ParseInputsDynamicStateFlags
			break;

		// Pre-rasterization:
		case VK_DYNAMIC_STATE_LINE_WIDTH:
			dynamicStateFlags.preRasterization.dynamicLineWidth = true;
			break;
		case VK_DYNAMIC_STATE_DEPTH_BIAS:
			dynamicStateFlags.preRasterization.dynamicDepthBias = true;
			break;
		case VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE:
			dynamicStateFlags.preRasterization.dynamicDepthBiasEnable = true;
			break;
		case VK_DYNAMIC_STATE_CULL_MODE:
			dynamicStateFlags.preRasterization.dynamicCullMode = true;
			break;
		case VK_DYNAMIC_STATE_FRONT_FACE:
			dynamicStateFlags.preRasterization.dynamicFrontFace = true;
			break;
		case VK_DYNAMIC_STATE_VIEWPORT:
			dynamicStateFlags.preRasterization.dynamicViewport = true;
			break;
		case VK_DYNAMIC_STATE_SCISSOR:
			dynamicStateFlags.preRasterization.dynamicScissor = true;
			break;
		case VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT:
			dynamicStateFlags.preRasterization.dynamicViewportWithCount = true;
			break;
		case VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT:
			dynamicStateFlags.preRasterization.dynamicScissorWithCount = true;
			break;
		case VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE:
			dynamicStateFlags.preRasterization.dynamicRasterizerDiscardEnable = true;
			break;

		// Fragment:
		case VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE:
			dynamicStateFlags.fragment.dynamicDepthTestEnable = true;
			break;
		case VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE:
			dynamicStateFlags.fragment.dynamicDepthWriteEnable = true;
			break;
		case VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE:
			dynamicStateFlags.fragment.dynamicDepthBoundsTestEnable = true;
			break;
		case VK_DYNAMIC_STATE_DEPTH_BOUNDS:
			dynamicStateFlags.fragment.dynamicDepthBounds = true;
			break;
		case VK_DYNAMIC_STATE_DEPTH_COMPARE_OP:
			dynamicStateFlags.fragment.dynamicDepthCompareOp = true;
			break;
		case VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE:
			dynamicStateFlags.fragment.dynamicStencilTestEnable = true;
			break;
		case VK_DYNAMIC_STATE_STENCIL_OP:
			dynamicStateFlags.fragment.dynamicStencilOp = true;
			break;
		case VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK:
			dynamicStateFlags.fragment.dynamicStencilCompareMask = true;
			break;
		case VK_DYNAMIC_STATE_STENCIL_WRITE_MASK:
			dynamicStateFlags.fragment.dynamicStencilWriteMask = true;
			break;
		case VK_DYNAMIC_STATE_STENCIL_REFERENCE:
			dynamicStateFlags.fragment.dynamicStencilReference = true;
			break;

		// Fragment output interface:
		case VK_DYNAMIC_STATE_BLEND_CONSTANTS:
			dynamicStateFlags.fragmentOutputInterface.dynamicBlendConstants = true;
			break;

		default:
			UNSUPPORTED("VkDynamicState %d", int(dynamicState));
		}
	}

	return dynamicStateFlags;
}
}  // namespace

namespace vk {

uint32_t IndexBuffer::bytesPerIndex() const
{
	return indexType == VK_INDEX_TYPE_UINT8_EXT ? 1u : indexType == VK_INDEX_TYPE_UINT16 ? 2u
	                                                                                     : 4u;
}

void IndexBuffer::setIndexBufferBinding(const VertexInputBinding &indexBufferBinding, VkIndexType type)
{
	binding = indexBufferBinding;
	indexType = type;
}

void IndexBuffer::getIndexBuffers(VkPrimitiveTopology topology, uint32_t count, uint32_t first, bool indexed, bool hasPrimitiveRestartEnable, std::vector<std::pair<uint32_t, void *>> *indexBuffers) const
{
	if(indexed)
	{
		const VkDeviceSize bufferSize = binding.buffer->getSize();
		if(binding.offset >= bufferSize)
		{
			return;  // Nothing to draw
		}

		const VkDeviceSize maxIndices = (bufferSize - binding.offset) / bytesPerIndex();
		if(first > maxIndices)
		{
			return;  // Nothing to draw
		}

		void *indexBuffer = binding.buffer->getOffsetPointer(binding.offset + first * bytesPerIndex());
		if(hasPrimitiveRestartEnable)
		{
			switch(indexType)
			{
			case VK_INDEX_TYPE_UINT8_EXT:
				ProcessPrimitiveRestart(static_cast<uint8_t *>(indexBuffer), topology, count, indexBuffers);
				break;
			case VK_INDEX_TYPE_UINT16:
				ProcessPrimitiveRestart(static_cast<uint16_t *>(indexBuffer), topology, count, indexBuffers);
				break;
			case VK_INDEX_TYPE_UINT32:
				ProcessPrimitiveRestart(static_cast<uint32_t *>(indexBuffer), topology, count, indexBuffers);
				break;
			default:
				UNSUPPORTED("VkIndexType %d", int(indexType));
			}
		}
		else
		{
			indexBuffers->push_back({ ComputePrimitiveCount(topology, count), indexBuffer });
		}
	}
	else
	{
		indexBuffers->push_back({ ComputePrimitiveCount(topology, count), nullptr });
	}
}

VkFormat Attachments::colorFormat(int location) const
{
	ASSERT((location >= 0) && (location < sw::MAX_COLOR_BUFFERS));

	if(colorBuffer[location])
	{
		return colorBuffer[location]->getFormat();
	}
	else
	{
		return VK_FORMAT_UNDEFINED;
	}
}

VkFormat Attachments::depthFormat() const
{
	if(depthBuffer)
	{
		return depthBuffer->getFormat();
	}
	else
	{
		return VK_FORMAT_UNDEFINED;
	}
}

VkFormat Attachments::depthStencilFormat() const
{
	if(depthBuffer)
	{
		return depthBuffer->getFormat();
	}
	else if(stencilBuffer)
	{
		return stencilBuffer->getFormat();
	}
	else
	{
		return VK_FORMAT_UNDEFINED;
	}
}

void Inputs::initialize(const VkPipelineVertexInputStateCreateInfo *vertexInputState, const VkPipelineDynamicStateCreateInfo *dynamicStateCreateInfo)
{
	dynamicStateFlags = ParseInputsDynamicStateFlags(dynamicStateCreateInfo);

	if(dynamicStateFlags.dynamicVertexInput)
	{
		return;
	}

	if(vertexInputState->flags != 0)
	{
		// Vulkan 1.2: "flags is reserved for future use." "flags must be 0"
		UNSUPPORTED("vertexInputState->flags");
	}

	// Temporary in-binding-order representation of buffer strides, to be consumed below
	// when considering attributes. TODO: unfuse buffers from attributes in backend, is old GL model.
	uint32_t vertexStrides[MAX_VERTEX_INPUT_BINDINGS];
	uint32_t instanceStrides[MAX_VERTEX_INPUT_BINDINGS];
	VkVertexInputRate inputRates[MAX_VERTEX_INPUT_BINDINGS];
	for(uint32_t i = 0; i < vertexInputState->vertexBindingDescriptionCount; i++)
	{
		const auto &desc = vertexInputState->pVertexBindingDescriptions[i];
		inputRates[desc.binding] = desc.inputRate;
		vertexStrides[desc.binding] = desc.inputRate == VK_VERTEX_INPUT_RATE_VERTEX ? desc.stride : 0;
		instanceStrides[desc.binding] = desc.inputRate == VK_VERTEX_INPUT_RATE_INSTANCE ? desc.stride : 0;
	}

	for(uint32_t i = 0; i < vertexInputState->vertexAttributeDescriptionCount; i++)
	{
		const auto &desc = vertexInputState->pVertexAttributeDescriptions[i];
		sw::Stream &input = stream[desc.location];
		input.format = desc.format;
		input.offset = desc.offset;
		input.binding = desc.binding;
		input.inputRate = inputRates[desc.binding];
		if(!dynamicStateFlags.dynamicVertexInputBindingStride)
		{
			// The following gets overriden with dynamic state anyway and setting it is
			// harmless.  But it is not done to be able to catch bugs with this dynamic
			// state easier.
			input.vertexStride = vertexStrides[desc.binding];
			input.instanceStride = instanceStrides[desc.binding];
		}
	}
}

void Inputs::updateDescriptorSets(const DescriptorSet::Array &dso,
                                  const DescriptorSet::Bindings &ds,
                                  const DescriptorSet::DynamicOffsets &ddo)
{
	descriptorSetObjects = dso;
	descriptorSets = ds;
	descriptorDynamicOffsets = ddo;
}

void Inputs::bindVertexInputs(int firstInstance)
{
	for(uint32_t i = 0; i < MAX_VERTEX_INPUT_BINDINGS; i++)
	{
		auto &attrib = stream[i];
		if(attrib.format != VK_FORMAT_UNDEFINED)
		{
			const auto &vertexInput = vertexInputBindings[attrib.binding];
			VkDeviceSize offset = attrib.offset + vertexInput.offset +
			                      getInstanceStride(i) * firstInstance;
			attrib.buffer = vertexInput.buffer ? vertexInput.buffer->getOffsetPointer(offset) : nullptr;

			VkDeviceSize size = vertexInput.buffer ? vertexInput.buffer->getSize() : 0;
			attrib.robustnessSize = (size > offset) ? size - offset : 0;
		}
	}
}

void Inputs::setVertexInputBinding(const VertexInputBinding bindings[], const DynamicState &dynamicState)
{
	for(uint32_t i = 0; i < MAX_VERTEX_INPUT_BINDINGS; ++i)
	{
		vertexInputBindings[i] = bindings[i];
	}

	if(dynamicStateFlags.dynamicVertexInput)
	{
		// If the entire vertex input state is dynamic, recalculate the contents of `stream`.
		// This is similar to Inputs::initialize.
		for(uint32_t i = 0; i < sw::MAX_INTERFACE_COMPONENTS / 4; i++)
		{
			const auto &desc = dynamicState.vertexInputAttributes[i];
			const auto &bindingDesc = dynamicState.vertexInputBindings[desc.binding];
			sw::Stream &input = stream[i];
			input.format = desc.format;
			input.offset = desc.offset;
			input.binding = desc.binding;
			input.inputRate = bindingDesc.inputRate;
		}
	}

	// Stride may come from two different dynamic states
	if(dynamicStateFlags.dynamicVertexInput || dynamicStateFlags.dynamicVertexInputBindingStride)
	{
		for(uint32_t i = 0; i < sw::MAX_INTERFACE_COMPONENTS / 4; i++)
		{
			sw::Stream &input = stream[i];
			const VkDeviceSize stride = dynamicState.vertexInputBindings[input.binding].stride;

			input.vertexStride = input.inputRate == VK_VERTEX_INPUT_RATE_VERTEX ? stride : 0;
			input.instanceStride = input.inputRate == VK_VERTEX_INPUT_RATE_INSTANCE ? stride : 0;
		}
	}
}

void Inputs::advanceInstanceAttributes()
{
	for(uint32_t i = 0; i < vk::MAX_VERTEX_INPUT_BINDINGS; i++)
	{
		auto &attrib = stream[i];

		VkDeviceSize instanceStride = getInstanceStride(i);
		if((attrib.format != VK_FORMAT_UNDEFINED) && instanceStride && (instanceStride < attrib.robustnessSize))
		{
			// Under the casts: attrib.buffer += instanceStride
			attrib.buffer = (const void *)((uintptr_t)attrib.buffer + instanceStride);
			attrib.robustnessSize -= instanceStride;
		}
	}
}

VkDeviceSize Inputs::getVertexStride(uint32_t i) const
{
	auto &attrib = stream[i];
	if(attrib.format != VK_FORMAT_UNDEFINED)
	{
		return attrib.vertexStride;
	}

	return 0;
}

VkDeviceSize Inputs::getInstanceStride(uint32_t i) const
{
	auto &attrib = stream[i];
	if(attrib.format != VK_FORMAT_UNDEFINED)
	{
		return attrib.instanceStride;
	}

	return 0;
}

void MultisampleState::set(const VkPipelineMultisampleStateCreateInfo *multisampleState)
{
	if(multisampleState->flags != 0)
	{
		// Vulkan 1.2: "flags is reserved for future use." "flags must be 0"
		UNSUPPORTED("pCreateInfo->pMultisampleState->flags 0x%08X", int(multisampleState->flags));
	}

	sampleShadingEnable = (multisampleState->sampleShadingEnable != VK_FALSE);
	if(sampleShadingEnable)
	{
		minSampleShading = multisampleState->minSampleShading;
	}

	if(multisampleState->alphaToOneEnable != VK_FALSE)
	{
		UNSUPPORTED("VkPhysicalDeviceFeatures::alphaToOne");
	}

	switch(multisampleState->rasterizationSamples)
	{
	case VK_SAMPLE_COUNT_1_BIT:
		sampleCount = 1;
		break;
	case VK_SAMPLE_COUNT_4_BIT:
		sampleCount = 4;
		break;
	default:
		UNSUPPORTED("Unsupported sample count");
	}

	VkSampleMask sampleMask;
	if(multisampleState->pSampleMask)
	{
		sampleMask = multisampleState->pSampleMask[0];
	}
	else  // "If pSampleMask is NULL, it is treated as if the mask has all bits set to 1."
	{
		sampleMask = ~0;
	}

	alphaToCoverage = (multisampleState->alphaToCoverageEnable != VK_FALSE);
	multiSampleMask = sampleMask & ((unsigned)0xFFFFFFFF >> (32 - sampleCount));
}

void VertexInputInterfaceState::initialize(const VkPipelineVertexInputStateCreateInfo *vertexInputState,
                                           const VkPipelineInputAssemblyStateCreateInfo *inputAssemblyState,
                                           const DynamicStateFlags &allDynamicStateFlags)
{
	dynamicStateFlags = allDynamicStateFlags.vertexInputInterface;

	if(vertexInputState && vertexInputState->flags != 0)
	{
		// Vulkan 1.2: "flags is reserved for future use." "flags must be 0"
		UNSUPPORTED("vertexInputState->flags");
	}

	if(inputAssemblyState->flags != 0)
	{
		// Vulkan 1.2: "flags is reserved for future use." "flags must be 0"
		UNSUPPORTED("pCreateInfo->pInputAssemblyState->flags 0x%08X", int(inputAssemblyState->flags));
	}

	primitiveRestartEnable = (inputAssemblyState->primitiveRestartEnable != VK_FALSE);
	topology = inputAssemblyState->topology;
}

void VertexInputInterfaceState::applyState(const DynamicState &dynamicState)
{
	if(dynamicStateFlags.dynamicPrimitiveRestartEnable)
	{
		primitiveRestartEnable = dynamicState.primitiveRestartEnable;
	}

	if(dynamicStateFlags.dynamicPrimitiveTopology)
	{
		topology = dynamicState.primitiveTopology;
	}
}

bool VertexInputInterfaceState::isDrawPoint(bool polygonModeAware, VkPolygonMode polygonMode) const
{
	switch(topology)
	{
	case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
		return true;
	case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
	case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
		return false;
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
		return polygonModeAware ? (polygonMode == VK_POLYGON_MODE_POINT) : false;
	default:
		UNSUPPORTED("topology %d", int(topology));
	}
	return false;
}

bool VertexInputInterfaceState::isDrawLine(bool polygonModeAware, VkPolygonMode polygonMode) const
{
	switch(topology)
	{
	case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
		return false;
	case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
	case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
		return true;
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
		return polygonModeAware ? (polygonMode == VK_POLYGON_MODE_LINE) : false;
	default:
		UNSUPPORTED("topology %d", int(topology));
	}
	return false;
}

bool VertexInputInterfaceState::isDrawTriangle(bool polygonModeAware, VkPolygonMode polygonMode) const
{
	switch(topology)
	{
	case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
	case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
	case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
		return false;
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
	case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
		return polygonModeAware ? (polygonMode == VK_POLYGON_MODE_FILL) : true;
	default:
		UNSUPPORTED("topology %d", int(topology));
	}
	return false;
}

void PreRasterizationState::initialize(const vk::Device *device,
                                       const PipelineLayout *layout,
                                       const VkPipelineViewportStateCreateInfo *viewportState,
                                       const VkPipelineRasterizationStateCreateInfo *rasterizationState,
                                       const vk::RenderPass *renderPass, uint32_t subpassIndex,
                                       const VkPipelineRenderingCreateInfo *rendering,
                                       const DynamicStateFlags &allDynamicStateFlags)
{
	pipelineLayout = layout;
	dynamicStateFlags = allDynamicStateFlags.preRasterization;

	if(rasterizationState->flags != 0)
	{
		// Vulkan 1.2: "flags is reserved for future use." "flags must be 0"
		UNSUPPORTED("pCreateInfo->pRasterizationState->flags 0x%08X", int(rasterizationState->flags));
	}

	rasterizerDiscard = rasterizationState->rasterizerDiscardEnable != VK_FALSE;
	cullMode = rasterizationState->cullMode;
	frontFace = rasterizationState->frontFace;
	polygonMode = rasterizationState->polygonMode;
	depthBiasEnable = rasterizationState->depthBiasEnable;
	constantDepthBias = rasterizationState->depthBiasConstantFactor;
	slopeDepthBias = rasterizationState->depthBiasSlopeFactor;
	depthBiasClamp = rasterizationState->depthBiasClamp;
	depthRangeUnrestricted = device->hasExtension(VK_EXT_DEPTH_RANGE_UNRESTRICTED_EXTENSION_NAME);
	depthClampEnable = rasterizationState->depthClampEnable != VK_FALSE;
	depthClipEnable = !depthClampEnable;

	// From the Vulkan spec for vkCmdSetDepthBias:
	//    The bias value O for a polygon is:
	//        O = dbclamp(...)
	//    where dbclamp(x) =
	//        * x                       depthBiasClamp = 0 or NaN
	//        * min(x, depthBiasClamp)  depthBiasClamp > 0
	//        * max(x, depthBiasClamp)  depthBiasClamp < 0
	// So it should be safe to resolve NaNs to 0.0f.
	if(std::isnan(depthBiasClamp))
	{
		depthBiasClamp = 0.0f;
	}

	if(!dynamicStateFlags.dynamicLineWidth)
	{
		lineWidth = rasterizationState->lineWidth;
	}

	const VkBaseInStructure *extensionCreateInfo = reinterpret_cast<const VkBaseInStructure *>(rasterizationState->pNext);
	while(extensionCreateInfo)
	{
		switch(extensionCreateInfo->sType)
		{
		case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_LINE_STATE_CREATE_INFO_EXT:
			{
				const VkPipelineRasterizationLineStateCreateInfoEXT *lineStateCreateInfo = reinterpret_cast<const VkPipelineRasterizationLineStateCreateInfoEXT *>(extensionCreateInfo);
				lineRasterizationMode = lineStateCreateInfo->lineRasterizationMode;
			}
			break;
		case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_PROVOKING_VERTEX_STATE_CREATE_INFO_EXT:
			{
				const VkPipelineRasterizationProvokingVertexStateCreateInfoEXT *provokingVertexModeCreateInfo =
				    reinterpret_cast<const VkPipelineRasterizationProvokingVertexStateCreateInfoEXT *>(extensionCreateInfo);
				provokingVertexMode = provokingVertexModeCreateInfo->provokingVertexMode;
			}
			break;
		case VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_DEPTH_CLIP_STATE_CREATE_INFO_EXT:
			{
				const auto *depthClipInfo = reinterpret_cast<const VkPipelineRasterizationDepthClipStateCreateInfoEXT *>(extensionCreateInfo);
				// Reserved for future use.
				ASSERT(depthClipInfo->flags == 0);
				depthClipEnable = depthClipInfo->depthClipEnable != VK_FALSE;
			}
			break;
		case VK_STRUCTURE_TYPE_APPLICATION_INFO:
			// SwiftShader doesn't interact with application info, but dEQP includes it
			break;
		case VK_STRUCTURE_TYPE_MAX_ENUM:
			// dEQP tests that this value is ignored.
			break;
		default:
			UNSUPPORTED("pCreateInfo->pRasterizationState->pNext sType = %s", vk::Stringify(extensionCreateInfo->sType).c_str());
			break;
		}

		extensionCreateInfo = extensionCreateInfo->pNext;
	}

	if(!rasterizerDiscard || dynamicStateFlags.dynamicRasterizerDiscardEnable)
	{
		extensionCreateInfo = reinterpret_cast<const VkBaseInStructure *>(viewportState->pNext);
		while(extensionCreateInfo != nullptr)
		{
			switch(extensionCreateInfo->sType)
			{
			case VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_DEPTH_CLIP_CONTROL_CREATE_INFO_EXT:
				{
					const auto *depthClipControl = reinterpret_cast<const VkPipelineViewportDepthClipControlCreateInfoEXT *>(extensionCreateInfo);
					depthClipNegativeOneToOne = depthClipControl->negativeOneToOne != VK_FALSE;
				}
				break;
			case VK_STRUCTURE_TYPE_MAX_ENUM:
				// dEQP passes this value expecting the driver to ignore it.
				break;
			default:
				UNSUPPORTED("pCreateInfo->pViewportState->pNext sType = %s", vk::Stringify(extensionCreateInfo->sType).c_str());
				break;
			}
			extensionCreateInfo = extensionCreateInfo->pNext;
		}

		if(viewportState->flags != 0)
		{
			// Vulkan 1.2: "flags is reserved for future use." "flags must be 0"
			UNSUPPORTED("pCreateInfo->pViewportState->flags 0x%08X", int(viewportState->flags));
		}

		if((viewportState->viewportCount > 1) ||
		   (viewportState->scissorCount > 1))
		{
			UNSUPPORTED("VkPhysicalDeviceFeatures::multiViewport");
		}

		if(!dynamicStateFlags.dynamicScissor && !dynamicStateFlags.dynamicScissorWithCount)
		{
			scissor = viewportState->pScissors[0];
		}

		if(!dynamicStateFlags.dynamicViewport && !dynamicStateFlags.dynamicViewportWithCount)
		{
			viewport = viewportState->pViewports[0];
		}
	}
}

void PreRasterizationState::applyState(const DynamicState &dynamicState)
{
	if(dynamicStateFlags.dynamicLineWidth)
	{
		lineWidth = dynamicState.lineWidth;
	}

	if(dynamicStateFlags.dynamicDepthBias)
	{
		constantDepthBias = dynamicState.depthBiasConstantFactor;
		slopeDepthBias = dynamicState.depthBiasSlopeFactor;
		depthBiasClamp = dynamicState.depthBiasClamp;
	}

	if(dynamicStateFlags.dynamicDepthBiasEnable)
	{
		depthBiasEnable = dynamicState.depthBiasEnable;
	}

	if(dynamicStateFlags.dynamicCullMode)
	{
		cullMode = dynamicState.cullMode;
	}

	if(dynamicStateFlags.dynamicFrontFace)
	{
		frontFace = dynamicState.frontFace;
	}

	if(dynamicStateFlags.dynamicViewport)
	{
		viewport = dynamicState.viewport;
	}

	if(dynamicStateFlags.dynamicScissor)
	{
		scissor = dynamicState.scissor;
	}

	if(dynamicStateFlags.dynamicViewportWithCount && dynamicState.viewportCount > 0)
	{
		viewport = dynamicState.viewports[0];
	}

	if(dynamicStateFlags.dynamicScissorWithCount && dynamicState.scissorCount > 0)
	{
		scissor = dynamicState.scissors[0];
	}

	if(dynamicStateFlags.dynamicRasterizerDiscardEnable)
	{
		rasterizerDiscard = dynamicState.rasterizerDiscardEnable;
	}
}

void FragmentState::initialize(
    const PipelineLayout *layout,
    const VkPipelineDepthStencilStateCreateInfo *depthStencilState,
    const vk::RenderPass *renderPass, uint32_t subpassIndex,
    const VkPipelineRenderingCreateInfo *rendering,
    const DynamicStateFlags &allDynamicStateFlags)
{
	pipelineLayout = layout;
	dynamicStateFlags = allDynamicStateFlags.fragment;

	if(renderPass)
	{
		const VkSubpassDescription &subpass = renderPass->getSubpass(subpassIndex);

		// Ignore pDepthStencilState when "the subpass of the render pass the pipeline
		// is created against does not use a depth/stencil attachment"
		if(subpass.pDepthStencilAttachment &&
		   subpass.pDepthStencilAttachment->attachment != VK_ATTACHMENT_UNUSED)
		{
			setDepthStencilState(depthStencilState);
		}
	}
	else  // No render pass
	{
		// When a pipeline is created without a VkRenderPass, if the VkPipelineRenderingCreateInfo structure
		// is present in the pNext chain of VkGraphicsPipelineCreateInfo, it specifies the view mask and
		// format of attachments used for rendering. If this structure is not specified, and the pipeline
		// does not include a VkRenderPass, viewMask and colorAttachmentCount are 0, and
		// depthAttachmentFormat and stencilAttachmentFormat are VK_FORMAT_UNDEFINED. If a graphics pipeline
		// is created with a valid VkRenderPass, parameters of this structure are ignored.

		if(rendering)
		{
			if((rendering->depthAttachmentFormat != VK_FORMAT_UNDEFINED) ||
			   (rendering->stencilAttachmentFormat != VK_FORMAT_UNDEFINED))
			{
				// If renderPass is VK_NULL_HANDLE, the pipeline is being created with fragment
				// shader state, and either of VkPipelineRenderingCreateInfo::depthAttachmentFormat
				// or VkPipelineRenderingCreateInfo::stencilAttachmentFormat are not
				// VK_FORMAT_UNDEFINED, pDepthStencilState must be a valid pointer to a valid
				// VkPipelineDepthStencilStateCreateInfo structure
				ASSERT(depthStencilState);

				setDepthStencilState(depthStencilState);
			}
		}
	}
}

void FragmentState::applyState(const DynamicState &dynamicState)
{
	if(dynamicStateFlags.dynamicDepthTestEnable)
	{
		depthTestEnable = dynamicState.depthTestEnable;
	}

	if(dynamicStateFlags.dynamicDepthWriteEnable)
	{
		depthWriteEnable = dynamicState.depthWriteEnable;
	}

	if(dynamicStateFlags.dynamicDepthBoundsTestEnable)
	{
		depthBoundsTestEnable = dynamicState.depthBoundsTestEnable;
	}

	if(dynamicStateFlags.dynamicDepthBounds && depthBoundsTestEnable)
	{
		minDepthBounds = dynamicState.minDepthBounds;
		maxDepthBounds = dynamicState.maxDepthBounds;
	}

	if(dynamicStateFlags.dynamicDepthCompareOp)
	{
		depthCompareMode = dynamicState.depthCompareOp;
	}

	if(dynamicStateFlags.dynamicStencilTestEnable)
	{
		stencilEnable = dynamicState.stencilTestEnable;
	}

	if(dynamicStateFlags.dynamicStencilOp && stencilEnable)
	{
		if(dynamicState.faceMask & VK_STENCIL_FACE_FRONT_BIT)
		{
			frontStencil.compareOp = dynamicState.frontStencil.compareOp;
			frontStencil.depthFailOp = dynamicState.frontStencil.depthFailOp;
			frontStencil.failOp = dynamicState.frontStencil.failOp;
			frontStencil.passOp = dynamicState.frontStencil.passOp;
		}

		if(dynamicState.faceMask & VK_STENCIL_FACE_BACK_BIT)
		{
			backStencil.compareOp = dynamicState.backStencil.compareOp;
			backStencil.depthFailOp = dynamicState.backStencil.depthFailOp;
			backStencil.failOp = dynamicState.backStencil.failOp;
			backStencil.passOp = dynamicState.backStencil.passOp;
		}
	}

	if(dynamicStateFlags.dynamicStencilCompareMask && stencilEnable)
	{
		frontStencil.compareMask = dynamicState.frontStencil.compareMask;
		backStencil.compareMask = dynamicState.backStencil.compareMask;
	}

	if(dynamicStateFlags.dynamicStencilWriteMask && stencilEnable)
	{
		frontStencil.writeMask = dynamicState.frontStencil.writeMask;
		backStencil.writeMask = dynamicState.backStencil.writeMask;
	}

	if(dynamicStateFlags.dynamicStencilReference && stencilEnable)
	{
		frontStencil.reference = dynamicState.frontStencil.reference;
		backStencil.reference = dynamicState.backStencil.reference;
	}
}

bool FragmentState::depthWriteActive(const Attachments &attachments) const
{
	// "Depth writes are always disabled when depthTestEnable is VK_FALSE."
	return depthTestActive(attachments) && depthWriteEnable;
}

bool FragmentState::depthTestActive(const Attachments &attachments) const
{
	return attachments.depthBuffer && depthTestEnable;
}

bool FragmentState::stencilActive(const Attachments &attachments) const
{
	return attachments.stencilBuffer && stencilEnable;
}

bool FragmentState::depthBoundsTestActive(const Attachments &attachments) const
{
	return attachments.depthBuffer && depthBoundsTestEnable;
}

void FragmentState::setDepthStencilState(const VkPipelineDepthStencilStateCreateInfo *depthStencilState)
{
	if((depthStencilState->flags &
	    ~(VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_DEPTH_ACCESS_BIT_EXT |
	      VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_STENCIL_ACCESS_BIT_EXT)) != 0)
	{
		UNSUPPORTED("depthStencilState->flags 0x%08X", int(depthStencilState->flags));
	}

	depthBoundsTestEnable = (depthStencilState->depthBoundsTestEnable != VK_FALSE);
	minDepthBounds = depthStencilState->minDepthBounds;
	maxDepthBounds = depthStencilState->maxDepthBounds;

	depthTestEnable = (depthStencilState->depthTestEnable != VK_FALSE);
	depthWriteEnable = (depthStencilState->depthWriteEnable != VK_FALSE);
	depthCompareMode = depthStencilState->depthCompareOp;

	stencilEnable = (depthStencilState->stencilTestEnable != VK_FALSE);
	if(stencilEnable)
	{
		frontStencil = depthStencilState->front;
		backStencil = depthStencilState->back;
	}
}

void FragmentOutputInterfaceState::initialize(const VkPipelineColorBlendStateCreateInfo *colorBlendState,
                                              const VkPipelineMultisampleStateCreateInfo *multisampleState,
                                              const vk::RenderPass *renderPass, uint32_t subpassIndex,
                                              const VkPipelineRenderingCreateInfo *rendering,
                                              const DynamicStateFlags &allDynamicStateFlags)
{
	dynamicStateFlags = allDynamicStateFlags.fragmentOutputInterface;

	multisample.set(multisampleState);

	if(renderPass)
	{
		const VkSubpassDescription &subpass = renderPass->getSubpass(subpassIndex);

		// Ignore pColorBlendState when "the subpass of the render pass the pipeline
		// is created against does not use any color attachments"
		for(uint32_t i = 0; i < subpass.colorAttachmentCount; i++)
		{
			if(subpass.pColorAttachments[i].attachment != VK_ATTACHMENT_UNUSED)
			{
				setColorBlendState(colorBlendState);
				break;
			}
		}
	}
	else  // No render pass
	{
		// When a pipeline is created without a VkRenderPass, if the VkPipelineRenderingCreateInfo structure
		// is present in the pNext chain of VkGraphicsPipelineCreateInfo, it specifies the view mask and
		// format of attachments used for rendering. If this structure is not specified, and the pipeline
		// does not include a VkRenderPass, viewMask and colorAttachmentCount are 0, and
		// depthAttachmentFormat and stencilAttachmentFormat are VK_FORMAT_UNDEFINED. If a graphics pipeline
		// is created with a valid VkRenderPass, parameters of this structure are ignored.

		if(rendering)
		{
			if(rendering->colorAttachmentCount > 0)
			{
				// If renderPass is VK_NULL_HANDLE, the pipeline is being created with fragment
				// output interface state, and VkPipelineRenderingCreateInfo::colorAttachmentCount
				// is not equal to 0, pColorBlendState must be a valid pointer to a valid
				// VkPipelineColorBlendStateCreateInfo structure
				ASSERT(colorBlendState);

				setColorBlendState(colorBlendState);
			}
		}
	}
}

void FragmentOutputInterfaceState::applyState(const DynamicState &dynamicState)
{
	if(dynamicStateFlags.dynamicBlendConstants)
	{
		blendConstants = dynamicState.blendConstants;
	}
}

void FragmentOutputInterfaceState::setColorBlendState(const VkPipelineColorBlendStateCreateInfo *colorBlendState)
{
	if(colorBlendState->flags != 0 &&
	   colorBlendState->flags != VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RASTERIZATION_ORDER_ATTACHMENT_ACCESS_BIT_EXT)
	{
		UNSUPPORTED("colorBlendState->flags 0x%08X", int(colorBlendState->flags));
	}

	if(colorBlendState->logicOpEnable != VK_FALSE)
	{
		UNSUPPORTED("VkPhysicalDeviceFeatures::logicOp");
	}

	if(!dynamicStateFlags.dynamicBlendConstants)
	{
		blendConstants.x = colorBlendState->blendConstants[0];
		blendConstants.y = colorBlendState->blendConstants[1];
		blendConstants.z = colorBlendState->blendConstants[2];
		blendConstants.w = colorBlendState->blendConstants[3];
	}

	const VkBaseInStructure *extensionColorBlendInfo = reinterpret_cast<const VkBaseInStructure *>(colorBlendState->pNext);
	while(extensionColorBlendInfo)
	{
		switch(extensionColorBlendInfo->sType)
		{
		case VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_ADVANCED_STATE_CREATE_INFO_EXT:
			{
				const VkPipelineColorBlendAdvancedStateCreateInfoEXT *colorBlendAdvancedCreateInfo = reinterpret_cast<const VkPipelineColorBlendAdvancedStateCreateInfoEXT *>(extensionColorBlendInfo);
				ASSERT(colorBlendAdvancedCreateInfo->blendOverlap == VK_BLEND_OVERLAP_UNCORRELATED_EXT);
				ASSERT(colorBlendAdvancedCreateInfo->dstPremultiplied == VK_TRUE);
				ASSERT(colorBlendAdvancedCreateInfo->srcPremultiplied == VK_TRUE);
			}
			break;
		case VK_STRUCTURE_TYPE_MAX_ENUM:
			// dEQP tests that this value is ignored.
			break;
		default:
			UNSUPPORTED("colorBlendState->pNext sType = %s", vk::Stringify(extensionColorBlendInfo->sType).c_str());
			break;
		}

		extensionColorBlendInfo = extensionColorBlendInfo->pNext;
	}

	ASSERT(colorBlendState->attachmentCount <= sw::MAX_COLOR_BUFFERS);
	for(auto i = 0u; i < colorBlendState->attachmentCount; i++)
	{
		const VkPipelineColorBlendAttachmentState &attachment = colorBlendState->pAttachments[i];
		colorWriteMask[i] = attachment.colorWriteMask;
		blendState[i] = { (attachment.blendEnable != VK_FALSE),
			              attachment.srcColorBlendFactor, attachment.dstColorBlendFactor, attachment.colorBlendOp,
			              attachment.srcAlphaBlendFactor, attachment.dstAlphaBlendFactor, attachment.alphaBlendOp };
	}
}

BlendState FragmentOutputInterfaceState::getBlendState(int location, const Attachments &attachments, bool fragmentContainsKill) const
{
	ASSERT((location >= 0) && (location < sw::MAX_COLOR_BUFFERS));
	const uint32_t index = attachments.locationToIndex[location];
	if(index == VK_ATTACHMENT_UNUSED)
	{
		return {};
	}

	ASSERT((index >= 0) && (index < sw::MAX_COLOR_BUFFERS));
	auto &state = blendState[index];

	BlendState activeBlendState = {};
	activeBlendState.alphaBlendEnable = alphaBlendActive(location, attachments, fragmentContainsKill);

	if(activeBlendState.alphaBlendEnable)
	{
		vk::Format format = attachments.colorBuffer[location]->getFormat(VK_IMAGE_ASPECT_COLOR_BIT);

		activeBlendState.sourceBlendFactor = blendFactor(state.blendOperation, state.sourceBlendFactor);
		activeBlendState.destBlendFactor = blendFactor(state.blendOperation, state.destBlendFactor);
		activeBlendState.blendOperation = blendOperation(state.blendOperation, state.sourceBlendFactor, state.destBlendFactor, format);
		activeBlendState.sourceBlendFactorAlpha = blendFactor(state.blendOperationAlpha, state.sourceBlendFactorAlpha);
		activeBlendState.destBlendFactorAlpha = blendFactor(state.blendOperationAlpha, state.destBlendFactorAlpha);
		activeBlendState.blendOperationAlpha = blendOperation(state.blendOperationAlpha, state.sourceBlendFactorAlpha, state.destBlendFactorAlpha, format);
	}

	return activeBlendState;
}

bool FragmentOutputInterfaceState::alphaBlendActive(int location, const Attachments &attachments, bool fragmentContainsKill) const
{
	ASSERT((location >= 0) && (location < sw::MAX_COLOR_BUFFERS));
	const uint32_t index = attachments.locationToIndex[location];
	if(index == VK_ATTACHMENT_UNUSED)
	{
		return false;
	}

	ASSERT((index >= 0) && (index < sw::MAX_COLOR_BUFFERS));
	auto &state = blendState[index];

	if(!attachments.colorBuffer[location] || !blendState[index].alphaBlendEnable)
	{
		return false;
	}

	if(!(colorWriteActive(attachments) || fragmentContainsKill))
	{
		return false;
	}

	vk::Format format = attachments.colorBuffer[location]->getFormat(VK_IMAGE_ASPECT_COLOR_BIT);
	bool colorBlend = blendOperation(state.blendOperation, state.sourceBlendFactor, state.destBlendFactor, format) != VK_BLEND_OP_SRC_EXT;
	bool alphaBlend = blendOperation(state.blendOperationAlpha, state.sourceBlendFactorAlpha, state.destBlendFactorAlpha, format) != VK_BLEND_OP_SRC_EXT;

	return colorBlend || alphaBlend;
}

VkBlendFactor FragmentOutputInterfaceState::blendFactor(VkBlendOp blendOperation, VkBlendFactor blendFactor) const
{
	switch(blendOperation)
	{
	case VK_BLEND_OP_ADD:
	case VK_BLEND_OP_SUBTRACT:
	case VK_BLEND_OP_REVERSE_SUBTRACT:
		return blendFactor;
	case VK_BLEND_OP_MIN:
	case VK_BLEND_OP_MAX:
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
		return VK_BLEND_FACTOR_ONE;
	default:
		ASSERT(false);
		return blendFactor;
	}
}

VkBlendOp FragmentOutputInterfaceState::blendOperation(VkBlendOp blendOperation, VkBlendFactor sourceBlendFactor, VkBlendFactor destBlendFactor, vk::Format format) const
{
	switch(blendOperation)
	{
	case VK_BLEND_OP_ADD:
		if(sourceBlendFactor == VK_BLEND_FACTOR_ZERO)
		{
			if(destBlendFactor == VK_BLEND_FACTOR_ZERO)
			{
				return VK_BLEND_OP_ZERO_EXT;
			}
			else if(destBlendFactor == VK_BLEND_FACTOR_ONE)
			{
				return VK_BLEND_OP_DST_EXT;
			}
		}
		else if(sourceBlendFactor == VK_BLEND_FACTOR_ONE)
		{
			if(destBlendFactor == VK_BLEND_FACTOR_ZERO)
			{
				return VK_BLEND_OP_SRC_EXT;
			}
		}
		break;
	case VK_BLEND_OP_SUBTRACT:
		if(sourceBlendFactor == VK_BLEND_FACTOR_ZERO)
		{
			if(destBlendFactor == VK_BLEND_FACTOR_ZERO)
			{
				return VK_BLEND_OP_ZERO_EXT;
			}
			else if(format.isUnsignedNormalized())
			{
				return VK_BLEND_OP_ZERO_EXT;  // Negative, clamped to zero
			}
		}
		else if(sourceBlendFactor == VK_BLEND_FACTOR_ONE)
		{
			if(destBlendFactor == VK_BLEND_FACTOR_ZERO)
			{
				return VK_BLEND_OP_SRC_EXT;
			}
		}
		break;
	case VK_BLEND_OP_REVERSE_SUBTRACT:
		if(sourceBlendFactor == VK_BLEND_FACTOR_ZERO)
		{
			if(destBlendFactor == VK_BLEND_FACTOR_ZERO)
			{
				return VK_BLEND_OP_ZERO_EXT;
			}
			else if(destBlendFactor == VK_BLEND_FACTOR_ONE)
			{
				return VK_BLEND_OP_DST_EXT;
			}
		}
		else
		{
			if(destBlendFactor == VK_BLEND_FACTOR_ZERO && format.isUnsignedNormalized())
			{
				return VK_BLEND_OP_ZERO_EXT;  // Negative, clamped to zero
			}
		}
		break;
	case VK_BLEND_OP_MIN:
		return VK_BLEND_OP_MIN;
	case VK_BLEND_OP_MAX:
		return VK_BLEND_OP_MAX;
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
		return blendOperation;
	default:
		ASSERT(false);
	}

	return blendOperation;
}

bool FragmentOutputInterfaceState::colorWriteActive(const Attachments &attachments) const
{
	for(int i = 0; i < sw::MAX_COLOR_BUFFERS; i++)
	{
		if(colorWriteActive(i, attachments))
		{
			return true;
		}
	}

	return false;
}

int FragmentOutputInterfaceState::colorWriteActive(int location, const Attachments &attachments) const
{
	ASSERT((location >= 0) && (location < sw::MAX_COLOR_BUFFERS));
	const uint32_t index = attachments.locationToIndex[location];
	if(index == VK_ATTACHMENT_UNUSED)
	{
		return 0;
	}

	ASSERT((index >= 0) && (index < sw::MAX_COLOR_BUFFERS));
	auto &state = blendState[index];

	if(!attachments.colorBuffer[location] || attachments.colorBuffer[location]->getFormat() == VK_FORMAT_UNDEFINED)
	{
		return 0;
	}

	vk::Format format = attachments.colorBuffer[location]->getFormat(VK_IMAGE_ASPECT_COLOR_BIT);

	if(blendOperation(state.blendOperation, state.sourceBlendFactor, state.destBlendFactor, format) == VK_BLEND_OP_DST_EXT &&
	   blendOperation(state.blendOperationAlpha, state.sourceBlendFactorAlpha, state.destBlendFactorAlpha, format) == VK_BLEND_OP_DST_EXT)
	{
		return 0;
	}

	return colorWriteMask[index];
}

GraphicsState::GraphicsState(const Device *device, const VkGraphicsPipelineCreateInfo *pCreateInfo,
                             const PipelineLayout *layout)
{
	if((pCreateInfo->flags &
	    ~(VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT |
	      VK_PIPELINE_CREATE_DERIVATIVE_BIT |
	      VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT |
	      VK_PIPELINE_CREATE_EARLY_RETURN_ON_FAILURE_BIT_EXT |
	      VK_PIPELINE_CREATE_FAIL_ON_PIPELINE_COMPILE_REQUIRED_BIT_EXT |
	      VK_PIPELINE_CREATE_LIBRARY_BIT_KHR |
	      VK_PIPELINE_CREATE_RETAIN_LINK_TIME_OPTIMIZATION_INFO_BIT_EXT |
	      VK_PIPELINE_CREATE_LINK_TIME_OPTIMIZATION_BIT_EXT)) != 0)
	{
		UNSUPPORTED("pCreateInfo->flags 0x%08X", int(pCreateInfo->flags));
	}

	DynamicStateFlags dynamicStateFlags = ParseDynamicStateFlags(pCreateInfo->pDynamicState);
	const auto *rendering = GetExtendedStruct<VkPipelineRenderingCreateInfo>(pCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO);

	// First, get the subset of state specified in pCreateInfo itself.
	validSubset = GraphicsPipeline::GetGraphicsPipelineSubset(pCreateInfo);

	// If rasterizer discard is enabled (and not dynamically overridable), ignore the fragment
	// and fragment output subsets, as they will not be used.
	if((validSubset & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) != 0 &&
	   pCreateInfo->pRasterizationState->rasterizerDiscardEnable &&
	   !dynamicStateFlags.preRasterization.dynamicRasterizerDiscardEnable)
	{
		validSubset &= ~(VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT | VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT);
	}

	if((validSubset & VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT) != 0)
	{
		vertexInputInterfaceState.initialize(pCreateInfo->pVertexInputState,
		                                     pCreateInfo->pInputAssemblyState,
		                                     dynamicStateFlags);
	}
	if((validSubset & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) != 0)
	{
		preRasterizationState.initialize(device,
		                                 layout,
		                                 pCreateInfo->pViewportState,
		                                 pCreateInfo->pRasterizationState,
		                                 vk::Cast(pCreateInfo->renderPass),
		                                 pCreateInfo->subpass,
		                                 rendering,
		                                 dynamicStateFlags);
	}
	if((validSubset & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) != 0)
	{
		fragmentState.initialize(layout,
		                         pCreateInfo->pDepthStencilState,
		                         vk::Cast(pCreateInfo->renderPass),
		                         pCreateInfo->subpass,
		                         rendering,
		                         dynamicStateFlags);
	}
	if((validSubset & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT) != 0)
	{
		fragmentOutputInterfaceState.initialize(pCreateInfo->pColorBlendState,
		                                        pCreateInfo->pMultisampleState,
		                                        vk::Cast(pCreateInfo->renderPass),
		                                        pCreateInfo->subpass,
		                                        rendering,
		                                        dynamicStateFlags);
	}

	// Then, apply state coming from pipeline libraries.
	const auto *libraryCreateInfo = vk::GetExtendedStruct<VkPipelineLibraryCreateInfoKHR>(pCreateInfo->pNext, VK_STRUCTURE_TYPE_PIPELINE_LIBRARY_CREATE_INFO_KHR);
	if(libraryCreateInfo)
	{
		for(uint32_t i = 0; i < libraryCreateInfo->libraryCount; ++i)
		{
			const auto *library = static_cast<const GraphicsPipeline *>(vk::Cast(libraryCreateInfo->pLibraries[i]));
			const GraphicsState &libraryState = library->getState();
			const VkGraphicsPipelineLibraryFlagsEXT librarySubset = libraryState.validSubset;

			// The library subsets should be disjoint
			ASSERT((libraryState.validSubset & validSubset) == 0);

			if((librarySubset & VK_GRAPHICS_PIPELINE_LIBRARY_VERTEX_INPUT_INTERFACE_BIT_EXT) != 0)
			{
				vertexInputInterfaceState = libraryState.vertexInputInterfaceState;
			}
			if((librarySubset & VK_GRAPHICS_PIPELINE_LIBRARY_PRE_RASTERIZATION_SHADERS_BIT_EXT) != 0)
			{
				preRasterizationState = libraryState.preRasterizationState;
				if(layout)
				{
					preRasterizationState.overridePipelineLayout(layout);
				}
			}
			if((librarySubset & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_SHADER_BIT_EXT) != 0)
			{
				fragmentState = libraryState.fragmentState;
				if(layout)
				{
					fragmentState.overridePipelineLayout(layout);
				}
			}
			if((librarySubset & VK_GRAPHICS_PIPELINE_LIBRARY_FRAGMENT_OUTPUT_INTERFACE_BIT_EXT) != 0)
			{
				fragmentOutputInterfaceState = libraryState.fragmentOutputInterfaceState;
			}

			validSubset |= libraryState.validSubset;
		}
	}
}

GraphicsState GraphicsState::combineStates(const DynamicState &dynamicState) const
{
	GraphicsState combinedState = *this;

	// Make a copy of the states for modification, then either keep the pipeline state or apply the dynamic state.
	combinedState.vertexInputInterfaceState.applyState(dynamicState);
	combinedState.preRasterizationState.applyState(dynamicState);
	combinedState.fragmentState.applyState(dynamicState);
	combinedState.fragmentOutputInterfaceState.applyState(dynamicState);

	return combinedState;
}

}  // namespace vk
