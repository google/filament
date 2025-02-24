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

class LayerDispatchTableOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        out = []
        out.append(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
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
        out.append('// NOLINTBEGIN') # Wrap for clang-tidy to ignore
        out.append('// clang-format off')

        out.append('''
#pragma once

typedef PFN_vkVoidFunction (VKAPI_PTR *PFN_GetPhysicalDeviceProcAddr)(VkInstance instance, const char* pName);

// Instance function pointer dispatch table
typedef struct VkLayerInstanceDispatchTable_ {
    PFN_GetPhysicalDeviceProcAddr GetPhysicalDeviceProcAddr;

''')
        guard_helper = PlatformGuardHelper()
        for command in [x for x in self.vk.commands.values() if x.instance]:
            out.extend(guard_helper.add_guard(command.protect))
            out.append(f'    PFN_{command.name} {command.name[2:]};\n')
        out.extend(guard_helper.add_guard(None))
        out.append('} VkLayerInstanceDispatchTable;\n')

        out.append('''
// Device function pointer dispatch table
typedef struct VkLayerDispatchTable_ {
''')
        for command in [x for x in self.vk.commands.values() if x.device]:
            out.extend(guard_helper.add_guard(command.protect))
            out.append(f'    PFN_{command.name} {command.name[2:]};\n')
        out.extend(guard_helper.add_guard(None))
        out.append('} VkLayerDispatchTable;\n')

        out.append('// clang-format on')
        out.append('// NOLINTEND') # Wrap for clang-tidy to ignore
        self.write("".join(out))
