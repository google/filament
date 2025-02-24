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

#if defined(__i386__) || defined(__x86_64__)
#	include <xmmintrin.h>
#	include <pmmintrin.h>
#endif

namespace sw {

static void cpuid(int registers[4], int info)
{
#if defined(__i386__) || defined(__x86_64__)
#	if defined(_WIN32)
	__cpuid(registers, info);
#	else
	__asm volatile("cpuid"
	               : "=a"(registers[0]), "=b"(registers[1]), "=c"(registers[2]), "=d"(registers[3])
	               : "a"(info));
#	endif
#else
	registers[0] = 0;
	registers[1] = 0;
	registers[2] = 0;
	registers[3] = 0;
#endif
}

bool CPUID::supportsMMX()
{
	int registers[4];
	cpuid(registers, 1);
	return (registers[3] & 0x00800000) != 0;
}

bool CPUID::supportsCMOV()
{
	int registers[4];
	cpuid(registers, 1);
	return (registers[3] & 0x00008000) != 0;
}

bool CPUID::supportsSSE()
{
	int registers[4];
	cpuid(registers, 1);
	return (registers[3] & 0x02000000) != 0;
}

bool CPUID::supportsSSE2()
{
	int registers[4];
	cpuid(registers, 1);
	return (registers[3] & 0x04000000) != 0;
}

bool CPUID::supportsSSE3()
{
	int registers[4];
	cpuid(registers, 1);
	return (registers[2] & 0x00000001) != 0;
}

bool CPUID::supportsSSSE3()
{
	int registers[4];
	cpuid(registers, 1);
	return (registers[2] & 0x00000200) != 0;
}

bool CPUID::supportsSSE4_1()
{
	int registers[4];
	cpuid(registers, 1);
	return (registers[2] & 0x00080000) != 0;
}

int CPUID::coreCount()
{
	int cores = 0;

#if defined(_WIN32)
	DWORD_PTR processAffinityMask = 1;
	DWORD_PTR systemAffinityMask = 1;

	GetProcessAffinityMask(GetCurrentProcess(), &processAffinityMask, &systemAffinityMask);

	while(systemAffinityMask)
	{
		if(systemAffinityMask & 1)
		{
			cores++;
		}

		systemAffinityMask >>= 1;
	}
#else
	cores = sysconf(_SC_NPROCESSORS_ONLN);
#endif

	if(cores < 1) cores = 1;
	if(cores > 16) cores = 16;

	return cores;  // FIXME: Number of physical cores
}

int CPUID::processAffinity()
{
	int cores = 0;

#if defined(_WIN32)
	DWORD_PTR processAffinityMask = 1;
	DWORD_PTR systemAffinityMask = 1;

	GetProcessAffinityMask(GetCurrentProcess(), &processAffinityMask, &systemAffinityMask);

	while(processAffinityMask)
	{
		if(processAffinityMask & 1)
		{
			cores++;
		}

		processAffinityMask >>= 1;
	}
#else
	return coreCount();  // FIXME: Assumes no affinity limitation
#endif

	if(cores < 1) cores = 1;
	if(cores > 16) cores = 16;

	return cores;
}

void CPUID::setFlushToZero(bool enableFTZ)
{
#if defined(_MSC_VER)
	unsigned int current = _controlfp(0, 0) & _MCW_DN;
	if(current == _DN_SAVE || current == _DN_SAVE_OPERANDS_FLUSH_RESULTS)  // DAZ off
	{
		_controlfp(enableFTZ ? _DN_SAVE_OPERANDS_FLUSH_RESULTS : _DN_SAVE, _MCW_DN);
	}
	else  // DAZ on
	{
		_controlfp(enableFTZ ? _DN_FLUSH : _DN_FLUSH_OPERANDS_SAVE_RESULTS, _MCW_DN);
	}
#elif defined(__i386__) || defined(__x86_64__)
	_MM_SET_FLUSH_ZERO_MODE(enableFTZ ? _MM_FLUSH_ZERO_ON : _MM_FLUSH_ZERO_OFF);
#else
	// Unimplemented
#endif
}

void CPUID::setDenormalsAreZero(bool enableDAZ)
{
#if defined(_MSC_VER)
	unsigned int current = _controlfp(0, 0) & _MCW_DN;
	if(current == _DN_SAVE || current == _DN_FLUSH_OPERANDS_SAVE_RESULTS)  // FTZ off
	{
		_controlfp(enableDAZ ? _DN_FLUSH_OPERANDS_SAVE_RESULTS : _DN_SAVE, _MCW_DN);
	}
	else  // FTZ on
	{
		_controlfp(enableDAZ ? _DN_FLUSH : _DN_SAVE_OPERANDS_FLUSH_RESULTS, _MCW_DN);
	}
#elif defined(__i386__) || defined(__x86_64__)
	_MM_SET_DENORMALS_ZERO_MODE(enableDAZ ? _MM_DENORMALS_ZERO_ON : _MM_DENORMALS_ZERO_OFF);
#else
	// Unimplemented
#endif
}

}  // namespace sw
