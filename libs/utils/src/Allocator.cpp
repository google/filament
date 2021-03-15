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

#include <utils/Allocator.h>

#include <utils/Log.h>

#include <algorithm>

#include <stdlib.h>
#include <assert.h>
#include <string.h>

namespace utils {

// ------------------------------------------------------------------------------------------------
// LinearAllocator
// ------------------------------------------------------------------------------------------------

LinearAllocator::LinearAllocator(void* begin, void* end) noexcept
    : mBegin(begin), mSize(uintptr_t(end) - uintptr_t(begin)) {
}

LinearAllocator::LinearAllocator(LinearAllocator&& rhs) noexcept {
    // class attributes have been initialized to default values
    this->swap(rhs);
}

LinearAllocator& LinearAllocator::operator=(LinearAllocator&& rhs) noexcept {
    if (this != &rhs) {
        this->swap(rhs);
    }
    return *this;
}

void LinearAllocator::swap(LinearAllocator& rhs) noexcept {
    std::swap(mBegin, rhs.mBegin);
    std::swap(mSize, rhs.mSize);
    std::swap(mCur, rhs.mCur);
}

// ------------------------------------------------------------------------------------------------
// FreeList
// ------------------------------------------------------------------------------------------------

FreeList::Node* FreeList::init(void* begin, void* end,
        size_t elementSize, size_t alignment, size_t extra) noexcept
{
    void* const p = pointermath::align(begin, alignment, extra);
    void* const n = pointermath::align(pointermath::add(p, elementSize), alignment, extra);
    assert(p >= begin && p < end);
    assert(n >= begin && n < end && n > p);

    const size_t d = uintptr_t(n) - uintptr_t(p);
    const size_t num = (uintptr_t(end) - uintptr_t(p)) / d;

    // set first entry
    Node* head = static_cast<Node*>(p);

    // next entry
    Node* cur = head;
    for (size_t i = 1; i < num; ++i) {
        Node* next = pointermath::add(cur, d);
        cur->next = next;
        cur = next;
    }
    assert(cur < end);
    assert(pointermath::add(cur, d) <= end);
    cur->next = nullptr;
    return head;
}

FreeList::FreeList(void* begin, void* end,
        size_t elementSize, size_t alignment, size_t extra) noexcept
        : mHead(init(begin, end, elementSize, alignment, extra))
#ifndef NDEBUG
        , mBegin(begin), mEnd(end)
#endif
{
}

AtomicFreeList::AtomicFreeList(void* begin, void* end,
        size_t elementSize, size_t alignment, size_t extra) noexcept
{
#ifdef ANDROID
    // on some platform (e.g. web) this returns false. we really only care about mobile though.
    assert(mHead.is_lock_free());
#endif

    void* const p = pointermath::align(begin, alignment, extra);
    void* const n = pointermath::align(pointermath::add(p, elementSize), alignment, extra);
    assert(p >= begin && p < end);
    assert(n >= begin && n < end && n > p);

    const size_t d = uintptr_t(n) - uintptr_t(p);
    const size_t num = (uintptr_t(end) - uintptr_t(p)) / d;

    // set first entry
    Node* head = static_cast<Node*>(p);
    mStorage = head;

    // next entry
    Node* cur = head;
    for (size_t i = 1; i < num; ++i) {
        Node* next = pointermath::add(cur, d);
        cur->next = next;
        cur = next;
    }
    assert(cur < end);
    assert(pointermath::add(cur, d) <= end);
    cur->next = nullptr;

    mHead.store({ int32_t(head - mStorage), 0 });
}

// ------------------------------------------------------------------------------------------------

void TrackingPolicy::HighWatermark::onAlloc(
        void* p, size_t size, size_t alignment, size_t extra) noexcept {
    mCurrent += uint32_t(size);
    mHighWaterMark = mCurrent > mHighWaterMark ? mCurrent : mHighWaterMark;
}

TrackingPolicy::HighWatermark::~HighWatermark() noexcept {
    if (mSize > 0) {
        size_t wm = mHighWaterMark;
        size_t wmpct = wm / (mSize / 100);
        if (wmpct > 80) {
            slog.d << mName << " arena: High watermark "
                   << wm / 1024 << " KiB (" << wmpct << "%)" << io::endl;
        }
    }
}

void TrackingPolicy::HighWatermark::onFree(void* p, size_t size) noexcept {
    assert(mCurrent >= size);
    mCurrent -= uint32_t(size);
}
void TrackingPolicy::HighWatermark::onReset() noexcept {
    // we should never be here if mBase is nullptr because compilation would have failed when
    // Arena::onReset() tries to call the underlying allocator's onReset()
    assert(mBase);
    mCurrent = 0;
}

void TrackingPolicy::HighWatermark::onRewind(void const* addr) noexcept {
    // we should never be here if mBase is nullptr because compilation would have failed when
    // Arena::onRewind() tries to call the underlying allocator's onReset()
    assert(mBase);
    assert(addr >= mBase);
    mCurrent = uint32_t(uintptr_t(addr) - uintptr_t(mBase));
}

// ------------------------------------------------------------------------------------------------

void TrackingPolicy::Debug::onAlloc(void* p, size_t size, size_t alignment, size_t extra) noexcept {
    if (p) {
        memset(p, 0xeb, size);
    }
}

void TrackingPolicy::Debug::onFree(void* p, size_t size) noexcept {
    if (p) {
        memset(p, 0xef, size);
    }
}

void TrackingPolicy::Debug::onReset() noexcept {
    // we should never be here if mBase is nullptr because compilation would have failed when
    // Arena::onReset() tries to call the underlying allocator's onReset()
    assert(mBase);
    memset(mBase, 0xec, mSize);
}

void TrackingPolicy::Debug::onRewind(void* addr) noexcept {
    // we should never be here if mBase is nullptr because compilation would have failed when
    // Arena::onRewind() tries to call the underlying allocator's onReset()
    assert(mBase);
    assert(addr >= mBase);
    memset(addr, 0x55, uintptr_t(mBase) + mSize - uintptr_t(addr));
}

} // namespace utils
