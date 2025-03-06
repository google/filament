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

#include "dawn/platform/tracing/EventTracer.h"
#include "dawn/common/Assert.h"
#include "dawn/platform/DawnPlatform.h"

namespace dawn::platform::tracing {

const unsigned char* GetTraceCategoryEnabledFlag(Platform* platform, TraceCategory category) {
    static unsigned char disabled = 0;
    if (platform == nullptr) {
        return &disabled;
    }

    const unsigned char* categoryEnabledFlag = platform->GetTraceCategoryEnabledFlag(category);
    if (categoryEnabledFlag != nullptr) {
        return categoryEnabledFlag;
    }

    return &disabled;
}

TraceEventHandle AddTraceEvent(Platform* platform,
                               char phase,
                               const unsigned char* categoryGroupEnabled,
                               const char* name,
                               uint64_t id,
                               int numArgs,
                               const char** argNames,
                               const unsigned char* argTypes,
                               const uint64_t* argValues,
                               unsigned char flags) {
    DAWN_ASSERT(platform != nullptr);

    double timestamp = platform->MonotonicallyIncreasingTime();
    if (timestamp != 0) {
        TraceEventHandle handle =
            platform->AddTraceEvent(phase, categoryGroupEnabled, name, id, timestamp, numArgs,
                                    argNames, argTypes, argValues, flags);
        return handle;
    }

    return static_cast<TraceEventHandle>(0);
}

}  // namespace dawn::platform::tracing
