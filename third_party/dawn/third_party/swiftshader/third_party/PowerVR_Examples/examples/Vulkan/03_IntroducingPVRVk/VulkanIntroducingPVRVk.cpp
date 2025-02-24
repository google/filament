/*!
\brief This demo provides an introduction to the PVRVk library.
* This demo makes use of the PVRVk library for creating, maintaining and using Vulkan objects.
\file VulkanIntroducingPVRVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

// Include files
// enables the use of the PVRCore module which provides a collection of supporting code for the PowerVR Framework
#include "PVRCore/PVRCore.h"
// enables the use of the PVRShell module which provides an abstract mechanism for the native platform primarily used for handling window creation and input handling.
#include "PVRShell/PVRShell.h"
// enables the use of the PVRVk module which provides an easy to use, minimal overhead abstraction layer on top of the Vulkan API
// giving default constructors for all Vulkan objects, deterministic life cycle management through reference counting and in general a clean, modern interface.
#include "PVRVk/PVRVk.h"

/// <summary>Maps a set of DebugUtilsMessageSeverityFlagsEXT to a particular type of log message.</summary>
/// <param name="flags">The DebugUtilsMessageSeverityFlagsEXT to map to a LogLevel.</param>
/// <returns>Returns a LogLevel deemed to correspond to the given pvrvk::DebugUtilsMessageSeverityFlagsEXT.</returns>
inline LogLevel mapDebugUtilsMessageSeverityFlagsToLogLevel(pvrvk::DebugUtilsMessageSeverityFlagsEXT flags)
{
	if ((flags & pvrvk::DebugUtilsMessageSeverityFlagsEXT::e_INFO_BIT_EXT) != 0) { return LogLevel::Information; }
	if ((flags & pvrvk::DebugUtilsMessageSeverityFlagsEXT::e_WARNING_BIT_EXT) != 0) { return LogLevel::Warning; }
	if ((flags & pvrvk::DebugUtilsMessageSeverityFlagsEXT::e_VERBOSE_BIT_EXT) != 0) { return LogLevel::Debug; }
	if ((flags & pvrvk::DebugUtilsMessageSeverityFlagsEXT::e_ERROR_BIT_EXT) != 0) { return LogLevel::Error; }
	return LogLevel::Information;
}

namespace {
std::string debugUtilsMessengerCallbackToString(
	VkDebugUtilsMessageSeverityFlagBitsEXT inMessageSeverity, VkDebugUtilsMessageTypeFlagsEXT inMessageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData)
{
	std::string messageSeverityString = pvrvk::to_string(static_cast<pvrvk::DebugUtilsMessageSeverityFlagsEXT>(inMessageSeverity));
	std::string messageTypeString = pvrvk::to_string(static_cast<pvrvk::DebugUtilsMessageTypeFlagsEXT>(inMessageTypes));

	std::string exceptionMessage = pvr::strings::createFormatted("%s (%s) - ID: %i, Name: \"%s\":\n\tMESSAGE: %s", messageSeverityString.c_str(), messageTypeString.c_str(),
		pCallbackData->messageIdNumber, pCallbackData->pMessageIdName, pCallbackData->pMessage);

	if (pCallbackData->objectCount > 0)
	{
		exceptionMessage += "\n";
		std::string objectsMessage = pvr::strings::createFormatted("\tAssociated Objects - (%u)\n", pCallbackData->objectCount);

		for (uint32_t i = 0; i < pCallbackData->objectCount; ++i)
		{
			std::string objectType = pvrvk::to_string(static_cast<pvrvk::ObjectType>(pCallbackData->pObjects[i].objectType));

			objectsMessage += pvr::strings::createFormatted("\t\tObject[%u] - Type %s, Value %p, Name \"%s\"\n", i, objectType.c_str(),
				(void*)(pCallbackData->pObjects[i].objectHandle), pCallbackData->pObjects[i].pObjectName);
		}

		exceptionMessage += objectsMessage;
	}

	if (pCallbackData->cmdBufLabelCount > 0)
	{
		exceptionMessage += "\n";
		std::string cmdBufferLabelsMessage = pvr::strings::createFormatted("\tAssociated Command Buffer Labels - (%u)\n", pCallbackData->cmdBufLabelCount);

		for (uint32_t i = 0; i < pCallbackData->cmdBufLabelCount; ++i)
		{
			cmdBufferLabelsMessage += pvr::strings::createFormatted("\t\tCommand Buffer Label[%u] - %s, Colour: {%f, %f, %f, %f}\n", i, pCallbackData->pCmdBufLabels[i].pLabelName,
				pCallbackData->pCmdBufLabels[i].color[0], pCallbackData->pCmdBufLabels[i].color[1], pCallbackData->pCmdBufLabels[i].color[2], pCallbackData->pCmdBufLabels[i].color[3]);
		}

		exceptionMessage += cmdBufferLabelsMessage;
	}

	if (pCallbackData->queueLabelCount > 0)
	{
		exceptionMessage += "\n";
		std::string queueLabelsMessage = pvr::strings::createFormatted("\tAssociated Queue Labels - (%u)\n", pCallbackData->queueLabelCount);

		for (uint32_t i = 0; i < pCallbackData->queueLabelCount; ++i)
		{
			queueLabelsMessage += pvr::strings::createFormatted("\t\tQueue Label[%u] - %s, Colour: {%f, %f, %f, %f}\n", i, pCallbackData->pQueueLabels[i].pLabelName,
				pCallbackData->pQueueLabels[i].color[0], pCallbackData->pQueueLabels[i].color[1], pCallbackData->pQueueLabels[i].color[2], pCallbackData->pQueueLabels[i].color[3]);
		}

		exceptionMessage += queueLabelsMessage;
	}
	return exceptionMessage;
}
} // namespace

/// <summary>An application defined callback used as the callback function specified in as pfnCallback in the
/// create info VkDebugUtilsMessengerCreateInfoEXT used when creating the debug utils messenger callback vkCreateDebugUtilsMessengerEXT.</summary>
VKAPI_ATTR VkBool32 VKAPI_CALL throwOnErrorDebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT inMessageSeverity, VkDebugUtilsMessageTypeFlagsEXT inMessageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	(void)pUserData;

	// throw an exception if the type of DebugUtilsMessageSeverityFlagsEXT contains the ERROR_BIT
	if ((static_cast<pvrvk::DebugUtilsMessageSeverityFlagsEXT>(inMessageSeverity) & (pvrvk::DebugUtilsMessageSeverityFlagsEXT::e_ERROR_BIT_EXT)) !=
		pvrvk::DebugUtilsMessageSeverityFlagsEXT::e_NONE)
	{ throw pvrvk::ErrorValidationFailedEXT(debugUtilsMessengerCallbackToString(inMessageSeverity, inMessageTypes, pCallbackData)); }
	return VK_FALSE;
}

/// <summary>The application defined callback used as the callback function specified in as pfnCallback in the
/// create info VkDebugUtilsMessengerCreateInfoEXT used when creating the debug utils messenger callback vkCreateDebugUtilsMessengerEXT.</summary>
VKAPI_ATTR VkBool32 VKAPI_CALL logMessageDebugUtilsMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT inMessageSeverity, VkDebugUtilsMessageTypeFlagsEXT inMessageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	(void)pUserData;

	Log(mapDebugUtilsMessageSeverityFlagsToLogLevel(static_cast<pvrvk::DebugUtilsMessageSeverityFlagsEXT>(inMessageSeverity)),
		debugUtilsMessengerCallbackToString(inMessageSeverity, inMessageTypes, pCallbackData).c_str());

	return VK_FALSE;
}

/// <summary>Map a pvrvk::DebugReportFlagsEXT variable to a corresponding log severity.</summary>
/// <param name="flags">A set of pvrvk::DebugReportFlagsEXT specifying the type of event which triggered the callback.</param>
/// <returns>A LogLevel corresponding to the pvrvk::DebugReportFlagsEXT.</returns>
LogLevel mapValidationTypeToLogType(pvrvk::DebugReportFlagsEXT flags)
{
	// Simply map the pvrvk::DebugReportFlagsEXT to a particular LogLevel.
	if ((flags & pvrvk::DebugReportFlagsEXT::e_INFORMATION_BIT_EXT) != 0) { return LogLevel::Information; }
	if ((flags & pvrvk::DebugReportFlagsEXT::e_WARNING_BIT_EXT) != 0) { return LogLevel::Warning; }
	if ((flags & pvrvk::DebugReportFlagsEXT::e_PERFORMANCE_WARNING_BIT_EXT) != 0) { return LogLevel::Performance; }
	if ((flags & pvrvk::DebugReportFlagsEXT::e_ERROR_BIT_EXT) != 0) { return LogLevel::Error; }
	if ((flags & pvrvk::DebugReportFlagsEXT::e_DEBUG_BIT_EXT) != 0) { return LogLevel::Debug; }

	return LogLevel::Information;
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

	// Throw an exception if the type of pvrvk::DebugReportFlagsEXT contains the ERROR_BIT.
	if ((static_cast<pvrvk::DebugReportFlagsEXT>(flags) & (pvrvk::DebugReportFlagsEXT::e_ERROR_BIT_EXT)) != pvrvk::DebugReportFlagsEXT(0))
	{
		throw pvrvk::ErrorValidationFailedEXT(
			std::string(pvrvk::to_string(static_cast<pvrvk::DebugReportObjectTypeEXT>(objectType)) + std::string(". VULKAN_LAYER_VALIDATION: ") + pMessage));
	}
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
	// map the VkDebugReportFlagsEXT to a suitable log type
	// map the VkDebugReportObjectTypeEXT to a stringified representation
	// Log the message generated by a lower layer
	Log(mapValidationTypeToLogType(static_cast<pvrvk::DebugReportFlagsEXT>(flags)),
		std::string(pvrvk::to_string(static_cast<pvrvk::DebugReportObjectTypeEXT>(objectType)) + std::string(". VULKAN_LAYER_VALIDATION: %s")).c_str(), pMessage);

	return VK_FALSE;
}

// In Vulkan, extensions may define additional Vulkan commands, structures and enumerations which are not included in or used by Core Vulkan.
// Functionality which isn't strictly necessary but which may provide additional or extended functionality may be defined via separate extensions.
// Here we define the set of instance and device extensions which may be used by various platforms and window systems supported by the demo.

// Helpfully extensions in Vulkan.h are protected via ifdef guards meaning we can conditionally compile our application to use the most appropriate set of extensions.
// Later we will filter out unsupported extensions and act accordingly based on those that are required and are supported by the chosen platform and window system combination.

// Of note is that the 'VK_EXT_XXXX_EXTENSION_NAME' syntax is provided by the vulkan headers as compile-time constants so that extension names can be used unambiguously avoiding
// typos in querying for them.
/// <summary>Container for a list of Instance extensions to be used when initialising the instance.</summary>
struct InstanceExtensions : public pvrvk::VulkanExtensionList
{
	/// <summary>Initialises a list of instance extensions.</summary>
	// Defines the set of global Vulkan instance extensions which may be required depending on the combination of platform and window system in use
	InstanceExtensions()
	{
		// The VK_KHR_surface extension declares the VkSurfaceKHR object and provides a function for destroying VkSurfaceKHR objects
		// note that the creation of VkSurfaceKHR objects is delegated to platform specific extensions but from the applications
		// point of view the handle is an opaque non-platform-specific type. Specifically for this demo VK_KHR_surface is required for creating VkSurfaceKHR objects which are
		// further used by the device extension VK_KHR_swapchain.
		addExtension(VK_KHR_SURFACE_EXTENSION_NAME);
		// The VK_KHR_display extension provides the functionality for enumerating display devices and creating VkSurfaceKHR objects that directly
		// target displays. This extension is particularly important for applications which render directly to display devices without
		// an intermediate window system such as embedded applications or when running on embedded platforms
		addExtension(VK_KHR_DISPLAY_EXTENSION_NAME);
#ifdef DEBUG
		// The VK_EXT_debug_utils and VK_EXT_debug_report extensions provides the functionality for defining a way in which layers and the implementation can
		// call back to the application for events of particular interest to the application. By enabling this extension the application
		// has the opportunity for receiving much more detailed feedback regarding the applications use of Vulkan. Note that VK_EXT_debug_report has been deprecated in
		// favour of the more forward looking extension VK_EXT_debug_utils.
		addExtension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		addExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		// The VK_KHR_win32_surface extension provides the necessary mechanism for creating a VkSurfaceKHR object which refers to a Win32 HWND in addition
		// to functions for querying the support for rendering to the windows desktop
		addExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
		// The VK_KHR_android_surface extension provides the necessary mechanism for creating a VkSurfaceKHR object which refers to an ANativeWindow, Android's native surface type
		addExtension(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
		// The VK_KHR_xlib_surface extension provides the necessary mechanism for creating a VkSurfaceKHR object which refers to an X11 Window using Xlib in addition to
		// functions for querying the support for rendering via Xlib
		addExtension(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_XCB_KHR)
		// The VK_KHR_xcb_surface extension provides the necessary mechanism for creating a VkSurfaceKHR object which refers to an XCB Window using Xlib in addition to
		// functions for querying the support for rendering via XCB
		addExtension(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
		// The VK_KHR_wayland_surface extension provides the necessary mechanism for creating a VkSurfaceKHR object which refers to a Wayland wl_surface in addition to
		// functions for querying the support for rendering to a Wayland compositor
		addExtension(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
		// The VK_MVK_macos_surface extension provides the necessary mechanism for creating a VkSurfaceKHR object which refers to a CAMetalLayer backed NSView
		addExtension(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#endif
	}
};

/// <summary>Container for a list of Device extensions to be used when creating the Device.</summary>
struct DeviceExtensions : public pvrvk::VulkanExtensionList
{
	/// <summary>Initialises a list of device extensions.</summary>
	DeviceExtensions()
	{
		// The VK_KHR_swapchain extension is the device specific companion to VK_KHR_surface which introduces VkSwapchainKHR objects
		// enabling the ability to present render images to specified surfaces
		addExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}
};

// Vulkan is a layered API with layers that may provide additional functionality over core Vulkan but do not add or modify existing Vulkan commands.
// In Vulkan the validation of correct API usage is left to validation layers so they are of particular importance.
// When a Vulkan layer is enabled it inserts itself into the call chain for Vulkan commands the specific layer is interested in
// the concept of using layers allows implementations to avoid performance penalties incurred for validating application behaviour and API usage
/// <summary>Container for a list of instance layers to be used for initialising an instance using the helper function 'createInstanceAndSurface'.</summary>
struct InstanceLayers : public pvrvk::VulkanLayerList
{
	InstanceLayers()
	{
#ifdef DEBUG
		// Khronos Validation is a layer which encompasses all of the functionality that used to be contained in VK_LAYER_GOOGLE_threading,
		// VK_LAYER_LUNARG_parameter_validation, VK_LAYER_LUNARG_object_tracker, VK_LAYER_LUNARG_core_validation, and VK_LAYER_GOOGLE_unique_objects
		addLayer("VK_LAYER_KHRONOS_validation");
		// Standard Validation is a (now deprecated) meta-layer managed by the LunarG Loader.
		// Using Standard Validation will cause the loader to load a standard set of validation layers in an optimal order: VK_LAYER_GOOGLE_threading,
		// VK_LAYER_LUNARG_parameter_validation, VK_LAYER_LUNARG_object_tracker, VK_LAYER_LUNARG_core_validation, and VK_LAYER_GOOGLE_unique_objects.
		addLayer("VK_LAYER_LUNARG_standard_validation");
		// PerfDoc is a Vulkan layer which attempts to identify API usage is may be discouraged primarily by validating applications
		// against the rules set out in the Mali Application Developer Best Practices document
		addLayer("VK_LAYER_ARM_mali_perf_doc");
		addLayer("VK_LAYER_IMG_powervr_perf_doc");
#endif
	}
};
// note that device specific layers have now been deprecated and all layers are enabled during instance creation, with all enabled instance layers able
// to intercept all commands operating on that instance including any of its child objects i.e. the device or commands operating on a specific device.

/// <summary>Retrieves the pvrvk::ImageAspectFlags based on the pvrvk::Format. The pvrvk::ImageAspectFlags specify the aspects of an image for purposes such as identifying a
/// sub-resource.</summary>
/// <param name="format">The pvrvk::Format to retrieve pvrvk::ImageAspectFlags for.</param>
/// <returns>The compatible pvrvk::ImageAspectFlags based on the input pvrvk::Format.</returns>
/// <details>This function simply infers the pvrvk::ImageAspectFlags based on the position of the given pvrvk::Format in the list of the pvrvk::Format enum.</details>
inline pvrvk::ImageAspectFlags formatToImageAspect(pvrvk::Format format)
{
	// Attempt to find a set of pvrvk::ImageAspectFlags compatible with the pvrvk::Format specified as input

	// Undefined formats do not have compatible pvrvk::ImageAspectFlags.
	if (format == pvrvk::Format::e_UNDEFINED) { throw pvr::PvrError("Cannot retrieve pvrvk::ImageAspectFlags from an undefined pvrvk::Format"); }

	// For pvrvk::Formats which correspond to anything other than the set of Depth/Stencil formats then the pvrvk::ImageAspectFlags can be assumed to be pvrvk::ImageAspectFlags::e_COLOR_BIT.
	if (format < pvrvk::Format::e_D16_UNORM || format > pvrvk::Format::e_D32_SFLOAT_S8_UINT) { return pvrvk::ImageAspectFlags::e_COLOR_BIT; }

	// If the pvrvk::Format is one of the Depth/Stencil formats then determine whether the compatible pvrvk::ImageAspectFlags includes pvrvk::ImageAspectFlags::e_DEPTH_BIT or
	// pvrvk::ImageAspectFlags::e_STENCIL_BIT or both.
	const pvrvk::ImageAspectFlags formats[] = {
		pvrvk::ImageAspectFlags::e_DEPTH_BIT, // pvrvk::Format::e_D16_UNORM
		pvrvk::ImageAspectFlags::e_DEPTH_BIT, // pvrvk::Format::e_X8_D24_UNORM_PACK32
		pvrvk::ImageAspectFlags::e_DEPTH_BIT, // pvrvk::Format::e_D32_SFLOAT
		pvrvk::ImageAspectFlags::e_STENCIL_BIT, // pvrvk::Format::e_S8_UINT
		pvrvk::ImageAspectFlags::e_DEPTH_BIT | pvrvk::ImageAspectFlags::e_STENCIL_BIT, // pvrvk::Format::e_D16_UNORM_S8_UINT
		pvrvk::ImageAspectFlags::e_DEPTH_BIT | pvrvk::ImageAspectFlags::e_STENCIL_BIT, // pvrvk::Format::e_D24_UNORM_S8_UINT
		pvrvk::ImageAspectFlags::e_DEPTH_BIT | pvrvk::ImageAspectFlags::e_STENCIL_BIT, // pvrvk::Format::e_D32_SFLOAT_S8_UINT
	};
	return formats[static_cast<uint32_t>(format) - static_cast<uint32_t>(pvrvk::Format::e_D16_UNORM)];
}

/// <summary>Attempts to find the index for a suitable memory type supporting the memory type bits required from the set of memory type bits supported.</summary>
/// <param name="physicalDevice">The physical device.</param>
/// <param name="allowedMemoryTypeBits">A set of allowed memory type bits for the required memory allocation. Retrieved from the memoryTypeBits member of the
/// pvrvk::MemoryRequirements retrieved using vkGetImageMemoryRequirements or vkGetBufferMemoryRequirements etc.</param>
/// <param name="requiredMemoryProperties">The memory property flags required for the memory allocation.</param>
/// <param name="optimalMemoryProperties">An optimal st of memory property flags to use for the memory allocation.</param>
/// <param name="outMemoryTypeIndex">The memory type index used for allocating the memory.</param>
/// <param name="outMemoryPropertyFlags">The memory property flags actually used when allocating the memory.</param>
inline void getMemoryTypeIndex(const pvrvk::PhysicalDevice& physicalDevice, const uint32_t allowedMemoryTypeBits, const pvrvk::MemoryPropertyFlags requiredMemoryProperties,
	const pvrvk::MemoryPropertyFlags optimalMemoryProperties, uint32_t& outMemoryTypeIndex, pvrvk::MemoryPropertyFlags& outMemoryPropertyFlags)
{
	// First attempt to find a memory type index which supports the optimal set of memory property flags
	pvrvk::MemoryPropertyFlags memoryPropertyFlags = optimalMemoryProperties;

	// We ensure that the optimal set of memory property flags is a superset of the required set of memory property flags.
	// This also handles cases where the optimal set of memory property flags hasn't been set but the required set has.
	memoryPropertyFlags |= requiredMemoryProperties;

	// Attempt to find a valid memory type index based on the optimal memory property flags.
	outMemoryTypeIndex = physicalDevice->getMemoryTypeIndex(allowedMemoryTypeBits, memoryPropertyFlags, outMemoryPropertyFlags);

	// if the optimal set cannot be found then fallback to the required set. The required set of memory property flags are expected to be supported and found. If not, an
	// exception will be thrown.
	if (outMemoryTypeIndex == static_cast<uint32_t>(-1))
	{
		memoryPropertyFlags = requiredMemoryProperties;
		outMemoryTypeIndex = physicalDevice->getMemoryTypeIndex(allowedMemoryTypeBits, memoryPropertyFlags, outMemoryPropertyFlags);
		if (outMemoryTypeIndex == static_cast<uint32_t>(-1)) { throw pvr::PvrError("Cannot find suitable memory type index for the set of pvrvk::MemoryPropertyFlags."); }
	}
}

// Filenames for the SPIR-V shader file binaries used in this demo
// Note that the binaries are pre-compiled using the "recompile script" included alongside the demo (recompile.sh/recompile.bat)
const char* VertShaderName = "VertShader.vsh.spv";
const char* FragShaderName = "FragShader.fsh.spv";

// Resources used throughout the demo.
struct DeviceResources
{
	// Per application Vulkan instance used to initialize the Vulkan library.
	// The Vulkan instance forms the basis for all interactions between the application and the implementation.
	pvrvk::Instance instance;

	// Stores the set of created Debug Utils Messengers which provide a mechanism for tools, layers and the implementation to call back to the application
	pvrvk::DebugUtilsMessenger debugUtilsMessengers[2];

	// Stores the set of created Debug Report Callbacks which provide a mechanism for the Vulkan layers and the implementation to call back to the application.
	pvrvk::DebugReportCallback debugReportCallbacks[2];

	// The Vulkan surface handle (pvrvk::Surface) abstracting the native platform surface
	pvrvk::Surface surface;

	// The logical device representing a logical connection to an underlying physical device
	pvrvk::Device device;

	// The WSI Swapchain object
	pvrvk::Swapchain swapchain;

	// The queue to which various command buffers will be submitted to.
	pvrvk::Queue queue;

	// The command pool from which the command buffers will be allocated.
	pvrvk::CommandPool commandPool;

	// The descriptor pool used for allocating descriptor sets.
	pvrvk::DescriptorPool descriptorPool;

	// The vertex buffer object used for rendering.
	pvrvk::Buffer vbo;

	// The model view projection buffer object used for rendering
	pvrvk::Buffer modelViewProjectionBuffer;

	// The descriptor set layouts for the static and dynamic descriptor set.
	pvrvk::DescriptorSetLayout staticDescriptorSetLayout;
	pvrvk::DescriptorSetLayout dynamicDescriptorSetLayout;

	// The Descriptor sets used for rendering.
	pvrvk::DescriptorSet staticDescriptorSet;
	pvrvk::DescriptorSet dynamicDescriptorSet;

	// The renderpass used for rendering frames. The renderpass encapsulates the high level structure of a frame.
	pvrvk::RenderPass renderPass;

	// The framebuffer specifies a set of attachments used by the renderpass.
	pvrvk::Framebuffer framebuffers[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	// The depth stencil images and views used for rendering.
	pvrvk::ImageView depthStencilImageViews[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	// Synchronisation primitives used for specifying dependencies and ordering during rendering frames.
	pvrvk::Semaphore imageAcquireSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Semaphore presentationSemaphores[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];
	pvrvk::Fence perFrameResourcesFences[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	// The pvrvk::ImageView handle created for the triangle texture.
	pvrvk::ImageView triangleImageView;

	// The sampler handle used when sampling the triangle texture.
	pvrvk::Sampler bilinearSampler;

	// The commands buffers to which commands are rendered. The commands can then be submitted together.
	pvrvk::CommandBuffer cmdBuffers[static_cast<uint32_t>(pvrvk::FrameworkCaps::MaxSwapChains)];

	// The layout specifying the descriptors used by the graphics pipeline.
	pvrvk::PipelineLayout pipelineLayout;

	// The graphics pipeline specifying the funnel for which certain sets of Vulkan commands are sent through.
	pvrvk::GraphicsPipeline graphicsPipeline;

	// A pipeline cache providing mechanism for the reuse of the results of pipeline creation.
	pvrvk::PipelineCache pipelineCache;

	// The shader modules used by the graphics pipeline.
	pvrvk::ShaderModule vertexShaderModule;
	pvrvk::ShaderModule fragmentShaderModule;

	~DeviceResources()
	{
		if (device) { device->waitIdle(); }
		uint32_t l = swapchain->getSwapchainLength();
		for (uint32_t i = 0; i < l; ++i)
		{
			if (perFrameResourcesFences[i]) perFrameResourcesFences[i]->wait();
		}
	}
};

/// <summary>VulkanIntroducingPVRVk is the main demo class implementing the pvr::Shell functionality required for rendering to the screen.
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
/// now use the shell to create a "Hello triangle" app (VulkanIntroducingPVRVk), with the end result being similar to what was shown in VulkanHelloApi.</summary>
class VulkanIntroducingPVRVk : public pvr::Shell
{
	// A convenient way to store the Vulkan resources so that they can be initialised and freed automatically
	std::unique_ptr<DeviceResources> _deviceResources;

	// Per frame indices used for synchronisation.
	uint32_t _currentFrameIndex;

	// The index into the set of supported queue families which supports both graphics and presentation capabilities.
	uint32_t _graphicsQueueFamilyIndex;

	// The aligned size for the dynamic buffers taking into account the minUniformBufferOffsetAlignment member of the limits for the pvrvk::PhysicalDeviceProperties structure.
	uint32_t _dynamicBufferAlignedSize;

	// Matrices used for animation.
	glm::mat4 _modelMatrix;
	glm::mat4 _viewProjectionMatrix;
	float _rotationAngle;

	// The viewport and scissors used for rendering handling the portions of the surface written to.
	pvrvk::Viewport _viewport;
	pvrvk::Rect2D _scissor;

	// The size of a single vertex corresponding to the stride of a vertex.
	uint32_t _vboStride;

	// The size and data included in the triangle texture.
	pvrvk::Extent2D _textureDimensions;
	std::vector<uint8_t> _textureData;

	// Records the number of debug utils messengers created by the application.
	uint32_t _numDebugUtilsMessengers;

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
	void initDebugUtilsCallbacks();
	void createSurface(void* window, void* display, void* connection);
	void createLogicalDevice();
	void createSwapchain();
	void createDepthStencilImages();
	void createRenderPass();
	void createFramebuffer();
	void createSynchronisationPrimitives();
	void createCommandPool();
	pvrvk::ShaderModule createShaderModule(const char* const shaderName);
	pvrvk::DeviceMemory allocateDeviceMemory(
		const pvrvk::MemoryRequirements& memoryRequirements, const pvrvk::MemoryPropertyFlags requiredMemFlags, const pvrvk::MemoryPropertyFlags optimalMemFlags);
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
	pvrvk::Buffer createBufferAndAllocateMemory(
		pvrvk::DeviceSize size, pvrvk::BufferUsageFlags usageFlags, pvrvk::MemoryPropertyFlags requiredMemFlags, pvrvk::MemoryPropertyFlags optimalMemFlags);
	uint32_t getCompatibleQueueFamily();

	/// <summary>Default constructor for VulkanIntroducingPVRVk used to initialise the variables used throughout the demo.</summary>
	VulkanIntroducingPVRVk()
		: // initialise variables used for animation
		  _modelMatrix(glm::mat4(1)), _viewProjectionMatrix(glm::mat4(1)), _rotationAngle(45.0f),

		  // initialise the other variables used throughout the demo
		  _currentFrameIndex(0), _viewport(), _scissor(), _graphicsQueueFamilyIndex(-1), _vboStride(-1), _dynamicBufferAlignedSize(-1), _textureDimensions(), _textureData(0),
		  _numDebugUtilsMessengers(0), _numDebugCallbacks(0)
	{}
};

/// <summary>Code in initApplication() will be called by Shell once per run, before the rendering context is created.
/// Used to initialize variables that are not dependent on it(e.g.external modules, loading meshes, etc.).If the rendering
/// context is lost, initApplication() will not be called again.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanIntroducingPVRVk::initApplication()
{
	setBackBufferColorspace(pvr::ColorSpace::lRGB);
	// Here we are setting the backbuffer colorspace value to lRGB for simplicity: We are working directly with the "final" sRGB
	// values in our textures and passing the values through.
	// Note, the default for PVRShell is sRGB: when doing anything but the most simplistic effects, you will need to
	// work with linear values in the shaders and then either perform gamma correction in the shader, or (if supported)
	// use an sRGB framebuffer (which performs this correction automatically).
	return pvr::Result::Success;
}

/// <summary>Code in initView() will be called by Shell upon initialization or after a change  in the rendering context. Used to initialize variables that are dependent on the
/// rendering context(e.g.textures, vertex buffers, etc.).</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanIntroducingPVRVk::initView()
{
	_deviceResources = std::make_unique<DeviceResources>();

	// Create the Vulkan instance object, initialise the Vulkan library and initialise the Vulkan instance function pointers
	createInstance();

#ifdef DEBUG
	// If supported enable the use of VkDebugUtilsMessengers from VK_EXT_debug_utils if supported else VkDebugReportCallbacks from VK_EXT_debug_report if supported
	// to enable logging of various validation layer messages.
	initDebugUtilsCallbacks();
#endif

	// Create the various Vulkan resources and objects used throughout this demo
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
	_viewport.setWidth(static_cast<float>(getWidth()));
	_viewport.setHeight(static_cast<float>(getHeight()));
	_viewport.setMinDepth(0.0f);
	_viewport.setMaxDepth(1.0f);
	_viewport.setX(0);
	_viewport.setY(0);

	// Set the extent to the surface dimensions and the offset to 0
	_scissor.setExtent(pvrvk::Extent2D(static_cast<uint32_t>(getWidth()), static_cast<uint32_t>(getHeight())));
	_scissor.setOffset(pvrvk::Offset2D(0, 0));

	createPipelineLayout();
	createPipeline();

	// we can destroy the shader modules after creating the pipeline.
	_deviceResources->vertexShaderModule.reset();
	_deviceResources->fragmentShaderModule.reset();

	// Allocate and record the various Vulkan commands to a set of command buffers.
	// Work is prepared, being validated during development, upfront and is buffered up and ready to go.
	// Each frame the pre validated, pre prepared work is submitted.
	allocateCommandBuffers();
	recordCommandBuffers();

	return pvr::Result::Success;
}

/// <summary>Main rendering loop function of the program. The shell will call this function every frame.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanIntroducingPVRVk::renderFrame()
{
	// As discussed in "createSwapchain", the application doesn't actually "own" the presentation images meaning they cannot "just" render to the image
	// but must acquire an image from the presentation engine prior to making use of it. The act of acquiring an image from the presentation engine guarantees that
	// the presentation engine has completely finished with the image.

	// As with various other tasks in Vulkan rendering an image and presenting it to the screen takes some explanation, various commands and a fair amount of thought.

	// We are using a "canonical" way to do synchronization that works in all but the most exotic of cases.
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
	_deviceResources->swapchain->acquireNextImage(uint64_t(-1), _deviceResources->imageAcquireSemaphores[_currentFrameIndex]);

	//
	// 2). Wait for the per frame resources fence to have been signalled meaning the resources/command buffers for the current virtual frame are finished with.
	//
	// Wait for the command buffer from swapChainLength frames ago to be finished with.
	_deviceResources->perFrameResourcesFences[_deviceResources->swapchain->getSwapchainIndex()]->wait();
	_deviceResources->perFrameResourcesFences[_deviceResources->swapchain->getSwapchainIndex()]->reset();

	// Update the model view projection buffer data
	{
		// Update our angle of rotation
		_rotationAngle += 0.02f;

		// Calculate the model matrix making use of the rotation angle
		_modelMatrix = glm::rotate(_rotationAngle, glm::vec3(0.0f, 0.0f, 1.0f));

		// Set the model view projection matrix
		const auto modelViewProjectionMatrix = _viewProjectionMatrix * _modelMatrix;

		// Update the model view projection matrix buffer data for the current swapchain index. Note that the memory for the whole buffer was mapped just after it was allocated
		// so care needs to be taken to only modify memory to use with the current swapchain. Other slices of the memory may still be in use.

		memcpy(static_cast<unsigned char*>(_deviceResources->modelViewProjectionBuffer->getDeviceMemory()->getMappedData()) +
				_dynamicBufferAlignedSize * _deviceResources->swapchain->getSwapchainIndex(),
			&modelViewProjectionMatrix, sizeof(_viewProjectionMatrix));

		// If the model view projection buffer memory was allocated with pvrvk::MemoryPropertyFlags including pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT indicating that
		// the host does not need to manage the memory accesses explicitly using the host cache management commands vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges
		// to flush host writes to the device meaning we can safely assume writes have taken place prior to making use of the model view projection buffer memory.
		if ((_deviceResources->modelViewProjectionBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			// Flush the memory guaranteeing that host writes to the memory ranges specified are made available to the device.
			_deviceResources->modelViewProjectionBuffer->getDeviceMemory()->flushRange(
				_dynamicBufferAlignedSize * _deviceResources->swapchain->getSwapchainIndex(), sizeof(_viewProjectionMatrix));
		}
	}

	//
	// 3). Render the image (update variables, vkQueueSubmit). We are using per swapchain pre-recorded command buffers so we only need to submit them on each frame.
	//
	// Submit the specified command buffer to the given queue.
	// The queue submission will wait on the corresponding image acquisition semaphore to have been signalled.
	// The queue submission will signal the corresponding image presentation semaphore.
	// The queue submission will signal the corresponding per frame command buffer fence.
	pvrvk::SubmitInfo submitInfo;
	pvrvk::PipelineStageFlags pipeWaitStageFlags = pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT;
	submitInfo.commandBuffers = &_deviceResources->cmdBuffers[_deviceResources->swapchain->getSwapchainIndex()];
	submitInfo.numCommandBuffers = 1;
	submitInfo.waitSemaphores = &_deviceResources->imageAcquireSemaphores[_currentFrameIndex];
	submitInfo.numWaitSemaphores = 1;
	submitInfo.signalSemaphores = &_deviceResources->presentationSemaphores[_currentFrameIndex];
	submitInfo.numSignalSemaphores = 1;
	submitInfo.waitDstStageMask = &pipeWaitStageFlags;
	_deviceResources->queue->submit(&submitInfo, 1, _deviceResources->perFrameResourcesFences[_deviceResources->swapchain->getSwapchainIndex()]);

	//
	// 4). Present the acquired and now rendered image. Presenting an image returns ownership of the image back to the presentation engine.
	//
	// Queues the current swapchain image for presentation.
	// The queue presentation will wait on the corresponding image presentation semaphore.
	pvrvk::PresentInfo presentInfo;
	presentInfo.swapchains = &_deviceResources->swapchain;
	presentInfo.numSwapchains = 1;
	presentInfo.waitSemaphores = &_deviceResources->presentationSemaphores[_currentFrameIndex];
	presentInfo.numWaitSemaphores = 1;
	presentInfo.imageIndices = &_deviceResources->swapchain->getSwapchainIndex();
	_deviceResources->queue->present(presentInfo);

	//
	// 5). Increment (and wrap) the virtual frame index
	//
	_currentFrameIndex = (_currentFrameIndex + 1) % _deviceResources->swapchain->getSwapchainLength();

	return pvr::Result::Success;
}

/// <summary>Code in releaseView() will be called by Shell when the application quits.</summary>
/// <returns>Result::Success if no error occurred.</returns>
pvr::Result VulkanIntroducingPVRVk::releaseView()
{
	// Cleanly release all resources prior to exiting the application.
	_deviceResources.reset();
	return pvr::Result::Success;
}

/// <summary>Code in quitApplication() will be called by pvr::Shell once per run, just before exiting the program.</summary>
/// <returns>Result::Success if no error occurred</returns>.
pvr::Result VulkanIntroducingPVRVk::quitApplication() { return pvr::Result::Success; }

/// <summary>Create the Vulkan application instance.</summary>
void VulkanIntroducingPVRVk::createInstance()
{
	// Initialise Vulkan by loading the Vulkan commands and creating the pvrvk::Instance.

	uint32_t major = -1;
	uint32_t minor = -1;
	uint32_t patch = -1;

	// We make use of vk_bindings.h and vk_bindings_helper.h for defining and initialising the Vulkan function pointer tables by calling pvrvk::getVkBindings()
	// Vulkan commands aren't all necessarily exposed statically on the target platform however all Vulkan commands can be retrieved using vkGetInstanceProcAddr.
	// In vk_bindings_helper.h the function pointer for vkGetInstanceProcAddr is obtained using GetProcAddress, dlsym etc.
	// The function pointer vkGetInstanceProcAddr is then used  to retrieve the following additional Vulkan commands:
	// vkEnumerateInstanceExtensionProperties, vkEnumerateInstanceLayerProperties and vkCreateInstance

	// If a valid function pointer for vkEnumerateInstanceVersion cannot be retrieved then Vulkan only 1.0 is supported by the implementation otherwise we can use
	// vkEnumerateInstanceVersion to determine the API version supported.
	if (pvrvk::getVkBindings().vkEnumerateInstanceVersion)
	{
		uint32_t supportedApiVersion;
		pvrvk::getVkBindings().vkEnumerateInstanceVersion(&supportedApiVersion);

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

	// Create the application creation info structure, specifying the application name and the intended Vulkan API version to be used.
	pvrvk::ApplicationInfo applicationInfo("VulkanIntroducingPVRVk", 1, "VulkanIntroducingPVRVk", 1, VK_MAKE_VERSION(major, minor, patch));

	// Create the instance creation info structure
	pvrvk::InstanceCreateInfo instanceCreateInfo(applicationInfo);

	// Print out the supported instance extensions
	std::vector<pvrvk::ExtensionProperties> extensionProperties;
	pvrvk::Extensions::enumerateInstanceExtensions(extensionProperties);

	Log(LogLevel::Information, "Supported Instance Extensions:");
	for (uint32_t i = 0; i < static_cast<uint32_t>(extensionProperties.size()); ++i)
	{ Log(LogLevel::Information, "\t%s : version [%u]", extensionProperties[i].getExtensionName(), extensionProperties[i].getSpecVersion()); }

	// Retrieve a list of supported instance extensions and filter them based on a set of requested instance extension to be enabled.
	InstanceExtensions instanceExtensions;
	if (instanceExtensions.getNumExtensions())
	{
		instanceCreateInfo.setExtensionList(pvrvk::Extensions::filterExtensions(extensionProperties, instanceExtensions));

		Log(LogLevel::Information, "Supported Instance Extensions to be Enabled:");
		for (uint32_t i = 0; i < instanceCreateInfo.getExtensionList().getNumExtensions(); ++i)
		{ Log(LogLevel::Information, "\t%s", instanceCreateInfo.getExtensionList().getExtension(i).getName().c_str()); }
	}

	// Vulkan, by nature of its minimalistic design, provides very little information to the developer regarding API issues. Error checking and validation of state is minimal.
	// One of the key principles of Vulkan is that the preparation and submission of work should be highly efficient; removing error checking and validation of state from
	// Vulkan implementations is one of the many ways in which this was enabled. Vulkan is a layered API whereby it can optionally make use of additional layers for debugging,
	// validation and other purposes with the core Vulkan layer being the lowest in the stack.

	// Generally implementations assume applications are using the Vulkan API correctly. When an application uses the Vulkan incorrectly core Vulkan may behave in
	// undefined ways including through program termination.

	// Generally the validation of correct API usage is left to a set of validation layers.
	// Applications should be developed using these validation layers extensively to help identify and fix errors however once applications are validated applications should
	// disable the validation layers prior to being released.

	// This application makes use of The Khronos Vulkan-LoaderAndValidationLayers: https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers
	// Other layers exist for various other reasons such as VK_LAYER_POWERVR_carbon and VK_LAYER_ARM_mali_perf_doc.
	std::vector<pvrvk::LayerProperties> layerProperties;
	pvrvk::Layers::enumerateInstanceLayers(layerProperties);

	Log(LogLevel::Information, "Supported Instance Layers:");
	for (uint32_t i = 0; i < static_cast<uint32_t>(layerProperties.size()); ++i)
	{
		Log(LogLevel::Information, "\t%s : Spec version [%u], Implementation version [%u]", layerProperties[i].getLayerName(), layerProperties[i].getSpecVersion(),
			layerProperties[i].getImplementationVersion());
	}

	InstanceLayers layers;
	if (layers.getNumLayers())
	{
		pvrvk::VulkanLayerList supportedLayers = pvrvk::Layers::filterLayers(layerProperties, layers);

		std::string standardValidationLayerString = "VK_LAYER_LUNARG_standard_validation";

		bool requestedStandardValidation = layers.containsLayer(standardValidationLayerString);
		bool supportsStandardValidation = supportedLayers.containsLayer(standardValidationLayerString);
		bool supportsKhronosValidation = supportedLayers.containsLayer("VK_LAYER_KHRONOS_validation");

		uint32_t stdValidationRequiredIndex = -1;

		// This code is to cover cases where VK_LAYER_LUNARG_standard_validation is requested but is not supported, where on some platforms the
		// component layers enabled via VK_LAYER_LUNARG_standard_validation may still be supported even though VK_LAYER_LUNARG_standard_validation is not.
		// Only perform the expansion if VK_LAYER_LUNARG_standard_validation is requested and not supported and the newer equivalent layer VK_LAYER_KHRONOS_validation is also not supported
		if (requestedStandardValidation && !supportsStandardValidation && !supportsKhronosValidation)
		{
			for (auto it = layerProperties.begin(); !supportsStandardValidation && it != layerProperties.end(); ++it)
			{ supportsStandardValidation = !strcmp(it->getLayerName(), "VK_LAYER_LUNARG_standard_validation"); }
			if (!supportsStandardValidation)
			{
				for (uint32_t i = 0; stdValidationRequiredIndex == static_cast<uint32_t>(-1) && i < layerProperties.size(); ++i)
				{
					if (!strcmp(layers.getLayer(i).getName().c_str(), "VK_LAYER_LUNARG_standard_validation")) { stdValidationRequiredIndex = i; }
				}

				for (uint32_t j = 0; j < layers.getNumLayers(); ++j)
				{
					if (stdValidationRequiredIndex == j && !supportsStandardValidation)
					{
						const char* stdValComponents[] = { "VK_LAYER_GOOGLE_threading", "VK_LAYER_LUNARG_parameter_validation", "VK_LAYER_LUNARG_object_tracker",
							"VK_LAYER_LUNARG_core_validation", "VK_LAYER_GOOGLE_unique_objects" };
						for (uint32_t k = 0; k < sizeof(stdValComponents) / sizeof(stdValComponents[0]); ++k)
						{
							for (uint32_t i = 0; i < layerProperties.size(); ++i)
							{
								if (!strcmp(stdValComponents[k], layerProperties[i].getLayerName()))
								{
									supportedLayers.addLayer(pvrvk::VulkanLayer(std::string(stdValComponents[k])));
									break;
								}
							}
						}
					}
				}

				// filter the layers again checking for support for the component layers enabled via VK_LAYER_LUNARG_standard_validation
				supportedLayers = pvrvk::Layers::filterLayers(layerProperties, supportedLayers);
			}
		}

		instanceCreateInfo.setLayerList(supportedLayers);

		Log(LogLevel::Information, "Supported Instance Layers to be Enabled:");
		for (uint32_t i = 0; i < instanceCreateInfo.getLayerList().getNumLayers(); ++i)
		{
			Log(LogLevel::Information, "\t%s : Spec version [%u], Spec version [%u]", instanceCreateInfo.getLayerList().getLayer(i).getName().c_str(),
				instanceCreateInfo.getLayerList().getLayer(i).getSpecVersion(), instanceCreateInfo.getLayerList().getLayer(i).getImplementationVersion());
		}
	}

	_deviceResources->instance = pvrvk::createInstance(instanceCreateInfo);
	_deviceResources->instance->retrievePhysicalDevices();
}

/// <summary>Creates Debug Report Callbacks which will provide .</summary>
void VulkanIntroducingPVRVk::initDebugUtilsCallbacks()
{
	// Create debug utils messengers using the VK_EXT_debug_utils extension providing a way for the Vulkan layers and the implementation itself to call back to the application
	// in particular circumstances.
	if (_deviceResources->instance->getEnabledExtensionTable().extDebugUtilsEnabled)
	{
		Log(LogLevel::Information, "Creating VkDebugUtilsMessengerEXT using VK_EXT_debug_utils");

		// Create a Debug Utils Messenger which will trigger our callback for logging messages for events of warning and error types of all severities
		pvrvk::DebugUtilsMessengerCreateInfo createInfo(
			pvrvk::DebugUtilsMessengerCreateInfo(pvrvk::DebugUtilsMessageSeverityFlagsEXT::e_ERROR_BIT_EXT | pvrvk::DebugUtilsMessageSeverityFlagsEXT::e_WARNING_BIT_EXT,
				pvrvk::DebugUtilsMessageTypeFlagsEXT::e_ALL_BITS, logMessageDebugUtilsMessengerCallback));

		_deviceResources->debugUtilsMessengers[0] = _deviceResources->instance->createDebugUtilsMessenger(createInfo);

		// Create a second Debug Utils Messenger for throwing exceptions for Error events.
		createInfo.setMessageSeverity(pvrvk::DebugUtilsMessageSeverityFlagsEXT::e_ERROR_BIT_EXT);
		createInfo.setCallback(throwOnErrorDebugUtilsMessengerCallback);

		_deviceResources->debugUtilsMessengers[1] = _deviceResources->instance->createDebugUtilsMessenger(createInfo);

		_numDebugUtilsMessengers = 2;
	}
	// Create debug report callbacks using the VK_EXT_debug_report extension providing a way for the Vulkan layers and the implementation itself to call back to the application
	// in particular circumstances.
	else if (_deviceResources->instance->getEnabledExtensionTable().extDebugReportEnabled)
	{
		Log(LogLevel::Information, "Creating VkDebugReportCallbackEXT using VK_EXT_debug_report");

		pvrvk::DebugReportCallbackCreateInfo createInfo(pvrvk::DebugReportFlagsEXT::e_ERROR_BIT_EXT | pvrvk::DebugReportFlagsEXT::e_WARNING_BIT_EXT |
				pvrvk::DebugReportFlagsEXT::e_PERFORMANCE_WARNING_BIT_EXT | pvrvk::DebugReportFlagsEXT::e_DEBUG_BIT_EXT,
			logMessageDebugReportCallback);

		// Register the first callback which logs messages of all pvrvk::DebugReportFlagsEXT types.
		_deviceResources->debugReportCallbacks[0] = _deviceResources->instance->createDebugReportCallback(createInfo);

		// Register the second callback which throws exceptions when events of type VK_DEBUG_REPORT_ERROR_BIT_EXT occur.
		createInfo.setFlags(pvrvk::DebugReportFlagsEXT::e_ERROR_BIT_EXT);
		createInfo.setCallback(throwOnErrorDebugReportCallback);

		// Register the callback
		_deviceResources->debugReportCallbacks[1] = _deviceResources->instance->createDebugReportCallback(createInfo);
		_numDebugCallbacks = 2;
	}
}

/// <summary>Creates the surface used by the demo.</summary>
/// <param name="window">A platform agnostic window.</param>
/// <param name="display">A platform agnostic display.</param>
/// <param name="connection">A platform agnostic connection.</param>
void VulkanIntroducingPVRVk::createSurface(void* window, void* display, void* connection)
{
	// Create the native platform surface abstracted via a VkSurfaceKHR object which this application will make use of in particular with the VK_KHR_swapchain extension.
	// Applications may also, on some platforms, present rendered images directly to display devices without the need for an intermediate Window System. The extension
	// VK_KHR_display in particular can be used for this task.

	// In Vulkan each platform may require unique window integration steps and therefore allows for an abstracted platform independent surface to be created.
	// To facilitate this, each platform provides its own Window System Integration (WSI) extension containing platform specific functions for using their own WSI.
	// Vulkan requires that the use of these extensions is guarded by preprocessor symbols defined in Vulkan's Window System-Specific Header Control appendix.
	// For VulkanIntroducingPVRVk to appropriately make use of the WSI extensions for a given platform it must #define the appropriate symbols for the platform prior to
	// including Vulkan.h header file. The appropriate set of preprocessor symbols are defined at the top of this file based on a set of compilation flags used to compile this demo.

	// Note that each WSI extension must be appropriately enabled as an instance extension prior to using them. This is controlled via the use of the array
	// Extensions::InstanceExtensions which is constructed at compile time based on the same set of compilation flags described above.

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	// Creates a pvrvk::Surface object for an Android native window
	_deviceResources->surface = pvrvk::Surface(_deviceResources->instance->createAndroidSurface(reinterpret_cast<ANativeWindow*>(window)));
#elif defined VK_USE_PLATFORM_WIN32_KHR
	(void)connection;
	(void)display;
	// Creates a pvrvk::Surface object for a Win32 window
	_deviceResources->surface = pvrvk::Surface(_deviceResources->instance->createWin32Surface(GetModuleHandle(NULL), static_cast<HWND>(window)));
#elif defined(VK_USE_PLATFORM_XCB_KHR)
	// Creates a pvrvk::Surface object for an XCB window, using the XCB client-side library.
	_deviceResources->surface = pvrvk::Surface(_deviceResources->instance->createXcbSurface(static_cast<xcb_connection_t*>(connection), *((xcb_window_t*)(&window))));
#elif defined(VK_USE_PLATFORM_XLIB_KHR)
	// Creates a pvrvk::Surface object for an X11 window, using the Xlib client-side library.
	_deviceResources->surface = pvrvk::Surface(_deviceResources->instance->createXlibSurface(static_cast<Display*>(display), reinterpret_cast<Window>(window)));
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
	// Creates a pvrvk::Surface object for a Wayland surface.
	_deviceResources->surface = pvrvk::Surface(_deviceResources->instance->createWaylandSurface(reinterpret_cast<wl_display*>(display), reinterpret_cast<wl_surface*>(window)));
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
	// Creates a pvrvk::Surface object for an MacOS Surface.
	_deviceResources->surface = pvrvk::Surface(_deviceResources->instance->createMacOSSurface(window));
#else
	Log("%u Displays supported by the physical device", _deviceResources->instance->getPhysicalDevice(0)->getNumDisplays());
	Log("Display properties:");

	for (uint32_t i = 0; i < _deviceResources->instance->getPhysicalDevice(0)->getNumDisplays(); ++i)
	{
		const pvrvk::Display& display = _deviceResources->instance->getPhysicalDevice(0)->getDisplay(i);
		Log("Properties for Display [%u]:", i);
		Log("	Display Name: '%s':", display->getDisplayName());
		Log("	Supports Persistent Content: %u", display->getPersistentContent());
		Log("	Physical Dimensions: (%u, %u)", display->getPhysicalDimensions().getWidth(), display->getPhysicalDimensions().getHeight());
		Log("	Physical Resolution: (%u, %u)", display->getPhysicalResolution().getWidth(), display->getPhysicalResolution().getHeight());
		Log("	Supported Transforms: %s", pvrvk::to_string(display->getSupportedTransforms()).c_str());
		Log("	Supports Plane Reorder: %u", display->getPlaneReorderPossible());

		Log("	Display supports [%u] display modes:", display->getNumDisplayModes());
		for (uint32_t j = 0; j < display->getNumDisplayModes(); ++j)
		{
			Log("	Properties for Display Mode [%u]:", j);
			const pvrvk::DisplayMode& displayMode = display->getDisplayMode(j);
			Log("		Refresh Rate: %f", displayMode->getParameters().getRefreshRate());
			Log("		Visible Region: (%u, %u)", displayMode->getParameters().getVisibleRegion().getWidth(), displayMode->getParameters().getVisibleRegion().getHeight());
		}
	}

	if (_deviceResources->instance->getPhysicalDevice(0)->getNumDisplays() == 0) { throw pvrvk::ErrorInitializationFailed("Could not find a suitable Vulkan Display."); }

	// We simply loop through the display planes and find a supported display and display mode
	for (uint32_t i = 0; i < _deviceResources->instance->getPhysicalDevice(0)->getNumDisplayPlanes(); ++i)
	{
		uint32_t currentStackIndex = -1;
		pvrvk::Display display = _deviceResources->instance->getPhysicalDevice(0)->getDisplayPlaneProperties(i, currentStackIndex);
		std::vector<pvrvk::Display> supportedDisplaysForPlane = _deviceResources->instance->getPhysicalDevice(0)->getDisplayPlaneSupportedDisplays(i);
		pvrvk::DisplayMode displayMode;

		// if a valid display can be found and its supported then make use of it
		if (display && std::find(supportedDisplaysForPlane.begin(), supportedDisplaysForPlane.end(), display) != supportedDisplaysForPlane.end())
		{ displayMode = display->getDisplayMode(0); } // else find the first supported display and grab its first display mode
		else if (supportedDisplaysForPlane.size())
		{
			pvrvk::Display& currentDisplay = supportedDisplaysForPlane[0];
			displayMode = currentDisplay->getDisplayMode(0);
		}

		if (displayMode)
		{
			pvrvk::DisplayPlaneCapabilitiesKHR capabilities = _deviceResources->instance->getPhysicalDevice(0)->getDisplayPlaneCapabilities(displayMode, i);
			Log("Capabilities for the chosen display mode for Display Plane [%u]:", i);
			Log("	Supported Alpha Flags: %s", pvrvk::to_string(capabilities.getSupportedAlpha()).c_str());
			Log("	Supported Min Src Position: (%u, %u)", capabilities.getMinSrcPosition().getX(), capabilities.getMinSrcPosition().getY());
			Log("	Supported Max Src Position: (%u, %u)", capabilities.getMaxSrcPosition().getX(), capabilities.getMaxSrcPosition().getY());
			Log("	Supported Min Src Extent: (%u, %u)", capabilities.getMinSrcExtent().getWidth(), capabilities.getMinSrcExtent().getHeight());
			Log("	Supported Max Src Extent: (%u, %u)", capabilities.getMaxSrcExtent().getWidth(), capabilities.getMaxSrcExtent().getHeight());
			Log("	Supported Min Dst Position: (%u, %u)", capabilities.getMinDstPosition().getX(), capabilities.getMinDstPosition().getY());
			Log("	Supported Max Dst Position: (%u, %u)", capabilities.getMaxDstPosition().getX(), capabilities.getMaxDstPosition().getY());
			Log("	Supported Min Dst Extent: (%u, %u)", capabilities.getMinDstExtent().getWidth(), capabilities.getMinDstExtent().getHeight());
			Log("	Supported Max Dst Extent: (%u, %u)", capabilities.getMaxDstExtent().getWidth(), capabilities.getMaxDstExtent().getHeight());

			_deviceResources->surface = pvrvk::Surface(_deviceResources->instance->createDisplayPlaneSurface(
				displayMode, displayMode->getParameters().getVisibleRegion(), pvrvk::DisplaySurfaceCreateFlagsKHR::e_NONE, i, currentStackIndex));
		}
	}
#endif
}

/// <summary>Get the compatible queue families from the device selected.</summary>
uint32_t VulkanIntroducingPVRVk::getCompatibleQueueFamily()
{
	// Attempts to retrieve a queue family which supports both graphics and presentation for the given application surface. This application has been
	// written in such a way which requires that the graphics and presentation queue families match.
	// Not all physical devices will support Window System Integration (WSI) support furthermore not all queue families for a particular physical device will support
	// presenting to the screen and thus these capabilities must be separately queried for support.

	// Retrieves the queue family properties for the queue families the physical device supports.
	const std::vector<pvrvk::QueueFamilyProperties>& queueFamilyProperties = _deviceResources->instance->getPhysicalDevice(0)->getQueueFamilyProperties();

	// For each queue family query whether it supports presentation and ensure the same queue family also supports graphics capabilities.
	for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i)
	{
		if (_deviceResources->instance->getPhysicalDevice(0)->getSurfaceSupport(i, _deviceResources->surface))
		{
			if (((queueFamilyProperties[i].getQueueFlags() & pvrvk::QueueFlags::e_GRAPHICS_BIT) != 0)) { return i; }
		}
	}

	throw pvr::PvrError("Could not find a compatible queue family supporting both graphics capabilities and presentation to the screen");
}

/// <summary>Create the logical device.</summary>
void VulkanIntroducingPVRVk::createLogicalDevice()
{
	// Create the logical device used throughout the demo.

	// Logical devices represent logical connections to underlying physical devices.
	// A logical device provides main interface for an application to access the resources of the physical device and the physical device itself including:
	//		Creation of queues.
	//		Creation and management of synchronization primitives.
	//		Allocation, release and management of memory.
	//		Creation and destruction of command buffers and command buffer pools.
	//		Creation, management and destruction of other graphics state including pipelines and resource descriptors.
	// Note that each physical device may correspond to multiple logical devices each of which specifying different extensions, capabilities and queues.

	// As part of logical device creation the application may also provide a set of queues that are requested for creation along with the logical device.
	// This application simply requests the creation of a single queue, from a single queue family specified by passing a single pvrvk::DeviceQueueCreateInfo structure
	// to the device creation structure as pQueueCreateInfos.

	// Attempt to find a suitable queue family which supports both Graphics and presentation.
	_graphicsQueueFamilyIndex = getCompatibleQueueFamily();

	// Queues are each assigned priorities ranging from 0.0 - 1.0 with higher priority queues having the potential to be allotted more processing time than queues with lower
	// priority although queue scheduling is completely implementation dependent.
	// Note that there are no guarantees about higher priority queues receiving more processing time or better quality of service than lower priority queues.
	// Also note that in our case we only have one queue so the priority specified doesn't matter.
	std::vector<float> queuePriorities;
	queuePriorities.push_back(1.0f);

	std::vector<pvrvk::DeviceQueueCreateInfo> queueCreateInfos = { pvrvk::DeviceQueueCreateInfo(_graphicsQueueFamilyIndex, queuePriorities) };

	pvrvk::DeviceCreateInfo deviceCreateInfo;
	deviceCreateInfo.setDeviceQueueCreateInfos(queueCreateInfos);

	// Another important part of logical device creation is the specification of any required device extensions to enable. As described above "Extensions" in
	// Vulkan extensions may provide additional functionality not included in or used by Core Vulkan. The set of device specific extensions to enable are defined
	// by Extensions::DeviceExtensions.

	// Retrieve a list of supported device extensions and filter them based on a set of requested instance extension to be enabled.

	// Print out the supported device extensions
	const std::vector<pvrvk::ExtensionProperties>& extensionProperties = _deviceResources->instance->getPhysicalDevice(0)->getDeviceExtensionsProperties();

	Log(LogLevel::Information, "Supported Device Extensions:");
	for (uint32_t i = 0; i < static_cast<uint32_t>(extensionProperties.size()); ++i)
	{ Log(LogLevel::Information, "\t%s : version [%u]", extensionProperties[i].getExtensionName(), extensionProperties[i].getSpecVersion()); }
	DeviceExtensions deviceExtensions;
	if (deviceExtensions.getNumExtensions())
	{
		deviceCreateInfo.setExtensionList(pvrvk::Extensions::filterExtensions(extensionProperties, deviceExtensions));

		if (deviceCreateInfo.getExtensionList().getNumExtensions() != deviceExtensions.getNumExtensions())
		{ Log(LogLevel::Warning, "Not all requested Logical device extensions are supported"); }
		Log(LogLevel::Information, "Supported Device Extensions:");
		for (uint32_t i = 0; i < deviceCreateInfo.getExtensionList().getNumExtensions(); ++i)
		{ Log(LogLevel::Information, "\t%s", deviceCreateInfo.getExtensionList().getExtension(i).getName().c_str()); }
	}

	// A physical device may well support a set of fine grained features which are not mandated by the specification, support for these features is retrieved and then enabled
	// feature by feature.
	pvrvk::PhysicalDeviceFeatures features = _deviceResources->instance->getPhysicalDevice(0)->getFeatures();

	// Ensure that robustBufferAccess is disabled
	features.setRobustBufferAccess(false);
	deviceCreateInfo.setEnabledFeatures(&features);

	_deviceResources->device = _deviceResources->instance->getPhysicalDevice(0)->createDevice(deviceCreateInfo);
	_deviceResources->device->retrieveQueues();

	// Get the queue
	_deviceResources->queue = _deviceResources->device->getQueue(_graphicsQueueFamilyIndex, 0);
}

/// <summary>Gets a set of corrected screen extents based on the surface's capabilities.</summary>
/// <param name="surfaceCapabilities">A set of capabilities for the application surface including min/max image counts and extents.</param>
/// <param name="width">Request window width which will be checked for compatibility with the surface.</param>
/// <param name="height">Request window height which will be checked for compatibility with the surface.</param>
/// <returns>A set of corrected window extents compatible with the application's surface's capabilities.</returns>
void correctWindowExtents(const pvrvk::SurfaceCapabilitiesKHR& surfaceCapabilities, pvr::DisplayAttributes& attr)
{
	// Retrieves a set of correct window extents based on the requested width, height and surface capabilities.
	if (attr.width == 0) { attr.width = surfaceCapabilities.getCurrentExtent().getWidth(); }
	if (attr.height == 0) { attr.height = surfaceCapabilities.getCurrentExtent().getHeight(); }

	attr.width = std::max<uint32_t>(surfaceCapabilities.getMinImageExtent().getWidth(), std::min<uint32_t>(attr.width, surfaceCapabilities.getMaxImageExtent().getWidth()));

	attr.height = std::max<uint32_t>(surfaceCapabilities.getMinImageExtent().getHeight(), std::min<uint32_t>(attr.height, surfaceCapabilities.getMaxImageExtent().getHeight()));
}

/// <summary>Selects the presentation mode to be used when creating the based on physical device surface presentation modes supported and
/// a preset orderer list of presentation modes.</summary>
/// <param name="modes">A list of presentation modes supported by the physical device surface.</param>
/// <param name="presentationMode">The chosen presentation mode will be returned by reference.</param>
/// <param name="displayAttributes">A set of display configuration parameters.</param>
void selectPresentMode(std::vector<pvrvk::PresentModeKHR>& modes, pvrvk::PresentModeKHR& presentationMode, pvr::DisplayAttributes& displayAttributes)
{
	// With pvrvk::PresentModeKHR::e_FIFO_KHR the presentation engine will wait for the next vblank (vertical blanking period) to update the current image. When using FIFO
	// tearing cannot occur. pvrvk::PresentModeKHR::e_FIFO_KHR is required to be supported.
	presentationMode = pvrvk::PresentModeKHR::e_FIFO_KHR;
	pvrvk::PresentModeKHR desiredSwapMode = pvrvk::PresentModeKHR::e_FIFO_KHR;

	// We make use of PVRShell for handling command line arguments for configuring vsync modes using the -vsync command line argument.
	switch (displayAttributes.vsyncMode)
	{
	case pvr::VsyncMode::Off: desiredSwapMode = pvrvk::PresentModeKHR::e_IMMEDIATE_KHR; break;
	case pvr::VsyncMode::Mailbox: desiredSwapMode = pvrvk::PresentModeKHR::e_MAILBOX_KHR; break;
	case pvr::VsyncMode::Relaxed:
		desiredSwapMode = pvrvk::PresentModeKHR::e_FIFO_RELAXED_KHR;
		break;
		// Default vsync mode.
	case pvr::VsyncMode::On: break;
	default: Log(LogLevel::Information, "Unexpected Vsync Mode specified. Defaulting to pvrvk::PresentModeKHR::e_FIFO_KHR");
	}

	// Verify that the desired presentation mode is present in the list of supported pvrvk::PresentModes.
	for (size_t i = 0; i < modes.size(); i++)
	{
		pvrvk::PresentModeKHR currentPresentMode = modes[i];

		// Primary matches : Check for a precise match between the desired presentation mode and the presentation modes supported.
		if (currentPresentMode == desiredSwapMode)
		{
			presentationMode = desiredSwapMode;
			break;
		}
		// Secondary matches : Immediate and Mailbox are better fits for each other than FIFO, so set them as secondary
		// If the user asked for Mailbox, and we found Immediate, set it (in case Mailbox is not found) and keep looking
		if ((desiredSwapMode == pvrvk::PresentModeKHR::e_MAILBOX_KHR) && (currentPresentMode == pvrvk::PresentModeKHR::e_IMMEDIATE_KHR))
		{ presentationMode = pvrvk::PresentModeKHR::e_IMMEDIATE_KHR; }
		// ... And vice versa: If the user asked for Immediate, and we found Mailbox, set it (in case Immediate is not found) and keep looking
		if ((desiredSwapMode == pvrvk::PresentModeKHR::e_IMMEDIATE_KHR) && (currentPresentMode == pvrvk::PresentModeKHR::e_MAILBOX_KHR))
		{ presentationMode = pvrvk::PresentModeKHR::e_MAILBOX_KHR; }
	}
	switch (presentationMode)
	{
	case pvrvk::PresentModeKHR::e_IMMEDIATE_KHR: Log(LogLevel::Information, "Presentation mode: Immediate (Vsync OFF)"); break;
	case pvrvk::PresentModeKHR::e_MAILBOX_KHR: Log(LogLevel::Information, "Presentation mode: Mailbox (Triple-buffering)"); break;
	case pvrvk::PresentModeKHR::e_FIFO_KHR: Log(LogLevel::Information, "Presentation mode: FIFO (Vsync ON)"); break;
	case pvrvk::PresentModeKHR::e_FIFO_RELAXED_KHR: Log(LogLevel::Information, "Presentation mode: Relaxed FIFO (Relaxed Vsync)"); break;
	default: assertion(false, "Unrecognised presentation mode"); break;
	}

	// Set the swapchain length if it has not already been set.
	if (!displayAttributes.swapLength) { displayAttributes.swapLength = 3; }
}

/// <summary>Creates swapchain to present images on the surface.</summary>
void VulkanIntroducingPVRVk::createSwapchain()
{
	// Creates the WSI Swapchain object providing the ability to present rendering results to the surface.

	// A swapchain provides the abstraction for a set of presentable images (pvrvk::Image objects), with a particular view (pvrvk::ImageView), associated with a surface
	// (VkSurfaceKHR), to be used for screen rendering.

	// The swapchain provides the necessary functionality for the application to explicitly handle multi buffering (double/triple buffering). The swapchain provides the
	// functionality to present a single image at a time but also allows the application to queue up other images for presentation. An application will render images and queue
	// them for presentation to the surface.

	// The physical device surface may well only support a certain set of pvrvk::Formats/pvrvk::ColorSpaceKHR pairs for the presentation images in their presentation engine.
	// Retrieve the number of pvrvk::Formats/pvrvk::ColorSpaceKHR pairs supported by the physical device surface.
	std::vector<pvrvk::SurfaceFormatKHR> surfaceFormats = _deviceResources->device->getPhysicalDevice()->getSurfaceFormats(_deviceResources->surface);

	// From the list of retrieved pvrvk::Formats/pvrvk::ColorSpaceKHR pairs supported by the physical device surface find one suitable from a list of preferred choices.
	pvrvk::SurfaceFormatKHR swapchainColorFormat;
	const uint32_t numPreferredColorFormats = 2;
	pvrvk::Format preferredColorFormats[numPreferredColorFormats] = { pvrvk::Format::e_R8G8B8A8_UNORM, pvrvk::Format::e_B8G8R8A8_UNORM };

	bool foundFormat = false;

	for (uint32_t i = 0; i < surfaceFormats.size() && !foundFormat; ++i)
	{
		for (uint32_t j = 0; j < numPreferredColorFormats; ++j)
		{
			if (surfaceFormats[i].getFormat() == preferredColorFormats[j])
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

	Log(LogLevel::Information, "Surface format selected: %s with colorspace %s", pvrvk::to_string(swapchainColorFormat.getFormat()).c_str(),
		pvrvk::to_string(swapchainColorFormat.getColorSpace()).c_str());

	pvrvk::SurfaceCapabilitiesKHR surfaceCapabilities = _deviceResources->device->getPhysicalDevice()->getSurfaceCapabilities(_deviceResources->surface);

	// Get a set of "corrected" extents (dimensions) for the surface window based on the pvr::Shell window width/height and surface capabilities.
	correctWindowExtents(surfaceCapabilities, this->getDisplayAttributes());

	// Retrieve the set of presentation modes supported by the physical device surface.
	std::vector<pvrvk::PresentModeKHR> surfacePresentationModes = _deviceResources->device->getPhysicalDevice()->getSurfacePresentModes(_deviceResources->surface);

	// Retrieve the pvr::DisplayAttributes from the pvr::Shell - the pvr::Shell pvr::DisplayAttributes will take into account any command line arguments used.
	pvr::DisplayAttributes& displayAttributes = getDisplayAttributes();

	// Create the swapchain info which will be used to create our swapchain.
	pvrvk::PresentModeKHR presentationMode = {};

	// Based on the pvr::DisplayAttributes, and supported presentation modes select a supported presentation mode.
	selectPresentMode(surfacePresentationModes, presentationMode, displayAttributes);

	// Check for a supported composite alpha value in a predefined order
	pvrvk::CompositeAlphaFlagsKHR supportedCompositeAlphaFlags = pvrvk::CompositeAlphaFlagsKHR::e_NONE;
	if ((surfaceCapabilities.getSupportedCompositeAlpha() & pvrvk::CompositeAlphaFlagsKHR::e_OPAQUE_BIT_KHR) != 0)
	{ supportedCompositeAlphaFlags = pvrvk::CompositeAlphaFlagsKHR::e_OPAQUE_BIT_KHR; }
	else if ((surfaceCapabilities.getSupportedCompositeAlpha() & pvrvk::CompositeAlphaFlagsKHR::e_INHERIT_BIT_KHR) != 0)
	{
		supportedCompositeAlphaFlags = pvrvk::CompositeAlphaFlagsKHR::e_INHERIT_BIT_KHR;
	}

	displayAttributes.swapLength = std::max<uint32_t>(displayAttributes.swapLength, surfaceCapabilities.getMinImageCount());
	if (surfaceCapabilities.getMaxImageCount()) { displayAttributes.swapLength = std::min<uint32_t>(displayAttributes.swapLength, surfaceCapabilities.getMaxImageCount()); }

	displayAttributes.swapLength = std::min<uint32_t>(displayAttributes.swapLength, pvrvk::FrameworkCaps::MaxSwapChains);

	if ((surfaceCapabilities.getSupportedTransforms() & pvrvk::SurfaceTransformFlagsKHR::e_IDENTITY_BIT_KHR) == 0)
	{ throw pvr::InvalidOperationError("Surface does not support pvrvk::SurfaceTransformFlagsKHR::e_IDENTITY_BIT_KHR transformation"); }
	pvrvk::SwapchainCreateInfo createInfo;
	createInfo.clipped = true;
	createInfo.compositeAlpha = supportedCompositeAlphaFlags;
	createInfo.surface = _deviceResources->surface;
	createInfo.minImageCount = displayAttributes.swapLength;
	createInfo.imageFormat = swapchainColorFormat.getFormat();
	createInfo.imageArrayLayers = 1;
	createInfo.imageColorSpace = swapchainColorFormat.getColorSpace();
	createInfo.imageExtent.setWidth(displayAttributes.width);
	createInfo.imageExtent.setHeight(displayAttributes.height);
	createInfo.imageUsage = pvrvk::ImageUsageFlags::e_COLOR_ATTACHMENT_BIT;
	createInfo.preTransform = pvrvk::SurfaceTransformFlagsKHR::e_IDENTITY_BIT_KHR;
	createInfo.imageSharingMode = pvrvk::SharingMode::e_EXCLUSIVE;
	createInfo.presentMode = presentationMode;
	createInfo.numQueueFamilyIndex = 1;
	uint32_t queueFamily = 0;
	createInfo.queueFamilyIndices = &queueFamily;

	_deviceResources->swapchain = _deviceResources->device->createSwapchain(createInfo, _deviceResources->surface);
}

/// <summary>Creates a set of pvrvk::Images and pvrvk::ImageViews which will be used as the depth/stencil buffers.</summary>
void VulkanIntroducingPVRVk::createDepthStencilImages()
{
	// Create swapchainLength pvrvk::Images and pvrvk::ImageViews which the application will use as depth stencil images.

	pvrvk::Format supportedDepthStencilFormat = pvrvk::Format::e_UNDEFINED;

	// Setup an ordered list of preferred pvrvk::Format to check for support when determining the format to use for the depth stencil images.
	pvrvk::Format preferredDepthStencilFormats[6] = {
		pvrvk::Format::e_D32_SFLOAT_S8_UINT,
		pvrvk::Format::e_D24_UNORM_S8_UINT,
		pvrvk::Format::e_D16_UNORM_S8_UINT,
		pvrvk::Format::e_D32_SFLOAT,
		pvrvk::Format::e_D16_UNORM,
		pvrvk::Format::e_X8_D24_UNORM_PACK32,
	};

	std::vector<pvrvk::Format> depthFormats;
	depthFormats.insert(depthFormats.begin(), &preferredDepthStencilFormats[0], &preferredDepthStencilFormats[6]);

	for (uint32_t i = 0; i < depthFormats.size(); ++i)
	{
		pvrvk::Format currentDepthStencilFormat = depthFormats[i];

		// In turn check the physical device as to whether it supports the pvrvk::Format in preferredDepthStencilFormats.
		pvrvk::FormatProperties prop = _deviceResources->device->getPhysicalDevice()->getFormatProperties(currentDepthStencilFormat);

		// Ensure that the format supports pvrvk::ImageTiling::e_OPTIMAL. Optimal tiling specifies that texels are laid out in an implementation dependent arrangement providing
		// more optimal memory access.
		if ((prop.getOptimalTilingFeatures() & pvrvk::FormatFeatureFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT) != 0)
		{
			supportedDepthStencilFormat = currentDepthStencilFormat;
			break;
		}
	}

	// the required memory property flags
	const pvrvk::MemoryPropertyFlags requiredMemoryProperties = pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT;

	// more optimal set of memory property flags
	const pvrvk::MemoryPropertyFlags optimalMemoryProperties = pvrvk::MemoryPropertyFlags::e_LAZILY_ALLOCATED_BIT;

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		pvrvk::ImageCreateInfo createInfo = pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, supportedDepthStencilFormat, pvrvk::Extent3D(getWidth(), getHeight(), 1u),
			pvrvk::ImageUsageFlags::e_DEPTH_STENCIL_ATTACHMENT_BIT | pvrvk::ImageUsageFlags::e_TRANSIENT_ATTACHMENT_BIT, 1, 1, pvrvk::SampleCountFlags::e_1_BIT,
			pvrvk::ImageCreateFlags(0), pvrvk::ImageTiling::e_OPTIMAL, pvrvk::SharingMode::e_EXCLUSIVE, pvrvk::ImageLayout::e_UNDEFINED, &_graphicsQueueFamilyIndex, 1);
		pvrvk::Image image = _deviceResources->device->createImage(createInfo);

		// get the image memory requirements, memory type index and memory property flags required for backing the PVRVk image
		const pvrvk::MemoryRequirements& memoryRequirements = image->getMemoryRequirement();
		uint32_t memoryTypeIndex;
		pvrvk::MemoryPropertyFlags memoryPropertyFlags;
		getMemoryTypeIndex(_deviceResources->device->getPhysicalDevice(), memoryRequirements.getMemoryTypeBits(), requiredMemoryProperties, optimalMemoryProperties,
			memoryTypeIndex, memoryPropertyFlags);

		// allocate the image memory using the retrieved memory type index and memory property flags
		pvrvk::DeviceMemory memBlock = _deviceResources->device->allocateMemory(pvrvk::MemoryAllocationInfo(memoryRequirements.getSize(), memoryTypeIndex));

		// attach the memory to the image
		image->bindMemoryNonSparse(memBlock);

		_deviceResources->depthStencilImageViews[i] = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(image));
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
pvrvk::Buffer VulkanIntroducingPVRVk::createBufferAndAllocateMemory(
	pvrvk::DeviceSize size, pvrvk::BufferUsageFlags usageFlags, pvrvk::MemoryPropertyFlags requiredMemFlags, pvrvk::MemoryPropertyFlags optimalMemFlags)
{
	// Creates a buffer based on the size and usage flags specified.
	// Allocates device memory based on the specified memory property flags.
	// Attaches the allocated memory to the created buffer.

	// A buffer is simply a linear array of data used for various purposes including reading/writing to them using graphics/compute pipelines.

	// Creates the buffer with the specified size and which supports the specified usage.
	pvrvk::BufferCreateInfo createInfo = pvrvk::BufferCreateInfo(size, usageFlags, pvrvk::BufferCreateFlags::e_NONE, pvrvk::SharingMode::e_EXCLUSIVE, &_graphicsQueueFamilyIndex, 1);
	pvrvk::Buffer buffer = _deviceResources->device->createBuffer(createInfo);

	// In Vulkan all resources are initially created as what are termed virtual allocations and have no real physical backing memory.
	// To provide resources with memory backing device memory must be allocated separately and then associated with the relevant resource.
	// Various resources and resource types have differing memory requirements as to the type, size and alignment of the memory. For buffers querying for
	// the memory requirements is made using a call to vkGetBufferMemoryRequirements passing the created buffer as an argument.
	// get the buffer memory requirements, memory type index and memory property flags required for backing the PVRVk buffer
	pvrvk::DeviceMemory deviceMemory = allocateDeviceMemory(buffer->getMemoryRequirement(), requiredMemFlags, optimalMemFlags);

	// attach the memory to the buffer
	buffer->bindMemory(deviceMemory, 0);

	return buffer;
}

/// <summary>Allocates device memory based on the provided arguments.</summary>
/// <param name="memoryRequirements">The requirements for the memory to allocate.</param>
/// <param name="requiredMemFlags">The required memory property flags for the memory to allocate.</param>
/// <param name="optimalMemFlags">An optimal set of memory property flags for the memory to allocate.</param>
/// <returns>The allocated pvrvk device memory object.</returns>
pvrvk::DeviceMemory VulkanIntroducingPVRVk::allocateDeviceMemory(
	const pvrvk::MemoryRequirements& memoryRequirements, const pvrvk::MemoryPropertyFlags requiredMemFlags, const pvrvk::MemoryPropertyFlags optimalMemFlags)
{
	// Allocate the device memory based on the specified set of requirements.
	// Device memory is memory which is visible to the device i.e. the contents of buffers or images which devices can make use of.

	uint32_t memoryTypeIndex;
	pvrvk::MemoryPropertyFlags memoryPropertyFlags;

	// Retrieve a suitable memory type index for the memory allocation.
	// Device memory will be allocated from a physical device from various memory heaps depending on the type of memory required.
	// Each memory heap may well expose a number of different memory types although allocations of different memory types from the same heap will make use of the same memory
	// resource consuming resources from the heap indicated by that memory type's heap index.
	getMemoryTypeIndex(_deviceResources->device->getPhysicalDevice(), memoryRequirements.getMemoryTypeBits(), requiredMemFlags, optimalMemFlags, memoryTypeIndex, memoryPropertyFlags);

	// allocate the memory using the retrieved memory type index and memory property flags
	return _deviceResources->device->allocateMemory(pvrvk::MemoryAllocationInfo(memoryRequirements.getSize(), memoryTypeIndex));
}

/// <summary>Loads a spirv shader binary from memory and creates a shader module for it.</summary>
/// <param name="shaderName">The name of the spirv shader binary to load from memory.</param>
/// <returns>The create pvrvk ShaderModule object.</returns>
pvrvk::ShaderModule VulkanIntroducingPVRVk::createShaderModule(const char* const shaderName)
{
	// Load the spirv shader binary from memory and create a shader module for it

	// Load the spirv shader binary from memory by creating a Stream object for it directly from the file system or from a platform specific store such as
	// Windows resources, Android .apk assets etc.
	// A shader itself specifies the programmable operations executed for a particular type of task - vertex, control point, tessellated vertex, primitive, fragment or compute
	// workgroup
	std::unique_ptr<pvr::Stream> stream = getAssetStream(shaderName);
	assertion(stream.get() != nullptr && "Invalid Shader source");
	std::vector<uint32_t> readData(stream->getSize());

	// Create the shader module using the asset stream.
	// A shader module contains the actual shader code to be executed as well as one or more entry points. A shader module can encapsulate multiple shaders with the shader chosen
	// via the use of an entry point as part of any pipeline creation making use of the shader module. The shader code making up the shader module is provided in the spirv format.
	return _deviceResources->device->createShaderModule(pvrvk::ShaderModuleCreateInfo(stream->readToEnd<uint32_t>()));
}

/// <summary>Allocates the command buffers used by the application. The number of command buffers allocated is equal to the swapchain length.</summary>
void VulkanIntroducingPVRVk::allocateCommandBuffers()
{
	// Command buffers are used to control the submission of various Vulkan commands to a set of devices via their queues.

	// A command buffer is initially created empty and must be recorded to. Once recorded a command buffer can be submitted once or many times to a queue for execution.

	_deviceResources->commandPool->allocateCommandBuffers(_deviceResources->swapchain->getSwapchainLength(), &_deviceResources->cmdBuffers[0]);
}

/// <summary>Records the rendering commands into a set of command buffers which can be subsequently submitted to a queue for execution.</summary>
void VulkanIntroducingPVRVk::recordCommandBuffers()
{
	// Record the rendering commands into a set of command buffers upfront (once). These command buffers can then be submitted to a device queue for execution resulting in
	// fewer state changes and less commands being dispatched to the implementation all resulting in less driver overhead.

	// Recorded commands will include pipelines to use and their descriptor sets, dynamic state modification commands, rendering commands (draws), compute commands
	// (dispatches), commands for executing secondary command buffers or commands to copy resources

	// Vulkan does not provide any kind of global state machine, neither does it provide any kind of default states this means that each command buffer manages its own state
	// independently of all other command buffers and each command buffer must independently configure all of the state relevant to its own set of commands.

	// Specify the clear values used by the RenderPass for clearing the specified framebuffer attachments
	pvrvk::ClearValue clearValues[2] = { pvrvk::ClearValue(0.00f, 0.70f, 0.67f, 1.0f), pvrvk::ClearValue(1.f, 0u) };

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		// Commands may only be recorded once the command buffer is in the recording state.
		// begin recording commands
		_deviceResources->cmdBuffers[i]->begin();

		// Begin the RenderPass specifying the framebuffer the renderpass instance will make use of.
		// The renderable area affected by the renderpass instance may also be configured in addition to an array of pvrvk::ClearValue structures specifying
		// clear values for each attachment of the framebuffer.

		// Initiates the start of a renderPass.
		// From this point until either vkCmdNextSubpass or vkCmdEndRenderPass is called commands will be recorded for the first subpass of the specified renderPass.

		// Bind the graphics pipeline through which commands will be funnelled.
		_deviceResources->cmdBuffers[i]->bindPipeline(_deviceResources->graphicsPipeline);

		_deviceResources->cmdBuffers[i]->beginRenderPass(_deviceResources->framebuffers[i], true, clearValues, 2);

		// Setup a list of descriptor sets which will be used for subsequent pipelines.
		pvrvk::DescriptorSet descriptorSets[] = { _deviceResources->staticDescriptorSet, _deviceResources->dynamicDescriptorSet };

		// Calculate the dynamic offset to use per swapchain controlling the "slice" of a buffer to be used for the current swapchain.
		uint32_t dynamicOffset = static_cast<uint32_t>(_dynamicBufferAlignedSize * i);

		// Bind the list of descriptor sets using the dynamic offset.
		_deviceResources->cmdBuffers[i]->bindDescriptorSets(pvrvk::PipelineBindPoint::e_GRAPHICS, _deviceResources->pipelineLayout, 0, descriptorSets, 2, &dynamicOffset, 1);

		// Bind the vertex buffer used for sourcing the triangle vertices
		_deviceResources->cmdBuffers[i]->bindVertexBuffer(_deviceResources->vbo, 0, 0);

		// Record a non-indexed draw command specifying the number of vertices
		_deviceResources->cmdBuffers[i]->draw(0, 3, 0, 1);

		// Ends the current renderPass instance.
		_deviceResources->cmdBuffers[i]->endRenderPass();

		// Ends the recording for the specified command buffer
		_deviceResources->cmdBuffers[i]->end();
	}
}

/// <summary>Create the pipeline cache used throughout the demo.</summary>
void VulkanIntroducingPVRVk::createPipelineCache()
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
	// The implementation handles updates to the pipeline cache and the application only needs to make use of the pipeline cache across all pipeline creation calls to achieve
	// the most possible gains.

	// It's heavily recommended to make use of pipeline caches as much as possible as they provide little to no overhead and provide the opportunity for
	// the implementation to provide optimisations for us. From the point of view of the application they provide an easy win in terms of work/benefit.

	_deviceResources->pipelineCache = _deviceResources->device->createPipelineCache(pvrvk::PipelineCacheCreateInfo());
}

/// <summary>Create the pvrvk::ShaderModule(s) used in the demo.</summary>
void VulkanIntroducingPVRVk::createShaderModules()
{
	// Creates the pvrvk::ShaderModule(s) used by the demo.

	// These shader modules contain shader code and entry points used by the graphics pipeline for rendering a textured triangle to the screen.
	// Note that the shader modules have been pre-compiled to SPIR-V format using the "recompile script" included alongside the demo (recompile.sh/recompile.bat).
	// Note that when creating our graphics pipeline the specific shader to use from the shader module is specified using an entry point.
	_deviceResources->vertexShaderModule = createShaderModule(VertShaderName);
	_deviceResources->fragmentShaderModule = createShaderModule(FragShaderName);
}

/// <summary>Create the Pipeline Layout used in the demo.</summary>
void VulkanIntroducingPVRVk::createPipelineLayout()
{
	// Create the pipeline layout used throughout the demo.

	// A pipeline layout describes the full set of resources which may be accessed by a pipeline making use of the pipeline layout.
	// A pipeline layout sets out a contract between a set of resources, each of which has a particular layout, and a pipeline.

	// Create a list of descriptor set layouts which are going to be used to create the pipeline layout.

	pvrvk::PipelineLayoutCreateInfo pipeLayoutInfo;
	pipeLayoutInfo.addDescSetLayout(_deviceResources->staticDescriptorSetLayout); /* set 0 */
	pipeLayoutInfo.addDescSetLayout(_deviceResources->dynamicDescriptorSetLayout); /* set 1 */
	_deviceResources->pipelineLayout = _deviceResources->device->createPipelineLayout(pipeLayoutInfo);
}

/// <summary>Creates the graphics pipeline used in the demo.</summary>
void VulkanIntroducingPVRVk::createPipeline()
{
	// Create the graphics pipeline used throughout the demo for rendering the triangle.

	// A pipeline effectively sets up and configures a processing pipeline of a particular type (pvrvk::PipelineBindPoint) which becomes the funnel for which certain sets of
	// Vulkan commands are sent through.

	// The pipeline used throughout this demo is fundamentally simple in nature but still illustrates how to make use of a graphics pipeline to render a geometric object even
	// if it is only a triangle. The pipeline makes use of vertex attributes (position, normal and UV), samples a particular texture writing the result in to a colour attachment
	// and also rendering to a depth stencil attachments. Pipelines are monolithic objects taking account of various bits of state which allow for a great deal of optimization
	// of shaders based on the pipeline description including shader inputs/outputs and fixed function stages.

	// The first part of a graphics pipeline will assemble a set of vertices to form geometric objects based on the requested primitive topology. These vertices may then
	// be transformed using a Vertex Shader computing their position and generating attributes for each of the vertices. The pvrvk::PipelineVertexInputStateCreateInfo and
	// pvrvk::PipelineInputAssemblyStateCreateInfo structures will control how these vertices are assembled.

	// The pvrvk::VertexInputBindingDescription structure specifies the way in which vertex attributes are taken from buffers.
	pvrvk::VertexInputBindingDescription vertexInputBindingDescription(0, _vboStride, pvrvk::VertexInputRate::e_VERTEX);

	// The pvrvk::VertexInputAttributeDescription structure specifies the structure of a particular vertex attribute (position, normal, uvs etc.).
	pvrvk::VertexInputAttributeDescription vertexInputAttributeDescription[2];
	vertexInputAttributeDescription[0].setBinding(0);
	vertexInputAttributeDescription[0].setFormat(pvrvk::Format::e_R32G32B32A32_SFLOAT);
	vertexInputAttributeDescription[0].setLocation(0);
	vertexInputAttributeDescription[0].setOffset(0);

	vertexInputAttributeDescription[1].setBinding(0);
	vertexInputAttributeDescription[1].setFormat(pvrvk::Format::e_R32G32_SFLOAT);
	vertexInputAttributeDescription[1].setLocation(1);
	vertexInputAttributeDescription[1].setOffset(sizeof(glm::vec4));

	// The pvrvk::PipelineVertexInputStateCreateInfo structure specifies a set of descriptions for the vertex attributes and vertex bindings.
	pvrvk::PipelineVertexInputStateCreateInfo vertexInputInfo;
	vertexInputInfo.addInputBinding(vertexInputBindingDescription);
	vertexInputInfo.addInputAttributes(vertexInputAttributeDescription, 2);

	// The pvrvk::PipelineInputAssemblyStateCreateInfo structure specifies how primitives are assembled.
	pvrvk::PipelineInputAssemblerStateCreateInfo inputAssemblyInfo;
	inputAssemblyInfo.setPrimitiveTopology(pvrvk::PrimitiveTopology::e_TRIANGLE_LIST);

	// The resulting primitives are clipped and sent to the next pipeline stage...

	// The next stage of the graphics pipeline, Rasterization, produces fragments based on the points, line segments or triangles constructed in the first stage.
	// Each of the generated fragments will be passed to the fragment shader carrying out the per fragment rendering - this is where the framebuffer operations occur. This
	// stage includes blending, masking, stencilling and other logical operations.

	// The pvrvk::PipelineRasterizationStateCreateInfo structure specifies how various aspects of rasterization occur including cull mode.
	pvrvk::PipelineRasterizationStateCreateInfo rasterizationInfo;
	rasterizationInfo.setCullMode(pvrvk::CullModeFlags::e_BACK_BIT);
	rasterizationInfo.setFrontFaceWinding(pvrvk::FrontFace::e_COUNTER_CLOCKWISE);

	// The pvrvk::PipelineColorBlendAttachmentState structure specifies blending state for a particular colour attachment.
	pvrvk::PipelineColorBlendAttachmentState colorBlendAttachment;

	// The pvrvk::PipelineColorBlendStateCreateInfo structure controls the per attachment blending.
	pvrvk::PipelineColorBlendStateCreateInfo colorBlendInfo;
	colorBlendInfo.setAttachmentState(0, colorBlendAttachment);

	// The pvrvk::PipelineViewportStateCreateInfo structure specifies the viewport and scissor regions used by the pipeline.
	pvrvk::PipelineViewportStateCreateInfo viewportInfo;
	viewportInfo.setViewportAndScissor(0, _viewport, _scissor);

	// The pvrvk::PipelineShaderStageCreateInfo structure specifies the creation of a particular stage of a graphics pipeline including vertex/fragment/tessellation
	// {control/evaluation} taking the shader to be used from a particular pvrvk::ShaderModule.
	pvrvk::PipelineShaderStageCreateInfo shaderStageCreateInfos[2];

	{
		shaderStageCreateInfos[0].setEntryPoint("main");
		shaderStageCreateInfos[0].setShader(_deviceResources->vertexShaderModule);
	}

	{
		shaderStageCreateInfos[1].setEntryPoint("main");
		shaderStageCreateInfos[1].setShader(_deviceResources->fragmentShaderModule);
	}

	// Create the graphics pipeline adding all the individual info structs.
	pvrvk::GraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.inputAssembler = inputAssemblyInfo;
	pipelineInfo.pipelineLayout = _deviceResources->pipelineLayout;
	pipelineInfo.rasterizer = rasterizationInfo;
	pipelineInfo.renderPass = _deviceResources->renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.vertexInput = vertexInputInfo;
	pipelineInfo.colorBlend = colorBlendInfo;
	pipelineInfo.vertexShader = shaderStageCreateInfos[0];
	pipelineInfo.fragmentShader = shaderStageCreateInfos[1];
	pipelineInfo.viewport = viewportInfo;

	// Create the graphics pipeline we'll use for rendering a triangle.
	_deviceResources->graphicsPipeline = _deviceResources->device->createGraphicsPipeline(pipelineInfo, _deviceResources->pipelineCache);
}

/// <summary>Initializes the vertex buffer objects used in the demo.</summary>
void VulkanIntroducingPVRVk::createVbo()
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

	// The use of pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT specifies that memory allocated with this memory property type is the most efficient for device access.
	// Note that memory property flag pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT has not been specified meaning the host application must manage the memory accesses to
	// this memory explicitly using the host cache management commands vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges to flush host writes to the device or make
	// device writes visible to the host respectively.
	pvrvk::MemoryPropertyFlags requiredFlags = pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT;
	pvrvk::MemoryPropertyFlags optimalFlags = requiredFlags | pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT;
	_deviceResources->vbo =
		createBufferAndAllocateMemory(vboSize, pvrvk::BufferUsageFlags::e_VERTEX_BUFFER_BIT | pvrvk::BufferUsageFlags::e_TRANSFER_DST_BIT, requiredFlags, optimalFlags);

	// Construct the triangle vertices.
	TriangleVertex triangle[3];
	triangle[0] = { glm::vec4(0.5f, -0.288f, 0.0f, 1.0f), { 1.f, 0.f } };
	triangle[1] = { glm::vec4(-0.5f, -0.288f, 0.0f, 1.0f), { 0.f, 0.f } };
	triangle[2] = { glm::vec4(0.0f, 0.577f, 0.0f, 1.0f), { .5f, 1.f } };

	// The use of pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT indicates that memory allocated with this memory property type can be mapped and unmapped enabling host
	// access using calls to vkMapMemory and vkUnmapMemory respectively. When this memory property type is used we are able to map/update/unmap the memory to update the
	// contents of the memory.
	if ((_deviceResources->vbo->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT) != 0)
	{
		// Memory created using vkAllocateMemory isn't directly accessible to the host and instead must be mapped manually.
		// Note that only memory created with the memory property flag pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT can be mapped.
		// vkMapMemory retrieves a host virtual address pointer to a region of a mappable memory object.
		void* mapped = _deviceResources->vbo->getDeviceMemory()->map(0, vboSize);

		memcpy(mapped, triangle, sizeof(triangle));

		// If the memory property flags for the allocated memory included the use of pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT then the host does not need to manage the
		// memory accesses explicitly using the host cache management commands vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges to flush host writes to
		// the device or make device writes visible to the host respectively. This behaviour is handled by the implementation.
		if ((_deviceResources->vbo->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			// Flush the memory guaranteeing that host writes to the memory ranges specified are made available to the device.
			_deviceResources->vbo->getDeviceMemory()->flushRange(0, vboSize);
		}

		// Note that simply unmapping non-coherent memory doesn't implicitly flush the mapped memory.
		_deviceResources->vbo->getDeviceMemory()->unmap();
	}
	else
	{
		// The use of pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT indicates that memory allocated with this memory property type can be mapped and unmapped enabling host
		// access using calls to vkMapMemory and vkUnmapMemory respectively. When this memory property type is used we are able to map/update/unmap the memory to update the
		// contents of the memory.
		pvrvk::MemoryPropertyFlags requiredMemoryFlags = pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT;
		pvrvk::MemoryPropertyFlags optimalMemoryFlags = requiredMemoryFlags;
		// We use our buffer creation function to generate a staging buffer. We pass the pvrvk::BufferUsageFlags::e_TRANSFER_SRC_BIT flag to specify its use.
		pvrvk::Buffer stagingBuffer = createBufferAndAllocateMemory(vboSize, pvrvk::BufferUsageFlags::e_TRANSFER_SRC_BIT, requiredMemoryFlags, optimalMemoryFlags);

		//
		// Map the staging buffer and copy the triangle vbo data into it.
		//

		{
			void* mapped = stagingBuffer->getDeviceMemory()->map(0, vboSize);
			memcpy(mapped, triangle, sizeof(triangle));

			// If the memory property flags for the allocated memory included the use of pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT then the host does not need to manage the
			// memory accesses explicitly using the host cache management commands vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges to flush host writes to
			// the device or make device writes visible to the host respectively. This behaviour is handled by the implementation.
			if ((stagingBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
			{
				// flush the memory
				stagingBuffer->getDeviceMemory()->flushRange(0, sizeof(triangle));
			}

			stagingBuffer->getDeviceMemory()->unmap();
		}

		// We create a command buffer to execute the copy operation from our command pool.
		pvrvk::CommandBuffer cmdBuffers = _deviceResources->commandPool->allocateCommandBuffer();

		// We start recording our command buffer operation
		cmdBuffers->begin();
		pvrvk::BufferCopy bufferCopy(0, 0, sizeof(triangle));
		cmdBuffers->copyBuffer(stagingBuffer, _deviceResources->vbo, 1, &bufferCopy);

		// We end the recording of our command buffer.
		cmdBuffers->end();

		// We create a fence to make sure that the command buffer is synchronized correctly.
		pvrvk::Fence copyFence = _deviceResources->device->createFence();

		// Submit the command buffer to the queue specified
		pvrvk::SubmitInfo submitInfo;
		submitInfo.commandBuffers = &cmdBuffers;
		submitInfo.numCommandBuffers = 1;

		_deviceResources->queue->submit(submitInfo, copyFence);

		// Wait for the specified fence to be signalled which ensures that the command buffer has finished executing.
		copyFence->wait();
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
void VulkanIntroducingPVRVk::createUniformBuffers()
{
	// Vulkan requires that when updating a descriptor of type VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER or VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC the
	// offset specified is an integer multiple of the minimum required alignment in bytes for the physical device - as must any dynamic alignments used.
	size_t minimumUboAlignment = static_cast<size_t>(_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMinUniformBufferOffsetAlignment());

	// The dynamic buffers will be used as uniform buffers (later used as a descriptor of type VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC and VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER).
	pvrvk::BufferUsageFlags usageFlags = pvrvk::BufferUsageFlags::e_UNIFORM_BUFFER_BIT;

	// The use of pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT indicates that memory allocated with this memory property type can be mapped and unmapped enabling host
	// access using calls to vkMapMemory and vkUnmapMemory respectively. The memory property flag pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT is guaranteed to be available
	// The use of pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT indicates the host does not need to manage the memory accesses explicitly using the host cache management
	// commands vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges to flush host writes to the device or make device writes visible to the host respectively. This
	// behaviour is handled by the implementation.
	pvrvk::MemoryPropertyFlags requiredPropertyFlags = pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT;
	pvrvk::MemoryPropertyFlags optimalPropertyFlags = pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT;

	{
		// Using the minimum uniform buffer offset alignment we calculate the minimum buffer slice size based on the size of the intended data or more specifically
		// the size of the smallest chunk of data which may be mapped or updated as a whole.
		size_t bufferDataSize = sizeof(glm::mat4);
		_dynamicBufferAlignedSize = static_cast<uint32_t>(getAlignedDataSize(bufferDataSize, minimumUboAlignment));

		// Calculate the size of the dynamic uniform buffer.
		// This buffer will be updated each frame and must therefore be multi-buffered to avoid issues with using partially updated data, or updating data already in used.
		// Rather than allocating multiple (swapchain) buffers we instead allocate a larger buffer and will instead use a slice per swapchain. This works as longer as the
		// buffer is created taking into account the minimum uniform buffer offset alignment.
		uint32_t modelViewProjectionBufferSize = _deviceResources->swapchain->getSwapchainLength() * _dynamicBufferAlignedSize;

		// Create the buffer, allocate the device memory and attach the memory to the newly created buffer object.
		_deviceResources->modelViewProjectionBuffer = createBufferAndAllocateMemory(modelViewProjectionBufferSize, usageFlags, requiredPropertyFlags, optimalPropertyFlags);

		// Memory created using vkAllocateMemory isn't directly accessible to the -host and instead must be mapped manually.
		// Note that only memory created with the memory property flag pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT can be mapped.
		// vkMapMemory retrieves a host virtual address pointer to a region of a mappable memory object.
		_deviceResources->modelViewProjectionBuffer->getDeviceMemory()->map(0, _deviceResources->modelViewProjectionBuffer->getSize());
	}
}

/// <summary>Generates a simple checker board texture.</summary>
void VulkanIntroducingPVRVk::generateTexture()
{
	// Generates a simple checkered texture which will be applied and used as a texture for the triangle we are going to render and rotate on screen.
	for (uint32_t x = 0; x < _textureDimensions.getWidth(); ++x)
	{
		for (uint32_t y = 0; y < _textureDimensions.getHeight(); ++y)
		{
			float g = 0.3f;
			if (x % 128 < 64 && y % 128 < 64) { g = 1; }
			if (x % 128 >= 64 && y % 128 >= 64) { g = 1; }

			uint8_t* pixel = _textureData.data() + (x * _textureDimensions.getHeight() * 4) + (y * 4);
			pixel[0] = static_cast<uint8_t>(100 * g);
			pixel[1] = static_cast<uint8_t>(80 * g);
			pixel[2] = static_cast<uint8_t>(70 * g);
			pixel[3] = 255;
		}
	}
}

/// <summary>Allocate the descriptor sets used throughout the demo.</summary>
void VulkanIntroducingPVRVk::allocateDescriptorSets()
{
	// Allocate the descriptor sets from the pool of descriptors.
	// Each descriptor set follows the layout specified by a predefined descriptor set layout.

	// Allocate the descriptor sets from the descriptor pool.
	_deviceResources->staticDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->staticDescriptorSetLayout);
	_deviceResources->dynamicDescriptorSet = _deviceResources->descriptorPool->allocateDescriptorSet(_deviceResources->dynamicDescriptorSetLayout);

	// Note that at this point the descriptor sets are largely uninitialised and all the descriptors are undefined although
	// the descriptor sets can still be bound to command buffers without issues.

	// In our case we will update the descriptor sets immediately using descriptor set write operations.

	std::vector<pvrvk::WriteDescriptorSet> writeDescSets;

	{
		pvrvk::WriteDescriptorSet writeDescSet(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, _deviceResources->staticDescriptorSet, 0, 0);
		writeDescSet.setImageInfo(0, pvrvk::DescriptorImageInfo(_deviceResources->triangleImageView, _deviceResources->bilinearSampler, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL));
		writeDescSets.push_back(writeDescSet);
	}

	{
		// Check the physical device limit specifying the maximum number of descriptor sets using dynamic buffers.
		if (_deviceResources->device->getPhysicalDevice()->getProperties().getLimits().getMaxDescriptorSetUniformBuffersDynamic() < 1)
		{ throw pvr::PvrError("The physical device must support at least 1 dynamic uniform buffer"); }

		pvrvk::WriteDescriptorSet writeDescSet(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, _deviceResources->dynamicDescriptorSet, 0, 0);
		writeDescSet.setBufferInfo(0, pvrvk::DescriptorBufferInfo(_deviceResources->modelViewProjectionBuffer, 0, _dynamicBufferAlignedSize));
		writeDescSets.push_back(writeDescSet);
	}

	// Write the descriptors to the descriptor sets
	_deviceResources->device->updateDescriptorSets(writeDescSets.data(), static_cast<uint32_t>(writeDescSets.size()), nullptr, 0);
}

/// <summary>Creates the descriptor set layouts used throughout the demo.</summary>
void VulkanIntroducingPVRVk::createDescriptorSetLayouts()
{
	// Create the descriptor set layouts used throughout the demo with a descriptor set layout being defined by an array of 0 or more descriptor set layout bindings.
	// Each descriptor set layout binding corresponds to a type of descriptor, its shader bindings, a set of shader stages which may access the descriptor and a array size
	// count. A descriptor set layout provides an interface for the resources used by the descriptor set and the interface between shader stages and shader resources.

	// Create the descriptor set layout for the static resources.
	{
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1, pvrvk::ShaderStageFlags::e_FRAGMENT_BIT);
		_deviceResources->staticDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}

	// Create the descriptor set layout for the dynamic resources.
	// Note that we use a descriptor of type VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC with dynamic offsets provided by swapchain. We could also have achieved the same result
	// using multiple descriptor sets each referencing the per swapchain slice of the same (dynamic) buffer without using a dynamic descriptor with dynamic offsets.
	{
		// dynamic ubo
		pvrvk::DescriptorSetLayoutCreateInfo descSetInfo;
		descSetInfo.setBinding(0, pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1, pvrvk::ShaderStageFlags::e_VERTEX_BIT); /*binding 0*/
		_deviceResources->dynamicDescriptorSetLayout = _deviceResources->device->createDescriptorSetLayout(descSetInfo);
	}
}

/// <summary>Creates the descriptor pool used throughout the demo.</summary>
void VulkanIntroducingPVRVk::createDescriptorPool()
{
	// Create the Descriptor Pool used throughout the demo.

	// A descriptor pool maintains a list of free descriptors from which descriptor sets can be allocated.

	// A pvrvk::DescriptorPoolSize structure sets out the number and type of descriptors of that type to allocate

	_deviceResources->descriptorPool = _deviceResources->device->createDescriptorPool(pvrvk::DescriptorPoolCreateInfo()
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_COMBINED_IMAGE_SAMPLER, 1)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER_DYNAMIC, 1)
																						  .addDescriptorInfo(pvrvk::DescriptorType::e_UNIFORM_BUFFER, 1)
																						  .setMaxDescriptorSets(2));
}

/// <summary>Creates a checker board texture which will be applied to the triangle during rendering.</summary>
void VulkanIntroducingPVRVk::createTexture()
{
	// Creates a checker board texture which will be applied to the triangle during rendering.

	// In Vulkan, uploading an image/texture requires a few more steps than those familiar with older APIs would expect however these steps are required due to the explicit
	// nature of Vulkan and the control Vulkan affords to the user making possible various performance optimisations. These steps include:

	// 1) Create the (CPU side) texture:
	//	  a) Create the texture data in CPU side memory.

	// 2) Create the (empty) (GPU side) texture:
	//    a) Creating the Vulkan texture definition - a "pvrvk::Image" object.
	//    b) Determining the pvrvk::Image memory requirements, creating the backing memory object ("pvrvk::DeviceMemory" object).
	//    c) Bind the memory (pvrvk::DeviceMemory) to the image (pvrvk::Image).

	// 3) Upload the data into the texture:
	//    a) Create a staging buffer and its backing memory object - "pvrvk::DeviceMemory" object.
	//    b) Map the staging buffer and copy the image data into it.
	//    c) Perform a vkCmdCopyBufferToImage operation to transfer the data into the image.

	// 4) Create a view for the image to make it accessible by pipeline shaders and a sampler object specifying how the image should be sampled:
	//    a) Create a view for the Vulkan texture so that it can be accessed by pipelines shaders for reading or writing to its image data - "pvrvk::ImageView" object
	//    b) Create a sampler controlling how the sampled image data is sampled when accessed by pipeline shaders.

	// A texture (Sampled Image) is stored in the GPU in an implementation-defined way, which may be completely different to the layout of the texture on disk/CPU side.
	// For that reason, it is not possible to map its memory and write directly the data for that image.
	// This is the reason for the second (Uploading) step: The vkCmdCopyBufferToImage command guarantees the correct translation/swizzling of the texture data.

	//
	// 1a). Create the texture data in CPU side memory.
	//

	// Setup the texture dimensions and the size of the texture itself.
	_textureDimensions.setWidth(256);
	_textureDimensions.setHeight(256);
	_textureData.resize(_textureDimensions.getWidth() * _textureDimensions.getHeight() * (sizeof(uint8_t) * 4));

	// This function generates our texture pattern on-the-fly into a block of CPU side memory (_textureData)
	generateTexture();

	//
	// 2a). Creating the Vulkan texture definition - a "pvrvk::Image" object.
	//

	// Record the pvrvk::Format of the texture.
	pvrvk::Format triangleImageFormat = pvrvk::Format::e_R8G8B8A8_UNORM;

	// We create the image info struct. We set the parameters for our texture (layout, format, usage etc..)
	pvrvk::ImageCreateInfo createInfo =
		pvrvk::ImageCreateInfo(pvrvk::ImageType::e_2D, triangleImageFormat, pvrvk::Extent3D(_textureDimensions.getWidth(), _textureDimensions.getHeight(), 1u),
			pvrvk::ImageUsageFlags::e_SAMPLED_BIT | pvrvk::ImageUsageFlags::e_TRANSFER_DST_BIT, 1, 1, pvrvk::SampleCountFlags::e_1_BIT, pvrvk::ImageCreateFlags(0),
			pvrvk::ImageTiling::e_OPTIMAL, pvrvk::SharingMode::e_EXCLUSIVE, pvrvk::ImageLayout::e_UNDEFINED, &_graphicsQueueFamilyIndex, 1);

	// We create the texture image.
	pvrvk::Image image = _deviceResources->device->createImage(createInfo);

	//
	// 2b). Determining the pvrvk::Image memory requirements, creating the backing memory object ("pvrvk::DeviceMemory" object).
	//

	// Allocate the device memory for the created image based on the arguments provided.
	pvrvk::DeviceMemory deviceMemory =
		allocateDeviceMemory(image->getMemoryRequirement(), pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT, pvrvk::MemoryPropertyFlags::e_DEVICE_LOCAL_BIT);

	//
	// 2c). Bind the memory (pvrvk::DeviceMemory) to the image (pvrvk::Image).
	//

	// Finally attach the allocated device memory to the created image.
	image->bindMemoryNonSparse(deviceMemory);

	//
	// 3a). Create a staging buffer and its backing memory object ("pvrvk::DeviceMemory" object).
	//

	// We use our buffer creation function to generate a staging buffer. We pass the pvrvk::BufferUsageFlags::e_TRANSFER_SRC_BIT flag to specify its use.
	pvrvk::MemoryPropertyFlags requiredMemoryFlags = pvrvk::MemoryPropertyFlags::e_HOST_VISIBLE_BIT;
	pvrvk::MemoryPropertyFlags optimalMemoryFlags = requiredMemoryFlags;

	pvrvk::Buffer stagingBuffer = createBufferAndAllocateMemory(_textureData.size(), pvrvk::BufferUsageFlags::e_TRANSFER_SRC_BIT, requiredMemoryFlags, optimalMemoryFlags);

	//
	// 3b). Map the staging buffer and copy the image data into it.
	//

	{
		void* mapped = stagingBuffer->getDeviceMemory()->map(0, _textureData.size());
		memcpy(mapped, _textureData.data(), _textureData.size());

		// If the memory property flags for the allocated memory included the use of pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT then the host does not need to manage the
		// memory accesses explicitly using the host cache management commands vkFlushMappedMemoryRanges and vkInvalidateMappedMemoryRanges to flush host writes to
		// the device or make device writes visible to the host respectively. This behaviour is handled by the implementation.
		if ((stagingBuffer->getDeviceMemory()->getMemoryFlags() & pvrvk::MemoryPropertyFlags::e_HOST_COHERENT_BIT) == 0)
		{
			// flush the memory
			stagingBuffer->getDeviceMemory()->flushRange(0, _textureData.size());
		}

		stagingBuffer->getDeviceMemory()->unmap();
	}

	//
	// 3c). Perform a vkCmdCopyBufferToImage operation to transfer the data into the image.
	//

	// We create command buffer to execute the copy operation from our command pool.
	pvrvk::CommandBuffer cmdBuffers = _deviceResources->commandPool->allocateCommandBuffer();

	// We start recording our command buffer operation
	cmdBuffers->begin();

	// We specify the sub resource range of our Image. In the case our Image the parameters are default as our image is very simple.
	pvrvk::ImageSubresourceRange subResourceRange(formatToImageAspect(triangleImageFormat), 0, 1, 0, 1);

	{
		// We need to create a memory barrier to make sure that the image layout is set up for a copy operation.
		pvrvk::MemoryBarrierSet barriers;
		barriers.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_NONE, pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT, image, subResourceRange,
			pvrvk::ImageLayout::e_UNDEFINED, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, _graphicsQueueFamilyIndex, _graphicsQueueFamilyIndex));

		// We use a pipeline barrier to change the image layout to accommodate the transfer operation
		cmdBuffers->pipelineBarrier(pvrvk::PipelineStageFlags::e_ALL_COMMANDS_BIT, pvrvk::PipelineStageFlags::e_ALL_COMMANDS_BIT, barriers, true);
	}

	// We copy the staging buffer data to memory bound to the image we just created.

	// We specify the region we want to copy from our Texture. In our case it's the entire Image so we pass
	// the Texture width and height as extents.
	pvrvk::ImageSubresourceLayers subResourceLayers(formatToImageAspect(triangleImageFormat), 0, 0, 1);

	pvrvk::BufferImageCopy copyRegion(0, static_cast<uint32_t>(_textureDimensions.getWidth()), static_cast<uint32_t>(_textureDimensions.getHeight()), subResourceLayers,
		pvrvk::Offset3D(0, 0, 0), pvrvk::Extent3D(static_cast<uint32_t>(_textureDimensions.getWidth()), static_cast<uint32_t>(_textureDimensions.getHeight()), 1));

	cmdBuffers->copyBufferToImage(stagingBuffer, image, pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

	{
		// We create a barrier to make sure that the Image layout is Shader read only.
		pvrvk::MemoryBarrierSet barriers;
		barriers.addBarrier(pvrvk::ImageMemoryBarrier(pvrvk::AccessFlags::e_NONE, pvrvk::AccessFlags::e_TRANSFER_WRITE_BIT, image, subResourceRange,
			pvrvk::ImageLayout::e_TRANSFER_DST_OPTIMAL, pvrvk::ImageLayout::e_SHADER_READ_ONLY_OPTIMAL, _graphicsQueueFamilyIndex, _graphicsQueueFamilyIndex));

		// We use a pipeline barrier to change the image layout to be optimized to be read by the shader.
		cmdBuffers->pipelineBarrier(pvrvk::PipelineStageFlags::e_ALL_COMMANDS_BIT, pvrvk::PipelineStageFlags::e_ALL_COMMANDS_BIT, barriers, true);
	}

	// We end the recording of our command buffer.
	cmdBuffers->end();

	// We create a fence to make sure that the command buffer is synchronized correctly.
	pvrvk::Fence copyFence = _deviceResources->device->createFence();

	// Submit the command buffer to the queue specified
	pvrvk::SubmitInfo submitInfo;
	submitInfo.commandBuffers = &cmdBuffers;
	submitInfo.numCommandBuffers = 1;

	_deviceResources->queue->submit(submitInfo, copyFence);

	// Wait for the specified fence to be signalled which ensures that the command buffer has finished executing.
	copyFence->wait();

	//
	// 4a). Create a view for the Vulkan texture so that it can be accessed by pipelines shaders for reading or writing to its image data - "pvrvk::ImageView" object
	//

	// After the Image is complete, and we copied all the texture data, we need to create an Image View to make sure
	// that API can understand what the Image is. We can provide information on the format for example.

	// We create an Image view info.
	_deviceResources->triangleImageView = _deviceResources->device->createImageView(pvrvk::ImageViewCreateInfo(image));

	//
	// 4b). Create a sampler controlling how the sampled image data is sampled when accessed by pipeline shaders.
	//

	// We create a sampler info struct. We'll need the sampler to pass
	// data to the fragment shader during the execution of the rendering phase.
	pvrvk::SamplerCreateInfo samplerInfo;
	samplerInfo.minFilter = samplerInfo.magFilter = pvrvk::Filter::e_LINEAR;
	samplerInfo.mipMapMode = pvrvk::SamplerMipmapMode::e_LINEAR;
	samplerInfo.wrapModeU = samplerInfo.wrapModeV = pvrvk::SamplerAddressMode::e_CLAMP_TO_EDGE;
	_deviceResources->bilinearSampler = _deviceResources->device->createSampler(samplerInfo);
}

/// <summary>Creates the RenderPass used throughout the demo.</summary>
void VulkanIntroducingPVRVk::createRenderPass()
{
	// Create the RenderPass used throughout the demo.

	// A RenderPass encapsulates a collection of attachments, one or more subpasses, dependencies between the subpasses and then provides a description for
	// how the attachments are used over the execution of the respective subpasses. A RenderPass allows an application to communicate a high level structure of a frame to the
	// implementation.

	// RenderPasses are one of the singly most important features included in Vulkan from the point of view of a tiled architecture. Before going into the gritty details of
	// what RenderPasses are and how they provide a heap of optimization opportunities a (very) brief introduction to tiled architectures - a tiled architecture like any other
	// takes triangles as input but will bin these triangles to particular tiles corresponding to regions of a Framebuffer and then for each tile in turn it will render the
	// subset of geometry binned only to that tile meaning the per tile access becomes very coherent and cache friendly. RenderPasses, subpasses and the use of transient
	// attachments (all explained below) let us exploit and make most of the benefits these kinds of architectures provide.

	// For more information on our TBDR (Tile Based Deferred Rendering) architecture check out our blog posts:
	//		https://www.imgtec.com/blog/a-look-at-the-powervr-graphics-architecture-tile-based-rendering/
	//		https://www.imgtec.com/blog/the-dr-in-tbdr-deferred-rendering-in-rogue/

	// Each RenderPass subpass may reference a subset of the RenderPass's Framebuffer attachments for reading or writing where each subpass containing information
	// about what happens to the attachment data when the subpass begins including whether to clear it, load it from memory or leave it uninitialised as well
	// as what to do with the attachment data when the subpass ends including storing it back to memory or discarding it.
	// RenderPasses require that applications explicitly set out the dependencies between the subpasses providing an implementation with the know-how to effectively optimize
	// when it should flush/clear/store memory in way it couldn't before. RenderPasses are a prime example of how Vulkan has replaced implementation guess work with the
	// application explicitness requiring them to set out their known and understood dependencies - Who is in the better place to properly understand and make decisions as to
	// dependencies between a particular set of commands, images or resources than the application making use of them?

	// Another important feature introduced by Vulkan is the use of transient images (specify VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT in the pvrvk::ImageUsageFlags member of their
	// pvrvk::ImageCreateInfo creation structure).
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
	pvrvk::RenderPassCreateInfo renderPassInfo;

	// An attachment description describes the structure of an attachment including formats, number of samples, image layout transitions and how the image should be handled at
	// the beginning and end of the RenderPass including whether to load or clear memory and store or discard memory respectively.
	pvrvk::AttachmentDescription attachmentDescriptions[2];

	// A subpass encapsulates a set of rendering commands corresponding to a particular phase of a rendering pass including the reading and writing of a subset of RenderPass
	// attachments. A subpass description specifies the subset of attachments involved in the particular phase of rendering corresponding to the subpass.
	pvrvk::SubpassDescription subpass;

	// The subpass makes use of a single colour attachment and the depth stencil attachment matching RenderPass attachments at index 0 and 1 respectively.
	subpass.setColorAttachmentReference(0, pvrvk::AttachmentReference(0, pvrvk::ImageLayout::e_COLOR_ATTACHMENT_OPTIMAL));
	subpass.setDepthStencilAttachmentReference(pvrvk::AttachmentReference(1, pvrvk::ImageLayout::e_DEPTH_STENCIL_ATTACHMENT_OPTIMAL));
	subpass.setPipelineBindPoint(pvrvk::PipelineBindPoint::e_GRAPHICS);

	renderPassInfo.setSubpass(0, subpass);

	// The first pvrvk::AttachmentDescription describes a colour attachment which will be undefined initially (VK_IMAGE_LAYOUT_UNDEFINED), transitioning to a layout suitable for
	// presenting to the screen (VK_IMAGE_LAYOUT_PRESENT_SRC_KHR), uses only a single sample per pixel (VK_SAMPLE_COUNT_1_BIT), a pvrvk::Format matching the format used by the
	// swapchain images, a pvrvk::AttachmentLoadOp specifying that the attachment will be cleared at the beginning of the first subpass in which the attachment is used, a
	// pvrvk::AttachmentStoreOp specifying that the attachment will be stored (VK_ATTACHMENT_STORE_OP_STORE) at the end of the subpass in which the attachment is last used. The
	// stencil load and store ops are set as VK_ATTACHMENT_LOAD_OP_DONT_CARE and VK_ATTACHMENT_STORE_OP_DONT_CARE respectively as the attachment has no stencil component.
	attachmentDescriptions[0] = pvrvk::AttachmentDescription::createColorDescription(_deviceResources->swapchain->getImageFormat(),
		_deviceResources->swapchain->getImage(0)->getInitialLayout(), pvrvk::ImageLayout::e_PRESENT_SRC_KHR, pvrvk::AttachmentLoadOp::e_CLEAR, pvrvk::AttachmentStoreOp::e_STORE);
	renderPassInfo.setAttachmentDescription(0, attachmentDescriptions[0]);

	// The first pvrvk::AttachmentDescription describes a depth/stencil attachment which will be undefined initially (VK_IMAGE_LAYOUT_UNDEFINED), transitioning to a layout suitable
	// for use as a depth stencil attachment (VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL), uses only a single sample per pixel (VK_SAMPLE_COUNT_1_BIT), a pvrvk::Format
	// matching the format used by the depth stencil images. Both the stencil and depth pvrvk::AttachmentLoadOp specify that the attachment will be cleared at the beginning of the
	// first subpass in which the attachment is used, and both the stencil and depth pvrvk::AttachmentStoreOps specify that the attachment will be stored
	// (VK_ATTACHMENT_STORE_OP_STORE) at the end of the subpass in which the attachment is last used.
	attachmentDescriptions[1] = pvrvk::AttachmentDescription::createDepthStencilDescription(
		_deviceResources->depthStencilImageViews[0]->getImage()->getFormat(), _deviceResources->depthStencilImageViews[0]->getImage()->getInitialLayout());
	renderPassInfo.setAttachmentDescription(1, attachmentDescriptions[1]);

	// A subpass dependency describes the execution and memory dependencies between subpasses.
	// In this demo only a single subpass is used so technically no subpass dependencies are strictly required however unless specified an implicit subpass dependency is
	// added from VK_SUBPASS_EXTERNAL to the first subpass that uses an attachment and another implicit subpass dependency is added from the last subpass that uses
	// an attachment to VK_SUBPASS_EXTERNAL.
	// As described above the application is in the best position to understand and make decisions about all of the memory dependencies and so we choose to explicitly
	// provide the otherwise implicit subpass dependencies.
	pvrvk::SubpassDependency dependencies[] = {
		// Adds an explicit subpass dependency from VK_SUBPASS_EXTERNAL to the first subpass that uses an attachment which is the first subpass (0).
		{ pvrvk::SubpassExternal, 0, pvrvk::PipelineStageFlags::e_BOTTOM_OF_PIPE_BIT, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::AccessFlags::e_NONE,
			pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::DependencyFlags::e_BY_REGION_BIT },
		// Adds an explicit subpass dependency from the first subpass that uses an attachment which is the first subpass (0) to VK_SUBPASS_EXTERNAL.
		{ 0, pvrvk::SubpassExternal, pvrvk::PipelineStageFlags::e_COLOR_ATTACHMENT_OUTPUT_BIT, pvrvk::PipelineStageFlags::e_BOTTOM_OF_PIPE_BIT,
			pvrvk::AccessFlags::e_COLOR_ATTACHMENT_WRITE_BIT, pvrvk::AccessFlags::e_NONE, pvrvk::DependencyFlags::e_BY_REGION_BIT },
	};

	// Add the set of dependencies to the RenderPass creation.
	renderPassInfo.addSubpassDependencies(dependencies, ARRAY_SIZE(dependencies));

	_deviceResources->renderPass = _deviceResources->device->createRenderPass(renderPassInfo);
}

/// <summary>Creates the Framebuffer objects used in this demo.</summary>
void VulkanIntroducingPVRVk::createFramebuffer()
{
	// Create the framebuffers which are used in conjunction with the application renderPass.

	// Framebuffers encapsulate a collection of attachments that a renderPass instance uses.

	pvrvk::FramebufferCreateInfo framebufferInfo;

	// Note that each element of pAttachments must have dimensions at least as large as the Framebuffer dimensions.
	framebufferInfo.setNumLayers(1);
	framebufferInfo.setDimensions(getWidth(), getHeight());
	// This Framebuffer is compatible with the application renderPass or with any other renderPass compatible with the application renderPass. For more information on
	// RenderPass compatibility please refer to the Vulkan spec section "Render Pass Compatibility".
	framebufferInfo.setRenderPass(_deviceResources->renderPass);

	// Create a Framebuffer per swapchain making use of the per swapchain presentation image and depth stencil image.
	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		framebufferInfo.setAttachment(0, _deviceResources->swapchain->getImageView(i));
		framebufferInfo.setAttachment(1, _deviceResources->depthStencilImageViews[i]);
		_deviceResources->framebuffers[i] = _deviceResources->swapchain->getDevice()->createFramebuffer(framebufferInfo);
	}
}

/// <summary>Creates the command pool used throughout the demo.</summary>
void VulkanIntroducingPVRVk::createCommandPool()
{
	// Create the command pool used for allocating the command buffers used throughout the demo

	// A command pool is an opaque object used for allocating command buffer memory from which applications can spread the cost of resource creation and command recording.
	// Command Pool flags can be used to specify usage behaviour of command buffers allocated from this command pool.
	// Designates the queue family to which commands buffers allocated from this command pool can be submitted.
	_deviceResources->commandPool =
		_deviceResources->device->createCommandPool(pvrvk::CommandPoolCreateInfo(_graphicsQueueFamilyIndex, pvrvk::CommandPoolCreateFlags::e_RESET_COMMAND_BUFFER_BIT));
}

/// <summary>Creates the fences and semaphores used for synchronization throughout this demo.</summary>
void VulkanIntroducingPVRVk::createSynchronisationPrimitives()
{
	// Create the fences and semaphores for synchronization throughout the demo.

	// One of the major changes in strategy introduced in Vulkan has been that there are fewer implicit guarantees as to the order in which commands are executed with respect
	// to other commands on the device and the host itself. Synchronization has now become the responsibility of the application.

	// Here we create the fences and semaphores used for synchronising image acquisition, the use of per frame resources, submission to device queues and finally the
	// presentation of images. Note that the use of these synchronization primitives are explained in detail in the renderFrame function.

	for (uint32_t i = 0; i < _deviceResources->swapchain->getSwapchainLength(); ++i)
	{
		// Semaphores are used for inserting dependencies between batches submitted to queues.
		_deviceResources->presentationSemaphores[i] = _deviceResources->device->createSemaphore();
		_deviceResources->imageAcquireSemaphores[i] = _deviceResources->device->createSemaphore();

		// Fences are used for indicating a dependency from the queue to the host.
		// The fences are created in the signalled state meaning we don't require any special logic for handling the first frame synchronization.
		_deviceResources->perFrameResourcesFences[i] = _deviceResources->device->createFence(pvrvk::FenceCreateFlags::e_SIGNALED_BIT);
	}
}

/// <summary>This function must be implemented by the user of the shell. The user should return its pvr::Shell object defining the behaviour of the application.</summary>
/// <returns>Return a unique ptr to the demo supplied by the user.</returns>
std::unique_ptr<pvr::Shell> pvr::newDemo() { return std::make_unique<VulkanIntroducingPVRVk>(); }
