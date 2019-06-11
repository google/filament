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

#ifndef TNT_FILAMENT_DRIVER_UNIFORMBUFFER_H
#define TNT_FILAMENT_DRIVER_UNIFORMBUFFER_H

#include <algorithm>

#include "private/backend/DriverApi.h"

#include <utils/Allocator.h>
#include <utils/compiler.h>
#include <utils/Log.h>

#include <backend/BufferDescriptor.h>

#include <math/mat3.h>
#include <math/mat4.h>

#include <stddef.h>
#include <assert.h>


namespace filament {

class UniformBuffer { // NOLINT(cppcoreguidelines-pro-type-member-init)
public:
    UniformBuffer() noexcept = default;

    // create a uniform buffer of a given size in bytes
    explicit UniformBuffer(size_t size) noexcept;

    // disallow copy-construction, since it's heavy.
    UniformBuffer(const UniformBuffer& rhs) = delete;

    // we forbid copy, which makes UniformBuffer's allocation immutable
    UniformBuffer& operator=(const UniformBuffer& rhs) = delete;

    // can be moved
    UniformBuffer(UniformBuffer&& rhs) noexcept;

    // can be moved (e.g. assigned from a temporary)
    UniformBuffer& operator=(UniformBuffer&& rhs) noexcept;

    ~UniformBuffer() noexcept {
        // inline this because there is no point in jumping into the library, just to
        // immediately jump into libc's free()
        if (mBuffer && !isLocalStorage()) {
            // test not necessary but avoids a call to libc (and this is a common enough case)
            UniformBuffer::free(mBuffer, mSize);
        }
    }

    UniformBuffer& setUniforms(const UniformBuffer& rhs) noexcept;

    // invalidate a range of uniforms and return a pointer to it. offset and size given in bytes
    void* invalidateUniforms(size_t offset, size_t size) {
        assert(offset + size <= mSize);
        mSomethingDirty = true;
        return static_cast<char*>(mBuffer) + offset;
    }

    void* invalidate() noexcept {
        return invalidateUniforms(0, mSize);
    }

    // pointer to the uniform buffer
    void const* getBuffer() const noexcept { return mBuffer; }

    // size of the uniform buffer in bytes
    size_t getSize() const noexcept { return mSize; }

    // return if any uniform has been changed
    bool isDirty() const noexcept { return mSomethingDirty; }

    // mark the whole buffer as clean (no modified uniforms)
    void clean() const noexcept { mSomethingDirty = false; }

    /*
     * -----------------------------------------------
     * inline helpers for case where the type is known
     * -----------------------------------------------
     */

    template <typename T>
    struct is_supported_type {
        using type = typename std::enable_if<
                std::is_same<bool, T>::value ||
                std::is_same<float, T>::value ||
                std::is_same<int32_t, T>::value ||
                std::is_same<uint32_t, T>::value ||
                std::is_same<math::quatf, T>::value ||
                std::is_same<math::bool2, T>::value ||
                std::is_same<math::bool3, T>::value ||
                std::is_same<math::bool4, T>::value ||
                std::is_same<math::int2, T>::value ||
                std::is_same<math::int3, T>::value ||
                std::is_same<math::int4, T>::value ||
                std::is_same<math::uint2, T>::value ||
                std::is_same<math::uint3, T>::value ||
                std::is_same<math::uint4, T>::value ||
                std::is_same<math::float2, T>::value ||
                std::is_same<math::float3, T>::value ||
                std::is_same<math::float4, T>::value ||
                std::is_same<math::mat3f, T>::value ||
                std::is_same<math::mat4f, T>::value
        >::type;
    };

    // invalidate an array of uniforms and return a pointer to the first element
    // offset in bytes, and count is the number of elements to invalidate
    template <typename T, typename = typename is_supported_type<T>::type>
    void setUniformArray(size_t offset, T const* UTILS_RESTRICT begin, size_t count) noexcept {
        // we need to align array elements to the size of a vec4 (see std140 layout)
        constexpr size_t stride = (sizeof(T) + 0xF) & ~0xF;
        T* UTILS_RESTRICT p = static_cast<T*>(invalidateUniforms(offset, stride * count));
        for (size_t i = 0; i < count; i++) {
            *p = begin[i];
            p = utils::pointermath::add(p, stride);
        }
    }

    template <typename T, typename = typename is_supported_type<T>::type>
    static void setUniform(void* addr, size_t offset, const T& v) noexcept {
        addr = static_cast<char*>(addr) + offset;
        T* p = static_cast<T*>(addr);
        *p = v;
    }

    // set uniform of known types to the proper offset (e.g.: use offsetof())
    // (see specialization for mat3f below)
    template <typename T, typename = typename is_supported_type<T>::type>
    void setUniform(size_t offset, const T& v) noexcept {
        setUniform(invalidateUniforms(offset, sizeof(T)), 0, v);
    }

    // get uniform of known types from the proper offset (e.g.: use offsetof())
    template<typename T, typename = typename is_supported_type<T>::type>
    T const& getUniform(size_t offset) const noexcept {
        // we don't support mat3f because a specialization would force us to return by value.
        static_assert(!std::is_same<math::mat3f, T>::value, "mat3f not supported");
        return *reinterpret_cast<T const*>(static_cast<char const*>(mBuffer) + offset);
    }

    // helper functions

    backend::BufferDescriptor toBufferDescriptor(backend::DriverApi& driver) const noexcept {
        return toBufferDescriptor(driver, 0, getSize());
    }

    // copy the UBO data and cleans the dirty bits
    backend::BufferDescriptor toBufferDescriptor(
            backend::DriverApi& driver, size_t offset, size_t size) const noexcept {
        backend::BufferDescriptor p;
        p.size = size;
        p.buffer = driver.allocate(p.size); // TODO: use out-of-line buffer if too large
        memcpy(p.buffer, static_cast<const char*>(getBuffer()) + offset, p.size);
        clean();
        return p;
    }

private:
#if !defined(NDEBUG)
    friend utils::io::ostream& operator<<(utils::io::ostream& out, const UniformBuffer& rhs);
#endif
    static void* alloc(size_t size) noexcept;
    static void free(void* addr, size_t size) noexcept;

    inline bool isLocalStorage() const noexcept { return mBuffer == mStorage; }

    // TODO: we need a better to calculate this local storage.
    // Probably the better thing to do would be to use a special allocator.
    // Local storage is limited by the total size of a handle (128 byte for GL)
    char mStorage[96];
    void *mBuffer = nullptr;
    uint32_t mSize = 0;
    mutable bool mSomethingDirty = false;
};

// specialization for mat3f (which has a different alignment, see std140 layout rules)
template<>
inline void UniformBuffer::setUniform(void* addr, size_t offset, const math::mat3f& v) noexcept {
    struct mat43 {
        float v[3][4];
    };

    addr = static_cast<char*>(addr) + offset;
    mat43& temp = *static_cast<mat43*>(addr);

    temp.v[0][0] = v[0][0];
    temp.v[0][1] = v[0][1];
    temp.v[0][2] = v[0][2];
    temp.v[0][3] = 0; // not needed, but doesn't cost anything

    temp.v[1][0] = v[1][0];
    temp.v[1][1] = v[1][1];
    temp.v[1][2] = v[1][2];
    temp.v[1][3] = 0; // not needed, but doesn't cost anything

    temp.v[2][0] = v[2][0];
    temp.v[2][1] = v[2][1];
    temp.v[2][2] = v[2][2];
    temp.v[2][3] = 0; // not needed, but doesn't cost anything
}

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_UNIFORMBUFFER_H
