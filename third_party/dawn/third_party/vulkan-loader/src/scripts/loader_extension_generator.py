#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2022 The Khronos Group Inc.
# Copyright (c) 2015-2022 Valve Corporation
# Copyright (c) 2015-2022 LunarG, Inc.
# Copyright (c) 2015-2017 Google Inc.
# Copyright (c) 2021-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# Copyright (c) 2023-2023 RasterGrid Kft.
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
# Author: Mark Young <marky@lunarg.com>
# Author: Mark Lobodzinski <mark@lunarg.com>
# Author: Charles Giessen <charles@lunarg.com>

import re
import os
from base_generator import BaseGenerator

WSI_EXT_NAMES = ['VK_KHR_surface',
                 'VK_KHR_display',
                 'VK_KHR_xlib_surface',
                 'VK_KHR_xcb_surface',
                 'VK_KHR_wayland_surface',
                 'VK_EXT_directfb_surface',
                 'VK_KHR_win32_surface',
                 'VK_KHR_android_surface',
                 'VK_GGP_stream_descriptor_surface',
                 'VK_MVK_macos_surface',
                 'VK_MVK_ios_surface',
                 'VK_EXT_headless_surface',
                 'VK_EXT_metal_surface',
                 'VK_FUCHSIA_imagepipe_surface',
                 'VK_KHR_swapchain',
                 'VK_KHR_display_swapchain',
                 'VK_KHR_get_display_properties2',
                 'VK_KHR_get_surface_capabilities2',
                 'VK_QNX_screen_surface',
                 'VK_NN_vi_surface']

ADD_INST_CMDS = ['vkCreateInstance',
                 'vkEnumerateInstanceExtensionProperties',
                 'vkEnumerateInstanceLayerProperties',
                 'vkEnumerateInstanceVersion']

AVOID_EXT_NAMES = ['VK_EXT_debug_report']

NULL_CHECK_EXT_NAMES= ['VK_EXT_debug_utils']

AVOID_CMD_NAMES = ['vkCreateDebugUtilsMessengerEXT',
                   'vkDestroyDebugUtilsMessengerEXT',
                   'vkSubmitDebugUtilsMessageEXT']

DEVICE_CMDS_NEED_TERM = ['vkGetDeviceProcAddr',
                         'vkCreateSwapchainKHR',
                         'vkCreateSharedSwapchainsKHR',
                         'vkGetDeviceGroupSurfacePresentModesKHR',
                         'vkDebugMarkerSetObjectTagEXT',
                         'vkDebugMarkerSetObjectNameEXT',
                         'vkSetDebugUtilsObjectNameEXT',
                         'vkSetDebugUtilsObjectTagEXT',
                         'vkQueueBeginDebugUtilsLabelEXT',
                         'vkQueueEndDebugUtilsLabelEXT',
                         'vkQueueInsertDebugUtilsLabelEXT',
                         'vkCmdBeginDebugUtilsLabelEXT',
                         'vkCmdEndDebugUtilsLabelEXT',
                         'vkCmdInsertDebugUtilsLabelEXT',
                         'vkGetDeviceGroupSurfacePresentModes2EXT']

DEVICE_CMDS_MUST_USE_TRAMP = ['vkSetDebugUtilsObjectNameEXT',
                              'vkSetDebugUtilsObjectTagEXT',
                              'vkDebugMarkerSetObjectNameEXT',
                              'vkDebugMarkerSetObjectTagEXT']

# These are the aliased functions that use the same terminator for both extension and core versions
# Generally, this is only applies to physical device level functions in instance extensions
SHARED_ALIASES = {
    # 1.1 aliases
    'vkEnumeratePhysicalDeviceGroupsKHR':                   'vkEnumeratePhysicalDeviceGroups',
    'vkGetPhysicalDeviceFeatures2KHR':                      'vkGetPhysicalDeviceFeatures2',
    'vkGetPhysicalDeviceProperties2KHR':                    'vkGetPhysicalDeviceProperties2',
    'vkGetPhysicalDeviceFormatProperties2KHR':              'vkGetPhysicalDeviceFormatProperties2',
    'vkGetPhysicalDeviceImageFormatProperties2KHR':         'vkGetPhysicalDeviceImageFormatProperties2',
    'vkGetPhysicalDeviceQueueFamilyProperties2KHR':         'vkGetPhysicalDeviceQueueFamilyProperties2',
    'vkGetPhysicalDeviceMemoryProperties2KHR':              'vkGetPhysicalDeviceMemoryProperties2',
    'vkGetPhysicalDeviceSparseImageFormatProperties2KHR':   'vkGetPhysicalDeviceSparseImageFormatProperties2',
    'vkGetPhysicalDeviceExternalBufferPropertiesKHR':       'vkGetPhysicalDeviceExternalBufferProperties',
    'vkGetPhysicalDeviceExternalSemaphorePropertiesKHR':    'vkGetPhysicalDeviceExternalSemaphoreProperties',
    'vkGetPhysicalDeviceExternalFencePropertiesKHR':        'vkGetPhysicalDeviceExternalFenceProperties',
}

PRE_INSTANCE_FUNCTIONS = ['vkEnumerateInstanceExtensionProperties',
                          'vkEnumerateInstanceLayerProperties',
                          'vkEnumerateInstanceVersion']

class LoaderExtensionGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

        self.core_commands = []
        self.extension_commands = []
        self.instance_extensions = []

    def generate(self):

        self.core_commands = [x for x in self.vk.commands.values() if len(x.extensions) == 0]
        self.extension_commands = [x for x in self.vk.commands.values() if len(x.extensions) > 0]

        self.instance_extensions = [x for x in self.vk.extensions.values() if x.instance]

        out = []
        self.add_preamble(out)

        if self.filename == 'vk_loader_extensions.h':
            self.print_vk_loader_extensions_h(out)
        elif self.filename == 'vk_loader_extensions.c':
            self.print_vk_loader_extensions_c(out)
        elif self.filename == 'vk_layer_dispatch_table.h':
            self.print_vk_layer_dispatch_table(out)

        out.append('// clang-format on')

        self.write(''.join(out))

    def add_preamble(self, out):
        out.append(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See {os.path.basename(__file__)} for modifications

/*
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2021-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * Copyright (c) 2023-2023 RasterGrid Kft.
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
 * Author: Mark Lobodzinski <mark@lunarg.com>
 * Author: Mark Young <marky@lunarg.com>
 * Author: Charles Giessen <charles@lunarg.com>
 */

// clang-format off
''')

    def print_vk_loader_extensions_h(self, out):
        out.append('#pragma once\n')
        out.append('\n')
        out.append('#include <stdbool.h>\n')
        out.append('#include <vulkan/vulkan.h>\n')
        out.append('#include <vulkan/vk_layer.h>\n')
        out.append('#include "vk_layer_dispatch_table.h"\n')
        out.append('\n\n')

        self.OutputPrototypesInHeader(out)
        self.OutputLoaderTerminators(out)
        self.OutputIcdDispatchTable(out)
        self.OutputIcdExtensionEnableUnion(out)
        self.OutputDeviceFunctionTerminatorDispatchTable(out)


    def print_vk_loader_extensions_c(self, out):
        out.append('#include <stdio.h>\n')
        out.append('#include <stdlib.h>\n')
        out.append('#include <string.h>\n')
        out.append('#include "loader.h"\n')
        out.append('#include "vk_loader_extensions.h"\n')
        out.append('#include <vulkan/vk_icd.h>\n')
        out.append('#include "wsi.h"\n')
        out.append('#include "debug_utils.h"\n')
        out.append('#include "extension_manual.h"\n')
        self.OutputUtilitiesInSource(out)
        self.OutputIcdDispatchTableInit(out)
        self.OutputLoaderDispatchTables(out)
        self.InitDeviceFunctionTerminatorDispatchTable(out)
        self.OutputDeviceFunctionTrampolinePrototypes(out)
        self.OutputLoaderLookupFunc(out)
        self.CreateTrampTermFuncs(out)
        self.InstExtensionGPA(out)
        self.InstantExtensionCreate(out)
        self.DeviceExtensionGetTerminator(out)
        self.InitInstLoaderExtensionDispatchTable(out)
        self.OutputInstantExtensionWhitelistArray(out)

    def print_vk_layer_dispatch_table(self, out):
        out.append('#pragma once\n')
        out.append('\n')
        out.append('#include <vulkan/vulkan.h>\n')
        out.append('\n')
        out.append('#if !defined(PFN_GetPhysicalDeviceProcAddr)\n')
        out.append('typedef PFN_vkVoidFunction (VKAPI_PTR *PFN_GetPhysicalDeviceProcAddr)(VkInstance instance, const char* pName);\n')
        out.append('#endif\n\n')
        self.OutputLayerInstanceDispatchTable(out)
        self.OutputLayerDeviceDispatchTable(out)

    # Convert an XML dependency expression to a C expression, taking a callback to replace extension names
    # See https://registry.khronos.org/vulkan/specs/1.4/registry.html#depends-expressions
    @staticmethod
    def ConvertDependencyExpression(expr, replace_func):
        # '(' and ')' can pass through unchanged
        expr = re.sub(',', ' || ', expr)
        expr = re.sub(r'\+', ' && ', expr)
        expr = re.sub(r'\w+', lambda match: replace_func(match.group()), expr)
        return expr

    def DescribeBlock(self, command, current_block, out, custom_commands_string = ' commands', indent = '    '):
        if command.extensions != current_block and command.version != current_block:
            if command.version is None and len(command.extensions) == 0: # special case for 1.0
                out.append(f'\n{indent}// ---- Core Vulkan 1.0{custom_commands_string}\n')
                return None
            elif command.version is not None:
                if command.version != current_block:
                    out.append(f"\n{indent}// ---- Core Vulkan 1.{command.version.name.split('_')[-1]}{custom_commands_string}\n")
                return command.version
            else:
                # don't repeat unless the first extension is different (while rest can vary)
                if not isinstance(current_block, list) or current_block[0].name != command.extensions[0].name:
                    out.append(f"\n{indent}// ---- {command.extensions[0].name if len(command.extensions) > 0 else ''} extension{custom_commands_string}\n")
                return command.extensions
        else:
            return current_block

    def OutputPrototypesInHeader(self, out: list):
        out.append('''// Structures defined externally, but used here
struct loader_instance;
struct loader_device;
struct loader_icd_term;
struct loader_dev_dispatch_table;

// Device extension error function
VKAPI_ATTR VkResult VKAPI_CALL vkDevExtError(VkDevice dev);

// Extension interception for vkGetInstanceProcAddr function, so we can return
// the appropriate information for any instance extensions we know about.
bool extension_instance_gpa(struct loader_instance *ptr_instance, const char *name, void **addr);

// Extension interception for vkCreateInstance function, so we can properly
// detect and enable any instance extension information for extensions we know
// about.
void extensions_create_instance(struct loader_instance *ptr_instance, const VkInstanceCreateInfo *pCreateInfo);

// Extension interception for vkGetDeviceProcAddr function, so we can return
// an appropriate terminator if this is one of those few device commands requiring
// a terminator.
PFN_vkVoidFunction get_extension_device_proc_terminator(struct loader_device *dev, const char *name, bool* found_name);

// Dispatch table properly filled in with appropriate terminators for the
// supported extensions.
extern const VkLayerInstanceDispatchTable instance_disp;

// Array of extension strings for instance extensions we support.
extern const char *const LOADER_INSTANCE_EXTENSIONS[];

VKAPI_ATTR bool VKAPI_CALL loader_icd_init_entries(struct loader_instance* inst, struct loader_icd_term *icd_term);

// Init Device function pointer dispatch table with core commands
VKAPI_ATTR void VKAPI_CALL loader_init_device_dispatch_table(struct loader_dev_dispatch_table *dev_table, PFN_vkGetDeviceProcAddr gpa,
                                                             VkDevice dev);

// Init Device function pointer dispatch table with extension commands
VKAPI_ATTR void VKAPI_CALL loader_init_device_extension_dispatch_table(struct loader_dev_dispatch_table *dev_table,
                                                                       PFN_vkGetInstanceProcAddr gipa,
                                                                       PFN_vkGetDeviceProcAddr gdpa,
                                                                       VkInstance inst,
                                                                       VkDevice dev);

// Init Instance function pointer dispatch table with core commands
VKAPI_ATTR void VKAPI_CALL loader_init_instance_core_dispatch_table(VkLayerInstanceDispatchTable *table, PFN_vkGetInstanceProcAddr gpa,
                                                                    VkInstance inst);

// Init Instance function pointer dispatch table with core commands
VKAPI_ATTR void VKAPI_CALL loader_init_instance_extension_dispatch_table(VkLayerInstanceDispatchTable *table, PFN_vkGetInstanceProcAddr gpa,
                                                                         VkInstance inst);

// Device command lookup function
VKAPI_ATTR void* VKAPI_CALL loader_lookup_device_dispatch_table(const VkLayerDispatchTable *table, const char *name, bool* name_found);

// Instance command lookup function
VKAPI_ATTR void* VKAPI_CALL loader_lookup_instance_dispatch_table(const VkLayerInstanceDispatchTable *table, const char *name,
                                                                  bool *found_name);

''')

    # Creates the prototypes for the loader's core instance command terminators
    def OutputLoaderTerminators(self, out):
        out.append('// Loader core instance terminators\n')

        for command in [x for x in self.vk.commands.values() if len(x.extensions) == 0]:
            if not (command.name in ADD_INST_CMDS) and not (command.params[0].type in ['VkInstance', 'VkPhysicalDevice']):
                continue

            mod_string = ''
            new_terminator = command.cPrototype
            mod_string = new_terminator.replace("VKAPI_CALL vk", "VKAPI_CALL terminator_")

            if command.name in PRE_INSTANCE_FUNCTIONS:
                pre_instance_basic_version = mod_string
                mod_string = mod_string.replace("terminator_", "terminator_pre_instance_")
                mod_string = mod_string.replace(command.name[2:] + '(\n', command.name[2:] + '(\n    const Vk' + command.name[2:] + 'Chain* chain,\n')

            if command.protect is not None:
                out.append(f'#if defined({command.protect})\n')

            if command.name in PRE_INSTANCE_FUNCTIONS:
                out.append(pre_instance_basic_version)
                out.append('\n')

            out.append(mod_string)
            out.append('\n')

            if command.protect is not None:
                out.append(f'#endif // {command.protect}\n')

        out.append('\n')

    # Create a dispatch table from the appropriate list
    def OutputIcdDispatchTable(self, out):
        skip_commands = ['vkGetInstanceProcAddr',
                         'vkEnumerateDeviceLayerProperties',
                        ]

        out.append('// ICD function pointer dispatch table\n')
        out.append('struct loader_icd_term_dispatch {\n')

        current_block = ''
        for command in self.vk.commands.values():
            if (command.name in skip_commands or command.device )and command.name != 'vkGetDeviceProcAddr':
                continue

            current_block = self.DescribeBlock(command, current_block, out)

            if command.protect:
                out.append(f'#if defined({command.protect})\n')

            out.append(f'    PFN_{command.name} {command.name[2:]};\n')

            if command.protect:
                out.append(f'#endif // {command.protect}\n')
        out.append('};\n\n')

    # Create the extension enable union
    def OutputIcdExtensionEnableUnion(self, out):

        out.append( 'struct loader_instance_extension_enables {\n')
        for extension in self.instance_extensions:
            if len(extension.commands) == 0 or extension.name in WSI_EXT_NAMES:
                continue
            out.append( f'    uint8_t {extension.name[3:].lower()};\n')

        out.append( '};\n\n')

    # Create a dispatch table solely for device functions which have custom terminators
    def OutputDeviceFunctionTerminatorDispatchTable(self, out):
        out.append('// Functions that required a terminator need to have a separate dispatch table which contains their corresponding\n')
        out.append('// device function. This is used in the terminators themselves.\n')
        out.append('struct loader_device_terminator_dispatch {\n')

        # last_protect = None
        current_block = ''
        for command in self.vk.commands.values():
            if len(command.extensions) == 0:
                continue
            if command.name in DEVICE_CMDS_NEED_TERM:
                if command.protect is not None:
                    out.append( f'#if defined({command.protect})\n')

                current_block = self.DescribeBlock(command, current_block, out)

                out.append( f'    PFN_{command.name} {command.name[2:]};\n')

                if command.protect is not None:
                    out.append( f'#endif // {command.protect}\n')

        out.append( '};\n\n')


    # Create a layer instance dispatch table from the appropriate list
    def OutputLayerInstanceDispatchTable(self, out):

        out.append('// Instance function pointer dispatch table\n')
        out.append('typedef struct VkLayerInstanceDispatchTable_ {\n')

        # First add in an entry for GetPhysicalDeviceProcAddr.  This will not
        # ever show up in the XML or header, so we have to manually add it.
        out.append('    // Manually add in GetPhysicalDeviceProcAddr entry\n')
        out.append('    PFN_GetPhysicalDeviceProcAddr GetPhysicalDeviceProcAddr;\n')

        current_block = ''
        for command_name, command in self.vk.commands.items():
            if not command.instance:
                continue

            current_block = self.DescribeBlock(command, current_block, out)

            if command.protect:
                out.append(f'#if defined({command.protect})\n')

            out.append(f'    PFN_{command_name} {command_name[2:]};\n')

            if command.protect:
                out.append(f'#endif // {command.protect}\n')

        out.append('} VkLayerInstanceDispatchTable;\n\n')

    # Create a layer device dispatch table from the appropriate list
    def OutputLayerDeviceDispatchTable(self, out):

        out.append('// Device function pointer dispatch table\n')
        out.append('#define DEVICE_DISP_TABLE_MAGIC_NUMBER 0x10ADED040410ADEDUL\n')
        out.append('typedef struct VkLayerDispatchTable_ {\n')
        out.append('    uint64_t magic; // Should be DEVICE_DISP_TABLE_MAGIC_NUMBER\n')

        current_block = ''
        for command in [x for x in self.vk.commands.values() if x.device]:

            current_block = self.DescribeBlock(command, current_block, out)

            if command.protect:
                out.append(f'#if defined({command.protect})\n')

            out.append(f'    PFN_{command.name} {command.name[2:]};\n')

            if command.protect:
                out.append(f'#endif // {command.protect}\n')
        out.append('} VkLayerDispatchTable;\n\n')

    def OutputUtilitiesInSource(self, out):
        out.append('''
// Device extension error function
VKAPI_ATTR VkResult VKAPI_CALL vkDevExtError(VkDevice dev) {
    struct loader_device *found_dev;
    // The device going in is a trampoline device
    struct loader_icd_term *icd_term = loader_get_icd_and_device(dev, &found_dev);

    if (icd_term)
        loader_log(icd_term->this_instance, VULKAN_LOADER_ERROR_BIT, 0,
                   "Bad destination in loader trampoline dispatch,"
                   "Are layers and extensions that you are calling enabled?");
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

''')


    # Init a dispatch table from the appropriate list
    def OutputIcdDispatchTableInit(self, out):
        out.append('VKAPI_ATTR bool VKAPI_CALL loader_icd_init_entries(struct loader_instance* inst, struct loader_icd_term *icd_term) {\n')
        out.append('    const PFN_vkGetInstanceProcAddr fp_gipa = icd_term->scanned_icd->GetInstanceProcAddr;\n')
        out.append('\n')
        out.append('#define LOOKUP_GIPA(func) icd_term->dispatch.func = (PFN_vk##func)fp_gipa(icd_term->instance, "vk" #func);\n')
        out.append('\n')
        out.append('#define LOOKUP_REQUIRED_GIPA(func)                                                      \\\n')
        out.append('    do {                                                                                \\\n')
        out.append('        LOOKUP_GIPA(func);                                                              \\\n')
        out.append('        if (!icd_term->dispatch.func) {                                                 \\\n')
        out.append('            loader_log(inst, VULKAN_LOADER_WARN_BIT, 0, "Unable to load %s from ICD %s",\\\n')
        out.append('                       "vk"#func, icd_term->scanned_icd->lib_name);                     \\\n')
        out.append('            return false;                                                               \\\n')
        out.append('        }                                                                               \\\n')
        out.append('    } while (0)\n')
        out.append('\n')


        skip_gipa_commands = ['vkGetInstanceProcAddr',
                              'vkEnumerateDeviceLayerProperties',
                              'vkCreateInstance',
                              'vkEnumerateInstanceExtensionProperties',
                              'vkEnumerateInstanceLayerProperties',
                              'vkEnumerateInstanceVersion',
                             ]

        current_block = ''
        for command in [x for x in self.vk.commands.values() if x.instance or x.name == 'vkGetDeviceProcAddr']:
            if command.name in skip_gipa_commands:
                continue
            custom_commands_string= ' commands' if len(command.extensions) > 0 else ''
            current_block = self.DescribeBlock(command, current_block, out, custom_commands_string=custom_commands_string)

            if command.protect is not None:
                out.append( f'#if defined({command.protect})\n')

            if command.version is None and len(command.extensions) == 0:
                # The Core Vulkan code will be wrapped in a feature called VK_VERSION_#_#
                # For example: VK_VERSION_1_0 wraps the core 1.0 Vulkan functionality
                out.append(f'    LOOKUP_REQUIRED_GIPA({command.name[2:]});\n')
            else:
                out.append( f'    LOOKUP_GIPA({command.name[2:]});\n')

            if command.protect is not None:
                out.append(f'#endif // {command.protect}\n')

        out.append('\n')
        out.append('#undef LOOKUP_REQUIRED_GIPA\n')
        out.append('#undef LOOKUP_GIPA\n')
        out.append('\n')
        out.append('    return true;\n')
        out.append('};\n\n')


    # Creates code to initialize the various dispatch tables
    def OutputLoaderDispatchTables(self, out):
        commands = []
        gpa_param = ''
        cur_type = ''

        for x in range(0, 4):
            if x == 0:
                cur_type = 'device'
                gpa_param = 'dev'
                commands = self.core_commands

                out.append('// Init Device function pointer dispatch table with core commands\n')
                out.append('VKAPI_ATTR void VKAPI_CALL loader_init_device_dispatch_table(struct loader_dev_dispatch_table *dev_table, PFN_vkGetDeviceProcAddr gpa,\n')
                out.append('                                                             VkDevice dev) {\n')
                out.append('    VkLayerDispatchTable *table = &dev_table->core_dispatch;\n')
                out.append('    if (table->magic != DEVICE_DISP_TABLE_MAGIC_NUMBER) { abort(); }\n')
                out.append('    for (uint32_t i = 0; i < MAX_NUM_UNKNOWN_EXTS; i++) dev_table->ext_dispatch[i] = (PFN_vkDevExt)vkDevExtError;\n')

            elif x == 1:
                cur_type = 'device'
                gpa_param = 'dev'
                commands = self.extension_commands

                out.append('// Init Device function pointer dispatch table with extension commands\n')
                out.append('VKAPI_ATTR void VKAPI_CALL loader_init_device_extension_dispatch_table(struct loader_dev_dispatch_table *dev_table,\n')
                out.append('                                                                       PFN_vkGetInstanceProcAddr gipa,\n')
                out.append('                                                                       PFN_vkGetDeviceProcAddr gdpa,\n')
                out.append('                                                                       VkInstance inst,\n')
                out.append('                                                                       VkDevice dev) {\n')
                out.append('    VkLayerDispatchTable *table = &dev_table->core_dispatch;\n')
                out.append('    table->magic = DEVICE_DISP_TABLE_MAGIC_NUMBER;\n')

            elif x == 2:
                cur_type = 'instance'
                gpa_param = 'inst'
                commands = self.core_commands

                out.append('// Init Instance function pointer dispatch table with core commands\n')
                out.append('VKAPI_ATTR void VKAPI_CALL loader_init_instance_core_dispatch_table(VkLayerInstanceDispatchTable *table, PFN_vkGetInstanceProcAddr gpa,\n')
                out.append('                                                                    VkInstance inst) {\n')

            else:
                cur_type = 'instance'
                gpa_param = 'inst'
                commands = self.extension_commands

                out.append('// Init Instance function pointer dispatch table with core commands\n')
                out.append('VKAPI_ATTR void VKAPI_CALL loader_init_instance_extension_dispatch_table(VkLayerInstanceDispatchTable *table, PFN_vkGetInstanceProcAddr gpa,\n')
                out.append('                                                                        VkInstance inst) {\n')

            current_block = ''
            for command in commands:
                is_inst_handle_type = command.params[0].type in ['VkInstance', 'VkPhysicalDevice']
                if ((cur_type == 'instance' and is_inst_handle_type) or (cur_type == 'device' and not is_inst_handle_type)):

                    current_block = self.DescribeBlock(command, current_block, out)

                    # Remove 'vk' from proto name
                    base_name = command.name[2:]

                    # Names to skip
                    if (base_name == 'CreateInstance' or base_name == 'CreateDevice' or
                        base_name == 'EnumerateInstanceExtensionProperties' or
                        base_name == 'EnumerateInstanceLayerProperties' or
                        base_name == 'EnumerateInstanceVersion'):
                        continue

                    if command.protect is not None:
                        out.append(f'#if defined({command.protect})\n')

                    # If we're looking for the proc we are passing in, just point the table to it.  This fixes the issue where
                    # a layer overrides the function name for the loader.
                    if x == 1:
                        if base_name == 'GetDeviceProcAddr':
                            out.append('    table->GetDeviceProcAddr = gdpa;\n')
                        elif len(command.extensions) > 0 and command.extensions[0].instance:
                            out.append(f'    table->{base_name} = (PFN_{command.name})gipa(inst, "{command.name}");\n')
                        else:
                            out.append(f'    table->{base_name} = (PFN_{command.name})gdpa(dev, "{command.name}");\n')
                    elif (x < 1 and base_name == 'GetDeviceProcAddr'):
                        out.append('    table->GetDeviceProcAddr = gpa;\n')
                    elif (x > 1 and base_name == 'GetInstanceProcAddr'):
                        out.append('    table->GetInstanceProcAddr = gpa;\n')
                    else:
                        out.append(f'    table->{base_name} = (PFN_{command.name})gpa({gpa_param}, "{command.name}");\n')

                    if command.protect is not None:
                        out.append(f'#endif // {command.protect}\n')

            out.append('}\n\n')

#
    # Create code to initialize a dispatch table from the appropriate list of extension entrypoints
    def InitDeviceFunctionTerminatorDispatchTable(self, out):
        out.append('// Functions that required a terminator need to have a separate dispatch table which contains their corresponding\n')
        out.append('// device function. This is used in the terminators themselves.\n')
        out.append('void init_extension_device_proc_terminator_dispatch(struct loader_device *dev) {\n')
        out.append('    struct loader_device_terminator_dispatch* dispatch = &dev->loader_dispatch.extension_terminator_dispatch;\n')
        out.append('    PFN_vkGetDeviceProcAddr gpda = (PFN_vkGetDeviceProcAddr)dev->phys_dev_term->this_icd_term->dispatch.GetDeviceProcAddr;\n')
        current_block = ''
        for command in [x for x in self.vk.commands.values() if x.extensions or x.version]:
            if command.name in DEVICE_CMDS_NEED_TERM:
                if command.protect is not None:
                    out.append(f'#if defined({command.protect})\n')

                current_block = self.DescribeBlock(command, current_block, out)
                if command.name == 'vkGetDeviceGroupSurfacePresentModes2EXT': # command.extensions[0].depends in [x for x in self.vk.commands.values() if x.device]:
                    # Hardcode the dependency expression as vulkan_object.py doesn't expose this information
                    dep_expr = self.ConvertDependencyExpression('VK_KHR_device_group,VK_VERSION_1_1', lambda ext_name: f'dev->driver_extensions.{ext_name[3:].lower()}_enabled')
                    out.append(f'    if (dev->driver_extensions.{command.extensions[0].name[3:].lower()}_enabled && ({dep_expr}))\n')
                    out.append(f'       dispatch->{command.name[2:]} = (PFN_{(command.name)})gpda(dev->icd_device, "{(command.name)}");\n')
                else:
                    out.append(f'    if (dev->driver_extensions.{command.extensions[0].name[3:].lower()}_enabled)\n')
                    out.append(f'       dispatch->{command.name[2:]} = (PFN_{(command.name)})gpda(dev->icd_device, "{(command.name)}");\n')

                if command.protect is not None:
                    out.append(f'#endif // {command.protect}\n')

        out.append('}\n\n')


    def OutputDeviceFunctionTrampolinePrototypes(self, out):
        out.append('// These are prototypes for functions that need their trampoline called in all circumstances.\n')
        out.append('// They are used in loader_lookup_device_dispatch_table but are defined afterwards.\n')
        current_block = ''
        for command in self.vk.commands.values():
            if command.name in DEVICE_CMDS_MUST_USE_TRAMP:

                if command.protect is not None:
                    out.append(f'#if defined({command.protect})\n')
                current_block = self.DescribeBlock(command, current_block, out)

                out.append(f'{command.cPrototype.replace("VKAPI_CALL vk", "VKAPI_CALL ")}\n')

                if command.protect is not None:
                    out.append(f'#endif // {command.protect}\n')
        out.append('\n')


    #
    # Create a lookup table function from the appropriate list of entrypoints
    def OutputLoaderLookupFunc(self, out):
        commands = []
        cur_type = ''

        for x in range(0, 2):
            if x == 0:
                cur_type = 'device'

                out.append('// Device command lookup function\n')
                out.append('VKAPI_ATTR void* VKAPI_CALL loader_lookup_device_dispatch_table(const VkLayerDispatchTable *table, const char *name, bool* found_name) {\n')
                out.append('    if (!name || name[0] != \'v\' || name[1] != \'k\') {\n')
                out.append('        *found_name = false;\n')
                out.append('        return NULL;\n')
                out.append('    }\n')
                out.append('\n')
                out.append('    name += 2;\n')
                out.append('    *found_name = true;\n')
                out.append('    struct loader_device* dev = (struct loader_device *)table;\n')
                out.append('    const struct loader_instance* inst = dev->phys_dev_term->this_icd_term->this_instance;\n')
                out.append('    uint32_t api_version = VK_MAKE_API_VERSION(0, inst->app_api_version.major, inst->app_api_version.minor, inst->app_api_version.patch);\n')
                out.append('\n')
            else:
                cur_type = 'instance'

                out.append('// Instance command lookup function\n')
                out.append('VKAPI_ATTR void* VKAPI_CALL loader_lookup_instance_dispatch_table(const VkLayerInstanceDispatchTable *table, const char *name,\n')
                out.append('                                                                 bool *found_name) {\n')
                out.append('    if (!name || name[0] != \'v\' || name[1] != \'k\') {\n')
                out.append('        *found_name = false;\n')
                out.append('        return NULL;\n')
                out.append('    }\n')
                out.append('\n')
                out.append('    *found_name = true;\n')
                out.append('    name += 2;\n')


            for command_list in [self.core_commands, self.extension_commands]:
                commands = command_list

                version_check = ''
                current_block = ''
                for command in commands:
                    is_inst_handle_type = command.params[0].type in ['VkInstance', 'VkPhysicalDevice']
                    if ((cur_type == 'instance' and is_inst_handle_type) or (cur_type == 'device' and not is_inst_handle_type)):

                        current_block = self.DescribeBlock(command, current_block, out)
                        if len(command.extensions) == 0:
                            if cur_type == 'device':
                                version_check = f"        if (dev->should_ignore_device_commands_from_newer_version && api_version < {command.version.nameApi if command.version else 'VK_API_VERSION_1_0'}) return NULL;\n"
                            else:
                                version_check = ''

                        # Remove 'vk' from proto name
                        base_name = command.name[2:]

                        if (base_name == 'CreateInstance' or base_name == 'CreateDevice' or
                            base_name == 'EnumerateInstanceExtensionProperties' or
                            base_name == 'EnumerateInstanceLayerProperties' or
                            base_name == 'EnumerateInstanceVersion'):
                            continue

                        if command.protect is not None:
                            out.append(f'#if defined({command.protect})\n')

                        out.append(f'    if (!strcmp(name, "{base_name}")) ')
                        if command.name in DEVICE_CMDS_MUST_USE_TRAMP:
                            if version_check != '':
                                out.append(f'{{\n{version_check}        return dev->layer_extensions.{command.extensions[0].name[3:].lower()}_enabled ? (void *){base_name} : NULL;\n    }}\n')
                            else:
                                out.append(f'return dev->layer_extensions.{command.extensions[0].name[3:].lower()}_enabled ? (void *){base_name} : NULL;\n')

                        else:
                            if version_check != '':
                                out.append(f'{{\n{version_check}        return (void *)table->{base_name};\n    }}\n')
                            else:
                                out.append(f'return (void *)table->{base_name};\n')

                        if command.protect is not None:
                            out.append(f'#endif // {command.protect}\n')

            out.append('\n')
            out.append('    *found_name = false;\n')
            out.append('    return NULL;\n')
            out.append('}\n\n')


    #
    # Create the appropriate trampoline (and possibly terminator) functions
    def CreateTrampTermFuncs(self, out):
        # Some extensions have to be manually added.  Skip those in the automatic
        # generation.  They will be manually added later.
        manual_ext_commands = ['vkEnumeratePhysicalDeviceGroupsKHR',
                               'vkGetPhysicalDeviceExternalImageFormatPropertiesNV',
                               'vkGetPhysicalDeviceFeatures2KHR',
                               'vkGetPhysicalDeviceProperties2KHR',
                               'vkGetPhysicalDeviceFormatProperties2KHR',
                               'vkGetPhysicalDeviceImageFormatProperties2KHR',
                               'vkGetPhysicalDeviceQueueFamilyProperties2KHR',
                               'vkGetPhysicalDeviceMemoryProperties2KHR',
                               'vkGetPhysicalDeviceSparseImageFormatProperties2KHR',
                               'vkGetPhysicalDeviceSurfaceCapabilities2KHR',
                               'vkGetPhysicalDeviceSurfaceFormats2KHR',
                               'vkGetPhysicalDeviceSurfaceCapabilities2EXT',
                               'vkReleaseDisplayEXT',
                               'vkAcquireXlibDisplayEXT',
                               'vkGetRandROutputDisplayEXT',
                               'vkGetPhysicalDeviceExternalBufferPropertiesKHR',
                               'vkGetPhysicalDeviceExternalSemaphorePropertiesKHR',
                               'vkGetPhysicalDeviceExternalFencePropertiesKHR',
                               'vkGetPhysicalDeviceDisplayProperties2KHR',
                               'vkGetPhysicalDeviceDisplayPlaneProperties2KHR',
                               'vkGetDisplayModeProperties2KHR',
                               'vkGetDisplayPlaneCapabilities2KHR',
                               'vkGetPhysicalDeviceSurfacePresentModes2EXT',
                               'vkGetDeviceGroupSurfacePresentModes2EXT',
                               'vkGetPhysicalDeviceToolPropertiesEXT']

        current_block = ''
        for command in [x for x in self.vk.commands.values() if x.extensions]:
            if (command.extensions[0].name in WSI_EXT_NAMES or
                command.extensions[0].name in AVOID_EXT_NAMES or
                command.name in AVOID_CMD_NAMES or
                command.name in manual_ext_commands):
                continue

            current_block = self.DescribeBlock(command=command, current_block=current_block, out=out, custom_commands_string=' trampoline/terminators\n', indent='')

            if command.protect is not None:
                out.append(f'#if defined({command.protect})\n')

            func_header = command.cPrototype.replace(";", " {\n")
            tramp_header = func_header.replace("VKAPI_CALL vk", "VKAPI_CALL ")
            return_prefix = '    '
            base_name = command.name[2:]
            has_surface = 0
            update_structure_surface = 0
            update_structure_string = ''
            requires_terminator = 0
            surface_var_name = ''
            phys_dev_var_name = ''
            instance_var_name = ''
            has_return_type = False
            always_use_param_name = True
            surface_type_to_replace = ''
            surface_name_replacement = ''
            physdev_type_to_replace = ''
            physdev_name_replacement = ''

            for param in command.params:
                if param.type == 'VkSurfaceKHR':
                    has_surface = 1
                    surface_var_name = param.name
                    requires_terminator = 1
                    always_use_param_name = False
                    surface_type_to_replace = 'VkSurfaceKHR'
                    surface_name_replacement = 'unwrapped_surface'
                if param.type == 'VkPhysicalDeviceSurfaceInfo2KHR':
                    has_surface = 1
                    surface_var_name = param.name + '->surface'
                    requires_terminator = 1
                    update_structure_surface = 1
                    update_structure_string = '        VkPhysicalDeviceSurfaceInfo2KHR info_copy = *pSurfaceInfo;\n'
                    update_structure_string += '        info_copy.surface = unwrapped_surface;\n'
                    always_use_param_name = False
                    surface_type_to_replace = 'VkPhysicalDeviceSurfaceInfo2KHR'
                    surface_name_replacement = '&info_copy'
                if param.type == 'VkPhysicalDevice':
                    requires_terminator = 1
                    phys_dev_var_name = param.name
                    always_use_param_name = False
                    physdev_type_to_replace = 'VkPhysicalDevice'
                    physdev_name_replacement = 'phys_dev_term->phys_dev'
                if param.type == 'VkInstance':
                    requires_terminator = 1
                    instance_var_name = param.name

            if command.returnType != 'void':
                return_prefix += 'return '
                has_return_type = True

            if ( command.params[0].type in ['VkInstance', 'VkPhysicalDevice'] or
                'DebugMarkerSetObject' in command.name or 'SetDebugUtilsObject' in command.name or
                command.name in DEVICE_CMDS_NEED_TERM):
                requires_terminator = 1

            if requires_terminator == 1:
                term_header = tramp_header.replace("VKAPI_CALL ", "VKAPI_CALL terminator_")

                out.append(tramp_header)

                if command.params[0].type == 'VkPhysicalDevice':
                    out.append('    const VkLayerInstanceDispatchTable *disp;\n')
                    out.append(f'    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device({phys_dev_var_name});\n')
                    out.append('    if (VK_NULL_HANDLE == unwrapped_phys_dev) {\n')
                    out.append('        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,\n')
                    out.append(f'                   "{command.name}: Invalid {phys_dev_var_name} "\n')
                    out.append(f'                   "[VUID-{command.name}-{phys_dev_var_name}-parameter]");\n')
                    out.append('        abort(); /* Intentionally fail so user can correct issue. */\n')
                    out.append('    }\n')
                    out.append(f'    disp = loader_get_instance_layer_dispatch({phys_dev_var_name});\n')
                elif command.params[0].type == 'VkInstance':
                    out.append(f'    struct loader_instance *inst = loader_get_instance({instance_var_name});\n')
                    out.append('    if (NULL == inst) {\n')
                    out.append('        loader_log(\n')
                    out.append('            NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,\n')
                    out.append(f'            "{command.name}: Invalid instance [VUID-{command.name}-{instance_var_name}-parameter]");\n')
                    out.append('        abort(); /* Intentionally fail so user can correct issue. */\n')
                    out.append('    }\n')
                    out.append('#error("Not implemented. Likely needs to be manually generated!");\n')
                else:
                    out.append('    const VkLayerDispatchTable *disp = loader_get_dispatch(')
                    out.append(command.params[0].name)
                    out.append(');\n')
                    out.append('    if (NULL == disp) {\n')
                    out.append('        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,\n')
                    out.append(f'                   "{command.name}: Invalid {command.params[0].name} "\n')
                    out.append(f'                   "[VUID-{command.name}-{command.params[0].name}-parameter]");\n')
                    out.append('        abort(); /* Intentionally fail so user can correct issue. */\n')
                    out.append('    }\n')

                if 'DebugMarkerSetObjectName' in command.name:
                    out.append('    VkDebugMarkerObjectNameInfoEXT local_name_info;\n')
                    out.append('    memcpy(&local_name_info, pNameInfo, sizeof(VkDebugMarkerObjectNameInfoEXT));\n')
                    out.append('    // If this is a physical device, we have to replace it with the proper one for the next call.\n')
                    out.append('    if (pNameInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT) {\n')
                    out.append('        struct loader_physical_device_tramp *phys_dev_tramp = (struct loader_physical_device_tramp *)(uintptr_t)pNameInfo->object;\n')
                    out.append('        local_name_info.object = (uint64_t)(uintptr_t)phys_dev_tramp->phys_dev;\n')
                    out.append('    }\n')
                    out.append('    if (pNameInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT) {\n')
                    out.append('        struct loader_instance* instance = (struct loader_instance *)(uintptr_t)pNameInfo->object;\n')
                    out.append('        local_name_info.object = (uint64_t)(uintptr_t)instance->instance;\n')
                    out.append('    }\n')
                elif 'DebugMarkerSetObjectTag' in command.name:
                    out.append('    VkDebugMarkerObjectTagInfoEXT local_tag_info;\n')
                    out.append('    memcpy(&local_tag_info, pTagInfo, sizeof(VkDebugMarkerObjectTagInfoEXT));\n')
                    out.append('    // If this is a physical device, we have to replace it with the proper one for the next call.\n')
                    out.append('    if (pTagInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT) {\n')
                    out.append('        struct loader_physical_device_tramp *phys_dev_tramp = (struct loader_physical_device_tramp *)(uintptr_t)pTagInfo->object;\n')
                    out.append('        local_tag_info.object = (uint64_t)(uintptr_t)phys_dev_tramp->phys_dev;\n')
                    out.append('    }\n')
                    out.append('    if (pTagInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT) {\n')
                    out.append('        struct loader_instance* instance = (struct loader_instance *)(uintptr_t)pTagInfo->object;\n')
                    out.append('        local_tag_info.object = (uint64_t)(uintptr_t)instance->instance;\n')
                    out.append('    }\n')
                elif 'SetDebugUtilsObjectName' in command.name:
                    out.append('    VkDebugUtilsObjectNameInfoEXT local_name_info;\n')
                    out.append('    memcpy(&local_name_info, pNameInfo, sizeof(VkDebugUtilsObjectNameInfoEXT));\n')
                    out.append('    // If this is a physical device, we have to replace it with the proper one for the next call.\n')
                    out.append('    if (pNameInfo->objectType == VK_OBJECT_TYPE_PHYSICAL_DEVICE) {\n')
                    out.append('        struct loader_physical_device_tramp *phys_dev_tramp = (struct loader_physical_device_tramp *)(uintptr_t)pNameInfo->objectHandle;\n')
                    out.append('        local_name_info.objectHandle = (uint64_t)(uintptr_t)phys_dev_tramp->phys_dev;\n')
                    out.append('    }\n')
                    out.append('    if (pNameInfo->objectType == VK_OBJECT_TYPE_INSTANCE) {\n')
                    out.append('        struct loader_instance* instance = (struct loader_instance *)(uintptr_t)pNameInfo->objectHandle;\n')
                    out.append('        local_name_info.objectHandle = (uint64_t)(uintptr_t)instance->instance;\n')
                    out.append('    }\n')
                elif 'SetDebugUtilsObjectTag' in command.name:
                    out.append('    VkDebugUtilsObjectTagInfoEXT local_tag_info;\n')
                    out.append('    memcpy(&local_tag_info, pTagInfo, sizeof(VkDebugUtilsObjectTagInfoEXT));\n')
                    out.append('    // If this is a physical device, we have to replace it with the proper one for the next call.\n')
                    out.append('    if (pTagInfo->objectType == VK_OBJECT_TYPE_PHYSICAL_DEVICE) {\n')
                    out.append('        struct loader_physical_device_tramp *phys_dev_tramp = (struct loader_physical_device_tramp *)(uintptr_t)pTagInfo->objectHandle;\n')
                    out.append('        local_tag_info.objectHandle = (uint64_t)(uintptr_t)phys_dev_tramp->phys_dev;\n')
                    out.append('    }\n')
                    out.append('    if (pTagInfo->objectType == VK_OBJECT_TYPE_INSTANCE) {\n')
                    out.append('        struct loader_instance* instance = (struct loader_instance *)(uintptr_t)pTagInfo->objectHandle;\n')
                    out.append('        local_tag_info.objectHandle = (uint64_t)(uintptr_t)instance->instance;\n')
                    out.append('    }\n')

                if command.extensions[0].name in NULL_CHECK_EXT_NAMES:
                    out.append('    if (disp->' + base_name + ' != NULL) {\n')
                    out.append('    ')
                out.append(return_prefix)
                if command.params[0].type == 'VkInstance':
                    out.append('inst->')
                out.append('disp->')
                out.append(base_name)
                out.append('(')
                count = 0
                for param in command.params:
                    if count != 0:
                        out.append(', ')

                    if param.type == 'VkPhysicalDevice':
                        out.append('unwrapped_phys_dev')
                    elif ('DebugMarkerSetObject' in command.name or 'SetDebugUtilsObject' in command.name) and param.name == 'pNameInfo':
                        out.append('&local_name_info')
                    elif ('DebugMarkerSetObject' in command.name or 'SetDebugUtilsObject' in command.name) and param.name == 'pTagInfo':
                        out.append('&local_tag_info')
                    else:
                        out.append(param.name)

                    count += 1
                out.append(');\n')
                if command.extensions[0].name in NULL_CHECK_EXT_NAMES:
                    if command.returnType != 'void':
                        out.append('    } else {\n')
                        out.append('        return VK_SUCCESS;\n')
                    out.append('    }\n')
                out.append('}\n\n')

                out.append(term_header)
                if command.params[0].type == 'VkPhysicalDevice':
                    out.append(f'    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *){phys_dev_var_name};\n')
                    out.append('    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;\n')
                    out.append('    if (NULL == icd_term->dispatch.')
                    out.append(base_name)
                    out.append(') {\n')
                    fatal_error_bit = '' if has_return_type and (len(command.extensions) == 0 or command.extensions[0].instance) else 'VULKAN_LOADER_FATAL_ERROR_BIT | '
                    out.append(f'        loader_log(icd_term->this_instance, {fatal_error_bit}VULKAN_LOADER_ERROR_BIT, 0,\n')
                    out.append('                   "ICD associated with VkPhysicalDevice does not support ')
                    out.append(base_name)
                    out.append('");\n')

                    # If this is an instance function taking a physical device (i.e. pre Vulkan 1.1), we need to behave and not crash so return an
                    # error here.
                    if has_return_type and (len(command.extensions) == 0 or command.extensions[0].instance):
                        out.append('        return VK_ERROR_EXTENSION_NOT_PRESENT;\n')
                    else:
                        out.append('        abort(); /* Intentionally fail so user can correct issue. */\n')
                    out.append('    }\n')

                    if has_surface == 1:
                        out.append(f'    VkSurfaceKHR unwrapped_surface = {surface_var_name};\n')
                        out.append('    VkResult unwrap_res = wsi_unwrap_icd_surface(icd_term, &unwrapped_surface);\n')
                        out.append('    if (unwrap_res == VK_SUCCESS) {\n')

                        # If there's a structure with a surface, we need to update its internals with the correct surface for the ICD
                        if update_structure_surface == 1:
                            out.append(update_structure_string)

                        out.append('    ' + return_prefix + 'icd_term->dispatch.')
                        out.append(base_name)
                        out.append('(')
                        count = 0
                        for param in command.params:
                            if count != 0:
                                out.append(', ')

                            if not always_use_param_name:
                                if surface_type_to_replace and surface_type_to_replace == param.type:
                                    out.append(surface_name_replacement)
                                elif physdev_type_to_replace and physdev_type_to_replace == param.type:
                                    out.append(physdev_name_replacement)
                                else:
                                    out.append(param.name)
                            else:
                                out.append(param.name)

                            count += 1
                        out.append(');\n')
                        if not has_return_type:
                            out.append('        return;\n')
                        out.append('    }\n')

                    out.append(return_prefix)
                    out.append('icd_term->dispatch.')
                    out.append(base_name)
                    out.append('(')
                    count = 0
                    for param in command.params:
                        if count != 0:
                            out.append(', ')

                        if param.type == 'VkPhysicalDevice':
                            out.append('phys_dev_term->phys_dev')
                        else:
                            out.append(param.name)

                        count += 1
                    out.append(');\n')


                elif command.params[0].type == 'VkInstance':
                    out.append(f'    struct loader_instance *inst = loader_get_instance({instance_var_name});\n')
                    out.append('    if (NULL == inst) {\n')
                    out.append('        loader_log(\n')
                    out.append('            NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,\n')
                    out.append(f'            "{command.name}: Invalid instance [VUID-{command.name}-{instance_var_name}-parameter]");\n')
                    out.append('        abort(); /* Intentionally fail so user can correct issue. */\n')
                    out.append('    }\n')
                    out.append('#error("Not implemented. Likely needs to be manually generated!");\n')
                elif command.extensions[0].name in ['VK_EXT_debug_utils', 'VK_EXT_debug_marker']:
                    if command.name in ['vkDebugMarkerSetObjectNameEXT', 'vkDebugMarkerSetObjectTagEXT', 'vkSetDebugUtilsObjectNameEXT' , 'vkSetDebugUtilsObjectTagEXT']:

                        is_debug_utils = command.extensions[0].name == "VK_EXT_debug_utils"
                        debug_struct_name = command.params[1].name
                        local_struct = 'local_name_info' if 'ObjectName' in command.name else 'local_tag_info'
                        member_name = 'objectHandle' if is_debug_utils else 'object'
                        phys_dev_check = 'VK_OBJECT_TYPE_PHYSICAL_DEVICE' if is_debug_utils else 'VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT'
                        surf_check = 'VK_OBJECT_TYPE_SURFACE_KHR' if is_debug_utils else 'VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT'
                        inst_check = 'VK_OBJECT_TYPE_INSTANCE' if is_debug_utils else 'VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT'
                        out.append('    struct loader_device *dev;\n')
                        out.append(f'    struct loader_icd_term *icd_term = loader_get_icd_and_device({ command.params[0].name}, &dev);\n')
                        out.append('    if (NULL == icd_term || NULL == dev) {\n')
                        out.append(f'        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0, "{command.name[2:]}: Invalid device handle");\n')
                        out.append('        abort(); /* Intentionally fail so user can correct issue. */\n')
                        out.append('    }\n')
                        out.append(f'    { command.params[1].type} {local_struct};\n')
                        out.append(f'    memcpy(&{local_struct}, {debug_struct_name}, sizeof({ command.params[1].type}));\n')
                        out.append('    // If this is a physical device, we have to replace it with the proper one for the next call.\n')
                        out.append(f'    if ({debug_struct_name}->objectType == {phys_dev_check}) {{\n')
                        out.append(f'        struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)(uintptr_t){debug_struct_name}->{member_name};\n')
                        out.append(f'        {local_struct}.{member_name} = (uint64_t)(uintptr_t)phys_dev_term->phys_dev;\n')
                        out.append('    // If this is a KHR_surface, and the ICD has created its own, we have to replace it with the proper one for the next call.\n')
                        out.append(f'    }} else if ({debug_struct_name}->objectType == {surf_check}) {{\n')
                        out.append('        if (NULL != dev && NULL != dev->loader_dispatch.core_dispatch.CreateSwapchainKHR) {\n')
                        out.append(f'            VkSurfaceKHR surface = (VkSurfaceKHR)(uintptr_t){debug_struct_name}->{member_name};\n')
                        out.append('            if (wsi_unwrap_icd_surface(icd_term, &surface) == VK_SUCCESS) {\n')
                        out.append(f'                {local_struct}.{member_name} = (uint64_t)surface;\n')
                        out.append('            }\n')
                        out.append('        }\n')
                        out.append('    // If this is an instance we have to replace it with the proper one for the next call.\n')
                        out.append(f'    }} else if ({debug_struct_name}->objectType == {inst_check}) {{\n')
                        out.append(f'        {local_struct}.{member_name} = (uint64_t)(uintptr_t)icd_term->instance;\n')
                        out.append('    }\n')
                        out.append('    // Exit early if the driver does not support the function - this can happen as a layer or the loader itself supports\n')
                        out.append('    // debug utils but the driver does not.\n')
                        out.append(f'    if (NULL == dev->loader_dispatch.extension_terminator_dispatch.{command.name[2:]})\n        return VK_SUCCESS;\n')
                        dispatch = 'dev->loader_dispatch.'
                    else:
                        out.append(f'    struct loader_dev_dispatch_table *dispatch_table = loader_get_dev_dispatch({command.params[0].name});\n')
                        out.append('    if (NULL == dispatch_table) {\n')
                        out.append(f'        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0, "{command.extensions[0].name}: Invalid device handle");\n')
                        out.append('        abort(); /* Intentionally fail so user can correct issue. */\n')
                        out.append('    }\n')
                        out.append('    // Only call down if the device supports the function\n')
                        out.append(f'    if (NULL != dispatch_table->extension_terminator_dispatch.{base_name})\n    ')
                        dispatch = 'dispatch_table->'
                    out.append('    ')
                    if has_return_type:
                        out.append('return ')
                    out.append(f'{dispatch}extension_terminator_dispatch.{base_name}(')
                    count = 0
                    for param in command.params:
                        if count != 0:
                            out.append(', ')

                        if param.type == 'VkPhysicalDevice':
                            out.append('phys_dev_term->phys_dev')
                        elif param.type == 'VkSurfaceKHR':
                            out.append( 'unwrapped_surface')
                        elif ('DebugMarkerSetObject' in command.name or 'SetDebugUtilsObject' in command.name) and param.name == 'pNameInfo':
                            out.append( '&local_name_info')
                        elif ('DebugMarkerSetObject' in command.name or 'SetDebugUtilsObject' in command.name) and param.name == 'pTagInfo':
                            out.append('&local_tag_info')
                        else:
                            out.append(param.name)
                        count += 1

                    out.append(');\n')

                else:
                    out.append('#error("Unknown error path!");\n')

                out.append('}\n\n')
            else:
                out.append(tramp_header)

                out.append('    const VkLayerDispatchTable *disp = loader_get_dispatch(')
                out.append(command.params[0].name)
                out.append(');\n')
                out.append('    if (NULL == disp) {\n')
                out.append('        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,\n')
                out.append(f'                   "{command.name}: Invalid {command.params[0].name} "\n')
                out.append(f'                   "[VUID-{command.name}-{command.params[0].name}-parameter]");\n')
                out.append('        abort(); /* Intentionally fail so user can correct issue. */\n')
                out.append('    }\n')

                if command.extensions[0].name in NULL_CHECK_EXT_NAMES:
                    out.append('    if (disp->' + base_name + ' != NULL) {\n')
                    out.append('    ')
                out.append(return_prefix)
                out.append('disp->')
                out.append(base_name)
                out.append('(')
                count = 0
                for param in command.params:
                    if count != 0:
                        out.append(', ')
                    out.append(param.name)
                    count += 1
                out.append(');\n')
                if command.extensions[0].name in NULL_CHECK_EXT_NAMES:
                    if command.returnType != 'void':
                        out.append('    } else {\n')
                        out.append('        return VK_SUCCESS;\n')
                    out.append('    }\n')
                out.append('}\n\n')

            if command.protect is not None:
                out.append(f'#endif // {command.protect}\n')



    #
    # Create a function for the extension GPA call
    def InstExtensionGPA(self, out):
        cur_extension_name = ''

        out.append( '// GPA helpers for extensions\n')
        out.append( 'bool extension_instance_gpa(struct loader_instance *ptr_instance, const char *name, void **addr) {\n')
        out.append( '    *addr = NULL;\n\n')

        for command in [x for x in self.vk.commands.values() if x.extensions]:
            if (command.version or
                command.extensions[0].name in WSI_EXT_NAMES or
                command.extensions[0].name in AVOID_EXT_NAMES or
                command.name in AVOID_CMD_NAMES ):
                continue

            if command.extensions[0].name != cur_extension_name:
                out.append( f'\n    // ---- {command.extensions[0].name} extension commands\n')
                cur_extension_name = command.extensions[0].name

            if command.protect is not None:
                out.append( f'#if defined({command.protect})\n')

            #base_name = command.name[2:]
            base_name = SHARED_ALIASES[command.name] if command.name in SHARED_ALIASES else command.name[2:]

            if len(command.extensions) > 0 and command.extensions[0].instance:
                out.append( f'    if (!strcmp("{command.name}", name)) {{\n')
                out.append( '        *addr = (ptr_instance->enabled_known_extensions.')
                out.append( command.extensions[0].name[3:].lower())
                out.append( ' == 1)\n')
                out.append( f'                     ? (void *){base_name}\n')
                out.append( '                     : NULL;\n')
                out.append( '        return true;\n')
                out.append( '    }\n')
            else:
                out.append( f'    if (!strcmp("{command.name}", name)) {{\n')
                out.append( f'        *addr = (void *){base_name};\n')
                out.append( '        return true;\n')
                out.append( '    }\n')

            if command.protect is not None:
                out.append( f'#endif // {command.protect}\n')

        out.append( '    return false;\n')
        out.append( '}\n\n')



    #
    # Create the extension name init function
    def InstantExtensionCreate(self, out):

        out.append( '// A function that can be used to query enabled extensions during a vkCreateInstance call\n')
        out.append( 'void extensions_create_instance(struct loader_instance *ptr_instance, const VkInstanceCreateInfo *pCreateInfo) {\n')
        out.append( '    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {\n')
        e = ''
        cur_extension_name = ''
        for extension in [x for x in self.vk.extensions.values() if x.instance and len(x.commands) > 0]:
            if extension.name in WSI_EXT_NAMES or extension.name in AVOID_EXT_NAMES or extension.name in AVOID_CMD_NAMES:
                continue

            if extension.name != cur_extension_name:
                out.append( f'\n    // ---- {extension.name} extension commands\n')
                cur_extension_name = extension.name

            if extension.protect is not None:
                out.append( f'#if defined({extension.protect})\n')

            out.append(f'        {e}if (0 == strcmp(pCreateInfo->ppEnabledExtensionNames[i], ')
            e = '} else '

            out.append( extension.nameString + ')) {\n')
            out.append( '            ptr_instance->enabled_known_extensions.')
            out.append( extension.name[3:].lower())
            out.append( ' = 1;\n')

            if extension.protect is not None:
                out.append( f'#endif // {extension.protect}\n')

        out.append( '        }\n')
        out.append( '    }\n')
        out.append( '}\n\n')


    #
    # Create code to initialize a dispatch table from the appropriate list of extension entrypoints
    def DeviceExtensionGetTerminator(self, out):
        out.append('// Some device commands still need a terminator because the loader needs to unwrap something about them.\n')
        out.append('// In many cases, the item needing unwrapping is a VkPhysicalDevice or VkSurfaceKHR object.  But there may be other items\n')
        out.append('// in the future.\n')
        out.append('PFN_vkVoidFunction get_extension_device_proc_terminator(struct loader_device *dev, const char *name, bool* found_name) {\n')
        out.append('''    *found_name = false;
    if (!name || name[0] != 'v' || name[1] != 'k') {
        return NULL;
    }
    name += 2;
''')
        last_protect = None
        last_ext = None
        for command in [x for x in self.vk.commands.values() if x.extensions]:
            version = command.version
            if command.name in DEVICE_CMDS_NEED_TERM:
                if version:
                    out.append(f'    // ---- Core {version.name} commands\n')
                else:
                    last_protect = command.protect
                    if command.protect is not None:
                        out.append(f'#if defined({command.protect})\n')
                    if last_ext != command.extensions[0].name:
                        out.append(f'    // ---- {command.extensions[0].name} extension commands\n')
                        last_ext = command.extensions[0].name

                out.append(f'    if (!strcmp(name, "{command.name[2:]}")) {{\n')
                out.append('        *found_name = true;\n')
                if command.name == 'vkGetDeviceGroupSurfacePresentModes2EXT': # command.extensions[0].depends in [x for x in self.vk.commands.values() if x.device]:
                    # Hardcode the dependency expression as vulkan_object.py doesn't expose this information
                    dep_expr = self.ConvertDependencyExpression('VK_KHR_device_group,VK_VERSION_1_1', lambda ext_name: f'dev->driver_extensions.{ext_name[3:].lower()}_enabled')
                    out.append(f'        return (dev->driver_extensions.{command.extensions[0].name[3:].lower()}_enabled && ({dep_expr})) ?\n')
                else:
                    out.append(f'        return dev->driver_extensions.{command.extensions[0].name[3:].lower()}_enabled ?\n')
                out.append(f'            (PFN_vkVoidFunction)terminator_{(command.name[2:])} : NULL;\n')
                out.append('    }\n')

        if last_protect is not None:
            out.append(f'#endif // {last_protect}\n')

        out.append('    return NULL;\n')
        out.append('}\n\n')



    #
    # Create code to initialize a dispatch table from the appropriate list of core and extension entrypoints
    def InitInstLoaderExtensionDispatchTable(self, out):
        commands = []

        out.append( '// This table contains the loader\'s instance dispatch table, which contains\n')
        out.append( '// default functions if no instance layers are activated.  This contains\n')
        out.append( '// pointers to "terminator functions".\n')
        out.append( 'const VkLayerInstanceDispatchTable instance_disp = {\n')

        for command_list in [self.core_commands, self.extension_commands]:
            commands = command_list

            current_block = ''
            for command in commands:
                if command.params[0].type in ['VkInstance', 'VkPhysicalDevice']:
                    current_block = self.DescribeBlock(command, current_block, out)
                    # Remove 'vk' from proto name
                    base_name = command.name[2:]
                    aliased_name = SHARED_ALIASES[command.name][2:] if command.name in SHARED_ALIASES else base_name

                    if (base_name == 'CreateInstance' or base_name == 'CreateDevice' or
                        base_name == 'EnumerateInstanceExtensionProperties' or
                        base_name == 'EnumerateInstanceLayerProperties' or
                        base_name == 'EnumerateInstanceVersion'):
                        continue

                    if command.protect is not None:
                        out.append( f'#if defined({command.protect})\n')

                    if base_name == 'GetInstanceProcAddr':
                        out.append( f'    .{base_name} = {command.name},\n')
                    else:
                        out.append( f'    .{base_name} = terminator_{aliased_name},\n')

                    if command.protect is not None:
                        out.append( f'#endif // {command.protect}\n')
        out.append( '};\n\n')



    #
    # Create the extension name whitelist array
    def OutputInstantExtensionWhitelistArray(self, out):
        out.append( '// A null-terminated list of all of the instance extensions supported by the loader.\n')
        out.append( '// If an instance extension name is not in this list, but it is exported by one or more of the\n')
        out.append( '// ICDs detected by the loader, then the extension name not in the list will be filtered out\n')
        out.append( '// before passing the list of extensions to the application.\n')
        out.append( 'const char *const LOADER_INSTANCE_EXTENSIONS[] = {\n')
        for extension in self.instance_extensions:

            if extension.protect is not None:
                out.append( f'#if defined({extension.protect})\n')
            out.append( '                                                  ')
            out.append( extension.nameString + ',\n')

            if extension.protect is not None:
                out.append( f'#endif // {extension.protect}\n')
        out.append( '                                                  NULL };\n')
