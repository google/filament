/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKANYCBCRCONVERSIONCACHE_H
#define TNT_FILAMENT_BACKEND_VULKANYCBCRCONVERSIONCACHE_H

#include "vulkan/utils/Definitions.h"

#include <backend/DriverEnums.h>

#include <utils/Hash.h>

#include <bluevk/BlueVK.h>
#include <tsl/robin_map.h>

namespace filament::backend {

// Simple manager for VkSamplerYcbcrConversion objects.
class VulkanYcbcrConversionCache {
public:
    struct Params {
        fvkutils::SamplerYcbcrConversion conversion = {}; // 4
        VkFormat format;                        // 4
        uint64_t externalFormat = 0;            // 8
    };
    static_assert(sizeof(Params) == 16);

    explicit VulkanYcbcrConversionCache(VkDevice device);
    VkSamplerYcbcrConversion getConversion(Params params);
    void terminate() noexcept;

private:
    VkDevice mDevice;

    struct ConversionEqualTo {
        bool operator()(Params lhs, Params rhs) const noexcept {
            fvkutils::SamplerYcbcrConversion::EqualTo equal;
            return equal(lhs.conversion, rhs.conversion) &&
                   lhs.externalFormat == rhs.externalFormat && lhs.format == rhs.format;
        }
    };
    using ConversionHashFn = utils::hash::MurmurHashFn<Params>;
    tsl::robin_map<Params, VkSamplerYcbcrConversion, ConversionHashFn, ConversionEqualTo> mCache;
};

} // namespace filament::backend

#endif// TNT_FILAMENT_BACKEND_VULKANYCBCRCONVERSIONCACHE_H
