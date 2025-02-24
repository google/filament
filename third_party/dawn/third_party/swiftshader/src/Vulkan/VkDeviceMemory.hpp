// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef VK_DEVICE_MEMORY_HPP_
#define VK_DEVICE_MEMORY_HPP_

#include "VkConfig.hpp"
#include "VkObject.hpp"

namespace vk {

class Device;

class DeviceMemory
{
public:
	struct ExtendedAllocationInfo
	{
		VkDeviceSize allocationSize = 0;
		uint32_t memoryTypeIndex = 0;
		uint64_t opaqueCaptureAddress = 0;
		const VkExportMemoryAllocateInfo *exportMemoryAllocateInfo = nullptr;
		const VkImportMemoryHostPointerInfoEXT *importMemoryHostPointerInfo = nullptr;
#if SWIFTSHADER_EXTERNAL_MEMORY_OPAQUE_FD
		const VkImportMemoryFdInfoKHR *importMemoryFdInfo = nullptr;
#endif
#if SWIFTSHADER_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER
		const VkImportAndroidHardwareBufferInfoANDROID *importAndroidHardwareBufferInfo = nullptr;
		const VkMemoryDedicatedAllocateInfo *dedicatedAllocateInfo = nullptr;
#endif
#if VK_USE_PLATFORM_FUCHSIA
		const VkImportMemoryZirconHandleInfoFUCHSIA *importMemoryZirconHandleInfo = nullptr;
#endif
	};

protected:
	DeviceMemory(const VkMemoryAllocateInfo *pCreateInfo, const DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo, Device *pDevice);

public:
	virtual ~DeviceMemory() {}

	static VkResult Allocate(const VkAllocationCallbacks *pAllocator, const VkMemoryAllocateInfo *pAllocateInfo, VkDeviceMemory *pMemory, Device *device);

	operator VkDeviceMemory()
	{
		return vk::TtoVkT<DeviceMemory, VkDeviceMemory>(this);
	}

	static inline DeviceMemory *Cast(VkDeviceMemory object)
	{
		return vk::VkTtoT<DeviceMemory, VkDeviceMemory>(object);
	}

	static size_t ComputeRequiredAllocationSize(const VkMemoryAllocateInfo *pCreateInfo);

#if SWIFTSHADER_EXTERNAL_MEMORY_OPAQUE_FD
	virtual VkResult exportFd(int *pFd) const;
#endif

#if SWIFTSHADER_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER
	virtual VkResult exportAndroidHardwareBuffer(struct AHardwareBuffer **pAhb) const;
	static VkResult GetAndroidHardwareBufferProperties(VkDevice &device, const struct AHardwareBuffer *buffer, VkAndroidHardwareBufferPropertiesANDROID *pProperties);
#endif

#if VK_USE_PLATFORM_FUCHSIA
	virtual VkResult exportHandle(zx_handle_t *pHandle) const;
#endif

	void destroy(const VkAllocationCallbacks *pAllocator);
	VkResult allocate();
	VkResult map(VkDeviceSize offset, VkDeviceSize size, void **ppData);
	VkDeviceSize getCommittedMemoryInBytes() const;
	void *getOffsetPointer(VkDeviceSize pOffset) const;
	uint64_t getOpaqueCaptureAddress() const;
	uint32_t getMemoryTypeIndex() const { return memoryTypeIndex; }

	// If this is external memory, return true iff its handle type matches the bitmask
	// provided by |supportedExternalHandleTypes|. Otherwise, always return true.
	bool checkExternalMemoryHandleType(
	    VkExternalMemoryHandleTypeFlags supportedExternalMemoryHandleType) const;

	// Some external device memories, such as Android hardware buffers, store per-plane properties.
	virtual bool hasExternalImagePlanes() const { return false; }
	virtual int externalImageRowPitchBytes(VkImageAspectFlagBits aspect) const { return 0; }
	virtual VkDeviceSize externalImageMemoryOffset(VkImageAspectFlagBits aspect) const { return 0; }

protected:
	// Allocate the memory according to `allocationSize`. On success return VK_SUCCESS and sets `buffer`.
	virtual VkResult allocateBuffer();

	// Free previously allocated memory at `buffer`.
	virtual void freeBuffer();

	// Return the handle type flag bit supported by this implementation.
	// A value of 0 corresponds to non-external memory.
	virtual VkExternalMemoryHandleTypeFlagBits getFlagBit() const;

#ifdef SWIFTSHADER_DEVICE_MEMORY_REPORT
	virtual bool isImport() const
	{
		return false;
	}

	virtual uint64_t getMemoryObjectId() const
	{
		return (uint64_t)buffer;
	}
#endif  // SWIFTSHADER_DEVICE_MEMORY_REPORT

	void *buffer = nullptr;
	const VkDeviceSize allocationSize;
	const uint32_t memoryTypeIndex;
	uint64_t opaqueCaptureAddress = 0;
	Device *const device;

private:
	static VkResult ParseAllocationInfo(const VkMemoryAllocateInfo *pAllocateInfo, DeviceMemory::ExtendedAllocationInfo *extendedAllocationInfo);
	static VkResult Allocate(const VkAllocationCallbacks *pAllocator, const VkMemoryAllocateInfo *pAllocateInfo, VkDeviceMemory *pMemory,
	                         const vk::DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo, Device *device);
};

// This class represents a DeviceMemory object with no external memory
class DeviceMemoryInternal : public DeviceMemory, public ObjectBase<DeviceMemoryInternal, VkDeviceMemory>
{
public:
	DeviceMemoryInternal(const VkMemoryAllocateInfo *pCreateInfo, void *mem, const DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo, Device *pDevice)
	    : DeviceMemory(pCreateInfo, extendedAllocationInfo, pDevice)
	{}
};

static inline DeviceMemory *Cast(VkDeviceMemory object)
{
	return DeviceMemory::Cast(object);
}

}  // namespace vk

#endif  // VK_DEVICE_MEMORY_HPP_
