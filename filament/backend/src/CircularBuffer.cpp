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

#if !defined(WIN32) && !defined(__EMSCRIPTEN__) && !defined(IOS)
#    include <sys/mman.h>
#    include <unistd.h>
#    define HAS_MMAP 1
#else
#    define HAS_MMAP 0
#endif

#include <stdio.h>

#include <utils/ashmem.h>
#include <utils/Log.h>
#include <utils/Panic.h>

using namespace utils;

namespace filament {
namespace backend {

CircularBuffer::CircularBuffer(size_t size) {
    mData = alloc(size);
    mSize = size;
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

void* CircularBuffer::alloc(size_t size) noexcept {
#if HAS_MMAP
    void* data = nullptr;
    void* vaddr = MAP_FAILED;
    void* vaddr_shadow = MAP_FAILED;
    void* vaddr_guard = MAP_FAILED;
    int fd = ashmem_create_region("filament::CircularBuffer", size + BLOCK_SIZE);
    if (fd >= 0) {
        // reserve/find enough address space
        void* reserve_vaddr = mmap(nullptr, size * 2 + BLOCK_SIZE,
                PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (reserve_vaddr != MAP_FAILED) {
            munmap(reserve_vaddr, size * 2 + BLOCK_SIZE);
            // map the circular buffer once...
            vaddr = mmap(reserve_vaddr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
            if (vaddr != MAP_FAILED) {
                // and map the circular buffer again, behind the previous copy...
                vaddr_shadow = mmap((char*)vaddr + size, size,
                        PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
                if (vaddr_shadow != MAP_FAILED && (vaddr_shadow == (char*)vaddr + size)) {
                    // finally map the guard page, to make sure we never corrupt memory
                    vaddr_guard = mmap((char*)vaddr_shadow + size, BLOCK_SIZE, PROT_NONE,
                            MAP_PRIVATE, fd, (off_t)size);
                    if (vaddr_guard != MAP_FAILED && (vaddr_guard == (char*)vaddr_shadow + size)) {
                        // woo-hoo success!
                        mUsesAshmem = fd;
                        data = vaddr;
                    }
                }
            }
        }
    }

    if (UTILS_UNLIKELY(mUsesAshmem < 0)) {
        // ashmem failed
        if (vaddr_guard != MAP_FAILED) {
            munmap(vaddr_guard, size);
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

        ASSERT_POSTCONDITION(data,
                "couldn't allocate %u KiB of virtual address space for the command buffer",
                (size * 2 / 1024));

        slog.d << "WARNING: Using soft CircularBuffer (" << (size * 2 / 1024) << " KiB)"
               << io::endl;

        // guard page at the end
        void* guard = (void*)(uintptr_t(data) + size * 2);
        mprotect(guard, BLOCK_SIZE, PROT_NONE);
    }
    return data;
#else
    return ::malloc(2 * size);
#endif
}

void CircularBuffer::dealloc() noexcept {
#if HAS_MMAP
    if (mData) {
        munmap(mData, mSize * 2 + BLOCK_SIZE);
        if (mUsesAshmem >= 0) {
            close(mUsesAshmem);
            mUsesAshmem = -1;
        }
    }
#else
    ::free(mData);
#endif
    mData = nullptr;
}


void CircularBuffer::circularize() noexcept {
    if (mUsesAshmem > 0) {
        intptr_t overflow = intptr_t(mHead) - (intptr_t(mData) + ssize_t(mSize));
        if (overflow >= 0) {
            assert(size_t(overflow) <= mSize);
            mHead = (void *) (intptr_t(mData) + overflow);
            #ifndef NDEBUG
            memset(mData, 0xA5, size_t(overflow));
            #endif
        }
    } else {
        // Only circularize if mHead if in the second buffer.
        if (intptr_t(mHead) - intptr_t(mData) > ssize_t(mSize)) {
            mHead = mData;
        }
    }
    mTail = mHead;
}

} // namespace backend
} // namespace filament
