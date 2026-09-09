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

#include "src/dawn/tests/perf_tests/DawnPerfTest.h"

#include <algorithm>
#include <fstream>
#include <limits>
#include <utility>

#include "src/dawn/platform/tracing/TraceEvent.h"
#include "src/dawn/tests/perf_tests/DawnPerfTestPlatform.h"
#include "src/dawn/utils/Timer.h"
#include "src/utils/assert.h"
#include "src/utils/compiler.h"
#include "src/utils/log.h"

#if defined(DAWN_USE_PERFETTO)
#include <perfetto/tracing/core/trace_config.h>
#include <perfetto/tracing/tracing.h>
#include <perfetto/tracing/track_event.h>
#endif
#if defined(DAWN_USE_PERFETTO_TRACE_PROCESSOR)
#include "perfetto/trace_processor/trace_processor.h"  // nogncheck
#endif

namespace dawn {
namespace {

#if defined(DAWN_USE_PERFETTO)
void InitializePerfetto() {
    if (!perfetto::Tracing::IsInitialized()) {
        perfetto::TracingInitArgs args;
        args.backends = perfetto::kInProcessBackend;
        perfetto::Tracing::Initialize(args);
    }
}

std::unique_ptr<perfetto::TracingSession> StartPerfettoTracing() {
    InitializePerfetto();

    perfetto::TraceConfig cfg;
    cfg.add_buffers()->set_size_kb(102400);  // 100MB

    auto* ds_cfg = cfg.add_data_sources()->mutable_config();
    ds_cfg->set_name("track_event");

    auto tracingSession = perfetto::Tracing::NewTrace();
    tracingSession->Setup(cfg);
    tracingSession->StartBlocking();
    return tracingSession;
}

std::vector<char> StopPerfettoTracing(perfetto::TracingSession* tracingSession,
                                      const char* filename) {
    tracingSession->StopBlocking();
    std::vector<char> traceData = tracingSession->ReadTraceBlocking();

    if (filename != nullptr) {
        std::ofstream outFile(filename, std::ios::binary);
        if (outFile) {
            outFile.write(traceData.data(), traceData.size());
            dawn::InfoLog() << "Wrote Perfetto trace to " << filename;
        } else {
            dawn::WarningLog() << "Error opening trace file " << filename << " for writing";
        }
    }
    return traceData;
}
#endif  // defined(DAWN_USE_PERFETTO)

DawnPerfTestEnvironment* gTestEnv = nullptr;

}  // namespace
}  // namespace dawn

void InitDawnPerfTestEnvironment(int argc, char** argv) {
    dawn::gTestEnv = new dawn::DawnPerfTestEnvironment(argc, argv);
    dawn::DawnTestEnvironment::SetEnvironment(dawn::gTestEnv);
    testing::AddGlobalTestEnvironment(dawn::gTestEnv);
}

namespace dawn {

DawnPerfTestEnvironment::DawnPerfTestEnvironment(int argc, char** argv)
    : DawnTestEnvironment(argc, argv) {
    size_t argLen = 0;  // Set when parsing --arg=X arguments
    for (int i = 1; i < argc; ++i) {
        if (DAWN_UNSAFE_TODO(strcmp("--calibration", argv[i])) == 0) {
            mIsCalibrating = true;
            continue;
        }

        constexpr const char kOverrideStepsArg[] = "--override-steps=";
        argLen = sizeof(kOverrideStepsArg) - 1;
        if (DAWN_UNSAFE_TODO(strncmp(argv[i], kOverrideStepsArg, argLen)) == 0) {
            const char* overrideSteps = DAWN_UNSAFE_TODO(argv[i] + argLen);
            if (overrideSteps[0] != '\0') {
                mOverrideStepsToRun = DAWN_UNSAFE_TODO(strtoul(overrideSteps, nullptr, 0));
            }
            continue;
        }

        constexpr const char kTraceFileArg[] = "--trace-file=";
        argLen = sizeof(kTraceFileArg) - 1;
        if (DAWN_UNSAFE_TODO(strncmp(argv[i], kTraceFileArg, argLen)) == 0) {
            const char* traceFile = DAWN_UNSAFE_TODO(argv[i] + argLen);
            if (traceFile[0] != '\0') {
                mTraceFile = traceFile;
            }
            continue;
        }

        if (DAWN_UNSAFE_TODO(strcmp("-h", argv[i])) == 0 ||
            DAWN_UNSAFE_TODO(strcmp("--help", argv[i])) == 0) {
            InfoLog() << "Additional flags:"
                      << " [--calibration] [--override-steps=x] [--trace-file=file]\n"
                      << "  --calibration: Only run calibration. Calibration allows the perf test"
                         " runner script to save some time.\n"
                      << " --override-steps: Set a fixed number of steps to run for each test\n"
                      << " --trace-file: The file to dump trace results.\n";
            continue;
        }
    }
}

DawnPerfTestEnvironment::~DawnPerfTestEnvironment() = default;

void DawnPerfTestEnvironment::SetUp() {
#if defined(DAWN_USE_PERFETTO)
    InitializePerfetto();
#endif
    mPlatform = std::make_unique<DawnPerfTestPlatform>();
    mInstance = CreateInstance(mPlatform.get());
    DAWN_ASSERT(mInstance);
}

void DawnPerfTestEnvironment::TearDown() {
    DawnTestEnvironment::TearDown();
}

bool DawnPerfTestEnvironment::IsCalibrating() const {
    return mIsCalibrating;
}

unsigned int DawnPerfTestEnvironment::OverrideStepsToRun() const {
    return mOverrideStepsToRun;
}

const char* DawnPerfTestEnvironment::GetTraceFile() const {
    return mTraceFile;
}

DawnPerfTestPlatform* DawnPerfTestEnvironment::GetPlatform() const {
    return mPlatform.get();
}

DawnPerfTestBase::DawnPerfTestBase(DawnTestBase* test,
                                   unsigned int iterationsPerStep,
                                   unsigned int maxStepsInFlight)
    : mTest(test),
      mIterationsPerStep(iterationsPerStep),
      mMaxStepsInFlight(maxStepsInFlight),
      mTimer(utils::CreateTimer()) {}

DawnPerfTestBase::~DawnPerfTestBase() = default;

void DawnPerfTestBase::AbortTest() {
    mRunning = false;
}

void DawnPerfTestBase::RunTest() {
    if (gTestEnv->OverrideStepsToRun() == 0) {
        // Run to compute the approximate number of steps to perform.
        mStepsToRun = std::numeric_limits<unsigned int>::max();

        // Do a warmup run for calibration.
        DoRunLoop(kCalibrationRunTimeSeconds);
        DoRunLoop(kCalibrationRunTimeSeconds);

        // Scale steps down according to the time that exceeded one second.
        double scale = kCalibrationRunTimeSeconds / mTimer->GetElapsedTime();
        mStepsToRun = static_cast<unsigned int>(static_cast<double>(mNumStepsPerformed) * scale);

        // Calibration allows the perf test runner script to save some time.
        if (gTestEnv->IsCalibrating()) {
            PrintResult("steps", mStepsToRun, "count", false);
            return;
        }
    } else {
        mStepsToRun = gTestEnv->OverrideStepsToRun();
    }

    // Do another warmup run. Seems to consistently improve results.
    DoRunLoop(kMaximumRunTimeSeconds);

    // Only enable trace event recording in this section.
    // We don't care about trace events during warmup and calibration.
    for (unsigned int trial = 0; trial < kNumTrials; ++trial) {
#if defined(DAWN_USE_PERFETTO)
        std::unique_ptr<perfetto::TracingSession> tracingSession;
        if (gTestEnv->GetTraceFile() != nullptr) {
            tracingSession = StartPerfettoTracing();
        }
#endif
        {
            TRACE_EVENT(DAWN_TRACE_CATEGORY(), "Trial");
            DoRunLoop(kMaximumRunTimeSeconds);
        }

        std::vector<char> traceData;
#if defined(DAWN_USE_PERFETTO)
        if (tracingSession) {
            traceData = StopPerfettoTracing(tracingSession.get(), gTestEnv->GetTraceFile());
        }
#endif
        OutputResults(traceData);
    }
}

void DawnPerfTestBase::DoRunLoop(double maxRunTime) {
    mNumStepsPerformed = 0;
    mCpuTime = 0;
    mGPUTime = std::nullopt;
    mRunning = true;

    uint64_t finishedIterations = 0;
    uint64_t submittedIterations = 0;

    mTimer->Start();

    // This loop can be canceled by calling AbortTest().
    while (mRunning) {
        // Wait if there are too many steps in flight on the GPU.
        while (submittedIterations - finishedIterations >= mMaxStepsInFlight) {
            mTest->WaitABit();
        }

        TRACE_EVENT(DAWN_TRACE_CATEGORY(), "Step");
        double stepStart = mTimer->GetElapsedTime();
        Step();
        mCpuTime += mTimer->GetElapsedTime() - stepStart;

        submittedIterations++;
        mTest->queue.OnSubmittedWorkDone(
            wgpu::CallbackMode::AllowProcessEvents,
            [&finishedIterations](wgpu::QueueWorkDoneStatus, wgpu::StringView) {
                finishedIterations++;
            });

        if (mRunning) {
            ++mNumStepsPerformed;
            if (mTimer->GetElapsedTime() > maxRunTime) {
                mRunning = false;
            } else if (mNumStepsPerformed >= mStepsToRun) {
                mRunning = false;
            }
        }
    }

    // Wait for all GPU commands to complete.
    mTest->WaitForAllOperations();

    mTimer->Stop();
}

void DawnPerfTestBase::OutputResults(const std::vector<char>& traceData) {
    // TODO(enga): When Dawn has multiple backgrounds threads, add a Device::WaitForIdleForTesting()
    // which waits for all threads to stop doing work. When we output results, there should
    // be no additional incoming trace events.
    PrintPerIterationResultFromSeconds("wall_time", mTimer->GetElapsedTime(), true);
    PrintPerIterationResultFromSeconds("cpu_time", mCpuTime, true);
    if (mGPUTime.has_value()) {
        PrintPerIterationResultFromSeconds("gpu_time", *mGPUTime, true);
    }

#if defined(DAWN_USE_PERFETTO_TRACE_PROCESSOR)
    if (!traceData.empty()) {
        perfetto::trace_processor::Config config;
        std::unique_ptr<perfetto::trace_processor::TraceProcessor> tp =
            perfetto::trace_processor::TraceProcessor::CreateInstance(config);

        auto buf = std::make_unique<uint8_t[]>(traceData.size());
        std::memcpy(buf.get(), traceData.data(), traceData.size());
        auto status = tp->Parse(std::move(buf), traceData.size());
        tp->NotifyEndOfFile();

        if (status.ok()) {
            auto run_query = [&](const std::string& query) -> double {
                auto iterator = tp->ExecuteQuery(query);
                if (iterator.Next()) {
                    auto val = iterator.Get(0);
                    if (val.type == perfetto::trace_processor::SqlValue::kDouble) {
                        return val.AsDouble();
                    }
                }
                return 0.0;
            };

            // Query validation time
            double validation_time = run_query(
                "SELECT SUM(dur) / 1e9 FROM slice WHERE category = 'gpu.dawn.validation'");
            PrintPerIterationResultFromSeconds("validation_time", validation_time, true);

            // Query recording time
            double recording_time =
                run_query("SELECT SUM(dur) / 1e9 FROM slice WHERE category = 'gpu.dawn.recording'");
            PrintPerIterationResultFromSeconds("recording_time", recording_time, true);
        } else {
            printf("Error parsing trace data: %s\n", status.c_message());
        }
    }
#endif
}

void DawnPerfTestBase::AddGPUTime(double time) {
    if (!mGPUTime.has_value()) {
        mGPUTime = time;
    } else {
        *mGPUTime += time;
    }
}

void DawnPerfTestBase::PrintPerIterationResultFromSeconds(const std::string& trace,
                                                          double valueInSeconds,
                                                          bool important) const {
    if (valueInSeconds == 0) {
        return;
    }

    double secondsPerIteration =
        valueInSeconds / static_cast<double>(mNumStepsPerformed * mIterationsPerStep);

    // Give the result a different name to ensure separate graphs if we transition.
    if (secondsPerIteration > 1) {
        PrintResult(trace, secondsPerIteration * 1e3, "ms", important);
    } else if (secondsPerIteration > 1e-3) {
        PrintResult(trace, secondsPerIteration * 1e6, "us", important);
    } else {
        PrintResult(trace, secondsPerIteration * 1e9, "ns", important);
    }
}

void DawnPerfTestBase::PrintResult(const std::string& trace,
                                   double value,
                                   const std::string& units,
                                   bool important) const {
    PrintResultImpl(trace, std::to_string(value), units, important);
}

void DawnPerfTestBase::PrintResult(const std::string& trace,
                                   unsigned int value,
                                   const std::string& units,
                                   bool important) const {
    PrintResultImpl(trace, std::to_string(value), units, important);
}

void DawnPerfTestBase::PrintResultImpl(const std::string& trace,
                                       const std::string& value,
                                       const std::string& units,
                                       bool important) const {
    const ::testing::TestInfo* const testInfo =
        ::testing::UnitTest::GetInstance()->current_test_info();

    std::string metric = std::string(testInfo->test_suite_name()) + "." + trace;

    std::string story = testInfo->name();
    std::replace(story.begin(), story.end(), '/', '_');

    // The results are printed according to the format specified at
    // [chromium]//src/tools/perf/generate_legacy_perf_dashboard_json.py
    InfoLog() << (important ? "*" : "") << "RESULT " << metric << ": " << story << "= " << value
              << " " << units;
}

}  // namespace dawn
