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

#include "driver/SamplerBuffer.h"
#include "driver/GPUBuffer.h"

namespace filament {

// create a sampler buffer
SamplerBuffer::SamplerBuffer(size_t count) noexcept : mSize(uint8_t(count)) {
    assert(count <= mBuffer.size());

    // samplers need to be initialized to their default so we can no-op in the driver
    // if the user forgets to set them
    Sampler* const b = mBuffer.data();
    Sampler* const e = b + mSize;
    std::fill(b, e, Sampler{ Handle<HwTexture>{}, SamplerParams{} });
}

SamplerBuffer::SamplerBuffer(const SamplerBuffer& rhs) noexcept
        : mDirty(rhs.mDirty),
          mSize(rhs.mSize) {
    Sampler const *b = rhs.mBuffer.data();
    Sampler const * const e = b + rhs.mSize;
    Sampler* dst = mBuffer.data();
    std::copy(b, e, dst);
}

SamplerBuffer& SamplerBuffer::operator=(const SamplerBuffer& rhs) noexcept {
    if (this != &rhs) {
        mDirty = rhs.mDirty;
        mSize = rhs.mSize;
        Sampler const *b = rhs.mBuffer.data();
        Sampler const * const e = b + rhs.mSize;
        Sampler* dst = mBuffer.data();
        std::copy(b, e, dst);
    }
    return *this;
}

void SamplerBuffer::setSampler(size_t index, Sampler const& sampler) noexcept {
    if (index < mSize) {
        mBuffer[index] = sampler;
        mDirty.set(index);
    }
}

#if !defined(NDEBUG)
utils::io::ostream& operator<<(utils::io::ostream& out, const SamplerBuffer& rhs) {
    return out << "SamplerBuffer(data=" << rhs.getBuffer() << ", size=" << rhs.getSize() << ")";
}
#endif

void SamplerBuffer::setBuffer(size_t index, GPUBuffer const& buffer) noexcept {
    setSampler(index, { buffer.getHandle(), buffer.getSamplerParams() });
}

} // namespace filament
