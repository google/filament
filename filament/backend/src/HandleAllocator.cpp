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
#include <utils/CString.h>
#include <utils/Log.h>
#include <utils/Panic.h>
#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/ostream.h>

#include <algorithm>
#include <exception>
#include <limits>
#include <mutex>
#include <utility>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

namespace filament::backend {

using namespace utils;

template <size_t P0, size_t P1, size_t P2>
UTILS_NOINLINE
HandleAllocator<P0, P1, P2>::Allocator::Allocator(AreaPolicy::HeapArea const& area,
        bool disableUseAfterFreeCheck)
        : mArea(area),
          mUseAfterFreeCheckDisabled(disableUseAfterFreeCheck) {

    // The largest handle this allocator can generate currently depends on the architecture's
    // min alignment, typically 8 or 16 bytes.
    // e.g. On Android armv8, the alignment is 16 bytes, so for a 1 MiB heap, the largest handle
    //      index will be 65536. Note that this is not the same as the number of handles (which
    //      will always be less).
    // Because our maximum representable handle currently is 0x07FFFFFF, the maximum no-nonsensical
    // heap size is 2 GiB, which amounts to 7.6 millions handles per pool (in the GL case).
    size_t const maxHeapSize = std::min(area.size(), HANDLE_INDEX_MASK * getAlignment());

    if (UTILS_UNLIKELY(maxHeapSize != area.size())) {
        slog.w << "HandleAllocator heap size reduced to "
               << maxHeapSize << " from " << area.size() << io::endl;
    }

    // make sure we start with a clean arena. This is needed to ensure that all blocks start
    // with an age of 0.
    memset(area.data(), 0, maxHeapSize);

    // size the different pools so that they can all contain the same number of handles
    size_t const count = maxHeapSize / (P0 + P1 + P2);
    char* const p0 = static_cast<char*>(area.begin());
    char* const p1 = p0 + count * P0;
    char* const p2 = p1 + count * P1;

    mPool0 = Pool<P0>(p0, count * P0);
    mPool1 = Pool<P1>(p1, count * P1);
    mPool2 = Pool<P2>(p2, count * P2);
}

// ------------------------------------------------------------------------------------------------

template <size_t P0, size_t P1, size_t P2>
HandleAllocator<P0, P1, P2>::HandleAllocator(const char* name, size_t size,
        bool disableUseAfterFreeCheck,
        bool disableHeapHandleTags) noexcept
    : mHandleArena(name, size, disableUseAfterFreeCheck),
      mUseAfterFreeCheckDisabled(disableUseAfterFreeCheck),
      mHeapHandleTagsDisabled(disableHeapHandleTags) {
}

template <size_t P0, size_t P1, size_t P2>
HandleAllocator<P0, P1, P2>::HandleAllocator(const char* name, size_t size) noexcept
    : HandleAllocator(name, size, false, false) {
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
HandleBase::HandleId HandleAllocator<P0, P1, P2>::allocateHandleSlow(size_t size) {
    void* p = ::malloc(size);
    std::unique_lock lock(mLock);

    HandleBase::HandleId id = (++mId) | HANDLE_HEAP_FLAG;

    FILAMENT_CHECK_POSTCONDITION(mId < HANDLE_HEAP_FLAG) <<
            "No more Handle ids available! This can happen if HandleAllocator arena has been full"
            " for a while. Please increase FILAMENT_OPENGL_HANDLE_ARENA_SIZE_IN_MB";

    mOverflowMap.emplace(id, p);
    lock.unlock();

    if (UTILS_UNLIKELY(id == (HANDLE_HEAP_FLAG | 1u))) { // meaning id was zero
        PANIC_LOG("HandleAllocator arena is full, using slower system heap. Please increase "
                  "the appropriate constant (e.g. FILAMENT_OPENGL_HANDLE_ARENA_SIZE_IN_MB).");
    }
    return id;
}

template <size_t P0, size_t P1, size_t P2>
void HandleAllocator<P0, P1, P2>::deallocateHandleSlow(HandleBase::HandleId id, size_t) noexcept {
    assert_invariant(id & HANDLE_HEAP_FLAG);
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

template<size_t P0, size_t P1, size_t P2>
UTILS_NOINLINE
CString HandleAllocator<P0, P1, P2>::getHandleTag(HandleBase::HandleId id) const noexcept {
    uint32_t key = id;
    if (UTILS_LIKELY(isPoolHandle(id))) {
        // Truncate the age to get the debug tag
        key &= ~(HANDLE_DEBUG_TAG_MASK ^ HANDLE_AGE_MASK);
    }
    return findHandleTag(key);
}

DebugTag::DebugTag() {
    // Reserve initial space for debug tags. This prevents excessive calls to malloc when the first
    // few tags are set.
    mDebugTags.reserve(512);
}

UTILS_NOINLINE
CString DebugTag::findHandleTag(HandleBase::HandleId key) const noexcept {
    std::unique_lock const lock(mDebugTagLock);
    if (auto pos = mDebugTags.find(key); pos != mDebugTags.end()) {
        return pos->second;
    }
    return "(no tag)";
}

UTILS_NOINLINE
void DebugTag::writePoolHandleTag(HandleBase::HandleId key, CString&& tag) noexcept {
    // This line is the costly part. In the future, we could potentially use a custom
    // allocator.
    std::unique_lock const lock(mDebugTagLock);
    // Pool based tags will be recycled after a certain age.
    mDebugTags[key] = std::move(tag);
}

UTILS_NOINLINE
void DebugTag::writeHeapHandleTag(HandleBase::HandleId key, CString&& tag) noexcept {
    // This line is the costly part. In the future, we could potentially use a custom
    // allocator.
    std::unique_lock const lock(mDebugTagLock);
    // FIXME: Heap-based tag will never be recycled, therefore, this can grow indefinitely, once we're in the slow mode.
    mDebugTags[key] = std::move(tag);
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

#if defined (FILAMENT_SUPPORTS_WEBGPU)
template class HandleAllocatorWGPU;
#endif

} // namespace filament::backend
