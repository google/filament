// Copyright 2020 The Dawn & Tint Authors
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

#include "dawn/platform/DawnPlatform.h"

#include <memory>

#include "dawn/common/Assert.h"
#include "dawn/platform/WorkerThread.h"

namespace dawn::platform {

CachingInterface::CachingInterface() = default;

CachingInterface::~CachingInterface() = default;

Platform::Platform() = default;

Platform::~Platform() = default;

const unsigned char* Platform::GetTraceCategoryEnabledFlag(TraceCategory category) {
    static unsigned char disabled = 0;
    return &disabled;
}

double Platform::MonotonicallyIncreasingTime() {
    return 0;
}

uint64_t Platform::AddTraceEvent(char phase,
                                 const unsigned char* categoryGroupEnabled,
                                 const char* name,
                                 uint64_t id,
                                 double timestamp,
                                 int numArgs,
                                 const char** argNames,
                                 const unsigned char* argTypes,
                                 const uint64_t* argValues,
                                 unsigned char flags) {
    // AddTraceEvent cannot be called if events are disabled.
    DAWN_UNREACHABLE();
}

void Platform::HistogramCustomCounts(const char* name,
                                     int sample,
                                     int min,
                                     int max,
                                     int bucketCount) {}

void Platform::HistogramCustomCountsHPC(const char* name,
                                        int sample,
                                        int min,
                                        int max,
                                        int bucketCount) {}

void Platform::HistogramEnumeration(const char* name, int sample, int boundaryValue) {}

void Platform::HistogramSparse(const char* name, int sample) {}

void Platform::HistogramBoolean(const char* name, bool sample) {}

dawn::platform::CachingInterface* Platform::GetCachingInterface() {
    return nullptr;
}

std::unique_ptr<dawn::platform::WorkerTaskPool> Platform::CreateWorkerTaskPool() {
    return std::make_unique<AsyncWorkerThreadPool>();
}

bool Platform::IsFeatureEnabled(Features feature) {
    switch (feature) {
        case Features::kWebGPUUseDXC:
#ifdef DAWN_USE_BUILT_DXC
            return true;
#else
            return false;
#endif
        case Features::kWebGPUUseTintIR:
#if defined(DAWN_OS_CHROMEOS)
            return true;
#else
            return false;
#endif
        case Features::kWebGPUUseVulkanMemoryModel:
            return false;
    }
    return false;
}

}  // namespace dawn::platform
