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

#include <utils/Panic.h>
#include <utils/architecture.h>
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

#ifdef __ANDROID__
#    include <sys/prctl.h>
#    ifndef PR_SET_VMA
#        define PR_SET_VMA 0x53564d41
#        define PR_SET_VMA_ANON_NAME 0
#    endif
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

// Allocate two linear banks of demand-paged virtual address space, each followed by a guard page
// when supported, without touching pages upfront. Physical pages are faulted in lazily by the OS
// only up to the high-water mark written in each bank.
UTILS_NOINLINE
void* CircularBuffer::alloc(size_t size) {
#if HAS_MMAP
    size_t const BLOCK_SIZE = getBlockSize();
    size_t const alignedSize = (size + (BLOCK_SIZE - 1u)) & ~(BLOCK_SIZE - 1u);
    mBankStride = alignedSize + BLOCK_SIZE;
    void* const data = mmap(nullptr, mBankStride * 2,
            PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    FILAMENT_CHECK_POSTCONDITION(data != MAP_FAILED) <<
            "couldn't allocate " << (mBankStride * 2 / 1024) <<
            " KiB of virtual address space for the command buffer";

#ifdef __ANDROID__
    prctl(PR_SET_VMA, PR_SET_VMA_ANON_NAME, data, mBankStride * 2, "filament::CircularBuffer");
#endif

    // guard page at the end of each bank
    void* const guard0 = reinterpret_cast<void*>(uintptr_t(data) + alignedSize);
    void* const guard1 = reinterpret_cast<void*>(uintptr_t(data) + mBankStride + alignedSize);
    mprotect(guard0, BLOCK_SIZE, PROT_NONE);
    mprotect(guard1, BLOCK_SIZE, PROT_NONE);
    return data;
#elif defined(WIN32)
    size_t const BLOCK_SIZE = getBlockSize();
    size_t const alignedSize = (size + (BLOCK_SIZE - 1u)) & ~(BLOCK_SIZE - 1u);
    mBankStride = alignedSize + BLOCK_SIZE;
    // On Windows, use VirtualAlloc instead of malloc to allocate virtual memory.
    // This allows us to set page protection by VirtualProtect.
    void* const data = VirtualAlloc(nullptr, mBankStride * 2,
            MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    FILAMENT_CHECK_POSTCONDITION(data != nullptr)
            << "couldn't allocate " << (mBankStride * 2 / 1024)
            << " KiB of virtual address space for the command buffer";

    // guard page at the end of each bank
    void* const guard0 = reinterpret_cast<void*>(uintptr_t(data) + alignedSize);
    void* const guard1 = reinterpret_cast<void*>(uintptr_t(data) + mBankStride + alignedSize);
    DWORD oldProtect = 0;
    BOOL const ok0 = VirtualProtect(guard0, BLOCK_SIZE, PAGE_NOACCESS, &oldProtect);
    BOOL const ok1 = VirtualProtect(guard1, BLOCK_SIZE, PAGE_NOACCESS, &oldProtect);
    FILAMENT_CHECK_POSTCONDITION(ok0 && ok1) << "VirtualProtect failed to set guard page";
    return data;
#else
    mBankStride = size;
    return ::malloc(mBankStride * 2);
#endif
}

UTILS_NOINLINE
void CircularBuffer::dealloc() noexcept {
#if HAS_MMAP
    if (mData) {
        munmap(mData, mBankStride * 2);
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
    assert_invariant(static_cast<char const*>(mHead) <=
            static_cast<char const*>(mData) + getBank(mTail) * mBankStride + mSize);
    mTail = mHead;
    return range;
}

} // namespace filament::backend
