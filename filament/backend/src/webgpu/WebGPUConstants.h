/*
 * Copyright (C) 2025 The Android Open Source Project
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

#ifndef TNT_FILAMENT_BACKEND_WEBGPUCONSTANTS_H
#define TNT_FILAMENT_BACKEND_WEBGPUCONSTANTS_H

#include <utils/Logger.h>

#include <cstdint>

// WebGPU requires that the source buffer of a writeBuffer call has a size that is a multiple of 4.
constexpr size_t FILAMENT_WEBGPU_BUFFER_SIZE_MODULUS = 4;

// WebGPU requires the offset of GPUBuffer:getMappedRange() to be a multiple of 8.
constexpr size_t FILAMENT_WEBGPU_MAPPED_RANGE_OFFSET_MODULUS = 8;

// FWGPU is short for Filament WebGPU

// turn on runtime validation, namely for debugging, that would normally not run (for release)
#define FWGPU_DEBUG_VALIDATION     0x00000001
// print/log system component details to the console, e.g. about the
// instance, surface, adapter, device, etc.
#define FWGPU_PRINT_SYSTEM         0x00000002

// Set this to enable logging "only" to one output stream. This is useful in the case where we want
// to debug with print statements and want ordered logging (e.g LOG(INFO) and LOG(ERROR) will not
// appear in order of calls).
#define FWGPU_DEBUG_FORCE_LOG_TO_I 0x00000004

// Set this to enable logging related to the WebGPU backend's update3DImage function, which would
// otherwise spam the logs.
#define FWGPU_DEBUG_UPDATE_IMAGE 0x00000008

// Set this to enable logging related to the WebGPU backend's WebGPUBlitter functionality,
// which would otherwise spam the logs.
#define FWGPU_DEBUG_BLIT 0x00000010

// Set this to enable logging relating to bind groups (bind group layouts, bind groups themselves,
// textures, samplers, and buffers,...), which would otherwise spam the logs.
#define FWGPU_DEBUG_BIND_GROUPS 0x00000020

// Enables Android systrace
#define FWGPU_DEBUG_SYSTRACE 0x00000040

// Enable a minimal set of traces to assess the performance of the backend.
// All other debug features must be disabled.
#define FWGPU_DEBUG_PROFILING 0x00000080

// Useful default combinations
#define FWGPU_DEBUG_EVERYTHING (0xFFFFFFFF & ~FWGPU_DEBUG_PROFILING)
#define FWGPU_DEBUG_PERFORMANCE FWGPU_DEBUG_SYSTRACE

#if defined(FILAMENT_BACKEND_DEBUG_FLAG)
    #define FWGPU_DEBUG_FORWARDED_FLAG (FILAMENT_BACKEND_DEBUG_FLAG & FWGPU_DEBUG_EVERYTHING)
#else
    #define FWGPU_DEBUG_FORWARDED_FLAG 0
#endif

#ifndef NDEBUG
    #define FWGPU_DEBUG_FLAGS (FWGPU_DEBUG_PERFORMANCE | FWGPU_DEBUG_FORWARDED_FLAG)
#else
    #define FWGPU_DEBUG_FLAGS FWGPU_DEBUG_SYSTRACE
#endif

// Override the debug flags if we are forcing profiling mode
#if defined(FILAMENT_FORCE_PROFILING_MODE)
#undef FWGPU_DEBUG_FLAGS
#define FWGPU_DEBUG_FLAGS (FWGPU_DEBUG_PROFILING)
#endif

#define FWGPU_ENABLED(flags) (((FWGPU_DEBUG_FLAGS) & (flags)) == (flags))

#if FWGPU_ENABLED(FWGPU_DEBUG_PROFILING) && FWGPU_DEBUG_FLAGS != FWGPU_DEBUG_PROFILING
#error PROFILING is exclusive; all other debug features must be disabled.
#endif

#if FWGPU_DEBUG_FLAGS == FWGPU_DEBUG_PROFILING

    #ifndef NDEBUG
        #error PROFILING is meaningless in DEBUG mode.
    #endif

    #define FWGPU_SYSTRACE_CONTEXT()
    #define FWGPU_SYSTRACE_START(marker)
    #define FWGPU_SYSTRACE_END()
    #define FWGPU_SYSTRACE_SCOPE()
    #define FWGPU_PROFILE_MARKER(marker)  PROFILE_SCOPE(marker)

#elif FWGPU_ENABLED(FWGPU_DEBUG_SYSTRACE)

    #include <private/utils/Tracing.h>

    #define FWGPU_SYSTRACE_CONTEXT()      FILAMENT_TRACING_CONTEXT(FILAMENT_TRACING_CATEGORY_FILAMENT)
    #define FWGPU_SYSTRACE_START(marker)  FILAMENT_TRACING_NAME_BEGIN(FILAMENT_TRACING_CATEGORY_FILAMENT, marker)
    #define FWGPU_SYSTRACE_END()          FILAMENT_TRACING_NAME_END(FILAMENT_TRACING_CATEGORY_FILAMENT)
    #define FWGPU_SYSTRACE_SCOPE()        FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT)
    #define FWGPU_PROFILE_MARKER(marker)  FILAMENT_TRACING_CALL(FILAMENT_TRACING_CATEGORY_FILAMENT)

#else
    #define FWGPU_SYSTRACE_CONTEXT()
    #define FWGPU_SYSTRACE_START(marker)
    #define FWGPU_SYSTRACE_END()
    #define FWGPU_SYSTRACE_SCOPE()
    #define FWGPU_PROFILE_MARKER(marker)
#endif

#if FWGPU_ENABLED(FWGPU_DEBUG_FORCE_LOG_TO_I)
    #define FWGPU_LOGI LOG(INFO)
    #define FWGPU_LOGD FWGPU_LOGI
    #define FWGPU_LOGE FWGPU_LOGI
    #define FWGPU_LOGW FWGPU_LOGI
#else
    #define FWGPU_LOGE LOG(ERROR)
    #define FWGPU_LOGW LOG(WARNING)
    #define FWGPU_LOGD DLOG(INFO)
    #define FWGPU_LOGI LOG(INFO)
#endif

constexpr uint64_t FILAMENT_WEBGPU_REQUEST_ADAPTER_TIMEOUT_NANOSECONDS =
        /* milliseconds */ 1000u * /* converted to ns */ 1000000u;

constexpr uint64_t FILAMENT_WEBGPU_REQUEST_DEVICE_TIMEOUT_NANOSECONDS =
        /* milliseconds */ 1000u * /* converted to ns */ 1000000u;

constexpr uint64_t FILAMENT_WEBGPU_SHADER_COMPILATION_TIMEOUT_NANOSECONDS =
        /* milliseconds */ 1000u * /* converted to ns */ 1000000u;

// if a render pipeline is not used in this number of consecutive frames,
// then expire/release it from the cache.
// A smaller number means more frequent pipeline creation events, taking more time.
// A larger number means more pipelines stored, taking more memory.
constexpr uint64_t FILAMENT_WEBGPU_RENDER_PIPELINE_EXPIRATION_IN_FRAME_COUNT = 45;

// if a pipeline layout is not used in this number of consecutive frames,
// then expire/release it from the cache.
// A smaller number means more frequent pipeline layout creation events, taking more time.
// A larger number means more layouts stored, taking more memory.
constexpr uint64_t FILAMENT_WEBGPU_PIPELINE_LAYOUT_EXPIRATION_IN_FRAME_COUNT = 90;

#endif// TNT_FILAMENT_BACKEND_WEBGPUCONSTANTS_H
