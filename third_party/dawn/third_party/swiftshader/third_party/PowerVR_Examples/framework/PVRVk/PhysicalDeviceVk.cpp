/*!
\brief Function definitions for the Physical Device class
\file PVRVk/PhysicalDeviceVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#include "PVRVk/PhysicalDeviceVk.h"
#include "PVRVk/SurfaceVk.h"
#include "PVRVk/InstanceVk.h"
#include "PVRVk/DeviceVk.h"
#include "PVRVk/DisplayVk.h"
#include "PVRVk/DisplayModeVk.h"
namespace pvrvk {
namespace impl {
bool PhysicalDevice_::getSurfaceSupport(uint32_t queueFamilyIndex, Surface surface) const
{
	VkBool32 supportsWsi = false;
	getInstance()->getVkBindings().vkGetPhysicalDeviceSurfaceSupportKHR(getVkHandle(), queueFamilyIndex, surface->getVkHandle(), &supportsWsi);
	return supportsWsi != 0;
}

const std::vector<Display>& PhysicalDevice_::getDisplays() const { return _displays; }

Display& PhysicalDevice_::getDisplay(uint32_t id) { return _displays[id]; }

const Display& PhysicalDevice_::getDisplay(uint32_t id) const { return _displays[id]; }

FormatProperties PhysicalDevice_::getFormatProperties(Format format)
{
	if (format == Format::e_UNDEFINED) { return FormatProperties(); }

	FormatProperties formatProperties;

	Instance instance = getInstance();
	// use VK_KHR_get_physical_device_properties2 if the extension is supported
	if (instance->getEnabledExtensionTable().khrGetPhysicalDeviceProperties2Enabled)
	{
		VkFormatProperties2KHR formatProperties2KHR = {};
		formatProperties2KHR.sType = static_cast<VkStructureType>(StructureType::e_FORMAT_PROPERTIES_2_KHR);

		instance->getVkBindings().vkGetPhysicalDeviceFormatProperties2KHR(getVkHandle(), static_cast<VkFormat>(format), &formatProperties2KHR);
		memcpy(&formatProperties, &formatProperties2KHR.formatProperties, sizeof(formatProperties2KHR.formatProperties));
	}
	else
	{
		instance->getVkBindings().vkGetPhysicalDeviceFormatProperties(_vkHandle, static_cast<VkFormat>(format), &formatProperties.get());
	}

	return formatProperties;
}

SurfaceCapabilitiesKHR PhysicalDevice_::getSurfaceCapabilities(const Surface& surface) const
{
	Instance instance = getInstance();
	SurfaceCapabilitiesKHR surfaceCapabilities;
	if (instance->getVkBindings().vkGetPhysicalDeviceSurfaceCapabilitiesKHR == nullptr)
	{ throw ErrorValidationFailedEXT("GetPhysicalDeviceSurfaceCapabilitiesKHR does not exist. Cannot get surface capabilities."); }
	instance->getVkBindings().vkGetPhysicalDeviceSurfaceCapabilitiesKHR(getVkHandle(), surface->getVkHandle(), &surfaceCapabilities.get());
	return surfaceCapabilities;
}

std::vector<SurfaceFormatKHR> PhysicalDevice_::getSurfaceFormats(const Surface& surface) const
{
	Instance instance = getInstance();
	uint32_t formatCount;
	// Retrieve the number of VkFormats/VkColorSpaceKHR pairs supported by the physical device surface.
	instance->getVkBindings().vkGetPhysicalDeviceSurfaceFormatsKHR(getVkHandle(), surface->getVkHandle(), &formatCount, NULL);
	// Retrieve the list of formatCount VkFormats/VkColorSpaceKHR pairs supported by the physical device surface.
	std::vector<SurfaceFormatKHR> surfaceFormats(formatCount);
	instance->getVkBindings().vkGetPhysicalDeviceSurfaceFormatsKHR(getVkHandle(), surface->getVkHandle(), &formatCount, (VkSurfaceFormatKHR*)surfaceFormats.data());
	return surfaceFormats;
}

std::vector<PresentModeKHR> PhysicalDevice_::getSurfacePresentModes(const Surface& surface) const
{
	Instance instance = getInstance();
	uint32_t numPresentModes;
	instance->getVkBindings().vkGetPhysicalDeviceSurfacePresentModesKHR(getVkHandle(), surface->getVkHandle(), &numPresentModes, NULL);
	std::vector<PresentModeKHR> presentationModes(numPresentModes);
	instance->getVkBindings().vkGetPhysicalDeviceSurfacePresentModesKHR(getVkHandle(), surface->getVkHandle(), &numPresentModes, (VkPresentModeKHR*)presentationModes.data());
	return presentationModes;
}

std::vector<ExtensionProperties>& PhysicalDevice_::getDeviceExtensionsProperties()
{
	if (_deviceExtensions.size()) { return _deviceExtensions; }
	Instance instance = getInstance();
	uint32_t numItems = 0;
	instance->getVkBindings().vkEnumerateDeviceExtensionProperties(getVkHandle(), nullptr, &numItems, nullptr);

	_deviceExtensions.resize(numItems);
	instance->getVkBindings().vkEnumerateDeviceExtensionProperties(getVkHandle(), nullptr, &numItems, (VkExtensionProperties*)_deviceExtensions.data());
	return _deviceExtensions;
}

DisplayMode PhysicalDevice_::createDisplayMode(pvrvk::Display& display, const pvrvk::DisplayModeCreateInfo& displayModeCreateInfo)
{
	PhysicalDevice physicalDevice = shared_from_this();
	return DisplayMode_::constructShared(physicalDevice, display, displayModeCreateInfo);
}

Display PhysicalDevice_::getDisplayPlaneProperties(uint32_t displayPlaneIndex, uint32_t& currentStackIndex)
{
	updateDisplayPlaneProperties();
	currentStackIndex = _displayPlaneProperties[displayPlaneIndex].getCurrentStackIndex();
	VkDisplayKHR displayVk = _displayPlaneProperties[displayPlaneIndex].getCurrentDisplay();

	for (uint32_t i = 0; i < getNumDisplays(); ++i)
	{
		if (_displays[i]->getVkHandle() == displayVk) { return _displays[i]; }
	}
	return Display();
}

DisplayPlaneCapabilitiesKHR PhysicalDevice_::getDisplayPlaneCapabilities(DisplayMode& mode, uint32_t planeIndex)
{
	DisplayPlaneCapabilitiesKHR capabilities;
	getInstance()->getVkBindings().vkGetDisplayPlaneCapabilitiesKHR(getVkHandle(), mode->getVkHandle(), planeIndex, &capabilities.get());
	return capabilities;
}

std::vector<Display> PhysicalDevice_::getDisplayPlaneSupportedDisplays(uint32_t planeIndex)
{
	std::vector<VkDisplayKHR> supportedDisplaysVk;
	Instance instance = getInstance();
	uint32_t numSupportedDisplays = 0;
	instance->getVkBindings().vkGetDisplayPlaneSupportedDisplaysKHR(getVkHandle(), planeIndex, &numSupportedDisplays, nullptr);
	supportedDisplaysVk.resize(numSupportedDisplays);

	instance->getVkBindings().vkGetDisplayPlaneSupportedDisplaysKHR(getVkHandle(), planeIndex, &numSupportedDisplays, supportedDisplaysVk.data());

	std::vector<Display> supportedDisplays;
	for (uint32_t i = 0; i < numSupportedDisplays; ++i)
	{
		for (uint32_t j = 0; j < getNumDisplays(); ++j)
		{
			if (_displays[j]->getVkHandle() == supportedDisplaysVk[i]) { supportedDisplays.emplace_back(_displays[j]); }
		}
	}
	return supportedDisplays;
}

void PhysicalDevice_::updateDisplayPlaneProperties()
{
	Instance instance = getInstance();
	uint32_t numProperties = 0;
	instance->getVkBindings().vkGetPhysicalDeviceDisplayPlanePropertiesKHR(getVkHandle(), &numProperties, NULL);

	_displayPlaneProperties.clear();
	_displayPlaneProperties.resize(numProperties);
	instance->getVkBindings().vkGetPhysicalDeviceDisplayPlanePropertiesKHR(getVkHandle(), &numProperties, (VkDisplayPlanePropertiesKHR*)_displayPlaneProperties.data());
}

const ImageFormatProperties PhysicalDevice_::getImageFormatProperties(Format format, ImageType imageType, ImageTiling tiling, ImageUsageFlags usage, ImageCreateFlags flags)
{
	ImageFormatProperties imageProperties = {};

	VkResult result = VK_SUCCESS;
	Instance instance = getInstance();

	// use VK_KHR_get_physical_device_properties2 if the extension is supported
	if (instance->getEnabledExtensionTable().khrGetPhysicalDeviceProperties2Enabled)
	{
		VkPhysicalDeviceImageFormatInfo2KHR imageFormatInfo = {};
		imageFormatInfo.sType = static_cast<VkStructureType>(StructureType::e_PHYSICAL_DEVICE_IMAGE_FORMAT_INFO_2_KHR);
		imageFormatInfo.flags = static_cast<VkImageCreateFlags>(flags);
		imageFormatInfo.format = static_cast<VkFormat>(format);
		imageFormatInfo.tiling = static_cast<VkImageTiling>(tiling);
		imageFormatInfo.type = static_cast<VkImageType>(imageType);
		imageFormatInfo.usage = static_cast<VkImageUsageFlags>(usage);

		VkImageFormatProperties2KHR imageFormatProperties = {};
		imageFormatProperties.sType = static_cast<VkStructureType>(StructureType::e_IMAGE_FORMAT_PROPERTIES_2_KHR);

		result = instance->getVkBindings().vkGetPhysicalDeviceImageFormatProperties2KHR(getVkHandle(), &imageFormatInfo, &imageFormatProperties);
		memcpy(&imageProperties, &imageFormatProperties.imageFormatProperties, sizeof(imageFormatProperties.imageFormatProperties));
	}
	else
	{
		result = instance->getVkBindings().vkGetPhysicalDeviceImageFormatProperties(getVkHandle(), static_cast<VkFormat>(format), static_cast<VkImageType>(imageType),
			static_cast<VkImageTiling>(tiling), static_cast<VkImageUsageFlags>(usage), static_cast<VkImageCreateFlags>(flags), &imageProperties.get());
	}

	if (result != VK_SUCCESS) { throw ErrorValidationFailedEXT("The combination of parameters used is not supported by the implementation for use in vkCreateImage"); }

	return imageProperties;
}

const std::vector<SparseImageFormatProperties> PhysicalDevice_::getSparseImageFormatProperties(
	Format format, ImageType imageType, SampleCountFlags sampleCount, ImageUsageFlags usage, ImageTiling tiling)
{
	std::vector<SparseImageFormatProperties> sparseImageFormatProperties = {};
	uint32_t propertiesCount;
	Instance instance = getInstance();

	// use VK_KHR_get_physical_device_properties2 if the extension is supported
	if (instance->getEnabledExtensionTable().khrGetPhysicalDeviceProperties2Enabled)
	{
		VkPhysicalDeviceSparseImageFormatInfo2KHR sparseImageFormatInfo = {};
		sparseImageFormatInfo.sType = static_cast<VkStructureType>(StructureType::e_PHYSICAL_DEVICE_SPARSE_IMAGE_FORMAT_INFO_2_KHR);
		sparseImageFormatInfo.format = static_cast<VkFormat>(format);
		sparseImageFormatInfo.type = static_cast<VkImageType>(imageType);
		sparseImageFormatInfo.samples = static_cast<VkSampleCountFlagBits>(sampleCount);
		sparseImageFormatInfo.usage = static_cast<VkImageUsageFlags>(usage);
		sparseImageFormatInfo.tiling = static_cast<VkImageTiling>(tiling);

		instance->getVkBindings().vkGetPhysicalDeviceSparseImageFormatProperties2KHR(getVkHandle(), &sparseImageFormatInfo, &propertiesCount, nullptr);
		sparseImageFormatProperties.resize(propertiesCount);

		std::vector<VkSparseImageFormatProperties2KHR> properties(propertiesCount);
		for (uint32_t i = 0; i < propertiesCount; i++) { properties[i].sType = static_cast<VkStructureType>(StructureType::e_SPARSE_IMAGE_FORMAT_PROPERTIES_2_KHR); }

		instance->getVkBindings().vkGetPhysicalDeviceSparseImageFormatProperties2KHR(getVkHandle(), &sparseImageFormatInfo, &propertiesCount, properties.data());
		for (uint32_t i = 0; i < propertiesCount; i++) { sparseImageFormatProperties[i] = properties[i].properties; }
	}
	else
	{
		// determine the properties count
		instance->getVkBindings().vkGetPhysicalDeviceSparseImageFormatProperties(getVkHandle(), static_cast<VkFormat>(format), static_cast<VkImageType>(imageType),
			static_cast<VkSampleCountFlagBits>(sampleCount), static_cast<VkImageUsageFlags>(usage), static_cast<VkImageTiling>(tiling), &propertiesCount, nullptr);
		sparseImageFormatProperties.resize(propertiesCount);

		// retrieve the sparse image format properties
		instance->getVkBindings().vkGetPhysicalDeviceSparseImageFormatProperties(getVkHandle(), static_cast<VkFormat>(format), static_cast<VkImageType>(imageType),
			static_cast<VkSampleCountFlagBits>(sampleCount), static_cast<VkImageUsageFlags>(usage), static_cast<VkImageTiling>(tiling), &propertiesCount,
			(VkSparseImageFormatProperties*)sparseImageFormatProperties.data());
	}

	return sparseImageFormatProperties;
}

//!\cond NO_DOXYGEN
PhysicalDevice_::PhysicalDevice_(make_shared_enabler, Instance& instance, const VkPhysicalDevice& vkPhysicalDevice) : PVRVkInstanceObjectBase(instance, vkPhysicalDevice)
{
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;

	// use VK_KHR_get_physical_device_properties2 if the extension is supported
	if (instance->getEnabledExtensionTable().khrGetPhysicalDeviceProperties2Enabled)
	{
		VkPhysicalDeviceMemoryProperties2KHR memoryProperties = {};
		memoryProperties.sType = static_cast<VkStructureType>(StructureType::e_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2_KHR);

		instance->getVkBindings().vkGetPhysicalDeviceMemoryProperties2KHR(getVkHandle(), &memoryProperties);
		memcpy(&physicalDeviceMemoryProperties, &memoryProperties.memoryProperties, sizeof(memoryProperties.memoryProperties));
	}
	else
	{
		instance->getVkBindings().vkGetPhysicalDeviceMemoryProperties(getVkHandle(), &physicalDeviceMemoryProperties);
	}

	memcpy(&_deviceMemoryProperties, &physicalDeviceMemoryProperties, sizeof(physicalDeviceMemoryProperties));

	uint32_t numQueueFamilies = 0;

	// use VK_KHR_get_physical_device_properties2 if the extension is supported
	if (instance->getEnabledExtensionTable().khrGetPhysicalDeviceProperties2Enabled)
	{
		instance->getVkBindings().vkGetPhysicalDeviceQueueFamilyProperties2KHR(getVkHandle(), &numQueueFamilies, nullptr);
		_queueFamilyPropeties.resize(numQueueFamilies);

		std::vector<VkQueueFamilyProperties2KHR> queueFamilyProperties(numQueueFamilies);
		for (uint32_t i = 0; i < numQueueFamilies; i++) { queueFamilyProperties[i].sType = static_cast<VkStructureType>(StructureType::e_QUEUE_FAMILY_PROPERTIES_2_KHR); }

		instance->getVkBindings().vkGetPhysicalDeviceQueueFamilyProperties2KHR(getVkHandle(), &numQueueFamilies, queueFamilyProperties.data());

		for (uint32_t i = 0; i < numQueueFamilies; ++i) { _queueFamilyPropeties[i] = queueFamilyProperties[i].queueFamilyProperties; }
	}
	else
	{
		instance->getVkBindings().vkGetPhysicalDeviceQueueFamilyProperties(getVkHandle(), &numQueueFamilies, nullptr);
		_queueFamilyPropeties.resize(numQueueFamilies);
		instance->getVkBindings().vkGetPhysicalDeviceQueueFamilyProperties(getVkHandle(), &numQueueFamilies, (VkQueueFamilyProperties*)_queueFamilyPropeties.data());
	}

	// use VK_KHR_get_physical_device_properties2 if the extension is supported
	if (instance->getEnabledExtensionTable().khrGetPhysicalDeviceProperties2Enabled)
	{
		{
			VkPhysicalDeviceFeatures2KHR deviceFeatures = {};
			deviceFeatures.sType = static_cast<VkStructureType>(StructureType::e_PHYSICAL_DEVICE_FEATURES_2_KHR);

			instance->getVkBindings().vkGetPhysicalDeviceFeatures2KHR(getVkHandle(), &deviceFeatures);
			memcpy(&_deviceFeatures, &deviceFeatures.features, sizeof(deviceFeatures.features));
		}

		{
			VkPhysicalDeviceProperties2KHR deviceProperties = {};
			deviceProperties.sType = static_cast<VkStructureType>(StructureType::e_PHYSICAL_DEVICE_PROPERTIES_2_KHR);

			instance->getVkBindings().vkGetPhysicalDeviceProperties2KHR(getVkHandle(), &deviceProperties);
			memcpy(&_deviceProperties, &deviceProperties.properties, sizeof(deviceProperties.properties));
		}
	}
	else
	{
		instance->getVkBindings().vkGetPhysicalDeviceFeatures(getVkHandle(), (VkPhysicalDeviceFeatures*)&_deviceFeatures);
		instance->getVkBindings().vkGetPhysicalDeviceProperties(getVkHandle(), (VkPhysicalDeviceProperties*)&_deviceProperties);
	}
}
//!\endcond

void PhysicalDevice_::retrieveDisplays()
{
	Instance instance = getInstance();
	if (instance->getEnabledExtensionTable().khrDisplayEnabled)
	{
		{
			uint32_t numProperties = 0;
			instance->getVkBindings().vkGetPhysicalDeviceDisplayPropertiesKHR(getVkHandle(), &numProperties, NULL);

			pvrvk::ArrayOrVector<VkDisplayPropertiesKHR, 4> _displayProperties(numProperties);
			instance->getVkBindings().vkGetPhysicalDeviceDisplayPropertiesKHR(getVkHandle(), &numProperties, _displayProperties.get());

			PhysicalDevice physicalDevice = shared_from_this();
			for (uint32_t i = 0; i < numProperties; ++i)
			{
				DisplayPropertiesKHR displayProperties = _displayProperties[i];
				_displays.emplace_back(Display_::constructShared(physicalDevice, displayProperties));
			}
		}

		{
			updateDisplayPlaneProperties();
		}
	}
}

uint32_t PhysicalDevice_::getMemoryTypeIndex(uint32_t allowedMemoryTypeBits, pvrvk::MemoryPropertyFlags requiredMemoryProperties, pvrvk::MemoryPropertyFlags& usedMemoryProperties) const
{
	const uint32_t memoryCount = _deviceMemoryProperties.getMemoryTypeCount();
	for (uint32_t memoryIndex = 0; memoryIndex < memoryCount; ++memoryIndex)
	{
		const uint32_t memoryTypeBits = (uint32_t)(1 << memoryIndex);
		const bool isRequiredMemoryType = static_cast<uint32_t>(allowedMemoryTypeBits & memoryTypeBits) != 0;

		usedMemoryProperties = _deviceMemoryProperties.getMemoryTypes()[memoryIndex].getPropertyFlags();
		const bool hasRequiredProperties = static_cast<uint32_t>(usedMemoryProperties & requiredMemoryProperties) == requiredMemoryProperties;

		if (isRequiredMemoryType && hasRequiredProperties) { return static_cast<uint32_t>(memoryIndex); }
	}

	// failed to find memory type
	return static_cast<uint32_t>(-1);
}

Device PhysicalDevice_::createDevice(const DeviceCreateInfo& deviceCreateInfo)
{
	PhysicalDevice physicalDevice = shared_from_this();
	return Device_::constructShared(physicalDevice, deviceCreateInfo);
}
} // namespace impl
} // namespace pvrvk
