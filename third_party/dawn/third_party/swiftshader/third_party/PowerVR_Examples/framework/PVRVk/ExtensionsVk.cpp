/*!
\brief Function definitions for Extensions functionality
\file PVRVk/ExtensionsVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

//!\cond NO_DOXYGEN
#include "PVRVk/ExtensionsVk.h"
#include "PVRVk/InstanceVk.h"

namespace pvrvk {
namespace Extensions {

VulkanExtensionList filterExtensions(const std::vector<ExtensionProperties>& extensionProperties, const VulkanExtensionList& extensionsToEnable)
{
	VulkanExtensionList outExtensions;
	for (uint32_t i = 0; i < extensionsToEnable.getNumExtensions(); ++i)
	{
		const VulkanExtension& currentRequestedExtension = extensionsToEnable.getExtension(i);
		bool foundExtension = false;
		for (uint32_t j = 0; j < extensionProperties.size(); ++j)
		{
			// Determine whether the extension names match
			if (!strcmp(currentRequestedExtension.getName().c_str(), extensionProperties[j].getExtensionName()))
			{
				// If a spec version has been provided then ensure the spec versions match
				if (currentRequestedExtension.getSpecVersion() != -1)
				{
					if (extensionProperties[j].getSpecVersion() == currentRequestedExtension.getSpecVersion())
					{
						outExtensions.addExtension(currentRequestedExtension);
						foundExtension = true;
						break;
					}
				}
				// If a spec version of -1 has been provided then accept the highest supported version of the extension
				else
				{
					// If the extension is already present then determine whether the new element has a higher spec version
					if (foundExtension)
					{
						// Get the last added extension
						const VulkanExtension& lastAddedExtension = outExtensions.getExtension(outExtensions.getNumExtensions() - 1);
						if (extensionProperties[j].getSpecVersion() > lastAddedExtension.getSpecVersion())
						{
							outExtensions.removeExtension(lastAddedExtension);
							outExtensions.addExtension(VulkanExtension(extensionProperties[j].getExtensionName(), extensionProperties[j].getSpecVersion()));
						}
					}
					else
					{
						outExtensions.addExtension(VulkanExtension(extensionProperties[j].getExtensionName(), extensionProperties[j].getSpecVersion()));
					}

					foundExtension = true;
				}
			}
		}
	}
	return outExtensions;
}

void enumerateInstanceExtensions(std::vector<ExtensionProperties>& outExtensions) { enumerateInstanceExtensions(outExtensions, ""); }

void enumerateInstanceExtensions(std::vector<ExtensionProperties>& outExtensions, const std::string& layerName)
{
	uint32_t numItems = 0;

	const char* pLayerName = nullptr;
	if (layerName.length() > 0) { pLayerName = layerName.c_str(); }

	pvrvk::impl::vkThrowIfFailed(
		pvrvk::getVkBindings().vkEnumerateInstanceExtensionProperties(pLayerName, &numItems, nullptr), "ExtensionsVk::Failed to enumerate instance extension properties");
	outExtensions.resize(numItems);
	pvrvk::impl::vkThrowIfFailed(pvrvk::getVkBindings().vkEnumerateInstanceExtensionProperties(pLayerName, &numItems, (VkExtensionProperties*)outExtensions.data()),
		"ExtensionsVk::Failed to enumerate instance extension properties");
}

bool isInstanceExtensionSupported(const std::string& extension)
{
	std::vector<ExtensionProperties> extensions;
	enumerateInstanceExtensions(extensions);
	for (uint32_t i = 0; i < extensions.size(); ++i)
	{
		if (!strcmp(extensions[i].getExtensionName(), extension.c_str())) { return true; }
	}
	return false;
}
} // namespace Extensions
} // namespace pvrvk
//!\endcond
