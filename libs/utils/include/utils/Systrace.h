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

#ifndef TNT_UTILS_SYSTRACE_H
#define TNT_UTILS_SYSTRACE_H

#define SYSTRACE_TAG_DISABLED       (0)
#define SYSTRACE_TAG_FILAMENT       (2) // don't change used in makefiles
#define SYSTRACE_TAG_JOBSYSTEM      (3)
#define SYSTRACE_TAG_GLTFIO         (4)

/*
 * The SYSTRACE_ macros use SYSTRACE_TAG as a category, which must be defined
 * before this file is included.
 */

#ifndef SYSTRACE_TAG
#   error SYSTRACE_TAG must be set to SYSTRACE_TAG_{DISABLED|FILAMENT|JOBSYSTEM}
#endif

// Systrace on Apple platforms is fragile and adds overhead, should only be enabled in dev builds.
#ifndef FILAMENT_APPLE_SYSTRACE
#   define FILAMENT_APPLE_SYSTRACE 0
#endif

#if defined(__ANDROID__)
#include <utils/android/Systrace.h>
#elif defined(__APPLE__) && FILAMENT_APPLE_SYSTRACE
#include <utils/darwin/Systrace.h>
#else

#define SYSTRACE_ENABLE()
#define SYSTRACE_CONTEXT()
#define SYSTRACE_NAME(name)
#define SYSTRACE_FRAME_ID(frame)
#define SYSTRACE_NAME_BEGIN(name)
#define SYSTRACE_NAME_END()
#define SYSTRACE_CALL()
#define SYSTRACE_ASYNC_BEGIN(name, cookie)
#define SYSTRACE_ASYNC_END(name, cookie)
#define SYSTRACE_VALUE32(name, val)
#define SYSTRACE_VALUE64(name, val)

#endif

#endif // TNT_UTILS_SYSTRACE_H
