//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ANGLEPerfTestArgs.cpp:
//   Parse command line arguments for angle_perftests.
//

#include "ANGLEPerfTestArgs.h"
#include <string.h>
#include <sstream>

#include "common/debug.h"
#include "util/test_utils.h"

#if defined(ANGLE_PLATFORM_ANDROID)
#    include "util/android/AndroidWindow.h"
#endif

namespace angle
{

constexpr int kDefaultStepsPerTrial    = std::numeric_limits<int>::max();
constexpr int kDefaultTrialTimeSeconds = 0;
constexpr int kDefaultTestTrials       = 3;

int gStepsPerTrial                 = kDefaultStepsPerTrial;
int gMaxStepsPerformed             = kDefaultMaxStepsPerformed;
bool gEnableTrace                  = false;
const char *gTraceFile             = "ANGLETrace.json";
const char *gScreenshotDir         = nullptr;
const char *gRenderTestOutputDir   = nullptr;
bool gSaveScreenshots              = false;
int gScreenshotFrame               = kDefaultScreenshotFrame;
bool gVerboseLogging               = false;
bool gWarmup                       = false;
int gTrialTimeSeconds              = kDefaultTrialTimeSeconds;
int gTestTrials                    = kDefaultTestTrials;
bool gNoFinish                     = false;
bool gRetraceMode                  = false;
bool gMinimizeGPUWork              = false;
bool gTraceTestValidation          = false;
const char *gPerfCounters          = nullptr;
const char *gUseANGLE              = nullptr;
const char *gUseGL                 = nullptr;
bool gOffscreen                    = false;
bool gVsync                        = false;
int gFpsLimit                      = 0;
bool gRunToKeyFrame                = false;
int gFixedTestTime                 = 0;
int gFixedTestTimeWithWarmup       = 0;
const char *gTraceInterpreter      = nullptr;
const char *gPrintExtensionsToFile = nullptr;
const char *gRequestedExtensions   = nullptr;
bool gIncludeInactiveResources     = false;

namespace
{
bool PerfTestArg(int *argc, char **argv, int argIndex)
{
    return ParseFlag("--run-to-key-frame", argc, argv, argIndex, &gRunToKeyFrame) ||
           ParseFlag("--enable-trace", argc, argv, argIndex, &gEnableTrace) ||
           ParseFlag("-v", argc, argv, argIndex, &gVerboseLogging) ||
           ParseFlag("--verbose", argc, argv, argIndex, &gVerboseLogging) ||
           ParseFlag("--verbose-logging", argc, argv, argIndex, &gVerboseLogging) ||
           ParseFlag("--no-finish", argc, argv, argIndex, &gNoFinish) ||
           ParseFlag("--warmup", argc, argv, argIndex, &gWarmup) ||
           ParseCStringArg("--trace-file", argc, argv, argIndex, &gTraceFile) ||
           ParseCStringArg("--perf-counters", argc, argv, argIndex, &gPerfCounters) ||
           ParseIntArg("--steps-per-trial", argc, argv, argIndex, &gStepsPerTrial) ||
           ParseIntArg("--max-steps-performed", argc, argv, argIndex, &gMaxStepsPerformed) ||
           ParseIntArg("--fixed-test-time", argc, argv, argIndex, &gFixedTestTime) ||
           ParseIntArg("--fixed-test-time-with-warmup", argc, argv, argIndex,
                       &gFixedTestTimeWithWarmup) ||
           ParseIntArg("--trial-time", argc, argv, argIndex, &gTrialTimeSeconds) ||
           ParseIntArg("--max-trial-time", argc, argv, argIndex, &gTrialTimeSeconds) ||
           ParseIntArg("--trials", argc, argv, argIndex, &gTestTrials);
}

bool TraceTestArg(int *argc, char **argv, int argIndex)
{
    return ParseFlag("--retrace-mode", argc, argv, argIndex, &gRetraceMode) ||
           ParseFlag("--validation", argc, argv, argIndex, &gTraceTestValidation) ||
           ParseFlag("--save-screenshots", argc, argv, argIndex, &gSaveScreenshots) ||
           ParseFlag("--offscreen", argc, argv, argIndex, &gOffscreen) ||
           ParseFlag("--vsync", argc, argv, argIndex, &gVsync) ||
           ParseFlag("--minimize-gpu-work", argc, argv, argIndex, &gMinimizeGPUWork) ||
           ParseCStringArg("--trace-interpreter", argc, argv, argIndex, &gTraceInterpreter) ||
           ParseIntArg("--screenshot-frame", argc, argv, argIndex, &gScreenshotFrame) ||
           ParseIntArg("--fps-limit", argc, argv, argIndex, &gFpsLimit) ||
           ParseCStringArgWithHandling("--render-test-output-dir", argc, argv, argIndex,
                                       &gRenderTestOutputDir, ArgHandling::Preserve) ||
           ParseCStringArg("--screenshot-dir", argc, argv, argIndex, &gScreenshotDir) ||
           ParseCStringArg("--use-angle", argc, argv, argIndex, &gUseANGLE) ||
           ParseCStringArg("--use-gl", argc, argv, argIndex, &gUseGL) ||
           ParseCStringArg("--print-extensions-to-file", argc, argv, argIndex,
                           &gPrintExtensionsToFile) ||
           ParseCStringArg("--request-extensions", argc, argv, argIndex, &gRequestedExtensions) ||
           ParseFlag("--include-inactive-resources", argc, argv, argIndex,
                     &gIncludeInactiveResources);
}
}  // namespace
}  // namespace angle

using namespace angle;

void ANGLEProcessPerfTestArgs(int *argc, char **argv)
{
    for (int argIndex = 1; argIndex < *argc;)
    {
        if (!PerfTestArg(argc, argv, argIndex))
        {
            argIndex++;
        }
    }

    if (gRunToKeyFrame || gMaxStepsPerformed > 0)
    {
        // Ensure defaults were provided for params we're about to set
        ASSERT(gTestTrials == kDefaultTestTrials && gTrialTimeSeconds == kDefaultTrialTimeSeconds);

        gTestTrials       = 1;
        gTrialTimeSeconds = 36000;
    }

    if (gFixedTestTime != 0)
    {
        // Ensure defaults were provided for params we're about to set
        ASSERT(gTrialTimeSeconds == kDefaultTrialTimeSeconds &&
               gStepsPerTrial == kDefaultStepsPerTrial && gTestTrials == kDefaultTestTrials);

        gTrialTimeSeconds = gFixedTestTime;
        gStepsPerTrial    = std::numeric_limits<int>::max();
        gTestTrials       = 1;
    }

    if (gFixedTestTimeWithWarmup != 0)
    {
        // Ensure defaults were provided for params we're about to set
        ASSERT(gTrialTimeSeconds == kDefaultTrialTimeSeconds &&
               gStepsPerTrial == kDefaultStepsPerTrial && gTestTrials == kDefaultTestTrials);

        // This option is primarily useful for trace replays when you want to iterate once through
        // the trace to warm caches, then run for a fixed amount of time. It is equivalent to:
        // --trial-time X --steps-per-trial INF --trials 1 --warmup
        gTrialTimeSeconds = gFixedTestTimeWithWarmup;
        gStepsPerTrial    = std::numeric_limits<int>::max();
        gTestTrials       = 1;
        gWarmup           = true;
    }

    if (gTrialTimeSeconds == 0)
    {
        gTrialTimeSeconds = 10;
    }
}

void ANGLEProcessTraceTestArgs(int *argc, char **argv)
{
    ANGLEProcessPerfTestArgs(argc, argv);

    for (int argIndex = 1; argIndex < *argc;)
    {
        if (!TraceTestArg(argc, argv, argIndex))
        {
            argIndex++;
        }
    }

    if (gScreenshotDir)
    {
        // implicitly set here but not when using kRenderTestOutputDir
        gSaveScreenshots = true;
    }

    if (gRenderTestOutputDir)
    {
        gScreenshotDir = gRenderTestOutputDir;
    }

    if (gTraceTestValidation)
    {
        gTestTrials       = 1;
        gTrialTimeSeconds = 600;
    }

    if (kStandaloneBenchmark)
    {
        gVerboseLogging = true;
#if defined(ANGLE_PLATFORM_ANDROID)
        gScreenshotDir = strdup((AndroidWindow::GetApplicationDirectory() + "/files").c_str());
#else
        gScreenshotDir = ".";
#endif
        gSaveScreenshots = true;
        gUseANGLE        = "vulkan";
    }
}
