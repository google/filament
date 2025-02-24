/*!*********************************************************************************************************************
\File         VulkanHelloAPI.cpp
\Title        Vulkan HelloAPI
\Author       PowerVR by Imagination, Developer Technology Team.
\Copyright    Copyright(c) Imagination Technologies Limited.
\brief        Build an introductory Vulkan application to show the process of getting started with Vulkan.
***********************************************************************************************************************/
#include "VulkanHelloAPI.h"
#include "VertShader.h"
#include "FragShader.h"

const static uint32_t NumInstanceLayers = 3;

const std::string InstanceLayers[NumInstanceLayers] = {
#ifdef DEBUG
	// Khronos Validation is a layer which encompasses all of the functionality that used to be contained in VK_LAYER_GOOGLE_threading,
	// VK_LAYER_LUNARG_parameter_validation, VK_LAYER_LUNARG_object_tracker, VK_LAYER_LUNARG_core_validation, and VK_LAYER_GOOGLE_unique_objects
	"VK_LAYER_KHRONOS_validation",
	// Standard Validation is a (now deprecated) meta-layer managed by the LunarG Loader.
	// Using Standard Validation will cause the loader to load a standard set of validation layers in an optimal order:
	// * VK_LAYER_GOOGLE_threading.
	// * VK_LAYER_LUNARG_parameter_validation.
	// * VK_LAYER_LUNARG_object_tracker.
	// * VK_LAYER_LUNARG_core_validation.
	// * VK_LAYER_GOOGLE_unique_objects.
	"VK_LAYER_LUNARG_standard_validation",
	// PerfDoc is a Vulkan layer which attempts to identify API usage that may be discouraged, primarily by validating applications
	// against the rules set out in the Mali Application Developer Best Practices document.
	"VK_LAYER_ARM_mali_perf_doc"
#else
	""
#endif
};

/// <summary>Checks whether validation layers that are supported have been enabled by the application</summary>
/// <param name="layerProperties">Vector of supported layer properties</param>
/// <param name="layersToEnable">Array of the names of layers which the application wants to enable</param>
/// <param name="layerCount">Size of layersToEnable</param>
/// <returns>Vector of the names of validation layer names which are supported and required by the application.</returns>
std::vector<std::string> filterLayers(const std::vector<VkLayerProperties>& layerProperties, const std::string* layersToEnable, uint32_t layerCount)
{
	// For each layer supported by a particular device, check whether the application has chosen to enable it. If the chosen layer to enable exists in the list
	// of layers to enable, then add the layer to a list of layers to return to the application.

	std::vector<std::string> outLayers;
	for (const auto& layerProperty : layerProperties)
	{
		for (uint32_t j = 0; j < layerCount; ++j)
		{
			if (!strcmp(layersToEnable[j].c_str(), layerProperty.layerName)) { outLayers.emplace_back(layersToEnable[j]); }
		}
	}
	return outLayers;
}

/// <summary>Gets the minimum aligned data size based on the size of the data to align and the minimum alignment size specified</summary>
/// <param name="dataSize">The size of the data to align based on the minimum alignment</param>
/// <param name="minimumAlignment">The minimum data size alignment supported</param>
/// <returns>The minimum aligned data size</returns>
inline size_t getAlignedDataSize(size_t dataSize, size_t minimumAlignment)
{
	return (dataSize / minimumAlignment) * minimumAlignment + ((dataSize % minimumAlignment) > 0 ? minimumAlignment : 0);
}

/// <summary>Finds the names of the required validation layers</summary>
/// <returns>Vector of the names of required validation layers which are to be activated</returns>
std::vector<std::string> VulkanHelloAPI::initLayers()
{
	// Due to the (intentionally) limited overhead in Vulkan, error checking is virtually non-existent.
	// Validation layers provide some error checking functionality but they will first need to be initialised.

	// Concept: Validation Layers
	// Validation layers help in tracking API objects and calls, making sure there are no validity errors in the code.
	// They are initialised by the Vulkan loader when vk::CreateInstance is called.

	// This vector will store the supported instance layers that will be returned.
	std::vector<std::string> layerNames;

	// This ensures validation layers will only be enabled during
	// debugging, reducing the overhead of the final release version.
#ifdef PVR_DEBUG
	// Create a vector to hold the layer properties.
	std::vector<VkLayerProperties> outLayers;
	uint32_t numItems = 0;
	// Enumerate all the layer properties to find the total number of items to add to the vector created above.
	debugAssertFunctionResult(vk::EnumerateInstanceLayerProperties(&numItems, nullptr), "Fetching Layer count");

	// Resize the vector to hold the result from vk::EnumerateInstanceLayerProperties.
	outLayers.resize(numItems);

	// Enumerate once more, this time pass the vector and fetch the layer properties themselves to store them in the vector.
	debugAssertFunctionResult(vk::EnumerateInstanceLayerProperties(&numItems, outLayers.data()), "Fetching Layer Data");

	// Log the supported layers on this system.
	Log(false, "---------- Supported Layers ----------");
	for (auto&& layer : outLayers) { Log(false, ">> %s", layer.layerName); }
	Log(false, "--------------------------------------");

	layerNames = filterLayers(outLayers, InstanceLayers, NumInstanceLayers);

	bool requestedStdValidation = false;
	bool supportsStdValidation = false;
	bool supportsKhronosValidation = false;
	uint32_t stdValidationRequiredIndex = -1;

	for (const auto& InstanceLayer : InstanceLayers)
	{
		if (!strcmp(InstanceLayer.c_str(), "VK_LAYER_LUNARG_standard_validation"))
		{
			requestedStdValidation = true;
			break;
		}
	}

	// This code is to cover cases where VK_LAYER_LUNARG_standard_validation is requested but is not supported. This is where on some platforms the
	// component layers enabled via VK_LAYER_LUNARG_standard_validation may still be supported, even though VK_LAYER_LUNARG_standard_validation itself is not.
	if (requestedStdValidation)
		for (const auto& SupportedInstanceLayer : layerNames)
		{
			if (!strcmp(SupportedInstanceLayer.c_str(), "VK_LAYER_LUNARG_standard_validation")) { supportsStdValidation = true; }
			if (!strcmp(SupportedInstanceLayer.c_str(), "VK_LAYER_KHRONOS_validation")) { supportsKhronosValidation = true; }
		}

	// This code is to cover cases where VK_LAYER_LUNARG_standard_validation is requested but is not supported, where on some platforms the
	// component layers enabled via VK_LAYER_LUNARG_standard_validation may still be supported even though VK_LAYER_LUNARG_standard_validation is not.
	// Only perform the expansion if VK_LAYER_LUNARG_standard_validation is requested and not supported and the newer equivalent layer VK_LAYER_KHRONOS_validation is also not supported
	if (requestedStdValidation && !supportsStdValidation && !supportsKhronosValidation)
	{
		for (auto it = outLayers.begin(); !supportsStdValidation && it != outLayers.end(); ++it)
		{ supportsStdValidation = !strcmp(it->layerName, "VK_LAYER_LUNARG_standard_validation"); }
		if (!supportsStdValidation)
		{
			for (uint32_t i = 0; stdValidationRequiredIndex == static_cast<uint32_t>(-1) && i < outLayers.size(); ++i)
			{
				if (!strcmp(InstanceLayers[i].c_str(), "VK_LAYER_LUNARG_standard_validation")) { stdValidationRequiredIndex = i; }
			}

			for (uint32_t j = 0; j < NumInstanceLayers; ++j)
			{
				if (stdValidationRequiredIndex == j && !supportsStdValidation)
				{
					const char* stdValComponents[] = { "VK_LAYER_GOOGLE_threading", "VK_LAYER_LUNARG_parameter_validation", "VK_LAYER_LUNARG_object_tracker",
						"VK_LAYER_LUNARG_core_validation", "VK_LAYER_GOOGLE_unique_objects" };
					for (auto& stdValComponent : stdValComponents)
					{
						for (auto& outLayer : outLayers)
						{
							if (!strcmp(stdValComponent, outLayer.layerName))
							{
								layerNames.emplace_back(stdValComponent);
								break;
							}
						}
					}
				}
			}

			// Filter the layers again. This time checking for support for the component layers enabled via VK_LAYER_LUNARG_standard_validation.
			layerNames = filterLayers(outLayers, layerNames.data(), static_cast<uint32_t>(layerNames.size()));
		}
	}

	Log(false, "---------- Supported Layers to be enabled ----------");
	for (auto&& layer : layerNames) { Log(false, ">> %s", layer.c_str()); }
	Log(false, "--------------------------------------");
#endif

	return layerNames;
}

/// <summary>Selects required instance-level extensions</summary>
/// <returns>Vector of the names of required instance-level extensions</returns>
std::vector<std::string> VulkanHelloAPI::initInstanceExtensions()
{
	// Concept: Extensions
	// Extensions extend the API's functionality; they may add additional features or commands. They can be used for a variety of purposes,
	// such as providing compatibility for specific hardware. Instance-level extensions are extensions with global-functionality; they affect
	// both the instance-level and device-level commands. Device-level extensions specifically affect the device they are bound to.
	// Surface and swapchain functionality are both found in extensions as Vulkan does not make assumptions about the type of application, as not all applications are graphical;
	// for example - compute applications. For this reason they are both considered extensions that add functionality to the core API. The surface extension is an instance-level
	// extension and is added to the instanceExtensionNames vector, while the swapchain is a device-level one and is added to deviceExtensionNames.

	// This function selects the two instance-level extensions which are required by this application.

	// This vector will store a list of supported instance extensions that will be returned. The general surface extension is added to this vector first.
	std::vector<std::string> extensionNames;

	extensionNames.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);

	// An additional surface extension needs to be loaded. This extension is platform-specific so needs to be selected based on the
	// platform the example is going to be deployed to.
	// Preprocessor directives are used here to select the correct platform.
#ifdef VK_USE_PLATFORM_WIN32_KHR
	extensionNames.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
	extensionNames.emplace_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
	extensionNames.emplace_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
	extensionNames.emplace_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
	extensionNames.emplace_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
#endif
#ifdef VK_USE_PLATFORM_MACOS_MVK
	extensionNames.emplace_back(VK_MVK_MACOS_SURFACE_EXTENSION_NAME);
#endif
#ifdef USE_PLATFORM_NULLWS
	extensionNames.emplace_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#endif

	return extensionNames;
}

/// <summary>Selects required device-level extensions</summary>
/// <returns>Vector of the names of required device-level extensions</returns>
std::vector<std::string> VulkanHelloAPI::initDeviceExtensions()
{
	// The VK_KHR_swapchain extension is device-level. The device-level extension names are stored in a
	// separate vector from the instance-level extension names.
	std::vector<std::string> extensionNames;
	extensionNames.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	return extensionNames;
}

/// <summary>Creates a Vulkan instance</summary>
/// <param name="extensionNames">Vector of the names of the required instance-level extensions</param>
/// <param name="layerNames">Vector of the names of the required validation layers</param>
void VulkanHelloAPI::initApplicationAndInstance(std::vector<std::string>& extensionNames, std::vector<std::string>& layerNames)
{
	// This is where the Vulkan instance is created. Vulkan does not have a global state like OpenGL, so a
	// handle is required to access its functions. The instance is the primary access to the API.

	// Declare and populate the application info.
	// When creating objects in Vulkan using "vkCreate..." functions, a creation struct must be defined. This struct contains information describing the properties of the
	// object which is going to be created. In this case, applicationInfo contains properties such as the chosen name of the application and the version of Vulkan used.
	VkApplicationInfo applicationInfo = {};
	applicationInfo.pNext = nullptr;
	applicationInfo.pApplicationName = "Vulkan Hello API Sample";
	applicationInfo.applicationVersion = 1;
	applicationInfo.engineVersion = 1;
	applicationInfo.pEngineName = "Vulkan Hello API Sample";
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.apiVersion = VK_API_VERSION_1_0;

	// Declare an instance creation info struct.
	// instanceInfo specifies the parameters of a newly created Vulkan instance. The
	// application info struct populated above is referenced here along with the instance layers and extensions.
	VkInstanceCreateInfo instanceInfo = {};
	instanceInfo.pNext = nullptr;
	instanceInfo.flags = 0;
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &applicationInfo;

	// Assign the number and names of the instance layers to be enabled.
	appManager.instanceLayerNames.resize(layerNames.size());
	for (uint32_t i = 0; i < layerNames.size(); ++i) { appManager.instanceLayerNames[i] = layerNames[i].c_str(); }

	instanceInfo.enabledLayerCount = static_cast<uint32_t>(appManager.instanceLayerNames.size());
	instanceInfo.ppEnabledLayerNames = appManager.instanceLayerNames.data();

	// Assign the number and names of the instance extensions to be enabled.
	appManager.instanceExtensionNames.resize(extensionNames.size());
	for (uint32_t i = 0; i < extensionNames.size(); ++i) { appManager.instanceExtensionNames[i] = extensionNames[i].c_str(); }

	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(appManager.instanceExtensionNames.size());
	instanceInfo.ppEnabledExtensionNames = appManager.instanceExtensionNames.data();

	// Create a Vulkan application instance using the instanceInfo struct defined above.
	// The handle to this new instance is stored in appManager.instance for access elsewhere.
	debugAssertFunctionResult(vk::CreateInstance(&instanceInfo, nullptr, &appManager.instance), "Create Instance");

	// The pointers to the functions which depend on the Vulkan instance need to be initialised. GetInstanceProcAddr is used to find the correct function
	// pointer associated with this instance. This is not necessary but it is a best practice. It provides a way to bypass the Vulkan loader and grants a
	// small performance boost.
	if (!vk::initVulkanInstance(appManager.instance)) { Log(true, "Could not initialise the instance function pointers."); }
}

/// <summary>Selects the physical device most compatible with application requirements</summary>
void VulkanHelloAPI::initPhysicalDevice()
{
	// Concept: Physical Devices
	// A physical device needs to be chosen. A physical device represents a GPU used for operations.
	// All physical devices will be queried, and the device with the greatest compatibility with the application's needs will be used.

	// This function selects an available physical device which is most compatible with the application's requirements.

	// This will hold the number of GPUs available.
	uint32_t gpuCount;

	// Query for the number of GPUs available.
	debugAssertFunctionResult(vk::EnumeratePhysicalDevices(appManager.instance, &gpuCount, nullptr), "GPUS Enumeration - Get Count");

	// Resize the GPUs vector to match the number of GPUs available.
	appManager.gpus.resize(gpuCount);

	// Populate the vector with a list of GPUs available on the platform.
	debugAssertFunctionResult(vk::EnumeratePhysicalDevices(appManager.instance, &gpuCount, appManager.gpus.data()), "GPUS Enumeration - Allocate Data");

	// Log some properties for each of the available physical devices.
	Log(false, "%s", "------------Properties for Physical Devices--------------");
	for (const auto& device : appManager.gpus)
	{
		// General device properties like vendor and driver version.
		VkPhysicalDeviceProperties deviceProperties;
		vk::GetPhysicalDeviceProperties(device, &deviceProperties);

		Log(false, "Device Name: %s", deviceProperties.deviceName);
		Log(false, "Device ID: 0x%X", deviceProperties.deviceID);
		Log(false, "Device Driver Version: 0x%X", deviceProperties.driverVersion);
		Log(false, "%s", "--------------------------------------");

		// Features are more in-depth information that is not needed right now so these are not outputted.
		VkPhysicalDeviceFeatures deviceFeatures;
		vk::GetPhysicalDeviceFeatures(device, &deviceFeatures);
	}

	// Get the device compatible with the needs of the application using a custom helper function.
	// The physical device is also queried for its memory properties which will be used later when allocating memory for buffers.
	appManager.physicalDevice = getCompatibleDevice();
	vk::GetPhysicalDeviceMemoryProperties(appManager.physicalDevice, &appManager.deviceMemoryProperties);

	// Get the compatible device's properties.
	// These properties will be used later when creating the surface and swapchain objects.
	vk::GetPhysicalDeviceProperties(appManager.physicalDevice, &appManager.deviceProperties);
}

/// <summary>Queries the physical device for supported queue families</summary>
void VulkanHelloAPI::initQueuesFamilies()
{
	// Concept: Queues and Queues Families
	// Queues are needed by Vulkan to execute commands on, such as drawing or memory transfers.
	// Queue families are in their simplest form a collection of queues that share properties related to the type of commands allowed to execute.
	// Queue families make sure that the collection of queues being used is compatible with the operations that the developer wants to execute.

	// This function queries the physical device for supported queue families and then identifies two queue families which support rendering and presenting.
	// These could be the same if one queue family supports both operations but this will be dealt with later.

	// This will hold the number of queue families available.
	uint32_t queueFamiliesCount;

	// Get the number of queue families the physical device supports.
	vk::GetPhysicalDeviceQueueFamilyProperties(appManager.physicalDevice, &queueFamiliesCount, nullptr);

	// Resize the vector to fit the number of supported queue families.
	appManager.queueFamilyProperties.resize(queueFamiliesCount);

	// Load the queue families data from the physical device to the list.
	vk::GetPhysicalDeviceQueueFamilyProperties(appManager.physicalDevice, &queueFamiliesCount, &appManager.queueFamilyProperties[0]);

	// Get the indices of compatible queue families.
	getCompatibleQueueFamilies(appManager.graphicsQueueFamilyIndex, appManager.presentQueueFamilyIndex);
}

/// <summary>Creates a Vulkan logical device</summary>
/// <param name="deviceExtensions">Vector of the names of the required device-level extensions</param>
void VulkanHelloAPI::initLogicalDevice(std::vector<std::string>& deviceExtensions)
{
	// A logical device is required to start using the API. This function creates a logical device
	// and a graphics queue to execute commands on.

	// Concept: Logical Devices
	// A logical device is an application view of the physical device that will be used. The logical device is
	// used to load the device extensions and create the rest of the Vulkan API objects.

	// There are priorities for queues (range: 0 - 1). Each queue in the same device is assigned a priority with higher priority queues
	// potentially being given more processing time than lower priority ones.
	// In this case there is only one, so it does not matter.
	float queuePriorities[1] = { 0.0f };

	// Populate the device queue creation info struct with the previously found compatible queue family
	// and number of queues to be created. Again, only one queue is needed.
	VkDeviceQueueCreateInfo deviceQueueInfo = {};
	deviceQueueInfo.pNext = nullptr;
	deviceQueueInfo.flags = 0;
	deviceQueueInfo.queueFamilyIndex = appManager.graphicsQueueFamilyIndex;
	deviceQueueInfo.pQueuePriorities = queuePriorities;
	deviceQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueInfo.queueCount = 1;

	// Declare and populate the logical device creation info struct. This will be used to create the logical device and its associated queues.
	// The device extensions that were looked up earlier are specified here. They will be initialised when the logical device
	// is created. Additionally, the physical device is queried for its supported features so the logical device can enable them.
	VkDeviceCreateInfo deviceInfo;
	deviceInfo.flags = 0;
	deviceInfo.pNext = nullptr;
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.enabledLayerCount = 0;
	deviceInfo.ppEnabledLayerNames = nullptr;

	appManager.deviceExtensionNames.resize(deviceExtensions.size());
	for (uint32_t i = 0; i < deviceExtensions.size(); ++i) { appManager.deviceExtensionNames[i] = deviceExtensions[i].c_str(); }

	deviceInfo.enabledExtensionCount = static_cast<uint32_t>(appManager.deviceExtensionNames.size());
	deviceInfo.ppEnabledExtensionNames = appManager.deviceExtensionNames.data();
	deviceInfo.queueCreateInfoCount = 1;
	deviceInfo.pQueueCreateInfos = &deviceQueueInfo;
	VkPhysicalDeviceFeatures features;
	vk::GetPhysicalDeviceFeatures(appManager.physicalDevice, &features);
	features.robustBufferAccess = false;
	deviceInfo.pEnabledFeatures = &features;

	// Create the logical device using the deviceInfo struct defined above.
	debugAssertFunctionResult(vk::CreateDevice(appManager.physicalDevice, &deviceInfo, nullptr, &appManager.device), "Logic Device Creation");

	// Initialise the function pointers that require the device address. This is the same process as for the instance function pointers.
	if (!vk::initVulkanDevice(appManager.device)) { Log(true, "Could not initialise the device function pointers."); }
}

/// <summary>Creates a rendering and a present queue for executing commands</summary>
void VulkanHelloAPI::initQueues()
{
	// The queues that will be used for executing commands on needs to be retrieved.
	// Two queues are needed: one for rendering and the other to present the rendering on the surface.
	// Some devices support both operations on the same queue family.

	// Get the queue from the logical device created earlier and save it for later.
	vk::GetDeviceQueue(appManager.device, appManager.graphicsQueueFamilyIndex, 0, &appManager.graphicQueue);

	// If the queue family indices are the same, then the same queue is used to do both operations.
	// If not, another queue is retrieved for presenting.
	if (appManager.graphicsQueueFamilyIndex == appManager.presentQueueFamilyIndex) { appManager.presentQueue = appManager.graphicQueue; }
	else
	{
		vk::GetDeviceQueue(appManager.device, appManager.presentQueueFamilyIndex, 0, &appManager.presentQueue);
	}
}

/// <summary>Initialises the surface that will be presented to</summary>
void VulkanHelloAPI::initSurface()
{
	// This function initialises the surface that will be needed to present this rendered example.

	// Surfaces are based on the platform (OS) that is being deployed to.
	// Pre-processors are used to select the correct function call and info struct data type to create a surface.

	// For Win32.
#ifdef VK_USE_PLATFORM_WIN32_KHR

	// Declare and populate the Win32 surface creation info struct with Win32 window instance and window handles.
	VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.flags = 0;
	surfaceInfo.pNext = nullptr;
	surfaceInfo.hinstance = surfaceData.connection;
	surfaceInfo.hwnd = surfaceData.window;
	surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;

	// Create the surface that will be rendered on using the creation info defined above.
	debugAssertFunctionResult(vk::CreateWin32SurfaceKHR(appManager.instance, &surfaceInfo, nullptr, &appManager.surface), "Windows Surface Creation");
#endif

	// For Xlib.
#ifdef VK_USE_PLATFORM_XLIB_KHR
	// Declare and populate the Xlib surface creation info struct, passing the Xlib display and window handles.
	VkXlibSurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.flags = 0;
	surfaceInfo.pNext = nullptr;
	surfaceInfo.dpy = surfaceData.display;
	surfaceInfo.window = surfaceData.window;

	// Create the xlib surface that will be presented on using the creation info defined above.
	debugAssertFunctionResult(vk::CreateXlibSurfaceKHR(appManager.instance, &surfaceInfo, nullptr, &appManager.surface), "XLIB Surface Creation");
#endif

	// For Xcb.
#ifdef VK_USE_PLATFORM_XCB_KHR

	// Declare and populate the Xlib surface creation info struct, passing the Xcb display and window handles.
	VkXcbSurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.connection = surfaceData.connection;
	surfaceInfo.window = surfaceData.window;

	// Create the xcb surface that will be presented on.
	debugAssertFunctionResult(vk::CreateXcbSurfaceKHR(appManager.instance, &surfaceInfo, nullptr, &appManager.surface), "XCB Surface Creation");
#endif

	// For Android.
#ifdef VK_USE_PLATFORM_ANDROID_KHR

	// Declare and populate the Android surface creation info struct, passing the Android window handle.
	VkAndroidSurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.flags = 0;
	surfaceInfo.pNext = 0;
	surfaceInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.window = surfaceData.window;

	// Create the Android surface that will be presented on using the creation info defined above.
	debugAssertFunctionResult(vk::CreateAndroidSurfaceKHR(appManager.instance, &surfaceInfo, nullptr, &appManager.surface), "Android Surface Creation");

#endif

	// For Wayland.
#ifdef VK_USE_PLATFORM_WAYLAND_KHR

	// Declare and populate the Wayland surface creation info struct, passing the Wayland display and surface handles.
	VkWaylandSurfaceCreateInfoKHR surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.display = surfaceData.display;
	surfaceInfo.surface = surfaceData.surface;

	// Create the Wayland surface that will be presented on using the creation info defined above.
	debugAssertFunctionResult(vk::CreateWaylandSurfaceKHR(appManager.instance, &surfaceInfo, NULL, &appManager.surface), "Wayland Surface Creation");

#endif

	// For MoltenVK
#ifdef VK_USE_PLATFORM_MACOS_MVK

	// Create the MacOS surface info, passing the NSView handle
	VkMacOSSurfaceCreateInfoMVK surfaceInfo = {};
	surfaceInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
	surfaceInfo.pNext = 0;
	surfaceInfo.flags = 0;
	// pView must be a valid NSView and must be backed by a CALayer instance of type CAMetalLayer.
	surfaceInfo.pView = surfaceData.view;

	// Create the MacOS surface that will be presented on.
	debugAssertFunctionResult(vk::CreateMacOSSurfaceMVK(appManager.instance, &surfaceInfo, NULL, &appManager.surface), "MacOS Surface Creation");

#endif

	// For NullWS
#ifdef USE_PLATFORM_NULLWS

	VkDisplayPropertiesKHR properties;
	uint32_t propertiesCount = 1;
	if (vk::GetPhysicalDeviceDisplayPropertiesKHR) { lastRes = vk::GetPhysicalDeviceDisplayPropertiesKHR(appManager.physicalDevice, &propertiesCount, &properties); }

	std::string supportedTransforms;
	if (properties.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) { supportedTransforms.append("none "); }
	if (properties.supportedTransforms & VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR) { supportedTransforms.append("rot90 "); }
	if (properties.supportedTransforms & VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR) { supportedTransforms.append("rot180 "); }
	if (properties.supportedTransforms & VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR) { supportedTransforms.append("rot270 "); }
	if (properties.supportedTransforms & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR) { supportedTransforms.append("h_mirror "); }
	if (properties.supportedTransforms & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR) { supportedTransforms.append("h_mirror+rot90 "); }
	if (properties.supportedTransforms & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR) { supportedTransforms.append("hmirror+rot180 "); }
	if (properties.supportedTransforms & VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR) { supportedTransforms.append("hmirror+rot270 "); }
	if (properties.supportedTransforms & VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR) { supportedTransforms.append("inherit "); }

	VkDisplayKHR nativeDisplay = properties.display;

	uint32_t modeCount = 0;
	vk::GetDisplayModePropertiesKHR(appManager.physicalDevice, nativeDisplay, &modeCount, NULL);
	std::vector<VkDisplayModePropertiesKHR> modeProperties;
	modeProperties.resize(modeCount);
	vk::GetDisplayModePropertiesKHR(appManager.physicalDevice, nativeDisplay, &modeCount, modeProperties.data());

	VkDisplaySurfaceCreateInfoKHR surfaceInfo = {};

	surfaceInfo.sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR;
	surfaceInfo.pNext = NULL;

	surfaceInfo.displayMode = modeProperties[0].displayMode;
	surfaceInfo.planeIndex = 0;
	surfaceInfo.planeStackIndex = 0;
	surfaceInfo.transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	surfaceInfo.globalAlpha = 0.0f;
	surfaceInfo.alphaMode = VK_DISPLAY_PLANE_ALPHA_PER_PIXEL_BIT_KHR;
	surfaceInfo.imageExtent = modeProperties[0].parameters.visibleRegion;

	debugAssertFunctionResult(vk::CreateDisplayPlaneSurfaceKHR(appManager.instance, &surfaceInfo, nullptr, &appManager.surface), "Surface Creation");
#endif
}

/// <summary>Creates a swapchain and defines its properties</summary>
void VulkanHelloAPI::initSwapChain()
{
	// If an application being developed needs to display something, then a swapchain is required.
	// This function creates a swapchain and defines its properties.

	// Concept: Swapchain
	// A swapchain is a series of images that are used to render and then present to the surface.
	// On changing the screen size or other changes, the swapchain needs to be destroyed
	// and recreated at runtime.

	// These variables are used to store the surface formats that have been retrieved from the physical device.
	uint32_t formatsCount;
	std::vector<VkSurfaceFormatKHR> formats;

	// Get the number of surface formats supported by the physical device.
	debugAssertFunctionResult(vk::GetPhysicalDeviceSurfaceFormatsKHR(appManager.physicalDevice, appManager.surface, &formatsCount, nullptr), "Swap Chain Format - Get Count");

	// Resize formats vector to the number of supported surface formats.
	formats.resize(formatsCount);

	// Populate the vector list with the surface formats.
	debugAssertFunctionResult(vk::GetPhysicalDeviceSurfaceFormatsKHR(appManager.physicalDevice, appManager.surface, &formatsCount, formats.data()), "Swap Chain Format - Allocate Data");

	// If the first format is undefined then pick a default format. VK_FORMAT_B8G8R8A8_UNORM is a very common image format.
	// Otherwise if the first format is defined choose that one.
	if (formatsCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
	{
		appManager.surfaceFormat.format = VK_FORMAT_B8G8R8A8_UNORM; // unsigned normalised BGRA with 8-bit in each component.
	}
	else
	{
		appManager.surfaceFormat = formats[0];
	}

	// Get the surface capabilities from the surface and the physical device.
	VkSurfaceCapabilitiesKHR surface_capabilities;
	debugAssertFunctionResult(vk::GetPhysicalDeviceSurfaceCapabilitiesKHR(appManager.physicalDevice, appManager.surface, &surface_capabilities), "Fetch Surface Capabilities");

	// Concept: Present Modes
	// Present modes are the methods with which images are presented to the surface.

	// The presentation modes that are supported by the surface need to be determined.

	// These variables are used to store the presentation modes that have been retrieved from the physical device.
	uint32_t presentModesCount;
	std::vector<VkPresentModeKHR> presentModes;

	// Get the number of supported present modes.
	debugAssertFunctionResult(
		vk::GetPhysicalDeviceSurfacePresentModesKHR(appManager.physicalDevice, appManager.surface, &presentModesCount, nullptr), "Surface Present Modes - Get Count");

	// Resize the vector and retrieve the supported present modes.
	presentModes.resize(presentModesCount);
	debugAssertFunctionResult(vk::GetPhysicalDeviceSurfacePresentModesKHR(appManager.physicalDevice, appManager.surface, &presentModesCount, presentModes.data()),
		"Surface Present Modes - Allocate Data");

	// Make use of VK_PRESENT_MODE_FIFO_KHR for presentation.
	appManager.presentMode = VK_PRESENT_MODE_FIFO_KHR;

	// Get the correct extent (dimensions) of the surface using a helper function.
	appManager.swapchainExtent = getCorrectExtent(surface_capabilities);

	// Get the minimum number of images supported on this surface.
	uint32_t surfaceImageCount = std::max<uint32_t>(3, surface_capabilities.minImageCount);

	// Populate a swapchain creation info struct with the information specified above.
	// The additional parameters specified here include what transformations to apply to the image before
	// presentation, how this surface will be composited with other surfaces, whether the implementation
	// can discard rendering operations that affect regions of the surface that are not visible, and the intended
	// usage of the swapchain images.
	VkSwapchainCreateInfoKHR swapchainInfo = {};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.flags = 0;
	swapchainInfo.pNext = nullptr;
	swapchainInfo.surface = appManager.surface;
	swapchainInfo.imageFormat = appManager.surfaceFormat.format;
	swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	if ((surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) == 0)
	{
		Log(true, "Surface does not support VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR transformation");
		assert(surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR);
	}

	VkCompositeAlphaFlagBitsKHR supportedCompositeAlphaFlags = (VkCompositeAlphaFlagBitsKHR)0;
	if ((surface_capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) != 0) { supportedCompositeAlphaFlags = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; }
	else if ((surface_capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) != 0)
	{
		supportedCompositeAlphaFlags = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	}
	swapchainInfo.compositeAlpha = supportedCompositeAlphaFlags;
	swapchainInfo.presentMode = appManager.presentMode;
	swapchainInfo.minImageCount = surfaceImageCount;
	swapchainInfo.oldSwapchain = VK_NULL_HANDLE;
	swapchainInfo.clipped = VK_TRUE;
	swapchainInfo.imageExtent.width = appManager.swapchainExtent.width;
	swapchainInfo.imageExtent.height = appManager.swapchainExtent.height;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageColorSpace = appManager.surfaceFormat.colorSpace;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// Fix the height and width of the surface in case they are not defined.
	if (surfaceData.width == 0 || surfaceData.height == 0)
	{
		surfaceData.width = static_cast<float>(swapchainInfo.imageExtent.width);
		surfaceData.height = static_cast<float>(swapchainInfo.imageExtent.height);
	}

	// Check if the present queue and the graphics queue are the same.
	// If they are, images do not need to be shared between multiple queues, so exclusive mode is selected.
	// If not, sharing mode concurrent is selected to allow these images to be accessed from multiple queue families simultaneously.
	if (appManager.graphicsQueueFamilyIndex == appManager.presentQueueFamilyIndex)
	{
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.queueFamilyIndexCount = 0;
		swapchainInfo.pQueueFamilyIndices = nullptr;
	}
	else
	{
		uint32_t queueFamilyIndices[] = { appManager.graphicsQueueFamilyIndex, appManager.presentQueueFamilyIndex };

		swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.queueFamilyIndexCount = 2;
		swapchainInfo.pQueueFamilyIndices = queueFamilyIndices;
	}

	// Finally, create the swapchain.
	debugAssertFunctionResult(vk::CreateSwapchainKHR(appManager.device, &swapchainInfo, nullptr, &appManager.swapchain), "SwapChain Creation");
}

/// <summary>Initialises the images of a previously created swapchain and creates an associated image view for each image</summary>
void VulkanHelloAPI::initImagesAndViews()
{
	// Concept: Images and Views
	// Images in Vulkan are the object representation of data. It can take many forms such as attachments, textures, and so on.
	// Views are a snapshot of the image's parameters. It describes how to access the image and which parts to access.
	// It helps to distinguish the type of image that is being working with.

	// Image objects are used to hold the swapchain images. When the swapchain was created, the
	// images were automatically created alongside it. This function creates an image view for each swapchain image.

	// This vector is used as a temporary vector to hold the retrieved images.
	uint32_t swapchainImageCount;
	std::vector<VkImage> images;

	// Get the number of the images which are held by the swapchain. This is set in InitSwapchain function and is the minimum number of images supported.
	debugAssertFunctionResult(vk::GetSwapchainImagesKHR(appManager.device, appManager.swapchain, &swapchainImageCount, nullptr), "SwapChain Images - Get Count");

	// Resize the temporary images vector to hold the number of images.
	images.resize(swapchainImageCount);

	// Resize the application's permanent swapchain images vector to be able to hold the number of images.
	appManager.swapChainImages.resize(swapchainImageCount);

	// Get all of the images from the swapchain and save them in a temporary vector.
	debugAssertFunctionResult(vk::GetSwapchainImagesKHR(appManager.device, appManager.swapchain, &swapchainImageCount, images.data()), "SwapChain Images - Allocate Data");

	// Iterate over each image in order to create an image view for each one.
	for (uint32_t i = 0; i < swapchainImageCount; ++i)
	{
		// Copy over the images to the permanent vector.
		appManager.swapChainImages[i].image = images[i];

		// Create the image view object itself, referencing one of the retrieved swapchain images.
		VkImageViewCreateInfo image_view_info = {};
		image_view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		image_view_info.pNext = nullptr;
		image_view_info.flags = 0;
		image_view_info.image = appManager.swapChainImages[i].image;
		image_view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_info.format = appManager.surfaceFormat.format;

		image_view_info.components.r = VK_COMPONENT_SWIZZLE_R;
		image_view_info.components.g = VK_COMPONENT_SWIZZLE_G;
		image_view_info.components.b = VK_COMPONENT_SWIZZLE_B;
		image_view_info.components.a = VK_COMPONENT_SWIZZLE_A;

		image_view_info.subresourceRange.layerCount = 1;
		image_view_info.subresourceRange.levelCount = 1;
		image_view_info.subresourceRange.baseArrayLayer = 0;
		image_view_info.subresourceRange.baseMipLevel = 0;
		image_view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		debugAssertFunctionResult(vk::CreateImageView(appManager.device, &image_view_info, nullptr, &appManager.swapChainImages[i].view), "SwapChain Images View Creation");
	}
}

/// <summary>Creates the vertex and fragment shader modules and loads in compiled SPIR-V code</summary>
void VulkanHelloAPI::initShaders()
{
	// In Vulkan, shaders are in SPIR-V format which is a byte-code format rather than a human-readable one.
	// SPIR-V can be used for both graphical and compute operations.
	// This function loads the compiled source code (see vertshader.h and fragshader.h) and creates shader modules that are going
	// to be used by the pipeline later on.

	createShaderModule(spv_VertShader_bin, sizeof(spv_VertShader_bin), 0, VK_SHADER_STAGE_VERTEX_BIT);

	createShaderModule(spv_FragShader_bin, sizeof(spv_FragShader_bin), 1, VK_SHADER_STAGE_FRAGMENT_BIT);
}

/// <summary>Creates a render pass object and defines its properties.</summary>
void VulkanHelloAPI::initRenderPass()
{
	// Concept: Render passes
	// In Vulkan, a render pass is a collection of data that describes a set of framebuffer
	// attachments that are needed for rendering. A render pass is composed of subpasses that
	// order the data. A render pass collects all the colour, depth, and stencil attachments,
	// making sure to explicitly define them so that the driver does not have to work them out for itself.

	// This function creates a render pass object using the descriptions of a colour attachment and a subpass.

	// Create a description of the colour attachment that will be added to the render pass.
	// This will tell the render pass what to do with the image (framebuffer) before, during, and after rendering.
	// In this case the contents of the image will be cleared at the start of the subpass and stored at the
	// end.
	// Additionally, this description tells Vulkan that only one sample per pixel will be allowed for this image and the pixel layout will
	// be transitioned to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR during the render pass. This layout is used
	// when an image is going to be presented to a surface.
	VkAttachmentDescription colorAttachmentDescription = {};
	colorAttachmentDescription.format = appManager.surfaceFormat.format;
	colorAttachmentDescription.flags = 0;
	colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	colorAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;

	// Create a colour attachment reference.
	// This tells the implementation that the first attachment at index 0 of this render pass will be a colour attachment.
	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Declare and populate a struct which contains a description of the subpass.
	// In this case the subpass only has a single colour attachment and will support a graphics pipeline.
	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.flags = 0;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentReference;
	subpassDescription.pDepthStencilAttachment = nullptr;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;
	subpassDescription.pResolveAttachments = nullptr;

	VkSubpassDependency subpassDependencies[2];
	subpassDependencies[0] = {};
	subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[0].dstSubpass = 0;
	subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[0].srcAccessMask = 0;
	subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	subpassDependencies[1] = {};
	subpassDependencies[1].srcSubpass = 0;
	subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependencies[1].dstAccessMask = 0;
	subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	// Populate a render pass creation info struct.
	// Again, this simply references the single colour attachment and subpass.
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.flags = 0;
	renderPassInfo.pNext = nullptr;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = &colorAttachmentDescription;
	renderPassInfo.pSubpasses = &subpassDescription; // the subpass that was just created.
	renderPassInfo.pDependencies = subpassDependencies;
	renderPassInfo.dependencyCount = 2;

	// Depth or stencil buffers are not needed since this application is simply rendering a
	// triangle with no depth testing.

	// Create the render pass object itself.
	debugAssertFunctionResult(vk::CreateRenderPass(appManager.device, &renderPassInfo, nullptr, &appManager.renderPass), "Render pass Creation");
}

/// <summary>Creates the uniform buffers used throughout the demo</summary>
void VulkanHelloAPI::initUniformBuffers()
{
	// This function creates a dynamic uniform buffer which will hold several transformation matrices. Each of these matrices is associated with a
	// swapchain image created earlier.

	// Vulkan requires that when updating a descriptor of type VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER or VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, the
	// offset specified is an integer multiple of the minimum required alignment in bytes for the physical device. This also applied to any dynamic alignments used.
	size_t minimumUboAlignment = static_cast<size_t>(appManager.deviceProperties.limits.minUniformBufferOffsetAlignment);

	// The dynamic buffers will be used as uniform buffers. These are later used with a descriptor of type VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC and VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER.
	VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

	{
		// Using the minimum uniform buffer offset alignment, the minimum buffer slice size is calculated based on the size of the intended data, or more specifically
		// the size of the smallest chunk of data which may be mapped or updated as a whole.
		// In this case the size of the intended data is the size of a 4 by 4 matrix.
		size_t bufferDataSizePerSwapchain = sizeof(float) * 4 * 4;
		bufferDataSizePerSwapchain = static_cast<uint32_t>(getAlignedDataSize(bufferDataSizePerSwapchain, minimumUboAlignment));

		// Calculate the size of the dynamic uniform buffer.
		// This buffer will be updated on each frame and must therefore be multi-buffered to avoid issues with using partially updated data, or updating data already in use.
		// Rather than allocating multiple (swapchain) buffers, a larger buffer is allocated and a slice of this buffer will be used per swapchain. This works as
		// long as the buffer is created taking into account the minimum uniform buffer offset alignment.
		appManager.dynamicUniformBufferData.size = bufferDataSizePerSwapchain * appManager.swapChainImages.size();

		// Create the buffer, allocate the device memory, and attach the memory to the newly created buffer object.
		createBuffer(appManager.dynamicUniformBufferData, nullptr, usageFlags);
		appManager.dynamicUniformBufferData.bufferInfo.range = bufferDataSizePerSwapchain;

		// Note that only memory created with the memory property flag VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT can be mapped.
		// vkMapMemory retrieves a host virtual address pointer to a region of a mappable memory object.
		debugAssertFunctionResult(vk::MapMemory(appManager.device, appManager.dynamicUniformBufferData.memory, 0, appManager.dynamicUniformBufferData.size, 0,
									  &appManager.dynamicUniformBufferData.mappedData),
			"Could not map the uniform buffer.");
	}
}

/// <summary>Defines the vertices of a simple triangle which can be passed to the vertex shader to be rendered on screen</summary>
void VulkanHelloAPI::initVertexBuffers()
{
	// This function defines the vertices of a simple triangle and creates a vertex buffer to hold this data.

	// Calculate the size of the vertex buffer to be passed to the vertex shader.
	appManager.vertexBuffer.size = sizeof(Vertex) * 3;

	// Set the values for the triangle's vertices.
	Vertex triangle[3];
	triangle[0] = { -0.5f, -0.288f, 0.0f, 1.0f, 0.0f, 0.0f };
	triangle[1] = { .5f, -.288f, 0.0f, 1.0f, 1.0f, 0.0f };
	triangle[2] = { 0.0f, .577f, 0.0f, 1.0f, 0.5f, 1.0f };

	// Create the buffer that will hold the data and be passed to the shaders.
	createBuffer(appManager.vertexBuffer, reinterpret_cast<uint8_t*>(triangle), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
}

/// <summary>Creates a texture image (VkImage) and maps it into GPU memory</summary>
void VulkanHelloAPI::initTexture()
{
	// In Vulkan, uploading an image requires multiple steps:

	// 1) Creating the texture.
	//		a) Create the texture definition ("VkImage" object).
	//		b) Determine its memory requirements and create the backing memory object ("VkDeviceMemory" object).
	//		c) Bind the memory to the image.

	// 2) Uploading the data into the texture.
	//		a) Create a staging buffer.
	//		b) Determine its memory requirements and create the backing memory object ("VkDeviceMemory" object).
	//		c) Map the staging buffer and copy the image data into it.
	//		d) Perform a copy from the staging buffer to the image using the vkCmdCopyBufferToImage command to transfer the data. This requires a command buffer and related objects.

	// A texture (sampled image) is stored in the GPU in an implementation-defined way, which may be completely different
	// to the layout of the texture on the disk/CPU-side.
	// For this reason, it is not possible to map its memory and write the data directly for that image.
	// Using the vkCmdCopyBufferToImage command in the second (uploading) step guarantees the correct
	// translation/swizzling of the texture data.

	// These steps are demonstrated below.

	// Set the width and height of the texture image.
	appManager.texture.textureDimensions.height = 256;
	appManager.texture.textureDimensions.width = 256;
	appManager.texture.data.resize(appManager.texture.textureDimensions.width * appManager.texture.textureDimensions.height * 4);

	// This function generates a texture pattern on-the-fly into a block of CPU-side memory: appManager.texture.data.
	generateTexture();

	// The BufferData struct has been defined in this application to hold the necessary data for the staging buffer.
	BufferData stagingBufferData;
	stagingBufferData.size = appManager.texture.data.size();

	// Use the buffer creation function to generate a staging buffer. The VK_BUFFER_USAGE_TRANSFER_SRC_BIT flag is passed to specify that the buffer
	// is going to be used as the source buffer of a transfer command.
	createBuffer(stagingBufferData, appManager.texture.data.data(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

	// Create the image object.
	// The format is set to the most common format, R8G8B8_UNORM, 8-bits per channel, unsigned, and normalised.
	// Additionally, the dimensions of the image, the number of mipmap levels, the intended usage of the image, the number of samples per texel,
	// and whether this image can be accessed concurrently by multiple queue families are all also set here.
	// Some of the other parameters specified include the tiling and the initialLayout.
	// The tiling parameter determines the layout of texel blocks in memory. This should be set
	// to VK_IMAGE_TILING_OPTIMAL for images used as textures in shaders.
	// The initialLayout parameter is set to VK_IMAGE_LAYOUT_UNDEFINED but the layout will be transitioned
	// later using a barrier.
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.flags = 0;
	imageInfo.pNext = nullptr;
	imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.extent = { appManager.texture.textureDimensions.width, appManager.texture.textureDimensions.height, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;

	debugAssertFunctionResult(vk::CreateImage(appManager.device, &imageInfo, nullptr, &appManager.texture.image), "Texture Image Creation");

	// Get the memory allocation requirements for the image.
	// These are used to allocate memory for the image that has just been created.
	VkMemoryRequirements memoryRequirments;
	vk::GetImageMemoryRequirements(appManager.device, appManager.texture.image, &memoryRequirments);

	// Populate a memory allocation info struct with the memory requirements size for the image.
	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.pNext = nullptr;
	allocateInfo.memoryTypeIndex = 0;
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirments.size;

	// This helper function queries available memory types to find memory with the features that are suitable for a sampled
	// image. Device Local memory is the preferred choice.
	getMemoryTypeFromProperties(appManager.deviceMemoryProperties, memoryRequirments.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &(allocateInfo.memoryTypeIndex));

	// Use all of this information to allocate memory with the correct features for the image and bind the memory to the texture buffer.
	debugAssertFunctionResult(vk::AllocateMemory(appManager.device, &allocateInfo, nullptr, &appManager.texture.memory), "Texture Image Memory Allocation");
	debugAssertFunctionResult(vk::BindImageMemory(appManager.device, appManager.texture.image, appManager.texture.memory, 0), "Texture Image Memory Binding");

	// Specify the region which should be copied from the texture. In this case it is the entire image, so
	// the texture width and height are passed as extents.
	VkBufferImageCopy copyRegion = {};
	copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.imageSubresource.mipLevel = 0;
	copyRegion.imageSubresource.baseArrayLayer = 0;
	copyRegion.imageSubresource.layerCount = 1;
	copyRegion.imageExtent.width = static_cast<uint32_t>(appManager.texture.textureDimensions.width);
	copyRegion.imageExtent.height = static_cast<uint32_t>(appManager.texture.textureDimensions.height);
	copyRegion.imageExtent.depth = 1;
	copyRegion.bufferOffset = 0;

	// Allocate a command buffer from the command pool. This command buffer will be used to execute the copy operation.
	// The allocation info struct below specifies that a single primary command buffer needs
	// to be allocated. Primary command buffers can be contrasted with secondary command buffers
	// which cannot be submitted directly to queues but instead are executed as part of a primary command
	// buffer.
	// The command pool referenced here was created in initCommandPoolAndBuffer().
	VkCommandBuffer commandBuffer;

	VkCommandBufferAllocateInfo commandAllocateInfo = {};
	commandAllocateInfo.commandPool = appManager.commandPool;
	commandAllocateInfo.pNext = nullptr;
	commandAllocateInfo.commandBufferCount = 1;
	commandAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

	debugAssertFunctionResult(vk::AllocateCommandBuffers(appManager.device, &commandAllocateInfo, &commandBuffer), "Allocate Command Buffers");

	// Begin recording the copy commands into the command buffer.
	VkCommandBufferBeginInfo commandBufferBeginInfo = {};
	commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	commandBufferBeginInfo.flags = 0;
	commandBufferBeginInfo.pNext = nullptr;
	commandBufferBeginInfo.pInheritanceInfo = nullptr;

	debugAssertFunctionResult(vk::BeginCommandBuffer(commandBuffer, &commandBufferBeginInfo), "Begin Image Copy to Staging Buffer Command Buffer Recording");

	// Specify the sub resource range of the image. In the case of this image, the parameters are default, with one mipmap level and layer,
	// because the image is very simple.
	VkImageSubresourceRange subResourceRange = {};
	subResourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subResourceRange.baseMipLevel = 0;
	subResourceRange.levelCount = 1;
	subResourceRange.layerCount = 1;

	// A memory barrier needs to be created to make sure that the image layout is set up for a copy operation.
	// The barrier will transition the image layout from VK_IMAGE_LAYOUT_UNDEFINED to
	// VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL. This new layout is optimal for images which are the destination
	// of a transfer command.
	VkImageMemoryBarrier copyMemoryBarrier = {};
	copyMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	copyMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	copyMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	copyMemoryBarrier.image = appManager.texture.image;
	copyMemoryBarrier.subresourceRange = subResourceRange;
	copyMemoryBarrier.srcAccessMask = 0;
	copyMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	// Use the pipeline barrier defined above.
	vk::CmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &copyMemoryBarrier);

	// Copy the staging buffer data to the image that was just created.
	vk::CmdCopyBufferToImage(commandBuffer, stagingBufferData.buffer, appManager.texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

	// Create a barrier to make sure that the image layout is shader read-only.
	// This barrier will transition the image layout from VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL to
	// VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL.
	VkImageMemoryBarrier layoutMemoryBarrier = {};
	layoutMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	layoutMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	layoutMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	layoutMemoryBarrier.image = appManager.texture.image;
	layoutMemoryBarrier.subresourceRange = subResourceRange;
	layoutMemoryBarrier.srcAccessMask = 0;
	layoutMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	// Use a pipeline barrier to change the image layout to be optimised for reading by the shader.
	vk::CmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &layoutMemoryBarrier);

	// End the recording of the command buffer.
	debugAssertFunctionResult(vk::EndCommandBuffer(commandBuffer), "End Image Copy to Staging Buffer Command Buffer Recording");

	// Create a fence object which will signal when all of the commands in this command buffer have been completed.
	VkFence copyFence;
	VkFenceCreateInfo copyFenceInfo = {};
	copyFenceInfo.flags = 0;
	copyFenceInfo.pNext = nullptr;
	copyFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

	debugAssertFunctionResult(vk::CreateFence(appManager.device, &copyFenceInfo, nullptr, &copyFence), "Image Copy to Staging Buffer Fence Creation");

	// Finally, submit the command buffer to the graphics queue to get the GPU to perform the copy operations.
	// When submitting command buffers, it is possible to set wait and signal semaphores to control synchronisation. These
	// are not used here but they will be used later during rendering.
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.pWaitDstStageMask = nullptr;
	submitInfo.waitSemaphoreCount = 0;
	submitInfo.pWaitSemaphores = nullptr;
	submitInfo.signalSemaphoreCount = 0;
	submitInfo.pSignalSemaphores = nullptr;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	debugAssertFunctionResult(vk::QueueSubmit(appManager.graphicQueue, 1, &submitInfo, copyFence), "Submit Image Copy to Staging Buffer Command Buffer");

	// Wait for the fence to be signalled. This ensures the command buffer has finished executing.
	debugAssertFunctionResult(vk::WaitForFences(appManager.device, 1, &copyFence, VK_TRUE, FENCE_TIMEOUT), "Image Copy to Staging Buffer Fence Signal");

	// After the image is complete and all the texture data has been copied, an image view needs to be created to make sure
	// that the API can understand what the image is. For example, information can be provided on the format or view type.
	// The image parameters used here are the same as for the swapchain images created earlier.
	VkImageViewCreateInfo imageViewInfo = {};
	imageViewInfo.flags = 0;
	imageViewInfo.pNext = nullptr;
	imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageViewInfo.image = appManager.texture.image;
	imageViewInfo.subresourceRange.layerCount = 1;
	imageViewInfo.subresourceRange.levelCount = 1;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;

	debugAssertFunctionResult(vk::CreateImageView(appManager.device, &imageViewInfo, nullptr, &appManager.texture.view), "Texture Image View Creation");

	// Create a texture sampler.
	// The sampler will be needed to sample the texture data and pass
	// it to the fragment shader during the execution of the rendering phase.
	// The parameters specified below define any filtering or transformations which are applied before
	// passing the colour data to the fragment shader.
	// In this case, anisotropic filtering is turned off and if the fragment shader samples outside of the image co-ordinates
	// it will return the colour at the nearest edge of the image.
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
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 5.0f;

	debugAssertFunctionResult(vk::CreateSampler(appManager.device, &samplerInfo, nullptr, &appManager.texture.sampler), "Texture Sampler Creation");

	// Clean up all the temporary data created for this operation.
	vk::DestroyFence(appManager.device, copyFence, nullptr);
	vk::FreeCommandBuffers(appManager.device, appManager.commandPool, 1, &commandBuffer);
	vk::FreeMemory(appManager.device, stagingBufferData.memory, nullptr);
	vk::DestroyBuffer(appManager.device, stagingBufferData.buffer, nullptr);
}

/// <summary>Creates a static and dynamic descriptor set</summary>
void VulkanHelloAPI::initDescriptorPoolAndSet()
{
	// Concept: Descriptors and Descriptor Sets
	// In Vulkan, to pass data to shaders, descriptor sets are required. Descriptors (as the name implies) are used to describe the data that is going to be passed. They hold
	// information that helps with binding data to shaders, and additionally describes any information Vulkan needs to know before executing the shader. Descriptors are not
	// passed individually (and are not visible to the application) but instead bundled in sets, known as descriptor sets.

	// The process of creating a descriptor set has three steps:

	// 1) Start by creating a descriptor pool that is used to allocate descriptor sets.
	// 2) Create a descriptor layout that defines how the descriptor set is laid out. This includes information on the binding points and the type of data passed to the shader. The
	// descriptor set layouts are used to create pipeline layouts. Pipeline layouts are essentially a list of all of the descriptor set layouts. They form a complete description of the set of
	// resources that can be accessed by the pipeline. They will be mentioned again when creating the graphics pipeline.
	// 3) Finally, the descriptor set is allocated from the previously created descriptor pool. The descriptor sets themselves hold the data, in the form of a pointer, that is passed to
	// the shader. This can include textures, uniform buffers, and so on.

	// These steps are demonstrated below.

	// This is the size of the descriptor pool. This establishes how many descriptors are needed and their type.
	VkDescriptorPoolSize descriptorPoolSize[2];

	descriptorPoolSize[0].descriptorCount = 1;
	descriptorPoolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

	descriptorPoolSize[1].descriptorCount = 1;
	descriptorPoolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	// This is the creation info struct for the descriptor pool.
	// This specifies the size of the pool
	// and the maximum number of descriptor sets that can be allocated out of it.
	// The VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT used here indicates that the descriptor
	// sets can return their allocated memory individually rather than all together.
	VkDescriptorPoolCreateInfo descriptorPoolInfo = {};
	descriptorPoolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	descriptorPoolInfo.pNext = nullptr;
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = 2;
	descriptorPoolInfo.pPoolSizes = descriptorPoolSize;
	descriptorPoolInfo.maxSets = 2;

	// Create the descriptor pool.
	debugAssertFunctionResult(vk::CreateDescriptorPool(appManager.device, &descriptorPoolInfo, nullptr, &appManager.descriptorPool), "Descriptor Pool Creation");
	{
		// Populate a descriptor layout binding struct. This defines the type of data that will be passed to the shader and the binding location in the shader stages.
		VkDescriptorSetLayoutBinding descriptorLayoutBinding;
		descriptorLayoutBinding.descriptorCount = 1;
		descriptorLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		descriptorLayoutBinding.binding = 0;
		descriptorLayoutBinding.pImmutableSamplers = nullptr;

		// Populate an info struct for the creation of the descriptor set layout. The number of bindings previously created is passed in here.
		VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
		descriptorLayoutInfo.flags = 0;
		descriptorLayoutInfo.pNext = nullptr;
		descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayoutInfo.bindingCount = 1;
		descriptorLayoutInfo.pBindings = &descriptorLayoutBinding;

		// Create the descriptor set layout for the descriptor set which provides access to the texture data.
		debugAssertFunctionResult(
			vk::CreateDescriptorSetLayout(appManager.device, &descriptorLayoutInfo, nullptr, &appManager.staticDescriptorSetLayout), "Descriptor Set Layout Creation");
	}

	// The process is then repeated for the descriptor set layout of the uniform buffer descriptor set.
	{
		VkDescriptorSetLayoutBinding descriptorLayoutBinding;
		descriptorLayoutBinding.descriptorCount = 1;
		descriptorLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		descriptorLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		descriptorLayoutBinding.binding = 0;
		descriptorLayoutBinding.pImmutableSamplers = nullptr;

		// Create the descriptor set layout using the array of VkDescriptorSetLayoutBindings.
		VkDescriptorSetLayoutCreateInfo descriptorLayoutInfo = {};
		descriptorLayoutInfo.flags = 0;
		descriptorLayoutInfo.pNext = nullptr;
		descriptorLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorLayoutInfo.bindingCount = 1;
		descriptorLayoutInfo.pBindings = &descriptorLayoutBinding;

		// Create the descriptor set layout for the uniform buffer descriptor set.
		debugAssertFunctionResult(
			vk::CreateDescriptorSetLayout(appManager.device, &descriptorLayoutInfo, nullptr, &appManager.dynamicDescriptorSetLayout), "Descriptor Set Layout Creation");
	}

	// Allocate the uniform buffer descriptor set from the descriptor pool.
	// This struct simply points to the layout of the uniform buffer descriptor set and also the descriptor pool created earlier.
	VkDescriptorSetAllocateInfo descriptorAllocateInfo = {};
	descriptorAllocateInfo.descriptorPool = appManager.descriptorPool;
	descriptorAllocateInfo.descriptorSetCount = 1;
	descriptorAllocateInfo.pNext = nullptr;
	descriptorAllocateInfo.pSetLayouts = &appManager.dynamicDescriptorSetLayout;
	descriptorAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;

	debugAssertFunctionResult(vk::AllocateDescriptorSets(appManager.device, &descriptorAllocateInfo, &appManager.dynamicDescSet), "Descriptor Set Creation");

	// Allocate the texture image descriptor set.
	// The allocation struct variable is updated to point to the layout of the texture image descriptor set.
	descriptorAllocateInfo.pSetLayouts = &appManager.staticDescriptorSetLayout;
	debugAssertFunctionResult(vk::AllocateDescriptorSets(appManager.device, &descriptorAllocateInfo, &appManager.staticDescSet), "Descriptor Set Creation");

	// This information references the texture sampler that will be passed to the shaders by way of
	// the descriptor set. The sampler determines how the pixel data of the texture image will be
	// sampled and how it will be passed to the fragment shader. It also contains the actual image
	// object (via its image view) and the image layout.
	// This image layout is optimised for read-only access by shaders. The image was transitioned to
	// this layout using a memory barrier in initTexture().
	VkDescriptorImageInfo descriptorImageInfo = {};
	descriptorImageInfo.sampler = appManager.texture.sampler;
	descriptorImageInfo.imageView = appManager.texture.view;
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	// Update the descriptor sets with the actual objects, in this case the texture image and the uniform buffer.
	// These structs specify which descriptor sets are going to be updated and hold a pointer to the actual objects.
	VkWriteDescriptorSet descriptorSetWrite[2] = {};

	descriptorSetWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorSetWrite[0].pNext = nullptr;
	descriptorSetWrite[0].dstSet = appManager.staticDescSet;
	descriptorSetWrite[0].descriptorCount = 1;
	descriptorSetWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorSetWrite[0].pImageInfo = &descriptorImageInfo; // Pass image object
	descriptorSetWrite[0].dstArrayElement = 0;
	descriptorSetWrite[0].dstBinding = 0;
	descriptorSetWrite[0].pBufferInfo = nullptr;
	descriptorSetWrite[0].pTexelBufferView = nullptr;

	descriptorSetWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorSetWrite[1].pNext = nullptr;
	descriptorSetWrite[1].dstSet = appManager.dynamicDescSet;
	descriptorSetWrite[1].descriptorCount = 1;
	descriptorSetWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	descriptorSetWrite[1].pBufferInfo = &appManager.dynamicUniformBufferData.bufferInfo; // Pass uniform buffer to this function.
	descriptorSetWrite[1].dstArrayElement = 0;
	descriptorSetWrite[1].dstBinding = 0;

	vk::UpdateDescriptorSets(appManager.device, 2, descriptorSetWrite, 0, nullptr);
}

/// <summary>Creates the graphics pipeline</summary>
void VulkanHelloAPI::initPipeline()
{
	// Concept: Pipelines
	// A pipeline is a collection of stages in the rendering or compute process. Each stage processes data and passes it on to the next stage.
	// In Vulkan, there are two types of pipelines: graphics and compute.
	// The graphics pipeline is used for rendering operations, and the compute pipeline allows the application to perform computational work such as physics calculations.
	// With Vulkan, the pipeline is stored in one object that is immutable; therefore each object that needs to be rendered will potentially use a different pipeline.
	// The pipeline in Vulkan needs to be prepared before the its use. This helps with increasing the performance of the application.

	// There are a lot of parameters to be populated in the graphics pipeline. Each of the structs below will configure a different aspect of the pipeline and will be referenced
	// by the final pipeline creation struct.

	// This is the description of the vertex buffers that will be bound, in this case it is just one.
	// The stride variable set here is the distance, in bytes, between consecutive vertices. The input rate
	// specifies at what rate vertex attributes are pulled from the vertex buffer. It can be set to: per instance or per vertex.
	VkVertexInputBindingDescription vertexInputBindingDescription = {};
	vertexInputBindingDescription.binding = 0;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	vertexInputBindingDescription.stride = sizeof(Vertex);

	// This is the description of the vertex attributes for the vertex input.
	// The location variable sets which vertex attribute to use. In this case there are two attributes: one for
	// position co-ordinates and one for the texture co-ordinates.
	// The offset variable specifies at what memory location within each vertex the attribute is found, and the format
	// parameter describes how the data is stored in each attribute.
	VkVertexInputAttributeDescription vertexInputAttributeDescription[2];
	vertexInputAttributeDescription[0].binding = 0;
	vertexInputAttributeDescription[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
	vertexInputAttributeDescription[0].location = 0;
	vertexInputAttributeDescription[0].offset = 0;

	vertexInputAttributeDescription[1].binding = 0;
	vertexInputAttributeDescription[1].format = VK_FORMAT_R32G32_SFLOAT;
	vertexInputAttributeDescription[1].location = 1;
	vertexInputAttributeDescription[1].offset = 4 * sizeof(float);

	// Combine the vertex bindings and the vertex attributes into the vertex input. This sums up all of the information about the vertices.
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &vertexInputBindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = sizeof(vertexInputAttributeDescription) / sizeof(vertexInputAttributeDescription[0]);
	vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributeDescription;

	// Declare and populate the input assembly info struct.
	// This describes how the pipeline should handle the incoming vertex data. In
	// this case the pipeline will form triangles from the incoming vertices.
	// Additionally, an index buffer is not being used so primitive restart is not required.
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = {};
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.flags = 0;
	inputAssemblyInfo.pNext = nullptr;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	// Define the rasterizer.
	// Here the rasterizer is set to fill the polygons with fragments, cull back faces, define the front face
	// by the clockwise winding direction and not use any depth bias.
	VkPipelineRasterizationStateCreateInfo rasterizationInfo = {};
	rasterizationInfo.pNext = nullptr;
	rasterizationInfo.flags = 0;
	rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizationInfo.lineWidth = 1.0f;
	rasterizationInfo.depthBiasClamp = 0.0f;
	rasterizationInfo.depthBiasConstantFactor = 0.0f;
	rasterizationInfo.depthBiasEnable = VK_FALSE;
	rasterizationInfo.depthBiasSlopeFactor = 0.0f;

	// This colour blend attachment state will be used by the colour blend info.
	// Only a single colour blend attachment is required because the render pass only has
	// one attachment.
	// No blending is needed so existing fragment values will be overwritten with incoming ones.
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = 0xf;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

	// Populate the colour blend info struct required by the pipeline.
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

	// Populate the multisampling info struct. Multisampling is not needed.
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

	// Create a list of dynamic states that will be used.
	// memset is used to initialise the block of memory pointed to by dynamicState to 0.
	VkDynamicState dynamicState[VK_DYNAMIC_STATE_RANGE_SIZE];
	memset(dynamicState, 0, sizeof(dynamicState));

	// Declare and populate the dynamic state info struct.
	VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.dynamicStateCount = 0;
	dynamicStateInfo.pNext = nullptr;
	dynamicStateInfo.pDynamicStates = dynamicState;

	// Populate a viewport state creation struct.
	VkPipelineViewportStateCreateInfo viewportInfo = {};
	viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportInfo.pNext = nullptr;
	viewportInfo.flags = 0;

	// Add the viewport and scissor as dynamic states in the dynamic state info struct.
	viewportInfo.viewportCount = 1;
	dynamicState[dynamicStateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	viewportInfo.pViewports = &appManager.viewport;

	viewportInfo.scissorCount = 1;
	dynamicState[dynamicStateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
	viewportInfo.pScissors = &appManager.scissor;

	// Create a list of the descriptor set layouts.
	// This were created earlier in initDescriptorPoolAndSet().
	VkDescriptorSetLayout descriptorSetLayout[] = { appManager.staticDescriptorSetLayout, appManager.dynamicDescriptorSetLayout };

	// Create the pipeline layout from the descriptor set layouts.
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 2; // The count of the descriptors is already known.
	pipelineLayoutInfo.pSetLayouts = descriptorSetLayout; // Add them to the pipeline layout info struct.
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	debugAssertFunctionResult(vk::CreatePipelineLayout(appManager.device, &pipelineLayoutInfo, nullptr, &appManager.pipelineLayout), "Pipeline Layout Creation");

	// Create the pipeline by putting all of these elements together.
	VkGraphicsPipelineCreateInfo pipelineInfo;
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.pNext = nullptr;
	pipelineInfo.layout = appManager.pipelineLayout;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = 0;
	pipelineInfo.flags = 0;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineInfo.pRasterizationState = &rasterizationInfo;
	pipelineInfo.pColorBlendState = &colorBlendInfo;
	pipelineInfo.pTessellationState = nullptr;
	pipelineInfo.pMultisampleState = &multisamplingInfo;
	pipelineInfo.pDynamicState = &dynamicStateInfo;
	pipelineInfo.pViewportState = &viewportInfo;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pStages = appManager.shaderStages;
	pipelineInfo.stageCount = 2;
	pipelineInfo.renderPass = appManager.renderPass;
	pipelineInfo.subpass = 0;

	debugAssertFunctionResult(vk::CreateGraphicsPipelines(appManager.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &appManager.pipeline), "Pipeline Creation");
}

/// <summary>Creates a number of framebuffer objects equal to the number of images in the swapchain</summary>
void VulkanHelloAPI::initFrameBuffers()
{
	// Concept: Framebuffers
	// In Vulkan, all the attachments used by the render pass are defined in framebuffers. Each frame in a framebuffer defines
	// the attachments related to it. This includes the textures (including the colour and depth / stencil attachments) and
	// the input attachments. This way of separating descriptions in render passes and definitions in framebuffers gives the option
	// of using different render passes with different framebuffers. However, the degree of flexibility with which this can be done is based on the
	// compatibility of the two.

	// This function creates a framebuffer for each swapchain image.

	// This is a placeholder handle for the attachment which will be stored in the VkFramebufferCreateInfo.
	VkImageView attachment = VK_NULL_HANDLE;

	// Populate a framebuffer info struct with the information that is needed to create the framebuffers. This includes its dimensions, its attachments, and the associated render
	// pass that will use the specified attachments. The attachment member will be a null variable for now.
	VkFramebufferCreateInfo frameBufferInfo = {};
	frameBufferInfo.flags = 0;
	frameBufferInfo.pNext = nullptr;
	frameBufferInfo.attachmentCount = 1;
	frameBufferInfo.height = appManager.swapchainExtent.height;
	frameBufferInfo.width = appManager.swapchainExtent.width;
	frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferInfo.renderPass = appManager.renderPass;
	frameBufferInfo.pAttachments = &attachment;
	frameBufferInfo.layers = 1;

	// Resize the vector which will contain all of the framebuffers based on the number of images in the swap chain.
	appManager.frameBuffers.resize(appManager.swapChainImages.size());

	// Create as many framebuffer objects as swapchain images and assign each image to a framebuffer.
	// Note that, above, the pAttachments variable has been assigned to the address of the
	// local variable "attachment". Every time pAttachments is reassigned a new image is used during framebuffer creation.
	for (size_t i = 0; i < appManager.swapChainImages.size(); ++i)
	{
		attachment = appManager.swapChainImages[i].view;

		debugAssertFunctionResult(vk::CreateFramebuffer(appManager.device, &frameBufferInfo, nullptr, &appManager.frameBuffers[i]), "Swapchain Frame buffer creation");
	}
}

/// <summary>Creates a command pool and then allocates out of it a number of command buffers equal to the number of swapchain images</summary>
void VulkanHelloAPI::initCommandPoolAndBuffer()
{
	// This function creates a command pool to reserve memory for the command buffers are created to execute commands.
	// After the command pool is created, command buffers are allocated from it. A number of command buffers equal to
	// the number of images in the swapchain are needed, assuming the command buffers are used for rendering.

	// Populate a command pool info struct with the queue family that will be used and the intended usage behaviour of command buffers
	// that can be allocated out of it.
	VkCommandPoolCreateInfo commandPoolInfo = {};
	commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolInfo.pNext = nullptr;
	commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolInfo.queueFamilyIndex = appManager.graphicsQueueFamilyIndex;

	// Create the actual command pool.
	debugAssertFunctionResult(vk::CreateCommandPool(appManager.device, &commandPoolInfo, nullptr, &appManager.commandPool), "Command Pool Creation");

	// Resize the vector to have a number of elements equal to the number of swapchain images.
	appManager.cmdBuffers.resize(appManager.swapChainImages.size());

	// Populate a command buffer info struct with a reference to the command pool from which the memory for the command buffer is taken.
	// Notice the "level" parameter which ensures these will be primary command buffers.
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.pNext = nullptr;
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = appManager.commandPool;
	commandBufferAllocateInfo.commandBufferCount = static_cast<uint32_t>(appManager.cmdBuffers.size());
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	// Allocate the command buffers from the command pool.
	debugAssertFunctionResult(vk::AllocateCommandBuffers(appManager.device, &commandBufferAllocateInfo, appManager.cmdBuffers.data()), "Command Buffer Creation");
}

/// <summary>Sets up the view port and also sets up the scissor</summary>
void VulkanHelloAPI::initViewportAndScissor()
{
	// The viewport is essentially the dimensions of the rendering area and
	// the scissor is a sub-section of this viewport which is actually stored.

	// Viewport and scissors are set dynamically with vkCmdSetViewport and vkCmdSetScissor.

	// This code sets up the values that will be used by these commands. In this example,
	// the extents of the scissor are the same as the viewport.

	// Set the viewport dimensions, depth, and starting coordinates.
	appManager.viewport.width = surfaceData.width;
	appManager.viewport.height = surfaceData.height;
	appManager.viewport.minDepth = 0.0f;
	appManager.viewport.maxDepth = 1.0f;
	appManager.viewport.x = 0;
	appManager.viewport.y = 0;

	// Set the extent to the dimensions of the surface and set the offset in both directions to 0.
	appManager.scissor.extent.width = static_cast<uint32_t>(surfaceData.width);
	appManager.scissor.extent.height = static_cast<uint32_t>(surfaceData.height);
	appManager.scissor.offset.x = 0;
	appManager.scissor.offset.y = 0;

	// The viewport and scissor are now ready to be set.
}

/// <summary>Creates a number of fences and semaphores which synchronise work on the CPU and GPU</summary>
void VulkanHelloAPI::initSemaphoreAndFence()
{
	// Concept: Fences and Semaphores
	// Fences and semaphores are used to synchronise work on the CPU and GPU that share the same resources.
	// Fences are GPU to CPU syncs. They are signalled by the GPU, and can only be waited on by the CPU. They need to be reset manually.
	// Semaphores are GPU to GPU syncs, specifically used to sync queue submissions on the same or different queue. Again, they are signalled by
	// the GPU but are waited on by the GPU. They are reset after they are waited on.

	// This function creates two sets of semaphores and a single fence for each swapchain image.

	// The first semaphore will wait until the image has been acquired successfully from the
	// swapchain before signalling, the second semaphore will wait until the render has finished
	// on the image, and finally the fence will wait until the commands in the command
	// buffer have finished executing.

	// The semaphores are created with default parameters, but the fence is created with the flags parameter set to
	// VK_FENCE_CREATE_SIGNALED_BIT. This is because of the specific way this example is structured. The
	// application waits for this fence to be signalled before starting to draw the frame, however, on the first
	// frame there is no previous frame to trigger the fence, so it must be created in a signalled state.

	// All of the objects created here are stored in std::vectors. The individual semaphores and fences
	// will be accessed later with an index relating to the frame that is currently being rendered.
	for (uint32_t i = 0; i < appManager.swapChainImages.size(); ++i)
	{
		VkSemaphore acquireSemaphore;
		VkSemaphore renderSemaphore;

		VkFence frameFence;

		VkSemaphoreCreateInfo acquireSemaphoreInfo = {};
		acquireSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		acquireSemaphoreInfo.pNext = nullptr;
		acquireSemaphoreInfo.flags = 0;

		debugAssertFunctionResult(vk::CreateSemaphore(appManager.device, &acquireSemaphoreInfo, nullptr, &acquireSemaphore), "Acquire Semaphore creation");

		appManager.acquireSemaphore.emplace_back(acquireSemaphore);

		VkSemaphoreCreateInfo renderSemaphoreInfo = {};
		renderSemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		renderSemaphoreInfo.pNext = nullptr;
		renderSemaphoreInfo.flags = 0;

		debugAssertFunctionResult(vk::CreateSemaphore(appManager.device, &renderSemaphoreInfo, nullptr, &renderSemaphore), "Render Semaphore creation");

		appManager.presentSemaphores.emplace_back(renderSemaphore);

		VkFenceCreateInfo FenceInfo;
		FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		FenceInfo.pNext = nullptr;
		FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start the fence as signalled.

		debugAssertFunctionResult(vk::CreateFence(appManager.device, &FenceInfo, nullptr, &frameFence), "Fence Creation");

		appManager.frameFences.emplace_back(frameFence);
	}
}

/// <summary>Creates a buffer, allocates it memory, maps the memory and copies the data into the buffer</summary>
/// <param name="inBuffer">Vkbuffer handle in which the newly-created buffer object is returned</param>
/// <param name="inData">Data to be copied into the buffer</param>
/// <param name="inUsage">Usage flag which determines what type of buffer will be created</param>
void VulkanHelloAPI::createBuffer(BufferData& inBuffer, const uint8_t* inData, const VkBufferUsageFlags& inUsage)
{
	// This is a generic function which is used to create buffers.
	// It is responsible for creating a buffer object, allocating the memory, mapping this memory, and
	// copying the data into the buffer. The usage flag that determines the type of buffer that is going to be used
	// is passed when this function is called.

	// Declare and populate a buffer creation info struct.
	// This tells the API the size of the buffer and how it is going to be used. Additionally, it specifies whether the
	// buffer is going to be accessed by multiple queue families at the same time and if so, what those queue families are.
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.flags = 0;
	bufferInfo.pNext = nullptr;
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = inBuffer.size;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	bufferInfo.usage = inUsage;
	bufferInfo.pQueueFamilyIndices = nullptr;
	bufferInfo.queueFamilyIndexCount = 0;

	// Create the buffer object itself.
	debugAssertFunctionResult(vk::CreateBuffer(appManager.device, &bufferInfo, nullptr, &inBuffer.buffer), "Buffer Creation");

	// Define a struct to hold the memory requirements for the buffer.
	VkMemoryRequirements memoryRequirments;

	// Extract the memory requirements for the buffer.
	vk::GetBufferMemoryRequirements(appManager.device, inBuffer.buffer, &memoryRequirments);

	// Populate an allocation info struct with the memory requirement size.
	VkMemoryAllocateInfo allocateInfo = {};
	allocateInfo.pNext = nullptr;
	allocateInfo.memoryTypeIndex = 0;
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.allocationSize = memoryRequirments.size;

	// Check if the memory that is going to be used supports the necessary flags for the usage of the buffer.
	// In this case it needs to be "Host Coherent" in order to be able to map it. If it is not, find a compatible one.
	bool pass = getMemoryTypeFromProperties(appManager.deviceMemoryProperties, memoryRequirments.memoryTypeBits,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &(allocateInfo.memoryTypeIndex));
	if (pass)
	{
		// This pointer will be used to pass the data into the buffer.
		uint8_t* pData;

		// Allocate the memory necessary for the data.
		debugAssertFunctionResult(vk::AllocateMemory(appManager.device, &allocateInfo, nullptr, &(inBuffer.memory)), "Allocate Buffer Memory");

		// Save the data in the buffer struct.
		inBuffer.bufferInfo.range = memoryRequirments.size;
		inBuffer.bufferInfo.offset = 0;
		inBuffer.bufferInfo.buffer = inBuffer.buffer;

		VkMemoryPropertyFlags flags = appManager.deviceMemoryProperties.memoryTypes[allocateInfo.memoryTypeIndex].propertyFlags;
		inBuffer.memPropFlags = flags;

		if (inData != nullptr)
		{
			// Map data to the memory.
			// inBuffer.memory is the device memory handle.
			// memoryRequirments.size is the size of the memory to be mapped, in this case it is the entire buffer.
			// &pData is an output variable and will contain a pointer to the mapped data.
			debugAssertFunctionResult(vk::MapMemory(appManager.device, inBuffer.memory, 0, inBuffer.size, 0, reinterpret_cast<void**>(&pData)), "Map Buffer Memory");

			// Copy the data into the pointer mapped to the memory.
			memcpy(pData, inData, inBuffer.size);

			VkMappedMemoryRange mapMemRange = {
				VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
				nullptr,
				inBuffer.memory,
				0,
				inBuffer.size,
			};

			// ONLY flush the memory if it does not support VK_MEMORY_PROPERTY_HOST_COHERENT_BIT.
			if (!(flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) { vk::FlushMappedMemoryRanges(appManager.device, 1, &mapMemRange); }
		}

		// Associate the allocated memory with the previously created buffer.
		// Mapping and binding do not need to occur in a particular order. This step could just as well be performed before mapping
		// and populating.
		debugAssertFunctionResult(vk::BindBufferMemory(appManager.device, inBuffer.buffer, inBuffer.memory, 0), "Bind Buffer Memory");
	}
}

/// <summary>Creates a dynamic uniform buffer and allocates its memory</summary>
/// <param name="inBuffer">Vkbuffer handle in which the newly-created buffer object is returned</param>
void VulkanHelloAPI::createDynamicUniformBuffer(BufferData& inBuffer)
{
	// This function is used to create a dynamic uniform buffer.

	// Concept: Dynamic Uniform Buffers
	// Dynamic uniform buffers are buffers that contain the data for multiple single uniform buffers (usually each associated with a frame) and use offsets to access this data.
	// This minimises the amount of descriptor sets required, and may help optimise write operations.

	// Query the physical device properties, such as the API version of the device, and the device name.
	VkPhysicalDeviceProperties deviceProperties;
	vk::GetPhysicalDeviceProperties(appManager.physicalDevice, &deviceProperties);

	// Check the limit of the dynamic buffers the physical device supports by consulting the device properties.
	if (deviceProperties.limits.maxDescriptorSetUniformBuffersDynamic > 1)
	{
		// Get the alignment of the uniform buffer.
		size_t uboAlignment = (size_t)deviceProperties.limits.minUniformBufferOffsetAlignment;

		// Calculate the size of each offset so that it aligns correctly with the device property alignment.
		appManager.offset = static_cast<uint32_t>(((sizeof(float) * 4) / uboAlignment) * uboAlignment + (((sizeof(float) * 4) % uboAlignment) > 0 ? uboAlignment : 0));

		// Calculate the full size of the buffer.
		inBuffer.size = appManager.swapChainImages.size() * appManager.offset;

		// Create a Buffer Creation info. This will tell the API what the purpose of the buffer is and how to use it.
		VkBufferCreateInfo bufferInfo = {};
		bufferInfo.flags = 0;
		bufferInfo.pNext = nullptr;
		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = inBuffer.size;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		bufferInfo.pQueueFamilyIndices = nullptr;
		bufferInfo.queueFamilyIndexCount = 0;

		// Create a buffer.
		debugAssertFunctionResult(vk::CreateBuffer(appManager.device, &bufferInfo, nullptr, &inBuffer.buffer), "Buffer Creation");

		// The memory requirements for the buffer.
		VkMemoryRequirements memoryRequirments;

		// Extract the memory requirements for the buffer.
		vk::GetBufferMemoryRequirements(appManager.device, inBuffer.buffer, &memoryRequirments);

		// Create an allocation info struct which defines the parameters of memory allocation.
		// Pass the memory requirements size.
		VkMemoryAllocateInfo allocateInfo = {};
		allocateInfo.pNext = nullptr;
		allocateInfo.memoryTypeIndex = 0;
		allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocateInfo.allocationSize = memoryRequirments.size;

		// Check the memory that is going to used is compatible with the operation of this application.
		// If it is not, find the compatible one.
		bool pass = getMemoryTypeFromProperties(appManager.deviceMemoryProperties, memoryRequirments.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &(allocateInfo.memoryTypeIndex));

		if (pass)
		{
			// Allocate the memory for the buffer.
			debugAssertFunctionResult(vk::AllocateMemory(appManager.device, &allocateInfo, nullptr, &(inBuffer.memory)), "Dynamic Buffer Memory Allocation");
			inBuffer.memPropFlags = appManager.deviceMemoryProperties.memoryTypes[allocateInfo.memoryTypeIndex].propertyFlags;

			// Save the data to the buffer struct.
			inBuffer.bufferInfo.range = memoryRequirments.size / appManager.swapChainImages.size();
			inBuffer.bufferInfo.offset = 0;
			inBuffer.bufferInfo.buffer = inBuffer.buffer;
		}
	}
}

/// <summary>Creates a shader module using pre-compiled SPIR-V shader source code</summary>
/// <param name="spvShader">Shader source code</param>
/// <param name="spvShaderSize">Size of the shader source code in bytes</param>
/// <param name="indx">Specifies which shader stage to define in appManager's shaderStages array</param>
/// <param name="shaderStage">Specifies the stage in the pipeline where the shader will exist</param>
void VulkanHelloAPI::createShaderModule(const uint32_t* spvShader, size_t spvShaderSize, int indx, VkShaderStageFlagBits shaderStage)
{
	// This function will create a shader module and update the shader stage array. The shader module will hold
	// the data from the pre-compiled SPIR-V shader. A shader stage will also be associated with this shader module. This identifies in which stage of the pipeline this shader
	// will be used.

	// Populate a shader module creation info struct with a pointer to the shader source code and the size of the shader in bytes.
	VkShaderModuleCreateInfo shaderModuleInfo = {};
	shaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleInfo.flags = 0;
	shaderModuleInfo.pCode = spvShader;
	shaderModuleInfo.codeSize = spvShaderSize;
	shaderModuleInfo.pNext = nullptr;

	// Set the stage of the pipeline that the shader module will be associated with.
	// The shader source code entry point ("main") is also set here.
	appManager.shaderStages[indx].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	appManager.shaderStages[indx].flags = 0;
	appManager.shaderStages[indx].pName = "main";
	appManager.shaderStages[indx].pNext = nullptr;
	appManager.shaderStages[indx].stage = shaderStage;
	appManager.shaderStages[indx].pSpecializationInfo = nullptr;

	// Create a shader module and add it to the shader stage corresponding to the VkShaderStageFlagBits stage.
	debugAssertFunctionResult(vk::CreateShaderModule(appManager.device, &shaderModuleInfo, nullptr, &(appManager.shaderStages[indx].module)), "Shader Module Creation");
}

/// <summary>Records rendering commands to the command buffers</summary>
void VulkanHelloAPI::recordCommandBuffer()
{
	// Concept: Command Buffers
	// Command buffers are containers that contain GPU commands. They are passed to the queues to be executed on the device.
	// Each command buffer when executed performs a different task. For instance, the command buffer required to render an object is
	// recorded before the rendering. When the rendering stage of the application is reached, the command buffer is submitted to execute its tasks.

	// This function will record a set of commands in the command buffers which will render a basic triangle on screen.

	// State the clear values for rendering.
	// This is the colour value that the framebuffer is cleared to at the start of the render pass.
	// The framebuffer is cleared because, during render pass creation, the loadOp parameter was set to VK_LOAD_OP_CLEAR. Remember
	// that this is crucial as it can reduce system memory bandwidth and reduce power consumption, particularly on PowerVR platforms.
	VkClearValue clearColor = { 0.00f, 0.70f, 0.67f, 1.0f };

	// This is a constant offset which specifies where the vertex data starts in the vertex
	// buffer. In this case the data just starts at the beginning of the buffer.
	const VkDeviceSize vertexOffsets[1] = { 0 };

	// Iterate through each created command buffer to record to it.
	for (size_t i = 0; i < appManager.cmdBuffers.size(); ++i)
	{
		// Reset the buffer to its initial state.
		debugAssertFunctionResult(vk::ResetCommandBuffer(appManager.cmdBuffers[i], 0), "Command Buffer Reset");

		// Begin the command buffer.
		VkCommandBufferBeginInfo cmd_begin_info = {};
		cmd_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmd_begin_info.pNext = nullptr;
		cmd_begin_info.flags = 0;
		cmd_begin_info.pInheritanceInfo = nullptr;

		debugAssertFunctionResult(vk::BeginCommandBuffer(appManager.cmdBuffers[i], &cmd_begin_info), "Command Buffer Recording Started.");

		// Start recording commands.
		// In Vulkan, commands are recorded by calling vkCmd... functions.
		// Set the viewport and scissor to previously defined values.
		vk::CmdSetViewport(appManager.cmdBuffers[i], 0, 1, &appManager.viewport);

		vk::CmdSetScissor(appManager.cmdBuffers[i], 0, 1, &appManager.scissor);

		// Begin the render pass.
		// The render pass and framebuffer instances are passed here, along with the clear colour value and the extents of
		// the rendering area. VK_SUBPASS_CONTENTS_INLINE means that the subpass commands will be recorded here. The alternative is to
		// record them in isolation in a secondary command buffer and then record them here with vkCmdExecuteCommands.
		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.pNext = nullptr;
		renderPassInfo.renderPass = appManager.renderPass;
		renderPassInfo.framebuffer = appManager.frameBuffers[i];
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;
		renderPassInfo.renderArea.extent = appManager.swapchainExtent;
		renderPassInfo.renderArea.offset.x = 0;
		renderPassInfo.renderArea.offset.y = 0;

		vk::CmdBeginRenderPass(appManager.cmdBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Bind the pipeline to the command buffer.
		vk::CmdBindPipeline(appManager.cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, appManager.pipeline);

		// A single large uniform buffer object is being used to hold all of the transformation matrices
		// associated with the swapchain images. It is for this reason that only a single descriptor set is
		// required for all of the frames.
		const VkDescriptorSet descriptorSet[] = { appManager.staticDescSet, appManager.dynamicDescSet };

		// An offset is used to select each slice of the uniform buffer object that contains the transformation
		// matrix related to each swapchain image.
		// Calculate the offset into the uniform buffer object for the current slice.
		uint32_t offset = static_cast<uint32_t>(appManager.dynamicUniformBufferData.bufferInfo.range * i);

		// Bind the descriptor sets. The &offset parameter is the offset into the dynamic uniform buffer which is
		// contained within the dynamic descriptor set.
		vk::CmdBindDescriptorSets(appManager.cmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, appManager.pipelineLayout, 0, NUM_DESCRIPTOR_SETS, descriptorSet, 1, &offset);

		// Bind the vertex buffer.
		vk::CmdBindVertexBuffers(appManager.cmdBuffers[i], 0, 1, &appManager.vertexBuffer.buffer, vertexOffsets);

		// Draw three vertices.
		vk::CmdDraw(appManager.cmdBuffers[i], 3, 1, 0, 0);

		// End the render pass.
		vk::CmdEndRenderPass(appManager.cmdBuffers[i]);

		// End the command buffer recording process.
		debugAssertFunctionResult(vk::EndCommandBuffer(appManager.cmdBuffers[i]), "Command Buffer Recording Ended.");

		// At this point the command buffer is ready to be submitted to a queue with all of the recorded operations executed
		// asynchronously after that. A command buffer can, and if possible should, be executed multiple times, unless
		// it is allocated with the VK_COMMAND_BUFFER_ONE_TIME_USE bit.
		// The command buffers recorded here will be reused across the lifetime of the application.
	}
}

/// <summary>Executes the recorded command buffers. The recorded operations will end up rendering and presenting the frame to the surface</summary>
void VulkanHelloAPI::drawFrame()
{
	// This is where the recorded command buffers are executed. The recorded operations will end up rendering
	// and presenting the frame to the surface.

	// Wait for the fence to be signalled before starting to render the current frame, then reset it so it can be reused.
	debugAssertFunctionResult(vk::WaitForFences(appManager.device, 1, &appManager.frameFences[frameId], true, FENCE_TIMEOUT), "Fence - Signalled");

	vk::ResetFences(appManager.device, 1, &appManager.frameFences[frameId]);

	// currentBuffer will be used to point to the correct frame/command buffer/uniform buffer data.
	// It is going to be the general index of the data being worked on.
	uint32_t currentBuffer = 0;
	VkPipelineStageFlags pipe_stage_flags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	// Acquire and get the index of the next available swapchain image.
	debugAssertFunctionResult(
		vk::AcquireNextImageKHR(appManager.device, appManager.swapchain, std::numeric_limits<uint64_t>::max(), appManager.acquireSemaphore[frameId], VK_NULL_HANDLE, &currentBuffer),
		"Draw - Acquire Image");

	// Use a helper function with the current frame index to calculate the transformation matrix and write it into the correct
	// slice of the uniform buffer.
	applyRotation(currentBuffer);

	// Submit the command buffer to the queue to start rendering.
	// The command buffer is submitted to the graphics queue which was created earlier.
	// Notice the wait (acquire) and signal (present) semaphores, and the fence.
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.pWaitDstStageMask = &pipe_stage_flags;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &appManager.acquireSemaphore[frameId];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &appManager.presentSemaphores[frameId];
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &appManager.cmdBuffers[currentBuffer];

	debugAssertFunctionResult(vk::QueueSubmit(appManager.graphicQueue, 1, &submitInfo, appManager.frameFences[frameId]), "Draw - Submit to Graphic Queue");

	// Queue the rendered image for presentation to the surface.
	// The currentBuffer is again used to select the correct swapchain images to present. A wait
	// semaphore is also set here which will be signalled when the command buffer has
	// finished execution.
	VkPresentInfoKHR presentInfo;
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &appManager.swapchain;
	presentInfo.pImageIndices = &currentBuffer;
	presentInfo.pWaitSemaphores = &appManager.presentSemaphores[frameId];
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pResults = nullptr;

	debugAssertFunctionResult(vk::QueuePresentKHR(appManager.presentQueue, &presentInfo), "Draw - Submit to Present Queue");

	// Update the frameId to get the next suitable one.
	frameId = (frameId + 1) % appManager.swapChainImages.size();
}

/// <summary>Finds the indices of compatible graphics and present queues and returns them</summary>
/// <param name="graphicsfamilyindex">Compatible graphics queue index that is returned</param>
/// <param name="presentfamilyindex">Compatible present queue index that is returned</param>
void VulkanHelloAPI::getCompatibleQueueFamilies(uint32_t& graphicsfamilyindex, uint32_t& presentfamilyindex)
{
	// This function iterates through all the queue families available on the selected device and selects a graphics queue
	// family and a present queue family by selecting their associated indices. It also checks that the present queue family
	// supports presenting.

	int i = 0;
	VkBool32 compatible = VK_FALSE;

	// Check if the family has queues, and that they are graphical and not compute queues.
	for (const auto& queuefamily : appManager.queueFamilyProperties)
	{
		// Look for a graphics queue.
		if (queuefamily.queueCount > 0 && queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			graphicsfamilyindex = i;
			break;
		}
		i++;
	}

	i = 0;

	// Check if the family has queues, and that they are graphical and not compute queues.
	for (const auto& queuefamily : appManager.queueFamilyProperties)
	{
		if (queuefamily.queueCount > 0 && queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			// Check if the queue family supports presenting.
			debugAssertFunctionResult(vk::GetPhysicalDeviceSurfaceSupportKHR(appManager.physicalDevice, i, appManager.surface, &compatible), "Querying Physical Device Surface Support");

			if (compatible)
			{
				presentfamilyindex = i;
				break;
			}
		}
		i++;
	}
}

/// <summary>Finds a physical device which is compatible with the applications's requirements</summary>
/// <returns>Handle to a compatible physical device</returns>
VkPhysicalDevice VulkanHelloAPI::getCompatibleDevice()
{
	// Iterate through the available physical devices to determine which one is compatible with the application's requirements.

	for (const auto& device : appManager.gpus)
	{
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		vk::GetPhysicalDeviceProperties(device, &deviceProperties);
		vk::GetPhysicalDeviceFeatures(device, &deviceFeatures);

		// Return the first device which is either a discrete GPU or an integrated GPU.
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		{
			Log(false, "Active Device is -- %s", deviceProperties.deviceName);
			return device;
		}
	}

	// If there is only one device, then return that one.
	if (appManager.gpus.size() == 1) { return appManager.gpus[0]; }

	// Return null if nothing is found.
	return nullptr;
}

/// <summary>Checks if the extents are correct based on the capabilities of the surface</summary>
/// <param name="inSurfCap">Capabilities of the current surface</param>
/// <returns>A valid correct extent</returns>
VkExtent2D VulkanHelloAPI::getCorrectExtent(const VkSurfaceCapabilitiesKHR& inSurfCap)
{
	// This function makes sure the extents are correct for the surface, based on the surface
	// capabilities. It also checks whether the extents are valid and the same as the one picked in
	// initSurface().

	// The width and height of the swapchain are either both 0xFFFFFFFF (max value for uint_32t) or they are both NOT 0xFFFFFFFF.
	if (inSurfCap.currentExtent.width == std::numeric_limits<uint32_t>::max() || inSurfCap.currentExtent.height == std::numeric_limits<uint32_t>::max())
	{
		// Pass the width and height from the surface.
		appManager.swapchainExtent.width = static_cast<uint32_t>(surfaceData.width);
		appManager.swapchainExtent.height = static_cast<uint32_t>(surfaceData.height);
		VkExtent2D currentExtent = appManager.swapchainExtent;

		// The swapchain extent width and height cannot be less than the minimum surface capability or greater than
		// the maximum surface capability.
		if (appManager.swapchainExtent.width < inSurfCap.minImageExtent.width) { currentExtent.width = inSurfCap.minImageExtent.width; }
		else if (appManager.swapchainExtent.width > inSurfCap.maxImageExtent.width)
		{
			currentExtent.width = inSurfCap.maxImageExtent.width;
		}

		if (appManager.swapchainExtent.height < inSurfCap.minImageExtent.height) { currentExtent.height = inSurfCap.minImageExtent.height; }
		else if (appManager.swapchainExtent.height > inSurfCap.maxImageExtent.height)
		{
			currentExtent.height = inSurfCap.maxImageExtent.height;
		}

		// If the extents are zero, use the values picked from the surface data.
		if (currentExtent.width == 0 && currentExtent.height == 0)
		{
			currentExtent.width = static_cast<uint32_t>(surfaceData.width);
			currentExtent.height = static_cast<uint32_t>(surfaceData.height);
		}

		return currentExtent;
	}

	// In the case where the width and height are both not 0xFFFFFFFF, make sure the extents are not zero.
	// As before, if they are zero then use values picked from the surface data.
	if (inSurfCap.currentExtent.width == 0 && inSurfCap.currentExtent.height == 0)
	{
		VkExtent2D currentExtent;
		currentExtent.width = static_cast<uint32_t>(surfaceData.width);
		currentExtent.height = static_cast<uint32_t>(surfaceData.height);
		return currentExtent;
	}

	return inSurfCap.currentExtent;
}

/// <summary>Generates a checkered texture on-the-fly</summary>
void VulkanHelloAPI::generateTexture()
{
	// This function will generate a checkered texture on the fly to be used on the triangle that is going
	// to be rendered and rotated on screen.
	for (uint16_t x = 0; x < appManager.texture.textureDimensions.width; ++x)
	{
		for (uint16_t y = 0; y < appManager.texture.textureDimensions.height; ++y)
		{
			float g = 0.3f;
			if (x % 128 < 64 && y % 128 < 64) { g = 1; }
			if (x % 128 >= 64 && y % 128 >= 64) { g = 1; }

			uint8_t* pixel = (static_cast<uint8_t*>(appManager.texture.data.data())) + (x * appManager.texture.textureDimensions.height * 4) + (y * 4);
			pixel[0] = static_cast<uint8_t>(100 * g);
			pixel[1] = static_cast<uint8_t>(80 * g);
			pixel[2] = static_cast<uint8_t>(70 * g);
			pixel[3] = 255;
		}
	}
}

/// <summary>Calculate a rotation matrix which provides a rotation around the z axis using the given angle</summary>
/// <param name="angle">The angle through which the rotation will be applied</param>
/// <param name="outRotationMatrix">The output rotation matrix. This matrix must have dimensions 4x4</param>
void rotateAroundZ(float angle, std::array<std::array<float, 4>, 4>& outRotationMatrix)
{
	const float c = cos(angle);
	const float s = sin(angle);

	// Rotation around z axis (0, 0, 1)
	outRotationMatrix[0][0] = c;
	outRotationMatrix[0][1] = s;
	outRotationMatrix[0][2] = 0;
	outRotationMatrix[0][3] = 0;

	outRotationMatrix[1][0] = -s;
	outRotationMatrix[1][1] = c;
	outRotationMatrix[1][2] = 0;
	outRotationMatrix[1][3] = 0;

	outRotationMatrix[2][0] = 0;
	outRotationMatrix[2][1] = 0;
	outRotationMatrix[2][2] = 1;
	outRotationMatrix[2][3] = 0;

	outRotationMatrix[3][0] = 0;
	outRotationMatrix[3][1] = 0;
	outRotationMatrix[3][2] = 0;
	outRotationMatrix[3][3] = 1;
}

/// <summary>Multiple two matrices together.</summary>
/// <param name="first">The first matrix</param>
/// <param name="second">The second matrix</param>
/// <param name="outMatrix">The output matrix</param>
void multiplyMatrices(std::array<std::array<float, 4>, 4>& first, std::array<std::array<float, 4>, 4>& second, std::array<std::array<float, 4>, 4>& outMatrix)
{
	for (uint32_t i = 0; i < 4; i++)
	{
		for (uint32_t j = 0; j < 4; j++)
		{
			for (uint32_t k = 0; k < 4; k++) { outMatrix[i][j] += first[i][k] * second[k][j]; }
		}
	}
}

/// <summary>Updates the dynamic uniform buffer with the new rotation value</summary>
/// <param name="idx">Index which selects the correct area of the buffer</param>
void VulkanHelloAPI::applyRotation(int idx)
{
	// This is called on every frame to update the dynamic uniform buffer with the new rotation
	// value.

	// An offset is used to point to the correct slice of the buffer that corresponds to the current
	// frame. The current frame is specified by the parameter, idx.
	// This memory is mapped persistently so it does not need to be mapped again on every frame. The pointer to this
	// consistently mapped memory is the variable appManager.dynamicUniformBufferData.mappedData.

	// Calculate the offset.
	VkDeviceSize offset = (appManager.offset * idx);

	// Update the angle of rotation and calculate the transformation matrix using the fixed projection
	// matrix and a freshly-calculated rotation matrix.
	appManager.angle += 0.02f;

	auto rotation = std::array<std::array<float, 4>, 4>();
	rotateAroundZ(appManager.angle, rotation);

	auto mvp = std::array<std::array<float, 4>, 4>();
	multiplyMatrices(rotation, viewProj, mvp);

	// Copy the matrix to the mapped memory using the offset calculated above.
	memcpy(static_cast<unsigned char*>(appManager.dynamicUniformBufferData.mappedData) + appManager.dynamicUniformBufferData.bufferInfo.range * idx, &mvp, sizeof(mvp));

	VkMappedMemoryRange mapMemRange = {
		VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
		nullptr,
		appManager.dynamicUniformBufferData.memory,
		offset,
		appManager.dynamicUniformBufferData.bufferInfo.range,
	};

	// ONLY flush the memory if it does not support VK_MEMORY_PROPERTY_HOST_COHERENT_BIT.
	if ((appManager.dynamicUniformBufferData.memPropFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0) { vk::FlushMappedMemoryRanges(appManager.device, 1, &mapMemRange); }
}

/// <summary>Initialises all Vulkan objects</summary>
void VulkanHelloAPI::initialize()
{
	// All the Vulkan objects are initialised here.
	// The vk::initVulkan() function is used to load the Vulkan library and definitions.

	// frameId is the index that will be used for synchronisation. It is going to be used mostly by
	// fences and semaphores to keep track of which one is currently free to work on.
	frameId = 0;

	// appManager holds all the object handles which need to be accessed "globally" such as the angle
	// of the rotation of the triangle that is going to be rendered on screen.
	appManager.angle = 45.0f;

	// Initialise all the pointers to Vulkan functions.
	vk::initVulkan();

	// Initialise all the Vulkan objects required to begin rendering.
	std::vector<std::string> layers = initLayers();
	std::vector<std::string> instanceExtensions = initInstanceExtensions();

	initApplicationAndInstance(instanceExtensions, layers);
	initPhysicalDevice();

	initSurface();

	initQueuesFamilies();

	std::vector<std::string> deviceExtensions = initDeviceExtensions();

	initLogicalDevice(deviceExtensions);
	initQueues();
	initSwapChain();
	initImagesAndViews();
	initCommandPoolAndBuffer();

	initShaders();
	initVertexBuffers();
	initUniformBuffers();
	initRenderPass();
	initTexture();
	initDescriptorPoolAndSet();

	initFrameBuffers();
	initPipeline();

	initViewportAndScissor();
	initSemaphoreAndFence();

	float aspect = 0.0f;
	// The screen is rotated.
	if (surfaceData.width < surfaceData.height) { aspect = surfaceData.height / surfaceData.width; }
	else
	{
		aspect = surfaceData.width / surfaceData.height;
	}

	float left = aspect;
	float right = -aspect;
	float bottom = 1.0;
	float top = -1.0f;

	viewProj[0][0] = 2.0f / (right - left);
	viewProj[1][1] = 2.0f / (top - bottom);
	viewProj[2][2] = -1.0f;
	viewProj[3][0] = -(right + left) / (right - left);
	viewProj[3][1] = -(top + bottom) / (top - bottom);
	viewProj[3][3] = 1.0f;
}

/// <summary>Ensures all created objects are cleaned up correctly and allocated memory is freed</summary>
void VulkanHelloAPI::deinitialize()
{
	// This function ensures that all the objects that were created are cleaned up correctly and nothing
	// is left "open" when the application is closed.

	// Wait for the device to have finished all operations before starting the clean up.
	debugAssertFunctionResult(vk::DeviceWaitIdle(appManager.device), "Device Wait for Idle");

	// Destroy the fence used to sync work between the CPU and GPU.
	vk::WaitForFences(appManager.device, static_cast<uint32_t>(appManager.frameFences.size()), appManager.frameFences.data(), true, uint64_t(-1));
	vk::ResetFences(appManager.device, static_cast<uint32_t>(appManager.frameFences.size()), appManager.frameFences.data());
	for (auto& fence : appManager.frameFences) { vk::DestroyFence(appManager.device, fence, nullptr); }

	// Destroy the semaphores used for image acquisition and rendering.
	for (auto& semaphore : appManager.acquireSemaphore) { vk::DestroySemaphore(appManager.device, semaphore, nullptr); }

	for (auto& semaphore : appManager.presentSemaphores) { vk::DestroySemaphore(appManager.device, semaphore, nullptr); }

	// Free the memory allocated for the descriptor sets.
	vk::FreeDescriptorSets(appManager.device, appManager.descriptorPool, 1, &appManager.staticDescSet);
	vk::FreeDescriptorSets(appManager.device, appManager.descriptorPool, 1, &appManager.dynamicDescSet);

	// Destroy both the descriptor layouts and descriptor pool.
	vk::DestroyDescriptorSetLayout(appManager.device, appManager.staticDescriptorSetLayout, nullptr);
	vk::DestroyDescriptorSetLayout(appManager.device, appManager.dynamicDescriptorSetLayout, nullptr);
	vk::DestroyDescriptorPool(appManager.device, appManager.descriptorPool, nullptr);

	// Destroy the uniform buffer and free the memory.
	vk::DestroyBuffer(appManager.device, appManager.dynamicUniformBufferData.buffer, nullptr);
	vk::FreeMemory(appManager.device, appManager.dynamicUniformBufferData.memory, nullptr);

	// Destroy the pipeline followed by the pipeline layout.
	vk::DestroyPipeline(appManager.device, appManager.pipeline, nullptr);
	vk::DestroyPipelineLayout(appManager.device, appManager.pipelineLayout, nullptr);

	// Destroy the texture image.
	vk::DestroyImage(appManager.device, appManager.texture.image, nullptr);

	// Destroy the texture image view.
	vk::DestroyImageView(appManager.device, appManager.texture.view, nullptr);

	// Free the memory allocated for the texture.
	vk::FreeMemory(appManager.device, appManager.texture.memory, nullptr);

	// Destroy the sampler.
	vk::DestroySampler(appManager.device, appManager.texture.sampler, nullptr);

	// Destroy then free the memory for the vertex buffer.
	vk::DestroyBuffer(appManager.device, appManager.vertexBuffer.buffer, nullptr);
	vk::FreeMemory(appManager.device, appManager.vertexBuffer.memory, nullptr);

	// Iterate through each of the framebuffers and destroy them.
	for (uint32_t i = 0; i < appManager.frameBuffers.size(); i++) { vk::DestroyFramebuffer(appManager.device, appManager.frameBuffers[i], nullptr); }

	// Destroy the two shader modules - vertex and fragment.
	vk::DestroyShaderModule(appManager.device, appManager.shaderStages[0].module, nullptr);
	vk::DestroyShaderModule(appManager.device, appManager.shaderStages[1].module, nullptr);

	// Destroy the render pass.
	vk::DestroyRenderPass(appManager.device, appManager.renderPass, nullptr);

	// Clean up the swapchain image views.
	for (auto& imagebuffers : appManager.swapChainImages) { vk::DestroyImageView(appManager.device, imagebuffers.view, nullptr); }

	// Free the allocated memory in the command buffers.
	vk::FreeCommandBuffers(appManager.device, appManager.commandPool, static_cast<uint32_t>(appManager.cmdBuffers.size()), appManager.cmdBuffers.data());

	// Destroy the command pool.
	vk::DestroyCommandPool(appManager.device, appManager.commandPool, nullptr);

	// Clean up the swapchain.
	vk::DestroySwapchainKHR(appManager.device, appManager.swapchain, nullptr);

	// Clean up the surface.
	vk::DestroySurfaceKHR(appManager.instance, appManager.surface, nullptr);

	// Destroy the logical device.
	vk::DestroyDevice(appManager.device, nullptr);
}
