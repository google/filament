/*!
\brief This demo provides an introduction to the PowerVR Shell library.
* This demo makes use of the PVRShell library to handles all of the OS specific initialisation code. It shows how making use of PVRShell a whole host of boilerplate
* code can be removed from an application meaning the developer can focus on what's important - the application code.
\file VulkanIntroducingPVRShell.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

// Platform and window system specific definitions to use for conditionally building the example.The VK_USE_PLATFORM_* defines are used by the Vulkan headers for
// enabling / disabling platform specific functionality.This example will primarily be using these defines for platform specific surface creation and making available
// the relevant functions required for this.
// The VK_USE_PLATFORM_* functions are set in CMakeLists.txt

// Include files
// defines a set of Vulkan function pointer tables (Vulkan, instance and device) and includes vulkan_IMG.h
#include "vk_bindings.h"
// defines a set of helper functions used for populating the Vulkan function pointer tables
#include "vk_bindings_helper.h"
// enables the use of the PVRCore module which provides a collection of supporting code for the PowerVR Framework
#include "PVRCore/PVRCore.h"
// enables the use of the PVRShell module which provides an abstract mechanism for the native platform primarily used for handling window creation and input handling.
#include "PVRShell/PVRShell.h"

// conditionally include dlfcn.h when the X11 XCB window system is to be used by the application. dlfcn.h is required for dynamically opening the xcb library using dlopen and
// closing it using dlclose.
#ifdef VK_USE_PLATFORM_XCB_KHR
#include <dlfcn.h>
#endif

/// <summary>Map a VkDebugReportFlagsEXT variable to a corresponding log severity.</summary>
/// <param name="flags">A set of VkDebugReportFlagsEXT specifying the type of event which triggered the callback.</param>
/// <returns>A LogLevel corresponding to the VkDebugReportFlagsEXT.</returns>
LogLevel mapValidationTypeToLogType(VkDebugReportFlagsEXT flags)
{
	// Simply map the VkDebugReportFlagsEXT to a particular LogLevel.
	if ((flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) != 0) { return LogLevel::Information; }
	if ((flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) != 0) { return LogLevel::Warning; }
	if ((flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) != 0) { return LogLevel::Performance; }
	if ((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0) { return LogLevel::Error; }
	if ((flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) != 0) { return LogLevel::Debug; }

	return LogLevel::Information;
}

/// <summary>Map a VkDebugReportObjectTypeEXT object type to a string representation.</summary>
/// <param name="objectType">A VkDebugReportObjectTypeEXT specifying the object where the issue was detected.</param>
/// <returns>A stringified version of the VkDebugReportObjectTypeEXT.</returns>
std::string mapDebugReportObjectTypeToString(VkDebugReportObjectTypeEXT objectType)
{
	// Simply maps the object type to a string matching the object type.
	switch (objectType)
	{
	case VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT: return "INSTANCE_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT: return "PHYSICAL_DEVICE_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT: return "DEVICE_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT: return "QUEUE_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT: return "SEMAPHORE_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT: return "COMMAND_BUFFER_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT: return "FENCE_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT: return "DEVICE_MEMORY_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT: return "BUFFER_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT: return "IMAGE_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT: return "EVENT_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT: return "QUERY_POOL_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT: return "BUFFER_VIEW_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT: return "IMAGE_VIEW_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT: return "SHADER_MODULE_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT: return "PIPELINE_CACHE_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT: return "PIPELINE_LAYOUT_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT: return "RENDER_PASS_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT: return "PIPELINE_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT: return "DESCRIPTOR_SET_LAYOUT_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT: return "SAMPLER_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT: return "DESCRIPTOR_POOL_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT: return "DESCRIPTOR_SET_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT: return "FRAMEBUFFER_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT: return "COMMAND_POOL_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT: return "SURFACE_KHR_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT: return "SWAPCHAIN_KHR_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT: return "DEBUG_REPORT_CALLBACK_EXT_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT: return "DISPLAY_KHR_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT: return "DISPLAY_MODE_KHR_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_OBJECT_TABLE_NVX_EXT: return "OBJECT_TABLE_NVX_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_INDIRECT_COMMANDS_LAYOUT_NVX_EXT: return "INDIRECT_COMMANDS_LAYOUT_NVX_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_KHR_EXT: return "DESCRIPTOR_UPDATE_TEMPLATE_KHR_EXT"; break;
	case VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT:
	default: return "UNKNOWN_EXT"; break;
	}
}

/// <summary>The application defined callback function used in combination with the extension VK_EXT_debug_report. The following defines the custom application defined
/// function provided as the pfnCallback member of the VkDebugReportCallbackCreateInfoEXT structure passed to vkCreateDebugReportCallbackEXT. The custom function
/// defines a way for the layers and the implementation to call back to the application for events of interest to the application.</summary>
/// <param name="flags">The set of VkDebugReportFlagBitsEXT that triggered the callback.</param>
/// <param name="objectType">The type of the object being used or created at the time the callback was triggered.</param>
/// <param name="object">The object handle where the issue was detected.</param>
/// <param name="location">A component defined value indicating the location of the trigger. This value may be optional.</param>
/// <param name="messageCode">A layer defined value indicating what test triggered the callback.</param>
/// <param name="pLayerPrefix">An abbreviation of the component making the callback.</param>
/// <param name="pMessage">The message detailing the trigger conditions.</param>
/// <param name="pUserData">The user data given when the DebugReportCallback was created.</param>
/// <returns>Returns True if the application should indicate to the calling layer that the Vulkan call should be aborted. Applications should generally
/// return False so the same behaviour is observed with and without the layers.</returns>
VKAPI_ATTR VkBool32 VKAPI_CALL throwOnErrorDebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
	int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	// The following callback values are ignored in this simple implementation of a VK_EXT_debug_report debug callback.
	(void)object;
	(void)location;
	(void)messageCode;
	(void)pLayerPrefix;
	(void)pUserData;

	// Throw an exception if the type of VkDebugReportFlagsEXT contains the ERROR_BIT.
	if ((flags & (VK_DEBUG_REPORT_ERROR_BIT_EXT)) != VkDebugReportFlagsEXT(0))
	{ throw pvr::PvrError(std::string(mapDebugReportObjectTypeToString(objectType) + std::string(". VULKAN_LAYER_VALIDATION: ") + pMessage)); }
	// Return false so that there is no differences in the behaviour observed with or without validation layers enabled.
	return VK_FALSE;
}

/// <summary>The application defined callback function used in combination with the extension VK_EXT_debug_report. The following defines the custom application defined
/// function provided as the pfnCallback member of the VkDebugReportCallbackCreateInfoEXT structure passed to vkCreateDebugReportCallbackEXT.
/// The custom function defines a way for the layers and the implementation to call back to the application for events of interest to the application.</summary>
/// <param name="flags">The set of VkDebugReportFlagBitsEXT that triggered the callback.</param>
/// <param name="objectType">The type of the object being used or created at the time the callback was triggered.</param>
/// <param name="object">The object handle where the issue was detected.</param>
/// <param name="location">A component defined value indicating the location of the trigger. This value may be optional.</param>
/// <param name="messageCode">A layer defined value indicating what test triggered the callback.</param>
/// <param name="pLayerPrefix">An abbreviation of the component making the callback.</param>
/// <param name="pMessage">The message detailing the trigger conditions.</param>
/// <param name="pUserData">The user data given when the DebugReportCallback was created.</param>
/// <returns>Returns True if the application should indicate to the calling layer that the Vulkan call should be aborted. Applications should generally
/// return False so the same behaviour is observed with and without the layers.</returns>
VKAPI_ATTR VkBool32 VKAPI_CALL logMessageDebugReportCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location,
	int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	// The following callback values are ignored in this simple implementation of a VK_EXT_debug_report debug callback.
	(void)object;
	(void)location;
	(void)messageCode;
	(void)pLayerPrefix;
	(void)pUserData;
	// Map the VkDebugReportFlagsEXT to a suitable log type
	// Map the VkDebugReportObjectTypeEXT to a stringified representation
	// Log the message generated by a lower layer
	Log(mapValidationTypeToLogType(flags), std::string(mapDebugReportObjectTypeToString(objectType) + std::string(". VULKAN_LAYER_VALIDATION: %s")).c_str(), pMessage);

	// Return false so that there is no differences in the behaviour observed with or without validation layers enabled.
	return VK_FALSE;
}

// In Vulkan, extensions may define additional Vulkan commands, structures and enumerations which are not included in or used by Core Vulkan.
// Functionality which isn't strictly necessary but which may provide additional or extended functionality may be defined via separate extensions.
// Here we define the set of instance and device extensions which may be used by various platforms and window systems supported by the demo.

// Helpfully extensions in Vulkan.h are protected via ifdef guards meaning we can conditionally compile our application to use the most appropriate set of extensions.
// Later we will filter out unsupported extensions and act accordingly based on those that are required and are supported by the chosen platform and window system combination.

// Of note is that the 'VK_EXT_XXXX_EXTENSION_NAME' syntax is provided by the vulkan headers as compile-time constants so that extension names can be used unambiguously avoiding
// typos in querying for them.
namespace Extensions {
// Defines the set of global Vulkan instance extensions which may be required depending on the combination of platform and window system in use
const std::string InstanceExtensions[] = {
	// The VK_KHR_surface extension declares the VkSurfaceKHR object and provides a function for destroying VkSurfaceKHR objects
	// note that the creation of VkSurfaceKHR objects is delegated to platform specific extensions but from the applications
	// point of view the handle is an opaque non-platform-specific type. Specifically for this demo VK_KHR_surface is required for creating VkSurfaceKHR objects which are further
	// used by the device extension VK_KHR_swapchain.
	VK_KHR_SURFACE_EXTENSION_NAME,
	// The VK_KHR_display extension provides the functionality for enumerating display devices and creating VkSurfaceKHR objects that directly
	// target displays. This extension is particularly important for applications which render directly to display devices without
	// an intermediate window system such as embedded applications or when running on embedded platforms
	VK_KHR_DISPLAY_EXTENSION_NAME,
#ifdef DEBUG
	// The VK_EXT_debug_report extension provides the functionality for defining a way in which layers and the implementation can
	// call back to the application for events of particular interest to the application. By enabling this extension the application
	// has the opportunity for receiving much more detailed feedback regarding the applications use of Vulkan
	VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
	// The VK_KHR_win32_surface extension provides the necessary mechanism for creating a VkSurfaceKHR object which refers to a Win32 HWND in addition
	// to functions for querying the support for rendering to the windows desktop
	VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
	// The VK_KHR_android_surface extension provides the necessary mechanism for creating a VkSurfaceKHR object which refers to an ANativeWindow, Android's native surface type
	VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	// The VK_KHR_xlib_surface extension provides the necessary mechanism for creating a VkSurfaceKHR object which refers to an X11 Window using Xlib in addition to functions
	// for querying the support for rendering via Xlib
	VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	// The VK_KHR_wayland_surface extension provides the necessary mechanism for creating a VkSurfaceKHR object which refers to a Wayland wl_surface in addition to functions
	// for querying the support for rendering to a Wayland compositor
	VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME,
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
	// The VK_MVK_macos_surface extension provides the necessary mechanism for creating a VkSurfaceKHR object which refers to a CAMetalLayer backed NSView
	VK_MVK_MACOS_SURFACE_EXTENSION_NAME,
#endif
};

// Defines a set of per device specific extensions to check for support
const std::string DeviceExtensions[] = {
	// The VK_KHR_swapchain extension is the device specific companion to VK_KHR_surface which introduces VkSwapchainKHR objects
	// enabling the ability to present render images to specified surfaces
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};
} // namespace Extensions

// Vulkan is a layered API with layers that may provide additional functionality over core Vulkan but do not add or modify existing Vulkan commands.
// In Vulkan the validation of correct API usage is left to validation layers so they are of particular importance.
// When a Vulkan layer is enabled it inserts itself into the call chain for Vulkan commands the specific layer is interested in
// the concept of using layers allows implementations to avoid performance penalties incurred for validating application behaviour and API usage
namespace Layers {
const std::string InstanceLayers[] = {
#ifdef DEBUG
	// Khronos Validation is a layer which encompasses all of the functionality that used to be contained in VK_LAYER_GOOGLE_threading,
	// VK_LAYER_LUNARG_parameter_validation, VK_LAYER_LUNARG_object_tracker, VK_LAYER_LUNARG_core_validation, and VK_LAYER_GOOGLE_unique_objects
	"VK_LAYER_KHRONOS_validation",
	// Standard Validation is a (now deprecated) meta-layer managed by the LunarG Loader.
	// Using Standard Validation will cause the loader to load a standard set of validation layers in an optimal order: VK_LAYER_GOOGLE_threading,
	// VK_LAYER_LUNARG_parameter_validation, VK_LAYER_LUNARG_object_tracker, VK_LAYER_LUNARG_core_validation, and VK_LAYER_GOOGLE_unique_objects.
	"VK_LAYER_LUNARG_standard_validation",
	// PerfDoc is a Vulkan layer which attempts to identify API usage is may be discouraged primarily by validating applications
	// against the rules set out in the Mali Application Developer Best Practices document
	"VK_LAYER_ARM_mali_perf_doc",
	"VK_LAYER_IMG_powervr_perf_doc", //

#else
	""
#endif
};
// note that device specific layers have now been deprecated and all layers are enabled during instance creation, with all enabled instance layers able
// to intercept all commands operating on that instance including any of its child objects i.e. the device or commands operating on a specific device.
} // namespace Layers

/// <summary>Determines whether the specified extension "extensionName" is supported and has been enabled. This is carried out by checking whether the specified extension exists in
/// the list of enabled extensions which have been specified as "enabledExtensionNames".</summary> <param name="enabledExtensionNames">A set of instance extension which have been
/// enabled via the ppEnabledExtensionNames member of the VkInstanceCreateInfo structure.</param> <param name="extensionName">The extension name to check for inclusion in the set
/// of enabled extensions.</param> <returns>Returns true if the extension "extensionName" can be found in the list of enabled extensions.</returns>
bool isExtensionEnabled(const std::vector<std::string>& enabledExtensionNames, const char* extensionName)
{
	for (uint32_t i = 0; i < enabledExtensionNames.size(); i++)
	{
		if (!strcmp(enabledExtensionNames[i].c_str(), extensionName)) { return true; }
	}

	return false;
}

/// <summary>Filters the set of supported extensions "extensionProperties" based on a set of extensions to enable "extensionsToEnable".</summary>
/// <param name="extensionProperties">A list of extensions supported retrieved via a previous call to vkEnumerateInstanceExtensionProperties.</param>
/// <param name="extensionsToEnable">A pointer to a list of "numExtensions" to check for support.</param>
/// <param name="numExtensions">The number of extensions in "extensionsToEnable" to check for support.</param>
/// <returns>The resulting set of extensions which exist in extensionProperties and extensionsToEnable.</returns>
std::vector<std::string> filterExtensions(const std::vector<VkExtensionProperties>& extensionProperties, const std::string* extensionsToEnable, uint32_t numExtensions)
{
	std::vector<std::string> outExtensions;
	for (uint32_t i = 0; i < extensionProperties.size(); ++i)
	{
		for (uint32_t j = 0; j < numExtensions; ++j)
		{
			if (!strcmp(extensionsToEnable[j].c_str(), extensionProperties[i].extensionName))
			{
				outExtensions.push_back(extensionsToEnable[j]);
				break;
			}
		}
	}
	return outExtensions;
}

/// <summary>Filters a list of VkLayerProperties supported by a particular device based on a set of application chosen layers to be used in this demo.</summary>
/// <param name="layerProperties">A list of VkLayerProperties supported by a particular device which will be used as the base for filtering.</param>
/// <param name="layersToEnable">A pointer to a list of application chosen layers to enable.</param>
/// <param name="layersCount">The number of layers in the array pointed to by layersToEnable.</param>
/// <returns>A set of device supported layers the application wishes to enable.</returns>
std::vector<std::string> filterLayers(const std::vector<VkLayerProperties>& layerProperties, const std::string* layersToEnable, uint32_t layersCount)
{
	// For each layer supported by a particular device check whether the application has chosen to enable it. If the chosen layer to enable exists in the list
	// of layers to enable then add the layer to a list of layers to return to the application.
	std::vector<std::string> outLayers;
	for (uint32_t i = 0; i < layerProperties.size(); ++i)
	{
		for (uint32_t j = 0; j < layersCount; ++j)
		{
			if (!strcmp(layersToEnable[j].c_str(), layerProperties[i].layerName)) { outLayers.push_back(layersToEnable[j]); }
		}
	}
	return outLayers;
}

/// <summary>Throw a runtime exception if the specified result is not VK_SUCCESS. Note that PVRShell is configured to catch all thrown exceptions and log appropriate error
/// messages for them. When a debugger is detected PVRShell will not catch exceptions and instead let the debugger handle them.</summary>
/// <param name="result">A Vulkan result code.</param>
/// <param name="msg">Print msg if the error code is not VK_SUCCESS and exit the application.</param>
inline void vulkanSuccessOrDie(VkResult result, const char* msg)
{
	if (result != VK_SUCCESS) { throw pvr::PvrError("Vulkan Raised an error: " + std::string(msg)); }
}

/// <summary>Attempts to find a suitable memory type for the specified set of allowed bits and memory property flags.</summary>
/// <param name="deviceMemProps">The physical device memory properties.</param>
/// <param name="allowedMemoryTypeBits">A set of allowed memory type bits for the required memory allocation. Retrieved from the memoryTypeBits member of the VkMemoryRequirements
/// retrieved using vkGetImageMemoryRequirements or vkGetBufferMemoryRequirements etc.</param>
/// <param name="requiredMemoryProperties">The memory property flags required for the memory allocation.</param>
/// <param name="usedMemoryProperties">The memory property flags actually used when allocating the memory.</param>
inline uint32_t getMemoryTypeIndexHelper(const VkPhysicalDeviceMemoryProperties& deviceMemProps, const uint32_t allowedMemoryTypeBits,
	const VkMemoryPropertyFlags requiredMemoryProperties, VkMemoryPropertyFlags& usedMemoryProperties)
{
	const uint32_t memoryCount = deviceMemProps.memoryTypeCount;
	for (uint32_t memoryIndex = 0; memoryIndex < memoryCount; ++memoryIndex)
	{
		const uint32_t memoryTypeBits = (1 << memoryIndex);
		const bool isRequiredMemoryType = static_cast<uint32_t>(allowedMemoryTypeBits & memoryTypeBits) != 0;

		usedMemoryProperties = deviceMemProps.memoryTypes[memoryIndex].propertyFlags;
		const bool hasRequiredProperties = static_cast<uint32_t>(usedMemoryProperties & requiredMemoryProperties) == requiredMemoryProperties;

		if (isRequiredMemoryType && hasRequiredProperties) { return static_cast<uint32_t>(memoryIndex); }
	}

	// Failed to find a suitable memory type index for the given arguments.
	return static_cast<uint32_t>(-1);
}

/// <summary>Attempts to find the index for a suitable memory type supporting the memory type bits required from the set of memory type bits supported.</summary>
/// <param name="deviceMemProps">The physical device memory properties.</param>
/// <param name="allowedMemoryTypeBits">A set of allowed memory type bits for the required memory allocation. Retrieved from the memoryTypeBits member of the VkMemoryRequirements
/// retrieved using vkGetImageMemoryRequirements or vkGetBufferMemoryRequirements etc.</param>
/// <param name="requiredMemoryProperties">The memory property flags required for the memory allocation.</param>
/// <param name="optimalMemoryProperties">An optimal set of memory property flags to use for the memory allocation.</param>
/// <param name="outMemoryTypeIndex">The memory type index used for allocating the memory.</param>
/// <param name="outMemoryPropertyFlags">The memory property flags actually used when allocating the memory.</param>
inline void getMemoryTypeIndex(const VkPhysicalDeviceMemoryProperties& deviceMemProps, const uint32_t allowedMemoryTypeBits, const VkMemoryPropertyFlags requiredMemoryProperties,
	const VkMemoryPropertyFlags optimalMemoryProperties, uint32_t& outMemoryTypeIndex, VkMemoryPropertyFlags& outMemoryPropertyFlags)
{
	// First attempt to find a memory type index which supports the optimal set of memory property flags
	VkMemoryPropertyFlags memoryPropertyFlags = optimalMemoryProperties;

	// We ensure that the optimal set of memory property flags is a superset of the required set of memory property flags.
	// This also handles cases where the optimal set of memory property flags hasn't been set but the required set has.
	memoryPropertyFlags |= requiredMemoryProperties;

	// Attempt to find a valid memory type index based on the optimal memory property flags.
	outMemoryTypeIndex = getMemoryTypeIndexHelper(deviceMemProps, allowedMemoryTypeBits, memoryPropertyFlags, outMemoryPropertyFlags);

	// if the optimal set cannot be found then fallback to the required set. The required set of memory property flags are expected to be supported and found. If not, an exception
	// will be thrown.
	if (outMemoryTypeIndex == static_cast<uint32_t>(-1))
	{
		memoryPropertyFlags = requiredMemoryProperties;
		outMemoryTypeIndex = getMemoryTypeIndexHelper(deviceMemProps, allowedMemoryTypeBits, memoryPropertyFlags, outMemoryPropertyFlags);
		if (outMemoryTypeIndex == static_cast<uint32_t>(-1)) { throw pvr::PvrError("Cannot find suitable memory type index for the set of VkMemoryPropertyFlags."); }
	}
}

/// <summary>Retrieves the VkImageAspectFlags based on the VkFormat. The VkImageAspectFlags specify the aspects of an image for purposes such as identifying a
/// sub-resource.</summary>
/// <param name="format">The VkFormat to retrieve VkImageAspectFlags for.</param>
/// <returns>The compatible VkImageAspectFlags based on the input VkFormat.</returns>
/// <details>This function simply infers the VkImageAspectFlags based on the position of the given VkFormat in the list of the VkFormat enum.</details>
inline VkImageAspectFlags formatToImageAspect(VkFormat format)
{
	// Attempt to find a set of VkImageAspectFlags compatible with the VkFormat specified as input

	// Undefined formats do not have compatible VkImageAspectFlags.
	if (format == VK_FORMAT_UNDEFINED) { throw pvr::PvrError("Cannot retrieve VkImageAspectFlags from an undefined VkFormat"); }

	// For VkFormats which correspond to anything other than the set of Depth/Stencil formats then the VkImageAspectFlags can be assumed to be VK_IMAGE_ASPECT_COLOR_BIT.
	if (format < VK_FORMAT_D16_UNORM || format > VK_FORMAT_D32_SFLOAT_S8_UINT) { return VK_IMAGE_ASPECT_COLOR_BIT; }

	// If the VkFormat is one of the Depth/Stencil formats then determine whether the compatible VkImageAspectFlags includes VK_IMAGE_ASPECT_DEPTH_BIT or
	// VK_IMAGE_ASPECT_STENCIL_BIT or both.
	const VkImageAspectFlags formats[] = {
		VK_IMAGE_ASPECT_DEPTH_BIT, // VK_FORMAT_D16_UNORM
		VK_IMAGE_ASPECT_DEPTH_BIT, // VK_FORMAT_X8_D24_UNORM_PACK32
		VK_IMAGE_ASPECT_DEPTH_BIT, // VK_FORMAT_D32_SFLOAT
		VK_IMAGE_ASPECT_STENCIL_BIT, // VK_FORMAT_S8_UINT
		VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, // VK_FORMAT_D16_UNORM_S8_UINT
		VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, // VK_FORMAT_D24_UNORM_S8_UINT
		VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, // VK_FORMAT_D32_SFLOAT_S8_UINT
	};
	return formats[static_cast<uint32_t>(format) - static_cast<uint32_t>(VK_FORMAT_D16_UNORM)];
}

// Filenames for the SPIR-V shader file binaries used in this demo
// Note that the binaries are pre-compiled using the "recompile script" included alongside the demo (recompile.sh/recompile.bat)
const char* VertShaderName = "VertShader.vsh.spv";
const char* FragShaderName = "FragShader.fsh.spv";

enum
{
	MAX_SWAPCHAIN_IMAGES = 4
};

/// <summary>VulkanIntroducingPVRShell is the main demo class implementing the pvr::Shell functionality required for rendering to the screen.
/// The PowerVR shell handles all OS specific initialisation code, and is extremely convenient for writing portable applications. It also has several built in
/// command line features, which allow you to specify attributes such as the method of vsync to use. The demo is constructed around a "PVRShell" superclass.
/// To make use of pvr::Shell you must define your app using a class which inherits from this, which should implement the following five methods,
/// which at execution time are essentially called in the order in which they are listed:
///		initApplication : This is called before any API initialisation has taken place, and can be used to set up any application data which does not require API calls,
///			for example object positions, or arrays containing vertex data, before they are uploaded.
///		initView : This is called after the API has initialized, and can be	used to do any remaining initialisation which requires API functionality.
///		renderFrame : This is called repeatedly to draw the geometry. Returning false from this function instructs the app to enter the quit sequence
///		releaseView : This function is called before the API is released, and is used to release any API resources.
///		quitApplication : This is called last of all, after the API has been released, and can be used to free any leftover user allocated memory.
/// The shell framework starts the application by calling a "pvr::newDemo" function, which must return an instance of the PVRShell class you defined. We will
/// now use the shell to create a "Hello triangle" app (VulkanIntroducingPVRShell), with the end result being similar to what was shown in VulkanHelloApi.</summary>
class VulkanIntroducingPVRShell : public pvr::Shell
{
	// Resources used throughout the demo.

	// The set of Vulkan commands used for initialising the Vulkan instance.
	// After being initialised the VkBindings struct will hold function pointers for the set of commands required for initialising the Vulkan instance.
	VkBindings _vkBindings;

	// Per application Vulkan instance used to initialize the Vulkan library.
	// The Vulkan instance forms the basis for all interactions between the application and the implementation.
	VkInstance _instance;
	// Function pointers for the various Vulkan functions requiring the Vulkan Instance handle.
	VkInstanceBindings _instanceVkFunctions;

	// Stores the set of enabled instance extension names
	std::vector<std::string> _enabledInstanceExtensionNames;
	// Stores the set of enabled layer names (Instance only)
	std::vector<std::string> _enabledLayerNames;

	// Stores the set of created Debug Report Callbacks which provide a mechanism for the Vulkan layers and the implementation to call back to the application.
	VkDebugReportCallbackEXT _debugReportCallbacks[MAX_SWAPCHAIN_IMAGES];

	// A physical device usually corresponding to a single device in the system
	VkPhysicalDevice _physicalDevice;

	// Stores the physical device memory properties specifying the memory heaps and corresponding memory types exposed by the platform
	VkPhysicalDeviceMemoryProperties _physicalDeviceMemoryProperties;

	// Stores the physical device properties
	VkPhysicalDeviceProperties _physicalDeviceProperties;

	// The Vulkan surface handle (VkSurfaceKHR) abstracting the native platform surface
	VkSurfaceKHR _surface;

	// The WSI Swapchain object
	VkSwapchainKHR _swapchain;

	// The length of the swapchain corresponding to the number of presentation images
	uint32_t _swapchainLength;

	// The format used for the set of presentation images used by the presentation engine
	VkFormat _swapchainColorFormat;

	// The set of images used for presenting images via the implementation's presentation engine
	VkImage _swapchainImages[MAX_SWAPCHAIN_IMAGES];

	// A set of VkImageViews used so the swapchain images can be used as framebuffer attachments
	VkImageView _swapchainImageViews[MAX_SWAPCHAIN_IMAGES];

	// The logical device representing a logical connection to an underlying physical device
	VkDevice _device;

	// Stores the set of enabled device extension names
	std::vector<std::string> _enabledDeviceExtensionNames;

	// Function pointers for the various Vulkan functions requiring the Vulkan Device handle.
	VkDeviceBindings _deviceVkFunctions;

	// The vertex buffer object used for rendering.
	VkBuffer _vbo;

	// The memory backing for the vertex buffer object.
	VkDeviceMemory _vboMemory;

	// The memory property flags used for allocating the memory for the vbo memory.
	VkMemoryPropertyFlags _vboMemoryFlags;

	// The model view projection buffer object used for rendering
	VkBuffer _modelViewProjectionBuffer;

	// The memory backing for the model view projection buffer object.
	VkDeviceMemory _modelViewProjectionMemory;

	// The memory property flags used for allocating the memory for the model view projection memory.
	VkMemoryPropertyFlags _modelViewProjectionMemoryFlags;

	// A pointer to the mapped memory for the model view projection buffer.
	void* _modelViewProjectionMappedMemory;

	// The size of the view projection buffer taking into account any alignment requirements.
	size_t _modelViewProjectionBufferSize;

	// The descriptor pool used for allocating descriptor sets.
	VkDescriptorPool _descriptorPool;

	// The descriptor set layouts for the static and dynamic descriptor set.
	VkDescriptorSetLayout _staticDescriptorSetLayout;
	VkDescriptorSetLayout _dynamicDescriptorSetLayout;

	// The Descriptor sets used for rendering.
	VkDescriptorSet _staticDescriptorSet;
	VkDescriptorSet _dynamicDescriptorSet;

	// The renderpass used for rendering frames. The renderpass encapsulates the high level structure of a frame.
	VkRenderPass _renderPass;

	// The framebuffer specifies a set of attachments used by the renderpass.
	VkFramebuffer _framebuffers[MAX_SWAPCHAIN_IMAGES];
	// The depth stencil images and views used for rendering.
	VkImage _depthStencilImages[MAX_SWAPCHAIN_IMAGES];
	VkImageView _depthStencilImageViews[MAX_SWAPCHAIN_IMAGES];

	// The format of the depth stencil images.
	VkFormat _depthStencilFormat;

	// The memory backing for the depth stencil images.
	VkDeviceMemory _depthStencilImageMemory[MAX_SWAPCHAIN_IMAGES];

	// Synchronisation primitives used for specifying dependencies and ordering during rendering frames.
	VkSemaphore _imageAcquireSemaphores[MAX_SWAPCHAIN_IMAGES];
	VkSemaphore _presentationSemaphores[MAX_SWAPCHAIN_IMAGES];
	VkFence _perFrameResourcesFences[MAX_SWAPCHAIN_IMAGES];

	// The queue to which various command buffers will be submitted to.
	VkQueue _queue;

	// The VkImage and VkImageView handle created for the triangle texture.
	VkImage _triangleImage;
	VkImageView _triangleImageView;

	// The VkFormat for the triangle texture.
	VkFormat _triangleImageFormat;

	// The memory backing for the triangle texture.
	VkDeviceMemory _triangleImageMemory;

	// The sampler handle used when sampling the triangle texture.
	VkSampler _bilinearSampler;

	// The command pool from which the command buffers will be allocated.
	VkCommandPool _commandPool;

	// The commands buffers to which commands are rendered. The commands can then be submitted together.
	VkCommandBuffer _cmdBuffers[MAX_SWAPCHAIN_IMAGES];

	// The layout specifying the descriptors used by the graphics pipeline.
	VkPipelineLayout _pipelineLayout;

	// The graphics pipeline specifying the funnel for which certain sets of Vulkan commands are sent through.
	VkPipeline _graphicsPipeline;

	// A pipeline cache providing mechanism for the reuse of the results of pipeline creation.
	VkPipelineCache _pipelineCache;

	// The shader modules used by the graphics pipeline.
	VkShaderModule _vertexShaderModule;
	VkShaderModule _fragmentShaderModule;

	// Per frame indices used for synchronisation.
	uint32_t _currentFrameIndex;
	uint32_t _swapchainIndex;

	// The index into the set of supported queue families which supports both graphics and presentation capabilities.
	uint32_t _graphicsQueueFamilyIndex;

	// The aligned size for the dynamic buffers taking into account the minUniformBufferOffsetAlignment member of the limits for the VkPhysicalDeviceProperties structure.
	uint32_t _dynamicBufferAlignedSize;

	// Matrices used for animation.
	glm::mat4 _modelMatrix;
	glm::mat4 _viewProjectionMatrix;
	float _rotationAngle;

	// The viewport and scissors used for rendering handling the portions of the surface written to.
	VkViewport _viewport;
	VkRect2D _scissor;

	// The size of a single vertex corresponding to the stride of a vertex.
	uint32_t _vboStride;

	// The size and data included in the triangle texture.
	VkExtent2D _textureDimensions;
	std::vector<uint8_t> _textureData;

	// Records the number of debug callback functions created by the application.
	uint32_t _numDebugCallbacks;

public:
	// The pvr::Shell functions which must be implemented by this application.
	pvr::Result initApplication();
	pvr::Result initView();
	pvr::Result releaseView();
	pvr::Result quitApplication();
	pvr::Result renderFrame();

	// Application specific functions.
	void createInstance();
	void initDebugCallbacks();
	void retrievePhysicalDevices();
	void createSurface(void* window, void* display, void* connection);
	void createLogicalDevice();
	void createSwapchain();
	void createDepthStencilImages();
	void createRenderPass();
	void createFramebuffer();
	void createSynchronisationPrimitives();
	void createCommandPool();
	void loadAndCreateShaderModules(const char* const shaderName, VkShaderModule& outShader);
	void allocateDeviceMemory(VkDevice& device, VkMemoryRequirements& memoryRequirements, VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties,
		const VkMemoryPropertyFlags requiredMemFlags, const VkMemoryPropertyFlags optimalMemFlags, VkDeviceMemory& outMemory, VkMemoryPropertyFlags& outMemFlags);
	void allocateCommandBuffers();
	void recordCommandBuffers();
	void createPipelineCache();
	void createPipeline();
	void createVbo();
	void createUniformBuffers();
	void createTexture();
	void generateTexture();
	void createDescriptorPool();
	void createDescriptorSetLayouts();
	void allocateDescriptorSets();
	void createShaderModules();
	void createPipelineLayout();
	void createBufferAndMemory(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags requiredMemFlags, VkMemoryPropertyFlags optimalMemFlags, VkBuffer& outBuffer,
		VkDeviceMemory& outMemory, VkMemoryPropertyFlags& outMemFlags);
	uint32_t getCompatibleQueueFamily();

	/// <summary>Default constructor for VulkanIntroducingPVRShell used to initialise the variables used throughout the demo.</summary>
	VulkanIntroducingPVRShell()
		: // Vulkan resource handles
		  _instance(VK_NULL_HANDLE), _physicalDevice(VK_NULL_HANDLE), _surface(VK_NULL_HANDLE), _swapchain(VK_NULL_HANDLE), _device(VK_NULL_HANDLE), _vbo(VK_NULL_HANDLE),
		  _vboMemory(VK_NULL_HANDLE), _modelViewProjectionMemory(VK_NULL_HANDLE), _modelViewProjectionBuffer(VK_NULL_HANDLE), _descriptorPool(VK_NULL_HANDLE),
		  _renderPass(VK_NULL_HANDLE), _queue(VK_NULL_HANDLE), _pipelineLayout(VK_NULL_HANDLE), _graphicsPipeline(VK_NULL_HANDLE), _commandPool(VK_NULL_HANDLE),
		  _pipelineCache(VK_NULL_HANDLE), _staticDescriptorSetLayout(VK_NULL_HANDLE), _dynamicDescriptorSetLayout(VK_NULL_HANDLE), _staticDescriptorSet(VK_NULL_HANDLE),
		  _dynamicDescriptorSet(VK_NULL_HANDLE), _triangleImage(VK_NULL_HANDLE), _triangleImageView(VK_NULL_HANDLE), _triangleImageMemory(VK_NULL_HANDLE),
		  _bilinearSampler(VK_NULL_HANDLE), _vertexShaderModule(VK_NULL_HANDLE), _fragmentShaderModule(VK_NULL_HANDLE),

		  // initialise variables used for animation
		  _modelMatrix(glm::mat4(1)), _viewProjectionMatrix(glm::mat4(1)), _rotationAngle(45.0f),

		  // initialise the other variables used throughout the demo
		  _enabledInstanceExtensionNames(0), _enabledLayerNames(0), _enabledDeviceExtensionNames(0), _physicalDeviceMemoryProperties(), _physicalDeviceProperties(),
		  _currentFrameIndex(0), _swapchainLength(0), _swapchainColorFormat(VK_FORMAT_UNDEFINED), _depthStencilFormat(VK_FORMAT_UNDEFINED),
		  _triangleImageFormat(VK_FORMAT_UNDEFINED), _swapchainIndex(-1), _viewport(), _scissor(), _vboStride(-1), _graphicsQueueFamilyIndex(-1),
		  _modelViewProjectionBufferSize(-1), _dynamicBufferAlignedSize(-1), _textureDimensions(), _textureData(0), _modelViewProjectionMappedMemory(0), _numDebugCallbacks(0)
	{
		for (uint32_t i = 0; i < MAX_SWAPCHAIN_IMAGES; ++i)
		{
			// per swapchain Vulkan resource handles
			_swapchainImages[i] = VK_NULL_HANDLE;
			_swapchainImageViews[i] = VK_NULL_HANDLE;
			_framebuffers[i] = VK_NULL_HANDLE;
			_depthStencilImages[i] = VK_NULL_HANDLE;
			_depthStencilImageMemory[i] = VK_NULL_HANDLE;
			_depthStencilImageViews[i] = VK_NULL_HANDLE;
			_imageAcquireSemaphores[i] = VK_NULL_HANDLE;
			_presentationSemaphores[i] = VK_NULL_HANDLE;
			_perFrameResourcesFences[i] = VK_NULL_HANDLE;
			_cmdBuffers[i] = VK_NULL_HANDLE;
			_debugReportCallbacks[i] = VK_NULL_HANDLE;
		}
	}
};

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it(e.g.external modules, loading meshes, etc.).If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanIntroducingPVRShell::initApplication()
{
	setBackBufferColorspace(pvr::ColorSpace::lRGB);
	// Here we are setting the back buffer colorspace value to lRGB for simplicity: We are working directly with the "final" sRGB
	// values in our textures and passing the values through.
	// Note, the default for PVRShell is sRGB: when doing anything but the most simplistic effects, you will need to
	// work with linear values in the shaders and then either perform gamma correction in the shader, or (if supported)
	// use an sRGB framebuffer (which performs this correction automatically).
	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change  in the rendering context. Used to initialize variables that are dependent on the
/// rendering context(e.g.textures, vertex buffers, etc.).</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanIntroducingPVRShell::initView()
{
	// Create the Vulkan instance object, initialise the Vulkan library and initialise the Vulkan instance function pointers
	createInstance();

#ifdef DEBUG
	// If supported enable the use of VkDebugReportCallbacks from VK_EXT_debug_report to enable logging of various validation layer messages.
	initDebugCallbacks();
#endif

	// Retrieve and create the various Vulkan resources and objects used throughout this demo
	retrievePhysicalDevices();
	createSurface(getWindow(), getDisplay(), getConnection());
	createLogicalDevice();
	createSwapchain();
	createDepthStencilImages();
	createRenderPass();
	createFramebuffer();
	createSynchronisationPrimitives();
	createCommandPool();
	createVbo();
	createUniformBuffers();
	createTexture();
	createDescriptorPool();
	createDescriptorSetLayouts();
	allocateDescriptorSets();
	createPipelineCache();
	createShaderModules();

	float aspect = 0.0f;
	// the screen is rotated
	if (isScreenRotated()) { aspect = static_cast<float>(getHeight()) / static_cast<float>(getWidth()); }
	else
	{
		aspect = static_cast<float>(getWidth()) / static_cast<float>(getHeight());
	}

	_viewProjectionMatrix = pvr::math::ortho(pvr::Api::Vulkan, aspect, -aspect, -1.f, 1.f);

	// Set the view port dimensions, depth and starting coordinates.
	_viewport.width = static_cast<float>(getWidth());
	_viewport.height = static_cast<float>(getHeight());
	_viewport.minDepth = 0.0f;
	_viewport.maxDepth = 1.0f;
	_viewport.x = 0;
	_viewport.y = 0;

	// Set the extent to the surface dimensions and the offset to 0
	_scissor.extent.width = static_cast<uint32_t>(getWidth());
	_scissor.extent.height = static_cast<uint32_t>(getHeight());
	_scissor.offset.x = 0;
	_scissor.offset.y = 0;

	createPipelineLayout();
	createPipeline();

	// we can destroy the shader modules after creating the pipeline.
	_deviceVkFunctions.vkDestroyShaderModule(_device, _vertexShaderModule, nullptr);
	_deviceVkFunctions.vkDestroyShaderModule(_device, _fragmentShaderModule, nullptr);

	// Allocate and record the various Vulkan commands to a set of command buffers.
	// Work is prepared, being validated during development, upfront and is buffered up and ready to go.
	// Each frame the pre validated, pre prepared work is submitted.
	allocateCommandBuffers();
	recordCommandBuffers();

	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanIntroducingPVRShell::renderFrame()
{
	// As discussed in "createSwapchain", the application doesn't actually "own" the presentation images meaning they cannot "just" render to the image
	// but must acquire an image from the presentation engine prior to making use of it. The act of acquiring an image from the presentation engine guarantees that
	// the presentation engine has completely finished with the image.

	// As with various other tasks in Vulkan rendering an image and presenting it to the screen takes some explanation, various commands and a fair amount of thought.

	// We are using a "canonical" way to do synchronisation that works in all but the most exotic of cases.
	// Calls to vkAcquireNextImageKHR, using a timeout of UINT64_MAX, will block until a presentable image from the swapchain can be acquired or will return an error.
	// Calls to vkAcquireNextImageKHR may return immediately and therefore we cannot rely simply on this call to meter our rendering speed, we instead make use of
	// The fence _perFrameResourcesFences[_swapchainIndex] to provides us with metered rendering and we make use of a semaphore _imageAcquireSemaphores[_currentFrameIndex] signalled
	// by the call to vkAcquireNextImageKHR to guarantee that the presentation engine has finished reading from the image meaning it is now safe for the image layout
	// and contents to be modified. The vkQueueSubmit call used to write to the swapchain image uses the semaphore _imageAcquireSemaphores[_currentFrameIndex] as a wait semaphore meaning
	// the vkQueueSubmit call will only be executed once the semaphore has been signalled by the vkAcquireNextImageKHR ensuring that the presentation
	// engine has relinquished control of the image. Only after this can the swapchain be safely modified.

	// A high level overview for rendering and presenting an image to the screen is as follows:
	// 1). Acquire a presentable image from the presentation engine. The index of the next image into which to render will be returned.
	// 2). Wait for the per frame resources fence to become signalled meaning the resources/command buffers for the current virtual frame are finished with.
	// 3). Render the image (update variables, vkQueueSubmit). We are using per swapchain pre-recorded command buffers so we only need to submit them on each frame.
	// 4). Present the acquired and now rendered image. Presenting an image returns ownership of the image back to the presentation engine.
	// 5). Increment (and wrap) the virtual frame index

	//
	// 1). Acquire a presentable image from the presentation engine. The index of the next image into which to render will be returned.
	//
	// The order in which images are acquired is implementation-dependent, and may be different than the order the images were presented
	vulkanSuccessOrDie(_deviceVkFunctions.vkAcquireNextImageKHR(_device, _swapchain, uint64_t(-1), _imageAcquireSemaphores[_currentFrameIndex], VK_NULL_HANDLE, &_swapchainIndex),
		"Failed to acquire next image");

	//
	// 2). Wait for the per frame resources fence to become signalled meaning the resources/command buffers for the current virtual frame are finished with.
	//
	// Wait for the command buffer from swapChainLength frames ago to be finished with.
	vulkanSuccessOrDie(
		_deviceVkFunctions.vkWaitForFences(_device, 1, &_perFrameResourcesFences[_swapchainIndex], true, uint64_t(-1)), "Failed to wait for per frame command buffer fence");
	vulkanSuccessOrDie(_deviceVkFunctions.vkResetFences(_device, 1, &_perFrameResourcesFences[_swapchainIndex]), "Failed to wait for per frame command buffer fence");

	// Update the model view projection buffer data
	{
		// Update our angle of rotation
		_rotationAngle += 0.02f;

		// Calculate the model matrix making use of the rotation angle
		_modelMatrix = glm::rotate(_rotationAngle, glm::vec3(0.0f, 0.0f, 1.0f));

		// Set the model view projection matrix
		const auto modelViewProjectionMatrix = _viewProjectionMatrix * _modelMatrix;

		// Update the model view projection matrix buffer data for the current swapchain index. Note that the memory for the whole buffer was mapped just after it was allocated so
		// care needs to be taken to only modify memory to use with the current swapchain. Other slices of the memory may still be in use.

		memcpy(static_cast<unsigned char*>(_modelViewProjectionMappedMemory) + _dynamicBufferAlignedSize * _swapchainIndex, &modelViewProjectionMatrix, sizeof(_viewProjectionMatrix));

		// If the model view projection buffer memory was allocated with VkMemoryPropertyFlags including VK_MEMORY_PROPERTY_HOST_COHERENT_BIT indicating that the host does
		// not need to manage the memory accesses explicitly using the host cache management commands vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges to flush host
		// writes to the device meaning we can safely assume writes have taken place prior to making use of the model view projection buffer memory.
		if ((_modelViewProjectionMemoryFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
		{
			// Flush the memory guaranteeing that host writes to the memory ranges specified are made available to the device.
			VkMappedMemoryRange memoryRange = {};
			memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			memoryRange.memory = _modelViewProjectionMemory;
			memoryRange.offset = _dynamicBufferAlignedSize * _swapchainIndex;
			memoryRange.size = sizeof(_viewProjectionMatrix);
			_deviceVkFunctions.vkFlushMappedMemoryRanges(_device, 1, &memoryRange);
		}
	}

	//
	// 3). Render the image (update variables, vkQueueSubmit). We are using per swapchain pre-recorded command buffers so we only need to submit them on each frame.
	//
	// Submit the specified command buffer to the given queue.
	// The queue submission will wait on the corresponding image acquisition semaphore to have been signalled.
	// The queue submission will signal the corresponding image presentation semaphore.
	// The queue submission will signal the corresponding per frame command buffer fence.
	VkSubmitInfo submitInfo = {};
	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pCommandBuffers = &_cmdBuffers[_swapchainIndex];
	submitInfo.pWaitSemaphores = &_imageAcquireSemaphores[_currentFrameIndex]; // wait for the image acquire to finish
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &_presentationSemaphores[_currentFrameIndex]; // signal submit
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	vulkanSuccessOrDie(_deviceVkFunctions.vkQueueSubmit(_queue, 1, &submitInfo, _perFrameResourcesFences[_swapchainIndex]), "Failed to submit queue");

	//
	// 4). Present the acquired and now rendered image. Presenting an image returns ownership of the image back to the presentation engine.
	//
	// Queues the current swapchain image for presentation.
	// The queue presentation will wait on the corresponding image presentation semaphore.
	VkPresentInfoKHR present = {};
	VkResult result;
	present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present.pImageIndices = &_swapchainIndex;
	present.swapchainCount = 1;
	present.pSwapchains = &_swapchain;
	present.pWaitSemaphores = &_presentationSemaphores[_currentFrameIndex]; // wait for the queue submit to finish
	present.waitSemaphoreCount = 1;
	present.pResults = &result;
	vulkanSuccessOrDie(_deviceVkFunctions.vkQueuePresentKHR(_queue, &present), "Failed to present the swapchain image");

	//
	// 5). Increment (and wrap) the virtual frame index
	//
	_currentFrameIndex = (_currentFrameIndex + 1) % _swapchainLength;

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanIntroducingPVRShell::releaseView()
{
	// Cleanly release all resources prior to exiting the application.

	// Wait for the device to finish with the resources prior to releasing them.
	if (_device) { vulkanSuccessOrDie(_deviceVkFunctions.vkDeviceWaitIdle(_device), "Failed to wait for the device to become idle"); }

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		if (_imageAcquireSemaphores[i])
		{
			_deviceVkFunctions.vkDestroySemaphore(_device, _imageAcquireSemaphores[i], nullptr);
			_imageAcquireSemaphores[i] = VK_NULL_HANDLE;
		}
		if (_presentationSemaphores[i])
		{
			_deviceVkFunctions.vkDestroySemaphore(_device, _presentationSemaphores[i], nullptr);
			_presentationSemaphores[i] = VK_NULL_HANDLE;
		}

		if (_perFrameResourcesFences[i])
		{
			vulkanSuccessOrDie(_deviceVkFunctions.vkWaitForFences(_device, 1, &_perFrameResourcesFences[i], true, uint64_t(-1)), "Failed to wait for per frame command buffer fence");
			vulkanSuccessOrDie(_deviceVkFunctions.vkResetFences(_device, 1, &_perFrameResourcesFences[i]), "Failed to reset per frame command buffer fence");

			_deviceVkFunctions.vkDestroyFence(_device, _perFrameResourcesFences[i], nullptr);
			_perFrameResourcesFences[i] = VK_NULL_HANDLE;
		}

		if (_framebuffers[i])
		{
			_deviceVkFunctions.vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
			_framebuffers[i] = VK_NULL_HANDLE;
		}
		if (_depthStencilImageViews[i])
		{
			_deviceVkFunctions.vkDestroyImageView(_device, _depthStencilImageViews[i], nullptr);
			_depthStencilImageViews[i] = VK_NULL_HANDLE;
		}
		if (_depthStencilImages[i])
		{
			_deviceVkFunctions.vkDestroyImage(_device, _depthStencilImages[i], nullptr);
			_depthStencilImages[i] = VK_NULL_HANDLE;
		}
		if (_swapchainImageViews[i])
		{
			_deviceVkFunctions.vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
			_swapchainImageViews[i] = VK_NULL_HANDLE;
		}
		if (_depthStencilImageMemory[i])
		{
			_deviceVkFunctions.vkFreeMemory(_device, _depthStencilImageMemory[i], nullptr);
			_depthStencilImageMemory[i] = VK_NULL_HANDLE;
		}
	}

	if (_renderPass)
	{
		_deviceVkFunctions.vkDestroyRenderPass(_device, _renderPass, nullptr);
		_renderPass = VK_NULL_HANDLE;
	}
	if (_pipelineLayout)
	{
		_deviceVkFunctions.vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
		_pipelineLayout = VK_NULL_HANDLE;
	}
	if (_graphicsPipeline)
	{
		_deviceVkFunctions.vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
		_graphicsPipeline = VK_NULL_HANDLE;
	}
	if (_pipelineCache)
	{
		_deviceVkFunctions.vkDestroyPipelineCache(_device, _pipelineCache, nullptr);
		_pipelineCache = VK_NULL_HANDLE;
	}
	if (_vbo)
	{
		_deviceVkFunctions.vkDestroyBuffer(_device, _vbo, nullptr);
		_vbo = VK_NULL_HANDLE;
	}
	if (_vboMemory)
	{
		_deviceVkFunctions.vkFreeMemory(_device, _vboMemory, nullptr);
		_vboMemory = VK_NULL_HANDLE;
	}

	if (_modelViewProjectionBuffer)
	{
		_deviceVkFunctions.vkDestroyBuffer(_device, _modelViewProjectionBuffer, nullptr);
		_modelViewProjectionBuffer = VK_NULL_HANDLE;
	}
	if (_modelViewProjectionMemory)
	{
		_deviceVkFunctions.vkFreeMemory(_device, _modelViewProjectionMemory, nullptr);
		_modelViewProjectionMemory = VK_NULL_HANDLE;
	}

	if (_triangleImageView)
	{
		_deviceVkFunctions.vkDestroyImageView(_device, _triangleImageView, nullptr);
		_triangleImageView = VK_NULL_HANDLE;
	}
	if (_triangleImage)
	{
		_deviceVkFunctions.vkDestroyImage(_device, _triangleImage, nullptr);
		_triangleImage = VK_NULL_HANDLE;
	}
	if (_triangleImageMemory)
	{
		_deviceVkFunctions.vkFreeMemory(_device, _triangleImageMemory, nullptr);
		_triangleImageMemory = VK_NULL_HANDLE;
	}

	if (_bilinearSampler)
	{
		_deviceVkFunctions.vkDestroySampler(_device, _bilinearSampler, nullptr);
		_bilinearSampler = VK_NULL_HANDLE;
	}

	if (_descriptorPool)
	{
		if (_staticDescriptorSet)
		{
			_deviceVkFunctions.vkFreeDescriptorSets(_device, _descriptorPool, 1, &_staticDescriptorSet);
			_staticDescriptorSet = VK_NULL_HANDLE;
		}
		if (_dynamicDescriptorSet)
		{
			_deviceVkFunctions.vkFreeDescriptorSets(_device, _descriptorPool, 1, &_dynamicDescriptorSet);
			_dynamicDescriptorSet = VK_NULL_HANDLE;
		}
		_deviceVkFunctions.vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
		_descriptorPool = VK_NULL_HANDLE;
	}

	if (_staticDescriptorSetLayout)
	{
		_deviceVkFunctions.vkDestroyDescriptorSetLayout(_device, _staticDescriptorSetLayout, nullptr);
		_staticDescriptorSetLayout = VK_NULL_HANDLE;
	}
	if (_dynamicDescriptorSetLayout)
	{
		_deviceVkFunctions.vkDestroyDescriptorSetLayout(_device, _dynamicDescriptorSetLayout, nullptr);
		_dynamicDescriptorSetLayout = VK_NULL_HANDLE;
	}

	if (_commandPool)
	{
		if (_cmdBuffers[0])
		{
			_deviceVkFunctions.vkFreeCommandBuffers(_device, _commandPool, _swapchainLength, &_cmdBuffers[0]);
			_cmdBuffers[0] = VK_NULL_HANDLE; // Only checking the first one
		}
		_deviceVkFunctions.vkDestroyCommandPool(_device, _commandPool, nullptr);
		_commandPool = VK_NULL_HANDLE;
	}

	if (_swapchain)
	{
		_deviceVkFunctions.vkDestroySwapchainKHR(_device, _swapchain, nullptr);
		_swapchain = VK_NULL_HANDLE;
	}
	if (_device)
	{
		_deviceVkFunctions.vkDestroyDevice(_device, nullptr);
		_device = VK_NULL_HANDLE;
	}
	if (_surface)
	{
		_instanceVkFunctions.vkDestroySurfaceKHR(_instance, _surface, nullptr);
		_surface = VK_NULL_HANDLE;
	}
	if (isExtensionEnabled(_enabledInstanceExtensionNames, VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
	{
		for (uint32_t i = 0; i < _numDebugCallbacks; i++)
		{
			if (_debugReportCallbacks[i])
			{
				_instanceVkFunctions.vkDestroyDebugReportCallbackEXT(_instance, _debugReportCallbacks[i], nullptr);
				_debugReportCallbacks[i] = VK_NULL_HANDLE;
			}
		}
	}
	if (_instance)
	{
		_instanceVkFunctions.vkDestroyInstance(_instance, nullptr);
		_instance = VK_NULL_HANDLE;
	}

	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.</summary>
/// <returns>Result::Success if no error occurred</returns>.
pvr::Result VulkanIntroducingPVRShell::quitApplication() { return pvr::Result::Success; }

/// <summary>Create the Vulkan application instance.</summary>
void VulkanIntroducingPVRShell::createInstance()
{
	// Initialise Vulkan by loading the Vulkan commands and creating the VkInstance.

	// We make use of vk_bindings.h and vk_bindings_helper.h for defining and initialising the Vulkan function pointer tables
	// Vulkan commands aren't all necessarily exposed statically on the target platform however all Vulkan commands can be retrieved using vkGetInstanceProcAddr.
	// In vk_bindings_helper.h the function pointer for vkGetInstanceProcAddr is obtained using GetProcAddress, dlsym etc.
	// The function pointer vkGetInstanceProcAddr is then used to retrieve the following additional Vulkan commands:
	// vkEnumerateInstanceExtensionProperties, vkEnumerateInstanceLayerProperties and vkCreateInstance
	if (!initVkBindings(&_vkBindings)) { throw pvr::PvrError("Unable to initialise Vulkan."); }

	uint32_t major = -1;
	uint32_t minor = -1;
	uint32_t patch = -1;

	// If a valid function pointer for vkEnumerateInstanceVersion cannot be retrieved then Vulkan only 1.0 is supported by the implementation otherwise we can use
	// vkEnumerateInstanceVersion to determine the API version supported.
	if (_vkBindings.vkEnumerateInstanceVersion)
	{
		uint32_t supportedApiVersion;
		_vkBindings.vkEnumerateInstanceVersion(&supportedApiVersion);

		major = VK_VERSION_MAJOR(supportedApiVersion);
		minor = VK_VERSION_MINOR(supportedApiVersion);
		patch = VK_VERSION_PATCH(supportedApiVersion);

		Log(LogLevel::Information, "The function pointer for 'vkEnumerateInstanceVersion' was valid. Supported instance version: ([%d].[%d].[%d]).", major, minor, patch);
	}
	else
	{
		major = 1;
		minor = 0;
		patch = 0;
		Log(LogLevel::Information, "Could not find a function pointer for 'vkEnumerateInstanceVersion'. Setting instance version to: ([%d].[%d].[%d]).", major, minor, patch);
	}

	// Fill in the application info structure which can help an implementation recognise behaviour inherent to various classes of applications
	VkApplicationInfo appInfo = {};
	appInfo.pApplicationName = "VulkanIntroducingPVRShell";
	appInfo.applicationVersion = 1;
	appInfo.engineVersion = 1;
	appInfo.pEngineName = "VulkanIntroducingPVRShell";
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.apiVersion = VK_MAKE_VERSION(major, minor, patch);

	// Retrieve a list of supported instance extensions and filter them based on a set of requested instance extension to be enabled.
	uint32_t numExtensions = 0;
	vulkanSuccessOrDie(_vkBindings.vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, nullptr), "Failed to enumerate Instance Extension properties");
	std::vector<VkExtensionProperties> extensiosProps(numExtensions);
	vulkanSuccessOrDie(_vkBindings.vkEnumerateInstanceExtensionProperties(nullptr, &numExtensions, extensiosProps.data()), "Failed to enumerate Instance Extension properties");

	_enabledInstanceExtensionNames = filterExtensions(extensiosProps, Extensions::InstanceExtensions, ARRAY_SIZE(Extensions::InstanceExtensions));

	// Vulkan, by nature of its minimalistic design, provides very little information to the developer regarding API issues. Error checking and validation of state is minimal.
	// One of the key principles of Vulkan is that the preparation and submission of work should be highly efficient; removing error checking and validation of state from Vulkan
	// implementations is one of the many ways in which this was enabled.
	// Vulkan is a layered API whereby it can optionally make use of additional layers for debugging, validation and other purposes with the core Vulkan layer being the lowest in
	// the stack.

	// Generally implementations assume applications are using the Vulkan API correctly. When an application uses the Vulkan incorrectly core Vulkan may behave in
	// undefined ways including through program termination.

	// Generally the validation of correct API usage is left to a set of validation layers.
	// Applications should be developed using these validation layers extensively to help identify and fix errors however once applications are validated applications should
	// disable the validation layers prior to being released.

	// This application makes use of The Khronos Vulkan-LoaderAndValidationLayers: https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers
	// Other layers exist for various other reasons such as VK_LAYER_POWERVR_carbon and VK_LAYER_ARM_mali_perf_doc.
	uint32_t numLayers = 0;
	vulkanSuccessOrDie(_vkBindings.vkEnumerateInstanceLayerProperties(&numLayers, nullptr), "Failed to enumerate Instance Layer properties");
	std::vector<VkLayerProperties> layerProps(numLayers);
	vulkanSuccessOrDie(_vkBindings.vkEnumerateInstanceLayerProperties(&numLayers, layerProps.data()), "Failed to enumerate Instance Layer properties");

	_enabledLayerNames = filterLayers(layerProps, Layers::InstanceLayers, ARRAY_SIZE(Layers::InstanceLayers));

	bool requestedStdValidation = false;
	bool supportsStdValidation = false;
	bool supportsKhronosValidation = false;
	uint32_t stdValidationRequiredIndex = -1;

	for (uint32_t i = 0; i < ARRAY_SIZE(Layers::InstanceLayers); ++i)
	{
		if (!strcmp(Layers::InstanceLayers[i].c_str(), "VK_LAYER_LUNARG_standard_validation"))
		{
			requestedStdValidation = true;
			break;
		}
	}

	for (const auto& SupportedInstanceLayer : _enabledLayerNames)
	{
		if (!strcmp(SupportedInstanceLayer.c_str(), "VK_LAYER_LUNARG_standard_validation")) { supportsStdValidation = true; }
		if (!strcmp(SupportedInstanceLayer.c_str(), "VK_LAYER_KHRONOS_validation")) { supportsKhronosValidation = true; }
	}

	// This code is to cover cases where VK_LAYER_LUNARG_standard_validation is requested but is not supported, where on some platforms the
	// component layers enabled via VK_LAYER_LUNARG_standard_validation may still be supported even though VK_LAYER_LUNARG_standard_validation is not.
	// Only perform the expansion if VK_LAYER_LUNARG_standard_validation is requested and not supported and the newer equivalent layer VK_LAYER_KHRONOS_validation is also not supported
	if (requestedStdValidation && !supportsStdValidation && !supportsKhronosValidation)
	{
		// This code is to cover cases where VK_LAYER_LUNARG_standard_validation is requested but is not supported, where on some platforms the
		// component layers enabled via VK_LAYER_LUNARG_standard_validation may still be supported even though VK_LAYER_LUNARG_standard_validation is not.
		for (auto it = layerProps.begin(); !supportsStdValidation && it != layerProps.end(); ++it)
		{ supportsStdValidation = !strcmp(it->layerName, "VK_LAYER_LUNARG_standard_validation"); }
		if (!supportsStdValidation)
		{
			for (uint32_t i = 0; stdValidationRequiredIndex == static_cast<uint32_t>(-1) && i < layerProps.size(); ++i)
			{
				if (!strcmp(Layers::InstanceLayers[i].c_str(), "VK_LAYER_LUNARG_standard_validation")) { stdValidationRequiredIndex = i; }
			}

			for (uint32_t j = 0; j < ARRAY_SIZE(Layers::InstanceLayers); ++j)
			{
				if (stdValidationRequiredIndex == j && !supportsStdValidation)
				{
					const char* stdValComponents[] = { "VK_LAYER_GOOGLE_threading", "VK_LAYER_LUNARG_parameter_validation", "VK_LAYER_LUNARG_object_tracker",
						"VK_LAYER_LUNARG_core_validation", "VK_LAYER_GOOGLE_unique_objects" };
					for (uint32_t k = 0; k < sizeof(stdValComponents) / sizeof(stdValComponents[0]); ++k)
					{
						for (uint32_t i = 0; i < layerProps.size(); ++i)
						{
							if (!strcmp(stdValComponents[k], layerProps[i].layerName))
							{
								_enabledLayerNames.push_back(std::string(stdValComponents[k]));
								break;
							}
						}
					}
				}
			}

			// filter the layers again checking for support for the component layers enabled via VK_LAYER_LUNARG_standard_validation
			_enabledLayerNames = filterLayers(layerProps, _enabledLayerNames.data(), static_cast<uint32_t>(_enabledLayerNames.size()));
		}
	}

	// copy the set of instance extensions and instance layers
	std::vector<const char*> enabledExtensions;
	std::vector<const char*> enableLayers;

	enabledExtensions.resize(_enabledInstanceExtensionNames.size());
	for (uint32_t i = 0; i < _enabledInstanceExtensionNames.size(); ++i) { enabledExtensions[i] = _enabledInstanceExtensionNames[i].c_str(); }

	enableLayers.resize(_enabledLayerNames.size());
	for (uint32_t i = 0; i < _enabledLayerNames.size(); ++i) { enableLayers[i] = _enabledLayerNames[i].c_str(); }

	// Here we create our Instance Info and assign our app info to it
	// along with our instance layers and extensions.
	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
	instanceInfo.ppEnabledExtensionNames = enabledExtensions.data();
	instanceInfo.enabledLayerCount = static_cast<uint32_t>(enableLayers.size());
	instanceInfo.ppEnabledLayerNames = enableLayers.data();

	// Create our Vulkan Application Instance.
	vulkanSuccessOrDie(_vkBindings.vkCreateInstance(&instanceInfo, nullptr, &_instance), "Failed to create the Vulkan Instance");

	// Initialize the Function pointers that require the Instance handle
	initVkInstanceBindings(_instance, &_instanceVkFunctions, _vkBindings.vkGetInstanceProcAddr);
}

/// <summary>Creates Debug Report Callbacks which will provide .</summary>
void VulkanIntroducingPVRShell::initDebugCallbacks()
{
	// Create debug report callbacks using the VK_EXT_debug_report extension providing a way for the Vulkan layers and the implementation itself to call back to the application
	// in particular circumstances.

	if (isExtensionEnabled(_enabledInstanceExtensionNames, VK_EXT_DEBUG_REPORT_EXTENSION_NAME))
	{
		// Setup callback creation information
		VkDebugReportCallbackCreateInfoEXT callbackCreateInfo;
		callbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		callbackCreateInfo.pNext = nullptr;
		// Specify which types of messages should be forwarded back to the call back function.
		callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
		// Specify the application defined callback function defined above.
		callbackCreateInfo.pfnCallback = &logMessageDebugReportCallback;
		callbackCreateInfo.pUserData = nullptr;

		// Register the first callback which logs messages of all VkDebugReportFlagsEXT types.
		vulkanSuccessOrDie(
			_instanceVkFunctions.vkCreateDebugReportCallbackEXT(_instance, &callbackCreateInfo, nullptr, &_debugReportCallbacks[0]), "Failed to create debug report callbacks");

		// Register the second callback which throws exceptions when events of type VK_DEBUG_REPORT_ERROR_BIT_EXT occur.
		callbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT;
		callbackCreateInfo.pfnCallback = &logMessageDebugReportCallback;

		// Register the callback
		vulkanSuccessOrDie(
			_instanceVkFunctions.vkCreateDebugReportCallbackEXT(_instance, &callbackCreateInfo, nullptr, &_debugReportCallbacks[1]), "Failed to create debug report callbacks");
		_numDebugCallbacks = 2;
	}
}

/// <summary>Retrieve the physical devices from the list of available physical devices of the instance.</summary>
void VulkanIntroducingPVRShell::retrievePhysicalDevices()
{
	// Retrieve a physical device to use for this demo.
	// Note that a physical device usually corresponds to a single device in the system.

	uint32_t physicalDeviceCount;

	// Retrieve the number of physical devices available. The number of physical devices available will be returned in physicalDeviceCount when pPhysicalDevices is NULL (nullptr).
	vulkanSuccessOrDie(_instanceVkFunctions.vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, nullptr), "Failed to enumerate the physical devices");

	// The number of physical devices must be larger than or equal to 1 in this demo.
	if (physicalDeviceCount == 0) { throw pvr::PvrError("Physical Device Count must be 1 or greater"); }

	VkPhysicalDevice physicalDevices[16];

	// Retrieves physicalDeviceCount physical devices which corresponds to the number of elements in the pPhysicalDevices array.
	vulkanSuccessOrDie(_instanceVkFunctions.vkEnumeratePhysicalDevices(_instance, &physicalDeviceCount, physicalDevices), "Failed to enumerate the physical devices");
	_physicalDevice = VK_NULL_HANDLE;
	for (uint32_t i = 0; i < physicalDeviceCount; ++i)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		_instanceVkFunctions.vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);
		_instanceVkFunctions.vkGetPhysicalDeviceFeatures(physicalDevices[i], &deviceFeatures);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		{
			// We return the device compatible with our needs.
			Log(LogLevel::Information, "Active Device is -- %s", deviceProperties.deviceName);
			_physicalDevice = physicalDevices[i];
			break;
		}
	}

	if (physicalDeviceCount && _physicalDevice == VK_NULL_HANDLE)
	{
		// If there's only one device we return that one.
		_physicalDevice = physicalDevices[0];
	}

	// Retrieve the VkPhysicalDeviceMemoryProperties for the physical device. This structure describes the memory heaps exposed by the platform in addition
	// to the memory types that may be used to access memory allocated from those heaps.
	// The memory heaps describe a memory resource of a particular size.
	// The memory types describe a set of properties which can be used with the corresponding memory heap.
	_instanceVkFunctions.vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &_physicalDeviceMemoryProperties);

	// Query for the set of general physical device properties including the device limits, supported API version and the type of the physical device.
	_instanceVkFunctions.vkGetPhysicalDeviceProperties(_physicalDevice, &_physicalDeviceProperties);
}

/// <summary>Creates the surface used by the demo.</summary>
/// <param name="window">A platform agnostic window.</param>
/// <param name="display">A platform agnostic display.</param>
/// <param name="connection">A platform agnostic connection.</param>
void VulkanIntroducingPVRShell::createSurface(void* window, void* display, void* connection)
{
	// Create the native platform surface abstracted via a VkSurfaceKHR object which this application will make use of in particular with the VK_KHR_swapchain extension.
	// Applications may also, on some platforms, present rendered images directly to display devices without the need for an intermediate Window System. The extension
	// VK_KHR_display in particular can be used for this task.

	// In Vulkan each platform may require unique window integration steps and therefore allows for an abstracted platform independent surface to be created.
	// To facilitate this, each platform provides its own Window System Integration (WSI) extension containing platform specific functions for using their own WSI.
	// Vulkan requires that the use of these extensions is guarded by preprocessor symbols defined in Vulkan's Window System-Specific Header Control appendix.
	// For VulkanIntroducingPVRShell to appropriately make use of the WSI extensions for a given platform it must #define the appropriate symbols for the platform prior to
	// including Vulkan.h header file. The appropriate set of preprocessor symbols are defined at the top of this file based on a set of compilation flags used to compile this demo.

	// Note that each WSI extension must be appropriately enabled as an instance extension prior to using them. This is controlled via the use of the array
	// Extensions::InstanceExtensions which is constructed at compile time based on the same set of compilation flags described above.

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	// Creates a VkSurfaceKHR object for an Android native window
	if (isExtensionEnabled(_enabledInstanceExtensionNames, VK_KHR_ANDROID_SURFACE_EXTENSION_NAME))
	{
		VkAndroidSurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.pNext = nullptr;
		surfaceInfo.flags = 0;
		surfaceInfo.window = (ANativeWindow*)window;
		vulkanSuccessOrDie(_instanceVkFunctions.vkCreateAndroidSurfaceKHR(_instance, &surfaceInfo, nullptr, &_surface), "Could not create Android Window Surface");
	}
	else
	{
		throw pvr::PvrError("Android surface instance extensions not supported");
	}
#elif defined VK_USE_PLATFORM_WIN32_KHR
	(void)connection;
	(void)display;
	// Creates a VkSurfaceKHR object for a Win32 window
	if (isExtensionEnabled(_enabledInstanceExtensionNames, VK_KHR_WIN32_SURFACE_EXTENSION_NAME))
	{
		VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.pNext = nullptr;
		surfaceInfo.hinstance = (HINSTANCE)GetModuleHandle(NULL);
		surfaceInfo.hwnd = (HWND)window;
		surfaceInfo.flags = 0;
		vulkanSuccessOrDie(_instanceVkFunctions.vkCreateWin32SurfaceKHR(_instance, &surfaceInfo, nullptr, &_surface), "Could not create Win32 Window Surface");
	}
	else
	{
		throw pvr::PvrError("Win32 surface instance extensions not supported");
	}
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	// Creates a VkSurfaceKHR object for an X11 window, using the XCB client-side library.
	if (isExtensionEnabled(_enabledInstanceExtensionNames, VK_KHR_XCB_SURFACE_EXTENSION_NAME))
	{
		VkXcbSurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.pNext = nullptr;
		surfaceInfo.connection = static_cast<xcb_connection_t*>(connection);
		surfaceInfo.window = *((xcb_window_t*)(&window));
		vulkanSuccessOrDie(_instanceVkFunctions.vkCreateXcbSurfaceKHR(_instance, &surfaceInfo, nullptr, &_surface), "Could not create XCB Window Surface");
	}
	else
	{
		throw pvr::PvrError("XCB surface instance extensions not supported");
	}
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	// Creates a VkSurfaceKHR object for an X11 window, using the Xlib client-side library.
	if (isExtensionEnabled(_enabledInstanceExtensionNames, VK_KHR_XLIB_SURFACE_EXTENSION_NAME))
	{
		VkXlibSurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.pNext = nullptr;
		surfaceInfo.dpy = (Display*)display;
		surfaceInfo.window = (Window)window;
		vulkanSuccessOrDie(_instanceVkFunctions.vkCreateXlibSurfaceKHR(_instance, &surfaceInfo, NULL, &_surface), "Could not create Xlib Window Surface");
	}
	else
	{
		throw pvr::PvrError("XLib surface instance extensions not supported");
	}
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	// Creates a VkSurfaceKHR object for a Wayland surface.
	if (isExtensionEnabled(_enabledInstanceExtensionNames, VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME))
	{
		VkWaylandSurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.pNext = nullptr;
		surfaceInfo.display = (wl_display*)display;
		surfaceInfo.surface = (wl_surface*)window;
		vulkanSuccessOrDie(_instanceVkFunctions.vkCreateWaylandSurfaceKHR(_instance, &surfaceInfo, NULL, &_surface), "Could not create Wayland Window Surface");
	}
	else
	{
		throw pvr::PvrError("Wayland surface instance extensions not supported");
	}
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
	// Creates a VkSurfaceKHR object from an NSView
	if (isExtensionEnabled(_enabledInstanceExtensionNames, VK_MVK_MACOS_SURFACE_EXTENSION_NAME))
	{
		VkMacOSSurfaceCreateInfoMVK surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
		surfaceInfo.pNext = 0;
		surfaceInfo.flags = 0;
		// pView must be a valid NSView and must be backed by a CALayer instance of type CAMetalLayer.
		surfaceInfo.pView = window;

		vulkanSuccessOrDie(_instanceVkFunctions.vkCreateMacOSSurfaceMVK(_instance, &surfaceInfo, NULL, &_surface), "Could not create Xlib Window Surface");
	}
#else
	if (isExtensionEnabled(_enabledInstanceExtensionNames, VK_KHR_DISPLAY_EXTENSION_NAME))
	{
		// Creates a VkSurfaceKHR structure for a display surface.
		VkDisplayPropertiesKHR properties;
		uint32_t propertiesCount = 0;

		vulkanSuccessOrDie(_instanceVkFunctions.vkGetPhysicalDeviceDisplayPropertiesKHR(_physicalDevice, &propertiesCount, NULL), "Could not get physical device display properties");
		if (propertiesCount != 1) { throw pvr::PvrError("Only a single display is supported"); }
		vulkanSuccessOrDie(
			_instanceVkFunctions.vkGetPhysicalDeviceDisplayPropertiesKHR(_physicalDevice, &propertiesCount, &properties), "Could not get physical device display properties");

		VkDisplayKHR nativeDisplay = properties.display;

		uint32_t modeCount = 0;
		vulkanSuccessOrDie(_instanceVkFunctions.vkGetDisplayModePropertiesKHR(_physicalDevice, nativeDisplay, &modeCount, NULL), "Could not get display mode properties");
		std::vector<VkDisplayModePropertiesKHR> modeProperties;
		modeProperties.resize(modeCount);
		vulkanSuccessOrDie(_instanceVkFunctions.vkGetDisplayModePropertiesKHR(_physicalDevice, nativeDisplay, &modeCount, modeProperties.data()), "Could not get display mode properties");

		VkDisplaySurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.pNext = nullptr;
		surfaceInfo.displayMode = modeProperties[0].displayMode;
		surfaceInfo.planeIndex = 0;
		surfaceInfo.planeStackIndex = 0;
		surfaceInfo.transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		surfaceInfo.globalAlpha = 0.0f;
		surfaceInfo.alphaMode = VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR;
		surfaceInfo.imageExtent = modeProperties[0].parameters.visibleRegion;

		vulkanSuccessOrDie(_instanceVkFunctions.vkCreateDisplayPlaneSurfaceKHR(_instance, &surfaceInfo, NULL, &_surface), "Could not create DisplayPlane Window Surface");
	}
	else
	{
		throw pvr::PvrError("Display surface instance extensions not supported");
	}
#endif
}

/// <summary>Get the compatible queue families from the device selected.</summary>
uint32_t VulkanIntroducingPVRShell::getCompatibleQueueFamily()
{
	// Attempts to retrieve a queue family which supports both graphics and presentation for the given application surface. This application has been
	// written in such a way which requires that the graphics and presentation queue families match.
	// Not all physical devices will support Window System Integration (WSI) support furthermore not all queue families for a particular physical device will support
	// presenting to the screen and thus these capabilities must be separately queried for support.

	uint32_t queueFamilyCount;
	// Retrieves the number of queue families the physical device supports.
	_instanceVkFunctions.vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);

	std::vector<VkBool32> queueFamilySupportsPresentation;
	queueFamilySupportsPresentation.resize(queueFamilyCount);

	std::vector<VkQueueFamilyProperties> queueFamilyProperties;
	queueFamilyProperties.resize(queueFamilyCount);

	// Retrieves the properties of queues available on the physical device
	_instanceVkFunctions.vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

	// For each queue family query whether it supports presentation and ensure the same queue family also supports graphics capabilities.
	for (uint32_t i = 0; i < queueFamilyCount; ++i)
	{
		vulkanSuccessOrDie(_instanceVkFunctions.vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _surface, &queueFamilySupportsPresentation[i]),
			"Unable to determine whether the specified queue family supports presentation for the given surface");
		if (((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) && queueFamilySupportsPresentation[i]) { return i; }
	}

	throw pvr::PvrError("Could not find a compatible queue family supporting both graphics capabilities and presentation to the screen");
}

/// <summary>Create the logical device.</summary>
void VulkanIntroducingPVRShell::createLogicalDevice()
{
	// Create the logical device used throughout the demo.

	// Logical devices represent logical connections to underlying physical devices.
	// A logical device provides main interface for an application to access the resources of the physical device and the physical device itself including:
	//		Creation of queues.
	//		Creation and management of synchronisation primitives.
	//		Allocation, release and management of memory.
	//		Creation and destruction of command buffers and command buffer pools.
	//		Creation, management and destruction of other graphics state including pipelines and resource descriptors.
	// Note that each physical device may correspond to multiple logical devices each of which specifying different extensions, capabilities and queues.

	// As part of logical device creation the application may also provide a set of queues that are requested for creation along with the logical device.
	// This application simply requests the creation of a single queue, from a single queue family specified by passing a single VkDeviceQueueCreateInfo structure
	// to the device creation structure as pQueueCreateInfos.

	// Attempt to find a suitable queue family which supports both Graphics and presentation.
	_graphicsQueueFamilyIndex = getCompatibleQueueFamily();

	// Queues are each assigned priorities ranging from 0.0 - 1.0 with higher priority queues having the potential to be allotted more processing time than queues with lower
	// priority although queue scheduling is completely implementation dependent.
	// Note that there are no guarantees about higher priority queues receiving more processing time or better quality of service than lower priority queues.
	// Also note that in our case we only have one queue so the priority specified doesn't matter.
	float queuePriorities[1] = { 1.0f };

	// Lets set up the device queue information.
	VkDeviceQueueCreateInfo queueInfo = {};
	queueInfo.queueFamilyIndex = _graphicsQueueFamilyIndex;
	queueInfo.pQueuePriorities = queuePriorities;
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueCount = 1;

	// Setup the VkDeviceCreateInfo structure used to create out logical device.
	VkDeviceCreateInfo deviceInfo = {};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	// Logical device layers are deprecated and the following members will be ignored by implementations.
	// For more information refer to "Device Layer Deprecation" in the Vulkan spec.
	{
		deviceInfo.enabledLayerCount = 0;
		deviceInfo.ppEnabledLayerNames = nullptr;
	}

	// Another important part of logical device creation is the specification of any required device extensions to enable. As described above "Extensions" in
	// Vulkan extensions may provide additional functionality not included in or used by Core Vulkan. The set of device specific extensions to enable are defined
	// by Extensions::DeviceExtensions.

	// Retrieve a list of supported device extensions and filter them based on a set of requested instance extension to be enabled.
	uint32_t numExtensions = 0;
	vulkanSuccessOrDie(_instanceVkFunctions.vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &numExtensions, nullptr), "Failed to enumerate Device Extension properties");
	std::vector<VkExtensionProperties> extensiosProps(numExtensions);
	vulkanSuccessOrDie(_instanceVkFunctions.vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &numExtensions, extensiosProps.data()),
		"Failed to enumerate Device Extension properties");

	_enabledDeviceExtensionNames = filterExtensions(extensiosProps, Extensions::DeviceExtensions, ARRAY_SIZE(Extensions::DeviceExtensions));

	// copy the set of device extensions
	std::vector<const char*> enabledExtensions;

	enabledExtensions.resize(_enabledDeviceExtensionNames.size());
	for (uint32_t i = 0; i < _enabledDeviceExtensionNames.size(); ++i) { enabledExtensions[i] = _enabledDeviceExtensionNames[i].c_str(); }

	deviceInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
	deviceInfo.ppEnabledExtensionNames = enabledExtensions.data();
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &queueInfo;

	// A physical device may well support a set of fine grained features which are not mandated by the specification, support for these features is retrieved and then enabled
	// feature by feature.

	// First query for the supported set of physical device features.
	VkPhysicalDeviceFeatures deviceFeatures;
	_instanceVkFunctions.vkGetPhysicalDeviceFeatures(_physicalDevice, &deviceFeatures);

	// Ensure that robustBufferAccess is disabled
	deviceFeatures.robustBufferAccess = false;

	deviceInfo.pEnabledFeatures = &deviceFeatures;

	// Create the logical device.
	vulkanSuccessOrDie(_instanceVkFunctions.vkCreateDevice(_physicalDevice, &deviceInfo, nullptr, &_device), "Failed to create the Vulkan logical device");

	// Initialize the Function pointers that require the Device address
	initVkDeviceBindings(_device, &_deviceVkFunctions, _instanceVkFunctions.vkGetDeviceProcAddr);

	// Retrieve the creation queue(s)
	_deviceVkFunctions.vkGetDeviceQueue(_device, queueInfo.queueFamilyIndex, 0, &_queue);
}

/// <summary>Gets a set of corrected screen extents based on the surface's capabilities.</summary>
/// <param name="surfaceCapabilities">A set of capabilities for the application surface including min/max image counts and extents.</param>
/// <param name="width">Request window width which will be checked for compatibility with the surface.</param>
/// <param name="height">Request window height which will be checked for compatibility with the surface.</param>
/// <returns>A set of corrected window extents compatible with the application's surface's capabilities.</returns>
void correctWindowExtents(const VkSurfaceCapabilitiesKHR& surfaceCapabilities, pvr::DisplayAttributes& attr)
{
	// Retrieves a set of correct window extents based on the requested width, height and surface capabilities.
	if (attr.width == 0) { attr.width = surfaceCapabilities.currentExtent.width; }
	if (attr.height == 0) { attr.height = surfaceCapabilities.currentExtent.height; }

	attr.width = std::max<uint32_t>(surfaceCapabilities.minImageExtent.width, std::min<uint32_t>(attr.width, surfaceCapabilities.maxImageExtent.width));

	attr.height = std::max<uint32_t>(surfaceCapabilities.minImageExtent.height, std::min<uint32_t>(attr.height, surfaceCapabilities.maxImageExtent.height));
}

/// <summary>Selects the presentation mode to be used when creating the based on physical device surface presentation modes supported and
/// a preset orderer list of presentation modes.</summary>
/// <param name="displayAttributes">A set of display configuration parameters.</param>
/// <param name="modes">A list of presentation modes supported by the physical device surface.</param>
/// <param name="presentationMode">The chosen presentation mode will be returned by reference.</param>
/// <param name="swapLength">The chosen swapchain length based on the presentation mode will be returned by reference.</param>
void selectPresentMode(const pvr::DisplayAttributes& displayAttributes, std::vector<VkPresentModeKHR>& modes, VkPresentModeKHR& presentationMode, uint32_t& swapLength)
{
	// With VK_PRESENT_MODE_FIFO_KHR the presentation engine will wait for the next vblank (vertical blanking period) to update the current image. When using FIFO tearing cannot
	// occur. VK_PRESENT_MODE_FIFO_KHR is required to be supported.
	presentationMode = VK_PRESENT_MODE_FIFO_KHR;
	VkPresentModeKHR desiredSwapMode = VK_PRESENT_MODE_FIFO_KHR;

	// We make use of PVRShell for handling command line arguments for configuring vsync modes using the -vsync command line argument.
	switch (displayAttributes.vsyncMode)
	{
	case pvr::VsyncMode::Off: desiredSwapMode = VK_PRESENT_MODE_IMMEDIATE_KHR; break;
	case pvr::VsyncMode::Mailbox: desiredSwapMode = VK_PRESENT_MODE_MAILBOX_KHR; break;
	case pvr::VsyncMode::Relaxed:
		desiredSwapMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
		break;
		// Default vsync mode.
	case pvr::VsyncMode::On: break;
	default: Log(LogLevel::Information, "Unexpected Vsync Mode specified. Defaulting to VK_PRESENT_MODE_FIFO_KHR");
	}

	// Verify that the desired presentation mode is present in the list of supported VkPresentModeKHRs.
	for (size_t i = 0; i < modes.size(); i++)
	{
		VkPresentModeKHR currentPresentMode = modes[i];

		// Primary matches : Check for a precise match between the desired presentation mode and the presentation modes supported.
		if (currentPresentMode == desiredSwapMode)
		{
			presentationMode = desiredSwapMode;
			break;
		}
		// Secondary matches : Immediate and Mailbox are better fits for each other than FIFO, so set them as secondary
		// If the user asked for Mailbox, and we found Immediate, set it (in case Mailbox is not found) and keep looking
		if ((desiredSwapMode == VK_PRESENT_MODE_MAILBOX_KHR) && (currentPresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)) { presentationMode = VK_PRESENT_MODE_IMMEDIATE_KHR; }
		// ... And vice versa: If the user asked for Immediate, and we found Mailbox, set it (in case Immediate is not found) and keep looking
		if ((desiredSwapMode == VK_PRESENT_MODE_IMMEDIATE_KHR) && (currentPresentMode == VK_PRESENT_MODE_MAILBOX_KHR)) { presentationMode = VK_PRESENT_MODE_MAILBOX_KHR; }
	}
	switch (presentationMode)
	{
	case VK_PRESENT_MODE_IMMEDIATE_KHR: Log(LogLevel::Information, "Presentation mode: Immediate (Vsync OFF)"); break;
	case VK_PRESENT_MODE_MAILBOX_KHR: Log(LogLevel::Information, "Presentation mode: Mailbox (Triple-buffering)"); break;
	case VK_PRESENT_MODE_FIFO_KHR: Log(LogLevel::Information, "Presentation mode: FIFO (Vsync ON)"); break;
	case VK_PRESENT_MODE_FIFO_RELAXED_KHR: Log(LogLevel::Information, "Presentation mode: Relaxed FIFO (Relaxed Vsync)"); break;
	default: assertion(false, "Unrecognised presentation mode"); break;
	}

	// Set the swapchain length if it has not already been set.
	if (!displayAttributes.swapLength) { swapLength = 3; }
}

/// <summary>Creates swapchain to present images on the surface.</summary>
void VulkanIntroducingPVRShell::createSwapchain()
{
	// Creates the WSI Swapchain object providing the ability to present rendering results to the surface.

	// A swapchain provides the abstraction for a set of presentable images (VkImage objects), with a particular view (VkImageView), associated with a surface
	// (VkSurfaceKHR), to be used for screen rendering.

	// The swapchain provides the necessary functionality for the application to explicitly handle multi buffering (double/triple buffering). The swapchain provides the
	// functionality to present a single image at a time but also allows the application to queue up other images for presentation. An application will render images and queue them
	// for presentation to the surface.

	// The physical device surface may well only support a certain set of VkFormats/VkColorSpaceKHR pairs for the presentation images in their presentation engine.
	uint32_t formatCount;
	// Retrieve the number of VkFormats/VkColorSpaceKHR pairs supported by the physical device surface.
	vulkanSuccessOrDie(_instanceVkFunctions.vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, nullptr), "Failed to retrieved physical device surface formats");
	// Retrieve the list of formatCount VkFormats/VkColorSpaceKHR pairs supported by the physical device surface.
	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	vulkanSuccessOrDie(_instanceVkFunctions.vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &formatCount, surfaceFormats.data()),
		"Failed to retrieved physical device surface formats");

	// From the list of retrieved VkFormats/VkColorSpaceKHR pairs supported by the physical device surface find one suitable from a list of preferred choices.
	VkSurfaceFormatKHR swapchainColorFormat = { VK_FORMAT_UNDEFINED, VkColorSpaceKHR(0) };
	const uint32_t numPreferredColorFormats = 2;
	VkFormat preferredColorFormats[numPreferredColorFormats] = { VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM };

	bool foundFormat = false;

	for (uint32_t i = 0; i < surfaceFormats.size() && !foundFormat; ++i)
	{
		for (uint32_t j = 0; j < numPreferredColorFormats; ++j)
		{
			if (surfaceFormats[i].format == preferredColorFormats[j])
			{
				swapchainColorFormat = surfaceFormats[i];
				foundFormat = true;
				break;
			}
		}
	}

	if (!foundFormat)
	{
		// No preference... Get the first one.
		if (surfaceFormats.size())
		{
			foundFormat = true;
			swapchainColorFormat = surfaceFormats[0];
		}
		else
		{
			throw pvr::PvrError("Failed to find a valid pvrvk::SurfaceFormatKHR to use for the swapchain");
		}
	}

	Log(LogLevel::Information, "Surface format selected: VkFormat with enumeration value %d", swapchainColorFormat.format);

	// Store the presentation image colour format as we will make use of it elsewhere.
	_swapchainColorFormat = swapchainColorFormat.format;

	// Get the surface capabilities from the surface and physical device.
	VkSurfaceCapabilitiesKHR surfaceCaps;
	vulkanSuccessOrDie(_instanceVkFunctions.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &surfaceCaps), "Failed to retrieve physical device surface capabilities");

	// Get a set of "corrected" extents (dimensions) for the surface window based on the pvr::Shell window width/height and surface capabilities.
	correctWindowExtents(surfaceCaps, this->getDisplayAttributes());

	// Retrieve the number of presentation modes supported by the physical device surface.
	uint32_t numPresentMode;
	vulkanSuccessOrDie(_instanceVkFunctions.vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &numPresentMode, nullptr), "Failed to retrieve presentation modes");
	if (numPresentMode == 0) { throw pvr::PvrError("This application requires more than one presentation mode"); }
	// Retrieve the numPresentMode presentation modes supported by the physical device surface.
	std::vector<VkPresentModeKHR> presentModes(numPresentMode);
	vulkanSuccessOrDie(
		_instanceVkFunctions.vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &numPresentMode, presentModes.data()), "Failed to retrieve presentation modes");

	// Retrieve the pvr::DisplayAttributes from the pvr::Shell - the pvr::Shell pvr::DisplayAttributes will take into account any command line arguments used.
	pvr::DisplayAttributes& displayAttributes = getDisplayAttributes();

	// Create the swapchain info which will be used to create our swapchain.
	VkSwapchainCreateInfoKHR swapChainInfo = {};

	// Based on the pvr::DisplayAttributes, and supported presentation modes select a supported presentation mode.
	selectPresentMode(displayAttributes, presentModes, swapChainInfo.presentMode, swapChainInfo.minImageCount);

	swapChainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapChainInfo.surface = _surface;
	swapChainInfo.imageFormat = _swapchainColorFormat;
	swapChainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	if ((surfaceCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) == 0)
	{ throw pvr::InvalidOperationError("Vulkan Surface does not support VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR transformation"); }
	swapChainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapChainInfo.clipped = VK_TRUE;
	swapChainInfo.imageExtent.width = displayAttributes.width;
	swapChainInfo.imageExtent.height = displayAttributes.height;
	swapChainInfo.imageArrayLayers = 1;
	swapChainInfo.imageColorSpace = swapchainColorFormat.colorSpace;
	swapChainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapChainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapChainInfo.queueFamilyIndexCount = 1;
	swapChainInfo.pQueueFamilyIndices = &_graphicsQueueFamilyIndex;

	// Create the swapchain.
	vulkanSuccessOrDie(_deviceVkFunctions.vkCreateSwapchainKHR(_device, &swapChainInfo, nullptr, &_swapchain), "Failed to create swapchain");

	// The application doesn't "create" swapchain images instead they are retrieved from the presentation engine. Even then the application must acquire (using
	// vkAcquireNextImageKHR) the use of a presentation image from the presentation engine prior to making use of it.

	// Retrieve the number of presentation images used by the presentation engine.
	vulkanSuccessOrDie(_deviceVkFunctions.vkGetSwapchainImagesKHR(_device, _swapchain, &_swapchainLength, nullptr), "Failed to get swapchain images");

	if (_swapchainLength <= 1 || _swapchainLength > 8) { throw pvr::PvrError("This application requires between 2 and 8 swapchain images"); }

	// Retrieve the _swapchainLength presentation images used by the presentation engine.
	vulkanSuccessOrDie(_deviceVkFunctions.vkGetSwapchainImagesKHR(_device, _swapchain, &_swapchainLength, &_swapchainImages[0]), "Failed to get swapchain images");

	// For each presentation image create a VkImageView for it so it can be used as a descriptor or as a framebuffer attachment
	{
		VkImageViewCreateInfo viewInfo = {};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.format = _swapchainColorFormat;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.components = VkComponentMapping{
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A,
		};

		viewInfo.subresourceRange.aspectMask = formatToImageAspect(_swapchainColorFormat);
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		for (uint32_t i = 0; i < _swapchainLength; ++i)
		{
			viewInfo.image = _swapchainImages[i];
			vulkanSuccessOrDie(_deviceVkFunctions.vkCreateImageView(_device, &viewInfo, nullptr, &_swapchainImageViews[i]), "Failed to create swapchain image view");
		}
	}
}

/// <summary>Creates a set of VkImages and VkImageViews which will be used as the depth/stencil buffers.</summary>
void VulkanIntroducingPVRShell::createDepthStencilImages()
{
	// Create swapchainLength VkImages and VkImageViews which the application will use as depth stencil images.

	// Setup an ordered list of preferred VkFormat to check for support when determining the format to use for the depth stencil images.
	VkFormat preferredDepthStencilFormats[6] = {
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D16_UNORM,
		VK_FORMAT_X8_D24_UNORM_PACK32,
	};

	std::vector<VkFormat> depthFormats;
	depthFormats.insert(depthFormats.begin(), &preferredDepthStencilFormats[0], &preferredDepthStencilFormats[6]);

	for (uint32_t i = 0; i < depthFormats.size(); ++i)
	{
		VkFormat currentDepthStencilFormat = depthFormats[i];

		// In turn check the physical device as to whether it supports the VkFormat in preferredDepthStencilFormats.
		VkFormatProperties formatProperties;
		_instanceVkFunctions.vkGetPhysicalDeviceFormatProperties(_physicalDevice, currentDepthStencilFormat, &formatProperties);

		// Ensure that the format supports VK_IMAGE_TILING_OPTIMAL. Optimal tiling specifies that texels are laid out in an implementation dependent arrangement providing more
		// optimal memory access.
		if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) != 0)
		{
			_depthStencilFormat = currentDepthStencilFormat;
			break;
		}
	}

	// the required memory property flags
	const VkMemoryPropertyFlags requiredMemoryProperties = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	// more optimal set of memory property flags
	const VkMemoryPropertyFlags optimalMemoryProperties = VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT;

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		VkImageCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		createInfo.flags = 0;
		createInfo.imageType = VK_IMAGE_TYPE_2D;
		createInfo.extent.width = getWidth();
		createInfo.extent.height = getHeight();
		createInfo.extent.depth = 1;
		createInfo.mipLevels = 1;
		createInfo.arrayLayers = 1;
		createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		createInfo.format = _depthStencilFormat;
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		// The use of VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT specifies that memory bound to this image will have been allocated with VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT (if
		// supported).
		createInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		createInfo.queueFamilyIndexCount = 1;
		createInfo.pQueueFamilyIndices = &_graphicsQueueFamilyIndex;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		// We use vkGetPhysicalDeviceImageFormatProperties to as a mechanism to validate the combination of parameters being used for creating the depth stencil images.
		VkImageFormatProperties imageFormatProperties = {};
		vulkanSuccessOrDie(_instanceVkFunctions.vkGetPhysicalDeviceImageFormatProperties(
							   _physicalDevice, createInfo.format, createInfo.imageType, createInfo.tiling, createInfo.usage, createInfo.flags, &imageFormatProperties),
			"The combination of parameters used is not supported by the implementation for use in vkCreateImage");

		// Create the depth stencil image (VkImage).
		vulkanSuccessOrDie(_deviceVkFunctions.vkCreateImage(_device, &createInfo, NULL, &_depthStencilImages[i]), "Failed to create depth stencil image");

		// Retrieve the required memory requirements for the depth stencil image.
		VkMemoryRequirements memoryRequirements = {};
		_deviceVkFunctions.vkGetImageMemoryRequirements(_device, _depthStencilImages[i], &memoryRequirements);

		// Allocate the device memory for the created image based on the arguments provided.
		VkMemoryPropertyFlags memoryProperties = 0;
		allocateDeviceMemory(
			_device, memoryRequirements, _physicalDeviceMemoryProperties, requiredMemoryProperties, optimalMemoryProperties, _depthStencilImageMemory[i], memoryProperties);

		// Finally attach the allocated device memory to the created image.
		vulkanSuccessOrDie(_deviceVkFunctions.vkBindImageMemory(_device, _depthStencilImages[i], _depthStencilImageMemory[i], 0), "Failed to bind a memory block to this image");

		// For each VkImage create a VkImageView so that we can use it as a descriptor or a framebuffer attachment.
		VkImageViewCreateInfo imageViewInfo = {};
		imageViewInfo.flags = 0;
		imageViewInfo.pNext = nullptr;
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewInfo.format = _depthStencilFormat;
		imageViewInfo.image = _depthStencilImages[i];
		imageViewInfo.subresourceRange.layerCount = 1;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.aspectMask = formatToImageAspect(_depthStencilFormat);
		imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;

		// Create the depth stencil VkImageView.
		vulkanSuccessOrDie(_deviceVkFunctions.vkCreateImageView(_device, &imageViewInfo, nullptr, &_depthStencilImageViews[i]), "Texture Image View Creation");
	}
}

/// <summary>Creates a Buffer, allocates its memory and attaches the memory to the newly created buffer.</summary>
/// <param name="size">The size of the buffer to create and the amount of memory to allocate.</param>
/// <param name="usageFlags">The intended buffer usage.</param>
/// <param name="requiredMemFlags">The set of flags specifying the required memory properties.</param>
/// <param name="optimalMemFlags">The set of flags specifying an optimal memory properties.</param>
/// <param name="outBuffer">The created buffer returned by reference.</param>
/// <param name="outMemory">The allocated memory returned by reference.</param>
/// <param name="outMemFlags">The set of flags supported by the memory used when allocating the memory.</param>
void VulkanIntroducingPVRShell::createBufferAndMemory(VkDeviceSize size, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags requiredMemFlags,
	VkMemoryPropertyFlags optimalMemFlags, VkBuffer& outBuffer, VkDeviceMemory& outMemory, VkMemoryPropertyFlags& outMemFlags)
{
	// Creates a buffer based on the size and usage flags specified.
	// Allocates device memory based on the specified memory property flags.
	// Attaches the allocated memory to the created buffer.

	// A buffer is simply a linear array of data used for various purposes including reading/writing to them using graphics/compute pipelines.

	// Creates the buffer with the specified size and which supports the specified usage.
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pQueueFamilyIndices = &_graphicsQueueFamilyIndex;
	bufferInfo.queueFamilyIndexCount = 1;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.size = size;
	bufferInfo.usage = usageFlags;
	vulkanSuccessOrDie(_deviceVkFunctions.vkCreateBuffer(_device, &bufferInfo, nullptr, &outBuffer), "Failed to create buffer");

	// In Vulkan all resources are initially created as what are termed virtual allocations and have no real physical backing memory.
	// To provide resources with memory backing device memory must be allocated separately and then associated with the relevant resource.
	// Various resources and resource types have differing memory requirements as to the type, size and alignment of the memory. For buffers querying for
	// the memory requirements is made using a call to vkGetBufferMemoryRequirements passing the created buffer as an argument.
	VkMemoryRequirements memRequirements;
	_deviceVkFunctions.vkGetBufferMemoryRequirements(_device, outBuffer, &memRequirements);

	// Allocate the device memory for the created buffer based on the arguments provided.
	allocateDeviceMemory(_device, memRequirements, _physicalDeviceMemoryProperties, requiredMemFlags, optimalMemFlags, outMemory, outMemFlags);

	// Finally attach the allocated device memory to the created buffer.
	vulkanSuccessOrDie(_deviceVkFunctions.vkBindBufferMemory(_device, outBuffer, outMemory, 0), "Failed to bind buffer memory");
}

/// <summary>Allocates device memory based on the provided arguments.</summary>
/// <param name="device">The device from which memory will be allocated.</param>
/// <param name="memoryRequirements">The requirements for the memory to allocate.</param>
/// <param name="physicalDeviceMemoryProperties">The physical device memory properties to use for determining a particular memory type index.</param>
/// <param name="requiredMemFlags">The required memory property flags for the memory to allocate.</param>
/// <param name="optimalMemFlags">An optimal set of memory property flags for the memory to allocate.</param>
/// <param name="outMemory">The allocated memory to return by reference.</param>
void VulkanIntroducingPVRShell::allocateDeviceMemory(VkDevice& device, VkMemoryRequirements& memoryRequirements, VkPhysicalDeviceMemoryProperties& physicalDeviceMemoryProperties,
	const VkMemoryPropertyFlags requiredMemFlags, const VkMemoryPropertyFlags optimalMemFlags, VkDeviceMemory& outMemory, VkMemoryPropertyFlags& outMemFlags)
{
	// Allocate the device memory based on the specified set of requirements.
	// Device memory is memory which is visible to the device i.e. the contents of buffers or images which devices can make use of.
	VkMemoryAllocateInfo memAllocateInfo = {};

	if (memoryRequirements.memoryTypeBits == 0) { throw pvr::PvrError("Allowed memory Bits must not be 0"); }

	memAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocateInfo.allocationSize = memoryRequirements.size;

	// Retrieve a suitable memory type index for the memory allocation.
	// Device memory will be allocated from a physical device from various memory heaps depending on the type of memory required.
	// Each memory heap may well expose a number of different memory types although allocations of different memory types from the same heap will make use of the same memory
	// resource consuming resources from the heap indicated by that memory type's heap index.
	getMemoryTypeIndex(physicalDeviceMemoryProperties, memoryRequirements.memoryTypeBits, requiredMemFlags, optimalMemFlags, memAllocateInfo.memoryTypeIndex, outMemFlags);
	if (memAllocateInfo.memoryTypeIndex == static_cast<uint32_t>(-1))
	{ throw pvr::PvrError("Could not get a Memory Type Index for the specified combination specified memory bits, properties and flags"); } // Allocate the device memory.
	vulkanSuccessOrDie(_deviceVkFunctions.vkAllocateMemory(device, &memAllocateInfo, nullptr, &outMemory), "Failed to allocate buffer memory");
}

/// <summary>Loads a spirv shader binary from memory and creates a shader module for it.</summary>
/// <param name="shaderName">The name of the spirv shader binary to load from memory.</param>
/// <param name="outShader">The shader module returned by reference.</param>
void VulkanIntroducingPVRShell::loadAndCreateShaderModules(const char* const shaderName, VkShaderModule& outShader)
{
	// Load the spirv shader binary from memory and create a shader module for it

	// Load the spirv shader binary from memory by creating a Stream object for it directly from the file system or from a platform specific store such as
	// Windows resources, Android .apk assets etc.
	// A shader itself specifies the programmable operations executed for a particular type of task - vertex, control point, tessellated vertex, primitive, fragment or compute
	// work-group
	std::unique_ptr<pvr::Stream> stream = getAssetStream(shaderName);
	assertion(stream.get() != nullptr && "Invalid Shader source");
	std::vector<uint32_t> readData(stream->getSize());
	size_t dataReadSize;
	stream->read(stream->getSize(), 1, readData.data(), dataReadSize);

	// Create the shader module using the asset stream.
	// A shader module contains the actual shader code to be executed as well as one or more entry points. A shader module can encapsulate multiple shaders with the shader chosen
	// via the use of an entry point as part of any pipeline creation making use of the shader module. The shader code making up the shader module is provided in the spirv format.
	VkShaderModuleCreateInfo shaderInfo = {};
	shaderInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderInfo.codeSize = stream->getSize();
	shaderInfo.pCode = readData.data();
	vulkanSuccessOrDie(_deviceVkFunctions.vkCreateShaderModule(_device, &shaderInfo, nullptr, &outShader), "Failed to create the VkShaderModule");
}

/// <summary>Allocates the command buffers used by the application. The number of command buffers allocated is equal to the swapchain length.</summary>
void VulkanIntroducingPVRShell::allocateCommandBuffers()
{
	// Command buffers are used to control the submission of various Vulkan commands to a set of devices via their queues.

	// A command buffer is initially created empty and must be recorded to. Once recorded a command buffer can be submitted once or many times to a queue for execution.

	VkCommandBufferAllocateInfo sAllocateInfo = {};
	sAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	sAllocateInfo.commandPool = _commandPool;
	// The command buffer being allocated here is a primary command buffer and can be directly submitted to a queue.
	sAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	sAllocateInfo.commandBufferCount = _swapchainLength;

	// Allocate _swapchainLength command buffers
	vulkanSuccessOrDie(_deviceVkFunctions.vkAllocateCommandBuffers(_device, &sAllocateInfo, &_cmdBuffers[0]), "Failed to allocate command buffers");
}

/// <summary>Records the rendering commands into a set of command buffers which can be subsequently submitted to a queue for execution.</summary>
void VulkanIntroducingPVRShell::recordCommandBuffers()
{
	// Record the rendering commands into a set of command buffers upfront (once). These command buffers can then be submitted to a device queue for execution resulting in fewer
	// state changes and less commands being dispatched to the implementation all resulting in less driver overhead.

	// Recorded commands will include pipelines to use and their descriptor sets, dynamic state modification commands, rendering commands (draws), compute commands (dispatches),
	// commands for executing secondary command buffers or commands to copy resources

	// Vulkan does not provide any kind of global state machine, neither does it provide any kind of default states this means that each command buffer manages its own state
	// independently of all other command buffers and each command buffer must independently configure all of the state relevant to its own set of commands.

	// Setup the VkCommandBufferBeginInfo
	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	// Specify the clear values used by the RenderPass for clearing the specified framebuffer attachments
	VkClearValue clearVals[2] = { { 0 } };
	clearVals[0].color.float32[0] = 0.00f;
	clearVals[0].color.float32[1] = 0.70f;
	clearVals[0].color.float32[2] = .67f;
	clearVals[0].color.float32[3] = 1.0f;
	clearVals[1].depthStencil.depth = 1.0f;
	clearVals[1].depthStencil.stencil = 0xFF;

	// Setup the VkRenderPassBeginInfo which specifies the renderpass to begin an instance of. The VkRenderPassBeginInfo structure also specifies the framebuffer the renderpass
	// instance will make use of. The renderable area affected by the renderpass instance may also be configured in addition to an array of VkClearValue structures specifying
	// clear values for each attachment of the framebuffer.
	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.renderPass = _renderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = getWidth();
	renderPassBeginInfo.renderArea.extent.height = getHeight();
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = &clearVals[0];
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		// Commands may only be recorded once the command buffer is in the recording state.
		vulkanSuccessOrDie(_deviceVkFunctions.vkBeginCommandBuffer(_cmdBuffers[i], &cmdBufferBeginInfo), "Failed to begin the command buffer");

		// Specify the per swapchain framebuffer to use as part of the VkRenderPassBeginInfo
		renderPassBeginInfo.framebuffer = _framebuffers[i];

		// Initiates the start of a renderPass.
		// From this point until either vkCmdNextSubpass or vkCmdEndRenderPass is called commands will be recorded for the first subpass of the specified renderPass.
		_deviceVkFunctions.vkCmdBeginRenderPass(_cmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Setup a list of descriptor sets which will be used for subsequent pipelines.
		VkDescriptorSet descriptorSets[] = { _staticDescriptorSet, _dynamicDescriptorSet };

		// Calculate the dynamic offset to use per swapchain controlling the "slice" of a buffer to be used for the current swapchain.
		uint32_t dynamicOffset = static_cast<uint32_t>(_dynamicBufferAlignedSize * i);

		// Bind the list of descriptor sets using the dynamic offset.
		_deviceVkFunctions.vkCmdBindDescriptorSets(_cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _pipelineLayout, 0, 2, descriptorSets, 1, &dynamicOffset);

		// Bind the graphics pipeline through which commands will be funnelled.
		_deviceVkFunctions.vkCmdBindPipeline(_cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

		// Bind the vertex buffer used for sourcing the triangle vertices
		VkDeviceSize vertexOffset = 0;
		_deviceVkFunctions.vkCmdBindVertexBuffers(_cmdBuffers[i], 0, 1, &_vbo, &vertexOffset);

		// Record a non-indexed draw command specifying the number of vertices
		_deviceVkFunctions.vkCmdDraw(_cmdBuffers[i], 3, 1, 0, 0);

		// Ends the current renderPass instance.
		_deviceVkFunctions.vkCmdEndRenderPass(_cmdBuffers[i]);

		// Ends the recording for the specified command buffer
		vulkanSuccessOrDie(_deviceVkFunctions.vkEndCommandBuffer(_cmdBuffers[i]), "Failed to end the command buffer");
	}
}

/// <summary>Create the pipeline cache used throughout the demo.</summary>
void VulkanIntroducingPVRShell::createPipelineCache()
{
	// Create the pipeline cache objects used throughout the demo.

	// Pipeline caches provide a convenient mechanism for the result of pipeline creation to be reused between pipelines and between runs of an application.
	// The use of a pipeline cache isn't strictly necessary and won't provide us with any benefits in this application due to the use of only a single pipeline in reuse only
	// a single run of the application however their use is recommended and so their use has been included in this demo for demonstrative purposes only - Using them
	// is definitely a best practice.

	// Our application only makes use of pipeline caches between pipelines in the same run of the application and we make no effort to save and load
	// the pipeline caches from disk which would potentially enable optimisations across different runs of the same application.

	// Once created, a pipeline cache can be conveniently passed to the Vulkan commands vkCreateGraphicsPipelines and vkCreateComputePipelines.
	// If the pipeline cache passed into these commands is not VK_NULL_HANDLE, the implementation will query it for possible reuse opportunities and update it with new content.
	// The implementation handles updates to the pipeline cache and the application only needs to make use of the pipeline cache across all pipeline creation calls to achieve the
	// most possible gains.

	// It's heavily recommended to make use of pipeline caches as much as possible as they provide little to no overhead and provide the opportunity for
	// the implementation to provide optimisations for us. From the point of view of the application they provide an easy win in terms of work/benefit.

	VkPipelineCacheCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.initialDataSize = 0;
	createInfo.pInitialData = nullptr;

	vulkanSuccessOrDie(_deviceVkFunctions.vkCreatePipelineCache(_device, &createInfo, nullptr, &_pipelineCache), "Could not create pipeline cache");
}

/// <summary>Create the VkShaderModule(s) used in the demo.</summary>
void VulkanIntroducingPVRShell::createShaderModules()
{
	// Creates the VkShaderModule(s) used by the demo.

	// These shader modules contain shader code and entry points used by the graphics pipeline for rendering a textured triangle to the screen.
	// Note that the shader modules have been pre-compiled to SPIR-V format using the "recompile script" included alongside the demo (recompile.sh/recompile.bat).
	// Note that when creating our graphics pipeline the specific shader to use from the shader module is specified using an entry point.
	loadAndCreateShaderModules(VertShaderName, _vertexShaderModule);
	loadAndCreateShaderModules(FragShaderName, _fragmentShaderModule);
}

/// <summary>Create the Pipeline Layout used in the demo.</summary>
void VulkanIntroducingPVRShell::createPipelineLayout()
{
	// Create the pipeline layout used throughout the demo.

	// A pipeline layout describes the full set of resources which may be accessed by a pipeline making use of the pipeline layout.
	// A pipeline layout sets out a contract between a set of resources, each of which has a particular layout, and a pipeline.

	// Create a list of descriptor set layouts which are going to be used to create the pipeline layout.
	VkDescriptorSetLayout descriptorSetLayouts[] = { _staticDescriptorSetLayout, _dynamicDescriptorSetLayout };

	// Create the pipeline layout creation info for generating the pipeline layout using the list of descriptor set layouts specified above and 0 push constant ranges.
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 2;
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = 0;

	// Create the pipeline layout.
	vulkanSuccessOrDie(_deviceVkFunctions.vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout), "Pipeline Layout Creation");
}

/// <summary>Creates the graphics pipeline used in the demo.</summary>
void VulkanIntroducingPVRShell::createPipeline()
{
	// Create the graphics pipeline used throughout the demo for rendering the triangle.

	// A pipeline effectively sets up and configures a processing pipeline of a particular type (VkPipelineBindPoint) which becomes the funnel for which certain sets of Vulkan
	// commands are sent through.

	// The pipeline used throughout this demo is fundamentally simple in nature but still illustrates how to make use of a graphics pipeline to render a geometric object even if it
	// is only a triangle. The pipeline makes use of vertex attributes (position, normal and UV), samples a particular texture writing the result in to a colour attachment and also
	// rendering to a depth stencil attachments.
	// Pipelines are monolithic objects taking account of various bits of state which allow for a great deal of optimization of shaders based on the pipeline description including
	// shader inputs/outputs and fixed function stages.

	// The first part of a graphics pipeline will assemble a set of vertices to form geometric objects based on the requested primitive topology. These vertices may then
	// be transformed using a Vertex Shader computing their position and generating attributes for each of the vertices. The VkPipelineVertexInputStateCreateInfo and
	// VkPipelineInputAssemblyStateCreateInfo structures will control how these vertices are assembled.

	// The VkVertexInputBindingDescription structure specifies the way in which vertex attributes are taken from buffers.
	VkVertexInputBindingDescription vertexInputBindingDescription = {};
	vertexInputBindingDescription.binding = 0;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexInputBindingDescription.stride = _vboStride;

	// The VkVertexInputAttributeDescription structure specifies the structure of a particular vertex attribute (position, normal, UVs etc.).
	VkVertexInputAttributeDescription vertexInputAttributeDescription[2];
	vertexInputAttributeDescription[0].binding = 0;
	vertexInputAttributeDescription[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexInputAttributeDescription[0].location = 0;
	vertexInputAttributeDescription[0].offset = 0;

	vertexInputAttributeDescription[1].binding = 0;
	vertexInputAttributeDescription[1].format = VK_FORMAT_R32G32_SFLOAT;
	vertexInputAttributeDescription[1].location = 1;
	vertexInputAttributeDescription[1].offset = sizeof(glm::vec4);

	// The VkPipelineVertexInputStateCreateInfo structure specifies a set of descriptions for the vertex attributes and vertex bindings.
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = 2;
	vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributeDescription;

	// The VkPipelineInputAssemblyStateCreateInfo structure specifies how primitives are assembled.
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.flags = 0;
	inputAssemblyInfo.pNext = nullptr;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	// The resulting primitives are clipped and sent to the next pipeline stage...

	// The next stage of the graphics pipeline, Rasterization, produces fragments based on the points, line segments or triangles constructed in the first stage.
	// Each of the generated fragments will be passed to the fragment shader carrying out the per fragment rendering - this is where the framebuffer operations occur. This stage
	// includes blending, masking, stencilling and other logical operations.

	// The VkPipelineRasterizationStateCreateInfo structure specifies how various aspects of rasterization occur including cull mode.
	VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
	rasterizationInfo.pNext = nullptr;
	rasterizationInfo.flags = 0;
	rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationInfo.lineWidth = 1.0f;
	rasterizationInfo.depthBiasClamp = 0.0f;
	rasterizationInfo.depthBiasConstantFactor = 0.0f;
	rasterizationInfo.depthBiasEnable = VK_FALSE;
	rasterizationInfo.depthBiasSlopeFactor = 0.0f;

	// The VkPipelineColorBlendAttachmentState structure specifies blending state for a particular colour attachment.
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

	// The VkPipelineColorBlendStateCreateInfo structure controls the per attachment blending.
	VkPipelineColorBlendStateCreateInfo colorBlendInfo = {};
	colorBlendInfo.flags = 0;
	colorBlendInfo.pNext = nullptr;
	colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendInfo.logicOpEnable = VK_FALSE;
	colorBlendInfo.attachmentCount = 1;
	colorBlendInfo.pAttachments = &colorBlendAttachment;
	colorBlendInfo.blendConstants[0] = 0.0f;
	colorBlendInfo.blendConstants[1] = 0.0f;
	colorBlendInfo.blendConstants[2] = 0.0f;
	colorBlendInfo.blendConstants[3] = 0.0f;

	// The VkPipelineMultisampleStateCreateInfo structure specifies the multisampling state used if any.
	VkPipelineMultisampleStateCreateInfo multisamplingInfo = {};
	multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingInfo.pNext = nullptr;
	multisamplingInfo.flags = 0;
	multisamplingInfo.pSampleMask = nullptr;
	multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisamplingInfo.sampleShadingEnable = VK_FALSE;
	multisamplingInfo.alphaToCoverageEnable = VK_FALSE;
	multisamplingInfo.alphaToOneEnable = VK_FALSE;
	multisamplingInfo.minSampleShading = 0.0f;

	// The VkPipelineViewportStateCreateInfo structure specifies the viewport and scissor regions used by the pipeline.
	VkPipelineViewportStateCreateInfo viewportInfo = {};
	viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo.pNext = nullptr;
	viewportInfo.flags = 0;
	viewportInfo.viewportCount = 1;
	viewportInfo.pViewports = &_viewport;
	viewportInfo.scissorCount = 1;
	viewportInfo.pScissors = &_scissor;

	// The VkPipelineDepthStencilStateCreateInfo structure specifies how depth bounds tests, stencil tests and depth tests are performed by the graphics pipeline.
	VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo = {};
	depthStencilCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilCreateInfo.pNext = nullptr;
	depthStencilCreateInfo.flags = 0;
	depthStencilCreateInfo.depthTestEnable = true;
	depthStencilCreateInfo.depthWriteEnable = true;
	depthStencilCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilCreateInfo.depthBoundsTestEnable = false;
	depthStencilCreateInfo.stencilTestEnable = false;
	depthStencilCreateInfo.minDepthBounds = 0.0f;
	depthStencilCreateInfo.maxDepthBounds = 1.0f;

	// The VkPipelineShaderStageCreateInfo structure specifies the creation of a particular stage of a graphics pipeline including vertex/fragment/tessellation {control/evaluation}
	// taking the shader to be used from a particular VkShaderModule.
	VkPipelineShaderStageCreateInfo shaderStageCreateInfos[2];

	{
		shaderStageCreateInfos[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageCreateInfos[0].flags = 0;
		shaderStageCreateInfos[0].pName = "main";
		shaderStageCreateInfos[0].pNext = nullptr;
		shaderStageCreateInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStageCreateInfos[0].pSpecializationInfo = nullptr;
		shaderStageCreateInfos[0].module = _vertexShaderModule;
	}

	{
		shaderStageCreateInfos[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStageCreateInfos[1].flags = 0;
		shaderStageCreateInfos[1].pName = "main";
		shaderStageCreateInfos[1].pNext = nullptr;
		shaderStageCreateInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStageCreateInfos[1].pSpecializationInfo = nullptr;
		shaderStageCreateInfos[1].module = _fragmentShaderModule;
	}

	// Create the graphics pipeline adding all the individual info structs.
	VkGraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;
	pipelineInfo.layout = _pipelineLayout;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = 0;
	pipelineInfo.flags = 0;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineInfo.pRasterizationState = &rasterizationInfo;
	pipelineInfo.pColorBlendState = &colorBlendInfo;
	pipelineInfo.pTessellationState = nullptr;
	pipelineInfo.pMultisampleState = &multisamplingInfo;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.pViewportState = &viewportInfo;
	pipelineInfo.pDepthStencilState = &depthStencilCreateInfo;
	pipelineInfo.pStages = shaderStageCreateInfos;
	pipelineInfo.stageCount = 2;

	// Framebuffer operations occur for a particular subpass of a given renderpass - in our case we use the first (and only) subpass of our renderpass.
	pipelineInfo.renderPass = _renderPass;
	pipelineInfo.subpass = 0;

	// Create the graphics pipeline we'll use for rendering a triangle.
	vulkanSuccessOrDie(_deviceVkFunctions.vkCreateGraphicsPipelines(_device, _pipelineCache, 1, &pipelineInfo, NULL, &_graphicsPipeline), "Pipeline Creation");
}

/// <summary>Initializes the vertex buffer objects used in the demo.</summary>
void VulkanIntroducingPVRShell::createVbo()
{
	// Creates the Vertex Buffer Object (vbo) and allocates its memory. This vbo is used for rendering a textured triangle to the screen.

	// Specifies the size of a particular Triangle vertex used inside the vbo.
	struct TriangleVertex
	{
		glm::vec4 vertex;
		float uv[2];
	};

	_vboStride = sizeof(TriangleVertex);
	// Calculate the size of the vbo taking into account multiple vertices.
	const uint32_t vboSize = _vboStride * 3;

	// The use of VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT specifies that memory allocated with this memory property type is the most efficient for device access.
	// Note that memory property flag VK_MEMORY_PROPERTY_HOST_COHERENT_BIT has not been specified meaning the host application must manage the memory accesses to this memory
	// explicitly using the host cache management commands vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges to flush host writes to the device or make device writes
	// visible to the host respectively.
	VkMemoryPropertyFlags requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	VkMemoryPropertyFlags optimalFlags = requiredFlags | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	createBufferAndMemory(vboSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, requiredFlags, optimalFlags, _vbo, _vboMemory, _vboMemoryFlags);

	// Construct the triangle vertices.
	TriangleVertex triangle[3];
	triangle[0] = { glm::vec4(0.5f, -0.288f, 0.0f, 1.0f), { 1.f, 0.f } };
	triangle[1] = { glm::vec4(-0.5f, -0.288f, 0.0f, 1.0f), { 0.f, 0.f } };
	triangle[2] = { glm::vec4(0.0f, 0.577f, 0.0f, 1.0f), { .5f, 1.f } };

	// The use of VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT indicates that memory allocated with this memory property type can be mapped and unmapped enabling host
	// access using calls to vkMapMemory and vkUnmapMemory respectively. When this memory property type is used we are able to map/update/unlap the memory to update the contents of
	// the memory.
	if ((_vboMemoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) != 0)
	{
		// Memory created using vkAllocateMemory isn't directly accessible to the host and instead must be mapped manually.
		// Note that only memory created with the memory property flag VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT can be mapped.
		// vkMapMemory retrieves a host virtual address pointer to a region of a mappable memory object.
		void* mapped = 0;
		vulkanSuccessOrDie(_deviceVkFunctions.vkMapMemory(_device, _vboMemory, 0, vboSize, 0, &mapped), "Failed to map the memory");

		memcpy(mapped, triangle, sizeof(triangle));

		// If the memory property flags for the allocated memory included the use of VK_MEMORY_PROPERTY_HOST_COHERENT_BIT then the host does not need to manage the
		// memory accesses explicitly using the host cache management commands vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges to flush host writes to
		// the device or make device writes visible to the host respectively. This behaviour is handled by the implementation.
		if ((_vboMemoryFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
		{
			// Flush the memory guaranteeing that host writes to the memory ranges specified are made available to the device.
			VkMappedMemoryRange memoryRange = {};
			memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			memoryRange.memory = _vboMemory;
			memoryRange.offset = 0;
			memoryRange.size = vboSize;
			_deviceVkFunctions.vkFlushMappedMemoryRanges(_device, 1, &memoryRange);
		}

		// Note that simply unmapping non-coherent memory doesn't implicitly flush the mapped memory.
		_deviceVkFunctions.vkUnmapMemory(_device, _vboMemory);
	}
	else
	{
		// We use our buffer creation function to generate a staging buffer. We pass the VK_BUFFER_USAGE_TRANSFER_SRC_BIT flag to specify its use.
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;

		// The use of VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT indicates that memory allocated with this memory property type can be mapped and unmapped enabling host
		// access using calls to vkMapMemory and vkUnmapMemory respectively. When this memory property type is used we are able to map/update/unmap the memory to update the
		// contents of the memory.
		VkMemoryPropertyFlags requiredMemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		VkMemoryPropertyFlags optimalMemoryFlags = requiredMemoryFlags;
		VkMemoryPropertyFlags stagingBufferMemoryFlags = 0;
		createBufferAndMemory(vboSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, requiredMemoryFlags, optimalMemoryFlags, stagingBuffer, stagingBufferMemory, stagingBufferMemoryFlags);

		//
		// Map the staging buffer and copy the triangle vbo data into it.
		//

		{
			void* mapped = nullptr;
			vulkanSuccessOrDie(_deviceVkFunctions.vkMapMemory(_device, stagingBufferMemory, 0, vboSize, VkMemoryMapFlags(0), &mapped), "Could not map the staging buffer.");
			memcpy(mapped, triangle, sizeof(triangle));

			// If the memory property flags for the allocated memory included the use of VK_MEMORY_PROPERTY_HOST_COHERENT_BIT then the host does not need to manage the
			// memory accesses explicitly using the host cache management commands vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges to flush host writes to
			// the device or make device writes visible to the host respectively. This behaviour is handled by the implementation.
			if ((stagingBufferMemoryFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			{
				// flush the memory
				VkMappedMemoryRange memoryRange = {};
				memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				memoryRange.memory = stagingBufferMemory;
				memoryRange.offset = 0;
				memoryRange.size = sizeof(triangle);
				_deviceVkFunctions.vkFlushMappedMemoryRanges(_device, 1, &memoryRange);
			}

			_deviceVkFunctions.vkUnmapMemory(_device, stagingBufferMemory);
		}

		// We create a command buffer to execute the copy operation from our command pool.
		VkCommandBuffer cmdBuffers;
		VkCommandBufferAllocateInfo commandAllocateInfo = {};
		commandAllocateInfo.commandPool = _commandPool;
		commandAllocateInfo.pNext = nullptr;
		commandAllocateInfo.commandBufferCount = 1;
		commandAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

		// We Allocate the command buffer from the command pool's memory.
		vulkanSuccessOrDie(_deviceVkFunctions.vkAllocateCommandBuffers(_device, &commandAllocateInfo, &cmdBuffers), "Allocate Command Buffers");

		// We start recording our command buffer operation
		VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufferBeginInfo.flags = 0;
		cmdBufferBeginInfo.pNext = nullptr;
		cmdBufferBeginInfo.pInheritanceInfo = nullptr;

		vulkanSuccessOrDie(_deviceVkFunctions.vkBeginCommandBuffer(cmdBuffers, &cmdBufferBeginInfo), "Begin Staging Buffer Copy to Buffer Command Buffer Recording");

		VkBufferCopy bufferCopy = {};
		bufferCopy.srcOffset = 0;
		bufferCopy.dstOffset = 0;
		bufferCopy.size = sizeof(triangle);

		_deviceVkFunctions.vkCmdCopyBuffer(cmdBuffers, stagingBuffer, _vbo, 1, &bufferCopy);

		// We end the recording of our command buffer.
		vulkanSuccessOrDie(_deviceVkFunctions.vkEndCommandBuffer(cmdBuffers), "End Staging Buffer Copy to Vbo Command Buffer Recording");

		// We create a fence to make sure that the command buffer is synchronised correctly.
		VkFence copyFence;
		VkFenceCreateInfo copyFenceInfo = {};
		copyFenceInfo.flags = 0;
		copyFenceInfo.pNext = 0;
		copyFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

		// We create the fence.
		vulkanSuccessOrDie(_deviceVkFunctions.vkCreateFence(_device, &copyFenceInfo, nullptr, &copyFence), "Copy Staging Buffer to Vbo Fence Creation");

		// Submit the command buffer to the queue specified
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.pWaitDstStageMask = nullptr;
		submitInfo.waitSemaphoreCount = 0;
		submitInfo.pWaitSemaphores = nullptr;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffers;

		vulkanSuccessOrDie(_deviceVkFunctions.vkQueueSubmit(_queue, 1, &submitInfo, copyFence), "Submit Staging Buffer to Vbo Copy Command Buffer");

		// Wait for the specified fence to be signalled which ensures that the command buffer has finished executing.
		vulkanSuccessOrDie(_deviceVkFunctions.vkWaitForFences(_device, 1, &copyFence, true, static_cast<uint64_t>(-1)), "Staging Buffer Copy to Buffer Fence Signal");

		// We clean up all the temporary data we created for this operation.
		_deviceVkFunctions.vkDestroyFence(_device, copyFence, nullptr);
		_deviceVkFunctions.vkFreeCommandBuffers(_device, _commandPool, 1, &cmdBuffers);
		_deviceVkFunctions.vkFreeMemory(_device, stagingBufferMemory, nullptr);
		_deviceVkFunctions.vkDestroyBuffer(_device, stagingBuffer, nullptr);
	}
}

/// <summary>Gets the minimum aligned data size based on the size of the data to align and the minimum alignment size specified.</summary>
/// <param name="dataSize">The size of the data to align based on the minimum alignment.</param>
/// <param name="minimumAlignment">The minimum data size alignment supported.</param>
/// <returns>The minimum aligned data size.</returns>
size_t getAlignedDataSize(size_t dataSize, size_t minimumAlignment)
{
	return (dataSize / minimumAlignment) * minimumAlignment + ((dataSize % minimumAlignment) > 0 ? minimumAlignment : 0);
}

/// <summary>Create the uniform buffers used throughout the demo.</summary>
void VulkanIntroducingPVRShell::createUniformBuffers()
{
	// Vulkan requires that when updating a descriptor of type VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER or VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC the
	// offset specified is an integer multiple of the minimum required alignment in bytes for the physical device - as must any dynamic alignments used.
	size_t minimumUboAlignment = static_cast<size_t>(_physicalDeviceProperties.limits.minUniformBufferOffsetAlignment);

	// The dynamic buffers will be used as uniform buffers (later used as a descriptor of type VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC and VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER).
	VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	// The use of VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT indicates that memory allocated with this memory property type can be mapped and unmapped enabling host
	// access using calls to vkMapMemory and vkUnmapMemory respectively. The memory property flag VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT is guaranteed to be available
	// The use of VK_MEMORY_PROPERTY_HOST_COHERENT_BIT indicates the host does not need to manage the memory accesses explicitly using the host cache management commands
	// vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges to flush host writes to the device or make device writes visible to the host respectively. This behaviour
	// is handled by the implementation.
	VkMemoryPropertyFlags requiredPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	VkMemoryPropertyFlags optimalPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	{
		// Using the minimum uniform buffer offset alignment we calculate the minimum buffer slice size based on the size of the intended data or more specifically
		// the size of the smallest chunk of data which may be mapped or updated as a whole.
		size_t bufferDataSize = sizeof(glm::mat4);
		_dynamicBufferAlignedSize = static_cast<uint32_t>(getAlignedDataSize(bufferDataSize, minimumUboAlignment));

		// Calculate the size of the dynamic uniform buffer.
		// This buffer will be updated each frame and must therefore be multi-buffered to avoid issues with using partially updated data, or updating data already in used.
		// Rather than allocating multiple (swapchain) buffers we instead allocate a larger buffer and will instead use a slice per swapchain. This works as longer as the buffer
		// is created taking into account the minimum uniform buffer offset alignment.
		_modelViewProjectionBufferSize = _swapchainLength * _dynamicBufferAlignedSize;

		// Create the buffer, allocate the device memory and attach the memory to the newly created buffer object.
		createBufferAndMemory(_modelViewProjectionBufferSize, usageFlags, requiredPropertyFlags, optimalPropertyFlags, _modelViewProjectionBuffer, _modelViewProjectionMemory,
			_modelViewProjectionMemoryFlags);

		// Memory created using vkAllocateMemory isn't directly accessible to the -host and instead must be mapped manually.
		// Note that only memory created with the memory property flag VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT can be mapped.
		// vkMapMemory retrieves a host virtual address pointer to a region of a mappable memory object.
		vulkanSuccessOrDie(_deviceVkFunctions.vkMapMemory(_device, _modelViewProjectionMemory, 0, _modelViewProjectionBufferSize, VkMemoryMapFlags(0), &_modelViewProjectionMappedMemory),
			"Could not map the uniform buffer.");
	}
}

/// <summary>Generates a simple checker board texture.</summary>
void VulkanIntroducingPVRShell::generateTexture()
{
	// Generates a simple checkered texture which will be applied and used as a texture for the triangle we are going to render and rotate on screen.
	for (uint32_t x = 0; x < _textureDimensions.width; ++x)
	{
		for (uint32_t y = 0; y < _textureDimensions.height; ++y)
		{
			float g = 0.3f;
			if (x % 128 < 64 && y % 128 < 64) { g = 1; }
			if (x % 128 >= 64 && y % 128 >= 64) { g = 1; }

			uint8_t* pixel = _textureData.data() + (x * _textureDimensions.height * 4) + (y * 4);
			pixel[0] = static_cast<uint8_t>(100 * g);
			pixel[1] = static_cast<uint8_t>(80 * g);
			pixel[2] = static_cast<uint8_t>(70 * g);
			pixel[3] = 255;
		}
	}
}

/// <summary>Allocate the descriptor sets used throughout the demo.</summary>
void VulkanIntroducingPVRShell::allocateDescriptorSets()
{
	// Allocate the descriptor sets from the pool of descriptors.
	// Each descriptor set follows the layout specified by a predefined descriptor set layout.

	// Construct a list of descriptor set layouts from which to base the allocated descriptor sets.
	VkDescriptorSetLayout descriptorSetLayouts[2] = { _staticDescriptorSetLayout, _dynamicDescriptorSetLayout };

	// Create the descriptor set allocation info struct used to allocate the descriptors from the descriptor pool.
	VkDescriptorSetAllocateInfo descriptorAllocateInfo = {};
	descriptorAllocateInfo.descriptorPool = _descriptorPool;
	descriptorAllocateInfo.descriptorSetCount = 2;
	descriptorAllocateInfo.pNext = nullptr;
	descriptorAllocateInfo.pSetLayouts = descriptorSetLayouts;
	descriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

	// Construct a list of VkDescriptorSets to be used as the target for the resulting descriptor set allocation handles.
	std::vector<VkDescriptorSet*> descriptorSets;
	descriptorSets.push_back(&_staticDescriptorSet);
	descriptorSets.push_back(&_dynamicDescriptorSet);

	// Allocate the descriptor sets
	vulkanSuccessOrDie(_deviceVkFunctions.vkAllocateDescriptorSets(_device, &descriptorAllocateInfo, descriptorSets.data()[0]), "Descriptor Set Creation");

	// Note that at this point the descriptor sets are largely uninitialized and all the descriptors are undefined although
	// the descriptor sets can still be bound to command buffers without issues.

	// In our case we will update the descriptor sets immediately using descriptor set write operations.

	VkWriteDescriptorSet descriptorSetWrites[2];

	VkDescriptorImageInfo descriptorImageInfo = {};
	descriptorImageInfo.sampler = _bilinearSampler;
	descriptorImageInfo.imageView = _triangleImageView;
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	// Fill the VkWriteDescriptorSet structures relating to the descriptors to use in the static descriptor set
	{
		descriptorSetWrites[0] = {};
		descriptorSetWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorSetWrites[0].pNext = nullptr;
		descriptorSetWrites[0].dstSet = _staticDescriptorSet;
		descriptorSetWrites[0].descriptorCount = 1;
		descriptorSetWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorSetWrites[0].pImageInfo = &descriptorImageInfo;
		descriptorSetWrites[0].dstArrayElement = 0;
		descriptorSetWrites[0].dstBinding = 0;
		descriptorSetWrites[0].pBufferInfo = nullptr;
		descriptorSetWrites[0].pTexelBufferView = nullptr;
	}

	VkDescriptorBufferInfo dynamicBufferInfo = {};
	dynamicBufferInfo.buffer = _modelViewProjectionBuffer;
	dynamicBufferInfo.offset = 0;
	dynamicBufferInfo.range = _dynamicBufferAlignedSize;

	// Fill the VkWriteDescriptorSet structures relating to the descriptors to use in the dynamic descriptor set
	{
		// Check the physical device limit specifying the maximum number of descriptor sets using dynamic buffers.
		if (_physicalDeviceProperties.limits.maxDescriptorSetUniformBuffersDynamic < 1)
		{ throw pvr::PvrError("The physical device must support at least 1 dynamic uniform buffer"); }

		// We use this info struct to define the info we'll be using to write the actual data to the descriptor set we created (we take info from uniform buffer).
		descriptorSetWrites[1] = {};
		descriptorSetWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorSetWrites[1].pNext = nullptr;
		descriptorSetWrites[1].dstSet = _dynamicDescriptorSet;
		descriptorSetWrites[1].descriptorCount = 1;
		descriptorSetWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		descriptorSetWrites[1].pBufferInfo = &dynamicBufferInfo; // pass Uniform buffer to this function
		descriptorSetWrites[1].dstArrayElement = 0;
		descriptorSetWrites[1].dstBinding = 0;
	}

	// Write the descriptors to the descriptor sets
	_deviceVkFunctions.vkUpdateDescriptorSets(_device, 2, &descriptorSetWrites[0], 0, NULL);
}

/// <summary>Creates the descriptor set layouts used throughout the demo.</summary>
void VulkanIntroducingPVRShell::createDescriptorSetLayouts()
{
	// Create the descriptor set layouts used throughout the demo with a descriptor set layout being defined by an array of 0 or more descriptor set layout bindings.
	// Each descriptor set layout binding corresponds to a type of descriptor, its shader bindings, a set of shader stages which may access the descriptor and a array size count.
	// A descriptor set layout provides an interface for the resources used by the descriptor set and the interface between shader stages and shader resources.

	// Create the descriptor set layout for the static resources.
	{
		// Create an array of descriptor set layout bindings.
		VkDescriptorSetLayoutBinding descriptorLayoutBinding[1];
		descriptorLayoutBinding[0].descriptorCount = 1;
		descriptorLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorLayoutBinding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptorLayoutBinding[0].binding = 0;
		descriptorLayoutBinding[0].pImmutableSamplers = nullptr;

		// Create the descriptor set layout using the array of VkDescriptorSetLayoutBindings.
		VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
		descriptorLayoutInfo.flags = 0;
		descriptorLayoutInfo.pNext = nullptr;
		descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayoutInfo.bindingCount = 1;
		descriptorLayoutInfo.pBindings = descriptorLayoutBinding;
		vulkanSuccessOrDie(_deviceVkFunctions.vkCreateDescriptorSetLayout(_device, &descriptorLayoutInfo, NULL, &_staticDescriptorSetLayout), "Descriptor Set Layout Creation");
	}

	// Create the descriptor set layout for the dynamic resources.
	// Note that we use a descriptor of type VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC with dynamic offsets provided by swapchain. We could also have achieved the same result
	// using multiple descriptor sets each referencing the per swapchain slice of the same (dynamic) buffer without using a dynamic descriptor with dynamic offsets.
	{
		// Create an array of descriptor set layout bindings.
		VkDescriptorSetLayoutBinding descriptorLayoutBinding[1];
		descriptorLayoutBinding[0].descriptorCount = 1;
		descriptorLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		descriptorLayoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		descriptorLayoutBinding[0].binding = 0;
		descriptorLayoutBinding[0].pImmutableSamplers = nullptr;

		// Create the descriptor set layout using the array of VkDescriptorSetLayoutBindings.
		VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
		descriptorLayoutInfo.flags = 0;
		descriptorLayoutInfo.pNext = nullptr;
		descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayoutInfo.bindingCount = 1;
		descriptorLayoutInfo.pBindings = descriptorLayoutBinding;
		vulkanSuccessOrDie(_deviceVkFunctions.vkCreateDescriptorSetLayout(_device, &descriptorLayoutInfo, NULL, &_dynamicDescriptorSetLayout), "Descriptor Set Layout Creation");
	}
}

/// <summary>Creates the descriptor pool used throughout the demo.</summary>
void VulkanIntroducingPVRShell::createDescriptorPool()
{
	// Create the Descriptor Pool used throughout the demo.

	// A descriptor pool maintains a list of free descriptors from which descriptor sets can be allocated.

	// A VkDescriptorPoolSize structure sets out the number and type of descriptors of that type to allocate
	VkDescriptorPoolSize descriptorPoolSize[3];

	// This demo makes use of 1 uniform buffer, 1 combined image sampler and 1 dynamic uniform buffer.
	descriptorPoolSize[0].descriptorCount = 1;
	descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorPoolSize[1].descriptorCount = 1;
	descriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorPoolSize[2].descriptorCount = 1;
	descriptorPoolSize[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

	// The VkDescriptorPoolCreateInfo structure sets out the set of descriptors allowed to be allocated from the descriptor pool as well as the maximum number of
	// descriptor sets which can be allocated.
	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolInfo.pNext = nullptr;
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = 3;
	descriptorPoolInfo.pPoolSizes = descriptorPoolSize;
	descriptorPoolInfo.maxSets = 2;

	// Create the actual descriptor Pool.
	vulkanSuccessOrDie(_deviceVkFunctions.vkCreateDescriptorPool(_device, &descriptorPoolInfo, NULL, &_descriptorPool), "Descriptor Pool Creation");
}

/// <summary>Creates a checker board texture which will be applied to the triangle during rendering.</summary>
void VulkanIntroducingPVRShell::createTexture()
{
	// Creates a checker board texture which will be applied to the triangle during rendering.

	// In Vulkan, uploading an image/texture requires a few more steps than those familiar with older APIs would expect however these steps are required due to the explicit nature
	// of Vulkan and the control Vulkan affords to the user making possible various performance optimisations. These steps include:

	// 1) Create the (CPU side) texture:
	//	  a) Create the texture data in CPU side memory.

	// 2) Create the (empty) (GPU side) texture:
	//    a) Creating the Vulkan texture definition - a "VkImage" object.
	//    b) Determining the VkImage memory requirements, creating the backing memory object ("VkDeviceMemory" object).
	//    c) Bind the memory (VkDeviceMemory) to the image (VkImage).

	// 3) Upload the data into the texture:
	//    a) Create a staging buffer and its backing memory object - "VkDeviceMemory" object.
	//    b) Map the staging buffer and copy the image data into it.
	//    c) Perform a vkCmdCopyBufferToImage operation to transfer the data into the image.

	// 4) Create a view for the image to make it accessible by pipeline shaders and a sampler object specifying how the image should be sampled:
	//    a) Create a view for the Vulkan texture so that it can be accessed by pipelines shaders for reading or writing to its image data - "VkImageView" object
	//    b) Create a sampler controlling how the sampled image data is sampled when accessed by pipeline shaders.

	// A texture (Sampled Image) is stored in the GPU in an implementation-defined way, which may be completely different to the layout of the texture on disk/CPU side.
	// For that reason, it is not possible to map its memory and write directly the data for that image.
	// This is the reason for the second (Uploading) step: The vkCmdCopyBufferToImage command guarantees the correct translation/swizzling of the texture data.

	//
	// 1a). Create the texture data in CPU side memory.
	//

	// Setup the texture dimensions and the size of the texture itself.
	_textureDimensions.height = 256;
	_textureDimensions.width = 256;
	_textureData.resize(_textureDimensions.width * _textureDimensions.height * (sizeof(uint8_t) * 4));

	// This function generates our texture pattern on-the-fly into a block of CPU side memory (_textureData)
	generateTexture();

	//
	// 2a). Creating the Vulkan texture definition - a "VkImage" object.
	//

	// Record the VkFormat of the texture.
	_triangleImageFormat = VK_FORMAT_R8G8B8A8_UNORM;

	// We create the image info struct. We set the parameters for our texture (layout, format, usage etc..)
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.flags = 0;
	imageInfo.pNext = nullptr;
	imageInfo.format = _triangleImageFormat;
	// Ensure that the format supports VK_IMAGE_TILING_OPTIMAL. Optimal tiling specifies that texels are laid out in an implementation dependent arrangement providing more optimal
	// memory access.
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.extent = { _textureDimensions.width, _textureDimensions.height, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;

	// We create the texture image handle.
	vulkanSuccessOrDie(_deviceVkFunctions.vkCreateImage(_device, &imageInfo, nullptr, &_triangleImage), "Texture Image Creation");

	//
	// 2b). Determining the VkImage memory requirements, creating the backing memory object ("VkDeviceMemory" object).
	//

	// We get the memory allocation requirements for our Image
	VkMemoryRequirements memoryRequirments;
	_deviceVkFunctions.vkGetImageMemoryRequirements(_device, _triangleImage, &memoryRequirments);

	// Allocate the device memory for the created image based on the arguments provided.
	VkMemoryPropertyFlags triangleMemoryPropertyFlags = 0;
	allocateDeviceMemory(_device, memoryRequirments, _physicalDeviceMemoryProperties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		_triangleImageMemory, triangleMemoryPropertyFlags);

	//
	// 2c). Bind the memory (VkDeviceMemory) to the image (VkImage).
	//

	// Finally attach the allocated device memory to the created image.
	vulkanSuccessOrDie(_deviceVkFunctions.vkBindImageMemory(_device, _triangleImage, _triangleImageMemory, 0), "Texture Image Memory Binding");

	//
	// 3a). Create a staging buffer and its backing memory object ("VkDeviceMemory" object).
	//

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	// We use our buffer creation function to generate a staging buffer. We pass the VK_BUFFER_USAGE_TRANSFER_SRC_BIT flag to specify its use.
	VkMemoryPropertyFlags requiredMemoryFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
	VkMemoryPropertyFlags optimalMemoryFlags = requiredMemoryFlags;
	VkMemoryPropertyFlags stagingBufferMemoryFlags = 0;
	createBufferAndMemory(_textureData.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, requiredMemoryFlags, optimalMemoryFlags, stagingBuffer, stagingBufferMemory, stagingBufferMemoryFlags);

	//
	// 3b). Map the staging buffer and copy the image data into it.
	//

	{
		void* mapped = nullptr;
		vulkanSuccessOrDie(_deviceVkFunctions.vkMapMemory(_device, stagingBufferMemory, 0, _textureData.size(), VkMemoryMapFlags(0), &mapped), "Could not map the staging buffer.");
		memcpy(mapped, _textureData.data(), _textureData.size());

		// If the memory property flags for the allocated memory included the use of VK_MEMORY_PROPERTY_HOST_COHERENT_BIT then the host does not need to manage the
		// memory accesses explicitly using the host cache management commands vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges to flush host writes to
		// the device or make device writes visible to the host respectively. This behaviour is handled by the implementation.
		if ((stagingBufferMemoryFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
		{
			// flush the memory
			VkMappedMemoryRange memoryRange = {};
			memoryRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			memoryRange.memory = stagingBufferMemory;
			memoryRange.offset = 0;
			memoryRange.size = _textureData.size();
			_deviceVkFunctions.vkFlushMappedMemoryRanges(_device, 1, &memoryRange);
		}

		_deviceVkFunctions.vkUnmapMemory(_device, stagingBufferMemory);
	}

	//
	// 3c). Perform a vkCmdCopyBufferToImage operation to transfer the data into the image.
	//

	// We specify the region we want to copy from our Texture. In our case it's the entire Image so we pass
	// the Texture width and height as extents.
	VkBufferImageCopy copyRegion = {};
	copyRegion.imageSubresource.aspectMask = formatToImageAspect(_triangleImageFormat);
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1;
	copyRegion.imageExtent.width = static_cast<uint32_t>(_textureDimensions.width);
	copyRegion.imageExtent.height = static_cast<uint32_t>(_textureDimensions.height);
	copyRegion.imageExtent.depth = 1;
	copyRegion.bufferOffset = 0;

	// We create command buffer to execute the copy operation from our command pool.
	VkCommandBuffer cmdBuffers;
	VkCommandBufferAllocateInfo commandAllocateInfo = {};
	commandAllocateInfo.commandPool = _commandPool;
	commandAllocateInfo.pNext = nullptr;
	commandAllocateInfo.commandBufferCount = 1;
	commandAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

	// We Allocate the command buffer from the command pool's memory.
	vulkanSuccessOrDie(_deviceVkFunctions.vkAllocateCommandBuffers(_device, &commandAllocateInfo, &cmdBuffers), "Allocate Command Buffers");

	// We start recording our command buffer operation
	VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
	cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufferBeginInfo.flags = 0;
	cmdBufferBeginInfo.pNext = nullptr;
	cmdBufferBeginInfo.pInheritanceInfo = nullptr;

	vulkanSuccessOrDie(_deviceVkFunctions.vkBeginCommandBuffer(cmdBuffers, &cmdBufferBeginInfo), "Begin Staging Buffer to Image Copy Command Buffer Recording");

	// We specify the sub resource range of our Image. In the case our Image the parameters are default as our image is very simple.
	VkImageSubresourceRange subResourceRange = {};
	subResourceRange.aspectMask = formatToImageAspect(_triangleImageFormat);
	subResourceRange.baseMipLevel = 0;
	subResourceRange.levelCount = 1;
	subResourceRange.layerCount = 1;

	// We need to create a memory barrier to make sure that the image layout is set up for a copy operation.
	VkImageMemoryBarrier copyMemoryBarrier = {};
	copyMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	copyMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	copyMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	copyMemoryBarrier.image = _triangleImage;
	copyMemoryBarrier.subresourceRange = subResourceRange;
	copyMemoryBarrier.srcAccessMask = 0;
	copyMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	// We use a pipeline barrier to change the image layout to accommodate the transfer operation
	_deviceVkFunctions.vkCmdPipelineBarrier(cmdBuffers, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &copyMemoryBarrier);

	// We copy the staging buffer data to memory bound to the image we just created.
	_deviceVkFunctions.vkCmdCopyBufferToImage(cmdBuffers, stagingBuffer, _triangleImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

	// We create a barrier to make sure that the Image layout is Shader read only.
	VkImageMemoryBarrier layoutMemoryBarrier = {};
	layoutMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	layoutMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	layoutMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	layoutMemoryBarrier.image = _triangleImage;
	layoutMemoryBarrier.subresourceRange = subResourceRange;
	layoutMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	layoutMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	// We use a pipeline barrier to change the image layout to be optimized to be read by the shader.
	_deviceVkFunctions.vkCmdPipelineBarrier(cmdBuffers, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &layoutMemoryBarrier);

	// We end the recording of our command buffer.
	vulkanSuccessOrDie(_deviceVkFunctions.vkEndCommandBuffer(cmdBuffers), "End Staging Buffer to Image Copy Command Buffer Recording");

	// We create a fence to make sure that the command buffer is synchronised correctly.
	VkFence copyFence;
	VkFenceCreateInfo copyFenceInfo = {};
	copyFenceInfo.flags = 0;
	copyFenceInfo.pNext = 0;
	copyFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	// We create the fence.
	vulkanSuccessOrDie(_deviceVkFunctions.vkCreateFence(_device, &copyFenceInfo, nullptr, &copyFence), "Staging Buffer Copy to Image Fence Creation");

	// Submit the command buffer to the queue specified
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffers;

	vulkanSuccessOrDie(_deviceVkFunctions.vkQueueSubmit(_queue, 1, &submitInfo, copyFence), "Submit Staging Buffer Copy to Image Command Buffer");

	// Wait for the specified fence to be signalled which ensures that the command buffer has finished executing.
	vulkanSuccessOrDie(_deviceVkFunctions.vkWaitForFences(_device, 1, &copyFence, true, static_cast<uint64_t>(-1)), "Staging Buffer Copy to Image Fence Signal");

	//
	// 4a). Create a view for the Vulkan texture so that it can be accessed by pipelines shaders for reading or writing to its image data - "VkImageView" object
	//

	// After the Image is complete, and we copied all the texture data, we need to create an Image View to make sure
	// that API can understand what the Image is. We can provide information on the format for example.

	// We create an Image view info.
	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.flags = 0;
	imageViewInfo.pNext = nullptr;
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = _triangleImageFormat;
	imageViewInfo.image = _triangleImage;
	imageViewInfo.subresourceRange.layerCount = 1;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.aspectMask = formatToImageAspect(_triangleImageFormat);
	imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;

	// We create the image view proper.
	vulkanSuccessOrDie(_deviceVkFunctions.vkCreateImageView(_device, &imageViewInfo, nullptr, &_triangleImageView), "Texture Image View Creation");

	//
	// 4b). Create a sampler controlling how the sampled image data is sampled when accessed by pipeline shaders.
	//

	// We create a sampler info struct. We'll need the sampler to pass
	// data to the fragment shader during the execution of the rendering phase.
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.flags = 0;
	samplerInfo.pNext = nullptr;
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE;

	// We create the sampler object.
	vulkanSuccessOrDie(_deviceVkFunctions.vkCreateSampler(_device, &samplerInfo, nullptr, &_bilinearSampler), "Texture Sampler Creation");

	// We clean up all the temporary data we created for this operation.
	_deviceVkFunctions.vkDestroyFence(_device, copyFence, nullptr);
	_deviceVkFunctions.vkFreeCommandBuffers(_device, _commandPool, 1, &cmdBuffers);
	_deviceVkFunctions.vkFreeMemory(_device, stagingBufferMemory, nullptr);
	_deviceVkFunctions.vkDestroyBuffer(_device, stagingBuffer, nullptr);
}

/// <summary>Creates the RenderPass used throughout the demo.</summary>
void VulkanIntroducingPVRShell::createRenderPass()
{
	// Create the RenderPass used throughout the demo.

	// A RenderPass encapsulates a collection of attachments, one or more subpasses, dependencies between the subpasses and then provides a description for
	// how the attachments are used over the execution of the respective subpasses. A RenderPass allows an application to communicate a high level structure of a frame to the
	// implementation.

	// RenderPasses are one of the singly most important features included in Vulkan from the point of view of a tiled architecture. Before going into the gritty details of what
	// RenderPasses are and how they provide a heap of optimization opportunities a (very) brief introduction to tiled architectures - a tiled architecture like any other takes
	// triangles as input but will bin these triangles to particular tiles corresponding to regions of a Framebuffer and then for each tile in turn it will render the subset of
	// geometry binned only to that tile meaning the per tile access becomes very coherent and cache friendly. RenderPasses, subpasses and the use of transient attachments (all
	// explained below) let us exploit and make most of the benefits these kinds of architectures provide.

	// For more information on our TBDR (Tile Based Deferred Rendering) architecture check out our blog posts:
	//		https://www.imgtec.com/blog/a-look-at-the-powervr-graphics-architecture-tile-based-rendering/
	//		https://www.imgtec.com/blog/the-dr-in-tbdr-deferred-rendering-in-rogue/

	// Each RenderPass subpass may reference a subset of the RenderPass's Framebuffer attachments for reading or writing where each subpass containing information
	// about what happens to the attachment data when the subpass begins including whether to clear it, load it from memory or leave it uninitialised as well
	// as what to do with the attachment data when the subpass ends including storing it back to memory or discarding it.
	// RenderPasses require that applications explicitly set out the dependencies between the subpasses providing an implementation with the know-how to effectively optimize when
	// it should flush/clear/store memory in way it couldn't before. RenderPasses are a prime example of how Vulkan has replaced implementation guess work with the application
	// explicitness requiring them to set out their known and understood dependencies - Who is in the better place to properly understand and make decisions as to dependencies
	// between a particular set of commands, images or resources than the application making use of them?

	// Another important feature introduced by Vulkan is the use of transient images (specify VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT in the VkImageUsageFlags member of their
	// VkImageCreateInfo creation structure).
	// Consider an attachment which is only ever written to and read during a single RenderPass, an attachment which begins either uninitialized or in a cleared state,
	// which is first written to by one or more subpasses and then read from by one or more subpasses with the resulting attachment data ultimately discarded then
	// technically the image data never needs to be written out to main memory, further it doesn't need true memory backing at all.
	// The image data only has a temporary lifetime and therefore can happily live only in cached on-chip memory.

	// RenderPass subpasses, input attachments and transient attachments make possible huge savings in bandwidth, critically for mobile architectures, but also reduce latency by
	// explicitly setting out their dependencies leading ultimately to a reduction in power consumption.

	// RenderPass subpasses and transient attachments owe a lot to the OpenGL ES extensions GL_EXT_shader_pixel_local_storage, GL_EXT_shader_pixel_local_storage2 pioneered by
	// mobile architectures. For more information on pixel local storage check out the extensions:
	//		https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_shader_pixel_local_storage.txt
	//		https://www.khronos.org/registry/OpenGL/extensions/EXT/EXT_shader_pixel_local_storage2.txt

	// This demo uses a very simple RenderPass with a colour and depth stencil attachment. The RenderPass then makes use of a single subpass referencing both the colour and
	// depth/stencil attachments of the RenderPass. More complicated examples are included in SDK showing off the benefits and optimizations made possible through the use of
	// RenderPasses with multiple subpasses including the use of input attachments and transient attachments. Check out our DeferredShading example to see how to make the most
	// RenderPasses and the benefits they can provide to a tiled architecture.
	VkRenderPassCreateInfo renderPassInfo = {};

	// An attachment description describes the structure of an attachment including formats, number of samples, image layout transitions and how the image should be handled at the
	// beginning and end of the RenderPass including whether to load or clear memory and store or discard memory respectively.
	VkAttachmentDescription attachmentDescriptions[2];

	// A subpass encapsulates a set of rendering commands corresponding to a particular phase of a rendering pass including the reading and writing of a subset of RenderPass
	// attachments. A subpass description specifies the subset of attachments involved in the particular phase of rendering corresponding to the subpass.
	VkSubpassDescription subpass = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = &attachmentDescriptions[0];
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.subpassCount = 1;

	// The first VkAttachmentDescription describes a colour attachment which will be undefined initially (VK_IMAGE_LAYOUT_UNDEFINED), transitioning to a layout suitable for
	// presenting to the screen (VK_IMAGE_LAYOUT_PRESENT_SRC_KHR), uses only a single sample per pixel (VK_SAMPLE_COUNT_1_BIT), a VkFormat matching the format used by the swapchain
	// images, a VkAttachmentLoadOp specifying that the attachment will be cleared at the beginning of the first subpass in which the attachment is used, a VkAttachmentStoreOp
	// specifying that the attachment will be stored (VK_ATTACHMENT_STORE_OP_STORE) at the end of the subpass in which the attachment is last used. The stencil load and store
	// ops are set as VK_ATTACHMENT_LOAD_OP_DONT_CARE and VK_ATTACHMENT_STORE_OP_DONT_CARE respectively as the attachment has no stencil component.
	attachmentDescriptions[0] = {};
	attachmentDescriptions[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescriptions[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDescriptions[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescriptions[0].format = _swapchainColorFormat;
	attachmentDescriptions[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptions[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescriptions[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescriptions[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// The first VkAttachmentDescription describes a depth/stencil attachment which will be undefined initially (VK_IMAGE_LAYOUT_UNDEFINED), transitioning to a layout suitable for
	// use as a depth stencil attachment (VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL), uses only a single sample per pixel (VK_SAMPLE_COUNT_1_BIT), a VkFormat matching the
	// format used by the depth stencil images. Both the stencil and depth VkAttachmentLoadOp specify that the attachment will be cleared at the beginning of the first subpass in
	// which the attachment is used, and both the stencil and depth VkAttachmentStoreOps specify that the attachment will be stored (VK_ATTACHMENT_STORE_OP_STORE) at the end of the
	// subpass in which the attachment is last used.
	attachmentDescriptions[1] = {};
	attachmentDescriptions[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescriptions[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDescriptions[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescriptions[1].format = _depthStencilFormat;
	attachmentDescriptions[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptions[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescriptions[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescriptions[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// Defines the set of RenderPass attachments the subsequent subpass will make use of and the layout these attachments must be in for the particular subpass.
	VkAttachmentReference attachmentReferences[2] = { { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL }, { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL } };

	// The subpass makes use of a single colour attachment and the depth stencil attachment matching RenderPass attachments at index 0 and 1 respectively.
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &attachmentReferences[0];
	subpass.pDepthStencilAttachment = &attachmentReferences[1];
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

	// A subpass dependency describes the execution and memory dependencies between subpasses.
	// In this demo only a single subpass is used so technically no subpass dependencies are strictly required however unless specified an implicit subpass dependency is
	// added from VK_SUBPASS_EXTERNAL to the first subpass that uses an attachment and another implicit subpass dependency is added from the last subpass that uses
	// an attachment to VK_SUBPASS_EXTERNAL.
	// As described above the application is in the best position to understand and make decisions about all of the memory dependencies and so we choose to explicitly
	// provide the otherwise implicit subpass dependencies.
	VkSubpassDependency subpassDependencies[2];

	// Adds an explicit subpass dependency from VK_SUBPASS_EXTERNAL to the first subpass that uses an attachment which is the first subpass (0).
	subpassDependencies[0] = {};
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].srcAccessMask = 0;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Adds an explicit subpass dependency from the first subpass that uses an attachment which is the first subpass (0) to VK_SUBPASS_EXTERNAL.
	subpassDependencies[1] = {};
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[1].dstAccessMask = 0;
	subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Add the set of dependencies to the RenderPass creation.
	renderPassInfo.pDependencies = subpassDependencies;
	renderPassInfo.dependencyCount = 2;
	vulkanSuccessOrDie(_deviceVkFunctions.vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass), "Failed to create the renderPass");
}

/// <summary>Creates the Framebuffer objects used in this demo.</summary>
void VulkanIntroducingPVRShell::createFramebuffer()
{
	// Create the framebuffers which are used in conjunction with the application renderPass.

	// Framebuffers encapsulate a collection of attachments that a renderPass instance uses.

	VkFramebufferCreateInfo framebufferInfo = {};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;

	// Note that each element of pAttachments must have dimensions at least as large as the Framebuffer dimensions.
	framebufferInfo.width = getWidth();
	framebufferInfo.height = getHeight();
	framebufferInfo.layers = 1;
	// This Framebuffer is compatible with the application renderPass or with any other renderPass compatible with the application renderPass. For more information on RenderPass
	// compatibility please refer to the Vulkan spec section "Render Pass Compatibility".
	framebufferInfo.renderPass = _renderPass;
	framebufferInfo.attachmentCount = 2;
	// Create a Framebuffer per swapchain making use of the per swapchain presentation image and depth stencil image.
	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		VkImageView imageViews[] = { _swapchainImageViews[i], _depthStencilImageViews[i] };
		framebufferInfo.pAttachments = imageViews;

		vulkanSuccessOrDie(_deviceVkFunctions.vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_framebuffers[i]), "Failed to create framebuffers");
	}
}

/// <summary>Creates the command pool used throughout the demo.</summary>
void VulkanIntroducingPVRShell::createCommandPool()
{
	// Create the command pool used for allocating the command buffers used throughout the demo

	// A command pool is an opaque object used for allocating command buffer memory from which applications can spread the cost of resource creation and command recording.
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	// Command Pool flags can be used to specify usage behaviour of command buffers allocated from this command pool.
	commandPoolCreateInfo.flags = 0;
	// Designates the queue family to which commands buffers allocated from this command pool can be submitted.
	commandPoolCreateInfo.queueFamilyIndex = _graphicsQueueFamilyIndex;
	vulkanSuccessOrDie(_deviceVkFunctions.vkCreateCommandPool(_device, &commandPoolCreateInfo, nullptr, &_commandPool), "Failed to create command pool");
}

/// <summary>Creates the fences and semaphores used for synchronisation throughout this demo.</summary>
void VulkanIntroducingPVRShell::createSynchronisationPrimitives()
{
	// Create the fences and semaphores for synchronisation throughout the demo.

	// One of the major changes in strategy introduced in Vulkan has been that there are fewer implicit guarantees as to the order in which commands are executed with respect to
	// other commands on the device and the host itself. Synchronisation has now become the responsibility of the application.

	// Here we create the fences and semaphores used for synchronising image acquisition, the use of per frame resources, submission to device queues and finally the presentation
	// of images. Note that the use of these synchronisation primitives are explained in detail in the renderFrame function.

	// Semaphores are used for inserting dependencies between batches submitted to queues.
	VkSemaphoreCreateInfo sci = {};
	sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	// Fences are used for indicating a dependency from the queue to the host.
	// The fences are created in the signalled state meaning we don't require any special logic for handling the first frame synchronisation.
	VkFenceCreateInfo fci = {};
	fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (uint32_t i = 0; i < _swapchainLength; ++i)
	{
		vulkanSuccessOrDie(_deviceVkFunctions.vkCreateFence(_device, &fci, nullptr, &_perFrameResourcesFences[i]), "Failed to create command buffer fence");
		vulkanSuccessOrDie(_deviceVkFunctions.vkCreateSemaphore(_device, &sci, nullptr, &_imageAcquireSemaphores[i]), "Failed to create semaphore");
		vulkanSuccessOrDie(_deviceVkFunctions.vkCreateSemaphore(_device, &sci, nullptr, &_presentationSemaphores[i]), "Failed to create semaphore");
	}
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanIntroducingPVRShell>(); }
