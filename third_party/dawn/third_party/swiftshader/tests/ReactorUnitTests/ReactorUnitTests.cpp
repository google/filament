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

#include "Assert.hpp"
#include "Coroutine.hpp"
#include "Print.hpp"
#include "Reactor.hpp"

#include "gtest/gtest.h"

#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <thread>
#include <tuple>

using namespace rr;

using float4 = float[4];
using int4 = int[4];

static std::string testName()
{
	auto info = ::testing::UnitTest::GetInstance()->current_test_info();
	return std::string{ info->test_suite_name() } + "_" + info->name();
}

int reference(int *p, int y)
{
	int x = p[-1];
	int z = 4;

	for(int i = 0; i < 10; i++)
	{
		z += (2 << i) - (i / 3);
	}

	int sum = x + y + z;

	return sum;
}

TEST(ReactorUnitTests, Sample)
{
	FunctionT<int(int *, int)> function;
	{
		Pointer<Int> p = function.Arg<0>();
		Int x = p[-1];
		Int y = function.Arg<1>();
		Int z = 4;

		For(Int i = 0, i < 10, i++)
		{
			z += (2 << i) - (i / 3);
		}

		Float4 v;
		v.z = As<Float>(z);
		z = As<Int>(Float(Float4(v.xzxx).y));

		Int sum = x + y + z;

		Return(sum);
	}

	auto routine = function(testName().c_str());

	int one[2] = { 1, 0 };
	int result = routine(&one[1], 2);
	EXPECT_EQ(result, reference(&one[1], 2));
}

// This test demonstrates the use of a 'trampoline', where a routine calls
// a static function which then generates another routine during the execution
// of the first routine. Also note the code generated for the second routine
// depends on a parameter passed to the first routine.
TEST(ReactorUnitTests, Trampoline)
{
	using SecondaryFunc = int(int, int);

	static auto generateSecondary = [](int upDown) {
		FunctionT<SecondaryFunc> secondary;
		{
			Int x = secondary.Arg<0>();
			Int y = secondary.Arg<1>();
			Int r;

			if(upDown > 0)
			{
				r = x + y;
			}
			else if(upDown < 0)
			{
				r = x - y;
			}
			else
			{
				r = 0;
			}

			Return(r);
		}

		static auto routine = secondary((testName() + "_secondary").c_str());
		return routine.getEntry();
	};

	using SecondaryGeneratorFunc = SecondaryFunc *(*)(int);
	SecondaryGeneratorFunc secondaryGenerator = (SecondaryGeneratorFunc)generateSecondary;

	using PrimaryFunc = int(int, int, int);

	FunctionT<PrimaryFunc> primary;
	{
		Int x = primary.Arg<0>();
		Int y = primary.Arg<1>();
		Int z = primary.Arg<2>();

		Pointer<Byte> secondary = Call(secondaryGenerator, z);
		Int r = Call<SecondaryFunc>(secondary, x, y);

		Return(r);
	}

	auto routine = primary((testName() + "_primary").c_str());

	int result = routine(100, 20, -3);
	EXPECT_EQ(result, 80);
}

TEST(ReactorUnitTests, Uninitialized)
{
#if __has_feature(memory_sanitizer)
	// Building the static C++ code with MemorySanitizer enabled does not
	// automatically enable MemorySanitizer instrumentation for Reactor
	// routines. False positives can also be prevented by unpoisoning all
	// memory writes. This Pragma ensures proper instrumentation is enabled.
	Pragma(MemorySanitizerInstrumentation, true);
#endif

	FunctionT<int()> function;
	{
		Int a;
		Int z = 4;
		Int q;
		Int c;
		Int p;
		Bool b;

		q += q;

		If(b)
		{
			c = p;
		}

		Return(a + z + q + c);
	}

	auto routine = function(testName().c_str());

	if(!__has_feature(memory_sanitizer))
	{
		int result = routine();
		EXPECT_EQ(result, result);  // Anything is fine, just don't crash
	}
	else
	{
		// Optimizations may turn the conditional If() in the Reactor code
		// into a conditional move or arithmetic operations, which would not
		// trigger a MemorySanitizer error. However, in that case the equals
		// operator below should trigger it before the abort is reached.
		EXPECT_DEATH(
		    {
			    int result = routine();
			    if(result == 0) abort();
		    },
		    "MemorySanitizer: use-of-uninitialized-value");
	}

	Pragma(MemorySanitizerInstrumentation, false);
}

TEST(ReactorUnitTests, Unreachable)
{
	FunctionT<int(int)> function;
	{
		Int a = function.Arg<0>();
		Int z = 4;

		Return(a + z);

		// Code beyond this point is unreachable but should not cause any
		// compilation issues.

		z += a;
	}

	auto routine = function(testName().c_str());

	int result = routine(16);
	EXPECT_EQ(result, 20);
}

// Stopping in the middle of a `Function<>` is supported and should not affect
// subsequent complete ones.
TEST(ReactorUnitTests, UnfinishedFunction)
{
	do
	{
		FunctionT<int(int)> function;
		{
			Int a = function.Arg<0>();
			Int z = 4;

			if((true)) break;  // Terminate do-while early.

			Return(a + z);
		}
	} while(true);

	FunctionT<int(int)> function;
	{
		Int a = function.Arg<0>();
		Int z = 4;

		Return(a - z);
	}

	auto routine = function(testName().c_str());

	int result = routine(16);
	EXPECT_EQ(result, 12);
}

// Deriving from `Function<>` and using Reactor variables as members can be a
// convenient way to 'name' function arguments and compose complex functions
// with helper methods. This test checks the interactions between the lifetime
// of the `Function<>` and the variables belonging to the derived class.
struct FunctionMembers : FunctionT<int(int)>
{
	FunctionMembers()
	    : level(Arg<0>())
	{
		For(Int i = 0, i < 3, i++)
		{
			pourSomeMore();
		}

		Return(level);
	}

	void pourSomeMore()
	{
		level += 2;
	}

	Int level;
};

TEST(ReactorUnitTests, FunctionMembers)
{
	FunctionMembers function;

	auto routine = function(testName().c_str());

	int result = routine(3);
	EXPECT_EQ(result, 9);
}

// This test excercises modifying the value of a local variable through a
// pointer to it.
TEST(ReactorUnitTests, VariableAddress)
{
	FunctionT<int(int)> function;
	{
		Int a = function.Arg<0>();
		Int z = 0;
		Pointer<Int> p = &z;
		*p = 4;

		Return(a + z);
	}

	auto routine = function(testName().c_str());

	int result = routine(16);
	EXPECT_EQ(result, 20);
}

// This test exercises taking the address of a local varible at the end of a
// loop and modifying its value through the pointer in the second iteration.
TEST(ReactorUnitTests, LateVariableAddress)
{
	FunctionT<int(void)> function;
	{
		Pointer<Int> p = nullptr;
		Int a = 0;

		While(a == 0)
		{
			If(p != Pointer<Int>(nullptr))
			{
				*p = 1;
			}

			p = &a;
		}

		Return(a);
	}

	auto routine = function(testName().c_str());

	int result = routine();
	EXPECT_EQ(result, 1);
}

// This test checks that the value of a local variable which has been modified
// though a pointer is correct at the point before its address is (statically)
// obtained.
TEST(ReactorUnitTests, LoadAfterIndirectStore)
{
	FunctionT<int(void)> function;
	{
		Pointer<Int> p = nullptr;
		Int a = 0;
		Int b = 0;

		While(a == 0)
		{
			If(p != Pointer<Int>(nullptr))
			{
				*p = 1;
			}

			// `a` must be loaded from memory here, despite not statically knowing
			// yet that its address will be taken below.
			b = a + 5;

			p = &a;
		}

		Return(b);
	}

	auto routine = function(testName().c_str());

	int result = routine();
	EXPECT_EQ(result, 6);
}

// This test checks that variables statically accessed after a Return statement
// are still loaded, modified, and stored correctly.
TEST(ReactorUnitTests, LoopAfterReturn)
{
	FunctionT<int(void)> function;
	{
		Int min = 100;
		Int max = 200;

		If(min > max)
		{
			Return(5);
		}

		While(min < max)
		{
			min++;
		}

		Return(7);
	}

	auto routine = function(testName().c_str());

	int result = routine();
	EXPECT_EQ(result, 7);
}

TEST(ReactorUnitTests, ConstantPointer)
{
	int c = 44;

	FunctionT<int()> function;
	{
		Int x = *Pointer<Int>(ConstantPointer(&c));

		Return(x);
	}

	auto routine = function(testName().c_str());

	int result = routine();
	EXPECT_EQ(result, 44);
}

// This test excercises the Optimizer::eliminateLoadsFollowingSingleStore() optimization pass.
// The three load operations for `y` should get eliminated.
TEST(ReactorUnitTests, EliminateLoadsFollowingSingleStore)
{
	FunctionT<int(int)> function;
	{
		Int x = function.Arg<0>();

		Int y;
		Int z;

		// This branch materializes the variables.
		If(x != 0)  // TODO(b/179922668): Support If(x)
		{
			y = x;
			z = y + y + y;
		}

		Return(z);
	}

	Nucleus::setOptimizerCallback([](const Nucleus::OptimizerReport *report) {
		EXPECT_EQ(report->allocas, 2);
		EXPECT_EQ(report->loads, 2);
		EXPECT_EQ(report->stores, 2);
	});

	auto routine = function(testName().c_str());

	int result = routine(11);
	EXPECT_EQ(result, 33);
}

// This test excercises the Optimizer::propagateAlloca() optimization pass.
// The pointer variable should not get stored to / loaded from memory.
TEST(ReactorUnitTests, PropagateAlloca)
{
	FunctionT<int(int)> function;
	{
		Int b = function.Arg<0>();

		Int a = 22;
		Pointer<Int> p;

		// This branch materializes both `a` and `p`, and ensures single basic block
		// optimizations don't also eliminate the pointer store and load.
		If(b != 0)  // TODO(b/179922668): Support If(b)
		{
			p = &a;
		}

		Return(Int(*p));  // TODO(b/179694472): Support Return(*p)
	}

	Nucleus::setOptimizerCallback([](const Nucleus::OptimizerReport *report) {
		EXPECT_EQ(report->allocas, 1);
		EXPECT_EQ(report->loads, 1);
		EXPECT_EQ(report->stores, 1);
	});

	auto routine = function(testName().c_str());

	int result = routine(true);
	EXPECT_EQ(result, 22);
}

// Corner case for Optimizer::propagateAlloca(). It should not replace loading of `p`
// with the addres of `a`, since it also got the address of `b` assigned.
TEST(ReactorUnitTests, PointerToPointer)
{
	FunctionT<int()> function;
	{
		Int a = 444;
		Int b = 555;

		Pointer<Int> p = &a;
		Pointer<Pointer<Int>> pp = &p;
		p = &b;

		Return(Int(*Pointer<Int>(*pp)));  // TODO(b/179694472): Support **pp
	}

	auto routine = function(testName().c_str());

	int result = routine();
	EXPECT_EQ(result, 555);
}

// Corner case for Optimizer::propagateAlloca(). It should not replace loading of `p[i]`
// with any of the addresses of the `a`, `b`, or `c`.
TEST(ReactorUnitTests, ArrayOfPointersToLocals)
{
	FunctionT<int(int)> function;
	{
		Int i = function.Arg<0>();

		Int a = 111;
		Int b = 222;
		Int c = 333;

		Array<Pointer<Int>, 3> p;
		p[0] = &a;
		p[1] = &b;
		p[2] = &c;

		Return(Int(*Pointer<Int>(p[i])));  // TODO(b/179694472): Support *p[i]
	}

	auto routine = function(testName().c_str());

	int result = routine(1);
	EXPECT_EQ(result, 222);
}

TEST(ReactorUnitTests, ModifyLocalThroughPointer)
{
	FunctionT<int(void)> function;
	{
		Int a = 1;

		Pointer<Int> p = &a;
		Pointer<Pointer<Int>> pp = &p;

		Pointer<Int> q = *pp;
		*q = 3;

		Return(a);
	}

	auto routine = function(testName().c_str());

	int result = routine();
	EXPECT_EQ(result, 3);
}

TEST(ReactorUnitTests, ScalarReplacementOfArray)
{
	FunctionT<int(void)> function;
	{
		Array<Int, 2> a;
		a[0] = 1;
		a[1] = 2;

		Return(a[0] + a[1]);
	}

	auto routine = function(testName().c_str());

	int result = routine();
	EXPECT_EQ(result, 3);
}

TEST(ReactorUnitTests, CArray)
{
	FunctionT<int(void)> function;
	{
		Int a[2];
		a[0] = 1;
		a[1] = 2;

		auto x = a[0];
		a[0] = a[1];
		a[1] = x;

		Return(a[0] + a[1]);
	}

	auto routine = function(testName().c_str());

	int result = routine();
	EXPECT_EQ(result, 3);
}

// SRoA should replace the array elements with scalars, which in turn enables
// eliminating all loads and stores.
TEST(ReactorUnitTests, ReactorArray)
{
	FunctionT<int(void)> function;
	{
		Array<Int, 2> a;
		a[0] = 1;
		a[1] = 2;

		Int x = a[0];
		a[0] = a[1];
		a[1] = x;

		Return(a[0] + a[1]);
	}

	Nucleus::setOptimizerCallback([](const Nucleus::OptimizerReport *report) {
		EXPECT_EQ(report->allocas, 0);
		EXPECT_EQ(report->loads, 0);
		EXPECT_EQ(report->stores, 0);
	});

	auto routine = function(testName().c_str());

	int result = routine();
	EXPECT_EQ(result, 3);
}

// Excercises the optimizeSingleBasicBlockLoadsStores optimization pass.
TEST(ReactorUnitTests, StoresInMultipleBlocks)
{
	FunctionT<int(int)> function;
	{
		Int b = function.Arg<0>();

		Int a = 13;

		If(b != 0)  // TODO(b/179922668): Support If(b)
		{
			a = 4;
			a = a + 3;
		}
		Else
		{
			a = 6;
			a = a + 5;
		}

		Return(a);
	}

	Nucleus::setOptimizerCallback([](const Nucleus::OptimizerReport *report) {
		EXPECT_EQ(report->allocas, 1);
		EXPECT_EQ(report->loads, 1);
		EXPECT_EQ(report->stores, 3);
	});

	auto routine = function(testName().c_str());

	int result = routine(true);
	EXPECT_EQ(result, 7);
}

// This is similar to the LoadAfterIndirectStore test except that the indirect
// store is preceded by a direct store. The subsequent load should not be replaced
// by the value written by the direct store.
TEST(ReactorUnitTests, StoreBeforeIndirectStore)
{
	FunctionT<int(int)> function;
	{
		// Int b = function.Arg<0>();

		Int b;
		Pointer<Int> p = &b;
		Int a = 13;

		For(Int i = 0, i < 2, i++)
		{
			a = 10;

			*p = 4;

			// This load of `a` should not be replaced by the 10 written above, since
			// in the second iteration `p` points to `a` and writes 4.
			b = a;

			p = &a;
		}

		Return(b);
	}

	auto routine = function(testName().c_str());

	int result = routine(true);
	EXPECT_EQ(result, 4);
}

TEST(ReactorUnitTests, AssertTrue)
{
	FunctionT<int()> function;
	{
		Int a = 3;
		Int b = 5;

		Assert(a < b);

		Return(a + b);
	}

	auto routine = function(testName().c_str());

	int result = routine();
	EXPECT_EQ(result, 8);
}

TEST(ReactorUnitTests, AssertFalse)
{
	FunctionT<int()> function;
	{
		Int a = 3;
		Int b = 5;

		Assert(a == b);

		Return(a + b);
	}

	auto routine = function(testName().c_str());

#ifndef NDEBUG
#	if !defined(__APPLE__)
	const char *stderrRegex = "AssertFalse";  // stderr should contain the assert's expression, file:line, and function
#	else
	const char *stderrRegex = "";  // TODO(b/156389924): On macOS an stderr redirect can cause googletest to fail the capture
#	endif

	EXPECT_DEATH(
	    {
		    int result = routine();
		    EXPECT_NE(result, result);  // We should never reach this
	    },
	    stderrRegex);
#else
	int result = routine();
	EXPECT_EQ(result, 8);
#endif
}

TEST(ReactorUnitTests, SubVectorLoadStore)
{
	FunctionT<int(void *, void *)> function;
	{
		Pointer<Byte> in = function.Arg<0>();
		Pointer<Byte> out = function.Arg<1>();

		*Pointer<Int4>(out + 16 * 0) = *Pointer<Int4>(in + 16 * 0);
		*Pointer<Short4>(out + 16 * 1) = *Pointer<Short4>(in + 16 * 1);
		*Pointer<Byte8>(out + 16 * 2) = *Pointer<Byte8>(in + 16 * 2);
		*Pointer<Byte4>(out + 16 * 3) = *Pointer<Byte4>(in + 16 * 3);
		*Pointer<Short2>(out + 16 * 4) = *Pointer<Short2>(in + 16 * 4);

		Return(0);
	}

	auto routine = function(testName().c_str());

	int8_t in[16 * 5] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
		                  17, 18, 19, 20, 21, 22, 23, 24, 0, 0, 0, 0, 0, 0, 0, 0,
		                  25, 26, 27, 28, 29, 30, 31, 32, 0, 0, 0, 0, 0, 0, 0, 0,
		                  33, 34, 35, 36, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		                  37, 38, 39, 40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	int8_t out[16 * 5] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		                   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		                   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		                   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		                   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

	routine(in, out);

	for(int row = 0; row < 5; row++)
	{
		for(int col = 0; col < 16; col++)
		{
			int i = row * 16 + col;

			if(in[i] == 0)
			{
				EXPECT_EQ(out[i], -1) << "Row " << row << " column " << col << " not left untouched.";
			}
			else
			{
				EXPECT_EQ(out[i], in[i]) << "Row " << row << " column " << col << " not equal to input.";
			}
		}
	}
}

TEST(ReactorUnitTests, VectorConstant)
{
	FunctionT<int(void *)> function;
	{
		Pointer<Byte> out = function.Arg<0>();

		*Pointer<Int4>(out + 16 * 0) = Int4(0x04030201, 0x08070605, 0x0C0B0A09, 0x100F0E0D);
		*Pointer<Short4>(out + 16 * 1) = Short4(0x1211, 0x1413, 0x1615, 0x1817);
		*Pointer<Byte8>(out + 16 * 2) = Byte8(0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20);
		*Pointer<Int2>(out + 16 * 3) = Int2(0x24232221, 0x28272625);

		Return(0);
	}

	auto routine = function(testName().c_str());

	int8_t out[16 * 4] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		                   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		                   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		                   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

	int8_t exp[16 * 4] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
		                   17, 18, 19, 20, 21, 22, 23, 24, -1, -1, -1, -1, -1, -1, -1, -1,
		                   25, 26, 27, 28, 29, 30, 31, 32, -1, -1, -1, -1, -1, -1, -1, -1,
		                   33, 34, 35, 36, 37, 38, 39, 40, -1, -1, -1, -1, -1, -1, -1, -1 };

	routine(out);

	for(int row = 0; row < 4; row++)
	{
		for(int col = 0; col < 16; col++)
		{
			int i = row * 16 + col;

			EXPECT_EQ(out[i], exp[i]);
		}
	}
}

TEST(ReactorUnitTests, Concatenate)
{
	FunctionT<int(void *)> function;
	{
		Pointer<Byte> out = function.Arg<0>();

		*Pointer<Int4>(out + 16 * 0) = Int4(Int2(0x04030201, 0x08070605), Int2(0x0C0B0A09, 0x100F0E0D));
		*Pointer<Short8>(out + 16 * 1) = Short8(Short4(0x0201, 0x0403, 0x0605, 0x0807), Short4(0x0A09, 0x0C0B, 0x0E0D, 0x100F));

		Return(0);
	}

	auto routine = function(testName().c_str());

	int8_t ref[16 * 5] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
		                   1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };

	int8_t out[16 * 5] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
		                   -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 };

	routine(out);

	for(int row = 0; row < 2; row++)
	{
		for(int col = 0; col < 16; col++)
		{
			int i = row * 16 + col;

			EXPECT_EQ(out[i], ref[i]) << "Row " << row << " column " << col << " not equal to reference.";
		}
	}
}

TEST(ReactorUnitTests, Cast)
{
	FunctionT<void(void *)> function;
	{
		Pointer<Byte> out = function.Arg<0>();

		Int4 c = Int4(0x01020304, 0x05060708, 0x09101112, 0x13141516);
		*Pointer<Short4>(out + 16 * 0) = Short4(c);
		*Pointer<Byte4>(out + 16 * 1 + 0) = Byte4(c);
		*Pointer<Byte4>(out + 16 * 1 + 4) = Byte4(As<Byte8>(c));
		*Pointer<Byte4>(out + 16 * 1 + 8) = Byte4(As<Short4>(c));
	}

	auto routine = function(testName().c_str());

	int out[2][4];

	memset(&out, 0, sizeof(out));

	routine(&out);

	EXPECT_EQ(out[0][0], 0x07080304);
	EXPECT_EQ(out[0][1], 0x15161112);

	EXPECT_EQ(out[1][0], 0x16120804);
	EXPECT_EQ(out[1][1], 0x01020304);
	EXPECT_EQ(out[1][2], 0x06080204);
}

static uint16_t swizzleCode4(int i)
{
	auto x = (i >> 0) & 0x03;
	auto y = (i >> 2) & 0x03;
	auto z = (i >> 4) & 0x03;
	auto w = (i >> 6) & 0x03;
	return static_cast<uint16_t>((x << 12) | (y << 8) | (z << 4) | (w << 0));
}

TEST(ReactorUnitTests, Swizzle4)
{
	FunctionT<void(void *)> function;
	{
		Pointer<Byte> out = function.Arg<0>();

		for(int i = 0; i < 256; i++)
		{
			*Pointer<Float4>(out + 16 * i) = Swizzle(Float4(1.0f, 2.0f, 3.0f, 4.0f), swizzleCode4(i));
		}

		for(int i = 0; i < 256; i++)
		{
			*Pointer<Float4>(out + 16 * (256 + i)) = ShuffleLowHigh(Float4(1.0f, 2.0f, 3.0f, 4.0f), Float4(5.0f, 6.0f, 7.0f, 8.0f), swizzleCode4(i));
		}

		*Pointer<Float4>(out + 16 * (512 + 0)) = UnpackLow(Float4(1.0f, 2.0f, 3.0f, 4.0f), Float4(5.0f, 6.0f, 7.0f, 8.0f));
		*Pointer<Float4>(out + 16 * (512 + 1)) = UnpackHigh(Float4(1.0f, 2.0f, 3.0f, 4.0f), Float4(5.0f, 6.0f, 7.0f, 8.0f));
		*Pointer<Int2>(out + 16 * (512 + 2)) = UnpackLow(Short4(1, 2, 3, 4), Short4(5, 6, 7, 8));
		*Pointer<Int2>(out + 16 * (512 + 3)) = UnpackHigh(Short4(1, 2, 3, 4), Short4(5, 6, 7, 8));
		*Pointer<Short4>(out + 16 * (512 + 4)) = UnpackLow(Byte8(1, 2, 3, 4, 5, 6, 7, 8), Byte8(9, 10, 11, 12, 13, 14, 15, 16));
		*Pointer<Short4>(out + 16 * (512 + 5)) = UnpackHigh(Byte8(1, 2, 3, 4, 5, 6, 7, 8), Byte8(9, 10, 11, 12, 13, 14, 15, 16));

		for(int i = 0; i < 256; i++)
		{
			*Pointer<Short4>(out + 16 * (512 + 6) + (8 * i)) =
			    Swizzle(Short4(1, 2, 3, 4), swizzleCode4(i));
		}

		for(int i = 0; i < 256; i++)
		{
			*Pointer<Int4>(out + 16 * (512 + 6 + i) + (8 * 256)) =
			    Swizzle(Int4(1, 2, 3, 4), swizzleCode4(i));
		}
	}

	auto routine = function(testName().c_str());

	struct
	{
		float f[256 + 256 + 2][4];
		int i[388][4];
	} out;

	memset(&out, 0, sizeof(out));

	routine(&out);

	for(int i = 0; i < 256; i++)
	{
		EXPECT_EQ(out.f[i][0], float((i >> 0) & 0x03) + 1.0f);
		EXPECT_EQ(out.f[i][1], float((i >> 2) & 0x03) + 1.0f);
		EXPECT_EQ(out.f[i][2], float((i >> 4) & 0x03) + 1.0f);
		EXPECT_EQ(out.f[i][3], float((i >> 6) & 0x03) + 1.0f);
	}

	for(int i = 0; i < 256; i++)
	{
		EXPECT_EQ(out.f[256 + i][0], float((i >> 0) & 0x03) + 1.0f);
		EXPECT_EQ(out.f[256 + i][1], float((i >> 2) & 0x03) + 1.0f);
		EXPECT_EQ(out.f[256 + i][2], float((i >> 4) & 0x03) + 5.0f);
		EXPECT_EQ(out.f[256 + i][3], float((i >> 6) & 0x03) + 5.0f);
	}

	EXPECT_EQ(out.f[512 + 0][0], 1.0f);
	EXPECT_EQ(out.f[512 + 0][1], 5.0f);
	EXPECT_EQ(out.f[512 + 0][2], 2.0f);
	EXPECT_EQ(out.f[512 + 0][3], 6.0f);

	EXPECT_EQ(out.f[512 + 1][0], 3.0f);
	EXPECT_EQ(out.f[512 + 1][1], 7.0f);
	EXPECT_EQ(out.f[512 + 1][2], 4.0f);
	EXPECT_EQ(out.f[512 + 1][3], 8.0f);

	EXPECT_EQ(out.i[0][0], 0x00050001);
	EXPECT_EQ(out.i[0][1], 0x00060002);
	EXPECT_EQ(out.i[0][2], 0x00000000);
	EXPECT_EQ(out.i[0][3], 0x00000000);

	EXPECT_EQ(out.i[1][0], 0x00070003);
	EXPECT_EQ(out.i[1][1], 0x00080004);
	EXPECT_EQ(out.i[1][2], 0x00000000);
	EXPECT_EQ(out.i[1][3], 0x00000000);

	EXPECT_EQ(out.i[2][0], 0x0A020901);
	EXPECT_EQ(out.i[2][1], 0x0C040B03);
	EXPECT_EQ(out.i[2][2], 0x00000000);
	EXPECT_EQ(out.i[2][3], 0x00000000);

	EXPECT_EQ(out.i[3][0], 0x0E060D05);
	EXPECT_EQ(out.i[3][1], 0x10080F07);
	EXPECT_EQ(out.i[3][2], 0x00000000);
	EXPECT_EQ(out.i[3][3], 0x00000000);

	for(int i = 0; i < 256; i++)
	{
		EXPECT_EQ(out.i[4 + i / 2][0 + (i % 2) * 2] & 0xFFFF,
		          ((i >> 0) & 0x03) + 1);
		EXPECT_EQ(out.i[4 + i / 2][0 + (i % 2) * 2] >> 16,
		          ((i >> 2) & 0x03) + 1);
		EXPECT_EQ(out.i[4 + i / 2][1 + (i % 2) * 2] & 0xFFFF,
		          ((i >> 4) & 0x03) + 1);
		EXPECT_EQ(out.i[4 + i / 2][1 + (i % 2) * 2] >> 16,
		          ((i >> 6) & 0x03) + 1);
	}

	for(int i = 0; i < 256; i++)
	{
		EXPECT_EQ(out.i[132 + i][0], ((i >> 0) & 0x03) + 1);
		EXPECT_EQ(out.i[132 + i][1], ((i >> 2) & 0x03) + 1);
		EXPECT_EQ(out.i[132 + i][2], ((i >> 4) & 0x03) + 1);
		EXPECT_EQ(out.i[132 + i][3], ((i >> 6) & 0x03) + 1);
	}
}

TEST(ReactorUnitTests, Swizzle)
{
	FunctionT<void(void *)> function;
	{
		Pointer<Byte> out = function.Arg<0>();

		Int4 c = Int4(0x01020304, 0x05060708, 0x09101112, 0x13141516);
		*Pointer<Byte16>(out + 16 * 0) = Swizzle(As<Byte16>(c), 0xFEDCBA9876543210ull);
		*Pointer<Byte8>(out + 16 * 1) = Swizzle(As<Byte8>(c), 0x76543210u);
		*Pointer<UShort8>(out + 16 * 2) = Swizzle(As<UShort8>(c), 0x76543210u);
	}

	auto routine = function(testName().c_str());

	int out[3][4];

	memset(&out, 0, sizeof(out));

	routine(&out);

	EXPECT_EQ(out[0][0], 0x16151413);
	EXPECT_EQ(out[0][1], 0x12111009);
	EXPECT_EQ(out[0][2], 0x08070605);
	EXPECT_EQ(out[0][3], 0x04030201);

	EXPECT_EQ(out[1][0], 0x08070605);
	EXPECT_EQ(out[1][1], 0x04030201);

	EXPECT_EQ(out[2][0], 0x15161314);
	EXPECT_EQ(out[2][1], 0x11120910);
	EXPECT_EQ(out[2][2], 0x07080506);
	EXPECT_EQ(out[2][3], 0x03040102);
}

TEST(ReactorUnitTests, Shuffle)
{
	// |select| is [0aaa:0bbb:0ccc:0ddd] where |aaa|, |bbb|, |ccc|
	// and |ddd| are 7-bit selection indices. For a total (1 << 12)
	// possibilities.
	const int kSelectRange = 1 << 12;

	// Unfortunately, testing the whole kSelectRange results in a test
	// that is far too slow to run, because LLVM spends exponentially more
	// time optimizing the function below as the number of test cases
	// increases.
	//
	// To work-around the problem, only test a subset of the range by
	// skipping every kRangeIncrement value.
	//
	// Set this value to 1 if you want to test the whole implementation,
	// which will take a little less than 2 minutes on a fast workstation.
	//
	// The default value here takes about 1390ms, which is a little more than
	// what the Swizzle test takes (993 ms) on my machine. A non-power-of-2
	// value ensures a better spread over possible values.
	const int kRangeIncrement = 11;

	auto rangeIndexToSelect = [](int i) {
		return static_cast<unsigned short>(
		    (((i >> 9) & 7) << 0) |
		    (((i >> 6) & 7) << 4) |
		    (((i >> 3) & 7) << 8) |
		    (((i >> 0) & 7) << 12));
	};

	FunctionT<int(void *)> function;
	{
		Pointer<Byte> out = function.Arg<0>();

		for(int i = 0; i < kSelectRange; i += kRangeIncrement)
		{
			unsigned short select = rangeIndexToSelect(i);

			*Pointer<Float4>(out + 16 * i) = Shuffle(Float4(1.0f, 2.0f, 3.0f, 4.0f),
			                                         Float4(5.0f, 6.0f, 7.0f, 8.0f),
			                                         select);

			*Pointer<Int4>(out + (kSelectRange + i) * 16) = Shuffle(Int4(10, 11, 12, 13),
			                                                        Int4(14, 15, 16, 17),
			                                                        select);

			*Pointer<UInt4>(out + (2 * kSelectRange + i) * 16) = Shuffle(UInt4(100, 101, 102, 103),
			                                                             UInt4(104, 105, 106, 107),
			                                                             select);
		}

		Return(0);
	}

	auto routine = function(testName().c_str());

	struct
	{
		float f[kSelectRange][4];
		int i[kSelectRange][4];
		unsigned u[kSelectRange][4];
	} out;

	memset(&out, 0, sizeof(out));

	routine(&out);

	for(int i = 0; i < kSelectRange; i += kRangeIncrement)
	{
		EXPECT_EQ(out.f[i][0], float(1.0f + (i & 7)));
		EXPECT_EQ(out.f[i][1], float(1.0f + ((i >> 3) & 7)));
		EXPECT_EQ(out.f[i][2], float(1.0f + ((i >> 6) & 7)));
		EXPECT_EQ(out.f[i][3], float(1.0f + ((i >> 9) & 7)));
	}

	for(int i = 0; i < kSelectRange; i += kRangeIncrement)
	{
		EXPECT_EQ(out.i[i][0], int(10 + (i & 7)));
		EXPECT_EQ(out.i[i][1], int(10 + ((i >> 3) & 7)));
		EXPECT_EQ(out.i[i][2], int(10 + ((i >> 6) & 7)));
		EXPECT_EQ(out.i[i][3], int(10 + ((i >> 9) & 7)));
	}

	for(int i = 0; i < kSelectRange; i += kRangeIncrement)
	{
		EXPECT_EQ(out.u[i][0], unsigned(100 + (i & 7)));
		EXPECT_EQ(out.u[i][1], unsigned(100 + ((i >> 3) & 7)));
		EXPECT_EQ(out.u[i][2], unsigned(100 + ((i >> 6) & 7)));
		EXPECT_EQ(out.u[i][3], unsigned(100 + ((i >> 9) & 7)));
	}
}

TEST(ReactorUnitTests, Broadcast)
{
	FunctionT<int()> function;
	{
		Int4 i = 2;
		Int j = 3 + i.x;
		Int4 k = i * 7;

		Return(k.z - j);
	}

	auto routine = function(testName().c_str());

	int result = routine();

	EXPECT_EQ(result, 9);
}

TEST(ReactorUnitTests, Branching)
{
	FunctionT<int()> function;
	{
		Int x = 0;

		For(Int i = 0, i < 8, i++)
		{
			If(i < 2)
			{
				x += 1;
			}
			Else If(i < 4)
			{
				x += 10;
			}
			Else If(i < 6)
			{
				x += 100;
			}
			Else
			{
				x += 1000;
			}

			For(Int i = 0, i < 5, i++)
			    x += 10000;
		}

		For(Int i = 0, i < 10, i++) for(int i = 0; i < 10; i++)
		    For(Int i = 0, i < 10, i++)
		{
			x += 1000000;
		}

		For(Int i = 0, i < 2, i++)
		    If(x == 1000402222)
		{
			If(x != 1000402222)
			    x += 1000000000;
		}
		Else
		    x = -5;

		Return(x);
	}

	auto routine = function(testName().c_str());

	int result = routine();

	EXPECT_EQ(result, 1000402222);
}

TEST(ReactorUnitTests, FMulAdd)
{
	Function<Void(Pointer<Float4>, Pointer<Float4>, Pointer<Float4>, Pointer<Float4>)> function;
	{
		Pointer<Float4> r = function.Arg<0>();
		Pointer<Float4> x = function.Arg<1>();
		Pointer<Float4> y = function.Arg<2>();
		Pointer<Float4> z = function.Arg<3>();

		*r = MulAdd(*x, *y, *z);
	}

	auto routine = function(testName().c_str());
	auto callable = (void (*)(float4 *, float4 *, float4 *, float4 *))routine->getEntry();

	float x[] = { 0.0f, 2.0f, 4.0f, 1.00000011920929f };
	float y[] = { 0.0f, 3.0f, 0.0f, 53400708.0f };
	float z[] = { 0.0f, 0.0f, 7.0f, -53400708.0f };

	for(size_t i = 0; i < std::size(x); i++)
	{
		float4 x_in = { x[i], x[i], x[i], x[i] };
		float4 y_in = { y[i], y[i], y[i], y[i] };
		float4 z_in = { z[i], z[i], z[i], z[i] };
		float4 r_out;

		callable(&r_out, &x_in, &y_in, &z_in);

		// Possible results
		float fma = fmaf(x[i], y[i], z[i]);
		float mul_add = x[i] * y[i] + z[i];

		// If the backend and the CPU support FMA instructions, we assume MulAdd to use
		// them. Otherwise it may behave as a multiplication followed by an addition.
		if(rr::Caps::fmaIsFast())
		{
			EXPECT_FLOAT_EQ(r_out[0], fma);
		}
		else if(r_out[0] != fma)
		{
			EXPECT_FLOAT_EQ(r_out[0], mul_add);
		}
	}
}

TEST(ReactorUnitTests, FMA)
{
	Function<Void(Pointer<Float4>, Pointer<Float4>, Pointer<Float4>, Pointer<Float4>)> function;
	{
		Pointer<Float4> r = function.Arg<0>();
		Pointer<Float4> x = function.Arg<1>();
		Pointer<Float4> y = function.Arg<2>();
		Pointer<Float4> z = function.Arg<3>();

		*r = FMA(*x, *y, *z);
	}

	auto routine = function(testName().c_str());
	auto callable = (void (*)(float4 *, float4 *, float4 *, float4 *))routine->getEntry();

	float x[] = { 0.0f, 2.0f, 4.0f, 1.00000011920929f };
	float y[] = { 0.0f, 3.0f, 0.0f, 53400708.0f };
	float z[] = { 0.0f, 0.0f, 7.0f, -53400708.0f };

	for(size_t i = 0; i < std::size(x); i++)
	{
		float4 x_in = { x[i], x[i], x[i], x[i] };
		float4 y_in = { y[i], y[i], y[i], y[i] };
		float4 z_in = { z[i], z[i], z[i], z[i] };
		float4 r_out;

		callable(&r_out, &x_in, &y_in, &z_in);

		float expected = fmaf(x[i], y[i], z[i]);
		EXPECT_FLOAT_EQ(r_out[0], expected);
	}
}

TEST(ReactorUnitTests, FAbs)
{
	Function<Void(Pointer<Float4>, Pointer<Float4>)> function;
	{
		Pointer<Float4> x = function.Arg<0>();
		Pointer<Float4> y = function.Arg<1>();

		*y = Abs(*x);
	}

	auto routine = function(testName().c_str());
	auto callable = (void (*)(float4 *, float4 *))routine->getEntry();

	float input[] = { 1.0f, -1.0f, -0.0f, 0.0f };

	for(float x : input)
	{
		float4 v_in = { x, x, x, x };
		float4 v_out;

		callable(&v_in, &v_out);

		float expected = fabs(x);
		EXPECT_FLOAT_EQ(v_out[0], expected);
	}
}

TEST(ReactorUnitTests, Abs)
{
	Function<Void(Pointer<Int4>, Pointer<Int4>)> function;
	{
		Pointer<Int4> x = function.Arg<0>();
		Pointer<Int4> y = function.Arg<1>();

		*y = Abs(*x);
	}

	auto routine = function(testName().c_str());
	auto callable = (void (*)(int4 *, int4 *))routine->getEntry();

	int input[] = { 1, -1, 0, (int)0x80000000 };

	for(int x : input)
	{
		int4 v_in = { x, x, x, x };
		int4 v_out;

		callable(&v_in, &v_out);

		float expected = abs(x);
		EXPECT_EQ(v_out[0], expected);
	}
}

TEST(ReactorUnitTests, MinMax)
{
	FunctionT<int(void *)> function;
	{
		Pointer<Byte> out = function.Arg<0>();

		*Pointer<Float4>(out + 16 * 0) = Min(Float4(1.0f, 0.0f, -0.0f, +0.0f), Float4(0.0f, 1.0f, +0.0f, -0.0f));
		*Pointer<Float4>(out + 16 * 1) = Max(Float4(1.0f, 0.0f, -0.0f, +0.0f), Float4(0.0f, 1.0f, +0.0f, -0.0f));

		*Pointer<Int4>(out + 16 * 2) = Min(Int4(1, 0, -1, -0), Int4(0, 1, 0, +0));
		*Pointer<Int4>(out + 16 * 3) = Max(Int4(1, 0, -1, -0), Int4(0, 1, 0, +0));
		*Pointer<UInt4>(out + 16 * 4) = Min(UInt4(1, 0, -1, -0), UInt4(0, 1, 0, +0));
		*Pointer<UInt4>(out + 16 * 5) = Max(UInt4(1, 0, -1, -0), UInt4(0, 1, 0, +0));

		*Pointer<Short4>(out + 16 * 6) = Min(Short4(1, 0, -1, -0), Short4(0, 1, 0, +0));
		*Pointer<Short4>(out + 16 * 7) = Max(Short4(1, 0, -1, -0), Short4(0, 1, 0, +0));
		*Pointer<UShort4>(out + 16 * 8) = Min(UShort4(1, 0, -1, -0), UShort4(0, 1, 0, +0));
		*Pointer<UShort4>(out + 16 * 9) = Max(UShort4(1, 0, -1, -0), UShort4(0, 1, 0, +0));

		Return(0);
	}

	auto routine = function(testName().c_str());

	unsigned int out[10][4];

	memset(&out, 0, sizeof(out));

	routine(&out);

	EXPECT_EQ(out[0][0], 0x00000000u);
	EXPECT_EQ(out[0][1], 0x00000000u);
	EXPECT_EQ(out[0][2], 0x00000000u);
	EXPECT_EQ(out[0][3], 0x80000000u);

	EXPECT_EQ(out[1][0], 0x3F800000u);
	EXPECT_EQ(out[1][1], 0x3F800000u);
	EXPECT_EQ(out[1][2], 0x00000000u);
	EXPECT_EQ(out[1][3], 0x80000000u);

	EXPECT_EQ(out[2][0], 0x00000000u);
	EXPECT_EQ(out[2][1], 0x00000000u);
	EXPECT_EQ(out[2][2], 0xFFFFFFFFu);
	EXPECT_EQ(out[2][3], 0x00000000u);

	EXPECT_EQ(out[3][0], 0x00000001u);
	EXPECT_EQ(out[3][1], 0x00000001u);
	EXPECT_EQ(out[3][2], 0x00000000u);
	EXPECT_EQ(out[3][3], 0x00000000u);

	EXPECT_EQ(out[4][0], 0x00000000u);
	EXPECT_EQ(out[4][1], 0x00000000u);
	EXPECT_EQ(out[4][2], 0x00000000u);
	EXPECT_EQ(out[4][3], 0x00000000u);

	EXPECT_EQ(out[5][0], 0x00000001u);
	EXPECT_EQ(out[5][1], 0x00000001u);
	EXPECT_EQ(out[5][2], 0xFFFFFFFFu);
	EXPECT_EQ(out[5][3], 0x00000000u);

	EXPECT_EQ(out[6][0], 0x00000000u);
	EXPECT_EQ(out[6][1], 0x0000FFFFu);
	EXPECT_EQ(out[6][2], 0x00000000u);
	EXPECT_EQ(out[6][3], 0x00000000u);

	EXPECT_EQ(out[7][0], 0x00010001u);
	EXPECT_EQ(out[7][1], 0x00000000u);
	EXPECT_EQ(out[7][2], 0x00000000u);
	EXPECT_EQ(out[7][3], 0x00000000u);

	EXPECT_EQ(out[8][0], 0x00000000u);
	EXPECT_EQ(out[8][1], 0x00000000u);
	EXPECT_EQ(out[8][2], 0x00000000u);
	EXPECT_EQ(out[8][3], 0x00000000u);

	EXPECT_EQ(out[9][0], 0x00010001u);
	EXPECT_EQ(out[9][1], 0x0000FFFFu);
	EXPECT_EQ(out[9][2], 0x00000000u);
	EXPECT_EQ(out[9][3], 0x00000000u);
}

TEST(ReactorUnitTests, NotNeg)
{
	FunctionT<int(void *)> function;
	{
		Pointer<Byte> out = function.Arg<0>();

		*Pointer<Int>(out + 16 * 0) = ~Int(0x55555555);
		*Pointer<Short>(out + 16 * 1) = ~Short(0x5555);
		*Pointer<Int4>(out + 16 * 2) = ~Int4(0x55555555, 0xAAAAAAAA, 0x00000000, 0xFFFFFFFF);
		*Pointer<Short4>(out + 16 * 3) = ~Short4(0x5555, 0xAAAA, 0x0000, 0xFFFF);

		*Pointer<Int>(out + 16 * 4) = -Int(0x55555555);
		*Pointer<Short>(out + 16 * 5) = -Short(0x5555);
		*Pointer<Int4>(out + 16 * 6) = -Int4(0x55555555, 0xAAAAAAAA, 0x00000000, 0xFFFFFFFF);
		*Pointer<Short4>(out + 16 * 7) = -Short4(0x5555, 0xAAAA, 0x0000, 0xFFFF);

		*Pointer<Float4>(out + 16 * 8) = -Float4(1.0f, -1.0f, 0.0f, -0.0f);

		Return(0);
	}

	auto routine = function(testName().c_str());

	unsigned int out[10][4];

	memset(&out, 0, sizeof(out));

	routine(&out);

	EXPECT_EQ(out[0][0], 0xAAAAAAAAu);
	EXPECT_EQ(out[0][1], 0x00000000u);
	EXPECT_EQ(out[0][2], 0x00000000u);
	EXPECT_EQ(out[0][3], 0x00000000u);

	EXPECT_EQ(out[1][0], 0x0000AAAAu);
	EXPECT_EQ(out[1][1], 0x00000000u);
	EXPECT_EQ(out[1][2], 0x00000000u);
	EXPECT_EQ(out[1][3], 0x00000000u);

	EXPECT_EQ(out[2][0], 0xAAAAAAAAu);
	EXPECT_EQ(out[2][1], 0x55555555u);
	EXPECT_EQ(out[2][2], 0xFFFFFFFFu);
	EXPECT_EQ(out[2][3], 0x00000000u);

	EXPECT_EQ(out[3][0], 0x5555AAAAu);
	EXPECT_EQ(out[3][1], 0x0000FFFFu);
	EXPECT_EQ(out[3][2], 0x00000000u);
	EXPECT_EQ(out[3][3], 0x00000000u);

	EXPECT_EQ(out[4][0], 0xAAAAAAABu);
	EXPECT_EQ(out[4][1], 0x00000000u);
	EXPECT_EQ(out[4][2], 0x00000000u);
	EXPECT_EQ(out[4][3], 0x00000000u);

	EXPECT_EQ(out[5][0], 0x0000AAABu);
	EXPECT_EQ(out[5][1], 0x00000000u);
	EXPECT_EQ(out[5][2], 0x00000000u);
	EXPECT_EQ(out[5][3], 0x00000000u);

	EXPECT_EQ(out[6][0], 0xAAAAAAABu);
	EXPECT_EQ(out[6][1], 0x55555556u);
	EXPECT_EQ(out[6][2], 0x00000000u);
	EXPECT_EQ(out[6][3], 0x00000001u);

	EXPECT_EQ(out[7][0], 0x5556AAABu);
	EXPECT_EQ(out[7][1], 0x00010000u);
	EXPECT_EQ(out[7][2], 0x00000000u);
	EXPECT_EQ(out[7][3], 0x00000000u);

	EXPECT_EQ(out[8][0], 0xBF800000u);
	EXPECT_EQ(out[8][1], 0x3F800000u);
	EXPECT_EQ(out[8][2], 0x80000000u);
	EXPECT_EQ(out[8][3], 0x00000000u);
}

TEST(ReactorUnitTests, RoundInt)
{
	FunctionT<int(void *)> function;
	{
		Pointer<Byte> out = function.Arg<0>();

		*Pointer<Int4>(out + 0) = RoundInt(Float4(3.1f, 3.6f, -3.1f, -3.6f));
		*Pointer<Int4>(out + 16) = RoundIntClamped(Float4(2147483648.0f, -2147483648.0f, 2147483520, -2147483520));

		Return(0);
	}

	auto routine = function(testName().c_str());

	int out[2][4];

	memset(&out, 0, sizeof(out));

	routine(&out);

	EXPECT_EQ(out[0][0], 3);
	EXPECT_EQ(out[0][1], 4);
	EXPECT_EQ(out[0][2], -3);
	EXPECT_EQ(out[0][3], -4);

	// x86 returns 0x80000000 for values which cannot be represented in a 32-bit
	// integer, but RoundIntClamped() clamps to ensure a positive value for
	// positive input. ARM saturates to the largest representable integers.
	EXPECT_GE(out[1][0], 2147483520);
	EXPECT_LT(out[1][1], -2147483647);
	EXPECT_EQ(out[1][2], 2147483520);
	EXPECT_EQ(out[1][3], -2147483520);
}

TEST(ReactorUnitTests, FPtoUI)
{
	FunctionT<int(void *)> function;
	{
		Pointer<Byte> out = function.Arg<0>();

		*Pointer<UInt>(out + 0) = UInt(Float(0xF0000000u));
		*Pointer<UInt>(out + 4) = UInt(Float(0xC0000000u));
		*Pointer<UInt>(out + 8) = UInt(Float(0x00000001u));
		*Pointer<UInt>(out + 12) = UInt(Float(0xF000F000u));

		*Pointer<UInt4>(out + 16) = UInt4(Float4(0xF0000000u, 0x80000000u, 0x00000000u, 0xCCCC0000u));

		Return(0);
	}

	auto routine = function(testName().c_str());

	unsigned int out[2][4];

	memset(&out, 0, sizeof(out));

	routine(&out);

	EXPECT_EQ(out[0][0], 0xF0000000u);
	EXPECT_EQ(out[0][1], 0xC0000000u);
	EXPECT_EQ(out[0][2], 0x00000001u);
	EXPECT_EQ(out[0][3], 0xF000F000u);

	EXPECT_EQ(out[1][0], 0xF0000000u);
	EXPECT_EQ(out[1][1], 0x80000000u);
	EXPECT_EQ(out[1][2], 0x00000000u);
	EXPECT_EQ(out[1][3], 0xCCCC0000u);
}

TEST(ReactorUnitTests, VectorCompare)
{
	FunctionT<int(void *)> function;
	{
		Pointer<Byte> out = function.Arg<0>();

		*Pointer<Int4>(out + 16 * 0) = CmpEQ(Float4(1.0f, 1.0f, -0.0f, +0.0f), Float4(0.0f, 1.0f, +0.0f, -0.0f));
		*Pointer<Int4>(out + 16 * 1) = CmpEQ(Int4(1, 0, -1, -0), Int4(0, 1, 0, +0));
		*Pointer<Byte8>(out + 16 * 2) = CmpEQ(SByte8(1, 2, 3, 4, 5, 6, 7, 8), SByte8(7, 6, 5, 4, 3, 2, 1, 0));

		*Pointer<Int4>(out + 16 * 3) = CmpNLT(Float4(1.0f, 1.0f, -0.0f, +0.0f), Float4(0.0f, 1.0f, +0.0f, -0.0f));
		*Pointer<Int4>(out + 16 * 4) = CmpNLT(Int4(1, 0, -1, -0), Int4(0, 1, 0, +0));
		*Pointer<Byte8>(out + 16 * 5) = CmpGT(SByte8(1, 2, 3, 4, 5, 6, 7, 8), SByte8(7, 6, 5, 4, 3, 2, 1, 0));

		Return(0);
	}

	auto routine = function(testName().c_str());

	unsigned int out[6][4];

	memset(&out, 0, sizeof(out));

	routine(&out);

	EXPECT_EQ(out[0][0], 0x00000000u);
	EXPECT_EQ(out[0][1], 0xFFFFFFFFu);
	EXPECT_EQ(out[0][2], 0xFFFFFFFFu);
	EXPECT_EQ(out[0][3], 0xFFFFFFFFu);

	EXPECT_EQ(out[1][0], 0x00000000u);
	EXPECT_EQ(out[1][1], 0x00000000u);
	EXPECT_EQ(out[1][2], 0x00000000u);
	EXPECT_EQ(out[1][3], 0xFFFFFFFFu);

	EXPECT_EQ(out[2][0], 0xFF000000u);
	EXPECT_EQ(out[2][1], 0x00000000u);

	EXPECT_EQ(out[3][0], 0xFFFFFFFFu);
	EXPECT_EQ(out[3][1], 0xFFFFFFFFu);
	EXPECT_EQ(out[3][2], 0xFFFFFFFFu);
	EXPECT_EQ(out[3][3], 0xFFFFFFFFu);

	EXPECT_EQ(out[4][0], 0xFFFFFFFFu);
	EXPECT_EQ(out[4][1], 0x00000000u);
	EXPECT_EQ(out[4][2], 0x00000000u);
	EXPECT_EQ(out[4][3], 0xFFFFFFFFu);

	EXPECT_EQ(out[5][0], 0x00000000u);
	EXPECT_EQ(out[5][1], 0xFFFFFFFFu);
}

TEST(ReactorUnitTests, SaturatedAddAndSubtract)
{
	FunctionT<int(void *)> function;
	{
		Pointer<Byte> out = function.Arg<0>();

		*Pointer<Byte8>(out + 8 * 0) =
		    AddSat(Byte8(1, 2, 3, 4, 5, 6, 7, 8),
		           Byte8(7, 6, 5, 4, 3, 2, 1, 0));
		*Pointer<Byte8>(out + 8 * 1) =
		    AddSat(Byte8(0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE),
		           Byte8(7, 6, 5, 4, 3, 2, 1, 0));
		*Pointer<Byte8>(out + 8 * 2) =
		    SubSat(Byte8(1, 2, 3, 4, 5, 6, 7, 8),
		           Byte8(7, 6, 5, 4, 3, 2, 1, 0));

		*Pointer<SByte8>(out + 8 * 3) =
		    AddSat(SByte8(1, 2, 3, 4, 5, 6, 7, 8),
		           SByte8(7, 6, 5, 4, 3, 2, 1, 0));
		*Pointer<SByte8>(out + 8 * 4) =
		    AddSat(SByte8(0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E, 0x7E),
		           SByte8(7, 6, 5, 4, 3, 2, 1, 0));
		*Pointer<SByte8>(out + 8 * 5) =
		    AddSat(SByte8(0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88),
		           SByte8(-7, -6, -5, -4, -3, -2, -1, -0));
		*Pointer<SByte8>(out + 8 * 6) =
		    SubSat(SByte8(0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88),
		           SByte8(7, 6, 5, 4, 3, 2, 1, 0));

		*Pointer<Short4>(out + 8 * 7) =
		    AddSat(Short4(1, 2, 3, 4), Short4(3, 2, 1, 0));
		*Pointer<Short4>(out + 8 * 8) =
		    AddSat(Short4(0x7FFE, 0x7FFE, 0x7FFE, 0x7FFE),
		           Short4(3, 2, 1, 0));
		*Pointer<Short4>(out + 8 * 9) =
		    AddSat(Short4(0x8001, 0x8002, 0x8003, 0x8004),
		           Short4(-3, -2, -1, -0));
		*Pointer<Short4>(out + 8 * 10) =
		    SubSat(Short4(0x8001, 0x8002, 0x8003, 0x8004),
		           Short4(3, 2, 1, 0));

		*Pointer<UShort4>(out + 8 * 11) =
		    AddSat(UShort4(1, 2, 3, 4), UShort4(3, 2, 1, 0));
		*Pointer<UShort4>(out + 8 * 12) =
		    AddSat(UShort4(0xFFFE, 0xFFFE, 0xFFFE, 0xFFFE),
		           UShort4(3, 2, 1, 0));
		*Pointer<UShort4>(out + 8 * 13) =
		    SubSat(UShort4(1, 2, 3, 4), UShort4(3, 2, 1, 0));

		Return(0);
	}

	auto routine = function(testName().c_str());

	unsigned int out[14][2];

	memset(&out, 0, sizeof(out));

	routine(&out);

	EXPECT_EQ(out[0][0], 0x08080808u);
	EXPECT_EQ(out[0][1], 0x08080808u);

	EXPECT_EQ(out[1][0], 0xFFFFFFFFu);
	EXPECT_EQ(out[1][1], 0xFEFFFFFFu);

	EXPECT_EQ(out[2][0], 0x00000000u);
	EXPECT_EQ(out[2][1], 0x08060402u);

	EXPECT_EQ(out[3][0], 0x08080808u);
	EXPECT_EQ(out[3][1], 0x08080808u);

	EXPECT_EQ(out[4][0], 0x7F7F7F7Fu);
	EXPECT_EQ(out[4][1], 0x7E7F7F7Fu);

	EXPECT_EQ(out[5][0], 0x80808080u);
	EXPECT_EQ(out[5][1], 0x88868482u);

	EXPECT_EQ(out[6][0], 0x80808080u);
	EXPECT_EQ(out[6][1], 0x88868482u);

	EXPECT_EQ(out[7][0], 0x00040004u);
	EXPECT_EQ(out[7][1], 0x00040004u);

	EXPECT_EQ(out[8][0], 0x7FFF7FFFu);
	EXPECT_EQ(out[8][1], 0x7FFE7FFFu);

	EXPECT_EQ(out[9][0], 0x80008000u);
	EXPECT_EQ(out[9][1], 0x80048002u);

	EXPECT_EQ(out[10][0], 0x80008000u);
	EXPECT_EQ(out[10][1], 0x80048002u);

	EXPECT_EQ(out[11][0], 0x00040004u);
	EXPECT_EQ(out[11][1], 0x00040004u);

	EXPECT_EQ(out[12][0], 0xFFFFFFFFu);
	EXPECT_EQ(out[12][1], 0xFFFEFFFFu);

	EXPECT_EQ(out[13][0], 0x00000000u);
	EXPECT_EQ(out[13][1], 0x00040002u);
}

TEST(ReactorUnitTests, Unpack)
{
	FunctionT<int(void *, void *)> function;
	{
		Pointer<Byte> in = function.Arg<0>();
		Pointer<Byte> out = function.Arg<1>();

		Byte4 test_byte_a = *Pointer<Byte4>(in + 4 * 0);
		Byte4 test_byte_b = *Pointer<Byte4>(in + 4 * 1);

		*Pointer<Short4>(out + 8 * 0) =
		    Unpack(test_byte_a, test_byte_b);

		*Pointer<Short4>(out + 8 * 1) = Unpack(test_byte_a);

		Return(0);
	}

	auto routine = function(testName().c_str());

	unsigned int in[1][2];
	unsigned int out[2][2];

	memset(&out, 0, sizeof(out));

	in[0][0] = 0xABCDEF12u;
	in[0][1] = 0x34567890u;

	routine(&in, &out);

	EXPECT_EQ(out[0][0], 0x78EF9012u);
	EXPECT_EQ(out[0][1], 0x34AB56CDu);

	EXPECT_EQ(out[1][0], 0xEFEF1212u);
	EXPECT_EQ(out[1][1], 0xABABCDCDu);
}

TEST(ReactorUnitTests, Pack)
{
	FunctionT<int(void *)> function;
	{
		Pointer<Byte> out = function.Arg<0>();

		*Pointer<SByte8>(out + 8 * 0) =
		    PackSigned(Short4(-1, -2, 1, 2),
		               Short4(3, 4, -3, -4));

		*Pointer<Byte8>(out + 8 * 1) =
		    PackUnsigned(Short4(-1, -2, 1, 2),
		                 Short4(3, 4, -3, -4));

		*Pointer<Short8>(out + 8 * 2) =
		    PackSigned(Int4(-1, -2, 1, 2),
		               Int4(3, 4, -3, -4));

		*Pointer<UShort8>(out + 8 * 4) =
		    PackUnsigned(Int4(-1, -2, 1, 2),
		                 Int4(3, 4, -3, -4));

		Return(0);
	}

	auto routine = function(testName().c_str());

	unsigned int out[6][2];

	memset(&out, 0, sizeof(out));

	routine(&out);

	EXPECT_EQ(out[0][0], 0x0201FEFFu);
	EXPECT_EQ(out[0][1], 0xFCFD0403u);

	EXPECT_EQ(out[1][0], 0x02010000u);
	EXPECT_EQ(out[1][1], 0x00000403u);

	EXPECT_EQ(out[2][0], 0xFFFEFFFFu);
	EXPECT_EQ(out[2][1], 0x00020001u);

	EXPECT_EQ(out[3][0], 0x00040003u);
	EXPECT_EQ(out[3][1], 0xFFFCFFFDu);

	EXPECT_EQ(out[4][0], 0x00000000u);
	EXPECT_EQ(out[4][1], 0x00020001u);

	EXPECT_EQ(out[5][0], 0x00040003u);
	EXPECT_EQ(out[5][1], 0x00000000u);
}

TEST(ReactorUnitTests, MulHigh)
{
	FunctionT<int(void *)> function;
	{
		Pointer<Byte> out = function.Arg<0>();

		*Pointer<Short4>(out + 16 * 0) =
		    MulHigh(Short4(0x01AA, 0x02DD, 0x03EE, 0xF422),
		            Short4(0x01BB, 0x02CC, 0x03FF, 0xF411));
		*Pointer<UShort4>(out + 16 * 1) =
		    MulHigh(UShort4(0x01AA, 0x02DD, 0x03EE, 0xF422),
		            UShort4(0x01BB, 0x02CC, 0x03FF, 0xF411));

		*Pointer<Int4>(out + 16 * 2) =
		    MulHigh(Int4(0x000001AA, 0x000002DD, 0xC8000000, 0xF8000000),
		            Int4(0x000001BB, 0x84000000, 0x000003EE, 0xD7000000));
		*Pointer<UInt4>(out + 16 * 3) =
		    MulHigh(UInt4(0x000001AAu, 0x000002DDu, 0xC8000000u, 0xD8000000u),
		            UInt4(0x000001BBu, 0x84000000u, 0x000003EEu, 0xD7000000u));

		*Pointer<Int4>(out + 16 * 4) =
		    MulHigh(Int4(0x7FFFFFFF, 0x7FFFFFFF, 0x80008000, 0xFFFFFFFF),
		            Int4(0x7FFFFFFF, 0x80000000, 0x80008000, 0xFFFFFFFF));
		*Pointer<UInt4>(out + 16 * 5) =
		    MulHigh(UInt4(0x7FFFFFFFu, 0x7FFFFFFFu, 0x80008000u, 0xFFFFFFFFu),
		            UInt4(0x7FFFFFFFu, 0x80000000u, 0x80008000u, 0xFFFFFFFFu));

		// (U)Short8 variants currently unimplemented.

		Return(0);
	}

	auto routine = function(testName().c_str());

	unsigned int out[6][4];

	memset(&out, 0, sizeof(out));

	routine(&out);

	EXPECT_EQ(out[0][0], 0x00080002u);
	EXPECT_EQ(out[0][1], 0x008D000Fu);

	EXPECT_EQ(out[1][0], 0x00080002u);
	EXPECT_EQ(out[1][1], 0xE8C0000Fu);

	EXPECT_EQ(out[2][0], 0x00000000u);
	EXPECT_EQ(out[2][1], 0xFFFFFE9Cu);
	EXPECT_EQ(out[2][2], 0xFFFFFF23u);
	EXPECT_EQ(out[2][3], 0x01480000u);

	EXPECT_EQ(out[3][0], 0x00000000u);
	EXPECT_EQ(out[3][1], 0x00000179u);
	EXPECT_EQ(out[3][2], 0x00000311u);
	EXPECT_EQ(out[3][3], 0xB5680000u);

	EXPECT_EQ(out[4][0], 0x3FFFFFFFu);
	EXPECT_EQ(out[4][1], 0xC0000000u);
	EXPECT_EQ(out[4][2], 0x3FFF8000u);
	EXPECT_EQ(out[4][3], 0x00000000u);

	EXPECT_EQ(out[5][0], 0x3FFFFFFFu);
	EXPECT_EQ(out[5][1], 0x3FFFFFFFu);
	EXPECT_EQ(out[5][2], 0x40008000u);
	EXPECT_EQ(out[5][3], 0xFFFFFFFEu);
}

TEST(ReactorUnitTests, MulAdd)
{
	FunctionT<int(void *)> function;
	{
		Pointer<Byte> out = function.Arg<0>();

		*Pointer<Int2>(out + 8 * 0) =
		    MulAdd(Short4(0x1aa, 0x2dd, 0x3ee, 0xF422),
		           Short4(0x1bb, 0x2cc, 0x3ff, 0xF411));

		// (U)Short8 variant is mentioned but unimplemented
		Return(0);
	}

	auto routine = function(testName().c_str());

	unsigned int out[1][2];

	memset(&out, 0, sizeof(out));

	routine(&out);

	EXPECT_EQ(out[0][0], 0x000AE34Au);
	EXPECT_EQ(out[0][1], 0x009D5254u);
}

TEST(ReactorUnitTests, PointersEqual)
{
	FunctionT<int(void *, void *)> function;
	{
		Pointer<Byte> ptrA = function.Arg<0>();
		Pointer<Byte> ptrB = function.Arg<1>();
		If(ptrA == ptrB)
		{
			Return(1);
		}
		Else
		{
			Return(0);
		}
	}

	auto routine = function(testName().c_str());
	int *a = reinterpret_cast<int *>(uintptr_t(0x0000000000000000));
	int *b = reinterpret_cast<int *>(uintptr_t(0x00000000F0000000));
	int *c = reinterpret_cast<int *>(uintptr_t(0xF000000000000000));
	EXPECT_EQ(routine(&a, &a), 1);
	EXPECT_EQ(routine(&b, &b), 1);
	EXPECT_EQ(routine(&c, &c), 1);

	EXPECT_EQ(routine(&a, &b), 0);
	EXPECT_EQ(routine(&b, &a), 0);
	EXPECT_EQ(routine(&b, &c), 0);
	EXPECT_EQ(routine(&c, &b), 0);
	EXPECT_EQ(routine(&c, &a), 0);
	EXPECT_EQ(routine(&a, &c), 0);
}

TEST(ReactorUnitTests, Args_2Mixed)
{
	// 2 mixed type args
	FunctionT<float(int, float)> function;
	{
		Int a = function.Arg<0>();
		Float b = function.Arg<1>();
		Return(Float(a) + b);
	}

	if(auto routine = function(testName().c_str()))
	{
		float result = routine(1, 2.f);
		EXPECT_EQ(result, 3.f);
	}
}

TEST(ReactorUnitTests, Args_4Mixed)
{
	// 4 mixed type args (max register allocation on Windows)
	FunctionT<float(int, float, int, float)> function;
	{
		Int a = function.Arg<0>();
		Float b = function.Arg<1>();
		Int c = function.Arg<2>();
		Float d = function.Arg<3>();
		Return(Float(a) + b + Float(c) + d);
	}

	if(auto routine = function(testName().c_str()))
	{
		float result = routine(1, 2.f, 3, 4.f);
		EXPECT_EQ(result, 10.f);
	}
}

TEST(ReactorUnitTests, Args_5Mixed)
{
	// 5 mixed type args (5th spills over to stack on Windows)
	FunctionT<float(int, float, int, float, int)> function;
	{
		Int a = function.Arg<0>();
		Float b = function.Arg<1>();
		Int c = function.Arg<2>();
		Float d = function.Arg<3>();
		Int e = function.Arg<4>();
		Return(Float(a) + b + Float(c) + d + Float(e));
	}

	if(auto routine = function(testName().c_str()))
	{
		float result = routine(1, 2.f, 3, 4.f, 5);
		EXPECT_EQ(result, 15.f);
	}
}

TEST(ReactorUnitTests, Args_GreaterThan5Mixed)
{
	// >5 mixed type args
	FunctionT<float(int, float, int, float, int, float, int, float, int, float)> function;
	{
		Int a = function.Arg<0>();
		Float b = function.Arg<1>();
		Int c = function.Arg<2>();
		Float d = function.Arg<3>();
		Int e = function.Arg<4>();
		Float f = function.Arg<5>();
		Int g = function.Arg<6>();
		Float h = function.Arg<7>();
		Int i = function.Arg<8>();
		Float j = function.Arg<9>();
		Return(Float(a) + b + Float(c) + d + Float(e) + f + Float(g) + h + Float(i) + j);
	}

	if(auto routine = function(testName().c_str()))
	{
		float result = routine(1, 2.f, 3, 4.f, 5, 6.f, 7, 8.f, 9, 10.f);
		EXPECT_EQ(result, 55.f);
	}
}

// This test was written because on Windows with Subzero, we would get a crash when executing a function
// with a large number of local variables. The problem was that on Windows, 4K pages are allocated as
// needed for the stack whenever an access is made in a "guard page", at which point the page is committed,
// and the next 4K page becomes the guard page. If a stack access is made that's beyond the guard page,
// a regular page fault occurs. To fix this, Subzero (and any compiler) now emits a call to __chkstk with
// the stack size in EAX, so that it can probe the stack in 4K increments up to that size, committing the
// required pages. See https://docs.microsoft.com/en-us/windows/win32/devnotes/-win32-chkstk.
TEST(ReactorUnitTests, LargeStack)
{
	// An empirically large enough value to access outside the guard pages
	constexpr int ArrayByteSize = 24 * 1024;
	constexpr int ArraySize = ArrayByteSize / sizeof(int32_t);

	FunctionT<void(int32_t * v)> function;
	{
		// Allocate a stack array large enough that writing to the first element will reach beyond
		// the guard page.
		Array<Int, ArraySize> largeStackArray;
		for(int i = 0; i < ArraySize; ++i)
		{
			largeStackArray[i] = i;
		}

		Pointer<Int> in = function.Arg<0>();
		for(int i = 0; i < ArraySize; ++i)
		{
			in[i] = largeStackArray[i];
		}
	}

	// LLVM takes very long to generate this routine when O2 optimizations are enabled. Disable for now.
	// TODO(b/174031014): Remove this once we fix LLVM taking so long.
	ScopedPragma O0(OptimizationLevel, 0);

	auto routine = function(testName().c_str());

	std::array<int32_t, ArraySize> v;

	// Run this in a thread, so that we get the default reserved stack size (8K on Win64).
	std::thread t([&] {
		routine(v.data());
	});
	t.join();

	for(int i = 0; i < ArraySize; ++i)
	{
		EXPECT_EQ(v[i], i);
	}
}

TEST(ReactorUnitTests, ShlSmallRHSScalar)
{
	// TODO(crbug.com/swiftshader/185): Testing a temporary LLVM workaround
	if(Caps::backendName().find("LLVM") == std::string::npos) return;

	FunctionT<unsigned()> function;
	{
		auto lhs = UInt(4);
		auto rhs = UInt(8);
		auto res = lhs << rhs;
		Return(res);
	}

	auto routine = function(testName().c_str());

	unsigned res = routine();
	EXPECT_EQ(res, 1u << 10u);
}

TEST(ReactorUnitTests, ShlLargeRHSScalar)
{
	// TODO(crbug.com/swiftshader/185): Testing a temporary LLVM workaround
	if(Caps::backendName().find("LLVM") == std::string::npos) return;

	FunctionT<unsigned()> function;
	{
		auto lhs = UInt(1);
		auto rhs = UInt(99);
		auto res = lhs << rhs;
		Return(res);
	}

	auto routine = function(testName().c_str());

	unsigned res = routine();
	EXPECT_EQ(res, 1u << 31u);
}

TEST(ReactorUnitTests, ShrSmallRHSScalar)
{
	// TODO(crbug.com/swiftshader/185): Testing a temporary LLVM workaround
	if(Caps::backendName().find("LLVM") == std::string::npos) return;

	FunctionT<unsigned()> function;
	{
		auto lhs = UInt(64);
		auto rhs = UInt(4);
		auto res = lhs >> rhs;
		Return(res);
	}

	auto routine = function(testName().c_str());

	unsigned res = routine();
	EXPECT_EQ(res, 4u);
}

TEST(ReactorUnitTests, ShrLargeRHSScalar)
{
	// TODO(crbug.com/swiftshader/185): Testing a temporary LLVM workaround
	if(Caps::backendName().find("LLVM") == std::string::npos) return;

	FunctionT<unsigned()> function;
	{
		auto lhs = UInt(4);
		auto rhs = UInt(99);
		auto res = lhs >> rhs;
		Return(res);
	}

	auto routine = function(testName().c_str());

	unsigned res = routine();
	EXPECT_EQ(res, 0u);
}

TEST(ReactorUnitTests, ShlRHSVector)
{
	// TODO(crbug.com/swiftshader/185): Testing a temporary LLVM workaround
	if(Caps::backendName().find("LLVM") == std::string::npos) return;

	FunctionT<void(unsigned *a, unsigned *b, unsigned *c, unsigned *d)> function;
	{
		Pointer<UInt> a = function.Arg<0>();
		Pointer<UInt> b = function.Arg<1>();
		Pointer<UInt> c = function.Arg<2>();
		Pointer<UInt> d = function.Arg<3>();

		auto lhs = UInt4(4, 3, 6, 5);
		auto rhs = UInt4(8, 99, 2, 50);
		UInt4 res = lhs << rhs;
		*a = res.x;
		*b = res.y;
		*c = res.z;
		*d = res.w;
	}

	auto routine = function(testName().c_str());

	unsigned a = 0;
	unsigned b = 0;
	unsigned c = 0;
	unsigned d = 0;
	routine(&a, &b, &c, &d);
	EXPECT_EQ(a, 1024u);
	EXPECT_EQ(b, 0x80000000u);
	EXPECT_EQ(c, 24u);
	EXPECT_EQ(d, 0x80000000u);
}

TEST(ReactorUnitTests, ShrRHSVector)
{
	// TODO(crbug.com/swiftshader/185): Testing a temporary LLVM workaround
	if(Caps::backendName().find("LLVM") == std::string::npos) return;

	FunctionT<void(unsigned *a, unsigned *b, unsigned *c, unsigned *d)> function;
	{
		Pointer<UInt> a = function.Arg<0>();
		Pointer<UInt> b = function.Arg<1>();
		Pointer<UInt> c = function.Arg<2>();
		Pointer<UInt> d = function.Arg<3>();

		auto lhs = UInt4(745, 23, 234, 54);
		auto rhs = UInt4(8, 99, 2, 50);
		UInt4 res = lhs >> rhs;
		*a = res.x;
		*b = res.y;
		*c = res.z;
		*d = res.w;
	}

	auto routine = function(testName().c_str());

	unsigned a = 0;
	unsigned b = 0;
	unsigned c = 0;
	unsigned d = 0;
	routine(&a, &b, &c, &d);
	EXPECT_EQ(a, 2u);
	EXPECT_EQ(b, 0u);
	EXPECT_EQ(c, 58u);
	EXPECT_EQ(d, 0u);
}

TEST(ReactorUnitTests, ShrLargeRHSVector)
{
	// TODO(crbug.com/swiftshader/185): Testing a temporary LLVM workaround
	if(Caps::backendName().find("LLVM") == std::string::npos) return;

	FunctionT<unsigned()> function;
	{
		auto lhs = UInt(4);
		auto rhs = UInt(99);
		auto res = lhs >> rhs;
		Return(res);
	}

	auto routine = function(testName().c_str());

	unsigned res = routine();
	EXPECT_EQ(res, 0u);
}

TEST(ReactorUnitTests, Call)
{
	struct Class
	{
		static int Callback(Class *p, int i, float f)
		{
			p->i = i;
			p->f = f;
			return i + int(f);
		}

		int i = 0;
		float f = 0.0f;
	};

	FunctionT<int(void *)> function;
	{
		Pointer<Byte> c = function.Arg<0>();
		auto res = Call(Class::Callback, c, 10, 20.0f);
		Return(res);
	}

	auto routine = function(testName().c_str());

	Class c;
	int res = routine(&c);
	EXPECT_EQ(res, 30);
	EXPECT_EQ(c.i, 10);
	EXPECT_EQ(c.f, 20.0f);
}

TEST(ReactorUnitTests, CallMemberFunction)
{
	struct Class
	{
		int Callback(int argI, float argF)
		{
			i = argI;
			f = argF;
			return i + int(f);
		}

		int i = 0;
		float f = 0.0f;
	};

	Class c;

	FunctionT<int()> function;
	{
		auto res = Call(&Class::Callback, &c, 10, 20.0f);
		Return(res);
	}

	auto routine = function(testName().c_str());

	int res = routine();
	EXPECT_EQ(res, 30);
	EXPECT_EQ(c.i, 10);
	EXPECT_EQ(c.f, 20.0f);
}

TEST(ReactorUnitTests, CallMemberFunctionIndirect)
{
	struct Class
	{
		int Callback(int argI, float argF)
		{
			i = argI;
			f = argF;
			return i + int(f);
		}

		int i = 0;
		float f = 0.0f;
	};

	FunctionT<int(void *)> function;
	{
		Pointer<Byte> c = function.Arg<0>();
		auto res = Call(&Class::Callback, c, 10, 20.0f);
		Return(res);
	}

	auto routine = function(testName().c_str());

	Class c;
	int res = routine(&c);
	EXPECT_EQ(res, 30);
	EXPECT_EQ(c.i, 10);
	EXPECT_EQ(c.f, 20.0f);
}

TEST(ReactorUnitTests, CallImplicitCast)
{
	struct Class
	{
		static void Callback(Class *c, const char *s)
		{
			c->str = s;
		}
		std::string str;
	};

	FunctionT<void(Class * c, const char *s)> function;
	{
		Pointer<Byte> c = function.Arg<0>();
		Pointer<Byte> s = function.Arg<1>();
		Call(Class::Callback, c, s);
	}

	auto routine = function(testName().c_str());

	Class c;
	routine(&c, "hello world");
	EXPECT_EQ(c.str, "hello world");
}

TEST(ReactorUnitTests, CallBoolReturnFunction)
{
	struct Class
	{
		static bool IsEven(int a)
		{
			return a % 2 == 0;
		}
	};

	FunctionT<int(int)> function;
	{
		Int a = function.Arg<0>();
		Bool res = Call(Class::IsEven, a);
		If(res)
		{
			Return(1);
		}
		Return(0);
	}

	auto routine = function(testName().c_str());

	for(int i = 0; i < 10; ++i)
	{
		EXPECT_EQ(routine(i), i % 2 == 0);
	}
}

TEST(ReactorUnitTests, Call_Args4)
{
	struct Class
	{
		static int Func(int a, int b, int c, int d)
		{
			return a + b + c + d;
		}
	};

	{
		FunctionT<int()> function;
		{
			auto res = Call(Class::Func, 1, 2, 3, 4);
			Return(res);
		}

		auto routine = function(testName().c_str());

		int res = routine();
		EXPECT_EQ(res, 1 + 2 + 3 + 4);
	}
}

TEST(ReactorUnitTests, Call_Args5)
{
	struct Class
	{
		static int Func(int a, int b, int c, int d, int e)
		{
			return a + b + c + d + e;
		}
	};

	{
		FunctionT<int()> function;
		{
			auto res = Call(Class::Func, 1, 2, 3, 4, 5);
			Return(res);
		}

		auto routine = function(testName().c_str());

		int res = routine();
		EXPECT_EQ(res, 1 + 2 + 3 + 4 + 5);
	}
}

TEST(ReactorUnitTests, Call_ArgsMany)
{
	struct Class
	{
		static int Func(int a, int b, int c, int d, int e, int f, int g, int h)
		{
			return a + b + c + d + e + f + g + h;
		}
	};

	{
		FunctionT<int()> function;
		{
			auto res = Call(Class::Func, 1, 2, 3, 4, 5, 6, 7, 8);
			Return(res);
		}

		auto routine = function(testName().c_str());

		int res = routine();
		EXPECT_EQ(res, 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8);
	}
}

TEST(ReactorUnitTests, Call_ArgsMixed)
{
	struct Class
	{
		static int Func(int a, float b, int *c, float *d, int e, float f, int *g, float *h)
		{
			return a + b + *c + *d + e + f + *g + *h;
		}
	};

	{
		FunctionT<int()> function;
		{
			Int c(3);
			Float d(4);
			Int g(7);
			Float h(8);
			auto res = Call(Class::Func, 1, 2.f, &c, &d, 5, 6.f, &g, &h);
			Return(res);
		}

		auto routine = function(testName().c_str());

		int res = routine();
		EXPECT_EQ(res, 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8);
	}
}

TEST(ReactorUnitTests, Call_ArgsPointer)
{
	struct Class
	{
		static int Func(int *a)
		{
			return *a;
		}
	};

	{
		FunctionT<int()> function;
		{
			Int a(12345);
			auto res = Call(Class::Func, &a);
			Return(res);
		}

		auto routine = function(testName().c_str());

		int res = routine();
		EXPECT_EQ(res, 12345);
	}
}

TEST(ReactorUnitTests, CallExternalCallRoutine)
{
	// routine1 calls Class::Func, passing it a pointer to routine2, and Class::Func calls routine2

	auto routine2 = [] {
		FunctionT<float(float, int)> function;
		{
			Float a = function.Arg<0>();
			Int b = function.Arg<1>();
			Return(a + Float(b));
		}
		return function("%s2", testName().c_str());
	}();

	struct Class
	{
		static float Func(void *p, float a, int b)
		{
			auto funcToCall = reinterpret_cast<float (*)(float, int)>(p);
			return funcToCall(a, b);
		}
	};

	auto routine1 = [] {
		FunctionT<float(void *, float, int)> function;
		{
			Pointer<Byte> funcToCall = function.Arg<0>();
			Float a = function.Arg<1>();
			Int b = function.Arg<2>();
			Float result = Call(Class::Func, funcToCall, a, b);
			Return(result);
		}
		return function(testName().c_str());
	}();

	float result = routine1((void *)routine2.getEntry(), 12.f, 13);
	EXPECT_EQ(result, 25.f);
}

// Check that a complex generated function which utilizes all 8 or 16 XMM
// registers computes the correct result.
// (Note that due to MSC's lack of support for inline assembly in x64,
// this test does not actually check that the register contents are
// preserved, just that the generated function computes the correct value.
// It's necessary to inspect the registers in a debugger to actually verify.)
TEST(ReactorUnitTests, PreserveXMMRegisters)
{
	FunctionT<void(void *, void *)> function;
	{
		Pointer<Byte> in = function.Arg<0>();
		Pointer<Byte> out = function.Arg<1>();

		Float4 a = *Pointer<Float4>(in + 16 * 0);
		Float4 b = *Pointer<Float4>(in + 16 * 1);
		Float4 c = *Pointer<Float4>(in + 16 * 2);
		Float4 d = *Pointer<Float4>(in + 16 * 3);
		Float4 e = *Pointer<Float4>(in + 16 * 4);
		Float4 f = *Pointer<Float4>(in + 16 * 5);
		Float4 g = *Pointer<Float4>(in + 16 * 6);
		Float4 h = *Pointer<Float4>(in + 16 * 7);
		Float4 i = *Pointer<Float4>(in + 16 * 8);
		Float4 j = *Pointer<Float4>(in + 16 * 9);
		Float4 k = *Pointer<Float4>(in + 16 * 10);
		Float4 l = *Pointer<Float4>(in + 16 * 11);
		Float4 m = *Pointer<Float4>(in + 16 * 12);
		Float4 n = *Pointer<Float4>(in + 16 * 13);
		Float4 o = *Pointer<Float4>(in + 16 * 14);
		Float4 p = *Pointer<Float4>(in + 16 * 15);

		Float4 ab = a + b;
		Float4 cd = c + d;
		Float4 ef = e + f;
		Float4 gh = g + h;
		Float4 ij = i + j;
		Float4 kl = k + l;
		Float4 mn = m + n;
		Float4 op = o + p;

		Float4 abcd = ab + cd;
		Float4 efgh = ef + gh;
		Float4 ijkl = ij + kl;
		Float4 mnop = mn + op;

		Float4 abcdefgh = abcd + efgh;
		Float4 ijklmnop = ijkl + mnop;
		Float4 sum = abcdefgh + ijklmnop;
		*Pointer<Float4>(out) = sum;
		Return();
	}

	auto routine = function(testName().c_str());
	assert(routine);

	float input[64] = { 1.0f, 0.0f, 0.0f, 0.0f,
		                -1.0f, 1.0f, -1.0f, 0.0f,
		                1.0f, 2.0f, -2.0f, 0.0f,
		                -1.0f, 3.0f, -3.0f, 0.0f,
		                1.0f, 4.0f, -4.0f, 0.0f,
		                -1.0f, 5.0f, -5.0f, 0.0f,
		                1.0f, 6.0f, -6.0f, 0.0f,
		                -1.0f, 7.0f, -7.0f, 0.0f,
		                1.0f, 8.0f, -8.0f, 0.0f,
		                -1.0f, 9.0f, -9.0f, 0.0f,
		                1.0f, 10.0f, -10.0f, 0.0f,
		                -1.0f, 11.0f, -11.0f, 0.0f,
		                1.0f, 12.0f, -12.0f, 0.0f,
		                -1.0f, 13.0f, -13.0f, 0.0f,
		                1.0f, 14.0f, -14.0f, 0.0f,
		                -1.0f, 15.0f, -15.0f, 0.0f };

	float result[4];

	routine(input, result);

	EXPECT_EQ(result[0], 0.0f);
	EXPECT_EQ(result[1], 120.0f);
	EXPECT_EQ(result[2], -120.0f);
	EXPECT_EQ(result[3], 0.0f);
}

template<typename T>
class CToReactorTCastTest : public ::testing::Test
{
public:
	using CType = typename std::tuple_element<0, T>::type;
	using ReactorType = typename std::tuple_element<1, T>::type;
};

using CToReactorTCastTestTypes = ::testing::Types<  // Subset of types that can be used as arguments.
                                                    //	std::pair<bool,         Bool>,    FIXME(capn): Not supported as argument type by Subzero.
                                                    //	std::pair<uint8_t,      Byte>,    FIXME(capn): Not supported as argument type by Subzero.
                                                    //	std::pair<int8_t,       SByte>,   FIXME(capn): Not supported as argument type by Subzero.
                                                    //	std::pair<int16_t,      Short>,   FIXME(capn): Not supported as argument type by Subzero.
                                                    //	std::pair<uint16_t,     UShort>,  FIXME(capn): Not supported as argument type by Subzero.
    std::pair<int, Int>,
    std::pair<unsigned int, UInt>,
    std::pair<float, Float>>;

TYPED_TEST_SUITE(CToReactorTCastTest, CToReactorTCastTestTypes);

TYPED_TEST(CToReactorTCastTest, Casts)
{
	using CType = typename TestFixture::CType;
	using ReactorType = typename TestFixture::ReactorType;

	std::shared_ptr<Routine> routine;

	{
		Function<Int(ReactorType)> function;
		{
			ReactorType a = function.template Arg<0>();
			ReactorType b = CType{};
			RValue<ReactorType> c = RValue<ReactorType>(CType{});
			Bool same = (a == b) && (a == c);
			Return(IfThenElse(same, Int(1), Int(0)));  // TODO: Ability to use Bools as return values.
		}

		routine = function(testName().c_str());

		auto callable = (int (*)(CType))routine->getEntry();
		CType in = {};
		EXPECT_EQ(callable(in), 1);
	}
}

template<typename T>
class GEPTest : public ::testing::Test
{
public:
	using CType = typename std::tuple_element<0, T>::type;
	using ReactorType = typename std::tuple_element<1, T>::type;
};

using GEPTestTypes = ::testing::Types<
    std::pair<bool, Bool>,
    std::pair<int8_t, Byte>,
    std::pair<int8_t, SByte>,
    std::pair<int8_t[4], Byte4>,
    std::pair<int8_t[4], SByte4>,
    std::pair<int8_t[8], Byte8>,
    std::pair<int8_t[8], SByte8>,
    std::pair<int8_t[16], Byte16>,
    std::pair<int8_t[16], SByte16>,
    std::pair<int16_t, Short>,
    std::pair<int16_t, UShort>,
    std::pair<int16_t[2], Short2>,
    std::pair<int16_t[2], UShort2>,
    std::pair<int16_t[4], Short4>,
    std::pair<int16_t[4], UShort4>,
    std::pair<int16_t[8], Short8>,
    std::pair<int16_t[8], UShort8>,
    std::pair<int, Int>,
    std::pair<int, UInt>,
    std::pair<int[2], Int2>,
    std::pair<int[2], UInt2>,
    std::pair<int[4], Int4>,
    std::pair<int[4], UInt4>,
    std::pair<int64_t, Long>,
    std::pair<int16_t, Half>,
    std::pair<float, Float>,
    std::pair<float[2], Float2>,
    std::pair<float[4], Float4>>;

TYPED_TEST_SUITE(GEPTest, GEPTestTypes);

TYPED_TEST(GEPTest, PtrOffsets)
{
	using CType = typename TestFixture::CType;
	using ReactorType = typename TestFixture::ReactorType;

	std::shared_ptr<Routine> routine;

	{
		Function<Pointer<ReactorType>(Pointer<ReactorType>, Int)> function;
		{
			Pointer<ReactorType> pointer = function.template Arg<0>();
			Int index = function.template Arg<1>();
			Return(&pointer[index]);
		}

		routine = function(testName().c_str());

		auto callable = (CType * (*)(CType *, unsigned int)) routine->getEntry();

		union PtrInt
		{
			CType *p;
			size_t i;
		};

		PtrInt base;
		base.i = 0x10000;

		for(int i = 0; i < 5; i++)
		{
			PtrInt reference;
			reference.p = &base.p[i];

			PtrInt result;
			result.p = callable(base.p, i);

			auto expect = reference.i - base.i;
			auto got = result.i - base.i;

			EXPECT_EQ(got, expect) << "i:" << i;
		}
	}
}

static const std::vector<int> fibonacci = {
	0,
	1,
	1,
	2,
	3,
	5,
	8,
	13,
	21,
	34,
	55,
	89,
	144,
	233,
	377,
	610,
	987,
	1597,
	2584,
	4181,
	6765,
	10946,
	17711,
	28657,
	46368,
	75025,
	121393,
	196418,
	317811,
};

TEST(ReactorUnitTests, Fibonacci)
{
	FunctionT<int(int)> function;
	{
		Int n = function.Arg<0>();
		Int current = 0;
		Int next = 1;
		For(Int i = 0, i < n, i++)
		{
			auto tmp = current + next;
			current = next;
			next = tmp;
		}
		Return(current);
	}

	auto routine = function(testName().c_str());

	for(size_t i = 0; i < fibonacci.size(); i++)
	{
		EXPECT_EQ(routine(i), fibonacci[i]);
	}
}

TEST(ReactorUnitTests, Coroutines_Fibonacci)
{
	if(!rr::Caps::coroutinesSupported())
	{
		SUCCEED() << "Coroutines not supported";
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
	function.finalize(testName().c_str());

	auto coroutine = function();

	for(size_t i = 0; i < fibonacci.size(); i++)
	{
		int out = 0;
		EXPECT_EQ(coroutine->await(out), true);
		EXPECT_EQ(out, fibonacci[i]);
	}
}

TEST(ReactorUnitTests, Coroutines_Parameters)
{
	if(!rr::Caps::coroutinesSupported())
	{
		SUCCEED() << "Coroutines not supported";
		return;
	}

	Coroutine<uint8_t(uint8_t * data, int count)> function;
	{
		Pointer<Byte> data = function.Arg<0>();
		Int count = function.Arg<1>();

		For(Int i = 0, i < count, i++)
		{
			Yield(data[i]);
		}
	}
	function.finalize(testName().c_str());

	uint8_t data[] = { 10, 20, 30 };
	auto coroutine = function(&data[0], 3);

	uint8_t out = 0;
	EXPECT_EQ(coroutine->await(out), true);
	EXPECT_EQ(out, 10);
	out = 0;
	EXPECT_EQ(coroutine->await(out), true);
	EXPECT_EQ(out, 20);
	out = 0;
	EXPECT_EQ(coroutine->await(out), true);
	EXPECT_EQ(out, 30);
	out = 99;
	EXPECT_EQ(coroutine->await(out), false);
	EXPECT_EQ(out, 99);
	EXPECT_EQ(coroutine->await(out), false);
	EXPECT_EQ(out, 99);
}

// This test was written because Subzero's handling of vector types
// failed when more than one function is generated, as is the case
// with coroutines.
TEST(ReactorUnitTests, Coroutines_Vectors)
{
	if(!rr::Caps::coroutinesSupported())
	{
		SUCCEED() << "Coroutines not supported";
		return;
	}

	Coroutine<int()> function;
	{
		Int4 a{ 1, 2, 3, 4 };
		Yield(rr::Extract(a, 2));
		Int4 b{ 5, 6, 7, 8 };
		Yield(rr::Extract(b, 1));
		Int4 c{ 9, 10, 11, 12 };
		Yield(rr::Extract(c, 1));
	}
	function.finalize(testName().c_str());

	auto coroutine = function();

	int out;
	coroutine->await(out);
	EXPECT_EQ(out, 3);
	coroutine->await(out);
	EXPECT_EQ(out, 6);
	coroutine->await(out);
	EXPECT_EQ(out, 10);
}

// This test was written to make sure a coroutine without a Yield()
// works correctly, by executing like a regular function with no
// return (the return type is ignored).
// We also run it twice to ensure per instance and/or global state
// is properly cleaned up in between.
TEST(ReactorUnitTests, Coroutines_NoYield)
{
	if(!rr::Caps::coroutinesSupported())
	{
		SUCCEED() << "Coroutines not supported";
		return;
	}

	for(int i = 0; i < 2; ++i)
	{
		Coroutine<int()> function;
		{
			Int a;
			a = 4;
		}
		function.finalize(testName().c_str());

		auto coroutine = function();
		int out;
		EXPECT_EQ(coroutine->await(out), false);
	}
}

// Test generating one coroutine, and executing it on multiple threads. This makes
// sure the implementation manages per-call instance data correctly.
TEST(ReactorUnitTests, Coroutines_Parallel)
{
	if(!rr::Caps::coroutinesSupported())
	{
		SUCCEED() << "Coroutines not supported";
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

	// Must call on same thread that creates the coroutine
	function.finalize(testName().c_str());

	std::vector<std::thread> threads;
	const size_t numThreads = 100;

	for(size_t t = 0; t < numThreads; ++t)
	{
		threads.emplace_back([&] {
			auto coroutine = function();

			for(size_t i = 0; i < fibonacci.size(); i++)
			{
				int out = 0;
				EXPECT_EQ(coroutine->await(out), true);
				EXPECT_EQ(out, fibonacci[i]);
			}
		});
	}

	for(auto &t : threads)
	{
		t.join();
	}
}

template<typename TestFuncType, typename RefFuncType, typename TestValueType>
struct IntrinsicTestParams
{
	std::function<TestFuncType> testFunc;   // Function we're testing (Reactor)
	std::function<RefFuncType> refFunc;     // Reference function to test against (C)
	std::vector<TestValueType> testValues;  // Values to input to functions
};

using IntrinsicTestParams_Float = IntrinsicTestParams<RValue<Float>(RValue<Float>), float(float), float>;
using IntrinsicTestParams_Float4 = IntrinsicTestParams<RValue<Float4>(RValue<Float4>), float(float), float>;
using IntrinsicTestParams_Float4_Float4 = IntrinsicTestParams<RValue<Float4>(RValue<Float4>, RValue<Float4>), float(float, float), std::pair<float, float>>;

// TODO(b/147818976): Each function has its own precision requirements for Vulkan, sometimes broken down
// by input range. These are currently validated by deqp, but we can improve our own tests as well.
// See https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#spirvenv-precision-operation
constexpr double INTRINSIC_PRECISION = 1e-4;

struct IntrinsicTest_Float : public testing::TestWithParam<IntrinsicTestParams_Float>
{
	void test()
	{
		FunctionT<float(float)> function;
		{
			Return(GetParam().testFunc((Float(function.Arg<0>()))));
		}

		auto routine = function(testName().c_str());

		for(auto &&v : GetParam().testValues)
		{
			SCOPED_TRACE(v);
			EXPECT_NEAR(routine(v), GetParam().refFunc(v), INTRINSIC_PRECISION);
		}
	}
};

// TODO: Move to Reactor.hpp
template<>
struct rr::CToReactor<int[4]>
{
	using type = Int4;
	static Int4 cast(float[4]);
};

// Value type wrapper around a <type>[4] (i.e. float4, int4)
template<typename T>
struct type4_value
{
	using E = typename std::remove_pointer_t<std::decay_t<T>>;

	type4_value() = default;
	explicit type4_value(E rep)
	    : v{ rep, rep, rep, rep }
	{}
	type4_value(E x, E y, E z, E w)
	    : v{ x, y, z, w }
	{}

	bool operator==(const type4_value &rhs) const
	{
		return std::equal(std::begin(v), std::end(v), rhs.v);
	}

	// For gtest printing
	friend std::ostream &operator<<(std::ostream &os, const type4_value &value)
	{
		return os << "[" << value.v[0] << ", " << value.v[1] << ", " << value.v[2] << ", " << value.v[3] << "]";
	}

	T v;
};

using float4_value = type4_value<float4>;
using int4_value = type4_value<int4>;

// Invoke a void(type4_value<T>*) routine on &v.v, returning wrapped result in v
template<typename RoutineType, typename T>
type4_value<T> invokeRoutine(RoutineType &routine, type4_value<T> v)
{
	routine(&v.v);
	return v;
}

// Invoke a void(type4_value<T>*, type4_value<T>*) routine on &v1.v, &v2.v returning wrapped result in v1
template<typename RoutineType, typename T>
type4_value<T> invokeRoutine(RoutineType &routine, type4_value<T> v1, type4_value<T> v2)
{
	routine(&v1.v, &v2.v);
	return v1;
}

struct IntrinsicTest_Float4 : public testing::TestWithParam<IntrinsicTestParams_Float4>
{
	void test()
	{
		FunctionT<void(float4 *)> function;
		{
			Pointer<Float4> a = function.Arg<0>();
			*a = GetParam().testFunc(*a);
			Return();
		}

		auto routine = function(testName().c_str());

		for(auto &&v : GetParam().testValues)
		{
			SCOPED_TRACE(v);
			float4_value result = invokeRoutine(routine, float4_value{ v });
			float4_value expected = float4_value{ GetParam().refFunc(v) };
			EXPECT_NEAR(result.v[0], expected.v[0], INTRINSIC_PRECISION);
			EXPECT_NEAR(result.v[1], expected.v[1], INTRINSIC_PRECISION);
			EXPECT_NEAR(result.v[2], expected.v[2], INTRINSIC_PRECISION);
			EXPECT_NEAR(result.v[3], expected.v[3], INTRINSIC_PRECISION);
		}
	}
};

struct IntrinsicTest_Float4_Float4 : public testing::TestWithParam<IntrinsicTestParams_Float4_Float4>
{
	void test()
	{
		FunctionT<void(float4 *, float4 *)> function;
		{
			Pointer<Float4> a = function.Arg<0>();
			Pointer<Float4> b = function.Arg<1>();
			*a = GetParam().testFunc(*a, *b);
			Return();
		}

		auto routine = function(testName().c_str());

		for(auto &&v : GetParam().testValues)
		{
			SCOPED_TRACE(v);
			float4_value result = invokeRoutine(routine, float4_value{ v.first }, float4_value{ v.second });
			float4_value expected = float4_value{ GetParam().refFunc(v.first, v.second) };
			EXPECT_NEAR(result.v[0], expected.v[0], INTRINSIC_PRECISION);
			EXPECT_NEAR(result.v[1], expected.v[1], INTRINSIC_PRECISION);
			EXPECT_NEAR(result.v[2], expected.v[2], INTRINSIC_PRECISION);
			EXPECT_NEAR(result.v[3], expected.v[3], INTRINSIC_PRECISION);
		}
	}
};

// clang-format off
INSTANTIATE_TEST_SUITE_P(IntrinsicTestParams_Float, IntrinsicTest_Float, testing::Values(
	IntrinsicTestParams_Float{ [](Float v) { return rr::Exp2(v); }, exp2f, {0.f, 1.f, 123.f} },
	IntrinsicTestParams_Float{ [](Float v) { return rr::Log2(v); }, log2f, {1.f, 123.f} },
	IntrinsicTestParams_Float{ [](Float v) { return rr::Sqrt(v); }, sqrtf, {0.f, 1.f, 123.f} }
));
// clang-format on

// TODO(b/149110874) Use coshf/sinhf when we've implemented SpirV versions at the SpirV level
float vulkan_sinhf(float a)
{
	return ((expf(a) - expf(-a)) / 2);
}
float vulkan_coshf(float a)
{
	return ((expf(a) + expf(-a)) / 2);
}

// clang-format off
constexpr float PI = 3.141592653589793f;
INSTANTIATE_TEST_SUITE_P(IntrinsicTestParams_Float4, IntrinsicTest_Float4, testing::Values(
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Sin(v); },   sinf,         {0.f, 1.f, PI, 123.f}  },
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Cos(v); },   cosf,         {0.f, 1.f, PI, 123.f}  },
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Tan(v); },   tanf,         {0.f, 1.f, PI, 123.f}  },
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Asin(v); },  asinf,        {0.f, 1.f, -1.f}  },
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Acos(v); },  acosf,        {0.f, 1.f, -1.f}  },
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Atan(v); },  atanf,        {0.f, 1.f, PI, 123.f}  },
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Sinh(v); },  vulkan_sinhf, {0.f, 1.f, PI}  },
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Cosh(v); },  vulkan_coshf, {0.f, 1.f, PI} },
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Tanh(v); },  tanhf,        {0.f, 1.f, PI}  },
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Asinh(v); }, asinhf,       {0.f, 1.f, PI, 123.f}  },
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Acosh(v); }, acoshf,       {     1.f, PI, 123.f}  },
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Atanh(v); }, atanhf,       {0.f, 0.9999f, -0.9999f}  },
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Exp(v); },   expf,         {0.f, 1.f, PI}  },
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Log(v); },   logf,         {1.f, PI, 123.f}  },
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Exp2(v); },  exp2f,        {0.f, 1.f, PI, 123.f}  },
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Log2(v); },  log2f,        {1.f, PI, 123.f}  },
	IntrinsicTestParams_Float4{ [](RValue<Float4> v) { return rr::Sqrt(v); },  sqrtf,        {0.f, 1.f, PI, 123.f}  }
));
// clang-format on

// clang-format off
INSTANTIATE_TEST_SUITE_P(IntrinsicTestParams_Float4_Float4, IntrinsicTest_Float4_Float4, testing::Values(
	IntrinsicTestParams_Float4_Float4{ [](RValue<Float4> v1, RValue<Float4> v2) { return Atan2(v1, v2); }, atan2f, { {0.f, 0.f}, {0.f, -1.f}, {-1.f, 0.f}, {123.f, 123.f} } },
	IntrinsicTestParams_Float4_Float4{ [](RValue<Float4> v1, RValue<Float4> v2) { return Pow(v1, v2); },   powf,   { {1.f, 0.f}, {1.f, -1.f}, {-1.f, 0.f} } }
));
// clang-format on

TEST_P(IntrinsicTest_Float, Test)
{
	test();
}
TEST_P(IntrinsicTest_Float4, Test)
{
	test();
}
TEST_P(IntrinsicTest_Float4_Float4, Test)
{
	test();
}

TEST(ReactorUnitTests, Intrinsics_Ctlz)
{
	// ctlz: counts number of leading zeros

	{
		Function<UInt(UInt x)> function;
		{
			UInt x = function.Arg<0>();
			Return(rr::Ctlz(x, false));
		}
		auto routine = function(testName().c_str());
		auto callable = (uint32_t(*)(uint32_t))routine->getEntry();

		for(uint32_t i = 0; i < 31; ++i)
		{
			uint32_t result = callable(1 << i);
			EXPECT_EQ(result, 31 - i);
		}

		// Input 0 should return 32 for isZeroUndef == false
		{
			uint32_t result = callable(0);
			EXPECT_EQ(result, 32u);
		}
	}

	{
		Function<Void(Pointer<UInt4>, UInt x)> function;
		{
			Pointer<UInt4> out = function.Arg<0>();
			UInt x = function.Arg<1>();
			*out = rr::Ctlz(UInt4(x), false);
		}
		auto routine = function(testName().c_str());
		auto callable = (void (*)(uint32_t *, uint32_t))routine->getEntry();

		uint32_t x[4];

		for(uint32_t i = 0; i < 31; ++i)
		{
			callable(x, 1 << i);
			EXPECT_EQ(x[0], 31 - i);
			EXPECT_EQ(x[1], 31 - i);
			EXPECT_EQ(x[2], 31 - i);
			EXPECT_EQ(x[3], 31 - i);
		}

		// Input 0 should return 32 for isZeroUndef == false
		{
			callable(x, 0);
			EXPECT_EQ(x[0], 32u);
			EXPECT_EQ(x[1], 32u);
			EXPECT_EQ(x[2], 32u);
			EXPECT_EQ(x[3], 32u);
		}
	}
}

TEST(ReactorUnitTests, Intrinsics_Cttz)
{
	// cttz: counts number of trailing zeros

	{
		Function<UInt(UInt x)> function;
		{
			UInt x = function.Arg<0>();
			Return(rr::Cttz(x, false));
		}
		auto routine = function(testName().c_str());
		auto callable = (uint32_t(*)(uint32_t))routine->getEntry();

		for(uint32_t i = 0; i < 31; ++i)
		{
			uint32_t result = callable(1 << i);
			EXPECT_EQ(result, i);
		}

		// Input 0 should return 32 for isZeroUndef == false
		{
			uint32_t result = callable(0);
			EXPECT_EQ(result, 32u);
		}
	}

	{
		Function<Void(Pointer<UInt4>, UInt x)> function;
		{
			Pointer<UInt4> out = function.Arg<0>();
			UInt x = function.Arg<1>();
			*out = rr::Cttz(UInt4(x), false);
		}
		auto routine = function(testName().c_str());
		auto callable = (void (*)(uint32_t *, uint32_t))routine->getEntry();

		uint32_t x[4];

		for(uint32_t i = 0; i < 31; ++i)
		{
			callable(x, 1 << i);
			EXPECT_EQ(x[0], i);
			EXPECT_EQ(x[1], i);
			EXPECT_EQ(x[2], i);
			EXPECT_EQ(x[3], i);
		}

		// Input 0 should return 32 for isZeroUndef == false
		{
			callable(x, 0);
			EXPECT_EQ(x[0], 32u);
			EXPECT_EQ(x[1], 32u);
			EXPECT_EQ(x[2], 32u);
			EXPECT_EQ(x[3], 32u);
		}
	}
}

TEST(ReactorUnitTests, ExtractFromRValue)
{
	Function<Void(Pointer<Int4> values, Pointer<Int4> result)> function;
	{
		Pointer<Int4> vIn = function.Arg<0>();
		Pointer<Int4> resultIn = function.Arg<1>();

		RValue<Int4> v = *vIn;

		Int4 result(678);

		If(Extract(v, 0) == 42)
		{
			result = Insert(result, 1, 0);
		}

		If(Extract(v, 1) == 42)
		{
			result = Insert(result, 1, 1);
		}

		*resultIn = result;

		Return();
	}

	auto routine = function(testName().c_str());
	auto entry = (void (*)(int *, int *))routine->getEntry();

	int v[4] = { 42, 42, 42, 42 };
	int result[4] = { 99, 99, 99, 99 };
	entry(v, result);
	EXPECT_EQ(result[0], 1);
	EXPECT_EQ(result[1], 1);
	EXPECT_EQ(result[2], 678);
	EXPECT_EQ(result[3], 678);
}

TEST(ReactorUnitTests, AddAtomic)
{
	FunctionT<uint32_t(uint32_t * p, uint32_t a)> function;
	{
		Pointer<UInt> p = function.Arg<0>();
		UInt a = function.Arg<1>();
		UInt r = rr::AddAtomic(p, a, std::memory_order_relaxed);
		Return(r);
	}

	auto routine = function(testName().c_str());
	uint32_t x = 123;
	uint32_t y = 456;
	uint32_t prevX = routine(&x, y);
	EXPECT_EQ(prevX, 123u);
	EXPECT_EQ(x, 579u);
}

TEST(ReactorUnitTests, SubAtomic)
{
	FunctionT<uint32_t(uint32_t * p, uint32_t a)> function;
	{
		Pointer<UInt> p = function.Arg<0>();
		UInt a = function.Arg<1>();
		UInt r = rr::SubAtomic(p, a, std::memory_order_relaxed);
		Return(r);
	}

	auto routine = function(testName().c_str());
	uint32_t x = 456;
	uint32_t y = 123;
	uint32_t prevX = routine(&x, y);
	EXPECT_EQ(prevX, 456u);
	EXPECT_EQ(x, 333u);
}

TEST(ReactorUnitTests, AndAtomic)
{
	FunctionT<uint32_t(uint32_t * p, uint32_t a)> function;
	{
		Pointer<UInt> p = function.Arg<0>();
		UInt a = function.Arg<1>();
		UInt r = rr::AndAtomic(p, a, std::memory_order_relaxed);
		Return(r);
	}

	auto routine = function(testName().c_str());
	uint32_t x = 0b1111'0000;
	uint32_t y = 0b1010'1100;
	uint32_t prevX = routine(&x, y);
	EXPECT_EQ(prevX, 0b1111'0000u);
	EXPECT_EQ(x, 0b1010'0000u);
}

TEST(ReactorUnitTests, OrAtomic)
{
	FunctionT<uint32_t(uint32_t * p, uint32_t a)> function;
	{
		Pointer<UInt> p = function.Arg<0>();
		UInt a = function.Arg<1>();
		UInt r = rr::OrAtomic(p, a, std::memory_order_relaxed);
		Return(r);
	}

	auto routine = function(testName().c_str());
	uint32_t x = 0b1111'0000;
	uint32_t y = 0b1010'1100;
	uint32_t prevX = routine(&x, y);
	EXPECT_EQ(prevX, 0b1111'0000u);
	EXPECT_EQ(x, 0b1111'1100u);
}

TEST(ReactorUnitTests, XorAtomic)
{
	FunctionT<uint32_t(uint32_t * p, uint32_t a)> function;
	{
		Pointer<UInt> p = function.Arg<0>();
		UInt a = function.Arg<1>();
		UInt r = rr::XorAtomic(p, a, std::memory_order_relaxed);
		Return(r);
	}

	auto routine = function(testName().c_str());
	uint32_t x = 0b1111'0000;
	uint32_t y = 0b1010'1100;
	uint32_t prevX = routine(&x, y);
	EXPECT_EQ(prevX, 0b1111'0000u);
	EXPECT_EQ(x, 0b0101'1100u);
}

TEST(ReactorUnitTests, MinAtomic)
{
	{
		FunctionT<uint32_t(uint32_t * p, uint32_t a)> function;
		{
			Pointer<UInt> p = function.Arg<0>();
			UInt a = function.Arg<1>();
			UInt r = rr::MinAtomic(p, a, std::memory_order_relaxed);
			Return(r);
		}

		auto routine = function(testName().c_str());
		uint32_t x = 123;
		uint32_t y = 100;
		uint32_t prevX = routine(&x, y);
		EXPECT_EQ(prevX, 123u);
		EXPECT_EQ(x, 100u);
	}

	{
		FunctionT<int32_t(int32_t * p, int32_t a)> function;
		{
			Pointer<Int> p = function.Arg<0>();
			Int a = function.Arg<1>();
			Int r = rr::MinAtomic(p, a, std::memory_order_relaxed);
			Return(r);
		}

		auto routine = function(testName().c_str());
		int32_t x = -123;
		int32_t y = -200;
		int32_t prevX = routine(&x, y);
		EXPECT_EQ(prevX, -123);
		EXPECT_EQ(x, -200);
	}
}

TEST(ReactorUnitTests, MaxAtomic)
{
	{
		FunctionT<uint32_t(uint32_t * p, uint32_t a)> function;
		{
			Pointer<UInt> p = function.Arg<0>();
			UInt a = function.Arg<1>();
			UInt r = rr::MaxAtomic(p, a, std::memory_order_relaxed);
			Return(r);
		}

		auto routine = function(testName().c_str());
		uint32_t x = 123;
		uint32_t y = 100;
		uint32_t prevX = routine(&x, y);
		EXPECT_EQ(prevX, 123u);
		EXPECT_EQ(x, 123u);
	}

	{
		FunctionT<int32_t(int32_t * p, int32_t a)> function;
		{
			Pointer<Int> p = function.Arg<0>();
			Int a = function.Arg<1>();
			Int r = rr::MaxAtomic(p, a, std::memory_order_relaxed);
			Return(r);
		}

		auto routine = function(testName().c_str());
		int32_t x = -123;
		int32_t y = -200;
		int32_t prevX = routine(&x, y);
		EXPECT_EQ(prevX, -123);
		EXPECT_EQ(x, -123);
	}
}

TEST(ReactorUnitTests, ExchangeAtomic)
{
	FunctionT<uint32_t(uint32_t * p, uint32_t a)> function;
	{
		Pointer<UInt> p = function.Arg<0>();
		UInt a = function.Arg<1>();
		UInt r = rr::ExchangeAtomic(p, a, std::memory_order_relaxed);
		Return(r);
	}

	auto routine = function(testName().c_str());
	uint32_t x = 123;
	uint32_t y = 456;
	uint32_t prevX = routine(&x, y);
	EXPECT_EQ(prevX, 123u);
	EXPECT_EQ(x, y);
}

TEST(ReactorUnitTests, CompareExchangeAtomic)
{
	FunctionT<uint32_t(uint32_t * x, uint32_t y, uint32_t compare)> function;
	{
		Pointer<UInt> x = function.Arg<0>();
		UInt y = function.Arg<1>();
		UInt compare = function.Arg<2>();
		UInt r = rr::CompareExchangeAtomic(x, y, compare, std::memory_order_relaxed, std::memory_order_relaxed);
		Return(r);
	}

	auto routine = function(testName().c_str());
	uint32_t x = 123;
	uint32_t y = 456;
	uint32_t compare = 123;
	uint32_t prevX = routine(&x, y, compare);
	EXPECT_EQ(prevX, 123u);
	EXPECT_EQ(x, y);

	x = 123;
	y = 456;
	compare = 456;
	prevX = routine(&x, y, compare);
	EXPECT_EQ(prevX, 123u);
	EXPECT_EQ(x, 123u);
}

TEST(ReactorUnitTests, SRem)
{
	FunctionT<void(int4 *, int4 *)> function;
	{
		Pointer<Int4> a = function.Arg<0>();
		Pointer<Int4> b = function.Arg<1>();
		*a = *a % *b;
	}

	auto routine = function(testName().c_str());

	int4_value result = invokeRoutine(routine, int4_value{ 10, 11, 12, 13 }, int4_value{ 3, 3, 3, 3 });
	int4_value expected = int4_value{ 10 % 3, 11 % 3, 12 % 3, 13 % 3 };
	EXPECT_FLOAT_EQ(result.v[0], expected.v[0]);
	EXPECT_FLOAT_EQ(result.v[1], expected.v[1]);
	EXPECT_FLOAT_EQ(result.v[2], expected.v[2]);
	EXPECT_FLOAT_EQ(result.v[3], expected.v[3]);
}

TEST(ReactorUnitTests, FRem)
{
	FunctionT<void(float4 *, float4 *)> function;
	{
		Pointer<Float4> a = function.Arg<0>();
		Pointer<Float4> b = function.Arg<1>();
		*a = *a % *b;
	}

	auto routine = function(testName().c_str());

	float4_value result = invokeRoutine(routine, float4_value{ 10.1f, 11.2f, 12.3f, 13.4f }, float4_value{ 3.f, 3.f, 3.f, 3.f });
	float4_value expected = float4_value{ fmodf(10.1f, 3.f), fmodf(11.2f, 3.f), fmodf(12.3f, 3.f), fmodf(13.4f, 3.f) };
	EXPECT_FLOAT_EQ(result.v[0], expected.v[0]);
	EXPECT_FLOAT_EQ(result.v[1], expected.v[1]);
	EXPECT_FLOAT_EQ(result.v[2], expected.v[2]);
	EXPECT_FLOAT_EQ(result.v[3], expected.v[3]);
}

// Subzero's load instruction assumes that a Constant ptr value is an offset, rather than an absolute
// pointer, and would fail during codegen. This was fixed by casting the constant to a non-const
// variable, and loading from it instead. This test makes sure this works.
TEST(ReactorUnitTests, LoadFromConstantData)
{
	const int value = 123;

	FunctionT<int()> function;
	{
		auto p = Pointer<Int>{ ConstantData(&value, sizeof(value)) };
		Int v = *p;
		Return(v);
	}

	const int result = function(testName().c_str())();
	EXPECT_EQ(result, value);
}

TEST(ReactorUnitTests, Multithreaded_Function)
{
	constexpr int numThreads = 8;
	constexpr int numLoops = 16;

	auto threads = std::unique_ptr<std::thread[]>(new std::thread[numThreads]);
	auto results = std::unique_ptr<int[]>(new int[numThreads * numLoops]);

	for(int t = 0; t < numThreads; t++)
	{
		auto threadFunc = [&](int t) {
			for(int l = 0; l < numLoops; l++)
			{
				FunctionT<int(int, int)> function;
				{
					Int a = function.Arg<0>();
					Int b = function.Arg<1>();
					Return((a << 16) | b);
				}

				auto f = function("%s_thread%d_loop%d", testName().c_str(), t, l);
				results[t * numLoops + l] = f(t, l);
			}
		};
		threads[t] = std::thread(threadFunc, t);
	}

	for(int t = 0; t < numThreads; t++)
	{
		threads[t].join();
	}

	for(int t = 0; t < numThreads; t++)
	{
		for(int l = 0; l < numLoops; l++)
		{
			auto expect = (t << 16) | l;
			auto result = results[t * numLoops + l];
			EXPECT_EQ(result, expect);
		}
	}
}

TEST(ReactorUnitTests, Multithreaded_Coroutine)
{
	if(!rr::Caps::coroutinesSupported())
	{
		SUCCEED() << "Coroutines not supported";
		return;
	}

	constexpr int numThreads = 8;
	constexpr int numLoops = 16;

	struct Result
	{
		bool yieldReturns[3];
		int yieldValues[3];
	};

	auto threads = std::unique_ptr<std::thread[]>(new std::thread[numThreads]);
	auto results = std::unique_ptr<Result[]>(new Result[numThreads * numLoops]);

	for(int t = 0; t < numThreads; t++)
	{
		auto threadFunc = [&](int t) {
			for(int l = 0; l < numLoops; l++)
			{
				Coroutine<int(int, int)> function;
				{
					Int a = function.Arg<0>();
					Int b = function.Arg<1>();
					Yield(a);
					Yield(b);
				}
				function.finalize((testName() + "_thread" + std::to_string(t) + "_loop" + std::to_string(l)).c_str());

				auto coroutine = function(t, l);

				auto &result = results[t * numLoops + l];
				result = {};
				result.yieldReturns[0] = coroutine->await(result.yieldValues[0]);
				result.yieldReturns[1] = coroutine->await(result.yieldValues[1]);
				result.yieldReturns[2] = coroutine->await(result.yieldValues[2]);
			}
		};
		threads[t] = std::thread(threadFunc, t);
	}

	for(int t = 0; t < numThreads; t++)
	{
		threads[t].join();
	}

	for(int t = 0; t < numThreads; t++)
	{
		for(int l = 0; l < numLoops; l++)
		{
			const auto &result = results[t * numLoops + l];
			EXPECT_EQ(result.yieldReturns[0], true);
			EXPECT_EQ(result.yieldValues[0], t);
			EXPECT_EQ(result.yieldReturns[1], true);
			EXPECT_EQ(result.yieldValues[1], l);
			EXPECT_EQ(result.yieldReturns[2], false);
			EXPECT_EQ(result.yieldValues[2], 0);
		}
	}
}

// For gtest printing of pairs
namespace std {
template<typename T, typename U>
std::ostream &operator<<(std::ostream &os, const std::pair<T, U> &value)
{
	return os << "{ " << value.first << ", " << value.second << " }";
}
}  // namespace std

class StdOutCapture
{
public:
	~StdOutCapture()
	{
		stopIfCapturing();
	}

	void start()
	{
		stopIfCapturing();
		capturing = true;
		testing::internal::CaptureStdout();
	}

	std::string stop()
	{
		assert(capturing);
		capturing = false;
		return testing::internal::GetCapturedStdout();
	}

private:
	void stopIfCapturing()
	{
		if(capturing)
		{
			// This stops the capture
			testing::internal::GetCapturedStdout();
		}
	}

	bool capturing = false;
};

std::vector<std::string> split(const std::string &s)
{
	std::vector<std::string> result;
	std::istringstream iss(s);
	for(std::string line; std::getline(iss, line);)
	{
		result.push_back(line);
	}
	return result;
}

TEST(ReactorUnitTests, PrintPrimitiveTypes)
{
#if defined(ENABLE_RR_PRINT) && !defined(ENABLE_RR_EMIT_PRINT_LOCATION)
	FunctionT<void()> function;
	{
		bool b(true);
		int8_t i8(-1);
		uint8_t ui8(1);
		int16_t i16(-1);
		uint16_t ui16(1);
		int32_t i32(-1);
		uint32_t ui32(1);
		int64_t i64(-1);
		uint64_t ui64(1);
		float f(1);
		double d(2);
		const char *cstr = "const char*";
		std::string str = "std::string";
		int *p = nullptr;

		RR_WATCH(b);
		RR_WATCH(i8);
		RR_WATCH(ui8);
		RR_WATCH(i16);
		RR_WATCH(ui16);
		RR_WATCH(i32);
		RR_WATCH(ui32);
		RR_WATCH(i64);
		RR_WATCH(ui64);
		RR_WATCH(f);
		RR_WATCH(d);
		RR_WATCH(cstr);
		RR_WATCH(str);
		RR_WATCH(p);
	}

	auto routine = function(testName().c_str());

	char pNullptr[64];
	snprintf(pNullptr, sizeof(pNullptr), "  p: %p", nullptr);

	const char *expected[] = {
		"  b: true",
		"  i8: -1",
		"  ui8: 1",
		"  i16: -1",
		"  ui16: 1",
		"  i32: -1",
		"  ui32: 1",
		"  i64: -1",
		"  ui64: 1",
		"  f: 1.000000",
		"  d: 2.000000",
		"  cstr: const char*",
		"  str: std::string",
		pNullptr,
	};
	constexpr size_t expectedSize = sizeof(expected) / sizeof(expected[0]);

	StdOutCapture capture;
	capture.start();
	routine();
	auto output = split(capture.stop());
	for(size_t i = 0, j = 1; i < expectedSize; ++i, j += 2)
	{
		ASSERT_EQ(expected[i], output[j]);
	}

#endif
}

TEST(ReactorUnitTests, PrintReactorTypes)
{
#if defined(ENABLE_RR_PRINT) && !defined(ENABLE_RR_EMIT_PRINT_LOCATION)
	FunctionT<void()> function;
	{
		Bool b(true);
		Int i(-1);
		Int2 i2(-1, -2);
		Int4 i4(-1, -2, -3, -4);
		UInt ui(1);
		UInt2 ui2(1, 2);
		UInt4 ui4(1, 2, 3, 4);
		Short s(-1);
		Short4 s4(-1, -2, -3, -4);
		UShort us(1);
		UShort4 us4(1, 2, 3, 4);
		Float f(1);
		Float4 f4(1, 2, 3, 4);
		Long l(i);
		Pointer<Int> pi = nullptr;
		RValue<Int> rvi = i;
		Byte by('a');
		Byte4 by4(i4);

		RR_WATCH(b);
		RR_WATCH(i);
		RR_WATCH(i2);
		RR_WATCH(i4);
		RR_WATCH(ui);
		RR_WATCH(ui2);
		RR_WATCH(ui4);
		RR_WATCH(s);
		RR_WATCH(s4);
		RR_WATCH(us);
		RR_WATCH(us4);
		RR_WATCH(f);
		RR_WATCH(f4);
		RR_WATCH(l);
		RR_WATCH(pi);
		RR_WATCH(rvi);
		RR_WATCH(by);
		RR_WATCH(by4);
	}

	auto routine = function(testName().c_str());

	char piNullptr[64];
	snprintf(piNullptr, sizeof(piNullptr), "  pi: %p", nullptr);

	const char *expected[] = {
		"  b: true",
		"  i: -1",
		"  i2: [-1, -2]",
		"  i4: [-1, -2, -3, -4]",
		"  ui: 1",
		"  ui2: [1, 2]",
		"  ui4: [1, 2, 3, 4]",
		"  s: -1",
		"  s4: [-1, -2, -3, -4]",
		"  us: 1",
		"  us4: [1, 2, 3, 4]",
		"  f: 1.000000",
		"  f4: [1.000000, 2.000000, 3.000000, 4.000000]",
		"  l: -1",
		piNullptr,
		"  rvi: -1",
		"  by: 97",
		"  by4: [255, 254, 253, 252]",
	};
	constexpr size_t expectedSize = sizeof(expected) / sizeof(expected[0]);

	StdOutCapture capture;
	capture.start();
	routine();
	auto output = split(capture.stop());
	for(size_t i = 0, j = 1; i < expectedSize; ++i, j += 2)
	{
		ASSERT_EQ(expected[i], output[j]);
	}

#endif
}

// Test constant <op> variable
template<typename T, typename Func>
T Arithmetic_LhsConstArg(T arg1, T arg2, Func f)
{
	using ReactorT = CToReactorT<T>;

	FunctionT<T(T)> function;
	{
		ReactorT lhs = arg1;
		ReactorT rhs = function.template Arg<0>();
		ReactorT result = f(lhs, rhs);
		Return(result);
	}

	auto routine = function(testName().c_str());
	return routine(arg2);
}

// Test variable <op> constant
template<typename T, typename Func>
T Arithmetic_RhsConstArg(T arg1, T arg2, Func f)
{
	using ReactorT = CToReactorT<T>;

	FunctionT<T(T)> function;
	{
		ReactorT lhs = function.template Arg<0>();
		ReactorT rhs = arg2;
		ReactorT result = f(lhs, rhs);
		Return(result);
	}

	auto routine = function(testName().c_str());
	return routine(arg1);
}

// Test constant <op> constant
template<typename T, typename Func>
T Arithmetic_TwoConstArgs(T arg1, T arg2, Func f)
{
	using ReactorT = CToReactorT<T>;

	FunctionT<T()> function;
	{
		ReactorT lhs = arg1;
		ReactorT rhs = arg2;
		ReactorT result = f(lhs, rhs);
		Return(result);
	}

	auto routine = function(testName().c_str());
	return routine();
}

template<typename T, typename Func>
void Arithmetic_ConstArgs(T arg1, T arg2, T expected, Func f)
{
	SCOPED_TRACE(std::to_string(arg1) + " <op> " + std::to_string(arg2) + " = " + std::to_string(expected));
	T result{};
	result = Arithmetic_LhsConstArg(arg1, arg2, std::forward<Func>(f));
	EXPECT_EQ(result, expected);
	result = Arithmetic_RhsConstArg(arg1, arg2, std::forward<Func>(f));
	EXPECT_EQ(result, expected);
	result = Arithmetic_TwoConstArgs(arg1, arg2, std::forward<Func>(f));
	EXPECT_EQ(result, expected);
}

// Test that we generate valid code for when one or both args to arithmetic operations
// are constant. In particular, we want to validate the case for two const args, as
// often lowered instructions do not support this case.
TEST(ReactorUnitTests, Arithmetic_ConstantArgs)
{
	Arithmetic_ConstArgs(2, 3, 5, [](auto c1, auto c2) { return c1 + c2; });
	Arithmetic_ConstArgs(5, 3, 2, [](auto c1, auto c2) { return c1 - c2; });
	Arithmetic_ConstArgs(2, 3, 6, [](auto c1, auto c2) { return c1 * c2; });
	Arithmetic_ConstArgs(6, 3, 2, [](auto c1, auto c2) { return c1 / c2; });
	Arithmetic_ConstArgs(0xF0F0, 0xAAAA, 0xA0A0, [](auto c1, auto c2) { return c1 & c2; });
	Arithmetic_ConstArgs(0xF0F0, 0xAAAA, 0xFAFA, [](auto c1, auto c2) { return c1 | c2; });
	Arithmetic_ConstArgs(0xF0F0, 0xAAAA, 0x5A5A, [](auto c1, auto c2) { return c1 ^ c2; });

	Arithmetic_ConstArgs(2.f, 3.f, 5.f, [](auto c1, auto c2) { return c1 + c2; });
	Arithmetic_ConstArgs(5.f, 3.f, 2.f, [](auto c1, auto c2) { return c1 - c2; });
	Arithmetic_ConstArgs(2.f, 3.f, 6.f, [](auto c1, auto c2) { return c1 * c2; });
	Arithmetic_ConstArgs(6.f, 3.f, 2.f, [](auto c1, auto c2) { return c1 / c2; });
}

// Test for Subzero bad code-gen that was fixed in swiftshader-cl/50008
// This tests the case of copying enough arguments to local variables so that the locals
// get spilled to the stack when no more registers remain, and making sure these copies
// are generated correctly. Without the aforementioned fix, this fails 100% on Windows x86.
TEST(ReactorUnitTests, SpillLocalCopiesOfArgs)
{
	struct Helpers
	{
		static bool True() { return true; }
	};

	const int numLoops = 5;  // 2 should be enough, but loop more to make sure

	FunctionT<int(int, int, int, int, int, int, int, int, int, int, int, int)> function;
	{
		Int result = 0;
		Int a1 = function.Arg<0>();
		Int a2 = function.Arg<1>();
		Int a3 = function.Arg<2>();
		Int a4 = function.Arg<3>();
		Int a5 = function.Arg<4>();
		Int a6 = function.Arg<5>();
		Int a7 = function.Arg<6>();
		Int a8 = function.Arg<7>();
		Int a9 = function.Arg<8>();
		Int a10 = function.Arg<9>();
		Int a11 = function.Arg<10>();
		Int a12 = function.Arg<11>();

		for(int i = 0; i < numLoops; ++i)
		{
			// Copy all arguments to locals so that Ice::LocalVariableSplitter::handleSimpleVarAssign
			// creates Variable copies of arguments. We loop so that we create enough of these so
			// that some spill over to the stack.
			Int i1 = a1;
			Int i2 = a2;
			Int i3 = a3;
			Int i4 = a4;
			Int i5 = a5;
			Int i6 = a6;
			Int i7 = a7;
			Int i8 = a8;
			Int i9 = a9;
			Int i10 = a10;
			Int i11 = a11;
			Int i12 = a12;

			// Forcibly materialize all variables so that Ice::Variable instances are created for each
			// local; otherwise, Reactor r-value optimizations kick in, and the locals are elided.
			Variable::materializeAll();

			// We also need to create a separate block that uses the variables declared above
			// so that rr::optimize() doesn't optimize them out when attempting to eliminate stores
			// followed by a load in the same block.
			If(Call(Helpers::True))
			{
				result += (i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 + i11 + i12);
			}
		}

		Return(result);
	}

	auto routine = function(testName().c_str());
	int result = routine(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12);
	int expected = numLoops * (1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 11 + 12);
	EXPECT_EQ(result, expected);
}

#if defined(ENABLE_RR_EMIT_ASM_FILE)
TEST(ReactorUnitTests, EmitAsm)
{
	// Only supported by LLVM for now
	if(Caps::backendName().find("LLVM") == std::string::npos) return;

	namespace fs = std::filesystem;

	FunctionT<int(void)> function;
	{
		Int sum;
		For(Int i = 0, i < 10, i++)
		{
			sum += i;
		}
		Return(sum);
	}

	auto routine = function(testName().c_str());

	// Returns path to first match of filename in current directory
	auto findFile = [](const std::string filename) -> fs::path {
		for(auto &p : fs::directory_iterator("."))
		{
			if(!p.is_regular_file())
				continue;
			auto currFilename = p.path().filename().string();
			auto index = currFilename.find(testName());
			if(index != std::string::npos)
			{
				return p.path();
			}
		}
		return {};
	};

	fs::path path = findFile(testName());
	EXPECT_FALSE(path.empty());

	// Make sure an asm file was created
	std::ifstream fin(path);
	EXPECT_TRUE(fin);

	// Make sure address of routine is in the file
	auto findAddressInFile = [](std::ifstream &fin, size_t address) {
		std::string addressString = [&] {
			std::stringstream addressSS;
			addressSS << "0x" << std::uppercase << std::hex << address;
			return addressSS.str();
		}();

		std::string token;
		while(fin >> token)
		{
			if(token.find(addressString) != std::string::npos)
				return true;
		}
		return false;
	};

	size_t address = reinterpret_cast<size_t>(routine.getEntry());
	EXPECT_TRUE(findAddressInFile(fin, address));

	// Delete the file in case subsequent runs generate one with a different sequence number
	fin.close();
	std::filesystem::remove(path);
}
#endif

////////////////////////////////
// Trait compile time checks. //
////////////////////////////////

// Assert CToReactorT resolves to expected types.
static_assert(std::is_same<CToReactorT<void>, Void>::value, "");
static_assert(std::is_same<CToReactorT<bool>, Bool>::value, "");
static_assert(std::is_same<CToReactorT<uint8_t>, Byte>::value, "");
static_assert(std::is_same<CToReactorT<int8_t>, SByte>::value, "");
static_assert(std::is_same<CToReactorT<int16_t>, Short>::value, "");
static_assert(std::is_same<CToReactorT<uint16_t>, UShort>::value, "");
static_assert(std::is_same<CToReactorT<int32_t>, Int>::value, "");
static_assert(std::is_same<CToReactorT<uint64_t>, Long>::value, "");
static_assert(std::is_same<CToReactorT<uint32_t>, UInt>::value, "");
static_assert(std::is_same<CToReactorT<float>, Float>::value, "");

// Assert CToReactorT for known pointer types resolves to expected types.
static_assert(std::is_same<CToReactorT<void *>, Pointer<Byte>>::value, "");
static_assert(std::is_same<CToReactorT<bool *>, Pointer<Bool>>::value, "");
static_assert(std::is_same<CToReactorT<uint8_t *>, Pointer<Byte>>::value, "");
static_assert(std::is_same<CToReactorT<int8_t *>, Pointer<SByte>>::value, "");
static_assert(std::is_same<CToReactorT<int16_t *>, Pointer<Short>>::value, "");
static_assert(std::is_same<CToReactorT<uint16_t *>, Pointer<UShort>>::value, "");
static_assert(std::is_same<CToReactorT<int32_t *>, Pointer<Int>>::value, "");
static_assert(std::is_same<CToReactorT<uint64_t *>, Pointer<Long>>::value, "");
static_assert(std::is_same<CToReactorT<uint32_t *>, Pointer<UInt>>::value, "");
static_assert(std::is_same<CToReactorT<float *>, Pointer<Float>>::value, "");
static_assert(std::is_same<CToReactorT<uint16_t **>, Pointer<Pointer<UShort>>>::value, "");
static_assert(std::is_same<CToReactorT<uint16_t ***>, Pointer<Pointer<Pointer<UShort>>>>::value, "");

// Assert CToReactorT for unknown pointer types resolves to Pointer<Byte>.
struct S
{};
static_assert(std::is_same<CToReactorT<S *>, Pointer<Byte>>::value, "");
static_assert(std::is_same<CToReactorT<S **>, Pointer<Pointer<Byte>>>::value, "");
static_assert(std::is_same<CToReactorT<S ***>, Pointer<Pointer<Pointer<Byte>>>>::value, "");

// Assert IsRValue<> resolves true for RValue<> types.
static_assert(IsRValue<RValue<Void>>::value, "");
static_assert(IsRValue<RValue<Bool>>::value, "");
static_assert(IsRValue<RValue<Byte>>::value, "");
static_assert(IsRValue<RValue<SByte>>::value, "");
static_assert(IsRValue<RValue<Short>>::value, "");
static_assert(IsRValue<RValue<UShort>>::value, "");
static_assert(IsRValue<RValue<Int>>::value, "");
static_assert(IsRValue<RValue<Long>>::value, "");
static_assert(IsRValue<RValue<UInt>>::value, "");
static_assert(IsRValue<RValue<Float>>::value, "");

// Assert IsLValue<> resolves true for LValue types.
static_assert(IsLValue<Bool>::value, "");
static_assert(IsLValue<Byte>::value, "");
static_assert(IsLValue<SByte>::value, "");
static_assert(IsLValue<Short>::value, "");
static_assert(IsLValue<UShort>::value, "");
static_assert(IsLValue<Int>::value, "");
static_assert(IsLValue<Long>::value, "");
static_assert(IsLValue<UInt>::value, "");
static_assert(IsLValue<Float>::value, "");

// Assert IsReference<> resolves true for Reference types.
static_assert(IsReference<Reference<Bool>>::value, "");
static_assert(IsReference<Reference<Byte>>::value, "");
static_assert(IsReference<Reference<SByte>>::value, "");
static_assert(IsReference<Reference<Short>>::value, "");
static_assert(IsReference<Reference<UShort>>::value, "");
static_assert(IsReference<Reference<Int>>::value, "");
static_assert(IsReference<Reference<Long>>::value, "");
static_assert(IsReference<Reference<UInt>>::value, "");
static_assert(IsReference<Reference<Float>>::value, "");

// Assert IsRValue<> resolves false for LValue types.
static_assert(!IsRValue<Void>::value, "");
static_assert(!IsRValue<Bool>::value, "");
static_assert(!IsRValue<Byte>::value, "");
static_assert(!IsRValue<SByte>::value, "");
static_assert(!IsRValue<Short>::value, "");
static_assert(!IsRValue<UShort>::value, "");
static_assert(!IsRValue<Int>::value, "");
static_assert(!IsRValue<Long>::value, "");
static_assert(!IsRValue<UInt>::value, "");
static_assert(!IsRValue<Float>::value, "");

// Assert IsRValue<> resolves false for Reference types.
static_assert(!IsRValue<Reference<Void>>::value, "");
static_assert(!IsRValue<Reference<Bool>>::value, "");
static_assert(!IsRValue<Reference<Byte>>::value, "");
static_assert(!IsRValue<Reference<SByte>>::value, "");
static_assert(!IsRValue<Reference<Short>>::value, "");
static_assert(!IsRValue<Reference<UShort>>::value, "");
static_assert(!IsRValue<Reference<Int>>::value, "");
static_assert(!IsRValue<Reference<Long>>::value, "");
static_assert(!IsRValue<Reference<UInt>>::value, "");
static_assert(!IsRValue<Reference<Float>>::value, "");

// Assert IsRValue<> resolves false for C types.
static_assert(!IsRValue<void>::value, "");
static_assert(!IsRValue<bool>::value, "");
static_assert(!IsRValue<uint8_t>::value, "");
static_assert(!IsRValue<int8_t>::value, "");
static_assert(!IsRValue<int16_t>::value, "");
static_assert(!IsRValue<uint16_t>::value, "");
static_assert(!IsRValue<int32_t>::value, "");
static_assert(!IsRValue<uint64_t>::value, "");
static_assert(!IsRValue<uint32_t>::value, "");
static_assert(!IsRValue<float>::value, "");

// Assert IsLValue<> resolves false for RValue<> types.
static_assert(!IsLValue<RValue<Void>>::value, "");
static_assert(!IsLValue<RValue<Bool>>::value, "");
static_assert(!IsLValue<RValue<Byte>>::value, "");
static_assert(!IsLValue<RValue<SByte>>::value, "");
static_assert(!IsLValue<RValue<Short>>::value, "");
static_assert(!IsLValue<RValue<UShort>>::value, "");
static_assert(!IsLValue<RValue<Int>>::value, "");
static_assert(!IsLValue<RValue<Long>>::value, "");
static_assert(!IsLValue<RValue<UInt>>::value, "");
static_assert(!IsLValue<RValue<Float>>::value, "");

// Assert IsLValue<> resolves false for Void type.
static_assert(!IsLValue<Void>::value, "");

// Assert IsLValue<> resolves false for Reference<> types.
static_assert(!IsLValue<Reference<Void>>::value, "");
static_assert(!IsLValue<Reference<Bool>>::value, "");
static_assert(!IsLValue<Reference<Byte>>::value, "");
static_assert(!IsLValue<Reference<SByte>>::value, "");
static_assert(!IsLValue<Reference<Short>>::value, "");
static_assert(!IsLValue<Reference<UShort>>::value, "");
static_assert(!IsLValue<Reference<Int>>::value, "");
static_assert(!IsLValue<Reference<Long>>::value, "");
static_assert(!IsLValue<Reference<UInt>>::value, "");
static_assert(!IsLValue<Reference<Float>>::value, "");

// Assert IsLValue<> resolves false for C types.
static_assert(!IsLValue<void>::value, "");
static_assert(!IsLValue<bool>::value, "");
static_assert(!IsLValue<uint8_t>::value, "");
static_assert(!IsLValue<int8_t>::value, "");
static_assert(!IsLValue<int16_t>::value, "");
static_assert(!IsLValue<uint16_t>::value, "");
static_assert(!IsLValue<int32_t>::value, "");
static_assert(!IsLValue<uint64_t>::value, "");
static_assert(!IsLValue<uint32_t>::value, "");
static_assert(!IsLValue<float>::value, "");

// Assert IsDefined<> resolves true for RValue<> types.
static_assert(IsDefined<RValue<Void>>::value, "");
static_assert(IsDefined<RValue<Bool>>::value, "");
static_assert(IsDefined<RValue<Byte>>::value, "");
static_assert(IsDefined<RValue<SByte>>::value, "");
static_assert(IsDefined<RValue<Short>>::value, "");
static_assert(IsDefined<RValue<UShort>>::value, "");
static_assert(IsDefined<RValue<Int>>::value, "");
static_assert(IsDefined<RValue<Long>>::value, "");
static_assert(IsDefined<RValue<UInt>>::value, "");
static_assert(IsDefined<RValue<Float>>::value, "");

// Assert IsDefined<> resolves true for LValue types.
static_assert(IsDefined<Void>::value, "");
static_assert(IsDefined<Bool>::value, "");
static_assert(IsDefined<Byte>::value, "");
static_assert(IsDefined<SByte>::value, "");
static_assert(IsDefined<Short>::value, "");
static_assert(IsDefined<UShort>::value, "");
static_assert(IsDefined<Int>::value, "");
static_assert(IsDefined<Long>::value, "");
static_assert(IsDefined<UInt>::value, "");
static_assert(IsDefined<Float>::value, "");

// Assert IsDefined<> resolves true for Reference<> types.
static_assert(IsDefined<Reference<Bool>>::value, "");
static_assert(IsDefined<Reference<Byte>>::value, "");
static_assert(IsDefined<Reference<SByte>>::value, "");
static_assert(IsDefined<Reference<Short>>::value, "");
static_assert(IsDefined<Reference<UShort>>::value, "");
static_assert(IsDefined<Reference<Int>>::value, "");
static_assert(IsDefined<Reference<Long>>::value, "");
static_assert(IsDefined<Reference<UInt>>::value, "");
static_assert(IsDefined<Reference<Float>>::value, "");

// Assert IsDefined<> resolves true for C types.
static_assert(IsDefined<void>::value, "");
static_assert(IsDefined<bool>::value, "");
static_assert(IsDefined<uint8_t>::value, "");
static_assert(IsDefined<int8_t>::value, "");
static_assert(IsDefined<int16_t>::value, "");
static_assert(IsDefined<uint16_t>::value, "");
static_assert(IsDefined<int32_t>::value, "");
static_assert(IsDefined<uint64_t>::value, "");
static_assert(IsDefined<uint32_t>::value, "");
static_assert(IsDefined<float>::value, "");
