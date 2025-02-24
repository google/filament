/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
 * Copyright (c) 2015-2024 Google, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 */

#include "../framework/layer_validation_tests.h"
#include "../framework/ray_tracing_helper_nv.h"

class PositiveRayTracingPipelineNV : public RayTracingTest {};

TEST_F(PositiveRayTracingPipelineNV, BasicUsage) {
    TEST_DESCRIPTION("Test VK_NV_ray_tracing.");
    RETURN_IF_SKIP(NvInitFrameworkForRayTracingTest());

    VkPhysicalDeviceRayTracingPropertiesNV rtnv_props = vku::InitStructHelper();
    GetPhysicalDeviceProperties2(rtnv_props);
    if (rtnv_props.maxDescriptorSetAccelerationStructures < 1) {
        GTEST_SKIP() << "VkPhysicalDeviceRayTracingPropertiesNV::maxDescriptorSetAccelerationStructures < 1";
    }

    RETURN_IF_SKIP(InitState());

    auto ignore_update = [](nv::rt::RayTracingPipelineHelper &helper) {};
    nv::rt::RayTracingPipelineHelper::OneshotPositiveTest(*this, ignore_update);
}
