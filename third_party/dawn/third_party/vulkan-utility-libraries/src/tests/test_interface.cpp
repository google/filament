// Copyright 2023 The Khronos Group Inc.
// Copyright 2023 Valve Corporation
// Copyright 2023 LunarG, Inc.
//
// SPDX-License-Identifier: Apache-2.0
//

#include <gtest/gtest.h>

#include <vulkan/utility/vk_dispatch_table.h>

// Only exists so that local_vkGetDeviceProcAddr can return a 'real' function pointer
inline VKAPI_ATTR void empty_func() {}

inline VKAPI_ATTR PFN_vkVoidFunction local_vkGetInstanceProcAddr(VkInstance instance, const char *pName) {
    if (instance == VK_NULL_HANDLE) {
        return NULL;
    }

    if (strcmp(pName, "vkGetInstanceProcAddr")) {
        return reinterpret_cast<PFN_vkVoidFunction>(&local_vkGetInstanceProcAddr);
    }

    return reinterpret_cast<PFN_vkVoidFunction>(&empty_func);
}

inline VKAPI_ATTR PFN_vkVoidFunction local_vkGetDeviceProcAddr(VkDevice device, const char *pName) {
    if (device == VK_NULL_HANDLE) {
        return NULL;
    }

    if (strcmp(pName, "vkGetDeviceProcAddr")) {
        return reinterpret_cast<PFN_vkVoidFunction>(&local_vkGetDeviceProcAddr);
    }

    return reinterpret_cast<PFN_vkVoidFunction>(&empty_func);
}

TEST(test_vk_dispatch_table, cpp_interface) {
    VkuDeviceDispatchTable device_dispatch_table{};
    VkuInstanceDispatchTable instance_dispatch_table{};

    VkInstance instance{};

    vkuInitInstanceDispatchTable(instance, &instance_dispatch_table, local_vkGetInstanceProcAddr);

    ASSERT_EQ(reinterpret_cast<PFN_vkVoidFunction>(instance_dispatch_table.GetInstanceProcAddr),
              reinterpret_cast<PFN_vkVoidFunction>(local_vkGetInstanceProcAddr));

    VkDevice device{};

    vkuInitDeviceDispatchTable(device, &device_dispatch_table, local_vkGetDeviceProcAddr);

    ASSERT_EQ(reinterpret_cast<PFN_vkVoidFunction>(device_dispatch_table.GetDeviceProcAddr),
              reinterpret_cast<PFN_vkVoidFunction>(local_vkGetDeviceProcAddr));
}
