/*!
\brief The Display class
\file PVRVk/DisplayVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/ForwardDecObjectsVk.h"
#include "PVRVk/PhysicalDeviceVk.h"
#include "PVRVk/PVRVkObjectBaseVk.h"
#include "PVRVk/DebugUtilsVk.h"
#include "PVRVk/DisplayModeVk.h"

namespace pvrvk {
namespace impl {
/// <summary>A display device can in some environments be used directly for Vulkan rendering without using intermediate windowing systems.</summary>
class Display_ : public PVRVkPhysicalDeviceObjectBase<VkDisplayKHR, ObjectType::e_DISPLAY_KHR>
{
private:
	friend class PhysicalDevice_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class Display_;
	};

	static Display constructShared(const PhysicalDeviceWeakPtr& physicalDevice, const DisplayPropertiesKHR& displayProperties)
	{
		return std::make_shared<Display_>(make_shared_enabler{}, physicalDevice, displayProperties);
	}

	std::vector<DisplayMode> _displayModes;
	DisplayPropertiesKHR _properties;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(Display_)
	Display_(make_shared_enabler, const PhysicalDeviceWeakPtr& physicalDevice, const DisplayPropertiesKHR& displayProperties);

	~Display_() { _displayModes.clear(); }
	//!\endcond

	/// <summary>Get the number of supported display modes</summary>
	/// <returns>Returns the number of supported display modes</returns>
	size_t getNumDisplayModes() const { return _displayModes.size(); }

	/// <summary>Get the supported display mode at displayModeIndex</summary>
	/// <param name="displayModeIndex">The index of the display mode to retrieve</param>
	/// <returns>Returns one of supported display modes</returns>
	DisplayMode& getDisplayMode(uint32_t displayModeIndex) { return _displayModes[displayModeIndex]; }

	/// <summary>Get the supported display mode at displayModeIndex (const)</summary>
	/// <param name="displayModeIndex">The index of the display mode to retrieve</param>
	/// <returns>Returns one of supported display modes (const)</returns>
	const DisplayMode& getDisplayMode(uint32_t displayModeIndex) const { return _displayModes[displayModeIndex]; }

	/// <summary>Gets the name of the display</summary>
	/// <returns>The name of the display</returns>
	inline const char* getDisplayName() const { return _properties.getDisplayName(); }

	/// <summary>Gets the physical dimensions of the display</summary>
	/// <returns>The physical dimensions of the display</returns>
	inline const Extent2D& getPhysicalDimensions() const { return _properties.getPhysicalDimensions(); }

	/// <summary>Gets the physical resolutions of the display</summary>
	/// <returns>The physical resolutions of the display</returns>
	inline const Extent2D& getPhysicalResolution() const { return _properties.getPhysicalResolution(); }

	/// <summary>Gets the set of supported surface transform flags for the display</summary>
	/// <returns>A SurfaceTransformFlagsKHR structure specifying the supported surface transform flags.</returns>
	inline SurfaceTransformFlagsKHR getSupportedTransforms() const { return _properties.getSupportedTransforms(); }

	/// <summary>Indicates whether the planes on this display can have their z order changed.</summary>
	/// <returns>If True then the application can re-arrange the planes on this display in any order relative to each other.</returns>
	inline bool getPlaneReorderPossible() const { return _properties.getPlaneReorderPossible() != 0; }

	/// <summary>Indicates whether the display supports self-refresh/internal buffering</summary>
	/// <returns>True if the application can submit persistent present operations on swapchains created against this display</returns>
	inline bool getPersistentContent() const { return _properties.getPersistentContent() != 0; }
};
} // namespace impl
} // namespace pvrvk
