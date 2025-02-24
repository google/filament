//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// process.cpp:
//    Process manages a child process. This is largely copied from chrome.
//

#include "libANGLE/renderer/metal/process.h"

#include <crt_externs.h>
#include <fcntl.h>
#include <mach/mach.h>
#include <os/availability.h>
#include <spawn.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/sysctl.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

#include "common/base/anglebase/logging.h"
#include "common/debug.h"

namespace rx
{
namespace mtl
{
namespace
{

// This code is copied from chrome's process launching code:
// (base/process/launch_mac.cc).

typedef pid_t ProcessId;
constexpr ProcessId kNullProcessId = 0;

// DPSXCHECK is a Debug Posix Spawn Check macro. The posix_spawn* family of
// functions return an errno value, as opposed to setting errno directly. This
// macro emulates a DPCHECK().
#define DPSXCHECK(expr)  \
    do                   \
    {                    \
        int rv = (expr); \
        DCHECK(rv == 0); \
    } while (0)

//         DCHECK(rv == 0) << #expr << ": -" << rv << " " << strerror(rv);

class PosixSpawnAttr
{
  public:
    PosixSpawnAttr() { DPSXCHECK(posix_spawnattr_init(&attr_)); }

    ~PosixSpawnAttr() { DPSXCHECK(posix_spawnattr_destroy(&attr_)); }

    posix_spawnattr_t *get() { return &attr_; }

  private:
    posix_spawnattr_t attr_;
};

class PosixSpawnFileActions
{
  public:
    PosixSpawnFileActions() { DPSXCHECK(posix_spawn_file_actions_init(&file_actions_)); }

    PosixSpawnFileActions(const PosixSpawnFileActions &)            = delete;
    PosixSpawnFileActions &operator=(const PosixSpawnFileActions &) = delete;

    ~PosixSpawnFileActions() { DPSXCHECK(posix_spawn_file_actions_destroy(&file_actions_)); }

    void Open(int filedes, const char *path, int mode)
    {
        DPSXCHECK(posix_spawn_file_actions_addopen(&file_actions_, filedes, path, mode, 0));
    }

    void Dup2(int filedes, int newfiledes)
    {
        DPSXCHECK(posix_spawn_file_actions_adddup2(&file_actions_, filedes, newfiledes));
    }

    void Inherit(int filedes)
    {
        DPSXCHECK(posix_spawn_file_actions_addinherit_np(&file_actions_, filedes));
    }

#if TARGET_OS_OSX
    void Chdir(const char *path) API_AVAILABLE(macos(10.15))
    {
        DPSXCHECK(posix_spawn_file_actions_addchdir_np(&file_actions_, path));
    }
#endif

    const posix_spawn_file_actions_t *get() const { return &file_actions_; }

  private:
    posix_spawn_file_actions_t file_actions_;
};

// This is a slimmed down version of chrome's LaunchProcess().
ProcessId LaunchProcess(const std::vector<std::string> &argv)
{
    PosixSpawnAttr attr;

    DPSXCHECK(posix_spawnattr_setflags(attr.get(), POSIX_SPAWN_CLOEXEC_DEFAULT));

    PosixSpawnFileActions file_actions;

    // Process file descriptors for the child. By default, LaunchProcess will
    // open stdin to /dev/null and inherit stdout and stderr.
    bool inherit_stdout = true, inherit_stderr = true;
    bool null_stdin = true;

    if (null_stdin)
    {
        file_actions.Open(STDIN_FILENO, "/dev/null", O_RDONLY);
    }
    if (inherit_stdout)
    {
        file_actions.Inherit(STDOUT_FILENO);
    }
    if (inherit_stderr)
    {
        file_actions.Inherit(STDERR_FILENO);
    }

    std::vector<char *> argv_cstr;
    argv_cstr.reserve(argv.size() + 1);
    for (const auto &arg : argv)
    {
        argv_cstr.push_back(const_cast<char *>(arg.c_str()));
    }
    argv_cstr.push_back(nullptr);

    const bool clear_environment = false;
    char *empty_environ          = nullptr;
    char **new_environ           = clear_environment ? &empty_environ : *_NSGetEnviron();

    const char *executable_path = argv_cstr[0];

    pid_t pid;
    // Use posix_spawnp as some callers expect to have PATH consulted.
    int rv = posix_spawnp(&pid, executable_path, file_actions.get(), attr.get(), &argv_cstr[0],
                          new_environ);

    if (rv != 0)
    {
        FATAL() << "posix_spawnp failed";
        return kNullProcessId;
    }

    return pid;
}

ProcessId GetParentProcessId(ProcessId process)
{
    struct kinfo_proc info;
    size_t length = sizeof(struct kinfo_proc);
    int mib[4]    = {CTL_KERN, KERN_PROC, KERN_PROC_PID, process};
    if (sysctl(mib, 4, &info, &length, NULL, 0) < 0)
    {
        FATAL() << "sysctl failed";
        return -1;
    }
    if (length == 0)
        return -1;
    return info.kp_eproc.e_ppid;
}

#define HANDLE_EINTR(x)                                         \
    ({                                                          \
        decltype(x) eintr_wrapper_result;                       \
        do                                                      \
        {                                                       \
            eintr_wrapper_result = (x);                         \
        } while (eintr_wrapper_result == -1 && errno == EINTR); \
        eintr_wrapper_result;                                   \
    })

bool WaitForExitImpl(ProcessId pid, int &exit_code)
{
    const ProcessId our_pid = getpid();
    ASSERT(pid != our_pid);

    const bool exited = (GetParentProcessId(pid) < 0);
    int status;
    const bool wait_result = HANDLE_EINTR(waitpid(pid, &status, 0)) > 0;
    if (!wait_result)
    {
        return exited;
    }
    if (WIFSIGNALED(status))
    {
        exit_code = -1;
        return true;
    }
    if (WIFEXITED(status))
    {
        exit_code = WEXITSTATUS(status);
        return true;
    }
    return exited;
}

}  // namespace

Process::Process(const std::vector<std::string> &argv) : pid_(LaunchProcess(argv)) {}

Process::~Process()
{
    // TODO: figure out if should terminate/wait/whatever.
}

bool Process::WaitForExit(int &exit_code)
{
    ASSERT(pid_ != kNullProcessId);
    return WaitForExitImpl(pid_, exit_code);
}

bool Process::DidLaunch() const
{
    return pid_ != kNullProcessId;
}

}  // namespace mtl
}  // namespace rx
