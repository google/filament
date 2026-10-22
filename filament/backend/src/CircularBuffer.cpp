/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "private/backend/CircularBuffer.h"

#include <utils/Logger.h>
#include <utils/Panic.h>
#include <utils/architecture.h>
#include <utils/ashmem.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/ostream.h>

#if !defined(WIN32) && !defined(__EMSCRIPTEN__)
#    include <sys/mman.h>
#    include <unistd.h>
#    define HAS_MMAP 1
#else
#    define HAS_MMAP 0
#endif

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(WIN32)
#    include <windows.h> // for VirtualAlloc, VirtualProtect, VirtualFree
#    include <utils/unwindows.h>
#endif

using namespace utils;

namespace filament::backend {

size_t CircularBuffer::sPageSize = arch::getPageSize();

CircularBuffer::CircularBuffer(size_t size)
    : mSize(size) {
    mData = alloc(size);
    mTail = mData;
    mHead = mData;
}

CircularBuffer::~CircularBuffer() noexcept {
    dealloc();
}

// If the system support mmap(), use it for creating a "hard circular buffer" where two virtual
// address ranges are mapped to the same physical pages.
//
// This code also has a soft circular buffer mode in case we're not able to allow sufficient
// continuous address spaces for both ranges.
//
// If the system does not support mmap, emulate soft circular buffer with two buffers next
// to each others and a special case in circularize()

UTILS_NOINLINE
void* CircularBuffer::alloc(size_t size) {
#if HAS_MMAP
    void* data = nullptr;
    void* vaddr = MAP_FAILED;
    void* vaddr_shadow = MAP_FAILED;
    void* vaddr_guard = MAP_FAILED;
    size_t const BLOCK_SIZE = getBlockSize();
    int const fd = ashmem_create_region("filament::CircularBuffer", size + BLOCK_SIZE);
    if (fd >= 0) {
        // reserve/find enough address space
        void* reserve_vaddr = mmap(nullptr, size * 2 + BLOCK_SIZE,
                PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (reserve_vaddr != MAP_FAILED) {
            munmap(reserve_vaddr, size * 2 + BLOCK_SIZE);
            // map the circular buffer once...
            vaddr = mmap(reserve_vaddr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
            if (vaddr != MAP_FAILED) {
                // populate the address space with pages (because this is a circular buffer,
                // all the pages will be allocated eventually, might as well do it now)
                memset(vaddr, 0, size);
                // and map the circular buffer again, behind the previous copy...
                vaddr_shadow = mmap((char*)vaddr + size, size,
                        PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
                if (vaddr_shadow != MAP_FAILED && (vaddr_shadow == (char*)vaddr + size)) {
                    // finally map the guard page, to make sure we never corrupt memory
                    vaddr_guard = mmap((char*)vaddr_shadow + size, BLOCK_SIZE, PROT_NONE,
                            MAP_PRIVATE, fd, (off_t)size);
                    if (vaddr_guard != MAP_FAILED && (vaddr_guard == (char*)vaddr_shadow + size)) {
                        // woo-hoo success!
                        mAshmemFd = fd;
                        data = vaddr;
                    }
                }
            }
        }
    }

    if (UTILS_UNLIKELY(mAshmemFd < 0)) {
        // ashmem failed
        if (vaddr_guard != MAP_FAILED) {
            munmap(vaddr_guard, BLOCK_SIZE);
        }

        if (vaddr_shadow != MAP_FAILED) {
            munmap(vaddr_shadow, size);
        }

        if (vaddr != MAP_FAILED) {
            munmap(vaddr, size);
        }

        if (fd >= 0) {
            close(fd);
        }

        data = mmap(nullptr, size * 2 + BLOCK_SIZE,
                PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        FILAMENT_CHECK_POSTCONDITION(data != MAP_FAILED) <<
                "couldn't allocate " << (size * 2 / 1024) <<
                " KiB of virtual address space for the command buffer";

        LOG(WARNING) << "Using 'soft' CircularBuffer (" << (size * 2 / 1024) << " KiB)";

        // guard page at the end
        void* guard = (void*)(uintptr_t(data) + size * 2);
        mprotect(guard, BLOCK_SIZE, PROT_NONE);
    }
    return data;
#elif defined(WIN32)
    size_t const BLOCK_SIZE = getBlockSize();
    // On Windows, use VirtualAlloc instead of malloc to allocate virtual memory.
    // This allows us to set page protection by VirtualProtect.
    void* data = VirtualAlloc(nullptr, size * 2 + BLOCK_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    FILAMENT_CHECK_POSTCONDITION(data != nullptr)
            << "couldn't allocate " << (size * 2 / 1024)
            << " KiB of virtual address space for the command buffer";

    // guard page at the end
    void* guard = (void*)(uintptr_t(data) + size * 2);
    DWORD oldProtect = 0;
    BOOL ok = VirtualProtect(guard, BLOCK_SIZE, PAGE_NOACCESS, &oldProtect);
    FILAMENT_CHECK_POSTCONDITION(ok) << "VirtualProtect failed to set guard page";
    return data;
#else
    return ::malloc(2 * size);
#endif
}

UTILS_NOINLINE
void CircularBuffer::dealloc() noexcept {
#if HAS_MMAP
    if (mData) {
        size_t const BLOCK_SIZE = getBlockSize();
        munmap(mData, mSize * 2 + BLOCK_SIZE);
        if (mAshmemFd >= 0) {
            close(mAshmemFd);
            mAshmemFd = -1;
        }
    }
#elif defined(WIN32)
    if (mData) {
        VirtualFree(mData, 0, MEM_RELEASE);
    }
#else
    ::free(mData);
#endif
    mData = nullptr;
}


CircularBuffer::Range CircularBuffer::getBuffer() noexcept {
    Range const range{ .tail = mTail, .head = mHead };

    char* const pData = static_cast<char*>(mData);
    char const* const pEnd = pData + mSize;
    char const* const pHead = static_cast<char const*>(mHead);
    if (UTILS_UNLIKELY(pHead >= pEnd)) {
        size_t const overflow = pHead - pEnd;
        if (UTILS_LIKELY(mAshmemFd > 0)) {
            assert_invariant(overflow <= mSize);
            mHead = static_cast<void*>(pData + overflow);
            // Data         Tail  End   Head              [virtual]
            //  v             v    v     v
            //  +-------------:----+-----:--------------+
            //  |             :    |     :              |
            //  +-----:------------+--------------------+
            //       Head          |<------ copy ------>| [physical]
        } else {
            // Data         Tail  End   Head
            //  v             v    v     v
            //  +-------------:----+-----:--------------+
            //  |             :    |     :              |
            //  +-----|------------+-----|--------------+
            //        |<---------------->|
            //           sliding window
            mHead = mData;
        }
    }
    mTail = mHead;

    return range;
}

} // namespace filament::backend
