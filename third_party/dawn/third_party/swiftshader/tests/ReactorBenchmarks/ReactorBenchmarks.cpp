// Copyright 2020 The SwiftShader Authors. All Rights Reserved.
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

#include "Coroutine.hpp"
#include "Reactor.hpp"

#include "benchmark/benchmark.h"

using namespace rr;

BENCHMARK_MAIN();

class Coroutines : public benchmark::Fixture
{
public:
	void SetUp(const ::benchmark::State &state) {}

	void TearDown(const ::benchmark::State &state) {}
};

BENCHMARK_DEFINE_F(Coroutines, Fibonacci)
(benchmark::State &state)
{
	if(!Caps::coroutinesSupported())
	{
		state.SkipWithError("Coroutines are not supported");
		return;
	}

	Coroutine<int()> function;
	{
		Yield(Int(0));
		Yield(Int(1));
		Int current = 1;
		Int next = 1;
		While(true)
		{
			Yield(next);
			auto tmp = current + next;
			current = next;
			next = tmp;
		}
	}

	auto coroutine = function();

	const auto iterations = state.range(0);

	int out = 0;
	for(auto _ : state)
	{
		for(int64_t i = 0; i < iterations; i++)
		{
			coroutine->await(out);
		}
	}
}

BENCHMARK_REGISTER_F(Coroutines, Fibonacci)->RangeMultiplier(8)->Range(1, 0x1000000)->ArgName("iterations");

// Macro that creates a lambda wrapper around the input overloaded function,
// creating a non-overload based on the args. This is useful for passing
// overloaded functions as template arguments.
// See https://stackoverflow.com/questions/25871381/c-overloaded-function-as-template-argument
#define LIFT(fname)                                          \
	[](auto &&...args) -> decltype(auto) {                   \
		return fname(std::forward<decltype(args)>(args)...); \
	}

template<typename Func, class... Args>
static void Transcedental1(benchmark::State &state, Func func, Args &&...args)
{
	FunctionT<float(float)> function;
	{
		Float a = function.Arg<0>();
		Float4 v{ a };
		Float4 r = func(v, args...);
		Return(Float(r.x));
	}

	auto routine = function("one");

	for(auto _ : state)
	{
		routine(1.f);
	}
}

template<typename Func, class... Args>
static void Transcedental2(benchmark::State &state, Func func, Args &&...args)
{
	FunctionT<float(float, float)> function;
	{
		Float a = function.Arg<0>();
		Float b = function.Arg<1>();
		Float4 v1{ a };
		Float4 v2{ b };
		Float4 r = func(v1, v2, args...);
		Return(Float(r.x));
	}

	auto routine = function("two");

	for(auto _ : state)
	{
		routine(0.456f, 0.789f);
	}
}

BENCHMARK_CAPTURE(Transcedental1, rr_Sin, Sin);
BENCHMARK_CAPTURE(Transcedental1, rr_Cos, Cos);
BENCHMARK_CAPTURE(Transcedental1, rr_Tan, Tan);

BENCHMARK_CAPTURE(Transcedental1, rr_Asin, Asin);
BENCHMARK_CAPTURE(Transcedental1, rr_Acos, Acos);

BENCHMARK_CAPTURE(Transcedental1, rr_Atan, Atan);
BENCHMARK_CAPTURE(Transcedental1, rr_Sinh, Sinh);
BENCHMARK_CAPTURE(Transcedental1, rr_Cosh, Cosh);
BENCHMARK_CAPTURE(Transcedental1, rr_Tanh, Tanh);

BENCHMARK_CAPTURE(Transcedental1, rr_Asinh, Asinh);
BENCHMARK_CAPTURE(Transcedental1, rr_Acosh, Acosh);
BENCHMARK_CAPTURE(Transcedental1, rr_Atanh, Atanh);
BENCHMARK_CAPTURE(Transcedental2, rr_Atan2, Atan2);

BENCHMARK_CAPTURE(Transcedental2, rr_Pow, Pow);
BENCHMARK_CAPTURE(Transcedental1, rr_Exp, Exp);
BENCHMARK_CAPTURE(Transcedental1, rr_Log, Log);
BENCHMARK_CAPTURE(Transcedental1, rr_Exp2, LIFT(Exp2));
BENCHMARK_CAPTURE(Transcedental1, rr_Log2, LIFT(Log2));
