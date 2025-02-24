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

#include "CPUID.hpp"

#if defined(_WIN32)
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <windows.h>
#	include <intrin.h>
#	include <float.h>
#else
#	include <unistd.h>
#	include <sched.h>
#	include <sys/types.h>
#endif

namespace rr {

static void cpuid(int eax_ebx_ecx_edx[4], int eax, int ecx = 0)
{
#if defined(__i386__) || defined(__x86_64__)
#	if defined(_WIN32)
	__cpuidex(eax_ebx_ecx_edx, eax, ecx);
#	else
	__asm volatile("cpuid"
	               : "=a"(eax_ebx_ecx_edx[0]), "=b"(eax_ebx_ecx_edx[1]), "=c"(eax_ebx_ecx_edx[2]), "=d"(eax_ebx_ecx_edx[3])
	               : "a"(eax), "c"(ecx));
#	endif
#else
	eax_ebx_ecx_edx[0] = 0;
	eax_ebx_ecx_edx[1] = 0;
	eax_ebx_ecx_edx[2] = 0;
	eax_ebx_ecx_edx[3] = 0;
#endif
}

bool CPUID::supportsSSE4_1()
{
	int eax_ebx_ecx_edx[4];
	cpuid(eax_ebx_ecx_edx, 1);
	return (eax_ebx_ecx_edx[2] & 0x00080000) != 0;
}

bool CPUID::supportsAVX2()
{
	int eax_ebx_ecx_edx[4];
	cpuid(eax_ebx_ecx_edx, 1);
	// Test bits 12 (FMA), 27 (OSXSAVE), and 28 (AVX) of ECX
	bool osxsave_avx_fma = (eax_ebx_ecx_edx[2] & 0x18001000) == 0x18001000;

	// AVX is a prerequisite of AVX2 and must be checked for first according to the Intel Software Developer's Manual.
	// OSXSAVE ensures the operating system can save/restore ymm registers on context switches.
	// FMA support is often considered an integral part of AVX2.
	if(!osxsave_avx_fma)
	{
		return false;
	}

	cpuid(eax_ebx_ecx_edx, 7, 0);  // Valid if AVX is supported
	return (eax_ebx_ecx_edx[1] & 0x00000020) != 0;
}

}  // namespace rr
