// Copyright 2019 The SwiftShader Authors. All Rights Reserved.
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
#include "System/Half.hpp"
#include "System/Math.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdlib>
#include <cmath>

using std::isnan;
using std::isinf;
using std::signbit;

using namespace sw;

// Implementation of frexp() which satisfies C++ <cmath> requirements.
float fast_frexp(float val, int *exp)
{
	int isNotZero = (val != 0.0f) ? 0xFFFFFFFF : 0x00000000;
	int v = bit_cast<int>(val);
	int isInfOrNaN = (v & 0x7F800000) == 0x7F800000 ? 0xFFFFFFFF : 0x00000000;

	// When val is a subnormal value we can't directly use its mantissa to construct the significand in
	// the range [0.5, 1.0). We need to multiply it by a factor that makes it normalized. For large
	// values the factor must avoid overflow to inifity.
	int factor = ((127 + 23) << 23) - (v & 0x3F800000);
	int nval = bit_cast<int>(val * bit_cast<float>(factor));

	// Extract the exponent of the normalized value and subtract the exponent of the normalizing factor.
	int exponent = ((((nval & 0x7F800000) - factor) >> 23) + 1) & isNotZero;

	// Substitute the exponent of 0.5f (if not zero) to obtain the significand.
	float significand = bit_cast<float>((nval & 0x807FFFFF) | (0x3F000000 & isNotZero) | (0x7F800000 & isInfOrNaN));

	*exp = exponent;
	return significand;
}

TEST(MathTest, Frexp)
{
	for(bool flush : { false, true })
	{
		CPUID::setDenormalsAreZero(flush);
		CPUID::setFlushToZero(flush);

		std::vector<float> a = {
			2.3f,
			0.1f,
			0.7f,
			1.7f,
			0.0f,
			-2.3f,
			-0.1f,
			-0.7f,
			-1.7f,
			-0.0f,
			100000000.0f,
			-100000000.0f,
			0.000000001f,
			-0.000000001f,
			FLT_MIN,
			-FLT_MIN,
			FLT_MAX,
			-FLT_MAX,
			FLT_TRUE_MIN,
			-FLT_TRUE_MIN,
			INFINITY,
			-INFINITY,
			NAN,
			bit_cast<float>(0x007FFFFF),  // Largest subnormal
			bit_cast<float>(0x807FFFFF),
			bit_cast<float>(0x00000001),  // Smallest subnormal
			bit_cast<float>(0x80000001),
		};

		for(float f : a)
		{
			int exp = -1000;
			float sig = fast_frexp(f, &exp);

			if(f == 0.0f)  // Could be subnormal if `flush` is true
			{
				// We don't rely on std::frexp here to produce a reference result because it may
				// return non-zero significands and exponents for subnormal arguments., while our
				// implementation is meant to respect denormals-are-zero / flush-to-zero.

				ASSERT_EQ(sig, 0.0f) << "Argument: " << std::hexfloat << f;
				ASSERT_TRUE(signbit(sig) == signbit(f)) << "Argument: " << std::hexfloat << f;
				ASSERT_EQ(exp, 0) << "Argument: " << std::hexfloat << f;
			}
			else
			{
				int ref_exp = -1000;
				float ref_sig = std::frexp(f, &ref_exp);

				if(!isnan(f))
				{
					ASSERT_EQ(sig, ref_sig) << "Argument: " << std::hexfloat << f;
				}
				else
				{
					ASSERT_TRUE(isnan(sig)) << "Significand: " << std::hexfloat << sig;
				}

				if(!isinf(f) && !isnan(f))  // If the argument is NaN or Inf the exponent is unspecified.
				{
					ASSERT_EQ(exp, ref_exp) << "Argument: " << std::hexfloat << f;
				}
			}
		}
	}
}

// Returns the whole-number ULP error of `a` relative to `x`.
// Use the doouble-precision version below. This just illustrates the principle.
[[deprecated]] float ULP_32(float x, float a)
{
	// Flip the last mantissa bit to compute the 'unit in the last place' error.
	float x1 = bit_cast<float>(bit_cast<uint32_t>(x) ^ 0x00000001);
	float ulp = abs(x1 - x);

	return abs(a - x) / ulp;
}

double ULP_32(double x, double a)
{
	// binary64 has 52 mantissa bits, while binary32 has 23, so the ULP for the latter is 29 bits shifted.
	double x1 = bit_cast<double>(bit_cast<uint64_t>(x) ^ 0x0000000020000000ull);
	double ulp = abs(x1 - x);

	return abs(a - x) / ulp;
}

float ULP_16(float x, float a)
{
	// binary32 has 23 mantissa bits, while binary16 has 10, so the ULP for the latter is 13 bits shifted.
	double x1 = bit_cast<float>(bit_cast<uint32_t>(x) ^ 0x00002000);
	float ulp = abs(x1 - x);

	return abs(a - x) / ulp;
}

// lolremez --float -d 2 -r "0:2^23" "(log2(x/2^23+1)-x/2^23)/x" "1/x"
// ULP-16: 0.797363281, abs: 0.0991751999
float f(float x)
{
	float u = 2.8017103e-22f;
	u = u * x + -8.373131e-15f;
	return u * x + 5.0615534e-8f;
}

float Log2Relaxed(float x)
{
	// Reinterpretation as an integer provides a piecewise linear
	// approximation of log2(). Scale to the radix and subtract exponent bias.
	int im = bit_cast<int>(x);
	float y = (float)im * (1.0f / (1 << 23)) - 127.0f;

	// Handle log2(inf) = inf.
	if(im == 0x7F800000) y = INFINITY;

	float m = (float)(im & 0x007FFFFF);  // Unnormalized mantissa of x.

	// Add a polynomial approximation of log2(m+1)-m to the result's mantissa.
	return f(m) * m + y;
}

TEST(MathTest, Log2RelaxedExhaustive)
{
	CPUID::setDenormalsAreZero(true);
	CPUID::setFlushToZero(true);

	float worst_margin = 0;
	float worst_ulp = 0;
	float worst_x = 0;
	float worst_val = 0;
	float worst_ref = 0;

	float worst_abs = 0;

	for(float x = 0.0f; x <= INFINITY; x = inc(x))
	{
		float val = Log2Relaxed(x);

		double ref = log2((double)x);

		if(ref == (int)ref)
		{
			ASSERT_EQ(val, ref);
		}
		else if(x >= 0.5f && x <= 2.0f)
		{
			const float tolerance = pow(2.0f, -7.0f);  // Absolute

			float margin = abs(val - ref) / tolerance;

			if(margin > worst_abs)
			{
				worst_abs = margin;
			}
		}
		else
		{
			const float tolerance = 3;  // ULP

			float ulp = (float)ULP_16(ref, (double)val);
			float margin = ulp / tolerance;

			if(margin > worst_margin)
			{
				worst_margin = margin;
				worst_ulp = ulp;
				worst_x = x;
				worst_val = val;
				worst_ref = ref;
			}
		}
	}

	ASSERT_TRUE(worst_margin < 1.0f) << " worst_x " << worst_x << " worst_val " << worst_val << " worst_ref " << worst_ref << " worst_ulp " << worst_ulp;
	ASSERT_TRUE(worst_abs <= 1.0f) << " worst_x " << worst_x << " worst_val " << worst_val << " worst_ref " << worst_ref << " worst_ulp " << worst_ulp;

	CPUID::setDenormalsAreZero(false);
	CPUID::setFlushToZero(false);
}

// lolremez --float -d 2 -r "0:1" "(2^x-x-1)/x" "1/x"
// ULP-16: 0.130859017
float Pr(float x)
{
	float u = 7.8145574e-2f;
	u = u * x + 2.2617357e-1f;
	return u * x + -3.0444314e-1f;
}

float Exp2Relaxed(float x)
{
	x = min(x, 128.0f);
	x = max(x, bit_cast<float>(int(0xC2FDFFFF)));  // -126.999992

	// 2^f - f - 1 as P(f) * f
	// This is a correction term to be added to 1+x to obtain 2^x.
	float f = x - floor(x);
	float y = Pr(f) * f + x;

	// bit_cast<float>(int(x * 2^23)) is a piecewise linear approximation of 2^(x-127).
	// See "Fast Exponential Computation on SIMD Architectures" by Malossi et al.
	return bit_cast<float>(int((1 << 23) * y + (127 << 23)));
}

TEST(MathTest, Exp2RelaxedExhaustive)
{
	CPUID::setDenormalsAreZero(true);
	CPUID::setFlushToZero(true);

	float worst_margin = 0;
	float worst_ulp = 0;
	float worst_x = 0;
	float worst_val = 0;
	float worst_ref = 0;

	for(float x = -10; x <= 10; x = inc(x))
	{
		float val = Exp2Relaxed(x);

		double ref = exp2((double)x);

		if(x == (int)x)
		{
			ASSERT_EQ(val, ref);
		}

		const float tolerance = (1 + 2 * abs(x));
		float ulp = ULP_16((float)ref, val);
		float margin = ulp / tolerance;

		if(margin > worst_margin)
		{
			worst_margin = margin;
			worst_ulp = ulp;
			worst_x = x;
			worst_val = val;
			worst_ref = ref;
		}
	}

	ASSERT_TRUE(worst_margin <= 1.0f) << " worst_x " << worst_x << " worst_val " << worst_val << " worst_ref " << worst_ref << " worst_ulp " << worst_ulp;

	CPUID::setDenormalsAreZero(false);
	CPUID::setFlushToZero(false);
}

// lolremez --float -d 7 -r "0:1" "(log2(x+1)-x)/x" "1/x"
// ULP-32: 1.69571960, abs: 0.360798746
float Pl(float x)
{
	float u = -9.3091638e-3f;
	u = u * x + 5.2059003e-2f;
	u = u * x + -1.3752135e-1f;
	u = u * x + 2.4186478e-1f;
	u = u * x + -3.4730109e-1f;
	u = u * x + 4.786837e-1f;
	u = u * x + -7.2116581e-1f;
	return u * x + 4.4268988e-1f;
}

float Log2(float x)
{
	// Reinterpretation as an integer provides a piecewise linear
	// approximation of log2(). Scale to the radix and subtract exponent bias.
	int im = bit_cast<int>(x);
	float y = (float)(im - (127 << 23)) * (1.0f / (1 << 23));

	// Handle log2(inf) = inf.
	if(im == 0x7F800000) y = INFINITY;

	float m = (float)(im & 0x007FFFFF) * (1.0f / (1 << 23));  // Normalized mantissa of x.

	// Add a polynomial approximation of log2(m+1)-m to the result's mantissa.
	return Pl(m) * m + y;
}

TEST(MathTest, Log2Exhaustive)
{
	CPUID::setDenormalsAreZero(true);
	CPUID::setFlushToZero(true);

	float worst_margin = 0;
	float worst_ulp = 0;
	float worst_x = 0;
	float worst_val = 0;
	float worst_ref = 0;

	float worst_abs = 0;

	for(float x = 0.0f; x <= INFINITY; x = inc(x))
	{
		float val = Log2(x);

		double ref = log2((double)x);

		if(ref == (int)ref)
		{
			ASSERT_EQ(val, ref);
		}
		else if(x >= 0.5f && x <= 2.0f)
		{
			const float tolerance = pow(2.0f, -21.0f);  // Absolute

			float margin = abs(val - ref) / tolerance;

			if(margin > worst_abs)
			{
				worst_abs = margin;
			}
		}
		else
		{
			const float tolerance = 3;  // ULP

			float ulp = (float)ULP_32(ref, (double)val);
			float margin = ulp / tolerance;

			if(margin > worst_margin)
			{
				worst_margin = margin;
				worst_ulp = ulp;
				worst_x = x;
				worst_val = val;
				worst_ref = ref;
			}
		}
	}

	ASSERT_TRUE(worst_margin < 1.0f) << " worst_x " << worst_x << " worst_val " << worst_val << " worst_ref " << worst_ref << " worst_ulp " << worst_ulp;
	ASSERT_TRUE(worst_abs <= 1.0f) << " worst_x " << worst_x << " worst_val " << worst_val << " worst_ref " << worst_ref << " worst_ulp " << worst_ulp;

	CPUID::setDenormalsAreZero(false);
	CPUID::setFlushToZero(false);
}

// lolremez --float -d 4 -r "0:1" "(2^x-x-1)/x" "1/x"
// ULP_32: 2.14694786, Vulkan margin: 0.686957061
float P(float x)
{
	float u = 1.8852974e-3f;
	u = u * x + 8.9733787e-3f;
	u = u * x + 5.5835927e-2f;
	u = u * x + 2.4015281e-1f;
	return u * x + -3.0684753e-1f;
}

float Exp2(float x)
{
	x = min(x, 128.0f);
	x = max(x, bit_cast<float>(0xC2FDFFFF));  // -126.999992

	// 2^f - f - 1 as P(f) * f
	// This is a correction term to be added to 1+x to obtain 2^x.
	float f = x - floor(x);
	float y = P(f) * f + x;

	// bit_cast<float>(int(x * 2^23)) is a piecewise linear approximation of 2^(x-127).
	// See "Fast Exponential Computation on SIMD Architectures" by Malossi et al.
	return bit_cast<float>(int(y * (1 << 23)) + (127 << 23));
}

TEST(MathTest, Exp2Exhaustive)
{
	CPUID::setDenormalsAreZero(true);
	CPUID::setFlushToZero(true);

	float worst_margin = 0;
	float worst_ulp = 0;
	float worst_x = 0;
	float worst_val = 0;
	float worst_ref = 0;

	for(float x = -10; x <= 10; x = inc(x))
	{
		float val = Exp2(x);

		double ref = exp2((double)x);

		if(x == (int)x)
		{
			ASSERT_EQ(val, ref);
		}

		const float tolerance = (3 + 2 * abs(x));
		float ulp = (float)ULP_32(ref, (double)val);
		float margin = ulp / tolerance;

		if(margin > worst_margin)
		{
			worst_margin = margin;
			worst_ulp = ulp;
			worst_x = x;
			worst_val = val;
			worst_ref = ref;
		}
	}

	ASSERT_TRUE(worst_margin <= 1.0f) << " worst_x " << worst_x << " worst_val " << worst_val << " worst_ref " << worst_ref << " worst_ulp " << worst_ulp;

	CPUID::setDenormalsAreZero(false);
	CPUID::setFlushToZero(false);
}

// Polynomial approximation of order 5 for sin(x * 2 * pi) in the range [-1/4, 1/4]
static float sin5(float x)
{
	// A * x^5 + B * x^3 + C * x
	// Exact at x = 0, 1/12, 1/6, 1/4, and their negatives, which correspond to x * 2 * pi = 0, pi/6, pi/3, pi/2
	const float A = (36288 - 20736 * sqrt(3)) / 5;
	const float B = 288 * sqrt(3) - 540;
	const float C = (47 - 9 * sqrt(3)) / 5;

	float x2 = x * x;

	return ((A * x2 + B) * x2 + C) * x;
}

TEST(MathTest, SinExhaustive)
{
	const float tolerance = powf(2.0f, -12.0f);  // Vulkan requires absolute error <= 2^−11 inside the range [−pi, pi]
	const float pi = 3.1415926535f;

	for(float x = -pi; x <= pi; x = inc(x))
	{
		// Range reduction and mirroring
		float x_2 = 0.25f - x * (0.5f / pi);
		float z = 0.25f - fabs(x_2 - round(x_2));

		float val = sin5(z);

		ASSERT_NEAR(val, sinf(x), tolerance);
	}
}

TEST(MathTest, CosExhaustive)
{
	const float tolerance = powf(2.0f, -12.0f);  // Vulkan requires absolute error <= 2^−11 inside the range [−pi, pi]
	const float pi = 3.1415926535f;

	for(float x = -pi; x <= pi; x = inc(x))
	{
		// Phase shift, range reduction, and mirroring
		float x_2 = x * (0.5f / pi);
		float z = 0.25f - fabs(x_2 - round(x_2));

		float val = sin5(z);

		ASSERT_NEAR(val, cosf(x), tolerance);
	}
}

TEST(MathTest, UnsignedFloat11_10)
{
	// Test the largest value which causes underflow to 0, and the smallest value
	// which produces a denormalized result.

	EXPECT_EQ(R11G11B10F::float32ToFloat11(bit_cast<float>(0x3500007F)), 0x0000);
	EXPECT_EQ(R11G11B10F::float32ToFloat11(bit_cast<float>(0x35000080)), 0x0001);

	EXPECT_EQ(R11G11B10F::float32ToFloat10(bit_cast<float>(0x3580003F)), 0x0000);
	EXPECT_EQ(R11G11B10F::float32ToFloat10(bit_cast<float>(0x35800040)), 0x0001);
}

// Clamps to the [0, hi] range. NaN input produces 0, hi must be non-NaN.
float clamp0hi(float x, float hi)
{
	// If x=NaN, x > 0 will compare false and we return 0.
	if(!(x > 0))
	{
		return 0;
	}

	// x is non-NaN at this point, so std::min() is safe for non-NaN hi.
	return std::min(x, hi);
}

unsigned int RGB9E5_reference(float r, float g, float b)
{
	// Vulkan 1.1.117 section 15.2.1 RGB to Shared Exponent Conversion

	// B is the exponent bias (15)
	constexpr int g_sharedexp_bias = 15;

	// N is the number of mantissa bits per component (9)
	constexpr int g_sharedexp_mantissabits = 9;

	// Emax is the maximum allowed biased exponent value (31)
	constexpr int g_sharedexp_maxexponent = 31;

	constexpr float g_sharedexp_max =
	    ((static_cast<float>(1 << g_sharedexp_mantissabits) - 1) /
	     static_cast<float>(1 << g_sharedexp_mantissabits)) *
	    static_cast<float>(1 << (g_sharedexp_maxexponent - g_sharedexp_bias));

	const float red_c = clamp0hi(r, g_sharedexp_max);
	const float green_c = clamp0hi(g, g_sharedexp_max);
	const float blue_c = clamp0hi(b, g_sharedexp_max);

	const float max_c = fmax(fmax(red_c, green_c), blue_c);
	const float exp_p = fmax(-g_sharedexp_bias - 1, floor(log2(max_c))) + 1 + g_sharedexp_bias;
	const int max_s = static_cast<int>(floor((max_c / exp2(exp_p - g_sharedexp_bias - g_sharedexp_mantissabits)) + 0.5f));
	const int exp_s = static_cast<int>((max_s < exp2(g_sharedexp_mantissabits)) ? exp_p : exp_p + 1);

	unsigned int R = static_cast<unsigned int>(floor((red_c / exp2(exp_s - g_sharedexp_bias - g_sharedexp_mantissabits)) + 0.5f));
	unsigned int G = static_cast<unsigned int>(floor((green_c / exp2(exp_s - g_sharedexp_bias - g_sharedexp_mantissabits)) + 0.5f));
	unsigned int B = static_cast<unsigned int>(floor((blue_c / exp2(exp_s - g_sharedexp_bias - g_sharedexp_mantissabits)) + 0.5f));
	unsigned int E = exp_s;

	return (E << 27) | (B << 18) | (G << 9) | R;
}

TEST(MathTest, SharedExponentSparse)
{
	for(uint64_t i = 0; i < 0x0000000100000000; i += 0x400)
	{
		float f = bit_cast<float>(i);

		unsigned int ref = RGB9E5_reference(f, 0.0f, 0.0f);
		unsigned int val = RGB9E5(f, 0.0f, 0.0f);

		EXPECT_EQ(ref, val);
	}
}

TEST(MathTest, SharedExponentRandom)
{
	srand(0);

	unsigned int x = 0;
	unsigned int y = 0;
	unsigned int z = 0;

	for(int i = 0; i < 10000000; i++)
	{
		float r = bit_cast<float>(x);
		float g = bit_cast<float>(y);
		float b = bit_cast<float>(z);

		unsigned int ref = RGB9E5_reference(r, g, b);
		unsigned int val = RGB9E5(r, g, b);

		EXPECT_EQ(ref, val);

		x += rand();
		y += rand();
		z += rand();
	}
}

TEST(MathTest, SharedExponentExhaustive)
{
	for(uint64_t i = 0; i < 0x0000000100000000; i += 1)
	{
		float f = bit_cast<float>(i);

		unsigned int ref = RGB9E5_reference(f, 0.0f, 0.0f);
		unsigned int val = RGB9E5(f, 0.0f, 0.0f);

		EXPECT_EQ(ref, val);
	}
}
