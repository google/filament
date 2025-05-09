/*
 * Copyright (C) 2016 The Android Open Source Project
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

#ifndef TNT_UTILS_FILAMENT_TRACING_H
#define TNT_UTILS_FILAMENT_TRACING_H

#ifndef FILAMENT_TRACING_ENABLED
#    define FILAMENT_TRACING_ENABLED true
#endif

#define FILAMENT_TRACING_CATEGORY_FILAMENT        "filament/filament"
#define FILAMENT_TRACING_CATEGORY_JOBSYSTEM       "filament/jobsystem"
#define FILAMENT_TRACING_CATEGORY_GLTFIO          "filament/gltfio"

// Systrace on Apple platforms is fragile and adds overhead, should only be enabled in dev builds.
#ifndef FILAMENT_APPLE_SYSTRACE
#   define FILAMENT_APPLE_SYSTRACE 0
#endif

#if defined(__ANDROID__)
#    include <private/utils/android/Tracing.h>
#elif defined(__APPLE__) && FILAMENT_APPLE_SYSTRACE
#    include <private/utils/darwin/Tracing.h>
#else

#define FILAMENT_TRACING_ENABLE(category)
#define FILAMENT_TRACING_CONTEXT(category)
#define FILAMENT_TRACING_NAME(category, name)
#define FILAMENT_TRACING_FRAME_ID(category, frame)
#define FILAMENT_TRACING_NAME_BEGIN(category, name)
#define FILAMENT_TRACING_NAME_END(category)
#define FILAMENT_TRACING_CALL(category)
#define FILAMENT_TRACING_ASYNC_BEGIN(category, name, cookie)
#define FILAMENT_TRACING_ASYNC_END(category, name, cookie)
#define FILAMENT_TRACING_VALUE(category, name, val)

#endif

#endif // TNT_UTILS_FILAMENT_TRACING_H
