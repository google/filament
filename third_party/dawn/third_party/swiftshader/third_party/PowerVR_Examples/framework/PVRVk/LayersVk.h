/*!
\brief Functionality for working with and managing the Vulkan Layers,
such as enumerating, enabling/disabling lists of them and similar functionality
\file PVRVk/LayersVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once

#include "PVRVk/TypesVk.h"
#include "PVRVk/pvrvk_vulkan_wrapper.h"

namespace pvrvk {
namespace Layers {
/// <summary>Filter layers</summary>
/// <param name="layerProperties"> Supportted layers</param>
/// <param name="layersToEnable">layers to filter</param>
/// <returns>Filtered layers</returns>
VulkanLayerList filterLayers(const std::vector<pvrvk::LayerProperties>& layerProperties, const VulkanLayerList& layersToEnable);

/// <summary>Enumerate instance layers</summary>
/// <param name="outLayers">Out layers</param>
void enumerateInstanceLayers(std::vector<LayerProperties>& outLayers);

/// <summary>Query if an Instance Layer is supported</summary>
/// <param name="layer">The layer string</param>
/// <returns>True if the instance supports the layer, otherwise false</param>
bool isInstanceLayerSupported(const std::string& layer);
} // namespace Layers
} // namespace pvrvk
