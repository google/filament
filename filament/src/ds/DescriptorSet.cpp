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

#include "DescriptorSet.h"

#include "DescriptorSetLayout.h"

#include "details/Engine.h"

#include <private/filament/EngineEnums.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/Log.h>

#include <utility>
#include <limits>

#include <stdint.h>

namespace filament {

DescriptorSet::DescriptorSet() noexcept = default;

DescriptorSet::~DescriptorSet() noexcept {
    // make sure we're not leaking the descriptor set handle
    assert_invariant(!mDescriptorSetHandle);
}

DescriptorSet::DescriptorSet(DescriptorSetLayout const& descriptorSetLayout) noexcept
        : mDescriptors(descriptorSetLayout.getMaxDescriptorBinding() + 1),
          mDirty(std::numeric_limits<uint64_t>::max()),
          mSetAfterCommitWarning(false) {
}

DescriptorSet::DescriptorSet(DescriptorSet&& rhs) noexcept = default;

DescriptorSet& DescriptorSet::operator=(DescriptorSet&& rhs) noexcept {
    if (this != &rhs) {
        // make sure we're not leaking the descriptor set handle
        assert_invariant(!mDescriptorSetHandle);
        mDescriptors = std::move(rhs.mDescriptors);
        mDescriptorSetHandle = std::move(rhs.mDescriptorSetHandle);
        mDirty = rhs.mDirty;
        mValid = rhs.mValid;
        mSetAfterCommitWarning = rhs.mSetAfterCommitWarning;
    }
    return *this;
}

void DescriptorSet::terminate(FEngine::DriverApi& driver) noexcept {
    if (mDescriptorSetHandle) {
        driver.destroyDescriptorSet(mDescriptorSetHandle);
        mDescriptorSetHandle.clear();
    }
}

void DescriptorSet::commitSlow(DescriptorSetLayout const& layout,
        FEngine::DriverApi& driver) noexcept {
    mDirty.clear();
    // if we have a dirty descriptor set,
    // we need to allocate a new one and reset all the descriptors
    if (UTILS_LIKELY(mDescriptorSetHandle)) {
        // note: if the descriptor-set is bound, doing this will essentially make it dangling.
        // This can result in a use-after-free in the driver if the new one isn't bound at some
        // point later.
        driver.destroyDescriptorSet(mDescriptorSetHandle);
    }
    mDescriptorSetHandle = driver.createDescriptorSet(layout.getHandle());
    mValid.forEachSetBit([&layout, &driver,
            dsh = mDescriptorSetHandle, descriptors = mDescriptors.data()]
            (backend::descriptor_binding_t const binding) {
        if (layout.isSampler(binding)) {
            driver.updateDescriptorSetTexture(dsh, binding,
                    descriptors[binding].texture.th,
                    descriptors[binding].texture.params);
        } else {
            driver.updateDescriptorSetBuffer(dsh, binding,
                    descriptors[binding].buffer.boh,
                    descriptors[binding].buffer.offset,
                    descriptors[binding].buffer.size);
        }
    });
}

void DescriptorSet::bind(FEngine::DriverApi& driver, DescriptorSetBindingPoints set) const noexcept {
    bind(driver, set, {});
}

void DescriptorSet::bind(FEngine::DriverApi& driver, DescriptorSetBindingPoints set,
        backend::DescriptorSetOffsetArray dynamicOffsets) const noexcept {
    // TODO: on debug check that dynamicOffsets is large enough

    assert_invariant(mDescriptorSetHandle);

    // TODO: Make sure clients do the right thing and not change material instance parameters
    // within the renderpass. We have to comment the assert out since it crashed a client's debug
    // build.
    // assert_invariant(mDirty.none());
    if (mDirty.any() && !mSetAfterCommitWarning) {
        mDirty.forEachSetBit([&](uint8_t binding) {
            utils::slog.w << "Descriptor set (handle=" << mDescriptorSetHandle.getId()
                          << ") binding=" << (int) binding
                          << " was set between begin/endRenderPass" << utils::io::endl;
        });
        mSetAfterCommitWarning = true;
    }
    driver.bindDescriptorSet(mDescriptorSetHandle, +set, std::move(dynamicOffsets));
}

void DescriptorSet::setBuffer(
        backend::descriptor_binding_t binding,
        backend::Handle<backend::HwBufferObject> boh, uint32_t offset, uint32_t size) noexcept {
    // TODO: validate it's the right kind of descriptor
    if (mDescriptors[binding].buffer.boh != boh || mDescriptors[binding].buffer.size != size) {
        // we don't set the dirty bit if only offset changes
        mDirty.set(binding);
    }
    mDescriptors[binding].buffer = { boh, offset, size };
    mValid.set(binding, (bool)boh);
}

void DescriptorSet::setSampler(
        backend::descriptor_binding_t binding,
        backend::Handle<backend::HwTexture> th, backend::SamplerParams params) noexcept {
    // TODO: validate it's the right kind of descriptor
    if (mDescriptors[binding].texture.th != th || mDescriptors[binding].texture.params != params) {
        mDirty.set(binding);
    }
    mDescriptors[binding].texture = { th, params };
    mValid.set(binding, (bool)th);
}

DescriptorSet DescriptorSet::duplicate(DescriptorSetLayout const& layout) const noexcept {
    DescriptorSet set{layout};
    set.mDescriptors = mDescriptors; // Use the vector's assignment operator
    set.mDirty = mDirty;
    set.mValid = mValid;
    return set;
}

} // namespace filament
