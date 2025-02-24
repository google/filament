/*!
\brief PVRVk Semaphore class.
\file PVRVk/SemaphoreVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
namespace impl {
/// <summary>Vulkan implementation of the Semaphore class.
/// Use to "serialize" access between CommandBuffer submissions and /Queues</summary>
class Semaphore_ : public PVRVkDeviceObjectBase<VkSemaphore, ObjectType::e_SEMAPHORE>, public DeviceObjectDebugUtils<Semaphore_>
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class Semaphore_;
	};

	static Semaphore constructShared(const DeviceWeakPtr& device, const SemaphoreCreateInfo& createInfo)
	{
		return std::make_shared<Semaphore_>(make_shared_enabler{}, device, createInfo);
	}

	/// <summary>Creation information used when creating the Semaphore.</summary>
	SemaphoreCreateInfo _createInfo;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(Semaphore_)
	Semaphore_(make_shared_enabler, const DeviceWeakPtr& device, const SemaphoreCreateInfo& createInfo);

	~Semaphore_();
	//!\endcond

	/// <summary>Get the Semaphore creation flags</summary>
	/// <returns>The set of Semaphore creation flags</returns>
	inline SemaphoreCreateFlags getFlags() const { return _createInfo.getFlags(); }

	/// <summary>Get this Semaphore's create flags</summary>
	/// <returns>SemaphoreCreateInfo</returns>
	SemaphoreCreateInfo getCreateInfo() const { return _createInfo; }
};
} // namespace impl
} // namespace pvrvk
