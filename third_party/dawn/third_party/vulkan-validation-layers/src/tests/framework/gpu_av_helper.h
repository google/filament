/*
 * Copyright (c) 2023 The Khronos Group Inc.
 * Copyright (c) 2023 Valve Corporation
 * Copyright (c) 2023 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#pragma once
// This checks any requirements needed for GPU-AV are met otherwise devices not meeting them will "fail" the tests
template <typename Test>
bool CanEnableGpuAV(Test &test) {
    // Check version first before trying to call GetPhysicalDeviceFeatures2
    if (test.DeviceValidationVersion() < VK_API_VERSION_1_1) {
        printf("At least Vulkan version 1.1 is required for GPU-AV\n");
        return false;
    }
    VkPhysicalDeviceFeatures2 features2 = vku::InitStructHelper();
    test.GetPhysicalDeviceFeatures2(features2);
    if (!features2.features.fragmentStoresAndAtomics || !features2.features.vertexPipelineStoresAndAtomics) {
        printf("fragmentStoresAndAtomics and vertexPipelineStoresAndAtomics are required for GPU-AV\n");
        return false;
    }
    else if (test.IsPlatformMockICD()) {
        printf("Test not supported by MockICD, GPU-Assisted validation test requires a driver that can draw\n");
        return false;
    }
    return true;
}
