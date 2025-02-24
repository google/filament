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

#include "ShaderCore.hpp"
#include "Reactor/Reactor.hpp"

#include "benchmark/benchmark.h"

#include <vector>

BENCHMARK_MAIN();

// Macro that creates a lambda wrapper around the input overloaded function,
// creating a non-overload based on the args. This is useful for passing
// overloaded functions as template arguments.
// See https://stackoverflow.com/questions/25871381/c-overloaded-function-as-template-argument
#define LIFT(fname)                                          \
	[](auto &&...args) -> decltype(auto) {                   \
		return fname(std::forward<decltype(args)>(args)...); \
	}

namespace sw {

template<typename Func, class... Args>
static void Transcendental1(benchmark::State &state, Func func, Args &&...args)
{
	const int REPS = state.range(0);

	FunctionT<void(float *, float *)> function;
	{
		Pointer<SIMD::Float> r = Pointer<Float>(function.Arg<0>());
		Pointer<SIMD::Float> a = Pointer<Float>(function.Arg<1>());

		for(int i = 0; i < REPS; i++)
		{
			r[i] = func(a[i], args...);
		}
	}

	auto routine = function("one");

	std::vector<float> r(REPS * SIMD::Width);
	std::vector<float> a(REPS * SIMD::Width, 1.0f);

	for(auto _ : state)
	{
		routine(r.data(), a.data());
	}
}

template<typename Func, class... Args>
static void Transcendental2(benchmark::State &state, Func func, Args &&...args)
{
	const int REPS = state.range(0);

	FunctionT<void(float *, float *, float *)> function;
	{
		Pointer<SIMD::Float> r = Pointer<Float>(function.Arg<0>());
		Pointer<SIMD::Float> a = Pointer<Float>(function.Arg<1>());
		Pointer<SIMD::Float> b = Pointer<Float>(function.Arg<2>());

		for(int i = 0; i < REPS; i++)
		{
			r[i] = func(a[i], b[i], args...);
		}
	}

	auto routine = function("two");

	std::vector<float> r(REPS * SIMD::Width);
	std::vector<float> a(REPS * SIMD::Width, 0.456f);
	std::vector<float> b(REPS * SIMD::Width, 0.789f);

	for(auto _ : state)
	{
		routine(r.data(), a.data(), b.data());
	}
}

// No operation; just copy the input to the output, for use as a baseline.
static SIMD::Float Nop(RValue<SIMD::Float> x)
{
	return x;
}

static const int REPS = 10;

BENCHMARK_CAPTURE(Transcendental1, Nop, Nop)->Arg(REPS);

BENCHMARK_CAPTURE(Transcendental1, rr_Sin, LIFT(rr::Sin))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Sin_highp, LIFT(sw::Sin), false /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Sin_mediump, LIFT(sw::Sin), true /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, rr_Cos, LIFT(rr::Cos))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Cos_highp, LIFT(sw::Cos), false /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Cos_mediump, LIFT(sw::Cos), true /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, rr_Tan, LIFT(rr::Tan))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Tan_highp, LIFT(sw::Tan), false /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Tan_mediump, LIFT(sw::Tan), true /* relaxedPrecision */)->Arg(REPS);

BENCHMARK_CAPTURE(Transcendental1, rr_Asin, LIFT(rr::Asin))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Asin_highp, LIFT(sw::Asin), false /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Asin_mediump, LIFT(sw::Asin), true /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, rr_Acos, LIFT(rr::Acos))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Acos_highp, LIFT(sw::Acos), false /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Acos_mediump, LIFT(sw::Acos), true /* relaxedPrecision */)->Arg(REPS);

BENCHMARK_CAPTURE(Transcendental1, rr_Atan, LIFT(rr::Atan))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Atan_highp, LIFT(sw::Atan), false /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Atan_mediump, LIFT(sw::Atan), true /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, rr_Sinh, LIFT(rr::Sinh))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Sinh_highp, LIFT(sw::Sinh), false /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Sinh_mediump, LIFT(sw::Sinh), true /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, rr_Cosh, LIFT(rr::Cosh))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Cosh_highp, LIFT(sw::Cosh), false /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Cosh_mediump, LIFT(sw::Cosh), true /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, rr_Tanh, LIFT(rr::Tanh))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Tanh_highp, LIFT(sw::Tanh), false /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Tanh_mediump, LIFT(sw::Tanh), true /* relaxedPrecision */)->Arg(REPS);

BENCHMARK_CAPTURE(Transcendental1, rr_Asinh, LIFT(rr::Asinh))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Asinh_highp, LIFT(sw::Asinh), false /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Asinh_mediump, LIFT(sw::Asinh), true /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, rr_Acosh, LIFT(rr::Acosh))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Acosh_highp, LIFT(sw::Acosh), false /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Acosh_mediump, LIFT(sw::Acosh), true /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, rr_Atanh, LIFT(rr::Atanh))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Atanh_highp, LIFT(sw::Atanh), false /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Atanh_mediump, LIFT(sw::Atanh), true /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental2, rr_Atan2, LIFT(rr::Atan2))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental2, sw_Atan2_highp, LIFT(sw::Atan2), false /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental2, sw_Atan2_mediump, LIFT(sw::Atan2), true /* relaxedPrecision */)->Arg(REPS);

BENCHMARK_CAPTURE(Transcendental2, rr_Pow, LIFT(rr::Pow))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental2, sw_Pow_highp, LIFT(sw::Pow<sw::Highp>))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental2, sw_Pow_mediump, LIFT(sw::Pow<sw::Mediump>))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, rr_Exp, LIFT(rr::Exp))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Exp_highp, LIFT(sw::Exp), false /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Exp_mediump, LIFT(sw::Exp), true /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, rr_Log, LIFT(rr::Log))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Log_highp, LIFT(sw::Log), false /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Log_mediump, LIFT(sw::Log), true /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, rr_Exp2, LIFT(rr::Exp2))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Exp2_highp, LIFT(sw::Exp2), false /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Exp2_mediump, LIFT(sw::Exp2), true /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, rr_Log2, LIFT(rr::Log2))->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Log2_highp, LIFT(sw::Log2), false /* relaxedPrecision */)->Arg(REPS);
BENCHMARK_CAPTURE(Transcendental1, sw_Log2_mediump, LIFT(sw::Log2), true /* relaxedPrecision */)->Arg(REPS);

}  // namespace sw