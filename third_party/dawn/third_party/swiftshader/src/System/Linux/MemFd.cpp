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

#include "MemFd.hpp"
#include "../Debug.hpp"

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#ifndef MFD_CLOEXEC
#	define MFD_CLOEXEC 0x0001U
#endif

#if __aarch64__
#	define __NR_memfd_create 279
#elif __arm__
#	define __NR_memfd_create 279
#elif __powerpc64__
#	define __NR_memfd_create 360
#elif __i386__
#	define __NR_memfd_create 356
#elif __x86_64__
#	define __NR_memfd_create 319
#endif /* __NR_memfd_create__ */

LinuxMemFd::~LinuxMemFd()
{
	close();
}

void LinuxMemFd::importFd(int fd)
{
	close();
	fd_ = fd;
}

int LinuxMemFd::exportFd() const
{
	if(fd_ < 0)
	{
		return fd_;
	}

	// Duplicate file descriptor while setting the clo-on-exec flag (Linux specific).
	return ::fcntl(fd_, F_DUPFD_CLOEXEC, 0);
}

bool LinuxMemFd::allocate(const char *name, size_t size)
{
	close();

#ifndef __NR_memfd_create
	TRACE("memfd_create() not supported on this system!");
	return false;
#else
	// In the event of no system call this returns -1 with errno set
	// as ENOSYS.
	fd_ = syscall(__NR_memfd_create, name, MFD_CLOEXEC);
	if(fd_ < 0)
	{
		TRACE("memfd_create() returned %d: %s", errno, strerror(errno));
		return false;
	}
	// Ensure there is enough space.
	if(size > 0 && ::ftruncate(fd_, size) < 0)
	{
		TRACE("ftruncate() %lld returned %d: %s", (long long)size, errno, strerror(errno));
		close();
		return false;
	}
#endif
	return true;
}

void LinuxMemFd::close()
{
	if(fd_ >= 0)
	{
		// WARNING: Never retry on close() failure, even with EINTR, see
		// https://lwn.net/Articles/576478/ for example.
		int ret = ::close(fd_);
		if(ret < 0)
		{
			TRACE("LinuxMemFd::close() failed with: %s", strerror(errno));
			assert(false);
		}
		fd_ = -1;
	}
}

void *LinuxMemFd::mapReadWrite(size_t offset, size_t size)
{
	void *addr = ::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_,
	                    static_cast<off_t>(offset));
	return (addr == MAP_FAILED) ? nullptr : addr;
}

bool LinuxMemFd::unmap(void *addr, size_t size)
{
	return ::munmap(addr, size) == 0;
}
