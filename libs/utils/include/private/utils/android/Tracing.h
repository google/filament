/*
* Copyright (C) 2023 The Android Open Source Project
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

#ifndef TNT_UTILS_ANDROID_FILAMENT_TRACING_H
#define TNT_UTILS_ANDROID_FILAMENT_TRACING_H

#include <stdint.h>

#if FILAMENT_TRACING_ENABLED == true && !defined(FILAMENT_TRACING_USES_SYSTRACE)
// Default to Perfetto if Systrace is not explicitly requested
#   define FILAMENT_TRACING_USES_PERFETTO
#endif

#if FILAMENT_TRACING_ENABLED == false

#   define FILAMENT_TRACING_ENABLE(category)
#   define FILAMENT_TRACING_CONTEXT(category)
#   define FILAMENT_TRACING_NAME(category, name)
#   define FILAMENT_TRACING_FRAME_ID(category, frame)
#   define FILAMENT_TRACING_NAME_BEGIN(category, name)
#   define FILAMENT_TRACING_NAME_END(category)
#   define FILAMENT_TRACING_CALL(category)
#   define FILAMENT_TRACING_ASYNC_BEGIN(category, name, cookie)
#   define FILAMENT_TRACING_ASYNC_END(category, name, cookie)
#   define FILAMENT_TRACING_VALUE(category, name, val)

#elif defined(FILAMENT_TRACING_USES_SYSTRACE)

#   include "utils/Systrace.h"
#   include "utils/android/Systrace.h"

#   define FILAMENT_TRACING_ENABLE(category) SYSTRACE_ENABLE()
#   define FILAMENT_TRACING_CONTEXT(category) SYSTRACE_CONTEXT()
#   define FILAMENT_TRACING_NAME(category, name) SYSTRACE_NAME(name)
#   define FILAMENT_TRACING_FRAME_ID(category, frame) SYSTRACE_FRAME_ID(frame)
#   define FILAMENT_TRACING_NAME_BEGIN(category, name) SYSTRACE_NAME_BEGIN(name)
#   define FILAMENT_TRACING_NAME_END(category) SYSTRACE_NAME_END()
#   define FILAMENT_TRACING_CALL(category) SYSTRACE_CALL()
#   define FILAMENT_TRACING_ASYNC_BEGIN(category, name, cookie) SYSTRACE_ASYNC_BEGIN(name, cookie)
#   define FILAMENT_TRACING_ASYNC_END(category, name, cookie) SYSTRACE_ASYNC_END(name, cookie)
#   define FILAMENT_TRACING_VALUE(category, name, val) SYSTRACE_VALUE64(name, val)

#elif defined(FILAMENT_TRACING_USES_PERFETTO)

#include <perfetto/perfetto.h>

PERFETTO_DEFINE_CATEGORIES_IN_NAMESPACE(tracing,
        perfetto::Category(FILAMENT_TRACING_CATEGORY_FILAMENT),
        perfetto::Category(FILAMENT_TRACING_CATEGORY_JOBSYSTEM),
        perfetto::Category(FILAMENT_TRACING_CATEGORY_GLTFIO));

PERFETTO_USE_CATEGORIES_FROM_NAMESPACE(tracing);

#   define FILAMENT_TRACING_ENABLE(category)
#   define FILAMENT_TRACING_CONTEXT(category)

#   define FILAMENT_TRACING_CALL(category) \
       auto constexpr FILAMENT_FILAMENT_TRACING_FUNCTION = perfetto::StaticString(__FUNCTION__); \
       TRACE_EVENT(category, FILAMENT_FILAMENT_TRACING_FUNCTION)

#   define FILAMENT_TRACING_NAME(category, name) TRACE_EVENT(category, nullptr,               \
           [&](perfetto::EventContext ctx) {                               \
               ctx.event()->set_name(name);                                \
           })

#   define FILAMENT_TRACING_NAME_BEGIN(category, name) TRACE_EVENT_BEGIN(category, nullptr,    \
           [&](perfetto::EventContext ctx) {                                \
               ctx.event()->set_name(name);                                 \
           })

#   define FILAMENT_TRACING_NAME_END(category) TRACE_EVENT_END(category)

#   define FILAMENT_TRACING_ASYNC_BEGIN(category, name, cookie) \
           TRACE_EVENT_BEGIN(category, name, perfetto::Track(cookie))

#   define FILAMENT_TRACING_ASYNC_END(category, name, cookie) \
           TRACE_EVENT_END(category, perfetto::Track(cookie))

#   define FILAMENT_TRACING_FRAME_ID(category, frame) \
           TRACE_EVENT_INSTANT(category, "frame", "id", frame)

#   define FILAMENT_TRACING_VALUE(category, name, val) \
        TRACE_COUNTER(category, name, val)

#else

#   error The tracing mode is not properly defined

#endif // FILAMENT_TRACING_ENABLED

#endif // TNT_UTILS_ANDROID_FILAMENT_TRACING_H
