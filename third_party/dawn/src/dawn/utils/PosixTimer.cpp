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

#include <stdint.h>
#include <time.h>

#include "dawn/utils/Timer.h"

namespace dawn::utils {

namespace {

uint64_t GetCurrentTimeNs() {
    struct timespec currentTime;
    clock_gettime(CLOCK_MONOTONIC, &currentTime);
    return currentTime.tv_sec * 1'000'000'000llu + currentTime.tv_nsec;
}

}  // anonymous namespace

class PosixTimer : public Timer {
  public:
    PosixTimer() : Timer(), mRunning(false) {}

    ~PosixTimer() override = default;

    void Start() override {
        mStartTimeNs = GetCurrentTimeNs();
        mRunning = true;
    }

    void Stop() override {
        mStopTimeNs = GetCurrentTimeNs();
        mRunning = false;
    }

    double GetElapsedTime() const override {
        uint64_t endTimeNs;
        if (mRunning) {
            endTimeNs = GetCurrentTimeNs();
        } else {
            endTimeNs = mStopTimeNs;
        }

        return (endTimeNs - mStartTimeNs) * 1e-9;
    }

    double GetAbsoluteTime() override { return GetCurrentTimeNs() * 1e-9; }

  private:
    bool mRunning;
    uint64_t mStartTimeNs;
    uint64_t mStopTimeNs;
};

Timer* CreateTimer() {
    return new PosixTimer();
}

}  // namespace dawn::utils
