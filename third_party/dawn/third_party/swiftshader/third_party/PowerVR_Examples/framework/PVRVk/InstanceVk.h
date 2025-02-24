/*!
\brief The PVRVk Instance class
\file PVRVk/InstanceVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/ExtensionsVk.h"
#include "PVRVk/PVRVkObjectBaseVk.h"
#include "PVRVk/DebugReportCallbackVk.h"
#include "PVRVk/DebugUtilsMessengerVk.h"
#include "PVRVk/DebugUtilsVk.h"
#include "PVRVk/PhysicalDeviceVk.h"
#include "PVRVk/ForwardDecObjectsVk.h"
#include "PVRVk/LayersVk.h"
#include "PVRVk/SurfaceVk.h"

namespace pvrvk {

/// <summary>Singleton global that returns vulkan non instance/device function pointers
/// (vkGetInstanceProcAddr, vkCreateInstance, vkEnumerateInstanceExtensionProperties, etc) </summary>
/// <returns>A reference to the singleton VkBindings object</returns>
VkBindings& getVkBindings();

/// <summary>Contains instant info used for creating Vulkan instance</summary>
struct InstanceCreateInfo
{
private:
	InstanceCreateFlags flags; //!< Reserved for future use
	ApplicationInfo applicationInfo; //!< NULL or a pointer to an instance of ApplicationInfo. If not NULL, this information helps implementations recognize behavior
									 //!< inherent to classes of applications.
	pvrvk::VulkanLayerList layers; //!<  Array of null-terminated UTF-8 strings containing the names of layers to enable for the created instance
	pvrvk::VulkanExtensionList extensions; //!<  Array of null-terminated UTF-8 strings containing the names of extensions to enable for the created instance

	DebugUtilsMessengerCreateInfo debugUtilsMessengerCreateInfo; //!<  Used to capture events that occur while creating or destroying an instance
	ValidationFeatures validationFeatures; //!<  Used to specify particular validation features to use

public:
	/// <summary>Constructor. Default initialised to 0</summary>
	InstanceCreateInfo() : flags(InstanceCreateFlags(0)) {}

	/// <summary>Constructor</summary>
	/// <param name="applicationInfo">A an application info structure</param>
	/// <param name="extensions">A set of extensions to enable</param>
	/// <param name="layers">A set of layers to enable</param>
	/// <param name="flags">A set of InstanceCreateFlags to use</param>
	/// <param name="debugUtilsMessengerCreateInfo">A creation structure which will optionally make use of a
	/// debug utils callback for validation of the instance construction</param>
	explicit InstanceCreateInfo(const ApplicationInfo& applicationInfo, const VulkanExtensionList& extensions = VulkanExtensionList(),
		const VulkanLayerList& layers = VulkanLayerList(), InstanceCreateFlags flags = InstanceCreateFlags::e_NONE,
		pvrvk::DebugUtilsMessengerCreateInfo debugUtilsMessengerCreateInfo = pvrvk::DebugUtilsMessengerCreateInfo())
		: flags(InstanceCreateFlags(flags)), applicationInfo(applicationInfo)
	{
		setExtensionList(extensions);
		setLayerList(layers);
		setDebugUtilsMessengerCreateInfo(debugUtilsMessengerCreateInfo);
	}

	/// <summary>Get the DebugUtilsMessengerCreateInfo which will be linked to the pNext element of the VkInstanceCreateInfo structure given to vkCreateInstance</summary>
	/// <returns>A DebugUtilsMessengerCreateInfo which will be used to determine whether events triggered during instance creation/destruction will be captured</returns>
	inline const pvrvk::DebugUtilsMessengerCreateInfo getDebugUtilsMessengerCreateInfo() const { return debugUtilsMessengerCreateInfo; }
	/// <summary>By setting a DebugUtilsMessengerCreateInfo the application will link a VkDebugUtilsMessengerCreateInfoEXT structure to the pNext element of the
	/// VkInstanceCreateInfo structure given to vkCreateInstance.
	/// This callback is only valid for the duration of the vkCreateInstance and the vkDestroyInstance call</summary>
	/// <param name="createInfo">A DebugUtilsMessengerCreateInfo which will determine the extent to which events will be captured when creating/destroying the instance</param>
	inline void setDebugUtilsMessengerCreateInfo(pvrvk::DebugUtilsMessengerCreateInfo& createInfo) { debugUtilsMessengerCreateInfo = createInfo; }

	/// <summary>Get the ValidationFeatures which will be linked to the pNext element of the VkInstanceCreateInfo structure given to vkCreateInstance</summary>
	/// <returns>A ValidationFeatures structure which will be used to determine the validation features to use</returns>
	inline const pvrvk::ValidationFeatures getValidationFeatures() const { return validationFeatures; }
	/// <summary>By setting a ValidationFeatures the application will link a VkValidationFeaturesEXT structure to the pNext element of the
	/// VkInstanceCreateInfo structure given to vkCreateInstance. This structure will determine the types of validation used.</summary>
	/// <param name="inValidationFeatures">A ValidationFeatures which will determine the types of validation used</param>
	inline void setValidationFeatures(pvrvk::ValidationFeatures& inValidationFeatures) { this->validationFeatures = inValidationFeatures; }

	/// <summary>Get the instance creation flags</summary>
	/// <returns>The instance creation flags</returns>
	inline const InstanceCreateFlags& getFlags() const { return flags; }
	/// <summary>Sets the instance creation flags</summary>
	/// <param name="inFlags">A set of InstanceCreateFlags to use</param>
	inline void setFlags(const InstanceCreateFlags& inFlags) { this->flags = inFlags; }
	/// <summary>Get the instance application info</summary>
	/// <returns>The instance application info</returns>
	inline const ApplicationInfo& getApplicationInfo() const { return applicationInfo; }
	/// <summary>Sets the application info structure</summary>
	/// <param name="inApplicationInfo">A new application info structure</param>
	inline void setApplicationInfo(const ApplicationInfo& inApplicationInfo) { this->applicationInfo = inApplicationInfo; }
	/// <summary>Get the list of extensions</summary>
	/// <returns>The list of instance extensions</returns>
	inline const VulkanExtensionList& getExtensionList() const { return extensions; }
	/// <summary>Sets the extension list</summary>
	/// <param name="inExtensions">A VulkanExtensionList</param>
	inline void setExtensionList(const VulkanExtensionList& inExtensions) { this->extensions = inExtensions; }
	/// <summary>Get the list of layers</summary>
	/// <returns>The list of instance layers</returns>
	inline const VulkanLayerList& getLayerList() const { return layers; }
	/// <summary>Sets the layer list</summary>
	/// <param name="inLayers">A VulkanLayerList</param>
	inline void setLayerList(const VulkanLayerList& inLayers) { this->layers = inLayers; }
};

namespace impl {
/// <summary>The Instance is a system-wide vulkan "implementation", similar in concept to the
/// "installation" of Vulkan libraries on a system. Contrast with the "Physical Device" which
/// for example represents a particular driver implementing Vulkan for a specific Device.
/// Conceptually, the Instance "Forwards" to the "Physical Device / Device"</summary>
class Instance_ : public PVRVkObjectBase<VkInstance, ObjectType::e_INSTANCE>, public std::enable_shared_from_this<Instance_>
{
private:
	friend class InstanceHelperFactory_;

	class make_shared_enabler
	{
	protected:
		make_shared_enabler() {}
		friend class Instance_;
	};

	static Instance constructShared(const InstanceCreateInfo& instanceCreateInfo) { return std::make_shared<Instance_>(make_shared_enabler{}, instanceCreateInfo); }

	InstanceCreateInfo _createInfo;
	VkInstanceBindings _vkBindings;
	InstanceExtensionTable _extensionTable;
	std::vector<PhysicalDevice> _physicalDevices;

public:
	//!\cond NO_DOXYGEN
	DECLARE_NO_COPY_SEMANTICS(Instance_)
	explicit Instance_(make_shared_enabler, const InstanceCreateInfo& instanceCreateInfo);

	~Instance_()
	{
		_physicalDevices.clear();
		if (getVkHandle() != VK_NULL_HANDLE)
		{
			_vkBindings.vkDestroyInstance(getVkHandle(), nullptr);
			_vkHandle = VK_NULL_HANDLE;
		}
	}
	//!\endcond

	/// <summary>Retrieve and initialise the list of physical devices</summary>
	void retrievePhysicalDevices();

	/// <summary>Get instance create info(const)</summary>
	/// <returns>const InstanceCreateInfo&</returns>
	const InstanceCreateInfo& getCreateInfo() const { return _createInfo; }

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
	/// <summary>Create an android surface</summary>
	/// <param name="window">A pointer to an Android Native Window</param>
	/// <param name="flags">A set of AndroidSurfaceCreateFlagsKHR flags to use when creating the android surface</param>
	/// <returns>Valid AndroidSurface object if success.</returns>
	AndroidSurface createAndroidSurface(ANativeWindow* window, AndroidSurfaceCreateFlagsKHR flags = AndroidSurfaceCreateFlagsKHR::e_NONE)
	{
		Instance instance = shared_from_this();
		return impl::AndroidSurface_::constructShared(instance, window, flags);
	}
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
	/// <summary>Create a Win32 surface</summary>
	/// <param name="hInstance">A Win32 HINSTANCE</param>
	/// <param name="hwnd">A Win32 HWND</param>
	/// <param name="flags">A set of Win32SurfaceCreateFlagsKHR flags to use when creating the Win32 surface</param>
	/// <returns>Valid Win32Surface object if success.</returns>
	Win32Surface createWin32Surface(HINSTANCE hInstance, HWND hwnd, Win32SurfaceCreateFlagsKHR flags = Win32SurfaceCreateFlagsKHR::e_NONE)
	{
		Instance instance = shared_from_this();
		return impl::Win32Surface_::constructShared(instance, hInstance, hwnd, flags);
	}
#endif

#if defined(VK_USE_PLATFORM_XCB_KHR)
	/// <summary>Create an XCB surface</summary>
	/// <param name="connection">An Xcb connection</param>
	/// <param name="window">An Xcb window</param>
	/// <param name="flags">A set of XcbSurfaceCreateFlagsKHR flags to use when creating the xcb surface</param>
	/// <returns>Valid XcbSurface object if success.</returns>
	XcbSurface createXcbSurface(xcb_connection_t* connection, xcb_window_t window, XcbSurfaceCreateFlagsKHR flags = XcbSurfaceCreateFlagsKHR::e_NONE)
	{
		Instance instance = shared_from_this();
		return impl::XcbSurface_::constructShared(instance, connection, window, flags);
	}
#endif

#if defined(VK_USE_PLATFORM_XLIB_KHR)
	/// <summary>Create an Xlib surface</summary>
	/// <param name="dpy">An xlib display</param>
	/// <param name="window">An xlib window</param>
	/// <param name="flags">A set of XlibSurfaceCreateFlagsKHR flags to use when creating the xlib surface</param>
	/// <returns>Valid XlibSurface object if success.</returns>
	XlibSurface createXlibSurface(::Display* dpy, Window window, XlibSurfaceCreateFlagsKHR flags = XlibSurfaceCreateFlagsKHR::e_NONE)
	{
		Instance instance = shared_from_this();
		return impl::XlibSurface_::constructShared(instance, dpy, window, flags);
	}
#endif

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
	/// <summary>Create a Wayland surface</summary>
	/// <param name="display">The Wayland display</param>
	/// <param name="surface">A Wayland surface</param>
	/// <param name="flags">A set of WaylandSurfaceCreateFlagsKHR flags to use when creating the Wayland surface</param>
	/// <returns>Valid WaylandSurface object if success.</returns>
	WaylandSurface createWaylandSurface(wl_display* display, wl_surface* surface, WaylandSurfaceCreateFlagsKHR flags = WaylandSurfaceCreateFlagsKHR::e_NONE)
	{
		Instance instance = shared_from_this();
		return impl::WaylandSurface_::constructShared(instance, display, surface, flags);
	}
#endif

#if defined(VK_USE_PLATFORM_MACOS_MVK)
	/// <summary>Create a MacOS surface</summary>
	/// <param name="view">A CAMetalLayer backed NSView</param>
	/// <returns>Valid MacOSSurface object if success.</returns>
	MacOSSurface createMacOSSurface(void* view)
	{
		Instance instance = shared_from_this();
		return impl::MacOSSurface_::constructShared(instance, view);
	}
#endif

	/// <summary>Create a DisplayPlane surface</summary>
	/// <param name="displayMode">A display mode to use for creating the DisplayPlane Surface</param>
	/// <param name="imageExtent">The image extent to use for creating the DisplayPlane Surface</param>
	/// <param name="flags">A set of DisplaySurfaceCreateFlagsKHR flags to use when creating the DisplayPlane surface</param>
	/// <param name="planeIndex">A plane index</param>
	/// <param name="planeStackIndex">A plane stack index</param>
	/// <param name="transformFlags">A set of SurfaceTransformFlagsKHR flags to use when creating the DisplayPlane surface</param>
	/// <param name="globalAlpha">A global alpha value</param>
	/// <param name="alphaFlags">A set of DisplayPlaneAlphaFlagsKHR flags to use when creating the DisplayPlane surface</param>
	/// <returns>Valid DisplayPlane object if success.</returns>
	DisplayPlaneSurface createDisplayPlaneSurface(const DisplayMode& displayMode, Extent2D imageExtent, const DisplaySurfaceCreateFlagsKHR flags = DisplaySurfaceCreateFlagsKHR::e_NONE,
		uint32_t planeIndex = 0, uint32_t planeStackIndex = 0, SurfaceTransformFlagsKHR transformFlags = SurfaceTransformFlagsKHR::e_IDENTITY_BIT_KHR, float globalAlpha = 0.0f,
		DisplayPlaneAlphaFlagsKHR alphaFlags = DisplayPlaneAlphaFlagsKHR::e_PER_PIXEL_BIT_KHR)
	{
		Instance instance = shared_from_this();
		return impl::DisplayPlaneSurface_::constructShared(instance, displayMode, imageExtent, flags, planeIndex, planeStackIndex, transformFlags, globalAlpha, alphaFlags);
	}

	/// <summary>Get a list of enabled extensions which includes names and spec versions</summary>
	/// <returns>VulkanExtensionList&</returns>
	const VulkanExtensionList& getEnabledExtensionsList() { return _createInfo.getExtensionList(); }

	/// <summary>Get a list of enabled layers which includes names and spec versions</summary>
	/// <returns>VulkanLayerList&</returns>
	const VulkanLayerList& getEnabledLayersList() { return _createInfo.getLayerList(); }

	/// <summary>Return a table which contains boolean members set to true/false corresponding to whether specific extensions have been enabled</summary>
	/// <returns>A table of extensions</returns>
	const InstanceExtensionTable& getEnabledExtensionTable() const { return _extensionTable; }

	/// <summary>Creates a debug utils messenger object</summary>
	/// <param name="createInfo">DebugUtilsMessengerCreateInfo structure specifying how the debug utils messenger should function.</param>
	/// <returns>Returns the created debug utils messenger object.</returns>
	inline DebugUtilsMessenger createDebugUtilsMessenger(const DebugUtilsMessengerCreateInfo& createInfo)
	{
		Instance instance = shared_from_this();
		return impl::DebugUtilsMessenger_::constructShared(instance, createInfo);
	}

	/// <summary>Creates a debug report callback object</summary>
	/// <param name="createInfo">DebugReportCallbackCreateInfo structure specifying how the debug report callback function should work.</param>
	/// <returns>Returns the created debug report callback object.</returns>
	inline DebugReportCallback createDebugReportCallback(const DebugReportCallbackCreateInfo& createInfo)
	{
		Instance instance = shared_from_this();
		return impl::DebugReportCallback_::constructShared(instance, createInfo);
	}

	/// <summary>Submits a debug report message directly into the debug stream.</summary>
	/// <param name="flags">Specifies the DebugReportFlagsEXT classification of this message.</param>
	/// <param name="objectType">Specifies the type of object being used or created at the time the event was triggered.</param>
	/// <param name="object">The object where the issue was detected. object can be VK_NULL_HANDLE if there is no object associated with the event.</param>
	/// <param name="location">An application defined value.</param>
	/// <param name="messageCode">An application defined value.</param>
	/// <param name="layerPrefix">The abbreviation of the component making this event/message.</param>
	/// <param name="message">The message itself detailing the trigger conditions.</param>
	inline void debugReportMessage(DebugReportFlagsEXT flags, DebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode,
		const std::string& layerPrefix, const std::string& message)
	{
		_vkBindings.vkDebugReportMessageEXT(getVkHandle(), static_cast<VkDebugReportFlagsEXT>(flags), static_cast<VkDebugReportObjectTypeEXT>(objectType), object, location,
			messageCode, layerPrefix.c_str(), message.c_str());
	}

	/// <summary>Submits a debug report message directly into the debug stream.</summary>
	/// <param name="flags">Specifies the DebugReportFlagsEXT classification of this message.</param>
	/// <param name="object">A PVRVkObjectBase object where the issue was detected.</param>
	/// <param name="location">An application defined value.</param>
	/// <param name="messageCode">An application defined value.</param>
	/// <param name="layerPrefix">The abbreviation of the component making this event/message.</param>
	/// <param name="message">The message itself detailing the trigger conditions.</param>
	inline void debugReportMessage(DebugReportFlagsEXT flags, PVRVkObjectBase object, size_t location, int32_t messageCode, const std::string& layerPrefix, const std::string& message)
	{
		debugReportMessage(flags, pvrvk::convertObjectTypeToDebugReportObjectType(object.getObjectType()),
			*static_cast<const uint64_t*>(static_cast<const void*>(object.getVkHandle())), location, messageCode, layerPrefix, message);
	}

	/// <summary>Submits a debug utils message directly into the debug stream.</summary>
	/// <param name="inMessageSeverity">the DebugUtilsMessageSeverityFlagBitsEXT severity of this event/message.</param>
	/// <param name="inMessageTypes">A bitmask of DebugUtilsMessageTypeFlagBitsEXT specifying which type of event(s) to identify with this message.</param>
	/// <param name="inCallbackData">Contains all the callback related data in the DebugUtilsMessengerCallbackDataEXT structure.</param>
	inline void submitDebugUtilsMessage(
		DebugUtilsMessageSeverityFlagsEXT inMessageSeverity, DebugUtilsMessageTypeFlagsEXT inMessageTypes, const DebugUtilsMessengerCallbackData& inCallbackData)
	{
		VkDebugUtilsMessengerCallbackDataEXT vkCallbackData = {};
		vkCallbackData.sType = static_cast<VkStructureType>(StructureType::e_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT);
		vkCallbackData.flags = static_cast<VkDebugUtilsMessengerCallbackDataFlagsEXT>(inCallbackData.getFlags());
		vkCallbackData.pMessageIdName = inCallbackData.getMessageIdName().c_str();
		vkCallbackData.messageIdNumber = inCallbackData.getMessageIdNumber();
		vkCallbackData.pMessage = inCallbackData.getMessage().c_str();

		pvrvk::ArrayOrVector<VkDebugUtilsLabelEXT, 4> vkQueueLabels(inCallbackData.getNumQueueLabels());
		pvrvk::ArrayOrVector<VkDebugUtilsLabelEXT, 4> vkCmdBufLabels(inCallbackData.getNumCmdBufLabels());
		pvrvk::ArrayOrVector<VkDebugUtilsObjectNameInfoEXT, 4> vkObjectNames(inCallbackData.getNumObjects());

		// Add queue labels
		if (inCallbackData.getNumQueueLabels())
		{
			vkCallbackData.queueLabelCount = inCallbackData.getNumQueueLabels();
			for (uint32_t i = 0; i < inCallbackData.getNumQueueLabels(); ++i)
			{
				DebugUtilsLabel queueLabel = inCallbackData.getQueueLabel(i);

				vkQueueLabels[i].sType = static_cast<VkStructureType>(StructureType::e_DEBUG_UTILS_LABEL_EXT);
				vkQueueLabels[i].pLabelName = queueLabel.getLabelName().c_str();
				vkQueueLabels[i].color[0] = queueLabel.getR();
				vkQueueLabels[i].color[1] = queueLabel.getG();
				vkQueueLabels[i].color[2] = queueLabel.getB();
				vkQueueLabels[i].color[3] = queueLabel.getA();
			}

			vkCallbackData.pQueueLabels = vkQueueLabels.get();
		}

		// Add command buffer labels
		if (inCallbackData.getNumCmdBufLabels())
		{
			vkCallbackData.cmdBufLabelCount = inCallbackData.getNumCmdBufLabels();
			for (uint32_t i = 0; i < inCallbackData.getNumCmdBufLabels(); ++i)
			{
				DebugUtilsLabel cmdBufLabel = inCallbackData.getCmdBufLabel(i);

				vkCmdBufLabels[i].sType = static_cast<VkStructureType>(StructureType::e_DEBUG_UTILS_LABEL_EXT);
				vkCmdBufLabels[i].pLabelName = cmdBufLabel.getLabelName().c_str();
				vkCmdBufLabels[i].color[0] = cmdBufLabel.getR();
				vkCmdBufLabels[i].color[1] = cmdBufLabel.getG();
				vkCmdBufLabels[i].color[2] = cmdBufLabel.getB();
				vkCmdBufLabels[i].color[3] = cmdBufLabel.getA();
			}

			vkCallbackData.pCmdBufLabels = vkCmdBufLabels.get();
		}

		// Add object names
		if (inCallbackData.getNumObjects())
		{
			vkCallbackData.objectCount = inCallbackData.getNumObjects();
			for (uint32_t i = 0; i < inCallbackData.getNumObjects(); ++i)
			{
				DebugUtilsObjectNameInfo objectName = inCallbackData.getObject(i);

				vkObjectNames[i].sType = static_cast<VkStructureType>(StructureType::e_DEBUG_UTILS_OBJECT_NAME_INFO_EXT);
				vkObjectNames[i].pObjectName = objectName.getObjectName().c_str();
				vkObjectNames[i].objectType = static_cast<VkObjectType>(objectName.getObjectType());
				vkObjectNames[i].objectHandle = objectName.getObjectHandle();
			}

			vkCallbackData.pObjects = vkObjectNames.get();
		}

		_vkBindings.vkSubmitDebugUtilsMessageEXT(
			getVkHandle(), static_cast<VkDebugUtilsMessageSeverityFlagBitsEXT>(inMessageSeverity), static_cast<VkDebugUtilsMessageTypeFlagsEXT>(inMessageTypes), &vkCallbackData);
	}

	/// <summary>Gets the instance dispatch table</summary>
	/// <returns>The instance dispatch table</returns>
	inline const VkInstanceBindings& getVkBindings() const { return _vkBindings; }

	/// <summary>Get the list of physical devices (const)</summary>
	/// <returns>const PhysicalDevice&</returns>
	const std::vector<PhysicalDevice>& getPhysicalDevices() const;

	/// <summary>Get physical device (const)</summary>
	/// <param name="id">Physcialdevice id</param>
	/// <returns>const PhysicalDevice&</returns>
	const PhysicalDevice& getPhysicalDevice(uint32_t id) const;

	/// <summary>Get physical device (const)</summary>
	/// <param name="id">Physcialdevice id</param>
	/// <returns>const PhysicalDevice&</returns>
	PhysicalDevice& getPhysicalDevice(uint32_t id);

	/// <summary>Get number of physcial device available</summary>
	/// <returns>uint32_t</returns>
	uint32_t getNumPhysicalDevices() const { return static_cast<uint32_t>(_physicalDevices.size()); }
};
} // namespace impl

/// <summary>Create a PVRVk Instance</summary>
/// <param name="createInfo">The Create Info object for created Instance</param>
/// <returns>A newly created instance. In case of failure, null instance</returns>
Instance createInstance(const InstanceCreateInfo& createInfo);
} // namespace pvrvk
