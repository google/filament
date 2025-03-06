/*
 * Copyright (C) 2018 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKANSAMPLERCACHE_H
#define TNT_FILAMENT_BACKEND_VULKANSAMPLERCACHE_H

#include "VulkanContext.h"

#include <tsl/robin_map.h>

namespace filament::backend {

// Simple manager for VkSampler objects.
class VulkanSamplerCache {
public:
    explicit VulkanSamplerCache(VkDevice device);
    VkSampler getSampler(SamplerParams params) noexcept;
    VkSampler getExternalSampler(SamplerParams spm, SamplerYcbcrConversion ycbcr, uint32_t extFmt) noexcept;
    void      storeExternalSampler(SamplerParams spm, SamplerYcbcrConversion ycbcr, uint32_t extFmt, VkSampler sampler) noexcept;
    void terminate() noexcept;
private:
    VkDevice mDevice;
    tsl::robin_map<SamplerParams, VkSampler, SamplerParams::Hasher, SamplerParams::EqualTo> mCache;
    tsl::robin_map<ExternalSamplerKey, VkSampler, ExternalSamplerKey::Hasher, ExternalSamplerKey::EqualTo> 
        mCacheExternalSamplers;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANSAMPLERCACHE_H
