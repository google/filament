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

#include "ShaderCore.hpp"

#include "Device/Renderer.hpp"
#include "Reactor/Assert.hpp"
#include "System/Debug.hpp"

#include <limits.h>

// TODO(chromium:1299047)
#ifndef SWIFTSHADER_LEGACY_PRECISION
#	define SWIFTSHADER_LEGACY_PRECISION false
#endif

namespace sw {

Vector4s::Vector4s()
{
}

Vector4s::Vector4s(unsigned short x, unsigned short y, unsigned short z, unsigned short w)
{
	this->x = Short4(x);
	this->y = Short4(y);
	this->z = Short4(z);
	this->w = Short4(w);
}

Vector4s::Vector4s(const Vector4s &rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	w = rhs.w;
}

Vector4s &Vector4s::operator=(const Vector4s &rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	w = rhs.w;

	return *this;
}

Short4 &Vector4s::operator[](int i)
{
	switch(i)
	{
	case 0: return x;
	case 1: return y;
	case 2: return z;
	case 3: return w;
	}

	return x;
}

Vector4f::Vector4f()
{
}

Vector4f::Vector4f(float x, float y, float z, float w)
{
	this->x = Float4(x);
	this->y = Float4(y);
	this->z = Float4(z);
	this->w = Float4(w);
}

Vector4f::Vector4f(const Vector4f &rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	w = rhs.w;
}

Vector4f &Vector4f::operator=(const Vector4f &rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	w = rhs.w;

	return *this;
}

Float4 &Vector4f::operator[](int i)
{
	switch(i)
	{
	case 0: return x;
	case 1: return y;
	case 2: return z;
	case 3: return w;
	}

	return x;
}

Vector4i::Vector4i()
{
}

Vector4i::Vector4i(int x, int y, int z, int w)
{
	this->x = Int4(x);
	this->y = Int4(y);
	this->z = Int4(z);
	this->w = Int4(w);
}

Vector4i::Vector4i(const Vector4i &rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	w = rhs.w;
}

Vector4i &Vector4i::operator=(const Vector4i &rhs)
{
	x = rhs.x;
	y = rhs.y;
	z = rhs.z;
	w = rhs.w;

	return *this;
}

Int4 &Vector4i::operator[](int i)
{
	switch(i)
	{
	case 0: return x;
	case 1: return y;
	case 2: return z;
	case 3: return w;
	}

	return x;
}

// Approximation of atan in [0..1]
static RValue<SIMD::Float> Atan_01(SIMD::Float x)
{
	// From 4.4.49, page 81 of the Handbook of Mathematical Functions, by Milton Abramowitz and Irene Stegun
	const SIMD::Float a2(-0.3333314528f);
	const SIMD::Float a4(0.1999355085f);
	const SIMD::Float a6(-0.1420889944f);
	const SIMD::Float a8(0.1065626393f);
	const SIMD::Float a10(-0.0752896400f);
	const SIMD::Float a12(0.0429096138f);
	const SIMD::Float a14(-0.0161657367f);
	const SIMD::Float a16(0.0028662257f);
	SIMD::Float x2 = x * x;
	return (x + x * (x2 * (a2 + x2 * (a4 + x2 * (a6 + x2 * (a8 + x2 * (a10 + x2 * (a12 + x2 * (a14 + x2 * a16)))))))));
}

// Polynomial approximation of order 5 for sin(x * 2 * pi) in the range [-1/4, 1/4]
static RValue<SIMD::Float> Sin5(SIMD::Float x)
{
	// A * x^5 + B * x^3 + C * x
	// Exact at x = 0, 1/12, 1/6, 1/4, and their negatives, which correspond to x * 2 * pi = 0, pi/6, pi/3, pi/2
	const SIMD::Float A = (36288 - 20736 * sqrt(3)) / 5;
	const SIMD::Float B = 288 * sqrt(3) - 540;
	const SIMD::Float C = (47 - 9 * sqrt(3)) / 5;

	SIMD::Float x2 = x * x;

	return MulAdd(MulAdd(A, x2, B), x2, C) * x;
}

RValue<SIMD::Float> Sin(RValue<SIMD::Float> x, bool relaxedPrecision)
{
	const SIMD::Float q = 0.25f;
	const SIMD::Float pi2 = 1 / (2 * 3.1415926535f);

	// Range reduction and mirroring
	SIMD::Float x_2 = MulAdd(x, -pi2, q);
	SIMD::Float z = q - Abs(x_2 - Round(x_2));

	return Sin5(z);
}

RValue<SIMD::Float> Cos(RValue<SIMD::Float> x, bool relaxedPrecision)
{
	const SIMD::Float q = 0.25f;
	const SIMD::Float pi2 = 1 / (2 * 3.1415926535f);

	// Phase shift, range reduction, and mirroring
	SIMD::Float x_2 = x * pi2;
	SIMD::Float z = q - Abs(x_2 - Round(x_2));

	return Sin5(z);
}

RValue<SIMD::Float> Tan(RValue<SIMD::Float> x, bool relaxedPrecision)
{
	return Sin(x, relaxedPrecision) / Cos(x, relaxedPrecision);
}

static RValue<SIMD::Float> Asin_4_terms(RValue<SIMD::Float> x)
{
	// From 4.4.45, page 81 of the Handbook of Mathematical Functions, by Milton Abramowitz and Irene Stegun
	// |e(x)| <= 5e-8
	const SIMD::Float half_pi(1.57079632f);
	const SIMD::Float a0(1.5707288f);
	const SIMD::Float a1(-0.2121144f);
	const SIMD::Float a2(0.0742610f);
	const SIMD::Float a3(-0.0187293f);
	SIMD::Float absx = Abs(x);
	return As<SIMD::Float>(As<SIMD::Int>(half_pi - Sqrt<Highp>(1.0f - absx) * (a0 + absx * (a1 + absx * (a2 + absx * a3)))) ^
	                       (As<SIMD::Int>(x) & SIMD::Int(0x80000000)));
}

static RValue<SIMD::Float> Asin_8_terms(RValue<SIMD::Float> x)
{
	// From 4.4.46, page 81 of the Handbook of Mathematical Functions, by Milton Abramowitz and Irene Stegun
	// |e(x)| <= 0e-8
	const SIMD::Float half_pi(1.5707963268f);
	const SIMD::Float a0(1.5707963050f);
	const SIMD::Float a1(-0.2145988016f);
	const SIMD::Float a2(0.0889789874f);
	const SIMD::Float a3(-0.0501743046f);
	const SIMD::Float a4(0.0308918810f);
	const SIMD::Float a5(-0.0170881256f);
	const SIMD::Float a6(0.006700901f);
	const SIMD::Float a7(-0.0012624911f);
	SIMD::Float absx = Abs(x);
	return As<SIMD::Float>(As<SIMD::Int>(half_pi - Sqrt<Highp>(1.0f - absx) * (a0 + absx * (a1 + absx * (a2 + absx * (a3 + absx * (a4 + absx * (a5 + absx * (a6 + absx * a7)))))))) ^
	                       (As<SIMD::Int>(x) & SIMD::Int(0x80000000)));
}

RValue<SIMD::Float> Asin(RValue<SIMD::Float> x, bool relaxedPrecision)
{
	// TODO(b/169755566): Surprisingly, deqp-vk's precision.acos.highp/mediump tests pass when using the 4-term polynomial
	// approximation version of acos, unlike for Asin, which requires higher precision algorithms.

	if(!relaxedPrecision)
	{
		return Asin(x);
	}

	return Asin_8_terms(x);
}

RValue<SIMD::Float> Acos(RValue<SIMD::Float> x, bool relaxedPrecision)
{
	// pi/2 - arcsin(x)
	return 1.57079632e+0f - Asin_4_terms(x);
}

RValue<SIMD::Float> Atan(RValue<SIMD::Float> x, bool relaxedPrecision)
{
	SIMD::Float absx = Abs(x);
	SIMD::Int O = CmpNLT(absx, 1.0f);
	SIMD::Float y = As<SIMD::Float>((O & As<SIMD::Int>(1.0f / absx)) | (~O & As<SIMD::Int>(absx)));  // FIXME: Vector select

	const SIMD::Float half_pi(1.57079632f);
	SIMD::Float theta = Atan_01(y);
	return As<SIMD::Float>(((O & As<SIMD::Int>(half_pi - theta)) | (~O & As<SIMD::Int>(theta))) ^  // FIXME: Vector select
	                       (As<SIMD::Int>(x) & SIMD::Int(0x80000000)));
}

RValue<SIMD::Float> Atan2(RValue<SIMD::Float> y, RValue<SIMD::Float> x, bool relaxedPrecision)
{
	const SIMD::Float pi(3.14159265f);             // pi
	const SIMD::Float minus_pi(-3.14159265f);      // -pi
	const SIMD::Float half_pi(1.57079632f);        // pi/2
	const SIMD::Float quarter_pi(7.85398163e-1f);  // pi/4

	// Rotate to upper semicircle when in lower semicircle
	SIMD::Int S = CmpLT(y, 0.0f);
	SIMD::Float theta = As<SIMD::Float>(S & As<SIMD::Int>(minus_pi));
	SIMD::Float x0 = As<SIMD::Float>((As<SIMD::Int>(y) & SIMD::Int(0x80000000)) ^ As<SIMD::Int>(x));
	SIMD::Float y0 = Abs(y);

	// Rotate to right quadrant when in left quadrant
	SIMD::Int Q = CmpLT(x0, 0.0f);
	theta += As<SIMD::Float>(Q & As<SIMD::Int>(half_pi));
	SIMD::Float x1 = As<SIMD::Float>((Q & As<SIMD::Int>(y0)) | (~Q & As<SIMD::Int>(x0)));   // FIXME: Vector select
	SIMD::Float y1 = As<SIMD::Float>((Q & As<SIMD::Int>(-x0)) | (~Q & As<SIMD::Int>(y0)));  // FIXME: Vector select

	// Mirror to first octant when in second octant
	SIMD::Int O = CmpNLT(y1, x1);
	SIMD::Float x2 = As<SIMD::Float>((O & As<SIMD::Int>(y1)) | (~O & As<SIMD::Int>(x1)));  // FIXME: Vector select
	SIMD::Float y2 = As<SIMD::Float>((O & As<SIMD::Int>(x1)) | (~O & As<SIMD::Int>(y1)));  // FIXME: Vector select

	// Approximation of atan in [0..1]
	SIMD::Int zero_x = CmpEQ(x2, 0.0f);
	SIMD::Int inf_y = IsInf(y2);  // Since x2 >= y2, this means x2 == y2 == inf, so we use 45 degrees or pi/4
	SIMD::Float atan2_theta = Atan_01(y2 / x2);
	theta += As<SIMD::Float>((~zero_x & ~inf_y & ((O & As<SIMD::Int>(half_pi - atan2_theta)) | (~O & (As<SIMD::Int>(atan2_theta))))) |  // FIXME: Vector select
	                         (inf_y & As<SIMD::Int>(quarter_pi)));

	// Recover loss of precision for tiny theta angles
	// This combination results in (-pi + half_pi + half_pi - atan2_theta) which is equivalent to -atan2_theta
	SIMD::Int precision_loss = S & Q & O & ~inf_y;

	return As<SIMD::Float>((precision_loss & As<SIMD::Int>(-atan2_theta)) | (~precision_loss & As<SIMD::Int>(theta)));  // FIXME: Vector select
}

// TODO(chromium:1299047)
static RValue<SIMD::Float> Exp2_legacy(RValue<SIMD::Float> x0)
{
	SIMD::Int i = RoundInt(x0 - 0.5f);
	SIMD::Float ii = As<SIMD::Float>((i + SIMD::Int(127)) << 23);

	SIMD::Float f = x0 - SIMD::Float(i);
	SIMD::Float ff = As<SIMD::Float>(SIMD::Int(0x3AF61905));
	ff = ff * f + As<SIMD::Float>(SIMD::Int(0x3C134806));
	ff = ff * f + As<SIMD::Float>(SIMD::Int(0x3D64AA23));
	ff = ff * f + As<SIMD::Float>(SIMD::Int(0x3E75EAD4));
	ff = ff * f + As<SIMD::Float>(SIMD::Int(0x3F31727B));
	ff = ff * f + 1.0f;

	return ii * ff;
}

RValue<SIMD::Float> Exp2(RValue<SIMD::Float> x, bool relaxedPrecision)
{
	// Clamp to prevent overflow past the representation of infinity.
	SIMD::Float x0 = x;
	x0 = Min(x0, 128.0f);
	x0 = Max(x0, As<SIMD::Float>(SIMD::Int(0xC2FDFFFF)));  // -126.999992

	if(SWIFTSHADER_LEGACY_PRECISION)  // TODO(chromium:1299047)
	{
		return Exp2_legacy(x0);
	}

	SIMD::Float xi = Floor(x0);
	SIMD::Float f = x0 - xi;

	if(!relaxedPrecision)  // highp
	{
		// Polynomial which approximates (2^x-x-1)/x. Multiplying with x
		// gives us a correction term to be added to 1+x to obtain 2^x.
		const SIMD::Float a = 1.8852974e-3f;
		const SIMD::Float b = 8.9733787e-3f;
		const SIMD::Float c = 5.5835927e-2f;
		const SIMD::Float d = 2.4015281e-1f;
		const SIMD::Float e = -3.0684753e-1f;

		SIMD::Float r = MulAdd(MulAdd(MulAdd(MulAdd(a, f, b), f, c), f, d), f, e);

		// bit_cast<float>(int(x * 2^23)) is a piecewise linear approximation of 2^x.
		// See "Fast Exponential Computation on SIMD Architectures" by Malossi et al.
		SIMD::Float y = MulAdd(r, f, x0);
		SIMD::Int i = SIMD::Int(y * (1 << 23)) + (127 << 23);

		return As<SIMD::Float>(i);
	}
	else  // RelaxedPrecision / mediump
	{
		// Polynomial which approximates (2^x-x-1)/x. Multiplying with x
		// gives us a correction term to be added to 1+x to obtain 2^x.
		const SIMD::Float a = 7.8145574e-2f;
		const SIMD::Float b = 2.2617357e-1f;
		const SIMD::Float c = -3.0444314e-1f;

		SIMD::Float r = MulAdd(MulAdd(a, f, b), f, c);

		// bit_cast<float>(int(x * 2^23)) is a piecewise linear approximation of 2^x.
		// See "Fast Exponential Computation on SIMD Architectures" by Malossi et al.
		SIMD::Float y = MulAdd(r, f, x0);
		SIMD::Int i = SIMD::Int(MulAdd((1 << 23), y, (127 << 23)));

		return As<SIMD::Float>(i);
	}
}

RValue<SIMD::Float> Log2_legacy(RValue<SIMD::Float> x)
{
	SIMD::Float x1 = As<SIMD::Float>(As<SIMD::Int>(x) & SIMD::Int(0x7F800000));
	x1 = As<SIMD::Float>(As<SIMD::UInt>(x1) >> 8);
	x1 = As<SIMD::Float>(As<SIMD::Int>(x1) | As<SIMD::Int>(SIMD::Float(1.0f)));
	x1 = (x1 - 1.4960938f) * 256.0f;
	SIMD::Float x0 = As<SIMD::Float>((As<SIMD::Int>(x) & SIMD::Int(0x007FFFFF)) | As<SIMD::Int>(SIMD::Float(1.0f)));

	SIMD::Float x2 = MulAdd(MulAdd(9.5428179e-2f, x0, 4.7779095e-1f), x0, 1.9782813e-1f);
	SIMD::Float x3 = MulAdd(MulAdd(MulAdd(1.6618466e-2f, x0, 2.0350508e-1f), x0, 2.7382900e-1f), x0, 4.0496687e-2f);

	x1 += (x0 - 1.0f) * (x2 / x3);

	SIMD::Int pos_inf_x = CmpEQ(As<SIMD::Int>(x), SIMD::Int(0x7F800000));
	return As<SIMD::Float>((pos_inf_x & As<SIMD::Int>(x)) | (~pos_inf_x & As<SIMD::Int>(x1)));
}

RValue<SIMD::Float> Log2(RValue<SIMD::Float> x, bool relaxedPrecision)
{
	if(SWIFTSHADER_LEGACY_PRECISION)  // TODO(chromium:1299047)
	{
		return Log2_legacy(x);
	}

	if(!relaxedPrecision)  // highp
	{
		// Reinterpretation as an integer provides a piecewise linear
		// approximation of log2(). Scale to the radix and subtract exponent bias.
		SIMD::Int im = As<SIMD::Int>(x);
		SIMD::Float y = SIMD::Float(im - (127 << 23)) * (1.0f / (1 << 23));

		// Handle log2(inf) = inf.
		y = As<SIMD::Float>(As<SIMD::Int>(y) | (CmpEQ(im, 0x7F800000) & As<SIMD::Int>(SIMD::Float::infinity())));

		SIMD::Float m = SIMD::Float(im & 0x007FFFFF) * (1.0f / (1 << 23));  // Normalized mantissa of x.

		// Add a polynomial approximation of log2(m+1)-m to the result's mantissa.
		const SIMD::Float a = -9.3091638e-3f;
		const SIMD::Float b = 5.2059003e-2f;
		const SIMD::Float c = -1.3752135e-1f;
		const SIMD::Float d = 2.4186478e-1f;
		const SIMD::Float e = -3.4730109e-1f;
		const SIMD::Float f = 4.786837e-1f;
		const SIMD::Float g = -7.2116581e-1f;
		const SIMD::Float h = 4.4268988e-1f;

		SIMD::Float z = MulAdd(MulAdd(MulAdd(MulAdd(MulAdd(MulAdd(MulAdd(a, m, b), m, c), m, d), m, e), m, f), m, g), m, h);

		return MulAdd(z, m, y);
	}
	else  // RelaxedPrecision / mediump
	{
		// Reinterpretation as an integer provides a piecewise linear
		// approximation of log2(). Scale to the radix and subtract exponent bias.
		SIMD::Int im = As<SIMD::Int>(x);
		SIMD::Float y = MulAdd(SIMD::Float(im), (1.0f / (1 << 23)), -127.0f);

		// Handle log2(inf) = inf.
		y = As<SIMD::Float>(As<SIMD::Int>(y) | (CmpEQ(im, 0x7F800000) & As<SIMD::Int>(SIMD::Float::infinity())));

		SIMD::Float m = SIMD::Float(im & 0x007FFFFF);  // Unnormalized mantissa of x.

		// Add a polynomial approximation of log2(m+1)-m to the result's mantissa.
		const SIMD::Float a = 2.8017103e-22f;
		const SIMD::Float b = -8.373131e-15f;
		const SIMD::Float c = 5.0615534e-8f;

		SIMD::Float f = MulAdd(MulAdd(a, m, b), m, c);

		return MulAdd(f, m, y);
	}
}

RValue<SIMD::Float> Exp(RValue<SIMD::Float> x, bool relaxedPrecision)
{
	return Exp2(1.44269504f * x, relaxedPrecision);  // 1/ln(2)
}

RValue<SIMD::Float> Log(RValue<SIMD::Float> x, bool relaxedPrecision)
{
	return 6.93147181e-1f * Log2(x, relaxedPrecision);  // ln(2)
}

RValue<SIMD::Float> Pow(RValue<SIMD::Float> x, RValue<SIMD::Float> y, bool relaxedPrecision)
{
	SIMD::Float log = Log2(x, relaxedPrecision);
	log *= y;
	return Exp2(log, relaxedPrecision);
}

RValue<SIMD::Float> Sinh(RValue<SIMD::Float> x, bool relaxedPrecision)
{
	return (Exp(x, relaxedPrecision) - Exp(-x, relaxedPrecision)) * 0.5f;
}

RValue<SIMD::Float> Cosh(RValue<SIMD::Float> x, bool relaxedPrecision)
{
	return (Exp(x, relaxedPrecision) + Exp(-x, relaxedPrecision)) * 0.5f;
}

RValue<SIMD::Float> Tanh(RValue<SIMD::Float> x, bool relaxedPrecision)
{
	SIMD::Float e_x = Exp(x, relaxedPrecision);
	SIMD::Float e_minus_x = Exp(-x, relaxedPrecision);
	return (e_x - e_minus_x) / (e_x + e_minus_x);
}

RValue<SIMD::Float> Asinh(RValue<SIMD::Float> x, bool relaxedPrecision)
{
	return Log(x + Sqrt(x * x + 1.0f, relaxedPrecision), relaxedPrecision);
}

RValue<SIMD::Float> Acosh(RValue<SIMD::Float> x, bool relaxedPrecision)
{
	return Log(x + Sqrt(x + 1.0f, relaxedPrecision) * Sqrt(x - 1.0f, relaxedPrecision), relaxedPrecision);
}

RValue<SIMD::Float> Atanh(RValue<SIMD::Float> x, bool relaxedPrecision)
{
	return Log((1.0f + x) / (1.0f - x), relaxedPrecision) * 0.5f;
}

RValue<SIMD::Float> Sqrt(RValue<SIMD::Float> x, bool relaxedPrecision)
{
	return Sqrt(x);  // TODO(b/222218659): Optimize for relaxed precision.
}

std::pair<SIMD::Float, SIMD::Int> Frexp(RValue<SIMD::Float> val)
{
	// Assumes IEEE 754
	auto isNotZero = CmpNEQ(val, 0.0f);
	auto v = As<SIMD::Int>(val);
	auto significand = As<SIMD::Float>((v & 0x807FFFFF) | (0x3F000000 & isNotZero));

	auto exponent = (((v >> 23) & 0xFF) - 126) & isNotZero;

	return std::make_pair(significand, exponent);
}

RValue<SIMD::Float> Ldexp(RValue<SIMD::Float> significand, RValue<SIMD::Int> exponent)
{
	// "load exponent"
	// Ldexp(significand,exponent) computes
	//     significand * 2**exponent
	// with edge case handling as permitted by the spec.
	//
	// The interesting cases are:
	// - significand is subnormal and the exponent is positive. The mantissa
	//   bits of the significand shift left. The result *may* be normal, and
	//   in that case the leading 1 bit in the mantissa is no longer explicitly
	//   represented. Computing the result with bit operations would be quite
	//   complex.
	// - significand has very small magnitude, and exponent is large.
	//   Example:  significand = 0x1p-125,  exponent = 250, result 0x1p125
	//   If we compute the result directly with the reference formula, then
	//   the intermediate value 2.0**exponent overflows, and then the result
	//   would overflow. Instead, it is sufficient to split the exponent
	//   and use two multiplies:
	//       (significand * 2**(exponent/2)) * (2**(exponent - exponent/2))
	//   In this formulation, the intermediates will not overflow when the
	//   correct result does not overflow. Also, this method naturally handles
	//   underflows, infinities, and NaNs.
	//
	// This implementation uses the two-multiplies approach described above,
	// and also used by Mesa.
	//
	// The SPIR-V GLSL.std.450 extended instruction spec says:
	//
	//  if exponent < -126 the result may be flushed to zero
	//  if exponent > 128 the result may be undefined
	//
	// Clamping exponent to [-254,254] allows us implement well beyond
	// what is required by the spec, but still use simple algorithms.
	//
	// We decompose as follows:
	//        2 ** exponent = powA * powB
	// where
	//        powA = 2 ** (exponent / 2)
	//        powB = 2 ** (exponent - exponent / 2)
	//
	// We use a helper expression to compute these powers of two as float
	// numbers using bit shifts, where X is an unbiased integer exponent
	// in range [-127,127]:
	//
	//        pow2i(X) = As<SIMD::Float>((X + 127)<<23)
	//
	// This places the biased exponent into position, and places all
	// zeroes in the mantissa bit positions. The implicit 1 bit in the
	// mantissa is hidden. When X = -127, the result is float 0.0, as
	// if the value was flushed to zero. Otherwise X is in [-126,127]
	// and the biased exponent is in [1,254] and the result is a normal
	// float number with value 2**X.
	//
	// So we have:
	//
	//        powA = pow2i(exponent/2)
	//        powB = pow2i(exponent - exponent/2)
	//
	// With exponent in [-254,254], we split into cases:
	//
	//     exponent = -254:
	//        exponent/2 = -127
	//        exponent - exponent/2 = -127
	//        powA = pow2i(exponent/2) = pow2i(-127) = 0.0
	//        powA * powB is 0.0, which is a permitted flush-to-zero case.
	//
	//     exponent = -253:
	//        exponent/2 = -126
	//        (exponent - exponent/2) = -127
	//        powB = pow2i(exponent - exponent/2) = pow2i(-127) = 0.0
	//        powA * powB is 0.0, which is a permitted flush-to-zero case.
	//
	//     exponent in [-252,254]:
	//        exponent/2 is in [-126, 127]
	//        (exponent - exponent/2) is in [-126, 127]
	//
	//        powA = pow2i(exponent/2), a normal number
	//        powB = pow2i(exponent - exponent/2), a normal number
	//
	// For the Mesa implementation, see
	// https://gitlab.freedesktop.org/mesa/mesa/-/blob/1eb7a85b55f0c7c2de6f5dac7b5f6209a6eb401c/src/compiler/nir/nir_opt_algebraic.py#L2241

	// Clamp exponent to limits
	auto exp = Min(Max(exponent, -254), 254);

	// Split exponent into two terms
	auto expA = exp >> 1;
	auto expB = exp - expA;
	// Construct two powers of 2 with the exponents above
	auto powA = As<SIMD::Float>((expA + 127) << 23);
	auto powB = As<SIMD::Float>((expB + 127) << 23);

	// Multiply the input value by the two powers to get the final value.
	// Note that multiplying powA and powB together may result in an overflow,
	// so ensure that significand is multiplied by powA, *then* the result of that with powB.
	return (significand * powA) * powB;
}

UInt4 halfToFloatBits(RValue<UInt4> halfBits)
{
	auto magic = UInt4(126 << 23);

	auto sign16 = halfBits & UInt4(0x8000);
	auto man16 = halfBits & UInt4(0x03FF);
	auto exp16 = halfBits & UInt4(0x7C00);

	auto isDnormOrZero = CmpEQ(exp16, UInt4(0));
	auto isInfOrNaN = CmpEQ(exp16, UInt4(0x7C00));

	auto sign32 = sign16 << 16;
	auto man32 = man16 << 13;
	auto exp32 = (exp16 + UInt4(0x1C000)) << 13;
	auto norm32 = (man32 | exp32) | (isInfOrNaN & UInt4(0x7F800000));

	auto denorm32 = As<UInt4>(As<Float4>(magic + man16) - As<Float4>(magic));

	return sign32 | (norm32 & ~isDnormOrZero) | (denorm32 & isDnormOrZero);
}

UInt4 floatToHalfBits(RValue<UInt4> floatBits, bool storeInUpperBits)
{
	UInt4 sign = floatBits & UInt4(0x80000000);
	UInt4 abs = floatBits & UInt4(0x7FFFFFFF);

	UInt4 normal = CmpNLE(abs, UInt4(0x38800000));

	UInt4 mantissa = (abs & UInt4(0x007FFFFF)) | UInt4(0x00800000);
	UInt4 e = UInt4(113) - (abs >> 23);
	UInt4 denormal = CmpLT(e, UInt4(24)) & (mantissa >> e);

	UInt4 base = (normal & abs) | (~normal & denormal);  // TODO: IfThenElse()

	// float exponent bias is 127, half bias is 15, so adjust by -112
	UInt4 bias = normal & UInt4(0xC8000000);

	UInt4 rounded = base + bias + UInt4(0x00000FFF) + ((base >> 13) & UInt4(1));
	UInt4 fp16u = rounded >> 13;

	// Infinity
	fp16u |= CmpNLE(abs, UInt4(0x47FFEFFF)) & UInt4(0x7FFF);

	return storeInUpperBits ? (sign | (fp16u << 16)) : ((sign >> 16) | fp16u);
}

SIMD::Float linearToSRGB(const SIMD::Float &c)
{
	SIMD::Float lc = c * 12.92f;
	SIMD::Float ec = MulAdd(1.055f, Pow<Mediump>(c, (1.0f / 2.4f)), -0.055f);  // TODO(b/149574741): Use a custom approximation.

	SIMD::Int linear = CmpLT(c, 0.0031308f);
	return As<SIMD::Float>((linear & As<SIMD::Int>(lc)) | (~linear & As<SIMD::Int>(ec)));  // TODO: IfThenElse()
}

SIMD::Float sRGBtoLinear(const SIMD::Float &c)
{
	SIMD::Float lc = c * (1.0f / 12.92f);
	SIMD::Float ec = Pow<Mediump>(MulAdd(c, 1.0f / 1.055f, 0.055f / 1.055f), 2.4f);  // TODO(b/149574741): Use a custom approximation.

	SIMD::Int linear = CmpLT(c, 0.04045f);
	return As<SIMD::Float>((linear & As<SIMD::Int>(lc)) | (~linear & As<SIMD::Int>(ec)));  // TODO: IfThenElse()
}

RValue<Float4> reciprocal(RValue<Float4> x, bool pp, bool exactAtPow2)
{
	return Rcp(x, pp, exactAtPow2);
}

RValue<SIMD::Float> reciprocal(RValue<SIMD::Float> x, bool pp, bool exactAtPow2)
{
	return Rcp(x, pp, exactAtPow2);
}

RValue<Float4> reciprocalSquareRoot(RValue<Float4> x, bool absolute, bool pp)
{
	Float4 abs = x;

	if(absolute)
	{
		abs = Abs(abs);
	}

	return Rcp(abs, pp);
}

// TODO(chromium:1299047): Eliminate when Chromium tests accept both fused and unfused multiply-add.
RValue<SIMD::Float> mulAdd(RValue<SIMD::Float> x, RValue<SIMD::Float> y, RValue<SIMD::Float> z)
{
	if(SWIFTSHADER_LEGACY_PRECISION)
	{
		return x * y + z;
	}

	return MulAdd(x, y, z);
}

RValue<Float4> Pow(RValue<Float4> x, RValue<Float4> y, bool relaxedPrecision)
{
	// TODO(b/214588983): Eliminate by using only the wide SIMD variant (or specialize or templatize the implementation).
	SIMD::Float xx;
	SIMD::Float yy;
	xx = Insert128(xx, x, 0);
	yy = Insert128(yy, y, 0);
	return Extract128(Pow(xx, yy, relaxedPrecision), 0);
}

RValue<Float4> Sqrt(RValue<Float4> x, bool relaxedPrecision)
{
	// TODO(b/214588983): Eliminate by using only the wide SIMD variant (or specialize or templatize the implementation).
	SIMD::Float xx;
	xx = Insert128(xx, x, 0);
	return Extract128(Sqrt(xx, relaxedPrecision), 0);
}

void transpose4x4(Short4 &row0, Short4 &row1, Short4 &row2, Short4 &row3)
{
	Int2 tmp0 = UnpackHigh(row0, row1);
	Int2 tmp1 = UnpackHigh(row2, row3);
	Int2 tmp2 = UnpackLow(row0, row1);
	Int2 tmp3 = UnpackLow(row2, row3);

	row0 = UnpackLow(tmp2, tmp3);
	row1 = UnpackHigh(tmp2, tmp3);
	row2 = UnpackLow(tmp0, tmp1);
	row3 = UnpackHigh(tmp0, tmp1);
}

void transpose4x3(Short4 &row0, Short4 &row1, Short4 &row2, Short4 &row3)
{
	Int2 tmp0 = UnpackHigh(row0, row1);
	Int2 tmp1 = UnpackHigh(row2, row3);
	Int2 tmp2 = UnpackLow(row0, row1);
	Int2 tmp3 = UnpackLow(row2, row3);

	row0 = UnpackLow(tmp2, tmp3);
	row1 = UnpackHigh(tmp2, tmp3);
	row2 = UnpackLow(tmp0, tmp1);
}

void transpose4x4(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3)
{
	Float4 tmp0 = UnpackLow(row0, row1);
	Float4 tmp1 = UnpackLow(row2, row3);
	Float4 tmp2 = UnpackHigh(row0, row1);
	Float4 tmp3 = UnpackHigh(row2, row3);

	row0 = Float4(tmp0.xy, tmp1.xy);
	row1 = Float4(tmp0.zw, tmp1.zw);
	row2 = Float4(tmp2.xy, tmp3.xy);
	row3 = Float4(tmp2.zw, tmp3.zw);
}

void transpose4x4zyxw(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3)
{
	Float4 tmp0 = UnpackLow(row0, row1);
	Float4 tmp1 = UnpackLow(row2, row3);
	Float4 tmp2 = UnpackHigh(row0, row1);
	Float4 tmp3 = UnpackHigh(row2, row3);

	row2 = Float4(tmp0.xy, tmp1.xy);
	row1 = Float4(tmp0.zw, tmp1.zw);
	row0 = Float4(tmp2.xy, tmp3.xy);
	row3 = Float4(tmp2.zw, tmp3.zw);
}

void transpose4x3(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3)
{
	Float4 tmp0 = UnpackLow(row0, row1);
	Float4 tmp1 = UnpackLow(row2, row3);
	Float4 tmp2 = UnpackHigh(row0, row1);
	Float4 tmp3 = UnpackHigh(row2, row3);

	row0 = Float4(tmp0.xy, tmp1.xy);
	row1 = Float4(tmp0.zw, tmp1.zw);
	row2 = Float4(tmp2.xy, tmp3.xy);
}

void transpose4x2(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3)
{
	Float4 tmp0 = UnpackLow(row0, row1);
	Float4 tmp1 = UnpackLow(row2, row3);

	row0 = Float4(tmp0.xy, tmp1.xy);
	row1 = Float4(tmp0.zw, tmp1.zw);
}

void transpose4x1(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3)
{
	Float4 tmp0 = UnpackLow(row0, row1);
	Float4 tmp1 = UnpackLow(row2, row3);

	row0 = Float4(tmp0.xy, tmp1.xy);
}

void transpose2x4(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3)
{
	Float4 tmp01 = UnpackLow(row0, row1);
	Float4 tmp23 = UnpackHigh(row0, row1);

	row0 = tmp01;
	row1 = Float4(tmp01.zw, row1.zw);
	row2 = tmp23;
	row3 = Float4(tmp23.zw, row3.zw);
}

void transpose4xN(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3, int N)
{
	switch(N)
	{
	case 1: transpose4x1(row0, row1, row2, row3); break;
	case 2: transpose4x2(row0, row1, row2, row3); break;
	case 3: transpose4x3(row0, row1, row2, row3); break;
	case 4: transpose4x4(row0, row1, row2, row3); break;
	}
}

SIMD::UInt halfToFloatBits(SIMD::UInt halfBits)
{
	auto magic = SIMD::UInt(126 << 23);

	auto sign16 = halfBits & SIMD::UInt(0x8000);
	auto man16 = halfBits & SIMD::UInt(0x03FF);
	auto exp16 = halfBits & SIMD::UInt(0x7C00);

	auto isDnormOrZero = CmpEQ(exp16, SIMD::UInt(0));
	auto isInfOrNaN = CmpEQ(exp16, SIMD::UInt(0x7C00));

	auto sign32 = sign16 << 16;
	auto man32 = man16 << 13;
	auto exp32 = (exp16 + SIMD::UInt(0x1C000)) << 13;
	auto norm32 = (man32 | exp32) | (isInfOrNaN & SIMD::UInt(0x7F800000));

	auto denorm32 = As<SIMD::UInt>(As<SIMD::Float>(magic + man16) - As<SIMD::Float>(magic));

	return sign32 | (norm32 & ~isDnormOrZero) | (denorm32 & isDnormOrZero);
}

SIMD::UInt floatToHalfBits(SIMD::UInt floatBits, bool storeInUpperBits)
{
	SIMD::UInt sign = floatBits & SIMD::UInt(0x80000000);
	SIMD::UInt abs = floatBits & SIMD::UInt(0x7FFFFFFF);

	SIMD::UInt normal = CmpNLE(abs, SIMD::UInt(0x38800000));

	SIMD::UInt mantissa = (abs & SIMD::UInt(0x007FFFFF)) | SIMD::UInt(0x00800000);
	SIMD::UInt e = SIMD::UInt(113) - (abs >> 23);
	SIMD::UInt denormal = CmpLT(e, SIMD::UInt(24)) & (mantissa >> e);

	SIMD::UInt base = (normal & abs) | (~normal & denormal);  // TODO: IfThenElse()

	// float exponent bias is 127, half bias is 15, so adjust by -112
	SIMD::UInt bias = normal & SIMD::UInt(0xC8000000);

	SIMD::UInt rounded = base + bias + SIMD::UInt(0x00000FFF) + ((base >> 13) & SIMD::UInt(1));
	SIMD::UInt fp16u = rounded >> 13;

	// Infinity
	fp16u |= CmpNLE(abs, SIMD::UInt(0x47FFEFFF)) & SIMD::UInt(0x7FFF);

	return storeInUpperBits ? (sign | (fp16u << 16)) : ((sign >> 16) | fp16u);
}

Float4 r11g11b10Unpack(UInt r11g11b10bits)
{
	// 10 (or 11) bit float formats are unsigned formats with a 5 bit exponent and a 5 (or 6) bit mantissa.
	// Since the Half float format also has a 5 bit exponent, we can convert these formats to half by
	// copy/pasting the bits so the the exponent bits and top mantissa bits are aligned to the half format.
	// In this case, we have:
	// MSB | B B B B B B B B B B G G G G G G G G G G G R R R R R R R R R R R | LSB
	UInt4 halfBits;
	halfBits = Insert(halfBits, (r11g11b10bits & UInt(0x000007FFu)) << 4, 0);
	halfBits = Insert(halfBits, (r11g11b10bits & UInt(0x003FF800u)) >> 7, 1);
	halfBits = Insert(halfBits, (r11g11b10bits & UInt(0xFFC00000u)) >> 17, 2);
	halfBits = Insert(halfBits, UInt(0x00003C00u), 3);
	return As<Float4>(halfToFloatBits(halfBits));
}

UInt r11g11b10Pack(const Float4 &value)
{
	// 10 and 11 bit floats are unsigned, so their minimal value is 0
	auto halfBits = floatToHalfBits(As<UInt4>(Max(value, Float4(0.0f))), true);
	// Truncates instead of rounding. See b/147900455
	UInt4 truncBits = halfBits & UInt4(0x7FF00000, 0x7FF00000, 0x7FE00000, 0);
	return (UInt(truncBits.x) >> 20) | (UInt(truncBits.y) >> 9) | (UInt(truncBits.z) << 1);
}

Float4 linearToSRGB(const Float4 &c)
{
	Float4 lc = c * 12.92f;
	Float4 ec = MulAdd(1.055f, Pow<Mediump>(c, (1.0f / 2.4f)), -0.055f);  // TODO(b/149574741): Use a custom approximation.

	Int4 linear = CmpLT(c, 0.0031308f);
	return As<Float4>((linear & As<Int4>(lc)) | (~linear & As<Int4>(ec)));  // TODO: IfThenElse()
}

Float4 sRGBtoLinear(const Float4 &c)
{
	Float4 lc = c * (1.0f / 12.92f);
	Float4 ec = Pow<Mediump>(MulAdd(c, 1.0f / 1.055f, 0.055f / 1.055f), 2.4f);  // TODO(b/149574741): Use a custom approximation.

	Int4 linear = CmpLT(c, 0.04045f);
	return As<Float4>((linear & As<Int4>(lc)) | (~linear & As<Int4>(ec)));  // TODO: IfThenElse()
}

rr::RValue<SIMD::Float> Sign(const rr::RValue<SIMD::Float> &val)
{
	return rr::As<SIMD::Float>((rr::As<SIMD::UInt>(val) & SIMD::UInt(0x80000000)) | SIMD::UInt(0x3f800000));
}

// Returns the <whole, frac> of val.
// Both whole and frac will have the same sign as val.
std::pair<rr::RValue<SIMD::Float>, rr::RValue<SIMD::Float>>
Modf(const rr::RValue<SIMD::Float> &val)
{
	auto abs = Abs(val);
	auto sign = Sign(val);
	auto whole = Floor(abs) * sign;
	auto frac = Frac(abs) * sign;
	return std::make_pair(whole, frac);
}

// Returns the number of 1s in bits, per lane.
SIMD::UInt CountBits(const rr::RValue<SIMD::UInt> &bits)
{
	// TODO: Add an intrinsic to reactor. Even if there isn't a
	// single vector instruction, there may be target-dependent
	// ways to make this faster.
	// https://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
	SIMD::UInt c = bits - ((bits >> 1) & SIMD::UInt(0x55555555));
	c = ((c >> 2) & SIMD::UInt(0x33333333)) + (c & SIMD::UInt(0x33333333));
	c = ((c >> 4) + c) & SIMD::UInt(0x0F0F0F0F);
	c = ((c >> 8) + c) & SIMD::UInt(0x00FF00FF);
	c = ((c >> 16) + c) & SIMD::UInt(0x0000FFFF);
	return c;
}

// Returns 1 << bits.
// If the resulting bit overflows a 32 bit integer, 0 is returned.
rr::RValue<SIMD::UInt> NthBit32(const rr::RValue<SIMD::UInt> &bits)
{
	return ((SIMD::UInt(1) << bits) & CmpLT(bits, SIMD::UInt(32)));
}

// Returns bitCount number of of 1's starting from the LSB.
rr::RValue<SIMD::UInt> Bitmask32(const rr::RValue<SIMD::UInt> &bitCount)
{
	return NthBit32(bitCount) - SIMD::UInt(1);
}

// Returns y if y < x; otherwise result is x.
// If one operand is a NaN, the other operand is the result.
// If both operands are NaN, the result is a NaN.
rr::RValue<SIMD::Float> NMin(const rr::RValue<SIMD::Float> &x, const rr::RValue<SIMD::Float> &y)
{
	auto xIsNan = IsNan(x);
	auto yIsNan = IsNan(y);
	return As<SIMD::Float>(
	    // If neither are NaN, return min
	    ((~xIsNan & ~yIsNan) & As<SIMD::Int>(Min(x, y))) |
	    // If one operand is a NaN, the other operand is the result
	    // If both operands are NaN, the result is a NaN.
	    ((~xIsNan & yIsNan) & As<SIMD::Int>(x)) |
	    (xIsNan & As<SIMD::Int>(y)));
}

// Returns y if y > x; otherwise result is x.
// If one operand is a NaN, the other operand is the result.
// If both operands are NaN, the result is a NaN.
rr::RValue<SIMD::Float> NMax(const rr::RValue<SIMD::Float> &x, const rr::RValue<SIMD::Float> &y)
{
	auto xIsNan = IsNan(x);
	auto yIsNan = IsNan(y);
	return As<SIMD::Float>(
	    // If neither are NaN, return max
	    ((~xIsNan & ~yIsNan) & As<SIMD::Int>(Max(x, y))) |
	    // If one operand is a NaN, the other operand is the result
	    // If both operands are NaN, the result is a NaN.
	    ((~xIsNan & yIsNan) & As<SIMD::Int>(x)) |
	    (xIsNan & As<SIMD::Int>(y)));
}

// Returns the determinant of a 2x2 matrix.
rr::RValue<SIMD::Float> Determinant(
    const rr::RValue<SIMD::Float> &a, const rr::RValue<SIMD::Float> &b,
    const rr::RValue<SIMD::Float> &c, const rr::RValue<SIMD::Float> &d)
{
	return a * d - b * c;
}

// Returns the determinant of a 3x3 matrix.
rr::RValue<SIMD::Float> Determinant(
    const rr::RValue<SIMD::Float> &a, const rr::RValue<SIMD::Float> &b, const rr::RValue<SIMD::Float> &c,
    const rr::RValue<SIMD::Float> &d, const rr::RValue<SIMD::Float> &e, const rr::RValue<SIMD::Float> &f,
    const rr::RValue<SIMD::Float> &g, const rr::RValue<SIMD::Float> &h, const rr::RValue<SIMD::Float> &i)
{
	return a * e * i + b * f * g + c * d * h - c * e * g - b * d * i - a * f * h;
}

// Returns the determinant of a 4x4 matrix.
rr::RValue<SIMD::Float> Determinant(
    const rr::RValue<SIMD::Float> &a, const rr::RValue<SIMD::Float> &b, const rr::RValue<SIMD::Float> &c, const rr::RValue<SIMD::Float> &d,
    const rr::RValue<SIMD::Float> &e, const rr::RValue<SIMD::Float> &f, const rr::RValue<SIMD::Float> &g, const rr::RValue<SIMD::Float> &h,
    const rr::RValue<SIMD::Float> &i, const rr::RValue<SIMD::Float> &j, const rr::RValue<SIMD::Float> &k, const rr::RValue<SIMD::Float> &l,
    const rr::RValue<SIMD::Float> &m, const rr::RValue<SIMD::Float> &n, const rr::RValue<SIMD::Float> &o, const rr::RValue<SIMD::Float> &p)
{
	return a * Determinant(f, g, h,
	                       j, k, l,
	                       n, o, p) -
	       b * Determinant(e, g, h,
	                       i, k, l,
	                       m, o, p) +
	       c * Determinant(e, f, h,
	                       i, j, l,
	                       m, n, p) -
	       d * Determinant(e, f, g,
	                       i, j, k,
	                       m, n, o);
}

// Returns the inverse of a 2x2 matrix.
std::array<rr::RValue<SIMD::Float>, 4> MatrixInverse(
    const rr::RValue<SIMD::Float> &a, const rr::RValue<SIMD::Float> &b,
    const rr::RValue<SIMD::Float> &c, const rr::RValue<SIMD::Float> &d)
{
	auto s = SIMD::Float(1.0f) / Determinant(a, b, c, d);
	return { { s * d, -s * b, -s * c, s * a } };
}

// Returns the inverse of a 3x3 matrix.
std::array<rr::RValue<SIMD::Float>, 9> MatrixInverse(
    const rr::RValue<SIMD::Float> &a, const rr::RValue<SIMD::Float> &b, const rr::RValue<SIMD::Float> &c,
    const rr::RValue<SIMD::Float> &d, const rr::RValue<SIMD::Float> &e, const rr::RValue<SIMD::Float> &f,
    const rr::RValue<SIMD::Float> &g, const rr::RValue<SIMD::Float> &h, const rr::RValue<SIMD::Float> &i)
{
	auto s = SIMD::Float(1.0f) / Determinant(
	                                 a, b, c,
	                                 d, e, f,
	                                 g, h, i);  // TODO: duplicate arithmetic calculating the det and below.

	return { {
		s * (e * i - f * h),
		s * (c * h - b * i),
		s * (b * f - c * e),
		s * (f * g - d * i),
		s * (a * i - c * g),
		s * (c * d - a * f),
		s * (d * h - e * g),
		s * (b * g - a * h),
		s * (a * e - b * d),
	} };
}

// Returns the inverse of a 4x4 matrix.
std::array<rr::RValue<SIMD::Float>, 16> MatrixInverse(
    const rr::RValue<SIMD::Float> &a, const rr::RValue<SIMD::Float> &b, const rr::RValue<SIMD::Float> &c, const rr::RValue<SIMD::Float> &d,
    const rr::RValue<SIMD::Float> &e, const rr::RValue<SIMD::Float> &f, const rr::RValue<SIMD::Float> &g, const rr::RValue<SIMD::Float> &h,
    const rr::RValue<SIMD::Float> &i, const rr::RValue<SIMD::Float> &j, const rr::RValue<SIMD::Float> &k, const rr::RValue<SIMD::Float> &l,
    const rr::RValue<SIMD::Float> &m, const rr::RValue<SIMD::Float> &n, const rr::RValue<SIMD::Float> &o, const rr::RValue<SIMD::Float> &p)
{
	auto s = SIMD::Float(1.0f) / Determinant(
	                                 a, b, c, d,
	                                 e, f, g, h,
	                                 i, j, k, l,
	                                 m, n, o, p);  // TODO: duplicate arithmetic calculating the det and below.

	auto kplo = k * p - l * o, jpln = j * p - l * n, jokn = j * o - k * n;
	auto gpho = g * p - h * o, fphn = f * p - h * n, fogn = f * o - g * n;
	auto glhk = g * l - h * k, flhj = f * l - h * j, fkgj = f * k - g * j;
	auto iplm = i * p - l * m, iokm = i * o - k * m, ephm = e * p - h * m;
	auto eogm = e * o - g * m, elhi = e * l - h * i, ekgi = e * k - g * i;
	auto injm = i * n - j * m, enfm = e * n - f * m, ejfi = e * j - f * i;

	return { {
		s * (f * kplo - g * jpln + h * jokn),
		s * (-b * kplo + c * jpln - d * jokn),
		s * (b * gpho - c * fphn + d * fogn),
		s * (-b * glhk + c * flhj - d * fkgj),

		s * (-e * kplo + g * iplm - h * iokm),
		s * (a * kplo - c * iplm + d * iokm),
		s * (-a * gpho + c * ephm - d * eogm),
		s * (a * glhk - c * elhi + d * ekgi),

		s * (e * jpln - f * iplm + h * injm),
		s * (-a * jpln + b * iplm - d * injm),
		s * (a * fphn - b * ephm + d * enfm),
		s * (-a * flhj + b * elhi - d * ejfi),

		s * (-e * jokn + f * iokm - g * injm),
		s * (a * jokn - b * iokm + c * injm),
		s * (-a * fogn + b * eogm - c * enfm),
		s * (a * fkgj - b * ekgi + c * ejfi),
	} };
}

}  // namespace sw
