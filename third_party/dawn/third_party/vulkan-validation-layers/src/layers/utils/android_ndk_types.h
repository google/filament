/* Copyright (c) 2018-2021 The Khronos Group Inc.
 * Copyright (c) 2018-2023 Valve Corporation
 * Copyright (c) 2018-2023 LunarG, Inc.
 * Copyright (C) 2018-2021 Google Inc.
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

// Overview of Android NDK headers:
//
// <android/hardware_buffer.h>
//    - All enums referenced by VK_ANDROID_external_memory_android_hardware_buffer
// <android/api-level.h>
//     - __ANDROID_API_
//     - The version of android we are compiling for
// <android/ndk-version.h>
//     - __NDK_MAJOR__
//     - The NDK version we are compiling with

#pragma once

#if defined(__ANDROID__) && !defined(VK_USE_PLATFORM_ANDROID_KHR)
#error "VK_USE_PLATFORM_ANDROID_KHR not defined for Android build!"
#endif

// Everyone should be able to include this file and ignore it if not building for Android
#if defined(VK_USE_PLATFORM_ANDROID_KHR)

#include <android/api-level.h>

#include <android/ndk-version.h>

#include <android/hardware_buffer.h>

#endif  // VK_USE_PLATFORM_ANDROID_KHR
