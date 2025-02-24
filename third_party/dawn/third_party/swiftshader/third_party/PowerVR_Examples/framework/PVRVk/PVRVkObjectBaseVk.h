/*!
\brief Defines a simple interface for a PVRVk Vulkan object.
\file PVRVk/PVRVkObjectBaseVk.h
\author PowerVR by Imagination, Developer Technology Team
\copyright Copyright (c) Imagination Technologies Limited.
*/

#pragma once
#include "PVRVk/TypesVk.h"
#include "PVRVk/ForwardDecObjectsVk.h"

/// <summary>Main PowerVR Framework Namespace</summary>
namespace pvrvk {
/// <summary>Contains internal objects and wrapped versions of the PVRVk module</summary>
namespace impl {
/// <summary>Defines a simple interface for a Vulkan object.</summary>
template<class VkHandleType, ObjectType PVRVkObjectType>
class PVRVkObjectBase
{
public:
	DECLARE_NO_COPY_SEMANTICS(PVRVkObjectBase)
	/// <summary>Get vulkan object (const)</summary>
	/// <returns>Returns the templated 'HandleType'</returns>
	inline const VkHandleType& getVkHandle() const { return _vkHandle; }

	/// <summary>Returns the specified object's type.</summary>
	/// <returns>The object type</returns>
	inline ObjectType getObjectType() const { return _objectType; }

protected:
	/// <summary>The Vulkan object handle representing the Vulkan object at an API level.</summary>
	VkHandleType _vkHandle;

	/// <summary>The Vulkan object type.</summary>
	ObjectType _objectType;

	/// <summary>default Constructor for an object handle</summary>
	PVRVkObjectBase() : _vkHandle(VK_NULL_HANDLE), _objectType(PVRVkObjectType) {}

	/// <summary>Constructor for an object handle initialising the Vulkan object handle</summary>
	/// <param name="handle">The Vulkan object handle given to the Vulkan object.</param>
	explicit PVRVkObjectBase(const VkHandleType& handle) : _vkHandle(handle), _objectType(PVRVkObjectType) { assert(_objectType != ObjectType::e_UNKNOWN); }
};

/// <summary>Defines a simple interface for a Vulkan which holds a reference to a Vulkan instance.</summary>
template<class VkHandleType, ObjectType PVRVkObjectType>
class PVRVkInstanceObjectBase : public PVRVkObjectBase<VkHandleType, PVRVkObjectType>
{
public:
	DECLARE_NO_COPY_SEMANTICS(PVRVkInstanceObjectBase)

	/// <summary>Get instance (const)</summary>
	/// <returns>Instance</returns>
	inline const Instance getInstance() const { return _instance.lock(); }

	/// <summary>Get instance</summary>
	/// <returns>Instance</returns>
	inline Instance getInstance() { return _instance.lock(); }

protected:
	/// <summary>The instance which was used to create this InstanceObject</summary>
	InstanceWeakPtr _instance;

	/// <summary>default Constructor for an instance object handle</summary>
	PVRVkInstanceObjectBase() : PVRVkObjectBase<VkHandleType, PVRVkObjectType>() {}
	/// <summary>Constructor for an instance object handle initialising the instance</summary>
	/// <param name="instance">The Vulkan instance used to create the Vulkan object.</param>
	explicit PVRVkInstanceObjectBase(const InstanceWeakPtr& instance) : PVRVkObjectBase<VkHandleType, PVRVkObjectType>(), _instance(instance) {}
	/// <summary>Constructor for an instance object handle initialising the Vulkan object handle</summary>
	/// <param name="handle">The Vulkan object handle given to the Vulkan object.</param>
	explicit PVRVkInstanceObjectBase(const VkHandleType& handle) : PVRVkObjectBase<VkHandleType, PVRVkObjectType>(handle) {}
	/// <summary>Constructor for an instance object handle initialising the instance and Vulkan object handle</summary>
	/// <param name="instance">The Vulkan instance used to create the Vulkan object.</param>
	/// <param name="handle">The Vulkan object handle given to the Vulkan object.</param>
	PVRVkInstanceObjectBase(const InstanceWeakPtr& instance, const VkHandleType& handle) : PVRVkObjectBase<VkHandleType, PVRVkObjectType>(handle), _instance(instance) {}
};

/// <summary>Defines a simple interface for a Vulkan which holds a reference to a Vulkan Physical Device.</summary>
template<class VkHandleType, ObjectType PVRVkObjectType>
class PVRVkPhysicalDeviceObjectBase : public PVRVkObjectBase<VkHandleType, PVRVkObjectType>
{
public:
	DECLARE_NO_COPY_SEMANTICS(PVRVkPhysicalDeviceObjectBase)

	/// <summary>Get physical device (const)</summary>
	/// <returns>PhysicalDevice</returns>
	inline const PhysicalDevice getPhysicalDevice() const { return _physicalDevice.lock(); }

	/// <summary>Get physical device</summary>
	/// <returns>PhysicalDevice</returns>
	inline PhysicalDevice getPhysicalDevice() { return _physicalDevice.lock(); }

protected:
	/// <summary>The physical device which was used to create this PhysicalDeviceObject</summary>
	PhysicalDeviceWeakPtr _physicalDevice;

	/// <summary>default Constructor for a physical device object handle</summary>
	PVRVkPhysicalDeviceObjectBase() : PVRVkObjectBase<VkHandleType, PVRVkObjectType>() {}
	/// <summary>Constructor for a physical device object handle initialising the physical device</summary>
	/// <param name="physicalDevice">The Vulkan physical device used to create the Vulkan object.</param>
	explicit PVRVkPhysicalDeviceObjectBase(const PhysicalDeviceWeakPtr& physicalDevice) : PVRVkObjectBase<VkHandleType, PVRVkObjectType>(), _physicalDevice(physicalDevice) {}
	/// <summary>Constructor for a physical device object handle initialising the physical device and Vulkan object handle</summary>
	/// <param name="handle">The Vulkan object handle given to the Vulkan object.</param>
	explicit PVRVkPhysicalDeviceObjectBase(const VkHandleType& handle) : PVRVkObjectBase<VkHandleType, PVRVkObjectType>(handle) {}
	/// <summary>Constructor for a physical device object handle initialising the physical device and Vulkan object handle</summary>
	/// <param name="physicalDevice">The Vulkan physical device used to create the Vulkan object.</param>
	/// <param name="handle">The Vulkan object handle given to the Vulkan object.</param>
	PVRVkPhysicalDeviceObjectBase(const PhysicalDeviceWeakPtr& physicalDevice, const VkHandleType& handle)
		: PVRVkObjectBase<VkHandleType, PVRVkObjectType>(handle), _physicalDevice(physicalDevice)
	{}
};

/// <summary>Defines a simple interface for a Vulkan object which holds a reference to a particular device.</summary>
template<class VkHandleType, ObjectType PVRVkObjectType>
class PVRVkDeviceObjectBase : public PVRVkObjectBase<VkHandleType, PVRVkObjectType>
{
public:
	DECLARE_NO_COPY_SEMANTICS(PVRVkDeviceObjectBase)

	/// <summary>Get Device (const)</summary>
	/// <returns>DeviceWeakPtr</returns>
	inline const Device getDevice() const { return _device.lock(); }

	/// <summary>Get Device</summary>
	/// <returns>DeviceWeakPtr</returns>
	inline Device getDevice() { return _device.lock(); }

protected:
	/// <summary>The device which was used to create this DeviceObject</summary>
	DeviceWeakPtr _device;

	/// <summary>default Constructor for a device object handle</summary>
	PVRVkDeviceObjectBase() : PVRVkObjectBase<VkHandleType, PVRVkObjectType>() {}
	/// <summary>Constructor for a device object handle initialising the Vulkan device</summary>
	/// <param name="device">The Vulkan device used to create the Vulkan object.</param>
	explicit PVRVkDeviceObjectBase(const DeviceWeakPtr& device) : PVRVkObjectBase<VkHandleType, PVRVkObjectType>(), _device(device) {}
	/// <summary>Constructor for a device object handle initialising the Vulkan object handle</summary>
	/// <param name="handle">The Vulkan object handle given to the Vulkan object.</param>
	explicit PVRVkDeviceObjectBase(const VkHandleType& handle) : PVRVkObjectBase<VkHandleType, PVRVkObjectType>(handle) {}
	/// <summary>Constructor for a device object handle initialising the Vulkan device and device object handle</summary>
	/// <param name="device">The Vulkan device used to create the Vulkan device.</param>
	/// <param name="handle">The Vulkan object handle given to the Vulkan object.</param>
	PVRVkDeviceObjectBase(const DeviceWeakPtr& device, const VkHandleType& handle) : PVRVkObjectBase<VkHandleType, PVRVkObjectType>(handle), _device(device) {}
};
} // namespace impl
} // namespace pvrvk
