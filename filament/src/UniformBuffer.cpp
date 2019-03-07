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

#include <stdlib.h>
#include <string.h>

using namespace filament::math;

namespace filament {

UniformBuffer::UniformBuffer(size_t size) noexcept
        : mBuffer(mStorage),
          mSize(uint32_t(size)),
          mSomethingDirty(true) {
    if (UTILS_LIKELY(size > sizeof(mStorage))) {
        mBuffer = UniformBuffer::alloc(size);
    }
    memset(mBuffer, 0, size);
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
            std::swap(mSize, rhs.mSize);
        }
    }
    return *this;
}

UniformBuffer& UniformBuffer::setUniforms(const UniformBuffer& rhs) noexcept {
    if (this != &rhs) {
        if (UTILS_UNLIKELY(mSize != rhs.mSize)) {
            // first free our storage if any
            if (mBuffer && !isLocalStorage()) {
                UniformBuffer::free(mBuffer, mSize);
            }
            // and allocate new storage
            mBuffer = mStorage;
            mSize = rhs.mSize;
            if (mSize > sizeof(mStorage)) {
                mBuffer = UniformBuffer::alloc(mSize);
            }
        }
        memcpy(mBuffer, rhs.mBuffer, rhs.mSize);
        // always invalidate ourselves
        invalidate();
    }
    return *this;
}

void* UniformBuffer::alloc(size_t size) noexcept {
    // these allocations have a long life span
    return ::malloc(size);
}

void UniformBuffer::free(void* addr, size_t size) noexcept {
    ::free(addr);
}

#if !defined(NDEBUG)

utils::io::ostream& operator<<(utils::io::ostream& out, const UniformBuffer& rhs) {
    return out << "UniformBuffer(data=" << rhs.getBuffer() << ", size=" << rhs.getSize() << ")";
}

#endif
} // namespace filament
