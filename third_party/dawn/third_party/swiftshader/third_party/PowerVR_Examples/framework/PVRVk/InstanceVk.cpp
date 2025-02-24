/*!
\brief Function definitions for the Instance class
\file PVRVk/InstanceVk.cpp
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

//!\cond NO_DOXYGEN
#include "InstanceVk.h"

namespace pvrvk {

VkBindings& getVkBindings()
{
	static bool isVkBindingsInitialized = false;
	static VkBindings vkBindings;

	if (!isVkBindingsInitialized)
	{
		if (!initVkBindings(&vkBindings)) { throw pvrvk::ErrorInitializationFailed("We were unable to retrieve Vulkan bindings"); }
		isVkBindingsInitialized = true;
	}

	return vkBindings;
}

namespace impl {
class InstanceHelperFactory_
{
public:
	static Instance createVkInstance(const InstanceCreateInfo& createInfo);
};

const std::vector<PhysicalDevice>& Instance_::getPhysicalDevices() const { return _physicalDevices; }

PhysicalDevice& Instance_::getPhysicalDevice(uint32_t id) { return _physicalDevices[id]; }

const PhysicalDevice& Instance_::getPhysicalDevice(uint32_t id) const { return _physicalDevices[id]; }

pvrvk::impl::Instance_::Instance_(make_shared_enabler, const InstanceCreateInfo& instanceCreateInfo) : PVRVkObjectBase()
{
	_createInfo = instanceCreateInfo;

	VkApplicationInfo appInfo = {};
	appInfo.sType = static_cast<VkStructureType>(StructureType::e_APPLICATION_INFO);
	appInfo.apiVersion = _createInfo.getApplicationInfo().getApiVersion();
	appInfo.pApplicationName = _createInfo.getApplicationInfo().getApplicationName().c_str();
	appInfo.applicationVersion = _createInfo.getApplicationInfo().getApplicationVersion();
	appInfo.pEngineName = _createInfo.getApplicationInfo().getEngineName().c_str();
	appInfo.engineVersion = _createInfo.getApplicationInfo().getEngineVersion();

	std::vector<const char*> enabledExtensions;
	std::vector<const char*> enableLayers;

	VkInstanceCreateInfo instanceCreateInfoVk = {};
	instanceCreateInfoVk.sType = static_cast<VkStructureType>(StructureType::e_INSTANCE_CREATE_INFO);
	instanceCreateInfoVk.pApplicationInfo = &appInfo;

	if (instanceCreateInfo.getExtensionList().getNumExtensions())
	{
		enabledExtensions.resize(instanceCreateInfo.getExtensionList().getNumExtensions());
		for (uint32_t i = 0; i < instanceCreateInfo.getExtensionList().getNumExtensions(); ++i)
		{ enabledExtensions[i] = instanceCreateInfo.getExtensionList().getExtension(i).getName().c_str(); }

		instanceCreateInfoVk.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
		instanceCreateInfoVk.ppEnabledExtensionNames = enabledExtensions.data();
	}

	if (instanceCreateInfo.getLayerList().getNumLayers())
	{
		enableLayers.resize(instanceCreateInfo.getLayerList().getNumLayers());
		for (uint32_t i = 0; i < instanceCreateInfo.getLayerList().getNumLayers(); ++i) { enableLayers[i] = _createInfo.getLayerList().getLayer(i).getName().c_str(); }

		instanceCreateInfoVk.enabledLayerCount = static_cast<uint32_t>(enableLayers.size());
		instanceCreateInfoVk.ppEnabledLayerNames = enableLayers.data();
	}

	// Setup callback creation information
	VkDebugUtilsMessengerCreateInfoEXT callbackCreateInfo = {};

	// Setup the debug utils messenger callback if one has been provided
	if (instanceCreateInfo.getDebugUtilsMessengerCreateInfo().getCallback())
	{
		callbackCreateInfo.sType = static_cast<VkStructureType>(StructureType::e_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT);
		callbackCreateInfo.pNext = nullptr;
		callbackCreateInfo.flags = static_cast<VkDebugUtilsMessengerCreateFlagsEXT>(instanceCreateInfo.getDebugUtilsMessengerCreateInfo().getFlags());
		callbackCreateInfo.messageSeverity = static_cast<VkDebugUtilsMessageSeverityFlagsEXT>(instanceCreateInfo.getDebugUtilsMessengerCreateInfo().getMessageSeverity());
		callbackCreateInfo.messageType = static_cast<VkDebugUtilsMessageTypeFlagsEXT>(instanceCreateInfo.getDebugUtilsMessengerCreateInfo().getMessageType());
		callbackCreateInfo.pfnUserCallback = instanceCreateInfo.getDebugUtilsMessengerCreateInfo().getCallback();
		callbackCreateInfo.pUserData = instanceCreateInfo.getDebugUtilsMessengerCreateInfo().getPUserData();

		appendPNext((VkBaseInStructure*)&instanceCreateInfoVk, &callbackCreateInfo);
	}

	// Setup validation features
	VkValidationFeaturesEXT validationFeatures = {};
	pvrvk::ArrayOrVector<VkValidationFeatureEnableEXT, 4> vkEnabledValidationFeatures(instanceCreateInfo.getValidationFeatures().getNumEnabledValidationFeatures());
	pvrvk::ArrayOrVector<VkValidationFeatureDisableEXT, 4> vkDisabledValidationFeatures(instanceCreateInfo.getValidationFeatures().getNumDisabledValidationFeatures());

	// Setup the validation features to enable or disable if they have been provided
	if (instanceCreateInfo.getValidationFeatures().getNumEnabledValidationFeatures() || instanceCreateInfo.getValidationFeatures().getNumDisabledValidationFeatures())
	{
		validationFeatures.sType = static_cast<VkStructureType>(StructureType::e_VALIDATION_FEATURES_EXT);
		validationFeatures.pNext = nullptr;
		validationFeatures.enabledValidationFeatureCount = instanceCreateInfo.getValidationFeatures().getNumEnabledValidationFeatures();
		validationFeatures.disabledValidationFeatureCount = instanceCreateInfo.getValidationFeatures().getNumDisabledValidationFeatures();

		for (uint32_t i = 0; i < instanceCreateInfo.getValidationFeatures().getNumEnabledValidationFeatures(); ++i)
		{ vkEnabledValidationFeatures[i] = static_cast<VkValidationFeatureEnableEXT>(instanceCreateInfo.getValidationFeatures().getEnabledValidationFeature(i)); }

		for (uint32_t i = 0; i < instanceCreateInfo.getValidationFeatures().getNumDisabledValidationFeatures(); ++i)
		{ vkDisabledValidationFeatures[i] = static_cast<VkValidationFeatureDisableEXT>(instanceCreateInfo.getValidationFeatures().getDisabledValidationFeature(i)); }

		validationFeatures.pEnabledValidationFeatures = vkEnabledValidationFeatures.get();
		validationFeatures.pDisabledValidationFeatures = vkDisabledValidationFeatures.get();

		appendPNext((VkBaseInStructure*)&instanceCreateInfoVk, &validationFeatures);
	}

	vkThrowIfFailed(pvrvk::getVkBindings().vkCreateInstance(&instanceCreateInfoVk, nullptr, &_vkHandle), "Instance Constructor");

	// Retrieve the function pointers for the functions taking a VkInstance as their dispatchable handles.
	initVkInstanceBindings(_vkHandle, &_vkBindings, pvrvk::getVkBindings().vkGetInstanceProcAddr);

	// setup the extension table which can be used to cheaply determine support for extensions
	_extensionTable.setEnabledExtensions(enabledExtensions);
}

void pvrvk::impl::Instance_::retrievePhysicalDevices()
{
	// Enumerate the list of installed physical devices
	uint32_t numPhysicalDevices = 0;
	getVkBindings().vkEnumeratePhysicalDevices(getVkHandle(), &numPhysicalDevices, nullptr);

	// Retreive the numPhysicalDevices installed on the system
	pvrvk::ArrayOrVector<VkPhysicalDevice, 2> vkPhysicalDevices(numPhysicalDevices);
	vkThrowIfFailed(getVkBindings().vkEnumeratePhysicalDevices(getVkHandle(), &numPhysicalDevices, vkPhysicalDevices.get()));

	Instance instance = shared_from_this();
	for (uint32_t i = 0; i < numPhysicalDevices; ++i)
	{
		_physicalDevices.emplace_back(PhysicalDevice_::constructShared(instance, vkPhysicalDevices[i]));
		_physicalDevices.back()->retrieveDisplays();
	}
}

pvrvk::Instance InstanceHelperFactory_::createVkInstance(const InstanceCreateInfo& createInfo) { return impl::Instance_::constructShared(createInfo); }
} // namespace impl

Instance createInstance(const InstanceCreateInfo& createInfo) { return impl::InstanceHelperFactory_::createVkInstance(createInfo); }
} // namespace pvrvk
  //!\endcond
