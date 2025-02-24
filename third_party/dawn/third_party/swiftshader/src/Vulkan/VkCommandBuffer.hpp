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

#ifndef VK_COMMAND_BUFFER_HPP_
#define VK_COMMAND_BUFFER_HPP_

#include "VkConfig.hpp"
#include "VkDescriptorSet.hpp"
#include "VkPipeline.hpp"
#include "System/Synchronization.hpp"

#include <memory>
#include <vector>

namespace sw {

class Context;
class Renderer;

}  // namespace sw

namespace vk {

class Device;
class Buffer;
class Event;
class Framebuffer;
class Image;
class Pipeline;
class PipelineLayout;
class QueryPool;
class RenderPass;

struct DynamicRendering
{
	DynamicRendering(const VkRenderingInfo *pRenderingInfo);

	void getAttachments(Attachments *attachments) const;
	VkRect2D getRenderArea() const { return renderArea; }
	uint32_t getLayerCount() const { return layerCount; }
	uint32_t getViewMask() const { return viewMask; }
	uint32_t getColorAttachmentCount() const { return colorAttachmentCount; }
	const VkRenderingAttachmentInfo *getColorAttachment(uint32_t i) const
	{
		return (i < colorAttachmentCount) ? &(colorAttachments[i]) : nullptr;
	}
	const VkRenderingAttachmentInfo &getDepthAttachment() const
	{
		return depthAttachment;
	}
	const VkRenderingAttachmentInfo &getStencilAttachment() const
	{
		return stencilAttachment;
	}
	bool suspend() const { return flags & VK_RENDERING_SUSPENDING_BIT; }
	bool resume() const { return flags & VK_RENDERING_RESUMING_BIT; }

private:
	VkRect2D renderArea = {};
	uint32_t layerCount = 0;
	uint32_t viewMask = 0;
	uint32_t colorAttachmentCount = 0;
	VkRenderingAttachmentInfo colorAttachments[sw::MAX_COLOR_BUFFERS] = { {} };
	bool hasDepthAttachment = false;
	VkRenderingAttachmentInfo depthAttachment = {};
	bool hasStencilAttachment = false;
	VkRenderingAttachmentInfo stencilAttachment = {};
	VkRenderingFlags flags = VkRenderingFlags(0);
};

class CommandBuffer
{
public:
	static constexpr VkSystemAllocationScope GetAllocationScope() { return VK_SYSTEM_ALLOCATION_SCOPE_OBJECT; }

	CommandBuffer(Device *device, VkCommandBufferLevel pLevel);

	void destroy(const VkAllocationCallbacks *pAllocator);

	VkResult begin(VkCommandBufferUsageFlags flags, const VkCommandBufferInheritanceInfo *pInheritanceInfo);
	VkResult end();
	VkResult reset(VkCommandPoolResetFlags flags);

	void beginRenderPass(RenderPass *renderPass, Framebuffer *framebuffer, VkRect2D renderArea,
	                     uint32_t clearValueCount, const VkClearValue *pClearValues, VkSubpassContents contents,
	                     const VkRenderPassAttachmentBeginInfo *attachmentBeginInfo);
	void nextSubpass(VkSubpassContents contents);
	void endRenderPass();
	void executeCommands(uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers);
	void beginRendering(const VkRenderingInfo *pRenderingInfo);
	void endRendering();

	void setDeviceMask(uint32_t deviceMask);
	void dispatchBase(uint32_t baseGroupX, uint32_t baseGroupY, uint32_t baseGroupZ,
	                  uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

	void pipelineBarrier(const VkDependencyInfo &pDependencyInfo);
	void bindPipeline(VkPipelineBindPoint pipelineBindPoint, Pipeline *pipeline);
	void bindVertexBuffers(uint32_t firstBinding, uint32_t bindingCount,
	                       const VkBuffer *pBuffers, const VkDeviceSize *pOffsets,
	                       const VkDeviceSize *pSizes, const VkDeviceSize *pStrides);

	void beginQuery(QueryPool *queryPool, uint32_t query, VkQueryControlFlags flags);
	void endQuery(QueryPool *queryPool, uint32_t query);
	void resetQueryPool(QueryPool *queryPool, uint32_t firstQuery, uint32_t queryCount);
	void writeTimestamp(VkPipelineStageFlags2 pipelineStage, QueryPool *queryPool, uint32_t query);
	void copyQueryPoolResults(const QueryPool *queryPool, uint32_t firstQuery, uint32_t queryCount,
	                          Buffer *dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags);
	void pushConstants(PipelineLayout *layout, VkShaderStageFlags stageFlags,
	                   uint32_t offset, uint32_t size, const void *pValues);

	void setViewport(uint32_t firstViewport, uint32_t viewportCount, const VkViewport *pViewports);
	void setScissor(uint32_t firstScissor, uint32_t scissorCount, const VkRect2D *pScissors);
	void setLineWidth(float lineWidth);
	void setDepthBias(float depthBiasConstantFactor, float depthBiasClamp, float depthBiasSlopeFactor);
	void setBlendConstants(const float blendConstants[4]);
	void setDepthBounds(float minDepthBounds, float maxDepthBounds);
	void setStencilCompareMask(VkStencilFaceFlags faceMask, uint32_t compareMask);
	void setStencilWriteMask(VkStencilFaceFlags faceMask, uint32_t writeMask);
	void setStencilReference(VkStencilFaceFlags faceMask, uint32_t reference);
	void setCullMode(VkCullModeFlags cullMode);
	void setDepthBoundsTestEnable(VkBool32 depthBoundsTestEnable);
	void setDepthCompareOp(VkCompareOp depthCompareOp);
	void setDepthTestEnable(VkBool32 depthTestEnable);
	void setDepthWriteEnable(VkBool32 depthWriteEnable);
	void setFrontFace(VkFrontFace frontFace);
	void setPrimitiveTopology(VkPrimitiveTopology primitiveTopology);
	void setScissorWithCount(uint32_t scissorCount, const VkRect2D *pScissors);
	void setStencilOp(VkStencilFaceFlags faceMask, VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp);
	void setStencilTestEnable(VkBool32 stencilTestEnable);
	void setViewportWithCount(uint32_t viewportCount, const VkViewport *pViewports);
	void setRasterizerDiscardEnable(VkBool32 rasterizerDiscardEnable);
	void setDepthBiasEnable(VkBool32 depthBiasEnable);
	void setPrimitiveRestartEnable(VkBool32 primitiveRestartEnable);
	void setVertexInput(uint32_t vertexBindingDescriptionCount,
			const VkVertexInputBindingDescription2EXT*  pVertexBindingDescriptions,
			uint32_t vertexAttributeDescriptionCount,
			const VkVertexInputAttributeDescription2EXT* pVertexAttributeDescriptions);
	void bindDescriptorSets(VkPipelineBindPoint pipelineBindPoint, const PipelineLayout *layout,
	                        uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets,
	                        uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets);
	void bindIndexBuffer(Buffer *buffer, VkDeviceSize offset, VkIndexType indexType);
	void dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
	void dispatchIndirect(Buffer *buffer, VkDeviceSize offset);
	void copyBuffer(const VkCopyBufferInfo2 &copyBufferInfo);
	void copyImage(const VkCopyImageInfo2 &copyImageInfo);
	void blitImage(const VkBlitImageInfo2 &blitImageInfo);
	void copyBufferToImage(const VkCopyBufferToImageInfo2 &copyBufferToImageInfo);
	void copyImageToBuffer(const VkCopyImageToBufferInfo2 &copyImageToBufferInfo);
	void updateBuffer(Buffer *dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize, const void *pData);
	void fillBuffer(Buffer *dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size, uint32_t data);
	void clearColorImage(Image *image, VkImageLayout imageLayout, const VkClearColorValue *pColor,
	                     uint32_t rangeCount, const VkImageSubresourceRange *pRanges);
	void clearDepthStencilImage(Image *image, VkImageLayout imageLayout, const VkClearDepthStencilValue *pDepthStencil,
	                            uint32_t rangeCount, const VkImageSubresourceRange *pRanges);
	void clearAttachments(uint32_t attachmentCount, const VkClearAttachment *pAttachments,
	                      uint32_t rectCount, const VkClearRect *pRects);
	void resolveImage(const VkResolveImageInfo2 &resolveImageInfo);
	void setEvent(Event *event, const VkDependencyInfo &pDependencyInfo);
	void resetEvent(Event *event, VkPipelineStageFlags2 stageMask);
	void waitEvents(uint32_t eventCount, const VkEvent *pEvents, const VkDependencyInfo &pDependencyInfo);

	void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
	void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance);
	void drawIndirect(Buffer *buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);
	void drawIndexedIndirect(Buffer *buffer, VkDeviceSize offset, uint32_t drawCount, uint32_t stride);

	void beginDebugUtilsLabel(const VkDebugUtilsLabelEXT *pLabelInfo);
	void endDebugUtilsLabel();
	void insertDebugUtilsLabel(const VkDebugUtilsLabelEXT *pLabelInfo);

	// TODO(sugoi): Move ExecutionState out of CommandBuffer (possibly into Device)
	struct ExecutionState
	{
		struct PipelineState
		{
			Pipeline *pipeline = nullptr;
			vk::DescriptorSet::Array descriptorSetObjects = {};
			vk::DescriptorSet::Bindings descriptorSets = {};
			vk::DescriptorSet::DynamicOffsets descriptorDynamicOffsets = {};
		};

		sw::Renderer *renderer = nullptr;
		sw::CountedEvent *events = nullptr;
		RenderPass *renderPass = nullptr;
		Framebuffer *renderPassFramebuffer = nullptr;
		DynamicRendering *dynamicRendering = nullptr;

		// VK_PIPELINE_BIND_POINT_GRAPHICS = 0
		// VK_PIPELINE_BIND_POINT_COMPUTE = 1
		std::array<PipelineState, 2> pipelineState;

		vk::DynamicState dynamicState;

		vk::Pipeline::PushConstantStorage pushConstants;

		VertexInputBinding vertexInputBindings[MAX_VERTEX_INPUT_BINDINGS] = {};
		VertexInputBinding indexBufferBinding;
		VkIndexType indexType;

		uint32_t subpassIndex = 0;

		void bindAttachments(Attachments *attachments);

		VkRect2D getRenderArea() const;
		uint32_t getLayerMask() const;
		uint32_t viewCount() const;
	};

	void submit(CommandBuffer::ExecutionState &executionState);
	void submitSecondary(CommandBuffer::ExecutionState &executionState) const;

	class Command
	{
	public:
		virtual void execute(ExecutionState &executionState) = 0;
		virtual std::string description() = 0;
		virtual ~Command() {}
	};

private:
	void resetState();
	template<typename T, typename... Args>
	void addCommand(Args &&...args);

	enum State
	{
		INITIAL,
		RECORDING,
		EXECUTABLE,
		PENDING,
		INVALID
	};

	Device *const device;
	State state = INITIAL;
	VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	// FIXME (b/119409619): replace this vector by an allocator so we can control all memory allocations
	std::vector<std::unique_ptr<Command>> commands;
};

using DispatchableCommandBuffer = DispatchableObject<CommandBuffer, VkCommandBuffer>;

static inline CommandBuffer *Cast(VkCommandBuffer object)
{
	return DispatchableCommandBuffer::Cast(object);
}

}  // namespace vk

#endif  // VK_COMMAND_BUFFER_HPP_
