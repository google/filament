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

#include "private/backend/SamplerGroup.h"

namespace filament {
namespace backend {

// create a sampler buffer
SamplerGroup::SamplerGroup(size_t count) noexcept : mSize(uint8_t(count)) {
    assert(count <= mBuffer.size());
    // samplers need to be initialized to their default so we can no-op in the driver
    // if the user forgets to set them
    constexpr Sampler prototype{{}, {}};
    std::fill_n(mBuffer.begin(), mSize, prototype);
}

SamplerGroup::SamplerGroup(const SamplerGroup& rhs) noexcept
        : mDirty(rhs.mDirty), mSize(rhs.mSize) {
    std::copy_n(rhs.mBuffer.begin(), rhs.mSize, mBuffer.begin());
}

SamplerGroup::SamplerGroup(SamplerGroup&& rhs) noexcept
        : mDirty(rhs.mDirty), mSize(rhs.mSize) {
    std::copy_n(rhs.mBuffer.begin(), rhs.mSize, mBuffer.begin());
    rhs.clean();
}

SamplerGroup& SamplerGroup::operator=(const SamplerGroup& rhs) noexcept {
    if (this != &rhs) {
        std::copy_n(rhs.mBuffer.begin(), rhs.mSize, mBuffer.begin());
        mDirty = rhs.mDirty;
        mSize = rhs.mSize;
    }
    return *this;
}

SamplerGroup& SamplerGroup::operator=(SamplerGroup&& rhs) noexcept {
    if (this != &rhs) {
        std::copy_n(rhs.mBuffer.begin(), rhs.mSize, mBuffer.begin());
        mDirty = rhs.mDirty;
        mSize = rhs.mSize;
        rhs.clean();
    }
    return *this;
}

SamplerGroup& SamplerGroup::toCommandStream() const noexcept {
    /*
     * This works because our move ctor preserves the data and cleans the dirty flags.
     * if we changed the implementation in the future to do a real move, we'd have to change
     * this method to return SamplerGroup by value, e.g.:
     *    SamplerGroup copy(*this);
     *    this->clean();
     *    return copy;
     */
    return const_cast<SamplerGroup&>(*this);
}

SamplerGroup& SamplerGroup::setSamplers(SamplerGroup const& rhs) noexcept {
    if (this != &rhs) {
        std::copy_n(rhs.mBuffer.begin(), rhs.mSize, mBuffer.begin());
        mDirty.setValue((1u << rhs.mSize) - 1u);
        mSize = rhs.mSize;
    }
    return *this;
}

void SamplerGroup::setSampler(size_t index, Sampler const& sampler) noexcept {
    if (index < mSize) {
        mBuffer[index] = sampler;
        mDirty.set(index);
    }
}

#if !defined(NDEBUG)
utils::io::ostream& operator<<(utils::io::ostream& out, const SamplerGroup& rhs) {
    return out << "SamplerGroup(data=" << rhs.getSamplers() << ", size=" << rhs.getSize() << ")";
}
#endif

} // namespace backend
} // namespace filament
