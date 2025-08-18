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
 * Author: Jon Ashburn <jon@LunarG.com>
 * Author: Mark Young <marky@lunarg.com>
 * Author: Charles Giessen <charles@lunarg.com>
 *
 */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vk_object_types.h"

#include "allocation.h"
#include "debug_utils.h"
#include "log.h"
#include "loader.h"
#include "vk_loader_platform.h"

// VK_EXT_debug_report related items

VkResult util_CreateDebugUtilsMessenger(struct loader_instance *inst, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT messenger) {
    VkLayerDbgFunctionNode *new_dbg_function_node = NULL;

    new_dbg_function_node = (VkLayerDbgFunctionNode *)loader_calloc_with_instance_fallback(
        pAllocator, inst, sizeof(VkLayerDbgFunctionNode), VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

    if (!new_dbg_function_node) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    new_dbg_function_node->is_messenger = true;
    new_dbg_function_node->messenger.messenger = messenger;
    new_dbg_function_node->messenger.pfnUserCallback = pCreateInfo->pfnUserCallback;
    new_dbg_function_node->messenger.messageSeverity = pCreateInfo->messageSeverity;
    new_dbg_function_node->messenger.messageType = pCreateInfo->messageType;
    new_dbg_function_node->pUserData = pCreateInfo->pUserData;
    new_dbg_function_node->pNext = inst->instance_only_dbg_function_head;
    inst->instance_only_dbg_function_head = new_dbg_function_node;
    inst->current_dbg_function_head = inst->instance_only_dbg_function_head;

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL debug_utils_CreateDebugUtilsMessengerEXT(VkInstance instance,
                                                                        const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                                        const VkAllocationCallbacks *pAllocator,
                                                                        VkDebugUtilsMessengerEXT *pMessenger) {
    struct loader_instance *inst = loader_get_instance(instance);
    loader_platform_thread_lock_mutex(&loader_lock);
    VkResult result = inst->disp->layer_inst_disp.CreateDebugUtilsMessengerEXT(inst->instance, pCreateInfo, pAllocator, pMessenger);
    loader_platform_thread_unlock_mutex(&loader_lock);
    return result;
}

VkBool32 util_SubmitDebugUtilsMessageEXT(const struct loader_instance *inst, VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                         VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                         const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData) {
    VkBool32 bail = false;

    if (NULL != pCallbackData) {
        VkLayerDbgFunctionNode *pTrav = inst->current_dbg_function_head;
        VkDebugReportObjectTypeEXT object_type = VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT;
        VkDebugReportFlagsEXT object_flags = 0;
        uint64_t object_handle = 0;

        debug_utils_AnnotFlagsToReportFlags(messageSeverity, messageTypes, &object_flags);
        if (0 < pCallbackData->objectCount) {
            debug_utils_AnnotObjectToDebugReportObject(pCallbackData->pObjects, &object_type, &object_handle);
        }
        while (pTrav) {
            if (pTrav->is_messenger && (pTrav->messenger.messageSeverity & messageSeverity) &&
                (pTrav->messenger.messageType & messageTypes)) {
                if (pTrav->messenger.pfnUserCallback(messageSeverity, messageTypes, pCallbackData, pTrav->pUserData)) {
                    bail = true;
                }
            }
            if (!pTrav->is_messenger && pTrav->report.msgFlags & object_flags) {
                if (pTrav->report.pfnMsgCallback(object_flags, object_type, object_handle, 0, pCallbackData->messageIdNumber,
                                                 pCallbackData->pMessageIdName, pCallbackData->pMessage, pTrav->pUserData)) {
                    bail = true;
                }
            }
            pTrav = pTrav->pNext;
        }
    }

    return bail;
}

void util_DestroyDebugUtilsMessenger(struct loader_instance *inst, VkDebugUtilsMessengerEXT messenger,
                                     const VkAllocationCallbacks *pAllocator) {
    VkLayerDbgFunctionNode *pTrav = inst->current_dbg_function_head;
    VkLayerDbgFunctionNode *pPrev = pTrav;

    while (pTrav) {
        if (pTrav->is_messenger && pTrav->messenger.messenger == messenger) {
            pPrev->pNext = pTrav->pNext;
            if (inst->current_dbg_function_head == pTrav) inst->current_dbg_function_head = pTrav->pNext;
            if (inst->instance_only_dbg_function_head == pTrav) inst->instance_only_dbg_function_head = pTrav->pNext;
            loader_free_with_instance_fallback(pAllocator, inst, pTrav);
            break;
        }
        pPrev = pTrav;
        pTrav = pTrav->pNext;
    }
}

VkResult util_CreateDebugUtilsMessengers(struct loader_instance *inst, const void *pChain,
                                         const VkAllocationCallbacks *pAllocator) {
    const void *pNext = pChain;
    while (pNext) {
        VkBaseInStructure in_structure = {0};
        memcpy(&in_structure, pNext, sizeof(VkBaseInStructure));
        if (in_structure.sType == VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT) {
            // Assign a unique handle to each messenger (just use the address of the VkDebugUtilsMessengerCreateInfoEXT)
            // This is only being used this way due to it being for an 'anonymous' callback during instance creation
            VkDebugUtilsMessengerEXT messenger_handle = (VkDebugUtilsMessengerEXT)(uintptr_t)pNext;
            VkResult ret = util_CreateDebugUtilsMessenger(inst, (const VkDebugUtilsMessengerCreateInfoEXT *)pNext, pAllocator,
                                                          messenger_handle);
            if (ret != VK_SUCCESS) {
                return ret;
            }
        }
        pNext = in_structure.pNext;
    }
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL debug_utils_SubmitDebugUtilsMessageEXT(VkInstance instance,
                                                                  VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                  VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                                  const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData) {
    struct loader_instance *inst = loader_get_instance(instance);

    inst->disp->layer_inst_disp.SubmitDebugUtilsMessageEXT(inst->instance, messageSeverity, messageTypes, pCallbackData);
}

VKAPI_ATTR void VKAPI_CALL debug_utils_DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                                     const VkAllocationCallbacks *pAllocator) {
    struct loader_instance *inst = loader_get_instance(instance);
    loader_platform_thread_lock_mutex(&loader_lock);

    inst->disp->layer_inst_disp.DestroyDebugUtilsMessengerEXT(inst->instance, messenger, pAllocator);

    loader_platform_thread_unlock_mutex(&loader_lock);
}

// This is the instance chain terminator function for CreateDebugUtilsMessenger
VKAPI_ATTR VkResult VKAPI_CALL terminator_CreateDebugUtilsMessengerEXT(VkInstance instance,
                                                                       const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                                       const VkAllocationCallbacks *pAllocator,
                                                                       VkDebugUtilsMessengerEXT *pMessenger) {
    struct loader_instance *inst = (struct loader_instance *)instance;
    VkResult res = VK_SUCCESS;
    VkLayerDbgFunctionNode *new_dbg_func_node = NULL;
    uint32_t next_index = 0;

    uint32_t *pNextIndex = loader_instance_heap_alloc(inst, sizeof(uint32_t), VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    if (NULL == pNextIndex) {
        res = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    }

    res = loader_get_next_available_entry(inst, &inst->debug_utils_messengers_list, &next_index, pAllocator);
    if (res != VK_SUCCESS) {
        goto out;
    }

    for (struct loader_icd_term *icd_term = inst->icd_terms; icd_term != NULL; icd_term = icd_term->next) {
        if (icd_term->debug_utils_messenger_list.list == NULL) {
            res = loader_init_generic_list(inst, (struct loader_generic_list *)&icd_term->debug_utils_messenger_list,
                                           sizeof(VkDebugUtilsMessengerEXT));
            if (res != VK_SUCCESS) {
                goto out;
            }
        } else if (icd_term->debug_utils_messenger_list.capacity <= next_index * sizeof(VkDebugUtilsMessengerEXT)) {
            res = loader_resize_generic_list(inst, (struct loader_generic_list *)&icd_term->debug_utils_messenger_list);
            if (res != VK_SUCCESS) {
                goto out;
            }
        }

        if (icd_term->dispatch.CreateDebugUtilsMessengerEXT) {
            res = icd_term->dispatch.CreateDebugUtilsMessengerEXT(icd_term->instance, pCreateInfo, pAllocator,
                                                                  &icd_term->debug_utils_messenger_list.list[next_index]);

            if (res != VK_SUCCESS) {
                goto out;
            }
        }
    }

    // Setup the debug report callback in the terminator since a layer may want
    // to grab the information itself (RenderDoc) and then return back to the
    // user callback a sub-set of the messages.
    new_dbg_func_node = (VkLayerDbgFunctionNode *)loader_calloc_with_instance_fallback(
        pAllocator, inst, sizeof(VkLayerDbgFunctionNode), VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    if (!new_dbg_func_node) {
        res = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    }

    new_dbg_func_node->is_messenger = true;
    new_dbg_func_node->messenger.pfnUserCallback = pCreateInfo->pfnUserCallback;
    new_dbg_func_node->messenger.messageSeverity = pCreateInfo->messageSeverity;
    new_dbg_func_node->messenger.messageType = pCreateInfo->messageType;
    new_dbg_func_node->pUserData = pCreateInfo->pUserData;
    new_dbg_func_node->pNext = inst->current_dbg_function_head;
    inst->current_dbg_function_head = new_dbg_func_node;
    *pNextIndex = next_index;
    *pMessenger = (VkDebugUtilsMessengerEXT)(uintptr_t)pNextIndex;
    new_dbg_func_node->messenger.messenger = *pMessenger;

out:

    // Roll back on errors
    if (VK_SUCCESS != res) {
        if (pNextIndex) {
            for (struct loader_icd_term *icd_term = inst->icd_terms; icd_term; icd_term = icd_term->next) {
                if (icd_term->debug_utils_messenger_list.list && icd_term->debug_utils_messenger_list.list[next_index] &&
                    NULL != icd_term->dispatch.DestroyDebugUtilsMessengerEXT) {
                    icd_term->dispatch.DestroyDebugUtilsMessengerEXT(
                        icd_term->instance, icd_term->debug_utils_messenger_list.list[next_index], pAllocator);
                }
            }
        }
        if (inst->debug_utils_messengers_list.list &&
            inst->debug_utils_messengers_list.capacity > (*pNextIndex) * sizeof(struct loader_used_object_status)) {
            inst->debug_utils_messengers_list.list[*pNextIndex].status = VK_FALSE;
            if (NULL != pAllocator) {
                inst->debug_utils_messengers_list.list[*pNextIndex].allocation_callbacks = *pAllocator;
            }
        }
        loader_free_with_instance_fallback(pAllocator, inst, new_dbg_func_node);
        loader_free_with_instance_fallback(pAllocator, inst, pNextIndex);
    }

    return res;
}

// This is the instance chain terminator function for DestroyDebugUtilsMessenger
VKAPI_ATTR void VKAPI_CALL terminator_DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                                    const VkAllocationCallbacks *pAllocator) {
    struct loader_instance *inst = (struct loader_instance *)instance;
    uint32_t *debug_messenger_index = (uint32_t *)(uintptr_t)messenger;
    // Make sure that messenger actually points to anything
    if (NULL == debug_messenger_index) {
        return;
    }

    for (struct loader_icd_term *icd_term = inst->icd_terms; icd_term; icd_term = icd_term->next) {
        if (icd_term->debug_utils_messenger_list.list && icd_term->debug_utils_messenger_list.list[*debug_messenger_index] &&
            NULL != icd_term->dispatch.DestroyDebugUtilsMessengerEXT) {
            icd_term->dispatch.DestroyDebugUtilsMessengerEXT(
                icd_term->instance, icd_term->debug_utils_messenger_list.list[*debug_messenger_index], pAllocator);
        }
    }

    util_DestroyDebugUtilsMessenger(inst, messenger, pAllocator);
    if (inst->debug_utils_messengers_list.list &&
        inst->debug_utils_messengers_list.capacity > (*debug_messenger_index) * sizeof(struct loader_used_object_status)) {
        inst->debug_utils_messengers_list.list[*debug_messenger_index].status = VK_FALSE;
        if (NULL != pAllocator) {
            inst->debug_utils_messengers_list.list[*debug_messenger_index].allocation_callbacks = *pAllocator;
        }
    }

    loader_free_with_instance_fallback(pAllocator, inst, debug_messenger_index);
}

// This is the instance chain terminator function for SubmitDebugUtilsMessageEXT
VKAPI_ATTR void VKAPI_CALL terminator_SubmitDebugUtilsMessageEXT(VkInstance instance,
                                                                 VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                                 VkDebugUtilsMessageTypeFlagsEXT messageTypes,
                                                                 const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData) {
    loader_platform_thread_lock_mutex(&loader_lock);
    // NOTE: Just make the callback ourselves because there could be one or more ICDs that support this extension
    //       and each one will trigger the callback to the user.  This would result in multiple callback triggers
    //       per message.  Instead, if we get a messaged up to here, then just trigger the message ourselves and
    //       return.  This would still allow the ICDs to trigger their own messages, but won't get any external ones.
    struct loader_instance *inst = (struct loader_instance *)instance;
    util_SubmitDebugUtilsMessageEXT(inst, messageSeverity, messageTypes, pCallbackData);
    loader_platform_thread_unlock_mutex(&loader_lock);
}

// VK_EXT_debug_report related items

VkResult util_CreateDebugReportCallback(struct loader_instance *inst, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                        const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT callback) {
    VkLayerDbgFunctionNode *new_dbg_func_node = NULL;

    new_dbg_func_node = (VkLayerDbgFunctionNode *)loader_calloc_with_instance_fallback(
        pAllocator, inst, sizeof(VkLayerDbgFunctionNode), VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    if (!new_dbg_func_node) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }

    new_dbg_func_node->is_messenger = false;
    new_dbg_func_node->report.msgCallback = callback;
    new_dbg_func_node->report.pfnMsgCallback = pCreateInfo->pfnCallback;
    new_dbg_func_node->report.msgFlags = pCreateInfo->flags;
    new_dbg_func_node->pUserData = pCreateInfo->pUserData;
    new_dbg_func_node->pNext = inst->instance_only_dbg_function_head;
    inst->instance_only_dbg_function_head = new_dbg_func_node;
    inst->current_dbg_function_head = inst->instance_only_dbg_function_head;

    return VK_SUCCESS;
}

VKAPI_ATTR VkResult VKAPI_CALL debug_utils_CreateDebugReportCallbackEXT(VkInstance instance,
                                                                        const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                                                        const VkAllocationCallbacks *pAllocator,
                                                                        VkDebugReportCallbackEXT *pCallback) {
    struct loader_instance *inst = loader_get_instance(instance);
    loader_platform_thread_lock_mutex(&loader_lock);
    VkResult result = inst->disp->layer_inst_disp.CreateDebugReportCallbackEXT(inst->instance, pCreateInfo, pAllocator, pCallback);
    loader_platform_thread_unlock_mutex(&loader_lock);
    return result;
}

// Utility function to handle reporting
VkBool32 util_DebugReportMessage(const struct loader_instance *inst, VkFlags msgFlags, VkDebugReportObjectTypeEXT objectType,
                                 uint64_t srcObject, size_t location, int32_t msgCode, const char *pLayerPrefix, const char *pMsg) {
    VkBool32 bail = false;
    VkLayerDbgFunctionNode *pTrav = inst->current_dbg_function_head;
    VkDebugUtilsMessageSeverityFlagBitsEXT severity;
    VkDebugUtilsMessageTypeFlagsEXT types;
    VkDebugUtilsMessengerCallbackDataEXT callback_data;
    VkDebugUtilsObjectNameInfoEXT object_name;

    debug_utils_ReportFlagsToAnnotFlags(msgFlags, false, &severity, &types);
    debug_utils_ReportObjectToAnnotObject(objectType, srcObject, &object_name);

    callback_data.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT;
    callback_data.pNext = NULL;
    callback_data.flags = 0;
    callback_data.pMessageIdName = pLayerPrefix;
    callback_data.messageIdNumber = msgCode;
    callback_data.pMessage = pMsg;
    callback_data.cmdBufLabelCount = 0;
    callback_data.pCmdBufLabels = NULL;
    callback_data.queueLabelCount = 0;
    callback_data.pQueueLabels = NULL;
    callback_data.objectCount = 1;
    callback_data.pObjects = &object_name;

    while (pTrav) {
        if (!pTrav->is_messenger && pTrav->report.msgFlags & msgFlags) {
            if (pTrav->report.pfnMsgCallback(msgFlags, objectType, srcObject, location, msgCode, pLayerPrefix, pMsg,
                                             pTrav->pUserData)) {
                bail = true;
            }
        }
        if (pTrav->is_messenger && (pTrav->messenger.messageSeverity & severity) && (pTrav->messenger.messageType & types)) {
            if (pTrav->messenger.pfnUserCallback(severity, types, &callback_data, pTrav->pUserData)) {
                bail = true;
            }
        }

        pTrav = pTrav->pNext;
    }

    return bail;
}

void util_DestroyDebugReportCallback(struct loader_instance *inst, VkDebugReportCallbackEXT callback,
                                     const VkAllocationCallbacks *pAllocator) {
    VkLayerDbgFunctionNode *pTrav = inst->current_dbg_function_head;
    VkLayerDbgFunctionNode *pPrev = pTrav;

    while (pTrav) {
        if (!pTrav->is_messenger && pTrav->report.msgCallback == callback) {
            pPrev->pNext = pTrav->pNext;
            if (inst->current_dbg_function_head == pTrav) inst->current_dbg_function_head = pTrav->pNext;
            if (inst->instance_only_dbg_function_head == pTrav) inst->instance_only_dbg_function_head = pTrav->pNext;
            if (inst->current_dbg_function_head == pTrav) inst->current_dbg_function_head = pTrav->pNext;
            loader_free_with_instance_fallback(pAllocator, inst, pTrav);
            break;
        }
        pPrev = pTrav;
        pTrav = pTrav->pNext;
    }
}

VkResult util_CreateDebugReportCallbacks(struct loader_instance *inst, const void *pChain,
                                         const VkAllocationCallbacks *pAllocator) {
    const void *pNext = pChain;
    while (pNext) {
        VkBaseInStructure in_structure = {0};
        memcpy(&in_structure, pNext, sizeof(VkBaseInStructure));
        if (in_structure.sType == VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT) {
            // Assign a unique handle to each callback (just use the address of the VkDebugReportCallbackCreateInfoEXT):
            // This is only being used this way due to it being for an 'anonymous' callback during instance creation
            VkDebugReportCallbackEXT report_handle = (VkDebugReportCallbackEXT)(uintptr_t)pNext;
            VkResult ret =
                util_CreateDebugReportCallback(inst, (const VkDebugReportCallbackCreateInfoEXT *)pNext, pAllocator, report_handle);
            if (ret != VK_SUCCESS) {
                return ret;
            }
        }
        pNext = in_structure.pNext;
    }
    return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL debug_utils_DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                                     const VkAllocationCallbacks *pAllocator) {
    struct loader_instance *inst = loader_get_instance(instance);
    loader_platform_thread_lock_mutex(&loader_lock);

    inst->disp->layer_inst_disp.DestroyDebugReportCallbackEXT(inst->instance, callback, pAllocator);

    loader_platform_thread_unlock_mutex(&loader_lock);
}

VKAPI_ATTR void VKAPI_CALL debug_utils_DebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                                             VkDebugReportObjectTypeEXT objType, uint64_t object, size_t location,
                                                             int32_t msgCode, const char *pLayerPrefix, const char *pMsg) {
    struct loader_instance *inst = loader_get_instance(instance);

    inst->disp->layer_inst_disp.DebugReportMessageEXT(inst->instance, flags, objType, object, location, msgCode, pLayerPrefix,
                                                      pMsg);
}

// This is the instance chain terminator function
// for CreateDebugReportCallback
VKAPI_ATTR VkResult VKAPI_CALL terminator_CreateDebugReportCallbackEXT(VkInstance instance,
                                                                       const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
                                                                       const VkAllocationCallbacks *pAllocator,
                                                                       VkDebugReportCallbackEXT *pCallback) {
    struct loader_instance *inst = (struct loader_instance *)instance;
    VkResult res = VK_SUCCESS;
    VkLayerDbgFunctionNode *new_dbg_func_node = NULL;
    uint32_t next_index = 0;

    uint32_t *pNextIndex = loader_instance_heap_alloc(inst, sizeof(uint32_t), VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
    if (NULL == pNextIndex) {
        res = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    }

    res = loader_get_next_available_entry(inst, &inst->debug_report_callbacks_list, &next_index, pAllocator);
    if (res != VK_SUCCESS) {
        goto out;
    }

    for (struct loader_icd_term *icd_term = inst->icd_terms; icd_term != NULL; icd_term = icd_term->next) {
        if (icd_term->debug_report_callback_list.list == NULL) {
            res = loader_init_generic_list(inst, (struct loader_generic_list *)&icd_term->debug_report_callback_list,
                                           sizeof(VkDebugUtilsMessengerEXT));
            if (res != VK_SUCCESS) {
                goto out;
            }
        } else if (icd_term->debug_report_callback_list.capacity <= next_index * sizeof(VkDebugReportCallbackEXT)) {
            res = loader_resize_generic_list(inst, (struct loader_generic_list *)&icd_term->debug_report_callback_list);
            if (res != VK_SUCCESS) {
                goto out;
            }
        }

        if (icd_term->dispatch.CreateDebugReportCallbackEXT) {
            res = icd_term->dispatch.CreateDebugReportCallbackEXT(icd_term->instance, pCreateInfo, pAllocator,
                                                                  &icd_term->debug_report_callback_list.list[next_index]);

            if (res != VK_SUCCESS) {
                goto out;
            }
        }
    }

    // Setup the debug report callback in the terminator since a layer may want
    // to grab the information itself (RenderDoc) and then return back to the
    // user callback a sub-set of the messages.
    new_dbg_func_node = (VkLayerDbgFunctionNode *)loader_calloc_with_instance_fallback(
        pAllocator, inst, sizeof(VkLayerDbgFunctionNode), VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);

    if (!new_dbg_func_node) {
        res = VK_ERROR_OUT_OF_HOST_MEMORY;
        goto out;
    }

    new_dbg_func_node->is_messenger = false;
    new_dbg_func_node->report.pfnMsgCallback = pCreateInfo->pfnCallback;
    new_dbg_func_node->report.msgFlags = pCreateInfo->flags;
    new_dbg_func_node->pUserData = pCreateInfo->pUserData;
    new_dbg_func_node->pNext = inst->current_dbg_function_head;
    inst->current_dbg_function_head = new_dbg_func_node;
    *pNextIndex = next_index;
    *pCallback = (VkDebugReportCallbackEXT)(uintptr_t)pNextIndex;
    new_dbg_func_node->report.msgCallback = *pCallback;

out:

    // Roll back on errors
    if (VK_SUCCESS != res) {
        if (pNextIndex) {
            for (struct loader_icd_term *icd_term = inst->icd_terms; icd_term; icd_term = icd_term->next) {
                if (icd_term->debug_report_callback_list.list && icd_term->debug_report_callback_list.list[next_index] &&
                    NULL != icd_term->dispatch.DestroyDebugReportCallbackEXT) {
                    icd_term->dispatch.DestroyDebugReportCallbackEXT(
                        icd_term->instance, icd_term->debug_report_callback_list.list[next_index], pAllocator);
                }
            }
        }
        if (inst->debug_report_callbacks_list.list &&
            inst->debug_report_callbacks_list.capacity > (*pNextIndex) * sizeof(struct loader_used_object_status)) {
            inst->debug_report_callbacks_list.list[*pNextIndex].status = VK_FALSE;
            if (NULL != pAllocator) {
                inst->debug_report_callbacks_list.list[*pNextIndex].allocation_callbacks = *pAllocator;
            }
        }
        loader_free_with_instance_fallback(pAllocator, inst, new_dbg_func_node);
        loader_free_with_instance_fallback(pAllocator, inst, pNextIndex);
    }

    return res;
}

// This is the instance chain terminator function for DestroyDebugReportCallback
VKAPI_ATTR void VKAPI_CALL terminator_DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
                                                                    const VkAllocationCallbacks *pAllocator) {
    struct loader_instance *inst = (struct loader_instance *)instance;
    uint32_t *debug_report_index = (uint32_t *)(uintptr_t)callback;
    // Make sure that callback actually points to anything
    if (NULL == debug_report_index) {
        return;
    }
    for (struct loader_icd_term *icd_term = inst->icd_terms; icd_term; icd_term = icd_term->next) {
        if (icd_term->debug_report_callback_list.list && icd_term->debug_report_callback_list.list[*debug_report_index] &&
            NULL != icd_term->dispatch.DestroyDebugReportCallbackEXT) {
            icd_term->dispatch.DestroyDebugReportCallbackEXT(
                icd_term->instance, icd_term->debug_report_callback_list.list[*debug_report_index], pAllocator);
        }
    }

    util_DestroyDebugReportCallback(inst, callback, pAllocator);
    if (inst->debug_report_callbacks_list.list &&
        inst->debug_report_callbacks_list.capacity > (*debug_report_index) * sizeof(struct loader_used_object_status)) {
        inst->debug_report_callbacks_list.list[*debug_report_index].status = VK_FALSE;
        if (NULL != pAllocator) {
            inst->debug_report_callbacks_list.list[*debug_report_index].allocation_callbacks = *pAllocator;
        }
    }
    loader_free_with_instance_fallback(pAllocator, inst, debug_report_index);
}

// This is the instance chain terminator function for DebugReportMessage
VKAPI_ATTR void VKAPI_CALL terminator_DebugReportMessageEXT(VkInstance instance, VkDebugReportFlagsEXT flags,
                                                            VkDebugReportObjectTypeEXT objType, uint64_t object, size_t location,
                                                            int32_t msgCode, const char *pLayerPrefix, const char *pMsg) {
    const struct loader_icd_term *icd_term;

    struct loader_instance *inst = (struct loader_instance *)instance;

    loader_platform_thread_lock_mutex(&loader_lock);
    for (icd_term = inst->icd_terms; icd_term; icd_term = icd_term->next) {
        if (icd_term->dispatch.DebugReportMessageEXT != NULL) {
            icd_term->dispatch.DebugReportMessageEXT(icd_term->instance, flags, objType, object, location, msgCode, pLayerPrefix,
                                                     pMsg);
        }
    }

    // Now that all ICDs have seen the message, call the necessary callbacks.  Ignoring "bail" return value
    // as there is nothing to bail from at this point.

    util_DebugReportMessage(inst, flags, objType, object, location, msgCode, pLayerPrefix, pMsg);

    loader_platform_thread_unlock_mutex(&loader_lock);
}

// General utilities

const VkExtensionProperties debug_utils_extension_info[] = {
    {VK_EXT_DEBUG_REPORT_EXTENSION_NAME, VK_EXT_DEBUG_REPORT_SPEC_VERSION},
    {VK_EXT_DEBUG_UTILS_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_SPEC_VERSION},
};

void destroy_debug_callbacks_chain(struct loader_instance *inst, const VkAllocationCallbacks *pAllocator) {
    VkLayerDbgFunctionNode *pTrav = inst->current_dbg_function_head;
    VkLayerDbgFunctionNode *pNext = NULL;
    while (pTrav) {
        pNext = pTrav->pNext;
        loader_free_with_instance_fallback(pAllocator, inst, pTrav);
        pTrav = pNext;
    }
    inst->current_dbg_function_head = NULL;
}

VkResult add_debug_extensions_to_ext_list(const struct loader_instance *inst, struct loader_extension_list *ext_list) {
    return loader_add_to_ext_list(inst, ext_list, sizeof(debug_utils_extension_info) / sizeof(VkExtensionProperties),
                                  debug_utils_extension_info);
}

bool debug_extensions_InstanceGpa(struct loader_instance *ptr_instance, const char *name, void **addr) {
    bool ret_type = false;

    *addr = NULL;

    if (!strcmp("vkCreateDebugReportCallbackEXT", name)) {
        *addr = ptr_instance->enabled_extensions.ext_debug_report == 1 ? (void *)debug_utils_CreateDebugReportCallbackEXT : NULL;
        ret_type = true;
    } else if (!strcmp("vkDestroyDebugReportCallbackEXT", name)) {
        *addr = ptr_instance->enabled_extensions.ext_debug_report == 1 ? (void *)debug_utils_DestroyDebugReportCallbackEXT : NULL;
        ret_type = true;
    } else if (!strcmp("vkDebugReportMessageEXT", name)) {
        *addr = ptr_instance->enabled_extensions.ext_debug_report == 1 ? (void *)debug_utils_DebugReportMessageEXT : NULL;
        return true;
    }
    if (!strcmp("vkCreateDebugUtilsMessengerEXT", name)) {
        *addr = ptr_instance->enabled_extensions.ext_debug_utils == 1 ? (void *)debug_utils_CreateDebugUtilsMessengerEXT : NULL;
        ret_type = true;
    } else if (!strcmp("vkDestroyDebugUtilsMessengerEXT", name)) {
        *addr = ptr_instance->enabled_extensions.ext_debug_utils == 1 ? (void *)debug_utils_DestroyDebugUtilsMessengerEXT : NULL;
        ret_type = true;
    } else if (!strcmp("vkSubmitDebugUtilsMessageEXT", name)) {
        *addr = ptr_instance->enabled_extensions.ext_debug_utils == 1 ? (void *)debug_utils_SubmitDebugUtilsMessageEXT : NULL;
        ret_type = true;
    }

    return ret_type;
}

bool debug_utils_ReportFlagsToAnnotFlags(VkDebugReportFlagsEXT dr_flags, bool default_flag_is_spec,
                                         VkDebugUtilsMessageSeverityFlagBitsEXT *da_severity,
                                         VkDebugUtilsMessageTypeFlagsEXT *da_type) {
    bool type_set = false;
    if (NULL == da_severity || NULL == da_type) {
        return false;
    }
    *da_type = 0;
    *da_severity = 0;

    if ((dr_flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) != 0) {
        *da_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT;
        *da_type |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
        type_set = true;
    } else if ((dr_flags & (VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)) != 0) {
        *da_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
    } else if ((dr_flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) != 0) {
        *da_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    } else if ((dr_flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) != 0) {
        *da_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
        *da_type |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
        type_set = true;
    }

    if ((dr_flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) != 0) {
        *da_type |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    } else if (!type_set) {
        if (default_flag_is_spec) {
            *da_type |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        } else {
            *da_type |= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
        }
    }

    return true;
}

bool debug_utils_AnnotFlagsToReportFlags(VkDebugUtilsMessageSeverityFlagBitsEXT da_severity,
                                         VkDebugUtilsMessageTypeFlagsEXT da_type, VkDebugReportFlagsEXT *dr_flags) {
    if (NULL == dr_flags) {
        return false;
    }

    *dr_flags = 0;

    if ((da_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) != 0) {
        *dr_flags |= VK_DEBUG_REPORT_ERROR_BIT_EXT;
    } else if ((da_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) != 0) {
        if ((da_type & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) != 0) {
            *dr_flags |= VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
        } else {
            *dr_flags |= VK_DEBUG_REPORT_WARNING_BIT_EXT;
        }
    } else if ((da_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) != 0) {
        *dr_flags |= VK_DEBUG_REPORT_INFORMATION_BIT_EXT;
    } else if ((da_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) != 0) {
        *dr_flags |= VK_DEBUG_REPORT_DEBUG_BIT_EXT;
    }

    return true;
}

bool debug_utils_ReportObjectToAnnotObject(VkDebugReportObjectTypeEXT dr_object_type, uint64_t object_handle,
                                           VkDebugUtilsObjectNameInfoEXT *da_object_name_info) {
    if (NULL == da_object_name_info) {
        return false;
    }
    da_object_name_info->sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
    da_object_name_info->pNext = NULL;
    da_object_name_info->objectHandle = (uint64_t)(uintptr_t)object_handle;
    da_object_name_info->pObjectName = NULL;
    da_object_name_info->objectType = convertDebugReportObjectToCoreObject(dr_object_type);
    return true;
}

bool debug_utils_AnnotObjectToDebugReportObject(const VkDebugUtilsObjectNameInfoEXT *da_object_name_info,
                                                VkDebugReportObjectTypeEXT *dr_object_type, uint64_t *dr_object_handle) {
    if (NULL == da_object_name_info || NULL == dr_object_type || NULL == dr_object_handle) {
        return false;
    }
    *dr_object_type = convertCoreObjectToDebugReportObject(da_object_name_info->objectType);
    *dr_object_handle = da_object_name_info->objectHandle;
    return true;
}
