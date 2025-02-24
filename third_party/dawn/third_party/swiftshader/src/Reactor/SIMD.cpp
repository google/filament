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

#include "SIMD.hpp"

#include "Assert.hpp"
#include "Debug.hpp"
#include "Print.hpp"

#include <cmath>

namespace rr {

SIMD::Int::Int()
    : XYZW(this)
{
}

SIMD::Int::Int(RValue<SIMD::Float> cast)
    : XYZW(this)
{
	Value *xyzw = Nucleus::createFPToSI(cast.value(), SIMD::Int::type());

	storeValue(xyzw);
}

SIMD::Int::Int(int broadcast)
    : XYZW(this)
{
	std::vector<int64_t> constantVector = { broadcast };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

SIMD::Int::Int(int x, int y, int z, int w)
    : XYZW(this)
{
	std::vector<int64_t> constantVector = { x, y, z, w };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

SIMD::Int::Int(std::vector<int> v)
    : XYZW(this)
{
	std::vector<int64_t> constantVector;
	for(int i : v) { constantVector.push_back(i); }
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

SIMD::Int::Int(std::function<int(int)> LaneValueProducer)
    : XYZW(this)
{
	std::vector<int64_t> constantVector;
	for(int i = 0; i < SIMD::Width; i++) { constantVector.push_back(LaneValueProducer(i)); }
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

SIMD::Int::Int(RValue<SIMD::Int> rhs)
    : XYZW(this)
{
	store(rhs);
}

SIMD::Int::Int(const SIMD::Int &rhs)
    : XYZW(this)
{
	store(rhs.load());
}

SIMD::Int::Int(const Reference<SIMD::Int> &rhs)
    : XYZW(this)
{
	store(rhs.load());
}

SIMD::Int::Int(RValue<SIMD::UInt> rhs)
    : XYZW(this)
{
	storeValue(rhs.value());
}

SIMD::Int::Int(const SIMD::UInt &rhs)
    : XYZW(this)
{
	storeValue(rhs.loadValue());
}

SIMD::Int::Int(const Reference<SIMD::UInt> &rhs)
    : XYZW(this)
{
	storeValue(rhs.loadValue());
}

SIMD::Int::Int(const scalar::Int &rhs)
    : XYZW(this)
{
	*this = RValue<scalar::Int>(rhs.loadValue());
}

SIMD::Int::Int(const Reference<scalar::Int> &rhs)
    : XYZW(this)
{
	*this = RValue<scalar::Int>(rhs.loadValue());
}

RValue<SIMD::Int> SIMD::Int::operator=(int x)
{
	return *this = SIMD::Int(x);
}

RValue<SIMD::Int> SIMD::Int::operator=(RValue<SIMD::Int> rhs)
{
	return store(rhs);
}

RValue<SIMD::Int> SIMD::Int::operator=(const SIMD::Int &rhs)
{
	return store(rhs.load());
}

RValue<SIMD::Int> SIMD::Int::operator=(const Reference<SIMD::Int> &rhs)
{
	return store(rhs.load());
}

RValue<SIMD::Int> operator+(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator-(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createSub(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator*(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createMul(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator/(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createSDiv(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator%(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createSRem(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator&(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator|(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator^(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createXor(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator<<(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createShl(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator>>(RValue<SIMD::Int> lhs, RValue<SIMD::Int> rhs)
{
	return RValue<SIMD::Int>(Nucleus::createAShr(lhs.value(), rhs.value()));
}

RValue<SIMD::Int> operator+=(SIMD::Int &lhs, RValue<SIMD::Int> rhs)
{
	return lhs = lhs + rhs;
}

RValue<SIMD::Int> operator-=(SIMD::Int &lhs, RValue<SIMD::Int> rhs)
{
	return lhs = lhs - rhs;
}

RValue<SIMD::Int> operator*=(SIMD::Int &lhs, RValue<SIMD::Int> rhs)
{
	return lhs = lhs * rhs;
}

//	RValue<SIMD::Int> operator/=(SIMD::Int &lhs, RValue<SIMD::Int> rhs)
//	{
//		return lhs = lhs / rhs;
//	}

//	RValue<SIMD::Int> operator%=(SIMD::Int &lhs, RValue<SIMD::Int> rhs)
//	{
//		return lhs = lhs % rhs;
//	}

RValue<SIMD::Int> operator&=(SIMD::Int &lhs, RValue<SIMD::Int> rhs)
{
	return lhs = lhs & rhs;
}

RValue<SIMD::Int> operator|=(SIMD::Int &lhs, RValue<SIMD::Int> rhs)
{
	return lhs = lhs | rhs;
}

RValue<SIMD::Int> operator^=(SIMD::Int &lhs, RValue<SIMD::Int> rhs)
{
	return lhs = lhs ^ rhs;
}

RValue<SIMD::Int> operator<<=(SIMD::Int &lhs, unsigned char rhs)
{
	return lhs = lhs << rhs;
}

RValue<SIMD::Int> operator>>=(SIMD::Int &lhs, unsigned char rhs)
{
	return lhs = lhs >> rhs;
}

RValue<SIMD::Int> operator+(RValue<SIMD::Int> val)
{
	return val;
}

RValue<SIMD::Int> operator-(RValue<SIMD::Int> val)
{
	return RValue<SIMD::Int>(Nucleus::createNeg(val.value()));
}

RValue<SIMD::Int> operator~(RValue<SIMD::Int> val)
{
	return RValue<SIMD::Int>(Nucleus::createNot(val.value()));
}

RValue<scalar::Int> Extract(RValue<SIMD::Int> x, int i)
{
	return RValue<scalar::Int>(Nucleus::createExtractElement(x.value(), scalar::Int::type(), i));
}

RValue<SIMD::Int> Insert(RValue<SIMD::Int> x, RValue<scalar::Int> element, int i)
{
	return RValue<SIMD::Int>(Nucleus::createInsertElement(x.value(), element.value(), i));
}

SIMD::UInt::UInt()
    : XYZW(this)
{
}

SIMD::UInt::UInt(int broadcast)
    : XYZW(this)
{
	std::vector<int64_t> constantVector = { broadcast };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

SIMD::UInt::UInt(int x, int y, int z, int w)
    : XYZW(this)
{
	std::vector<int64_t> constantVector = { x, y, z, w };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

SIMD::UInt::UInt(std::vector<int> v)
    : XYZW(this)
{
	std::vector<int64_t> constantVector;
	for(int i : v) { constantVector.push_back(i); }
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

SIMD::UInt::UInt(std::function<int(int)> LaneValueProducer)
    : XYZW(this)
{
	std::vector<int64_t> constantVector;
	for(int i = 0; i < SIMD::Width; i++) { constantVector.push_back(LaneValueProducer(i)); }
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

SIMD::UInt::UInt(RValue<SIMD::UInt> rhs)
    : XYZW(this)
{
	store(rhs);
}

SIMD::UInt::UInt(const SIMD::UInt &rhs)
    : XYZW(this)
{
	store(rhs.load());
}

SIMD::UInt::UInt(const Reference<SIMD::UInt> &rhs)
    : XYZW(this)
{
	store(rhs.load());
}

SIMD::UInt::UInt(RValue<SIMD::Int> rhs)
    : XYZW(this)
{
	storeValue(rhs.value());
}

SIMD::UInt::UInt(const SIMD::Int &rhs)
    : XYZW(this)
{
	storeValue(rhs.loadValue());
}

SIMD::UInt::UInt(const Reference<SIMD::Int> &rhs)
    : XYZW(this)
{
	storeValue(rhs.loadValue());
}

SIMD::UInt::UInt(const scalar::UInt &rhs)
    : XYZW(this)
{
	*this = RValue<scalar::UInt>(rhs.loadValue());
}

SIMD::UInt::UInt(const Reference<scalar::UInt> &rhs)
    : XYZW(this)
{
	*this = RValue<scalar::UInt>(rhs.loadValue());
}

RValue<SIMD::UInt> SIMD::UInt::operator=(RValue<SIMD::UInt> rhs)
{
	return store(rhs);
}

RValue<SIMD::UInt> SIMD::UInt::operator=(const SIMD::UInt &rhs)
{
	return store(rhs.load());
}

RValue<SIMD::UInt> SIMD::UInt::operator=(const Reference<SIMD::UInt> &rhs)
{
	return store(rhs.load());
}

RValue<SIMD::UInt> operator+(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator-(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createSub(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator*(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createMul(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator/(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createUDiv(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator%(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createURem(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator&(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator|(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator^(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createXor(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator<<(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createShl(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator>>(RValue<SIMD::UInt> lhs, RValue<SIMD::UInt> rhs)
{
	return RValue<SIMD::UInt>(Nucleus::createLShr(lhs.value(), rhs.value()));
}

RValue<SIMD::UInt> operator+=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs)
{
	return lhs = lhs + rhs;
}

RValue<SIMD::UInt> operator-=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs)
{
	return lhs = lhs - rhs;
}

RValue<SIMD::UInt> operator*=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs)
{
	return lhs = lhs * rhs;
}

//	RValue<SIMD::UInt> operator/=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs)
//	{
//		return lhs = lhs / rhs;
//	}

//	RValue<SIMD::UInt> operator%=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs)
//	{
//		return lhs = lhs % rhs;
//	}

RValue<SIMD::UInt> operator&=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs)
{
	return lhs = lhs & rhs;
}

RValue<SIMD::UInt> operator|=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs)
{
	return lhs = lhs | rhs;
}

RValue<SIMD::UInt> operator^=(SIMD::UInt &lhs, RValue<SIMD::UInt> rhs)
{
	return lhs = lhs ^ rhs;
}

RValue<SIMD::UInt> operator<<=(SIMD::UInt &lhs, unsigned char rhs)
{
	return lhs = lhs << rhs;
}

RValue<SIMD::UInt> operator>>=(SIMD::UInt &lhs, unsigned char rhs)
{
	return lhs = lhs >> rhs;
}

RValue<SIMD::UInt> operator+(RValue<SIMD::UInt> val)
{
	return val;
}

RValue<SIMD::UInt> operator-(RValue<SIMD::UInt> val)
{
	return RValue<SIMD::UInt>(Nucleus::createNeg(val.value()));
}

RValue<SIMD::UInt> operator~(RValue<SIMD::UInt> val)
{
	return RValue<SIMD::UInt>(Nucleus::createNot(val.value()));
}

RValue<scalar::UInt> Extract(RValue<SIMD::UInt> x, int i)
{
	return RValue<scalar::UInt>(Nucleus::createExtractElement(x.value(), scalar::Int::type(), i));
}

RValue<SIMD::UInt> Insert(RValue<SIMD::UInt> x, RValue<scalar::UInt> element, int i)
{
	return RValue<SIMD::UInt>(Nucleus::createInsertElement(x.value(), element.value(), i));
}

SIMD::Float::Float(RValue<SIMD::Int> cast)
    : XYZW(this)
{
	Value *xyzw = Nucleus::createSIToFP(cast.value(), SIMD::Float::type());

	storeValue(xyzw);
}

SIMD::Float::Float(RValue<SIMD::UInt> cast)
    : XYZW(this)
{
	RValue<SIMD::Float> result = SIMD::Float(SIMD::Int(cast & SIMD::UInt(0x7FFFFFFF))) +
	                             As<SIMD::Float>((As<SIMD::Int>(cast) >> 31) & As<SIMD::Int>(SIMD::Float(0x80000000u)));

	storeValue(result.value());
}

SIMD::Float::Float()
    : XYZW(this)
{
}

SIMD::Float::Float(float broadcast)
    : XYZW(this)
{
	// See rr::Float(float) constructor for the rationale behind this assert.
	ASSERT(std::isfinite(broadcast));

	std::vector<double> constantVector = { broadcast };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

SIMD::Float::Float(float x, float y, float z, float w)
    : XYZW(this)
{
	std::vector<double> constantVector = { x, y, z, w };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

SIMD::Float::Float(std::vector<float> v)
    : XYZW(this)
{
	std::vector<double> constantVector;
	for(int f : v) { constantVector.push_back(f); }
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

SIMD::Float::Float(std::function<float(int)> LaneValueProducer)
    : XYZW(this)
{
	std::vector<double> constantVector;
	for(int i = 0; i < SIMD::Width; i++) { constantVector.push_back(LaneValueProducer(i)); }
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

SIMD::Float SIMD::Float::infinity()
{
	SIMD::Float result;

	constexpr double inf = std::numeric_limits<double>::infinity();
	std::vector<double> constantVector = { inf };
	result.storeValue(Nucleus::createConstantVector(constantVector, type()));

	return result;
}

SIMD::Float::Float(RValue<SIMD::Float> rhs)
    : XYZW(this)
{
	store(rhs);
}

SIMD::Float::Float(const SIMD::Float &rhs)
    : XYZW(this)
{
	store(rhs.load());
}

SIMD::Float::Float(const Reference<SIMD::Float> &rhs)
    : XYZW(this)
{
	store(rhs.load());
}

SIMD::Float::Float(const scalar::Float &rhs)
    : XYZW(this)
{
	*this = RValue<scalar::Float>(rhs.loadValue());
}

SIMD::Float::Float(const Reference<scalar::Float> &rhs)
    : XYZW(this)
{
	*this = RValue<scalar::Float>(rhs.loadValue());
}

SIMD::Float::Float(RValue<packed::Float4> rhs)
    : XYZW(this)
{
	ASSERT(SIMD::Width == 4);
	*this = Insert128(*this, rhs, 0);
}

RValue<SIMD::Float> SIMD::Float::operator=(RValue<packed::Float4> rhs)
{
	return *this = SIMD::Float(rhs);
}

RValue<SIMD::Float> SIMD::Float::operator=(float x)
{
	return *this = SIMD::Float(x);
}

RValue<SIMD::Float> SIMD::Float::operator=(RValue<SIMD::Float> rhs)
{
	return store(rhs);
}

RValue<SIMD::Float> SIMD::Float::operator=(const SIMD::Float &rhs)
{
	return store(rhs.load());
}

RValue<SIMD::Float> SIMD::Float::operator=(const Reference<SIMD::Float> &rhs)
{
	return store(rhs.load());
}

RValue<SIMD::Float> SIMD::Float::operator=(RValue<scalar::Float> rhs)
{
	return *this = SIMD::Float(rhs);
}

RValue<SIMD::Float> SIMD::Float::operator=(const scalar::Float &rhs)
{
	return *this = SIMD::Float(rhs);
}

RValue<SIMD::Float> SIMD::Float::operator=(const Reference<scalar::Float> &rhs)
{
	return *this = SIMD::Float(rhs);
}

RValue<SIMD::Float> operator+(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs)
{
	return RValue<SIMD::Float>(Nucleus::createFAdd(lhs.value(), rhs.value()));
}

RValue<SIMD::Float> operator-(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs)
{
	return RValue<SIMD::Float>(Nucleus::createFSub(lhs.value(), rhs.value()));
}

RValue<SIMD::Float> operator*(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs)
{
	return RValue<SIMD::Float>(Nucleus::createFMul(lhs.value(), rhs.value()));
}

RValue<SIMD::Float> operator/(RValue<SIMD::Float> lhs, RValue<SIMD::Float> rhs)
{
	return RValue<SIMD::Float>(Nucleus::createFDiv(lhs.value(), rhs.value()));
}

RValue<SIMD::Float> operator+=(SIMD::Float &lhs, RValue<SIMD::Float> rhs)
{
	return lhs = lhs + rhs;
}

RValue<SIMD::Float> operator-=(SIMD::Float &lhs, RValue<SIMD::Float> rhs)
{
	return lhs = lhs - rhs;
}

RValue<SIMD::Float> operator*=(SIMD::Float &lhs, RValue<SIMD::Float> rhs)
{
	return lhs = lhs * rhs;
}

RValue<SIMD::Float> operator/=(SIMD::Float &lhs, RValue<SIMD::Float> rhs)
{
	return lhs = lhs / rhs;
}

RValue<SIMD::Float> operator%=(SIMD::Float &lhs, RValue<SIMD::Float> rhs)
{
	return lhs = lhs % rhs;
}

RValue<SIMD::Float> operator+(RValue<SIMD::Float> val)
{
	return val;
}

RValue<SIMD::Float> operator-(RValue<SIMD::Float> val)
{
	return RValue<SIMD::Float>(Nucleus::createFNeg(val.value()));
}

RValue<SIMD::Float> Rcp(RValue<SIMD::Float> x, bool relaxedPrecision, bool exactAtPow2)
{
	ASSERT(SIMD::Width == 4);
	return SIMD::Float(Rcp(Extract128(x, 0), relaxedPrecision, exactAtPow2));
}

RValue<SIMD::Float> RcpSqrt(RValue<SIMD::Float> x, bool relaxedPrecision)
{
	ASSERT(SIMD::Width == 4);
	return SIMD::Float(RcpSqrt(Extract128(x, 0), relaxedPrecision));
}

RValue<SIMD::Float> Insert(RValue<SIMD::Float> x, RValue<scalar::Float> element, int i)
{
	return RValue<SIMD::Float>(Nucleus::createInsertElement(x.value(), element.value(), i));
}

RValue<scalar::Float> Extract(RValue<SIMD::Float> x, int i)
{
	return RValue<scalar::Float>(Nucleus::createExtractElement(x.value(), scalar::Float::type(), i));
}

RValue<SIMD::Int> IsInf(RValue<SIMD::Float> x)
{
	return CmpEQ(As<SIMD::Int>(x) & SIMD::Int(0x7FFFFFFF), SIMD::Int(0x7F800000));
}

RValue<SIMD::Int> IsNan(RValue<SIMD::Float> x)
{
	return ~CmpEQ(x, x);
}

RValue<SIMD::Float> Sin(RValue<SIMD::Float> x)
{
	return ScalarizeCall(sinf, x);
}

RValue<SIMD::Float> Cos(RValue<SIMD::Float> x)
{
	return ScalarizeCall(cosf, x);
}

RValue<SIMD::Float> Tan(RValue<SIMD::Float> x)
{
	return ScalarizeCall(tanf, x);
}

RValue<SIMD::Float> Asin(RValue<SIMD::Float> x)
{
	return ScalarizeCall(asinf, x);
}

RValue<SIMD::Float> Acos(RValue<SIMD::Float> x)
{
	return ScalarizeCall(acosf, x);
}

RValue<SIMD::Float> Atan(RValue<SIMD::Float> x)
{
	return ScalarizeCall(atanf, x);
}

RValue<SIMD::Float> Sinh(RValue<SIMD::Float> x)
{
	return ScalarizeCall(sinhf, x);
}

RValue<SIMD::Float> Cosh(RValue<SIMD::Float> x)
{
	return ScalarizeCall(coshf, x);
}

RValue<SIMD::Float> Tanh(RValue<SIMD::Float> x)
{
	return ScalarizeCall(tanhf, x);
}

RValue<SIMD::Float> Asinh(RValue<SIMD::Float> x)
{
	return ScalarizeCall(asinhf, x);
}

RValue<SIMD::Float> Acosh(RValue<SIMD::Float> x)
{
	return ScalarizeCall(acoshf, x);
}

RValue<SIMD::Float> Atanh(RValue<SIMD::Float> x)
{
	return ScalarizeCall(atanhf, x);
}

RValue<SIMD::Float> Atan2(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	return ScalarizeCall(atan2f, x, y);
}

RValue<SIMD::Float> Pow(RValue<SIMD::Float> x, RValue<SIMD::Float> y)
{
	return ScalarizeCall(powf, x, y);
}

RValue<SIMD::Float> Exp(RValue<SIMD::Float> x)
{
	return ScalarizeCall(expf, x);
}

RValue<SIMD::Float> Log(RValue<SIMD::Float> x)
{
	return ScalarizeCall(logf, x);
}

RValue<SIMD::Float> Exp2(RValue<SIMD::Float> x)
{
	return ScalarizeCall(exp2f, x);
}

RValue<SIMD::Float> Log2(RValue<SIMD::Float> x)
{
	return ScalarizeCall(log2f, x);
}

RValue<Int> SignMask(RValue<SIMD::Int> x)
{
	ASSERT(SIMD::Width == 4);
	return SignMask(Extract128(x, 0));
}

RValue<SIMD::UInt> Ctlz(RValue<SIMD::UInt> x, bool isZeroUndef)
{
	ASSERT(SIMD::Width == 4);
	SIMD::UInt result;
	return Insert128(result, Ctlz(Extract128(x, 0), isZeroUndef), 0);
}

RValue<SIMD::UInt> Cttz(RValue<SIMD::UInt> x, bool isZeroUndef)
{
	ASSERT(SIMD::Width == 4);
	SIMD::UInt result;
	return Insert128(result, Cttz(Extract128(x, 0), isZeroUndef), 0);
}

RValue<SIMD::Int> MulHigh(RValue<SIMD::Int> x, RValue<SIMD::Int> y)
{
	ASSERT(SIMD::Width == 4);
	SIMD::Int result;
	return Insert128(result, MulHigh(Extract128(x, 0), Extract128(y, 0)), 0);
}

RValue<SIMD::UInt> MulHigh(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y)
{
	ASSERT(SIMD::Width == 4);
	SIMD::UInt result;
	return Insert128(result, MulHigh(Extract128(x, 0), Extract128(y, 0)), 0);
}

RValue<Bool> AnyTrue(const RValue<SIMD::Int> &bools)
{
	ASSERT(SIMD::Width == 4);
	return AnyTrue(Extract128(bools, 0));
}

RValue<Bool> AnyFalse(const RValue<SIMD::Int> &bools)
{
	ASSERT(SIMD::Width == 4);
	return AnyFalse(Extract128(bools, 0));
}

RValue<Bool> Divergent(const RValue<SIMD::Int> &ints)
{
	ASSERT(SIMD::Width == 4);
	return Divergent(Extract128(ints, 0));
}

RValue<SIMD::Int> Swizzle(RValue<SIMD::Int> x, uint16_t select)
{
	ASSERT(SIMD::Width == 4);
	SIMD::Int result;
	return Insert128(result, Swizzle(Extract128(x, 0), select), 0);
}

RValue<SIMD::UInt> Swizzle(RValue<SIMD::UInt> x, uint16_t select)
{
	ASSERT(SIMD::Width == 4);
	SIMD::UInt result;
	return Insert128(result, Swizzle(Extract128(x, 0), select), 0);
}

RValue<SIMD::Float> Swizzle(RValue<SIMD::Float> x, uint16_t select)
{
	ASSERT(SIMD::Width == 4);
	SIMD::Float result;
	return Insert128(result, Swizzle(Extract128(x, 0), select), 0);
}

RValue<SIMD::Int> Shuffle(RValue<SIMD::Int> x, RValue<SIMD::Int> y, uint16_t select)
{
	ASSERT(SIMD::Width == 4);
	SIMD::Int result;
	return Insert128(result, Shuffle(Extract128(x, 0), Extract128(y, 0), select), 0);
}

RValue<SIMD::UInt> Shuffle(RValue<SIMD::UInt> x, RValue<SIMD::UInt> y, uint16_t select)
{
	ASSERT(SIMD::Width == 4);
	SIMD::UInt result;
	return Insert128(result, Shuffle(Extract128(x, 0), Extract128(y, 0), select), 0);
}

RValue<SIMD::Float> Shuffle(RValue<SIMD::Float> x, RValue<SIMD::Float> y, uint16_t select)
{
	ASSERT(SIMD::Width == 4);
	SIMD::Float result;
	return Insert128(result, Shuffle(Extract128(x, 0), Extract128(y, 0), select), 0);
}

SIMD::Pointer::Pointer(scalar::Pointer<Byte> base, rr::Int limit)
    : base(base)
    , dynamicLimit(limit)
    , staticLimit(0)
    , dynamicOffsets(0)
    , staticOffsets(SIMD::Width)
    , hasDynamicLimit(true)
    , hasDynamicOffsets(false)
    , isBasePlusOffset(true)
{}

SIMD::Pointer::Pointer(scalar::Pointer<Byte> base, unsigned int limit)
    : base(base)
    , dynamicLimit(0)
    , staticLimit(limit)
    , dynamicOffsets(0)
    , staticOffsets(SIMD::Width)
    , hasDynamicLimit(false)
    , hasDynamicOffsets(false)
    , isBasePlusOffset(true)
{}

SIMD::Pointer::Pointer(scalar::Pointer<Byte> base, rr::Int limit, SIMD::Int offset)
    : base(base)
    , dynamicLimit(limit)
    , staticLimit(0)
    , dynamicOffsets(offset)
    , staticOffsets(SIMD::Width)
    , hasDynamicLimit(true)
    , hasDynamicOffsets(true)
    , isBasePlusOffset(true)
{}

SIMD::Pointer::Pointer(scalar::Pointer<Byte> base, unsigned int limit, SIMD::Int offset)
    : base(base)
    , dynamicLimit(0)
    , staticLimit(limit)
    , dynamicOffsets(offset)
    , staticOffsets(SIMD::Width)
    , hasDynamicLimit(false)
    , hasDynamicOffsets(true)
    , isBasePlusOffset(true)
{}

SIMD::Pointer::Pointer(std::vector<scalar::Pointer<Byte>> pointers)
    : pointers(pointers)
    , isBasePlusOffset(false)
{}

SIMD::Pointer::Pointer(SIMD::UInt cast)
    : pointers(SIMD::Width)
    , isBasePlusOffset(false)
{
	assert(sizeof(void *) == 4);
	for(int i = 0; i < SIMD::Width; i++)
	{
		pointers[i] = As<rr::Pointer<Byte>>(Extract(cast, i));
	}
}

SIMD::Pointer::Pointer(SIMD::UInt castLow, SIMD::UInt castHigh)
    : pointers(SIMD::Width)
    , isBasePlusOffset(false)
{
	assert(sizeof(void *) == 8);
	for(int i = 0; i < SIMD::Width; i++)
	{
		UInt2 address;
		address = Insert(address, Extract(castLow, i), 0);
		address = Insert(address, Extract(castHigh, i), 1);
		pointers[i] = As<rr::Pointer<Byte>>(address);
	}
}

SIMD::Pointer &SIMD::Pointer::operator+=(SIMD::Int i)
{
	if(isBasePlusOffset)
	{
		dynamicOffsets += i;
		hasDynamicOffsets = true;
	}
	else
	{
		for(int el = 0; el < SIMD::Width; el++) { pointers[el] += Extract(i, el); }
	}
	return *this;
}

SIMD::Pointer SIMD::Pointer::operator+(SIMD::Int i)
{
	SIMD::Pointer p = *this;
	p += i;
	return p;
}

SIMD::Pointer &SIMD::Pointer::operator+=(int i)
{
	if(isBasePlusOffset)
	{
		for(int el = 0; el < SIMD::Width; el++) { staticOffsets[el] += i; }
	}
	else
	{
		for(int el = 0; el < SIMD::Width; el++) { pointers[el] += i; }
	}
	return *this;
}

SIMD::Pointer SIMD::Pointer::operator+(int i)
{
	SIMD::Pointer p = *this;
	p += i;
	return p;
}

SIMD::Int SIMD::Pointer::offsets() const
{
	ASSERT_MSG(isBasePlusOffset, "No offsets for this type of pointer");
	return dynamicOffsets + SIMD::Int(staticOffsets);
}

SIMD::Int SIMD::Pointer::isInBounds(unsigned int accessSize, OutOfBoundsBehavior robustness) const
{
	ASSERT(accessSize > 0);

	if(isStaticallyInBounds(accessSize, robustness))
	{
		return SIMD::Int(0xFFFFFFFF);
	}

	if(!hasDynamicOffsets && !hasDynamicLimit)
	{
		ASSERT(SIMD::Width == 4);
		// Common fast paths.
		return SIMD::Int(
		    (staticOffsets[0] + accessSize - 1 < staticLimit) ? 0xFFFFFFFF : 0,
		    (staticOffsets[1] + accessSize - 1 < staticLimit) ? 0xFFFFFFFF : 0,
		    (staticOffsets[2] + accessSize - 1 < staticLimit) ? 0xFFFFFFFF : 0,
		    (staticOffsets[3] + accessSize - 1 < staticLimit) ? 0xFFFFFFFF : 0);
	}

	return CmpGE(offsets(), 0) & CmpLT(offsets() + SIMD::Int(accessSize - 1), limit());
}

bool SIMD::Pointer::isStaticallyInBounds(unsigned int accessSize, OutOfBoundsBehavior robustness) const
{
	if(hasDynamicOffsets)
	{
		return false;
	}

	if(hasDynamicLimit)
	{
		if(hasStaticEqualOffsets() || hasStaticSequentialOffsets(accessSize))
		{
			switch(robustness)
			{
			case OutOfBoundsBehavior::UndefinedBehavior:
				// With this robustness setting the application/compiler guarantees in-bounds accesses on active lanes,
				// but since it can't know in advance which branches are taken this must be true even for inactives lanes.
				return true;
			case OutOfBoundsBehavior::Nullify:
			case OutOfBoundsBehavior::RobustBufferAccess:
			case OutOfBoundsBehavior::UndefinedValue:
				return false;
			}
		}
	}

	for(int i = 0; i < SIMD::Width; i++)
	{
		if(staticOffsets[i] + accessSize - 1 >= staticLimit)
		{
			return false;
		}
	}

	return true;
}

SIMD::Int SIMD::Pointer::limit() const
{
	return dynamicLimit + staticLimit;
}

// Returns true if all offsets are compile-time static and sequential
// (N+0*step, N+1*step, N+2*step, N+3*step)
bool SIMD::Pointer::hasStaticSequentialOffsets(unsigned int step) const
{
	ASSERT_MSG(isBasePlusOffset, "No offsets for this type of pointer");
	if(hasDynamicOffsets)
	{
		return false;
	}

	for(int i = 1; i < SIMD::Width; i++)
	{
		if(staticOffsets[i - 1] + int32_t(step) != staticOffsets[i])
		{
			return false;
		}
	}

	return true;
}

// Returns true if all offsets are compile-time static and equal
// (N, N, N, N)
bool SIMD::Pointer::hasStaticEqualOffsets() const
{
	ASSERT_MSG(isBasePlusOffset, "No offsets for this type of pointer");
	if(hasDynamicOffsets)
	{
		return false;
	}

	for(int i = 1; i < SIMD::Width; i++)
	{
		if(staticOffsets[0] != staticOffsets[i])
		{
			return false;
		}
	}

	return true;
}

scalar::Pointer<Byte> SIMD::Pointer::getUniformPointer() const
{
#ifndef NDEBUG
	if(isBasePlusOffset)
	{
		SIMD::Int uniform = offsets();
		scalar::Int x = Extract(uniform, 0);

		for(int i = 1; i < SIMD::Width; i++)
		{
			Assert(x == Extract(uniform, i));
		}
	}
	else
	{
		for(int i = 1; i < SIMD::Width; i++)
		{
			Assert(pointers[0] == pointers[i]);
		}
	}
#endif

	return getPointerForLane(0);
}

scalar::Pointer<Byte> SIMD::Pointer::getPointerForLane(int lane) const
{
	if(isBasePlusOffset)
	{
		return base + Extract(offsets(), lane);
	}
	else
	{
		return pointers[lane];
	}
}

void SIMD::Pointer::castTo(SIMD::UInt &bits) const
{
	assert(sizeof(void *) == 4);
	for(int i = 0; i < SIMD::Width; i++)
	{
		bits = Insert(bits, As<scalar::UInt>(pointers[i]), i);
	}
}

void SIMD::Pointer::castTo(SIMD::UInt &lowerBits, SIMD::UInt &upperBits) const
{
	assert(sizeof(void *) == 8);
	for(int i = 0; i < SIMD::Width; i++)
	{
		UInt2 address = As<UInt2>(pointers[i]);
		lowerBits = Insert(lowerBits, Extract(address, 0), i);
		upperBits = Insert(upperBits, Extract(address, 1), i);
	}
}

SIMD::Pointer SIMD::Pointer::IfThenElse(SIMD::Int condition, const SIMD::Pointer &lhs, const SIMD::Pointer &rhs)
{
	std::vector<scalar::Pointer<Byte>> pointers(SIMD::Width);
	for(int i = 0; i < SIMD::Width; i++)
	{
		If(Extract(condition, i) != 0)
		{
			pointers[i] = lhs.getPointerForLane(i);
		}
		Else
		{
			pointers[i] = rhs.getPointerForLane(i);
		}
	}

	return { pointers };
}

#ifdef ENABLE_RR_PRINT
std::vector<rr::Value *> SIMD::Pointer::getPrintValues() const
{
	if(isBasePlusOffset)
	{
		return PrintValue::vals(base, offsets());
	}
	else
	{
		std::vector<Value *> vals;
		for(int i = 0; i < SIMD::Width; i++)
		{
			vals.push_back(RValue<scalar::Pointer<Byte>>(pointers[i]).value());
		}
		return vals;
	}
}
#endif

}  // namespace rr
