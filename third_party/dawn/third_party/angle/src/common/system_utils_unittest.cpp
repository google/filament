//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// system_utils_unittest.cpp: Unit tests for ANGLE's system utility functions

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "common/mathutil.h"
#include "common/system_utils.h"
#include "util/test_utils.h"

#include <fstream>
#include <sstream>
#include <vector>

#if defined(ANGLE_PLATFORM_POSIX)
#    include <signal.h>
#endif

using namespace angle;

namespace
{
// Test getting the executable path
TEST(SystemUtils, ExecutablePath)
{
    // TODO: fuchsia support. http://anglebug.com/42261836
#if !defined(ANGLE_PLATFORM_FUCHSIA)
    std::string executablePath = GetExecutablePath();
    EXPECT_NE("", executablePath);
#endif
}

// Test getting the executable directory
TEST(SystemUtils, ExecutableDir)
{
    // TODO: fuchsia support. http://anglebug.com/42261836
#if !defined(ANGLE_PLATFORM_FUCHSIA)
    std::string executableDir = GetExecutableDirectory();
    EXPECT_NE("", executableDir);

    std::string executablePath = GetExecutablePath();
    EXPECT_LT(executableDir.size(), executablePath.size());
    EXPECT_EQ(0, strncmp(executableDir.c_str(), executablePath.c_str(), executableDir.size()));
#endif
}

// Test setting environment variables
TEST(SystemUtils, Environment)
{
    constexpr char kEnvVarName[]  = "UNITTEST_ENV_VARIABLE";
    constexpr char kEnvVarValue[] = "The quick brown fox jumps over the lazy dog";

    bool setEnvDone = SetEnvironmentVar(kEnvVarName, kEnvVarValue);
    EXPECT_TRUE(setEnvDone);

    std::string readback = GetEnvironmentVar(kEnvVarName);
    EXPECT_EQ(kEnvVarValue, readback);

    bool unsetEnvDone = UnsetEnvironmentVar(kEnvVarName);
    EXPECT_TRUE(unsetEnvDone);

    readback = GetEnvironmentVar(kEnvVarName);
    EXPECT_EQ("", readback);
}

// Test CPU time measurement with a small operation
// (pretty much the measurement itself)
TEST(SystemUtils, CpuTimeSmallOp)
{
    double cpuTimeStart = GetCurrentProcessCpuTime();
    double cpuTimeEnd   = GetCurrentProcessCpuTime();
    EXPECT_GE(cpuTimeEnd, cpuTimeStart);
}

// Test CPU time measurement with a sleepy operation
TEST(SystemUtils, CpuTimeSleepy)
{
    double cpuTimeStart = GetCurrentProcessCpuTime();
    angle::Sleep(1);
    double cpuTimeEnd = GetCurrentProcessCpuTime();
    EXPECT_GE(cpuTimeEnd, cpuTimeStart);
}

// Test CPU time measurement with a heavy operation
TEST(SystemUtils, CpuTimeHeavyOp)
{
    constexpr size_t bufferSize = 1048576;
    std::vector<uint8_t> buffer(bufferSize, 1);
    double cpuTimeStart = GetCurrentProcessCpuTime();
    memset(buffer.data(), 0, bufferSize);
    double cpuTimeEnd = GetCurrentProcessCpuTime();
    EXPECT_GE(cpuTimeEnd, cpuTimeStart);
}

#if defined(ANGLE_PLATFORM_POSIX)
TEST(SystemUtils, ConcatenatePathSimple)
{
    std::string path1    = "/this/is/path1";
    std::string path2    = "this/is/path2";
    std::string expected = "/this/is/path1/this/is/path2";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePath1Empty)
{
    std::string path1    = "";
    std::string path2    = "this/is/path2";
    std::string expected = "this/is/path2";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePath2Empty)
{
    std::string path1    = "/this/is/path1";
    std::string path2    = "";
    std::string expected = "/this/is/path1";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePath2FullPath)
{
    std::string path1    = "/this/is/path1";
    std::string path2    = "/this/is/path2";
    std::string expected = "/this/is/path2";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePathRedundantSeparators)
{
    std::string path1    = "/this/is/path1/";
    std::string path2    = "this/is/path2";
    std::string expected = "/this/is/path1/this/is/path2";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, IsFullPath)
{
    std::string path1 = "/this/is/path1/";
    std::string path2 = "this/is/path2";
    EXPECT_TRUE(IsFullPath(path1));
    EXPECT_FALSE(IsFullPath(path2));
}
#elif defined(ANGLE_PLATFORM_WINDOWS)
TEST(SystemUtils, ConcatenatePathSimple)
{
    std::string path1    = "C:\\this\\is\\path1";
    std::string path2    = "this\\is\\path2";
    std::string expected = "C:\\this\\is\\path1\\this\\is\\path2";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePath1Empty)
{
    std::string path1    = "";
    std::string path2    = "this\\is\\path2";
    std::string expected = "this\\is\\path2";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePath2Empty)
{
    std::string path1    = "C:\\this\\is\\path1";
    std::string path2    = "";
    std::string expected = "C:\\this\\is\\path1";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePath2FullPath)
{
    std::string path1    = "C:\\this\\is\\path1";
    std::string path2    = "C:\\this\\is\\path2";
    std::string expected = "C:\\this\\is\\path2";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePathRedundantSeparators)
{
    std::string path1    = "C:\\this\\is\\path1\\";
    std::string path2    = "this\\is\\path2";
    std::string expected = "C:\\this\\is\\path1\\this\\is\\path2";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePathRedundantSeparators2)
{
    std::string path1    = "C:\\this\\is\\path1\\";
    std::string path2    = "\\this\\is\\path2";
    std::string expected = "C:\\this\\is\\path1\\this\\is\\path2";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, ConcatenatePathRedundantSeparators3)
{
    std::string path1    = "C:\\this\\is\\path1";
    std::string path2    = "\\this\\is\\path2";
    std::string expected = "C:\\this\\is\\path1\\this\\is\\path2";
    EXPECT_EQ(ConcatenatePath(path1, path2), expected);
}

TEST(SystemUtils, IsFullPath)
{
    std::string path1 = "C:\\this\\is\\path1\\";
    std::string path2 = "this\\is\\path2";
    EXPECT_TRUE(IsFullPath(path1));
    EXPECT_FALSE(IsFullPath(path2));
}
#endif

// Temporary file creation is not supported on Android right now.
#if defined(ANGLE_PLATFORM_ANDROID)
#    define MAYBE_CreateAndDeleteTemporaryFile DISABLED_CreateAndDeleteTemporaryFile
#    define MAYBE_CreateAndDeleteFileInTempDir DISABLED_CreateAndDeleteFileInTempDir
#else
#    define MAYBE_CreateAndDeleteTemporaryFile CreateAndDeleteTemporaryFile
#    define MAYBE_CreateAndDeleteFileInTempDir CreateAndDeleteFileInTempDir
#endif  // defined(ANGLE_PLATFORM_ANDROID)

// Test creating/using temporary file
TEST(SystemUtils, MAYBE_CreateAndDeleteTemporaryFile)
{
    Optional<std::string> path = CreateTemporaryFile();
    ASSERT_TRUE(path.valid());
    ASSERT_TRUE(!path.value().empty());

    const std::string testContents = "test output";

    // Test writing
    std::ofstream out;
    out.open(path.value());
    ASSERT_TRUE(out.is_open());
    out << testContents;
    EXPECT_TRUE(out.good());
    out.close();

    // Test reading
    std::ifstream in;
    in.open(path.value());
    EXPECT_TRUE(in.is_open());
    std::ostringstream sstr;
    sstr << in.rdbuf();
    EXPECT_EQ(sstr.str(), testContents);
    in.close();

    // Test deleting
    EXPECT_TRUE(DeleteSystemFile(path.value().c_str()));
}

// Test creating/using file created in system's temporary directory
TEST(SystemUtils, MAYBE_CreateAndDeleteFileInTempDir)
{
    Optional<std::string> tempDir = GetTempDirectory();
    ASSERT_TRUE(tempDir.valid());

    Optional<std::string> path = CreateTemporaryFileInDirectory(tempDir.value());
    ASSERT_TRUE(path.valid());
    ASSERT_TRUE(!path.value().empty());

    const std::string testContents = "test output";

    // Test writing
    std::ofstream out;
    out.open(path.value());
    ASSERT_TRUE(out.is_open());
    out << "test output";
    EXPECT_TRUE(out.good());
    out.close();

    // Test reading
    std::ifstream in;
    in.open(path.value());
    EXPECT_TRUE(in.is_open());
    std::ostringstream sstr;
    sstr << in.rdbuf();
    EXPECT_EQ(sstr.str(), testContents);
    in.close();

    // Test deleting
    EXPECT_TRUE(DeleteSystemFile(path.value().c_str()));
}

// Test retrieving page size
TEST(SystemUtils, PageSize)
{
    size_t pageSize = GetPageSize();
    EXPECT_TRUE(pageSize > 0);
}

// mprotect is not supported on Fuchsia right now.
#if defined(ANGLE_PLATFORM_FUCHSIA)
#    define MAYBE_PageFaultHandlerInit DISABLED_PageFaultHandlerInit
#    define MAYBE_PageFaultHandlerProtect DISABLED_PageFaultHandlerProtect
#    define MAYBE_PageFaultHandlerDefaultHandler DISABLED_PageFaultHandlerDefaultHandler
// mprotect tests hang on macOS M1. Also applies to the iOS simulator.
#elif ANGLE_PLATFORM_MACOS || ANGLE_PLATFORM_IOS_FAMILY_SIMULATOR
#    define MAYBE_PageFaultHandlerInit PageFaultHandlerInit
#    define MAYBE_PageFaultHandlerProtect DISABLED_PageFaultHandlerProtect
#    define MAYBE_PageFaultHandlerDefaultHandler DISABLED_PageFaultHandlerDefaultHandler
#else
#    define MAYBE_PageFaultHandlerInit PageFaultHandlerInit
#    define MAYBE_PageFaultHandlerProtect PageFaultHandlerProtect
#    define MAYBE_PageFaultHandlerDefaultHandler PageFaultHandlerDefaultHandler
#endif  // defined(ANGLE_PLATFORM_FUCHSIA)

// Test initializing and cleaning up page fault handler.
TEST(SystemUtils, MAYBE_PageFaultHandlerInit)
{
    PageFaultCallback callback = [](uintptr_t address) {
        return PageFaultHandlerRangeType::InRange;
    };

    std::unique_ptr<PageFaultHandler> handler(CreatePageFaultHandler(callback));

    EXPECT_TRUE(handler->enable());
    EXPECT_TRUE(handler->disable());
}

// Test write protecting and unprotecting memory
TEST(SystemUtils, MAYBE_PageFaultHandlerProtect)
{
    size_t pageSize = GetPageSize();
    EXPECT_TRUE(pageSize > 0);

    std::vector<float> data   = std::vector<float>(100);
    size_t dataSize           = sizeof(float) * data.size();
    uintptr_t dataStart       = reinterpret_cast<uintptr_t>(data.data());
    uintptr_t protectionStart = rx::roundDownPow2(dataStart, pageSize);
    uintptr_t protectionEnd   = rx::roundUpPow2(dataStart + dataSize, pageSize);

    std::mutex mutex;
    bool handlerCalled = false;

    PageFaultCallback callback = [&mutex, pageSize, dataStart, dataSize,
                                  &handlerCalled](uintptr_t address) {
        if (address >= dataStart && address < dataStart + dataSize)
        {
            std::lock_guard<std::mutex> lock(mutex);
            uintptr_t pageStart = rx::roundDownPow2(address, pageSize);
            EXPECT_TRUE(UnprotectMemory(pageStart, pageSize));
            handlerCalled = true;
            return PageFaultHandlerRangeType::InRange;
        }
        else
        {
            return PageFaultHandlerRangeType::OutOfRange;
        }
    };

    std::unique_ptr<PageFaultHandler> handler(CreatePageFaultHandler(callback));
    handler->enable();

    size_t protectionSize = protectionEnd - protectionStart;

    // Test Protect
    EXPECT_TRUE(ProtectMemory(protectionStart, protectionSize));

    data[0] = 0.0;

    {
        std::lock_guard<std::mutex> lock(mutex);
        EXPECT_TRUE(handlerCalled);
    }

    // Test Protect and unprotect
    EXPECT_TRUE(ProtectMemory(protectionStart, protectionSize));
    EXPECT_TRUE(UnprotectMemory(protectionStart, protectionSize));

    handlerCalled = false;
    data[0]       = 0.0;
    EXPECT_FALSE(handlerCalled);

    // Clean up
    EXPECT_TRUE(handler->disable());
}

// Tests basic usage of StripFilenameFromPath.
TEST(SystemUtils, StripFilenameFromPathUsage)
{
    EXPECT_EQ(StripFilenameFromPath("/path/to/tests/angle_tests"), "/path/to/tests");
    EXPECT_EQ(StripFilenameFromPath("C:\\tests\\angle_tests.exe"), "C:\\tests");
    EXPECT_EQ(StripFilenameFromPath("angle_tests"), "");
}

#if defined(ANGLE_PLATFORM_POSIX)
std::mutex gCustomHandlerMutex;
bool gCustomHandlerCalled = false;

void CustomSegfaultHandlerFunction(int sig, siginfo_t *info, void *unused)
{
    std::lock_guard<std::mutex> lock(gCustomHandlerMutex);
    gCustomHandlerCalled = true;
}

// Test if the default page fault handler is called on OutOfRange.
TEST(SystemUtils, MAYBE_PageFaultHandlerDefaultHandler)
{
    size_t pageSize = GetPageSize();
    EXPECT_TRUE(pageSize > 0);

    std::vector<float> data   = std::vector<float>(100);
    size_t dataSize           = sizeof(float) * data.size();
    uintptr_t dataStart       = reinterpret_cast<uintptr_t>(data.data());
    uintptr_t protectionStart = rx::roundDownPow2(dataStart, pageSize);
    uintptr_t protectionEnd   = rx::roundUpPow2(dataStart + dataSize, pageSize);
    size_t protectionSize     = protectionEnd - protectionStart;

    // Create custom handler
    struct sigaction sigAction = {};
    sigAction.sa_flags         = SA_SIGINFO;
    sigAction.sa_sigaction     = &CustomSegfaultHandlerFunction;
    sigemptyset(&sigAction.sa_mask);

    struct sigaction oldSigSegv = {};
    ASSERT_TRUE(sigaction(SIGSEGV, &sigAction, &oldSigSegv) == 0);
    struct sigaction oldSigBus = {};
    ASSERT_TRUE(sigaction(SIGBUS, &sigAction, &oldSigBus) == 0);

    PageFaultCallback callback = [protectionStart, protectionSize](uintptr_t address) {
        EXPECT_TRUE(UnprotectMemory(protectionStart, protectionSize));
        return PageFaultHandlerRangeType::OutOfRange;
    };

    std::unique_ptr<PageFaultHandler> handler(CreatePageFaultHandler(callback));
    handler->enable();

    // Test Protect
    EXPECT_TRUE(ProtectMemory(protectionStart, protectionSize));

    data[0] = 0.0;

    {
        std::lock_guard<std::mutex> lock(gCustomHandlerMutex);
        EXPECT_TRUE(gCustomHandlerCalled);
    }

    // Clean up
    EXPECT_TRUE(handler->disable());

    ASSERT_TRUE(sigaction(SIGSEGV, &oldSigSegv, nullptr) == 0);
    ASSERT_TRUE(sigaction(SIGBUS, &oldSigBus, nullptr) == 0);
}
#else
TEST(SystemUtils, MAYBE_PageFaultHandlerDefaultHandler) {}
#endif

// Tests basic usage of GetCurrentThreadId.
TEST(SystemUtils, GetCurrentThreadId)
{
    constexpr size_t kThreadCount = 64;

    std::mutex mutex;
    std::condition_variable condVar;

    std::vector<std::thread> threads;
    std::set<ThreadId> threadIds;
    size_t readyCount = 0;

    for (size_t i = 0; i < kThreadCount; ++i)
    {
        threads.emplace_back([&]() {
            std::unique_lock<std::mutex> lock(mutex);

            // Get threadId
            const angle::ThreadId threadId = angle::GetCurrentThreadId();
            EXPECT_NE(threadId, angle::InvalidThreadId());
            threadIds.insert(threadId);

            // Allow thread to finish only when all threads received the id.
            // Otherwise new thread may reuse Id of already completed but not joined thread.
            // Problem can be reproduced on Window platform, for example.
            ++readyCount;
            if (readyCount < kThreadCount)
            {
                condVar.wait(lock, [&]() { return readyCount == kThreadCount; });
            }
            else
            {
                condVar.notify_all();
            }

            // Check that threadId is still the same.
            EXPECT_EQ(threadId, angle::GetCurrentThreadId());
        });
    }

    for (size_t i = 0; i < kThreadCount; ++i)
    {
        threads[i].join();
    }

    // Check that all threadIds were unique.
    EXPECT_EQ(threadIds.size(), kThreadCount);
}

}  // anonymous namespace
