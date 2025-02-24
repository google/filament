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

#include "VkDeviceMemory.hpp"
#include "VkDeviceMemoryExternalHost.hpp"

#include "VkBuffer.hpp"
#include "VkConfig.hpp"
#include "VkDevice.hpp"
#include "VkImage.hpp"
#include "VkMemory.hpp"
#include "VkStringify.hpp"

#if SWIFTSHADER_EXTERNAL_MEMORY_OPAQUE_FD

// Helper struct which reads the parsed allocation info and
// extracts relevant information related to the handle type
// supported by this DeviceMemory subclass.
struct OpaqueFdAllocateInfo
{
	bool importFd = false;
	bool exportFd = false;
	int fd = -1;

	OpaqueFdAllocateInfo() = default;

	// Read the parsed allocation info to initialize an OpaqueFdAllocateInfo.
	OpaqueFdAllocateInfo(const vk::DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo)
	{
		if(extendedAllocationInfo.importMemoryFdInfo)
		{
			if(extendedAllocationInfo.importMemoryFdInfo->handleType != VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT)
			{
				UNSUPPORTED("VkImportMemoryFdInfoKHR::handleType %d", int(extendedAllocationInfo.importMemoryFdInfo->handleType));
			}
			importFd = true;
			fd = extendedAllocationInfo.importMemoryFdInfo->fd;
		}

		if(extendedAllocationInfo.exportMemoryAllocateInfo)
		{
			if(extendedAllocationInfo.exportMemoryAllocateInfo->handleTypes != VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT)
			{
				UNSUPPORTED("VkExportMemoryAllocateInfo::handleTypes %d", int(extendedAllocationInfo.exportMemoryAllocateInfo->handleTypes));
			}
			exportFd = true;
		}
	}
};

#	if defined(__APPLE__)
#		include "VkDeviceMemoryExternalMac.hpp"
#	elif defined(__linux__) && !defined(__ANDROID__)
#		include "VkDeviceMemoryExternalLinux.hpp"
#	else
#		error "Missing VK_KHR_external_memory_fd implementation for this platform!"
#	endif
#endif

#if SWIFTSHADER_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER
#	if defined(__ANDROID__)
#		include "VkDeviceMemoryExternalAndroid.hpp"
#	else
#		error "Missing VK_ANDROID_external_memory_android_hardware_buffer implementation for this platform!"
#	endif
#endif

#if VK_USE_PLATFORM_FUCHSIA
#	include "VkDeviceMemoryExternalFuchsia.hpp"
#endif

namespace vk {

VkResult DeviceMemory::Allocate(const VkAllocationCallbacks *pAllocator, const VkMemoryAllocateInfo *pAllocateInfo, VkDeviceMemory *pMemory, Device *device)
{
	*pMemory = VK_NULL_HANDLE;

	vk::DeviceMemory::ExtendedAllocationInfo extendedAllocationInfo = {};
	VkResult result = vk::DeviceMemory::ParseAllocationInfo(pAllocateInfo, &extendedAllocationInfo);
	if(result != VK_SUCCESS)
	{
		return result;
	}

	result = Allocate(pAllocator, pAllocateInfo, pMemory, extendedAllocationInfo, device);
	if(result != VK_SUCCESS)
	{
		return result;
	}

	// Make sure the memory allocation is done now so that OOM errors can be checked now
	return vk::Cast(*pMemory)->allocate();
}

VkResult DeviceMemory::Allocate(const VkAllocationCallbacks *pAllocator, const VkMemoryAllocateInfo *pAllocateInfo, VkDeviceMemory *pMemory,
                                const vk::DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo, Device *device)
{
	VkMemoryAllocateInfo allocateInfo = *pAllocateInfo;
	// Add 15 bytes of padding to ensure that any type of attribute within
	// buffers and images can be read using 16-byte accesses.
	if(allocateInfo.allocationSize > UINT64_MAX - 15)
	{
		return VK_ERROR_OUT_OF_DEVICE_MEMORY;
	}
	allocateInfo.allocationSize += 15;

#if SWIFTSHADER_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER
	if(AHardwareBufferExternalMemory::SupportsAllocateInfo(extendedAllocationInfo))
	{
		return AHardwareBufferExternalMemory::Create(pAllocator, &allocateInfo, pMemory, extendedAllocationInfo, device);
	}
#endif
#if SWIFTSHADER_EXTERNAL_MEMORY_OPAQUE_FD
	if(OpaqueFdExternalMemory::SupportsAllocateInfo(extendedAllocationInfo))
	{
		return OpaqueFdExternalMemory::Create(pAllocator, &allocateInfo, pMemory, extendedAllocationInfo, device);
	}
#endif
#if VK_USE_PLATFORM_FUCHSIA
	if(zircon::VmoExternalMemory::supportsAllocateInfo(extendedAllocationInfo))
	{
		return zircon::VmoExternalMemory::Create(pAllocator, &allocateInfo, pMemory, extendedAllocationInfo, device);
	}
#endif
	if(ExternalMemoryHost::SupportsAllocateInfo(extendedAllocationInfo))
	{
		return ExternalMemoryHost::Create(pAllocator, &allocateInfo, pMemory, extendedAllocationInfo, device);
	}

	return vk::DeviceMemoryInternal::Create(pAllocator, &allocateInfo, pMemory, extendedAllocationInfo, device);
}

DeviceMemory::DeviceMemory(const VkMemoryAllocateInfo *pAllocateInfo, const DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo, Device *pDevice)
    : allocationSize(pAllocateInfo->allocationSize)
    , memoryTypeIndex(pAllocateInfo->memoryTypeIndex)
    , opaqueCaptureAddress(extendedAllocationInfo.opaqueCaptureAddress)
    , device(pDevice)
{
	ASSERT(allocationSize);
}

void DeviceMemory::destroy(const VkAllocationCallbacks *pAllocator)
{
#ifdef SWIFTSHADER_DEVICE_MEMORY_REPORT
	VkDeviceMemoryReportEventTypeEXT eventType = isImport() ? VK_DEVICE_MEMORY_REPORT_EVENT_TYPE_UNIMPORT_EXT : VK_DEVICE_MEMORY_REPORT_EVENT_TYPE_FREE_EXT;
	device->emitDeviceMemoryReport(eventType, getMemoryObjectId(), 0 /* size */, VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)(void *)VkDeviceMemory(*this));
#endif  // SWIFTSHADER_DEVICE_MEMORY_REPORT

	if(buffer)
	{
		freeBuffer();
		buffer = nullptr;
	}
}

size_t DeviceMemory::ComputeRequiredAllocationSize(const VkMemoryAllocateInfo *pAllocateInfo)
{
	return 0;
}

VkResult DeviceMemory::ParseAllocationInfo(const VkMemoryAllocateInfo *pAllocateInfo, DeviceMemory::ExtendedAllocationInfo *extendedAllocationInfo)
{
	const VkBaseInStructure *allocationInfo = reinterpret_cast<const VkBaseInStructure *>(pAllocateInfo->pNext);
	while(allocationInfo)
	{
		switch(allocationInfo->sType)
		{
		case VK_STRUCTURE_TYPE_MEMORY_DEDICATED_ALLOCATE_INFO:
			// This can safely be ignored on most platforms, as the Vulkan spec mentions:
			// "If the pNext chain includes a VkMemoryDedicatedAllocateInfo structure, then that structure
			//  includes a handle of the sole buffer or image resource that the memory *can* be bound to."
#if SWIFTSHADER_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER
			extendedAllocationInfo->dedicatedAllocateInfo = reinterpret_cast<const VkMemoryDedicatedAllocateInfo *>(allocationInfo);
#endif
			break;
		case VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO:
			// This extension controls on which physical devices the memory gets allocated.
			// SwiftShader only has a single physical device, so this extension does nothing in this case.
			break;
#if SWIFTSHADER_EXTERNAL_MEMORY_OPAQUE_FD
		case VK_STRUCTURE_TYPE_IMPORT_MEMORY_FD_INFO_KHR:
			extendedAllocationInfo->importMemoryFdInfo = reinterpret_cast<const VkImportMemoryFdInfoKHR *>(allocationInfo);
			if(extendedAllocationInfo->importMemoryFdInfo->handleType != VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT)
			{
				UNSUPPORTED("extendedAllocationInfo->importMemoryFdInfo->handleType %u", extendedAllocationInfo->importMemoryFdInfo->handleType);
				return VK_ERROR_INVALID_EXTERNAL_HANDLE;
			}
			break;
#endif  // SWIFTSHADER_EXTERNAL_MEMORY_OPAQUE_FD
		case VK_STRUCTURE_TYPE_EXPORT_MEMORY_ALLOCATE_INFO:
			extendedAllocationInfo->exportMemoryAllocateInfo = reinterpret_cast<const VkExportMemoryAllocateInfo *>(allocationInfo);
			switch(extendedAllocationInfo->exportMemoryAllocateInfo->handleTypes)
			{
#if SWIFTSHADER_EXTERNAL_MEMORY_OPAQUE_FD
			case VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT:
				break;
#endif
#if SWIFTSHADER_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER
			case VK_EXTERNAL_MEMORY_HANDLE_TYPE_ANDROID_HARDWARE_BUFFER_BIT_ANDROID:
				break;
#endif
#if VK_USE_PLATFORM_FUCHSIA
			case VK_EXTERNAL_MEMORY_HANDLE_TYPE_ZIRCON_VMO_BIT_FUCHSIA:
				break;
#endif
			default:
				UNSUPPORTED("extendedAllocationInfo->exportMemoryAllocateInfo->handleTypes %u", extendedAllocationInfo->exportMemoryAllocateInfo->handleTypes);
				return VK_ERROR_INVALID_EXTERNAL_HANDLE;
			}
			break;
#if SWIFTSHADER_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER
		case VK_STRUCTURE_TYPE_IMPORT_ANDROID_HARDWARE_BUFFER_INFO_ANDROID:
			extendedAllocationInfo->importAndroidHardwareBufferInfo = reinterpret_cast<const VkImportAndroidHardwareBufferInfoANDROID *>(allocationInfo);
			break;
#endif  // SWIFTSHADER_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER
		case VK_STRUCTURE_TYPE_IMPORT_MEMORY_HOST_POINTER_INFO_EXT:
			extendedAllocationInfo->importMemoryHostPointerInfo = reinterpret_cast<const VkImportMemoryHostPointerInfoEXT *>(allocationInfo);
			if((extendedAllocationInfo->importMemoryHostPointerInfo->handleType != VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_ALLOCATION_BIT_EXT) &&
			   (extendedAllocationInfo->importMemoryHostPointerInfo->handleType != VK_EXTERNAL_MEMORY_HANDLE_TYPE_HOST_MAPPED_FOREIGN_MEMORY_BIT_EXT))
			{
				UNSUPPORTED("extendedAllocationInfo->importMemoryHostPointerInfo->handleType %u", extendedAllocationInfo->importMemoryHostPointerInfo->handleType);
				return VK_ERROR_INVALID_EXTERNAL_HANDLE;
			}
			break;
#if VK_USE_PLATFORM_FUCHSIA
		case VK_STRUCTURE_TYPE_IMPORT_MEMORY_ZIRCON_HANDLE_INFO_FUCHSIA:
			extendedAllocationInfo->importMemoryZirconHandleInfo = reinterpret_cast<const VkImportMemoryZirconHandleInfoFUCHSIA *>(allocationInfo);
			if(extendedAllocationInfo->importMemoryZirconHandleInfo->handleType != VK_EXTERNAL_MEMORY_HANDLE_TYPE_ZIRCON_VMO_BIT_FUCHSIA)
			{
				UNSUPPORTED("extendedAllocationInfo->importMemoryZirconHandleInfo->handleType %u", extendedAllocationInfo->importMemoryZirconHandleInfo->handleType);
				return VK_ERROR_INVALID_EXTERNAL_HANDLE;
			}
			break;
#endif  // VK_USE_PLATFORM_FUCHSIA
		case VK_STRUCTURE_TYPE_MEMORY_OPAQUE_CAPTURE_ADDRESS_ALLOCATE_INFO:
			extendedAllocationInfo->opaqueCaptureAddress =
			    reinterpret_cast<const VkMemoryOpaqueCaptureAddressAllocateInfo *>(allocationInfo)->opaqueCaptureAddress;
			break;
		default:
			UNSUPPORTED("pAllocateInfo->pNext sType = %s", vk::Stringify(allocationInfo->sType).c_str());
			break;
		}

		allocationInfo = allocationInfo->pNext;
	}

	return VK_SUCCESS;
}

VkResult DeviceMemory::allocate()
{
	if(allocationSize > MAX_MEMORY_ALLOCATION_SIZE)
	{
#ifdef SWIFTSHADER_DEVICE_MEMORY_REPORT
		device->emitDeviceMemoryReport(VK_DEVICE_MEMORY_REPORT_EVENT_TYPE_ALLOCATION_FAILED_EXT, 0 /* memoryObjectId */, allocationSize, VK_OBJECT_TYPE_DEVICE_MEMORY, 0 /* objectHandle */);
#endif  // SWIFTSHADER_DEVICE_MEMORY_REPORT

		return VK_ERROR_OUT_OF_DEVICE_MEMORY;
	}

	VkResult result = VK_SUCCESS;
	if(!buffer)
	{
		result = allocateBuffer();
	}

#ifdef SWIFTSHADER_DEVICE_MEMORY_REPORT
	if(result == VK_SUCCESS)
	{
		VkDeviceMemoryReportEventTypeEXT eventType = isImport() ? VK_DEVICE_MEMORY_REPORT_EVENT_TYPE_IMPORT_EXT : VK_DEVICE_MEMORY_REPORT_EVENT_TYPE_ALLOCATE_EXT;
		device->emitDeviceMemoryReport(eventType, getMemoryObjectId(), allocationSize, VK_OBJECT_TYPE_DEVICE_MEMORY, (uint64_t)(void *)VkDeviceMemory(*this));
	}
	else
	{
		device->emitDeviceMemoryReport(VK_DEVICE_MEMORY_REPORT_EVENT_TYPE_ALLOCATION_FAILED_EXT, 0 /* memoryObjectId */, allocationSize, VK_OBJECT_TYPE_DEVICE_MEMORY, 0 /* objectHandle */);
	}
#endif  // SWIFTSHADER_DEVICE_MEMORY_REPORT

	return result;
}

VkResult DeviceMemory::map(VkDeviceSize pOffset, VkDeviceSize pSize, void **ppData)
{
	*ppData = getOffsetPointer(pOffset);

	return VK_SUCCESS;
}

VkDeviceSize DeviceMemory::getCommittedMemoryInBytes() const
{
	return allocationSize;
}

void *DeviceMemory::getOffsetPointer(VkDeviceSize pOffset) const
{
	ASSERT(buffer);
	return reinterpret_cast<char *>(buffer) + pOffset;
}

uint64_t DeviceMemory::getOpaqueCaptureAddress() const
{
	return (opaqueCaptureAddress != 0) ? opaqueCaptureAddress : static_cast<uint64_t>(reinterpret_cast<uintptr_t>(buffer));
}

bool DeviceMemory::checkExternalMemoryHandleType(
    VkExternalMemoryHandleTypeFlags supportedHandleTypes) const
{
	if(!supportedHandleTypes)
	{
		// This image or buffer does not need to be stored on external
		// memory, so this check should always pass.
		return true;
	}
	VkExternalMemoryHandleTypeFlagBits handle_type_bit = getFlagBit();
	if(!handle_type_bit)
	{
		// This device memory is not external and can accommodate
		// any image or buffer as well.
		return true;
	}
	// Return true only if the external memory type is compatible with the
	// one specified during VkCreate{Image,Buffer}(), through a
	// VkExternalMemory{Image,Buffer}AllocateInfo struct.
	return (supportedHandleTypes & handle_type_bit) != 0;
}

// Allocate the memory according to `allocationSize`. On success return VK_SUCCESS
// and sets `buffer`.
VkResult DeviceMemory::allocateBuffer()
{
	buffer = vk::allocateDeviceMemory(allocationSize, vk::DEVICE_MEMORY_ALLOCATION_ALIGNMENT);
	if(!buffer)
	{
		return VK_ERROR_OUT_OF_DEVICE_MEMORY;
	}

	return VK_SUCCESS;
}

// Free previously allocated memory at `buffer`.
void DeviceMemory::freeBuffer()
{
	vk::freeDeviceMemory(buffer);
	buffer = nullptr;
}

// Return the handle type flag bit supported by this implementation.
// A value of 0 corresponds to non-external memory.
VkExternalMemoryHandleTypeFlagBits DeviceMemory::getFlagBit() const
{
	// Does not support any external memory type at all.
	static const VkExternalMemoryHandleTypeFlagBits typeFlagBit = (VkExternalMemoryHandleTypeFlagBits)0;
	return typeFlagBit;
}

#if SWIFTSHADER_EXTERNAL_MEMORY_OPAQUE_FD
VkResult DeviceMemory::exportFd(int *pFd) const
{
	return VK_ERROR_INVALID_EXTERNAL_HANDLE;
}
#endif

#if SWIFTSHADER_EXTERNAL_MEMORY_ANDROID_HARDWARE_BUFFER
VkResult DeviceMemory::exportAndroidHardwareBuffer(struct AHardwareBuffer **pAhb) const
{
	return VK_ERROR_OUT_OF_HOST_MEMORY;
}

VkResult DeviceMemory::GetAndroidHardwareBufferProperties(VkDevice &ahbDevice, const struct AHardwareBuffer *buffer, VkAndroidHardwareBufferPropertiesANDROID *pProperties)
{
	return AHardwareBufferExternalMemory::GetAndroidHardwareBufferProperties(ahbDevice, buffer, pProperties);
}
#endif

#if VK_USE_PLATFORM_FUCHSIA
VkResult DeviceMemory::exportHandle(zx_handle_t *pHandle) const
{
	return VK_ERROR_INVALID_EXTERNAL_HANDLE;
}
#endif

}  // namespace vk
