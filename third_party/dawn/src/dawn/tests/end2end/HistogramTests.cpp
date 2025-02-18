// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/platform/metrics/HistogramMacros.h"
#include "dawn/tests/DawnTest.h"
#include "partition_alloc/pointers/raw_ptr.h"

namespace dawn {
namespace {

using ::testing::_;
using ::testing::InSequence;
using ::testing::NiceMock;

class DawnHistogramMockPlatform : public dawn::platform::Platform {
  public:
    void SetTime(double time) { mTime = time; }

    double MonotonicallyIncreasingTime() override { return mTime; }

    MOCK_METHOD(void,
                HistogramCustomCounts,
                (const char* name, int sample, int min, int max, int bucketCount),
                (override));

    MOCK_METHOD(void,
                HistogramCustomCountsHPC,
                (const char* name, int sample, int min, int max, int bucketCount),
                (override));

    MOCK_METHOD(void,
                HistogramEnumeration,
                (const char* name, int sample, int boundaryValue),
                (override));

    MOCK_METHOD(void, HistogramSparse, (const char* name, int sample), (override));

    MOCK_METHOD(void, HistogramBoolean, (const char* name, bool sample), (override));

  private:
    double mTime = 0.0;
};

class HistogramTests : public DawnTest {
  protected:
    void SetUp() override { DawnTest::SetUp(); }

    std::unique_ptr<platform::Platform> CreateTestPlatform() override {
        auto p = std::make_unique<NiceMock<DawnHistogramMockPlatform>>();
        mMockPlatform = p.get();
        return p;
    }

    raw_ptr<NiceMock<DawnHistogramMockPlatform>> mMockPlatform;
};

TEST_P(HistogramTests, Times) {
    InSequence seq;
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("times", 1, _, _, _));
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("medium_times", 2, _, _, _));
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("long_times", 3, _, _, _));
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("long_times_100", 4, _, _, _));
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("custom_times", 5, 0, 10, 42));
    EXPECT_CALL(*mMockPlatform,
                HistogramCustomCountsHPC("custom_microsecond_times", 6, 0, 100, 420));

    DAWN_HISTOGRAM_TIMES(mMockPlatform, "times", 1);
    DAWN_HISTOGRAM_MEDIUM_TIMES(mMockPlatform, "medium_times", 2);
    DAWN_HISTOGRAM_LONG_TIMES(mMockPlatform, "long_times", 3);
    DAWN_HISTOGRAM_LONG_TIMES_100(mMockPlatform, "long_times_100", 4);
    DAWN_HISTOGRAM_CUSTOM_TIMES(mMockPlatform, "custom_times", 5, 0, 10, 42);
    DAWN_HISTOGRAM_CUSTOM_MICROSECOND_TIMES(mMockPlatform, "custom_microsecond_times", 6, 0, 100,
                                            420);
}

TEST_P(HistogramTests, Percentage) {
    InSequence seq;
    EXPECT_CALL(*mMockPlatform, HistogramEnumeration("percentage", 0, 101));
    EXPECT_CALL(*mMockPlatform, HistogramEnumeration("percentage", 42, 101));
    EXPECT_CALL(*mMockPlatform, HistogramEnumeration("percentage", 100, 101));

    DAWN_HISTOGRAM_PERCENTAGE(mMockPlatform, "percentage", 0);
    DAWN_HISTOGRAM_PERCENTAGE(mMockPlatform, "percentage", 42);
    DAWN_HISTOGRAM_PERCENTAGE(mMockPlatform, "percentage", 100);
}

TEST_P(HistogramTests, Boolean) {
    InSequence seq;
    EXPECT_CALL(*mMockPlatform, HistogramBoolean("boolean", false));
    EXPECT_CALL(*mMockPlatform, HistogramBoolean("boolean", true));

    DAWN_HISTOGRAM_BOOLEAN(mMockPlatform, "boolean", false);
    DAWN_HISTOGRAM_BOOLEAN(mMockPlatform, "boolean", true);
}

TEST_P(HistogramTests, Enumeration) {
    enum Animal { Dog, Cat, Bear, Count };

    InSequence seq;
    EXPECT_CALL(*mMockPlatform, HistogramEnumeration("animal", Animal::Dog, Animal::Count));
    EXPECT_CALL(*mMockPlatform, HistogramEnumeration("animal", Animal::Cat, Animal::Count));
    EXPECT_CALL(*mMockPlatform, HistogramEnumeration("animal", Animal::Bear, Animal::Count));

    DAWN_HISTOGRAM_ENUMERATION(mMockPlatform, "animal", Animal::Dog, Animal::Count);
    DAWN_HISTOGRAM_ENUMERATION(mMockPlatform, "animal", Animal::Cat, Animal::Count);
    DAWN_HISTOGRAM_ENUMERATION(mMockPlatform, "animal", Animal::Bear, Animal::Count);
}

TEST_P(HistogramTests, Memory) {
    InSequence seq;
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("kb", 1, _, _, _));
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("mb", 2, _, _, _));

    DAWN_HISTOGRAM_MEMORY_KB(mMockPlatform, "kb", 1);
    DAWN_HISTOGRAM_MEMORY_MB(mMockPlatform, "mb", 2);
}

TEST_P(HistogramTests, Sparse) {
    InSequence seq;
    EXPECT_CALL(*mMockPlatform, HistogramSparse("sparse", 1));
    EXPECT_CALL(*mMockPlatform, HistogramSparse("sparse", 2));

    DAWN_HISTOGRAM_SPARSE(mMockPlatform, "sparse", 1);
    DAWN_HISTOGRAM_SPARSE(mMockPlatform, "sparse", 2);
}

TEST_P(HistogramTests, ScopedTimer) {
    InSequence seq;
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("timer0", 2'500, _, _, _));
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("timer1", 15'500, _, _, _));
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("timer4", 2'500, _, _, _));
    EXPECT_CALL(*mMockPlatform, HistogramCustomCounts("timer3", 6'000, _, _, _));
    EXPECT_CALL(*mMockPlatform, HistogramCustomCountsHPC("timer5", 1'500'000, _, _, _));

    {
        mMockPlatform->SetTime(1.0);
        SCOPED_DAWN_HISTOGRAM_TIMER(mMockPlatform, "timer0");
        mMockPlatform->SetTime(3.5);
    }
    {
        mMockPlatform->SetTime(10.0);
        SCOPED_DAWN_HISTOGRAM_LONG_TIMER(mMockPlatform, "timer1");
        mMockPlatform->SetTime(25.5);
    }
    {
        mMockPlatform->SetTime(1.0);
        SCOPED_DAWN_HISTOGRAM_TIMER(mMockPlatform, "timer3");
        mMockPlatform->SetTime(4.5);
        SCOPED_DAWN_HISTOGRAM_LONG_TIMER(mMockPlatform, "timer4");
        mMockPlatform->SetTime(7.0);
    }
    {
        mMockPlatform->SetTime(1.0);
        SCOPED_DAWN_HISTOGRAM_TIMER_MICROS(mMockPlatform, "timer5");
        mMockPlatform->SetTime(2.5);
    }
}

DAWN_INSTANTIATE_TEST(HistogramTests,
                      D3D11Backend(),
                      D3D12Backend(),
                      MetalBackend(),
                      NullBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

}  // namespace
}  // namespace dawn
