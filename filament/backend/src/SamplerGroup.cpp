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

void SamplerGroup::setSampler(size_t index, Sampler sampler) noexcept {
    if (UTILS_LIKELY(index < mBuffer.size())) {
        if (mBuffer[index].s.u != sampler.s.u || mBuffer[index].t != sampler.t) {
            // it's a big deal to mutate a sampler group, so we avoid doing it.
            mBuffer[index] = sampler;
            mDirty = true;
        }
    }
}

#if !defined(NDEBUG)
utils::io::ostream& operator<<(utils::io::ostream& out, const SamplerGroup& rhs) {
    return out << "SamplerGroup(data=" << rhs.getSamplers() << ", size=" << rhs.getSize() << ")";
}
#endif

} // namespace filament::backend
