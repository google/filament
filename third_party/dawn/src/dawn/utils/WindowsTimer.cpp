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

#include "dawn/common/windows_with_undefs.h"
#include "dawn/utils/Timer.h"

namespace dawn::utils {

class WindowsTimer : public Timer {
  public:
    WindowsTimer() : Timer(), mRunning(false), mFrequency(0) {}

    ~WindowsTimer() override = default;

    void Start() override {
        LARGE_INTEGER curTime;
        QueryPerformanceCounter(&curTime);
        mStartTime = curTime.QuadPart;

        // Cache the frequency
        GetFrequency();

        mRunning = true;
    }

    void Stop() override {
        LARGE_INTEGER curTime;
        QueryPerformanceCounter(&curTime);
        mStopTime = curTime.QuadPart;

        mRunning = false;
    }

    double GetElapsedTime() const override {
        LONGLONG endTime;
        if (mRunning) {
            LARGE_INTEGER curTime;
            QueryPerformanceCounter(&curTime);
            endTime = curTime.QuadPart;
        } else {
            endTime = mStopTime;
        }

        return static_cast<double>(endTime - mStartTime) / mFrequency;
    }

    double GetAbsoluteTime() override {
        LARGE_INTEGER curTime;
        QueryPerformanceCounter(&curTime);

        return static_cast<double>(curTime.QuadPart) / GetFrequency();
    }

  private:
    LONGLONG GetFrequency() {
        if (mFrequency == 0) {
            LARGE_INTEGER frequency = {};
            QueryPerformanceFrequency(&frequency);

            mFrequency = frequency.QuadPart;
        }

        return mFrequency;
    }

    bool mRunning;
    LONGLONG mStartTime;
    LONGLONG mStopTime;
    LONGLONG mFrequency;
};

Timer* CreateTimer() {
    return new WindowsTimer();
}

}  // namespace dawn::utils
