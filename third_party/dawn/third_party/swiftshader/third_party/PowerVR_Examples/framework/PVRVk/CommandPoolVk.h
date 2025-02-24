/*!
\brief The PVRVk CommandPool, a pool that can Allocate and Free Command Buffers
\file PVRVk/CommandPoolVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
/// <summary>Command pool creation descriptor.</summary>
struct CommandPoolCreateInfo
{
public:
	/// <summary>Constructor</summary>
	/// <param name="queueFamilyIndex">Designates a queue family, all command buffers allocated from this command pool must be submitted to queues from the same queue
	/// family</param> <param name="flags">Flags to use for creating the command pool</param>
	explicit CommandPoolCreateInfo(uint32_t queueFamilyIndex, CommandPoolCreateFlags flags = CommandPoolCreateFlags::e_NONE) : _flags(flags), _queueFamilyIndex(queueFamilyIndex) {}

	/// <summary>Get the command pool creation flags</summary>
	/// <returns>The set of command pool creation flags</returns>
	inline CommandPoolCreateFlags getFlags() const { return _flags; }
	/// <summary>Set the command pool creation flags</summary>
	/// <param name="flags">The command pool creation flags</param>
	inline void setFlags(CommandPoolCreateFlags flags) { this->_flags = flags; }
	/// <summary>Get Queue family index</summary>
	/// <returns>The queue family index to which all command buffers allocated from this pool can be submitted to</returns>
	inline uint32_t getQueueFamilyIndex() const { return _queueFamilyIndex; }
	/// <summary>Set the Queue family index</summary>
	/// <param name="queueFamilyIndex">The queue family index to which all command buffers allocated from this pool can be submitted to</param>
	inline void setQueueFamilyIndex(uint32_t queueFamilyIndex) { this->_queueFamilyIndex = queueFamilyIndex; }

private:
	/// <summary>Flags to use for creating the command pool</summary>
	CommandPoolCreateFlags _flags;
	/// <summary>Designates a queue family, all command buffers allocated from this command pool must be submitted to queues from the same queue family</summary>
	uint32_t _queueFamilyIndex;
};

namespace impl {
/// <summary>Vulkan implementation of the Command Pool class.
/// Destroying the commandpool will also destroys the commandbuffers allocated from this pool</summary>
class CommandPool_ : public PVRVkDeviceObjectBase<VkCommandPool, ObjectType::e_COMMAND_POOL>, public DeviceObjectDebugUtils<CommandPool_>, public std::enable_shared_from_this<CommandPool_>
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class CommandPool_;
	};

	static CommandPool constructShared(const DeviceWeakPtr& device, const CommandPoolCreateInfo& createInfo)
	{
		return std::make_shared<CommandPool_>(make_shared_enabler{}, device, createInfo);
	}

	/// <summary>Creation information used when creating the command pool.</summary>
	CommandPoolCreateInfo _createInfo;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(CommandPool_)
	CommandPool_(make_shared_enabler, const DeviceWeakPtr& device, const CommandPoolCreateInfo& createInfo);

	~CommandPool_()
	{
		if (getVkHandle() != VK_NULL_HANDLE)
		{
			if (!_device.expired())
			{
				getDevice()->getVkBindings().vkDestroyCommandPool(getDevice()->getVkHandle(), getVkHandle(), nullptr);
				_vkHandle = VK_NULL_HANDLE;
			}
			else
			{
				reportDestroyedAfterDevice();
			}
		}
	}
	//!\endcond

	/// <summary>Allocate a primary commandBuffer</summary>
	/// <returns>Return a valid commandbuffer if allocation is successful, otherwise a null CommandBuffer</returns>
	CommandBuffer allocateCommandBuffer();

	/// <summary>Allocate primary commandbuffers</summary>
	/// <param name="numCommandbuffers">Number of commandbuffers to allocate</param>
	/// <param name="outCmdBuffers">Allocated commandbuffers</param>
	void allocateCommandBuffers(uint32_t numCommandbuffers, CommandBuffer* outCmdBuffers);

	/// <summary>Allocate a secondary commandBuffer</summary>
	/// <returns>Return a valid commandbuffer if allocation success, otherwise a null CommandBuffer</returns>
	SecondaryCommandBuffer allocateSecondaryCommandBuffer();

	/// <summary>Allocate secondary commandbuffers</summary>
	/// <param name="numCommandbuffers">Number of commmandbuffers to allocate</param>
	/// <param name="outCmdBuffers">allocated commandbuffers</param>
	void allocateSecondaryCommandBuffers(uint32_t numCommandbuffers, SecondaryCommandBuffer* outCmdBuffers);

	/// <summary>Get the command pool creation flags</summary>
	/// <returns>The set of command pool creation flags</returns>
	inline CommandPoolCreateFlags getFlags() const { return _createInfo.getFlags(); }
	/// <summary>Get the queue family index</summary>
	/// <returns>The queue family index, all command buffers allocated from this command pool must be submitted to queues from the same queue family</returns>
	inline uint32_t getQueueFamilyIndex() const { return _createInfo.getQueueFamilyIndex(); }

	/// <summary>Resets the command pool and also optionally recycles all of the resoources of all of the command buffers allocated from the command pool.</summary>
	/// <param name="flags">VkCommandPoolResetFlags controls the reset operation</param>
	/// <returns>Return true if success</returns>
	void reset(pvrvk::CommandPoolResetFlags flags);
};
} // namespace impl
} // namespace pvrvk
