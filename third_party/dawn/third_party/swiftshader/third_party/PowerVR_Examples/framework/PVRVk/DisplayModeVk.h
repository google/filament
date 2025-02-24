/*!
\brief The DisplayMode class
\file PVRVk/DisplayModeVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/ForwardDecObjectsVk.h"
#include "PVRVk/PhysicalDeviceVk.h"
#include "PVRVk/PVRVkObjectBaseVk.h"
#include "PVRVk/DebugUtilsVk.h"

namespace pvrvk {
/// <summary>Display Mode creation descriptor.</summary>
struct DisplayModeCreateInfo
{
public:
	/// <summary>Constructor (zero initialization)</summary>
	DisplayModeCreateInfo() : _flags(DisplayModeCreateFlagsKHR::e_NONE), _parameters(DisplayModeParametersKHR()) {}

	/// <summary>Constructor</summary>
	/// <param name="parameters">Display mode parameters used to initialise the display mode creation structure</param>
	/// <param name="flags">A set of pvrvk::DisplayModeCreateFlagsKHR defining how to create a given pvrvk::DisplayMode</param>
	DisplayModeCreateInfo(DisplayModeParametersKHR parameters, DisplayModeCreateFlagsKHR flags = DisplayModeCreateFlagsKHR::e_NONE) : _flags(flags), _parameters(parameters) {}

	/// <summary>Getter for the display mode creation flags</summary>
	/// <returns>A DisplayModeCreateFlagsKHR structure</returns>
	DisplayModeCreateFlagsKHR getFlags() const { return _flags; }

	/// <summary>Setter for the display mode creation flags</summary>
	/// <param name="flags">A set of DisplayModeCreateFlagsKHR</param>
	/// <returns>This object</returns>
	DisplayModeCreateInfo& setFlags(DisplayModeCreateFlagsKHR flags)
	{
		this->_flags = flags;
		return *this;
	}

	/// <summary>Getter for the display mode parameters</summary>
	/// <returns>A DisplayModeParametersKHR structure</returns>
	DisplayModeParametersKHR getParameters() const { return _parameters; }

	/// <summary>Setter for the display mode parameters</summary>
	/// <param name="parameters">A set of DisplayModeParametersKHR</param>
	/// <returns>This object</returns>
	DisplayModeCreateInfo& setParameters(DisplayModeParametersKHR parameters)
	{
		this->_parameters = parameters;
		return *this;
	}

private:
	/// <summary>The set of DisplayModeCreateFlagsKHR used when creating the display mode</summary>
	DisplayModeCreateFlagsKHR _flags;
	/// <summary>The set of DisplayModeParametersKHR used when creating the display mode</summary>
	DisplayModeParametersKHR _parameters;
};

namespace impl {
/// <summary>Each display has one or more supported modes associated with it by default. These are called the display modes.</summary>
class DisplayMode_ : public PVRVkPhysicalDeviceObjectBase<VkDisplayModeKHR, ObjectType::e_DISPLAY_MODE_KHR>
{
private:
	friend class PhysicalDevice_;
	friend class Display_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class DisplayMode_;
	};

	static DisplayMode constructShared(const PhysicalDeviceWeakPtr& physicalDevice, const DisplayModePropertiesKHR& displayModeProperties)
	{
		return std::make_shared<DisplayMode_>(make_shared_enabler{}, physicalDevice, displayModeProperties);
	}

	static DisplayMode constructShared(const PhysicalDeviceWeakPtr& physicalDevice, pvrvk::Display& display, const pvrvk::DisplayModeCreateInfo& displayModeCreateInfo)
	{
		return std::make_shared<DisplayMode_>(make_shared_enabler{}, physicalDevice, display, displayModeCreateInfo);
	}

	DisplayModeParametersKHR _parameters;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(DisplayMode_)
	DisplayMode_(make_shared_enabler, const PhysicalDeviceWeakPtr& physicalDevice, const DisplayModePropertiesKHR& displayModeProperties);
	DisplayMode_(make_shared_enabler, const PhysicalDeviceWeakPtr& physicalDevice, pvrvk::Display& display, const pvrvk::DisplayModeCreateInfo& displayModeCreateInfo);
	//!\endcond

	/// <summary>Returns the display mode parameters</summary>
	/// <returns>A DisplayModeParametersKHR structure specifying the display mode parameters for the display mode</returns>
	DisplayModeParametersKHR getParameters() const { return _parameters; }
};
} // namespace impl
} // namespace pvrvk
