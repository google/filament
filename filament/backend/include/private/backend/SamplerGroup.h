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

#ifndef TNT_FILAMENT_SAMPLERGROUP_H
#define TNT_FILAMENT_SAMPLERGROUP_H

#include <array>
#include <stddef.h>
#include <assert.h>

#include <utils/compiler.h>
#include <utils/bitset.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

namespace filament {
namespace backend {

class SamplerGroup {
public:

    using SamplerParams = backend::SamplerParams;

    struct Sampler {
        Handle<HwTexture> t;
        SamplerParams s{};
    };

    SamplerGroup() noexcept { } // NOLINT

    // create a sampler group
    explicit SamplerGroup(size_t count) noexcept;

    // can be copied -- this preserves dirty bits
    SamplerGroup(const SamplerGroup& rhs) noexcept;
    SamplerGroup& operator=(const SamplerGroup& rhs) noexcept;

    // and moved -- this cleans rhs's dirty flags
    SamplerGroup(SamplerGroup&& rhs) noexcept;
    SamplerGroup& operator=(SamplerGroup&& rhs) noexcept;

    // copy the rhs samplers into this group and sets the dirty flag
    SamplerGroup& setSamplers(SamplerGroup const& rhs) noexcept;

    ~SamplerGroup() noexcept = default;

    // Efficiently move a SamplerGroup to the command stream. Always use std::move() on the
    // returned value, as in the future this might return SamplerGroup by value.
    SamplerGroup& toCommandStream() const noexcept;

    // pointer to the sampler group
    Sampler const* getSamplers() const noexcept { return mBuffer.data(); }

    // sampler count
    size_t getSize() const noexcept { return mBuffer.size(); }

    // return if any samplers has been changed
    bool isDirty() const noexcept { return mDirty.any(); }

    // mark the whole group as clean (no modified uniforms)
    void clean() const noexcept { mDirty.reset(); }

    // set sampler at given index
    void setSampler(size_t index, Sampler const& sampler) noexcept;

    inline void setSampler(size_t index, Handle<HwTexture> t, SamplerParams s)  {
        setSampler(index, { t, s });
    }

private:
#if !defined(NDEBUG)
    friend utils::io::ostream& operator<<(utils::io::ostream& out, const SamplerGroup& rhs);
#endif

    // This could probably be cleaned-up and moved to libutils
    template<class T, size_t N>
    class static_vector { //NOLINT
        typename std::aligned_storage<sizeof(T), alignof(T)>::type mData[N];
        uint32_t mSize = 0;
    public:
        static_vector() = default; //NOLINT

        ~static_vector() noexcept {
            for (auto& elem : *this) {
                elem.~T();
            }
        }

        explicit static_vector(size_t count) noexcept : mSize(count) {
            assert(count < N);
            std::uninitialized_fill_n(begin(), count, T{});
        }

        static_vector(static_vector const& rhs) noexcept : mSize(rhs.mSize) {
            std::uninitialized_copy(rhs.begin(), rhs.end(), begin());
        }

        size_t size() const noexcept { return mSize; }

        T* data() noexcept { return reinterpret_cast<T*>(&mData[0]); }

        T const* data() const noexcept { return reinterpret_cast<T const*>(&mData[0]); }

        static_vector& operator=(static_vector const& rhs) noexcept {
            if (this != &rhs) {
                const size_t n = std::min(mSize, rhs.mSize);
                std::copy_n(rhs.begin(), n, begin());
                for (size_t pos = n, c = mSize; pos < c; ++pos) {
                    data()[pos].~T();
                }
                std::uninitialized_copy(rhs.begin() + n, rhs.end(), begin() + n);
                mSize = rhs.mSize;
            }
            return *this;
        }

        const T& operator[](size_t pos) const noexcept {
            assert(pos < mSize);
            return data()[pos];
        }

        T& operator[](size_t pos) noexcept {
            assert(pos < mSize);
            return data()[pos];
        }

        T* begin() { return data(); }
        T* end() { return data() + mSize; }
        T const* begin() const { return data(); }
        T const* end() const { return data() + mSize; }
    };

    static_vector<Sampler, backend::MAX_SAMPLER_COUNT> mBuffer;    // 128 bytes
    mutable utils::bitset32 mDirty;
};

} // namespace backend
} // namespace filament

#endif // TNT_FILAMENT_SAMPLERGROUP_H
