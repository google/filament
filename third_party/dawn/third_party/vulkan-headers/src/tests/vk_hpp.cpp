/*
 * Copyright 2025 The Khronos Group Inc.
 * Copyright 2025 Valve Corporation
 * Copyright 2025 LunarG, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <vulkan/vulkan.hpp>

int test_version()
{
    return static_cast<int>(vk::makeApiVersion(1, 0, 0, 0));
}