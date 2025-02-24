/*
 * Copyright (c) 2015-2024 The Khronos Group Inc.
 * Copyright (c) 2015-2024 Valve Corporation
 * Copyright (c) 2015-2024 LunarG, Inc.
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
 */
#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vk_enum_string_helper.h>
#include <vulkan/utility/vk_struct_helper.hpp>

// GTest and Xlib collide due to redefinitions of "None" and "Bool"
#ifdef VK_USE_PLATFORM_XLIB_KHR
#pragma push_macro("None")
#pragma push_macro("Bool")
#undef None
#undef Bool
#endif

#include <gtest/gtest.h>

// Redefine Xlib definitions
#ifdef VK_USE_PLATFORM_XLIB_KHR
#pragma pop_macro("Bool")
#pragma pop_macro("None")
#endif

// GTest has a GTEST_SKIP macro, but the test can't be actually "skipped" unless you are at the top function in the test.
// The macro will call the function and then call 'return' if a skip has been called inside
#define RETURN_IF_SKIP(function) \
    function;                    \
    if (::testing::Test::IsSkipped()) return;

// Stream operator for VkResult so GTEST will print out error codes as strings (automatically)
inline std::ostream& operator<<(std::ostream& os, const VkResult& result) { return os << string_VkResult(result); }
