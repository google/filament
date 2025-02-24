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

#include <CoreServices/CoreServices.h>
#include <mach/mach.h>
#include <mach/mach_time.h>

#include "dawn/utils/Timer.h"

namespace dawn::utils {

class OSXTimer : public Timer {
  public:
    OSXTimer() : Timer(), mRunning(false), mSecondCoeff(0) {}

    ~OSXTimer() override = default;

    void Start() override {
        mStartTime = mach_absolute_time();
        // Cache secondCoeff
        GetSecondCoeff();
        mRunning = true;
    }

    void Stop() override {
        mStopTime = mach_absolute_time();
        mRunning = false;
    }

    double GetElapsedTime() const override {
        if (mRunning) {
            return mSecondCoeff * (mach_absolute_time() - mStartTime);
        } else {
            return mSecondCoeff * (mStopTime - mStartTime);
        }
    }

    double GetAbsoluteTime() override { return GetSecondCoeff() * mach_absolute_time(); }

  private:
    double GetSecondCoeff() {
        // If this is the first time we've run, get the timebase.
        if (mSecondCoeff == 0.0) {
            mach_timebase_info_data_t timebaseInfo;
            mach_timebase_info(&timebaseInfo);

            mSecondCoeff = timebaseInfo.numer * (1.0 / 1000000000) / timebaseInfo.denom;
        }

        return mSecondCoeff;
    }

    bool mRunning;
    uint64_t mStartTime;
    uint64_t mStopTime;
    double mSecondCoeff;
};

Timer* CreateTimer() {
    return new OSXTimer();
}

}  // namespace dawn::utils
