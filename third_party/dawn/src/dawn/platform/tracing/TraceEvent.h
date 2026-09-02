// Copyright 2019 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// Trace events are for tracking application performance and resource usage.
// Dawn uses TRACE_EVENT* macros which map to Perfetto when enabled,
// and are no-ops otherwise.
#ifndef SRC_DAWN_PLATFORM_TRACING_TRACEEVENT_H_
#define SRC_DAWN_PLATFORM_TRACING_TRACEEVENT_H_

#include <cstdint>

#if defined(DAWN_USE_PERFETTO)

// TODO(crbug.com/432427382): Remove legacy trace events when all callsites are
// migrated.
#ifndef PERFETTO_ENABLE_LEGACY_TRACE_EVENTS
#define PERFETTO_ENABLE_LEGACY_TRACE_EVENTS 1
#endif

#include "perfetto/tracing/track_event.h"
#include "perfetto/tracing/track_event_legacy.h"
#include "src/dawn/platform/tracing/trace_categories.h"

#else

// No-op Perfetto V2 macros
#define TRACE_EVENT(category, name, ...) ((void)0)
#define TRACE_EVENT_BEGIN(category, name, ...) ((void)0)
#define TRACE_EVENT_END(category, ...) ((void)0)
#define TRACE_EVENT_INSTANT(category, name, ...) ((void)0)

// TODO(crbug.com/432427382): Remove legacy trace events when all callsites are
// migrated. No-op legacy macros (still used at some callsites)
#define TRACE_EVENT_NESTABLE_ASYNC_BEGIN0(category, name, id) ((void)0)
#define TRACE_EVENT_NESTABLE_ASYNC_END0(category, name, id) ((void)0)

#endif  // defined(DAWN_USE_PERFETTO)

#endif  // SRC_DAWN_PLATFORM_TRACING_TRACEEVENT_H_
