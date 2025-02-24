// Copyright 2018 The SwiftShader Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "Debug.hpp"

#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <string>

#if __ANDROID__
#	include <android/log.h>
#endif

#if defined(__unix__)
#	define PTRACE
#	include <sys/ptrace.h>
#	include <sys/types.h>
#elif defined(_WIN32) || defined(_WIN64)
#	include <windows.h>
#elif defined(__APPLE__) || defined(__MACH__)
#	include <sys/sysctl.h>
#	include <unistd.h>
#endif

#ifdef ERROR
#	undef ERROR  // b/127920555
#endif

#ifndef REACTOR_LOGGING_LEVEL
#	define REACTOR_LOGGING_LEVEL Info
#endif

namespace {

bool IsUnderDebugger()
{
#if defined(PTRACE) && !defined(__APPLE__) && !defined(__MACH__)
	static bool checked = false;
	static bool res = false;

	if(!checked)
	{
		// If a debugger is attached then we're already being ptraced and ptrace
		// will return a non-zero value.
		checked = true;
		if(ptrace(PTRACE_TRACEME, 0, 1, 0) != 0)
		{
			res = true;
		}
		else
		{
			ptrace(PTRACE_DETACH, 0, 1, 0);
		}
	}

	return res;
#elif defined(_WIN32) || defined(_WIN64)
	return IsDebuggerPresent() != 0;
#elif defined(__APPLE__) || defined(__MACH__)
	// Code comes from the Apple Technical Q&A QA1361

	// Tell sysctl what info we're requestion. Specifically we're asking for
	// info about this our PID.
	int res = 0;
	int request[4] = {
		CTL_KERN,
		KERN_PROC,
		KERN_PROC_PID,
		getpid()
	};
	struct kinfo_proc info;
	size_t size = sizeof(info);

	info.kp_proc.p_flag = 0;

	// Get the info we're requesting, if sysctl fails then info.kp_proc.p_flag will remain 0.
	res = sysctl(request, sizeof(request) / sizeof(*request), &info, &size, NULL, 0);
	ASSERT_MSG(res == 0, "syscl returned %d", res);

	// We're being debugged if the P_TRACED flag is set
	return ((info.kp_proc.p_flag & P_TRACED) != 0);
#else
	return false;
#endif
}

enum class Level
{
	Debug,
	Info,
	Warn,
	Error,
	Fatal,
};

#ifdef __ANDROID__
[[maybe_unused]] void logv_android(Level level, const char *msg)
{
	switch(level)
	{
	case Level::Debug:
		__android_log_write(ANDROID_LOG_DEBUG, "SwiftShader", msg);
		break;
	case Level::Info:
		__android_log_write(ANDROID_LOG_INFO, "SwiftShader", msg);
		break;
	case Level::Warn:
		__android_log_write(ANDROID_LOG_WARN, "SwiftShader", msg);
		break;
	case Level::Error:
		__android_log_write(ANDROID_LOG_ERROR, "SwiftShader", msg);
		break;
	case Level::Fatal:
		__android_log_write(ANDROID_LOG_FATAL, "SwiftShader", msg);
		break;
	}
}
#else
[[maybe_unused]] void logv_std(Level level, const char *msg)
{
	switch(level)
	{
	case Level::Debug:
	case Level::Info:
		fprintf(stdout, "%s", msg);
		break;
	case Level::Warn:
	case Level::Error:
	case Level::Fatal:
		fprintf(stderr, "%s", msg);
		break;
	}
}
#endif

void logv(Level level, const char *format, va_list args)
{
	if(static_cast<int>(level) < static_cast<int>(Level::REACTOR_LOGGING_LEVEL))
	{
		return;
	}

#ifndef SWIFTSHADER_DISABLE_TRACE
	char buffer[2048];
	vsnprintf(buffer, sizeof(buffer), format, args);

#	if defined(__ANDROID__)
	logv_android(level, buffer);
#	elif defined(_WIN32)
	logv_std(level, buffer);
	::OutputDebugString(buffer);
#	else
	logv_std(level, buffer);
#	endif

	const bool traceToFile = false;
	if(traceToFile)
	{
		FILE *file = fopen(TRACE_OUTPUT_FILE, "a");

		if(file)
		{
			vfprintf(file, format, args);
			fclose(file);
		}
	}
#endif  // SWIFTSHADER_DISABLE_TRACE
}

}  // anonymous namespace

namespace rr {

void trace(const char *format, ...)
{
	va_list vararg;
	va_start(vararg, format);
	logv(Level::Debug, format, vararg);
	va_end(vararg);
}

void warn(const char *format, ...)
{
	va_list vararg;
	va_start(vararg, format);
	logv(Level::Warn, format, vararg);
	va_end(vararg);
}

void abort(const char *format, ...)
{
	va_list vararg;

	va_start(vararg, format);
	logv(Level::Fatal, format, vararg);
	va_end(vararg);

	::abort();
}

void trace_assert(const char *format, ...)
{
	static std::atomic<bool> asserted = { false };
	if(IsUnderDebugger() && !asserted.exchange(true))
	{
		// Abort after tracing and printing to stderr
		va_list vararg;
		va_start(vararg, format);
		logv(Level::Fatal, format, vararg);
		va_end(vararg);

		::abort();
	}
	else if(!asserted)
	{
		va_list vararg;
		va_start(vararg, format);
		logv(Level::Fatal, format, vararg);
		va_end(vararg);
	}
}

}  // namespace rr
