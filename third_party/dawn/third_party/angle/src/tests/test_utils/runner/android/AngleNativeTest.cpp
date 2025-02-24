// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// angle_native_test:
//   Contains native implementation for com.android.angle.test.AngleNativeTest.

#include <jni.h>
#include <vector>

#include <android/log.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#include "common/angleutils.h"
#include "common/string_utils.h"

// The main function of the program to be wrapped as a test apk.
extern int main(int argc, char **argv);

namespace
{

const char kLogTag[]        = "chromium";
const char kCrashedMarker[] = "[ CRASHED      ]\n";

// The list of signals which are considered to be crashes.
const int kExceptionSignals[] = {SIGSEGV, SIGABRT, SIGFPE, SIGILL, SIGBUS, -1};

struct sigaction g_old_sa[NSIG];

class [[nodiscard]] ScopedMainEntryLogger
{
  public:
    ScopedMainEntryLogger() { printf(">>ScopedMainEntryLogger\n"); }

    ~ScopedMainEntryLogger()
    {
        printf("<<ScopedMainEntryLogger\n");
        fflush(stdout);
        fflush(stderr);
    }
};

// This function runs in a compromised context. It should not allocate memory.
void SignalHandler(int sig, siginfo_t *info, void *reserved)
{
    // Output the crash marker.
    write(STDOUT_FILENO, kCrashedMarker, sizeof(kCrashedMarker) - 1);
    g_old_sa[sig].sa_sigaction(sig, info, reserved);
}

std::string ASCIIJavaStringToUTF8(JNIEnv *env, jstring str)
{
    if (!str)
    {
        return "";
    }

    const jsize length = env->GetStringLength(str);
    if (!length)
    {
        return "";
    }

    // JNI's GetStringUTFChars() returns strings in Java "modified" UTF8, so
    // instead get the String in UTF16. As the input is ASCII, drop the higher
    // bytes.
    const jchar *jchars   = env->GetStringChars(str, NULL);
    const char16_t *chars = reinterpret_cast<const char16_t *>(jchars);
    std::string out(chars, chars + length);
    env->ReleaseStringChars(str, jchars);
    return out;
}

size_t ArgsToArgv(const std::vector<std::string> &args, std::vector<char *> *argv)
{
    // We need to pass in a non-const char**.
    size_t argc = args.size();

    argv->resize(argc + 1);
    for (size_t i = 0; i < argc; ++i)
    {
        (*argv)[i] = const_cast<char *>(args[i].c_str());
    }
    (*argv)[argc] = NULL;  // argv must be NULL terminated.

    return argc;
}

void InstallExceptionHandlers()
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_sigaction = SignalHandler;
    sa.sa_flags     = SA_SIGINFO;

    for (unsigned int i = 0; kExceptionSignals[i] != -1; ++i)
    {
        sigaction(kExceptionSignals[i], &sa, &g_old_sa[kExceptionSignals[i]]);
    }
}

void AndroidLog(int priority, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    __android_log_vprint(priority, kLogTag, format, args);
    va_end(args);
}

}  // anonymous namespace

extern "C" JNIEXPORT void JNICALL
Java_com_android_angle_test_AngleNativeTest_nativeRunTests(JNIEnv *env,
                                                           jclass clazz,
                                                           jstring jcommandLineFlags,
                                                           jstring jcommandLineFilePath,
                                                           jstring jstdoutFilePath)
{
    InstallExceptionHandlers();

    const std::string commandLineFlags(ASCIIJavaStringToUTF8(env, jcommandLineFlags));
    const std::string commandLineFilePath(ASCIIJavaStringToUTF8(env, jcommandLineFilePath));
    const std::string stdoutFilePath(ASCIIJavaStringToUTF8(env, jstdoutFilePath));

    std::vector<std::string> args;
    if (commandLineFilePath.empty())
    {
        args.push_back("_");
    }
    else
    {
        std::string commandLineString;
        if (angle::ReadFileToString(commandLineFilePath, &commandLineString))
        {
            angle::SplitStringAlongWhitespace(commandLineString, &args);
        }
    }
    angle::SplitStringAlongWhitespace(commandLineFlags, &args);

    // A few options, such "--gtest_list_tests", will just use printf directly
    // Always redirect stdout to a known file.
    FILE *stdoutFile = fopen(stdoutFilePath.c_str(), "a+");
    if (stdoutFile == NULL)
    {
        AndroidLog(ANDROID_LOG_ERROR, "Failed to open stdout file: %s: %s\n",
                   stdoutFilePath.c_str(), strerror(errno));
        exit(EXIT_FAILURE);
    }

    int oldStdout = dup(STDOUT_FILENO);
    if (oldStdout == -1)
    {
        AndroidLog(ANDROID_LOG_ERROR, "Failed to dup stdout: %d\n", errno);
        fclose(stdoutFile);
        exit(EXIT_FAILURE);
    }

    int retVal = dup2(fileno(stdoutFile), STDOUT_FILENO);
    if (retVal == -1)
    {
        AndroidLog(ANDROID_LOG_ERROR, "Failed to dup2 stdout to file: %d\n", errno);
        fclose(stdoutFile);
        close(oldStdout);
        exit(EXIT_FAILURE);
    }

    dup2(STDOUT_FILENO, STDERR_FILENO);

    // When using a temp path on `/data`, performance is good enough we can line buffer
    // stdout/stderr.  This makes e.g. FATAL() << "message"; show up in the logs in CI
    // or local runs. Do *not* enable this on /sdcard/ (see https://crrev.com/c/3615081)
    if (stdoutFilePath.rfind("/data/", 0) == 0)
    {
        setlinebuf(stdout);
        setlinebuf(stderr);
    }

    std::vector<char *> argv;
    size_t argc = ArgsToArgv(args, &argv);

    {
        ScopedMainEntryLogger scoped_main_entry_logger;
        main(static_cast<int>(argc), &argv[0]);
    }

    fclose(stdoutFile);
    dup2(oldStdout, STDOUT_FILENO);
    close(oldStdout);
}
