//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// test_utils_unittest.cpp: Unit tests for ANGLE's test utility functions

#include "gtest/gtest.h"

#include "common/system_utils.h"
#include "tests/test_utils/runner/TestSuite.h"
#include "util/Timer.h"
#include "util/test_utils.h"
#include "util/test_utils_unittest_helper.h"

using namespace angle;

namespace
{
#if defined(ANGLE_PLATFORM_WINDOWS)
constexpr char kRunAppHelperExecutable[] = "test_utils_unittest_helper.exe";
#elif ANGLE_PLATFORM_IOS_FAMILY
constexpr char kRunAppHelperExecutable[] =
    "../test_utils_unittest_helper.app/test_utils_unittest_helper";
#else
constexpr char kRunAppHelperExecutable[] = "test_utils_unittest_helper";
#endif

// Transforms various line endings into C/Unix line endings:
//
// - A\nB -> A\nB
// - A\rB -> A\nB
// - A\r\nB -> A\nB
std::string NormalizeNewLines(const std::string &str)
{
    std::string result;

    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == '\r')
        {
            if (i + 1 < str.size() && str[i + 1] == '\n')
            {
                ++i;
            }
            result += '\n';
        }
        else
        {
            result += str[i];
        }
    }

    return result;
}

// Tests that Sleep() actually waits some time.
TEST(TestUtils, Sleep)
{
    Timer timer;
    timer.start();
    angle::Sleep(500);
    timer.stop();

    // Use a slightly fuzzy range
    EXPECT_GT(timer.getElapsedWallClockTime(), 0.48);
}

// TODO: android support. http://anglebug.com/40096559
#if defined(ANGLE_PLATFORM_ANDROID)
#    define MAYBE_RunApp DISABLED_RunApp
#    define MAYBE_RunAppAsync DISABLED_RunAppAsync
#    define MAYBE_RunAppAsyncRedirectStderrToStdout DISABLED_RunAppAsyncRedirectStderrToStdout
// TODO: fuchsia support. http://anglebug.com/42265786
#elif defined(ANGLE_PLATFORM_FUCHSIA)
#    define MAYBE_RunApp DISABLED_RunApp
#    define MAYBE_RunAppAsync DISABLED_RunAppAsync
#    define MAYBE_RunAppAsyncRedirectStderrToStdout DISABLED_RunAppAsyncRedirectStderrToStdout
// TODO: iOS simulator support. http://anglebug.com/42266562
#elif ANGLE_PLATFORM_IOS_FAMILY_SIMULATOR
#    define MAYBE_RunApp DISABLED_RunApp
#    define MAYBE_RunAppAsync DISABLED_RunAppAsync
#    define MAYBE_RunAppAsyncRedirectStderrToStdout DISABLED_RunAppAsyncRedirectStderrToStdout
#else
#    define MAYBE_RunApp RunApp
#    define MAYBE_RunAppAsync RunAppAsync
#    define MAYBE_RunAppAsyncRedirectStderrToStdout RunAppAsyncRedirectStderrToStdout
#endif  // defined(ANGLE_PLATFORM_ANDROID)

std::string GetTestAppExecutablePath()
{
    std::string testExecutableName = angle::TestSuite::GetInstance()->getTestExecutableName();
    std::string executablePath     = angle::StripFilenameFromPath(testExecutableName);

    EXPECT_NE(executablePath, "");
    executablePath += "/";
    executablePath += kRunAppHelperExecutable;

    return executablePath;
}

// Test running an external application and receiving its output
TEST(TestUtils, MAYBE_RunApp)
{
    std::string executablePath = GetTestAppExecutablePath();

    std::vector<const char *> args = {executablePath.c_str(), kRunAppTestArg1, kRunAppTestArg2};

    // Test that the application can be executed.
    {
        ProcessHandle process(args, ProcessOutputCapture::StdoutAndStderrSeparately);
        EXPECT_TRUE(process->started());
        EXPECT_TRUE(process->finish());
        EXPECT_TRUE(process->finished());

        EXPECT_GT(process->getElapsedTimeSeconds(), 0.0);
        EXPECT_EQ(kRunAppTestStdout, NormalizeNewLines(process->getStdout()));
        EXPECT_EQ(kRunAppTestStderr, NormalizeNewLines(process->getStderr()));
        EXPECT_EQ(EXIT_SUCCESS, process->getExitCode());
    }

    // Test that environment variables reach the child.
    {
        bool setEnvDone = SetEnvironmentVar(kRunAppTestEnvVarName, kRunAppTestEnvVarValue);
        EXPECT_TRUE(setEnvDone);

        ProcessHandle process(LaunchProcess(args, ProcessOutputCapture::StdoutAndStderrSeparately));
        EXPECT_TRUE(process->started());
        EXPECT_TRUE(process->finish());

        EXPECT_GT(process->getElapsedTimeSeconds(), 0.0);
        EXPECT_EQ("", process->getStdout());
        EXPECT_EQ(kRunAppTestEnvVarValue, NormalizeNewLines(process->getStderr()));
        EXPECT_EQ(EXIT_SUCCESS, process->getExitCode());

        // Unset environment var.
        SetEnvironmentVar(kRunAppTestEnvVarName, "");
    }
}

// Test running an external application and receiving its output asynchronously.
TEST(TestUtils, MAYBE_RunAppAsync)
{
    std::string executablePath = GetTestAppExecutablePath();

    std::vector<const char *> args = {executablePath.c_str(), kRunAppTestArg1, kRunAppTestArg2};

    // Test that the application can be executed and the output is captured correctly.
    {
        ProcessHandle process(args, ProcessOutputCapture::StdoutAndStderrSeparately);
        EXPECT_TRUE(process->started());

        constexpr double kTimeout = 3.0;

        Timer timer;
        timer.start();
        while (!process->finished() && timer.getElapsedWallClockTime() < kTimeout)
        {
            angle::Sleep(1);
        }

        EXPECT_TRUE(process->finished());
        EXPECT_GT(process->getElapsedTimeSeconds(), 0.0);
        EXPECT_EQ(kRunAppTestStdout, NormalizeNewLines(process->getStdout()));
        EXPECT_EQ(kRunAppTestStderr, NormalizeNewLines(process->getStderr()));
        EXPECT_EQ(EXIT_SUCCESS, process->getExitCode());
    }
}

// Test running an external application and receiving its stdout and stderr output interleaved.
TEST(TestUtils, MAYBE_RunAppAsyncRedirectStderrToStdout)
{
    std::string executablePath = GetTestAppExecutablePath();

    std::vector<const char *> args = {executablePath.c_str(), kRunAppTestArg1, kRunAppTestArg2};

    // Test that the application can be executed and the output is captured correctly.
    {
        ProcessHandle process(args, ProcessOutputCapture::StdoutAndStderrInterleaved);
        EXPECT_TRUE(process->started());

        constexpr double kTimeout = 3.0;

        Timer timer;
        timer.start();
        while (!process->finished() && timer.getElapsedWallClockTime() < kTimeout)
        {
            angle::Sleep(1);
        }

        EXPECT_TRUE(process->finished());
        EXPECT_GT(process->getElapsedTimeSeconds(), 0.0);
        EXPECT_EQ(std::string(kRunAppTestStdout) + kRunAppTestStderr,
                  NormalizeNewLines(process->getStdout()));
        EXPECT_EQ("", process->getStderr());
        EXPECT_EQ(EXIT_SUCCESS, process->getExitCode());
    }
}

// Verify that NumberOfProcessors returns something reasonable.
TEST(TestUtils, NumberOfProcessors)
{
    int numProcs = angle::NumberOfProcessors();
    EXPECT_GT(numProcs, 0);
    EXPECT_LT(numProcs, 1000);
}
}  // namespace
