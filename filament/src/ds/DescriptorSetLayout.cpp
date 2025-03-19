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

#include "DescriptorSetLayout.h"

#include "HwDescriptorSetLayoutFactory.h"

#include "details/Engine.h"

#include <backend/DriverEnums.h>

#include <algorithm>
#include <utility>

namespace filament {

DescriptorSetLayout::DescriptorSetLayout() noexcept = default;

DescriptorSetLayout::DescriptorSetLayout(
        HwDescriptorSetLayoutFactory& factory,
        backend::DriverApi& driver,
        backend::DescriptorSetLayout descriptorSetLayout) noexcept  {
    for (auto&& desc : descriptorSetLayout.bindings) {
        mMaxDescriptorBinding = std::max(mMaxDescriptorBinding, desc.binding);
        mSamplers.set(desc.binding, desc.type == backend::DescriptorType::SAMPLER);
        mUniformBuffers.set(desc.binding,
                desc.type == backend::DescriptorType::UNIFORM_BUFFER);

        if (desc.type == backend::DescriptorType::SAMPLER_EXTERNAL) {
            mSamplerExternals.set(desc.binding, true);
            auto& sampler = mImmutables[desc.binding];
            assert_invariant(
                    desc.externalSamplerDataIndex != backend::EXTERNAL_SAMPLER_DATA_INDEX_UNUSED);
            auto& extSamplerData =
                    descriptorSetLayout.externalSamplerData[desc.externalSamplerDataIndex];
            sampler.sampler.conversion = extSamplerData.YcbcrConversion;
            sampler.sampler.params = extSamplerData.samplerParams;
            sampler.sampler.internalFormat = extSamplerData.externalFormat;
        }
    }

    mDescriptorSetLayoutHandle = factory.create(driver,
            std::move(descriptorSetLayout));
}

void DescriptorSetLayout::terminate(
        HwDescriptorSetLayoutFactory& factory,
        backend::DriverApi& driver) noexcept {
    if (mDescriptorSetLayoutHandle) {
        factory.destroy(driver, mDescriptorSetLayoutHandle);
    }
}

DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& rhs) noexcept = default;

DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& rhs) noexcept = default;

} // namespace filament
