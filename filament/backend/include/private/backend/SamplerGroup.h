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

#ifndef TNT_FILAMENT_BACKEND_PRIVATE_SAMPLERGROUP_H
#define TNT_FILAMENT_BACKEND_PRIVATE_SAMPLERGROUP_H

#include <backend/DriverApiForward.h>
#include <backend/DriverEnums.h>
#include <backend/SamplerDescriptor.h>

#include <utils/FixedCapacityVector.h>
#include <utils/ostream.h>

#include <stddef.h>

namespace filament::backend {

class BufferDescriptor;

/*
 * FIXME: this should eventually be moved into Filament, outside of backend.
 *        (but it is currently used by metal/vulkan backens)
 */

class SamplerGroup {
public:

    using SamplerParams = backend::SamplerParams;

    SamplerGroup() noexcept {} // NOLINT

    // create a sampler group
    explicit SamplerGroup(size_t count) noexcept;

    // can be copied. Sets dirty flag.
    SamplerGroup(const SamplerGroup& rhs) noexcept;
    SamplerGroup& operator=(const SamplerGroup& rhs) noexcept;

    // and moved. Leaves rhs empty, keep diry flag on new SamplerGroup.
    SamplerGroup(SamplerGroup&& rhs) noexcept = default;
    SamplerGroup& operator=(SamplerGroup&& rhs) = default;

    ~SamplerGroup() noexcept = default;

    BufferDescriptor toBufferDescriptor(DriverApi& driver) const noexcept;

    // sampler count
    size_t getSize() const noexcept { return mBuffer.size(); }

    // return if any samplers has been changed
    bool isDirty() const noexcept {
        return mDirty;
    }

    // mark the whole group as clean (no modified uniforms)
    void clean() const noexcept { mDirty = false; }

    // set sampler at given index
    void setSampler(size_t index, backend::SamplerDescriptor sampler) noexcept;

    inline void clearSampler(size_t index) {
        setSampler(index, {});
    }


    // FIXME: This is now [[deprecated]]. Currently it is only used by the Vulkan/Metal backends.
    backend::SamplerDescriptor* data() noexcept { return mBuffer.data(); }

private:
#if !defined(NDEBUG)
    friend utils::io::ostream& operator<<(utils::io::ostream& out, const SamplerGroup& rhs);
#endif

    utils::FixedCapacityVector<backend::SamplerDescriptor> mBuffer;
    mutable bool mDirty = false;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_PRIVATE_SAMPLERGROUP_H
