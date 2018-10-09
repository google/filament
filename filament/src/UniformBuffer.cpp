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

#include "UniformBuffer.h"

#include <utils/Mutex.h>

#include <mutex>
#include <type_traits>
#include <unordered_map>

#include <stdlib.h>
#include <string.h>

using namespace math;

namespace filament {

class Pool {
    // TODO: not sure what's a good value here
    static constexpr size_t DEFAULT_BIN_COUNT = 256;
    static constexpr size_t SIZE_ALIGNMENT = 32;

public:
    Pool() noexcept {
        mBlocks.reserve(mMaxElementCount);
    }

    explicit Pool(size_t maxCount) noexcept : mMaxElementCount(maxCount) {
        mBlocks.reserve(maxCount);
    }

    ~Pool() noexcept {
        for (auto& item : mBlocks) {
            free(item.second);
        }
    }

    // always round size up to reduce the number of bins
    static size_t align(size_t size) noexcept {
        return (size + (SIZE_ALIGNMENT - 1)) & ~(SIZE_ALIGNMENT - 1);
    }

    void* get(size_t size) noexcept {
        size = align(size);
        std::unique_lock<utils::Mutex> lock(mLock);
        void* addr = nullptr;
        auto& blocks = mBlocks;
        auto pos = blocks.find(size);
        if (UTILS_LIKELY(pos != blocks.end())) {
            addr = pos->second;
            mTotalMemory -= pos->first;
            blocks.erase(pos);
        } else {
            lock.unlock();
            addr = malloc(size);
        }
        return addr;
    }

    void put(void* addr, size_t size) noexcept {
        size = align(size);
        std::unique_lock<utils::Mutex> lock(mLock);
        auto& blocks = mBlocks;
        if (UTILS_LIKELY(blocks.size() < mMaxElementCount)) {
            mTotalMemory += size;
            // TODO: maybe we should purge the Pool at some point
            blocks.emplace(size, addr);
        } else {
            lock.unlock();
            free(addr);
        }
    }

private:
    utils::Mutex mLock;
    std::unordered_multimap<size_t, void*> mBlocks;
    size_t mMaxElementCount = DEFAULT_BIN_COUNT;
    size_t mTotalMemory = 0;
};

static Pool sMemoryPool;


UniformBuffer::UniformBuffer(size_t size) noexcept
    : mBuffer(mStorage),
      mSize(uint32_t(size)),
      mSomethingDirty(true) {
    if (UTILS_LIKELY(size > sizeof(mStorage))) {
        mBuffer = UniformBuffer::alloc(size);
    }
    memset(mBuffer, 0, size);
}

UniformBuffer::UniformBuffer(UniformInterfaceBlock const& uib) noexcept
    : UniformBuffer(uib.getSize()) {
}

UniformBuffer::UniformBuffer(const UniformBuffer& rhs)
        : mBuffer(mStorage),
          mSize(rhs.mSize),
          mSomethingDirty(rhs.mSomethingDirty) {
    if (UTILS_LIKELY(mSize > sizeof(mStorage))) {
        mBuffer = UniformBuffer::alloc(rhs.mSize);
    }
    memcpy(mBuffer, rhs.mBuffer, mSize);
}

UniformBuffer::UniformBuffer(const UniformBuffer& rhs, size_t trim)
        : mBuffer(mStorage),
          mSize(std::min(uint32_t(trim), rhs.mSize)),
          mSomethingDirty(rhs.mSomethingDirty) {
    if (UTILS_LIKELY(mSize > sizeof(mStorage))) {
        mBuffer = UniformBuffer::alloc(rhs.mSize);
    }
    memcpy(mBuffer, rhs.mBuffer, mSize);
}

UniformBuffer::UniformBuffer(UniformBuffer&& rhs) noexcept
        : mBuffer(rhs.mBuffer),
          mSize(rhs.mSize),
          mSomethingDirty(rhs.mSomethingDirty) {
    if (UTILS_LIKELY(rhs.isLocalStorage())) {
        mBuffer = mStorage;
        memcpy(mBuffer, rhs.mBuffer, mSize);
    }
    rhs.mBuffer = nullptr;
    rhs.mSize = 0;
}

UniformBuffer& UniformBuffer::operator=(UniformBuffer&& rhs) noexcept {
    if (this != &rhs) {
        mSomethingDirty = rhs.mSomethingDirty;
        if (UTILS_LIKELY(rhs.isLocalStorage())) {
            mBuffer = mStorage;
            mSize = rhs.mSize;
            memcpy(mBuffer, rhs.mBuffer, rhs.mSize);
        } else {
            std::swap(mBuffer, rhs.mBuffer);
            std::swap(mSize,   rhs.mSize);
        }
    }
    return *this;
}

void* UniformBuffer::alloc(size_t size) noexcept {
    return sMemoryPool.get(size);
}

void UniformBuffer::free(void* addr, size_t size) noexcept {
    sMemoryPool.put(addr, size);
}

#if !defined(NDEBUG)
utils::io::ostream& operator<<(utils::io::ostream& out, const UniformBuffer& rhs) {
    return out << "UniformBuffer(data=" << rhs.getBuffer() << ", size=" << rhs.getSize() << ")";
}
#endif
} // namespace filament
