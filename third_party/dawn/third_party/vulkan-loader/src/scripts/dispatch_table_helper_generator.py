#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2021 The Khronos Group Inc.
# Copyright (c) 2015-2021 Valve Corporation
# Copyright (c) 2015-2021 LunarG, Inc.
# Copyright (c) 2015-2021 Google Inc.
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
#
# Author: Mark Lobodzinski <mark@lunarg.com>
# Author: Charles Giessen <charles@lunarg.com>

import os
from base_generator import BaseGenerator

class DispatchTableHelperGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        out = []

        out.append(f'''#pragma once
// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See {os.path.basename(__file__)} for modifications

/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
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
 *
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Charles Giessen <charles@lunarg.com>
 */

#include <vulkan/vulkan.h>
#include <vulkan/vk_layer.h>
#include <string.h>
#include "vk_layer_dispatch_table.h"

''')

        self.OutputDispatchTableHelper(out, 'device')
        out.append('\n\n')
        self.OutputDispatchTableHelper(out, 'instance')

        self.write(''.join(out))

    # Create a dispatch table from the corresponding table_type and append it to out
    def OutputDispatchTableHelper(self, out: list, table_type: str):
        if table_type == 'device':
            out.append('static inline void layer_init_device_dispatch_table(VkDevice device, VkLayerDispatchTable *table, PFN_vkGetDeviceProcAddr gpa) {\n')
            out.append('    memset(table, 0, sizeof(*table));\n')
            out.append('    table->magic = DEVICE_DISP_TABLE_MAGIC_NUMBER;\n\n')
            out.append('    // Device function pointers\n')
        else:
            out.append('static inline void layer_init_instance_dispatch_table(VkInstance instance, VkLayerInstanceDispatchTable *table, PFN_vkGetInstanceProcAddr gpa) {\n')
            out.append('    memset(table, 0, sizeof(*table));\n\n')
            out.append('    // Instance function pointers\n')

        for command_name, command in self.vk.commands.items():
            if (table_type == 'device' and not command.device) or (table_type == 'instance' and command.device):
                continue

            if command.protect is not None:
                out.append( f'#if defined({command.protect})\n')

            # If we're looking for the proc we are passing in, just point the table to it.  This fixes the issue where
            # a layer overrides the function name for the loader.
            if (table_type == 'device' and command_name == 'vkGetDeviceProcAddr'):
                out.append( '    table->GetDeviceProcAddr = gpa;\n')
            elif (table_type != 'device' and command_name == 'vkGetInstanceProcAddr'):
                out.append( '    table->GetInstanceProcAddr = gpa;\n')
            else:
                out.append( f'    table->{command_name[2:]} = (PFN_{command_name})gpa({table_type}, "{command_name}");\n')
            if command.protect is not None:
                out.append( f'#endif  // {command.protect}\n')
        out.append('}')
