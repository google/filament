#!/usr/bin/python3 -i
#
# Copyright 2023 The Khronos Group Inc.
# Copyright 2023 Valve Corporation
# Copyright 2023 LunarG, Inc.
#
# SPDX-License-Identifier: Apache-2.0

import os
from base_generator import BaseGenerator
from generators.generator_utils import PlatformGuardHelper

class DispatchTableOutputGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        out = []
        out.append(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See {os.path.basename(__file__)} for modifications
// Copyright 2023 The Khronos Group Inc.
// Copyright 2023 Valve Corporation
// Copyright 2023 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
''')

        out.append('''
#pragma once

#include <vulkan/vulkan.h>

#include <string.h>

// clang-format off

typedef PFN_vkVoidFunction(VKAPI_PTR *PFN_GetPhysicalDeviceProcAddr)(VkInstance instance, const char *pName);
''')
        out.append('''
// Instance function pointer dispatch table
typedef struct VkuInstanceDispatchTable_ {
    PFN_GetPhysicalDeviceProcAddr GetPhysicalDeviceProcAddr;

''')
        guard_helper = PlatformGuardHelper()
        for command in [x for x in self.vk.commands.values() if x.instance]:
            out.extend(guard_helper.add_guard(command.protect))
            out.append(f'    PFN_{command.name} {command.name[2:]};\n')
        out.extend(guard_helper.add_guard(None))
        out.append('} VkuInstanceDispatchTable;\n')

        out.append('''
// Device function pointer dispatch table
typedef struct VkuDeviceDispatchTable_ {
''')
        for command in [x for x in self.vk.commands.values() if x.device]:
            out.extend(guard_helper.add_guard(command.protect))
            out.append(f'    PFN_{command.name} {command.name[2:]};\n')
        out.extend(guard_helper.add_guard(None))
        out.append('} VkuDeviceDispatchTable;\n')

        out.append('''
static inline void vkuInitDeviceDispatchTable(VkDevice device, VkuDeviceDispatchTable *table, PFN_vkGetDeviceProcAddr gdpa) {
    memset(table, 0, sizeof(*table));
    // Device function pointers
    table->GetDeviceProcAddr = gdpa;
''')

        for command in [x for x in self.vk.commands.values() if x.device and x.name != 'vkGetDeviceProcAddr']:
            out.extend(guard_helper.add_guard(command.protect))
            out.append(f'    table->{command.name[2:]} = (PFN_{command.name})gdpa(device, "{command.name}");\n')
        out.extend(guard_helper.add_guard(None))
        out.append('}\n')

        out.append('''
static inline void vkuInitInstanceDispatchTable(VkInstance instance, VkuInstanceDispatchTable *table, PFN_vkGetInstanceProcAddr gipa) {
    memset(table, 0, sizeof(*table));
    // Instance function pointers
    table->GetInstanceProcAddr = gipa;
    table->GetPhysicalDeviceProcAddr = (PFN_GetPhysicalDeviceProcAddr)gipa(instance, "vk_layerGetPhysicalDeviceProcAddr");
''')

        for command in [x for x in self.vk.commands.values() if x.instance and x.name not in [
                'vkCreateInstance',
                'vkCreateDevice',
                'vkGetPhysicalDeviceProcAddr',
                'vkEnumerateInstanceExtensionProperties',
                'vkEnumerateInstanceLayerProperties',
                'vkEnumerateInstanceVersion',
                'vkGetInstanceProcAddr',
        ]]:
            out.extend(guard_helper.add_guard(command.protect))
            out.append(f'    table->{command.name[2:]} = (PFN_{command.name})gipa(instance, "{command.name}");\n')
        out.extend(guard_helper.add_guard(None))
        out.append('}\n')

        out.append('// clang-format on')

        self.write("".join(out))
