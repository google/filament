/*
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef TNT_FILAMENT_DETAILS_DESCRIPTORSET_H
#define TNT_FILAMENT_DETAILS_DESCRIPTORSET_H

#include "DescriptorSetLayout.h"

#include <private/filament/EngineEnums.h>

#include <backend/DescriptorSetOffsetArray.h>
#include <backend/DriverApiForward.h>
#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/compiler.h>
#include <utils/bitset.h>
#include <utils/FixedCapacityVector.h>

#include <stdint.h>

namespace filament {

class DescriptorSet {
public:
    DescriptorSet() noexcept;
    explicit DescriptorSet(DescriptorSetLayout const& descriptorSetLayout) noexcept;
    DescriptorSet(DescriptorSet const&) = delete;
    DescriptorSet(DescriptorSet&& rhs) noexcept;
    DescriptorSet& operator=(DescriptorSet const&) = delete;
    DescriptorSet& operator=(DescriptorSet&& rhs) noexcept;
    ~DescriptorSet() noexcept;

    void terminate(backend::DriverApi& driver) noexcept;

    // update the descriptors if needed
    void commit(DescriptorSetLayout const& layout, backend::DriverApi& driver) noexcept {
        if (UTILS_UNLIKELY(mDirty.any())) {
            commitSlow(layout, driver);
        }
    }

    void commitSlow(DescriptorSetLayout const& layout, backend::DriverApi& driver) noexcept;

    // bind the descriptor set
    void bind(backend::DriverApi& driver, DescriptorSetBindingPoints set) const noexcept;

    void bind(backend::DriverApi& driver, DescriptorSetBindingPoints set,
            backend::DescriptorSetOffsetArray dynamicOffsets) const noexcept;

    // sets a ubo/ssbo descriptor
    void setBuffer(backend::descriptor_binding_t binding,
            backend::Handle<backend::HwBufferObject> boh,
            uint32_t offset, uint32_t size) noexcept;

    // sets a sampler descriptor
    void setSampler(backend::descriptor_binding_t binding,
            backend::Handle<backend::HwTexture> th,
            backend::SamplerParams params) noexcept;

    // Used for duplicating material
    DescriptorSet duplicate(DescriptorSetLayout const& layout) const noexcept;

    backend::DescriptorSetHandle getHandle() const noexcept {
        return mDescriptorSetHandle;
    }

    utils::bitset64 getValidDescriptors() const noexcept {
        return mValid;
    }

private:
    struct Desc {
        Desc() noexcept { }
        union {
            struct {
                backend::Handle<backend::HwBufferObject> boh;
                uint32_t offset;
                uint32_t size;
            } buffer{};
            struct {
                backend::Handle<backend::HwTexture> th;
                backend::SamplerParams params;
            } texture;
        };
    };

    utils::FixedCapacityVector<Desc> mDescriptors;          // 16
    mutable utils::bitset64 mDirty;                         //  8
    mutable utils::bitset64 mValid;                         //  8
    backend::DescriptorSetHandle mDescriptorSetHandle;      //  4
    mutable bool mSetAfterCommitWarning = false;            //  1
};

} // namespace filament

#endif //TNT_FILAMENT_DETAILS_DESCRIPTORSET_H
