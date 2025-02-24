// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See dispatch_table_helper_generator.py for modifications

/***************************************************************************
 *
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2025 Google Inc.
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
 ****************************************************************************/

// NOLINTBEGIN

#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <cstring>
#include <string>
#include "vk_layer_dispatch_table.h"
#include "vk_extension_helper.h"

// Using the above code-generated map of APINames-to-parent extension names, this function will:
//   o  Determine if the API has an associated extension
//   o  If it does, determine if that extension name is present in the passed-in set of device or instance enabled_ext_names
//   If the APIname has no parent extension, OR its parent extension name is IN one of the sets, return TRUE, else FALSE
bool ApiParentExtensionEnabled(const std::string api_name, const DeviceExtensions* device_extension_info);
void layer_init_device_dispatch_table(VkDevice device, VkLayerDispatchTable* table, PFN_vkGetDeviceProcAddr gpa);
void layer_init_instance_dispatch_table(VkInstance instance, VkLayerInstanceDispatchTable* table, PFN_vkGetInstanceProcAddr gpa);
// NOLINTEND
