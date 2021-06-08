/*
 * Copyright (C) 2021 The Android Open Source Project
 *
* Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TNT_FILAMENT_DRIVER_VULKANCONSTANTS_H
#define TNT_FILAMENT_DRIVER_VULKANCONSTANTS_H

#define FILAMENT_VULKAN_VERBOSE 0
#define FILAMENT_VULKAN_DUMP_API 0

// In debug builds, we enable validation layers and set up a debug callback.
//
// To enable validation layers in Android, also be sure to set the jniLibs property in the gradle
// file for your app by adding the following lines into the "android" section. This copies the
// appropriate libraries from the NDK to the device. This makes the aar much larger, so it should be
// avoided in release builds.
//
// sourceSets.main.jniLibs {
//   srcDirs = ["${android.ndkDirectory}/sources/third_party/vulkan/src/build-android/jniLibs"]
// }
//
#if defined(NDEBUG)
#define VK_ENABLE_VALIDATION 0
#else
#define VK_ENABLE_VALIDATION 1
#endif

#define VK_REPORT_STALLS 0

// All vkCreate* functions take an optional allocator. For now we select the default allocator by
// passing in a null pointer, and we highlight the argument by using the VKALLOC constant.
constexpr struct VkAllocationCallbacks* VKALLOC = nullptr;

// At the time of this writing, our copy of MoltenVK supports Vulkan 1.0 only.
constexpr static const int VK_REQUIRED_VERSION_MAJOR = 1;
constexpr static const int VK_REQUIRED_VERSION_MINOR = 0;

// Maximum number of VkCommandBuffer handles managed simultaneously by VulkanCommands.
//
// This includes the "current" command buffer that is being written into, as well as any command
// buffers that have been submitted but have not yet finished rendering. Note that Filament can
// issue multiple commit calls in a single frame, and that we use a triple buffered swap chain on
// some platforms.
constexpr static const int VK_MAX_COMMAND_BUFFERS = 10;

// Number of command buffer submissions that should occur before an unused pipeline is removed
// from the cache.
//
// If this number is low, VkPipeline construction will occur frequently, which can
// be extremely slow. If this number is high, the memory footprint will be large.
constexpr static const int VK_MAX_PIPELINE_AGE = 10;

// VulkanPipelineCache does not track which command buffers contain references to which pipelines,
// instead it simply waits for at least VK_MAX_COMMAND_BUFFERS submissions to occur before
// destroying any unused pipeline object.
static_assert(VK_MAX_PIPELINE_AGE >= VK_MAX_COMMAND_BUFFERS);

#endif
