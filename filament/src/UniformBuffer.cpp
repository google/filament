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

void UniformBuffer::free(void* addr, size_t) noexcept {
    ::free(addr);
}

// ------------------------------------------------------------------------------------------------

template<size_t Size>
void UniformBuffer::setUniformUntyped(size_t offset, void const* UTILS_RESTRICT v) noexcept{
    setUniformUntyped<Size>(invalidateUniforms(offset, Size), 0ul, v);
}

template
void UniformBuffer::setUniformUntyped<4ul>(size_t offset, void const* UTILS_RESTRICT v) noexcept;
template
void UniformBuffer::setUniformUntyped<8ul>(size_t offset, void const* UTILS_RESTRICT v) noexcept;
template
void UniformBuffer::setUniformUntyped<12ul>(size_t offset, void const* UTILS_RESTRICT v) noexcept;
template
void UniformBuffer::setUniformUntyped<16ul>(size_t offset, void const* UTILS_RESTRICT v) noexcept;
template
void UniformBuffer::setUniformUntyped<64ul>(size_t offset, void const* UTILS_RESTRICT v) noexcept;

template<size_t Size>
void UniformBuffer::setUniformArrayUntyped(size_t offset, void const* UTILS_RESTRICT begin, size_t count) noexcept {
    constexpr size_t stride = (Size + 0xFu) & ~0xFu;
    size_t arraySize = stride * count - stride + Size;
    void* UTILS_RESTRICT p = invalidateUniforms(offset, arraySize);
    for (size_t i = 0; i < count; i++) {
        setUniformUntyped<Size>(p, 0ul, static_cast<const char *>(begin) + i * Size);
        p = utils::pointermath::add(p, stride);
    }
}

template
void UniformBuffer::setUniformArrayUntyped<4ul>(size_t offset, void const* UTILS_RESTRICT begin, size_t count) noexcept;
template
void UniformBuffer::setUniformArrayUntyped<8ul>(size_t offset, void const* UTILS_RESTRICT begin, size_t count) noexcept;
template
void UniformBuffer::setUniformArrayUntyped<12ul>(size_t offset, void const* UTILS_RESTRICT begin, size_t count) noexcept;
template
void UniformBuffer::setUniformArrayUntyped<16ul>(size_t offset, void const* UTILS_RESTRICT begin, size_t count) noexcept;
template
void UniformBuffer::setUniformArrayUntyped<64ul>(size_t offset, void const* UTILS_RESTRICT begin, size_t count) noexcept;

// specialization for mat3f (which has a different alignment, see std140 layout rules)
template<>
UTILS_NOINLINE
void UniformBuffer::setUniform(void* addr, size_t offset, const math::mat3f& v) noexcept {
    struct mat43 {
        float v[3][4];
    };

    addr = static_cast<char*>(addr) + offset;
    mat43& temp = *static_cast<mat43*>(addr);

    temp.v[0][0] = v[0][0];
    temp.v[0][1] = v[0][1];
    temp.v[0][2] = v[0][2];

    temp.v[1][0] = v[1][0];
    temp.v[1][1] = v[1][1];
    temp.v[1][2] = v[1][2];

    temp.v[2][0] = v[2][0];
    temp.v[2][1] = v[2][1];
    temp.v[2][2] = v[2][2];

    // don't store anything in temp.v[][3] because there could be uniforms packed there
}

template<>
void UniformBuffer::setUniform(size_t offset, const math::mat3f& v) noexcept {
    setUniform(invalidateUniforms(offset, sizeof(v)), 0, v);
}

#if !defined(NDEBUG)

utils::io::ostream& operator<<(utils::io::ostream& out, const UniformBuffer& rhs) {
    return out << "UniformBuffer(data=" << rhs.getBuffer() << ", size=" << rhs.getSize() << ")";
}

#endif
} // namespace filament
