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

#ifndef SRC_DAWN_TESTS_PERF_TESTS_DAWNPERFTESTPLATFORM_H_
#define SRC_DAWN_TESTS_PERF_TESTS_DAWNPERFTESTPLATFORM_H_

#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "dawn/platform/DawnPlatform.h"

namespace dawn {
namespace utils {
class Timer;
}

class DawnPerfTestPlatform : public platform::Platform {
  public:
    // These are trace events according to Google's "Trace Event Format".
    // See https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU
    // Only a subset of the properties are implemented.
    struct TraceEvent final {
        TraceEvent() {}
        TraceEvent(char phaseIn,
                   platform::TraceCategory categoryIn,
                   const char* nameIn,
                   uint64_t idIn,
                   double timestampIn)
            : phase(phaseIn),
              category(categoryIn),
              name(nameIn),
              id(idIn),
              timestamp(timestampIn) {}

        char phase = 0;
        platform::TraceCategory category;
        const char* name = nullptr;
        uint64_t id = 0;
        std::string threadId;
        double timestamp = 0;
    };

    DawnPerfTestPlatform();
    ~DawnPerfTestPlatform() override;

    void EnableTraceEventRecording(bool enable);
    std::vector<TraceEvent> AcquireTraceEventBuffer();

  private:
    const unsigned char* GetTraceCategoryEnabledFlag(platform::TraceCategory category) override;

    double MonotonicallyIncreasingTime() override;

    std::vector<TraceEvent>* GetLocalTraceEventBuffer();

    uint64_t AddTraceEvent(char phase,
                           const unsigned char* categoryGroupEnabled,
                           const char* name,
                           uint64_t id,
                           double timestamp,
                           int numArgs,
                           const char** argNames,
                           const unsigned char* argTypes,
                           const uint64_t* argValues,
                           unsigned char flags) override;

    bool mRecordTraceEvents = false;
    std::unique_ptr<utils::Timer> mTimer;

    // Trace event record.
    // Each uses their own trace event buffer, but the PerfTestPlatform owns all of them in
    // this map. The map stores all of them so we can iterate through them and flush when
    // AcquireTraceEventBuffer is called.
    std::unordered_map<std::thread::id, std::unique_ptr<std::vector<TraceEvent>>>
        mTraceEventBuffers;
    std::mutex mTraceEventBufferMapMutex;
};

}  // namespace dawn

#endif  // SRC_DAWN_TESTS_PERF_TESTS_DAWNPERFTESTPLATFORM_H_
