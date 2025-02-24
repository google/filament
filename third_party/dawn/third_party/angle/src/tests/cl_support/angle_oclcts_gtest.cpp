#include <gtest/gtest.h>
#if !defined(__APPLE__)
#    include <CL/cl.h>
#endif
#include <stdlib.h>
#include <cassert>
#include <fstream>

#include "harness/testHarness.h"

#include "common/string_utils.h"
#include "common/system_utils.h"
#include "src/tests/test_utils/runner/TestSuite.h"
#include "util/OSWindow.h"

#define MAX_LINE_LENGTH 3000

// The |main| function in each CTS executable is renamed to |ANGLE_oclcts_main|.  It is called by
// the test functions.
extern int ANGLE_oclcts_main(int argc, const char **argv);

namespace
{
constexpr char kInfoTag[] = "*RESULT";

std::string TrimString(const std::string &input, const std::string &trimChars)
{
    auto begin = input.find_first_not_of(trimChars);
    if (begin == std::string::npos)
    {
        return "";
    }

    std::string::size_type end = input.find_last_not_of(trimChars);
    if (end == std::string::npos)
    {
        return input.substr(begin);
    }

    return input.substr(begin, end - begin + 1);
}

class OpenCLCaseList
{
  public:
    OpenCLCaseList();

    struct CaseInfo
    {
        CaseInfo(const std::string &testNameIn, int expectationIn)
            : testCaseDescription(testNameIn), expectation(expectationIn)
        {
            // The test list is specified in the form <testSuiteName.testName testArgs>. Parse the
            // string as such.
            size_t startPos  = testCaseDescription.find('.');
            testSuiteName    = testCaseDescription.substr(0, startPos);
            testNameWithArgs = "";
            if (startPos != std::string::npos)
                testNameWithArgs = testCaseDescription.substr(startPos + 1);
        }

        std::string testCaseDescription;
        std::string testSuiteName;
        std::string testNameWithArgs;
        int expectation;
    };

    void initialize(angle::TestSuite *instance);

    const CaseInfo &getCaseInfo(size_t caseIndex) const
    {
        assert(mInitialized);
        assert(caseIndex < mCaseInfoList.size());
        return mCaseInfoList[caseIndex];
    }

    size_t numCases() const
    {
        assert(mInitialized);
        return mCaseInfoList.size();
    }

  private:
    std::vector<CaseInfo> mCaseInfoList;
    bool mInitialized = false;
};

OpenCLCaseList::OpenCLCaseList() {}

const OpenCLCaseList &GetTestList()
{
    angle::TestSuite *instance = angle::TestSuite::GetInstance();
    static OpenCLCaseList sCaseList;
    sCaseList.initialize(instance);
    return sCaseList;
}

void OpenCLCaseList::initialize(angle::TestSuite *instance)
{
    mInitialized                           = true;
    constexpr char kTestExpectationsPath[] = "src/tests/cl_support/openclcts_expectations.txt";
    constexpr char kTestMustPassPath[]     = "src/tests/cl_support/openclcts_mustpass.txt";
    constexpr size_t kMaxPath              = 512;
    std::array<char, kMaxPath> foundDataPath;
    if (!angle::FindTestDataPath(kTestExpectationsPath, foundDataPath.data(), foundDataPath.size()))
    {
        std::cerr << "Unable to find test expectations path (" << kTestExpectationsPath << ")\n";
        exit(EXIT_FAILURE);
    }
    if (!instance->loadAllTestExpectationsFromFile(std::string(foundDataPath.data())))
    {
        exit(EXIT_FAILURE);
    }
    if (!angle::FindTestDataPath(kTestMustPassPath, foundDataPath.data(), foundDataPath.size()))
    {
        std::cerr << "Unable to find test must pass list path (" << kTestMustPassPath << ")\n";
        exit(EXIT_FAILURE);
    }
    std::ifstream caseListStream(std::string(foundDataPath.data()));
    if (caseListStream.fail())
    {
        std::cerr << "Failed to load the case list." << std::endl;
        exit(EXIT_FAILURE);
    }

    while (!caseListStream.eof())
    {
        std::string inString;
        std::getline(caseListStream, inString);

        std::string testName = TrimString(inString, angle::kWhitespaceASCII);
        if (testName.empty())
            continue;
        int expectation = instance->getTestExpectation(testName);
        mCaseInfoList.push_back(CaseInfo(testName, expectation));
    }
}

class OpenCLTestSuiteStats
{
  public:
    OpenCLTestSuiteStats() {}

  private:
    void setUpTestStats();
    void printTestStats();
    void countTestResult(int result);

    uint32_t mTestCount;
    uint32_t mPassedTestCount;
    uint32_t mFailedTestCount;
    uint32_t mTestExceptionCount;
    uint32_t mNotSupportedTestCount;
    uint32_t mSkippedTestCount;

    std::vector<std::string> mUnexpectedFailed;
    std::vector<std::string> mUnexpectedPasses;

    friend class OpenCLTest;
};

void OpenCLTestSuiteStats::setUpTestStats()
{
    mPassedTestCount       = 0;
    mFailedTestCount       = 0;
    mNotSupportedTestCount = 0;
    mTestExceptionCount    = 0;
    mTestCount             = 0;
    mSkippedTestCount      = 0;
    mUnexpectedPasses.clear();
    mUnexpectedFailed.clear();
}

std::string GetTestStatLine(const std::string &key, const std::string &value)
{
    return std::string(kInfoTag) + ": " + key + ": " + value + "\n";
}

void OpenCLTestSuiteStats::printTestStats()
{
    uint32_t crashedCount =
        mTestCount - (mPassedTestCount + mFailedTestCount + mNotSupportedTestCount +
                      mTestExceptionCount + mSkippedTestCount);

    std::cout << GetTestStatLine("Total", std::to_string(mTestCount));
    std::cout << GetTestStatLine("Passed", std::to_string(mPassedTestCount));
    std::cout << GetTestStatLine("Failed", std::to_string(mFailedTestCount));
    std::cout << GetTestStatLine("Skipped", std::to_string(mSkippedTestCount));
    std::cout << GetTestStatLine("Not Supported", std::to_string(mNotSupportedTestCount));
    std::cout << GetTestStatLine("Exception", std::to_string(mTestExceptionCount));
    std::cout << GetTestStatLine("Crashed", std::to_string(crashedCount));

    if (!mUnexpectedPasses.empty())
    {
        std::cout << GetTestStatLine("Unexpected Passed Count",
                                     std::to_string(mUnexpectedPasses.size()));
        for (const std::string &testName : mUnexpectedPasses)
        {
            std::cout << GetTestStatLine("Unexpected Passed Tests", testName);
        }
    }

    if (!mUnexpectedFailed.empty())
    {
        std::cout << GetTestStatLine("Unexpected Failed Count",
                                     std::to_string(mUnexpectedFailed.size()));
        for (const std::string &testName : mUnexpectedFailed)
        {
            std::cout << GetTestStatLine("Unexpected Failed Tests", testName);
        }
    }
}

void OpenCLTestSuiteStats::countTestResult(int result)
{
    switch (result)
    {
        case EXIT_SUCCESS:
            mPassedTestCount++;
            break;
        default:
            mFailedTestCount++;
            break;
    }
}

class OpenCLTest : public testing::Test
{
  public:
    OpenCLTest(size_t caseIndex) : mTestCaseIndex(caseIndex) {}

    static void SetUpTestSuite();
    static void TearDownTestSuite();

  protected:
    void TestBody() override;

  private:
    size_t mTestCaseIndex = 0;

    static OpenCLTestSuiteStats sTestSuiteData;
};

OpenCLTestSuiteStats OpenCLTest::sTestSuiteData = OpenCLTestSuiteStats();

// static function called once before running all of OpenCLTest under the same test suite
void OpenCLTest::SetUpTestSuite()
{
    sTestSuiteData.setUpTestStats();
}

// static function called once after running all of OpenCLTest under the same test suite
void OpenCLTest::TearDownTestSuite()
{
    sTestSuiteData.printTestStats();
}

void OpenCLTest::TestBody()
{
    const auto &caseInfo = GetTestList().getCaseInfo(mTestCaseIndex);

    // Tests that crash exit the harness before collecting the result. To tally the number of
    // crashed tests we track how many tests we "tried" to run.
    sTestSuiteData.mTestCount++;

    if (caseInfo.expectation == angle::GPUTestExpectationsParser::kGpuTestSkip)
    {
        sTestSuiteData.mSkippedTestCount++;
        std::cout << "Test skipped.\n";
        return;
    }

    // test name and options
    std::vector<std::string> testNameVector;
    angle::SplitStringAlongWhitespace(caseInfo.testNameWithArgs, &testNameVector);

    std::vector<const char *> argv;
    argv.push_back(caseInfo.testSuiteName.c_str());
    for (const auto &str : testNameVector)
    {
        argv.push_back(str.c_str());
    }
    int argc = argv.size();
    // C requires argv[argc] shall be null pointer
    argv.push_back(nullptr);

    const int result = ANGLE_oclcts_main(argc, argv.data());

    sTestSuiteData.countTestResult(result);
    if (caseInfo.expectation == angle::GPUTestExpectationsParser::kGpuTestPass ||
        caseInfo.expectation == angle::GPUTestExpectationsParser::kGpuTestFlaky)
    {
        EXPECT_EQ(result, EXIT_SUCCESS);
        if (result != EXIT_SUCCESS)
        {
            sTestSuiteData.mUnexpectedFailed.push_back(caseInfo.testCaseDescription);
        }
    }
    else if (result == EXIT_SUCCESS)
    {
        std::cout << "Test expected to fail but passed!" << std::endl;
        sTestSuiteData.mUnexpectedPasses.push_back(caseInfo.testCaseDescription);
    }
}

void RegisterCLCTSTests()
{
    const OpenCLCaseList &testList = GetTestList();
    for (size_t caseIndex = 0; caseIndex < testList.numCases(); caseIndex++)
    {
        auto factory = [caseIndex]() { return new OpenCLTest(caseIndex); };

        // The mustpass list contains lines in the form suite.name.  There is one executable per
        // test suite, and the suite name can be found in ANGLE_CL_SUITE_NAME.  Tests that are from
        // other suites are excluded for this executable.
        //
        // Note that the CTS has groups of test suites, but so far the test suite names alone are
        // unique so the group name is not placed in the mustpass.
        const std::string &testSuiteName = testList.getCaseInfo(caseIndex).testSuiteName;
        const std::string &testName      = testList.getCaseInfo(caseIndex).testNameWithArgs;

        if (testSuiteName != ANGLE_CL_SUITE_NAME)
        {
            continue;
        }

        testing::RegisterTest(testSuiteName.c_str(), testName.c_str(), nullptr, nullptr, __FILE__,
                              __LINE__, factory);
    }
}
}  // anonymous namespace

int main(int argc, char **argv)
{
    // Set up the environment for the CTS
#ifdef ANGLE_OPENCL_ICD_PATH
    {
        // The icd file is placed in the executable directory
        const std::string moduleDir = angle::GetModuleDirectory();
        const std::string icdPath   = angle::ConcatenatePath(moduleDir, ANGLE_OPENCL_ICD_PATH);
        angle::SetEnvironmentVar("OCL_ICD_VENDORS", icdPath.c_str());
    }
#endif  // ANGLE_OPENCL_ICD_PATH

    // TODO: Fix TestSuite so that it "consumes" the args that it picks up, leaving any that is not
    // recognized.  The left over should then be passed to `ANGLE_oclcts_main` in `TestBody`.
    // http://anglebug.com/372722560
    angle::TestSuite testSuite(&argc, argv, RegisterCLCTSTests);
    return testSuite.run();
}
