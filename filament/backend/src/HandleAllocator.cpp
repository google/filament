/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include "private/backend/HandleAllocator.h"

#include <backend/Handle.h>

#include <utils/Allocator.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Panic.h>

#include <stdlib.h>

#include <mutex>

namespace filament::backend {

using namespace utils;

template <size_t P0, size_t P1, size_t P2>
UTILS_NOINLINE
HandleAllocator<P0, P1, P2>::Allocator::Allocator(AreaPolicy::HeapArea const& area)
        : mArea(area) {

    // size the different pools so that they can all contain the same number of handles
    size_t const count = area.size() / (P0 + P1 + P2);
    char* const p0 = static_cast<char*>(area.begin());
    char* const p1 = p0 + count * P0;
    char* const p2 = p1 + count * P1;

    mPool0 = PoolAllocator< P0, 16>(p0,              count * P0);
    mPool1 = PoolAllocator< P1, 16>(p1 + count * P0, count * P1);
    mPool2 = PoolAllocator< P2, 16>(p2 + count * P0, count * P2);
}

// ------------------------------------------------------------------------------------------------

template <size_t P0, size_t P1, size_t P2>
HandleAllocator<P0, P1, P2>::HandleAllocator(const char* name, size_t size) noexcept
    : mHandleArena(name, size) {
}

template <size_t P0, size_t P1, size_t P2>
HandleAllocator<P0, P1, P2>::~HandleAllocator() {
    auto& overflowMap = mOverflowMap;
    if (!overflowMap.empty()) {
        PANIC_LOG("Not all handles have been freed. Probably leaking memory.");
        // Free remaining handle memory
        for (auto& entry : overflowMap) {
            ::free(entry.second);
        }
    }
}

template <size_t P0, size_t P1, size_t P2>
UTILS_NOINLINE
void* HandleAllocator<P0, P1, P2>::handleToPointerSlow(HandleBase::HandleId id) const noexcept {
    auto& overflowMap = mOverflowMap;
    std::lock_guard lock(mLock);
    auto pos = overflowMap.find(id);
    if (pos != overflowMap.end()) {
        return pos.value();
    }
    return nullptr;
}

template <size_t P0, size_t P1, size_t P2>
HandleBase::HandleId HandleAllocator<P0, P1, P2>::allocateHandleSlow(size_t size) noexcept {
    void* p = ::malloc(size);
    std::unique_lock lock(mLock);
    HandleBase::HandleId id = (++mId) | HEAP_HANDLE_FLAG;
    mOverflowMap.emplace(id, p);
    lock.unlock();

    if (UTILS_UNLIKELY(id == (HEAP_HANDLE_FLAG|1u))) { // meaning id was zero
        PANIC_LOG("HandleAllocator arena is full, using slower system heap. Please increase "
                  "the appropriate constant (e.g. FILAMENT_OPENGL_HANDLE_ARENA_SIZE_IN_MB).");
    }
    return id;
}

template <size_t P0, size_t P1, size_t P2>
void HandleAllocator<P0, P1, P2>::deallocateHandleSlow(HandleBase::HandleId id, size_t) noexcept {
    assert_invariant(id & HEAP_HANDLE_FLAG);
    void* p = nullptr;
    auto& overflowMap = mOverflowMap;

    std::unique_lock lock(mLock);
    auto pos = overflowMap.find(id);
    if (pos != overflowMap.end()) {
        p = pos.value();
        overflowMap.erase(pos);
    }
    lock.unlock();

    ::free(p);
}

// Explicit template instantiations.
#if defined (FILAMENT_SUPPORTS_OPENGL)
template class HandleAllocatorGL;
#endif

#if defined (FILAMENT_DRIVER_SUPPORTS_VULKAN)
template class HandleAllocatorVK;
#endif

#if defined (FILAMENT_SUPPORTS_METAL)
template class HandleAllocatorMTL;
#endif

} // namespace filament::backend
