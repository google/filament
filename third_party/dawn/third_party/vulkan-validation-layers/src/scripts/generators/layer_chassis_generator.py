#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2025 Valve Corporation
# Copyright (c) 2015-2025 LunarG, Inc.
# Copyright (c) 2015-2025 Google Inc.
# Copyright (c) 2023-2024 RasterGrid Kft.
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
# This script generates the dispatch portion of a factory layer which intercepts
# all Vulkan  functions. The resultant factory layer allows rapid development of
# layers and interceptors.

import os
from generators.vulkan_object import Command
from generators.base_generator import BaseGenerator
from generators.generator_utils import PlatformGuardHelper

# This class is a container for any source code, data, or other behavior that is necessary to
# customize the generator script for a specific target API variant (e.g. Vulkan SC). As such,
# all of these API-specific interfaces and their use in the generator script are part of the
# contract between this repository and its downstream users. Changing or removing any of these
# interfaces or their use in the generator script will have downstream effects and thus
# should be avoided unless absolutely necessary.
class APISpecific:

    # Returns the list of instance extensions exposed by the validation layers
    @staticmethod
    def getInstanceExtensionList(targetApiName: str) -> list[str]:
        match targetApiName:

            # Vulkan specific instance extension list
            case 'vulkan':
                return [
                    'VK_EXT_debug_report',
                    'VK_EXT_debug_utils',
                    'VK_EXT_validation_features',
                    'VK_EXT_layer_settings'
                ]


    # Returns the list of device extensions exposed by the validation layers
    @staticmethod
    def getDeviceExtensionList(targetApiName: str) -> list[str]:
        match targetApiName:

            # Vulkan specific device extension list
            case 'vulkan':
                return [
                'VK_EXT_validation_cache',
                'VK_EXT_debug_marker',
                'VK_EXT_tooling_info'
            ]


class LayerChassisOutputGenerator(BaseGenerator):
    ignore_functions = (
        'vkEnumerateInstanceVersion',
    )

    manual_functions = (
        # Include functions here to be interecpted w/ manually implemented function bodies
        'vkGetDeviceProcAddr',
        'vkGetInstanceProcAddr',
        'vkCreateDevice',
        'vkDestroyDevice',
        'vkCreateInstance',
        'vkDestroyInstance',
        'vkEnumerateInstanceLayerProperties',
        'vkEnumerateInstanceExtensionProperties',
        'vkEnumerateDeviceLayerProperties',
        'vkEnumerateDeviceExtensionProperties',
        # Functions that are handled explicitly due to chassis architecture violations
        # Note: If added, may need to add to skip_intercept_id_functions list as well
        'vkCreateGraphicsPipelines',
        'vkCreateComputePipelines',
        'vkCreateRayTracingPipelinesNV',
        'vkCreateRayTracingPipelinesKHR',
        'vkCreatePipelineLayout',
        'vkCreateShaderModule',
        'vkCreateShadersEXT',
        'vkAllocateDescriptorSets',
        'vkCreateBuffer',
        'vkQueuePresentKHR',
        # Need to inject HandleData logic
        'vkBeginCommandBuffer',
        # ValidationCache functions do not get dispatched
        'vkCreateValidationCacheEXT',
        'vkDestroyValidationCacheEXT',
        'vkMergeValidationCachesEXT',
        'vkGetValidationCacheDataEXT',
        'vkGetPhysicalDeviceToolProperties',
        'vkGetPhysicalDeviceToolPropertiesEXT',
    )

    def __init__(self):
        BaseGenerator.__init__(self)

    def getApiFunctionType(self, command: Command) -> str:
            if command.name in [
                    'vkCreateInstance',
                    'vkEnumerateInstanceVersion',
                    'vkEnumerateInstanceLayerProperties',
                    'vkEnumerateInstanceExtensionProperties',
                ]:
                return 'kFuncTypeInst'
            elif command.params[0].type == 'VkInstance':
                return'kFuncTypeInst'
            elif command.params[0].type == 'VkPhysicalDevice':
                return'kFuncTypePdev'
            else:
                return'kFuncTypeDev'

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2015-2025 The Khronos Group Inc.
            * Copyright (c) 2015-2025 Valve Corporation
            * Copyright (c) 2015-2025 LunarG, Inc.
            * Copyright (c) 2015-2024 Google Inc.
            * Copyright (c) 2023-2024 RasterGrid Kft.
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

        if self.filename == 'validation_object_instance_methods.h':
            self.generateInstanceMethods()
        elif self.filename == 'validation_object_device_methods.h':
            self.generateDeviceMethods()
        elif self.filename == 'validation_object.cpp':
            self.generateVOSource()
        elif self.filename == 'chassis.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateMethods(self, want_instance):
        out = []

        guard_helper = PlatformGuardHelper()
        for command in [x for x in self.vk.commands.values() if x.name not in self.ignore_functions and 'ValidationCache' not in x.name]:
            if command.instance != want_instance:
                continue
            parameters = (command.cPrototype.split('(')[1])[:-2] # leaves just the parameters
            parameters = parameters.replace('\n', '')
            parameters = ' '.join(parameters.split()) # remove duplicate whitespace

            out.extend(guard_helper.add_guard(command.protect))
            out.append(f'        virtual bool PreCallValidate{command.name[2:]}({parameters}, const ErrorObject& error_obj) const {{ return false; }}\n')
            out.append(f'        virtual void PreCallRecord{command.name[2:]}({parameters}, const RecordObject& record_obj) {{}}\n')
            out.append(f'        virtual void PostCallRecord{command.name[2:]}({parameters}, const RecordObject& record_obj) {{}}\n')
        out.extend(guard_helper.add_guard(None))
        self.write("".join(out))

    def generateInstanceMethods(self):
        out = []
        out.append('''
            // This file contains methods for class vvl::base::Instance and it is designed to ONLY be
            // included into validation_object.h.
            ''')
        self.write("".join(out))

        self.generateMethods(True)

    def generateDeviceMethods(self):
        out = []
        out.append('''
            // This file contains methods for class vvl::base::Device and it is designed to ONLY be
            // included into validation_object.h.
            ''')
        self.write("".join(out))

        self.generateMethods(False)

    def generateVOSource(self):
        out = []
        out.append('''
            #include <array>
            #include <cstring>
            #include <mutex>

            #include "chassis/validation_object.h"

            namespace vvl::base {
            thread_local WriteLockGuard* Device::record_guard{};

            } // namespace vvl::base
        ''')
        self.write("".join(out))


    def generateSource(self):
        out = []
        out.append('''
            #include "chassis/chassis.h"
            #include <array>
            #include <cstring>
            #include <mutex>

            #include "chassis/dispatch_object.h"
            #include "chassis/validation_object.h"
            #include "layer_options.h"
            #include "state_tracker/descriptor_sets.h"
            #include "chassis/chassis_modification_state.h"
            #include "core_checks/core_validation.h"

            #include "profiling/profiling.h"

            ''')

        out.append('// Extension exposed by the validation layer\n')

        instance_exts = APISpecific.getInstanceExtensionList(self.targetApiName)
        out.append(f'static constexpr std::array<VkExtensionProperties, {len(instance_exts)}> kInstanceExtensions = {{\n')
        for ext in [x.upper() for x in instance_exts]:
            out.append(f'    VkExtensionProperties{{{ext}_EXTENSION_NAME, {ext}_SPEC_VERSION}},\n')
        out.append('};\n')

        device_exts = APISpecific.getDeviceExtensionList(self.targetApiName)
        out.append(f'static constexpr std::array<VkExtensionProperties, {len(device_exts)}> kDeviceExtensions = {{\n')
        for ext in [x.upper() for x in device_exts]:
            out.append(f'    VkExtensionProperties{{{ext}_EXTENSION_NAME, {ext}_SPEC_VERSION}},\n')
        out.append('};\n')

        out.append('namespace vulkan_layer_chassis {')
        guard_helper = PlatformGuardHelper()
        out.append('''
static const VkLayerProperties global_layer = {
    OBJECT_LAYER_NAME,
    VK_LAYER_API_VERSION,
    1,
    "LunarG validation Layer",
};

// These functions reference generated data so they cannot be part of chassis_main.cpp
VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceLayerProperties(uint32_t* pCount, VkLayerProperties* pProperties) {
    return util_GetLayerProperties(1, &global_layer, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t* pCount,
                                                              VkLayerProperties* pProperties) {
    return util_GetLayerProperties(1, &global_layer, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateInstanceExtensionProperties(const char* pLayerName, uint32_t* pCount,
                                                                    VkExtensionProperties* pProperties) {
    if (pLayerName && !strcmp(pLayerName, global_layer.layerName)) {
        return util_GetExtensionProperties(static_cast<uint32_t>(kInstanceExtensions.size()), kInstanceExtensions.data(), pCount,
                                           pProperties);
    }

    return VK_ERROR_LAYER_NOT_PRESENT;
}

VKAPI_ATTR VkResult VKAPI_CALL EnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char* pLayerName,
                                                                  uint32_t* pCount, VkExtensionProperties* pProperties) {
    if (pLayerName && !strcmp(pLayerName, global_layer.layerName)) {
        return util_GetExtensionProperties(static_cast<uint32_t>(kDeviceExtensions.size()), kDeviceExtensions.data(), pCount,
                                           pProperties);
    }

    assert(physicalDevice);
    auto layer_data = vvl::dispatch::GetData(physicalDevice);
    return layer_data->instance_dispatch_table.EnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pCount, pProperties);
}
            ''')

        for command in [x for x in self.vk.commands.values() if x.name not in self.ignore_functions and x.name not in self.manual_functions]:
            out.extend(guard_helper.add_guard(command.protect))
            prototype = command.cPrototype.replace('VKAPI_CALL vk', 'VKAPI_CALL ').replace(');', ') {\n')
            out.append(prototype)

            paramsList = ', '.join([param.name for param in command.params])

            dispatch = 'device_dispatch' if not command.instance else 'instance_dispatch'
            # Setup common to call wrappers. First parameter is always dispatchable
            out.append('VVL_ZoneScoped;\n\n')
            out.append(f'auto {dispatch} = vvl::dispatch::GetData({command.params[0].name});\n')

            # Declare result variable, if any.
            return_map = {
                'PFN_vkVoidFunction': 'return nullptr;',
                'VkBool32': 'return VK_FALSE;',
                'VkDeviceAddress': 'return 0;',
                'VkDeviceSize': 'return 0;',
                'VkResult': 'return VK_ERROR_VALIDATION_FAILED_EXT;',
                'void': 'return;',
                'uint32_t': 'return 0;',
                'uint64_t': 'return 0;'
            }

            # Set up skip and locking
            out.append('bool skip = false;\n')

            out.append(f'ErrorObject error_obj(vvl::Func::{command.name}, VulkanTypedHandle({command.params[0].name}, kVulkanObjectType{command.params[0].type[2:]}));\n')

            # Generate pre-call validation source code
            out.append('''{
                VVL_ZoneScopedN("PreCallValidate");
            ''')
            if not command.instance:
                out.append(f'for (const auto& vo : {dispatch}->intercept_vectors[InterceptIdPreCallValidate{command.name[2:]}]) {{\n')
                out.append('    auto lock = vo->ReadLock();\n')
            else:
                out.append(f'for (const auto& vo : {dispatch}->object_dispatch) {{\n')
            out.append(f'    skip |= vo->PreCallValidate{command.name[2:]}({paramsList}, error_obj);\n')
            out.append(f'    if (skip) {return_map[command.returnType]}\n')
            out.append('}\n')
            out.append('}\n')

            # Generate pre-call state recording source code
            out.append(f'RecordObject record_obj(vvl::Func::{command.name});\n')
            out.append('''{
                VVL_ZoneScopedN("PreCallRecord");
            ''')
            if not command.instance:
                out.append(f'for (auto& vo : {dispatch}->intercept_vectors[InterceptIdPreCallRecord{command.name[2:]}]) {{\n')
                out.append('    auto lock = vo->WriteLock();\n')
            else:
                out.append(f'for (auto& vo : {dispatch}->object_dispatch) {{\n')
            out.append(f'vo->PreCallRecord{command.name[2:]}({paramsList}, record_obj);\n')
            out.append('    }\n')
            out.append('}\n')

            # Insert pre-dispatch debug utils function call
            pre_dispatch_debug_utils_functions = {
                'vkDebugMarkerSetObjectNameEXT' : f'{dispatch}->debug_report->SetMarkerObjectName(pNameInfo);',
                'vkSetDebugUtilsObjectNameEXT' : f'{dispatch}->debug_report->SetUtilsObjectName(pNameInfo);',
                'vkQueueBeginDebugUtilsLabelEXT' : f'{dispatch}->debug_report->BeginQueueDebugUtilsLabel(queue, pLabelInfo);',
                'vkQueueInsertDebugUtilsLabelEXT' : f'{dispatch}->debug_report->InsertQueueDebugUtilsLabel(queue, pLabelInfo);',
            }
            if command.name in pre_dispatch_debug_utils_functions:
                out.append(f'    {pre_dispatch_debug_utils_functions[command.name]}\n')

            # Output dispatch (down-chain) function call
            if (command.returnType != 'void'):
                out.append(f'{command.returnType} result;')

            # Tracy profiler
            out.append('''{
                VVL_ZoneScopedN("Dispatch");
            ''')
            gpu_begin_render_commands = ["BeginRender"]
            if any(s in command.name for s in gpu_begin_render_commands):
                out.append(f'VVL_TracyVkNamedZoneStart(GetTracyVkCtx(), commandBuffer, "gpu_{command.name[10:]}");\n')

            assignResult = f'result = ' if (command.returnType != 'void') else ''
            method_name = command.name.replace('vk', f'{dispatch}->')
            out.append(f'        {assignResult}{method_name}({paramsList});\n')

            # Tracy profiler
            gpu_end_render_commands = ["EndRender"]
            if any(s in command.name for s in gpu_end_render_commands):
                out.append(f'VVL_TracyVkNamedZoneEnd(commandBuffer);\n')


            # Tracy submit GPU queries reset command buffer
            if "QueueSubmit" in command.name:
                out.append('''#if defined(VVL_TRACY_GPU)
                    TracyVkCollector::TrySubmitCollectCb(queue);
                #endif
                ''')
            out.append('}\n')

            # Insert post-dispatch debug utils function call
            post_dispatch_debug_utils_functions = {
                'vkQueueEndDebugUtilsLabelEXT' : f'{dispatch}->debug_report->EndQueueDebugUtilsLabel(queue);',
                'vkCreateDebugReportCallbackEXT' : f'LayerCreateReportCallback({dispatch}->debug_report, false, pCreateInfo, pCallback);',
                'vkDestroyDebugReportCallbackEXT' : f'LayerDestroyCallback({dispatch}->debug_report, callback);',
                'vkCreateDebugUtilsMessengerEXT' : f'LayerCreateMessengerCallback({dispatch}->debug_report, false, pCreateInfo, pMessenger);',
                'vkDestroyDebugUtilsMessengerEXT' : f'LayerDestroyCallback({dispatch}->debug_report, messenger);',
            }
            if command.name in post_dispatch_debug_utils_functions:
                out.append(f'    {post_dispatch_debug_utils_functions[command.name]}\n')

            if command.returnType == 'VkResult':
                out.append('record_obj.result = result;\n')
            elif command.returnType == 'VkDeviceAddress':
                out.append('record_obj.device_address = result;\n')

            # Generate post-call object processing source code
            out.append('''{
                VVL_ZoneScopedN("PostCallRecord");
            ''')

            if not command.instance:
                out.append(f'for (auto& vo : {dispatch}->intercept_vectors[InterceptIdPostCallRecord{command.name[2:]}]) {{\n')
            else:
                out.append(f'for (auto& vo : {dispatch}->object_dispatch) {{\n')

            # These commands perform blocking operations during PostRecord phase. We might need to
            # release base::Device's lock for the period of blocking operation to avoid deadlocks.
            # The released mutex can be re-acquired by the command that sets wait finish condition.
            # This functionality is needed when fine grained locking is disabled or not implemented.
            commands_with_blocking_operations = [
                'vkWaitSemaphores',
                'vkWaitSemaphoresKHR',

                # Note that get semaphore counter API commands do not block, but here we consider only
                # PostRecord phase which might block
                'vkGetSemaphoreCounterValue',
                'vkGetSemaphoreCounterValueKHR',
            ]
            if not command.instance:
                if command.name not in commands_with_blocking_operations:
                    out.append('auto lock = vo->WriteLock();\n')
                else:
                    out.append('vvl::base::Device::BlockingOperationGuard lock(vo);\n')

            # Because each intercept is a copy of vvl::base::Device, we need to update it for each
            if not command.instance and command.errorCodes and 'VK_ERROR_DEVICE_LOST' in command.errorCodes:
                out.append('''
                    if (result == VK_ERROR_DEVICE_LOST) {
                        vo->is_device_lost = true;
                    }
                ''')

            out.append(f'vo->PostCallRecord{command.name[2:]}({paramsList}, record_obj);\n')
            out.append('    }\n')
            out.append('}\n')

            # Return result variable, if any.
            if command.returnType != 'void':
                out.append('    return result;\n')

            # Tracy create GPU queries collectors
            if command.name == "vkGetDeviceQueue":
                out.append('''#if defined(VVL_TRACY_GPU)
                    TracyVkCollector::Create(device, *pQueue, queueFamilyIndex);
                #endif
                ''')

            if command.name == "vkGetDeviceQueue2":
                out.append('''#if defined(VVL_TRACY_GPU)
                    TracyVkCollector::Create(device, *pQueue, pQueueInfo->queueFamilyIndex);
                #endif
                ''')
            out.append('}\n')
            out.append('\n')

        out.extend(guard_helper.add_guard(None))

        out.append('''
// Map of intercepted ApiName to its associated function data
#ifdef _MSC_VER
#pragma warning( suppress: 6262 ) // VS analysis: this uses more than 16 kiB, which is fine here at global scope
#endif

const vvl::unordered_map<std::string, function_data> &GetNameToFuncPtrMap() {
    static const vvl::unordered_map<std::string, function_data> name_to_func_ptr_map = {
    {"vk_layerGetPhysicalDeviceProcAddr", {kFuncTypeInst, (void*)GetPhysicalDeviceProcAddr}},
''')
        for command in [x for x in self.vk.commands.values() if x.name not in self.ignore_functions]:
            out.extend(guard_helper.add_guard(command.protect))
            out.append(f'    {{"{command.name}", {{{self.getApiFunctionType(command)}, (void*){command.name[2:]}}}}},\n')
        out.extend(guard_helper.add_guard(None))
        out.append('};\n')
        out.append(' return name_to_func_ptr_map;\n')
        out.append('}\n')
        out.append('} // namespace vulkan_layer_chassis\n')

        out.append('''
            VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_layerGetPhysicalDeviceProcAddr(VkInstance instance, const char *funcName) {
                return vulkan_layer_chassis::GetPhysicalDeviceProcAddr(instance, funcName);
            }

            #if defined(__GNUC__) && __GNUC__ >= 4
            #define VVL_EXPORT __attribute__((visibility("default")))
            #else
            #define VVL_EXPORT
            #endif

            // The following functions need to match the `/DEF` and `--version-script` files
            // for consistency across platforms that don't accept those linker options.
            extern "C" {

            VVL_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char *funcName) {
                return vulkan_layer_chassis::GetInstanceProcAddr(instance, funcName);
            }

            VVL_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice dev, const char *funcName) {
                return vulkan_layer_chassis::GetDeviceProcAddr(dev, funcName);
            }

            VVL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t *pCount, VkLayerProperties *pProperties) {
                return vulkan_layer_chassis::EnumerateInstanceLayerProperties(pCount, pProperties);
            }

            VVL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pCount, VkExtensionProperties *pProperties) {
                return vulkan_layer_chassis::EnumerateInstanceExtensionProperties(pLayerName, pCount, pProperties);
            }

            VVL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface *pVersionStruct) {
                assert(pVersionStruct != nullptr);
                assert(pVersionStruct->sType == LAYER_NEGOTIATE_INTERFACE_STRUCT);

                // Fill in the function pointers if our version is at least capable of having the structure contain them.
                if (pVersionStruct->loaderLayerInterfaceVersion >= 2) {
                    pVersionStruct->pfnGetInstanceProcAddr = vulkan_layer_chassis::GetInstanceProcAddr;
                    pVersionStruct->pfnGetDeviceProcAddr = vulkan_layer_chassis::GetDeviceProcAddr;
                    pVersionStruct->pfnGetPhysicalDeviceProcAddr = vulkan_layer_chassis::GetPhysicalDeviceProcAddr;
                }

                return VK_SUCCESS;
            }

            #if defined(VK_USE_PLATFORM_ANDROID_KHR)
            VVL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pCount, VkLayerProperties *pProperties) {
                // the layer command handles VK_NULL_HANDLE just fine internally
                assert(physicalDevice == VK_NULL_HANDLE);
                return vulkan_layer_chassis::EnumerateDeviceLayerProperties(VK_NULL_HANDLE, pCount, pProperties);
            }

            VVL_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char *pLayerName, uint32_t *pCount, VkExtensionProperties *pProperties) {
                // the layer command handles VK_NULL_HANDLE just fine internally
                assert(physicalDevice == VK_NULL_HANDLE);
                return vulkan_layer_chassis::EnumerateDeviceExtensionProperties(VK_NULL_HANDLE, pLayerName, pCount, pProperties);
            }
            #endif

            }  // extern "C"
            ''')
        self.write("".join(out))

