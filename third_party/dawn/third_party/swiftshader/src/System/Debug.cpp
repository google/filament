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

#include <atomic>
#include <cstdarg>
#include <cstdio>
#include <string>

#ifdef ERROR
#	undef ERROR  // b/127920555
#endif

#ifndef SWIFTSHADER_LOGGING_LEVEL
#	define SWIFTSHADER_LOGGING_LEVEL Info
#endif

namespace {

enum class Level
{
	Verbose,
	Debug,
	Info,
	Warn,
	Error,
	Fatal,
	Disabled,
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
	default:
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
	default:
		break;
	}
}
#endif

void logv(Level level, const char *format, va_list args)
{
	if(static_cast<int>(level) >= static_cast<int>(Level::SWIFTSHADER_LOGGING_LEVEL))
	{
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
	}

	const Level traceToFileLevel = Level::Disabled;
	if(static_cast<int>(level) >= static_cast<int>(traceToFileLevel))
	{
		FILE *file = fopen(TRACE_OUTPUT_FILE, "a");

		if(file)
		{
			vfprintf(file, format, args);
			fclose(file);
		}
#endif  // SWIFTSHADER_DISABLE_TRACE
	}
}

}  // anonymous namespace

namespace sw {

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

}  // namespace sw
