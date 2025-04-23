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

#ifndef TNT_UTILS_ANDROID_SYSTRACE_H
#define TNT_UTILS_ANDROID_SYSTRACE_H

#include <perfetto/perfetto.h>

#include <stdint.h>

PERFETTO_DEFINE_CATEGORIES_IN_NAMESPACE(systrace,
        perfetto::Category("filament"),
        perfetto::Category("jobsystem"),
        perfetto::Category("gltfio"));

PERFETTO_USE_CATEGORIES_FROM_NAMESPACE(systrace);

#if SYSTRACE_TAG == SYSTRACE_TAG_FILAMENT
#   define UTILS_PERFETTO_CATEGORY "filament"
#elif SYSTRACE_TAG == SYSTRACE_TAG_JOBSYSTEM
#   define UTILS_PERFETTO_CATEGORY "jobsystem"
#elif SYSTRACE_TAG == SYSTRACE_TAG_GLTFIO
#   define UTILS_PERFETTO_CATEGORY "gltfio"
#endif

#if SYSTRACE_TAG == SYSTRACE_TAG_DISABLED

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

#else

#define SYSTRACE_ENABLE()
#define SYSTRACE_CONTEXT()

#define SYSTRACE_CALL() \
    auto constexpr FILAMENT_SYSTRACE_FUNCTION = perfetto::StaticString(__FUNCTION__); \
    TRACE_EVENT(UTILS_PERFETTO_CATEGORY, FILAMENT_SYSTRACE_FUNCTION)

#define SYSTRACE_NAME(name) TRACE_EVENT(UTILS_PERFETTO_CATEGORY, nullptr,   \
        [&](perfetto::EventContext ctx) {                                   \
            ctx.event()->set_name(name);                                    \
        })

#define SYSTRACE_NAME_BEGIN(name) TRACE_EVENT_BEGIN(UTILS_PERFETTO_CATEGORY, nullptr,   \
        [&](perfetto::EventContext ctx) {                                               \
            ctx.event()->set_name(name);                                                \
        })

#define SYSTRACE_NAME_END() TRACE_EVENT_END(UTILS_PERFETTO_CATEGORY)

#define SYSTRACE_ASYNC_BEGIN(name, cookie) \
        TRACE_EVENT_BEGIN(UTILS_PERFETTO_CATEGORY, name, perfetto::Track(cookie))

#define SYSTRACE_ASYNC_END(name, cookie) \
        TRACE_EVENT_END(UTILS_PERFETTO_CATEGORY, perfetto::Track(cookie))

#define SYSTRACE_FRAME_ID(frame) \
    TRACE_EVENT_INSTANT(UTILS_PERFETTO_CATEGORY, "frame", "id", frame)

#define SYSTRACE_VALUE32(name, val) \
    TRACE_COUNTER(UTILS_PERFETTO_CATEGORY, name, val)

#define SYSTRACE_VALUE64(name, val) \
    TRACE_COUNTER(UTILS_PERFETTO_CATEGORY, name, val)

#endif // SYSTRACE_TAG == SYSTRACE_TAG_DISABLED

#endif // TNT_UTILS_ANDROID_SYSTRACE_H
