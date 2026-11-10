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

#ifndef TNT_FILAMENT_BACKEND_PRIVATE_CIRCULARBUFFER_H
#define TNT_FILAMENT_BACKEND_PRIVATE_CIRCULARBUFFER_H

#include <utils/debug.h>

#include <stddef.h>
#include <stdint.h>

namespace filament::backend {

class CircularBuffer {
public:
    // bufferSize: total buffer size.
    //      This must be at least 2*requiredSize to avoid blocking on flush, however
    //      because sometimes the display can get ahead of the render() thread, it's good
    //      to set it to 3*requiredSize to avoid blocking the render thread (usually the UI thread).
    explicit CircularBuffer(size_t bufferSize);

    // can't be moved or copy-constructed
    CircularBuffer(CircularBuffer const& rhs) = delete;
    CircularBuffer(CircularBuffer&& rhs) noexcept = delete;
    CircularBuffer& operator=(CircularBuffer const& rhs) = delete;
    CircularBuffer& operator=(CircularBuffer&& rhs) noexcept = delete;

    ~CircularBuffer() noexcept;

    static size_t getBlockSize() noexcept { return sPageSize; }

    // Total size of circular buffer. This is a constant.
    size_t size() const noexcept { return mSize; }

    // Allocates `s` bytes in the circular buffer and returns a pointer to the memory. All
    // allocations must not exceed size() bytes.
    inline void* allocate(size_t s) noexcept {
        // We can never allocate more that size().
        assert_invariant(getUsed() + s <= size());
        char* const cur = static_cast<char*>(mHead);
        mHead = cur + s;
        return cur;
    }

    // Returns true if the buffer is empty, i.e.: no allocations were made since
    // calling getBuffer();
    bool empty() const noexcept { return mTail == mHead; }

    // Returns the size used since the last call to getBuffer()
    size_t getUsed() const noexcept { return intptr_t(mHead) - intptr_t(mTail); }

    // Retrieves the current allocated range and frees it. It is the responsibility of the caller
    // to make sure the returned range is no longer in use by the time allocate() allocates
    // (size() - getUsed()) bytes.
    struct Range {
        void* tail;
        void* head;
    };
    Range getBuffer() noexcept;

    // The underlying virtual address space consists of two linear banks of size() bytes each, with
    // each bank followed by a guard page:
    //
    //  mData                                    mData + mBankStride
    //  v                                        v
    //  +-------------+-------------+--------+   +-------------+-------------+--------+
    //  |/////////////|. . . . . . .| Guard  |   |/////////////|. . . . . . .| Guard  |
    //  |<- Bank 0  ->| (untouched) |  Page  |   |<- Bank 1  ->| (untouched) |  Page  |
    //  |   in-flight |   pages     |        |   |   in-flight |   pages     |        |
    //  +-------------+-------------+--------+   +-------------+-------------+--------+
    //  |<--------- mSize --------->|            |<--------- mSize --------->|
    //  |<----------- mBankStride ---------->|   |<----------- mBankStride ---------->|
    //
    // Neither bank ever wraps around. CommandBufferQueue bumps linearly within the active bank and
    // resets to the base of Bank 0 (mData) or Bank 1 (mData + mBankStride) whenever that bank has
    // no in-flight buffers, keeping physical page faults bounded by the in-flight high-water mark
    // of each bank.
    size_t getBank(void const* p) const noexcept {
        char const* const base = static_cast<char const*>(mData);
        char const* const ptr = static_cast<char const*>(p);
        assert_invariant(ptr >= base && ptr < base + 2 * mBankStride);
        return (ptr < base + mBankStride) ? 0u : 1u;
    }

    void setBank(size_t bank) noexcept {
        assert_invariant(bank <= 1);
        assert_invariant(empty());
        mTail = static_cast<char*>(mData) + bank * mBankStride;
        mHead = mTail;
    }

    size_t getBankStride() const noexcept { return mBankStride; }

private:
    void* alloc(size_t size);
    void dealloc() noexcept;

    // pointer to the beginning of the circular buffer (constant)
    void* mData = nullptr;

    // size of each bank (constant)
    size_t const mSize;

    // virtual address distance between Bank 0 and Bank 1 (includes guard page)
    size_t mBankStride = 0;

    // pointer to the beginning of recorded data
    void* mTail = nullptr;

    // pointer to the next available command
    void* mHead = nullptr;

    // system page size
    static size_t sPageSize;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_PRIVATE_CIRCULARBUFFER_H
