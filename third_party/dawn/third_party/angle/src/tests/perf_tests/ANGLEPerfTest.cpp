//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ANGLEPerfTests:
//   Base class for google test performance tests
//

#include "ANGLEPerfTest.h"

#if defined(ANGLE_PLATFORM_ANDROID)
#    include <android/log.h>
#    include <dlfcn.h>
#endif
#include "ANGLEPerfTestArgs.h"
#include "common/base/anglebase/trace_event/trace_event.h"
#include "common/debug.h"
#include "common/gl_enum_utils.h"
#include "common/mathutil.h"
#include "common/platform.h"
#include "common/string_utils.h"
#include "common/system_utils.h"
#include "common/utilities.h"
#include "test_utils/runner/TestSuite.h"
#include "third_party/perf/perf_test.h"
#include "util/shader_utils.h"
#include "util/test_utils.h"

#if defined(ANGLE_PLATFORM_ANDROID)
#    include "util/android/AndroidWindow.h"
#endif

#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>
#include <sstream>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>

#if defined(ANGLE_USE_UTIL_LOADER) && defined(ANGLE_PLATFORM_WINDOWS)
#    include "util/windows/WGLWindow.h"
#endif  // defined(ANGLE_USE_UTIL_LOADER) &&defined(ANGLE_PLATFORM_WINDOWS)

using namespace angle;
namespace js = rapidjson;

namespace
{
constexpr size_t kInitialTraceEventBufferSize            = 50000;
constexpr double kMilliSecondsPerSecond                  = 1e3;
constexpr double kMicroSecondsPerSecond                  = 1e6;
constexpr double kNanoSecondsPerSecond                   = 1e9;
constexpr size_t kNumberOfStepsPerformedToComputeGPUTime = 16;
constexpr char kPeakMemoryMetric[]                       = ".memory_max";
constexpr char kMedianMemoryMetric[]                     = ".memory_median";

struct TraceCategory
{
    unsigned char enabled;
    const char *name;
};

constexpr TraceCategory gTraceCategories[2] = {
    {1, "gpu.angle"},
    {1, "gpu.angle.gpu"},
};

void EmptyPlatformMethod(PlatformMethods *, const char *) {}

void CustomLogError(PlatformMethods *platform, const char *errorMessage)
{
    auto *angleRenderTest = static_cast<ANGLERenderTest *>(platform->context);
    angleRenderTest->onErrorMessage(errorMessage);
}

TraceEventHandle AddPerfTraceEvent(PlatformMethods *platform,
                                   char phase,
                                   const unsigned char *categoryEnabledFlag,
                                   const char *name,
                                   unsigned long long id,
                                   double timestamp,
                                   int numArgs,
                                   const char **argNames,
                                   const unsigned char *argTypes,
                                   const unsigned long long *argValues,
                                   unsigned char flags)
{
    if (!gEnableTrace)
        return 0;

    // Discover the category name based on categoryEnabledFlag.  This flag comes from the first
    // parameter of TraceCategory, and corresponds to one of the entries in gTraceCategories.
    static_assert(offsetof(TraceCategory, enabled) == 0,
                  "|enabled| must be the first field of the TraceCategory class.");
    const TraceCategory *category = reinterpret_cast<const TraceCategory *>(categoryEnabledFlag);

    ANGLERenderTest *renderTest = static_cast<ANGLERenderTest *>(platform->context);

    std::lock_guard<std::mutex> lock(renderTest->getTraceEventMutex());

    uint32_t tid = renderTest->getCurrentThreadSerial();

    std::vector<TraceEvent> &buffer = renderTest->getTraceEventBuffer();
    buffer.emplace_back(phase, category->name, name, timestamp, tid);
    return buffer.size();
}

const unsigned char *GetPerfTraceCategoryEnabled(PlatformMethods *platform,
                                                 const char *categoryName)
{
    if (gEnableTrace)
    {
        for (const TraceCategory &category : gTraceCategories)
        {
            if (strcmp(category.name, categoryName) == 0)
            {
                return &category.enabled;
            }
        }
    }

    constexpr static unsigned char kZero = 0;
    return &kZero;
}

void UpdateTraceEventDuration(PlatformMethods *platform,
                              const unsigned char *categoryEnabledFlag,
                              const char *name,
                              TraceEventHandle eventHandle)
{
    // Not implemented.
}

double MonotonicallyIncreasingTime(PlatformMethods *platform)
{
    return GetHostTimeSeconds();
}

bool WriteJsonFile(const std::string &outputFile, js::Document *doc)
{
    FILE *fp = fopen(outputFile.c_str(), "w");
    if (!fp)
    {
        return false;
    }

    constexpr size_t kBufferSize = 0xFFFF;
    std::vector<char> writeBuffer(kBufferSize);
    js::FileWriteStream os(fp, writeBuffer.data(), kBufferSize);
    js::PrettyWriter<js::FileWriteStream> writer(os);
    if (!doc->Accept(writer))
    {
        fclose(fp);
        return false;
    }
    fclose(fp);
    return true;
}

void DumpTraceEventsToJSONFile(const std::vector<TraceEvent> &traceEvents,
                               const char *outputFileName)
{
    js::Document doc(js::kObjectType);
    js::Document::AllocatorType &allocator = doc.GetAllocator();

    js::Value events(js::kArrayType);

    for (const TraceEvent &traceEvent : traceEvents)
    {
        js::Value value(js::kObjectType);

        const uint64_t microseconds = static_cast<uint64_t>(traceEvent.timestamp * 1000.0 * 1000.0);

        js::Document::StringRefType eventName(traceEvent.name);
        js::Document::StringRefType categoryName(traceEvent.categoryName);
        js::Document::StringRefType pidName(
            strcmp(traceEvent.categoryName, "gpu.angle.gpu") == 0 ? "GPU" : "ANGLE");

        value.AddMember("name", eventName, allocator);
        value.AddMember("cat", categoryName, allocator);
        value.AddMember("ph", std::string(1, traceEvent.phase), allocator);
        value.AddMember("ts", microseconds, allocator);
        value.AddMember("pid", pidName, allocator);
        value.AddMember("tid", traceEvent.tid, allocator);

        events.PushBack(value, allocator);
    }

    doc.AddMember("traceEvents", events, allocator);

    if (WriteJsonFile(outputFileName, &doc))
    {
        printf("Wrote trace file to %s\n", outputFileName);
    }
    else
    {
        printf("Error writing trace file to %s\n", outputFileName);
    }
}

[[maybe_unused]] void KHRONOS_APIENTRY PerfTestDebugCallback(GLenum source,
                                                             GLenum type,
                                                             GLuint id,
                                                             GLenum severity,
                                                             GLsizei length,
                                                             const GLchar *message,
                                                             const void *userParam)
{
    // Early exit on non-errors.
    if (type != GL_DEBUG_TYPE_ERROR || !userParam)
    {
        return;
    }

    ANGLERenderTest *renderTest =
        const_cast<ANGLERenderTest *>(reinterpret_cast<const ANGLERenderTest *>(userParam));
    renderTest->onErrorMessage(message);
}

double ComputeMean(const std::vector<double> &values)
{
    double sum = std::accumulate(values.begin(), values.end(), 0.0);

    double mean = sum / static_cast<double>(values.size());
    return mean;
}

void FinishAndCheckForContextLoss()
{
    glFinish();
    if (glGetError() == GL_CONTEXT_LOST)
    {
        FAIL() << "Context lost";
    }
}

void DumpFpsValues(const char *test, double mean_time)
{
#if defined(ANGLE_PLATFORM_ANDROID)
    std::ofstream fp(AndroidWindow::GetExternalStorageDirectory() + "/traces_fps.txt",
                     std::ios::app);
#else
    std::ofstream fp("traces_fps.txt", std::ios::app);
#endif
    double fps_value = 1000 / mean_time;
    fp << test << " " << fps_value << std::endl;
    fp.close();
}

#if defined(ANGLE_PLATFORM_ANDROID)
constexpr bool kHasATrace = true;

void *gLibAndroid = nullptr;
bool (*gATraceIsEnabled)(void);
bool (*gATraceSetCounter)(const char *counterName, int64_t counterValue);

void SetupATrace()
{
    if (gLibAndroid == nullptr)
    {
        gLibAndroid       = dlopen("libandroid.so", RTLD_NOW | RTLD_LOCAL);
        gATraceIsEnabled  = (decltype(gATraceIsEnabled))dlsym(gLibAndroid, "ATrace_isEnabled");
        gATraceSetCounter = (decltype(gATraceSetCounter))dlsym(gLibAndroid, "ATrace_setCounter");
    }
}

bool ATraceEnabled()
{
    return gATraceIsEnabled();
}
#else
constexpr bool kHasATrace = false;
void SetupATrace() {}
bool ATraceEnabled()
{
    return false;
}
#endif
}  // anonymous namespace

TraceEvent::TraceEvent(char phaseIn,
                       const char *categoryNameIn,
                       const char *nameIn,
                       double timestampIn,
                       uint32_t tidIn)
    : phase(phaseIn), categoryName(categoryNameIn), name{}, timestamp(timestampIn), tid(tidIn)
{
    ASSERT(strlen(nameIn) < kMaxNameLen);
    strcpy(name, nameIn);
}

ANGLEPerfTest::ANGLEPerfTest(const std::string &name,
                             const std::string &backend,
                             const std::string &story,
                             unsigned int iterationsPerStep,
                             const char *units)
    : mName(name),
      mBackend(backend),
      mStory(story),
      mGPUTimeNs(0),
      mSkipTest(false),
      mStepsToRun(std::max(gStepsPerTrial, gMaxStepsPerformed)),
      mTrialNumStepsPerformed(0),
      mTotalNumStepsPerformed(0),
      mIterationsPerStep(iterationsPerStep),
      mRunning(true),
      mPerfMonitor(0)
{
    if (mStory == "")
    {
        mStory = "baseline_story";
    }
    if (mStory[0] == '_')
    {
        mStory = mStory.substr(1);
    }
    mReporter = std::make_unique<perf_test::PerfResultReporter>(mName + mBackend, mStory);
    mReporter->RegisterImportantMetric(".wall_time", units);
    mReporter->RegisterImportantMetric(".cpu_time", units);
    mReporter->RegisterImportantMetric(".gpu_time", units);
    mReporter->RegisterFyiMetric(".trial_steps", "count");
    mReporter->RegisterFyiMetric(".total_steps", "count");

    if (kHasATrace)
    {
        SetupATrace();
    }
}

ANGLEPerfTest::~ANGLEPerfTest() {}

void ANGLEPerfTest::run()
{
    printf("running test name: \"%s\", backend: \"%s\", story: \"%s\"\n", mName.c_str(),
           mBackend.c_str(), mStory.c_str());
#if defined(ANGLE_PLATFORM_ANDROID)
    __android_log_print(ANDROID_LOG_INFO, "ANGLE",
                        "running test name: \"%s\", backend: \"%s\", story: \"%s\"", mName.c_str(),
                        mBackend.c_str(), mStory.c_str());
#endif
    if (mSkipTest)
    {
        GTEST_SKIP() << mSkipTestReason;
        // GTEST_SKIP returns.
    }

    uint32_t numTrials = OneFrame() ? 1 : gTestTrials;
    if (gVerboseLogging)
    {
        printf("Test Trials: %d\n", static_cast<int>(numTrials));
    }

    atraceCounter("TraceStage", 3);

    for (uint32_t trial = 0; trial < numTrials; ++trial)
    {
        runTrial(gTrialTimeSeconds, mStepsToRun, RunTrialPolicy::RunContinuously);
        processResults();
        if (gVerboseLogging)
        {
            double trialTime = mTrialTimer.getElapsedWallClockTime();
            printf("Trial %d time: %.2lf seconds.\n", trial + 1, trialTime);

            double secondsPerStep      = trialTime / static_cast<double>(mTrialNumStepsPerformed);
            double secondsPerIteration = secondsPerStep / static_cast<double>(mIterationsPerStep);
            mTestTrialResults.push_back(secondsPerIteration * 1000.0);
        }
    }

    atraceCounter("TraceStage", 0);

    if (gVerboseLogging && !mTestTrialResults.empty())
    {
        double numResults = static_cast<double>(mTestTrialResults.size());
        double mean       = ComputeMean(mTestTrialResults);

        double variance = 0;
        for (double trialResult : mTestTrialResults)
        {
            double difference = trialResult - mean;
            variance += difference * difference;
        }
        variance /= numResults;

        double standardDeviation      = std::sqrt(variance);
        double coefficientOfVariation = standardDeviation / mean;

        if (mean < 0.001)
        {
            printf("Mean result time: %.4lf ns.\n", mean * 1000.0);
        }
        else
        {
            printf("Mean result time: %.4lf ms.\n", mean);
        }

        if (kStandaloneBenchmark)
        {
            DumpFpsValues(mStory.c_str(), mean);
        }

        printf("Coefficient of variation: %.2lf%%\n", coefficientOfVariation * 100.0);
    }
}

void ANGLEPerfTest::runTrial(double maxRunTime, int maxStepsToRun, RunTrialPolicy runPolicy)
{
    mTrialNumStepsPerformed = 0;
    mRunning                = true;
    mGPUTimeNs              = 0;
    int stepAlignment       = getStepAlignment();
    mTrialTimer.start();
    startTest();

    int loopStepsPerformed  = 0;
    double lastLoopWallTime = 0;
    while (mRunning)
    {
        // When ATrace enabled, track average frame time before the first frame of each trace loop.
        if (ATraceEnabled() && stepAlignment > 1 && runPolicy == RunTrialPolicy::RunContinuously &&
            mTrialNumStepsPerformed % stepAlignment == 0)
        {
            double wallTime = mTrialTimer.getElapsedWallClockTime();
            if (loopStepsPerformed > 0)  // 0 at the first frame of the first loop
            {
                int frameTimeAvgUs = int(1e6 * (wallTime - lastLoopWallTime) / loopStepsPerformed);
                atraceCounter("TraceLoopFrameTimeAvgUs", frameTimeAvgUs);
                loopStepsPerformed = 0;
            }
            lastLoopWallTime = wallTime;
        }

        // Only stop on aligned steps or in a few special case modes
        if (mTrialNumStepsPerformed % stepAlignment == 0 || gStepsPerTrial == 1 || gRunToKeyFrame ||
            gMaxStepsPerformed != kDefaultMaxStepsPerformed)
        {
            if (gMaxStepsPerformed > 0 && mTotalNumStepsPerformed >= gMaxStepsPerformed)
            {
                if (gVerboseLogging)
                {
                    printf("Stopping test after %d total steps.\n", mTotalNumStepsPerformed);
                }
                mRunning = false;
                break;
            }
            if (mTrialTimer.getElapsedWallClockTime() > maxRunTime)
            {
                if (gVerboseLogging)
                {
                    printf("Stopping test after %.2lf seconds.\n",
                           mTrialTimer.getElapsedWallClockTime());
                }
                mRunning = false;
                break;
            }
            if (mTrialNumStepsPerformed >= maxStepsToRun)
            {
                if (gVerboseLogging)
                {
                    printf("Stopping test after %d trial steps.\n", mTrialNumStepsPerformed);
                }
                mRunning = false;
                break;
            }
        }

        if (gFpsLimit)
        {
            double wantTime    = mTrialNumStepsPerformed / double(gFpsLimit);
            double currentTime = mTrialTimer.getElapsedWallClockTime();
            if (currentTime < wantTime)
            {
                std::this_thread::sleep_for(std::chrono::duration<double>(wantTime - currentTime));
            }
        }
        step();

        if (runPolicy == RunTrialPolicy::FinishEveryStep)
        {
            FinishAndCheckForContextLoss();
        }

        if (mRunning)
        {
            mTrialNumStepsPerformed++;
            mTotalNumStepsPerformed++;
            loopStepsPerformed++;
        }

        if ((mTotalNumStepsPerformed % kNumberOfStepsPerformedToComputeGPUTime) == 0)
        {
            computeGPUTime();
        }
    }

    if (runPolicy == RunTrialPolicy::RunContinuously)
    {
        atraceCounter("TraceLoopFrameTimeAvgUs", 0);
    }
    finishTest();
    mTrialTimer.stop();
    computeGPUTime();
}

void ANGLEPerfTest::SetUp()
{
    if (gWarmup)
    {
        atraceCounter("TraceStage", 1);

        // Trace tests run with glFinish for a loop (getStepAlignment == frameCount).
        int warmupSteps = getStepAlignment();
        if (gVerboseLogging)
        {
            printf("Warmup: %d steps\n", warmupSteps);
        }

        Timer warmupTimer;
        warmupTimer.start();

        runTrial(gTrialTimeSeconds, warmupSteps, RunTrialPolicy::FinishEveryStep);

        if (warmupSteps > 1)  // trace tests only: getStepAlignment() is 1 otherwise
        {
            atraceCounter("TraceStage", 2);

            // Short traces (e.g. 10 frames) have some spikes after the first loop b/308975999
            const double kMinWarmupTime = 1.5;
            double remainingTime        = kMinWarmupTime - warmupTimer.getElapsedWallClockTime();
            if (remainingTime > 0)
            {
                printf("Warmup: Looping for remaining warmup time (%.2f seconds).\n",
                       remainingTime);
                runTrial(remainingTime, std::numeric_limits<int>::max(),
                         RunTrialPolicy::RunContinuouslyWarmup);
            }
        }

        if (gVerboseLogging)
        {
            printf("Warmup took %.2lf seconds.\n", warmupTimer.getElapsedWallClockTime());
        }
    }
}

void ANGLEPerfTest::TearDown() {}

void ANGLEPerfTest::recordIntegerMetric(const char *metric, size_t value, const std::string &units)
{
    // Prints "RESULT ..." to stdout
    mReporter->AddResult(metric, value);

    // Saves results to file if enabled
    TestSuite::GetMetricWriter().writeInfo(mName, mBackend, mStory, metric, units);
    TestSuite::GetMetricWriter().writeIntegerValue(value);
}

void ANGLEPerfTest::recordDoubleMetric(const char *metric, double value, const std::string &units)
{
    // Prints "RESULT ..." to stdout
    mReporter->AddResult(metric, value);

    // Saves results to file if enabled
    TestSuite::GetMetricWriter().writeInfo(mName, mBackend, mStory, metric, units);
    TestSuite::GetMetricWriter().writeDoubleValue(value);
}

void ANGLEPerfTest::addHistogramSample(const char *metric, double value, const std::string &units)
{
    std::string measurement = mName + mBackend + metric;
    // Output histogram JSON set format if enabled.
    TestSuite::GetInstance()->addHistogramSample(measurement, mStory, value, units);
}

void ANGLEPerfTest::processResults()
{
    processClockResult(".cpu_time", mTrialTimer.getElapsedCpuTime());
    processClockResult(".wall_time", mTrialTimer.getElapsedWallClockTime());

    if (mGPUTimeNs > 0)
    {
        processClockResult(".gpu_time", mGPUTimeNs * 1e-9);
    }

    if (gVerboseLogging)
    {
        double fps = static_cast<double>(mTrialNumStepsPerformed * mIterationsPerStep) /
                     mTrialTimer.getElapsedWallClockTime();
        printf("Ran %0.2lf iterations per second\n", fps);
    }

    mReporter->AddResult(".trial_steps", static_cast<size_t>(mTrialNumStepsPerformed));
    mReporter->AddResult(".total_steps", static_cast<size_t>(mTotalNumStepsPerformed));

    if (!mProcessMemoryUsageKBSamples.empty())
    {
        std::sort(mProcessMemoryUsageKBSamples.begin(), mProcessMemoryUsageKBSamples.end());

        // Compute median.
        size_t medianIndex      = mProcessMemoryUsageKBSamples.size() / 2;
        uint64_t medianMemoryKB = mProcessMemoryUsageKBSamples[medianIndex];
        auto peakMemoryIterator = std::max_element(mProcessMemoryUsageKBSamples.begin(),
                                                   mProcessMemoryUsageKBSamples.end());
        uint64_t peakMemoryKB   = *peakMemoryIterator;

        processMemoryResult(kMedianMemoryMetric, medianMemoryKB);
        processMemoryResult(kPeakMemoryMetric, peakMemoryKB);
    }

    for (const auto &iter : mPerfCounterInfo)
    {
        const std::string &counterName = iter.second.name;
        std::vector<GLuint64> samples  = iter.second.samples;

        // Median
        {
            size_t midpoint = samples.size() / 2;
            std::nth_element(samples.begin(), samples.begin() + midpoint, samples.end());

            std::string medianName = "." + counterName + "_median";
            recordIntegerMetric(medianName.c_str(), static_cast<size_t>(samples[midpoint]),
                                "count");
            addHistogramSample(medianName.c_str(), static_cast<double>(samples[midpoint]), "count");
        }

        // Maximum
        {
            const auto &maxIt = std::max_element(samples.begin(), samples.end());

            std::string maxName = "." + counterName + "_max";
            recordIntegerMetric(maxName.c_str(), static_cast<size_t>(*maxIt), "count");
            addHistogramSample(maxName.c_str(), static_cast<double>(*maxIt), "count");
        }

        // Sum
        {
            GLuint64 sum =
                std::accumulate(samples.begin(), samples.end(), static_cast<GLuint64>(0));

            std::string sumName = "." + counterName + "_max";
            recordIntegerMetric(sumName.c_str(), static_cast<size_t>(sum), "count");
            addHistogramSample(sumName.c_str(), static_cast<double>(sum), "count");
        }
    }
}

void ANGLEPerfTest::processClockResult(const char *metric, double resultSeconds)
{
    double secondsPerStep      = resultSeconds / static_cast<double>(mTrialNumStepsPerformed);
    double secondsPerIteration = secondsPerStep / static_cast<double>(mIterationsPerStep);

    perf_test::MetricInfo metricInfo;
    std::string units;
    bool foundMetric = mReporter->GetMetricInfo(metric, &metricInfo);
    if (!foundMetric)
    {
        fprintf(stderr, "Error getting metric info for %s.\n", metric);
        return;
    }
    units = metricInfo.units;

    double result;

    if (units == "ms")
    {
        result = secondsPerIteration * kMilliSecondsPerSecond;
    }
    else if (units == "us")
    {
        result = secondsPerIteration * kMicroSecondsPerSecond;
    }
    else
    {
        result = secondsPerIteration * kNanoSecondsPerSecond;
    }
    recordDoubleMetric(metric, result, units);
    addHistogramSample(metric, secondsPerIteration * kMilliSecondsPerSecond,
                       "msBestFitFormat_smallerIsBetter");
}

void ANGLEPerfTest::processMemoryResult(const char *metric, uint64_t resultKB)
{
    perf_test::MetricInfo metricInfo;
    if (!mReporter->GetMetricInfo(metric, &metricInfo))
    {
        mReporter->RegisterImportantMetric(metric, "sizeInBytes");
    }

    recordIntegerMetric(metric, static_cast<size_t>(resultKB * 1000), "sizeInBytes");
    addHistogramSample(metric, static_cast<double>(resultKB) * 1000.0,
                       "sizeInBytes_smallerIsBetter");
}

double ANGLEPerfTest::normalizedTime(size_t value) const
{
    return static_cast<double>(value) / static_cast<double>(mTrialNumStepsPerformed);
}

int ANGLEPerfTest::getStepAlignment() const
{
    // Default: No special alignment rules.
    return 1;
}

void ANGLEPerfTest::atraceCounter(const char *counterName, int64_t counterValue)
{
#if defined(ANGLE_PLATFORM_ANDROID)
    if (ATraceEnabled())
    {
        gATraceSetCounter(counterName, counterValue);
    }
#endif
}

RenderTestParams::RenderTestParams()
{
#if defined(ANGLE_DEBUG_LAYERS_ENABLED)
    eglParameters.debugLayersEnabled = true;
#else
    eglParameters.debugLayersEnabled = false;
#endif
}

std::string RenderTestParams::backend() const
{
    std::stringstream strstr;

    switch (driver)
    {
        case GLESDriverType::AngleEGL:
            break;
        case GLESDriverType::AngleVulkanSecondariesEGL:
            strstr << "_vulkan_secondaries";
            break;
        case GLESDriverType::SystemWGL:
        case GLESDriverType::SystemEGL:
            strstr << "_native";
            break;
        case GLESDriverType::ZinkEGL:
            strstr << "_zink";
            break;
        default:
            assert(0);
            return "_unk";
    }

    switch (getRenderer())
    {
        case EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE:
            break;
        case EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE:
            strstr << "_d3d11";
            break;
        case EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE:
            strstr << "_d3d9";
            break;
        case EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE:
            strstr << "_gl";
            break;
        case EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE:
            strstr << "_gles";
            break;
        case EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE:
            strstr << "_vulkan";
            break;
        case EGL_PLATFORM_ANGLE_TYPE_METAL_ANGLE:
            strstr << "_metal";
            break;
        default:
            assert(0);
            return "_unk";
    }

    switch (eglParameters.deviceType)
    {
        case EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE:
            strstr << "_null";
            break;
        case EGL_PLATFORM_ANGLE_DEVICE_TYPE_SWIFTSHADER_ANGLE:
            strstr << "_swiftshader";
            break;
        default:
            break;
    }

    return strstr.str();
}

std::string RenderTestParams::story() const
{
    std::stringstream strstr;

    switch (surfaceType)
    {
        case SurfaceType::Window:
            break;
        case SurfaceType::WindowWithVSync:
            strstr << "_vsync";
            break;
        case SurfaceType::Offscreen:
            strstr << "_offscreen";
            break;
        default:
            UNREACHABLE();
            return "";
    }

    if (multisample)
    {
        strstr << "_" << samples << "_samples";
    }

    return strstr.str();
}

std::string RenderTestParams::backendAndStory() const
{
    return backend() + story();
}

ANGLERenderTest::ANGLERenderTest(const std::string &name,
                                 const RenderTestParams &testParams,
                                 const char *units)
    : ANGLEPerfTest(name,
                    testParams.backend(),
                    testParams.story(),
                    OneFrame() ? 1 : testParams.iterationsPerStep,
                    units),
      mTestParams(testParams),
      mIsTimestampQueryAvailable(false),
      mGLWindow(nullptr),
      mOSWindow(nullptr),
      mSwapEnabled(true)
{
    // Force fast tests to make sure our slowest bots don't time out.
    if (OneFrame())
    {
        const_cast<RenderTestParams &>(testParams).iterationsPerStep = 1;
    }

    // Try to ensure we don't trigger allocation during execution.
    mTraceEventBuffer.reserve(kInitialTraceEventBufferSize);

    switch (testParams.driver)
    {
        case GLESDriverType::AngleEGL:
            mGLWindow = EGLWindow::New(testParams.majorVersion, testParams.minorVersion);
            mEntryPointsLib.reset(OpenSharedLibrary(ANGLE_EGL_LIBRARY_NAME, SearchType::ModuleDir));
            break;
        case GLESDriverType::AngleVulkanSecondariesEGL:
            mGLWindow = EGLWindow::New(testParams.majorVersion, testParams.minorVersion);
            mEntryPointsLib.reset(OpenSharedLibrary(ANGLE_VULKAN_SECONDARIES_EGL_LIBRARY_NAME,
                                                    SearchType::ModuleDir));
            break;
        case GLESDriverType::SystemEGL:
#if defined(ANGLE_USE_UTIL_LOADER) && !defined(ANGLE_PLATFORM_WINDOWS)
            mGLWindow = EGLWindow::New(testParams.majorVersion, testParams.minorVersion);
            mEntryPointsLib.reset(OpenSharedLibraryWithExtension(
                GetNativeEGLLibraryNameWithExtension(), SearchType::SystemDir));
#else
            skipTest("Not implemented.");
#endif  // defined(ANGLE_USE_UTIL_LOADER) && !defined(ANGLE_PLATFORM_WINDOWS)
            break;
        case GLESDriverType::SystemWGL:
#if defined(ANGLE_USE_UTIL_LOADER) && defined(ANGLE_PLATFORM_WINDOWS)
            mGLWindow = WGLWindow::New(testParams.majorVersion, testParams.minorVersion);
            mEntryPointsLib.reset(OpenSharedLibrary("opengl32", SearchType::SystemDir));
#else
            skipTest("WGL driver not available.");
#endif  // defined(ANGLE_USE_UTIL_LOADER) && defined(ANGLE_PLATFORM_WINDOWS)
            break;
        case GLESDriverType::ZinkEGL:
            mGLWindow = EGLWindow::New(testParams.majorVersion, testParams.minorVersion);
            mEntryPointsLib.reset(
                OpenSharedLibrary(ANGLE_MESA_EGL_LIBRARY_NAME, SearchType::ModuleDir));
            break;
        default:
            skipTest("Error in switch.");
            break;
    }
}

ANGLERenderTest::~ANGLERenderTest()
{
    OSWindow::Delete(&mOSWindow);
    GLWindowBase::Delete(&mGLWindow);
}

void ANGLERenderTest::addExtensionPrerequisite(std::string extensionName)
{
    mExtensionPrerequisites.push_back(extensionName);
}

void ANGLERenderTest::addIntegerPrerequisite(GLenum target, int min)
{
    mIntegerPrerequisites.push_back({target, min});
}

void ANGLERenderTest::SetUp()
{
    if (mSkipTest)
    {
        return;
    }

    // Set a consistent CPU core affinity and high priority.
    StabilizeCPUForBenchmarking();

    mOSWindow = OSWindow::New();

    if (!mGLWindow)
    {
        skipTest("!mGLWindow");
        return;
    }

    mPlatformMethods.logError                    = CustomLogError;
    mPlatformMethods.logWarning                  = EmptyPlatformMethod;
    mPlatformMethods.logInfo                     = EmptyPlatformMethod;
    mPlatformMethods.addTraceEvent               = AddPerfTraceEvent;
    mPlatformMethods.getTraceCategoryEnabledFlag = GetPerfTraceCategoryEnabled;
    mPlatformMethods.updateTraceEventDuration    = UpdateTraceEventDuration;
    mPlatformMethods.monotonicallyIncreasingTime = MonotonicallyIncreasingTime;
    mPlatformMethods.context                     = this;

    if (!mOSWindow->initialize(mName, mTestParams.windowWidth, mTestParams.windowHeight))
    {
        failTest("Failed initializing OSWindow");
        return;
    }

    // Override platform method parameter.
    EGLPlatformParameters withMethods = mTestParams.eglParameters;
    withMethods.platformMethods       = &mPlatformMethods;

    // Request a common framebuffer config
    mConfigParams.redBits     = 8;
    mConfigParams.greenBits   = 8;
    mConfigParams.blueBits    = 8;
    mConfigParams.alphaBits   = 8;
    mConfigParams.depthBits   = 24;
    mConfigParams.stencilBits = 8;
    mConfigParams.colorSpace  = mTestParams.colorSpace;
    mConfigParams.multisample = mTestParams.multisample;
    mConfigParams.samples     = mTestParams.samples;
    if (mTestParams.surfaceType != SurfaceType::WindowWithVSync)
    {
        mConfigParams.swapInterval = 0;
    }

    if (gPrintExtensionsToFile != nullptr || gRequestedExtensions != nullptr)
    {
        mConfigParams.extensionsEnabled = false;
    }

    GLWindowResult res = mGLWindow->initializeGLWithResult(
        mOSWindow, mEntryPointsLib.get(), mTestParams.driver, withMethods, mConfigParams);
    switch (res)
    {
        case GLWindowResult::NoColorspaceSupport:
            skipTest("Missing support for color spaces.");
            return;
        case GLWindowResult::Error:
            failTest("Failed initializing GL Window");
            return;
        default:
            break;
    }

    if (gPrintExtensionsToFile)
    {
        std::ofstream fout(gPrintExtensionsToFile);
        if (fout.is_open())
        {
            int numExtensions = 0;
            glGetIntegerv(GL_NUM_REQUESTABLE_EXTENSIONS_ANGLE, &numExtensions);
            for (int ext = 0; ext < numExtensions; ext++)
            {
                fout << glGetStringi(GL_REQUESTABLE_EXTENSIONS_ANGLE, ext) << std::endl;
            }
            fout.close();
            std::stringstream statusString;
            statusString << "Wrote out to file: " << gPrintExtensionsToFile;
            skipTest(statusString.str());
        }
        else
        {
            std::stringstream failStr;
            failStr << "Failed to open file: " << gPrintExtensionsToFile;
            failTest(failStr.str());
        }
        return;
    }

    if (gRequestedExtensions != nullptr)
    {
        std::istringstream ss{gRequestedExtensions};
        std::string ext;
        while (std::getline(ss, ext, ' '))
        {
            glRequestExtensionANGLE(ext.c_str());
        }
    }

    // Disable vsync (if not done by the window init).
    if (mTestParams.surfaceType != SurfaceType::WindowWithVSync)
    {
        if (!mGLWindow->setSwapInterval(0))
        {
            failTest("Failed setting swap interval");
            return;
        }
    }

    if (mTestParams.trackGpuTime)
    {
        mIsTimestampQueryAvailable = EnsureGLExtensionEnabled("GL_EXT_disjoint_timer_query");
    }

    skipTestIfMissingExtensionPrerequisites();
    skipTestIfFailsIntegerPrerequisite();

    if (mSkipTest)
    {
        GTEST_SKIP() << mSkipTestReason;
        // GTEST_SKIP returns.
    }

#if defined(ANGLE_ENABLE_ASSERTS)
    if (IsGLExtensionEnabled("GL_KHR_debug") && mEnableDebugCallback)
    {
        EnableDebugCallback(&PerfTestDebugCallback, this);
    }
#endif

    initializeBenchmark();

    if (mSkipTest)
    {
        GTEST_SKIP() << mSkipTestReason;
        // GTEST_SKIP returns.
    }

    if (mTestParams.iterationsPerStep == 0)
    {
        failTest("Please initialize 'iterationsPerStep'.");
        return;
    }

    if (gVerboseLogging)
    {
        printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
        printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
    }

    mTestTrialResults.reserve(gTestTrials);

    // Runs warmup if enabled
    ANGLEPerfTest::SetUp();

    initPerfCounters();
}

void ANGLERenderTest::TearDown()
{
    ASSERT(mTimestampQueries.empty());

    if (!mPerfCounterInfo.empty())
    {
        glDeletePerfMonitorsAMD(1, &mPerfMonitor);
        mPerfMonitor = 0;
    }

    if (!mSkipTest)
    {
        destroyBenchmark();
    }

    if (mGLWindow)
    {
        mGLWindow->destroyGL();
        mGLWindow = nullptr;
    }

    if (mOSWindow)
    {
        mOSWindow->destroy();
        mOSWindow = nullptr;
    }

    // Dump trace events to json file.
    if (gEnableTrace)
    {
        DumpTraceEventsToJSONFile(mTraceEventBuffer, gTraceFile);
    }

    ANGLEPerfTest::TearDown();
}

void ANGLERenderTest::initPerfCounters()
{
    if (!gPerfCounters)
    {
        return;
    }

    if (!IsGLExtensionEnabled(kPerfMonitorExtensionName))
    {
        fprintf(stderr, "Cannot report perf metrics because %s is not available.\n",
                kPerfMonitorExtensionName);
        return;
    }

    CounterNameToIndexMap indexMap = BuildCounterNameToIndexMap();

    std::vector<std::string> counters =
        angle::SplitString(gPerfCounters, ":", angle::WhitespaceHandling::TRIM_WHITESPACE,
                           angle::SplitResult::SPLIT_WANT_NONEMPTY);
    for (const std::string &counter : counters)
    {
        bool found = false;

        for (const auto &indexMapIter : indexMap)
        {
            const std::string &indexMapName = indexMapIter.first;
            if (NamesMatchWithWildcard(counter.c_str(), indexMapName.c_str()))
            {
                {
                    std::stringstream medianStr;
                    medianStr << '.' << indexMapName << "_median";
                    std::string medianName = medianStr.str();
                    mReporter->RegisterImportantMetric(medianName, "count");
                }

                {
                    std::stringstream maxStr;
                    maxStr << '.' << indexMapName << "_max";
                    std::string maxName = maxStr.str();
                    mReporter->RegisterImportantMetric(maxName, "count");
                }

                {
                    std::stringstream sumStr;
                    sumStr << '.' << indexMapName << "_sum";
                    std::string sumName = sumStr.str();
                    mReporter->RegisterImportantMetric(sumName, "count");
                }

                GLuint index            = indexMapIter.second;
                mPerfCounterInfo[index] = {indexMapName, {}};

                found = true;
            }
        }

        if (!found)
        {
            fprintf(stderr, "'%s' does not match any available perf counters.\n", counter.c_str());
        }
    }

    if (!mPerfCounterInfo.empty())
    {
        glGenPerfMonitorsAMD(1, &mPerfMonitor);
        // Note: technically, glSelectPerfMonitorCountersAMD should be used to select the counters,
        // but currently ANGLE always captures all counters.
    }
}

void ANGLERenderTest::updatePerfCounters()
{
    if (mPerfCounterInfo.empty())
    {
        return;
    }

    std::vector<PerfMonitorTriplet> perfData = GetPerfMonitorTriplets();
    ASSERT(!perfData.empty());

    for (auto &iter : mPerfCounterInfo)
    {
        uint32_t counter               = iter.first;
        std::vector<GLuint64> &samples = iter.second.samples;
        samples.push_back(perfData[counter].value);
    }
}

void ANGLERenderTest::beginInternalTraceEvent(const char *name)
{
    if (gEnableTrace)
    {
        mTraceEventBuffer.emplace_back(TRACE_EVENT_PHASE_BEGIN, gTraceCategories[0].name, name,
                                       MonotonicallyIncreasingTime(&mPlatformMethods),
                                       getCurrentThreadSerial());
    }
}

void ANGLERenderTest::endInternalTraceEvent(const char *name)
{
    if (gEnableTrace)
    {
        mTraceEventBuffer.emplace_back(TRACE_EVENT_PHASE_END, gTraceCategories[0].name, name,
                                       MonotonicallyIncreasingTime(&mPlatformMethods),
                                       getCurrentThreadSerial());
    }
}

void ANGLERenderTest::beginGLTraceEvent(const char *name, double hostTimeSec)
{
    if (gEnableTrace)
    {
        mTraceEventBuffer.emplace_back(TRACE_EVENT_PHASE_BEGIN, gTraceCategories[1].name, name,
                                       hostTimeSec, getCurrentThreadSerial());
    }
}

void ANGLERenderTest::endGLTraceEvent(const char *name, double hostTimeSec)
{
    if (gEnableTrace)
    {
        mTraceEventBuffer.emplace_back(TRACE_EVENT_PHASE_END, gTraceCategories[1].name, name,
                                       hostTimeSec, getCurrentThreadSerial());
    }
}

void ANGLERenderTest::step()
{
    beginInternalTraceEvent("step");

    // Clear events that the application did not process from this frame
    Event event;
    bool closed = false;
    while (popEvent(&event))
    {
        // If the application did not catch a close event, close now
        if (event.Type == Event::EVENT_CLOSED)
        {
            closed = true;
        }
    }

    if (closed)
    {
        abortTest();
    }
    else
    {
        drawBenchmark();

        // Swap is needed so that the GPU driver will occasionally flush its
        // internal command queue to the GPU. This is enabled for null back-end
        // devices because some back-ends (e.g. Vulkan) also accumulate internal
        // command queues.
        if (mSwapEnabled)
        {
            updatePerfCounters();
            mGLWindow->swap();
        }
        mOSWindow->messageLoop();

#if defined(ANGLE_ENABLE_ASSERTS)
        if (!gRetraceMode)
        {
            EXPECT_EQ(static_cast<GLenum>(GL_NO_ERROR), glGetError());
        }
#endif  // defined(ANGLE_ENABLE_ASSERTS)

        // Sample system memory
        uint64_t processMemoryUsageKB = GetProcessMemoryUsageKB();
        if (processMemoryUsageKB)
        {
            mProcessMemoryUsageKBSamples.push_back(processMemoryUsageKB);
        }
    }

    endInternalTraceEvent("step");
}

void ANGLERenderTest::startGpuTimer()
{
    if (mTestParams.trackGpuTime && mIsTimestampQueryAvailable)
    {
        glGenQueriesEXT(1, &mCurrentTimestampBeginQuery);
        glQueryCounterEXT(mCurrentTimestampBeginQuery, GL_TIMESTAMP_EXT);
    }
}

void ANGLERenderTest::stopGpuTimer()
{
    if (mTestParams.trackGpuTime && mIsTimestampQueryAvailable)
    {
        GLuint endQuery = 0;
        glGenQueriesEXT(1, &endQuery);
        glQueryCounterEXT(endQuery, GL_TIMESTAMP_EXT);
        mTimestampQueries.push({mCurrentTimestampBeginQuery, endQuery});
    }
}

void ANGLERenderTest::computeGPUTime()
{
    if (mTestParams.trackGpuTime && mIsTimestampQueryAvailable)
    {
        while (!mTimestampQueries.empty())
        {
            const TimestampSample &sample = mTimestampQueries.front();
            GLuint available              = GL_FALSE;
            glGetQueryObjectuivEXT(sample.endQuery, GL_QUERY_RESULT_AVAILABLE_EXT, &available);
            if (available != GL_TRUE)
            {
                // query is not completed yet, bail out
                break;
            }

            // frame's begin query must also completed.
            glGetQueryObjectuivEXT(sample.beginQuery, GL_QUERY_RESULT_AVAILABLE_EXT, &available);
            ASSERT(available == GL_TRUE);

            // Retrieve query result
            uint64_t beginGLTimeNs = 0;
            uint64_t endGLTimeNs   = 0;
            glGetQueryObjectui64vEXT(sample.beginQuery, GL_QUERY_RESULT_EXT, &beginGLTimeNs);
            glGetQueryObjectui64vEXT(sample.endQuery, GL_QUERY_RESULT_EXT, &endGLTimeNs);
            glDeleteQueriesEXT(1, &sample.beginQuery);
            glDeleteQueriesEXT(1, &sample.endQuery);
            mTimestampQueries.pop();

            // compute GPU time
            mGPUTimeNs += endGLTimeNs - beginGLTimeNs;
        }
    }
}

void ANGLERenderTest::startTest()
{
    if (!mPerfCounterInfo.empty())
    {
        glBeginPerfMonitorAMD(mPerfMonitor);
    }
}

void ANGLERenderTest::finishTest()
{
    if (mTestParams.eglParameters.deviceType != EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE &&
        !gNoFinish && !gRetraceMode)
    {
        FinishAndCheckForContextLoss();
    }

    if (!mPerfCounterInfo.empty())
    {
        glEndPerfMonitorAMD(mPerfMonitor);
    }
}

bool ANGLERenderTest::popEvent(Event *event)
{
    return mOSWindow->popEvent(event);
}

OSWindow *ANGLERenderTest::getWindow()
{
    return mOSWindow;
}

GLWindowBase *ANGLERenderTest::getGLWindow()
{
    return mGLWindow;
}

void ANGLERenderTest::skipTestIfMissingExtensionPrerequisites()
{
    for (std::string extension : mExtensionPrerequisites)
    {
        if (!CheckExtensionExists(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)),
                                  extension))
        {
            skipTest(std::string("Test skipped due to missing extension: ") + extension);
            return;
        }
    }
}

void ANGLERenderTest::skipTestIfFailsIntegerPrerequisite()
{
    for (const auto [target, minRequired] : mIntegerPrerequisites)
    {
        GLint driverValue;
        glGetIntegerv(target, &driverValue);
        if (static_cast<int>(driverValue) < minRequired)
        {
            std::stringstream ss;
            ss << "Test skipped due to value (" << std::to_string(static_cast<int>(driverValue))
               << ") being less than the prerequisite minimum (" << std::to_string(minRequired)
               << ") for GL constant " << gl::GLenumToString(gl::GLESEnum::AllEnums, target);
            skipTest(ss.str());
        }
    }
}

void ANGLERenderTest::setWebGLCompatibilityEnabled(bool webglCompatibility)
{
    mConfigParams.webGLCompatibility = webglCompatibility;
}

void ANGLERenderTest::setRobustResourceInit(bool enabled)
{
    mConfigParams.robustResourceInit = enabled;
}

std::vector<TraceEvent> &ANGLERenderTest::getTraceEventBuffer()
{
    return mTraceEventBuffer;
}

void ANGLERenderTest::onErrorMessage(const char *errorMessage)
{
    abortTest();
    std::ostringstream err;
    err << "Failing test because of unexpected error:\n" << errorMessage << "\n";
    failTest(err.str());
}

uint32_t ANGLERenderTest::getCurrentThreadSerial()
{
    uint64_t id = angle::GetCurrentThreadUniqueId();

    for (uint32_t serial = 0; serial < static_cast<uint32_t>(mThreadIDs.size()); ++serial)
    {
        if (mThreadIDs[serial] == id)
        {
            return serial + 1;
        }
    }

    mThreadIDs.push_back(id);
    return static_cast<uint32_t>(mThreadIDs.size());
}

namespace angle
{
double GetHostTimeSeconds()
{
    // Move the time origin to the first call to this function, to avoid generating unnecessarily
    // large timestamps.
    static double origin = GetCurrentSystemTime();
    return GetCurrentSystemTime() - origin;
}
}  // namespace angle
