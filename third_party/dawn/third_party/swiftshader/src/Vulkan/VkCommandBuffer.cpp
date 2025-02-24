// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
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

#include "VkCommandBuffer.hpp"

#include "VkBuffer.hpp"
#include "VkConfig.hpp"
#include "VkDevice.hpp"
#include "VkEvent.hpp"
#include "VkFence.hpp"
#include "VkFramebuffer.hpp"
#include "VkImage.hpp"
#include "VkImageView.hpp"
#include "VkPipeline.hpp"
#include "VkPipelineLayout.hpp"
#include "VkQueryPool.hpp"
#include "VkRenderPass.hpp"
#include "Device/Renderer.hpp"

#include "./Debug/Context.hpp"
#include "./Debug/File.hpp"
#include "./Debug/Thread.hpp"

#include "marl/defer.h"

#include <bitset>
#include <cstring>

namespace {

class CmdBeginRenderPass : public vk::CommandBuffer::Command
{
public:
	CmdBeginRenderPass(vk::RenderPass *renderPass, vk::Framebuffer *framebuffer, VkRect2D renderArea,
	                   uint32_t clearValueCount, const VkClearValue *pClearValues,
	                   const VkRenderPassAttachmentBeginInfo *attachmentInfo)
	    : renderPass(renderPass)
	    , framebuffer(framebuffer)
	    , renderArea(renderArea)
	    , clearValueCount(clearValueCount)
	    , attachmentCount(attachmentInfo ? attachmentInfo->attachmentCount : 0)
	    , attachments(nullptr)
	{
		// FIXME(b/119409619): use an allocator here so we can control all memory allocations
		clearValues = new VkClearValue[clearValueCount];
		memcpy(clearValues, pClearValues, clearValueCount * sizeof(VkClearValue));
		if(attachmentCount > 0)
		{
			attachments = new vk::ImageView *[attachmentCount];
			for(uint32_t i = 0; i < attachmentCount; i++)
			{
				attachments[i] = vk::Cast(attachmentInfo->pAttachments[i]);
			}
		}
	}

	~CmdBeginRenderPass() override
	{
		delete[] clearValues;
		delete[] attachments;
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.renderPass = renderPass;
		executionState.renderPassFramebuffer = framebuffer;
		executionState.subpassIndex = 0;

		for(uint32_t i = 0; i < attachmentCount; i++)
		{
			framebuffer->setAttachment(attachments[i], i);
		}

		// Vulkan specifies that the attachments' `loadOp` gets executed "at the beginning of the subpass where it is first used."
		// Since we don't discard any contents between subpasses, this is equivalent to executing it at the start of the renderpass.
		framebuffer->executeLoadOp(executionState.renderPass, clearValueCount, clearValues, renderArea);
	}

	std::string description() override { return "vkCmdBeginRenderPass()"; }

private:
	vk::RenderPass *const renderPass;
	vk::Framebuffer *const framebuffer;
	const VkRect2D renderArea;
	const uint32_t clearValueCount;
	VkClearValue *clearValues;
	uint32_t attachmentCount;
	vk::ImageView **attachments;
};

class CmdNextSubpass : public vk::CommandBuffer::Command
{
public:
	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		bool hasResolveAttachments = (executionState.renderPass->getSubpass(executionState.subpassIndex).pResolveAttachments != nullptr);
		if(hasResolveAttachments)
		{
			// TODO(b/197691918): Avoid halt-the-world synchronization.
			executionState.renderer->synchronize();

			// TODO(b/197691917): Eliminate redundant resolve operations.
			executionState.renderPassFramebuffer->resolve(executionState.renderPass, executionState.subpassIndex);
		}

		executionState.subpassIndex++;
	}

	std::string description() override { return "vkCmdNextSubpass()"; }
};

class CmdEndRenderPass : public vk::CommandBuffer::Command
{
public:
	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		// Execute (implicit or explicit) VkSubpassDependency to VK_SUBPASS_EXTERNAL.
		// TODO(b/197691918): Avoid halt-the-world synchronization.
		executionState.renderer->synchronize();

		// TODO(b/197691917): Eliminate redundant resolve operations.
		executionState.renderPassFramebuffer->resolve(executionState.renderPass, executionState.subpassIndex);

		executionState.renderPass = nullptr;
		executionState.renderPassFramebuffer = nullptr;
	}

	std::string description() override { return "vkCmdEndRenderPass()"; }
};

class CmdBeginRendering : public vk::CommandBuffer::Command
{
public:
	CmdBeginRendering(const VkRenderingInfo *pRenderingInfo)
	    : dynamicRendering(pRenderingInfo)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicRendering = &dynamicRendering;

		if(!executionState.dynamicRendering->resume())
		{
			VkClearRect rect = {};
			rect.rect = executionState.dynamicRendering->getRenderArea();
			rect.layerCount = executionState.dynamicRendering->getLayerCount();
			uint32_t viewMask = executionState.dynamicRendering->getViewMask();

			// Vulkan specifies that the attachments' `loadOp` gets executed "at the beginning of the subpass where it is first used."
			// Since we don't discard any contents between subpasses, this is equivalent to executing it at the start of the renderpass.
			for(uint32_t i = 0; i < dynamicRendering.getColorAttachmentCount(); i++)
			{
				const VkRenderingAttachmentInfo *colorAttachment = dynamicRendering.getColorAttachment(i);

				if(colorAttachment->loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
				{
					vk::ImageView *imageView = vk::Cast(colorAttachment->imageView);
					if(imageView)
					{
						imageView->clear(colorAttachment->clearValue, VK_IMAGE_ASPECT_COLOR_BIT, rect, viewMask);
					}
				}
			}

			const VkRenderingAttachmentInfo &stencilAttachment = dynamicRendering.getStencilAttachment();
			if(stencilAttachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
			{
				vk::ImageView *imageView = vk::Cast(stencilAttachment.imageView);
				if(imageView)
				{
					imageView->clear(stencilAttachment.clearValue, VK_IMAGE_ASPECT_STENCIL_BIT, rect, viewMask);
				}
			}

			const VkRenderingAttachmentInfo &depthAttachment = dynamicRendering.getDepthAttachment();
			if(depthAttachment.loadOp == VK_ATTACHMENT_LOAD_OP_CLEAR)
			{
				vk::ImageView *imageView = vk::Cast(depthAttachment.imageView);

				if(imageView)
				{
					imageView->clear(depthAttachment.clearValue, VK_IMAGE_ASPECT_DEPTH_BIT, rect, viewMask);
				}
			}
		}
	}

	std::string description() override { return "vkCmdBeginRendering()"; }

private:
	vk::DynamicRendering dynamicRendering;
};

class CmdEndRendering : public vk::CommandBuffer::Command
{
public:
	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		// TODO(b/197691918): Avoid halt-the-world synchronization.
		executionState.renderer->synchronize();

		if(!executionState.dynamicRendering->suspend())
		{
			uint32_t viewMask = executionState.dynamicRendering->getViewMask();

			// TODO(b/197691917): Eliminate redundant resolve operations.
			uint32_t colorAttachmentCount = executionState.dynamicRendering->getColorAttachmentCount();
			for(uint32_t i = 0; i < colorAttachmentCount; i++)
			{
				const VkRenderingAttachmentInfo *colorAttachment = executionState.dynamicRendering->getColorAttachment(i);
				if(colorAttachment && colorAttachment->resolveMode != VK_RESOLVE_MODE_NONE)
				{
					vk::ImageView *imageView = vk::Cast(colorAttachment->imageView);
					vk::ImageView *resolveImageView = vk::Cast(colorAttachment->resolveImageView);
					imageView->resolve(resolveImageView, viewMask);
				}
			}

			const VkRenderingAttachmentInfo &depthAttachment = executionState.dynamicRendering->getDepthAttachment();
			if(depthAttachment.resolveMode != VK_RESOLVE_MODE_NONE)
			{
				vk::ImageView *imageView = vk::Cast(depthAttachment.imageView);
				vk::ImageView *resolveImageView = vk::Cast(depthAttachment.resolveImageView);
				imageView->resolveDepthStencil(resolveImageView, depthAttachment.resolveMode, VK_RESOLVE_MODE_NONE);
			}

			const VkRenderingAttachmentInfo &stencilAttachment = executionState.dynamicRendering->getStencilAttachment();
			if(stencilAttachment.resolveMode != VK_RESOLVE_MODE_NONE)
			{
				vk::ImageView *imageView = vk::Cast(stencilAttachment.imageView);
				vk::ImageView *resolveImageView = vk::Cast(stencilAttachment.resolveImageView);
				imageView->resolveDepthStencil(resolveImageView, VK_RESOLVE_MODE_NONE, stencilAttachment.resolveMode);
			}
		}

		executionState.dynamicRendering = nullptr;
	}

	std::string description() override { return "vkCmdEndRendering()"; }
};

class CmdExecuteCommands : public vk::CommandBuffer::Command
{
public:
	CmdExecuteCommands(const vk::CommandBuffer *commandBuffer)
	    : commandBuffer(commandBuffer)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		commandBuffer->submitSecondary(executionState);
	}

	std::string description() override { return "vkCmdExecuteCommands()"; }

private:
	const vk::CommandBuffer *const commandBuffer;
};

class CmdPipelineBind : public vk::CommandBuffer::Command
{
public:
	CmdPipelineBind(VkPipelineBindPoint pipelineBindPoint, vk::Pipeline *pipeline)
	    : pipelineBindPoint(pipelineBindPoint)
	    , pipeline(pipeline)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.pipelineState[pipelineBindPoint].pipeline = pipeline;
	}

	std::string description() override { return "vkCmdPipelineBind()"; }

private:
	const VkPipelineBindPoint pipelineBindPoint;
	vk::Pipeline *const pipeline;
};

class CmdDispatch : public vk::CommandBuffer::Command
{
public:
	CmdDispatch(uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
	    : baseGroupX(baseGroupX)
	    , baseGroupY(baseGroupY)
	    , baseGroupZ(baseGroupZ)
	    , groupCountX(groupCountX)
	    , groupCountY(groupCountY)
	    , groupCountZ(groupCountZ)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		const auto &pipelineState = executionState.pipelineState[VK_PIPELINE_BIND_POINT_COMPUTE];

		vk::ComputePipeline *pipeline = static_cast<vk::ComputePipeline *>(pipelineState.pipeline);
		pipeline->run(baseGroupX, baseGroupY, baseGroupZ,
		              groupCountX, groupCountY, groupCountZ,
		              pipelineState.descriptorSetObjects,
		              pipelineState.descriptorSets,
		              pipelineState.descriptorDynamicOffsets,
		              executionState.pushConstants);
	}

	std::string description() override { return "vkCmdDispatch()"; }

private:
	const uint32_t baseGroupX;
	const uint32_t baseGroupY;
	const uint32_t baseGroupZ;
	const uint32_t groupCountX;
	const uint32_t groupCountY;
	const uint32_t groupCountZ;
};

class CmdDispatchIndirect : public vk::CommandBuffer::Command
{
public:
	CmdDispatchIndirect(vk::Buffer *buffer, VkDeviceSize offset)
	    : buffer(buffer)
	    , offset(offset)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		const auto *cmd = reinterpret_cast<const VkDispatchIndirectCommand *>(buffer->getOffsetPointer(offset));

		const auto &pipelineState = executionState.pipelineState[VK_PIPELINE_BIND_POINT_COMPUTE];

		auto *pipeline = static_cast<vk::ComputePipeline *>(pipelineState.pipeline);
		pipeline->run(0, 0, 0, cmd->x, cmd->y, cmd->z,
		              pipelineState.descriptorSetObjects,
		              pipelineState.descriptorSets,
		              pipelineState.descriptorDynamicOffsets,
		              executionState.pushConstants);
	}

	std::string description() override { return "vkCmdDispatchIndirect()"; }

private:
	const vk::Buffer *const buffer;
	const VkDeviceSize offset;
};

class CmdVertexBufferBind : public vk::CommandBuffer::Command
{
public:
	CmdVertexBufferBind(uint32_t binding, vk::Buffer *buffer, const VkDeviceSize offset, const VkDeviceSize size, const VkDeviceSize stride, bool hasStride)
	    : binding(binding)
	    , buffer(buffer)
	    , offset(offset)
	    , size(size)
	    , stride(stride)
	    , hasStride(hasStride)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.vertexInputBindings[binding] = { buffer, offset, size };
		if(hasStride)
		{
			executionState.dynamicState.vertexInputBindings[binding].stride = stride;
		}
	}

	std::string description() override { return "vkCmdVertexBufferBind()"; }

private:
	const uint32_t binding;
	vk::Buffer *const buffer;
	const VkDeviceSize offset;
	const VkDeviceSize size;
	const VkDeviceSize stride;
	const bool hasStride;
};

class CmdIndexBufferBind : public vk::CommandBuffer::Command
{
public:
	CmdIndexBufferBind(vk::Buffer *buffer, const VkDeviceSize offset, const VkIndexType indexType)
	    : buffer(buffer)
	    , offset(offset)
	    , indexType(indexType)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.indexBufferBinding = { buffer, offset, 0 };
		executionState.indexType = indexType;
	}

	std::string description() override { return "vkCmdIndexBufferBind()"; }

private:
	vk::Buffer *const buffer;
	const VkDeviceSize offset;
	const VkIndexType indexType;
};

class CmdSetViewport : public vk::CommandBuffer::Command
{
public:
	CmdSetViewport(const VkViewport &viewport, uint32_t viewportID)
	    : viewport(viewport)
	    , viewportID(viewportID)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.viewport = viewport;
	}

	std::string description() override { return "vkCmdSetViewport()"; }

private:
	const VkViewport viewport;
	const uint32_t viewportID;
};

class CmdSetScissor : public vk::CommandBuffer::Command
{
public:
	CmdSetScissor(const VkRect2D &scissor, uint32_t scissorID)
	    : scissor(scissor)
	    , scissorID(scissorID)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.scissor = scissor;
	}

	std::string description() override { return "vkCmdSetScissor()"; }

private:
	const VkRect2D scissor;
	const uint32_t scissorID;
};

class CmdSetLineWidth : public vk::CommandBuffer::Command
{
public:
	CmdSetLineWidth(float lineWidth)
	    : lineWidth(lineWidth)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.lineWidth = lineWidth;
	}

	std::string description() override { return "vkCmdSetLineWidth()"; }

private:
	const float lineWidth;
};

class CmdSetDepthBias : public vk::CommandBuffer::Command
{
public:
	CmdSetDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
	    : depthBiasConstantFactor(depthBiasConstantFactor)
	    , depthBiasClamp(depthBiasClamp)
	    , depthBiasSlopeFactor(depthBiasSlopeFactor)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.depthBiasConstantFactor = depthBiasConstantFactor;
		executionState.dynamicState.depthBiasClamp = depthBiasClamp;
		executionState.dynamicState.depthBiasSlopeFactor = depthBiasSlopeFactor;
	}

	std::string description() override { return "vkCmdSetDepthBias()"; }

private:
	const float depthBiasConstantFactor;
	const float depthBiasClamp;
	const float depthBiasSlopeFactor;
};

class CmdSetBlendConstants : public vk::CommandBuffer::Command
{
public:
	CmdSetBlendConstants(const float blendConstants[4])
	{
		memcpy(this->blendConstants, blendConstants, sizeof(this->blendConstants));
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		memcpy(&(executionState.dynamicState.blendConstants[0]), blendConstants, sizeof(blendConstants));
	}

	std::string description() override { return "vkCmdSetBlendConstants()"; }

private:
	float blendConstants[4];
};

class CmdSetDepthBounds : public vk::CommandBuffer::Command
{
public:
	CmdSetDepthBounds(float minDepthBounds, float maxDepthBounds)
	    : minDepthBounds(minDepthBounds)
	    , maxDepthBounds(maxDepthBounds)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.minDepthBounds = minDepthBounds;
		executionState.dynamicState.maxDepthBounds = maxDepthBounds;
	}

	std::string description() override { return "vkCmdSetDepthBounds()"; }

private:
	const float minDepthBounds;
	const float maxDepthBounds;
};

class CmdSetStencilCompareMask : public vk::CommandBuffer::Command
{
public:
	CmdSetStencilCompareMask(VkStencilFaceFlags faceMask, uint32_t compareMask)
	    : faceMask(faceMask)
	    , compareMask(compareMask)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		if(faceMask & VK_STENCIL_FACE_FRONT_BIT)
		{
			executionState.dynamicState.frontStencil.compareMask = compareMask;
		}

		if(faceMask & VK_STENCIL_FACE_BACK_BIT)
		{
			executionState.dynamicState.backStencil.compareMask = compareMask;
		}
	}

	std::string description() override { return "vkCmdSetStencilCompareMask()"; }

private:
	const VkStencilFaceFlags faceMask;
	const uint32_t compareMask;
};

class CmdSetStencilWriteMask : public vk::CommandBuffer::Command
{
public:
	CmdSetStencilWriteMask(VkStencilFaceFlags faceMask, uint32_t writeMask)
	    : faceMask(faceMask)
	    , writeMask(writeMask)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		if(faceMask & VK_STENCIL_FACE_FRONT_BIT)
		{
			executionState.dynamicState.frontStencil.writeMask = writeMask;
		}

		if(faceMask & VK_STENCIL_FACE_BACK_BIT)
		{
			executionState.dynamicState.backStencil.writeMask = writeMask;
		}
	}

	std::string description() override { return "vkCmdSetStencilWriteMask()"; }

private:
	const VkStencilFaceFlags faceMask;
	const uint32_t writeMask;
};

class CmdSetStencilReference : public vk::CommandBuffer::Command
{
public:
	CmdSetStencilReference(VkStencilFaceFlags faceMask, uint32_t reference)
	    : faceMask(faceMask)
	    , reference(reference)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		if(faceMask & VK_STENCIL_FACE_FRONT_BIT)
		{
			executionState.dynamicState.frontStencil.reference = reference;
		}
		if(faceMask & VK_STENCIL_FACE_BACK_BIT)
		{
			executionState.dynamicState.backStencil.reference = reference;
		}
	}

	std::string description() override { return "vkCmdSetStencilReference()"; }

private:
	const VkStencilFaceFlags faceMask;
	const uint32_t reference;
};

class CmdSetCullMode : public vk::CommandBuffer::Command
{
public:
	CmdSetCullMode(VkCullModeFlags cullMode)
	    : cullMode(cullMode)
	{}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.cullMode = cullMode;
	}

	std::string description() override { return "vkCmdSetCullModeEXT()"; }

private:
	const VkCullModeFlags cullMode;
};

class CmdSetDepthBoundsTestEnable : public vk::CommandBuffer::Command
{
public:
	CmdSetDepthBoundsTestEnable(VkBool32 depthBoundsTestEnable)
	    : depthBoundsTestEnable(depthBoundsTestEnable)
	{}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.depthBoundsTestEnable = depthBoundsTestEnable;
	}

	std::string description() override { return "vkCmdSetDepthBoundsTestEnableEXT()"; }

private:
	const VkBool32 depthBoundsTestEnable;
};

class CmdSetDepthCompareOp : public vk::CommandBuffer::Command
{
public:
	CmdSetDepthCompareOp(VkCompareOp depthCompareOp)
	    : depthCompareOp(depthCompareOp)
	{}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.depthCompareOp = depthCompareOp;
	}

	std::string description() override { return "vkCmdSetDepthCompareOpEXT()"; }

private:
	const VkCompareOp depthCompareOp;
};

class CmdSetDepthTestEnable : public vk::CommandBuffer::Command
{
public:
	CmdSetDepthTestEnable(VkBool32 depthTestEnable)
	    : depthTestEnable(depthTestEnable)
	{}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.depthTestEnable = depthTestEnable;
	}

	std::string description() override { return "vkCmdSetDepthTestEnableEXT()"; }

private:
	const VkBool32 depthTestEnable;
};

class CmdSetDepthWriteEnable : public vk::CommandBuffer::Command
{
public:
	CmdSetDepthWriteEnable(VkBool32 depthWriteEnable)
	    : depthWriteEnable(depthWriteEnable)
	{}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.depthWriteEnable = depthWriteEnable;
	}

	std::string description() override { return "vkCmdSetDepthWriteEnableEXT()"; }

private:
	const VkBool32 depthWriteEnable;
};

class CmdSetFrontFace : public vk::CommandBuffer::Command
{
public:
	CmdSetFrontFace(VkFrontFace frontFace)
	    : frontFace(frontFace)
	{}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.frontFace = frontFace;
	}

	std::string description() override { return "vkCmdSetFrontFaceEXT()"; }

private:
	const VkFrontFace frontFace;
};

class CmdSetPrimitiveTopology : public vk::CommandBuffer::Command
{
public:
	CmdSetPrimitiveTopology(VkPrimitiveTopology primitiveTopology)
	    : primitiveTopology(primitiveTopology)
	{}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.primitiveTopology = primitiveTopology;
	}

	std::string description() override { return "vkCmdSetPrimitiveTopologyEXT()"; }

private:
	const VkPrimitiveTopology primitiveTopology;
};

class CmdSetScissorWithCount : public vk::CommandBuffer::Command
{
public:
	CmdSetScissorWithCount(uint32_t scissorCount, const VkRect2D *pScissors)
	    : scissorCount(scissorCount)
	{
		memcpy(&(scissors[0]), pScissors, scissorCount * sizeof(VkRect2D));
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.scissorCount = scissorCount;
		for(uint32_t i = 0; i < scissorCount; i++)
		{
			executionState.dynamicState.scissors[i] = scissors[i];
		}
	}

	std::string description() override { return "vkCmdSetScissorWithCountEXT()"; }

private:
	const uint32_t scissorCount;
	VkRect2D scissors[vk::MAX_VIEWPORTS];
};

class CmdSetStencilOp : public vk::CommandBuffer::Command
{
public:
	CmdSetStencilOp(VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp)
	    : faceMask(faceMask)
	    , failOp(failOp)
	    , passOp(passOp)
	    , depthFailOp(depthFailOp)
	    , compareOp(compareOp)
	{}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.faceMask |= faceMask;

		if(faceMask & VK_STENCIL_FACE_FRONT_BIT)
		{
			executionState.dynamicState.frontStencil.failOp = failOp;
			executionState.dynamicState.frontStencil.passOp = passOp;
			executionState.dynamicState.frontStencil.depthFailOp = depthFailOp;
			executionState.dynamicState.frontStencil.compareOp = compareOp;
		}
		if(faceMask & VK_STENCIL_FACE_BACK_BIT)
		{
			executionState.dynamicState.backStencil.failOp = failOp;
			executionState.dynamicState.backStencil.passOp = passOp;
			executionState.dynamicState.backStencil.depthFailOp = depthFailOp;
			executionState.dynamicState.backStencil.compareOp = compareOp;
		}
	}

	std::string description() override { return "vkCmdSetStencilOpEXT()"; }

private:
	const VkStencilFaceFlags faceMask;
	const VkStencilOp failOp;
	const VkStencilOp passOp;
	const VkStencilOp depthFailOp;
	const VkCompareOp compareOp;
};

class CmdSetStencilTestEnable : public vk::CommandBuffer::Command
{
public:
	CmdSetStencilTestEnable(VkBool32 stencilTestEnable)
	    : stencilTestEnable(stencilTestEnable)
	{}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.stencilTestEnable = stencilTestEnable;
	}

	std::string description() override { return "vkCmdSetStencilTestEnableEXT()"; }

private:
	const VkBool32 stencilTestEnable;
};

class CmdSetViewportWithCount : public vk::CommandBuffer::Command
{
public:
	CmdSetViewportWithCount(uint32_t viewportCount, const VkViewport *pViewports)
	    : viewportCount(viewportCount)
	{
		memcpy(viewports, pViewports, sizeof(VkViewport) * viewportCount);
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.viewportCount = viewportCount;
		for(uint32_t i = 0; i < viewportCount; i++)
		{
			executionState.dynamicState.viewports[i] = viewports[i];
		}
	}

	std::string description() override { return "vkCmdSetViewportWithCountEXT()"; }

private:
	const uint32_t viewportCount;
	VkViewport viewports[vk::MAX_VIEWPORTS];
};

class CmdSetRasterizerDiscardEnable : public vk::CommandBuffer::Command
{
public:
	CmdSetRasterizerDiscardEnable(VkBool32 rasterizerDiscardEnable)
	    : rasterizerDiscardEnable(rasterizerDiscardEnable)
	{}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.rasterizerDiscardEnable = rasterizerDiscardEnable;
	}

	std::string description() override { return "vkCmdSetRasterizerDiscardEnable()"; }

private:
	const VkBool32 rasterizerDiscardEnable;
};

class CmdSetDepthBiasEnable : public vk::CommandBuffer::Command
{
public:
	CmdSetDepthBiasEnable(VkBool32 depthBiasEnable)
	    : depthBiasEnable(depthBiasEnable)
	{}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.depthBiasEnable = depthBiasEnable;
	}

	std::string description() override { return "vkCmdSetDepthBiasEnable()"; }

private:
	const VkBool32 depthBiasEnable;
};

class CmdSetPrimitiveRestartEnable : public vk::CommandBuffer::Command
{
public:
	CmdSetPrimitiveRestartEnable(VkBool32 primitiveRestartEnable)
	    : primitiveRestartEnable(primitiveRestartEnable)
	{}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.dynamicState.primitiveRestartEnable = primitiveRestartEnable;
	}

	std::string description() override { return "vkCmdSetPrimitiveRestartEnable()"; }

private:
	const VkBool32 primitiveRestartEnable;
};

class CmdSetVertexInput : public vk::CommandBuffer::Command
{
public:
	CmdSetVertexInput(uint32_t vertexBindingDescriptionCount,
	                  const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions,
	                  uint32_t vertexAttributeDescriptionCount,
	                  const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions)
	    :  // Note: the pNext values are unused, so this copy is currently safe.
	    vertexBindingDescriptions(pVertexBindingDescriptions, pVertexBindingDescriptions + vertexBindingDescriptionCount)
	    , vertexAttributeDescriptions(pVertexAttributeDescriptions, pVertexAttributeDescriptions + vertexAttributeDescriptionCount)
	{}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		for(const auto &desc : vertexBindingDescriptions)
		{
			vk::DynamicVertexInputBindingState &state = executionState.dynamicState.vertexInputBindings[desc.binding];
			state.inputRate = desc.inputRate;
			state.stride = desc.stride;
			state.divisor = desc.divisor;
		}

		for(const auto &desc : vertexAttributeDescriptions)
		{
			vk::DynamicVertexInputAttributeState &state = executionState.dynamicState.vertexInputAttributes[desc.location];
			state.format = desc.format;
			state.offset = desc.offset;
			state.binding = desc.binding;
		}
	}

	std::string description() override { return "vkCmdSetVertexInputEXT()"; }

private:
	const std::vector<VkVertexInputBindingDescription2EXT> vertexBindingDescriptions;
	const std::vector<VkVertexInputAttributeDescription2EXT> vertexAttributeDescriptions;
};

class CmdDrawBase : public vk::CommandBuffer::Command
{
public:
	void draw(vk::CommandBuffer::ExecutionState &executionState, bool indexed,
	          uint32_t count, uint32_t instanceCount, uint32_t first, int32_t vertexOffset, uint32_t firstInstance)
	{
		const auto &pipelineState = executionState.pipelineState[VK_PIPELINE_BIND_POINT_GRAPHICS];

		auto *pipeline = static_cast<vk::GraphicsPipeline *>(pipelineState.pipeline);

		vk::Attachments &attachments = pipeline->getAttachments();
		executionState.bindAttachments(&attachments);

		vk::Inputs &inputs = pipeline->getInputs();
		inputs.updateDescriptorSets(pipelineState.descriptorSetObjects,
		                            pipelineState.descriptorSets,
		                            pipelineState.descriptorDynamicOffsets);
		inputs.setVertexInputBinding(executionState.vertexInputBindings, executionState.dynamicState);
		inputs.bindVertexInputs(firstInstance);

		if(indexed)
		{
			vk::IndexBuffer &indexBuffer = pipeline->getIndexBuffer();
			indexBuffer.setIndexBufferBinding(executionState.indexBufferBinding, executionState.indexType);
		}

		std::vector<std::pair<uint32_t, void *>> indexBuffers;
		pipeline->getIndexBuffers(executionState.dynamicState, count, first, indexed, &indexBuffers);

		VkRect2D renderArea = executionState.getRenderArea();

		for(uint32_t instance = firstInstance; instance != firstInstance + instanceCount; instance++)
		{
			// FIXME: reconsider instances/views nesting.
			auto layerMask = executionState.getLayerMask();
			while(layerMask)
			{
				int layer = sw::log2i(layerMask);
				layerMask &= ~(1 << layer);

				for(auto indexBuffer : indexBuffers)
				{
					executionState.renderer->draw(pipeline, executionState.dynamicState, indexBuffer.first, vertexOffset,
					                              executionState.events, instance, layer, indexBuffer.second,
					                              renderArea, executionState.pushConstants);
				}
			}

			if(instanceCount > 1)
			{
				UNOPTIMIZED("Optimize instancing to use a single draw call.");  // TODO(b/137740918)
				inputs.advanceInstanceAttributes();
			}
		}
	}
};

class CmdDraw : public CmdDrawBase
{
public:
	CmdDraw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
	    : vertexCount(vertexCount)
	    , instanceCount(instanceCount)
	    , firstVertex(firstVertex)
	    , firstInstance(firstInstance)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		draw(executionState, false, vertexCount, instanceCount, 0, firstVertex, firstInstance);
	}

	std::string description() override { return "vkCmdDraw()"; }

private:
	const uint32_t vertexCount;
	const uint32_t instanceCount;
	const uint32_t firstVertex;
	const uint32_t firstInstance;
};

class CmdDrawIndexed : public CmdDrawBase
{
public:
	CmdDrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
	    : indexCount(indexCount)
	    , instanceCount(instanceCount)
	    , firstIndex(firstIndex)
	    , vertexOffset(vertexOffset)
	    , firstInstance(firstInstance)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		draw(executionState, true, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}

	std::string description() override { return "vkCmdDrawIndexed()"; }

private:
	const uint32_t indexCount;
	const uint32_t instanceCount;
	const uint32_t firstIndex;
	const int32_t vertexOffset;
	const uint32_t firstInstance;
};

class CmdDrawIndirect : public CmdDrawBase
{
public:
	CmdDrawIndirect(vk::Buffer *buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
	    : buffer(buffer)
	    , offset(offset)
	    , drawCount(drawCount)
	    , stride(stride)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		for(auto drawId = 0u; drawId < drawCount; drawId++)
		{
			const auto *cmd = reinterpret_cast<const VkDrawIndirectCommand *>(buffer->getOffsetPointer(offset + drawId * stride));
			draw(executionState, false, cmd->vertexCount, cmd->instanceCount, 0, cmd->firstVertex, cmd->firstInstance);
		}
	}

	std::string description() override { return "vkCmdDrawIndirect()"; }

private:
	const vk::Buffer *const buffer;
	const VkDeviceSize offset;
	const uint32_t drawCount;
	const uint32_t stride;
};

class CmdDrawIndexedIndirect : public CmdDrawBase
{
public:
	CmdDrawIndexedIndirect(vk::Buffer *buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
	    : buffer(buffer)
	    , offset(offset)
	    , drawCount(drawCount)
	    , stride(stride)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		for(auto drawId = 0u; drawId < drawCount; drawId++)
		{
			const auto *cmd = reinterpret_cast<const VkDrawIndexedIndirectCommand *>(buffer->getOffsetPointer(offset + drawId * stride));
			draw(executionState, true, cmd->indexCount, cmd->instanceCount, cmd->firstIndex, cmd->vertexOffset, cmd->firstInstance);
		}
	}

	std::string description() override { return "vkCmdDrawIndexedIndirect()"; }

private:
	const vk::Buffer *const buffer;
	const VkDeviceSize offset;
	const uint32_t drawCount;
	const uint32_t stride;
};

class CmdCopyImage : public vk::CommandBuffer::Command
{
public:
	CmdCopyImage(const vk::Image *srcImage, vk::Image *dstImage, const VkImageCopy2 &region)
	    : srcImage(srcImage)
	    , dstImage(dstImage)
	    , region(region)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		srcImage->copyTo(dstImage, region);
	}

	std::string description() override { return "vkCmdCopyImage()"; }

private:
	const vk::Image *const srcImage;
	vk::Image *const dstImage;
	const VkImageCopy2 region;
};

class CmdCopyBuffer : public vk::CommandBuffer::Command
{
public:
	CmdCopyBuffer(const vk::Buffer *srcBuffer, vk::Buffer *dstBuffer, const VkBufferCopy2 &region)
	    : srcBuffer(srcBuffer)
	    , dstBuffer(dstBuffer)
	    , region(region)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		srcBuffer->copyTo(dstBuffer, region);
	}

	std::string description() override { return "vkCmdCopyBuffer()"; }

private:
	const vk::Buffer *const srcBuffer;
	vk::Buffer *const dstBuffer;
	const VkBufferCopy2 region;
};

class CmdCopyImageToBuffer : public vk::CommandBuffer::Command
{
public:
	CmdCopyImageToBuffer(vk::Image *srcImage, vk::Buffer *dstBuffer, const VkBufferImageCopy2 &region)
	    : srcImage(srcImage)
	    , dstBuffer(dstBuffer)
	    , region(region)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		srcImage->copyTo(dstBuffer, region);
	}

	std::string description() override { return "vkCmdCopyImageToBuffer()"; }

private:
	vk::Image *const srcImage;
	vk::Buffer *const dstBuffer;
	const VkBufferImageCopy2 region;
};

class CmdCopyBufferToImage : public vk::CommandBuffer::Command
{
public:
	CmdCopyBufferToImage(vk::Buffer *srcBuffer, vk::Image *dstImage, const VkBufferImageCopy2 &region)
	    : srcBuffer(srcBuffer)
	    , dstImage(dstImage)
	    , region(region)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		dstImage->copyFrom(srcBuffer, region);
	}

	std::string description() override { return "vkCmdCopyBufferToImage()"; }

private:
	vk::Buffer *const srcBuffer;
	vk::Image *const dstImage;
	const VkBufferImageCopy2 region;
};

class CmdFillBuffer : public vk::CommandBuffer::Command
{
public:
	CmdFillBuffer(vk::Buffer *dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data)
	    : dstBuffer(dstBuffer)
	    , dstOffset(dstOffset)
	    , size(size)
	    , data(data)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		dstBuffer->fill(dstOffset, size, data);
	}

	std::string description() override { return "vkCmdFillBuffer()"; }

private:
	vk::Buffer *const dstBuffer;
	const VkDeviceSize dstOffset;
	const VkDeviceSize size;
	const uint32_t data;
};

class CmdUpdateBuffer : public vk::CommandBuffer::Command
{
public:
	CmdUpdateBuffer(vk::Buffer *dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const uint8_t *pData)
	    : dstBuffer(dstBuffer)
	    , dstOffset(dstOffset)
	    , data(pData, &pData[dataSize])
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		dstBuffer->update(dstOffset, data.size(), data.data());
	}

	std::string description() override { return "vkCmdUpdateBuffer()"; }

private:
	vk::Buffer *const dstBuffer;
	const VkDeviceSize dstOffset;
	const std::vector<uint8_t> data;  // FIXME(b/119409619): replace this vector by an allocator so we can control all memory allocations
};

class CmdClearColorImage : public vk::CommandBuffer::Command
{
public:
	CmdClearColorImage(vk::Image *image, const VkClearColorValue &color, const VkImageSubresourceRange &range)
	    : image(image)
	    , color(color)
	    , range(range)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		image->clear(color, range);
	}

	std::string description() override { return "vkCmdClearColorImage()"; }

private:
	vk::Image *const image;
	const VkClearColorValue color;
	const VkImageSubresourceRange range;
};

class CmdClearDepthStencilImage : public vk::CommandBuffer::Command
{
public:
	CmdClearDepthStencilImage(vk::Image *image, const VkClearDepthStencilValue &depthStencil, const VkImageSubresourceRange &range)
	    : image(image)
	    , depthStencil(depthStencil)
	    , range(range)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		image->clear(depthStencil, range);
	}

	std::string description() override { return "vkCmdClearDepthStencilImage()"; }

private:
	vk::Image *const image;
	const VkClearDepthStencilValue depthStencil;
	const VkImageSubresourceRange range;
};

class CmdClearAttachment : public vk::CommandBuffer::Command
{
public:
	CmdClearAttachment(const VkClearAttachment &attachment, const VkClearRect &rect)
	    : attachment(attachment)
	    , rect(rect)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		// attachment clears are drawing operations, and so have rasterization-order guarantees.
		// however, we don't do the clear through the rasterizer, so need to ensure prior drawing
		// has completed first.
		executionState.renderer->synchronize();

		if(executionState.renderPassFramebuffer)
		{
			executionState.renderPassFramebuffer->clearAttachment(executionState.renderPass, executionState.subpassIndex, attachment, rect);
		}
		else if(executionState.dynamicRendering)
		{
			uint32_t viewMask = executionState.dynamicRendering->getViewMask();

			if(attachment.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT)
			{
				ASSERT(attachment.colorAttachment < executionState.dynamicRendering->getColorAttachmentCount());

				const VkRenderingAttachmentInfo *colorAttachment =
				    executionState.dynamicRendering->getColorAttachment(attachment.colorAttachment);

				if(colorAttachment)
				{
					vk::ImageView *imageView = vk::Cast(colorAttachment->imageView);
					if(imageView)
					{
						imageView->clear(attachment.clearValue, VK_IMAGE_ASPECT_COLOR_BIT, rect, viewMask);
					}
				}
			}

			if(attachment.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT)
			{
				const VkRenderingAttachmentInfo &depthAttachment = executionState.dynamicRendering->getDepthAttachment();

				vk::ImageView *imageView = vk::Cast(depthAttachment.imageView);
				if(imageView)
				{
					imageView->clear(attachment.clearValue, VK_IMAGE_ASPECT_DEPTH_BIT, rect, viewMask);
				}
			}

			if(attachment.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT)
			{
				const VkRenderingAttachmentInfo &stencilAttachment = executionState.dynamicRendering->getStencilAttachment();

				vk::ImageView *imageView = vk::Cast(stencilAttachment.imageView);
				if(imageView)
				{
					imageView->clear(attachment.clearValue, VK_IMAGE_ASPECT_STENCIL_BIT, rect, viewMask);
				}
			}
		}
	}

	std::string description() override { return "vkCmdClearAttachment()"; }

private:
	const VkClearAttachment attachment;
	const VkClearRect rect;
};

class CmdBlitImage : public vk::CommandBuffer::Command
{
public:
	CmdBlitImage(const vk::Image *srcImage, vk::Image *dstImage, const VkImageBlit2 &region, VkFilter filter)
	    : srcImage(srcImage)
	    , dstImage(dstImage)
	    , region(region)
	    , filter(filter)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		srcImage->blitTo(dstImage, region, filter);
	}

	std::string description() override { return "vkCmdBlitImage()"; }

private:
	const vk::Image *const srcImage;
	vk::Image *const dstImage;
	const VkImageBlit2 region;
	const VkFilter filter;
};

class CmdResolveImage : public vk::CommandBuffer::Command
{
public:
	CmdResolveImage(const vk::Image *srcImage, vk::Image *dstImage, const VkImageResolve2 &region)
	    : srcImage(srcImage)
	    , dstImage(dstImage)
	    , region(region)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		srcImage->resolveTo(dstImage, region);
	}

	std::string description() override { return "vkCmdBlitImage()"; }

private:
	const vk::Image *const srcImage;
	vk::Image *const dstImage;
	const VkImageResolve2 region;
};

class CmdPipelineBarrier : public vk::CommandBuffer::Command
{
public:
	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		// This is a very simple implementation that simply calls sw::Renderer::synchronize(),
		// since the driver is free to move the source stage towards the bottom of the pipe
		// and the target stage towards the top, so a full pipeline sync is spec compliant.
		executionState.renderer->synchronize();

		// Right now all buffers are read-only in drawcalls but a similar mechanism will be required once we support SSBOs.

		// Also note that this would be a good moment to update cube map borders or decompress compressed textures, if necessary.
	}

	std::string description() override { return "vkCmdPipelineBarrier()"; }
};

class CmdSignalEvent : public vk::CommandBuffer::Command
{
public:
	CmdSignalEvent(vk::Event *ev)
	    : ev(ev)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.renderer->synchronize();
		ev->signal();
	}

	std::string description() override { return "vkCmdSignalEvent()"; }

private:
	vk::Event *const ev;
};

class CmdResetEvent : public vk::CommandBuffer::Command
{
public:
	CmdResetEvent(vk::Event *ev, VkPipelineStageFlags2 stageMask)
	    : ev(ev)
	    , stageMask(stageMask)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		ev->reset();
	}

	std::string description() override { return "vkCmdResetEvent()"; }

private:
	vk::Event *const ev;
	const VkPipelineStageFlags2 stageMask;  // FIXME(b/117835459): We currently ignore the flags and reset the event at the last stage
};

class CmdWaitEvent : public vk::CommandBuffer::Command
{
public:
	CmdWaitEvent(vk::Event *ev)
	    : ev(ev)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		executionState.renderer->synchronize();
		ev->wait();
	}

	std::string description() override { return "vkCmdWaitEvent()"; }

private:
	vk::Event *const ev;
};

class CmdBindDescriptorSets : public vk::CommandBuffer::Command
{
public:
	CmdBindDescriptorSets(VkPipelineBindPoint pipelineBindPoint,
	                      uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets,
	                      uint32_t firstDynamicOffset, uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets)
	    : pipelineBindPoint(pipelineBindPoint)
	    , firstSet(firstSet)
	    , descriptorSetCount(descriptorSetCount)
	    , firstDynamicOffset(firstDynamicOffset)
	    , dynamicOffsetCount(dynamicOffsetCount)
	{
		for(uint32_t i = 0; i < descriptorSetCount; i++)
		{
			// We need both a descriptor set object for updates and a descriptor set data pointer for routines
			descriptorSetObjects[firstSet + i] = vk::Cast(pDescriptorSets[i]);
			descriptorSets[firstSet + i] = vk::Cast(pDescriptorSets[i])->getDataAddress();
		}

		for(uint32_t i = 0; i < dynamicOffsetCount; i++)
		{
			dynamicOffsets[firstDynamicOffset + i] = pDynamicOffsets[i];
		}
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		ASSERT((size_t)pipelineBindPoint < executionState.pipelineState.size());
		ASSERT(firstSet + descriptorSetCount <= vk::MAX_BOUND_DESCRIPTOR_SETS);
		ASSERT(firstDynamicOffset + dynamicOffsetCount <= vk::MAX_DESCRIPTOR_SET_COMBINED_BUFFERS_DYNAMIC);

		auto &pipelineState = executionState.pipelineState[pipelineBindPoint];

		for(uint32_t i = firstSet; i < firstSet + descriptorSetCount; i++)
		{
			pipelineState.descriptorSetObjects[i] = descriptorSetObjects[i];
			pipelineState.descriptorSets[i] = descriptorSets[i];
		}

		for(uint32_t i = firstDynamicOffset; i < firstDynamicOffset + dynamicOffsetCount; i++)
		{
			pipelineState.descriptorDynamicOffsets[i] = dynamicOffsets[i];
		}
	}

	std::string description() override { return "vkCmdBindDescriptorSets()"; }

private:
	const VkPipelineBindPoint pipelineBindPoint;
	const uint32_t firstSet;
	const uint32_t descriptorSetCount;
	const uint32_t firstDynamicOffset;
	const uint32_t dynamicOffsetCount;

	vk::DescriptorSet::Array descriptorSetObjects;
	vk::DescriptorSet::Bindings descriptorSets;
	vk::DescriptorSet::DynamicOffsets dynamicOffsets;
};

class CmdSetPushConstants : public vk::CommandBuffer::Command
{
public:
	CmdSetPushConstants(uint32_t offset, uint32_t size, const void *pValues)
	    : offset(offset)
	    , size(size)
	{
		ASSERT(offset < vk::MAX_PUSH_CONSTANT_SIZE);
		ASSERT(offset + size <= vk::MAX_PUSH_CONSTANT_SIZE);

		memcpy(data, pValues, size);
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		memcpy(&executionState.pushConstants.data[offset], data, size);
	}

	std::string description() override { return "vkCmdSetPushConstants()"; }

private:
	const uint32_t offset;
	const uint32_t size;
	unsigned char data[vk::MAX_PUSH_CONSTANT_SIZE];
};

class CmdBeginQuery : public vk::CommandBuffer::Command
{
public:
	CmdBeginQuery(vk::QueryPool *queryPool, uint32_t query, VkQueryControlFlags flags)
	    : queryPool(queryPool)
	    , query(query)
	    , flags(flags)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		// "If queries are used while executing a render pass instance that has multiview enabled, the query uses
		//  N consecutive query indices in the query pool (starting at `query`)"
		for(uint32_t i = 0; i < executionState.viewCount(); i++)
		{
			queryPool->begin(query + i, flags);
		}

		// The renderer accumulates the result into a single query.
		ASSERT(queryPool->getType() == VK_QUERY_TYPE_OCCLUSION);
		executionState.renderer->addQuery(queryPool->getQuery(query));
	}

	std::string description() override { return "vkCmdBeginQuery()"; }

private:
	vk::QueryPool *const queryPool;
	const uint32_t query;
	const VkQueryControlFlags flags;
};

class CmdEndQuery : public vk::CommandBuffer::Command
{
public:
	CmdEndQuery(vk::QueryPool *queryPool, uint32_t query)
	    : queryPool(queryPool)
	    , query(query)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		// The renderer accumulates the result into a single query.
		ASSERT(queryPool->getType() == VK_QUERY_TYPE_OCCLUSION);
		executionState.renderer->removeQuery(queryPool->getQuery(query));

		// "implementations may write the total result to the first query and write zero to the other queries."
		for(uint32_t i = 1; i < executionState.viewCount(); i++)
		{
			queryPool->getQuery(query + i)->set(0);
		}

		for(uint32_t i = 0; i < executionState.viewCount(); i++)
		{
			queryPool->end(query + i);
		}
	}

	std::string description() override { return "vkCmdEndQuery()"; }

private:
	vk::QueryPool *const queryPool;
	const uint32_t query;
};

class CmdResetQueryPool : public vk::CommandBuffer::Command
{
public:
	CmdResetQueryPool(vk::QueryPool *queryPool, uint32_t firstQuery, uint32_t queryCount)
	    : queryPool(queryPool)
	    , firstQuery(firstQuery)
	    , queryCount(queryCount)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		queryPool->reset(firstQuery, queryCount);
	}

	std::string description() override { return "vkCmdResetQueryPool()"; }

private:
	vk::QueryPool *const queryPool;
	const uint32_t firstQuery;
	const uint32_t queryCount;
};

class CmdWriteTimeStamp : public vk::CommandBuffer::Command
{
public:
	CmdWriteTimeStamp(vk::QueryPool *queryPool, uint32_t query, VkPipelineStageFlagBits2 stage)
	    : queryPool(queryPool)
	    , query(query)
	    , stage(stage)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		if(stage & ~(VK_PIPELINE_STAGE_2_TOP_OF_PIPE_BIT | VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT))
		{
			// The `top of pipe` and `draw indirect` stages are handled in command buffer processing so a timestamp write
			// done in those stages can just be done here without any additional synchronization.
			// Everything else is deferred to the Renderer; we will treat those stages all as if they were
			// `bottom of pipe`.
			//
			// FIXME(chrisforbes): once Marl is integrated, do this in a task so we don't have to stall here.
			executionState.renderer->synchronize();
		}

		// "the timestamp uses N consecutive query indices in the query pool (starting at `query`) where
		//  N is the number of bits set in the view mask of the subpass the command is executed in."
		for(uint32_t i = 0; i < executionState.viewCount(); i++)
		{
			queryPool->writeTimestamp(query + i);
		}
	}

	std::string description() override { return "vkCmdWriteTimeStamp()"; }

private:
	vk::QueryPool *const queryPool;
	const uint32_t query;
	const VkPipelineStageFlagBits2 stage;
};

class CmdCopyQueryPoolResults : public vk::CommandBuffer::Command
{
public:
	CmdCopyQueryPoolResults(const vk::QueryPool *queryPool, uint32_t firstQuery, uint32_t queryCount,
	                        vk::Buffer *dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags)
	    : queryPool(queryPool)
	    , firstQuery(firstQuery)
	    , queryCount(queryCount)
	    , dstBuffer(dstBuffer)
	    , dstOffset(dstOffset)
	    , stride(stride)
	    , flags(flags)
	{
	}

	void execute(vk::CommandBuffer::ExecutionState &executionState) override
	{
		queryPool->getResults(firstQuery, queryCount, dstBuffer->getSize() - dstOffset,
		                      dstBuffer->getOffsetPointer(dstOffset), stride, flags);
	}

	std::string description() override { return "vkCmdCopyQueryPoolResults()"; }

private:
	const vk::QueryPool *const queryPool;
	const uint32_t firstQuery;
	const uint32_t queryCount;
	vk::Buffer *const dstBuffer;
	const VkDeviceSize dstOffset;
	const VkDeviceSize stride;
	const VkQueryResultFlags flags;
};

}  // anonymous namespace

namespace vk {

DynamicRendering::DynamicRendering(const VkRenderingInfo *pRenderingInfo)
    : renderArea(pRenderingInfo->renderArea)
    , layerCount(pRenderingInfo->layerCount)
    , viewMask(pRenderingInfo->viewMask)
    , colorAttachmentCount(pRenderingInfo->colorAttachmentCount)
    , flags(pRenderingInfo->flags)
{
	if(colorAttachmentCount > 0)
	{
		for(uint32_t i = 0; i < colorAttachmentCount; ++i)
		{
			colorAttachments[i] = pRenderingInfo->pColorAttachments[i];
		}
	}

	if(pRenderingInfo->pDepthAttachment)
	{
		depthAttachment = *pRenderingInfo->pDepthAttachment;
	}

	if(pRenderingInfo->pStencilAttachment)
	{
		stencilAttachment = *pRenderingInfo->pStencilAttachment;
	}
}

void DynamicRendering::getAttachments(Attachments *attachments) const
{
	for(uint32_t i = 0; i < sw::MAX_COLOR_BUFFERS; ++i)
	{
		attachments->colorBuffer[i] = nullptr;
	}
	for(uint32_t i = 0; i < sw::MAX_COLOR_BUFFERS; ++i)
	{
		const uint32_t location = attachments->indexToLocation[i];
		if(i < colorAttachmentCount && location != VK_ATTACHMENT_UNUSED)
		{
			attachments->colorBuffer[location] = vk::Cast(colorAttachments[i].imageView);
		}
	}
	attachments->depthBuffer = vk::Cast(depthAttachment.imageView);
	attachments->stencilBuffer = vk::Cast(stencilAttachment.imageView);
}

CommandBuffer::CommandBuffer(Device *device, VkCommandBufferLevel pLevel)
    : device(device)
    , level(pLevel)
{
}

void CommandBuffer::destroy(const VkAllocationCallbacks *pAllocator)
{
}

void CommandBuffer::resetState()
{
	// FIXME (b/119409619): replace this vector by an allocator so we can control all memory allocations
	commands.clear();

	state = INITIAL;
}

VkResult CommandBuffer::begin(VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo *pInheritanceInfo)
{
	ASSERT((state != RECORDING) && (state != PENDING));

	// Nothing interesting to do based on flags. We don't have any optimizations
	// to apply for ONE_TIME_SUBMIT or (lack of) SIMULTANEOUS_USE. RENDER_PASS_CONTINUE
	// must also provide a non-null pInheritanceInfo, which we don't implement yet, but is caught below.
	(void)flags;

	// pInheritanceInfo merely contains optimization hints, so we currently ignore it

	// "pInheritanceInfo is a pointer to a VkCommandBufferInheritanceInfo structure, used if commandBuffer is a
	//  secondary command buffer. If this is a primary command buffer, then this value is ignored."
	if(level == VK_COMMAND_BUFFER_LEVEL_SECONDARY)
	{
		if(pInheritanceInfo->queryFlags != 0)
		{
			// "If the inherited queries feature is not enabled, queryFlags must be 0"
			UNSUPPORTED("VkPhysicalDeviceFeatures::inheritedQueries");
		}
	}

	if(state != INITIAL)
	{
		// Implicit reset
		resetState();
	}

	state = RECORDING;

	return VK_SUCCESS;
}

VkResult CommandBuffer::end()
{
	ASSERT(state == RECORDING);

	state = EXECUTABLE;

	return VK_SUCCESS;
}

VkResult CommandBuffer::reset(VkCommandPoolResetFlags flags)
{
	ASSERT(state != PENDING);

	resetState();

	return VK_SUCCESS;
}

template<typename T, typename... Args>
void CommandBuffer::addCommand(Args &&...args)
{
	// FIXME (b/119409619): use an allocator here so we can control all memory allocations
	commands.push_back(std::make_unique<T>(std::forward<Args>(args)...));
}

void CommandBuffer::beginRenderPass(RenderPass *renderPass, Framebuffer *framebuffer, VkRect2D renderArea,
                                    uint32_t clearValueCount, const VkClearValue *clearValues, VkSubpassContents contents,
                                    const VkRenderPassAttachmentBeginInfo *attachmentInfo)
{
	ASSERT(state == RECORDING);

	addCommand<::CmdBeginRenderPass>(renderPass, framebuffer, renderArea, clearValueCount, clearValues, attachmentInfo);
}

void CommandBuffer::nextSubpass(VkSubpassContents contents)
{
	ASSERT(state == RECORDING);

	addCommand<::CmdNextSubpass>();
}

void CommandBuffer::endRenderPass()
{
	addCommand<::CmdEndRenderPass>();
}

void CommandBuffer::executeCommands(uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers)
{
	ASSERT(state == RECORDING);

	for(uint32_t i = 0; i < commandBufferCount; ++i)
	{
		addCommand<::CmdExecuteCommands>(vk::Cast(pCommandBuffers[i]));
	}
}

void CommandBuffer::beginRendering(const VkRenderingInfo *pRenderingInfo)
{
	ASSERT(state == RECORDING);

	addCommand<::CmdBeginRendering>(pRenderingInfo);
}

void CommandBuffer::endRendering()
{
	ASSERT(state == RECORDING);

	addCommand<::CmdEndRendering>();
}

void CommandBuffer::setDeviceMask(uint32_t deviceMask)
{
	// SwiftShader only has one device, so we ignore the device mask
}

void CommandBuffer::dispatchBase(uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
                                 uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	addCommand<::CmdDispatch>(baseGroupX, baseGroupY, baseGroupZ, groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::pipelineBarrier(const VkDependencyInfo &pDependencyInfo)
{
	addCommand<::CmdPipelineBarrier>();
}

void CommandBuffer::bindPipeline(VkPipelineBindPoint pipelineBindPoint, Pipeline *pipeline)
{
	switch(pipelineBindPoint)
	{
	case VK_PIPELINE_BIND_POINT_COMPUTE:
	case VK_PIPELINE_BIND_POINT_GRAPHICS:
		addCommand<::CmdPipelineBind>(pipelineBindPoint, pipeline);
		break;
	default:
		UNSUPPORTED("VkPipelineBindPoint %d", int(pipelineBindPoint));
	}
}

void CommandBuffer::bindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount,
                                      const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
                                      const VkDeviceSize *pSizes, const VkDeviceSize *pStrides)
{
	for(uint32_t i = 0; i < bindingCount; ++i)
	{
		addCommand<::CmdVertexBufferBind>(i + firstBinding, vk::Cast(pBuffers[i]), pOffsets[i],
		                                  pSizes ? pSizes[i] : 0,
		                                  pStrides ? pStrides[i] : 0,
		                                  pStrides);
	}
}

void CommandBuffer::beginQuery(QueryPool *queryPool, uint32_t query, VkQueryControlFlags flags)
{
	addCommand<::CmdBeginQuery>(queryPool, query, flags);
}

void CommandBuffer::endQuery(QueryPool *queryPool, uint32_t query)
{
	addCommand<::CmdEndQuery>(queryPool, query);
}

void CommandBuffer::resetQueryPool(QueryPool *queryPool, uint32_t firstQuery, uint32_t queryCount)
{
	addCommand<::CmdResetQueryPool>(queryPool, firstQuery, queryCount);
}

void CommandBuffer::writeTimestamp(VkPipelineStageFlags2 pipelineStage, QueryPool *queryPool, uint32_t query)
{
	addCommand<::CmdWriteTimeStamp>(queryPool, query, pipelineStage);
}

void CommandBuffer::copyQueryPoolResults(const QueryPool *queryPool, uint32_t firstQuery, uint32_t queryCount,
                                         Buffer *dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags)
{
	addCommand<::CmdCopyQueryPoolResults>(queryPool, firstQuery, queryCount, dstBuffer, dstOffset, stride, flags);
}

void CommandBuffer::pushConstants(PipelineLayout *layout, VkShaderStageFlags stageFlags,
                                  uint32_t offset, uint32_t size, const void *pValues)
{
	addCommand<::CmdSetPushConstants>(offset, size, pValues);
}

void CommandBuffer::setViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport *pViewports)
{
	if(firstViewport != 0 || viewportCount > 1)
	{
		UNSUPPORTED("VkPhysicalDeviceFeatures::multiViewport");
	}

	for(uint32_t i = 0; i < viewportCount; i++)
	{
		addCommand<::CmdSetViewport>(pViewports[i], i + firstViewport);
	}
}

void CommandBuffer::setScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D *pScissors)
{
	if(firstScissor != 0 || scissorCount > 1)
	{
		UNSUPPORTED("VkPhysicalDeviceFeatures::multiViewport");
	}

	for(uint32_t i = 0; i < scissorCount; i++)
	{
		addCommand<::CmdSetScissor>(pScissors[i], i + firstScissor);
	}
}

void CommandBuffer::setLineWidth(float lineWidth)
{
	addCommand<::CmdSetLineWidth>(lineWidth);
}

void CommandBuffer::setDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor)
{
	addCommand<::CmdSetDepthBias>(depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

void CommandBuffer::setBlendConstants(const float blendConstants[4])
{
	addCommand<::CmdSetBlendConstants>(blendConstants);
}

void CommandBuffer::setDepthBounds(float minDepthBounds, float maxDepthBounds)
{
	addCommand<::CmdSetDepthBounds>(minDepthBounds, maxDepthBounds);
}

void CommandBuffer::setStencilCompareMask(VkStencilFaceFlags faceMask, uint32_t compareMask)
{
	// faceMask must not be 0
	ASSERT(faceMask != 0);

	addCommand<::CmdSetStencilCompareMask>(faceMask, compareMask);
}

void CommandBuffer::setStencilWriteMask(VkStencilFaceFlags faceMask, uint32_t writeMask)
{
	// faceMask must not be 0
	ASSERT(faceMask != 0);

	addCommand<::CmdSetStencilWriteMask>(faceMask, writeMask);
}

void CommandBuffer::setStencilReference(VkStencilFaceFlags faceMask, uint32_t reference)
{
	// faceMask must not be 0
	ASSERT(faceMask != 0);

	addCommand<::CmdSetStencilReference>(faceMask, reference);
}

void CommandBuffer::setCullMode(VkCullModeFlags cullMode)
{
	addCommand<::CmdSetCullMode>(cullMode);
}

void CommandBuffer::setDepthBoundsTestEnable(VkBool32 depthBoundsTestEnable)
{
	addCommand<::CmdSetDepthBoundsTestEnable>(depthBoundsTestEnable);
}

void CommandBuffer::setDepthCompareOp(VkCompareOp depthCompareOp)
{
	addCommand<::CmdSetDepthCompareOp>(depthCompareOp);
}

void CommandBuffer::setDepthTestEnable(VkBool32 depthTestEnable)
{
	addCommand<::CmdSetDepthTestEnable>(depthTestEnable);
}

void CommandBuffer::setDepthWriteEnable(VkBool32 depthWriteEnable)
{
	addCommand<::CmdSetDepthWriteEnable>(depthWriteEnable);
}

void CommandBuffer::setFrontFace(VkFrontFace frontFace)
{
	addCommand<::CmdSetFrontFace>(frontFace);
}

void CommandBuffer::setPrimitiveTopology(VkPrimitiveTopology primitiveTopology)
{
	addCommand<::CmdSetPrimitiveTopology>(primitiveTopology);
}

void CommandBuffer::setScissorWithCount(uint32_t scissorCount, const VkRect2D *pScissors)
{
	addCommand<::CmdSetScissorWithCount>(scissorCount, pScissors);
}

void CommandBuffer::setStencilOp(VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp)
{
	addCommand<::CmdSetStencilOp>(faceMask, failOp, passOp, depthFailOp, compareOp);
}

void CommandBuffer::setStencilTestEnable(VkBool32 stencilTestEnable)
{
	addCommand<::CmdSetStencilTestEnable>(stencilTestEnable);
}

void CommandBuffer::setViewportWithCount(uint32_t viewportCount, const VkViewport *pViewports)
{
	addCommand<::CmdSetViewportWithCount>(viewportCount, pViewports);
}

void CommandBuffer::setRasterizerDiscardEnable(VkBool32 rasterizerDiscardEnable)
{
	addCommand<::CmdSetRasterizerDiscardEnable>(rasterizerDiscardEnable);
}

void CommandBuffer::setDepthBiasEnable(VkBool32 depthBiasEnable)
{
	addCommand<::CmdSetDepthBiasEnable>(depthBiasEnable);
}

void CommandBuffer::setPrimitiveRestartEnable(VkBool32 primitiveRestartEnable)
{
	addCommand<::CmdSetPrimitiveRestartEnable>(primitiveRestartEnable);
}

void CommandBuffer::setVertexInput(uint32_t vertexBindingDescriptionCount,
                                   const VkVertexInputBindingDescription2EXT *pVertexBindingDescriptions,
                                   uint32_t vertexAttributeDescriptionCount,
                                   const VkVertexInputAttributeDescription2EXT *pVertexAttributeDescriptions)
{
	addCommand<::CmdSetVertexInput>(vertexBindingDescriptionCount, pVertexBindingDescriptions,
	                                vertexAttributeDescriptionCount, pVertexAttributeDescriptions);
}

void CommandBuffer::bindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, const PipelineLayout *pipelineLayout,
                                       uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets,
                                       uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets)
{
	ASSERT(state == RECORDING);

	auto firstDynamicOffset = (dynamicOffsetCount != 0) ? pipelineLayout->getDynamicOffsetIndex(firstSet, 0) : 0;

	addCommand<::CmdBindDescriptorSets>(
	    pipelineBindPoint, firstSet, descriptorSetCount, pDescriptorSets,
	    firstDynamicOffset, dynamicOffsetCount, pDynamicOffsets);
}

void CommandBuffer::bindIndexBuffer(Buffer *buffer, VkDeviceSize offset, VkIndexType indexType)
{
	addCommand<::CmdIndexBufferBind>(buffer, offset, indexType);
}

void CommandBuffer::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	addCommand<::CmdDispatch>(0, 0, 0, groupCountX, groupCountY, groupCountZ);
}

void CommandBuffer::dispatchIndirect(Buffer *buffer, VkDeviceSize offset)
{
	addCommand<::CmdDispatchIndirect>(buffer, offset);
}

void CommandBuffer::copyBuffer(const VkCopyBufferInfo2 &copyBufferInfo)
{
	ASSERT(state == RECORDING);

	for(uint32_t i = 0; i < copyBufferInfo.regionCount; i++)
	{
		addCommand<::CmdCopyBuffer>(
		    vk::Cast(copyBufferInfo.srcBuffer),
		    vk::Cast(copyBufferInfo.dstBuffer),
		    copyBufferInfo.pRegions[i]);
	}
}

void CommandBuffer::copyImage(const VkCopyImageInfo2 &copyImageInfo)
{
	ASSERT(state == RECORDING);
	ASSERT(copyImageInfo.srcImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL ||
	       copyImageInfo.srcImageLayout == VK_IMAGE_LAYOUT_GENERAL);
	ASSERT(copyImageInfo.dstImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ||
	       copyImageInfo.dstImageLayout == VK_IMAGE_LAYOUT_GENERAL);

	for(uint32_t i = 0; i < copyImageInfo.regionCount; i++)
	{
		addCommand<::CmdCopyImage>(
		    vk::Cast(copyImageInfo.srcImage),
		    vk::Cast(copyImageInfo.dstImage),
		    copyImageInfo.pRegions[i]);
	}
}

void CommandBuffer::blitImage(const VkBlitImageInfo2 &blitImageInfo)
{
	ASSERT(state == RECORDING);
	ASSERT(blitImageInfo.srcImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL ||
	       blitImageInfo.srcImageLayout == VK_IMAGE_LAYOUT_GENERAL);
	ASSERT(blitImageInfo.dstImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ||
	       blitImageInfo.dstImageLayout == VK_IMAGE_LAYOUT_GENERAL);

	for(uint32_t i = 0; i < blitImageInfo.regionCount; i++)
	{
		addCommand<::CmdBlitImage>(
		    vk::Cast(blitImageInfo.srcImage),
		    vk::Cast(blitImageInfo.dstImage),
		    blitImageInfo.pRegions[i],
		    blitImageInfo.filter);
	}
}

void CommandBuffer::copyBufferToImage(const VkCopyBufferToImageInfo2 &copyBufferToImageInfo)
{
	ASSERT(state == RECORDING);

	for(uint32_t i = 0; i < copyBufferToImageInfo.regionCount; i++)
	{
		addCommand<::CmdCopyBufferToImage>(
		    vk::Cast(copyBufferToImageInfo.srcBuffer),
		    vk::Cast(copyBufferToImageInfo.dstImage),
		    copyBufferToImageInfo.pRegions[i]);
	}
}

void CommandBuffer::copyImageToBuffer(const VkCopyImageToBufferInfo2 &copyImageToBufferInfo)
{
	ASSERT(state == RECORDING);
	ASSERT(copyImageToBufferInfo.srcImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL ||
	       copyImageToBufferInfo.srcImageLayout == VK_IMAGE_LAYOUT_GENERAL);

	for(uint32_t i = 0; i < copyImageToBufferInfo.regionCount; i++)
	{
		addCommand<::CmdCopyImageToBuffer>(
		    vk::Cast(copyImageToBufferInfo.srcImage),
		    vk::Cast(copyImageToBufferInfo.dstBuffer),
		    copyImageToBufferInfo.pRegions[i]);
	}
}

void CommandBuffer::updateBuffer(Buffer *dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData)
{
	ASSERT(state == RECORDING);

	addCommand<::CmdUpdateBuffer>(dstBuffer, dstOffset, dataSize, reinterpret_cast<const uint8_t *>(pData));
}

void CommandBuffer::fillBuffer(Buffer *dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data)
{
	ASSERT(state == RECORDING);

	addCommand<::CmdFillBuffer>(dstBuffer, dstOffset, size, data);
}

void CommandBuffer::clearColorImage(Image *image, VkImageLayout imageLayout, const VkClearColorValue *pColor,
                                    uint32_t rangeCount, const VkImageSubresourceRange *pRanges)
{
	ASSERT(state == RECORDING);

	for(uint32_t i = 0; i < rangeCount; i++)
	{
		addCommand<::CmdClearColorImage>(image, *pColor, pRanges[i]);
	}
}

void CommandBuffer::clearDepthStencilImage(Image *image, VkImageLayout imageLayout, const VkClearDepthStencilValue *pDepthStencil,
                                           uint32_t rangeCount, const VkImageSubresourceRange *pRanges)
{
	ASSERT(state == RECORDING);

	for(uint32_t i = 0; i < rangeCount; i++)
	{
		addCommand<::CmdClearDepthStencilImage>(image, *pDepthStencil, pRanges[i]);
	}
}

void CommandBuffer::clearAttachments(uint32_t attachmentCount, const VkClearAttachment *pAttachments,
                                     uint32_t rectCount, const VkClearRect *pRects)
{
	ASSERT(state == RECORDING);

	for(uint32_t i = 0; i < attachmentCount; i++)
	{
		for(uint32_t j = 0; j < rectCount; j++)
		{
			addCommand<::CmdClearAttachment>(pAttachments[i], pRects[j]);
		}
	}
}

void CommandBuffer::resolveImage(const VkResolveImageInfo2 &resolveImageInfo)
{
	ASSERT(state == RECORDING);
	ASSERT(resolveImageInfo.srcImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL ||
	       resolveImageInfo.srcImageLayout == VK_IMAGE_LAYOUT_GENERAL);
	ASSERT(resolveImageInfo.dstImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ||
	       resolveImageInfo.dstImageLayout == VK_IMAGE_LAYOUT_GENERAL);

	for(uint32_t i = 0; i < resolveImageInfo.regionCount; i++)
	{
		addCommand<::CmdResolveImage>(
		    vk::Cast(resolveImageInfo.srcImage),
		    vk::Cast(resolveImageInfo.dstImage),
		    resolveImageInfo.pRegions[i]);
	}
}

void CommandBuffer::setEvent(Event *event, const VkDependencyInfo &pDependencyInfo)
{
	ASSERT(state == RECORDING);

	// TODO(b/117835459): We currently ignore the flags and signal the event at the last stage

	addCommand<::CmdSignalEvent>(event);
}

void CommandBuffer::resetEvent(Event *event, VkPipelineStageFlags2 stageMask)
{
	ASSERT(state == RECORDING);

	addCommand<::CmdResetEvent>(event, stageMask);
}

void CommandBuffer::waitEvents(uint32_t eventCount, const VkEvent *pEvents, const VkDependencyInfo &pDependencyInfo)
{
	ASSERT(state == RECORDING);

	// TODO(b/117835459): Since we always do a full barrier, all memory barrier related arguments are ignored

	// Note: srcStageMask and dstStageMask are currently ignored
	for(uint32_t i = 0; i < eventCount; i++)
	{
		addCommand<::CmdWaitEvent>(vk::Cast(pEvents[i]));
	}
}

void CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	addCommand<::CmdDraw>(vertexCount, instanceCount, firstVertex, firstInstance);
}

void CommandBuffer::drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
	addCommand<::CmdDrawIndexed>(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void CommandBuffer::drawIndirect(Buffer *buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
	addCommand<::CmdDrawIndirect>(buffer, offset, drawCount, stride);
}

void CommandBuffer::drawIndexedIndirect(Buffer *buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride)
{
	addCommand<::CmdDrawIndexedIndirect>(buffer, offset, drawCount, stride);
}

void CommandBuffer::beginDebugUtilsLabel(const VkDebugUtilsLabelEXT *pLabelInfo)
{
	// Optional debug label region
}

void CommandBuffer::endDebugUtilsLabel()
{
	// Close debug label region opened with beginDebugUtilsLabel()
}

void CommandBuffer::insertDebugUtilsLabel(const VkDebugUtilsLabelEXT *pLabelInfo)
{
	// Optional single debug label
}

void CommandBuffer::submit(CommandBuffer::ExecutionState &executionState)
{
	// Perform recorded work
	state = PENDING;

	for(auto &command : commands)
	{
		command->execute(executionState);
	}

	// After work is completed
	state = EXECUTABLE;
}

void CommandBuffer::submitSecondary(CommandBuffer::ExecutionState &executionState) const
{
	for(auto &command : commands)
	{
		command->execute(executionState);
	}
}

void CommandBuffer::ExecutionState::bindAttachments(Attachments *attachments)
{
	// Binds all the attachments for the current subpass
	// Ideally this would be performed by BeginRenderPass and NextSubpass, but
	// there is too much stomping of the renderer's state by setContext() in
	// draws.

	if(renderPass)
	{
		const auto &subpass = renderPass->getSubpass(subpassIndex);

		for(auto i = 0u; i < subpass.colorAttachmentCount; i++)
		{
			auto attachmentReference = subpass.pColorAttachments[i];
			if(attachmentReference.attachment != VK_ATTACHMENT_UNUSED)
			{
				attachments->colorBuffer[i] = renderPassFramebuffer->getAttachment(attachmentReference.attachment);
			}
		}

		auto attachmentReference = subpass.pDepthStencilAttachment;
		if(attachmentReference && attachmentReference->attachment != VK_ATTACHMENT_UNUSED)
		{
			auto *attachment = renderPassFramebuffer->getAttachment(attachmentReference->attachment);
			if(attachment->hasDepthAspect())
			{
				attachments->depthBuffer = attachment;
			}
			if(attachment->hasStencilAspect())
			{
				attachments->stencilBuffer = attachment;
			}
		}
	}
	else if(dynamicRendering)
	{
		dynamicRendering->getAttachments(attachments);
	}
}

VkRect2D CommandBuffer::ExecutionState::getRenderArea() const
{
	VkRect2D renderArea = {};

	if(renderPassFramebuffer)
	{
		renderArea.extent = renderPassFramebuffer->getExtent();
	}
	else if(dynamicRendering)
	{
		renderArea = dynamicRendering->getRenderArea();
	}

	return renderArea;
}

// The layer mask is the same as the view mask when multiview is enabled,
// or 1 if multiview is disabled.
uint32_t CommandBuffer::ExecutionState::getLayerMask() const
{
	uint32_t layerMask = 1;

	if(renderPass)
	{
		layerMask = renderPass->getViewMask(subpassIndex);
	}
	else if(dynamicRendering)
	{
		layerMask = dynamicRendering->getViewMask();
	}

	return sw::max(layerMask, 1u);
}

// Returns the number of bits set in the view mask, or 1 if multiview is disabled.
uint32_t CommandBuffer::ExecutionState::viewCount() const
{
	return static_cast<uint32_t>(std::bitset<32>(getLayerMask()).count());
}

}  // namespace vk
