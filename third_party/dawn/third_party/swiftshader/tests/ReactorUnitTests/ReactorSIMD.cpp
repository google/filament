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

#include "Reactor.hpp"
#include "SIMD.hpp"

#include "gtest/gtest.h"

using namespace rr;

static std::string testName()
{
	auto info = ::testing::UnitTest::GetInstance()->current_test_info();
	return std::string{ info->test_suite_name() } + "_" + info->name();
}

TEST(ReactorSIMD, Add)
{
	ASSERT_GE(SIMD::Width, 4);

	constexpr int arrayLength = 1024;

	FunctionT<void(int *, int *, int *)> function;
	{
		Pointer<Int> r = Pointer<Int>(function.Arg<0>());
		Pointer<Int> a = Pointer<Int>(function.Arg<1>());
		Pointer<Int> b = Pointer<Int>(function.Arg<2>());

		For(Int i = 0, i < arrayLength, i += SIMD::Width)
		{
			SIMD::Int x = *Pointer<SIMD::Int>(&a[i]);
			SIMD::Int y = *Pointer<SIMD::Int>(&b[i]);

			SIMD::Int z = x + y;

			*Pointer<SIMD::Int>(&r[i]) = z;
		}
	}

	auto routine = function(testName().c_str());

	int r[arrayLength] = {};
	int a[arrayLength];
	int b[arrayLength];

	for(int i = 0; i < arrayLength; i++)
	{
		a[i] = i;
		b[i] = arrayLength + i;
	}

	routine(r, a, b);

	for(int i = 0; i < arrayLength; i++)
	{
		ASSERT_EQ(r[i], arrayLength + 2 * i);
	}
}

TEST(ReactorSIMD, Broadcast)
{
	FunctionT<void(int *, int)> function;
	{
		Pointer<Int> r = Pointer<Int>(function.Arg<0>());
		Int a = function.Arg<1>();

		SIMD::Int x = a;

		*Pointer<SIMD::Int>(r) = x;
	}

	auto routine = function(testName().c_str());

	std::vector<int> r(SIMD::Width);

	for(int a = -2; a <= 2; a++)
	{
		routine(r.data(), a);

		for(int i = 0; i < SIMD::Width; i++)
		{
			ASSERT_EQ(r[i], a);
		}
	}
}

TEST(ReactorSIMD, InsertExtract128)
{
	FunctionT<void(int *, int *)> function;
	{
		Pointer<Int> r = Pointer<Int>(function.Arg<0>());
		Pointer<Int> a = Pointer<Int>(function.Arg<1>());

		SIMD::Int x = *Pointer<SIMD::Int>(a);
		SIMD::Int y = *Pointer<SIMD::Int>(r);

		x -= y;

		for(int i = 0; i < SIMD::Width / 4; i++)
		{
			y = Insert128(y, Extract128(x, i) << (i + 1), i);
		}

		*Pointer<SIMD::Int>(r) = y;
	}

	auto routine = function(testName().c_str());

	std::vector<int> r(SIMD::Width);
	std::vector<int> a(SIMD::Width);

	for(int i = 0; i < SIMD::Width; i++)
	{
		r[i] = 0;
		a[i] = 1 + i;
	}

	routine(r.data(), a.data());

	for(int i = 0; i < SIMD::Width; i++)
	{
		ASSERT_EQ(r[i], a[i] << (i / 4 + 1));
	}
}

TEST(ReactorSIMD, Intrinsics_Scatter)
{
	Function<Void(Pointer<Float> base, Pointer<SIMD::Float> val, Pointer<SIMD::Int> offsets)> function;
	{
		Pointer<Float> base = function.Arg<0>();
		Pointer<SIMD::Float> val = function.Arg<1>();
		Pointer<SIMD::Int> offsets = function.Arg<2>();

		SIMD::Int mask = ~0;
		unsigned int alignment = 1;
		Scatter(base, *val, *offsets, mask, alignment);
	}

	std::vector<float> buffer(10 + 10 * SIMD::Width);
	std::vector<int> offsets(SIMD::Width);
	std::vector<float> val(SIMD::Width);

	for(int i = 0; i < SIMD::Width; i++)
	{
		offsets[i] = (3 + 7 * i) * sizeof(float);
		val[i] = 13.0f + 17.0f * i;
	}

	auto routine = function(testName().c_str());
	auto entry = (void (*)(float *, float *, int *))routine->getEntry();

	entry(buffer.data(), val.data(), offsets.data());

	for(int i = 0; i < SIMD::Width; i++)
	{
		EXPECT_EQ(buffer[offsets[i] / sizeof(float)], val[i]);
	}
}

TEST(ReactorSIMD, Intrinsics_Gather)
{
	Function<Void(Pointer<Float> base, Pointer<SIMD::Int> offsets, Pointer<SIMD::Float> result)> function;
	{
		Pointer<Float> base = function.Arg<0>();
		Pointer<SIMD::Int> offsets = function.Arg<1>();
		Pointer<SIMD::Float> result = function.Arg<2>();

		SIMD::Int mask = ~0;
		unsigned int alignment = 1;
		bool zeroMaskedLanes = true;
		*result = Gather(base, *offsets, mask, alignment, zeroMaskedLanes);
	}

	std::vector<float> buffer(10 + 10 * SIMD::Width);
	std::vector<int> offsets(SIMD::Width);

	std::vector<float> val(SIMD::Width);

	for(int i = 0; i < SIMD::Width; i++)
	{
		offsets[i] = (3 + 7 * i) * sizeof(float);
		val[i] = 13.0f + 17.0f * i;

		buffer[offsets[i] / sizeof(float)] = val[i];
	}

	auto routine = function(testName().c_str());
	auto entry = (void (*)(float *, int *, float *))routine->getEntry();

	std::vector<float> result(SIMD::Width);
	entry(buffer.data(), offsets.data(), result.data());

	for(int i = 0; i < SIMD::Width; i++)
	{
		EXPECT_EQ(result[i], val[i]);
	}
}
