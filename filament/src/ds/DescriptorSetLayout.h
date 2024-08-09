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

#ifndef TNT_FILAMENT_DESCRIPTORSETLAYOUT_H
#define TNT_FILAMENT_DESCRIPTORSETLAYOUT_H

#include <backend/DriverEnums.h>

#include <backend/DriverApiForward.h>
#include <backend/Handle.h>

#include <utils/bitset.h>

#include <stddef.h>
#include <stdint.h>

namespace filament {

class DescriptorSetLayout {
public:
    DescriptorSetLayout() noexcept;
    DescriptorSetLayout(backend::DriverApi& driver,
            backend::DescriptorSetLayout descriptorSetLayout) noexcept;

    DescriptorSetLayout(DescriptorSetLayout const&) = delete;
    DescriptorSetLayout(DescriptorSetLayout&& rhs) noexcept;
    DescriptorSetLayout& operator=(DescriptorSetLayout const&) = delete;
    DescriptorSetLayout& operator=(DescriptorSetLayout&& rhs) noexcept;

    void terminate(backend::DriverApi& driver) noexcept;

    backend::DescriptorSetLayoutHandle getHandle() const noexcept {
        return mDescriptorSetLayoutHandle;
    }

    size_t getMaxDescriptorBinding() const noexcept {
        return mMaxDescriptorBinding;
    }

    bool isSampler(backend::descriptor_binding_t binding) const noexcept {
        return mSamplers[binding];
    }

    utils::bitset64 getSamplerDescriptors() const noexcept {
        return mSamplers;
    }

    utils::bitset64 getUniformBufferDescriptors() const noexcept {
        return mUniformBuffers;
    }

private:
    backend::DescriptorSetLayoutHandle mDescriptorSetLayoutHandle;
    utils::bitset64 mSamplers;
    utils::bitset64 mUniformBuffers;
    uint8_t mMaxDescriptorBinding = 0;
};


} // namespace filament

#endif //TNT_FILAMENT_DESCRIPTORSETLAYOUT_H
