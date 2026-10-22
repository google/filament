/*
 * Copyright (c) 2015-2021 The Khronos Group Inc.
 * Copyright (c) 2015-2021 Valve Corporation
 * Copyright (c) 2015-2021 LunarG, Inc.
 * Copyright (C) 2015-2016 Google Inc.
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
 * Author: Mark Young <markyk@lunarg.com>
 * Author: Charles Giessen <charles@lunarg.com>
 */

#pragma once

#include "loader_common.h"

// General utilities

VkResult add_debug_extensions_to_ext_list(const struct loader_instance *inst, struct loader_extension_list *ext_list);
void check_for_enabled_debug_extensions(struct loader_instance *ptr_instance, const VkInstanceCreateInfo *pCreateInfo);
bool debug_extensions_InstanceGpa(struct loader_instance *ptr_instance, const char *name, void **addr);
bool debug_utils_ReportFlagsToAnnotFlags(VkDebugReportFlagsEXT dr_flags, bool default_flag_is_spec,
                                         VkDebugUtilsMessageSeverityFlagBitsEXT *da_severity,
                                         VkDebugUtilsMessageTypeFlagsEXT *da_type);
bool debug_utils_AnnotFlagsToReportFlags(VkDebugUtilsMessageSeverityFlagBitsEXT da_severity,
                                         VkDebugUtilsMessageTypeFlagsEXT da_type, VkDebugReportFlagsEXT *dr_flags);
bool debug_utils_ReportObjectToAnnotObject(VkDebugReportObjectTypeEXT dr_object_type, uint64_t object_handle,
                                           VkDebugUtilsObjectNameInfoEXT *da_object_name_info);
bool debug_utils_AnnotObjectToDebugReportObject(const VkDebugUtilsObjectNameInfoEXT *da_object_name_info,
                                                VkDebugReportObjectTypeEXT *dr_object_type, uint64_t *dr_object_handle);

void destroy_debug_callbacks_chain(struct loader_instance *inst, const VkAllocationCallbacks *pAllocator);

// VK_EXT_debug_utils related items

VKAPI_ATTR VkResult VKAPI_CALL terminator_CreateDebugUtilsMessengerEXT(VkInstance instance,
                                                                       const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                                       const VkAllocationCallbacks *pAllocator,
                                                                       VkDebugUtilsMessengerEXT *pMessenger);
VKAPI_ATTR void VKAPI_CALL terminator_DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                                    const VkAllocationCallbacks *pAllocator);
VKAPI_ATTR void VKAPI_CALL terminator_SubmitDebugUtilsMessageEXT(VkInstance instance,
                                                                 VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                 VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                                 const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData);
VkResult util_CreateDebugUtilsMessengers(struct loader_instance *inst, const void *pChain, const VkAllocationCallbacks *pAllocator);
VkBool32 util_SubmitDebugUtilsMessageEXT(const struct loader_instance *inst, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                         VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                         const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData);

// VK_EXT_debug_report related items

VKAPI_ATTR VkResult VKAPI_CALL terminator_CreateDebugReportCallbackEXT(VkInstance instance,
                                                                       const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                                                       const VkAllocationCallbacks *pAllocator,
                                                                       VkDebugReportCallbackEXT *pCallback);

VKAPI_ATTR void VKAPI_CALL terminator_DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                                    const VkAllocationCallbacks *pAllocator);

VKAPI_ATTR void VKAPI_CALL terminator_DebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                                            VkDebugReportObjectTypeEXT objType, uint64_t object, size_t location,
                                                            int32_t msgCode, const char *pLayerPrefix, const char *pMsg);

VkResult util_CreateDebugReportCallbacks(struct loader_instance *inst, const void *pChain, const VkAllocationCallbacks *pAllocator);
VkBool32 util_DebugReportMessage(const struct loader_instance *inst, VkFlags msgFlags, VkDebugReportObjectTypeEXT objectType,
                                 uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg);
