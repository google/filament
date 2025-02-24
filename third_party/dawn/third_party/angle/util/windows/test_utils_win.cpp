//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// test_utils_win.cpp: Implementation of OS-specific functions for Windows

#include "util/test_utils.h"

#include <aclapi.h>
#include <stdarg.h>
#include <windows.h>
#include <array>
#include <iostream>
#include <vector>

#include "anglebase/no_destructor.h"
#include "common/angleutils.h"
#include "common/platform_helpers.h"

namespace angle
{
namespace
{
struct ScopedPipe
{
    ~ScopedPipe()
    {
        closeReadHandle();
        closeWriteHandle();
    }
    bool closeReadHandle()
    {
        if (readHandle)
        {
            if (::CloseHandle(readHandle) == FALSE)
            {
                std::cerr << "Error closing write handle: " << GetLastError();
                return false;
            }
            readHandle = nullptr;
        }

        return true;
    }
    bool closeWriteHandle()
    {
        if (writeHandle)
        {
            if (::CloseHandle(writeHandle) == FALSE)
            {
                std::cerr << "Error closing write handle: " << GetLastError();
                return false;
            }
            writeHandle = nullptr;
        }

        return true;
    }

    bool valid() const { return readHandle != nullptr || writeHandle != nullptr; }

    bool initPipe(SECURITY_ATTRIBUTES *securityAttribs)
    {
        if (::CreatePipe(&readHandle, &writeHandle, securityAttribs, 0) == FALSE)
        {
            std::cerr << "Error creating pipe: " << GetLastError() << "\n";
            return false;
        }

#if !defined(ANGLE_ENABLE_WINDOWS_UWP)
        // Ensure the read handles to the pipes are not inherited.
        if (::SetHandleInformation(readHandle, HANDLE_FLAG_INHERIT, 0) == FALSE)
        {
            std::cerr << "Error setting handle info on pipe: " << GetLastError() << "\n";
            return false;
        }
#endif  // !defined(ANGLE_ENABLE_WINDOWS_UWP)

        return true;
    }

    HANDLE readHandle  = nullptr;
    HANDLE writeHandle = nullptr;
};

// Returns false on EOF or error.
void ReadFromFile(bool blocking, HANDLE handle, std::string *out)
{
    char buffer[8192];
    DWORD bytesRead = 0;

    while (true)
    {
        if (!blocking)
        {
            BOOL success = ::PeekNamedPipe(handle, nullptr, 0, nullptr, &bytesRead, nullptr);
            if (success == FALSE || bytesRead == 0)
                return;
        }

        BOOL success = ::ReadFile(handle, buffer, sizeof(buffer), &bytesRead, nullptr);
        if (success == FALSE || bytesRead == 0)
            return;

        out->append(buffer, bytesRead);
    }

    // unreachable.
}

// Returns the Win32 last error code or ERROR_SUCCESS if the last error code is
// ERROR_FILE_NOT_FOUND or ERROR_PATH_NOT_FOUND. This is useful in cases where
// the absence of a file or path is a success condition (e.g., when attempting
// to delete an item in the filesystem).
bool ReturnSuccessOnNotFound()
{
    const DWORD error_code = ::GetLastError();
    return (error_code == ERROR_FILE_NOT_FOUND || error_code == ERROR_PATH_NOT_FOUND);
}

// Job objects seems to have problems on the Chromium CI and Windows 7.
bool ShouldUseJobObjects()
{
#if defined(ANGLE_ENABLE_WINDOWS_UWP)
    return false;
#else
    return IsWindows10OrLater();
#endif
}

class WindowsProcess : public Process
{
  public:
    WindowsProcess(const std::vector<const char *> &commandLineArgs,
                   ProcessOutputCapture captureOutput)
    {
        mProcessInfo.hProcess = INVALID_HANDLE_VALUE;
        mProcessInfo.hThread  = INVALID_HANDLE_VALUE;

        std::vector<char> commandLineString;
        for (const char *arg : commandLineArgs)
        {
            if (arg)
            {
                if (!commandLineString.empty())
                {
                    commandLineString.push_back(' ');
                }
                commandLineString.insert(commandLineString.end(), arg, arg + strlen(arg));
            }
        }
        commandLineString.push_back('\0');

        // Set the bInheritHandle flag so pipe handles are inherited.
        SECURITY_ATTRIBUTES securityAttribs;
        securityAttribs.nLength              = sizeof(SECURITY_ATTRIBUTES);
        securityAttribs.bInheritHandle       = TRUE;
        securityAttribs.lpSecurityDescriptor = nullptr;

        STARTUPINFOA startInfo = {};

        const bool captureStdout = captureOutput != ProcessOutputCapture::Nothing;
        const bool captureStderr =
            captureOutput == ProcessOutputCapture::StdoutAndStderrInterleaved ||
            captureOutput == ProcessOutputCapture::StdoutAndStderrSeparately;
        const bool pipeStderrToStdout =
            captureOutput == ProcessOutputCapture::StdoutAndStderrInterleaved;

        // Create pipes for stdout and stderr.
        startInfo.cb        = sizeof(STARTUPINFOA);
        startInfo.hStdInput = ::GetStdHandle(STD_INPUT_HANDLE);
        if (captureStdout)
        {
            if (!mStdoutPipe.initPipe(&securityAttribs))
            {
                return;
            }
            startInfo.hStdOutput = mStdoutPipe.writeHandle;
        }
        else
        {
            startInfo.hStdOutput = ::GetStdHandle(STD_OUTPUT_HANDLE);
        }

        if (pipeStderrToStdout)
        {
            startInfo.hStdError = startInfo.hStdOutput;
        }
        else if (captureStderr)
        {
            if (!mStderrPipe.initPipe(&securityAttribs))
            {
                return;
            }
            startInfo.hStdError = mStderrPipe.writeHandle;
        }
        else
        {
            startInfo.hStdError = ::GetStdHandle(STD_ERROR_HANDLE);
        }

#if !defined(ANGLE_ENABLE_WINDOWS_UWP)
        if (captureStdout || captureStderr)
        {
            startInfo.dwFlags |= STARTF_USESTDHANDLES;
        }

        if (ShouldUseJobObjects())
        {
            // Create job object. Job objects allow us to automatically force child processes to
            // exit if the parent process is unexpectedly killed. This should prevent ghost
            // processes from hanging around.
            mJobHandle = ::CreateJobObjectA(nullptr, nullptr);
            if (mJobHandle == NULL)
            {
                std::cerr << "Error creating job object: " << GetLastError() << "\n";
                return;
            }

            JOBOBJECT_EXTENDED_LIMIT_INFORMATION limitInfo = {};
            limitInfo.BasicLimitInformation.LimitFlags     = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            if (::SetInformationJobObject(mJobHandle, JobObjectExtendedLimitInformation, &limitInfo,
                                          sizeof(limitInfo)) == FALSE)
            {
                std::cerr << "Error setting job information: " << GetLastError() << "\n";
                return;
            }
        }
#endif  // !defined(ANGLE_ENABLE_WINDOWS_UWP)

        // Create the child process.
        if (::CreateProcessA(nullptr, commandLineString.data(), nullptr, nullptr,
                             TRUE,  // Handles are inherited.
                             0, nullptr, nullptr, &startInfo, &mProcessInfo) == FALSE)
        {
            std::cerr << "CreateProcessA Error code: " << GetLastError() << "\n";
            return;
        }

#if !defined(ANGLE_ENABLE_WINDOWS_UWP)
        if (mJobHandle != nullptr)
        {
            if (::AssignProcessToJobObject(mJobHandle, mProcessInfo.hProcess) == FALSE)
            {
                std::cerr << "AssignProcessToJobObject failed: " << GetLastError() << "\n";
                return;
            }
        }
#endif  // !defined(ANGLE_ENABLE_WINDOWS_UWP)

        // Close the write end of the pipes, so EOF can be generated when child exits.
        if (!mStdoutPipe.closeWriteHandle() || !mStderrPipe.closeWriteHandle())
            return;

        mStarted = true;
        mTimer.start();
    }

    ~WindowsProcess() override
    {
        if (mProcessInfo.hProcess != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(mProcessInfo.hProcess);
        }
        if (mProcessInfo.hThread != INVALID_HANDLE_VALUE)
        {
            ::CloseHandle(mProcessInfo.hThread);
        }
        if (mJobHandle != nullptr)
        {
            ::CloseHandle(mJobHandle);
        }
    }

    bool started() override { return mStarted; }

    bool finish() override
    {
        if (mStdoutPipe.valid())
        {
            ReadFromFile(true, mStdoutPipe.readHandle, &mStdout);
        }

        if (mStderrPipe.valid())
        {
            ReadFromFile(true, mStderrPipe.readHandle, &mStderr);
        }

        DWORD result = ::WaitForSingleObject(mProcessInfo.hProcess, INFINITE);
        mTimer.stop();
        return result == WAIT_OBJECT_0;
    }

    bool finished() override
    {
        if (!mStarted)
            return false;

        // Pipe stdin and stdout.
        if (mStdoutPipe.valid())
        {
            ReadFromFile(false, mStdoutPipe.readHandle, &mStdout);
        }

        if (mStderrPipe.valid())
        {
            ReadFromFile(false, mStderrPipe.readHandle, &mStderr);
        }

        DWORD result = ::WaitForSingleObject(mProcessInfo.hProcess, 0);
        if (result == WAIT_OBJECT_0)
        {
            mTimer.stop();
            return true;
        }
        if (result == WAIT_TIMEOUT)
            return false;

        mTimer.stop();
        std::cerr << "Unexpected result from WaitForSingleObject: " << result
                  << ". Last error: " << ::GetLastError() << "\n";
        return false;
    }

    int getExitCode() override
    {
        if (!mStarted)
            return -1;

        if (mProcessInfo.hProcess == INVALID_HANDLE_VALUE)
            return -1;

        DWORD exitCode = 0;
        if (::GetExitCodeProcess(mProcessInfo.hProcess, &exitCode) == FALSE)
            return -1;

        return static_cast<int>(exitCode);
    }

    bool kill() override
    {
        if (!mStarted)
            return true;

        HANDLE newHandle;
        if (::DuplicateHandle(::GetCurrentProcess(), mProcessInfo.hProcess, ::GetCurrentProcess(),
                              &newHandle, PROCESS_ALL_ACCESS, false,
                              DUPLICATE_CLOSE_SOURCE) == FALSE)
        {
            std::cerr << "Error getting permission to terminate process: " << ::GetLastError()
                      << "\n";
            return false;
        }
        mProcessInfo.hProcess = newHandle;

#if !defined(ANGLE_ENABLE_WINDOWS_UWP)
        if (::TerminateThread(mProcessInfo.hThread, 1) == FALSE)
        {
            std::cerr << "TerminateThread failed: " << GetLastError() << "\n";
            return false;
        }
#endif  // !defined(ANGLE_ENABLE_WINDOWS_UWP)

        if (::TerminateProcess(mProcessInfo.hProcess, 1) == FALSE)
        {
            std::cerr << "TerminateProcess failed: " << GetLastError() << "\n";
            return false;
        }

        mStarted = false;
        mTimer.stop();
        return true;
    }

  private:
    bool mStarted = false;
    ScopedPipe mStdoutPipe;
    ScopedPipe mStderrPipe;
    PROCESS_INFORMATION mProcessInfo = {};
    HANDLE mJobHandle                = nullptr;
};
}  // namespace

void Sleep(unsigned int milliseconds)
{
    ::Sleep(static_cast<DWORD>(milliseconds));
}

void WriteDebugMessage(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int size = vsnprintf(nullptr, 0, format, args);
    va_end(args);

    std::vector<char> buffer(size + 2);
    va_start(args, format);
    vsnprintf(buffer.data(), size + 1, format, args);
    va_end(args);

    OutputDebugStringA(buffer.data());
}

Process *LaunchProcess(const std::vector<const char *> &args, ProcessOutputCapture captureOutput)
{
    return new WindowsProcess(args, captureOutput);
}

bool DeleteSystemFile(const char *path)
{
    if (strlen(path) >= MAX_PATH)
        return false;

    const DWORD attr = ::GetFileAttributesA(path);
    // Report success if the file or path does not exist.
    if (attr == INVALID_FILE_ATTRIBUTES)
    {
        return ReturnSuccessOnNotFound();
    }

    // Clear the read-only bit if it is set.
    if ((attr & FILE_ATTRIBUTE_READONLY) &&
        !::SetFileAttributesA(path, attr & ~FILE_ATTRIBUTE_READONLY))
    {
        // It's possible for |path| to be gone now under a race with other deleters.
        return ReturnSuccessOnNotFound();
    }

    // We don't handle directories right now.
    if ((attr & FILE_ATTRIBUTE_DIRECTORY) != 0)
    {
        return false;
    }

    return !!::DeleteFileA(path) ? true : ReturnSuccessOnNotFound();
}

const char *GetNativeEGLLibraryNameWithExtension()
{
    return "libEGL.dll";
}
}  // namespace angle
