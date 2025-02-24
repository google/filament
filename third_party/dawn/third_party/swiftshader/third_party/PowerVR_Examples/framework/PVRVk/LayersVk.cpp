/*!
\brief Function definitions for LayersVk header file
\file PVRVk/LayersVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/
//!\cond NO_DOXYGEN

#include "PVRVk/LayersVk.h"
#include "PVRVk/InstanceVk.h"

#include <sstream>

namespace pvrvk {
namespace Layers {
VulkanLayerList filterLayers(const std::vector<LayerProperties>& layerProperties, const VulkanLayerList& layersToEnable)
{
	VulkanLayerList outLayers;
	for (uint32_t i = 0; i < layersToEnable.getNumLayers(); ++i)
	{
		const VulkanLayer& currentRequestedLayer = layersToEnable.getLayer(i);
		bool foundLayer = false;
		for (uint32_t j = 0; j < layerProperties.size(); ++j)
		{
			// Determine whether the layers names match
			if (!strcmp(currentRequestedLayer.getName().c_str(), layerProperties[j].getLayerName()))
			{
				// If a spec version has been provided then ensure the spec versions match
				if (currentRequestedLayer.getSpecVersion() != -1)
				{
					if (layerProperties[j].getSpecVersion() == currentRequestedLayer.getSpecVersion())
					{
						outLayers.addLayer(VulkanLayer(layerProperties[j].getLayerName(), layerProperties[j].getSpecVersion(), layerProperties[j].getImplementationVersion(),
							layerProperties[j].getDescription()));
						foundLayer = true;
						break;
					}
				}
				// If a spec version of -1 has been provided then accept the highest supported version of the layer
				else
				{
					// If the extension is already present then determine whether the new element has a higher spec version
					if (foundLayer)
					{
						// Get the last added layer
						const VulkanLayer& lastAddedLayer = outLayers.getLayer(outLayers.getNumLayers() - 1);
						if (layerProperties[j].getSpecVersion() > lastAddedLayer.getSpecVersion())
						{
							outLayers.removeLayer(lastAddedLayer);
							outLayers.addLayer(VulkanLayer(layerProperties[j].getLayerName(), layerProperties[j].getSpecVersion(), layerProperties[j].getImplementationVersion(),
								layerProperties[j].getDescription()));
						}
					}
					else
					{
						outLayers.addLayer(VulkanLayer(layerProperties[j].getLayerName(), layerProperties[j].getSpecVersion(), layerProperties[j].getImplementationVersion(),
							layerProperties[j].getDescription()));
					}

					foundLayer = true;
				}
			}
		}
	}
	return outLayers;
}

void enumerateInstanceLayers(std::vector<LayerProperties>& outLayers)
{
	uint32_t numItems = 0;
	pvrvk::impl::vkThrowIfFailed(pvrvk::getVkBindings().vkEnumerateInstanceLayerProperties(&numItems, nullptr), "LayersVk::Failed to enumerate instance layer properties");
	outLayers.resize(numItems);
	pvrvk::impl::vkThrowIfFailed(
		pvrvk::getVkBindings().vkEnumerateInstanceLayerProperties(&numItems, (VkLayerProperties*)outLayers.data()), "LayersVk::Failed to enumerate instance layer properties");
}

bool isInstanceLayerSupported(const std::string& layer)
{
	std::vector<LayerProperties> layers;
	enumerateInstanceLayers(layers);
	for (uint32_t i = 0; i < layers.size(); ++i)
	{
		if (!strcmp(layers[i].getLayerName(), layer.c_str())) { return true; }
	}
	return false;
}
} // namespace Layers
} // namespace pvrvk
//!\endcond
