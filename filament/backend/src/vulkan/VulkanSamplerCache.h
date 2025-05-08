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

#include <backend/DriverEnums.h>

#include <utils/Hash.h>

#include <bluevk/BlueVK.h>
#include <tsl/robin_map.h>

namespace filament::backend {

// Simple manager for VkSampler objects.
class VulkanSamplerCache {
public:
    struct Params {
        SamplerParams sampler = {};
        uint32_t padding = 0;
        VkSamplerYcbcrConversion conversion = VK_NULL_HANDLE;
    };

    static_assert(sizeof(Params) == 16);

    explicit VulkanSamplerCache(VkDevice device);
    VkSampler getSampler(Params params);
    void terminate() noexcept;
private:
    VkDevice mDevice;

    struct SamplerEqualTo {
        bool operator()(Params lhs, Params rhs) const noexcept {
            SamplerParams::EqualTo equal;
            return equal(lhs.sampler, rhs.sampler) && lhs.conversion == rhs.conversion;
        }
    };
    using SamplerHashFn = utils::hash::MurmurHashFn<Params>;
    tsl::robin_map<Params, VkSampler, SamplerHashFn, SamplerEqualTo> mCache;
};

} // namespace filament::backend

#endif // TNT_FILAMENT_BACKEND_VULKANSAMPLERCACHE_H
