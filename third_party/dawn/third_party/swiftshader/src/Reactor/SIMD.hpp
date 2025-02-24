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

#ifndef rr_SIMD_hpp
#define rr_SIMD_hpp

#include "Reactor.hpp"

#include <functional>
#include <vector>

namespace rr {

namespace scalar {
using Int = rr::Int;
using UInt = rr::UInt;
using Float = rr::Float;
template<class T>
using Pointer = rr::Pointer<T>;
}  // namespace scalar

namespace packed {
using Int4 = rr::Int4;
using UInt4 = rr::UInt4;
using Float4 = rr::Float4;
}  // namespace packed

namespace SIMD {

extern const int Width;

class Int;
class UInt;
class Float;
class Pointer;

class Int : public LValue<SIMD::Int>,
            public XYZW<SIMD::Int>  // TODO(b/214583550): Eliminate and replace with SwizzleQuad() and/or other intrinsics.
{
public:
	explicit Int(RValue<SIMD::Float> cast);

	Int();
	Int(int broadcast);
	Int(int x, int y, int z, int w);
	Int(std::vector<int> v);
	Int(std::function<int(int)> LaneValueProducer);
	Int(RValue<SIMD::Int> rhs);
	Int(const Int &rhs);
	Int(const Reference<SIMD::Int> &rhs);
	Int(RValue<SIMD::UInt> rhs);
	Int(const UInt &rhs);
	Int(const Reference<SIMD::UInt> &rhs);
	Int(RValue<scalar::Int> rhs);
	Int(const scalar::Int &rhs);
	Int(const Reference<scalar::Int> &rhs);

	template<int T>
	Int(const SwizzleMask1<packed::Int4, T> &rhs);

	RValue<SIMD::Int> operator=(int broadcast);
	RValue<SIMD::Int> operator=(RValue<SIMD::Int> rhs);
	RValue<SIMD::Int> operator=(const Int &rhs);
	RValue<SIMD::Int> operator=(const Reference<SIMD::Int> &rhs);

	static Type *type();
	static int element_count() { return SIMD::Width; }
};

class UInt : public LValue<SIMD::UInt>,
             public XYZW<SIMD::UInt>  // TODO(b/214583550): Eliminate and replace with SwizzleQuad() and/or other intrinsics.
{
public:
	explicit UInt(RValue<SIMD::Float> cast);

	UInt();
	UInt(int broadcast);
	UInt(int x, int y, int z, int w);
	UInt(std::vector<int> v);
	UInt(std::function<int(int)> LaneValueProducer);
	UInt(RValue<SIMD::UInt> rhs);
	UInt(const UInt &rhs);
	UInt(const Reference<SIMD::UInt> &rhs);
	UInt(RValue<SIMD::Int> rhs);
	UInt(const Int &rhs);
	UInt(const Reference<SIMD::Int> &rhs);
	UInt(RValue<scalar::UInt> rhs);
	UInt(const scalar::UInt &rhs);
	UInt(const Reference<scalar::UInt> &rhs);

	RValue<SIMD::UInt> operator=(RValue<SIMD::UInt> rhs);
	RValue<SIMD::UInt> operator=(const UInt &rhs);
	RValue<SIMD::UInt> operator=(const Reference<SIMD::UInt> &rhs);

	static Type *type();
	static int element_count() { return SIMD::Width; }
};

class Float : public LValue<SIMD::Float>,
              public XYZW<SIMD::Float>  // TODO(b/214583550): Eliminate and replace with SwizzleQuad() and/or other intrinsics.
{
public:
	explicit Float(RValue<SIMD::Int> cast);
	explicit Float(RValue<SIMD::UInt> cast);

	Float();
	Float(float broadcast);
	Float(float x, float y, float z, float w);
	Float(std::vector<float> v);
	Float(std::function<float(int)> LaneValueProducer);
	Float(RValue<SIMD::Float> rhs);
	Float(const Float &rhs);
	Float(const Reference<SIMD::Float> &rhs);
	Float(RValue<scalar::Float> rhs);
	Float(const scalar::Float &rhs);
	Float(const Reference<scalar::Float> &rhs);

	Float(RValue<packed::Float4> rhs);
	RValue<SIMD::Float> operator=(RValue<packed::Float4> rhs);
	template<int T>
	Float(const SwizzleMask1<packed::Float4, T> &rhs);

	RValue<SIMD::Float> operator=(float broadcast);
	RValue<SIMD::Float> operator=(RValue<SIMD::Float> rhs);
	RValue<SIMD::Float> operator=(const Float &rhs);
	RValue<SIMD::Float> operator=(const Reference<SIMD::Float> &rhs);
	RValue<SIMD::Float> operator=(RValue<scalar::Float> rhs);
	RValue<SIMD::Float> operator=(const scalar::Float &rhs);
	RValue<SIMD::Float> operator=(const Reference<scalar::Float> &rhs);

	static SIMD::Float infinity();

	static Type *type();
	static int element_count() { return SIMD::Width; }
};

class Pointer
{
public:
	Pointer(scalar::Pointer<Byte> base, scalar::Int limit);
	Pointer(scalar::Pointer<Byte> base, unsigned int limit);
	Pointer(scalar::Pointer<Byte> base, scalar::Int limit, SIMD::Int offset);
	Pointer(scalar::Pointer<Byte> base, unsigned int limit, SIMD::Int offset);
	Pointer(std::vector<scalar::Pointer<Byte>> pointers);
	explicit Pointer(SIMD::UInt cast);                           // Cast from 32-bit integers to 32-bit pointers
	explicit Pointer(SIMD::UInt castLow, SIMD::UInt castHight);  // Cast from pairs of 32-bit integers to 64-bit pointers

	Pointer &operator+=(SIMD::Int i);
	Pointer operator+(SIMD::Int i);
	Pointer &operator+=(int i);
	Pointer operator+(int i);

	SIMD::Int offsets() const;

	SIMD::Int isInBounds(unsigned int accessSize, OutOfBoundsBehavior robustness) const;

	bool isStaticallyInBounds(unsigned int accessSize, OutOfBoundsBehavior robustness) const;

	Int limit() const;

	// Returns true if all offsets are compile-time static and sequential
	// (N+0*step, N+1*step, N+2*step, N+3*step)
	bool hasStaticSequentialOffsets(unsigned int step) const;

	// Returns true if all offsets are compile-time static and equal
	// (N, N, N, N)
	bool hasStaticEqualOffsets() const;

	template<typename T>
	inline T Load(OutOfBoundsBehavior robustness, SIMD::Int mask, bool atomic = false, std::memory_order order = std::memory_order_relaxed, int alignment = sizeof(float));

	template<typename T>
	inline void Store(T val, OutOfBoundsBehavior robustness, SIMD::Int mask, bool atomic = false, std::memory_order order = std::memory_order_relaxed);

	template<typename T>
	inline void Store(RValue<T> val, OutOfBoundsBehavior robustness, SIMD::Int mask, bool atomic = false, std::memory_order order = std::memory_order_relaxed);

	scalar::Pointer<Byte> getUniformPointer() const;
	scalar::Pointer<Byte> getPointerForLane(int lane) const;
	static Pointer IfThenElse(SIMD::Int condition, const Pointer &lhs, const Pointer &rhs);

	void castTo(SIMD::UInt &bits) const;                              // Cast from 32-bit pointers to 32-bit integers
	void castTo(SIMD::UInt &lowerBits, SIMD::UInt &upperBits) const;  // Cast from 64-bit pointers to pairs of 32-bit integers

#ifdef ENABLE_RR_PRINT
	std::vector<rr::Value *> getPrintValues() const;
#endif

private:
	// Base address for the pointer, common across all lanes.
	scalar::Pointer<Byte> base;
	// Per-lane address for dealing with non-uniform data
	std::vector<scalar::Pointer<Byte>> pointers;

public:
	// Upper (non-inclusive) limit for offsets from base.
	scalar::Int dynamicLimit;  // If hasDynamicLimit is false, dynamicLimit is zero.
	unsigned int staticLimit = 0;

	// Per lane offsets from base.
	SIMD::Int dynamicOffsets;  // If hasDynamicOffsets is false, all dynamicOffsets are zero.
	std::vector<int32_t> staticOffsets;

	bool hasDynamicLimit = false;    // True if dynamicLimit is non-zero.
	bool hasDynamicOffsets = false;  // True if any dynamicOffsets are non-zero.
	bool isBasePlusOffset = false;   // True if this uses base+offset. False if this is a collection of Pointers
};

}  // namespace SIMD

RValue<SIMD::Int> operator+(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator-(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator*(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator/(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator%(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator&(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator|(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator^(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator<<(RValue<SIMD::Int> lhs, unsigned char rhs);
RValue<SIMD::Int> operator>>(RValue<SIMD::Int> lhs, unsigned char rhs);
RValue<SIMD::Int> operator<<(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator>>(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator+=(SIMD::Int &lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator-=(SIMD::Int &lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator*=(SIMD::Int &lhs, RValue<SIMD::Int> rhs);
//	RValue<SIMD::Int> operator/=(SIMD::Int &lhs, RValue<SIMD::Int> rhs);
//	RValue<SIMD::Int> operator%=(SIMD::Int &lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator&=(SIMD::Int &lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator|=(SIMD::Int &lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator^=(SIMD::Int &lhs, RValue<SIMD::Int> rhs);
RValue<SIMD::Int> operator<<=(SIMD::Int &lhs, unsigned char rhs);
RValue<SIMD::Int> operator>>=(SIMD::Int &lhs, unsigned char rhs);
RValue<SIMD::Int> operator+(RValue<SIMD::Int> val);
RValue<SIMD::Int> operator-(RValue<SIMD::Int> val);
RValue<SIMD::Int> operator~(RValue<SIMD::Int> val);
//	RValue<SIMD::Int> operator++(SIMD::Int &val, int);   // Post-increment
//	const Int &operator++(SIMD::Int &val);   // Pre-increment
//	RValue<SIMD::Int> operator--(SIMD::Int &val, int);   // Post-decrement
//	const Int &operator--(SIMD::Int &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
//	RValue<Bool> operator<=(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
//	RValue<Bool> operator>(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
//	RValue<Bool> operator>=(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
//	RValue<Bool> operator!=(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);
//	RValue<Bool> operator==(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs);

RValue<SIMD::Int> CmpEQ(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
RValue<SIMD::Int> CmpLT(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
RValue<SIMD::Int> CmpLE(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
RValue<SIMD::Int> CmpNEQ(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
RValue<SIMD::Int> CmpNLT(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
RValue<SIMD::Int> CmpNLE(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
inline RValue<SIMD::Int> CmpGT(RValue<SIMD::Int> x, RValue<SIMD::Int> y)
{
	return CmpNLE(x, y);
}
inline RValue<SIMD::Int> CmpGE(RValue<SIMD::Int> x, RValue<SIMD::Int> y)
{
	return CmpNLT(x, y);
}
RValue<SIMD::Int> Abs(RValue<SIMD::Int> x);
RValue<SIMD::Int> Max(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
RValue<SIMD::Int> Min(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
// Convert to nearest integer. If a converted value is outside of the integer
// range, the returned result is undefined.
RValue<SIMD::Int> RoundInt(RValue<SIMD::Float> cast);
// Rounds to the nearest integer, but clamps very large values to an
// implementation-dependent range.
// Specifically, on x86, values larger than 2147483583.0 are converted to
// 2147483583 (0x7FFFFFBF) instead of producing 0x80000000.
RValue<SIMD::Int> RoundIntClamped(RValue<SIMD::Float> cast);
RValue<scalar::Int> Extract(RValue<SIMD::Int> val, int i);
RValue<SIMD::Int> Insert(RValue<SIMD::Int> val, RValue<scalar::Int> element, int i);
RValue<packed::Int4> Extract128(RValue<SIMD::Int> val, int i);
RValue<SIMD::Int> Insert128(RValue<SIMD::Int> val, RValue<packed::Int4> element, int i);

RValue<SIMD::UInt> operator+(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator-(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator*(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator/(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator%(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator&(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator|(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator^(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator<<(RValue<SIMD::UInt> lhs, unsigned char rhs);
RValue<SIMD::UInt> operator>>(RValue<SIMD::UInt> lhs, unsigned char rhs);
RValue<SIMD::UInt> operator<<(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator>>(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator+=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator-=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator*=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs);
//	RValue<SIMD::UInt> operator/=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs);
//	RValue<SIMD::UInt> operator%=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator&=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator|=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator^=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs);
RValue<SIMD::UInt> operator<<=(SIMD::UInt &lhs, unsigned char rhs);
RValue<SIMD::UInt> operator>>=(SIMD::UInt &lhs, unsigned char rhs);
RValue<SIMD::UInt> operator+(RValue<SIMD::UInt> val);
RValue<SIMD::UInt> operator-(RValue<SIMD::UInt> val);
RValue<SIMD::UInt> operator~(RValue<SIMD::UInt> val);
//	RValue<SIMD::UInt> operator++(SIMD::UInt &val, int);   // Post-increment
//	const UInt &operator++(SIMD::UInt &val);   // Pre-increment
//	RValue<SIMD::UInt> operator--(SIMD::UInt &val, int);   // Post-decrement
//	const UInt &operator--(SIMD::UInt &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
//	RValue<Bool> operator<=(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
//	RValue<Bool> operator>(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
//	RValue<Bool> operator>=(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
//	RValue<Bool> operator!=(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);
//	RValue<Bool> operator==(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs);

RValue<SIMD::UInt> CmpEQ(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y);
RValue<SIMD::UInt> CmpLT(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y);
RValue<SIMD::UInt> CmpLE(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y);
RValue<SIMD::UInt> CmpNEQ(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y);
RValue<SIMD::UInt> CmpNLT(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y);
RValue<SIMD::UInt> CmpNLE(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y);
inline RValue<SIMD::UInt> CmpGT(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y)
{
	return CmpNLE(x, y);
}
inline RValue<SIMD::UInt> CmpGE(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y)
{
	return CmpNLT(x, y);
}
RValue<SIMD::UInt> Max(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y);
RValue<SIMD::UInt> Min(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y);
RValue<scalar::UInt> Extract(RValue<SIMD::UInt> val, int i);
RValue<SIMD::UInt> Insert(RValue<SIMD::UInt> val, RValue<scalar::UInt> element, int i);
RValue<packed::UInt4> Extract128(RValue<SIMD::UInt> val, int i);
RValue<SIMD::UInt> Insert128(RValue<SIMD::UInt> val, RValue<packed::UInt4> element, int i);
//	RValue<SIMD::UInt> RoundInt(RValue<SIMD::Float> cast);

RValue<SIMD::Float> operator+(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator-(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator*(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator/(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator%(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator+=(SIMD::Float &lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator-=(SIMD::Float &lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator*=(SIMD::Float &lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator/=(SIMD::Float &lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator%=(SIMD::Float &lhs, RValue<SIMD::Float> rhs);
RValue<SIMD::Float> operator+(RValue<SIMD::Float> val);
RValue<SIMD::Float> operator-(RValue<SIMD::Float> val);

// Computes `x * y + z`, which may be fused into one operation to produce a higher-precision result.
RValue<SIMD::Float> MulAdd(RValue<SIMD::Float> x, RValue<SIMD::Float> y, RValue<SIMD::Float> z);
// Computes a fused `x * y + z` operation. Caps::fmaIsFast indicates whether it emits an FMA instruction.
RValue<SIMD::Float> FMA(RValue<SIMD::Float> x, RValue<SIMD::Float> y, RValue<SIMD::Float> z);

RValue<SIMD::Float> Abs(RValue<SIMD::Float> x);
RValue<SIMD::Float> Max(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Float> Min(RValue<SIMD::Float> x, RValue<SIMD::Float> y);

RValue<SIMD::Float> Rcp(RValue<SIMD::Float> x, bool relaxedPrecision, bool exactAtPow2 = false);
RValue<SIMD::Float> RcpSqrt(RValue<SIMD::Float> x, bool relaxedPrecision);
RValue<SIMD::Float> Sqrt(RValue<SIMD::Float> x);
RValue<SIMD::Float> Insert(RValue<SIMD::Float> val, RValue<rr ::Float> element, int i);
RValue<rr ::Float> Extract(RValue<SIMD::Float> x, int i);
RValue<packed::Float4> Extract128(RValue<SIMD::Float> val, int i);
RValue<SIMD::Float> Insert128(RValue<SIMD::Float> val, RValue<packed::Float4> element, int i);

// Ordered comparison functions
RValue<SIMD::Int> CmpEQ(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpLT(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpLE(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpNEQ(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpNLT(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpNLE(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
inline RValue<SIMD::Int> CmpGT(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	return CmpNLE(x, y);
}
inline RValue<SIMD::Int> CmpGE(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	return CmpNLT(x, y);
}

// Unordered comparison functions
RValue<SIMD::Int> CmpUEQ(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpULT(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpULE(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpUNEQ(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpUNLT(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Int> CmpUNLE(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
inline RValue<SIMD::Int> CmpUGT(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	return CmpUNLE(x, y);
}
inline RValue<SIMD::Int> CmpUGE(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	return CmpUNLT(x, y);
}

RValue<SIMD::Int> IsInf(RValue<SIMD::Float> x);
RValue<SIMD::Int> IsNan(RValue<SIMD::Float> x);
RValue<SIMD::Float> Round(RValue<SIMD::Float> x);
RValue<SIMD::Float> Trunc(RValue<SIMD::Float> x);
RValue<SIMD::Float> Frac(RValue<SIMD::Float> x);
RValue<SIMD::Float> Floor(RValue<SIMD::Float> x);
RValue<SIMD::Float> Ceil(RValue<SIMD::Float> x);

// Trigonometric functions
RValue<SIMD::Float> Sin(RValue<SIMD::Float> x);
RValue<SIMD::Float> Cos(RValue<SIMD::Float> x);
RValue<SIMD::Float> Tan(RValue<SIMD::Float> x);
RValue<SIMD::Float> Asin(RValue<SIMD::Float> x);
RValue<SIMD::Float> Acos(RValue<SIMD::Float> x);
RValue<SIMD::Float> Atan(RValue<SIMD::Float> x);
RValue<SIMD::Float> Sinh(RValue<SIMD::Float> x);
RValue<SIMD::Float> Cosh(RValue<SIMD::Float> x);
RValue<SIMD::Float> Tanh(RValue<SIMD::Float> x);
RValue<SIMD::Float> Asinh(RValue<SIMD::Float> x);
RValue<SIMD::Float> Acosh(RValue<SIMD::Float> x);
RValue<SIMD::Float> Atanh(RValue<SIMD::Float> x);
RValue<SIMD::Float> Atan2(RValue<SIMD::Float> x, RValue<SIMD::Float> y);

// Exponential functions
RValue<SIMD::Float> Pow(RValue<SIMD::Float> x, RValue<SIMD::Float> y);
RValue<SIMD::Float> Exp(RValue<SIMD::Float> x);
RValue<SIMD::Float> Log(RValue<SIMD::Float> x);
RValue<SIMD::Float> Exp2(RValue<SIMD::Float> x);
RValue<SIMD::Float> Log2(RValue<SIMD::Float> x);

RValue<Int> SignMask(RValue<SIMD::Int> x);
RValue<SIMD::UInt> Ctlz(RValue<SIMD::UInt> x, bool isZeroUndef);
RValue<SIMD::UInt> Cttz(RValue<SIMD::UInt> x, bool isZeroUndef);
RValue<SIMD::Int> MulHigh(RValue<SIMD::Int> x, RValue<SIMD::Int> y);
RValue<SIMD::UInt> MulHigh(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y);
RValue<Bool> AnyTrue(const RValue<SIMD::Int> &bools);
RValue<Bool> AnyFalse(const RValue<SIMD::Int> &bools);
RValue<Bool> Divergent(const RValue<SIMD::Int> &ints);
RValue<SIMD::Int> Swizzle(RValue<SIMD::Int> x, uint16_t select);
RValue<SIMD::UInt> Swizzle(RValue<SIMD::UInt> x, uint16_t select);
RValue<SIMD::Float> Swizzle(RValue<SIMD::Float> x, uint16_t select);
RValue<SIMD::Int> Shuffle(RValue<SIMD::Int> x, RValue<SIMD::Int> y, uint16_t select);
RValue<SIMD::UInt> Shuffle(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y, uint16_t select);
RValue<SIMD::Float> Shuffle(RValue<SIMD::Float> x, RValue<SIMD::Float> y, uint16_t select);

RValue<SIMD::Float> Gather(RValue<Pointer<Float>> base, RValue<SIMD::Int> offsets, RValue<SIMD::Int> mask, unsigned int alignment, bool zeroMaskedLanes = false);
RValue<SIMD::Int> Gather(RValue<Pointer<Int>> base, RValue<SIMD::Int> offsets, RValue<SIMD::Int> mask, unsigned int alignment, bool zeroMaskedLanes = false);
void Scatter(RValue<Pointer<Float>> base, RValue<SIMD::Float> val, RValue<SIMD::Int> offsets, RValue<SIMD::Int> mask, unsigned int alignment);
void Scatter(RValue<Pointer<Int>> base, RValue<SIMD::Int> val, RValue<SIMD::Int> offsets, RValue<SIMD::Int> mask, unsigned int alignment);

template<>
inline RValue<SIMD::Int>::RValue(int i)
    : val(broadcast(i, SIMD::Int::type()))
{
	RR_DEBUG_INFO_EMIT_VAR(val);
}

template<>
inline RValue<SIMD::UInt>::RValue(unsigned int i)
    : val(broadcast(int(i), SIMD::UInt::type()))
{
	RR_DEBUG_INFO_EMIT_VAR(val);
}

template<>
inline RValue<SIMD::Float>::RValue(float f)
    : val(broadcast(f, SIMD::Float::type()))
{
	RR_DEBUG_INFO_EMIT_VAR(val);
}

template<int T>
SIMD::Int::Int(const SwizzleMask1<packed::Int4, T> &rhs)
    : XYZW(this)
{
	*this = rhs.operator RValue<scalar::Int>();
}

template<int T>
SIMD::Float::Float(const SwizzleMask1<packed::Float4, T> &rhs)
    : XYZW(this)
{
	*this = rhs.operator RValue<scalar::Float>();
}

template<typename T>
inline T SIMD::Pointer::Load(OutOfBoundsBehavior robustness, SIMD::Int mask, bool atomic /* = false */, std::memory_order order /* = std::memory_order_relaxed */, int alignment /* = sizeof(float) */)
{
	using EL = typename Scalar<T>::Type;

	if(!isBasePlusOffset)
	{
		T out = T(0);
		for(int i = 0; i < SIMD::Width; i++)
		{
			If(Extract(mask, i) != 0)
			{
				auto el = rr::Load(scalar::Pointer<EL>(pointers[i]), alignment, atomic, order);
				out = Insert(out, el, i);
			}
		}
		return out;
	}

	if(isStaticallyInBounds(sizeof(float), robustness))
	{
		// All elements are statically known to be in-bounds.
		// We can avoid costly conditional on masks.

		if(hasStaticSequentialOffsets(sizeof(float)))
		{
			// Offsets are sequential. Perform regular load.
			return rr::Load(scalar::Pointer<T>(base + staticOffsets[0]), alignment, atomic, order);
		}

		if(hasStaticEqualOffsets())
		{
			// Load one, replicate.
			return T(*scalar::Pointer<EL>(base + staticOffsets[0], alignment));
		}
	}
	else
	{
		switch(robustness)
		{
		case OutOfBoundsBehavior::Nullify:
		case OutOfBoundsBehavior::RobustBufferAccess:
		case OutOfBoundsBehavior::UndefinedValue:
			mask &= isInBounds(sizeof(float), robustness);  // Disable out-of-bounds reads.
			break;
		case OutOfBoundsBehavior::UndefinedBehavior:
			// Nothing to do. Application/compiler must guarantee no out-of-bounds accesses.
			break;
		}
	}

	auto offs = offsets();

	if(!atomic && order == std::memory_order_relaxed)
	{
		if(hasStaticEqualOffsets())
		{
			// Load one, replicate.
			// Be careful of the case where the post-bounds-check mask
			// is 0, in which case we must not load.
			T out = T(0);
			If(AnyTrue(mask))
			{
				EL el = *scalar::Pointer<EL>(base + staticOffsets[0], alignment);
				out = T(el);
			}
			return out;
		}

		bool zeroMaskedLanes = true;
		switch(robustness)
		{
		case OutOfBoundsBehavior::Nullify:
		case OutOfBoundsBehavior::RobustBufferAccess:  // Must either return an in-bounds value, or zero.
			zeroMaskedLanes = true;
			break;
		case OutOfBoundsBehavior::UndefinedValue:
		case OutOfBoundsBehavior::UndefinedBehavior:
			zeroMaskedLanes = false;
			break;
		}

		// TODO(b/195446858): Optimize static sequential offsets case by using masked load.

		return Gather(scalar::Pointer<EL>(base), offs, mask, alignment, zeroMaskedLanes);
	}
	else
	{
		T out;
		auto anyLanesDisabled = AnyFalse(mask);
		If(hasStaticEqualOffsets() && !anyLanesDisabled)
		{
			// Load one, replicate.
			auto offset = Extract(offs, 0);
			out = T(rr::Load(scalar::Pointer<EL>(&base[offset]), alignment, atomic, order));
		}
		Else If(hasStaticSequentialOffsets(sizeof(float)) && !anyLanesDisabled)
		{
			// Load all elements in a single SIMD instruction.
			auto offset = Extract(offs, 0);
			out = rr::Load(scalar::Pointer<T>(&base[offset]), alignment, atomic, order);
		}
		Else
		{
			// Divergent offsets or masked lanes.
			out = T(0);
			for(int i = 0; i < SIMD::Width; i++)
			{
				If(Extract(mask, i) != 0)
				{
					auto offset = Extract(offs, i);
					auto el = rr::Load(scalar::Pointer<EL>(&base[offset]), alignment, atomic, order);
					out = Insert(out, el, i);
				}
			}
		}
		return out;
	}
}

template<>
inline SIMD::Pointer SIMD::Pointer::Load(OutOfBoundsBehavior robustness, SIMD::Int mask, bool atomic /* = false */, std::memory_order order /* = std::memory_order_relaxed */, int alignment /* = sizeof(float) */)
{
	std::vector<scalar::Pointer<Byte>> pointers(SIMD::Width);

	for(int i = 0; i < SIMD::Width; i++)
	{
		If(Extract(mask, i) != 0)
		{
			pointers[i] = rr::Load(scalar::Pointer<scalar::Pointer<Byte>>(getPointerForLane(i)), alignment, atomic, order);
		}
	}

	return SIMD::Pointer(pointers);
}

template<typename T>
inline void SIMD::Pointer::Store(T val, OutOfBoundsBehavior robustness, SIMD::Int mask, bool atomic /* = false */, std::memory_order order /* = std::memory_order_relaxed */)
{
	using EL = typename Scalar<T>::Type;
	constexpr size_t alignment = sizeof(float);

	if(!isBasePlusOffset)
	{
		for(int i = 0; i < SIMD::Width; i++)
		{
			If(Extract(mask, i) != 0)
			{
				rr::Store(Extract(val, i), scalar::Pointer<EL>(pointers[i]), alignment, atomic, order);
			}
		}
		return;
	}

	auto offs = offsets();
	switch(robustness)
	{
	case OutOfBoundsBehavior::Nullify:
	case OutOfBoundsBehavior::RobustBufferAccess:       // TODO: Allows writing anywhere within bounds. Could be faster than masking.
	case OutOfBoundsBehavior::UndefinedValue:           // Should not be used for store operations. Treat as robust buffer access.
		mask &= isInBounds(sizeof(float), robustness);  // Disable out-of-bounds writes.
		break;
	case OutOfBoundsBehavior::UndefinedBehavior:
		// Nothing to do. Application/compiler must guarantee no out-of-bounds accesses.
		break;
	}

	if(!atomic && order == std::memory_order_relaxed)
	{
		if(hasStaticEqualOffsets())
		{
			If(AnyTrue(mask))
			{
				assert(SIMD::Width == 4);

				// All equal. One of these writes will win -- elect the winning lane.
				auto v0111 = SIMD::Int(0, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
				auto elect = mask & ~(v0111 & (mask.xxyz | mask.xxxy | mask.xxxx));
				auto maskedVal = As<SIMD::Int>(val) & elect;
				auto scalarVal = Extract(maskedVal, 0) |
				                 Extract(maskedVal, 1) |
				                 Extract(maskedVal, 2) |
				                 Extract(maskedVal, 3);
				*scalar::Pointer<EL>(base + staticOffsets[0], alignment) = As<EL>(scalarVal);
			}
		}
		else if(hasStaticSequentialOffsets(sizeof(float)) &&
		        isStaticallyInBounds(sizeof(float), robustness))
		{
			// TODO(b/195446858): Optimize using masked store.
			// Pointer has no elements OOB, and the store is not atomic.
			// Perform a read-modify-write.
			auto p = scalar::Pointer<SIMD::Int>(base + staticOffsets[0], alignment);
			auto prev = *p;
			*p = (prev & ~mask) | (As<SIMD::Int>(val) & mask);
		}
		else
		{
			Scatter(scalar::Pointer<EL>(base), val, offs, mask, alignment);
		}
	}
	else
	{
		auto anyLanesDisabled = AnyFalse(mask);
		If(hasStaticSequentialOffsets(sizeof(float)) && !anyLanesDisabled)
		{
			// Store all elements in a single SIMD instruction.
			auto offset = Extract(offs, 0);
			rr::Store(val, scalar::Pointer<T>(&base[offset]), alignment, atomic, order);
		}
		Else
		{
			// Divergent offsets or masked lanes.
			for(int i = 0; i < SIMD::Width; i++)
			{
				If(Extract(mask, i) != 0)
				{
					auto offset = Extract(offs, i);
					rr::Store(Extract(val, i), scalar::Pointer<EL>(&base[offset]), alignment, atomic, order);
				}
			}
		}
	}
}

template<>
inline void SIMD::Pointer::Store(SIMD::Pointer val, OutOfBoundsBehavior robustness, SIMD::Int mask, bool atomic /* = false */, std::memory_order order /* = std::memory_order_relaxed */)
{
	constexpr size_t alignment = sizeof(void *);

	for(int i = 0; i < SIMD::Width; i++)
	{
		If(Extract(mask, i) != 0)
		{
			rr::Store(val.getPointerForLane(i), scalar::Pointer<scalar::Pointer<Byte>>(getPointerForLane(i)), alignment, atomic, order);
		}
	}
}

template<typename T>
inline void SIMD::Pointer::Store(RValue<T> val, OutOfBoundsBehavior robustness, SIMD::Int mask, bool atomic /* = false */, std::memory_order order /* = std::memory_order_relaxed */)
{
	Store(T(val), robustness, mask, atomic, order);
}

}  // namespace rr

#endif  // rr_SIMD_hpp
