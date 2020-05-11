// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "vulkan_core.h"

// THIS FILE SHOULD BE DELETED IF VK_GOOGLE_sampler_filtering_precision IS EVER ADDED TO THE VULKAN HEADERS
#ifdef VK_GOOGLE_sampler_filtering_precision
#error "VK_GOOGLE_sampler_filtering_precision is already defined in the Vulkan headers, you can delete this file"
#endif

static constexpr VkStructureType VK_STRUCTURE_TYPE_SAMPLER_FILTERING_PRECISION_GOOGLE = static_cast<VkStructureType>(1000264000);

#define VK_GOOGLE_sampler_filtering_precisions 1
#define VK_GOOGLE_SAMPLER_FILTERING_PRECISION_SPEC_VERSION 1
#define VK_GOOGLE_SAMPLER_FILTERING_PRECISION_EXTENSION_NAME "VK_GOOGLE_sampler_filtering_precision"

typedef enum VkSamplerFilteringPrecisionModeGOOGLE {
    VK_SAMPLER_FILTERING_PRECISION_MODE_LOW_GOOGLE = 0,
    VK_SAMPLER_FILTERING_PRECISION_MODE_HIGH_GOOGLE = 1,
    VK_SAMPLER_FILTERING_PRECISION_MODE_BEGIN_RANGE_GOOGLE = VK_SAMPLER_FILTERING_PRECISION_MODE_LOW_GOOGLE,
    VK_SAMPLER_FILTERING_PRECISION_MODE_END_RANGE_GOOGLE = VK_SAMPLER_FILTERING_PRECISION_MODE_HIGH_GOOGLE,
    VK_SAMPLER_FILTERING_PRECISION_MODE_RANGE_SIZE_GOOGLE = (VK_SAMPLER_FILTERING_PRECISION_MODE_HIGH_GOOGLE - VK_SAMPLER_FILTERING_PRECISION_MODE_LOW_GOOGLE + 1),
    VK_SAMPLER_FILTERING_PRECISION_MODE_MAX_ENUM_GOOGLE = 0x7FFFFFFF
} VkSamplerFilteringPrecisionModeGOOGLE;

typedef struct VkSamplerFilteringPrecisionGOOGLE {
    VkStructureType                       sType;
    const void*                           pNext;
    VkSamplerFilteringPrecisionModeGOOGLE samplerFilteringPrecisionMode;
} VkSamplerFilteringPrecisionGOOGLE;
