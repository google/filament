// Copyright 2023 The Khronos Group Inc.
// Copyright 2023 Valve Corporation
// Copyright 2023 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
#include <vulkan/utility/vk_dispatch_table.h>

PFN_vkVoidFunction VKAPI_PTR local_vkGetInstanceProcAddr(VkInstance instance, const char *pName) {
    (void)instance;
    (void)pName;
    return NULL;
}

PFN_vkVoidFunction VKAPI_PTR local_vkGetDeviceProcAddr(VkDevice device, const char *pName) {
    (void)device;
    (void)pName;
    return NULL;
}

void vk_dispatch_table() {
    VkuDeviceDispatchTable device_dispatch_table;
    VkuInstanceDispatchTable instance_dispatch_table;

    VkInstance instance = VK_NULL_HANDLE;

    vkuInitInstanceDispatchTable(instance, &instance_dispatch_table, local_vkGetInstanceProcAddr);

    VkDevice device = VK_NULL_HANDLE;

    vkuInitDeviceDispatchTable(device, &device_dispatch_table, local_vkGetDeviceProcAddr);
}
