/*!
\brief A wrapper providing support for object annotation i.e. naming and tagging via the use of VK_EXT_debug_marker or VK_EXT_debug_utils. PVRVk supports the
 extensions "VK_EXT_debug_maker" and "VK_EXT_debug_utils" by inheriting from DeviceObjectDebugUtils.
\file PVRVk/DebugUtilsVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/ForwardDecObjectsVk.h"

/// <summary>Main PowerVR Framework Namespace</summary>
namespace pvrvk {

/// <summary>DebugMarkerMarkerInfo used to define a debug marker.</summary>
struct DebugMarkerMarkerInfo
{
public:
	/// <summary>Constructor</summary>
	DebugMarkerMarkerInfo() : _markerName("")
	{
		_color[0] = { 0.0f };
		_color[1] = { 0.0f };
		_color[2] = { 0.0f };
		_color[3] = { 0.0f };
	}

	/// <summary>Constructor</summary>
	/// <param name="markerName">The marker name to use</param>
	/// <param name="colorR">The red color component</param>
	/// <param name="colorG">The green color component</param>
	/// <param name="colorB">The blue color component</param>
	/// <param name="colorA">The alpha color component</param>
	DebugMarkerMarkerInfo(const std::string& markerName, float colorR = 183.0f / 255.0f, float colorG = 26.0f / 255.0f, float colorB = 139.0f / 255.0f, float colorA = 1.0f)
		: _markerName(markerName)
	{
		_color[0] = colorR;
		_color[1] = colorG;
		_color[2] = colorB;
		_color[3] = colorA;
	}

	/// <summary>Get the marker</summary>
	/// <returns>The marker name</returns>
	const std::string& getMarkerName() const { return _markerName; }

	/// <summary>Set the label</summary>
	/// <param name="markerName">A new marker name</param>
	void setMarkerName(const std::string& markerName) { this->_markerName = markerName; }

	/// <summary>Get red floating point component</summary>
	/// <returns>Red component</returns>
	float getR() const { return _color[0]; }

	/// <summary>Set red floating point component</summary>
	/// <param name="r">Red component</param>
	void setR(const float r) { this->_color[0] = r; }

	/// <summary>Get green floating point component</summary>
	/// <returns>Green component</returns>
	float getG() const { return _color[1]; }

	/// <summary>Set green floating point component</summary>
	/// <param name="g">green component</param>
	void setG(const float g) { this->_color[1] = g; }

	/// <summary>Get blue floating point component</summary>
	/// <returns>Blue component</returns>
	float getB() const { return _color[2]; }

	/// <summary>Set blue floating point component</summary>
	/// <param name="b">Blue component</param>
	void setB(const float b) { this->_color[2] = b; }

	/// <summary>Get alpha floating point component</summary>
	/// <returns>Alpha component</returns>
	float getA() const { return _color[3]; }

	/// <summary>Set alpha floating point component</summary>
	/// <param name="a">Alpha component</param>
	void setA(const float a) { this->_color[3] = a; }

private:
	std::string _markerName;
	float _color[4];
};

/// <summary>Contains internal objects and wrapped versions of the PVRVk module</summary>
namespace impl {
/// <summary>Implementation for the Debug Marker wrapper for PVRVk objects. Handles the actual naming and tagging calls for the debug extensions
/// "VK_EXT_debug_maker" or "VK_EXT_debug_utils" depending on thier support.</summary>
class DeviceDebugUtilsImpl
{
public:
	template<typename>
	friend class DeviceObjectDebugUtils;

	/// <summary>Destructor.</summary>
	~DeviceDebugUtilsImpl() {}

	/// <summary>Makes use of the extension VK_EXT_debug_marker or VK_EXT_debug_utils to provide a name for a specified object.</summary>
	/// <param name="device">The device from which the object being named was created</param>
	/// <param name="vkHandle">The Vulkan object handle of the object being named</param>
	/// <param name="objectType">The object type of the object being named</param>
	/// <param name="objectName">The name to use for the object</param>
	void setObjectName(const Device_& device, uint64_t vkHandle, ObjectType objectType, const std::string& objectName);

	/// <summary>Makes use of the extension VK_EXT_debug_marker or VK_EXT_debug_utils to provide a tag for a specified object.</summary>
	/// <param name="device">The device from which the object getting tagged was created</param>
	/// <param name="vkHandle">The Vulkan object handle of the object getting a tag was created.</param>
	/// <param name="objectType">The object type of the object being named</param>
	/// <param name="tagName">A numerical identifier of the tag for the object</param>
	/// <param name="tagSize">The number of bytes of data to attach to the object.</param>
	/// <param name="tag">An array of tagSize bytes containing the data to be associated with the object.</param>
	void setObjectTag(const Device_& device, uint64_t vkHandle, ObjectType objectType, uint64_t tagName, size_t tagSize, const void* tag);

	/// <summary>Resets the name of a specified object using the extension VK_EXT_debug_marker or VK_EXT_debug_utils.</summary>
	/// <param name="device">The device from which the object having its name reset was created</param>
	/// <param name="vkHandle">The Vulkan object handle of the object having its name reset.</param>
	/// <param name="objectType">The object type of the object being named</param>
	void resetObjectName(const Device_& device, uint64_t vkHandle, ObjectType objectType) { setObjectName(device, vkHandle, objectType, ""); }

	/// <summary>Returns whether the specified object has already been provided with a name.</summary>
	/// <returns>True if the object has a name, otherwise false.</returns>
	bool hasName() const { return _objectName.length() > 0; }

	/// <summary>Returns the specified object's name.</summary>
	/// <returns>The object name</returns>
	const std::string& getName() const { return _objectName; }

private:
	/// <summary>Default Constructor for a DebugUtilsImpl.</summary>
	DeviceDebugUtilsImpl() : _objectName("") {}

	std::string _objectName;
};

/// <summary>A Debug Marker wrapper for PVRVk Device allocated objects. Handles naming and tagging calls for the extension "VK_EXT_debug_maker".</summary>
template<class PVRVkDeviceObject>
class DeviceObjectDebugUtils
{
public:
	/// <summary>Constructor for a DeviceObjectDebugUtils.</summary>
	explicit DeviceObjectDebugUtils() : _debugUtils() {}

	/// <summary>Makes use of the extension VK_EXT_debug_marker or VK_EXT_debug_utils to provide a name for a specified object.</summary>
	/// <param name="objectName">The name to use for the object</param>
	void setObjectName(const std::string& objectName)
	{
		_debugUtils.setObjectName(*static_cast<PVRVkDeviceObject&>(*this).getDevice().get(),
			*static_cast<const uint64_t*>(static_cast<const void*>(&static_cast<PVRVkDeviceObject&>(*this).getVkHandle())), static_cast<PVRVkDeviceObject&>(*this).getObjectType(),
			objectName);
	}

	/// <summary>Gets the Debug Marker name.</summary>
	/// <returns>The object name</returns>
	const std::string& getObjectName() const { return _debugUtils.getName(); }

	/// <summary>Resets the name of a specified object using the extension VK_EXT_debug_marker or VK_EXT_debug_utils.</summary>
	void resetObjectName()
	{
		_debugUtils.setObjectName(*static_cast<PVRVkDeviceObject&>(*this).getDevice().get(),
			*static_cast<const uint64_t*>(static_cast<const void*>(&static_cast<PVRVkDeviceObject&>(*this).getVkHandle())), static_cast<PVRVkDeviceObject&>(*this).getObjectType(), "");
	}

	/// <summary>Makes use of the extension VK_EXT_debug_marker or VK_EXT_debug_utils to provide a tag for a specified object.</summary>
	/// <param name="tagName">A numerical identifier of the tag for the object</param>
	/// <param name="tagSize">The number of bytes of data to attach to the object.</param>
	/// <param name="tag">An array of tagSize bytes containing the data to be associated with the object.</param>
	void setObjectTag(uint64_t tagName, size_t tagSize, const void* tag)
	{
		_debugUtils.setObjectTag(*static_cast<PVRVkDeviceObject&>(*this).getDevice().get(),
			*static_cast<const uint64_t*>(static_cast<const void*>(&static_cast<PVRVkDeviceObject&>(*this).getVkHandle())), static_cast<PVRVkDeviceObject&>(*this).getObjectType(),
			tagName, tagSize, tag);
	}

private:
	DeviceDebugUtilsImpl _debugUtils;
};
} // namespace impl
} // namespace pvrvk
