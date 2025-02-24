//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// angle_deqp_gtest:
//   dEQP and GoogleTest integration logic. Calls through to the random
//   order executor.

#include <stdint.h>
#include <array>
#include <fstream>

#include <gtest/gtest.h>

#include "angle_deqp_libtester.h"
#include "common/Optional.h"
#include "common/angleutils.h"
#include "common/base/anglebase/no_destructor.h"
#include "common/debug.h"
#include "common/platform.h"
#include "common/string_utils.h"
#include "common/system_utils.h"
#include "platform/PlatformMethods.h"
#include "tests/test_utils/runner/TestSuite.h"
#include "util/OSWindow.h"
#include "util/test_utils.h"

namespace angle
{
namespace
{
#if !defined(NDEBUG)
constexpr bool kIsDebug = true;
#else
constexpr bool kIsDebug = false;
#endif  // !defined(NDEBUG)

bool gGlobalError = false;
bool gExpectError = false;
bool gVerbose     = false;

// Set this to true temporarily to enable image logging in release. Useful for diagnosing
// errors.
bool gLogImages = kIsDebug;

constexpr char kInfoTag[] = "*RESULT";

void HandlePlatformError(PlatformMethods *platform, const char *errorMessage)
{
    if (!gExpectError)
    {
        FAIL() << errorMessage;
    }
    gGlobalError = true;
}

// Relative to the ANGLE root folder.
constexpr char kCTSRootPath[] = "third_party/VK-GL-CTS/src/";
constexpr char kSupportPath[] = "src/tests/deqp_support/";

#define GLES_CTS_DIR(PATH) "external/openglcts/data/gl_cts/data/mustpass/gles/" PATH
#define GL_CTS_DIR(PATH) "external/openglcts/data/gl_cts/data/mustpass/gl/" PATH
#define EGL_CTS_DIR(PATH) "external/openglcts/data/gl_cts/data/mustpass/egl/" PATH

const char *gCaseListFiles[] = {
    EGL_CTS_DIR("aosp_mustpass/main/egl-main.txt"),
    GLES_CTS_DIR("aosp_mustpass/main/gles2-main.txt"),
    GLES_CTS_DIR("aosp_mustpass/main/gles3-main.txt"),
    GLES_CTS_DIR("aosp_mustpass/main/gles31-main.txt"),
    GLES_CTS_DIR("khronos_mustpass/main/gles2-khr-main.txt"),
    GLES_CTS_DIR("khronos_mustpass/main/gles3-khr-main.txt"),
    GLES_CTS_DIR("khronos_mustpass/main/gles31-khr-main.txt"),
    GLES_CTS_DIR("khronos_mustpass/main/gles32-khr-main.txt"),
    GLES_CTS_DIR("khronos_mustpass_noctx/main/gles2-khr-noctx-main.txt"),
    GLES_CTS_DIR("khronos_mustpass_noctx/main/gles32-khr-noctx-main.txt"),
    GLES_CTS_DIR("khronos_mustpass_single/main/gles32-khr-single.txt"),
    GLES_CTS_DIR("aosp_mustpass/main/gles3-rotate-landscape.txt"),
    GLES_CTS_DIR("aosp_mustpass/main/gles3-rotate-reverse-portrait.txt"),
    GLES_CTS_DIR("aosp_mustpass/main/gles3-rotate-reverse-landscape.txt"),
    GLES_CTS_DIR("aosp_mustpass/main/gles31-rotate-landscape.txt"),
    GLES_CTS_DIR("aosp_mustpass/main/gles31-rotate-reverse-portrait.txt"),
    GLES_CTS_DIR("aosp_mustpass/main/gles31-rotate-reverse-landscape.txt"),
    GLES_CTS_DIR("aosp_mustpass/main/gles3-multisample.txt"),
    GLES_CTS_DIR("aosp_mustpass/main/gles3-565-no-depth-no-stencil.txt"),
    GLES_CTS_DIR("aosp_mustpass/main/gles31-multisample.txt"),
    GLES_CTS_DIR("aosp_mustpass/main/gles31-565-no-depth-no-stencil.txt"),
};

const std::vector<const char *> gTestSuiteConfigParameters[] = {
    {"--deqp-gl-config-name=rgba8888d24s8ms0"},  // egl
    {"--deqp-gl-config-name=rgba8888d24s8ms0"},  // gles2
    {"--deqp-gl-config-name=rgba8888d24s8ms0"},  // gles3
    {"--deqp-gl-config-name=rgba8888d24s8ms0"},  // gles31
    {"--deqp-gl-config-name=rgba8888d24s8ms0"},  // gles2_khr
    {"--deqp-gl-config-name=rgba8888d24s8ms0"},  // gles3_khr
    {"--deqp-gl-config-name=rgba8888d24s8ms0"},  // gles31_khr
    {"--deqp-gl-config-name=rgba8888d24s8ms0"},  // gles32_khr
    {"--deqp-gl-config-name=rgba8888d24s8ms0"},  // gles2_khr_noctx
    {"--deqp-gl-config-name=rgba8888d24s8ms0"},  // gles32_khr_noctx
    {"--deqp-gl-config-name=rgba8888d24s8ms0"},  // gles32_khr_single
    {"--deqp-gl-config-name=rgba8888d24s8ms0"},  // gles3_rotate90
    {"--deqp-gl-config-name=rgba8888d24s8ms0"},  // gles3_rotate180
    {"--deqp-gl-config-name=rgba8888d24s8ms0"},  // gles3_rotate270
    {"--deqp-gl-config-name=rgba8888d24s8ms0"},  // gles31_rotate90
    {"--deqp-gl-config-name=rgba8888d24s8ms0"},  // gles31_rotate180
    {"--deqp-gl-config-name=rgba8888d24s8ms0"},  // gles31_rotate270
    {"--deqp-gl-config-name=rgba8888d24s8ms4"},  // gles3_multisample
    {"--deqp-gl-config-name=rgb565d0s0ms0"},     // gles3_rgb565_no_depth_no_stencil
    {"--deqp-gl-config-name=rgba8888d24s8ms4"},  // gles31_multisample
    {"--deqp-gl-config-name=rgb565d0s0ms0"},     // gles31_rgb565_no_depth_no_stencil
};

#undef GLES_CTS_DIR
#undef GL_CTS_DIR

const char *gTestExpectationsFiles[] = {
    "deqp_egl_test_expectations.txt",
    "deqp_gles2_test_expectations.txt",
    "deqp_gles3_test_expectations.txt",
    "deqp_gles31_test_expectations.txt",
    "deqp_khr_gles2_test_expectations.txt",
    "deqp_khr_gles3_test_expectations.txt",
    "deqp_khr_gles31_test_expectations.txt",
    "deqp_khr_gles32_test_expectations.txt",
    "deqp_khr_noctx_gles2_test_expectations.txt",
    "deqp_khr_noctx_gles32_test_expectations.txt",
    "deqp_khr_single_gles32_test_expectations.txt",
    "deqp_gles3_rotate_test_expectations.txt",
    "deqp_gles3_rotate_test_expectations.txt",
    "deqp_gles3_rotate_test_expectations.txt",
    "deqp_gles31_rotate_test_expectations.txt",
    "deqp_gles31_rotate_test_expectations.txt",
    "deqp_gles31_rotate_test_expectations.txt",
    "deqp_gles3_multisample_test_expectations.txt",
    "deqp_gles3_565_no_depth_no_stencil_test_expectations.txt",
    "deqp_gles31_multisample_test_expectations.txt",
    "deqp_gles31_565_no_depth_no_stencil_test_expectations.txt",
};

using APIInfo = std::pair<const char *, GPUTestConfig::API>;

constexpr APIInfo kEGLDisplayAPIs[] = {
#if defined(ANGLE_PLATFORM_ANDROID)
    {"native-gles", GPUTestConfig::kAPIGLES},
#endif
    {"angle-d3d9", GPUTestConfig::kAPID3D9},
    {"angle-d3d11", GPUTestConfig::kAPID3D11},
    {"angle-d3d11-ref", GPUTestConfig::kAPID3D11},
    {"angle-gl", GPUTestConfig::kAPIGLDesktop},
    {"angle-gles", GPUTestConfig::kAPIGLES},
    {"angle-metal", GPUTestConfig::kAPIMetal},
    {"angle-null", GPUTestConfig::kAPIUnknown},
    {"angle-swiftshader", GPUTestConfig::kAPISwiftShader},
    {"angle-vulkan", GPUTestConfig::kAPIVulkan},
    {"angle-webgpu", GPUTestConfig::kAPIWgpu},
    {"win32", GPUTestConfig::kAPIUnknown},
    {"x11", GPUTestConfig::kAPIUnknown}

};

constexpr char kdEQPEGLString[]             = "--deqp-egl-display-type=";
constexpr char kANGLEEGLString[]            = "--use-angle=";
constexpr char kANGLEPreRotation[]          = "--emulated-pre-rotation=";
constexpr char kdEQPCaseString[]            = "--deqp-case=";
constexpr char kVerboseString[]             = "--verbose";
constexpr char kRenderDocString[]           = "--renderdoc";
constexpr char kNoRenderDocString[]         = "--no-renderdoc";
constexpr char kdEQPFlagsPrefix[]           = "--deqp-";
constexpr char kGTestFilter[]               = "--gtest_filter=";
constexpr char kdEQPSurfaceWidth[]          = "--deqp-surface-width=";
constexpr char kdEQPSurfaceHeight[]         = "--deqp-surface-height=";
constexpr char kdEQPBaseSeed[]              = "--deqp-base-seed";
constexpr const char gdEQPLogImagesString[] = "--deqp-log-images=";

// Use the config name defined in gTestSuiteConfigParameters by default
// If gEGLConfigNameFromCmdLine is overwritten by --deqp-gl-config-name passed from command
// line arguments, for example:
// out/Debug/angle_deqp_egl_tests --verbose --deqp-gl-config-name=rgba8888d24s8
// use gEGLConfigNameFromCmdLine (rgba8888d24s8) instead.
// Invalid --deqp-gl-config-name value passed from command line arguments will be caught by
// glu::parseConfigBitsFromName() defined in gluRenderConfig.cpp, and it will cause tests
// to fail
constexpr const char gdEQPEGLConfigNameString[] = "--deqp-gl-config-name=";
const char *gEGLConfigNameFromCmdLine           = "";

angle::base::NoDestructor<std::vector<char>> gFilterStringBuffer;

// For angle_deqp_gles3*_rotateN_tests, default gOptions.preRotation to N.
#if defined(ANGLE_DEQP_GLES3_ROTATE90_TESTS) || defined(ANGLE_DEQP_GLES31_ROTATE90_TESTS)
constexpr uint32_t kDefaultPreRotation = 90;
#elif defined(ANGLE_DEQP_GLES3_ROTATE180_TESTS) || defined(ANGLE_DEQP_GLES31_ROTATE180_TESTS)
constexpr uint32_t kDefaultPreRotation = 180;
#elif defined(ANGLE_DEQP_GLES3_ROTATE270_TESTS) || defined(ANGLE_DEQP_GLES31_ROTATE270_TESTS)
constexpr uint32_t kDefaultPreRotation = 270;
#else
constexpr uint32_t kDefaultPreRotation = 0;
#endif

#if defined(ANGLE_TEST_ENABLE_RENDERDOC_CAPTURE)
constexpr bool kEnableRenderDocCapture = true;
#else
constexpr bool kEnableRenderDocCapture = false;
#endif

constexpr dEQPDriverOption kDeqpDriverOption = dEQPDriverOption::ANGLE;

const APIInfo *gInitAPI = nullptr;
dEQPOptions gOptions    = {
    kDefaultPreRotation,      // preRotation
    kEnableRenderDocCapture,  // enableRenderDocCapture
    kDeqpDriverOption,        // useANGLE
};

std::vector<const char *> gdEQPForwardFlags;

// Returns the default API for a platform.
const char *GetDefaultAPIName()
{
#if defined(ANGLE_PLATFORM_ANDROID) || defined(ANGLE_PLATFORM_LINUX) || \
    defined(ANGLE_PLATFORM_WINDOWS)
    return "angle-vulkan";
#elif defined(ANGLE_PLATFORM_APPLE)
    return "angle-gl";
#else
#    error Unknown platform.
#endif
}

const APIInfo *FindAPIInfo(const std::string &arg)
{
    for (auto &displayAPI : kEGLDisplayAPIs)
    {
        if (arg == displayAPI.first)
        {
            return &displayAPI;
        }
    }
    return nullptr;
}

const APIInfo *GetDefaultAPIInfo()
{
    const APIInfo *defaultInfo = FindAPIInfo(GetDefaultAPIName());
    ASSERT(defaultInfo);
    return defaultInfo;
}

std::string GetTestStatLine(const std::string &key, const std::string &value)
{
    return std::string(kInfoTag) + ": " + key + ": " + value + "\n";
}

// During the CaseList initialization we cannot use the GTEST FAIL macro to quit the program
// because the initialization is called outside of tests the first time.
void Die()
{
    exit(EXIT_FAILURE);
}

Optional<std::string> FindFileFromPath(const char *dirPath, const char *filePath)
{
    std::stringstream strstr;
    strstr << dirPath << filePath;
    std::string path = strstr.str();

    constexpr size_t kMaxFoundPathLen = 1000;
    char foundPath[kMaxFoundPathLen];
    if (angle::FindTestDataPath(path.c_str(), foundPath, kMaxFoundPathLen))
    {
        return std::string(foundPath);
    }

    return Optional<std::string>::Invalid();
}

Optional<std::string> FindCaseListPath(size_t testModuleIndex)
{
    return FindFileFromPath(kCTSRootPath, gCaseListFiles[testModuleIndex]);
}

Optional<std::string> FindTestExpectationsPath(size_t testModuleIndex)
{
    return FindFileFromPath(kSupportPath, gTestExpectationsFiles[testModuleIndex]);
}

size_t GetTestModuleIndex()
{
#ifdef ANGLE_DEQP_EGL_TESTS
    return 0;
#endif

#ifdef ANGLE_DEQP_GLES2_TESTS
    return 1;
#endif

#ifdef ANGLE_DEQP_GLES3_TESTS
    return 2;
#endif

#ifdef ANGLE_DEQP_GLES31_TESTS
    return 3;
#endif

#ifdef ANGLE_DEQP_KHR_GLES2_TESTS
    return 4;
#endif

#ifdef ANGLE_DEQP_KHR_GLES3_TESTS
    return 5;
#endif

#ifdef ANGLE_DEQP_KHR_GLES31_TESTS
    return 6;
#endif

#ifdef ANGLE_DEQP_KHR_GLES32_TESTS
    return 7;
#endif

#ifdef ANGLE_DEQP_KHR_NOCTX_GLES2_TESTS
    return 8;
#endif

#ifdef ANGLE_DEQP_KHR_NOCTX_GLES32_TESTS
    return 9;
#endif

#ifdef ANGLE_DEQP_KHR_SINGLE_GLES32_TESTS
    return 10;
#endif

#ifdef ANGLE_DEQP_GLES3_ROTATE90_TESTS
    return 11;
#endif

#ifdef ANGLE_DEQP_GLES3_ROTATE180_TESTS
    return 12;
#endif

#ifdef ANGLE_DEQP_GLES3_ROTATE270_TESTS
    return 13;
#endif

#ifdef ANGLE_DEQP_GLES31_ROTATE90_TESTS
    return 14;
#endif

#ifdef ANGLE_DEQP_GLES31_ROTATE180_TESTS
    return 15;
#endif

#ifdef ANGLE_DEQP_GLES31_ROTATE270_TESTS
    return 16;
#endif

#ifdef ANGLE_DEQP_GLES3_MULTISAMPLE_TESTS
    return 17;
#endif

#ifdef ANGLE_DEQP_GLES3_565_NO_DEPTH_NO_STENCIL_TESTS
    return 18;
#endif

#ifdef ANGLE_DEQP_GLES31_MULTISAMPLE_TESTS
    return 19;
#endif

#ifdef ANGLE_DEQP_GLES31_565_NO_DEPTH_NO_STENCIL_TESTS
    return 20;
#endif
}

class dEQPCaseList
{
  public:
    dEQPCaseList(size_t testModuleIndex);

    struct CaseInfo
    {
        CaseInfo(const std::string &testNameIn, int expectationIn)
            : testName(testNameIn), expectation(expectationIn)
        {}

        std::string testName;
        int expectation;
    };

    void initialize();

    const CaseInfo &getCaseInfo(size_t caseIndex) const
    {
        ASSERT(mInitialized);
        ASSERT(caseIndex < mCaseInfoList.size());
        return mCaseInfoList[caseIndex];
    }

    size_t numCases() const
    {
        ASSERT(mInitialized);
        return mCaseInfoList.size();
    }

  private:
    std::vector<CaseInfo> mCaseInfoList;
    size_t mTestModuleIndex;
    bool mInitialized = false;
};

dEQPCaseList::dEQPCaseList(size_t testModuleIndex) : mTestModuleIndex(testModuleIndex) {}

void dEQPCaseList::initialize()
{
    if (mInitialized)
    {
        return;
    }

    mInitialized = true;

    Optional<std::string> caseListPath = FindCaseListPath(mTestModuleIndex);
    if (!caseListPath.valid())
    {
        std::cerr << "Failed to find case list file." << std::endl;
        Die();
    }

    Optional<std::string> testExpectationsPath = FindTestExpectationsPath(mTestModuleIndex);
    if (!testExpectationsPath.valid())
    {
        std::cerr << "Failed to find test expectations file." << std::endl;
        Die();
    }

    GPUTestConfig::API api = GetDefaultAPIInfo()->second;
    // Set the API from the command line, or using the default platform API.
    if (gInitAPI)
    {
        api = gInitAPI->second;
    }

    GPUTestConfig testConfig = GPUTestConfig(api, gOptions.preRotation);

#if !defined(ANGLE_PLATFORM_ANDROID)
    // Note: These prints mess up parsing of test list when running on Android.
    std::cout << "Using test config with:" << std::endl;
    for (uint32_t condition : testConfig.getConditions())
    {
        const char *name = GetConditionName(condition);
        if (name != nullptr)
        {
            std::cout << "  " << name << std::endl;
        }
    }
#endif

    TestSuite *testSuite = TestSuite::GetInstance();

    if (!testSuite->loadTestExpectationsFromFileWithConfig(testConfig,
                                                           testExpectationsPath.value()))
    {
        Die();
    }

    std::ifstream caseListStream(caseListPath.value());
    if (caseListStream.fail())
    {
        std::cerr << "Failed to load the case list." << std::endl;
        Die();
    }

    while (!caseListStream.eof())
    {
        std::string inString;
        std::getline(caseListStream, inString);

        std::string testName = TrimString(inString, kWhitespaceASCII);
        if (testName.empty())
            continue;
        int expectation = testSuite->getTestExpectation(testName);
        mCaseInfoList.push_back(CaseInfo(testName, expectation));
    }
}

bool IsPassingResult(dEQPTestResult result)
{
    // Check the global error flag for unexpected platform errors.
    if (gGlobalError)
    {
        gGlobalError = false;
        return false;
    }

    switch (result)
    {
        case dEQPTestResult::Fail:
        case dEQPTestResult::Exception:
            return false;

        default:
            return true;
    }
}

const dEQPCaseList &GetTestList(size_t testModuleIndex)
{
    static dEQPCaseList sCaseList(testModuleIndex);
    sCaseList.initialize();
    return sCaseList;
}

const std::string GetTestCaseName(size_t testModuleIndex, size_t caseIndex)
{
    const auto &caseInfo = GetTestList(testModuleIndex).getCaseInfo(caseIndex);
    return caseInfo.testName;
}

class dEQPTestSuiteStats
{
  public:
    dEQPTestSuiteStats() {}

  private:
    void setUpTestStats();
    void printTestStats();
    void countTestResult(dEQPTestResult result);

    uint32_t mTestCount;
    uint32_t mPassedTestCount;
    uint32_t mFailedTestCount;
    uint32_t mTestExceptionCount;
    uint32_t mNotSupportedTestCount;
    uint32_t mSkippedTestCount;

    std::vector<std::string> mUnexpectedFailed;
    std::vector<std::string> mUnexpectedPasses;

    friend class dEQPTest;
};

void dEQPTestSuiteStats::setUpTestStats()
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

void dEQPTestSuiteStats::printTestStats()
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

void dEQPTestSuiteStats::countTestResult(dEQPTestResult result)
{
    switch (result)
    {
        case dEQPTestResult::Pass:
            mPassedTestCount++;
            break;
        case dEQPTestResult::Fail:
            mFailedTestCount++;
            break;
        case dEQPTestResult::NotSupported:
            mNotSupportedTestCount++;
            break;
        case dEQPTestResult::Exception:
            mTestExceptionCount++;
            break;
        default:
            std::cerr << "Unexpected test result code: " << static_cast<int>(result) << "\n";
            break;
    }
}

class dEQPTest : public testing::Test
{
  public:
    dEQPTest(size_t testModuleIndex, size_t caseIndex)
        : mTestModuleIndex(testModuleIndex), mTestCaseIndex(caseIndex)
    {}

    static void SetUpTestSuite();
    static void TearDownTestSuite();

  protected:
    void TestBody() override;

  private:
    size_t mTestModuleIndex = 0;
    size_t mTestCaseIndex   = 0;

    static dEQPTestSuiteStats sTestSuiteData;
};

dEQPTestSuiteStats dEQPTest::sTestSuiteData = dEQPTestSuiteStats();

// static function called once before running all of dEQPTest under the same test suite
void dEQPTest::SetUpTestSuite()
{
    sTestSuiteData.setUpTestStats();

    std::vector<const char *> argv;

    // Reserve one argument for the binary name.
    argv.push_back("");

    // Add init api.
    const char *targetApi    = gInitAPI ? gInitAPI->first : GetDefaultAPIName();
    std::string apiArgString = std::string(kdEQPEGLString) + targetApi;
    argv.push_back(apiArgString.c_str());

    std::string configNameFromCmdLineString =
        std::string(gdEQPEGLConfigNameString) + gEGLConfigNameFromCmdLine;

    // Add test config parameters
    for (const char *configParam : gTestSuiteConfigParameters[GetTestModuleIndex()])
    {
        // Check if we pass --deqp-gl-config-name from the command line, if yes, use the one from
        // command line
        if (std::strlen(gEGLConfigNameFromCmdLine) > 0 &&
            std::strncmp(configParam, gdEQPEGLConfigNameString, strlen(gdEQPEGLConfigNameString)) ==
                0)
        {
            configParam = configNameFromCmdLineString.c_str();
        }
        argv.push_back(configParam);
    }

    // Hide SwiftShader window to prevent a race with Xvfb causing hangs on test bots
    if (gInitAPI && gInitAPI->second == GPUTestConfig::kAPISwiftShader)
    {
        argv.push_back("--deqp-visibility=hidden");
    }

    TestSuite *testSuite = TestSuite::GetInstance();

    std::stringstream logNameStream;
    logNameStream << "TestResults";
    if (testSuite->getBatchId() != -1)
    {
        logNameStream << "-Batch" << std::setfill('0') << std::setw(3) << testSuite->getBatchId();
    }
    logNameStream << ".qpa";

    std::stringstream logArgStream;
    logArgStream << "--deqp-log-filename="
                 << testSuite->reserveTestArtifactPath(logNameStream.str());

    std::string logNameString = logArgStream.str();
    argv.push_back(logNameString.c_str());

    if (!gLogImages)
    {
        argv.push_back("--deqp-log-images=disable");
    }

    // Flushing during multi-process execution punishes HDDs. http://anglebug.com/42263718
    if (testSuite->getBatchId() != -1)
    {
        argv.push_back("--deqp-log-flush=disable");
    }

    // Add any additional flags specified from command line to be forwarded to dEQP.
    argv.insert(argv.end(), gdEQPForwardFlags.begin(), gdEQPForwardFlags.end());

    // Init the platform.
    if (!deqp_libtester_init_platform(static_cast<int>(argv.size()), argv.data(),
                                      reinterpret_cast<void *>(&HandlePlatformError), gOptions))
    {
        std::cout << "Aborting test due to dEQP initialization error." << std::endl;
        exit(1);
    }
}

// static function called once after running all of dEQPTest under the same test suite
void dEQPTest::TearDownTestSuite()
{
    sTestSuiteData.printTestStats();
    deqp_libtester_shutdown_platform();
}

// TestBody() is called once for each dEQPTest
void dEQPTest::TestBody()
{
    if (sTestSuiteData.mTestExceptionCount > 1)
    {
        std::cout << "Too many exceptions, skipping all remaining tests." << std::endl;
        return;
    }

    const auto &caseInfo = GetTestList(mTestModuleIndex).getCaseInfo(mTestCaseIndex);

    // Tests that crash exit the harness before collecting the result. To tally the number of
    // crashed tests we track how many tests we "tried" to run.
    sTestSuiteData.mTestCount++;

    if (caseInfo.expectation == GPUTestExpectationsParser::kGpuTestSkip)
    {
        sTestSuiteData.mSkippedTestCount++;
        std::cout << "Test skipped.\n";
        return;
    }

    TestSuite *testSuite = TestSuite::GetInstance();
    testSuite->maybeUpdateTestTimeout(caseInfo.expectation);

    gExpectError          = (caseInfo.expectation != GPUTestExpectationsParser::kGpuTestPass);
    dEQPTestResult result = deqp_libtester_run(caseInfo.testName.c_str());

    bool testSucceeded = IsPassingResult(result);

    if (!testSucceeded && caseInfo.expectation == GPUTestExpectationsParser::kGpuTestFlaky)
    {
        result        = deqp_libtester_run(caseInfo.testName.c_str());
        testSucceeded = IsPassingResult(result);
    }

    sTestSuiteData.countTestResult(result);

    if (caseInfo.expectation == GPUTestExpectationsParser::kGpuTestPass ||
        caseInfo.expectation == GPUTestExpectationsParser::kGpuTestFlaky)
    {
        EXPECT_TRUE(testSucceeded);

        if (!testSucceeded)
        {
            sTestSuiteData.mUnexpectedFailed.push_back(caseInfo.testName);
        }
    }
    else if (testSucceeded)
    {
        std::cout << "Test expected to fail but passed!" << std::endl;
        sTestSuiteData.mUnexpectedPasses.push_back(caseInfo.testName);
    }
}

void HandleDisplayType(const char *displayTypeString)
{
    std::stringstream argStream;

    if (gInitAPI)
    {
        std::cout << "Cannot specify two EGL displays!" << std::endl;
        exit(1);
    }

    argStream << displayTypeString;
    std::string arg = argStream.str();
    gInitAPI        = FindAPIInfo(arg);

    if (!gInitAPI && strncmp(displayTypeString, "angle-", strlen("angle-")) != 0)
    {
        std::stringstream argStream2;
        argStream2 << "angle-" << displayTypeString;
        std::string arg2 = argStream2.str();
        gInitAPI         = FindAPIInfo(arg2);
    }

    if (!gInitAPI)
    {
        std::cout << "Unknown API: " << displayTypeString << std::endl;
        exit(1);
    }
}

void HandlePreRotation(const char *preRotationString)
{
    std::istringstream argStream(preRotationString);

    uint32_t preRotation = 0;
    argStream >> preRotation;

    if (!argStream ||
        (preRotation != 0 && preRotation != 90 && preRotation != 180 && preRotation != 270))
    {
        std::cout << "Invalid PreRotation '" << preRotationString
                  << "'; must be either 0, 90, 180 or 270" << std::endl;
        exit(1);
    }

    gOptions.preRotation = preRotation;
}

void HandleEGLConfigName(const char *configNameString)
{
    gEGLConfigNameFromCmdLine = configNameString;
}

// The --deqp-case flag takes a case expression that is parsed into a --gtest_filter. It
// converts the "dEQP" style names (functional.thing.*) into "GoogleTest" style names
// (functional_thing_*). Currently it does not handle multiple tests and multiple filters in
// different arguments.
void HandleFilterArg(const char *filterString, int *argc, int argIndex, char **argv)
{
    std::string googleTestFilter = ReplaceDashesWithQuestionMark(filterString);

    gFilterStringBuffer->resize(googleTestFilter.size() + 3 + strlen(kGTestFilter), 0);
    std::fill(gFilterStringBuffer->begin(), gFilterStringBuffer->end(), 0);

    int bytesWritten = snprintf(gFilterStringBuffer->data(), gFilterStringBuffer->size() - 1,
                                "%s*%s", kGTestFilter, googleTestFilter.c_str());
    if (bytesWritten <= 0 || static_cast<size_t>(bytesWritten) >= gFilterStringBuffer->size() - 1)
    {
        std::cout << "Error parsing filter string: " << filterString;
        exit(1);
    }

    argv[argIndex] = gFilterStringBuffer->data();
}

void HandleLogImages(const char *logImagesString)
{
    if (strcmp(logImagesString, "enable") == 0)
    {
        gLogImages = true;
    }
    else if (strcmp(logImagesString, "disable") == 0)
    {
        gLogImages = false;
    }
    else
    {
        std::cout << "Error parsing log images setting. Use enable/disable.";
        exit(1);
    }
}

void RegisterGLCTSTests()
{
    size_t testModuleIndex = GetTestModuleIndex();

    const dEQPCaseList &caseList = GetTestList(testModuleIndex);

    for (size_t caseIndex = 0; caseIndex < caseList.numCases(); ++caseIndex)
    {
        auto factory = [testModuleIndex, caseIndex]() {
            return new dEQPTest(testModuleIndex, caseIndex);
        };

        const std::string testCaseName = GetTestCaseName(testModuleIndex, caseIndex);
        size_t pos                     = testCaseName.find('.');
        ASSERT(pos != std::string::npos);
        // testCaseName comes from one of the mustpass files in gCaseListFiles.
        // All of the testCaseName in the same mustpass file starts with the same testSuiteName
        // prefix. Which mustpass file to load the set of testCaseName depends on testModuleIndex
        // compiled into the deqp test application binary. For now, only one testModuleIndex is
        // compiled in a deqp test application binary, meaning all of the tests invoked by same deqp
        // test application binary are under the same test suite.
        std::string testSuiteName = testCaseName.substr(0, pos);
        std::string testName      = testCaseName.substr(pos + 1);
        testing::RegisterTest(testSuiteName.c_str(), testName.c_str(), nullptr, nullptr, __FILE__,
                              __LINE__, factory);
    }
}
}  // anonymous namespace

// Called from main() to process command-line arguments.
int RunGLCTSTests(int *argc, char **argv)
{
    int argIndex = 0;
    while (argIndex < *argc)
    {
        if (strncmp(argv[argIndex], kdEQPEGLString, strlen(kdEQPEGLString)) == 0)
        {
            HandleDisplayType(argv[argIndex] + strlen(kdEQPEGLString));
        }
        else if (strncmp(argv[argIndex], kANGLEEGLString, strlen(kANGLEEGLString)) == 0)
        {
            HandleDisplayType(argv[argIndex] + strlen(kANGLEEGLString));
        }
        else if (strncmp(argv[argIndex], kANGLEPreRotation, strlen(kANGLEPreRotation)) == 0)
        {
            HandlePreRotation(argv[argIndex] + strlen(kANGLEPreRotation));
        }
        else if (strncmp(argv[argIndex], gdEQPEGLConfigNameString,
                         strlen(gdEQPEGLConfigNameString)) == 0)
        {
            HandleEGLConfigName(argv[argIndex] + strlen(gdEQPEGLConfigNameString));
        }
        else if (strncmp(argv[argIndex], kdEQPCaseString, strlen(kdEQPCaseString)) == 0)
        {
            HandleFilterArg(argv[argIndex] + strlen(kdEQPCaseString), argc, argIndex, argv);
        }
        else if (strncmp(argv[argIndex], kGTestFilter, strlen(kGTestFilter)) == 0)
        {
            HandleFilterArg(argv[argIndex] + strlen(kGTestFilter), argc, argIndex, argv);
        }
        else if (strncmp(argv[argIndex], kVerboseString, strlen(kVerboseString)) == 0 ||
                 strcmp(argv[argIndex], "-v") == 0)
        {
            gVerbose = true;
        }
        else if (strncmp(argv[argIndex], gdEQPLogImagesString, strlen(gdEQPLogImagesString)) == 0)
        {
            HandleLogImages(argv[argIndex] + strlen(gdEQPLogImagesString));
        }
        else if (strncmp(argv[argIndex], kRenderDocString, strlen(kRenderDocString)) == 0)
        {
            gOptions.enableRenderDocCapture = true;
        }
        else if (strncmp(argv[argIndex], kNoRenderDocString, strlen(kNoRenderDocString)) == 0)
        {
            gOptions.enableRenderDocCapture = false;
        }
        else if (strncmp(argv[argIndex], kdEQPFlagsPrefix, strlen(kdEQPFlagsPrefix)) == 0)
        {
            gdEQPForwardFlags.push_back(argv[argIndex]);
        }
        else if (strncmp(argv[argIndex], kdEQPSurfaceWidth, strlen(kdEQPSurfaceWidth)) == 0)
        {
            gdEQPForwardFlags.push_back(argv[argIndex]);
        }
        else if (strncmp(argv[argIndex], kdEQPSurfaceHeight, strlen(kdEQPSurfaceHeight)) == 0)
        {
            gdEQPForwardFlags.push_back(argv[argIndex]);
        }
        else if (strncmp(argv[argIndex], kdEQPBaseSeed, strlen(kdEQPBaseSeed)) == 0)
        {
            gdEQPForwardFlags.push_back(argv[argIndex]);
        }
        argIndex++;
    }

    GPUTestConfig::API api = GetDefaultAPIInfo()->second;
    if (gInitAPI)
    {
        api = gInitAPI->second;
        // On Android, if --deqp-egl-display-type=native-gles, set driverOption to NATIVE
        // We will load egl libs from native gles driver instead of ANGLE.
#if defined(ANGLE_PLATFORM_ANDROID)
        if (strcmp(gInitAPI->first, "native-gles") == 0)
        {
            gOptions.driverOption = dEQPDriverOption::NATIVE;
        }
#endif
    }
    if (gOptions.preRotation != 0 && api != GPUTestConfig::kAPIVulkan &&
        api != GPUTestConfig::kAPISwiftShader)
    {
        std::cout << "PreRotation is only supported on Vulkan" << std::endl;
        exit(1);
    }

    angle::TestSuite testSuite(argc, argv, RegisterGLCTSTests);
    return testSuite.run();
}
}  // namespace angle
