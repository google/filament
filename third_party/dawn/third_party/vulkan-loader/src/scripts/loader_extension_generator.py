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

import re
import sys
from collections import namedtuple
from generator import *
from common_codegen import *


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

#
# API Version
class APIVersion:
    def __init__(self, token, apiname = 'Vulkan', supported = True):
        self.token = token
        self.constant = token.replace('_VERSION_', '_API_VERSION_')
        self.number = token[token.find('_VERSION_') + len('_VERSION_'):].replace('_', '.')
        self.name = f'{apiname} {self.number}'
        self.supported = supported

#
# LoaderExtensionGeneratorOptions - subclass of GeneratorOptions.
class LoaderExtensionGeneratorOptions(GeneratorOptions):
    def __init__(self,
                 conventions = None,
                 filename = None,
                 directory = '.',
                 genpath = None,
                 apiname = None,
                 profile = None,
                 versions = '.*',
                 emitversions = '.*',
                 defaultExtensions = None,
                 addExtensions = None,
                 removeExtensions = None,
                 emitExtensions = None,
                 sortProcedure = regSortFeatures,
                 prefixText = "",
                 genFuncPointers = True,
                 protectFile = True,
                 protectFeature = True,
                 apicall = '',
                 apientry = '',
                 apientryp = '',
                 indentFuncProto = True,
                 indentFuncPointer = False,
                 alignFuncParam = 0,
                 expandEnumerants = True):
        GeneratorOptions.__init__(self,
                conventions = conventions,
                filename = filename,
                directory = directory,
                genpath = genpath,
                apiname = apiname,
                profile = profile,
                versions = versions,
                emitversions = emitversions,
                defaultExtensions = defaultExtensions,
                addExtensions = addExtensions,
                removeExtensions = removeExtensions,
                emitExtensions = emitExtensions,
                sortProcedure = sortProcedure)
        self.prefixText      = prefixText
        self.prefixText      = None
        self.apicall         = apicall
        self.apientry        = apientry
        self.apientryp       = apientryp
        self.alignFuncParam  = alignFuncParam
        self.expandEnumerants = expandEnumerants

#
# LoaderExtensionOutputGenerator - subclass of OutputGenerator.
# Generates dispatch table helper header files for LVL
class LoaderExtensionOutputGenerator(OutputGenerator):
    """Generate dispatch table helper header based on XML element attributes"""
    def __init__(self,
                 errFile = sys.stderr,
                 warnFile = sys.stderr,
                 diagFile = sys.stdout):
        OutputGenerator.__init__(self, errFile, warnFile, diagFile)

        # Internal state - accumulators for different inner block text
        self.ext_instance_dispatch_list = []  # List of extension entries for instance dispatch list
        self.ext_device_dispatch_list = []    # List of extension entries for device dispatch list
        self.core_commands = []               # List of CommandData records for core Vulkan commands
        self.ext_commands = []                # List of CommandData records for extension Vulkan commands
        self.CommandParam = namedtuple('CommandParam', ['type', 'name', 'cdecl'])
        self.CommandData = namedtuple('CommandData', ['name', 'ext_name', 'ext_type', 'require', 'protect', 'return_type', 'handle_type', 'params', 'cdecl'])
        self.instanceExtensions = []
        self.ExtensionData = namedtuple('ExtensionData', ['name', 'type', 'protect', 'define', 'num_commands'])

    #
    # Called once at the beginning of each run
    def beginFile(self, genOpts):
        OutputGenerator.beginFile(self, genOpts)

        # User-supplied prefix text, if any (list of strings)
        if genOpts.prefixText:
            for s in genOpts.prefixText:
                write(s, file=self.outFile)

        # File Comment
        file_comment = '// *** THIS FILE IS GENERATED - DO NOT EDIT ***\n'
        file_comment += '// See loader_extension_generator.py for modifications\n'
        write(file_comment, file=self.outFile)

        # Copyright Notice
        copyright =  '/*\n'
        copyright += ' * Copyright (c) 2015-2022 The Khronos Group Inc.\n'
        copyright += ' * Copyright (c) 2015-2022 Valve Corporation\n'
        copyright += ' * Copyright (c) 2015-2022 LunarG, Inc.\n'
        copyright += ' * Copyright (c) 2021-2023 NVIDIA CORPORATION & AFFILIATES. All rights reserved.\n'
        copyright += ' * Copyright (c) 2023-2023 RasterGrid Kft.\n'
        copyright += ' *\n'
        copyright += ' * Licensed under the Apache License, Version 2.0 (the "License");\n'
        copyright += ' * you may not use this file except in compliance with the License.\n'
        copyright += ' * You may obtain a copy of the License at\n'
        copyright += ' *\n'
        copyright += ' *     http://www.apache.org/licenses/LICENSE-2.0\n'
        copyright += ' *\n'
        copyright += ' * Unless required by applicable law or agreed to in writing, software\n'
        copyright += ' * distributed under the License is distributed on an "AS IS" BASIS,\n'
        copyright += ' * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n'
        copyright += ' * See the License for the specific language governing permissions and\n'
        copyright += ' * limitations under the License.\n'
        copyright += ' *\n'
        copyright += ' * Author: Mark Lobodzinski <mark@lunarg.com>\n'
        copyright += ' * Author: Mark Young <marky@lunarg.com>\n'
        copyright += ' */\n'

        preamble = ''

        preamble += '// clang-format off\n'

        if self.genOpts.filename == 'vk_loader_extensions.h':
            preamble += '#pragma once\n'
            preamble += '\n'
            preamble += '#include <stdbool.h>\n'
            preamble += '#include <vulkan/vulkan.h>\n'
            preamble += '#include <vulkan/vk_layer.h>\n'
            preamble += '#include "vk_layer_dispatch_table.h"\n'
            preamble += '\n'


        elif self.genOpts.filename == 'vk_loader_extensions.c':
            preamble += '#include <stdio.h>\n'
            preamble += '#include <stdlib.h>\n'
            preamble += '#include <string.h>\n'
            preamble += '#include "loader.h"\n'
            preamble += '#include "vk_loader_extensions.h"\n'
            preamble += '#include <vulkan/vk_icd.h>\n'
            preamble += '#include "wsi.h"\n'
            preamble += '#include "debug_utils.h"\n'
            preamble += '#include "extension_manual.h"\n'

        elif self.genOpts.filename == 'vk_layer_dispatch_table.h':
            preamble += '#pragma once\n'
            preamble += '\n'
            preamble += '#include <vulkan/vulkan.h>\n'
            preamble += '\n'
            preamble += '#if !defined(PFN_GetPhysicalDeviceProcAddr)\n'
            preamble += 'typedef PFN_vkVoidFunction (VKAPI_PTR *PFN_GetPhysicalDeviceProcAddr)(VkInstance instance, const char* pName);\n'
            preamble += '#endif\n'

        write(copyright, file=self.outFile)
        write(preamble, file=self.outFile)

    #
    # Write generate and write dispatch tables to output file
    def endFile(self):
        file_data = ''

        if self.genOpts.filename == 'vk_loader_extensions.h':
            file_data += self.OutputPrototypesInHeader()
            file_data += self.OutputLoaderTerminators()
            file_data += self.OutputIcdDispatchTable()
            file_data += self.OutputIcdExtensionEnableUnion()
            file_data += self.OutputDeviceFunctionTerminatorDispatchTable()

        elif self.genOpts.filename == 'vk_loader_extensions.c':
            file_data += self.OutputUtilitiesInSource()
            file_data += self.OutputIcdDispatchTableInit()
            file_data += self.OutputLoaderDispatchTables()
            file_data += self.InitDeviceFunctionTerminatorDispatchTable()
            file_data += self.OutputDeviceFunctionTrampolinePrototypes()
            file_data += self.OutputLoaderLookupFunc()
            file_data += self.CreateTrampTermFuncs()
            file_data += self.InstExtensionGPA()
            file_data += self.InstantExtensionCreate()
            file_data += self.DeviceExtensionGetTerminator()
            file_data += self.InitInstLoaderExtensionDispatchTable()
            file_data += self.OutputInstantExtensionWhitelistArray()

        elif self.genOpts.filename == 'vk_layer_dispatch_table.h':
            file_data += self.OutputLayerInstanceDispatchTable()
            file_data += self.OutputLayerDeviceDispatchTable()

        file_data += '// clang-format on'

        write(file_data, file=self.outFile)

        # Finish processing in superclass
        OutputGenerator.endFile(self)

    def beginFeature(self, interface, emit):
        # Start processing in superclass
        OutputGenerator.beginFeature(self, interface, emit)
        self.featureExtraProtect = GetFeatureProtect(interface)

        enums = interface[0].findall('enum')
        self.currentExtension = ''
        self.name_definition = ''

        for item in enums:
            name_definition = item.get('name')
            if 'EXTENSION_NAME' in name_definition:
                self.name_definition = name_definition

        self.type = interface.get('type')
        self.num_commands = 0
        name = interface.get('name')
        self.currentExtension = name

    #
    # Process commands, adding to appropriate dispatch tables
    def genCmd(self, cmdinfo, name, alias):
        OutputGenerator.genCmd(self, cmdinfo, name, alias)

        # Get first param type
        params = cmdinfo.elem.findall('param')
        info = self.getTypeNameTuple(params[0])

        self.num_commands += 1

        if 'android' not in name:
            self.AddCommandToDispatchList(self.currentExtension, self.type, name, cmdinfo, info[0])

    def endFeature(self):

        if 'android' not in self.currentExtension:
            self.instanceExtensions.append(self.ExtensionData(name=self.currentExtension,
                                                              type=self.type,
                                                              protect=self.featureExtraProtect,
                                                              define=self.name_definition,
                                                              num_commands=self.num_commands))

        # Finish processing in superclass
        OutputGenerator.endFeature(self)

    #
    # Retrieve the value of the len tag
    def getLen(self, param):
        result = None
        length = param.attrib.get('len')
        if length and length != 'null-terminated':
            # For string arrays, 'len' can look like 'count,null-terminated',
            # indicating that we have a null terminated array of strings.  We
            # strip the null-terminated from the 'len' field and only return
            # the parameter specifying the string count
            if 'null-terminated' in length:
                result = length.split(',')[0]
            else:
                result = length
            result = str(result).replace('::', '->')
        return result

    #
    # Returns an APIVersion object corresponding to the specified version token or None
    def getAPIVersion(self, token):
        if self.genOpts.apiname == 'vulkansc':
            if token in ['VK_VERSION_1_0', 'VK_VERSION_1_1', 'VK_VERSION_1_2']:
                # Vulkan 1.0-1.2 is included in Vulkan SC 1.0
                token = 'VKSC_VERSION_1_0'

            if token.startswith('VKSC_VERSION_'):
                return APIVersion(token, 'Vulkan SC', True)
            elif token.startswith('VK_VERSION_'):
                # Unsupported Vulkan version
                return APIVersion(token, 'Vulkan', False)
            else:
                return None

        if token.startswith('VK_VERSION_'):
            return APIVersion(token)
        return None

    #
    # Determine if this API should be ignored or added to the instance or device dispatch table
    def AddCommandToDispatchList(self, extension_name, extension_type, name, cmdinfo, handle_type):
        handle = self.registry.tree.find("types/type/[name='" + handle_type + "'][@category='handle']")

        return_type =  cmdinfo.elem.find('proto/type')
        if (return_type is not None and return_type.text == 'void'):
            return_type = None

        require = None
        if name == 'vkGetDeviceGroupSurfacePresentModes2EXT':
            require_node = self.registry.tree.find(f"./extensions/extension[@name='{extension_name}']/require/command[@name='{name}']/..")
            if 'depends' in require_node.attrib:
                require = require_node.attrib['depends']

        cmd_params = []

        # Generate a list of commands for use in printing the necessary
        # core instance terminator prototypes
        params = cmdinfo.elem.findall('param')
        lens = set()
        for param in params:
            length = self.getLen(param)
            if length:
                lens.add(length)

        for param in params:
            paramInfo = self.getTypeNameTuple(param)
            param_type = paramInfo[0]
            param_name = paramInfo[1]
            param_cdecl = self.makeCParamDecl(param, 0)
            cmd_params.append(self.CommandParam(type=param_type, name=param_name,
                                                cdecl=param_cdecl))

        version = self.getAPIVersion(extension_name)
        if version and not version.supported:
            # Skip commands in unsupported versions
            return

        if handle is not None and handle_type != 'VkInstance' and handle_type != 'VkPhysicalDevice':
            # The Core Vulkan code will be wrapped in a feature called VK_VERSION_#_#
            # For example: VK_VERSION_1_0 wraps the core 1.0 Vulkan functionality
            if version:
                self.core_commands.append(
                    self.CommandData(name=name, ext_name=version.token,
                                     ext_type='device',
                                     require=require,
                                     protect=self.featureExtraProtect,
                                     return_type = return_type,
                                     handle_type = handle_type,
                                     params = cmd_params,
                                     cdecl=self.makeCDecls(cmdinfo.elem)[0]))
            else:
                self.ext_device_dispatch_list.append((name, self.featureExtraProtect))
                self.ext_commands.append(
                    self.CommandData(name=name, ext_name=extension_name,
                                     ext_type=extension_type,
                                     require=require,
                                     protect=self.featureExtraProtect,
                                     return_type = return_type,
                                     handle_type = handle_type,
                                     params = cmd_params,
                                     cdecl=self.makeCDecls(cmdinfo.elem)[0]))
        else:
            # The Core Vulkan code will be wrapped in a feature called VK_VERSION_#_#
            # For example: VK_VERSION_1_0 wraps the core 1.0 Vulkan functionality
            if version:
                self.core_commands.append(
                    self.CommandData(name=name, ext_name=version.token,
                                     ext_type='instance',
                                     require=require,
                                     protect=self.featureExtraProtect,
                                     return_type = return_type,
                                     handle_type = handle_type,
                                     params = cmd_params,
                                     cdecl=self.makeCDecls(cmdinfo.elem)[0]))

            else:
                self.ext_instance_dispatch_list.append((name, self.featureExtraProtect))
                self.ext_commands.append(
                    self.CommandData(name=name, ext_name=extension_name,
                                     ext_type=extension_type,
                                     require=require,
                                     protect=self.featureExtraProtect,
                                     return_type = return_type,
                                     handle_type = handle_type,
                                     params = cmd_params,
                                     cdecl=self.makeCDecls(cmdinfo.elem)[0]))

    #
    # Retrieve the type and name for a parameter
    def getTypeNameTuple(self, param):
        t = ''
        n = ''
        for elem in param:
            if elem.tag == 'type':
                t = noneStr(elem.text)
            elif elem.tag == 'name':
                n = noneStr(elem.text)
        return (t, n)

    # Convert an XML dependency expression to a C expression, taking a callback to replace extension names
    # See https://registry.khronos.org/vulkan/specs/1.4/registry.html#depends-expressions
    @staticmethod
    def ConvertDependencyExpression(expr, replace_func):
        # '(' and ')' can pass through unchanged
        expr = re.sub(',', ' || ', expr)
        expr = re.sub(r'\+', ' && ', expr)
        expr = re.sub(r'\w+', lambda match: replace_func(match.group()), expr)
        return expr

    def OutputPrototypesInHeader(self):
        protos = ''
        protos += '// Structures defined externally, but used here\n'
        protos += 'struct loader_instance;\n'
        protos += 'struct loader_device;\n'
        protos += 'struct loader_icd_term;\n'
        protos += 'struct loader_dev_dispatch_table;\n'
        protos += '\n'
        protos += '// Device extension error function\n'
        protos += 'VKAPI_ATTR VkResult VKAPI_CALL vkDevExtError(VkDevice dev);\n'
        protos += '\n'
        protos += '// Extension interception for vkGetInstanceProcAddr function, so we can return\n'
        protos += '// the appropriate information for any instance extensions we know about.\n'
        protos += 'bool extension_instance_gpa(struct loader_instance *ptr_instance, const char *name, void **addr);\n'
        protos += '\n'
        protos += '// Extension interception for vkCreateInstance function, so we can properly\n'
        protos += '// detect and enable any instance extension information for extensions we know\n'
        protos += '// about.\n'
        protos += 'void extensions_create_instance(struct loader_instance *ptr_instance, const VkInstanceCreateInfo *pCreateInfo);\n'
        protos += '\n'
        protos += '// Extension interception for vkGetDeviceProcAddr function, so we can return\n'
        protos += '// an appropriate terminator if this is one of those few device commands requiring\n'
        protos += '// a terminator.\n'
        protos += 'PFN_vkVoidFunction get_extension_device_proc_terminator(struct loader_device *dev, const char *name, bool* found_name);\n'
        protos += '\n'
        protos += '// Dispatch table properly filled in with appropriate terminators for the\n'
        protos += '// supported extensions.\n'
        protos += 'extern const VkLayerInstanceDispatchTable instance_disp;\n'
        protos += '\n'
        protos += '// Array of extension strings for instance extensions we support.\n'
        protos += 'extern const char *const LOADER_INSTANCE_EXTENSIONS[];\n'
        protos += '\n'
        protos += 'VKAPI_ATTR bool VKAPI_CALL loader_icd_init_entries(struct loader_instance* inst, struct loader_icd_term *icd_term);\n'
        protos += '\n'
        protos += '// Init Device function pointer dispatch table with core commands\n'
        protos += 'VKAPI_ATTR void VKAPI_CALL loader_init_device_dispatch_table(struct loader_dev_dispatch_table *dev_table, PFN_vkGetDeviceProcAddr gpa,\n'
        protos += '                                                             VkDevice dev);\n'
        protos += '\n'
        protos += '// Init Device function pointer dispatch table with extension commands\n'
        protos += 'VKAPI_ATTR void VKAPI_CALL loader_init_device_extension_dispatch_table(struct loader_dev_dispatch_table *dev_table,\n'
        protos += '                                                                       PFN_vkGetInstanceProcAddr gipa,\n'
        protos += '                                                                       PFN_vkGetDeviceProcAddr gdpa,\n'
        protos += '                                                                       VkInstance inst,\n'
        protos += '                                                                       VkDevice dev);\n'
        protos += '\n'
        protos += '// Init Instance function pointer dispatch table with core commands\n'
        protos += 'VKAPI_ATTR void VKAPI_CALL loader_init_instance_core_dispatch_table(VkLayerInstanceDispatchTable *table, PFN_vkGetInstanceProcAddr gpa,\n'
        protos += '                                                                    VkInstance inst);\n'
        protos += '\n'
        protos += '// Init Instance function pointer dispatch table with core commands\n'
        protos += 'VKAPI_ATTR void VKAPI_CALL loader_init_instance_extension_dispatch_table(VkLayerInstanceDispatchTable *table, PFN_vkGetInstanceProcAddr gpa,\n'
        protos += '                                                                         VkInstance inst);\n'
        protos += '\n'
        protos += '// Device command lookup function\n'
        protos += 'VKAPI_ATTR void* VKAPI_CALL loader_lookup_device_dispatch_table(const VkLayerDispatchTable *table, const char *name, bool* name_found);\n'
        protos += '\n'
        protos += '// Instance command lookup function\n'
        protos += 'VKAPI_ATTR void* VKAPI_CALL loader_lookup_instance_dispatch_table(const VkLayerInstanceDispatchTable *table, const char *name,\n'
        protos += '                                                                  bool *found_name);\n'
        protos += '\n'
        return protos

    def OutputUtilitiesInSource(self):
        protos = ''
        protos += '// Device extension error function\n'
        protos += 'VKAPI_ATTR VkResult VKAPI_CALL vkDevExtError(VkDevice dev) {\n'
        protos += '    struct loader_device *found_dev;\n'
        protos += '    // The device going in is a trampoline device\n'
        protos += '    struct loader_icd_term *icd_term = loader_get_icd_and_device(dev, &found_dev);\n'
        protos += '\n'
        protos += '    if (icd_term)\n'
        protos += '        loader_log(icd_term->this_instance, VULKAN_LOADER_ERROR_BIT, 0,\n'
        protos += '                   "Bad destination in loader trampoline dispatch,"\n'
        protos += '                   "Are layers and extensions that you are calling enabled?");\n'
        protos += '    return VK_ERROR_EXTENSION_NOT_PRESENT;\n'
        protos += '}\n\n'
        return protos

    #
    # Create a layer instance dispatch table from the appropriate list and return it as a string
    def OutputLayerInstanceDispatchTable(self):
        commands = []
        table = ''
        cur_extension_name = ''

        table += '// Instance function pointer dispatch table\n'
        table += 'typedef struct VkLayerInstanceDispatchTable_ {\n'

        # First add in an entry for GetPhysicalDeviceProcAddr.  This will not
        # ever show up in the XML or header, so we have to manually add it.
        table += '    // Manually add in GetPhysicalDeviceProcAddr entry\n'
        table += '    PFN_GetPhysicalDeviceProcAddr GetPhysicalDeviceProcAddr;\n'

        for x in range(0, 2):
            if x == 0:
                commands = self.core_commands
            else:
                commands = self.ext_commands

            for cur_cmd in commands:
                version = self.getAPIVersion(cur_cmd.ext_name)
                is_inst_handle_type = cur_cmd.name in ADD_INST_CMDS or cur_cmd.handle_type == 'VkInstance' or cur_cmd.handle_type == 'VkPhysicalDevice'
                if is_inst_handle_type:

                    if cur_cmd.ext_name != cur_extension_name:
                        if version:
                            table += f'\n    // ---- Core {version.name} commands\n'
                        else:
                            table += f'\n    // ---- {cur_cmd.ext_name} extension commands\n'
                        cur_extension_name = cur_cmd.ext_name

                    # Remove 'vk' from proto name
                    base_name = cur_cmd.name[2:]

                    if cur_cmd.protect is not None:
                        table += f'#if defined({cur_cmd.protect})\n'

                    table += f'    PFN_{cur_cmd.name} {base_name};\n'

                    if cur_cmd.protect is not None:
                        table += f'#endif // {cur_cmd.protect}\n'

        table += '} VkLayerInstanceDispatchTable;\n\n'
        return table

    #
    # Create a layer device dispatch table from the appropriate list and return it as a string
    def OutputLayerDeviceDispatchTable(self):
        commands = []
        table = ''
        cur_extension_name = ''

        table += '// Device function pointer dispatch table\n'
        table += '#define DEVICE_DISP_TABLE_MAGIC_NUMBER 0x10ADED040410ADEDUL\n'
        table += 'typedef struct VkLayerDispatchTable_ {\n'
        table += '    uint64_t magic; // Should be DEVICE_DISP_TABLE_MAGIC_NUMBER\n'

        for x in range(0, 2):
            if x == 0:
                commands = self.core_commands
            else:
                commands = self.ext_commands

            for cur_cmd in commands:
                version = self.getAPIVersion(cur_cmd.ext_name)
                is_inst_handle_type = cur_cmd.name in ADD_INST_CMDS or cur_cmd.handle_type == 'VkInstance' or cur_cmd.handle_type == 'VkPhysicalDevice'
                if not is_inst_handle_type:

                    if cur_cmd.ext_name != cur_extension_name:
                        if version:
                            table += f'\n    // ---- Core {version.name} commands\n'
                        else:
                            table += f'\n    // ---- {cur_cmd.ext_name} extension commands\n'
                        cur_extension_name = cur_cmd.ext_name

                    # Remove 'vk' from proto name
                    base_name = cur_cmd.name[2:]

                    if cur_cmd.protect is not None:
                        table += f'#if defined({cur_cmd.protect})\n'

                    table += f'    PFN_{cur_cmd.name} {base_name};\n'
                    if cur_cmd.protect is not None:
                        table += f'#endif // {cur_cmd.protect}\n'

        table += '} VkLayerDispatchTable;\n\n'
        return table

    #
    # Common code between the dispatch table struct and the function filling out said struct
    def ShouldPrintInIcdDispatchTable(self, cur_cmd, skip_list):
        return cur_cmd.name == 'vkGetDeviceProcAddr' or \
            (cur_cmd.handle_type not in ['VkDevice', 'VkCommandBuffer', 'VkQueue'] and cur_cmd.name not in skip_list)

    #
    # Create a dispatch table from the appropriate list and return it as a string
    def OutputIcdDispatchTable(self):
        commands = []
        table = ''
        cur_extension_name = ''

        skip_commands = ['vkGetInstanceProcAddr',
                         'vkEnumerateDeviceLayerProperties',
                        ]

        table += '// ICD function pointer dispatch table\n'
        table += 'struct loader_icd_term_dispatch {\n'

        for x in range(0, 2):
            if x == 0:
                commands = self.core_commands
            else:
                commands = self.ext_commands

            for cur_cmd in commands:
                version = self.getAPIVersion(cur_cmd.ext_name)
                if self.ShouldPrintInIcdDispatchTable(cur_cmd, skip_commands):
                    if cur_cmd.ext_name != cur_extension_name:
                        if version:
                            table += f'\n    // ---- Core {version.name} commands\n'
                        else:
                            table += f'\n    // ---- {cur_cmd.ext_name} extension commands\n'
                        cur_extension_name = cur_cmd.ext_name

                    # Remove 'vk' from proto name
                    base_name = cur_cmd.name[2:]

                    if cur_cmd.protect is not None:
                        table += f'#if defined({cur_cmd.protect})\n'

                    table += f'    PFN_{cur_cmd.name} {base_name};\n'

                    if cur_cmd.protect is not None:
                        table += f'#endif // {cur_cmd.protect}\n'

        table += '};\n\n'
        return table

    #
    # Init a dispatch table from the appropriate list and return it as a string
    def OutputIcdDispatchTableInit(self):
        commands = []
        cur_extension_name = ''

        table = ''
        table += 'VKAPI_ATTR bool VKAPI_CALL loader_icd_init_entries(struct loader_instance* inst, struct loader_icd_term *icd_term) {\n'
        table += '    const PFN_vkGetInstanceProcAddr fp_gipa = icd_term->scanned_icd->GetInstanceProcAddr;\n'
        table += '\n'
        table += '#define LOOKUP_GIPA(func) icd_term->dispatch.func = (PFN_vk##func)fp_gipa(icd_term->instance, "vk" #func);\n'
        table += '\n'
        table += '#define LOOKUP_REQUIRED_GIPA(func)                                                      \\\n'
        table += '    do {                                                                                \\\n'
        table += '        LOOKUP_GIPA(func);                                                              \\\n'
        table += '        if (!icd_term->dispatch.func) {                                                 \\\n'
        table += '            loader_log(inst, VULKAN_LOADER_WARN_BIT, 0, "Unable to load %s from ICD %s",\\\n'
        table += '                       "vk"#func, icd_term->scanned_icd->lib_name);                     \\\n'
        table += '            return false;                                                               \\\n'
        table += '        }                                                                               \\\n'
        table += '    } while (0)\n'
        table += '\n'


        skip_gipa_commands = ['vkGetInstanceProcAddr',
                              'vkEnumerateDeviceLayerProperties',
                              'vkCreateInstance',
                              'vkEnumerateInstanceExtensionProperties',
                              'vkEnumerateInstanceLayerProperties',
                              'vkEnumerateInstanceVersion',
                             ]

        for x in range(0, 2):
            if x == 0:
                commands = self.core_commands
            else:
                commands = self.ext_commands

            required = False
            for cur_cmd in commands:
                version = self.getAPIVersion(cur_cmd.ext_name)
                if self.ShouldPrintInIcdDispatchTable(cur_cmd, skip_gipa_commands):

                    if cur_cmd.ext_name != cur_extension_name:
                        if version:
                            table += f'\n    // ---- Core {version.name}\n'
                            required = version.number == '1.0'
                        else:
                            table += f'\n    // ---- {cur_cmd.ext_name} extension commands\n'
                            required = False
                        cur_extension_name = cur_cmd.ext_name

                    # Remove 'vk' from proto name
                    base_name = cur_cmd.name[2:]

                    if cur_cmd.protect is not None:
                        table += f'#if defined({cur_cmd.protect})\n'

                    if required:
                        # The Core Vulkan code will be wrapped in a feature called VK_VERSION_#_#
                        # For example: VK_VERSION_1_0 wraps the core 1.0 Vulkan functionality
                        table += f'    LOOKUP_REQUIRED_GIPA({base_name});\n'
                    else:
                        table += f'    LOOKUP_GIPA({base_name});\n'
                    if cur_cmd.protect is not None:
                        table += f'#endif // {cur_cmd.protect}\n'

        table += '\n'
        table += '#undef LOOKUP_REQUIRED_GIPA\n'
        table += '#undef LOOKUP_GIPA\n'
        table += '\n'
        table += '    return true;\n'
        table += '};\n\n'
        return table

    #
    # Create the extension enable union
    def OutputIcdExtensionEnableUnion(self):
        extensions = self.instanceExtensions

        union = ''
        union += 'struct loader_instance_extension_enables {\n'
        for ext in extensions:
            if (self.getAPIVersion(ext.name) or ext.name in WSI_EXT_NAMES or
                ext.type == 'device' or ext.num_commands == 0):
                continue

            union += f'    uint8_t {ext.name[3:].lower()};\n'

        union += '};\n\n'
        return union

    #
    # Creates the prototypes for the loader's core instance command terminators
    def OutputLoaderTerminators(self):
        terminators = ''
        terminators += '// Loader core instance terminators\n'

        for cur_cmd in self.core_commands:
            is_inst_handle_type = cur_cmd.name in ADD_INST_CMDS or cur_cmd.handle_type == 'VkInstance' or cur_cmd.handle_type == 'VkPhysicalDevice'
            if is_inst_handle_type:
                mod_string = ''
                new_terminator = cur_cmd.cdecl
                mod_string = new_terminator.replace("VKAPI_CALL vk", "VKAPI_CALL terminator_")

                if cur_cmd.name in PRE_INSTANCE_FUNCTIONS:
                    pre_instance_basic_version = mod_string
                    mod_string = mod_string.replace("terminator_", "terminator_pre_instance_")
                    mod_string = mod_string.replace(cur_cmd.name[2:] + '(\n', cur_cmd.name[2:] + '(\n    const Vk' + cur_cmd.name[2:] + 'Chain* chain,\n')

                if cur_cmd.protect is not None:
                    terminators += f'#if defined({cur_cmd.protect})\n'

                if cur_cmd.name in PRE_INSTANCE_FUNCTIONS:
                    terminators += pre_instance_basic_version
                    terminators += '\n'

                terminators += mod_string
                terminators += '\n'

                if cur_cmd.protect is not None:
                    terminators += f'#endif // {cur_cmd.protect}\n'

        terminators += '\n'
        return terminators

    #
    # Creates code to initialize the various dispatch tables
    def OutputLoaderDispatchTables(self):
        commands = []
        tables = ''
        gpa_param = ''
        cur_type = ''
        cur_extension_name = ''

        for x in range(0, 4):
            if x == 0:
                cur_type = 'device'
                gpa_param = 'dev'
                commands = self.core_commands

                tables += '// Init Device function pointer dispatch table with core commands\n'
                tables += 'VKAPI_ATTR void VKAPI_CALL loader_init_device_dispatch_table(struct loader_dev_dispatch_table *dev_table, PFN_vkGetDeviceProcAddr gpa,\n'
                tables += '                                                             VkDevice dev) {\n'
                tables += '    VkLayerDispatchTable *table = &dev_table->core_dispatch;\n'
                tables += '    if (table->magic != DEVICE_DISP_TABLE_MAGIC_NUMBER) { abort(); }\n'
                tables += '    for (uint32_t i = 0; i < MAX_NUM_UNKNOWN_EXTS; i++) dev_table->ext_dispatch[i] = (PFN_vkDevExt)vkDevExtError;\n'

            elif x == 1:
                cur_type = 'device'
                gpa_param = 'dev'
                commands = self.ext_commands

                tables += '// Init Device function pointer dispatch table with extension commands\n'
                tables += 'VKAPI_ATTR void VKAPI_CALL loader_init_device_extension_dispatch_table(struct loader_dev_dispatch_table *dev_table,\n'
                tables += '                                                                       PFN_vkGetInstanceProcAddr gipa,\n'
                tables += '                                                                       PFN_vkGetDeviceProcAddr gdpa,\n'
                tables += '                                                                       VkInstance inst,\n'
                tables += '                                                                       VkDevice dev) {\n'
                tables += '    VkLayerDispatchTable *table = &dev_table->core_dispatch;\n'
                tables += '    table->magic = DEVICE_DISP_TABLE_MAGIC_NUMBER;\n'

            elif x == 2:
                cur_type = 'instance'
                gpa_param = 'inst'
                commands = self.core_commands

                tables += '// Init Instance function pointer dispatch table with core commands\n'
                tables += 'VKAPI_ATTR void VKAPI_CALL loader_init_instance_core_dispatch_table(VkLayerInstanceDispatchTable *table, PFN_vkGetInstanceProcAddr gpa,\n'
                tables += '                                                                    VkInstance inst) {\n'

            else:
                cur_type = 'instance'
                gpa_param = 'inst'
                commands = self.ext_commands

                tables += '// Init Instance function pointer dispatch table with core commands\n'
                tables += 'VKAPI_ATTR void VKAPI_CALL loader_init_instance_extension_dispatch_table(VkLayerInstanceDispatchTable *table, PFN_vkGetInstanceProcAddr gpa,\n'
                tables += '                                                                        VkInstance inst) {\n'

            for cur_cmd in commands:
                version = self.getAPIVersion(cur_cmd.ext_name)
                is_inst_handle_type = cur_cmd.handle_type == 'VkInstance' or cur_cmd.handle_type == 'VkPhysicalDevice'
                if ((cur_type == 'instance' and is_inst_handle_type) or (cur_type == 'device' and not is_inst_handle_type)):
                    if cur_cmd.ext_name != cur_extension_name:
                        if version:
                            tables += f'\n    // ---- Core {version.name} commands\n'
                        else:
                            tables += f'\n    // ---- {cur_cmd.ext_name} extension commands\n'
                        cur_extension_name = cur_cmd.ext_name

                    # Remove 'vk' from proto name
                    base_name = cur_cmd.name[2:]

                    # Names to skip
                    if (base_name == 'CreateInstance' or base_name == 'CreateDevice' or
                        base_name == 'EnumerateInstanceExtensionProperties' or
                        base_name == 'EnumerateInstanceLayerProperties' or
                        base_name == 'EnumerateInstanceVersion'):
                        continue

                    if cur_cmd.protect is not None:
                        tables += f'#if defined({cur_cmd.protect})\n'

                    # If we're looking for the proc we are passing in, just point the table to it.  This fixes the issue where
                    # a layer overrides the function name for the loader.
                    if x == 1:
                        if base_name == 'GetDeviceProcAddr':
                            tables += '    table->GetDeviceProcAddr = gdpa;\n'
                        elif cur_cmd.ext_type == 'instance':
                            tables += f'    table->{base_name} = (PFN_{cur_cmd.name})gipa(inst, "{cur_cmd.name}");\n'
                        else:
                            tables += f'    table->{base_name} = (PFN_{cur_cmd.name})gdpa(dev, "{cur_cmd.name}");\n'
                    elif (x < 1 and base_name == 'GetDeviceProcAddr'):
                        tables += '    table->GetDeviceProcAddr = gpa;\n'
                    elif (x > 1 and base_name == 'GetInstanceProcAddr'):
                        tables += '    table->GetInstanceProcAddr = gpa;\n'
                    else:
                        tables += f'    table->{base_name} = (PFN_{cur_cmd.name})gpa({gpa_param}, "{cur_cmd.name}");\n'

                    if cur_cmd.protect is not None:
                        tables += f'#endif // {cur_cmd.protect}\n'

            tables += '}\n\n'
        return tables

    #
    # Create a lookup table function from the appropriate list of entrypoints and
    # return it as a string
    def OutputLoaderLookupFunc(self):
        commands = []
        tables = ''
        cur_type = ''
        cur_extension_name = ''

        for x in range(0, 2):
            if x == 0:
                cur_type = 'device'

                tables += '// Device command lookup function\n'
                tables += 'VKAPI_ATTR void* VKAPI_CALL loader_lookup_device_dispatch_table(const VkLayerDispatchTable *table, const char *name, bool* found_name) {\n'
                tables += '    if (!name || name[0] != \'v\' || name[1] != \'k\') {\n'
                tables += '        *found_name = false;\n'
                tables += '        return NULL;\n'
                tables += '    }\n'
                tables += '\n'
                tables += '    name += 2;\n'
                tables += '    *found_name = true;\n'
                tables += '    struct loader_device* dev = (struct loader_device *)table;\n'
                tables += '    const struct loader_instance* inst = dev->phys_dev_term->this_icd_term->this_instance;\n'
                tables += '    uint32_t api_version = VK_MAKE_API_VERSION(0, inst->app_api_version.major, inst->app_api_version.minor, inst->app_api_version.patch);\n'
                tables += '\n'
            else:
                cur_type = 'instance'

                tables += '// Instance command lookup function\n'
                tables += 'VKAPI_ATTR void* VKAPI_CALL loader_lookup_instance_dispatch_table(const VkLayerInstanceDispatchTable *table, const char *name,\n'
                tables += '                                                                 bool *found_name) {\n'
                tables += '    if (!name || name[0] != \'v\' || name[1] != \'k\') {\n'
                tables += '        *found_name = false;\n'
                tables += '        return NULL;\n'
                tables += '    }\n'
                tables += '\n'
                tables += '    *found_name = true;\n'
                tables += '    name += 2;\n'


            for y in range(0, 2):
                if y == 0:
                    commands = self.core_commands
                else:
                    commands = self.ext_commands

                for cur_cmd in commands:
                    version = self.getAPIVersion(cur_cmd.ext_name)
                    is_inst_handle_type = cur_cmd.handle_type == 'VkInstance' or cur_cmd.handle_type == 'VkPhysicalDevice'
                    if ((cur_type == 'instance' and is_inst_handle_type) or (cur_type == 'device' and not is_inst_handle_type)):
                        if cur_cmd.ext_name != cur_extension_name:
                            if version:
                                tables += f'\n    // ---- Core {version.name} commands\n'
                                if cur_type == 'device':
                                    version_check = f'        if (dev->should_ignore_device_commands_from_newer_version && api_version < {version.constant}) return NULL;\n'
                            else:

                                tables += f'\n    // ---- {cur_cmd.ext_name} extension commands\n'
                                version_check = ''
                            cur_extension_name = cur_cmd.ext_name

                        # Remove 'vk' from proto name
                        base_name = cur_cmd.name[2:]

                        if (base_name == 'CreateInstance' or base_name == 'CreateDevice' or
                            base_name == 'EnumerateInstanceExtensionProperties' or
                            base_name == 'EnumerateInstanceLayerProperties' or
                            base_name == 'EnumerateInstanceVersion'):
                            continue

                        if cur_cmd.protect is not None:
                            tables += f'#if defined({cur_cmd.protect})\n'

                        tables += f'    if (!strcmp(name, "{base_name}")) '
                        if cur_cmd.name in DEVICE_CMDS_MUST_USE_TRAMP:
                            if version_check != '':
                                tables += f'{{\n{version_check}        return dev->layer_extensions.{cur_cmd.ext_name[3:].lower()}_enabled ? (void *){base_name} : NULL;\n    }}\n'
                            else:
                                tables += f'return dev->layer_extensions.{cur_cmd.ext_name[3:].lower()}_enabled ? (void *){base_name} : NULL;\n'

                        else:
                            if version_check != '':
                                tables += f'{{\n{version_check}        return (void *)table->{base_name};\n    }}\n'
                            else:
                                tables += f'return (void *)table->{base_name};\n'

                        if cur_cmd.protect is not None:
                            tables += f'#endif // {cur_cmd.protect}\n'

            tables += '\n'
            tables += '    *found_name = false;\n'
            tables += '    return NULL;\n'
            tables += '}\n\n'
        return tables

    #
    # Create the appropriate trampoline (and possibly terminator) functions
    def CreateTrampTermFuncs(self):
        funcs = ''
        cur_extension_name = ''

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

        for ext_cmd in self.ext_commands:
            if (ext_cmd.ext_name in WSI_EXT_NAMES or
                ext_cmd.ext_name in AVOID_EXT_NAMES or
                ext_cmd.name in AVOID_CMD_NAMES or
                ext_cmd.name in manual_ext_commands):
                continue

            version = self.getAPIVersion(ext_cmd.ext_name)
            if ext_cmd.ext_name != cur_extension_name:
                if version:
                    funcs += f'\n// ---- Core {version.name} trampoline/terminators\n\n'
                else:
                    funcs += f'\n// ---- {ext_cmd.ext_name} extension trampoline/terminators\n\n'
                cur_extension_name = ext_cmd.ext_name

            if ext_cmd.protect is not None:
                funcs += f'#if defined({ext_cmd.protect})\n'

            func_header = ext_cmd.cdecl.replace(";", " {\n")
            tramp_header = func_header.replace("VKAPI_CALL vk", "VKAPI_CALL ")
            return_prefix = '    '
            base_name = ext_cmd.name[2:]
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

            for param in ext_cmd.params:
                if param.type == 'VkSurfaceKHR':
                    has_surface = 1
                    surface_var_name = param.name
                    requires_terminator = 1
                    always_use_param_name = False
                    surface_type_to_replace = 'VkSurfaceKHR'
                    surface_name_replacement = 'icd_term->surface_list[icd_surface->surface_index]'
                if param.type == 'VkPhysicalDeviceSurfaceInfo2KHR':
                    has_surface = 1
                    surface_var_name = param.name + '->surface'
                    requires_terminator = 1
                    update_structure_surface = 1
                    update_structure_string = '        VkPhysicalDeviceSurfaceInfo2KHR info_copy = *pSurfaceInfo;\n'
                    update_structure_string += '        info_copy.surface = icd_term->surface_list[icd_surface->surface_index];\n'
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

            if ext_cmd.return_type is not None:
                return_prefix += 'return '
                has_return_type = True

            if (ext_cmd.handle_type == 'VkInstance' or ext_cmd.handle_type == 'VkPhysicalDevice' or
                'DebugMarkerSetObject' in ext_cmd.name or 'SetDebugUtilsObject' in ext_cmd.name or
                ext_cmd.name in DEVICE_CMDS_NEED_TERM):
                requires_terminator = 1

            if requires_terminator == 1:
                term_header = tramp_header.replace("VKAPI_CALL ", "VKAPI_CALL terminator_")

                funcs += tramp_header

                if ext_cmd.handle_type == 'VkPhysicalDevice':
                    funcs += '    const VkLayerInstanceDispatchTable *disp;\n'
                    funcs += f'    VkPhysicalDevice unwrapped_phys_dev = loader_unwrap_physical_device({phys_dev_var_name});\n'
                    funcs += '    if (VK_NULL_HANDLE == unwrapped_phys_dev) {\n'
                    funcs += '        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,\n'
                    funcs += f'                   "{ext_cmd.name}: Invalid {phys_dev_var_name} "\n'
                    funcs += f'                   "[VUID-{ext_cmd.name}-{phys_dev_var_name}-parameter]");\n'
                    funcs += '        abort(); /* Intentionally fail so user can correct issue. */\n'
                    funcs += '    }\n'
                    funcs += f'    disp = loader_get_instance_layer_dispatch({phys_dev_var_name});\n'
                elif ext_cmd.handle_type == 'VkInstance':
                    funcs += f'    struct loader_instance *inst = loader_get_instance({instance_var_name});\n'
                    funcs += '    if (NULL == inst) {\n'
                    funcs += '        loader_log(\n'
                    funcs += '            NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,\n'
                    funcs += f'            "{ext_cmd.name}: Invalid instance [VUID-{ext_cmd.name}-{instance_var_name}-parameter]");\n'
                    funcs += '        abort(); /* Intentionally fail so user can correct issue. */\n'
                    funcs += '    }\n'
                    funcs += '#error("Not implemented. Likely needs to be manually generated!");\n'
                else:
                    funcs += '    const VkLayerDispatchTable *disp = loader_get_dispatch('
                    funcs += ext_cmd.params[0].name
                    funcs += ');\n'
                    funcs += '    if (NULL == disp) {\n'
                    funcs += '        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,\n'
                    funcs += f'                   "{ext_cmd.name}: Invalid {ext_cmd.params[0].name} "\n'
                    funcs += f'                   "[VUID-{ext_cmd.name}-{ext_cmd.params[0].name}-parameter]");\n'
                    funcs += '        abort(); /* Intentionally fail so user can correct issue. */\n'
                    funcs += '    }\n'

                if 'DebugMarkerSetObjectName' in ext_cmd.name:
                    funcs += '    VkDebugMarkerObjectNameInfoEXT local_name_info;\n'
                    funcs += '    memcpy(&local_name_info, pNameInfo, sizeof(VkDebugMarkerObjectNameInfoEXT));\n'
                    funcs += '    // If this is a physical device, we have to replace it with the proper one for the next call.\n'
                    funcs += '    if (pNameInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT) {\n'
                    funcs += '        struct loader_physical_device_tramp *phys_dev_tramp = (struct loader_physical_device_tramp *)(uintptr_t)pNameInfo->object;\n'
                    funcs += '        local_name_info.object = (uint64_t)(uintptr_t)phys_dev_tramp->phys_dev;\n'
                    funcs += '    }\n'
                    funcs += '    if (pNameInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT) {\n'
                    funcs += '        struct loader_instance* instance = (struct loader_instance *)(uintptr_t)pNameInfo->object;\n'
                    funcs += '        local_name_info.object = (uint64_t)(uintptr_t)instance->instance;\n'
                    funcs += '    }\n'
                elif 'DebugMarkerSetObjectTag' in ext_cmd.name:
                    funcs += '    VkDebugMarkerObjectTagInfoEXT local_tag_info;\n'
                    funcs += '    memcpy(&local_tag_info, pTagInfo, sizeof(VkDebugMarkerObjectTagInfoEXT));\n'
                    funcs += '    // If this is a physical device, we have to replace it with the proper one for the next call.\n'
                    funcs += '    if (pTagInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT) {\n'
                    funcs += '        struct loader_physical_device_tramp *phys_dev_tramp = (struct loader_physical_device_tramp *)(uintptr_t)pTagInfo->object;\n'
                    funcs += '        local_tag_info.object = (uint64_t)(uintptr_t)phys_dev_tramp->phys_dev;\n'
                    funcs += '    }\n'
                    funcs += '    if (pTagInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT) {\n'
                    funcs += '        struct loader_instance* instance = (struct loader_instance *)(uintptr_t)pTagInfo->object;\n'
                    funcs += '        local_tag_info.object = (uint64_t)(uintptr_t)instance->instance;\n'
                    funcs += '    }\n'
                elif 'SetDebugUtilsObjectName' in ext_cmd.name:
                    funcs += '    VkDebugUtilsObjectNameInfoEXT local_name_info;\n'
                    funcs += '    memcpy(&local_name_info, pNameInfo, sizeof(VkDebugUtilsObjectNameInfoEXT));\n'
                    funcs += '    // If this is a physical device, we have to replace it with the proper one for the next call.\n'
                    funcs += '    if (pNameInfo->objectType == VK_OBJECT_TYPE_PHYSICAL_DEVICE) {\n'
                    funcs += '        struct loader_physical_device_tramp *phys_dev_tramp = (struct loader_physical_device_tramp *)(uintptr_t)pNameInfo->objectHandle;\n'
                    funcs += '        local_name_info.objectHandle = (uint64_t)(uintptr_t)phys_dev_tramp->phys_dev;\n'
                    funcs += '    }\n'
                    funcs += '    if (pNameInfo->objectType == VK_OBJECT_TYPE_INSTANCE) {\n'
                    funcs += '        struct loader_instance* instance = (struct loader_instance *)(uintptr_t)pNameInfo->objectHandle;\n'
                    funcs += '        local_name_info.objectHandle = (uint64_t)(uintptr_t)instance->instance;\n'
                    funcs += '    }\n'
                elif 'SetDebugUtilsObjectTag' in ext_cmd.name:
                    funcs += '    VkDebugUtilsObjectTagInfoEXT local_tag_info;\n'
                    funcs += '    memcpy(&local_tag_info, pTagInfo, sizeof(VkDebugUtilsObjectTagInfoEXT));\n'
                    funcs += '    // If this is a physical device, we have to replace it with the proper one for the next call.\n'
                    funcs += '    if (pTagInfo->objectType == VK_OBJECT_TYPE_PHYSICAL_DEVICE) {\n'
                    funcs += '        struct loader_physical_device_tramp *phys_dev_tramp = (struct loader_physical_device_tramp *)(uintptr_t)pTagInfo->objectHandle;\n'
                    funcs += '        local_tag_info.objectHandle = (uint64_t)(uintptr_t)phys_dev_tramp->phys_dev;\n'
                    funcs += '    }\n'
                    funcs += '    if (pTagInfo->objectType == VK_OBJECT_TYPE_INSTANCE) {\n'
                    funcs += '        struct loader_instance* instance = (struct loader_instance *)(uintptr_t)pTagInfo->objectHandle;\n'
                    funcs += '        local_tag_info.objectHandle = (uint64_t)(uintptr_t)instance->instance;\n'
                    funcs += '    }\n'

                if ext_cmd.ext_name in NULL_CHECK_EXT_NAMES:
                    funcs += '    if (disp->' + base_name + ' != NULL) {\n'
                    funcs += '    '
                funcs += return_prefix
                if ext_cmd.handle_type == 'VkInstance':
                    funcs += 'inst->'
                funcs += 'disp->'
                funcs += base_name
                funcs += '('
                count = 0
                for param in ext_cmd.params:
                    if count != 0:
                        funcs += ', '

                    if param.type == 'VkPhysicalDevice':
                        funcs += 'unwrapped_phys_dev'
                    elif ('DebugMarkerSetObject' in ext_cmd.name or 'SetDebugUtilsObject' in ext_cmd.name) and param.name == 'pNameInfo':
                        funcs += '&local_name_info'
                    elif ('DebugMarkerSetObject' in ext_cmd.name or 'SetDebugUtilsObject' in ext_cmd.name) and param.name == 'pTagInfo':
                        funcs += '&local_tag_info'
                    else:
                        funcs += param.name

                    count += 1
                funcs += ');\n'
                if ext_cmd.ext_name in NULL_CHECK_EXT_NAMES:
                    if ext_cmd.return_type is not None:
                        funcs += '    } else {\n'
                        funcs += '        return VK_SUCCESS;\n'
                    funcs += '    }\n'
                funcs += '}\n\n'

                funcs += term_header
                if ext_cmd.handle_type == 'VkPhysicalDevice':
                    funcs += f'    struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *){phys_dev_var_name};\n'
                    funcs += '    struct loader_icd_term *icd_term = phys_dev_term->this_icd_term;\n'
                    funcs += '    if (NULL == icd_term->dispatch.'
                    funcs += base_name
                    funcs += ') {\n'
                    fatal_error_bit = '' if ext_cmd.ext_type =='instance' and has_return_type else 'VULKAN_LOADER_FATAL_ERROR_BIT | '
                    funcs += f'        loader_log(icd_term->this_instance, {fatal_error_bit}VULKAN_LOADER_ERROR_BIT, 0,\n'
                    funcs += '                   "ICD associated with VkPhysicalDevice does not support '
                    funcs += base_name
                    funcs += '");\n'

                    # If this is an instance function taking a physical device (i.e. pre Vulkan 1.1), we need to behave and not crash so return an
                    # error here.
                    if ext_cmd.ext_type =='instance' and has_return_type:
                        funcs += '        return VK_ERROR_EXTENSION_NOT_PRESENT;\n'
                    else:
                        funcs += '        abort(); /* Intentionally fail so user can correct issue. */\n'
                    funcs += '    }\n'

                    if has_surface == 1:
                        funcs += '    VkIcdSurface *icd_surface = NULL;\n'
                        funcs += f'    if (NULL != {surface_var_name}) {{\n'
                        funcs += f'        icd_surface = (VkIcdSurface *)(uintptr_t)({surface_var_name});\n'
                        funcs += '    }\n'
                        funcs += '    if (NULL != icd_surface && NULL != icd_term->surface_list.list && icd_term->surface_list.capacity > icd_surface->surface_index * sizeof(VkSurfaceKHR) && icd_term->surface_list[icd_surface->surface_index]) {\n'

                        # If there's a structure with a surface, we need to update its internals with the correct surface for the ICD
                        if update_structure_surface == 1:
                            funcs += update_structure_string

                        funcs += '    ' + return_prefix + 'icd_term->dispatch.'
                        funcs += base_name
                        funcs += '('
                        count = 0
                        for param in ext_cmd.params:
                            if count != 0:
                                funcs += ', '

                            if not always_use_param_name:
                                if surface_type_to_replace and surface_type_to_replace == param.type:
                                    funcs += surface_name_replacement
                                elif physdev_type_to_replace and physdev_type_to_replace == param.type:
                                    funcs += physdev_name_replacement
                                else:
                                    funcs += param.name
                            else:
                                funcs += param.name

                            count += 1
                        funcs += ');\n'
                        if not has_return_type:
                            funcs += '        return;\n'
                        funcs += '    }\n'

                    funcs += return_prefix
                    funcs += 'icd_term->dispatch.'
                    funcs += base_name
                    funcs += '('
                    count = 0
                    for param in ext_cmd.params:
                        if count != 0:
                            funcs += ', '

                        if param.type == 'VkPhysicalDevice':
                            funcs += 'phys_dev_term->phys_dev'
                        else:
                            funcs += param.name

                        count += 1
                    funcs += ');\n'


                elif ext_cmd.handle_type == 'VkInstance':
                    funcs += f'    struct loader_instance *inst = loader_get_instance({instance_var_name});\n'
                    funcs += '    if (NULL == inst) {\n'
                    funcs += '        loader_log(\n'
                    funcs += '            NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,\n'
                    funcs += f'            "{ext_cmd.name}: Invalid instance [VUID-{ext_cmd.name}-{instance_var_name}-parameter]");\n'
                    funcs += '        abort(); /* Intentionally fail so user can correct issue. */\n'
                    funcs += '    }\n'
                    funcs += '#error("Not implemented. Likely needs to be manually generated!");\n'
                elif ext_cmd.ext_name in ['VK_EXT_debug_utils', 'VK_EXT_debug_marker']:
                    if ext_cmd.name in ['vkDebugMarkerSetObjectNameEXT', 'vkDebugMarkerSetObjectTagEXT', 'vkSetDebugUtilsObjectNameEXT' , 'vkSetDebugUtilsObjectTagEXT']:

                        is_debug_utils = ext_cmd.ext_name == "VK_EXT_debug_utils"
                        debug_struct_name = ext_cmd.params[1].name
                        local_struct = 'local_name_info' if 'ObjectName' in ext_cmd.name else 'local_tag_info'
                        member_name = 'objectHandle' if is_debug_utils else 'object'
                        phys_dev_check = 'VK_OBJECT_TYPE_PHYSICAL_DEVICE' if is_debug_utils else 'VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT'
                        surf_check = 'VK_OBJECT_TYPE_SURFACE_KHR' if is_debug_utils else 'VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT'
                        inst_check = 'VK_OBJECT_TYPE_INSTANCE' if is_debug_utils else 'VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT'
                        funcs += '    struct loader_device *dev;\n'
                        funcs += f'    struct loader_icd_term *icd_term = loader_get_icd_and_device({ ext_cmd.params[0].name}, &dev);\n'
                        funcs += '    if (NULL == icd_term || NULL == dev) {\n'
                        funcs += f'        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0, "{ext_cmd.name[2:]}: Invalid device handle");\n'
                        funcs += '        abort(); /* Intentionally fail so user can correct issue. */\n'
                        funcs += '    }\n'
                        funcs += f'    { ext_cmd.params[1].type} {local_struct};\n'
                        funcs += f'    memcpy(&{local_struct}, {debug_struct_name}, sizeof({ ext_cmd.params[1].type}));\n'
                        funcs += '    // If this is a physical device, we have to replace it with the proper one for the next call.\n'
                        funcs += f'    if ({debug_struct_name}->objectType == {phys_dev_check}) {{\n'
                        funcs += f'        struct loader_physical_device_term *phys_dev_term = (struct loader_physical_device_term *)(uintptr_t){debug_struct_name}->{member_name};\n'
                        funcs += f'        {local_struct}.{member_name} = (uint64_t)(uintptr_t)phys_dev_term->phys_dev;\n'
                        funcs += '    // If this is a KHR_surface, and the ICD has created its own, we have to replace it with the proper one for the next call.\n'
                        funcs += f'    }} else if ({debug_struct_name}->objectType == {surf_check}) {{\n'
                        funcs += '        if (NULL != dev && NULL != dev->loader_dispatch.core_dispatch.CreateSwapchainKHR) {\n'
                        funcs += f'            VkIcdSurface *icd_surface = (VkIcdSurface *)(uintptr_t){debug_struct_name}->{member_name};\n'
                        funcs += '            if (NULL != icd_term->surface_list.list && icd_term->surface_list.capacity > icd_surface->surface_index * sizeof(VkSurfaceKHR)\n'
                        funcs += '                && icd_term->surface_list.list[icd_surface->surface_index]) {\n'
                        funcs += f'                {local_struct}.{member_name} = (uint64_t)icd_term->surface_list.list[icd_surface->surface_index];\n'
                        funcs += '            }\n'
                        funcs += '        }\n'
                        funcs += '    // If this is an instance we have to replace it with the proper one for the next call.\n'
                        funcs += f'    }} else if ({debug_struct_name}->objectType == {inst_check}) {{\n'
                        funcs += f'        {local_struct}.{member_name} = (uint64_t)(uintptr_t)icd_term->instance;\n'
                        funcs += '    }\n'
                        funcs += '    // Exit early if the driver does not support the function - this can happen as a layer or the loader itself supports\n'
                        funcs += '    // debug utils but the driver does not.\n'
                        funcs += f'    if (NULL == dev->loader_dispatch.extension_terminator_dispatch.{ext_cmd.name[2:]})\n        return VK_SUCCESS;\n'
                        dispatch = 'dev->loader_dispatch.'
                    else:
                        funcs += f'    struct loader_dev_dispatch_table *dispatch_table = loader_get_dev_dispatch({ext_cmd.params[0].name});\n'
                        funcs += '    if (NULL == dispatch_table) {\n'
                        funcs += f'        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0, "{ext_cmd.ext_name}: Invalid device handle");\n'
                        funcs += '        abort(); /* Intentionally fail so user can correct issue. */\n'
                        funcs += '    }\n'
                        funcs += '    // Only call down if the device supports the function\n'
                        funcs += f'    if (NULL != dispatch_table->extension_terminator_dispatch.{base_name})\n    '
                        dispatch = 'dispatch_table->'
                    funcs += '    '
                    if has_return_type:
                        funcs += 'return '
                    funcs += f'{dispatch}extension_terminator_dispatch.{base_name}('
                    count = 0
                    for param in ext_cmd.params:
                        if count != 0:
                            funcs += ', '

                        if param.type == 'VkPhysicalDevice':
                            funcs += 'phys_dev_term->phys_dev'
                        elif param.type == 'VkSurfaceKHR':
                            funcs += 'icd_term->surface_list[icd_surface->surface_index]'
                        elif ('DebugMarkerSetObject' in ext_cmd.name or 'SetDebugUtilsObject' in ext_cmd.name) and param.name == 'pNameInfo':
                            funcs += '&local_name_info'
                        elif ('DebugMarkerSetObject' in ext_cmd.name or 'SetDebugUtilsObject' in ext_cmd.name) and param.name == 'pTagInfo':
                            funcs += '&local_tag_info'
                        else:
                            funcs += param.name
                        count += 1

                    funcs += ');\n'

                else:
                    funcs += '#error("Unknown error path!");\n'

                funcs += '}\n\n'
            else:
                funcs += tramp_header

                funcs += '    const VkLayerDispatchTable *disp = loader_get_dispatch('
                funcs += ext_cmd.params[0].name
                funcs += ');\n'
                funcs += '    if (NULL == disp) {\n'
                funcs += '        loader_log(NULL, VULKAN_LOADER_FATAL_ERROR_BIT | VULKAN_LOADER_ERROR_BIT | VULKAN_LOADER_VALIDATION_BIT, 0,\n'
                funcs += f'                   "{ext_cmd.name}: Invalid {ext_cmd.params[0].name} "\n'
                funcs += f'                   "[VUID-{ext_cmd.name}-{ext_cmd.params[0].name}-parameter]");\n'
                funcs += '        abort(); /* Intentionally fail so user can correct issue. */\n'
                funcs += '    }\n'

                if ext_cmd.ext_name in NULL_CHECK_EXT_NAMES:
                    funcs += '    if (disp->' + base_name + ' != NULL) {\n'
                    funcs += '    '
                funcs += return_prefix
                funcs += 'disp->'
                funcs += base_name
                funcs += '('
                count = 0
                for param in ext_cmd.params:
                    if count != 0:
                        funcs += ', '
                    funcs += param.name
                    count += 1
                funcs += ');\n'
                if ext_cmd.ext_name in NULL_CHECK_EXT_NAMES:
                    if ext_cmd.return_type is not None:
                        funcs += '    } else {\n'
                        funcs += '        return VK_SUCCESS;\n'
                    funcs += '    }\n'
                funcs += '}\n\n'

            if ext_cmd.protect is not None:
                funcs += f'#endif // {ext_cmd.protect}\n'

        return funcs


    #
    # Create a function for the extension GPA call
    def InstExtensionGPA(self):
        gpa_func = ''
        cur_extension_name = ''

        gpa_func += '// GPA helpers for extensions\n'
        gpa_func += 'bool extension_instance_gpa(struct loader_instance *ptr_instance, const char *name, void **addr) {\n'
        gpa_func += '    *addr = NULL;\n\n'

        for cur_cmd in self.ext_commands:
            if (self.getAPIVersion(cur_cmd.ext_name) or
                cur_cmd.ext_name in WSI_EXT_NAMES or
                cur_cmd.ext_name in AVOID_EXT_NAMES or
                cur_cmd.name in AVOID_CMD_NAMES ):
                continue

            if cur_cmd.ext_name != cur_extension_name:
                gpa_func += f'\n    // ---- {cur_cmd.ext_name} extension commands\n'
                cur_extension_name = cur_cmd.ext_name

            if cur_cmd.protect is not None:
                gpa_func += f'#if defined({cur_cmd.protect})\n'

            #base_name = cur_cmd.name[2:]
            base_name = SHARED_ALIASES[cur_cmd.name] if cur_cmd.name in SHARED_ALIASES else cur_cmd.name[2:]

            if cur_cmd.ext_type == 'instance':
                gpa_func += f'    if (!strcmp("{cur_cmd.name}", name)) {{\n'
                gpa_func += '        *addr = (ptr_instance->enabled_known_extensions.'
                gpa_func += cur_cmd.ext_name[3:].lower()
                gpa_func += ' == 1)\n'
                gpa_func += f'                     ? (void *){base_name}\n'
                gpa_func += '                     : NULL;\n'
                gpa_func += '        return true;\n'
                gpa_func += '    }\n'
            else:
                gpa_func += f'    if (!strcmp("{cur_cmd.name}", name)) {{\n'
                gpa_func += f'        *addr = (void *){base_name};\n'
                gpa_func += '        return true;\n'
                gpa_func += '    }\n'

            if cur_cmd.protect is not None:
                gpa_func += f'#endif // {cur_cmd.protect}\n'

        gpa_func += '    return false;\n'
        gpa_func += '}\n\n'

        return gpa_func

    #
    # Create the extension name init function
    def InstantExtensionCreate(self):
        entries = []
        entries = self.instanceExtensions
        count = 0
        cur_extension_name = ''

        create_func = ''
        create_func += '// A function that can be used to query enabled extensions during a vkCreateInstance call\n'
        create_func += 'void extensions_create_instance(struct loader_instance *ptr_instance, const VkInstanceCreateInfo *pCreateInfo) {\n'
        create_func += '    for (uint32_t i = 0; i < pCreateInfo->enabledExtensionCount; i++) {\n'
        for ext in entries:
            if (self.getAPIVersion(ext.name) or ext.name in WSI_EXT_NAMES or
                ext.name in AVOID_EXT_NAMES or ext.name in AVOID_CMD_NAMES or
                ext.type == 'device' or ext.num_commands == 0):
                continue

            if ext.name != cur_extension_name:
                create_func += f'\n    // ---- {ext.name} extension commands\n'
                cur_extension_name = ext.name

            if ext.protect is not None:
                create_func += f'#if defined({ext.protect})\n'
            if count == 0:
                create_func += '        if (0 == strcmp(pCreateInfo->ppEnabledExtensionNames[i], '
            else:
                create_func += '        } else if (0 == strcmp(pCreateInfo->ppEnabledExtensionNames[i], '

            create_func += ext.define + ')) {\n'
            create_func += '            ptr_instance->enabled_known_extensions.'
            create_func += ext.name[3:].lower()
            create_func += ' = 1;\n'

            if ext.protect is not None:
                create_func += f'#endif // {ext.protect}\n'
            count += 1

        create_func += '        }\n'
        create_func += '    }\n'
        create_func += '}\n\n'
        return create_func

    #
    # Create code to initialize a dispatch table from the appropriate list of
    # extension entrypoints and return it as a string
    def DeviceExtensionGetTerminator(self):
        term_func = ''

        term_func += '// Some device commands still need a terminator because the loader needs to unwrap something about them.\n'
        term_func += '// In many cases, the item needing unwrapping is a VkPhysicalDevice or VkSurfaceKHR object.  But there may be other items\n'
        term_func += '// in the future.\n'
        term_func += 'PFN_vkVoidFunction get_extension_device_proc_terminator(struct loader_device *dev, const char *name, bool* found_name) {\n'
        term_func += '''    *found_name = false;
    if (!name || name[0] != 'v' || name[1] != 'k') {
        return NULL;
    }
    name += 2;
'''
        last_protect = None
        last_ext = None
        for ext_cmd in self.ext_commands:
            version = self.getAPIVersion(ext_cmd.ext_name)
            if ext_cmd.name in DEVICE_CMDS_NEED_TERM:
                if version:
                    term_func += f'    // ---- Core {version.name} commands\n'
                else:
                    last_protect = ext_cmd.protect
                    if ext_cmd.protect is not None:
                        term_func += f'#if defined({ext_cmd.protect})\n'
                    if last_ext != ext_cmd.ext_name:
                        term_func += f'    // ---- {ext_cmd.ext_name} extension commands\n'
                        last_ext = ext_cmd.ext_name

                term_func += f'    if (!strcmp(name, "{ext_cmd.name[2:]}")) {{\n'
                term_func += '        *found_name = true;\n'
                if ext_cmd.require:
                    dep_expr = self.ConvertDependencyExpression(ext_cmd.require, lambda ext_name: f'dev->driver_extensions.{ext_name[3:].lower()}_enabled')
                    term_func += f'        return (dev->driver_extensions.{ext_cmd.ext_name[3:].lower()}_enabled && ({dep_expr})) ?\n'
                else:
                    term_func += f'        return dev->driver_extensions.{ext_cmd.ext_name[3:].lower()}_enabled ?\n'
                term_func += f'            (PFN_vkVoidFunction)terminator_{(ext_cmd.name[2:])} : NULL;\n'
                term_func += '    }\n'

        if last_protect is not None:
            term_func += f'#endif // {last_protect}\n'

        term_func += '    return NULL;\n'
        term_func += '}\n\n'

        return term_func

    #
    # Create a dispatch table solely for device functions which have custom terminators
    def OutputDeviceFunctionTerminatorDispatchTable(self):
        term_func = ''
        term_func += '// Functions that required a terminator need to have a separate dispatch table which contains their corresponding\n'
        term_func += '// device function. This is used in the terminators themselves.\n'
        term_func += 'struct loader_device_terminator_dispatch {\n'

        last_protect = None
        last_ext = None
        for ext_cmd in self.ext_commands:
            version = self.getAPIVersion(ext_cmd.ext_name)
            if ext_cmd.name in DEVICE_CMDS_NEED_TERM:
                if version:
                    term_func += f'    // ---- Core {version.name} commands\n'
                else:
                    last_protect = ext_cmd.protect
                    if ext_cmd.protect is not None:
                        term_func += f'#if defined({ext_cmd.protect})\n'
                    if last_ext != ext_cmd.ext_name:
                        term_func += f'    // ---- {ext_cmd.ext_name} extension commands\n'
                        last_ext = ext_cmd.ext_name

                term_func += f'    PFN_{ext_cmd.name} {ext_cmd.name[2:]};\n'

        if last_protect is not None:
            term_func += f'#endif // {last_protect}\n'

        term_func += '};\n\n'

        return term_func

    def OutputDeviceFunctionTrampolinePrototypes(self):
        tramp_protos = ''
        tramp_protos += '// These are prototypes for functions that need their trampoline called in all circumstances.\n'
        tramp_protos += '// They are used in loader_lookup_device_dispatch_table but are defined afterwards.\n'
        last_protect = None
        last_ext = None
        for ext_cmd in self.ext_commands:
            version = self.getAPIVersion(ext_cmd.ext_name)
            if ext_cmd.name in DEVICE_CMDS_MUST_USE_TRAMP:
                if version:
                    tramp_protos += f'    // ---- Core {version.name} commands\n'
                else:
                    last_protect = ext_cmd.protect
                    if ext_cmd.protect is not None:
                        tramp_protos += f'#if defined({ext_cmd.protect})\n'
                    if last_ext != ext_cmd.ext_name:
                        tramp_protos += f'    // ---- {ext_cmd.ext_name} extension commands\n'
                        last_ext = ext_cmd.ext_name

                tramp_protos += f'{ext_cmd.cdecl.replace("VKAPI_CALL vk", "VKAPI_CALL ")}\n'

        if last_protect is not None:
            tramp_protos += f'#endif // {last_protect}\n'
        tramp_protos += '\n'
        return tramp_protos

    #
    # Create code to initialize a dispatch table from the appropriate list of
    # extension entrypoints and return it as a string
    def InitDeviceFunctionTerminatorDispatchTable(self):
        term_func = ''

        term_func += '// Functions that required a terminator need to have a separate dispatch table which contains their corresponding\n'
        term_func += '// device function. This is used in the terminators themselves.\n'
        term_func += 'void init_extension_device_proc_terminator_dispatch(struct loader_device *dev) {\n'
        term_func += '    struct loader_device_terminator_dispatch* dispatch = &dev->loader_dispatch.extension_terminator_dispatch;\n'
        term_func += '    PFN_vkGetDeviceProcAddr gpda = (PFN_vkGetDeviceProcAddr)dev->phys_dev_term->this_icd_term->dispatch.GetDeviceProcAddr;\n'
        last_protect = None
        last_ext = None
        for ext_cmd in self.ext_commands:
            version = self.getAPIVersion(ext_cmd.ext_name)
            if ext_cmd.name in DEVICE_CMDS_NEED_TERM:
                if version:
                    term_func += f'    // ---- Core {version.name} commands\n'
                else:
                    last_protect = ext_cmd.protect
                    if ext_cmd.protect is not None:
                        term_func += f'#if defined({ext_cmd.protect})\n'
                    if last_ext != ext_cmd.ext_name:
                        term_func += f'    // ---- {ext_cmd.ext_name} extension commands\n'
                        last_ext = ext_cmd.ext_name


                if ext_cmd.require:
                    dep_expr = self.ConvertDependencyExpression(ext_cmd.require, lambda ext_name: f'dev->driver_extensions.{ext_name[3:].lower()}_enabled')
                    term_func += f'    if (dev->driver_extensions.{ext_cmd.ext_name[3:].lower()}_enabled && ({dep_expr}))\n'
                    term_func += f'       dispatch->{ext_cmd.name[2:]} = (PFN_{(ext_cmd.name)})gpda(dev->icd_device, "{(ext_cmd.name)}");\n'
                else:
                    term_func += f'    if (dev->driver_extensions.{ext_cmd.ext_name[3:].lower()}_enabled)\n'
                    term_func += f'       dispatch->{ext_cmd.name[2:]} = (PFN_{(ext_cmd.name)})gpda(dev->icd_device, "{(ext_cmd.name)}");\n'

        if last_protect is not None:
            term_func += f'#endif // {last_protect}\n'

        term_func += '}\n\n'

        return term_func

    #
    # Create code to initialize a dispatch table from the appropriate list of
    # core and extension entrypoints and return it as a string
    def InitInstLoaderExtensionDispatchTable(self):
        commands = []
        table = ''
        cur_extension_name = ''

        table += '// This table contains the loader\'s instance dispatch table, which contains\n'
        table += '// default functions if no instance layers are activated.  This contains\n'
        table += '// pointers to "terminator functions".\n'
        table += 'const VkLayerInstanceDispatchTable instance_disp = {\n'

        for x in range(0, 2):
            if x == 0:
                commands = self.core_commands
            else:
                commands = self.ext_commands

            for cur_cmd in commands:
                version = self.getAPIVersion(cur_cmd.ext_name)
                if cur_cmd.handle_type == 'VkInstance' or cur_cmd.handle_type == 'VkPhysicalDevice':
                    if cur_cmd.ext_name != cur_extension_name:
                        if version:
                            table += f'\n    // ---- Core {version.name} commands\n'
                        else:
                            table += f'\n    // ---- {cur_cmd.ext_name} extension commands\n'
                        cur_extension_name = cur_cmd.ext_name

                    # Remove 'vk' from proto name
                    base_name = cur_cmd.name[2:]
                    aliased_name = SHARED_ALIASES[cur_cmd.name][2:] if cur_cmd.name in SHARED_ALIASES else base_name

                    if (base_name == 'CreateInstance' or base_name == 'CreateDevice' or
                        base_name == 'EnumerateInstanceExtensionProperties' or
                        base_name == 'EnumerateInstanceLayerProperties' or
                        base_name == 'EnumerateInstanceVersion'):
                        continue

                    if cur_cmd.protect is not None:
                        table += f'#if defined({cur_cmd.protect})\n'

                    if base_name == 'GetInstanceProcAddr':
                        table += f'    .{base_name} = {cur_cmd.name},\n'
                    else:
                        table += f'    .{base_name} = terminator_{aliased_name},\n'

                    if cur_cmd.protect is not None:
                        table += f'#endif // {cur_cmd.protect}\n'
        table += '};\n\n'

        return table

    #
    # Create the extension name whitelist array
    def OutputInstantExtensionWhitelistArray(self):
        extensions = self.instanceExtensions

        table = ''
        table += '// A null-terminated list of all of the instance extensions supported by the loader.\n'
        table += '// If an instance extension name is not in this list, but it is exported by one or more of the\n'
        table += '// ICDs detected by the loader, then the extension name not in the list will be filtered out\n'
        table += '// before passing the list of extensions to the application.\n'
        table += 'const char *const LOADER_INSTANCE_EXTENSIONS[] = {\n'
        for ext in extensions:
            if ext.type == 'device' or self.getAPIVersion(ext.name):
                continue

            if ext.protect is not None:
                table += f'#if defined({ext.protect})\n'
            table += '                                                  '
            table += ext.define + ',\n'

            if ext.protect is not None:
                table += f'#endif // {ext.protect}\n'
        table += '                                                  NULL };\n'
        return table
