//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TestSuite_unittest.cpp: Unit tests for ANGLE's test harness.
//

#include <gtest/gtest.h>

#include "../angle_test_instantiate.h"
#include "TestSuite.h"
#include "common/debug.h"
#include "common/system_utils.h"
#include "util/test_utils.h"
#include "util/test_utils_unittest_helper.h"

#include <rapidjson/document.h>

using namespace angle;

namespace js = rapidjson;

// This file is included in both angle_unittests and test_utils_unittest_helper. This variable is
// defined separately in each test target's main file.
extern bool gVerbose;

namespace
{
constexpr char kTestHelperExecutable[] = "test_utils_unittest_helper";
constexpr int kFlakyRetries            = 3;

class TestSuiteTest : public testing::Test
{
  protected:
    void TearDown() override
    {
        if (!mTempFileName.empty())
        {
            angle::DeleteSystemFile(mTempFileName.c_str());
        }
    }

    bool runTestSuite(const std::vector<std::string> &extraArgs,
                      TestResults *actualResults,
                      bool validateStderr)
    {
        std::string executablePath = GetExecutableDirectory();
        EXPECT_NE(executablePath, "");
        executablePath += std::string("/") + kTestHelperExecutable + GetExecutableExtension();

        const Optional<std::string> tempDirName = GetTempDirectory();
        if (!tempDirName.valid())
        {
            return false;
        }

        Optional<std::string> tempFile = CreateTemporaryFileInDirectory(tempDirName.value());
        if (!tempFile.valid())
        {
            return false;
        }

        mTempFileName               = tempFile.value();
        std::string resultsFileName = "--results-file=" + mTempFileName;

        std::vector<const char *> args = {
            executablePath.c_str(), kRunTestSuite,      "--gtest_also_run_disabled_tests",
            "--bot-mode",           "--test-timeout=5", resultsFileName.c_str()};

        for (const std::string &arg : extraArgs)
        {
            args.push_back(arg.c_str());
        }

        if (gVerbose)
        {
            printf("Test arguments:\n");
            for (const char *arg : args)
            {
                printf("%s ", arg);
            }
            printf("\n");
        }

        ProcessHandle process(args, ProcessOutputCapture::StdoutAndStderrSeparately);
        EXPECT_TRUE(process->started());
        EXPECT_TRUE(process->finish());
        EXPECT_TRUE(process->finished());

        if (validateStderr)
        {
            EXPECT_EQ(process->getStderr(), "");
        }

        if (gVerbose)
        {
            printf("stdout:\n%s\n", process->getStdout().c_str());
        }

        return GetTestResultsFromFile(mTempFileName.c_str(), actualResults);
    }

    std::string mTempFileName;
};

// Tests the ANGLE standalone testing harness. Runs four tests with different ending conditions.
// Verifies that Pass, Fail, Crash and Timeout are all handled correctly.
TEST_F(TestSuiteTest, RunMockTests)
{
    std::vector<std::string> extraArgs = {"--gtest_filter=MockTestSuiteTest.DISABLED_*"};

    TestResults actual;
    ASSERT_TRUE(runTestSuite(extraArgs, &actual, true));

    std::map<TestIdentifier, TestResult> expectedResults = {
        {{"MockTestSuiteTest", "DISABLED_Pass"},
         {TestResultType::Pass, std::vector<double>({0.0})}},
        {{"MockTestSuiteTest", "DISABLED_Fail"},
         {TestResultType::Fail, std::vector<double>({0.0})}},
        {{"MockTestSuiteTest", "DISABLED_Skip"},
         {TestResultType::Skip, std::vector<double>({0.0})}},
        {{"MockTestSuiteTest", "DISABLED_Timeout"},
         {TestResultType::Timeout, std::vector<double>({0.0})}},
    };

    EXPECT_EQ(expectedResults, actual.results);
}

// Verifies the flaky retry feature works as expected.
TEST_F(TestSuiteTest, RunFlakyTests)
{
    std::vector<std::string> extraArgs = {"--gtest_filter=MockFlakyTestSuiteTest.DISABLED_Flaky",
                                          "--flaky-retries=" + std::to_string(kFlakyRetries)};

    TestResults actual;
    ASSERT_TRUE(runTestSuite(extraArgs, &actual, true));

    std::vector<double> times;
    for (int i = 0; i < kFlakyRetries; i++)
    {
        times.push_back(0.0);
    }
    std::map<TestIdentifier, TestResult> expectedResults = {
        {{"MockFlakyTestSuiteTest", "DISABLED_Flaky"},
         {TestResultType::Pass, times, kFlakyRetries - 1}}};

    EXPECT_EQ(expectedResults, actual.results);
}

// Verifies that crashes are handled even without the crash handler.
TEST_F(TestSuiteTest, RunCrashingTests)
{
    std::vector<std::string> extraArgs = {
        "--gtest_filter=MockTestSuiteTest.DISABLED_Pass:MockTestSuiteTest.DISABLED_Fail:"
        "MockTestSuiteTest.DISABLED_Skip:"
        "MockCrashTestSuiteTest.DISABLED_*",
        "--disable-crash-handler"};

    TestResults actual;
    ASSERT_TRUE(runTestSuite(extraArgs, &actual, false));

    std::map<TestIdentifier, TestResult> expectedResults = {
        {{"MockTestSuiteTest", "DISABLED_Pass"},
         {TestResultType::Pass, std::vector<double>({0.0})}},
        {{"MockTestSuiteTest", "DISABLED_Fail"},
         {TestResultType::Fail, std::vector<double>({0.0})}},
        {{"MockTestSuiteTest", "DISABLED_Skip"},
         {TestResultType::Skip, std::vector<double>({0.0})}},
        {{"MockCrashTestSuiteTest", "DISABLED_Crash"},
         {TestResultType::Crash, std::vector<double>({0.0})}},
        {{"MockCrashTestSuiteTest", "DISABLED_PassAfterCrash"},
         {TestResultType::Pass, std::vector<double>({0.0})}},
        {{"MockCrashTestSuiteTest", "DISABLED_SkipAfterCrash"},
         {TestResultType::Skip, std::vector<double>({0.0})}},
    };

    EXPECT_EQ(expectedResults, actual.results);
}

// Normal passing test.
TEST(MockTestSuiteTest, DISABLED_Pass)
{
    EXPECT_TRUE(true);
}

// Normal failing test.
TEST(MockTestSuiteTest, DISABLED_Fail)
{
    EXPECT_TRUE(false);
}

// Trigger a test timeout.
TEST(MockTestSuiteTest, DISABLED_Timeout)
{
    angle::Sleep(20000);
}

// Trigger a test skip.
TEST(MockTestSuiteTest, DISABLED_Skip)
{
    GTEST_SKIP() << "Test skipped.";
}

// Trigger a flaky test failure.
TEST(MockFlakyTestSuiteTest, DISABLED_Flaky)
{
    const Optional<std::string> tempDirName = GetTempDirectory();
    ASSERT_TRUE(tempDirName.valid());

    std::stringstream tempFNameStream;
    tempFNameStream << tempDirName.value() << GetPathSeparator() << "flaky_temp.txt";
    std::string tempFileName = tempFNameStream.str();

    int fails = 0;
    {
        FILE *fp = fopen(tempFileName.c_str(), "r");
        if (fp)
        {
            ASSERT_EQ(fscanf(fp, "%d", &fails), 1);
            fclose(fp);
        }
    }

    if (fails >= kFlakyRetries - 1)
    {
        angle::DeleteSystemFile(tempFileName.c_str());
    }
    else
    {
        EXPECT_TRUE(false);

        FILE *fp = fopen(tempFileName.c_str(), "w");
        ASSERT_NE(fp, nullptr);

        fprintf(fp, "%d", fails + 1);
        fclose(fp);
    }
}

// Trigger a test crash.
TEST(MockCrashTestSuiteTest, DISABLED_Crash)
{
    ANGLE_CRASH();
}

// This test runs after the crash test.
TEST(MockCrashTestSuiteTest, DISABLED_PassAfterCrash)
{
    EXPECT_TRUE(true);
}

// This test runs after the crash test.
TEST(MockCrashTestSuiteTest, DISABLED_SkipAfterCrash)
{
    GTEST_SKIP() << "Test skipped.";
}
}  // namespace
