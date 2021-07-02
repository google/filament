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

namespace filament::backend {

using namespace utils;

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

bool HandleAllocator::Allocator::isPoolAllocation(void* p, size_t size) const noexcept {
    return (p >= mArea.begin() && (char*)p + size <= (char*)mArea.end());
}

void* HandleAllocator::Allocator::alloc_slow(size_t size, size_t alignment, size_t extra) noexcept {
    return nullptr;
}

void HandleAllocator::Allocator::free_slow(void* p, size_t size) noexcept {
}

// ------------------------------------------------------------------------------------------------

HandleAllocator::HandleAllocator(const char* name, size_t size) noexcept
    : mHandleArena(name, size) {
}

void* HandleAllocator::handleToPointer(HandleBase::HandleId id, size_t size) const noexcept {
    char* const base = (char*)mHandleArena.getArea().begin();
    size_t offset = id << Allocator::MIN_ALIGNMENT_SHIFT;
    assert_invariant(mHandleArena.getAllocator().isPoolAllocation(base + offset, size));
    return static_cast<void*>(base + offset);
}

HandleBase::HandleId HandleAllocator::pointerToHandle(void* p) const noexcept {
    char* const base = (char*)mHandleArena.getArea().begin();
    size_t offset = (char*)p - base;
    return HandleBase::HandleId(offset >> Allocator::MIN_ALIGNMENT_SHIFT);
}


} // namespace filament::backend
