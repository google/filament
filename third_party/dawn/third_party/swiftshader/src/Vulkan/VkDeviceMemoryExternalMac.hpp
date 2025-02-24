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

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#ifndef __APPLE__
#	error "This file is for macOS only!"
#endif  // __APPLE__

#if __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_12
#	include <mach/mach_time.h>
#endif  // __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_12

namespace {

struct timespec GetTime()
{
	struct timespec tv;

#if __MAC_OS_X_VERSION_MIN_REQUIRED >= __MAC_10_12
	clock_gettime(CLOCK_REALTIME, &tv);
#else
	mach_timebase_info_data_t timebase;
	mach_timebase_info(&timebase);
	uint64_t time;
	time = mach_absolute_time();

	double convert_ratio = (double)timebase.numer / (double)timebase.denom;
	uint64_t secs = (uint64_t)((double)time * convert_ratio / 1e-9);
	uint64_t usecs = (uint64_t)((double)time * convert_ratio - secs * 1e9);
	tv.tv_sec = secs;
	tv.tv_nsec = usecs;
#endif
	return tv;
}

}  // namespace

// An implementation of OpaqueFdExternalMemory that relies on shm_open().
// Useful on OS X which do not have Linux memfd regions.
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
		if(shm_fd_ >= 0)
		{
			::close(shm_fd_);
			shm_fd_ = -1;
		}
	}

	VkResult allocateBuffer() override
	{
		if(allocateInfo.importFd)
		{
			shm_fd_ = allocateInfo.fd;
			if(shm_fd_ < 0)
			{
				return VK_ERROR_INVALID_EXTERNAL_HANDLE;
			}
		}
		else
		{
			ASSERT(allocateInfo.exportFd);
			// Create shared memory region with shm_open() and a randomly-generated region name.
			static const char kPrefix[] = "/SwiftShader-";
			const size_t kPrefixSize = sizeof(kPrefix) - 1;
			const size_t kRandomSize = 8;

			char name[kPrefixSize + kRandomSize + 1u];
			memcpy(name, kPrefix, kPrefixSize);

			int fd = -1;
			for(int tries = 0; tries < 6; ++tries)
			{
				struct timespec tv = GetTime();
				uint64_t r = (uint64_t)tv.tv_sec + (uint64_t)tv.tv_nsec;
				for(size_t pos = 0; pos < kRandomSize; ++pos, r /= 8)
				{
					name[kPrefixSize + pos] = '0' + (r % 8);
				}
				name[kPrefixSize + kRandomSize] = '\0';

				fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL | O_NOFOLLOW, 0600);
				if(fd >= 0)
					break;

				if(errno != EEXIST)
				{
					TRACE("shm_open() failed with: %s", strerror(errno));
					break;
				}
			}

			// Unlink the name since it's not needed anymore.
			if(fd >= 0)
			{
				if(shm_unlink(name) == -1)
				{
					TRACE("shm_unlink() failed with: %s", strerror(errno));
					close(fd);
					fd = -1;
				}
			}

			// Ensure there is enough space.
			if(fd >= 0 && allocationSize > 0)
			{
				if(::ftruncate(fd, allocationSize) < 0)
				{
					TRACE("ftruncate() failed with: %s", strerror(errno));
					close(fd);
					fd = -1;
				}
			}

			if(fd < 0)
			{
				TRACE("Could not allocate shared memory region");
				return VK_ERROR_OUT_OF_DEVICE_MEMORY;
			}

			shm_fd_ = fd;
		}

		void *addr = ::mmap(nullptr, allocationSize, PROT_READ | PROT_WRITE, MAP_SHARED,
		                    shm_fd_, 0);

		if(addr == MAP_FAILED)
		{
			return VK_ERROR_MEMORY_MAP_FAILED;
		}
		buffer = addr;
		return VK_SUCCESS;
	}

	void freeBuffer() override
	{
		::munmap(buffer, allocationSize);
	}

	VkExternalMemoryHandleTypeFlagBits getFlagBit() const override
	{
		return typeFlagBit;
	}

	VkResult exportFd(int *pFd) const override
	{
		int fd = dup(shm_fd_);
		if(fd < 0)
		{
			return VK_ERROR_INVALID_EXTERNAL_HANDLE;
		}

		// Set the clo-on-exec flag.
		int flags = ::fcntl(fd, F_GETFD);
		::fcntl(fd, F_SETFL, flags | FD_CLOEXEC);

		*pFd = fd;
		return VK_SUCCESS;
	}

private:
	int shm_fd_ = -1;
	OpaqueFdAllocateInfo allocateInfo;
};
