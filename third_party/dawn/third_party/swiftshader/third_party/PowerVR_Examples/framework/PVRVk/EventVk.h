/*!
\brief PVRVk Event class.
\file PVRVk/EventVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
#pragma once
#include "PVRVk/DeviceVk.h"

namespace pvrvk {
namespace impl {
/// <summary>Vulkan implementation of the Event class.
/// Event can be used by the host to do fine-grained synchronization of commands, and it
/// can be signalled either from the host (calling set()) or the device (submitting a setEvent() command).</summary>
class Event_ : public PVRVkDeviceObjectBase<VkEvent, ObjectType::e_EVENT>, public DeviceObjectDebugUtils<Event_>
{
private:
	friend class Device_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class Event_;
	};

	static Event constructShared(const DeviceWeakPtr& device, const EventCreateInfo& createInfo) { return std::make_shared<Event_>(make_shared_enabler{}, device, createInfo); }

	/// <summary>Creation information used when creating the event.</summary>
	EventCreateInfo _createInfo;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(Event_)
	Event_(make_shared_enabler, const DeviceWeakPtr& device, const EventCreateInfo& createInfo);
	~Event_();
	//!\endcond
	/// <summary>Set this event</summary>
	void set();

	/// <summary>Reset this event</summary>
	void reset();

	/// <summary>Return true if this event is set</summary>
	/// <returns>Return true if this event is set</returns>
	bool isSet();

	/// <summary>Get the event creation flags</summary>
	/// <returns>The set of event creation flags</returns>
	inline EventCreateFlags getFlags() const { return _createInfo.getFlags(); }

	/// <summary>Get this event's create flags</summary>
	/// <returns>EventCreateInfo</returns>
	EventCreateInfo getCreateInfo() const { return _createInfo; }
};
} // namespace impl
} // namespace pvrvk
