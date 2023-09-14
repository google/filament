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

#ifndef TNT_FILAMENT_BACKEND_VULKANCONSTANTS_H
#define TNT_FILAMENT_BACKEND_VULKANCONSTANTS_H

#include <stdint.h>

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
// If gradle claims that your NDK is not installed, try checking what versions you have with
// `ls $ANDROID_HOME/ndk` then direct gradle by adding something like this to the "android" section:
//
//     ndkVersion "23.1.7779620"
//
// Also consider changing the root `gradle.properties` to point to a debug build, although this is
// not required for validation.

// FVK is short for Filament Vulkan
#define FVK_DEBUG_SYSTRACE                0x00000001
#define FVK_DEBUG_GROUP_MARKERS           0x00000002
#define FVK_DEBUG_TEXTURE                 0x00000004
#define FVK_DEBUG_LAYOUT_TRANSITION       0x00000008
#define FVK_DEBUG_COMMAND_BUFFER          0x00000010
#define FVK_DEBUG_DUMP_API                0x00000020
#define FVK_DEBUG_VALIDATION              0x00000040
#define FVK_DEBUG_PRINT_GROUP_MARKERS     0x00000080
#define FVK_DEBUG_BLIT_FORMAT             0x00000100
#define FVK_DEBUG_BLITTER                 0x00000200
#define FVK_DEBUG_FBO_CACHE               0x00000400
#define FVK_DEBUG_SHADER_MODULE           0x00000800
#define FVK_DEBUG_READ_PIXELS             0x00001000
#define FVK_DEBUG_PIPELINE_CACHE          0x00002000
#define FVK_DEBUG_ALLOCATION              0x00004000

// Usefaul default combinations
#define FVK_DEBUG_EVERYTHING              0xFFFFFFFF
#define FVK_DEBUG_PERFORMANCE     \
    FVK_DEBUG_SYSTRACE |          \
    FVK_DEBUG_GROUP_MARKERS

#define FVK_DEBUG_CORRECTNESS     \
    FVK_DEBUG_VALIDATION |        \
    FVK_DEBUG_SHADER_MODULE |     \
    FVK_DEBUG_TEXTURE |           \
    FVK_DEBUG_LAYOUT_TRANSITION

#define FVK_DEBUG_RENDER_PASSES   \
    FVK_DEBUG_GROUP_MARKERS |     \
    FVK_DEBUG_PRINT_GROUP_MARKERS

#ifndef NDEBUG
#define FVK_DEBUG_FLAGS FVK_DEBUG_PERFORMANCE
#else
#define FVK_DEBUG_FLAGS 0
#endif

#define FVK_ENABLED(flags) ((FVK_DEBUG_FLAGS) & (flags))
#define FVK_ENABLED_BOOL(flags) ((bool) FVK_ENABLED(flags))

// Ensure dependencies are met between debug options
#if FVK_ENABLED(FVK_DEBUG_PRINT_GROUP_MARKERS)
static_assert(FVK_ENABLED(FVK_DEBUG_GROUP_MARKERS));
#endif

// end dependcy checks

#if FVK_ENABLED(FVK_DEBUG_SYSTRACE)

#include <utils/Systrace.h>

#define FVK_SYSTRACE_CONTEXT()      SYSTRACE_CONTEXT()
#define FVK_SYSTRACE_START(marker)  SYSTRACE_NAME_BEGIN(marker)
#define FVK_SYSTRACE_END()          SYSTRACE_NAME_END()
#else
#define FVK_SYSTRACE_CONTEXT()
#define FVK_SYSTRACE_START(marker)
#define FVK_SYSTRACE_END()
#endif

#ifndef FVK_HANDLE_ARENA_SIZE_IN_MB
#define FVK_HANDLE_ARENA_SIZE_IN_MB 8
#endif

// All vkCreate* functions take an optional allocator. For now we select the default allocator by
// passing in a null pointer, and we highlight the argument by using the VKALLOC constant.
constexpr struct VkAllocationCallbacks* VKALLOC = nullptr;

constexpr static const int FVK_REQUIRED_VERSION_MAJOR = 1;
constexpr static const int FVK_REQUIRED_VERSION_MINOR = 1;

// Maximum number of VkCommandBuffer handles managed simultaneously by VulkanCommands.
//
// This includes the "current" command buffer that is being written into, as well as any command
// buffers that have been submitted but have not yet finished rendering. Note that Filament can
// issue multiple commit calls in a single frame, and that we use a triple buffered swap chain on
// some platforms.
constexpr static const int FVK_MAX_COMMAND_BUFFERS = 10;

// Number of command buffer submissions that should occur before an unused pipeline is removed
// from the cache.
//
// If this number is low, VkPipeline construction will occur frequently, which can
// be extremely slow. If this number is high, the memory footprint will be large.
constexpr static const int FVK_MAX_PIPELINE_AGE = 10;

// VulkanPipelineCache does not track which command buffers contain references to which pipelines,
// instead it simply waits for at least FVK_MAX_COMMAND_BUFFERS submissions to occur before
// destroying any unused pipeline object.
static_assert(FVK_MAX_PIPELINE_AGE >= FVK_MAX_COMMAND_BUFFERS);

#endif
