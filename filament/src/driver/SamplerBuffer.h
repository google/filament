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

#ifndef TNT_FILAMENT_DRIVER_SAMPLERBUFFER_H
#define TNT_FILAMENT_DRIVER_SAMPLERBUFFER_H

#include <array>
#include <stddef.h>
#include <assert.h>

#include <utils/compiler.h>
#include <utils/bitset.h>

#include <private/filament/SamplerInterfaceBlock.h>

#include "driver/Handle.h"

namespace filament {

class GPUBuffer;

class SamplerBuffer {
public:

    using SamplerParams = driver::SamplerParams;

    struct Sampler {
        // we use a no-init default ctor so that the mBuffer array doesn't initializes itself
        // needlessly.
        Sampler() noexcept : t(Handle<HwTexture>::NO_INIT), s(SamplerParams::NO_INIT) { }
        Sampler(Handle<HwTexture> t, SamplerParams s) noexcept : t(t), s(s) { }
        Handle<HwTexture> t;
        SamplerParams s;
    };

    SamplerBuffer() noexcept { } // NOLINT(modernize-use-equals-default)

    // create a sampler buffer
    explicit SamplerBuffer(size_t count) noexcept;

    explicit SamplerBuffer(SamplerInterfaceBlock const& sib) noexcept
            : SamplerBuffer(sib.getSize()) { }


    // can be copied and moved
    SamplerBuffer(const SamplerBuffer& rhs) noexcept;

    // don't use copy-and-swap idiom (less efficient)
    SamplerBuffer& operator=(const SamplerBuffer& rhs) noexcept;

    ~SamplerBuffer() noexcept = default;

    // pointer to the sampler buffer
    Sampler const* getBuffer() const noexcept { return mBuffer.data(); }

    // sampler count
    size_t getSize() const noexcept { return mSize; }

    // return if any samplers has been changed
    bool isDirty() const noexcept { return mDirty.any(); }

    // return if the specified sampler  is dirty
    bool isDirty(const SamplerInterfaceBlock::SamplerInfo& info) const noexcept {
        return mDirty[info.offset];
    }

    // mark the whole buffer as clean (no modified uniforms)
    void clean() const noexcept { mDirty.reset(); }

    // get sampler from SamplerInfo
    Sampler const& getSampler(
            const SamplerInterfaceBlock::SamplerInfo& info, size_t index = 0) const {
        return mBuffer[info.offset];
    }

    // set sampler at given index
    void setSampler(size_t index, Sampler const& sampler) noexcept;

    inline void setSampler(size_t index,
            Handle<HwTexture> t, SamplerInterfaceBlock::SamplerParams s)  {
        setSampler(index, { t, s });
    }

    void setBuffer(size_t index, GPUBuffer const& buffer) noexcept;

    // set sampler by SamplerInfo
    void setSampler(const SamplerInterfaceBlock::SamplerInfo& info,
            size_t index, Sampler const& v) {
        setSampler(info.offset, v);
    }

    // set sampler by name
    void setSampler(const SamplerInterfaceBlock& uib,
                    const char* name, size_t index, Sampler const& v) {
        const SamplerInterfaceBlock::SamplerInfo* const info = uib.getSamplerInfo(name);
        if (UTILS_LIKELY(info)) {
            setSampler(*info, index, v);
        }
    }

private:
#if !defined(NDEBUG)
    friend utils::io::ostream& operator<<(utils::io::ostream& out, const SamplerBuffer& rhs);
#endif

    std::array<Sampler, 16> mBuffer;    // 128 bytes
    mutable utils::bitset32 mDirty;
    uint8_t mSize = 0;
};

} // namespace filament

#endif // TNT_FILAMENT_DRIVER_SAMPLERBUFFER_H
