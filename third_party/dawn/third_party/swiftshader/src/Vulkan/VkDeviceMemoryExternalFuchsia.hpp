// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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
#include "VkStringify.hpp"

#include "System/Debug.hpp"

#include <zircon/process.h>
#include <zircon/syscalls.h>

namespace zircon {

class VmoExternalMemory : public vk::DeviceMemory, public vk::ObjectBase<VmoExternalMemory, VkDeviceMemory>
{
public:
	// Helper struct which reads the parsed allocation info and
	// extracts relevant information related to the handle type
	// supported by this DeviceMemory subclass.
	struct AllocateInfo
	{
		bool importHandle = false;
		bool exportHandle = false;
		zx_handle_t handle = ZX_HANDLE_INVALID;

		AllocateInfo() = default;

		// Use the parsed allocation info to initialize a AllocateInfo.
		AllocateInfo(const vk::DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo)
		{
			if(extendedAllocationInfo.importMemoryZirconHandleInfo)
			{
				if(extendedAllocationInfo.importMemoryZirconHandleInfo->handleType != VK_EXTERNAL_MEMORY_HANDLE_TYPE_ZIRCON_VMO_BIT_FUCHSIA)
				{
					UNSUPPORTED("extendedAllocationInfo.importMemoryZirconHandleInfo->handleType");
				}
				importHandle = true;
				handle = extendedAllocationInfo.importMemoryZirconHandleInfo->handle;
			}

			if(extendedAllocationInfo.exportMemoryAllocateInfo)
			{
				if(extendedAllocationInfo.exportMemoryAllocateInfo->handleTypes != VK_EXTERNAL_MEMORY_HANDLE_TYPE_ZIRCON_VMO_BIT_FUCHSIA)
				{
					UNSUPPORTED("extendedAllocationInfo.exportMemoryAllocateInfo->handleTypes");
				}
				exportHandle = true;
			}
		}
	};

	static const VkExternalMemoryHandleTypeFlagBits typeFlagBit = VK_EXTERNAL_MEMORY_HANDLE_TYPE_ZIRCON_VMO_BIT_FUCHSIA;

	static bool supportsAllocateInfo(const vk::DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo)
	{
		AllocateInfo info(extendedAllocationInfo);
		return info.importHandle || info.exportHandle;
	}

	explicit VmoExternalMemory(const VkMemoryAllocateInfo *pCreateInfo, void *mem, const vk::DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo, vk::Device *pDevice)
	    : vk::DeviceMemory(pCreateInfo, extendedAllocationInfo, pDevice)
	    , allocateInfo(extendedAllocationInfo)
	{
	}

	~VmoExternalMemory()
	{
		closeVmo();
	}

	VkResult allocateBuffer() override
	{
		if(allocateInfo.importHandle)
		{
			// NOTE: handle ownership is passed to the VkDeviceMemory.
			vmoHandle = allocateInfo.handle;
		}
		else
		{
			ASSERT(allocateInfo.exportHandle);
			zx_status_t status = zx_vmo_create(allocationSize, 0, &vmoHandle);
			if(status != ZX_OK)
			{
				TRACE("zx_vmo_create() returned %d", status);
				return VK_ERROR_OUT_OF_DEVICE_MEMORY;
			}
		}

		// Now map it directly.
		zx_vaddr_t addr = 0;
		zx_status_t status = zx_vmar_map(zx_vmar_root_self(),
		                                 ZX_VM_PERM_READ | ZX_VM_PERM_WRITE,
		                                 0,  // vmar_offset
		                                 vmoHandle,
		                                 0,  // vmo_offset
		                                 allocationSize,
		                                 &addr);
		if(status != ZX_OK)
		{
			TRACE("zx_vmar_map() failed with %d", status);
			return VK_ERROR_MEMORY_MAP_FAILED;
		}
		buffer = reinterpret_cast<void *>(addr);
		return VK_SUCCESS;
	}

	void freeBuffer() override
	{
		zx_status_t status = zx_vmar_unmap(zx_vmar_root_self(),
		                                   reinterpret_cast<zx_vaddr_t>(buffer),
		                                   allocationSize);
		if(status != ZX_OK)
		{
			TRACE("zx_vmar_unmap() failed with %d", status);
		}
		closeVmo();
	}

	VkExternalMemoryHandleTypeFlagBits getFlagBit() const override
	{
		return typeFlagBit;
	}

	VkResult exportHandle(zx_handle_t *pHandle) const override
	{
		if(vmoHandle == ZX_HANDLE_INVALID)
		{
			return VK_ERROR_INVALID_EXTERNAL_HANDLE;
		}
		zx_status_t status = zx_handle_duplicate(vmoHandle, ZX_RIGHT_SAME_RIGHTS, pHandle);
		if(status != ZX_OK)
		{
			TRACE("zx_handle_duplicate() returned %d", status);
			return VK_ERROR_INVALID_EXTERNAL_HANDLE;
		}
		return VK_SUCCESS;
	}

private:
	void closeVmo()
	{
		if(vmoHandle != ZX_HANDLE_INVALID)
		{
			zx_handle_close(vmoHandle);
			vmoHandle = ZX_HANDLE_INVALID;
		}
	}

	zx_handle_t vmoHandle = ZX_HANDLE_INVALID;
	AllocateInfo allocateInfo;
};

}  // namespace zircon
