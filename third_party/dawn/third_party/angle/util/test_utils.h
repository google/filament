//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// test_utils.h: declaration of OS-specific utility functions

#ifndef UTIL_TEST_UTILS_H_
#define UTIL_TEST_UTILS_H_

#include <functional>
#include <string>
#include <vector>

#include "common/angleutils.h"
#include "util/Timer.h"

namespace angle
{
// Cross platform equivalent of the Windows Sleep function
void Sleep(unsigned int milliseconds);

void SetLowPriorityProcess();

// Write a debug message, either to a standard output or Debug window.
void WriteDebugMessage(const char *format, ...);

// Set thread affinity and priority.
bool StabilizeCPUForBenchmarking();

// Set a crash handler to print stack traces.
using CrashCallback = std::function<void()>;
void InitCrashHandler(CrashCallback *callback);
void TerminateCrashHandler();

// Print a stack back trace.
void PrintStackBacktrace();

// Deletes a file or directory.
bool DeleteSystemFile(const char *path);

// Reads a file contents into a string. Note: this method cannot be exported across a shared module
// boundary because it does memory allocation.
bool ReadEntireFileToString(const char *filePath, std::string *contentsOut);

// Compute a file's size.
bool GetFileSize(const char *filePath, uint32_t *sizeOut);

class ProcessHandle;

class Process : angle::NonCopyable
{
  public:
    virtual bool started()    = 0;
    virtual bool finished()   = 0;
    virtual bool finish()     = 0;
    virtual bool kill()       = 0;
    virtual int getExitCode() = 0;

    double getElapsedTimeSeconds() const { return mTimer.getElapsedWallClockTime(); }
    const std::string &getStdout() const { return mStdout; }
    const std::string &getStderr() const { return mStderr; }

  protected:
    friend class ProcessHandle;
    virtual ~Process();

    Timer mTimer;
    std::string mStdout;
    std::string mStderr;
};

enum class ProcessOutputCapture
{
    Nothing,
    // Capture stdout only
    StdoutOnly,
    // Capture stdout, and pipe stderr to stdout
    StdoutAndStderrInterleaved,
    // Capture stdout and stderr separately
    StdoutAndStderrSeparately,
};

class ProcessHandle final : angle::NonCopyable
{
  public:
    ProcessHandle();
    ProcessHandle(Process *process);
    ProcessHandle(const std::vector<const char *> &args, ProcessOutputCapture captureOutput);
    ~ProcessHandle();
    ProcessHandle(ProcessHandle &&other);
    ProcessHandle &operator=(ProcessHandle &&rhs);

    Process *operator->() { return mProcess; }
    const Process *operator->() const { return mProcess; }

    operator bool() const { return mProcess != nullptr; }

    void reset();

  private:
    Process *mProcess;
};

// Launch a process and optionally get the output. Uses a vector of c strings as command line
// arguments to the child process. Returns a Process handle which can be used to retrieve
// the stdout and stderr outputs as well as the exit code.
//
// Pass false for stdoutOut/stderrOut if you don't need to capture them.
//
// On success, returns a Process pointer with started() == true.
// On failure, returns a Process pointer with started() == false.
Process *LaunchProcess(const std::vector<const char *> &args, ProcessOutputCapture captureOutput);

int NumberOfProcessors();

const char *GetNativeEGLLibraryNameWithExtension();

// Intercept Metal shader cache access to avoid slow caching mechanism that caused the test timeout
// in the past. Note:
// - If there is NO "--skip-file-hooking" switch in the argument list:
//   - This function will re-launch the app with additional argument "--skip-file-hooking".
//   - The running process's image & memory will be re-created.
// - If there is "--skip-file-hooking" switch in the argument list, this function will do nothing.
#if defined(ANGLE_PLATFORM_APPLE)
void InitMetalFileAPIHooking(int argc, char **argv);
#endif

enum ArgHandling
{
    Delete,
    Preserve,
};

bool ParseIntArg(const char *flag, int *argc, char **argv, int argIndex, int *valueOut);
bool ParseFlag(const char *flag, int *argc, char **argv, int argIndex, bool *flagOut);
bool ParseStringArg(const char *flag, int *argc, char **argv, int argIndex, std::string *valueOut);
bool ParseCStringArg(const char *flag, int *argc, char **argv, int argIndex, const char **valueOut);

// Note: return value is always false with ArgHandling::Preserve handling
bool ParseIntArgWithHandling(const char *flag,
                             int *argc,
                             char **argv,
                             int argIndex,
                             int *valueOut,
                             ArgHandling handling);
bool ParseCStringArgWithHandling(const char *flag,
                                 int *argc,
                                 char **argv,
                                 int argIndex,
                                 const char **valueOut,
                                 ArgHandling handling);

void AddArg(int *argc, char **argv, const char *arg);

uint32_t GetPlatformANGLETypeFromArg(const char *useANGLEArg, uint32_t defaultPlatformType);
uint32_t GetANGLEDeviceTypeFromArg(const char *useANGLEArg, uint32_t defaultDeviceType);
}  // namespace angle

#endif  // UTIL_TEST_UTILS_H_
