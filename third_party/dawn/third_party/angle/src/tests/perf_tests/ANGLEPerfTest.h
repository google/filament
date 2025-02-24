//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ANGLEPerfTests:
//   Base class for google test performance tests
//

#ifndef PERF_TESTS_ANGLE_PERF_TEST_H_
#define PERF_TESTS_ANGLE_PERF_TEST_H_

#include <gtest/gtest.h>

#include <mutex>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#include "platform/PlatformMethods.h"
#include "test_utils/angle_test_configs.h"
#include "test_utils/angle_test_instantiate.h"
#include "test_utils/angle_test_platform.h"
#include "third_party/perf/perf_result_reporter.h"
#include "util/EGLWindow.h"
#include "util/OSWindow.h"
#include "util/Timer.h"
#include "util/util_gl.h"

class Event;

#if !defined(ASSERT_GL_NO_ERROR)
#    define ASSERT_GL_NO_ERROR() ASSERT_EQ(static_cast<GLenum>(GL_NO_ERROR), glGetError())
#endif  // !defined(ASSERT_GL_NO_ERROR)

#if !defined(ASSERT_GLENUM_EQ)
#    define ASSERT_GLENUM_EQ(expected, actual) \
        ASSERT_EQ(static_cast<GLenum>(expected), static_cast<GLenum>(actual))
#endif  // !defined(ASSERT_GLENUM_EQ)

// These are trace events according to Google's "Trace Event Format".
// See https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU
// Only a subset of the properties are implemented.
struct TraceEvent final
{
    TraceEvent() {}
    TraceEvent(char phaseIn,
               const char *categoryNameIn,
               const char *nameIn,
               double timestampIn,
               uint32_t tidIn);

    static constexpr uint32_t kMaxNameLen = 64;

    char phase               = 0;
    const char *categoryName = nullptr;
    char name[kMaxNameLen]   = {};
    double timestamp         = 0;
    uint32_t tid             = 0;
};

class ANGLEPerfTest : public testing::Test, angle::NonCopyable
{
  public:
    ANGLEPerfTest(const std::string &name,
                  const std::string &backend,
                  const std::string &story,
                  unsigned int iterationsPerStep,
                  const char *units = "ns");
    ~ANGLEPerfTest() override;

    virtual void step() = 0;

    // Called right after the timer starts to let the test initialize other metrics if necessary
    virtual void startTest() {}
    // Called right before timer is stopped to let the test wait for asynchronous operations.
    virtual void finishTest() {}
    virtual void flush() {}

    // Can be overridden in child tests that require a certain number of steps per trial.
    virtual int getStepAlignment() const;

    virtual bool isRenderTest() const { return false; }

  protected:
    enum class RunTrialPolicy
    {
        FinishEveryStep,
        RunContinuouslyWarmup,
        RunContinuously,
    };

    void run();
    void SetUp() override;
    void TearDown() override;

    // Normalize a time value according to the number of test trial iterations (mFrameCount)
    double normalizedTime(size_t value) const;

    // Call if the test step was aborted and the test should stop running.
    void abortTest() { mRunning = false; }

    int getNumStepsPerformed() const { return mTrialNumStepsPerformed; }

    void runTrial(double maxRunTime, int maxStepsToRun, RunTrialPolicy runPolicy);

    // Overriden in trace perf tests.
    virtual void computeGPUTime() {}

    void calibrateStepsToRun();
    int estimateStepsToRun() const;

    void recordIntegerMetric(const char *metric, size_t value, const std::string &units);
    void recordDoubleMetric(const char *metric, double value, const std::string &units);
    void addHistogramSample(const char *metric, double value, const std::string &units);

    void processResults();
    void processClockResult(const char *metric, double resultSeconds);
    void processMemoryResult(const char *metric, uint64_t resultKB);

    void skipTest(const std::string &reason)
    {
        mSkipTestReason = reason;
        mSkipTest       = true;
    }

    void failTest(const std::string &reason)
    {
        skipTest(reason);
        FAIL() << reason;
    }

    void atraceCounter(const char *counterName, int64_t counterValue);

    std::string mName;
    std::string mBackend;
    std::string mStory;
    Timer mTrialTimer;
    uint64_t mGPUTimeNs;
    bool mSkipTest;
    std::string mSkipTestReason;
    std::unique_ptr<perf_test::PerfResultReporter> mReporter;
    int mStepsToRun;
    int mTrialNumStepsPerformed;
    int mTotalNumStepsPerformed;
    int mIterationsPerStep;
    bool mRunning;
    std::vector<double> mTestTrialResults;

    struct CounterInfo
    {
        std::string name;
        std::vector<GLuint64> samples;
    };
    std::map<GLuint, CounterInfo> mPerfCounterInfo;
    GLuint mPerfMonitor;
    std::vector<uint64_t> mProcessMemoryUsageKBSamples;
};

enum class SurfaceType
{
    Window,
    WindowWithVSync,
    Offscreen,
};

struct RenderTestParams : public angle::PlatformParameters
{
    RenderTestParams();
    virtual ~RenderTestParams() {}

    virtual std::string backend() const;
    virtual std::string story() const;
    std::string backendAndStory() const;

    EGLint windowWidth             = 64;
    EGLint windowHeight            = 64;
    unsigned int iterationsPerStep = 0;
    bool trackGpuTime              = false;
    SurfaceType surfaceType        = SurfaceType::Window;
    EGLenum colorSpace             = EGL_COLORSPACE_LINEAR;
    bool multisample               = false;
    EGLint samples                 = -1;
};

class ANGLERenderTest : public ANGLEPerfTest
{
  public:
    ANGLERenderTest(const std::string &name,
                    const RenderTestParams &testParams,
                    const char *units = "ns");
    ~ANGLERenderTest() override;

    void addExtensionPrerequisite(std::string extensionName);
    void addIntegerPrerequisite(GLenum target, int min);

    virtual void initializeBenchmark() {}
    virtual void destroyBenchmark() {}

    virtual void drawBenchmark() = 0;

    bool popEvent(Event *event);

    OSWindow *getWindow();
    GLWindowBase *getGLWindow();

    std::vector<TraceEvent> &getTraceEventBuffer();

    virtual void overrideWorkaroundsD3D(angle::FeaturesD3D *featuresD3D) {}
    void onErrorMessage(const char *errorMessage);

    uint32_t getCurrentThreadSerial();
    std::mutex &getTraceEventMutex() { return mTraceEventMutex; }
    bool isRenderTest() const override { return true; }

  protected:
    const RenderTestParams &mTestParams;

    void setWebGLCompatibilityEnabled(bool webglCompatibility);
    void setRobustResourceInit(bool enabled);

    void startGpuTimer();
    void stopGpuTimer();

    void beginInternalTraceEvent(const char *name);
    void endInternalTraceEvent(const char *name);
    void beginGLTraceEvent(const char *name, double hostTimeSec);
    void endGLTraceEvent(const char *name, double hostTimeSec);

    void disableTestHarnessSwap() { mSwapEnabled = false; }
    void updatePerfCounters();

    bool mIsTimestampQueryAvailable;
    bool mEnableDebugCallback = true;

    void startTest() override;
    void finishTest() override;

  private:
    void SetUp() override;
    void TearDown() override;

    void step() override;
    void computeGPUTime() override;

    void skipTestIfMissingExtensionPrerequisites();
    void skipTestIfFailsIntegerPrerequisite();

    void initPerfCounters();

    GLWindowBase *mGLWindow;
    OSWindow *mOSWindow;
    std::vector<std::string> mExtensionPrerequisites;
    struct IntegerPrerequisite
    {
        GLenum target;
        int min;
    };
    std::vector<IntegerPrerequisite> mIntegerPrerequisites;
    angle::PlatformMethods mPlatformMethods;
    ConfigParameters mConfigParams;
    bool mSwapEnabled;

    struct TimestampSample
    {
        GLuint beginQuery;
        GLuint endQuery;
    };

    GLuint mCurrentTimestampBeginQuery = 0;
    std::queue<TimestampSample> mTimestampQueries;

    // Trace event record that can be output.
    std::vector<TraceEvent> mTraceEventBuffer;

    // Handle to the entry point binding library.
    std::unique_ptr<angle::Library> mEntryPointsLib;

    std::vector<uint64_t> mThreadIDs;
    std::mutex mTraceEventMutex;
};

// Mixins.
namespace params
{
template <typename ParamsT>
ParamsT Offscreen(const ParamsT &input)
{
    ParamsT output     = input;
    output.surfaceType = SurfaceType::Offscreen;
    return output;
}

template <typename ParamsT>
ParamsT NullDevice(const ParamsT &input)
{
    ParamsT output                  = input;
    output.eglParameters.deviceType = EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE;
    output.trackGpuTime             = false;
    return output;
}

template <typename ParamsT>
ParamsT Passthrough(const ParamsT &input)
{
    return input;
}
}  // namespace params

namespace angle
{
// Returns the time of the host since the application started in seconds.
double GetHostTimeSeconds();
}  // namespace angle
#endif  // PERF_TESTS_ANGLE_PERF_TEST_H_
