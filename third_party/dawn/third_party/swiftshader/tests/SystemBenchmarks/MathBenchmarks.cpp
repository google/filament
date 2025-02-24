// Copyright 2022 The SwiftShader Authors. All Rights Reserved.
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

#include "System/CPUID.hpp"
#include "System/Math.hpp"

#include "benchmark/benchmark.h"

#include <limits>

using namespace sw;

// Global variable the C++ compiler can't eliminate.
volatile float y;

static void Subnormals(benchmark::State &state, bool FTZ, bool DAZ)
{
	CPUID::setFlushToZero(FTZ);
	CPUID::setDenormalsAreZero(DAZ);

	for(auto _ : state)
	{
		// 2^-64 is a normalized value which when squared produces 2^-128, a denormalized value.
		volatile float x = exp2(-64);

		for(int i = 0; i < state.range(); i++)
		{
			y = x * x;
		}
	}

	CPUID::setFlushToZero(false);
	CPUID::setDenormalsAreZero(false);
}

BENCHMARK_CAPTURE(Subnormals, Default, false, false)->Arg(100000)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(Subnormals, FlushToZero, true, false)->Arg(100000)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(Subnormals, DenormalsAreZero, false, true)->Arg(100000)->Unit(benchmark::kMillisecond);
BENCHMARK_CAPTURE(Subnormals, FTZ_and_DAZ, true, true)->Arg(100000)->Unit(benchmark::kMillisecond);