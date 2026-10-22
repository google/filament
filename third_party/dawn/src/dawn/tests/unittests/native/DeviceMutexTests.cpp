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

#include <memory>
#include <utility>

#include "dawn/platform/DawnPlatform.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "src/dawn/common/Compiler.h"
#include "src/dawn/native/DeviceGuard.h"

namespace dawn::native {

class DeviceMutexTest : public ::testing::Test {
  protected:
    // TODO(533405609) Thread safety analysis flags this whole test as
    // problematic. The issue tracks revisiting the need for a recursive mutex in
    // the first place.
    void Lock(DeviceMutex& mutex) DAWN_NO_THREAD_SAFETY_ANALYSIS { mutex.Lock(); }
    void Unlock(DeviceMutex& mutex) DAWN_NO_THREAD_SAFETY_ANALYSIS { mutex.Unlock(); }
};

namespace {

using ::testing::_;
using ::testing::NiceMock;

class MockPlatform : public dawn::platform::Platform {
  public:
    double MonotonicallyIncreasingTime() override {
        mTime += mIncrement;
        return mTime;
    }

    MOCK_METHOD(void,
                HistogramCustomCountsHPC,
                (const char* name, int sample, int min, int max, int bucketCount),
                (override));

    double mTime = 0.0;
    double mIncrement = 0.0;
};

// Test that DeviceMutex records average and max lock acquisition time after 100 non-recursive
// locks.
TEST_F(DeviceMutexTest, UMARecording) {
    NiceMock<MockPlatform> mockPlatform;
    // Increment time by 5 microseconds (0.000005 seconds) per call to MonotonicallyIncreasingTime.
    // Lock() calls it twice (before and after lock), so elapsed time per lock will be 5us.
    mockPlatform.mIncrement = 0.000005;

    DeviceMutex mutex(&mockPlatform);

    // We expect both the Average and Max histograms to be called exactly once after 100 lock
    // acquisitions.
    EXPECT_CALL(mockPlatform,
                HistogramCustomCountsHPC(::testing::StrEq("DeviceLockAcquireTimeAvgUs"), 5, 1,
                                         1'000'000, 50))
        .Times(1);

    EXPECT_CALL(mockPlatform,
                HistogramCustomCountsHPC(::testing::StrEq("DeviceLockAcquireTimeMaxUs"), 5, 1,
                                         1'000'000, 50))
        .Times(1);

    for (int i = 0; i < 100; ++i) {
        Lock(mutex);
        Unlock(mutex);
    }
}

// Test that recursive locks are not measured or counted.
TEST_F(DeviceMutexTest, RecursiveLocksIgnored) {
    NiceMock<MockPlatform> mockPlatform;
    mockPlatform.mIncrement = 0.000005;

    DeviceMutex mutex(&mockPlatform);

    // Even if we perform 100 total locks, since they are recursive, they shouldn't trigger UMA
    // emission.
    EXPECT_CALL(mockPlatform, HistogramCustomCountsHPC(
                                  ::testing::StrEq("DeviceLockAcquireTimeAvgUs"), _, _, _, _))
        .Times(0);
    EXPECT_CALL(mockPlatform, HistogramCustomCountsHPC(
                                  ::testing::StrEq("DeviceLockAcquireTimeMaxUs"), _, _, _, _))
        .Times(0);

    Lock(mutex);  // Outermost lock - increment starts.
    for (int i = 0; i < 99; ++i) {
        Lock(mutex);  // Recursive locks.
    }
    for (int i = 0; i < 100; ++i) {
        Unlock(mutex);
    }
}

}  // namespace
}  // namespace dawn::native
