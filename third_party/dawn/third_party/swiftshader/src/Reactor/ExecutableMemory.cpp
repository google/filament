// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
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

#include "ExecutableMemory.hpp"

#include "Debug.hpp"

#if defined(_WIN32)
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <Windows.h>
#	include <intrin.h>
#elif defined(__Fuchsia__)
#	include <unistd.h>
#	include <zircon/process.h>
#	include <zircon/syscalls.h>
#else
#	include <errno.h>
#	include <sys/mman.h>
#	include <stdlib.h>
#	include <unistd.h>
#endif

#if defined(__ANDROID__) && !defined(ANDROID_HOST_BUILD) && !defined(ANDROID_NDK_BUILD)
#	include <sys/prctl.h>
#endif

#include <memory.h>

#undef allocate
#undef deallocate

#if(defined(__i386__) || defined(_M_IX86) || defined(__x86_64__) || defined(_M_X64)) && !defined(__x86__)
#	define __x86__
#endif

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

// A Clang extension to determine compiler features.
// We use it to detect Sanitizer builds (e.g. -fsanitize=memory).
#ifndef __has_feature
#	define __has_feature(x) 0
#endif

namespace rr {
namespace {

struct Allocation
{
	// size_t bytes;
	unsigned char *block;
};

void *allocateRaw(size_t bytes, size_t alignment)
{
	ASSERT((alignment & (alignment - 1)) == 0);  // Power of 2 alignment.

#if defined(__linux__) && defined(REACTOR_ANONYMOUS_MMAP_NAME)
	if(alignment < sizeof(void *))
	{
		return malloc(bytes);
	}
	else
	{
		void *allocation;
		int result = posix_memalign(&allocation, alignment, bytes);
		if(result != 0)
		{
			errno = result;
			allocation = nullptr;
		}
		return allocation;
	}
#else
	unsigned char *block = new unsigned char[bytes + sizeof(Allocation) + alignment];
	unsigned char *aligned = nullptr;

	if(block)
	{
		aligned = (unsigned char *)((uintptr_t)(block + sizeof(Allocation) + alignment - 1) & -(intptr_t)alignment);
		Allocation *allocation = (Allocation *)(aligned - sizeof(Allocation));

		// allocation->bytes = bytes;
		allocation->block = block;
	}

	return aligned;
#endif
}

#if defined(_WIN32)
DWORD permissionsToProtectMode(int permissions)
{
	switch(permissions)
	{
	case PERMISSION_READ:
		return PAGE_READONLY;
	case PERMISSION_EXECUTE:
		return PAGE_EXECUTE;
	case PERMISSION_READ | PERMISSION_WRITE:
		return PAGE_READWRITE;
	case PERMISSION_READ | PERMISSION_EXECUTE:
		return PAGE_EXECUTE_READ;
	case PERMISSION_READ | PERMISSION_WRITE | PERMISSION_EXECUTE:
		return PAGE_EXECUTE_READWRITE;
	}
	return PAGE_NOACCESS;
}
#endif

#if !defined(_WIN32) && !defined(__Fuchsia__)
int permissionsToMmapProt(int permissions)
{
	int result = 0;
	if(permissions & PERMISSION_READ)
	{
		result |= PROT_READ;
	}
	if(permissions & PERMISSION_WRITE)
	{
		result |= PROT_WRITE;
	}
	if(permissions & PERMISSION_EXECUTE)
	{
		result |= PROT_EXEC;
	}
	return result;
}
#endif  // !defined(_WIN32) && !defined(__Fuchsia__)

#if defined(__linux__) && defined(REACTOR_ANONYMOUS_MMAP_NAME)
#	if !defined(__ANDROID__) || defined(ANDROID_HOST_BUILD) || defined(ANDROID_NDK_BUILD)
// Create a file descriptor for anonymous memory with the given
// name. Returns -1 on failure.
// TODO: remove once libc wrapper exists.
static int memfd_create(const char *name, unsigned int flags)
{
#		if __aarch64__
#			define __NR_memfd_create 279
#		elif __arm__
#			define __NR_memfd_create 279
#		elif __powerpc64__
#			define __NR_memfd_create 360
#		elif __i386__
#			define __NR_memfd_create 356
#		elif __x86_64__
#			define __NR_memfd_create 319
#		endif /* __NR_memfd_create__ */
#		ifdef __NR_memfd_create
	// In the event of no system call this returns -1 with errno set
	// as ENOSYS.
	return syscall(__NR_memfd_create, name, flags);
#		else
	return -1;
#		endif
}

// Returns a file descriptor for use with an anonymous mmap, if
// memfd_create fails, -1 is returned. Note, the mappings should be
// MAP_PRIVATE so that underlying pages aren't shared.
int anonymousFd()
{
	static int fd = memfd_create(MACRO_STRINGIFY(REACTOR_ANONYMOUS_MMAP_NAME), 0);
	return fd;
}
#	else   // __ANDROID__ && !ANDROID_HOST_BUILD && !ANDROID_NDK_BUILD
int anonymousFd()
{
	return -1;
}
#	endif  // __ANDROID__ && !ANDROID_HOST_BUILD && !ANDROID_NDK_BUILD

// Ensure there is enough space in the "anonymous" fd for length.
void ensureAnonFileSize(int anonFd, size_t length)
{
	static size_t fileSize = 0;
	if(length > fileSize)
	{
		[[maybe_unused]] int result = ftruncate(anonFd, length);
		ASSERT(result == 0);
		fileSize = length;
	}
}
#endif  // defined(__linux__) && defined(REACTOR_ANONYMOUS_MMAP_NAME)

#if defined(__Fuchsia__)
zx_vm_option_t permissionsToZxVmOptions(int permissions)
{
	zx_vm_option_t result = 0;
	if(permissions & PERMISSION_READ)
	{
		result |= ZX_VM_PERM_READ;
	}
	if(permissions & PERMISSION_WRITE)
	{
		result |= ZX_VM_PERM_WRITE;
	}
	if(permissions & PERMISSION_EXECUTE)
	{
		result |= ZX_VM_PERM_EXECUTE;
	}
	return result;
}
#endif  // defined(__Fuchsia__)

}  // anonymous namespace

size_t memoryPageSize()
{
	static int pageSize = [] {
#if defined(_WIN32)
		SYSTEM_INFO systemInfo;
		GetSystemInfo(&systemInfo);
		return systemInfo.dwPageSize;
#else
		return sysconf(_SC_PAGESIZE);
#endif
	}();

	return pageSize;
}

void *allocate(size_t bytes, size_t alignment)
{
	void *memory = allocateRaw(bytes, alignment);

	// Zero-initialize the memory, for security reasons.
	// MemorySanitizer builds skip this so that we can detect when we
	// inadvertently rely on this, which would indicate a bug.
	if(memory && !__has_feature(memory_sanitizer))
	{
		memset(memory, 0, bytes);
	}

	return memory;
}

void deallocate(void *memory)
{
#if defined(__linux__) && defined(REACTOR_ANONYMOUS_MMAP_NAME)
	free(memory);
#else
	if(memory)
	{
		unsigned char *aligned = (unsigned char *)memory;
		Allocation *allocation = (Allocation *)(aligned - sizeof(Allocation));

		delete[] allocation->block;
	}
#endif
}

// Rounds |x| up to a multiple of |m|, where |m| is a power of 2.
inline uintptr_t roundUp(uintptr_t x, uintptr_t m)
{
	ASSERT(m > 0 && (m & (m - 1)) == 0);  // |m| must be a power of 2.
	return (x + m - 1) & ~(m - 1);
}

void *allocateMemoryPages(size_t bytes, int permissions, bool need_exec)
{
	size_t pageSize = memoryPageSize();
	size_t length = roundUp(bytes, pageSize);
	void *mapping = nullptr;

#if defined(_WIN32)
	return VirtualAlloc(nullptr, length, MEM_COMMIT | MEM_RESERVE,
	                    permissionsToProtectMode(permissions));
#elif defined(__linux__) && defined(REACTOR_ANONYMOUS_MMAP_NAME)
	int flags = MAP_PRIVATE;

	// Try to name the memory region for the executable code,
	// to aid profilers.
	int anonFd = anonymousFd();
	if(anonFd == -1)
	{
		flags |= MAP_ANONYMOUS;
	}
	else
	{
		ensureAnonFileSize(anonFd, length);
	}

	mapping = mmap(
	    nullptr, length, permissionsToMmapProt(permissions), flags, anonFd, 0);

	if(mapping == MAP_FAILED)
	{
		mapping = nullptr;
	}
#	if defined(__ANDROID__) && !defined(ANDROID_HOST_BUILD) && !defined(ANDROID_NDK_BUILD)
	else
	{
		// On Android, prefer to use a non-standard prctl called
		// PR_SET_VMA_ANON_NAME to set the name of a private anonymous
		// mapping, as Android restricts EXECUTE permission on
		// CoW/shared anonymous mappings with sepolicy neverallows.
		prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, mapping, length,
		      MACRO_STRINGIFY(REACTOR_ANONYMOUS_MMAP_NAME));
	}
#	endif  // __ANDROID__ && !ANDROID_HOST_BUILD && !ANDROID_NDK_BUILD
#elif defined(__Fuchsia__)
	zx_handle_t vmo;
	if(zx_vmo_create(length, 0, &vmo) != ZX_OK)
	{
		return nullptr;
	}
	if(need_exec &&
	   zx_vmo_replace_as_executable(vmo, ZX_HANDLE_INVALID, &vmo) != ZX_OK)
	{
		return nullptr;
	}
	zx_vaddr_t reservation;
	zx_status_t status = zx_vmar_map(
	    zx_vmar_root_self(), permissionsToZxVmOptions(permissions), 0, vmo,
	    0, length, &reservation);
	zx_handle_close(vmo);
	if(status != ZX_OK)
	{
		return nullptr;
	}

	// zx_vmar_map() returns page-aligned address.
	ASSERT(roundUp(reservation, pageSize) == reservation);

	mapping = reinterpret_cast<void *>(reservation);
#elif defined(__APPLE__)
	int prot = permissionsToMmapProt(permissions);
	int flags = MAP_PRIVATE | MAP_ANONYMOUS;
	// On macOS 10.14 and higher, executables that are code signed with the
	// "runtime" option cannot execute writable memory by default. They can opt
	// into this capability by specifying the "com.apple.security.cs.allow-jit"
	// code signing entitlement and allocating the region with the MAP_JIT flag.
	mapping = mmap(nullptr, length, prot, flags | MAP_JIT, -1, 0);

	if(mapping == MAP_FAILED)
	{
		// Retry without MAP_JIT (for older macOS versions).
		mapping = mmap(nullptr, length, prot, flags, -1, 0);
	}

	if(mapping == MAP_FAILED)
	{
		mapping = nullptr;
	}
#else
	mapping = allocate(length, pageSize);
	protectMemoryPages(mapping, length, permissions);
#endif

	return mapping;
}

void protectMemoryPages(void *memory, size_t bytes, int permissions)
{
	if(bytes == 0)
	{
		return;
	}

	bytes = roundUp(bytes, memoryPageSize());

#if defined(_WIN32)
	unsigned long oldProtection;
	BOOL result =
	    VirtualProtect(memory, bytes, permissionsToProtectMode(permissions),
	                   &oldProtection);
	ASSERT(result);
#elif defined(__Fuchsia__)
	zx_status_t status = zx_vmar_protect(
	    zx_vmar_root_self(), permissionsToZxVmOptions(permissions),
	    reinterpret_cast<zx_vaddr_t>(memory), bytes);
	ASSERT(status == ZX_OK);
#else
	int result =
	    mprotect(memory, bytes, permissionsToMmapProt(permissions));
	ASSERT(result == 0);
#endif
}

void deallocateMemoryPages(void *memory, size_t bytes)
{
#if defined(_WIN32)
	unsigned long oldProtection;
	BOOL result;
	result = VirtualProtect(memory, bytes, PAGE_READWRITE, &oldProtection);
	ASSERT(result);
	result = VirtualFree(memory, 0, MEM_RELEASE);
	ASSERT(result);
#elif defined(__APPLE__) || (defined(__linux__) && defined(REACTOR_ANONYMOUS_MMAP_NAME))
	size_t pageSize = memoryPageSize();
	size_t length = (bytes + pageSize - 1) & ~(pageSize - 1);
	int result = munmap(memory, length);
	ASSERT(result == 0);
#elif defined(__Fuchsia__)
	size_t pageSize = memoryPageSize();
	size_t length = roundUp(bytes, pageSize);
	zx_status_t status = zx_vmar_unmap(
	    zx_vmar_root_self(), reinterpret_cast<zx_vaddr_t>(memory), length);
	ASSERT(status == ZX_OK);
#else
	int result = mprotect(memory, bytes, PROT_READ | PROT_WRITE);
	ASSERT(result == 0);
	deallocate(memory);
#endif
}

}  // namespace rr
