/* Copyright (c) 2015-2020 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstring>
#include "vk_layer_extension_utils.h"

VkResult util_GetExtensionProperties(const uint32_t count, const VkExtensionProperties *layer_extensions, uint32_t *pCount,
                                     VkExtensionProperties *pProperties) {
    if (pProperties == nullptr || layer_extensions == nullptr) {
        *pCount = count;
        return VK_SUCCESS;
    }

    const uint32_t copy_size = *pCount < count ? *pCount : count;
    std::memcpy(pProperties, layer_extensions, copy_size * sizeof(VkExtensionProperties));
    *pCount = copy_size;
    if (copy_size < count) {
        return VK_INCOMPLETE;
    }

    return VK_SUCCESS;
}

VkResult util_GetLayerProperties(const uint32_t count, const VkLayerProperties *layer_properties, uint32_t *pCount,
                                 VkLayerProperties *pProperties) {
    if (pProperties == nullptr || layer_properties == nullptr) {
        *pCount = count;
        return VK_SUCCESS;
    }

    const uint32_t copy_size = *pCount < count ? *pCount : count;
    std::memcpy(pProperties, layer_properties, copy_size * sizeof(VkLayerProperties));
    *pCount = copy_size;
    if (copy_size < count) {
        return VK_INCOMPLETE;
    }

    return VK_SUCCESS;
}
