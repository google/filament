/*
 * Copyright (c) 2015-2022 Valve Corporation
 * Copyright (c) 2015-2022 LunarG, Inc.
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
 * Author: Jon Ashburn <jon@lunarg.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string>
#include <algorithm>
#include <assert.h>
#include <unordered_map>
#include <memory>
#include <vector>

#include "vulkan/vk_layer.h"
#include "vk_dispatch_table_helper.h"
#include "loader/vk_loader_layer.h"

// Export full support of instance extension VK_EXT_direct_mode_display extension
#if !defined(TEST_LAYER_EXPORT_DIRECT_DISP)
#define TEST_LAYER_EXPORT_DIRECT_DISP 0
#endif

// Export full support of instance extension VK_EXT_display_surface_counter extension
#if !defined(TEST_LAYER_EXPORT_DISP_SURF_COUNT)
#define TEST_LAYER_EXPORT_DISP_SURF_COUNT 0
#endif

// Export full support of device extension VK_KHR_maintenance1 extension
#if !defined(TEST_LAYER_EXPORT_MAINT_1)
#define TEST_LAYER_EXPORT_MAINT_1 0
#endif

// Export full support of device extension VK_KHR_shared_presentable_image extension
#if !defined(TEST_LAYER_EXPORT_PRESENT_IMAGE)
#define TEST_LAYER_EXPORT_PRESENT_IMAGE 0
#endif

#if !defined(VK_LAYER_EXPORT)
#if defined(__GNUC__) && __GNUC__ >= 4
#define VK_LAYER_EXPORT __attribute__((visibility("default")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590)
#define VK_LAYER_EXPORT __attribute__((visibility("default")))
#else
#define VK_LAYER_EXPORT
#endif
#endif

struct wrapped_phys_dev_obj {
    VkLayerInstanceDispatchTable *loader_disp;
    struct wrapped_inst_obj *inst;  // parent instance object
    void *obj;
};

struct wrapped_inst_obj {
    VkLayerInstanceDispatchTable *loader_disp;
    VkLayerInstanceDispatchTable layer_disp;  // this layer's dispatch table
    PFN_vkSetInstanceLoaderData pfn_inst_init;
    struct wrapped_phys_dev_obj *ptr_phys_devs;  // any enumerated phys devs
    VkInstance obj;
    bool layer_is_implicit;
    bool direct_display_enabled;
    bool display_surf_counter_enabled;
    bool debug_utils_enabled;
};

struct wrapped_dev_obj {
    VkLayerDispatchTable *loader_disp;
    VkLayerDispatchTable disp;
    PFN_vkSetDeviceLoaderData pfn_dev_init;
    PFN_vkGetDeviceProcAddr pfn_get_dev_proc_addr;
    VkDevice obj;
    bool maintanence_1_enabled;
    bool present_image_enabled;
    bool debug_utils_enabled;
    bool debug_report_enabled;
    bool debug_marker_enabled;
};

struct wrapped_debug_util_mess_obj {
    VkInstance inst;
    VkDebugUtilsMessengerEXT obj;
};

struct saved_wrapped_handles_storage {
    std::vector<wrapped_inst_obj *> instances;
    std::vector<wrapped_phys_dev_obj *> physical_devices;
    std::vector<wrapped_dev_obj *> devices;
    std::vector<wrapped_debug_util_mess_obj *> debug_util_messengers;
};

saved_wrapped_handles_storage saved_wrapped_handles;

VkInstance unwrap_instance(const VkInstance instance, wrapped_inst_obj **inst) {
    *inst = reinterpret_cast<wrapped_inst_obj *>(instance);
    auto it = std::find(saved_wrapped_handles.instances.begin(), saved_wrapped_handles.instances.end(), *inst);
    return (it == saved_wrapped_handles.instances.end()) ? VK_NULL_HANDLE : (*inst)->obj;
}

VkPhysicalDevice unwrap_phys_dev(const VkPhysicalDevice physical_device, wrapped_phys_dev_obj **phys_dev) {
    *phys_dev = reinterpret_cast<wrapped_phys_dev_obj *>(physical_device);
    auto it = std::find(saved_wrapped_handles.physical_devices.begin(), saved_wrapped_handles.physical_devices.end(), *phys_dev);
    return (it == saved_wrapped_handles.physical_devices.end()) ? VK_NULL_HANDLE
                                                                : reinterpret_cast<VkPhysicalDevice>((*phys_dev)->obj);
}

VkDevice unwrap_device(const VkDevice device, wrapped_dev_obj **dev) {
    *dev = reinterpret_cast<wrapped_dev_obj *>(device);
    auto it = std::find(saved_wrapped_handles.devices.begin(), saved_wrapped_handles.devices.end(), *dev);
    return (it == saved_wrapped_handles.devices.end()) ? VK_NULL_HANDLE : (*dev)->obj;
}

VkDebugUtilsMessengerEXT unwrap_debug_util_messenger(const VkDebugUtilsMessengerEXT messenger, wrapped_debug_util_mess_obj **mess) {
    *mess = reinterpret_cast<wrapped_debug_util_mess_obj *>(messenger);
    auto it =
        std::find(saved_wrapped_handles.debug_util_messengers.begin(), saved_wrapped_handles.debug_util_messengers.end(), *mess);
    return (it == saved_wrapped_handles.debug_util_messengers.end()) ? VK_NULL_HANDLE : (*mess)->obj;
}

VkLayerInstanceCreateInfo *get_chain_info(const VkInstanceCreateInfo *pCreateInfo, VkLayerFunction func) {
    VkLayerInstanceCreateInfo *chain_info = (VkLayerInstanceCreateInfo *)pCreateInfo->pNext;
    while (chain_info && !(chain_info->sType == VK_STRUCTURE_TYPE_LOADER_INSTANCE_CREATE_INFO && chain_info->function == func)) {
        chain_info = (VkLayerInstanceCreateInfo *)chain_info->pNext;
    }
    assert(chain_info != NULL);
    return chain_info;
}

VkLayerDeviceCreateInfo *get_chain_info(const VkDeviceCreateInfo *pCreateInfo, VkLayerFunction func) {
    VkLayerDeviceCreateInfo *chain_info = (VkLayerDeviceCreateInfo *)pCreateInfo->pNext;
    while (chain_info && !(chain_info->sType == VK_STRUCTURE_TYPE_LOADER_DEVICE_CREATE_INFO && chain_info->function == func)) {
        chain_info = (VkLayerDeviceCreateInfo *)chain_info->pNext;
    }
    assert(chain_info != NULL);
    return chain_info;
}

namespace wrap_objects {

const VkLayerProperties global_layer = {
    "VK_LAYER_LUNARG_wrap_objects",
    VK_HEADER_VERSION_COMPLETE,
    1,
    "LunarG Test Layer",
};

uint32_t loader_layer_if_version = CURRENT_LOADER_LAYER_INTERFACE_VERSION;

VKAPI_ATTR VkResult VKAPI_CALL wrap_vkCreateInstance(const VkInstanceCreateInfo *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkInstance *pInstance) {
    VkLayerInstanceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkCreateInstance fpCreateInstance = (PFN_vkCreateInstance)fpGetInstanceProcAddr(NULL, "vkCreateInstance");
    if (fpCreateInstance == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    // Advance the link info for the next element on the chain
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;
    VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
    if (result != VK_SUCCESS) {
        return result;
    }
    auto inst = new wrapped_inst_obj;
    if (!inst) return VK_ERROR_OUT_OF_HOST_MEMORY;
    saved_wrapped_handles.instances.push_back(inst);
    memset(inst, 0, sizeof(*inst));
    inst->obj = (*pInstance);
    *pInstance = reinterpret_cast<VkInstance>(inst);
    // store the loader callback for initializing created dispatchable objects
    chain_info = get_chain_info(pCreateInfo, VK_LOADER_DATA_CALLBACK);
    if (chain_info) {
        inst->pfn_inst_init = chain_info->u.pfnSetInstanceLoaderData;
        result = inst->pfn_inst_init(inst->obj, reinterpret_cast<void *>(inst));
        if (VK_SUCCESS != result) return result;
    } else {
        inst->pfn_inst_init = NULL;
        inst->loader_disp = *(reinterpret_cast<VkLayerInstanceDispatchTable **>(inst->obj));
    }
    layer_init_instance_dispatch_table(inst->obj, &inst->layer_disp, fpGetInstanceProcAddr);
    bool found = false;
    for (uint32_t layer = 0; layer < pCreateInfo->enabledLayerCount; ++layer) {
        std::string layer_name = pCreateInfo->ppEnabledLayerNames[layer];
        std::transform(layer_name.begin(), layer_name.end(), layer_name.begin(),
                       [](char c) { return static_cast<char>(::tolower(static_cast<char>(c))); });
        if (layer_name.find("wrap") != std::string::npos && layer_name.find("obj") != std::string::npos) {
            found = true;
            break;
        }
    }
    if (!found) {
        inst->layer_is_implicit = true;
    }

    for (uint32_t ext = 0; ext < pCreateInfo->enabledExtensionCount; ++ext) {
        if (!strcmp(pCreateInfo->ppEnabledExtensionNames[ext], VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME)) {
#if TEST_LAYER_EXPORT_DIRECT_DISP
            inst->direct_display_enabled = true;
#endif
        }
        if (!strcmp(pCreateInfo->ppEnabledExtensionNames[ext], VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME)) {
#if TEST_LAYER_EXPORT_DISP_SURF_COUNT
            inst->display_surf_counter_enabled = true;
#endif
        }
        if (!strcmp(pCreateInfo->ppEnabledExtensionNames[ext], VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
            inst->debug_utils_enabled = true;
        }
    }

    return result;
}

VKAPI_ATTR void VKAPI_CALL wrap_vkDestroyInstance(VkInstance instance, const VkAllocationCallbacks *pAllocator) {
    wrapped_inst_obj *inst;
    auto vk_inst = unwrap_instance(instance, &inst);
    VkLayerInstanceDispatchTable *pDisp = &inst->layer_disp;
    pDisp->DestroyInstance(vk_inst, pAllocator);
    if (inst->ptr_phys_devs) delete[] inst->ptr_phys_devs;
    delete inst;
}

VKAPI_ATTR VkResult VKAPI_CALL wrap_vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                                   const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                                   const VkAllocationCallbacks *pAllocator,
                                                                   VkDebugUtilsMessengerEXT *pMessenger) {
    wrapped_inst_obj *inst;
    auto vk_inst = unwrap_instance(instance, &inst);
    VkLayerInstanceDispatchTable *pDisp = &inst->layer_disp;
    VkResult result = pDisp->CreateDebugUtilsMessengerEXT(vk_inst, pCreateInfo, pAllocator, pMessenger);
    auto mess = new wrapped_debug_util_mess_obj;
    if (!mess) return VK_ERROR_OUT_OF_HOST_MEMORY;
    saved_wrapped_handles.debug_util_messengers.push_back(mess);
    memset(mess, 0, sizeof(*mess));
    mess->obj = (*pMessenger);
    *pMessenger = reinterpret_cast<VkDebugUtilsMessengerEXT>(mess);
    return result;
}

VKAPI_ATTR void VKAPI_CALL wrap_vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                                const VkAllocationCallbacks *pAllocator) {
    wrapped_inst_obj *inst;
    auto vk_inst = unwrap_instance(instance, &inst);
    VkLayerInstanceDispatchTable *pDisp = &inst->layer_disp;
    wrapped_debug_util_mess_obj *mess;
    auto vk_mess = unwrap_debug_util_messenger(messenger, &mess);
    pDisp->DestroyDebugUtilsMessengerEXT(vk_inst, vk_mess, pAllocator);
    delete mess;
}

VKAPI_ATTR void VKAPI_CALL wrap_vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface,
                                                    const VkAllocationCallbacks *pAllocator) {
    wrapped_inst_obj *inst;
    auto vk_inst = unwrap_instance(instance, &inst);
    VkLayerInstanceDispatchTable *pDisp = &inst->layer_disp;
    pDisp->DestroySurfaceKHR(vk_inst, surface, pAllocator);
}
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
VKAPI_ATTR VkResult VKAPI_CALL wrap_vkCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    wrapped_inst_obj *inst;
    auto vk_inst = unwrap_instance(instance, &inst);
    VkLayerInstanceDispatchTable *pDisp = &inst->layer_disp;
    return pDisp->CreateAndroidSurfaceKHR(vk_inst, pCreateInfo, pAllocator, pSurface);
}
#endif

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VKAPI_ATTR VkResult VKAPI_CALL wrap_vkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    wrapped_inst_obj *inst;
    auto vk_inst = unwrap_instance(instance, &inst);
    VkLayerInstanceDispatchTable *pDisp = &inst->layer_disp;
    return pDisp->CreateWin32SurfaceKHR(vk_inst, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
VKAPI_ATTR VkResult VKAPI_CALL wrap_vkCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    wrapped_inst_obj *inst;
    auto vk_inst = unwrap_instance(instance, &inst);
    VkLayerInstanceDispatchTable *pDisp = &inst->layer_disp;
    return pDisp->CreateWaylandSurfaceKHR(vk_inst, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#if defined(VK_USE_PLATFORM_XCB_KHR)
VKAPI_ATTR VkResult VKAPI_CALL wrap_vkCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    wrapped_inst_obj *inst;
    auto vk_inst = unwrap_instance(instance, &inst);
    VkLayerInstanceDispatchTable *pDisp = &inst->layer_disp;
    return pDisp->CreateXcbSurfaceKHR(vk_inst, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_XCB_KHR

#if defined(VK_USE_PLATFORM_XLIB_KHR)
VKAPI_ATTR VkResult VKAPI_CALL wrap_vkCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
                                                           const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    wrapped_inst_obj *inst;
    auto vk_inst = unwrap_instance(instance, &inst);
    VkLayerInstanceDispatchTable *pDisp = &inst->layer_disp;
    return pDisp->CreateXlibSurfaceKHR(vk_inst, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_XLIB_KHR

#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
VKAPI_ATTR VkResult VKAPI_CALL wrap_vkCreateDirectFBSurfaceEXT(VkInstance instance,
                                                               const VkDirectFBSurfaceCreateInfoEXT *pCreateInfo,
                                                               const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    wrapped_inst_obj *inst;
    auto vk_inst = unwrap_instance(instance, &inst);
    VkLayerInstanceDispatchTable *pDisp = &inst->layer_disp;
    return pDisp->CreateDirectFBSurfaceEXT(vk_inst, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT

#if defined(VK_USE_PLATFORM_MACOS_MVK)
VKAPI_ATTR VkResult VKAPI_CALL wrap_vkCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    wrapped_inst_obj *inst;
    auto vk_inst = unwrap_instance(instance, &inst);
    VkLayerInstanceDispatchTable *pDisp = &inst->layer_disp;
    return pDisp->CreateMacOSSurfaceMVK(vk_inst, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_MACOS_MVK

#if defined(VK_USE_PLATFORM_IOS_MVK)
VKAPI_ATTR VkResult VKAPI_CALL wrap_vkCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    wrapped_inst_obj *inst;
    auto vk_inst = unwrap_instance(instance, &inst);
    VkLayerInstanceDispatchTable *pDisp = &inst->layer_disp;
    return pDisp->CreateIOSSurfaceMVK(vk_inst, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_IOS_MVK

#if defined(VK_USE_PLATFORM_GGP)
VKAPI_ATTR VkResult VKAPI_CALL wrap_vkCreateStreamDescriptorSurfaceGGP(VkInstance instance,
                                                                       const VkStreamDescriptorSurfaceCreateInfoGGP *pCreateInfo,
                                                                       const VkAllocationCallbacks *pAllocator,
                                                                       VkSurfaceKHR *pSurface) {
    wrapped_inst_obj *inst;
    auto vk_inst = unwrap_instance(instance, &inst);
    VkLayerInstanceDispatchTable *pDisp = &inst->layer_disp;
    return pDisp->CreateStreamDescriptorSurfaceGGP(vk_inst, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_GGP

#if defined(VK_USE_PLATFORM_METAL_EXT)
VKAPI_ATTR VkResult VKAPI_CALL wrap_vkCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT *pCreateInfo,
                                                            const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    wrapped_inst_obj *inst;
    auto vk_inst = unwrap_instance(instance, &inst);
    VkLayerInstanceDispatchTable *pDisp = &inst->layer_disp;
    return pDisp->CreateMetalSurfaceEXT(vk_inst, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_METAL_EXT

#if defined(VK_USE_PLATFORM_SCREEN_QNX)
VKAPI_ATTR VkResult VKAPI_CALL wrap_vkCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX *pCreateInfo,
                                                             const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    wrapped_inst_obj *inst;
    auto vk_inst = unwrap_instance(instance, &inst);
    VkLayerInstanceDispatchTable *pDisp = &inst->layer_disp;
    return pDisp->CreateScreenSurfaceQNX(vk_inst, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

VKAPI_ATTR VkResult VKAPI_CALL wrap_vkEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
                                                               VkPhysicalDevice *pPhysicalDevices) {
    wrapped_inst_obj *inst;
    auto vk_inst = unwrap_instance(instance, &inst);
    VkResult result = inst->layer_disp.EnumeratePhysicalDevices(vk_inst, pPhysicalDeviceCount, pPhysicalDevices);

    if (VK_SUCCESS != result) return result;

    if (pPhysicalDevices != NULL) {
        assert(pPhysicalDeviceCount);
        auto phys_devs = new wrapped_phys_dev_obj[*pPhysicalDeviceCount];
        if (!phys_devs) return VK_ERROR_OUT_OF_HOST_MEMORY;
        saved_wrapped_handles.physical_devices.push_back(phys_devs);
        if (inst->ptr_phys_devs) delete[] inst->ptr_phys_devs;
        inst->ptr_phys_devs = phys_devs;
        for (uint32_t i = 0; i < *pPhysicalDeviceCount; i++) {
            if (inst->pfn_inst_init == NULL) {
                phys_devs[i].loader_disp = *(reinterpret_cast<VkLayerInstanceDispatchTable **>(pPhysicalDevices[i]));
            } else {
                result = inst->pfn_inst_init(vk_inst, reinterpret_cast<void *>(&phys_devs[i]));
                if (VK_SUCCESS != result) return result;
            }
            phys_devs[i].obj = reinterpret_cast<void *>(pPhysicalDevices[i]);
            phys_devs[i].inst = inst;
            pPhysicalDevices[i] = reinterpret_cast<VkPhysicalDevice>(&phys_devs[i]);
        }
    }
    return result;
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties *pProperties) {
    wrapped_phys_dev_obj *phys_dev;
    auto vk_phys_dev = unwrap_phys_dev(physicalDevice, &phys_dev);
    phys_dev->inst->layer_disp.GetPhysicalDeviceProperties(vk_phys_dev, pProperties);
}

VKAPI_ATTR void VKAPI_CALL wrap_vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
                                                                         uint32_t *pQueueFamilyPropertyCount,
                                                                         VkQueueFamilyProperties *pQueueFamilyProperties) {
    wrapped_phys_dev_obj *phys_dev;
    auto vk_phys_dev = unwrap_phys_dev(physicalDevice, &phys_dev);
    phys_dev->inst->layer_disp.GetPhysicalDeviceQueueFamilyProperties(vk_phys_dev, pQueueFamilyPropertyCount,
                                                                      pQueueFamilyProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL wrap_vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char *pLayerName,
                                                                         uint32_t *pPropertyCount,
                                                                         VkExtensionProperties *pProperties) {
    VkResult result = VK_SUCCESS;
    wrapped_phys_dev_obj *phys_dev;
    auto vk_phys_dev = unwrap_phys_dev(physicalDevice, &phys_dev);

    if (phys_dev->inst->layer_is_implicit || (pLayerName && !strcmp(pLayerName, global_layer.layerName))) {
        uint32_t ext_count = 0;
#if TEST_LAYER_EXPORT_MAINT_1
        ext_count++;
#endif
#if TEST_LAYER_EXPORT_PRESENT_IMAGE
        ext_count++;
#endif
        if (pPropertyCount) {
            if (pProperties) {
                [[maybe_unused]] uint32_t count = ext_count;
                if (count > *pPropertyCount) {
                    count = *pPropertyCount;
                    result = VK_INCOMPLETE;
                }

                ext_count = 0;
#if TEST_LAYER_EXPORT_MAINT_1
                if (ext_count < count) {
#if defined(_WIN32)
                    strncpy_s(pProperties[ext_count].extensionName, VK_MAX_EXTENSION_NAME_SIZE, VK_KHR_MAINTENANCE1_EXTENSION_NAME,
                              strlen(VK_KHR_MAINTENANCE1_EXTENSION_NAME) + 1);
#else
                    strncpy(pProperties[ext_count].extensionName, VK_KHR_MAINTENANCE1_EXTENSION_NAME, VK_MAX_EXTENSION_NAME_SIZE);
#endif
                    pProperties[ext_count].specVersion = 2;
                    ext_count++;
                }
#endif
#if TEST_LAYER_EXPORT_PRESENT_IMAGE
                if (ext_count < count) {
#if defined(_WIN32)
                    strncpy_s(pProperties[ext_count].extensionName, VK_MAX_EXTENSION_NAME_SIZE,
                              VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME,
                              strlen(VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME) + 1);
#else
                    strncpy(pProperties[ext_count].extensionName, VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME,
                            VK_MAX_EXTENSION_NAME_SIZE);
#endif
                    pProperties[ext_count].specVersion = 1;
                    ext_count++;
                }
#endif
            }
            *pPropertyCount = ext_count;
        }
        return result;
    } else {
        return phys_dev->inst->layer_disp.EnumerateDeviceExtensionProperties(vk_phys_dev, pLayerName, pPropertyCount, pProperties);
    }
}

VKAPI_ATTR VkResult VKAPI_CALL wrap_vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
                                                   const VkAllocationCallbacks *pAllocator, VkDevice *pDevice) {
    wrapped_phys_dev_obj *phys_dev;
    auto vk_phys_dev = unwrap_phys_dev(physicalDevice, &phys_dev);
    VkLayerDeviceCreateInfo *chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);
    PFN_vkGetInstanceProcAddr pfn_get_inst_proc_addr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr pfn_get_dev_proc_addr = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
    PFN_vkCreateDevice pfn_create_device = (PFN_vkCreateDevice)pfn_get_inst_proc_addr(phys_dev->inst->obj, "vkCreateDevice");
    if (pfn_create_device == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }
    // Advance the link info for the next element on the chain
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;
    VkResult result = pfn_create_device(vk_phys_dev, pCreateInfo, pAllocator, pDevice);
    if (result != VK_SUCCESS) {
        return result;
    }
    auto dev = new wrapped_dev_obj;
    if (!dev) {
        return VK_ERROR_OUT_OF_HOST_MEMORY;
    }
    saved_wrapped_handles.devices.push_back(dev);
    memset(dev, 0, sizeof(*dev));
    dev->obj = *pDevice;
    dev->pfn_get_dev_proc_addr = pfn_get_dev_proc_addr;
    *pDevice = reinterpret_cast<VkDevice>(dev);

    // Store the loader callback for initializing created dispatchable objects
    chain_info = get_chain_info(pCreateInfo, VK_LOADER_DATA_CALLBACK);
    if (chain_info) {
        dev->pfn_dev_init = chain_info->u.pfnSetDeviceLoaderData;
        result = dev->pfn_dev_init(dev->obj, reinterpret_cast<void *>(dev));
        if (VK_SUCCESS != result) {
            return result;
        }
    } else {
        dev->pfn_dev_init = NULL;
    }

    // Initialize layer's dispatch table
    layer_init_device_dispatch_table(dev->obj, &dev->disp, pfn_get_dev_proc_addr);

    for (uint32_t ext = 0; ext < pCreateInfo->enabledExtensionCount; ++ext) {
        if (!strcmp(pCreateInfo->ppEnabledExtensionNames[ext], VK_KHR_MAINTENANCE1_EXTENSION_NAME)) {
#if TEST_LAYER_EXPORT_MAINT_1
            dev->maintanence_1_enabled = true;
#endif
        }
        if (!strcmp(pCreateInfo->ppEnabledExtensionNames[ext], VK_KHR_SHARED_PRESENTABLE_IMAGE_EXTENSION_NAME)) {
#if TEST_LAYER_EXPORT_PRESENT_IMAGE
            dev->present_image_enabled = true;
#endif
        }
        if (!strcmp(pCreateInfo->ppEnabledExtensionNames[ext], VK_EXT_DEBUG_MARKER_EXTENSION_NAME)) {
            dev->debug_marker_enabled = true;
        }
    }
    dev->debug_utils_enabled = phys_dev->inst->debug_utils_enabled;

    return result;
}

VKAPI_ATTR void VKAPI_CALL wrap_vkDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator) {
    wrapped_dev_obj *dev;
    auto vk_dev = unwrap_device(device, &dev);
    dev->disp.DestroyDevice(vk_dev, pAllocator);
    delete dev;
}

// Fake instance extension support
VKAPI_ATTR VkResult VKAPI_CALL wrap_vkReleaseDisplayEXT(VkPhysicalDevice, VkDisplayKHR) { return VK_SUCCESS; }

VKAPI_ATTR VkResult VKAPI_CALL wrap_vkGetPhysicalDeviceSurfaceCapabilities2EXT(VkPhysicalDevice, VkSurfaceKHR,
                                                                               VkSurfaceCapabilities2EXT *pSurfaceCapabilities) {
    if (nullptr != pSurfaceCapabilities) {
        pSurfaceCapabilities->minImageCount = 7;
        pSurfaceCapabilities->maxImageCount = 12;
        pSurfaceCapabilities->maxImageArrayLayers = 365;
    }
    return VK_SUCCESS;
}

// Fake device extension support
VKAPI_ATTR void VKAPI_CALL wrap_vkTrimCommandPoolKHR(VkDevice, VkCommandPool, VkCommandPoolTrimFlags) {}

// Return an odd error so we can verify that this actually got called
VKAPI_ATTR VkResult VKAPI_CALL wrap_vkGetSwapchainStatusKHR(VkDevice, VkSwapchainKHR) { return VK_ERROR_NATIVE_WINDOW_IN_USE_KHR; }

// Debug utils & debug marker ext stubs
VKAPI_ATTR VkResult VKAPI_CALL wrap_vkDebugMarkerSetObjectTagEXT(VkDevice device, const VkDebugMarkerObjectTagInfoEXT *pTagInfo) {
    VkDebugMarkerObjectTagInfoEXT new_info = *pTagInfo;
    wrapped_dev_obj *dev;
    auto vk_dev = unwrap_device(device, &dev);
    if (pTagInfo && pTagInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT) {
        wrapped_phys_dev_obj *phys_dev;
        auto vk_phys_dev = unwrap_phys_dev((VkPhysicalDevice)(uintptr_t)(pTagInfo->object), &phys_dev);
        if (vk_phys_dev == VK_NULL_HANDLE) return VK_ERROR_DEVICE_LOST;
        new_info.object = (uint64_t)(uintptr_t)vk_phys_dev;
    }
    if (pTagInfo && pTagInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT) {
        // TODO
    }
    if (pTagInfo && pTagInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT) {
        wrapped_inst_obj *inst;
        auto instance = unwrap_instance((VkInstance)(uintptr_t)(pTagInfo->object), &inst);
        if (instance == VK_NULL_HANDLE) return VK_ERROR_DEVICE_LOST;
        new_info.object = (uint64_t)(uintptr_t)instance;
    }
    return dev->disp.DebugMarkerSetObjectTagEXT(vk_dev, &new_info);
}
VKAPI_ATTR VkResult VKAPI_CALL wrap_vkDebugMarkerSetObjectNameEXT(VkDevice device,
                                                                  const VkDebugMarkerObjectNameInfoEXT *pNameInfo) {
    VkDebugMarkerObjectNameInfoEXT new_info = *pNameInfo;
    wrapped_dev_obj *dev;
    auto vk_dev = unwrap_device(device, &dev);
    if (pNameInfo && pNameInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT) {
        wrapped_phys_dev_obj *phys_dev;
        auto vk_phys_dev = unwrap_phys_dev((VkPhysicalDevice)(uintptr_t)(pNameInfo->object), &phys_dev);
        if (vk_phys_dev == VK_NULL_HANDLE) return VK_ERROR_DEVICE_LOST;
        new_info.object = (uint64_t)(uintptr_t)vk_phys_dev;
    }
    if (pNameInfo && pNameInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT) {
        // TODO
    }
    if (pNameInfo && pNameInfo->objectType == VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT) {
        wrapped_inst_obj *inst;
        auto instance = unwrap_instance((VkInstance)(uintptr_t)(pNameInfo->object), &inst);
        if (instance == VK_NULL_HANDLE) return VK_ERROR_DEVICE_LOST;
        new_info.object = (uint64_t)(uintptr_t)instance;
    }
    return dev->disp.DebugMarkerSetObjectNameEXT(vk_dev, &new_info);
}
// Debug Marker functions that are not supported:
// vkCmdDebugMarkerBeginEXT
// vkCmdDebugMarkerEndEXT
// vkCmdDebugMarkerInsertEXT

VKAPI_ATTR VkResult VKAPI_CALL wrap_vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT *pNameInfo) {
    VkDebugUtilsObjectNameInfoEXT new_info = *pNameInfo;
    wrapped_dev_obj *dev;
    auto vk_dev = unwrap_device(device, &dev);
    if (pNameInfo && pNameInfo->objectType == VK_OBJECT_TYPE_PHYSICAL_DEVICE) {
        wrapped_phys_dev_obj *phys_dev;
        auto vk_phys_dev = unwrap_phys_dev((VkPhysicalDevice)(uintptr_t)(pNameInfo->objectHandle), &phys_dev);
        if (vk_phys_dev == VK_NULL_HANDLE) return VK_ERROR_DEVICE_LOST;
        new_info.objectHandle = (uint64_t)(uintptr_t)vk_phys_dev;
    }
    if (pNameInfo && pNameInfo->objectType == VK_OBJECT_TYPE_SURFACE_KHR) {
        // TODO
    }
    if (pNameInfo && pNameInfo->objectType == VK_OBJECT_TYPE_INSTANCE) {
        wrapped_inst_obj *inst;
        auto instance = unwrap_instance((VkInstance)(uintptr_t)(pNameInfo->objectHandle), &inst);
        if (instance == VK_NULL_HANDLE) return VK_ERROR_DEVICE_LOST;
        new_info.objectHandle = (uint64_t)(uintptr_t)instance;
    }
    return dev->disp.SetDebugUtilsObjectNameEXT(vk_dev, &new_info);
}
VKAPI_ATTR VkResult VKAPI_CALL wrap_vkSetDebugUtilsObjectTagEXT(VkDevice device, const VkDebugUtilsObjectTagInfoEXT *pTagInfo) {
    VkDebugUtilsObjectTagInfoEXT new_info = *pTagInfo;
    wrapped_dev_obj *dev;
    auto vk_dev = unwrap_device(device, &dev);
    if (pTagInfo && pTagInfo->objectType == VK_OBJECT_TYPE_PHYSICAL_DEVICE) {
        wrapped_phys_dev_obj *phys_dev;
        auto vk_phys_dev = unwrap_phys_dev((VkPhysicalDevice)(uintptr_t)(pTagInfo->objectHandle), &phys_dev);
        if (vk_phys_dev == VK_NULL_HANDLE) return VK_ERROR_DEVICE_LOST;
        new_info.objectHandle = (uint64_t)(uintptr_t)vk_phys_dev;
    }
    if (pTagInfo && pTagInfo->objectType == VK_OBJECT_TYPE_SURFACE_KHR) {
        // TODO
    }
    if (pTagInfo && pTagInfo->objectType == VK_OBJECT_TYPE_INSTANCE) {
        wrapped_inst_obj *inst;
        auto instance = unwrap_instance((VkInstance)(uintptr_t)(pTagInfo->objectHandle), &inst);
        if (instance == VK_NULL_HANDLE) return VK_ERROR_DEVICE_LOST;
        new_info.objectHandle = (uint64_t)(uintptr_t)instance;
    }
    return dev->disp.SetDebugUtilsObjectTagEXT(vk_dev, &new_info);
}
// Debug utils functions that are not supported
// vkQueueBeginDebugUtilsLabelEXT
// vkQueueEndDebugUtilsLabelEXT
// vkQueueInsertDebugUtilsLabelEXT
// vkCmdBeginDebugUtilsLabelEXT
// vkCmdEndDebugUtilsLabelEXT
// vkCmdInsertDebugUtilsLabelEXT

PFN_vkVoidFunction layer_intercept_device_proc(wrapped_dev_obj *dev, const char *name) {
    if (!name || name[0] != 'v' || name[1] != 'k') return NULL;

    name += 2;
    if (!strcmp(name, "CreateDevice")) return (PFN_vkVoidFunction)wrap_vkCreateDevice;
    if (!strcmp(name, "DestroyDevice")) return (PFN_vkVoidFunction)wrap_vkDestroyDevice;

    if (dev->maintanence_1_enabled && !strcmp(name, "TrimCommandPoolKHR")) return (PFN_vkVoidFunction)wrap_vkTrimCommandPoolKHR;
    if (dev->present_image_enabled && !strcmp(name, "GetSwapchainStatusKHR"))
        return (PFN_vkVoidFunction)wrap_vkGetSwapchainStatusKHR;

    if (dev->debug_marker_enabled && !strcmp(name, "DebugMarkerSetObjectTagEXT"))
        return (PFN_vkVoidFunction)wrap_vkDebugMarkerSetObjectTagEXT;
    if (dev->debug_marker_enabled && !strcmp(name, "DebugMarkerSetObjectNameEXT"))
        return (PFN_vkVoidFunction)wrap_vkDebugMarkerSetObjectNameEXT;
    if (dev->debug_utils_enabled && !strcmp(name, "SetDebugUtilsObjectNameEXT"))
        return (PFN_vkVoidFunction)wrap_vkSetDebugUtilsObjectNameEXT;
    if (dev->debug_utils_enabled && !strcmp(name, "SetDebugUtilsObjectTagEXT"))
        return (PFN_vkVoidFunction)wrap_vkSetDebugUtilsObjectTagEXT;

    return NULL;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL wrap_vkGetDeviceProcAddr(VkDevice device, const char *funcName) {
    PFN_vkVoidFunction addr;

    if (!strcmp("vkGetDeviceProcAddr", funcName)) {
        return (PFN_vkVoidFunction)wrap_vkGetDeviceProcAddr;
    }

    if (device == VK_NULL_HANDLE) {
        return NULL;
    }

    wrapped_dev_obj *dev;
    unwrap_device(device, &dev);

    addr = layer_intercept_device_proc(dev, funcName);
    if (addr) return addr;

    return dev->pfn_get_dev_proc_addr(dev->obj, funcName);
}

PFN_vkVoidFunction layer_intercept_instance_proc(wrapped_inst_obj *inst, const char *name) {
    if (!name || name[0] != 'v' || name[1] != 'k') return NULL;

    name += 2;
    if (!strcmp(name, "DestroyInstance")) return (PFN_vkVoidFunction)wrap_vkDestroyInstance;
    if (!strcmp(name, "CreateDevice")) return (PFN_vkVoidFunction)wrap_vkCreateDevice;
    if (!strcmp(name, "EnumeratePhysicalDevices")) return (PFN_vkVoidFunction)wrap_vkEnumeratePhysicalDevices;

    if (!strcmp(name, "EnumerateDeviceExtensionProperties")) return (PFN_vkVoidFunction)wrap_vkEnumerateDeviceExtensionProperties;

    if (!strcmp(name, "CreateDebugUtilsMessengerEXT")) return (PFN_vkVoidFunction)wrap_vkCreateDebugUtilsMessengerEXT;
    if (!strcmp(name, "DestroyDebugUtilsMessengerEXT")) return (PFN_vkVoidFunction)wrap_vkDestroyDebugUtilsMessengerEXT;

    if (!strcmp(name, "GetPhysicalDeviceProperties")) return (PFN_vkVoidFunction)vkGetPhysicalDeviceProperties;
    if (!strcmp(name, "GetPhysicalDeviceQueueFamilyProperties"))
        return (PFN_vkVoidFunction)wrap_vkGetPhysicalDeviceQueueFamilyProperties;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
    if (!strcmp(name, "CreateAndroidSurfaceKHR")) return (PFN_vkVoidFunction)wrap_vkCreateAndroidSurfaceKHR;
#endif  // VK_USE_PLATFORM_ANDROID_KHR

#if defined(VK_USE_PLATFORM_WIN32_KHR)
    if (!strcmp(name, "CreateWin32SurfaceKHR")) return (PFN_vkVoidFunction)wrap_vkCreateWin32SurfaceKHR;
#endif  // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
    if (!strcmp(name, "CreateWaylandSurfaceKHR")) return (PFN_vkVoidFunction)wrap_vkCreateWaylandSurfaceKHR;
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#if defined(VK_USE_PLATFORM_XCB_KHR)
    if (!strcmp(name, "CreateXcbSurfaceKHR")) return (PFN_vkVoidFunction)wrap_vkCreateXcbSurfaceKHR;
#endif  // VK_USE_PLATFORM_XCB_KHR

#if defined(VK_USE_PLATFORM_XLIB_KHR)
    if (!strcmp(name, "CreateXlibSurfaceKHR")) return (PFN_vkVoidFunction)wrap_vkCreateXlibSurfaceKHR;
#endif  // VK_USE_PLATFORM_XLIB_KHR

#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
    if (!strcmp(name, "CreateDirectFBSurfaceEXT")) return (PFN_vkVoidFunction)wrap_vkCreateDirectFBSurfaceEXT;
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT

#if defined(VK_USE_PLATFORM_MACOS_MVK)
    if (!strcmp(name, "CreateMacOSSurfaceMVK")) return (PFN_vkVoidFunction)wrap_vkCreateMacOSSurfaceMVK;
#endif  // VK_USE_PLATFORM_MACOS_MVK

#if defined(VK_USE_PLATFORM_IOS_MVK)
    if (!strcmp(name, "CreateIOSSurfaceMVK")) return (PFN_vkVoidFunction)wrap_vkCreateIOSSurfaceMVK;
#endif  // VK_USE_PLATFORM_IOS_MVK

#if defined(VK_USE_PLATFORM_GGP)
    if (!strcmp(name, "CreateStreamDescriptorSurfaceGGP")) return (PFN_vkVoidFunction)wrap_vkCreateStreamDescriptorSurfaceGGP;
#endif  // VK_USE_PLATFORM_GGP

#if defined(VK_USE_PLATFORM_METAL_EXT)
    if (!strcmp(name, "CreateMetalSurfaceEXT")) return (PFN_vkVoidFunction)wrap_vkCreateMetalSurfaceEXT;
#endif  // VK_USE_PLATFORM_METAL_EXT

#if defined(VK_USE_PLATFORM_SCREEN_QNX)
    if (!strcmp(name, "CreateScreenSurfaceQNX")) return (PFN_vkVoidFunction)wrap_vkCreateScreenSurfaceQNX;
#endif  // VK_USE_PLATFORM_SCREEN_QNX
    if (!strcmp(name, "DestroySurfaceKHR")) return (PFN_vkVoidFunction)wrap_vkDestroySurfaceKHR;

    if (inst->direct_display_enabled && !strcmp(name, "ReleaseDisplayEXT")) return (PFN_vkVoidFunction)wrap_vkReleaseDisplayEXT;
    if (inst->display_surf_counter_enabled && !strcmp(name, "GetPhysicalDeviceSurfaceCapabilities2EXT"))
        return (PFN_vkVoidFunction)wrap_vkGetPhysicalDeviceSurfaceCapabilities2EXT;

    // instance_proc needs to be able to query device commands even if the extension isn't enabled (because it isn't known at this
    // time)
    if (!strcmp(name, "TrimCommandPoolKHR")) return (PFN_vkVoidFunction)wrap_vkTrimCommandPoolKHR;
    if (!strcmp(name, "GetSwapchainStatusKHR")) return (PFN_vkVoidFunction)wrap_vkGetSwapchainStatusKHR;

    if (!strcmp(name, "DebugMarkerSetObjectTagEXT")) return (PFN_vkVoidFunction)wrap_vkDebugMarkerSetObjectTagEXT;
    if (!strcmp(name, "DebugMarkerSetObjectNameEXT")) return (PFN_vkVoidFunction)wrap_vkDebugMarkerSetObjectNameEXT;
    if (inst->debug_utils_enabled && !strcmp(name, "SetDebugUtilsObjectNameEXT"))
        return (PFN_vkVoidFunction)wrap_vkSetDebugUtilsObjectNameEXT;
    if (inst->debug_utils_enabled && !strcmp(name, "SetDebugUtilsObjectTagEXT"))
        return (PFN_vkVoidFunction)wrap_vkSetDebugUtilsObjectTagEXT;

    return NULL;
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL wrap_vkGetInstanceProcAddr(VkInstance instance, const char *funcName) {
    PFN_vkVoidFunction addr;

    if (!strcmp(funcName, "vkGetInstanceProcAddr")) return (PFN_vkVoidFunction)wrap_vkGetInstanceProcAddr;
    if (!strcmp(funcName, "vkCreateInstance")) return (PFN_vkVoidFunction)wrap_vkCreateInstance;

    if (instance == VK_NULL_HANDLE) {
        return NULL;
    }

    wrapped_inst_obj *inst;
    unwrap_instance(instance, &inst);

    addr = layer_intercept_instance_proc(inst, funcName);
    if (addr) return addr;

    VkLayerInstanceDispatchTable *pTable = &inst->layer_disp;

    if (pTable->GetInstanceProcAddr == NULL) return NULL;
    return pTable->GetInstanceProcAddr(inst->obj, funcName);
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL GetPhysicalDeviceProcAddr(VkInstance instance, const char *funcName) {
    assert(instance);

    wrapped_inst_obj *inst;
    unwrap_instance(instance, &inst);
    VkLayerInstanceDispatchTable *pTable = &inst->layer_disp;

    if (pTable->GetPhysicalDeviceProcAddr == NULL) return NULL;
    return pTable->GetPhysicalDeviceProcAddr(inst->obj, funcName);
}

}  // namespace wrap_objects

extern "C" {
// loader-layer interface v0, just wrappers since there is only a layer
VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(VkInstance instance, const char *funcName) {
    return wrap_objects::wrap_vkGetInstanceProcAddr(instance, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(VkDevice device, const char *funcName) {
    return wrap_objects::wrap_vkGetDeviceProcAddr(device, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char *, uint32_t *,
                                                                                      VkExtensionProperties *) {
    assert(0);  // TODO return wrap_objects::EnumerateInstanceExtensionProperties(pLayerName, pCount, pProperties);
    return VK_SUCCESS;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t *, VkLayerProperties *) {
    assert(0);  // TODO return wrap_objects::EnumerateInstanceLayerProperties(pCount, pProperties);
    return VK_SUCCESS;
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL
vkEnumerateDeviceExtensionProperties([[maybe_unused]] VkPhysicalDevice physicalDevice, const char *pLayerName, uint32_t *pCount,
                                     VkExtensionProperties *pProperties) {
    // the layer command handles VK_NULL_HANDLE just fine internally
    assert(physicalDevice == VK_NULL_HANDLE);
    return wrap_objects::wrap_vkEnumerateDeviceExtensionProperties(VK_NULL_HANDLE, pLayerName, pCount, pProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance,
                                                              const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator,
                                                              VkDebugUtilsMessengerEXT *pMessenger) {
    return wrap_objects::wrap_vkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
                                                           const VkAllocationCallbacks *pAllocator) {
    return wrap_objects::wrap_vkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

VKAPI_ATTR void VKAPI_CALL vkDestroySurfaceKHR(VkInstance instance, VkSurfaceKHR surface, const VkAllocationCallbacks *pAllocator) {
    return wrap_objects::wrap_vkDestroySurfaceKHR(instance, surface, pAllocator);
}

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
VKAPI_ATTR VkResult VKAPI_CALL test_vkCreateAndroidSurfaceKHR(VkInstance instance, const VkAndroidSurfaceCreateInfoKHR *pCreateInfo,
                                                              const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    return wrap_objects::wrap_vkCreateAndroidSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_WIN32_KHR)
VKAPI_ATTR VkResult VKAPI_CALL vkCreateWin32SurfaceKHR(VkInstance instance, const VkWin32SurfaceCreateInfoKHR *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    return wrap_objects::wrap_vkCreateWin32SurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_WIN32_KHR

#if defined(VK_USE_PLATFORM_WAYLAND_KHR)
VKAPI_ATTR VkResult VKAPI_CALL vkCreateWaylandSurfaceKHR(VkInstance instance, const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
                                                         const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    return wrap_objects::wrap_vkCreateWaylandSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_WAYLAND_KHR

#if defined(VK_USE_PLATFORM_XCB_KHR)
VKAPI_ATTR VkResult VKAPI_CALL vkCreateXcbSurfaceKHR(VkInstance instance, const VkXcbSurfaceCreateInfoKHR *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    return wrap_objects::wrap_vkCreateXcbSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_XCB_KHR

#if defined(VK_USE_PLATFORM_XLIB_KHR)
VKAPI_ATTR VkResult VKAPI_CALL vkCreateXlibSurfaceKHR(VkInstance instance, const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
                                                      const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    return wrap_objects::wrap_vkCreateXlibSurfaceKHR(instance, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_XLIB_KHR

#if defined(VK_USE_PLATFORM_DIRECTFB_EXT)
VKAPI_ATTR VkResult VKAPI_CALL vkCreateDirectFBSurfaceEXT(VkInstance instance, const VkDirectFBSurfaceCreateInfoEXT *pCreateInfo,
                                                          const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    return wrap_objects::wrap_vkCreateDirectFBSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_DIRECTFB_EXT

#if defined(VK_USE_PLATFORM_MACOS_MVK)
VKAPI_ATTR VkResult VKAPI_CALL vkCreateMacOSSurfaceMVK(VkInstance instance, const VkMacOSSurfaceCreateInfoMVK *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    return wrap_objects::wrap_vkCreateMacOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_MACOS_MVK

#if defined(VK_USE_PLATFORM_IOS_MVK)
VKAPI_ATTR VkResult VKAPI_CALL vkCreateIOSSurfaceMVK(VkInstance instance, const VkIOSSurfaceCreateInfoMVK *pCreateInfo,
                                                     const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    return wrap_objects::wrap_vkCreateIOSSurfaceMVK(instance, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_IOS_MVK

#if defined(VK_USE_PLATFORM_GGP)
VKAPI_ATTR VkResult VKAPI_CALL vkCreateStreamDescriptorSurfaceGGP(VkInstance instance,
                                                                  const VkStreamDescriptorSurfaceCreateInfoGGP *pCreateInfo,
                                                                  const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    return wrap_objects::wrap_vkCreateStreamDescriptorSurfaceGGP(instance, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_GGP

#if defined(VK_USE_PLATFORM_METAL_EXT)
VKAPI_ATTR VkResult VKAPI_CALL vkCreateMetalSurfaceEXT(VkInstance instance, const VkMetalSurfaceCreateInfoEXT *pCreateInfo,
                                                       const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    return wrap_objects::wrap_vkCreateMetalSurfaceEXT(instance, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_METAL_EXT

#if defined(VK_USE_PLATFORM_SCREEN_QNX)
VKAPI_ATTR VkResult VKAPI_CALL vkCreateScreenSurfaceQNX(VkInstance instance, const VkScreenSurfaceCreateInfoQNX *pCreateInfo,
                                                        const VkAllocationCallbacks *pAllocator, VkSurfaceKHR *pSurface) {
    return wrap_objects::wrap_vkCreateScreenSurfaceQNX(instance, pCreateInfo, pAllocator, pSurface);
}
#endif  // VK_USE_PLATFORM_SCREEN_QNX

VK_LAYER_EXPORT VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vk_layerGetPhysicalDeviceProcAddr(VkInstance instance,
                                                                                           const char *funcName) {
    return wrap_objects::GetPhysicalDeviceProcAddr(instance, funcName);
}

VK_LAYER_EXPORT VKAPI_ATTR VkResult VKAPI_CALL vkNegotiateLoaderLayerInterfaceVersion(VkNegotiateLayerInterface *pVersionStruct) {
    assert(pVersionStruct != NULL);
    assert(pVersionStruct->sType == LAYER_NEGOTIATE_INTERFACE_STRUCT);

    // Fill in the function pointers if our version is at least capable of having the structure contain them.
    if (pVersionStruct->loaderLayerInterfaceVersion >= 2) {
        pVersionStruct->pfnGetInstanceProcAddr = wrap_objects::wrap_vkGetInstanceProcAddr;
        pVersionStruct->pfnGetDeviceProcAddr = wrap_objects::wrap_vkGetDeviceProcAddr;
        pVersionStruct->pfnGetPhysicalDeviceProcAddr = vk_layerGetPhysicalDeviceProcAddr;
    }

    if (pVersionStruct->loaderLayerInterfaceVersion < CURRENT_LOADER_LAYER_INTERFACE_VERSION) {
        wrap_objects::loader_layer_if_version = pVersionStruct->loaderLayerInterfaceVersion;
    } else if (pVersionStruct->loaderLayerInterfaceVersion > CURRENT_LOADER_LAYER_INTERFACE_VERSION) {
        pVersionStruct->loaderLayerInterfaceVersion = CURRENT_LOADER_LAYER_INTERFACE_VERSION;
    }

    return VK_SUCCESS;
}
}
