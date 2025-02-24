//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TestSuite:
//   Basic implementation of a test harness in ANGLE.

#include "TestSuite.h"

#include "common/debug.h"
#include "common/hash_containers.h"
#include "common/platform.h"
#include "common/string_utils.h"
#include "common/system_utils.h"
#include "util/Timer.h"

#include <stdlib.h>
#include <time.h>

#include <fstream>
#include <unordered_map>

#include <gtest/gtest.h>
#include <rapidjson/document.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/prettywriter.h>

// We directly call into a function to register the parameterized tests. This saves spinning up
// a subprocess with a new gtest filter.
#include <gtest/../../src/gtest-internal-inl.h>

namespace js = rapidjson;

namespace angle
{
namespace
{
constexpr char kBatchId[]             = "--batch-id";
constexpr char kFilterFileArg[]       = "--filter-file";
constexpr char kResultFileArg[]       = "--results-file";
constexpr char kTestTimeoutArg[]      = "--test-timeout";
constexpr char kDisableCrashHandler[] = "--disable-crash-handler";
constexpr char kIsolatedOutDir[]      = "--isolated-outdir";

constexpr char kStartedTestString[] = "[ RUN      ] ";
constexpr char kPassedTestString[]  = "[       OK ] ";
constexpr char kFailedTestString[]  = "[  FAILED  ] ";
constexpr char kSkippedTestString[] = "[  SKIPPED ] ";

constexpr char kArtifactsFakeTestName[] = "TestArtifactsFakeTest";

constexpr char kTSanOptionsEnvVar[]  = "TSAN_OPTIONS";
constexpr char kUBSanOptionsEnvVar[] = "UBSAN_OPTIONS";

[[maybe_unused]] constexpr char kVkLoaderDisableDLLUnloadingEnvVar[] =
    "VK_LOADER_DISABLE_DYNAMIC_LIBRARY_UNLOADING";

// Note: we use a fairly high test timeout to allow for the first test in a batch to be slow.
// Ideally we could use a separate timeout for the slow first test.
// Allow sanitized tests to run more slowly.
#if defined(NDEBUG) && !defined(ANGLE_WITH_SANITIZER)
constexpr int kDefaultTestTimeout  = 60;
constexpr int kDefaultBatchTimeout = 300;
#else
constexpr int kDefaultTestTimeout  = 120;
constexpr int kDefaultBatchTimeout = 700;
#endif
constexpr int kSlowTestTimeoutScale  = 3;
constexpr int kDefaultBatchSize      = 256;
constexpr double kIdleMessageTimeout = 15.0;
constexpr int kDefaultMaxProcesses   = 16;
constexpr int kDefaultMaxFailures    = 100;

const char *ResultTypeToString(TestResultType type)
{
    switch (type)
    {
        case TestResultType::Crash:
            return "CRASH";
        case TestResultType::Fail:
            return "FAIL";
        case TestResultType::NoResult:
            return "NOTRUN";
        case TestResultType::Pass:
            return "PASS";
        case TestResultType::Skip:
            return "SKIP";
        case TestResultType::Timeout:
            return "TIMEOUT";
        case TestResultType::Unknown:
        default:
            return "UNKNOWN";
    }
}

TestResultType GetResultTypeFromString(const std::string &str)
{
    if (str == "CRASH")
        return TestResultType::Crash;
    if (str == "FAIL")
        return TestResultType::Fail;
    if (str == "PASS")
        return TestResultType::Pass;
    if (str == "NOTRUN")
        return TestResultType::NoResult;
    if (str == "SKIP")
        return TestResultType::Skip;
    if (str == "TIMEOUT")
        return TestResultType::Timeout;
    return TestResultType::Unknown;
}

bool IsFailedResult(TestResultType resultType)
{
    return resultType != TestResultType::Pass && resultType != TestResultType::Skip;
}

js::Value ResultTypeToJSString(TestResultType type, js::Document::AllocatorType *allocator)
{
    js::Value jsName;
    jsName.SetString(ResultTypeToString(type), *allocator);
    return jsName;
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

// Writes out a TestResults to the Chromium JSON Test Results format.
// https://chromium.googlesource.com/chromium/src.git/+/main/docs/testing/json_test_results_format.md
void WriteResultsFile(bool interrupted,
                      const TestResults &testResults,
                      const std::string &outputFile)
{
    time_t ltime;
    time(&ltime);
    struct tm *timeinfo = gmtime(&ltime);
    ltime               = mktime(timeinfo);

    uint64_t secondsSinceEpoch = static_cast<uint64_t>(ltime);

    js::Document doc;
    doc.SetObject();

    js::Document::AllocatorType &allocator = doc.GetAllocator();

    doc.AddMember("interrupted", interrupted, allocator);
    doc.AddMember("path_delimiter", ".", allocator);
    doc.AddMember("version", 3, allocator);
    doc.AddMember("seconds_since_epoch", secondsSinceEpoch, allocator);

    js::Value tests;
    tests.SetObject();

    // If we have any test artifacts, make a fake test to house them.
    if (!testResults.testArtifactPaths.empty())
    {
        js::Value artifactsTest;
        artifactsTest.SetObject();

        artifactsTest.AddMember("actual", "PASS", allocator);
        artifactsTest.AddMember("expected", "PASS", allocator);

        js::Value artifacts;
        artifacts.SetObject();

        for (const std::string &testArtifactPath : testResults.testArtifactPaths)
        {
            std::vector<std::string> pieces =
                SplitString(testArtifactPath, "/\\", WhitespaceHandling::TRIM_WHITESPACE,
                            SplitResult::SPLIT_WANT_NONEMPTY);
            ASSERT(!pieces.empty());

            js::Value basename;
            basename.SetString(pieces.back(), allocator);

            js::Value artifactPath;
            artifactPath.SetString(testArtifactPath, allocator);

            js::Value artifactArray;
            artifactArray.SetArray();
            artifactArray.PushBack(artifactPath, allocator);

            artifacts.AddMember(basename, artifactArray, allocator);
        }

        artifactsTest.AddMember("artifacts", artifacts, allocator);

        js::Value fakeTestName;
        fakeTestName.SetString(testResults.testArtifactsFakeTestName, allocator);
        tests.AddMember(fakeTestName, artifactsTest, allocator);
    }

    std::map<TestResultType, uint32_t> counts;

    for (const auto &resultIter : testResults.results)
    {
        const TestIdentifier &id = resultIter.first;
        const TestResult &result = resultIter.second;

        js::Value jsResult;
        jsResult.SetObject();

        counts[result.type]++;

        std::string actualResult;
        for (uint32_t fail = 0; fail < result.flakyFailures; ++fail)
        {
            actualResult += "FAIL ";
        }

        actualResult += ResultTypeToString(result.type);

        std::string expectedResult = "PASS";
        if (result.type == TestResultType::Skip)
        {
            expectedResult = "SKIP";
        }

        // Handle flaky passing tests.
        if (result.flakyFailures > 0 && result.type == TestResultType::Pass)
        {
            expectedResult = "FAIL PASS";
            jsResult.AddMember("is_flaky", true, allocator);
        }

        jsResult.AddMember("actual", actualResult, allocator);
        jsResult.AddMember("expected", expectedResult, allocator);

        if (IsFailedResult(result.type))
        {
            jsResult.AddMember("is_unexpected", true, allocator);
        }

        js::Value times;
        times.SetArray();
        for (double elapsedTimeSeconds : result.elapsedTimeSeconds)
        {
            times.PushBack(elapsedTimeSeconds, allocator);
        }

        jsResult.AddMember("times", times, allocator);

        char testName[500];
        id.snprintfName(testName, sizeof(testName));
        js::Value jsName;
        jsName.SetString(testName, allocator);

        tests.AddMember(jsName, jsResult, allocator);
    }

    js::Value numFailuresByType;
    numFailuresByType.SetObject();

    for (const auto &countIter : counts)
    {
        TestResultType type = countIter.first;
        uint32_t count      = countIter.second;

        js::Value jsCount(count);
        numFailuresByType.AddMember(ResultTypeToJSString(type, &allocator), jsCount, allocator);
    }

    doc.AddMember("num_failures_by_type", numFailuresByType, allocator);

    doc.AddMember("tests", tests, allocator);

    printf("Writing test results to %s\n", outputFile.c_str());

    if (!WriteJsonFile(outputFile, &doc))
    {
        printf("Error writing test results file.\n");
    }
}

void WriteHistogramJson(const HistogramWriter &histogramWriter, const std::string &outputFile)
{
    js::Document doc;
    doc.SetArray();

    histogramWriter.getAsJSON(&doc);

    printf("Writing histogram json to %s\n", outputFile.c_str());

    if (!WriteJsonFile(outputFile, &doc))
    {
        printf("Error writing histogram json file.\n");
    }
}

void UpdateCurrentTestResult(const testing::TestResult &resultIn, TestResults *resultsOut)
{
    TestResult &resultOut = resultsOut->results[resultsOut->currentTest];

    // Note: Crashes and Timeouts are detected by the crash handler and a watchdog thread.
    if (resultIn.Skipped())
    {
        resultOut.type = TestResultType::Skip;
    }
    else if (resultIn.Failed())
    {
        resultOut.type = TestResultType::Fail;
    }
    else
    {
        resultOut.type = TestResultType::Pass;
    }

    resultOut.elapsedTimeSeconds.back() = resultsOut->currentTestTimer.getElapsedWallClockTime();
}

TestIdentifier GetTestIdentifier(const testing::TestInfo &testInfo)
{
    return {testInfo.test_suite_name(), testInfo.name()};
}

bool IsTestDisabled(const testing::TestInfo &testInfo)
{
    return ::strstr(testInfo.name(), "DISABLED_") == testInfo.name();
}

using TestIdentifierFilter = std::function<bool(const TestIdentifier &id)>;

std::vector<TestIdentifier> FilterTests(std::map<TestIdentifier, FileLine> *fileLinesOut,
                                        TestIdentifierFilter filter,
                                        bool alsoRunDisabledTests)
{
    std::vector<TestIdentifier> tests;

    const testing::UnitTest &testProgramInfo = *testing::UnitTest::GetInstance();
    for (int suiteIndex = 0; suiteIndex < testProgramInfo.total_test_suite_count(); ++suiteIndex)
    {
        const testing::TestSuite &testSuite = *testProgramInfo.GetTestSuite(suiteIndex);
        for (int testIndex = 0; testIndex < testSuite.total_test_count(); ++testIndex)
        {
            const testing::TestInfo &testInfo = *testSuite.GetTestInfo(testIndex);
            TestIdentifier id                 = GetTestIdentifier(testInfo);
            if (filter(id) && (!IsTestDisabled(testInfo) || alsoRunDisabledTests))
            {
                tests.emplace_back(id);

                if (fileLinesOut)
                {
                    (*fileLinesOut)[id] = {testInfo.file(), testInfo.line()};
                }
            }
        }
    }

    return tests;
}

std::vector<TestIdentifier> GetFilteredTests(std::map<TestIdentifier, FileLine> *fileLinesOut,
                                             bool alsoRunDisabledTests)
{
    TestIdentifierFilter gtestIDFilter = [](const TestIdentifier &id) {
        return testing::internal::UnitTestOptions::FilterMatchesTest(id.testSuiteName, id.testName);
    };

    return FilterTests(fileLinesOut, gtestIDFilter, alsoRunDisabledTests);
}

std::vector<TestIdentifier> GetShardTests(const std::vector<TestIdentifier> &allTests,
                                          int shardIndex,
                                          int shardCount,
                                          std::map<TestIdentifier, FileLine> *fileLinesOut,
                                          bool alsoRunDisabledTests)
{
    std::vector<TestIdentifier> shardTests;

    for (int testIndex = shardIndex; testIndex < static_cast<int>(allTests.size());
         testIndex += shardCount)
    {
        shardTests.emplace_back(allTests[testIndex]);
    }

    return shardTests;
}

std::string GetTestFilter(const std::vector<TestIdentifier> &tests)
{
    std::stringstream filterStream;

    filterStream << "--gtest_filter=";

    for (size_t testIndex = 0; testIndex < tests.size(); ++testIndex)
    {
        if (testIndex != 0)
        {
            filterStream << ":";
        }

        filterStream << ReplaceDashesWithQuestionMark(tests[testIndex].testSuiteName) << "."
                     << ReplaceDashesWithQuestionMark(tests[testIndex].testName);
    }

    return filterStream.str();
}

bool GetTestArtifactsFromJSON(const js::Value::ConstObject &obj,
                              std::vector<std::string> *testArtifactPathsOut)
{
    if (!obj.HasMember("artifacts"))
    {
        printf("No artifacts member.\n");
        return false;
    }

    const js::Value &jsArtifacts = obj["artifacts"];
    if (!jsArtifacts.IsObject())
    {
        printf("Artifacts are not an object.\n");
        return false;
    }

    const js::Value::ConstObject &artifacts = jsArtifacts.GetObj();
    for (const auto &artifactMember : artifacts)
    {
        const js::Value &artifact = artifactMember.value;
        if (!artifact.IsArray())
        {
            printf("Artifact is not an array of strings of size 1.\n");
            return false;
        }

        const js::Value::ConstArray &artifactArray = artifact.GetArray();
        if (artifactArray.Size() != 1)
        {
            printf("Artifact is not an array of strings of size 1.\n");
            return false;
        }

        const js::Value &artifactName = artifactArray[0];
        if (!artifactName.IsString())
        {
            printf("Artifact is not an array of strings of size 1.\n");
            return false;
        }

        testArtifactPathsOut->push_back(artifactName.GetString());
    }

    return true;
}

bool GetSingleTestResultFromJSON(const js::Value &name,
                                 const js::Value::ConstObject &obj,
                                 TestResults *resultsOut)
{

    TestIdentifier id;
    if (!TestIdentifier::ParseFromString(name.GetString(), &id))
    {
        printf("Could not parse test identifier.\n");
        return false;
    }

    if (!obj.HasMember("expected") || !obj.HasMember("actual"))
    {
        printf("No expected or actual member.\n");
        return false;
    }

    const js::Value &expected = obj["expected"];
    const js::Value &actual   = obj["actual"];

    if (!expected.IsString() || !actual.IsString())
    {
        printf("Expected or actual member is not a string.\n");
        return false;
    }

    const std::string actualStr = actual.GetString();

    TestResultType resultType = TestResultType::Unknown;
    int flakyFailures         = 0;
    if (actualStr.find(' '))
    {
        std::istringstream strstr(actualStr);
        std::string token;
        while (std::getline(strstr, token, ' '))
        {
            resultType = GetResultTypeFromString(token);
            if (resultType == TestResultType::Unknown)
            {
                printf("Failed to parse result type.\n");
                return false;
            }
            if (IsFailedResult(resultType))
            {
                flakyFailures++;
            }
        }
    }
    else
    {
        resultType = GetResultTypeFromString(actualStr);
        if (resultType == TestResultType::Unknown)
        {
            printf("Failed to parse result type.\n");
            return false;
        }
    }

    std::vector<double> elapsedTimeSeconds;
    if (obj.HasMember("times"))
    {
        const js::Value &times = obj["times"];
        if (!times.IsArray())
        {
            return false;
        }

        const js::Value::ConstArray &timesArray = times.GetArray();
        if (timesArray.Size() < 1)
        {
            return false;
        }
        for (const js::Value &time : timesArray)
        {
            if (!time.IsDouble())
            {
                return false;
            }

            elapsedTimeSeconds.push_back(time.GetDouble());
        }
    }

    TestResult &result        = resultsOut->results[id];
    result.elapsedTimeSeconds = elapsedTimeSeconds;
    result.type               = resultType;
    result.flakyFailures      = flakyFailures;
    return true;
}

bool GetTestResultsFromJSON(const js::Document &document, TestResults *resultsOut)
{
    if (!document.HasMember("tests") || !document["tests"].IsObject())
    {
        printf("JSON document has no tests member.\n");
        return false;
    }

    const js::Value::ConstObject &tests = document["tests"].GetObj();
    for (const auto &testMember : tests)
    {
        // Get test identifier.
        const js::Value &name = testMember.name;
        if (!name.IsString())
        {
            printf("Name is not a string.\n");
            return false;
        }

        // Get test result.
        const js::Value &value = testMember.value;
        if (!value.IsObject())
        {
            printf("Test result is not an object.\n");
            return false;
        }

        const js::Value::ConstObject &obj = value.GetObj();

        if (BeginsWith(name.GetString(), kArtifactsFakeTestName))
        {
            if (!GetTestArtifactsFromJSON(obj, &resultsOut->testArtifactPaths))
            {
                return false;
            }
        }
        else
        {
            if (!GetSingleTestResultFromJSON(name, obj, resultsOut))
            {
                return false;
            }
        }
    }

    return true;
}

bool MergeTestResults(TestResults *input, TestResults *output, int flakyRetries)
{
    for (auto &resultsIter : input->results)
    {
        const TestIdentifier &id = resultsIter.first;
        TestResult &inputResult  = resultsIter.second;
        TestResult &outputResult = output->results[id];

        if (inputResult.type != TestResultType::NoResult)
        {
            if (outputResult.type != TestResultType::NoResult)
            {
                printf("Warning: duplicate entry for %s.%s.\n", id.testSuiteName.c_str(),
                       id.testName.c_str());
                return false;
            }

            // Mark the tests that haven't exhausted their retries as 'SKIP'. This makes ANGLE
            // attempt the test again.
            uint32_t runCount = outputResult.flakyFailures + 1;
            if (IsFailedResult(inputResult.type) && runCount < static_cast<uint32_t>(flakyRetries))
            {
                printf("Retrying flaky test: %s.%s.\n", id.testSuiteName.c_str(),
                       id.testName.c_str());
                inputResult.type = TestResultType::NoResult;
                outputResult.flakyFailures++;
            }
            else
            {
                outputResult.type = inputResult.type;
            }
            if (runCount == 1)
            {
                outputResult.elapsedTimeSeconds = inputResult.elapsedTimeSeconds;
            }
            else
            {
                outputResult.elapsedTimeSeconds.insert(outputResult.elapsedTimeSeconds.end(),
                                                       inputResult.elapsedTimeSeconds.begin(),
                                                       inputResult.elapsedTimeSeconds.end());
            }
        }
    }

    output->testArtifactPaths.insert(output->testArtifactPaths.end(),
                                     input->testArtifactPaths.begin(),
                                     input->testArtifactPaths.end());

    return true;
}

void PrintTestOutputSnippet(const TestIdentifier &id,
                            const TestResult &result,
                            const std::string &fullOutput)
{
    std::stringstream nameStream;
    nameStream << id;
    std::string fullName = nameStream.str();

    size_t runPos = fullOutput.find(std::string(kStartedTestString) + fullName);
    if (runPos == std::string::npos)
    {
        printf("Cannot locate test output snippet.\n");
        return;
    }

    size_t endPos = fullOutput.find(std::string(kFailedTestString) + fullName, runPos);
    // Only clip the snippet to the "OK" message if the test really
    // succeeded. It still might have e.g. crashed after printing it.
    if (endPos == std::string::npos && result.type == TestResultType::Pass)
    {
        endPos = fullOutput.find(std::string(kPassedTestString) + fullName, runPos);
    }
    if (endPos != std::string::npos)
    {
        size_t newline_pos = fullOutput.find("\n", endPos);
        if (newline_pos != std::string::npos)
            endPos = newline_pos + 1;
    }

    std::cout << "\n";
    if (endPos != std::string::npos)
    {
        std::cout << fullOutput.substr(runPos, endPos - runPos);
    }
    else
    {
        std::cout << fullOutput.substr(runPos);
    }
}

std::string GetConfigNameFromTestIdentifier(const TestIdentifier &id)
{
    size_t slashPos = id.testName.find('/');
    if (slashPos == std::string::npos)
    {
        return "default";
    }

    size_t doubleUnderscorePos = id.testName.find("__");
    if (doubleUnderscorePos == std::string::npos)
    {
        std::string configName = id.testName.substr(slashPos + 1);

        if (!BeginsWith(configName, "ES"))
        {
            return "default";
        }

        return configName;
    }
    else
    {
        return id.testName.substr(slashPos + 1, doubleUnderscorePos - slashPos - 1);
    }
}

TestQueue BatchTests(const std::vector<TestIdentifier> &tests, int batchSize)
{
    // First sort tests by configuration.
    angle::HashMap<std::string, std::vector<TestIdentifier>> testsSortedByConfig;
    for (const TestIdentifier &id : tests)
    {
        std::string config = GetConfigNameFromTestIdentifier(id);
        testsSortedByConfig[config].push_back(id);
    }

    // Then group into batches by 'batchSize'.
    TestQueue testQueue;
    for (const auto &configAndIds : testsSortedByConfig)
    {
        const std::vector<TestIdentifier> &configTests = configAndIds.second;

        // Count the number of batches needed for this config.
        int batchesForConfig = static_cast<int>(configTests.size() + batchSize - 1) / batchSize;

        // Create batches with striping to split up slow tests.
        for (int batchIndex = 0; batchIndex < batchesForConfig; ++batchIndex)
        {
            std::vector<TestIdentifier> batchTests;
            for (size_t testIndex = batchIndex; testIndex < configTests.size();
                 testIndex += batchesForConfig)
            {
                batchTests.push_back(configTests[testIndex]);
            }
            testQueue.emplace(std::move(batchTests));
            ASSERT(batchTests.empty());
        }
    }

    return testQueue;
}

void ListTests(const std::map<TestIdentifier, TestResult> &resultsMap)
{
    std::cout << "Tests list:\n";

    for (const auto &resultIt : resultsMap)
    {
        const TestIdentifier &id = resultIt.first;
        std::cout << id << "\n";
    }

    std::cout << "End tests list.\n";
}

// Prints the names of the tests matching the user-specified filter flag.
// This matches the output from googletest/src/gtest.cc but is much much faster for large filters.
// See http://anglebug.com/42263725
void GTestListTests(const std::map<TestIdentifier, TestResult> &resultsMap)
{
    std::map<std::string, std::vector<std::string>> suites;

    for (const auto &resultIt : resultsMap)
    {
        const TestIdentifier &id = resultIt.first;
        suites[id.testSuiteName].push_back(id.testName);
    }

    for (const auto &testSuiteIt : suites)
    {
        bool printedTestSuiteName = false;

        const std::string &suiteName              = testSuiteIt.first;
        const std::vector<std::string> &testNames = testSuiteIt.second;

        for (const std::string &testName : testNames)
        {
            if (!printedTestSuiteName)
            {
                printedTestSuiteName = true;
                printf("%s.\n", suiteName.c_str());
            }
            printf("  %s\n", testName.c_str());
        }
    }
}

// On Android, batching is done on the host, i.e. externally.
// TestSuite executes on the device and should just passthrough all args to GTest.
bool UsesExternalBatching()
{
#if defined(ANGLE_PLATFORM_ANDROID)
    return true;
#else
    return false;
#endif
}
}  // namespace

void MetricWriter::enable(const std::string &testArtifactDirectory)
{
    mPath = testArtifactDirectory + GetPathSeparator() + "angle_metrics";
}

void MetricWriter::writeInfo(const std::string &name,
                             const std::string &backend,
                             const std::string &story,
                             const std::string &metric,
                             const std::string &units)
{
    if (mPath.empty())
    {
        return;
    }

    if (mFile == nullptr)
    {
        mFile = fopen(mPath.c_str(), "w");
    }
    ASSERT(mFile != nullptr);

    fprintf(mFile, "{\"name\":\"%s\",", name.c_str());
    fprintf(mFile, "\"backend\":\"%s\",", backend.c_str());
    fprintf(mFile, "\"story\":\"%s\",", story.c_str());
    fprintf(mFile, "\"metric\":\"%s\",", metric.c_str());
    fprintf(mFile, "\"units\":\"%s\",", units.c_str());
    // followed by writing value, so no closing bracket yet
}

void MetricWriter::writeDoubleValue(double value)
{
    if (mFile != nullptr)
    {
        fprintf(mFile, "\"value\":\"%lf\"}\n", value);
    }
}

void MetricWriter::writeIntegerValue(size_t value)
{
    if (mFile != nullptr)
    {
        fprintf(mFile, "\"value\":\"%zu\"}\n", value);
    }
}

void MetricWriter::close()
{
    if (mFile != nullptr)
    {
        fclose(mFile);
        mFile = nullptr;
    }
}

// static
TestSuite *TestSuite::mInstance = nullptr;

TestIdentifier::TestIdentifier() = default;

TestIdentifier::TestIdentifier(const std::string &suiteNameIn, const std::string &nameIn)
    : testSuiteName(suiteNameIn), testName(nameIn)
{}

TestIdentifier::TestIdentifier(const TestIdentifier &other) = default;

TestIdentifier::~TestIdentifier() = default;

TestIdentifier &TestIdentifier::operator=(const TestIdentifier &other) = default;

void TestIdentifier::snprintfName(char *outBuffer, size_t maxLen) const
{
    snprintf(outBuffer, maxLen, "%s.%s", testSuiteName.c_str(), testName.c_str());
}

// static
bool TestIdentifier::ParseFromString(const std::string &str, TestIdentifier *idOut)
{
    size_t separator = str.find(".");
    if (separator == std::string::npos)
    {
        return false;
    }

    idOut->testSuiteName = str.substr(0, separator);
    idOut->testName      = str.substr(separator + 1, str.length() - separator - 1);
    return true;
}

TestResults::TestResults() = default;

TestResults::~TestResults() = default;

ProcessInfo::ProcessInfo() = default;

ProcessInfo &ProcessInfo::operator=(ProcessInfo &&rhs)
{
    process         = std::move(rhs.process);
    testsInBatch    = std::move(rhs.testsInBatch);
    resultsFileName = std::move(rhs.resultsFileName);
    filterFileName  = std::move(rhs.filterFileName);
    commandLine     = std::move(rhs.commandLine);
    filterString    = std::move(rhs.filterString);
    return *this;
}

ProcessInfo::~ProcessInfo() = default;

ProcessInfo::ProcessInfo(ProcessInfo &&other)
{
    *this = std::move(other);
}

class TestSuite::TestEventListener : public testing::EmptyTestEventListener
{
  public:
    // Note: TestResults is owned by the TestSuite. It should outlive TestEventListener.
    TestEventListener(TestSuite *testSuite) : mTestSuite(testSuite) {}

    void OnTestStart(const testing::TestInfo &testInfo) override
    {
        std::lock_guard<std::mutex> guard(mTestSuite->mTestResults.currentTestMutex);
        mTestSuite->mTestResults.currentTest = GetTestIdentifier(testInfo);
        mTestSuite->mTestResults.currentTestTimer.start();
    }

    void OnTestEnd(const testing::TestInfo &testInfo) override
    {
        std::lock_guard<std::mutex> guard(mTestSuite->mTestResults.currentTestMutex);
        mTestSuite->mTestResults.currentTestTimer.stop();
        const testing::TestResult &resultIn = *testInfo.result();
        UpdateCurrentTestResult(resultIn, &mTestSuite->mTestResults);
        mTestSuite->mTestResults.currentTest = TestIdentifier();
    }

    void OnTestProgramEnd(const testing::UnitTest &testProgramInfo) override
    {
        std::lock_guard<std::mutex> guard(mTestSuite->mTestResults.currentTestMutex);
        mTestSuite->mTestResults.allDone = true;
        mTestSuite->writeOutputFiles(false);
    }

  private:
    TestSuite *mTestSuite;
};

TestSuite::TestSuite(int *argc, char **argv) : TestSuite(argc, argv, []() {}) {}

TestSuite::TestSuite(int *argc, char **argv, std::function<void()> registerTestsCallback)
    : mShardCount(-1),
      mShardIndex(-1),
      mBotMode(false),
      mDebugTestGroups(false),
      mGTestListTests(false),
      mListTests(false),
      mPrintTestStdout(false),
      mDisableCrashHandler(false),
      mBatchSize(kDefaultBatchSize),
      mCurrentResultCount(0),
      mTotalResultCount(0),
      mMaxProcesses(std::min(NumberOfProcessors(), kDefaultMaxProcesses)),
      mTestTimeout(kDefaultTestTimeout),
      mBatchTimeout(kDefaultBatchTimeout),
      mBatchId(-1),
      mFlakyRetries(0),
      mMaxFailures(kDefaultMaxFailures),
      mFailureCount(0),
      mModifiedPreferredDevice(false)
{
    ASSERT(mInstance == nullptr);
    mInstance = this;

    Optional<int> filterArgIndex;
    bool alsoRunDisabledTests = false;

#if defined(ANGLE_PLATFORM_MACOS)
    // By default, we should hook file API functions on macOS to avoid slow Metal shader caching
    // file access.
    angle::InitMetalFileAPIHooking(*argc, argv);
#endif

#if defined(ANGLE_PLATFORM_WINDOWS)
    GTEST_FLAG_SET(catch_exceptions, false);
#endif

    if (*argc <= 0)
    {
        printf("Missing test arguments.\n");
        exit(EXIT_FAILURE);
    }

    mTestExecutableName = argv[0];

    for (int argIndex = 1; argIndex < *argc;)
    {
        if (parseSingleArg(argc, argv, argIndex))
        {
            continue;
        }

        if (strstr(argv[argIndex], "--gtest_filter=") == argv[argIndex])
        {
            filterArgIndex = argIndex;
        }
        else
        {
            // Don't include disabled tests in test lists unless the user asks for them.
            if (strcmp("--gtest_also_run_disabled_tests", argv[argIndex]) == 0)
            {
                alsoRunDisabledTests = true;
            }

            mChildProcessArgs.push_back(argv[argIndex]);
        }
        ++argIndex;
    }

    if (mTestArtifactDirectory.empty())
    {
        mTestArtifactDirectory = GetEnvironmentVar("ISOLATED_OUTDIR");
    }

#if defined(ANGLE_PLATFORM_FUCHSIA)
    if (mBotMode)
    {
        printf("Note: Bot mode is not available on Fuchsia. See http://anglebug.com/42265786\n");
        mBotMode = false;
    }
#endif

    if (UsesExternalBatching() && mBotMode)
    {
        printf("Bot mode is mutually exclusive with external batching.\n");
        exit(EXIT_FAILURE);
    }

    mTestResults.currentTestTimeout = mTestTimeout;

    if (!mDisableCrashHandler)
    {
        // Note that the crash callback must be owned and not use global constructors.
        mCrashCallback = [this]() { onCrashOrTimeout(TestResultType::Crash); };
        InitCrashHandler(&mCrashCallback);
    }

#if defined(ANGLE_PLATFORM_WINDOWS) || defined(ANGLE_PLATFORM_LINUX)
    if (IsASan())
    {
        // Set before `registerTestsCallback()` call
        SetEnvironmentVar(kVkLoaderDisableDLLUnloadingEnvVar, "1");
    }
#endif

    registerTestsCallback();

    std::string envShardIndex = angle::GetEnvironmentVar("GTEST_SHARD_INDEX");
    if (!envShardIndex.empty())
    {
        angle::UnsetEnvironmentVar("GTEST_SHARD_INDEX");
        if (mShardIndex == -1)
        {
            std::stringstream shardIndexStream(envShardIndex);
            shardIndexStream >> mShardIndex;
        }
    }

    std::string envTotalShards = angle::GetEnvironmentVar("GTEST_TOTAL_SHARDS");
    if (!envTotalShards.empty())
    {
        angle::UnsetEnvironmentVar("GTEST_TOTAL_SHARDS");
        if (mShardCount == -1)
        {
            std::stringstream shardCountStream(envTotalShards);
            shardCountStream >> mShardCount;
        }
    }

    // The test harness reads the active GPU from SystemInfo and uses that for test expectations.
    // However, some ANGLE backends don't have a concept of an "active" GPU, and instead use power
    // preference to select GPU. We can use the environment variable ANGLE_PREFERRED_DEVICE to
    // ensure ANGLE's selected GPU matches the GPU expected for this test suite.
    const GPUTestConfig testConfig      = GPUTestConfig();
    const char kPreferredDeviceEnvVar[] = "ANGLE_PREFERRED_DEVICE";
    if (GetEnvironmentVar(kPreferredDeviceEnvVar).empty())
    {
        mModifiedPreferredDevice                        = true;
        const GPUTestConfig::ConditionArray &conditions = testConfig.getConditions();
        if (conditions[GPUTestConfig::kConditionAMD])
        {
            SetEnvironmentVar(kPreferredDeviceEnvVar, "amd");
        }
        else if (conditions[GPUTestConfig::kConditionNVIDIA])
        {
            SetEnvironmentVar(kPreferredDeviceEnvVar, "nvidia");
        }
        else if (conditions[GPUTestConfig::kConditionIntel])
        {
            SetEnvironmentVar(kPreferredDeviceEnvVar, "intel");
        }
        else if (conditions[GPUTestConfig::kConditionApple])
        {
            SetEnvironmentVar(kPreferredDeviceEnvVar, "apple");
        }
    }

    // Special handling for TSAN and UBSAN to force crashes when run in automated testing.
    if (IsTSan())
    {
        std::string tsanOptions = GetEnvironmentVar(kTSanOptionsEnvVar);
        tsanOptions += " halt_on_error=1";
        SetEnvironmentVar(kTSanOptionsEnvVar, tsanOptions.c_str());
    }

    if (IsUBSan())
    {
        std::string ubsanOptions = GetEnvironmentVar(kUBSanOptionsEnvVar);
        ubsanOptions += " halt_on_error=1";
        SetEnvironmentVar(kUBSanOptionsEnvVar, ubsanOptions.c_str());
    }

    if ((mShardIndex == -1) != (mShardCount == -1))
    {
        printf("Shard index and shard count must be specified together.\n");
        exit(EXIT_FAILURE);
    }

    if (!mFilterFile.empty())
    {
        if (filterArgIndex.valid())
        {
            printf("Cannot use gtest_filter in conjunction with a filter file.\n");
            exit(EXIT_FAILURE);
        }

        std::string fileContents;
        if (!ReadEntireFileToString(mFilterFile.c_str(), &fileContents))
        {
            printf("Error loading filter file: %s\n", mFilterFile.c_str());
            exit(EXIT_FAILURE);
        }
        mFilterString.assign(fileContents.data());

        if (mFilterString.substr(0, strlen("--gtest_filter=")) != std::string("--gtest_filter="))
        {
            printf("Filter file must start with \"--gtest_filter=\".\n");
            exit(EXIT_FAILURE);
        }

        // Note that we only add a filter string if we previously deleted a shader filter file
        // argument. So we will have space for the new filter string in argv.
        AddArg(argc, argv, mFilterString.c_str());
    }

    // Call into gtest internals to force parameterized test name registration.
    testing::internal::UnitTestImpl *impl = testing::internal::GetUnitTestImpl();
    impl->RegisterParameterizedTests();

    // Initialize internal GoogleTest filter arguments so we can call "FilterMatchesTest".
    testing::internal::ParseGoogleTestFlagsOnly(argc, argv);

    std::vector<TestIdentifier> testSet = GetFilteredTests(&mTestFileLines, alsoRunDisabledTests);

    if (mShardCount == 0)
    {
        printf("Shard count must be > 0.\n");
        exit(EXIT_FAILURE);
    }
    else if (mShardCount > 0)
    {
        if (mShardIndex >= mShardCount)
        {
            printf("Shard index must be less than shard count.\n");
            exit(EXIT_FAILURE);
        }

        // If there's only one shard, we can use the testSet as defined above.
        if (mShardCount > 1)
        {
            if (!mBotMode && !UsesExternalBatching())
            {
                printf("Sharding is only supported in bot mode or external batching.\n");
                exit(EXIT_FAILURE);
            }
            // With external batching, we must use exactly the testSet as defined externally.
            // But when listing tests, we do need to apply sharding ourselves,
            // since we use our own implementation for listing tests and not GTest directly.
            if (!UsesExternalBatching() || mGTestListTests || mListTests)
            {
                testSet = GetShardTests(testSet, mShardIndex, mShardCount, &mTestFileLines,
                                        alsoRunDisabledTests);
            }
        }
    }

    if (!testSet.empty())
    {
        std::stringstream fakeTestName;
        fakeTestName << kArtifactsFakeTestName << '-' << testSet[0].testName;
        mTestResults.testArtifactsFakeTestName = fakeTestName.str();
    }

    if (mBotMode)
    {
        // Split up test batches.
        mTestQueue = BatchTests(testSet, mBatchSize);

        if (mDebugTestGroups)
        {
            std::cout << "Test Groups:\n";

            while (!mTestQueue.empty())
            {
                const std::vector<TestIdentifier> &tests = mTestQueue.front();
                std::cout << GetConfigNameFromTestIdentifier(tests[0]) << " ("
                          << static_cast<int>(tests.size()) << ")\n";
                mTestQueue.pop();
            }

            exit(EXIT_SUCCESS);
        }
    }

    testing::InitGoogleTest(argc, argv);

    mTotalResultCount = testSet.size();

    if ((mBotMode || !mResultsDirectory.empty()) && mResultsFile.empty())
    {
        // Create a default output file in bot mode.
        mResultsFile = "output.json";
    }

    if (!mResultsDirectory.empty())
    {
        std::stringstream resultFileName;
        resultFileName << mResultsDirectory << GetPathSeparator() << mResultsFile;
        mResultsFile = resultFileName.str();
    }

    if (!mTestArtifactDirectory.empty())
    {
        mMetricWriter.enable(mTestArtifactDirectory);
    }

    if (!mBotMode)
    {
        testing::TestEventListeners &listeners = testing::UnitTest::GetInstance()->listeners();
        listeners.Append(new TestEventListener(this));

        for (const TestIdentifier &id : testSet)
        {
            mTestResults.results[id].type = TestResultType::NoResult;
        }
    }
}

TestSuite::~TestSuite()
{
    const char kPreferredDeviceEnvVar[] = "ANGLE_PREFERRED_DEVICE";
    if (mModifiedPreferredDevice && !angle::GetEnvironmentVar(kPreferredDeviceEnvVar).empty())
    {
        angle::UnsetEnvironmentVar(kPreferredDeviceEnvVar);
    }

    if (mWatchdogThread.joinable())
    {
        mWatchdogThread.detach();
    }
    TerminateCrashHandler();
}

bool TestSuite::parseSingleArg(int *argc, char **argv, int argIndex)
{
    // Note: Flags should be documented in README.md.
    return ParseIntArg("--shard-count", argc, argv, argIndex, &mShardCount) ||
           ParseIntArg("--shard-index", argc, argv, argIndex, &mShardIndex) ||
           ParseIntArg("--batch-size", argc, argv, argIndex, &mBatchSize) ||
           ParseIntArg("--max-processes", argc, argv, argIndex, &mMaxProcesses) ||
           ParseIntArg(kTestTimeoutArg, argc, argv, argIndex, &mTestTimeout) ||
           ParseIntArg("--batch-timeout", argc, argv, argIndex, &mBatchTimeout) ||
           ParseIntArg("--flaky-retries", argc, argv, argIndex, &mFlakyRetries) ||
           ParseIntArg("--max-failures", argc, argv, argIndex, &mMaxFailures) ||
           // Other test functions consume the batch ID, so keep it in the list.
           ParseIntArgWithHandling(kBatchId, argc, argv, argIndex, &mBatchId,
                                   ArgHandling::Preserve) ||
           ParseStringArg("--results-directory", argc, argv, argIndex, &mResultsDirectory) ||
           ParseStringArg(kResultFileArg, argc, argv, argIndex, &mResultsFile) ||
           ParseStringArg("--isolated-script-test-output", argc, argv, argIndex, &mResultsFile) ||
           ParseStringArg(kFilterFileArg, argc, argv, argIndex, &mFilterFile) ||
           ParseStringArg("--histogram-json-file", argc, argv, argIndex, &mHistogramJsonFile) ||
           // We need these overloads to work around technical debt in the Android test runner.
           ParseStringArg("--isolated-script-test-perf-output", argc, argv, argIndex,
                          &mHistogramJsonFile) ||
           ParseStringArg("--isolated_script_test_perf_output", argc, argv, argIndex,
                          &mHistogramJsonFile) ||
           ParseStringArg("--render-test-output-dir", argc, argv, argIndex,
                          &mTestArtifactDirectory) ||
           ParseStringArg("--isolated-outdir", argc, argv, argIndex, &mTestArtifactDirectory) ||
           ParseFlag("--test-launcher-bot-mode", argc, argv, argIndex, &mBotMode) ||
           ParseFlag("--bot-mode", argc, argv, argIndex, &mBotMode) ||
           ParseFlag("--debug-test-groups", argc, argv, argIndex, &mDebugTestGroups) ||
           ParseFlag("--gtest_list_tests", argc, argv, argIndex, &mGTestListTests) ||
           ParseFlag("--list-tests", argc, argv, argIndex, &mListTests) ||
           ParseFlag("--print-test-stdout", argc, argv, argIndex, &mPrintTestStdout) ||
           ParseFlag(kDisableCrashHandler, argc, argv, argIndex, &mDisableCrashHandler);
}

void TestSuite::onCrashOrTimeout(TestResultType crashOrTimeout)
{
    std::lock_guard<std::mutex> guard(mTestResults.currentTestMutex);
    if (mTestResults.currentTest.valid())
    {
        TestResult &result               = mTestResults.results[mTestResults.currentTest];
        result.type                      = crashOrTimeout;
        result.elapsedTimeSeconds.back() = mTestResults.currentTestTimer.getElapsedWallClockTime();
    }

    if (mResultsFile.empty())
    {
        printf("No results file specified.\n");
        return;
    }

    writeOutputFiles(true);
}

bool TestSuite::launchChildTestProcess(uint32_t batchId,
                                       const std::vector<TestIdentifier> &testsInBatch)
{
    // Create a temporary file to store the test list
    ProcessInfo processInfo;

    Optional<std::string> filterBuffer = CreateTemporaryFile();
    if (!filterBuffer.valid())
    {
        std::cerr << "Error creating temporary file for test list.\n";
        return false;
    }
    processInfo.filterFileName.assign(filterBuffer.value());

    std::string filterString = GetTestFilter(testsInBatch);

    FILE *fp = fopen(processInfo.filterFileName.c_str(), "w");
    if (!fp)
    {
        std::cerr << "Error opening temporary file for test list.\n";
        return false;
    }
    fprintf(fp, "%s", filterString.c_str());
    fclose(fp);

    processInfo.filterString = filterString;

    std::string filterFileArg = kFilterFileArg + std::string("=") + processInfo.filterFileName;

    // Create a temporary file to store the test output.
    Optional<std::string> resultsBuffer = CreateTemporaryFile();
    if (!resultsBuffer.valid())
    {
        std::cerr << "Error creating temporary file for test list.\n";
        return false;
    }
    processInfo.resultsFileName.assign(resultsBuffer.value());

    std::string resultsFileArg = kResultFileArg + std::string("=") + processInfo.resultsFileName;

    // Construct command line for child process.
    std::vector<const char *> args;

    args.push_back(mTestExecutableName.c_str());
    args.push_back(filterFileArg.c_str());
    args.push_back(resultsFileArg.c_str());

    std::stringstream batchIdStream;
    batchIdStream << kBatchId << "=" << batchId;
    std::string batchIdString = batchIdStream.str();
    args.push_back(batchIdString.c_str());

    for (const std::string &arg : mChildProcessArgs)
    {
        args.push_back(arg.c_str());
    }

    if (mDisableCrashHandler)
    {
        args.push_back(kDisableCrashHandler);
    }

    std::string timeoutStr;
    if (mTestTimeout != kDefaultTestTimeout)
    {
        std::stringstream timeoutStream;
        timeoutStream << kTestTimeoutArg << "=" << mTestTimeout;
        timeoutStr = timeoutStream.str();
        args.push_back(timeoutStr.c_str());
    }

    std::string artifactsDir;
    if (!mTestArtifactDirectory.empty())
    {
        std::stringstream artifactsDirStream;
        artifactsDirStream << kIsolatedOutDir << "=" << mTestArtifactDirectory;
        artifactsDir = artifactsDirStream.str();
        args.push_back(artifactsDir.c_str());
    }

    // Launch child process and wait for completion.
    processInfo.process = LaunchProcess(args, ProcessOutputCapture::StdoutAndStderrInterleaved);

    if (!processInfo.process->started())
    {
        std::cerr << "Error launching child process.\n";
        return false;
    }

    std::stringstream commandLineStr;
    for (const char *arg : args)
    {
        commandLineStr << arg << " ";
    }

    processInfo.commandLine  = commandLineStr.str();
    processInfo.testsInBatch = testsInBatch;
    mCurrentProcesses.emplace_back(std::move(processInfo));
    return true;
}

void ParseTestIdentifierAndSetResult(const std::string &testName,
                                     TestResultType result,
                                     TestResults *results)
{
    // Trim off any whitespace + extra stuff at the end of the string.
    std::string modifiedTestName = testName.substr(0, testName.find(' '));
    modifiedTestName             = modifiedTestName.substr(0, testName.find('\r'));
    TestIdentifier id;
    bool ok = TestIdentifier::ParseFromString(modifiedTestName, &id);
    ASSERT(ok);
    results->results[id] = {result};
}

bool TestSuite::finishProcess(ProcessInfo *processInfo)
{
    // Get test results and merge into main list.
    TestResults batchResults;

    if (!GetTestResultsFromFile(processInfo->resultsFileName.c_str(), &batchResults))
    {
        std::cerr << "Warning: could not find test results file from child process.\n";

        // First assume all tests get skipped.
        for (const TestIdentifier &id : processInfo->testsInBatch)
        {
            batchResults.results[id] = {TestResultType::NoResult};
        }

        // Attempt to reconstruct passing list from stdout snippets.
        const std::string &batchStdout = processInfo->process->getStdout();
        std::istringstream linesStream(batchStdout);

        std::string line;
        while (std::getline(linesStream, line))
        {
            size_t startPos   = line.find(kStartedTestString);
            size_t failPos    = line.find(kFailedTestString);
            size_t passPos    = line.find(kPassedTestString);
            size_t skippedPos = line.find(kSkippedTestString);

            if (startPos != std::string::npos)
            {
                // Assume a test that's started crashed until we see it completed.
                std::string testName = line.substr(strlen(kStartedTestString));
                ParseTestIdentifierAndSetResult(testName, TestResultType::Crash, &batchResults);
            }
            else if (failPos != std::string::npos)
            {
                std::string testName = line.substr(strlen(kFailedTestString));
                ParseTestIdentifierAndSetResult(testName, TestResultType::Fail, &batchResults);
            }
            else if (passPos != std::string::npos)
            {
                std::string testName = line.substr(strlen(kPassedTestString));
                ParseTestIdentifierAndSetResult(testName, TestResultType::Pass, &batchResults);
            }
            else if (skippedPos != std::string::npos)
            {
                std::string testName = line.substr(strlen(kSkippedTestString));
                ParseTestIdentifierAndSetResult(testName, TestResultType::Skip, &batchResults);
            }
        }
    }

    if (!MergeTestResults(&batchResults, &mTestResults, mFlakyRetries))
    {
        std::cerr << "Error merging batch test results.\n";
        return false;
    }

    if (!batchResults.results.empty())
    {
        const TestIdentifier &id = batchResults.results.begin()->first;
        std::string config       = GetConfigNameFromTestIdentifier(id);
        printf("Completed batch with config: %s\n", config.c_str());

        for (const auto &resultIter : batchResults.results)
        {
            const TestResult &result = resultIter.second;
            if (result.type != TestResultType::NoResult && IsFailedResult(result.type))
            {
                printf("To reproduce the batch, use filter:\n%s\n",
                       processInfo->filterString.c_str());
                break;
            }
        }
    }

    // Process results and print unexpected errors.
    for (const auto &resultIter : batchResults.results)
    {
        const TestIdentifier &id = resultIter.first;
        const TestResult &result = resultIter.second;

        // Skip results aren't procesed since they're added back to the test queue below.
        if (result.type == TestResultType::NoResult)
        {
            continue;
        }

        mCurrentResultCount++;

        printf("[%d/%d] %s.%s", mCurrentResultCount, mTotalResultCount, id.testSuiteName.c_str(),
               id.testName.c_str());

        if (mPrintTestStdout)
        {
            const std::string &batchStdout = processInfo->process->getStdout();
            PrintTestOutputSnippet(id, result, batchStdout);
        }
        else if (result.type == TestResultType::Pass)
        {
            printf(" (%0.1lf ms)\n", result.elapsedTimeSeconds.back() * 1000.0);
        }
        else if (result.type == TestResultType::Skip)
        {
            printf(" (skipped)\n");
        }
        else if (result.type == TestResultType::Timeout)
        {
            printf(" (TIMEOUT in %0.1lf s)\n", result.elapsedTimeSeconds.back());
            mFailureCount++;

            const std::string &batchStdout = processInfo->process->getStdout();
            PrintTestOutputSnippet(id, result, batchStdout);
        }
        else
        {
            printf(" (%s)\n", ResultTypeToString(result.type));
            mFailureCount++;

            const std::string &batchStdout = processInfo->process->getStdout();
            PrintTestOutputSnippet(id, result, batchStdout);
        }
    }

    // On unexpected exit, re-queue any unfinished tests.
    std::vector<TestIdentifier> unfinishedTests;
    for (const auto &resultIter : batchResults.results)
    {
        const TestIdentifier &id = resultIter.first;
        const TestResult &result = resultIter.second;

        if (result.type == TestResultType::NoResult)
        {
            unfinishedTests.push_back(id);
        }
    }

    if (!unfinishedTests.empty())
    {
        mTestQueue.emplace(std::move(unfinishedTests));
    }

    // Clean up any dirty temporary files.
    for (const std::string &tempFile : {processInfo->filterFileName, processInfo->resultsFileName})
    {
        // Note: we should be aware that this cleanup won't happen if the harness itself
        // crashes. If this situation comes up in the future we should add crash cleanup to the
        // harness.
        if (!angle::DeleteSystemFile(tempFile.c_str()))
        {
            std::cerr << "Warning: Error cleaning up temp file: " << tempFile << "\n";
        }
    }

    processInfo->process.reset();
    return true;
}

int TestSuite::run()
{
#if defined(ANGLE_PLATFORM_ANDROID)
    if (mListTests && mGTestListTests)
    {
        // Workaround for the Android test runner requiring a GTest test list.
        printf("PlaceholderTest.\n  Placeholder\n");
        return EXIT_SUCCESS;
    }
#endif  // defined(ANGLE_PLATFORM_ANDROID)

    if (mListTests)
    {
        ListTests(mTestResults.results);

#if defined(ANGLE_PLATFORM_ANDROID)
        // Because of quirks with the Chromium-provided Android test runner, we need to use a few
        // tricks to get the test list output. We add placeholder output for a single test to trick
        // the test runner into thinking it ran the tests successfully. We also add an end marker
        // for the tests list so we can parse the list from the more spammy Android stdout log.
        static constexpr char kPlaceholderTestTest[] = R"(
[==========] Running 1 test from 1 test suite.
[----------] Global test environment set-up.
[----------] 1 test from PlaceholderTest
[ RUN      ] PlaceholderTest.Placeholder
[       OK ] PlaceholderTest.Placeholder (0 ms)
[----------] 1 test from APITest (0 ms total)

[----------] Global test environment tear-down
[==========] 1 test from 1 test suite ran. (24 ms total)
[  PASSED  ] 1 test.
)";
        printf(kPlaceholderTestTest);
#endif  // defined(ANGLE_PLATFORM_ANDROID)

        return EXIT_SUCCESS;
    }

    if (mGTestListTests)
    {
        GTestListTests(mTestResults.results);
        return EXIT_SUCCESS;
    }

    // Run tests serially.
    if (!mBotMode)
    {
        // Only start the watchdog if the debugger is not attached and we're a child process.
        if (!angle::IsDebuggerAttached() && mBatchId != -1)
        {
            startWatchdog();
        }

        int retVal = RUN_ALL_TESTS();
        {
            std::lock_guard<std::mutex> guard(mTestResults.currentTestMutex);
            mTestResults.allDone = true;
        }

        if (mWatchdogThread.joinable())
        {
            mWatchdogThread.join();
        }
        return retVal;
    }

    Timer totalRunTime;
    totalRunTime.start();

    Timer messageTimer;
    messageTimer.start();

    uint32_t batchId = 0;

    while (!mTestQueue.empty() || !mCurrentProcesses.empty())
    {
        bool progress = false;

        // Spawn a process if needed and possible.
        if (static_cast<int>(mCurrentProcesses.size()) < mMaxProcesses && !mTestQueue.empty())
        {
            std::vector<TestIdentifier> testsInBatch = mTestQueue.front();
            mTestQueue.pop();

            if (!launchChildTestProcess(++batchId, testsInBatch))
            {
                return 1;
            }

            progress = true;
        }

        // Check for process completion.
        uint32_t totalTestCount = 0;
        for (auto processIter = mCurrentProcesses.begin(); processIter != mCurrentProcesses.end();)
        {
            ProcessInfo &processInfo = *processIter;
            if (processInfo.process->finished())
            {
                if (!finishProcess(&processInfo))
                {
                    return 1;
                }
                processIter = mCurrentProcesses.erase(processIter);
                progress    = true;
            }
            else if (processInfo.process->getElapsedTimeSeconds() > mBatchTimeout)
            {
                // Terminate the process and record timeouts for the batch.
                // Because we can't determine which sub-test caused a timeout, record the whole
                // batch as a timeout failure. Can be improved by using socket message passing.
                if (!processInfo.process->kill())
                {
                    return 1;
                }

                const std::string &batchStdout = processInfo.process->getStdout();
                std::vector<std::string> lines =
                    SplitString(batchStdout, "\r\n", WhitespaceHandling::TRIM_WHITESPACE,
                                SplitResult::SPLIT_WANT_NONEMPTY);
                constexpr size_t kKeepLines = 10;
                printf("\nBatch timeout! Last %zu lines of batch stdout:\n", kKeepLines);
                printf("---------------------------------------------\n");
                for (size_t lineNo = lines.size() - std::min(lines.size(), kKeepLines);
                     lineNo < lines.size(); ++lineNo)
                {
                    printf("%s\n", lines[lineNo].c_str());
                }
                printf("---------------------------------------------\n\n");

                for (const TestIdentifier &testIdentifier : processInfo.testsInBatch)
                {
                    // Because the whole batch failed we can't know how long each test took.
                    mTestResults.results[testIdentifier].type = TestResultType::Timeout;
                    mFailureCount++;
                }

                processIter = mCurrentProcesses.erase(processIter);
                progress    = true;
            }
            else
            {
                totalTestCount += static_cast<uint32_t>(processInfo.testsInBatch.size());
                processIter++;
            }
        }

        if (progress)
        {
            messageTimer.start();
        }
        else if (messageTimer.getElapsedWallClockTime() > kIdleMessageTimeout)
        {
            const ProcessInfo &processInfo = mCurrentProcesses[0];
            double processTime             = processInfo.process->getElapsedTimeSeconds();
            printf("Running %d tests in %d processes, longest for %d seconds.\n", totalTestCount,
                   static_cast<int>(mCurrentProcesses.size()), static_cast<int>(processTime));
            messageTimer.start();
        }

        // Early exit if we passed the maximum failure threshold. Still wait for current tests.
        if (mFailureCount > mMaxFailures && !mTestQueue.empty())
        {
            printf("Reached maximum failure count (%d), clearing test queue.\n", mMaxFailures);
            TestQueue emptyTestQueue;
            std::swap(mTestQueue, emptyTestQueue);
        }

        // Sleep briefly and continue.
        angle::Sleep(100);
    }

    // Dump combined results.
    if (mFailureCount > mMaxFailures)
    {
        printf(
            "Omitted results files because the failure count (%d) exceeded the maximum number of "
            "failures (%d).\n",
            mFailureCount, mMaxFailures);
    }
    else
    {
        writeOutputFiles(false);
    }

    totalRunTime.stop();
    printf("Tests completed in %lf seconds\n", totalRunTime.getElapsedWallClockTime());

    return printFailuresAndReturnCount() == 0 ? 0 : 1;
}

int TestSuite::printFailuresAndReturnCount() const
{
    std::vector<std::string> failures;
    uint32_t skipCount = 0;

    for (const auto &resultIter : mTestResults.results)
    {
        const TestIdentifier &id = resultIter.first;
        const TestResult &result = resultIter.second;

        if (result.type == TestResultType::Skip)
        {
            skipCount++;
        }
        else if (result.type != TestResultType::Pass)
        {
            const FileLine &fileLine = mTestFileLines.find(id)->second;

            std::stringstream failureMessage;
            failureMessage << id << " (" << fileLine.file << ":" << fileLine.line << ") ("
                           << ResultTypeToString(result.type) << ")";
            failures.emplace_back(failureMessage.str());
        }
    }

    if (failures.empty())
        return 0;

    printf("%zu test%s failed:\n", failures.size(), failures.size() > 1 ? "s" : "");
    for (const std::string &failure : failures)
    {
        printf("    %s\n", failure.c_str());
    }
    if (skipCount > 0)
    {
        printf("%u tests skipped.\n", skipCount);
    }

    return static_cast<int>(failures.size());
}

void TestSuite::startWatchdog()
{
    auto watchdogMain = [this]() {
        do
        {
            {
                std::lock_guard<std::mutex> guard(mTestResults.currentTestMutex);
                if (mTestResults.currentTestTimer.getElapsedWallClockTime() >
                    mTestResults.currentTestTimeout)
                {
                    break;
                }

                if (mTestResults.allDone)
                    return;
            }

            angle::Sleep(500);
        } while (true);
        onCrashOrTimeout(TestResultType::Timeout);
        ::_Exit(EXIT_FAILURE);
    };
    mWatchdogThread = std::thread(watchdogMain);
}

void TestSuite::addHistogramSample(const std::string &measurement,
                                   const std::string &story,
                                   double value,
                                   const std::string &units)
{
    mHistogramWriter.addSample(measurement, story, value, units);
}

bool TestSuite::hasTestArtifactsDirectory() const
{
    return !mTestArtifactDirectory.empty();
}

std::string TestSuite::reserveTestArtifactPath(const std::string &artifactName)
{
    mTestResults.testArtifactPaths.push_back(artifactName);

    if (mTestArtifactDirectory.empty())
    {
        return artifactName;
    }

    std::stringstream pathStream;
    pathStream << mTestArtifactDirectory << GetPathSeparator() << artifactName;
    return pathStream.str();
}

bool GetTestResultsFromFile(const char *fileName, TestResults *resultsOut)
{
    std::ifstream ifs(fileName);
    if (!ifs.is_open())
    {
        std::cerr << "Error opening " << fileName << "\n";
        return false;
    }

    js::IStreamWrapper ifsWrapper(ifs);
    js::Document document;
    document.ParseStream(ifsWrapper);

    if (document.HasParseError())
    {
        std::cerr << "Parse error reading JSON document: " << document.GetParseError() << "\n";
        return false;
    }

    if (!GetTestResultsFromJSON(document, resultsOut))
    {
        std::cerr << "Error getting test results from JSON.\n";
        return false;
    }

    return true;
}

void TestSuite::dumpTestExpectationsErrorMessages()
{
    std::stringstream errorMsgStream;
    for (const auto &message : mTestExpectationsParser.getErrorMessages())
    {
        errorMsgStream << std::endl << " " << message;
    }

    std::cerr << "Failed to load test expectations." << errorMsgStream.str() << std::endl;
}

bool TestSuite::loadTestExpectationsFromFileWithConfig(const GPUTestConfig &config,
                                                       const std::string &fileName)
{
    if (!mTestExpectationsParser.loadTestExpectationsFromFile(config, fileName))
    {
        dumpTestExpectationsErrorMessages();
        return false;
    }
    return true;
}

bool TestSuite::loadAllTestExpectationsFromFile(const std::string &fileName)
{
    if (!mTestExpectationsParser.loadAllTestExpectationsFromFile(fileName))
    {
        dumpTestExpectationsErrorMessages();
        return false;
    }
    return true;
}

bool TestSuite::logAnyUnusedTestExpectations()
{
    std::stringstream unusedMsgStream;
    bool anyUnused = false;
    for (const auto &message : mTestExpectationsParser.getUnusedExpectationsMessages())
    {
        anyUnused = true;
        unusedMsgStream << std::endl << " " << message;
    }
    if (anyUnused)
    {
        std::cerr << "Found unused test expectations:" << unusedMsgStream.str() << std::endl;
        return true;
    }
    return false;
}

int32_t TestSuite::getTestExpectation(const std::string &testName)
{
    return mTestExpectationsParser.getTestExpectation(testName);
}

void TestSuite::maybeUpdateTestTimeout(uint32_t testExpectation)
{
    double testTimeout = (testExpectation == GPUTestExpectationsParser::kGpuTestTimeout)
                             ? getSlowTestTimeout()
                             : mTestTimeout;
    std::lock_guard<std::mutex> guard(mTestResults.currentTestMutex);
    mTestResults.currentTestTimeout = testTimeout;
}

int32_t TestSuite::getTestExpectationWithConfigAndUpdateTimeout(const GPUTestConfig &config,
                                                                const std::string &testName)
{
    uint32_t expectation = mTestExpectationsParser.getTestExpectationWithConfig(config, testName);
    maybeUpdateTestTimeout(expectation);
    return expectation;
}

int TestSuite::getSlowTestTimeout() const
{
    return mTestTimeout * kSlowTestTimeoutScale;
}

void TestSuite::writeOutputFiles(bool interrupted)
{
    if (!mResultsFile.empty())
    {
        WriteResultsFile(interrupted, mTestResults, mResultsFile);
    }

    if (!mHistogramJsonFile.empty())
    {
        WriteHistogramJson(mHistogramWriter, mHistogramJsonFile);
    }

    mMetricWriter.close();
}

const char *TestResultTypeToString(TestResultType type)
{
    switch (type)
    {
        case TestResultType::Crash:
            return "Crash";
        case TestResultType::Fail:
            return "Fail";
        case TestResultType::NoResult:
            return "NoResult";
        case TestResultType::Pass:
            return "Pass";
        case TestResultType::Skip:
            return "Skip";
        case TestResultType::Timeout:
            return "Timeout";
        case TestResultType::Unknown:
        default:
            return "Unknown";
    }
}

// This code supports using "-" in test names, which happens often in dEQP. GTest uses as a marker
// for the beginning of the exclusion filter. Work around this by replacing "-" with "?" which
// matches any single character.
std::string ReplaceDashesWithQuestionMark(std::string dashesString)
{
    std::string noDashesString = dashesString;
    ReplaceAllSubstrings(&noDashesString, "-", "?");
    return noDashesString;
}
}  // namespace angle
