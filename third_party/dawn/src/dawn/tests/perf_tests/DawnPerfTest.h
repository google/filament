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

#ifndef SRC_DAWN_TESTS_PERF_TESTS_DAWNPERFTEST_H_
#define SRC_DAWN_TESTS_PERF_TESTS_DAWNPERFTEST_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "dawn/tests/DawnTest.h"
#include "partition_alloc/pointers/raw_ptr.h"

void InitDawnPerfTestEnvironment(int argc, char** argv);

namespace dawn {
namespace utils {
class Timer;
}

class DawnPerfTestPlatform;

class DawnPerfTestEnvironment : public DawnTestEnvironment {
  public:
    DawnPerfTestEnvironment(int argc, char** argv);
    ~DawnPerfTestEnvironment() override;

    void SetUp() override;
    void TearDown() override;

    bool IsCalibrating() const;
    unsigned int OverrideStepsToRun() const;

    // Returns the path to the trace file, or nullptr if traces should
    // not be written to a json file.
    const char* GetTraceFile() const;

    DawnPerfTestPlatform* GetPlatform() const;

  private:
    // Only run calibration which allows the perf test runner to save time.
    bool mIsCalibrating = false;

    // If non-zero, overrides the number of steps.
    unsigned int mOverrideStepsToRun = 0;

    const char* mTraceFile = nullptr;

    std::unique_ptr<DawnPerfTestPlatform> mPlatform;
};

class DawnPerfTestBase {
    static constexpr double kCalibrationRunTimeSeconds = 1.0;
    static constexpr double kMaximumRunTimeSeconds = 10.0;
    static constexpr unsigned int kNumTrials = 3;

  public:
    // Perf test results are reported as the amortized time of |mStepsToRun| * |mIterationsPerStep|.
    // A test deriving from |DawnPerfTestBase| must call the base contructor with
    // |iterationsPerStep| appropriately to reflect the amount of work performed.
    // |maxStepsInFlight| may be used to mimic having multiple frames or workloads in flight which
    // is common with double or triple buffered applications.
    DawnPerfTestBase(DawnTestBase* test,
                     unsigned int iterationsPerStep,
                     unsigned int maxStepsInFlight);
    virtual ~DawnPerfTestBase();

  protected:
    // Call if the test step was aborted and the test should stop running.
    void AbortTest();

    void RunTest();
    void PrintPerIterationResultFromSeconds(const std::string& trace,
                                            double valueInSeconds,
                                            bool important) const;
    void PrintResult(const std::string& trace,
                     double value,
                     const std::string& units,
                     bool important) const;
    void PrintResult(const std::string& trace,
                     unsigned int value,
                     const std::string& units,
                     bool important) const;
    void AddGPUTime(double GPUTime);

  private:
    void DoRunLoop(double maxRunTime);
    void OutputResults();

    void PrintResultImpl(const std::string& trace,
                         const std::string& value,
                         const std::string& units,
                         bool important) const;

    virtual void Step() = 0;

    raw_ptr<DawnTestBase> mTest;
    bool mRunning = false;
    const unsigned int mIterationsPerStep;
    const unsigned int mMaxStepsInFlight;
    unsigned int mStepsToRun = 0;
    unsigned int mNumStepsPerformed = 0;
    double mCpuTime;
    std::unique_ptr<utils::Timer> mTimer;
    std::optional<double> mGPUTime;
};

template <typename Params = AdapterTestParam>
class DawnPerfTestWithParams : public DawnTestWithParams<Params>, public DawnPerfTestBase {
  protected:
    DawnPerfTestWithParams(unsigned int iterationsPerStep, unsigned int maxStepsInFlight)
        : DawnTestWithParams<Params>(),
          DawnPerfTestBase(this, iterationsPerStep, maxStepsInFlight) {}
    void SetUp() override {
        DawnTestWithParams<Params>::SetUp();

        DAWN_TEST_UNSUPPORTED_IF(this->IsCPU());

        if (mSupportsTimestampQuery) {
            InitializeGPUTimer();
        }
    }
    ~DawnPerfTestWithParams() override = default;

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {wgpu::FeatureName::TimestampQuery};
        mSupportsTimestampQuery = DawnTestWithParams<Params>::SupportsFeatures(requiredFeatures);
        if (mSupportsTimestampQuery) {
            return requiredFeatures;
        }
        return {};
    }

    bool SupportsTimestampQuery() const { return mSupportsTimestampQuery; }

    void RecordBeginTimestamp(wgpu::CommandEncoder encoder) {
        encoder.WriteTimestamp(mTimestampQuerySet, 0);
    }

    void RecordEndTimestampAndResolveQuerySet(wgpu::CommandEncoder encoder) {
        encoder.WriteTimestamp(mTimestampQuerySet, 1);
        ResolveTimestamps(encoder);
    }

    wgpu::PassTimestampWrites GetPassTimestampWrites() const {
        wgpu::PassTimestampWrites timestampWrites;
        timestampWrites.querySet = mTimestampQuerySet;
        timestampWrites.beginningOfPassWriteIndex = 0;
        timestampWrites.endOfPassWriteIndex = 1;
        return timestampWrites;
    }

    void ResolveTimestamps(wgpu::CommandEncoder encoder) {
        encoder.ResolveQuerySet(mTimestampQuerySet, 0, kTimestampQueryCount, mResolveBuffer, 0);
        encoder.CopyBufferToBuffer(mResolveBuffer, 0, mReadbackBuffer, 0,
                                   sizeof(uint64_t) * kTimestampQueryCount);
    }

    void ComputeGPUElapsedTime() {
        bool done = false;
        mReadbackBuffer.MapAsync(wgpu::MapMode::Read, 0, sizeof(uint64_t) * kTimestampQueryCount,
                                 wgpu::CallbackMode::AllowProcessEvents,
                                 [&done](wgpu::MapAsyncStatus, wgpu::StringView) { done = true; });
        while (!done) {
            DawnTestWithParams<Params>::WaitABit();
        }
        const uint64_t* readbackValues =
            static_cast<const uint64_t*>(mReadbackBuffer.GetConstMappedRange());
        ASSERT_EQ(2u, kTimestampQueryCount);
        double gpuTimeElapsed = (readbackValues[1] - readbackValues[0]) / 1e9;
        AddGPUTime(gpuTimeElapsed);
        mReadbackBuffer.Unmap();
    }

  private:
    void InitializeGPUTimer() {
        DAWN_ASSERT(mSupportsTimestampQuery);

        wgpu::Device device = this->device;

        wgpu::QuerySetDescriptor querySetDescriptor;
        querySetDescriptor.count = kTimestampQueryCount;
        querySetDescriptor.type = wgpu::QueryType::Timestamp;
        mTimestampQuerySet = device.CreateQuerySet(&querySetDescriptor);

        wgpu::BufferDescriptor resolveBufferDescriptor;
        resolveBufferDescriptor.size = kTimestampQueryCount * sizeof(uint64_t);
        resolveBufferDescriptor.usage = wgpu::BufferUsage::QueryResolve |
                                        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        mResolveBuffer = device.CreateBuffer(&resolveBufferDescriptor);

        wgpu::BufferDescriptor readbackBufferDescriptor;
        readbackBufferDescriptor.size = kTimestampQueryCount * sizeof(uint64_t);
        readbackBufferDescriptor.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
        mReadbackBuffer = device.CreateBuffer(&readbackBufferDescriptor);
    }

    static constexpr uint32_t kTimestampQueryCount = 2;

    bool mSupportsTimestampQuery = false;

    wgpu::QuerySet mTimestampQuerySet;
    wgpu::Buffer mResolveBuffer;
    wgpu::Buffer mReadbackBuffer;
};

using DawnPerfTest = DawnPerfTestWithParams<>;

}  // namespace dawn

#endif  // SRC_DAWN_TESTS_PERF_TESTS_DAWNPERFTEST_H_
