// Copyright 2026 The Dawn & Tint Authors
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

#ifndef DAWN_TRACING_TRACE_CATEGORIES_H_
#define DAWN_TRACING_TRACE_CATEGORIES_H_

#include "dawn/platform/dawn_platform_export.h"
#include "perfetto/tracing/track_event.h"

#define DAWN_TRACE_CATEGORY(...) "gpu.dawn" __VA_OPT__(".") __VA_ARGS__

// List of categories used by Dawn trace events.
PERFETTO_DEFINE_CATEGORIES_IN_NAMESPACE_WITH_ATTRS(
    dawn,
    DAWN_PLATFORM_EXPORT,
    perfetto::Category(DAWN_TRACE_CATEGORY()),
    perfetto::Category(DAWN_TRACE_CATEGORY("validation")),
    perfetto::Category(DAWN_TRACE_CATEGORY("recording")),
    perfetto::Category(DAWN_TRACE_CATEGORY("gpu_work")));

PERFETTO_USE_CATEGORIES_FROM_NAMESPACE(dawn);

#endif  // DAWN_TRACING_TRACE_CATEGORIES_H_
