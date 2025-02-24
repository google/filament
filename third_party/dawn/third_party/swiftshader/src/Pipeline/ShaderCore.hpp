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

#ifndef sw_ShaderCore_hpp
#define sw_ShaderCore_hpp

#include "Reactor/Print.hpp"
#include "Reactor/Reactor.hpp"
#include "Reactor/SIMD.hpp"
#include "System/Debug.hpp"

#include <array>
#include <atomic>   // std::memory_order
#include <utility>  // std::pair

namespace sw {

using namespace rr;

class Vector4s
{
public:
	Vector4s();
	Vector4s(unsigned short x, unsigned short y, unsigned short z, unsigned short w);
	Vector4s(const Vector4s &rhs);

	Short4 &operator[](int i);
	Vector4s &operator=(const Vector4s &rhs);

	Short4 x;
	Short4 y;
	Short4 z;
	Short4 w;
};

class Vector4f
{
public:
	Vector4f();
	Vector4f(float x, float y, float z, float w);
	Vector4f(const Vector4f &rhs);

	Float4 &operator[](int i);
	Vector4f &operator=(const Vector4f &rhs);

	Float4 x;
	Float4 y;
	Float4 z;
	Float4 w;
};

class Vector4i
{
public:
	Vector4i();
	Vector4i(int x, int y, int z, int w);
	Vector4i(const Vector4i &rhs);

	Int4 &operator[](int i);
	Vector4i &operator=(const Vector4i &rhs);

	Int4 x;
	Int4 y;
	Int4 z;
	Int4 w;
};

namespace SIMD {

using namespace rr::SIMD;

struct Float4
{
	SIMD::Float x;
	SIMD::Float y;
	SIMD::Float z;
	SIMD::Float w;
};

struct Int4
{
	SIMD::Int x;
	SIMD::Int y;
	SIMD::Int z;
	SIMD::Int w;
};

}  // namespace SIMD

// Vulkan 'SPIR-V Extended Instructions for GLSL' (GLSL.std.450) compliant transcendental functions
RValue<SIMD::Float> Sin(RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Cos(RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Tan(RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Asin(RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Acos(RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Atan(RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Atan2(RValue<SIMD::Float> y, RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Exp2(RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Log2(RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Exp(RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Log(RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Pow(RValue<SIMD::Float> x, RValue<SIMD::Float> y, bool relaxedPrecision);
RValue<SIMD::Float> Sinh(RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Cosh(RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Tanh(RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Asinh(RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Acosh(RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Atanh(RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Sqrt(RValue<SIMD::Float> x, bool relaxedPrecision);

// Splits x into a floating-point significand in the range [0.5, 1.0)
// and an integral exponent of two, such that:
//   x = significand * 2^exponent
// Returns the pair <significand, exponent>
std::pair<SIMD::Float, SIMD::Int> Frexp(RValue<SIMD::Float> val);

RValue<SIMD::Float> Ldexp(RValue<SIMD::Float> significand, RValue<SIMD::Int> exponent);

// Math functions with uses outside of shaders can be invoked using a verbose template argument instead
// of a Boolean argument to indicate precision. For example Sqrt<Mediump>(x) equals Sqrt(x, true).
enum Precision
{
	Highp,
	Relaxed,
	Mediump = Relaxed,  // GLSL defines mediump and lowp as corresponding with SPIR-V's RelaxedPrecision
};

// clang-format off
template<Precision precision> RValue<SIMD::Float> Pow(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
template<> inline RValue<SIMD::Float> Pow<Highp>(RValue<SIMD::Float> x, RValue<SIMD::Float> y) { return Pow(x, y, false); }
template<> inline RValue<SIMD::Float> Pow<Mediump>(RValue<SIMD::Float> x, RValue<SIMD::Float> y) { return Pow(x, y, true); }

template<Precision precision> RValue<SIMD::Float> Sqrt(RValue<SIMD::Float> x);
template<> inline RValue<SIMD::Float> Sqrt<Highp>(RValue<SIMD::Float> x) { return Sqrt(x, false); }
template<> inline RValue<SIMD::Float> Sqrt<Mediump>(RValue<SIMD::Float> x) { return Sqrt(x, true); }
// clang-format on

SIMD::UInt halfToFloatBits(SIMD::UInt halfBits);
SIMD::UInt floatToHalfBits(SIMD::UInt floatBits, bool storeInUpperBits);
SIMD::Float linearToSRGB(const SIMD::Float &c);
SIMD::Float sRGBtoLinear(const SIMD::Float &c);

RValue<Float4> reciprocal(RValue<Float4> x, bool pp = false, bool exactAtPow2 = false);
RValue<SIMD::Float> reciprocal(RValue<SIMD::Float> x, bool pp = false, bool exactAtPow2 = false);
RValue<Float4> reciprocalSquareRoot(RValue<Float4> x, bool abs, bool pp = false);

RValue<SIMD::Float> mulAdd(RValue<SIMD::Float> x, RValue<SIMD::Float> y, RValue<SIMD::Float> z);  // TODO(chromium:1299047)

RValue<Float4> Pow(RValue<Float4> x, RValue<Float4> y, bool relaxedPrecision);
RValue<Float4> Sqrt(RValue<Float4> x, bool relaxedPrecision);

// clang-format off
template<Precision precision> RValue<Float4> Pow(RValue<Float4> x, RValue<Float4> y);
template<> inline RValue<Float4> Pow<Highp>(RValue<Float4> x, RValue<Float4> y) { return Pow(x, y, false); }
template<> inline RValue<Float4> Pow<Mediump>(RValue<Float4> x, RValue<Float4> y) { return Pow(x, y, true); }

template<Precision precision> RValue<Float4> Sqrt(RValue<Float4> x);
template<> inline RValue<Float4> Sqrt<Highp>(RValue<Float4> x) { return Sqrt(x, false); }
template<> inline RValue<Float4> Sqrt<Mediump>(RValue<Float4> x) { return Sqrt(x, true); }
// clang-format on

void transpose4x4(Short4 &row0, Short4 &row1, Short4 &row2, Short4 &row3);
void transpose4x3(Short4 &row0, Short4 &row1, Short4 &row2, Short4 &row3);
void transpose4x4(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3);
void transpose4x4zyxw(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3);
void transpose4x3(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3);
void transpose4x2(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3);
void transpose4x1(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3);
void transpose2x4(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3);
void transpose4xN(Float4 &row0, Float4 &row1, Float4 &row2, Float4 &row3, int N);

UInt4 halfToFloatBits(RValue<UInt4> halfBits);
UInt4 floatToHalfBits(RValue<UInt4> floatBits, bool storeInUpperBits);
Float4 r11g11b10Unpack(UInt r11g11b10bits);
UInt r11g11b10Pack(const Float4 &value);
Float4 linearToSRGB(const Float4 &c);
Float4 sRGBtoLinear(const Float4 &c);

template<typename T>
inline rr::RValue<T> AndAll(const rr::RValue<T> &mask);

template<typename T>
inline rr::RValue<T> OrAll(const rr::RValue<T> &mask);

rr::RValue<SIMD::Float> Sign(const rr::RValue<SIMD::Float> &val);

// Returns the <whole, frac> of val.
// Both whole and frac will have the same sign as val.
std::pair<rr::RValue<SIMD::Float>, rr::RValue<SIMD::Float>>
Modf(const rr::RValue<SIMD::Float> &val);

// Returns the number of 1s in bits, per lane.
SIMD::UInt CountBits(const rr::RValue<SIMD::UInt> &bits);

// Returns 1 << bits.
// If the resulting bit overflows a 32 bit integer, 0 is returned.
rr::RValue<SIMD::UInt> NthBit32(const rr::RValue<SIMD::UInt> &bits);

// Returns bitCount number of of 1's starting from the LSB.
rr::RValue<SIMD::UInt> Bitmask32(const rr::RValue<SIMD::UInt> &bitCount);

// Computes `a * b + c`, which may be fused into one operation to produce a higher-precision result.
rr::RValue<SIMD::Float> FMA(
    const rr::RValue<SIMD::Float> &a,
    const rr::RValue<SIMD::Float> &b,
    const rr::RValue<SIMD::Float> &c);

// Returns y if y < x; otherwise result is x.
// If one operand is a NaN, the other operand is the result.
// If both operands are NaN, the result is a NaN.
rr::RValue<SIMD::Float> NMin(const rr::RValue<SIMD::Float> &x, const rr::RValue<SIMD::Float> &y);

// Returns y if y > x; otherwise result is x.
// If one operand is a NaN, the other operand is the result.
// If both operands are NaN, the result is a NaN.
rr::RValue<SIMD::Float> NMax(const rr::RValue<SIMD::Float> &x, const rr::RValue<SIMD::Float> &y);

// Returns the determinant of a 2x2 matrix.
rr::RValue<SIMD::Float> Determinant(
    const rr::RValue<SIMD::Float> &a, const rr::RValue<SIMD::Float> &b,
    const rr::RValue<SIMD::Float> &c, const rr::RValue<SIMD::Float> &d);

// Returns the determinant of a 3x3 matrix.
rr::RValue<SIMD::Float> Determinant(
    const rr::RValue<SIMD::Float> &a, const rr::RValue<SIMD::Float> &b, const rr::RValue<SIMD::Float> &c,
    const rr::RValue<SIMD::Float> &d, const rr::RValue<SIMD::Float> &e, const rr::RValue<SIMD::Float> &f,
    const rr::RValue<SIMD::Float> &g, const rr::RValue<SIMD::Float> &h, const rr::RValue<SIMD::Float> &i);

// Returns the determinant of a 4x4 matrix.
rr::RValue<SIMD::Float> Determinant(
    const rr::RValue<SIMD::Float> &a, const rr::RValue<SIMD::Float> &b, const rr::RValue<SIMD::Float> &c, const rr::RValue<SIMD::Float> &d,
    const rr::RValue<SIMD::Float> &e, const rr::RValue<SIMD::Float> &f, const rr::RValue<SIMD::Float> &g, const rr::RValue<SIMD::Float> &h,
    const rr::RValue<SIMD::Float> &i, const rr::RValue<SIMD::Float> &j, const rr::RValue<SIMD::Float> &k, const rr::RValue<SIMD::Float> &l,
    const rr::RValue<SIMD::Float> &m, const rr::RValue<SIMD::Float> &n, const rr::RValue<SIMD::Float> &o, const rr::RValue<SIMD::Float> &p);

// Returns the inverse of a 2x2 matrix.
std::array<rr::RValue<SIMD::Float>, 4> MatrixInverse(
    const rr::RValue<SIMD::Float> &a, const rr::RValue<SIMD::Float> &b,
    const rr::RValue<SIMD::Float> &c, const rr::RValue<SIMD::Float> &d);

// Returns the inverse of a 3x3 matrix.
std::array<rr::RValue<SIMD::Float>, 9> MatrixInverse(
    const rr::RValue<SIMD::Float> &a, const rr::RValue<SIMD::Float> &b, const rr::RValue<SIMD::Float> &c,
    const rr::RValue<SIMD::Float> &d, const rr::RValue<SIMD::Float> &e, const rr::RValue<SIMD::Float> &f,
    const rr::RValue<SIMD::Float> &g, const rr::RValue<SIMD::Float> &h, const rr::RValue<SIMD::Float> &i);

// Returns the inverse of a 4x4 matrix.
std::array<rr::RValue<SIMD::Float>, 16> MatrixInverse(
    const rr::RValue<SIMD::Float> &a, const rr::RValue<SIMD::Float> &b, const rr::RValue<SIMD::Float> &c, const rr::RValue<SIMD::Float> &d,
    const rr::RValue<SIMD::Float> &e, const rr::RValue<SIMD::Float> &f, const rr::RValue<SIMD::Float> &g, const rr::RValue<SIMD::Float> &h,
    const rr::RValue<SIMD::Float> &i, const rr::RValue<SIMD::Float> &j, const rr::RValue<SIMD::Float> &k, const rr::RValue<SIMD::Float> &l,
    const rr::RValue<SIMD::Float> &m, const rr::RValue<SIMD::Float> &n, const rr::RValue<SIMD::Float> &o, const rr::RValue<SIMD::Float> &p);

////////////////////////////////////////////////////////////////////////////
// Inline functions
////////////////////////////////////////////////////////////////////////////

template<typename T>
inline rr::RValue<T> AndAll(const rr::RValue<T> &mask)
{
	T v1 = mask;               // [x]    [y]    [z]    [w]
	T v2 = v1.xzxz & v1.ywyw;  // [xy]   [zw]   [xy]   [zw]
	return v2.xxxx & v2.yyyy;  // [xyzw] [xyzw] [xyzw] [xyzw]
}

template<typename T>
inline rr::RValue<T> OrAll(const rr::RValue<T> &mask)
{
	T v1 = mask;               // [x]    [y]    [z]    [w]
	T v2 = v1.xzxz | v1.ywyw;  // [xy]   [zw]   [xy]   [zw]
	return v2.xxxx | v2.yyyy;  // [xyzw] [xyzw] [xyzw] [xyzw]
}

}  // namespace sw

#ifdef ENABLE_RR_PRINT
namespace rr {
template<>
struct PrintValue::Ty<sw::Vector4f>
{
	static std::string fmt(const sw::Vector4f &v)
	{
		return "[x: " + PrintValue::fmt(v.x) +
		       ", y: " + PrintValue::fmt(v.y) +
		       ", z: " + PrintValue::fmt(v.z) +
		       ", w: " + PrintValue::fmt(v.w) + "]";
	}

	static std::vector<rr::Value *> val(const sw::Vector4f &v)
	{
		return PrintValue::vals(v.x, v.y, v.z, v.w);
	}
};
template<>
struct PrintValue::Ty<sw::Vector4s>
{
	static std::string fmt(const sw::Vector4s &v)
	{
		return "[x: " + PrintValue::fmt(v.x) +
		       ", y: " + PrintValue::fmt(v.y) +
		       ", z: " + PrintValue::fmt(v.z) +
		       ", w: " + PrintValue::fmt(v.w) + "]";
	}

	static std::vector<rr::Value *> val(const sw::Vector4s &v)
	{
		return PrintValue::vals(v.x, v.y, v.z, v.w);
	}
};
template<>
struct PrintValue::Ty<sw::Vector4i>
{
	static std::string fmt(const sw::Vector4i &v)
	{
		return "[x: " + PrintValue::fmt(v.x) +
		       ", y: " + PrintValue::fmt(v.y) +
		       ", z: " + PrintValue::fmt(v.z) +
		       ", w: " + PrintValue::fmt(v.w) + "]";
	}

	static std::vector<rr::Value *> val(const sw::Vector4i &v)
	{
		return PrintValue::vals(v.x, v.y, v.z, v.w);
	}
};
}  // namespace rr
#endif  // ENABLE_RR_PRINT

#endif  // sw_ShaderCore_hpp
