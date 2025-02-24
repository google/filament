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

#include "System/Debug.hpp"
#include "System/Linux/MemFd.hpp"

#include <errno.h>
#include <string.h>
#include <sys/mman.h>

class OpaqueFdExternalMemory : public vk::DeviceMemory, public vk::ObjectBase<OpaqueFdExternalMemory, VkDeviceMemory>
{
public:
	static const VkExternalMemoryHandleTypeFlagBits typeFlagBit = VK_EXTERNAL_MEMORY_HANDLE_TYPE_OPAQUE_FD_BIT;

	static bool SupportsAllocateInfo(const vk::DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo)
	{
		OpaqueFdAllocateInfo info(extendedAllocationInfo);
		return info.importFd || info.exportFd;
	}

	explicit OpaqueFdExternalMemory(const VkMemoryAllocateInfo *pCreateInfo, void *mem, const vk::DeviceMemory::ExtendedAllocationInfo &extendedAllocationInfo, vk::Device *pDevice)
	    : vk::DeviceMemory(pCreateInfo, extendedAllocationInfo, pDevice)
	    , allocateInfo(extendedAllocationInfo)
	{
	}

	~OpaqueFdExternalMemory()
	{
		memfd.close();
	}

	VkResult allocateBuffer() override
	{
		if(allocateInfo.importFd)
		{
			memfd.importFd(allocateInfo.fd);
			if(!memfd.isValid())
			{
				return VK_ERROR_INVALID_EXTERNAL_HANDLE;
			}
		}
		else
		{
			ASSERT(allocateInfo.exportFd);
			static int counter = 0;
			char name[40];
			snprintf(name, sizeof(name), "SwiftShader.Memory.%d", ++counter);
			if(!memfd.allocate(name, allocationSize))
			{
				TRACE("memfd.allocate() returned %s", strerror(errno));
				return VK_ERROR_OUT_OF_DEVICE_MEMORY;
			}
		}
		void *addr = memfd.mapReadWrite(0, allocationSize);
		if(!addr)
		{
			return VK_ERROR_MEMORY_MAP_FAILED;
		}
		buffer = addr;
		return VK_SUCCESS;
	}

	void freeBuffer() override
	{
		memfd.unmap(buffer, allocationSize);
	}

	VkExternalMemoryHandleTypeFlagBits getFlagBit() const override
	{
		return typeFlagBit;
	}

	VkResult exportFd(int *pFd) const override
	{
		int fd = memfd.exportFd();
		if(fd < 0)
		{
			return VK_ERROR_INVALID_EXTERNAL_HANDLE;
		}
		*pFd = fd;
		return VK_SUCCESS;
	}

private:
	LinuxMemFd memfd;
	OpaqueFdAllocateInfo allocateInfo;
};
