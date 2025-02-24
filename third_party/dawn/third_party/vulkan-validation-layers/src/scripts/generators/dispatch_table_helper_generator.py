#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2025 The Khronos Group Inc.
# Copyright (c) 2015-2025 Valve Corporation
# Copyright (c) 2015-2025 LunarG, Inc.
# Copyright (c) 2015-2025 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
from generators.base_generator import BaseGenerator
from generators.generator_utils import PlatformGuardHelper

class DispatchTableHelperOutputGenerator(BaseGenerator):
    """Generate dispatch tables header based on XML element attributes"""
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

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
            ****************************************************************************/\n''')
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        if self.filename == 'vk_dispatch_table_helper.h':
            self.generateHeader()
        elif self.filename == 'vk_dispatch_table_helper.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once

            #include <vulkan/vulkan.h>
            #include <vulkan/vk_layer.h>
            #include <cstring>
            #include <string>
            #include "vk_layer_dispatch_table.h"
            #include "vk_extension_helper.h"
            \n''')

        out.append('''
            // Using the above code-generated map of APINames-to-parent extension names, this function will:
            //   o  Determine if the API has an associated extension
            //   o  If it does, determine if that extension name is present in the passed-in set of device or instance enabled_ext_names
            //   If the APIname has no parent extension, OR its parent extension name is IN one of the sets, return TRUE, else FALSE
            bool ApiParentExtensionEnabled(const std::string api_name, const DeviceExtensions* device_extension_info);''')

        out.append('void layer_init_device_dispatch_table(VkDevice device, VkLayerDispatchTable* table, PFN_vkGetDeviceProcAddr gpa);')

        out.append('void layer_init_instance_dispatch_table(VkInstance instance, VkLayerInstanceDispatchTable *table, PFN_vkGetInstanceProcAddr gpa);')
        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('#include "vk_dispatch_table_helper.h"\n')

        guard_helper = PlatformGuardHelper()

        for command in [x for x in self.vk.commands.values() if x.extensions or x.version]:
            if command.name == 'vkEnumerateInstanceVersion':
                continue # TODO - Figure out how this can be automatically detected
            out.extend(guard_helper.add_guard(command.protect))

            prototype = ' '.join(command.cPrototype.split()) # remove duplicate whitespace
            prototype = prototype.replace('\n', '').replace('( ', '(').replace(');', ')').replace(' vk', ' Stub')
            # Remove the parameter names so that we don't get any unreferenced parameter warnings
            for param in command.params:
                prototype = prototype.replace(f'{param.name},', ',').replace(f'{param.name})', ')').replace(f' {param.name}[', '[')

            result = '' if command.returnType == 'void' else 'return 0;'
            result = 'return VK_SUCCESS;' if command.returnType == 'VkResult' else result
            result = 'return VK_FALSE;' if command.returnType == 'VkBool32' else result

            out.append(f'static {prototype} {{ {result} }}\n')
        out.extend(guard_helper.add_guard(None))
        out.append('\n')

        out.append('const auto &GetApiPromotedMap() {\n')
        out.append('    static const vvl::unordered_map<std::string, std::string> api_promoted_map {\n')
        for command in [x for x in self.vk.commands.values() if x.version and x.device]:
            out.append(f'    {{ "{command.name}", {{ "{command.version.name}" }} }},\n')
        out.append('    };\n')
        out.append('    return api_promoted_map;\n')
        out.append('}\n')

        out.append('const auto &GetApiExtensionMap() {\n')
        out.append('    static const vvl::unordered_map<std::string, small_vector<vvl::Extension, 2, size_t>> api_extension_map {\n')
        for command in [x for x in self.vk.commands.values() if x.extensions and x.device]:
            extensions = ', '.join(f'vvl::Extension::_{x.name}' for x in command.extensions)
            out.append(f'    {{ "{command.name}", {{ {extensions} }} }},\n')
        out.append('    };\n')
        out.append('    return api_extension_map;\n')
        out.append('}\n')

        out.append('''
            // Using the above code-generated map of APINames-to-parent extension names, this function will:
            //   o  Determine if the API has an associated extension
            //   o  If it does, determine if that extension name is present in the passed-in set of device or instance enabled_ext_names
            //   If the APIname has no parent extension, OR its parent extension name is IN one of the sets, return TRUE, else FALSE
            bool ApiParentExtensionEnabled(const std::string api_name, const DeviceExtensions* device_extension_info) {
                auto promoted_api = GetApiPromotedMap().find(api_name);
                if (promoted_api != GetApiPromotedMap().end()) {
                    auto info = GetDeviceVersionMap(promoted_api->second.c_str());
                    assert(info.state);
                    return (device_extension_info->*(info.state) == kEnabledByCreateinfo);
                }

                auto has_ext = GetApiExtensionMap().find(api_name);
                // Is this API part of an extension or feature group?
                if (has_ext != GetApiExtensionMap().end()) {
                    // Was the extension for this API enabled in the CreateDevice call?
                    for (const auto& extension : has_ext->second) {
                        auto info = device_extension_info->GetInfo(extension);
                        if (info.state) {
                            if (device_extension_info->*(info.state) == kEnabledByCreateinfo ||
                                device_extension_info->*(info.state) == kEnabledByInteraction) {
                                return true;
                            }
                        }
                    }

                    // Was the extension for this API enabled in the CreateInstance call?
                    auto instance_extension_info = static_cast<const InstanceExtensions*>(device_extension_info);
                    for (const auto& extension : has_ext->second) {
                        auto info = instance_extension_info->GetInfo(extension);
                        if (info.state) {
                            if (instance_extension_info->*(info.state) == kEnabledByCreateinfo ||
                                instance_extension_info->*(info.state) == kEnabledByInteraction) {
                                return true;
                            }
                        }
                    }
                    return false;
                }
                return true;
            }
            ''')
        out.append('''
            void layer_init_device_dispatch_table(VkDevice device, VkLayerDispatchTable* table, PFN_vkGetDeviceProcAddr gpa) {
                memset(table, 0, sizeof(*table));
                // Device function pointers
                table->GetDeviceProcAddr = gpa;
            ''')
        for command in [x for x in self.vk.commands.values() if x.device and x.name != 'vkGetDeviceProcAddr']:
            out.extend(guard_helper.add_guard(command.protect))
            out.append(f'    table->{command.name[2:]} = (PFN_{command.name}) gpa(device, "{command.name}");\n')
            if command.version or command.extensions:
                out.append(f'    if (table->{command.name[2:]} == nullptr) {{ table->{command.name[2:]} = (PFN_{command.name})Stub{command.name[2:]}; }}\n')
        out.extend(guard_helper.add_guard(None))
        out.append('}\n')

        out.append('''
            void layer_init_instance_dispatch_table(VkInstance instance, VkLayerInstanceDispatchTable *table, PFN_vkGetInstanceProcAddr gpa) {
                memset(table, 0, sizeof(*table));
                // Instance function pointers
                table->GetInstanceProcAddr = gpa;
                table->GetPhysicalDeviceProcAddr = (PFN_GetPhysicalDeviceProcAddr) gpa(instance, "vk_layerGetPhysicalDeviceProcAddr");
            ''')
        ignoreList = [
              'vkCreateInstance',
              'vkCreateDevice',
              'vkGetPhysicalDeviceProcAddr',
              'vkEnumerateInstanceExtensionProperties',
              'vkEnumerateInstanceLayerProperties',
              'vkEnumerateInstanceVersion',
              'vkGetInstanceProcAddr',
        ]
        for command in [x for x in self.vk.commands.values() if x.instance and x.name not in ignoreList]:
            out.extend(guard_helper.add_guard(command.protect))
            out.append(f'    table->{command.name[2:]} = (PFN_{command.name}) gpa(instance, "{command.name}");\n')
            if command.version or command.extensions:
                out.append(f'    if (table->{command.name[2:]} == nullptr) {{ table->{command.name[2:]} = (PFN_{command.name})Stub{command.name[2:]}; }}\n')
        out.extend(guard_helper.add_guard(None))
        out.append('}\n')

        self.write("".join(out))
