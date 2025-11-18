/*
 * Copyright (C) 2022 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_VULKANSAMPLERSTATESERIALIZER_H
#define TNT_FILAMENT_BACKEND_VULKANSAMPLERSTATESERIALIZER_H

#include <backend/DriverEnums.h>
#include "VulkanYcbcrConversionCache.h"
#include "VulkanSamplerCache.h"

#include <bluevk/BlueVK.h>
#include <vector>
#include <iostream>
#include <sstream>

namespace filament::backend {

class VulkanYcbcrConversionSerializer {
public:
    VulkanYcbcrConversionSerializer(VulkanYcbcrConversionSerializer const&) = delete;
    VulkanYcbcrConversionSerializer& operator=(VulkanYcbcrConversionSerializer const&) = delete;

    VulkanYcbcrConversionSerializer(const VulkanYcbcrConversionCache::Params& params);
};
        // VulkanPipelineStateSerializer stores to a file the pipeline states.
class VulkanSamplerStateSerializer {
public:
    VulkanSamplerStateSerializer(VulkanSamplerStateSerializer const&) = delete;
    VulkanSamplerStateSerializer& operator=(VulkanSamplerStateSerializer const&) = delete;

    VulkanSamplerStateSerializer(const VulkanSamplerCache::Params& params, uint32_t ycbcr_conv_key);
};
}

#endif //TNT_FILAMENT_BACKEND_VULKANSAMPLERSTATESERIALIZER_H
