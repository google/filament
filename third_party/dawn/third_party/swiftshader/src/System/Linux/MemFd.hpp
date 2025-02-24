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

#ifndef MEMFD_LINUX
#define MEMFD_LINUX

#include <cstddef>

// Implementation of shared-memory regions backed by memfd_create(), which
// unfortunately is not exported by older GLibc versions, though it has been
// supported by the Linux kernel since 3.17 (good enough for Android and desktop
// Linux).

class LinuxMemFd
{
public:
	LinuxMemFd() = default;

	LinuxMemFd(const char *name, size_t size)
	    : LinuxMemFd()
	{
		allocate(name, size);
	}

	~LinuxMemFd();

	// Return true iff the region is valid/allocated.
	bool isValid() const { return fd_ >= 0; }

	// Return region's internal file descriptor value. Useful for mapping.
	int fd() const { return fd_; }

	// Set the internal handle to |fd|.
	void importFd(int fd);

	// Return a copy of this instance's file descriptor (with CLO_EXEC set).
	int exportFd() const;

	// Implement memfd_create() through direct syscalls if possible.
	// On success, return true and sets |fd| accordingly. On failure, return
	// false and sets errno.
	bool allocate(const char *name, size_t size);

	// Map a segment of |size| bytes from |offset| from the region.
	// Both |offset| and |size| should be page-aligned. Returns nullptr/errno
	// on failure.
	void *mapReadWrite(size_t offset, size_t size);

	// Unmap a region segment starting at |addr| of |size| bytes.
	// Both |addr| and |size| should be page-aligned. Returns true on success
	// or false/errno on failure.
	bool unmap(void *addr, size_t size);

	void close();

private:
	int fd_ = -1;
};

#endif  // MEMFD_LINUX
