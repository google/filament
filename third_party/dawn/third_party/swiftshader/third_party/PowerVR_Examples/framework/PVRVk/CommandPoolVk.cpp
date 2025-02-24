/*!
\brief Function implementations for the Command Pool.
\file PVRVk/CommandPoolVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#include "PVRVk/CommandPoolVk.h"
#include "PVRVk/CommandBufferVk.h"
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
namespace impl {
//!\cond NO_DOXYGEN
CommandPool_::CommandPool_(make_shared_enabler, const DeviceWeakPtr& device, const CommandPoolCreateInfo& createInfo)
	: PVRVkDeviceObjectBase(device), DeviceObjectDebugUtils(), _createInfo(createInfo)
{
	VkCommandPoolCreateInfo vkCreateInfo = {};
	vkCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_COMMAND_POOL_CREATE_INFO);
	vkCreateInfo.flags = static_cast<VkCommandPoolCreateFlags>(_createInfo.getFlags());
	vkCreateInfo.queueFamilyIndex = _createInfo.getQueueFamilyIndex();
	vkThrowIfFailed(getDevice()->getVkBindings().vkCreateCommandPool(getDevice()->getVkHandle(), &vkCreateInfo, NULL, &_vkHandle), "Failed to create CommandPool");
}
//!\endcond

CommandBuffer CommandPool_::allocateCommandBuffer()
{
	VkCommandBuffer cmdBuffer;
	VkCommandBufferAllocateInfo nfo = {};
	nfo.commandBufferCount = 1;
	nfo.commandPool = getVkHandle();
	nfo.level = static_cast<VkCommandBufferLevel>(CommandBufferLevel::e_PRIMARY);
	nfo.sType = static_cast<VkStructureType>(StructureType::e_COMMAND_BUFFER_ALLOCATE_INFO);
	vkThrowIfFailed(getDevice()->getVkBindings().vkAllocateCommandBuffers(getDevice()->getVkHandle(), &nfo, &cmdBuffer), "CommandBuffer Allocation Failure.");
	CommandPool commandPool = shared_from_this();
	return CommandBuffer_::constructShared(_device, commandPool, cmdBuffer);
}

void CommandPool_::allocateCommandBuffers(uint32_t numCommandbuffers, CommandBuffer* outCmdBuffers)
{
	ArrayOrVector<VkCommandBuffer, 4> cmdBuffer(numCommandbuffers);
	VkCommandBufferAllocateInfo nfo = {};
	nfo.commandBufferCount = numCommandbuffers;
	nfo.commandPool = getVkHandle();
	nfo.level = static_cast<VkCommandBufferLevel>(CommandBufferLevel::e_PRIMARY);
	nfo.sType = static_cast<VkStructureType>(StructureType::e_COMMAND_BUFFER_ALLOCATE_INFO);
	vkThrowIfFailed(getDevice()->getVkBindings().vkAllocateCommandBuffers(getDevice()->getVkHandle(), &nfo, cmdBuffer.get()), "CommandBuffer Allocation Failed.");
	CommandPool commandPool = shared_from_this();
	for (uint32_t i = 0; i < numCommandbuffers; ++i) { outCmdBuffers[i] = CommandBuffer_::constructShared(_device, commandPool, cmdBuffer[i]); }
}

SecondaryCommandBuffer CommandPool_::allocateSecondaryCommandBuffer()
{
	VkCommandBuffer cmdBuffer;

	VkCommandBufferAllocateInfo nfo = {};
	nfo.commandBufferCount = 1;
	nfo.commandPool = getVkHandle();
	nfo.level = static_cast<VkCommandBufferLevel>(CommandBufferLevel::e_SECONDARY);
	nfo.sType = static_cast<VkStructureType>(StructureType::e_COMMAND_BUFFER_ALLOCATE_INFO);
	vkThrowIfFailed(getDevice()->getVkBindings().vkAllocateCommandBuffers(getDevice()->getVkHandle(), &nfo, &cmdBuffer), "CommandBuffer Allocation Failed.");
	CommandPool commandPool = shared_from_this();
	return SecondaryCommandBuffer_::constructShared(_device, commandPool, cmdBuffer);
}

void CommandPool_::allocateSecondaryCommandBuffers(uint32_t numCommandbuffers, SecondaryCommandBuffer* outCmdBuffers)
{
	ArrayOrVector<VkCommandBuffer, 4> cmdBuffer(numCommandbuffers);
	VkCommandBufferAllocateInfo nfo = {};
	nfo.commandBufferCount = numCommandbuffers;
	nfo.commandPool = getVkHandle();
	nfo.level = static_cast<VkCommandBufferLevel>(CommandBufferLevel::e_SECONDARY);
	nfo.sType = static_cast<VkStructureType>(StructureType::e_COMMAND_BUFFER_ALLOCATE_INFO);
	vkThrowIfFailed(getDevice()->getVkBindings().vkAllocateCommandBuffers(getDevice()->getVkHandle(), &nfo, cmdBuffer.get()), "CommandBuffer Allocation Failure.");
	CommandPool commandPool = shared_from_this();
	for (uint32_t i = 0; i < numCommandbuffers; ++i) { outCmdBuffers[i] = SecondaryCommandBuffer_::constructShared(_device, commandPool, cmdBuffer[i]); }
}

void CommandPool_::reset(CommandPoolResetFlags flags)
{
	vkThrowIfFailed(
		getDevice()->getVkBindings().vkResetCommandPool(getDevice()->getVkHandle(), getVkHandle(), static_cast<VkCommandPoolResetFlags>(flags)), "CommandBuffer Reset Failure.");
}
} // namespace impl
} // namespace pvrvk
