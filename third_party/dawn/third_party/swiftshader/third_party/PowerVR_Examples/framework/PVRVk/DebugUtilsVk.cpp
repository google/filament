/*!
\brief Function implementations for the DebugUtilsImpl class
\file PVRVk/DebugUtilsVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN
#include "PVRVk/DebugUtilsVk.h"
#include "PVRVk/DeviceVk.h"

/// <summary>Main PowerVR Framework Namespace</summary>
namespace pvrvk {
/// <summary>Contains internal objects and wrapped versions of the PVRVk module</summary>
namespace impl {
void DeviceDebugUtilsImpl::setObjectName(const Device_& device, uint64_t vkHandle, ObjectType objectType, const std::string& objectName)
{
	assert(device.getVkHandle() != VK_NULL_HANDLE);
	assert(vkHandle != VK_NULL_HANDLE);

	_objectName = objectName;

	// if VK_EXT_debug_utils is supported then use otherwise fallback to VK_EXT_debug_marker
	if (device.getPhysicalDevice()->getInstance()->getEnabledExtensionTable().extDebugUtilsEnabled)
	{
		DebugUtilsObjectNameInfo objectNameInfo(objectType, vkHandle, objectName);
		VkDebugUtilsObjectNameInfoEXT vkObjectNameInfo = {};
		vkObjectNameInfo.sType = static_cast<VkStructureType>(StructureType::e_DEBUG_UTILS_OBJECT_NAME_INFO_EXT);
		// The VkObjectType type of the object to be named
		vkObjectNameInfo.objectType = static_cast<VkObjectType>(objectNameInfo.getObjectType());
		// The actual object handle of the object to name
		vkObjectNameInfo.objectHandle = objectNameInfo.getObjectHandle();
		// The name to use for the object
		vkObjectNameInfo.pObjectName = objectNameInfo.getObjectName().c_str();
		vkThrowIfFailed(device.getPhysicalDevice()->getInstance()->getVkBindings().vkSetDebugUtilsObjectNameEXT(device.getVkHandle(), &vkObjectNameInfo),
			"Failed to set ObjectName with vkSetDebugUtilsObjectNameEXT");
	}
	// if VK_EXT_debug_marker is supported then set the object name
	else if (device.getEnabledExtensionTable().extDebugMarkerEnabled)
	{
		VkDebugMarkerObjectNameInfoEXT objectNameInfo = {};
		objectNameInfo.sType = static_cast<VkStructureType>(StructureType::e_DEBUG_MARKER_OBJECT_NAME_INFO_EXT);
		// The VkDebugReportObjectTypeEXT of the object to be named
		objectNameInfo.objectType = static_cast<VkDebugReportObjectTypeEXT>(pvrvk::convertObjectTypeToDebugReportObjectType(objectType));
		// The actual object handle of the object to name
		objectNameInfo.object = vkHandle;
		// The name to use for the object
		objectNameInfo.pObjectName = _objectName.c_str();

		vkThrowIfFailed(device.getVkBindings().vkDebugMarkerSetObjectNameEXT(device.getVkHandle(), &objectNameInfo), "Failed to set ObjectName with vkDebugMarkerSetObjectNameEXT");
	}
}

void DeviceDebugUtilsImpl::setObjectTag(const Device_& device, uint64_t vkHandle, ObjectType objectType, uint64_t tagName, size_t tagSize, const void* tag)
{
	assert(device.getVkHandle() != VK_NULL_HANDLE);
	assert(vkHandle != VK_NULL_HANDLE);
	// if VK_EXT_debug_utils is supported then use otherwise fallback to VK_EXT_debug_marker
	if (device.getPhysicalDevice()->getInstance()->getEnabledExtensionTable().extDebugUtilsEnabled)
	{
		DebugUtilsObjectTagInfo objectTagInfo(objectType, vkHandle, tagName, tagSize, tag);
		VkDebugUtilsObjectTagInfoEXT vkObjectTagInfo = {};
		vkObjectTagInfo.sType = static_cast<VkStructureType>(StructureType::e_DEBUG_UTILS_OBJECT_TAG_INFO_EXT);
		// The VkObjectType type of the object to be tagged
		vkObjectTagInfo.objectType = static_cast<VkObjectType>(objectTagInfo.getObjectType());
		// The actual object handle of the object to tag
		vkObjectTagInfo.objectHandle = objectTagInfo.getObjectHandle();
		// The tag name to use for the object
		vkObjectTagInfo.tagName = objectTagInfo.getTagName();
		// The number of bytes of data to attach to the object
		vkObjectTagInfo.tagSize = objectTagInfo.getTagSize();
		// An array of tagSize bytes containing the data to be associated with the object
		vkObjectTagInfo.pTag = objectTagInfo.getTag();
		vkThrowIfFailed(device.getPhysicalDevice()->getInstance()->getVkBindings().vkSetDebugUtilsObjectTagEXT(device.getVkHandle(), &vkObjectTagInfo),
			"Failed to set ObjectTag with vkSetDebugUtilsObjectTagEXT");
	}
	// if VK_EXT_debug_marker is supported then set the object tag
	else if (device.getEnabledExtensionTable().extDebugMarkerEnabled)
	{
		VkDebugMarkerObjectTagInfoEXT objectTagInfo = {};
		objectTagInfo.sType = static_cast<VkStructureType>(StructureType::e_DEBUG_MARKER_OBJECT_TAG_INFO_EXT);
		// The VK_DEBUG_REPORT_OBJECT_TYPE type of the object to be tagged
		objectTagInfo.objectType = static_cast<VkDebugReportObjectTypeEXT>(pvrvk::convertObjectTypeToDebugReportObjectType(objectType));
		// The actual object handle of the object to tag
		objectTagInfo.object = vkHandle;
		// The tag name to use for the object
		objectTagInfo.tagName = tagName;
		// The number of bytes of data to attach to the object
		objectTagInfo.tagSize = tagSize;
		// An array of tagSize bytes containing the data to be associated with the object
		objectTagInfo.pTag = tag;
		vkThrowIfFailed(device.getVkBindings().vkDebugMarkerSetObjectTagEXT(device.getVkHandle(), &objectTagInfo), "Failed to set ObjectTag with vkDebugMarkerSetObjectTagEXT");
	}
}
} // namespace impl
} // namespace pvrvk
//!\endcond
