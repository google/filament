/*!
\brief PVRVk Fence class.
\file PVRVk/FenceVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
namespace impl {
/// <summary>Vulkan implementation of the Fence class
/// Fence can be used by the host to determine completion of execution of subimmisions to queues. The host
/// can be polled for the fence signal
/// .</summary>
class Fence_ : public PVRVkDeviceObjectBase<VkFence, ObjectType::e_FENCE>, public DeviceObjectDebugUtils<Fence_>
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class Fence_;
	};

	static Fence constructShared(const DeviceWeakPtr& device, const FenceCreateInfo& createInfo) { return std::make_shared<Fence_>(make_shared_enabler{}, device, createInfo); }

	/// <summary>Creation information used when creating the fence.</summary>
	FenceCreateInfo _createInfo;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(Fence_)
	Fence_(make_shared_enabler, const DeviceWeakPtr& device, const FenceCreateInfo& createInfo);
	~Fence_();
	//!\endcond

	/// <summary>Host to wait for this fence to be signaled</summary>
	/// <param name="timeoutNanos">Time out period in nanoseconds</param>
	/// <returns>True if successfully waited, false if timed out</returns>
	bool wait(uint64_t timeoutNanos = static_cast<uint64_t>(-1));

	/// <summary>Return true if this fence is signaled</summary>
	/// <returns>True if the fence is signalled, otherwise false</returns>
	bool isSignalled();

	/// <summary>Reset this fence</summary>
	void reset();

	/// <summary>Get the fence creation flags</summary>
	/// <returns>The set of fence creation flags</returns>
	inline FenceCreateFlags getFlags() const { return _createInfo.getFlags(); }

	/// <summary>Get this fence's create flags</summary>
	/// <returns>FenceCreateInfo</returns>
	FenceCreateInfo getCreateInfo() const { return _createInfo; }
};
} // namespace impl
} // namespace pvrvk
