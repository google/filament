#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2017 The Khronos Group Inc.
# Copyright (c) 2015-2017 Valve Corporation
# Copyright (c) 2015-2017 LunarG, Inc.
# Copyright (c) 2015-2017 Google Inc.
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
# Author: Tobin Ehlis <tobine@google.com>
# Author: John Zulauf <jzulauf@lunarg.com>
# Author: Charles Giessen <charles@lunarg.com>

import re
import os
from base_generator import BaseGenerator

class HelperFileGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

        # Helper for VkDebugReportObjectTypeEXT
        # Maps [ 'VkBuffer' : 'VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT' ]
        # Will be 'VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT' if no type
        self.debugReportObject = dict()
        # Maps [ 'VkBuffer' : 'VK_OBJECT_TYPE_BUFFER' ]
        self.objectType = dict()

    # Takes a VK_OBJECT_TYPE_THE_TYP_EXT and turns it into TheTypeEXT
    def get_kVulkanObjectName(self, str_to_process):
        removed_prefix = str_to_process.split('_')[3:]
        if ''.join(removed_prefix[-1:]) in self.vk.vendorTags:
            return ''.join(str_to_process.title().split('_')[3:-1] + removed_prefix[-1:])
        else:
            return ''.join(str_to_process.title().split('_')[3:])

    def split_handle_by_capital_case(self, str_to_split):
        name = str_to_split[2:]
        name = name.replace('NVX', 'Nvx')
        # Force vendor tags to be title case so we can split by capital letter
        for tag in self.vk.vendorTags:
            name = name.replace(tag, tag.title())
        # Split by capital letters so we can concoct a VK_DEBUG_REPORT_<>_EXT string out of it
        return re.split('(?<=.)(?=[A-Z])', f'{name}')


    def convert_to_VK_OBJECT_TYPE(self, str_to_convert):
        return '_'.join(['VK_OBJECT_TYPE'] + self.split_handle_by_capital_case(str_to_convert)).upper()

    def convert_to_VK_DEBUG_REPORT_OBJECT_TYPE(self, str_to_convert):
        return '_'.join(['VK_DEBUG_REPORT_OBJECT_TYPE'] + self.split_handle_by_capital_case(str_to_convert) + ['EXT']).upper()

    def generate(self):
        # Search all fields of the Enum to see if has a DEBUG_REPORT_OBJECT
        for handle in self.vk.handles.values():
            debugObjects = ([enum.name for enum in self.vk.enums['VkDebugReportObjectTypeEXT'].fields if f'{handle.type[3:]}_EXT' in enum.name])
            object = 'VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT' if len(debugObjects) == 0 else debugObjects[0]
            self.debugReportObject[handle.name] = object



        out = []
        out.append(f'''// clang-format off
// *** THIS FILE IS GENERATED - DO NOT EDIT ***
// See {os.path.basename(__file__)} for modifications


/***************************************************************************
 *
 * Copyright (c) 2015-2025 The Khronos Group Inc.
 * Copyright (c) 2015-2025 Valve Corporation
 * Copyright (c) 2015-2025 LunarG, Inc.
 * Copyright (c) 2015-2017 Google Inc.
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
 * Author: Courtney Goeltzenleuchter <courtneygo@google.com>
 * Author: Tobin Ehlis <tobine@google.com>
 * Author: Chris Forbes <chrisforbes@google.com>
 * Author: John Zulauf<jzulauf@lunarg.com>
 * Author: Charles Giessen<charles@lunarg.com>
 *
 ****************************************************************************/


#pragma once

#include <vulkan/vulkan.h>

''')

        # Get the list of field names from VkDebugReportObjectTypeEXT
        VkDebugReportObjectTypeEXT_field_names = []
        for field in self.vk.enums['VkDebugReportObjectTypeEXT'].fields:
            VkDebugReportObjectTypeEXT_field_names.append(field.name)

        # Create a list with the needed information so that when we print, we print in the exact same order, as we are printing arrays where external code may index into it
        object_types = []
        object_type_aliases = {}
        enum_num = 1 # start at one since the zero place is manually printed with the UNKNOWN value
        for handle in self.vk.handles.values():
            debug_report_object_name = self.convert_to_VK_DEBUG_REPORT_OBJECT_TYPE(handle.name)
            if debug_report_object_name in VkDebugReportObjectTypeEXT_field_names:
                object_types.append((f'{handle.name[2:]}', enum_num, debug_report_object_name))
            else:
                object_types.append((f'{handle.name[2:]}', enum_num, 'VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT'))
            enum_num += 1
            if len(handle.aliases) > 0:
                object_type_aliases[handle.name] = handle.aliases

        out.append('// Object Type enum for validation layer internal object handling\n')
        out.append('typedef enum VulkanObjectType {\n')
        out.append('    kVulkanObjectTypeUnknown = 0,\n')
        for name, number, debug_report in object_types:
            out.append(f'    kVulkanObjectType{name} = {number},\n')
        out.append(f'    kVulkanObjectTypeMax = {enum_num},\n')

        out.append('    // Aliases for backwards compatibilty of "promoted" types\n')
        for name, aliases in object_type_aliases.items():
            for alias in aliases:
                out.append(f'    kVulkanObjectType{alias[2:]} = kVulkanObjectType{name[2:]},\n')

        out.append('} VulkanObjectType;\n')
        out.append('\n')

        out.append('// Array of object name strings for OBJECT_TYPE enum conversion\n')
        out.append('static const char * const object_string[kVulkanObjectTypeMax] = {\n')
        out.append('    \"Unknown\",\n')
        for name, number, debug_report in object_types:
            out.append(f'    \"{name}\",\n')
        out.append('};\n')
        out.append('\n')

        out.append('// Helper array to get Vulkan VK_EXT_debug_report object type enum from the internal layers version\n')
        out.append('const VkDebugReportObjectTypeEXT get_debug_report_enum[] = {\n')
        out.append('    VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT, // kVulkanObjectTypeUnknown\n')
        for name, number, debug_report in object_types:
            out.append(f'    {debug_report},   // kVulkanObjectType{name}\n')
        out.append('};\n')
        out.append('\n')

        out.append('// Helper array to get Official Vulkan VkObjectType enum from the internal layers version\n')
        out.append('const VkObjectType get_object_type_enum[] = {\n')
        out.append('    VK_OBJECT_TYPE_UNKNOWN, // kVulkanObjectTypeUnknown\n')
        for handle in self.vk.handles.values():
            object_name = self.convert_to_VK_OBJECT_TYPE(handle.name)
            out.append(f'    {object_name},   // kVulkanObjectType{handle.name[2:]}\n')
        out.append('};\n')
        out.append('\n')

        out.append('// Helper function to convert from VkDebugReportObjectTypeEXT to VkObjectType\n')
        out.append('static inline VkObjectType convertDebugReportObjectToCoreObject(VkDebugReportObjectTypeEXT debug_report_obj){\n')
        out.append('    if (debug_report_obj == VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT) {\n')
        out.append('        return VK_OBJECT_TYPE_UNKNOWN;\n')
        for field in self.vk.enums['VkObjectType'].fields:
            enum_field_name = f'VK_DEBUG_REPORT_{field.name[3:]}_EXT'
            for debug_report_field in self.vk.enums['VkDebugReportObjectTypeEXT'].fields:
                if enum_field_name == debug_report_field.name:
                    out.append(f'    }} else if (debug_report_obj == VK_DEBUG_REPORT_{field.name[3:]}_EXT) {{\n')
                    out.append(f'        return {field.name};\n')
                    break
        out.append('    }\n')
        out.append('    return VK_OBJECT_TYPE_UNKNOWN;\n')
        out.append('}\n')
        out.append('\n')

        out.append('// Helper function to convert from VkDebugReportObjectTypeEXT to VkObjectType\n')
        out.append('static inline VkDebugReportObjectTypeEXT convertCoreObjectToDebugReportObject(VkObjectType core_report_obj){\n')
        out.append('    if (core_report_obj == VK_OBJECT_TYPE_UNKNOWN) {\n')
        out.append('        return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;\n')
        for field in self.vk.enums['VkObjectType'].fields:
            enum_field_name = f'VK_DEBUG_REPORT_{field.name[3:]}_EXT'
            for debug_report_field in self.vk.enums['VkDebugReportObjectTypeEXT'].fields:
                if enum_field_name == debug_report_field.name:
                    out.append(f'    }} else if (core_report_obj == {field.name}) {{\n')
                    out.append(f'        return VK_DEBUG_REPORT_{field.name[3:]}_EXT;\n')
                    break
        out.append('    }\n')
        out.append('    return VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;\n')
        out.append('}\n')

        out.append('// clang-format on')
        self.write("".join(out))
