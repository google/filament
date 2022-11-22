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

#include "private/backend/DriverApi.h"

#include "backend/BufferDescriptor.h"

namespace filament::backend {

// create a sampler buffer
SamplerGroup::SamplerGroup(size_t count) noexcept
        : mBuffer(count) {
}

SamplerGroup::SamplerGroup(const SamplerGroup& rhs) noexcept :
    mBuffer(rhs.mBuffer), mDirty(true) {
}

SamplerGroup& SamplerGroup::operator=(const SamplerGroup& rhs) noexcept {
    if (this != &rhs) {
        mBuffer = rhs.mBuffer;
        mDirty = true;
    }
    return *this;
}

void SamplerGroup::setSampler(size_t index, SamplerDescriptor sampler) noexcept {
    if (UTILS_LIKELY(index < mBuffer.size())) {
        // We cannot compare two texture handles to determine if an update is needed. Texture
        // handles are (quickly) recycled and therefore can't be used for that purpose. e.g. if a
        // texture is destroyed, its handle could be reused quickly by another texture.
        // TODO: find a way to avoid marking dirty if the texture does not change.
        mBuffer[index] = sampler;
        mDirty = true;
    }
}

BufferDescriptor SamplerGroup::toBufferDescriptor(DriverApi& driver) const noexcept {
    BufferDescriptor p;
    p.size = mBuffer.size() * sizeof(SamplerDescriptor);
    p.buffer = driver.allocate(p.size); // TODO: use out-of-line buffer if too large
    memcpy(p.buffer, static_cast<const void*>(mBuffer.data()), p.size); // inlined
    clean();
    return p;
}

#if !defined(NDEBUG)
utils::io::ostream& operator<<(utils::io::ostream& out, const SamplerGroup& rhs) {
    return out << "SamplerGroup(size=" << rhs.getSize() << ")";
}
#endif

} // namespace filament::backend
