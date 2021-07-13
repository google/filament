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

#include <utils/Panic.h>

#include <stdlib.h>

namespace filament::backend {

using namespace utils;

UTILS_NOINLINE
HandleAllocator::Allocator::Allocator(AreaPolicy::HeapArea const& area)
        : mArea(area) {
    // TODO: we probably need a better way to set the size of these pools
    const size_t unit = area.size() / 32;
    const size_t offsetPool1 =      unit;
    const size_t offsetPool2 = 16 * unit;
    char* const p = (char*)area.begin();
    mPool0 = PoolAllocator< 16, 16>(p, p + offsetPool1);
    mPool1 = PoolAllocator< 64, 16>(p + offsetPool1, p + offsetPool2);
    mPool2 = PoolAllocator<208, 16>(p + offsetPool2, area.end());
}

// ------------------------------------------------------------------------------------------------

HandleAllocator::HandleAllocator(const char* name, size_t size) noexcept
    : mHandleArena(name, size) {
}

HandleAllocator::~HandleAllocator() {
    auto& overflowMap = mOverflowMap;
    if (!overflowMap.empty()) {
        PANIC_LOG("Not all handles have been freed. Probably leaking memory.");
        // Free remaining handle memory
        for (auto& entry : overflowMap) {
            ::free(entry.second);
        }
    }
}

UTILS_NOINLINE
void* HandleAllocator::handleToPointerSlow(HandleBase::HandleId id) const noexcept {
    auto& overflowMap = mOverflowMap;
    std::lock_guard lock(mLock);
    auto pos = overflowMap.find(id);
    if (pos != overflowMap.end()) {
        return pos.value();
    }
    return nullptr;
}

HandleBase::HandleId HandleAllocator::allocateHandleSlow(size_t size) noexcept {
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

void HandleAllocator::deallocateHandleSlow(HandleBase::HandleId id, size_t) noexcept {
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

} // namespace filament::backend
