// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// GEN_BUILD:CONDITION(tint_build_is_win)

#include "src/tint/utils/command/command.h"

#define WIN32_LEAN_AND_MEAN 1
#include <Windows.h>
#include <dbghelp.h>
#include <string>

#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/system/executable_path.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint {

namespace {

/// Handle is a simple wrapper around the Win32 HANDLE
class Handle {
  public:
    /// Constructor
    Handle() : handle_(nullptr) {}

    /// Constructor
    explicit Handle(HANDLE handle) : handle_(handle) {}

    /// Destructor
    ~Handle() { Close(); }

    /// Move assignment operator
    Handle& operator=(Handle&& rhs) {
        Close();
        handle_ = rhs.handle_;
        rhs.handle_ = nullptr;
        return *this;
    }

    /// Closes the handle (if it wasn't already closed)
    void Close() {
        if (handle_) {
            CloseHandle(handle_);
        }
        handle_ = nullptr;
    }

    /// @returns the handle
    operator HANDLE() { return handle_; }

    /// @returns true if the handle is not invalid
    explicit operator bool() { return handle_ != nullptr; }

  private:
    Handle(const Handle&) = delete;
    Handle& operator=(const Handle&) = delete;

    HANDLE handle_ = nullptr;
};

/// Pipe is a simple wrapper around a Win32 CreatePipe() function
class Pipe {
  public:
    /// Constructs the pipe
    explicit Pipe(bool for_read) {
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.bInheritHandle = TRUE;
        sa.lpSecurityDescriptor = nullptr;

        HANDLE hread;
        HANDLE hwrite;
        if (CreatePipe(&hread, &hwrite, &sa, 0)) {
            read = Handle(hread);
            write = Handle(hwrite);
            // Ensure the read handle to the pipe is not inherited
            if (!SetHandleInformation(for_read ? read : write, HANDLE_FLAG_INHERIT, 0)) {
                read.Close();
                write.Close();
            }
        }
    }

    /// @returns true if the pipe has an open read or write file
    explicit operator bool() { return read || write; }

    /// The reader end of the pipe
    Handle read;

    /// The writer end of the pipe
    Handle write;
};

/// Queries whether the file at the given path is an executable or DLL.
bool ExecutableExists(const std::string& path) {
    auto file = Handle(CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr,
                                   OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, nullptr));
    if (!file) {
        return false;
    }

    auto map = Handle(CreateFileMappingA(file, nullptr, PAGE_READONLY, 0, 0, nullptr));
    if (map == INVALID_HANDLE_VALUE) {
        return false;
    }

    void* addr_header = MapViewOfFileEx(map, FILE_MAP_READ, 0, 0, 0, nullptr);
    TINT_DEFER(UnmapViewOfFile(addr_header));

    // Dynamically obtain the address of, and call ImageNtHeader. This is done to avoid tint.exe
    // needing to statically link Dbghelp.lib.
    static auto* dbg_help = LoadLibraryA("Dbghelp.dll");  // Leaks, but who cares?
    if (dbg_help) {
        if (FARPROC proc = GetProcAddress(dbg_help, "ImageNtHeader")) {
            using ImageNtHeaderPtr = decltype(&ImageNtHeader);
            auto* image_nt_header = reinterpret_cast<ImageNtHeaderPtr>(proc)(addr_header);
            return image_nt_header != nullptr;
        }
    }

    // Couldn't call ImageNtHeader, assume it is executable
    return false;
}

std::string GetCWD() {
    char cwd[MAX_PATH] = "";
    GetCurrentDirectoryA(sizeof(cwd), cwd);
    return cwd;
}

std::string FindExecutable(const std::string& name) {
    auto in_cwd = GetCWD() + "/" + name;
    if (ExecutableExists(in_cwd)) {
        return in_cwd;
    }
    if (ExecutableExists(in_cwd + ".exe")) {
        return in_cwd + ".exe";
    }

    auto in_exe_path = tint::ExecutableDirectory() + "/" + name;
    if (ExecutableExists(in_exe_path)) {
        return in_exe_path;
    }
    if (ExecutableExists(in_exe_path + ".exe")) {
        return in_exe_path + ".exe";
    }

    if (ExecutableExists(name)) {
        return name;
    }
    if (ExecutableExists(name + ".exe")) {
        return name + ".exe";
    }

    if (name.find("/") == std::string::npos && name.find("\\") == std::string::npos) {
        char* path_env = nullptr;
        size_t path_env_len = 0;
        if (_dupenv_s(&path_env, &path_env_len, "PATH")) {
            return "";
        }
        std::istringstream path{path_env};
        free(path_env);
        std::string dir;
        while (getline(path, dir, ';')) {
            auto test = dir + "\\" + name;
            if (ExecutableExists(test)) {
                return test;
            }
            if (ExecutableExists(test + ".exe")) {
                return test + ".exe";
            }
        }
    }
    return "";
}

}  // namespace

Command::Command(const std::string& path) : path_(path) {}

Command Command::LookPath(const std::string& executable) {
    return Command(FindExecutable(executable));
}

bool Command::Found() const {
    return ExecutableExists(path_);
}

Command::Output Command::Exec(std::initializer_list<std::string> arguments) const {
    Pipe stdout_pipe(true);
    Pipe stderr_pipe(true);
    Pipe stdin_pipe(false);
    if (!stdin_pipe || !stdout_pipe || !stderr_pipe) {
        Output output;
        output.err = "Command::Exec(): Failed to create pipes";
        return output;
    }

    if (!input_.empty()) {
        if (!WriteFile(stdin_pipe.write, input_.data(), input_.size(), nullptr, nullptr)) {
            Output output;
            output.err = "Command::Exec() Failed to write stdin";
            return output;
        }
    }
    stdin_pipe.write.Close();

    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdOutput = stdout_pipe.write;
    si.hStdError = stderr_pipe.write;
    si.hStdInput = stdin_pipe.read;

    StringStream args;
    args << path_;
    for (auto& arg : arguments) {
        if (!arg.empty()) {
            args << " " << arg;
        }
    }

    PROCESS_INFORMATION pi{};
    if (!CreateProcessA(nullptr,                                // No module name (use command line)
                        const_cast<LPSTR>(args.str().c_str()),  // Command line
                        nullptr,                                // Process handle not inheritable
                        nullptr,                                // Thread handle not inheritable
                        TRUE,                                   // Handles are inherited
                        0,                                      // No creation flags
                        nullptr,                                // Use parent's environment block
                        nullptr,                                // Use parent's starting directory
                        &si,                                    // Pointer to STARTUPINFO structure
                        &pi)) {  // Pointer to PROCESS_INFORMATION structure
        Output out;
        out.err = "Command::Exec() CreateProcess('" + args.str() + "') failed";
        out.error_code = 1;
        return out;
    }

    stdin_pipe.read.Close();
    stdout_pipe.write.Close();
    stderr_pipe.write.Close();

    struct StreamReadThreadArgs {
        HANDLE stream;
        std::string output;
    };

    auto stream_read_thread = [](LPVOID user) -> DWORD {
        auto* thread_args = reinterpret_cast<StreamReadThreadArgs*>(user);
        DWORD n = 0;
        char buf[256];
        while (ReadFile(thread_args->stream, buf, sizeof(buf), &n, NULL)) {
            auto s = std::string(buf, buf + n);
            thread_args->output += std::string(buf, buf + n);
        }
        return 0;
    };

    StreamReadThreadArgs stdout_read_args{stdout_pipe.read, {}};
    auto* stdout_read_thread =
        ::CreateThread(nullptr, 0, stream_read_thread, &stdout_read_args, 0, nullptr);

    StreamReadThreadArgs stderr_read_args{stderr_pipe.read, {}};
    auto* stderr_read_thread =
        ::CreateThread(nullptr, 0, stream_read_thread, &stderr_read_args, 0, nullptr);

    HANDLE handles[] = {pi.hProcess, stdout_read_thread, stderr_read_thread};
    constexpr DWORD num_handles = sizeof(handles) / sizeof(handles[0]);

    Output output;

    auto res = WaitForMultipleObjects(num_handles, handles, /* wait_all = */ TRUE, INFINITE);
    if (res >= WAIT_OBJECT_0 && res < WAIT_OBJECT_0 + num_handles) {
        output.out = stdout_read_args.output;
        output.err = stderr_read_args.output;
        DWORD exit_code = 0;
        GetExitCodeProcess(pi.hProcess, &exit_code);
        output.error_code = static_cast<int>(exit_code);
    } else {
        output.err = "Command::Exec() WaitForMultipleObjects() returned " + std::to_string(res);
    }

    return output;
}

}  // namespace tint
