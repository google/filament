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

#ifndef TNT_FILAMENT_UNIFORMBUFFER_H
#define TNT_FILAMENT_UNIFORMBUFFER_H

#include "private/backend/DriverApi.h"

#include <backend/BufferDescriptor.h>

#include <utils/compiler.h>
#include <utils/debug.h>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/quat.h>
#include <math/vec2.h>
#include <math/vec3.h>
#include <math/vec4.h>

#include <type_traits>

#include <stdint.h>
#include <stddef.h>
#include <string.h>

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

    // Can be moved (e.g. assigned from a temporary)
    UniformBuffer& operator=(UniformBuffer&& rhs) noexcept;

    ~UniformBuffer() noexcept {
        // inline this because there is no point in jumping into the library, just to
        // immediately jump into libc's free()
        if (mBuffer && !isLocalStorage()) {
            // test not necessary but avoids a call to libc (and this is a common enough case)
            free(mBuffer, mSize);
        }
    }

    UniformBuffer& setUniforms(const UniformBuffer& rhs) noexcept;

    // Checks if a range needs to be invalidated by comparing the current value.
    template<size_t Size>
    bool invalidateNeeded(size_t const offset, void const* UTILS_RESTRICT v) const {
        assert_invariant(offset + Size <= mSize);
        void* const UTILS_RESTRICT addr = getUniformAddress(offset);
        return bool(memcmp(addr, v, Size)); // inlined
    }

    // Invalidate a range of uniforms and return a pointer to it. offset and size given in bytes
    void invalidateUniforms(size_t const offset, size_t const size) const {
        assert_invariant(offset + size <= mSize);
        mSomethingDirty = true;
    }

    void invalidate() const noexcept {
        invalidateUniforms(0, mSize);
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

    // Note: we purposely not include types that require conversion here
    // (e.g. bool and bool vectors)
    template <typename T>
    struct is_supported_type {
        using type = std::enable_if_t<
                std::is_same_v<float, T> ||
                std::is_same_v<int32_t, T> ||
                std::is_same_v<uint32_t, T> ||
                std::is_same_v<math::quatf, T> ||
                std::is_same_v<math::int2, T> ||
                std::is_same_v<math::int3, T> ||
                std::is_same_v<math::int4, T> ||
                std::is_same_v<math::uint2, T> ||
                std::is_same_v<math::uint3, T> ||
                std::is_same_v<math::uint4, T> ||
                std::is_same_v<math::float2, T> ||
                std::is_same_v<math::float3, T> ||
                std::is_same_v<math::float4, T> ||
                std::is_same_v<math::mat3f, T> ||
                std::is_same_v<math::mat4f, T>
        >;
    };

    // Invalidates an array of uniforms and returns a pointer to the first element.
    //
    // The offset is in bytes, and the count is the number of elements to invalidate.
    // Note that Filament treats arrays of size 1 as being equivalent to a scalar.
    //
    // To compute the size occupied by the array, we account for std140 alignment, which specifies
    // that the start of each array element is aligned to the size of a vec4. Consider an array that
    // has three floats. It would be laid out in memory as follows, where each letter is a 32-bit
    // word:
    //
    //      a x x x b x x x c
    //
    // The "x" symbols represent dummy words.
    template<typename T, typename = typename is_supported_type<T>::type>
    UTILS_ALWAYS_INLINE
    void setUniformArray(size_t const offset, T const* UTILS_RESTRICT begin, size_t const count) noexcept {
        static_assert(!std::is_same_v<T, math::mat3f>);
        setUniformArrayUntyped<sizeof(T)>(offset, begin, count);
    }

    // (see specialization for mat3f below)
    template<typename T, typename = typename is_supported_type<T>::type>
    UTILS_ALWAYS_INLINE
    static void setUniform(void* addr, const T& v) noexcept {
        static_assert(!std::is_same_v<T, math::mat3f>);
        setUniformUntyped<sizeof(T)>(addr, &v);
    }

    template<typename T, typename = typename is_supported_type<T>::type>
    UTILS_ALWAYS_INLINE
    void setUniform(size_t const offset, const T& v) noexcept {
        static_assert(!std::is_same_v<T, math::mat3f>);
        setUniformUntyped<sizeof(T)>(offset, &v);
    }

    // get uniform of known types from the proper offset (e.g.: use offsetof())
    template<typename T, typename = typename is_supported_type<T>::type>
    T getUniform(size_t const offset) const noexcept {
        static_assert(!std::is_same_v<T, math::mat3f>);
        return *reinterpret_cast<T const*>(static_cast<char const*>(mBuffer) + offset);
    }

    // helper functions

    backend::BufferDescriptor toBufferDescriptor(backend::DriverApi& driver) const noexcept {
        return toBufferDescriptor(driver, 0, getSize());
    }

    // copy the UBO data and cleans the dirty bits
    backend::BufferDescriptor toBufferDescriptor(
            backend::DriverApi& driver, size_t const offset, size_t const size) const noexcept {
        backend::BufferDescriptor p;
        p.size = size;
        p.buffer = driver.allocate(p.size); // TODO: use out-of-line buffer if too large
        memcpy(p.buffer, static_cast<const char*>(getBuffer()) + offset, p.size); // inlined
        clean();
        return p;
    }

    // set uniform of known types to the proper offset (e.g.: use offsetof())
    template<size_t Size>
    void setUniformUntyped(size_t offset, void const* UTILS_RESTRICT v) noexcept;

    template<size_t Size>
    void setUniformArrayUntyped(size_t offset, void const* UTILS_RESTRICT begin, size_t count) noexcept;

private:

#if !defined(NDEBUG)
    friend utils::io::ostream& operator<<(utils::io::ostream& out, const UniformBuffer& rhs);
#endif
    static void* alloc(size_t size) noexcept;
    static void free(void* addr, size_t size) noexcept;

    template<size_t Size, std::enable_if_t<
            Size == 4 || Size == 8 || Size == 12 || Size == 16 || Size == 64, bool> = true>
    UTILS_ALWAYS_INLINE
    static void setUniformUntyped(void* UTILS_RESTRICT addr, void const* v) noexcept {
        memcpy(addr, v, Size); // inlined
    }

    bool isLocalStorage() const noexcept { return mBuffer == mStorage; }

    void* getUniformAddress(size_t const offset) const noexcept {
        return static_cast<char*>(mBuffer) + offset;
    }

    char mStorage[96]; // 6 lines (6 x vec4 x 4)
    void *mBuffer = nullptr;
    uint32_t mSize = 0;
    mutable bool mSomethingDirty = false;
    // we have 3 padding bytes here
};

// Specialization for mat3f (which has a different alignment, see std140 layout rules), we declare it
// but don't define it, so that we are sure it's never called.
template<>
void UniformBuffer::setUniform(void* addr, const math::mat3f& v) noexcept;

// The specialization for mat3f (which has a different alignment, see std140 layout rules) is handled as
// an array of three float3 (which therefore have an alignment of 16)
template<>
inline void UniformBuffer::setUniform(size_t const offset, const math::mat3f& v) noexcept {
    setUniformArrayUntyped<sizeof(math::float3)>(offset, &v, 3);
}

template<>
inline void UniformBuffer::setUniformArray(
        size_t const offset, math::mat3f const* UTILS_RESTRICT begin, size_t const count) noexcept {
    // pretend each mat3 is an array of 3 float3
    setUniformArray(offset, reinterpret_cast<math::float3 const*>(begin), count * 3);
}

template<>
inline math::mat3f UniformBuffer::getUniform(size_t const offset) const noexcept {
    math::float4 const* p = reinterpret_cast<math::float4 const*>(
            static_cast<char const*>(mBuffer) + offset);
    return { p[0].xyz, p[1].xyz, p[2].xyz };
}

} // namespace filament

#endif // TNT_FILAMENT_UNIFORMBUFFER_H
