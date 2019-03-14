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

#include <stdlib.h>
#include <assert.h>

#include <algorithm>

#include <utils/Log.h>

namespace utils {

// ------------------------------------------------------------------------------------------------
// LinearAllocator
// ------------------------------------------------------------------------------------------------

LinearAllocator::LinearAllocator(void* begin, void* end) noexcept
    : mBegin(begin), mEnd(end), mCurrent(begin) {
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
    std::swap(mEnd, rhs.mEnd);
    std::swap(mCurrent, rhs.mCurrent);
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

TrackingPolicy::HighWatermark::~HighWatermark() noexcept {
    size_t wm = mHighWaterMark;
    size_t wmpct = wm / (mSize / 100);
    if (wmpct > 80) {
        slog.d << mName << " arena: High watermark "
            << wm / 1024 << " KiB (" << wmpct << "%)" << io::endl;
    }
}

} // namespace utils
