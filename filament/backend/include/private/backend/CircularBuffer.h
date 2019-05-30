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

#ifndef TNT_FILAMENT_DRIVER_CIRCULARBUFFER_H
#define TNT_FILAMENT_DRIVER_CIRCULARBUFFER_H

#include <stddef.h>
#include <stdint.h>

#include <utils/compiler.h>

namespace filament {
namespace backend {

class CircularBuffer {
public:
// all allocations are at least one page
    static constexpr size_t BLOCK_BITS = 12;    // 4KB
    static constexpr size_t BLOCK_SIZE = 1 << BLOCK_BITS;
    static constexpr size_t BLOCK_MASK = BLOCK_SIZE - 1;

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

    // allocates 'size' bytes in the circular buffer and returns a pointer to the memory
    // return the current head and moves it forward by size bytes
    inline void* allocate(size_t size) noexcept {
        char* const cur = static_cast<char*>(mHead);
        mHead = cur + size;
        return cur;
    }

    // Total size of circular buffer
    size_t size() const noexcept { return mSize; }

    // returns true if the buffer is empty (e.g. after calling flush)
    bool empty() const noexcept { return mTail == mHead; }

    void* getHead() const noexcept { return mHead; }

    void* getTail() const noexcept { return mTail; }

    // call at least once every getRequiredSize() bytes allocated from the buffer
    void circularize() noexcept;

private:
    void* alloc(size_t size) noexcept;
    void dealloc() noexcept;

    // pointer to the beginning of the circular buffer (constant)
    void* mData = nullptr;
    int mUsesAshmem = -1;

    // size of the circular buffer (constant)
    size_t mSize = 0;

    // pointer to the beginning of recorded data
    void* mTail = nullptr;

    // pointer to the next available command
    void* mHead = nullptr;
};

} // namespace backend
} // namespace filament

#endif // TNT_FILAMENT_DRIVER_CIRCULARBUFFER_H
