#!/usr/bin/python3 -i
#
# Copyright (c) 2024-2025 The Khronos Group Inc.
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

class TestIcdGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

        self.manual_functions = [
            'vkCreateInstance',
            'vkDestroyInstance',
            'vkAllocateCommandBuffers',
            'vkFreeCommandBuffers',
            'vkCreateCommandPool',
            'vkDestroyCommandPool',
            'vkEnumeratePhysicalDevices',
            'vkCreateDevice',
            'vkDestroyDevice',
            'vkGetDeviceQueue',
            'vkGetDeviceQueue2',
            'vkEnumerateInstanceLayerProperties',
            'vkEnumerateInstanceVersion',
            'vkEnumerateDeviceLayerProperties',
            'vkEnumerateInstanceExtensionProperties',
            'vkEnumerateDeviceExtensionProperties',
            'vkGetPhysicalDeviceSurfacePresentModesKHR',
            'vkGetPhysicalDeviceSurfaceFormatsKHR',
            'vkGetPhysicalDeviceSurfaceFormats2KHR',
            'vkGetPhysicalDeviceSurfaceSupportKHR',
            'vkGetPhysicalDeviceSurfaceCapabilitiesKHR',
            'vkGetPhysicalDeviceSurfaceCapabilities2KHR',
            'vkGetPhysicalDeviceSurfaceCapabilities2EXT',
            'vkGetInstanceProcAddr',
            'vkGetDeviceProcAddr',
            'vkGetPhysicalDeviceMemoryProperties',
            'vkGetPhysicalDeviceMemoryProperties2',
            'vkGetPhysicalDeviceQueueFamilyProperties',
            'vkGetPhysicalDeviceQueueFamilyProperties2',
            'vkGetPhysicalDeviceFeatures',
            'vkGetPhysicalDeviceFeatures2',
            'vkGetPhysicalDeviceFormatProperties',
            'vkGetPhysicalDeviceFormatProperties2',
            'vkGetPhysicalDeviceImageFormatProperties',
            'vkGetPhysicalDeviceImageFormatProperties2',
            'vkGetPhysicalDeviceSparseImageFormatProperties',
            'vkGetPhysicalDeviceSparseImageFormatProperties2',
            'vkGetPhysicalDeviceProperties',
            'vkGetPhysicalDeviceProperties2',
            'vkGetPhysicalDeviceExternalSemaphoreProperties',
            'vkGetPhysicalDeviceExternalFenceProperties',
            'vkGetPhysicalDeviceExternalBufferProperties',
            'vkGetBufferMemoryRequirements',
            'vkGetBufferMemoryRequirements2',
            'vkGetDeviceBufferMemoryRequirements',
            'vkGetImageMemoryRequirements',
            'vkGetImageMemoryRequirements2',
            'vkGetDeviceImageMemoryRequirements',
            'vkAllocateMemory',
            'vkFreeMemory',
            'vkMapMemory',
            'vkMapMemory2',
            'vkUnmapMemory',
            'vkUnmapMemory2',
            'vkGetImageSubresourceLayout',
            'vkCreateSwapchainKHR',
            'vkDestroySwapchainKHR',
            'vkGetSwapchainImagesKHR',
            'vkAcquireNextImageKHR',
            'vkAcquireNextImage2KHR',
            'vkCreateBuffer',
            'vkDestroyBuffer',
            'vkCreateImage',
            'vkDestroyImage',
            'vkEnumeratePhysicalDeviceGroups',
            'vkGetPhysicalDeviceMultisamplePropertiesEXT',
            'vkGetPhysicalDeviceFragmentShadingRatesKHR',
            'vkGetPhysicalDeviceCalibrateableTimeDomainsKHR',
            'vkGetFenceWin32HandleKHR',
            'vkGetFenceFdKHR',
            'vkEnumeratePhysicalDeviceQueueFamilyPerformanceQueryCountersKHR',
            'vkGetPhysicalDeviceQueueFamilyPerformanceQueryPassesKHR',
            'vkGetShaderModuleIdentifierEXT',
            'vkGetImageSparseMemoryRequirements',
            'vkGetImageSparseMemoryRequirements2',
            'vkGetBufferDeviceAddress',
            'vkGetDescriptorSetLayoutSizeEXT',
            'vkGetAccelerationStructureBuildSizesKHR',
            'vkGetAccelerationStructureMemoryRequirementsNV',
            'vkGetAccelerationStructureDeviceAddressKHR',
            'vkGetVideoSessionMemoryRequirementsKHR',
            'vkGetPhysicalDeviceCooperativeMatrixPropertiesKHR',
            'vkGetPhysicalDeviceVideoCapabilitiesKHR',
            'vkGetPhysicalDeviceVideoFormatPropertiesKHR',
            'vkGetDescriptorSetLayoutSupport',
            'vkGetRenderAreaGranularity',
            'vkGetMemoryFdKHR',
            'vkGetMemoryHostPointerPropertiesEXT',
            'vkGetAndroidHardwareBufferPropertiesANDROID',
            'vkGetPhysicalDeviceDisplayPropertiesKHR',
            'vkRegisterDisplayEventEXT',
            'vkQueueSubmit',
            'vkGetMemoryWin32HandlePropertiesKHR',
            'vkRegisterDisplayEventEXT',
            'vkCreatePipelineBinariesKHR',
            'vkGetPipelineBinaryDataKHR',
        ]

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2024-2025 LunarG, Inc.
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

        if self.filename == 'test_icd_helper.h':
            self.generateHeader()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once

            #include <stdint.h>
            #include <cstring>
            #include <string>
            #include <unordered_map>
            #include <vulkan/vulkan.h>

            namespace icd {

            ''')
        guard_helper = PlatformGuardHelper()

        # Needed because things like VK_KHR_get_physical_device_properties2 profile layer checks in driver
        out.append('''
            // Map of instance extension name to version
            static const std::unordered_map<std::string, uint32_t> instance_extension_map = {
            ''')
        for extension in [x for x in self.vk.extensions.values() if x.instance]:
            out.extend(guard_helper.add_guard(extension.protect))
            out.append(f'{{{extension.nameString}, {extension.specVersion}}},\n')
        out.extend(guard_helper.add_guard(None))
        out.append('};\n')

        out.append('''
            // Map of device extension name to version
            static const std::unordered_map<std::string, uint32_t> device_extension_map = {
            ''')
        for extension in [x for x in self.vk.extensions.values() if not x.instance]:
            out.extend(guard_helper.add_guard(extension.protect))
            out.append(f'{{{extension.nameString}, {extension.specVersion}}},\n')
        out.extend(guard_helper.add_guard(None))
        out.append('};\n')

        for command in self.vk.commands.values():
            prototype = "static " + command.cPrototype
            prototype = prototype.replace("VKAPI_CALL vk", "VKAPI_CALL ")
            out.extend(guard_helper.add_guard(command.protect))
            out.append(f'{prototype}\n')
        out.extend(guard_helper.add_guard(None))

        out.append('''
            // Map of all APIs to be intercepted by this layer
            static const std::unordered_map<std::string, void*> name_to_func_ptr_map = {
            ''')

        for command in self.vk.commands.values():
            out.extend(guard_helper.add_guard(command.protect))
            out.append(f'{{"{command.name}", (void*){command.name[2:]}}},\n')
        out.extend(guard_helper.add_guard(None))
        out.append('};\n')

        for command in [x for x in self.vk.commands.values() if x.name not in self.manual_functions]:
            out.extend(guard_helper.add_guard(command.protect))
            prototype = "static " + command.cPrototype
            prototype = prototype.replace("VKAPI_CALL vk", "VKAPI_CALL ")
            prototype = prototype.replace(");", ") {")
            out.append(f'{prototype}\n')

            voidReturn = command.returnType == 'void'
            # For alias that are promoted, just point to new function
            if command.alias and command.alias in self.vk.commands:
                paramList = [param.name for param in command.params]
                params = ', '.join(paramList)
                returnName = '' if voidReturn else 'return '
                out.append(f'{returnName}{command.alias[2:]}({params});')
            elif 'vkCreate' in command.name or 'vkAllocate' in command.name:
                last_param = command.params[-1]
                out.append('unique_lock_t lock(global_lock);\n')
                if (last_param.length):
                    out.append(f'for (uint32_t i = 0; i < {last_param.length}; ++i) {{\n')
                    out.append(f'{last_param.name}[i] = ({last_param.type})global_unique_handle++;\n')
                    out.append('}\n')
                else:
                    out.append(f'*{last_param.name} = ({last_param.type})global_unique_handle++;\n')
                out.append('return VK_SUCCESS;\n')
            elif not voidReturn:
                out.append('return VK_SUCCESS;')

            out.append('}\n\n')
        out.extend(guard_helper.add_guard(None))

        out.append('\n} // namespace icd')
        self.write("".join(out))
