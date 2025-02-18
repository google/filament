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

#include "dawn/tests/perf_tests/DawnPerfTestPlatform.h"

#include <algorithm>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/HashUtils.h"
#include "dawn/platform/tracing/TraceEvent.h"
#include "dawn/tests/perf_tests/DawnPerfTest.h"
#include "dawn/utils/Timer.h"

namespace dawn {
namespace {

struct TraceCategoryInfo {
    unsigned char enabled;
    platform::TraceCategory category;
};

constexpr TraceCategoryInfo gTraceCategories[4] = {
    {1, platform::TraceCategory::General},
    {1, platform::TraceCategory::Validation},
    {1, platform::TraceCategory::Recording},
    {1, platform::TraceCategory::GPUWork},
};

static_assert(static_cast<uint32_t>(platform::TraceCategory::General) == 0);
static_assert(static_cast<uint32_t>(platform::TraceCategory::Validation) == 1);
static_assert(static_cast<uint32_t>(platform::TraceCategory::Recording) == 2);
static_assert(static_cast<uint32_t>(platform::TraceCategory::GPUWork) == 3);

}  // anonymous namespace

DawnPerfTestPlatform::DawnPerfTestPlatform() : platform::Platform(), mTimer(utils::CreateTimer()) {}

DawnPerfTestPlatform::~DawnPerfTestPlatform() = default;

const unsigned char* DawnPerfTestPlatform::GetTraceCategoryEnabledFlag(
    platform::TraceCategory category) {
    switch (category) {
        case platform::TraceCategory::General:
        case platform::TraceCategory::Validation:
        case platform::TraceCategory::Recording:
        case platform::TraceCategory::GPUWork:
            break;
        default:
            DAWN_UNREACHABLE();
    }
    return &gTraceCategories[static_cast<uint32_t>(category)].enabled;
}

double DawnPerfTestPlatform::MonotonicallyIncreasingTime() {
    // Move the time origin to the first call to this function, to avoid generating
    // unnecessarily large timestamps.
    static double origin = mTimer->GetAbsoluteTime();
    return mTimer->GetAbsoluteTime() - origin;
}

std::vector<DawnPerfTestPlatform::TraceEvent>* DawnPerfTestPlatform::GetLocalTraceEventBuffer() {
    // Cache the pointer to the vector in thread_local storage
    thread_local std::vector<TraceEvent>* traceEventBuffer = nullptr;

    if (traceEventBuffer == nullptr) {
        auto buffer = std::make_unique<std::vector<TraceEvent>>();
        traceEventBuffer = buffer.get();

        // Add a new buffer to the map
        std::lock_guard<std::mutex> guard(mTraceEventBufferMapMutex);
        mTraceEventBuffers[std::this_thread::get_id()] = std::move(buffer);
    }

    return traceEventBuffer;
}

// TODO(enga): Simplify this API.
uint64_t DawnPerfTestPlatform::AddTraceEvent(char phase,
                                             const unsigned char* categoryGroupEnabled,
                                             const char* name,
                                             uint64_t id,
                                             double timestamp,
                                             int numArgs,
                                             const char** argNames,
                                             const unsigned char* argTypes,
                                             const uint64_t* argValues,
                                             unsigned char flags) {
    if (!mRecordTraceEvents) {
        return 0;
    }

    // Discover the category name based on categoryGroupEnabled.  This flag comes from the first
    // parameter of TraceCategory, and corresponds to one of the entries in gTraceCategories.
    static_assert(offsetof(TraceCategoryInfo, enabled) == 0,
                  "|enabled| must be the first field of the TraceCategoryInfo class.");

    const TraceCategoryInfo* info =
        reinterpret_cast<const TraceCategoryInfo*>(categoryGroupEnabled);

    std::vector<TraceEvent>* buffer = GetLocalTraceEventBuffer();
    buffer->emplace_back(phase, info->category, name, id, timestamp);

    size_t hash = 0;
    HashCombine(&hash, buffer->size());
    HashCombine(&hash, std::this_thread::get_id());
    return static_cast<uint64_t>(hash);
}

void DawnPerfTestPlatform::EnableTraceEventRecording(bool enable) {
    mRecordTraceEvents = enable;
}

std::vector<DawnPerfTestPlatform::TraceEvent> DawnPerfTestPlatform::AcquireTraceEventBuffer() {
    std::vector<TraceEvent> traceEventBuffer;
    {
        // AcquireTraceEventBuffer should only be called when Dawn is completely idle. There should
        // be no threads inserting trace events.
        // Right now, this is safe because AcquireTraceEventBuffer is called after waiting on a
        // fence for all GPU commands to finish executing. When Dawn has multiple background threads
        // for other work (creation, validation, submission, residency, etc), we will need to ensure
        // all work on those threads is stopped as well.
        std::lock_guard<std::mutex> guard(mTraceEventBufferMapMutex);
        for (auto it = mTraceEventBuffers.begin(); it != mTraceEventBuffers.end(); ++it) {
            std::ostringstream stream;
            stream << it->first;
            std::string threadId = stream.str();

            std::transform(it->second->begin(), it->second->end(),
                           std::back_inserter(traceEventBuffer), [&threadId](TraceEvent ev) {
                               ev.threadId = threadId;
                               return ev;
                           });
            it->second->clear();
        }
    }
    return traceEventBuffer;
}

}  // namespace dawn
