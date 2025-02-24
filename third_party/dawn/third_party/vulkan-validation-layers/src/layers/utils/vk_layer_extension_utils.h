/* Copyright (c) 2015-2016 The Khronos Group Inc.
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
#pragma once

#include "vulkan/vulkan_core.h"

VkResult util_GetExtensionProperties(const uint32_t count, const VkExtensionProperties *layer_extensions, uint32_t *pCount,
                                     VkExtensionProperties *pProperties);

VkResult util_GetLayerProperties(const uint32_t count, const VkLayerProperties *layer_properties, uint32_t *pCount,
                                 VkLayerProperties *pProperties);
