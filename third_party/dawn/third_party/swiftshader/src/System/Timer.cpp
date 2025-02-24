// Copyright 2016 The SwiftShader Authors. All Rights Reserved.
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

#include "Timer.hpp"

#if !defined(__i386__) && defined(_M_IX86)
#	define __i386__ 1
#endif

#if !defined(__x86_64__) && (defined(_M_AMD64) || defined(_M_X64))
#	define __x86_64__ 1
#endif

#if defined(_WIN32)
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <windows.h>
#	include <intrin.h>
#else
#	include <sys/time.h>
#	if defined(__i386__) || defined(__x86_64__)
#		include <x86intrin.h>
#	endif
#endif

namespace sw {

Timer::Timer()
{
}

Timer::~Timer()
{
}

double Timer::seconds()
{
#if defined(_WIN32)
	return (double)counter() / (double)frequency();
#else
	timeval t;
	gettimeofday(&t, 0);
	return (double)t.tv_sec + (double)t.tv_usec * 1.0e-6;
#endif
}

int64_t Timer::ticks()
{
#if defined(_WIN32)
#	if defined(_M_ARM64)
	return _ReadStatusReg(ARM64_PMCCNTR_EL0);
#	else
	return __rdtsc();
#	endif
#elif defined(__i386__) || defined(__x86_64__)
	int64_t tsc;
	__asm volatile("rdtsc"
	               : "=A"(tsc));
	return tsc;
#else
	return 0;
#endif
}

int64_t Timer::counter()
{
#if defined(_WIN32)
	int64_t counter;
	QueryPerformanceCounter((LARGE_INTEGER *)&counter);
	return counter;
#else
	timeval t;
	gettimeofday(&t, 0);
	return t.tv_sec * 1000000 + t.tv_usec;
#endif
}

int64_t Timer::frequency()
{
#if defined(_WIN32)
	int64_t frequency;
	QueryPerformanceFrequency((LARGE_INTEGER *)&frequency);
	return frequency;
#else
	return 1000000;  // gettimeofday uses microsecond resolution
#endif
}

}  // namespace sw
