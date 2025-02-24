/*!
\brief Functionality that helps management of Vulkan extensions, such as
enumerating, enabling/disabling
\file PVRVk/ExtensionsVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once

#include "PVRVk/TypesVk.h"
#include "PVRVk/pvrvk_vulkan_wrapper.h"

namespace pvrvk {
namespace Extensions {

/// <summary>Filter the extensions</summary>
/// <param name="extensionProperties">Extension properties</param>
/// <param name="extensionsToEnable">Extensions to enable</param>
/// <returns>VulkanExtensionList</returns>
extern VulkanExtensionList filterExtensions(const std::vector<pvrvk::ExtensionProperties>& extensionProperties, const VulkanExtensionList& extensionsToEnable);

/// <summary>Get list of all supported instance extension properties</summary>
/// <param name="outExtensions">Returned extensions</param>
void enumerateInstanceExtensions(std::vector<ExtensionProperties>& outExtensions);

/// <summary>Get list of all supported instance extension properties for a given layer</summary>
/// <param name="outExtensions">Returned extensions</param>
/// <param name="layerName">Layer from which to retrieve supported extensions</param>
void enumerateInstanceExtensions(std::vector<ExtensionProperties>& outExtensions, const std::string& layerName);

/// <summary>Query if an Instance Extension is supported</summary>
/// <param name="extension">The extension string</param>
/// <returns>True if the instance supports the extension, otherwise false</param>
bool isInstanceExtensionSupported(const std::string& extension);
} // namespace Extensions
} // namespace pvrvk
