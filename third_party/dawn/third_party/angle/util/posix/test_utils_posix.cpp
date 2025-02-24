//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// test_utils_posix.cpp: Implementation of OS-specific functions for Posix systems

#include "util/test_utils.h"

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <cstdarg>
#include <cstring>
#include <iostream>

#include "common/debug.h"
#include "common/platform.h"
#include "common/system_utils.h"

#if !defined(ANGLE_PLATFORM_FUCHSIA)
#    include <sys/resource.h>
#endif

#if defined(ANGLE_PLATFORM_MACOS)
#    include <crt_externs.h>
#endif

namespace angle
{
namespace
{

#if defined(ANGLE_PLATFORM_MACOS)
// Argument to skip the file hooking step. Might be automatically added by InitMetalFileAPIHooking()
constexpr char kSkipFileHookingArg[] = "--skip-file-hooking";
#endif

struct ScopedPipe
{
    ~ScopedPipe()
    {
        closeEndPoint(0);
        closeEndPoint(1);
    }

    void closeEndPoint(int index)
    {
        if (fds[index] >= 0)
        {
            close(fds[index]);
            fds[index] = -1;
        }
    }

    bool valid() const { return fds[0] != -1 || fds[1] != -1; }

    int fds[2] = {
        -1,
        -1,
    };
};

enum class ReadResult
{
    NoData,
    GotData,
};

ReadResult ReadFromFile(int fd, std::string *out)
{
    constexpr size_t kBufSize = 2048;
    char buffer[kBufSize];
    ssize_t bytesRead = read(fd, buffer, sizeof(buffer));

    if (bytesRead < 0 && errno == EINTR)
    {
        return ReadResult::GotData;
    }

    if (bytesRead <= 0)
    {
        return ReadResult::NoData;
    }

    out->append(buffer, bytesRead);
    return ReadResult::GotData;
}

void ReadEntireFile(int fd, std::string *out)
{
    while (ReadFromFile(fd, out) == ReadResult::GotData)
    {
    }
}

class PosixProcess : public Process
{
  public:
    PosixProcess(const std::vector<const char *> &commandLineArgs,
                 ProcessOutputCapture captureOutput)
    {
        if (commandLineArgs.empty())
        {
            return;
        }

        const bool captureStdout = captureOutput != ProcessOutputCapture::Nothing;
        const bool captureStderr =
            captureOutput == ProcessOutputCapture::StdoutAndStderrInterleaved ||
            captureOutput == ProcessOutputCapture::StdoutAndStderrSeparately;
        const bool pipeStderrToStdout =
            captureOutput == ProcessOutputCapture::StdoutAndStderrInterleaved;

        // Create pipes for stdout and stderr.
        if (captureStdout)
        {
            if (pipe(mStdoutPipe.fds) != 0)
            {
                std::cerr << "Error calling pipe: " << errno << "\n";
                return;
            }
            if (fcntl(mStdoutPipe.fds[0], F_SETFL, O_NONBLOCK) == -1)
            {
                std::cerr << "Error calling fcntl: " << errno << "\n";
                return;
            }
        }
        if (captureStderr && !pipeStderrToStdout)
        {
            if (pipe(mStderrPipe.fds) != 0)
            {
                std::cerr << "Error calling pipe: " << errno << "\n";
                return;
            }
            if (fcntl(mStderrPipe.fds[0], F_SETFL, O_NONBLOCK) == -1)
            {
                std::cerr << "Error calling fcntl: " << errno << "\n";
                return;
            }
        }

        mPID = fork();
        if (mPID < 0)
        {
            return;
        }

        mStarted = true;
        mTimer.start();

        if (mPID == 0)
        {
            // Child.  Execute the application.

            // Redirect stdout and stderr to the pipe fds.
            if (captureStdout)
            {
                if (dup2(mStdoutPipe.fds[1], STDOUT_FILENO) < 0)
                {
                    _exit(errno);
                }
                mStdoutPipe.closeEndPoint(1);
            }
            if (pipeStderrToStdout)
            {
                if (dup2(STDOUT_FILENO, STDERR_FILENO) < 0)
                {
                    _exit(errno);
                }
            }
            else if (captureStderr)
            {
                if (dup2(mStderrPipe.fds[1], STDERR_FILENO) < 0)
                {
                    _exit(errno);
                }
                mStderrPipe.closeEndPoint(1);
            }

            // Execute the application, which doesn't return unless failed.  Note: execv takes argv
            // as `char * const *` for historical reasons.  It is safe to const_cast it:
            //
            // http://pubs.opengroup.org/onlinepubs/9699919799/functions/exec.html
            //
            // > The statement about argv[] and envp[] being constants is included to make explicit
            // to future writers of language bindings that these objects are completely constant.
            // Due to a limitation of the ISO C standard, it is not possible to state that idea in
            // standard C. Specifying two levels of const- qualification for the argv[] and envp[]
            // parameters for the exec functions may seem to be the natural choice, given that these
            // functions do not modify either the array of pointers or the characters to which the
            // function points, but this would disallow existing correct code. Instead, only the
            // array of pointers is noted as constant.
            std::vector<char *> args;
            for (const char *arg : commandLineArgs)
            {
                args.push_back(const_cast<char *>(arg));
            }
            args.push_back(nullptr);

            execv(commandLineArgs[0], args.data());
            std::cerr << "Error calling evecv: " << errno;
            _exit(errno);
        }
        // Parent continues execution.
        mStdoutPipe.closeEndPoint(1);
        mStderrPipe.closeEndPoint(1);
    }

    ~PosixProcess() override {}

    bool started() override { return mStarted; }

    bool finish() override
    {
        if (!mStarted)
        {
            return false;
        }

        if (mFinished)
        {
            return true;
        }

        while (!finished())
        {
            angle::Sleep(1);
        }

        return true;
    }

    bool finished() override
    {
        if (!mStarted)
        {
            return false;
        }

        if (mFinished)
        {
            return true;
        }

        int status        = 0;
        pid_t returnedPID = ::waitpid(mPID, &status, WNOHANG);

        if (returnedPID == -1 && errno != ECHILD)
        {
            std::cerr << "Error calling waitpid: " << ::strerror(errno) << "\n";
            return true;
        }

        if (returnedPID == mPID)
        {
            mFinished = true;
            mTimer.stop();
            readPipes();
            mExitCode = WEXITSTATUS(status);
            return true;
        }

        if (mStdoutPipe.valid())
        {
            ReadEntireFile(mStdoutPipe.fds[0], &mStdout);
        }

        if (mStderrPipe.valid())
        {
            ReadEntireFile(mStderrPipe.fds[0], &mStderr);
        }

        return false;
    }

    int getExitCode() override { return mExitCode; }

    bool kill() override
    {
        if (!mStarted)
        {
            return false;
        }

        if (finished())
        {
            return true;
        }

        return (::kill(mPID, SIGTERM) == 0);
    }

  private:
    void readPipes()
    {
        // Close the write end of the pipes, so EOF can be generated when child exits.
        // Then read back the output of the child.
        if (mStdoutPipe.valid())
        {
            ReadEntireFile(mStdoutPipe.fds[0], &mStdout);
        }
        if (mStderrPipe.valid())
        {
            ReadEntireFile(mStderrPipe.fds[0], &mStderr);
        }
    }

    bool mStarted  = false;
    bool mFinished = false;
    ScopedPipe mStdoutPipe;
    ScopedPipe mStderrPipe;
    int mExitCode = 0;
    pid_t mPID    = -1;
};
}  // anonymous namespace

void Sleep(unsigned int milliseconds)
{
    // On Windows Sleep(0) yields while it isn't guaranteed by Posix's sleep
    // so we replicate Windows' behavior with an explicit yield.
    if (milliseconds == 0)
    {
        sched_yield();
    }
    else
    {
        long milliseconds_long = milliseconds;
        timespec sleepTime     = {
                .tv_sec  = milliseconds_long / 1000,
                .tv_nsec = (milliseconds_long % 1000) * 1000000,
        };

        nanosleep(&sleepTime, nullptr);
    }
}

void SetLowPriorityProcess()
{
#if !defined(ANGLE_PLATFORM_FUCHSIA)
    setpriority(PRIO_PROCESS, getpid(), 10);
#endif
}

void WriteDebugMessage(const char *format, ...)
{
    va_list vararg;
    va_start(vararg, format);
    vfprintf(stderr, format, vararg);
    va_end(vararg);
}

bool StabilizeCPUForBenchmarking()
{
#if !defined(ANGLE_PLATFORM_FUCHSIA)
    bool success = true;
    errno        = 0;
    setpriority(PRIO_PROCESS, getpid(), -20);
    if (errno)
    {
        // A friendly warning in case the test was run without appropriate permission.
        perror(
            "Warning: setpriority failed in StabilizeCPUForBenchmarking. Process will retain "
            "default priority");
        success = false;
    }
#    if ANGLE_PLATFORM_LINUX
    cpu_set_t affinity;
    CPU_SET(0, &affinity);
    errno = 0;
    if (sched_setaffinity(getpid(), sizeof(affinity), &affinity))
    {
        perror(
            "Warning: sched_setaffinity failed in StabilizeCPUForBenchmarking. Process will retain "
            "default affinity");
        success = false;
    }
#    else
    // TODO(jmadill): Implement for non-linux. http://anglebug.com/40096532
#    endif

    return success;
#else  // defined(ANGLE_PLATFORM_FUCHSIA)
    return false;
#endif
}

bool DeleteSystemFile(const char *path)
{
    return unlink(path) == 0;
}

Process *LaunchProcess(const std::vector<const char *> &args, ProcessOutputCapture captureOutput)
{
    return new PosixProcess(args, captureOutput);
}

int NumberOfProcessors()
{
    // sysconf returns the number of "logical" (not "physical") processors on both
    // Mac and Linux.  So we get the number of max available "logical" processors.
    //
    // Note that the number of "currently online" processors may be fewer than the
    // returned value of NumberOfProcessors(). On some platforms, the kernel may
    // make some processors offline intermittently, to save power when system
    // loading is low.
    //
    // One common use case that needs to know the processor count is to create
    // optimal number of threads for optimization. It should make plan according
    // to the number of "max available" processors instead of "currently online"
    // ones. The kernel should be smart enough to make all processors online when
    // it has sufficient number of threads waiting to run.
    long res = sysconf(_SC_NPROCESSORS_CONF);
    if (res == -1)
    {
        return 1;
    }

    return static_cast<int>(res);
}

const char *GetNativeEGLLibraryNameWithExtension()
{
#if defined(ANGLE_PLATFORM_ANDROID)
    return "libEGL.so";
#elif defined(ANGLE_PLATFORM_LINUX)
    return "libEGL.so.1";
#else
    return "unknown_libegl";
#endif
}

#if defined(ANGLE_PLATFORM_MACOS)
void InitMetalFileAPIHooking(int argc, char **argv)
{
    if (argc < 1)
    {
        return;
    }

    for (int i = 0; i < argc; ++i)
    {
        if (strncmp(argv[i], kSkipFileHookingArg, strlen(kSkipFileHookingArg)) == 0)
        {
            return;
        }
    }

    constexpr char kInjectLibVarName[]    = "DYLD_INSERT_LIBRARIES";
    constexpr size_t kInjectLibVarNameLen = sizeof(kInjectLibVarName) - 1;

    std::string exeDir = GetExecutableDirectory();
    if (!exeDir.empty() && exeDir.back() != '/')
    {
        exeDir += "/";
    }

    // Intercept Metal shader cache access and return as if the cache doesn't exist.
    // This is to avoid slow shader cache mechanism that caused the test timeout in the past.
    // In order to do that, we need to hook the file API functions by making sure
    // libmetal_shader_cache_file_hooking.dylib library is loaded first before any other libraries.
    std::string injectLibsVar =
        std::string(kInjectLibVarName) + "=" + exeDir + "libmetal_shader_cache_file_hooking.dylib";

    char skipHookOption[sizeof(kSkipFileHookingArg)];
    memcpy(skipHookOption, kSkipFileHookingArg, sizeof(kSkipFileHookingArg));

    // Construct environment variables
    std::vector<char *> newEnv;
    char **environ = *_NSGetEnviron();
    for (int i = 0; environ[i]; ++i)
    {
        if (strncmp(environ[i], kInjectLibVarName, kInjectLibVarNameLen) == 0)
        {
            injectLibsVar += ':';
            injectLibsVar += environ[i] + kInjectLibVarNameLen + 1;
        }
        else
        {
            newEnv.push_back(environ[i]);
        }
    }
    newEnv.push_back(strdup(injectLibsVar.data()));
    newEnv.push_back(nullptr);

    // Construct arguments with kSkipFileHookingArg flag to skip the hooking after re-launching.
    std::vector<char *> newArgs;
    newArgs.push_back(argv[0]);
    newArgs.push_back(skipHookOption);
    for (int i = 1; i < argc; ++i)
    {
        newArgs.push_back(argv[i]);
    }
    newArgs.push_back(nullptr);

    // Re-launch the app with file API hooked.
    ASSERT(-1 != execve(argv[0], newArgs.data(), newEnv.data()));
}
#endif

}  // namespace angle
