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

#include "Reactor.hpp"

#include "Assert.hpp"
#include "CPUID.hpp"
#include "Debug.hpp"
#include "Print.hpp"

#if defined(_WIN32)
#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN
#	endif
#	include <windows.h>
#endif

#include <algorithm>
#include <cmath>

// Define REACTOR_MATERIALIZE_LVALUES_ON_DEFINITION to non-zero to ensure all
// variables have a stack location obtained through alloca().
#ifndef REACTOR_MATERIALIZE_LVALUES_ON_DEFINITION
#	define REACTOR_MATERIALIZE_LVALUES_ON_DEFINITION 0
#endif

namespace rr {

thread_local Variable::UnmaterializedVariables *Variable::unmaterializedVariables = nullptr;

void Variable::UnmaterializedVariables::add(const Variable *v)
{
	variables.emplace(v, counter++);
}

void Variable::UnmaterializedVariables::remove(const Variable *v)
{
	auto iter = variables.find(v);
	if(iter != variables.end())
	{
		variables.erase(iter);
	}
}

void Variable::UnmaterializedVariables::clear()
{
	variables.clear();
}

void Variable::UnmaterializedVariables::materializeAll()
{
	// Flatten map of Variable* to monotonically increasing counter to a vector,
	// then sort it by the counter, so that we materialize in variable usage order.
	std::vector<std::pair<const Variable *, int>> sorted;
	sorted.resize(variables.size());
	std::copy(variables.begin(), variables.end(), sorted.begin());
	std::sort(sorted.begin(), sorted.end(), [&](auto &lhs, auto &rhs) {
		return lhs.second < rhs.second;
	});

	for(auto &v : sorted)
	{
		v.first->materialize();
	}

	variables.clear();
}

Variable::Variable(Type *type, int arraySize)
    : type(type)
    , arraySize(arraySize)
{
#if REACTOR_MATERIALIZE_LVALUES_ON_DEFINITION
	materialize();
#else
	unmaterializedVariables->add(this);
#endif
}

Variable::~Variable()
{
	// `unmaterializedVariables` can be null at this point due to the function
	// already having been finalized, while classes derived from `Function<>`
	// can have member `Variable` fields which are destructed afterwards.
	if(unmaterializedVariables)
	{
		unmaterializedVariables->remove(this);
	}
}

void Variable::materialize() const
{
	if(!address)
	{
		address = Nucleus::allocateStackVariable(getType(), arraySize);
		RR_DEBUG_INFO_EMIT_VAR(address);

		if(rvalue)
		{
			storeValue(rvalue);
			rvalue = nullptr;
		}
	}
}

Value *Variable::loadValue() const
{
	if(rvalue)
	{
		return rvalue;
	}

	if(!address)
	{
		// TODO: Return undef instead.
		materialize();
	}

	return Nucleus::createLoad(address, getType(), false, 0);
}

Value *Variable::storeValue(Value *value) const
{
	if(address)
	{
		return Nucleus::createStore(value, address, getType(), false, 0);
	}

	rvalue = value;

	return value;
}

Value *Variable::getBaseAddress() const
{
	materialize();

	return address;
}

Value *Variable::getElementPointer(Value *index, bool unsignedIndex) const
{
	return Nucleus::createGEP(getBaseAddress(), getType(), index, unsignedIndex);
}

void Variable::materializeAll()
{
	unmaterializedVariables->materializeAll();
}

void Variable::killUnmaterialized()
{
	unmaterializedVariables->clear();
}

// NOTE: Only 12 bits out of 16 of the |select| value are used.
// More specifically, the value should look like:
//
//    msb               lsb
//     v                 v
//    [.xxx|.yyy|.zzz|.www]    where '.' means an ignored bit
//
// This format makes it easy to write calls with hexadecimal select values,
// since each hex digit is a separate swizzle index.
//
// For example:
//      createShuffle4( [a,b,c,d], [e,f,g,h], 0x0123 ) -> [a,b,c,d]
//      createShuffle4( [a,b,c,d], [e,f,g,h], 0x4567 ) -> [e,f,g,h]
//      createShuffle4( [a,b,c,d], [e,f,g,h], 0x4012 ) -> [e,a,b,c]
//
static Value *createShuffle4(Value *lhs, Value *rhs, uint16_t select)
{
	std::vector<int> swizzle = {
		(select >> 12) & 0x07,
		(select >> 8) & 0x07,
		(select >> 4) & 0x07,
		(select >> 0) & 0x07,
	};

	return Nucleus::createShuffleVector(lhs, rhs, swizzle);
}

// NOTE: Only 8 bits out of 16 of the |select| value are used.
// More specifically, the value should look like:
//
//    msb               lsb
//     v                 v
//    [..xx|..yy|..zz|..ww]    where '.' means an ignored bit
//
// This format makes it easy to write calls with hexadecimal select values,
// since each hex digit is a separate swizzle index.
//
// For example:
//      createSwizzle4( [a,b,c,d], 0x0123 ) -> [a,b,c,d]
//      createSwizzle4( [a,b,c,d], 0x0033 ) -> [a,a,d,d]
//
static Value *createSwizzle4(Value *val, uint16_t select)
{
	std::vector<int> swizzle = {
		(select >> 12) & 0x03,
		(select >> 8) & 0x03,
		(select >> 4) & 0x03,
		(select >> 0) & 0x03,
	};

	return Nucleus::createShuffleVector(val, val, swizzle);
}

static Value *createMask4(Value *lhs, Value *rhs, uint16_t select)
{
	bool mask[4] = { false, false, false, false };

	mask[(select >> 12) & 0x03] = true;
	mask[(select >> 8) & 0x03] = true;
	mask[(select >> 4) & 0x03] = true;
	mask[(select >> 0) & 0x03] = true;

	std::vector<int> swizzle = {
		mask[0] ? 4 : 0,
		mask[1] ? 5 : 1,
		mask[2] ? 6 : 2,
		mask[3] ? 7 : 3,
	};

	return Nucleus::createShuffleVector(lhs, rhs, swizzle);
}

Bool::Bool(Argument<Bool> argument)
{
	store(argument.rvalue());
}

Bool::Bool(bool x)
{
	storeValue(Nucleus::createConstantBool(x));
}

Bool::Bool(RValue<Bool> rhs)
{
	store(rhs);
}

Bool::Bool(const Bool &rhs)
{
	store(rhs.load());
}

Bool::Bool(const Reference<Bool> &rhs)
{
	store(rhs.load());
}

RValue<Bool> Bool::operator=(RValue<Bool> rhs)
{
	return store(rhs);
}

RValue<Bool> Bool::operator=(const Bool &rhs)
{
	return store(rhs.load());
}

RValue<Bool> Bool::operator=(const Reference<Bool> &rhs)
{
	return store(rhs.load());
}

RValue<Bool> operator!(RValue<Bool> val)
{
	return RValue<Bool>(Nucleus::createNot(val.value()));
}

RValue<Bool> operator&&(RValue<Bool> lhs, RValue<Bool> rhs)
{
	return RValue<Bool>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<Bool> operator||(RValue<Bool> lhs, RValue<Bool> rhs)
{
	return RValue<Bool>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<Bool> operator!=(RValue<Bool> lhs, RValue<Bool> rhs)
{
	return RValue<Bool>(Nucleus::createICmpNE(lhs.value(), rhs.value()));
}

RValue<Bool> operator==(RValue<Bool> lhs, RValue<Bool> rhs)
{
	return RValue<Bool>(Nucleus::createICmpEQ(lhs.value(), rhs.value()));
}

Byte::Byte(Argument<Byte> argument)
{
	store(argument.rvalue());
}

Byte::Byte(RValue<Int> cast)
{
	Value *integer = Nucleus::createTrunc(cast.value(), Byte::type());

	storeValue(integer);
}

Byte::Byte(RValue<UInt> cast)
{
	Value *integer = Nucleus::createTrunc(cast.value(), Byte::type());

	storeValue(integer);
}

Byte::Byte(RValue<UShort> cast)
{
	Value *integer = Nucleus::createTrunc(cast.value(), Byte::type());

	storeValue(integer);
}

Byte::Byte(int x)
{
	storeValue(Nucleus::createConstantByte((unsigned char)x));
}

Byte::Byte(unsigned char x)
{
	storeValue(Nucleus::createConstantByte(x));
}

Byte::Byte(RValue<Byte> rhs)
{
	store(rhs);
}

Byte::Byte(const Byte &rhs)
{
	store(rhs.load());
}

Byte::Byte(const Reference<Byte> &rhs)
{
	store(rhs.load());
}

RValue<Byte> Byte::operator=(RValue<Byte> rhs)
{
	return store(rhs);
}

RValue<Byte> Byte::operator=(const Byte &rhs)
{
	return store(rhs.load());
}

RValue<Byte> Byte::operator=(const Reference<Byte> &rhs)
{
	return store(rhs.load());
}

RValue<Byte> operator+(RValue<Byte> lhs, RValue<Byte> rhs)
{
	return RValue<Byte>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<Byte> operator-(RValue<Byte> lhs, RValue<Byte> rhs)
{
	return RValue<Byte>(Nucleus::createSub(lhs.value(), rhs.value()));
}

RValue<Byte> operator*(RValue<Byte> lhs, RValue<Byte> rhs)
{
	return RValue<Byte>(Nucleus::createMul(lhs.value(), rhs.value()));
}

RValue<Byte> operator/(RValue<Byte> lhs, RValue<Byte> rhs)
{
	return RValue<Byte>(Nucleus::createUDiv(lhs.value(), rhs.value()));
}

RValue<Byte> operator%(RValue<Byte> lhs, RValue<Byte> rhs)
{
	return RValue<Byte>(Nucleus::createURem(lhs.value(), rhs.value()));
}

RValue<Byte> operator&(RValue<Byte> lhs, RValue<Byte> rhs)
{
	return RValue<Byte>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<Byte> operator|(RValue<Byte> lhs, RValue<Byte> rhs)
{
	return RValue<Byte>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<Byte> operator^(RValue<Byte> lhs, RValue<Byte> rhs)
{
	return RValue<Byte>(Nucleus::createXor(lhs.value(), rhs.value()));
}

RValue<Byte> operator<<(RValue<Byte> lhs, RValue<Byte> rhs)
{
	return RValue<Byte>(Nucleus::createShl(lhs.value(), rhs.value()));
}

RValue<Byte> operator>>(RValue<Byte> lhs, RValue<Byte> rhs)
{
	return RValue<Byte>(Nucleus::createLShr(lhs.value(), rhs.value()));
}

RValue<Byte> operator+=(Byte &lhs, RValue<Byte> rhs)
{
	return lhs = lhs + rhs;
}

RValue<Byte> operator-=(Byte &lhs, RValue<Byte> rhs)
{
	return lhs = lhs - rhs;
}

RValue<Byte> operator*=(Byte &lhs, RValue<Byte> rhs)
{
	return lhs = lhs * rhs;
}

RValue<Byte> operator/=(Byte &lhs, RValue<Byte> rhs)
{
	return lhs = lhs / rhs;
}

RValue<Byte> operator%=(Byte &lhs, RValue<Byte> rhs)
{
	return lhs = lhs % rhs;
}

RValue<Byte> operator&=(Byte &lhs, RValue<Byte> rhs)
{
	return lhs = lhs & rhs;
}

RValue<Byte> operator|=(Byte &lhs, RValue<Byte> rhs)
{
	return lhs = lhs | rhs;
}

RValue<Byte> operator^=(Byte &lhs, RValue<Byte> rhs)
{
	return lhs = lhs ^ rhs;
}

RValue<Byte> operator<<=(Byte &lhs, RValue<Byte> rhs)
{
	return lhs = lhs << rhs;
}

RValue<Byte> operator>>=(Byte &lhs, RValue<Byte> rhs)
{
	return lhs = lhs >> rhs;
}

RValue<Byte> operator+(RValue<Byte> val)
{
	return val;
}

RValue<Byte> operator-(RValue<Byte> val)
{
	return RValue<Byte>(Nucleus::createNeg(val.value()));
}

RValue<Byte> operator~(RValue<Byte> val)
{
	return RValue<Byte>(Nucleus::createNot(val.value()));
}

RValue<Byte> operator++(Byte &val, int)  // Post-increment
{
	RValue<Byte> res = val;

	Value *inc = Nucleus::createAdd(res.value(), Nucleus::createConstantByte((unsigned char)1));
	val.storeValue(inc);

	return res;
}

const Byte &operator++(Byte &val)  // Pre-increment
{
	Value *inc = Nucleus::createAdd(val.loadValue(), Nucleus::createConstantByte((unsigned char)1));
	val.storeValue(inc);

	return val;
}

RValue<Byte> operator--(Byte &val, int)  // Post-decrement
{
	RValue<Byte> res = val;

	Value *inc = Nucleus::createSub(res.value(), Nucleus::createConstantByte((unsigned char)1));
	val.storeValue(inc);

	return res;
}

const Byte &operator--(Byte &val)  // Pre-decrement
{
	Value *inc = Nucleus::createSub(val.loadValue(), Nucleus::createConstantByte((unsigned char)1));
	val.storeValue(inc);

	return val;
}

RValue<Bool> operator<(RValue<Byte> lhs, RValue<Byte> rhs)
{
	return RValue<Bool>(Nucleus::createICmpULT(lhs.value(), rhs.value()));
}

RValue<Bool> operator<=(RValue<Byte> lhs, RValue<Byte> rhs)
{
	return RValue<Bool>(Nucleus::createICmpULE(lhs.value(), rhs.value()));
}

RValue<Bool> operator>(RValue<Byte> lhs, RValue<Byte> rhs)
{
	return RValue<Bool>(Nucleus::createICmpUGT(lhs.value(), rhs.value()));
}

RValue<Bool> operator>=(RValue<Byte> lhs, RValue<Byte> rhs)
{
	return RValue<Bool>(Nucleus::createICmpUGE(lhs.value(), rhs.value()));
}

RValue<Bool> operator!=(RValue<Byte> lhs, RValue<Byte> rhs)
{
	return RValue<Bool>(Nucleus::createICmpNE(lhs.value(), rhs.value()));
}

RValue<Bool> operator==(RValue<Byte> lhs, RValue<Byte> rhs)
{
	return RValue<Bool>(Nucleus::createICmpEQ(lhs.value(), rhs.value()));
}

SByte::SByte(Argument<SByte> argument)
{
	store(argument.rvalue());
}

SByte::SByte(RValue<Int> cast)
{
	Value *integer = Nucleus::createTrunc(cast.value(), SByte::type());

	storeValue(integer);
}

SByte::SByte(RValue<Short> cast)
{
	Value *integer = Nucleus::createTrunc(cast.value(), SByte::type());

	storeValue(integer);
}

SByte::SByte(signed char x)
{
	storeValue(Nucleus::createConstantByte(x));
}

SByte::SByte(RValue<SByte> rhs)
{
	store(rhs);
}

SByte::SByte(const SByte &rhs)
{
	store(rhs.load());
}

SByte::SByte(const Reference<SByte> &rhs)
{
	store(rhs.load());
}

RValue<SByte> SByte::operator=(RValue<SByte> rhs)
{
	return store(rhs);
}

RValue<SByte> SByte::operator=(const SByte &rhs)
{
	return store(rhs.load());
}

RValue<SByte> SByte::operator=(const Reference<SByte> &rhs)
{
	return store(rhs.load());
}

RValue<SByte> operator+(RValue<SByte> lhs, RValue<SByte> rhs)
{
	return RValue<SByte>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<SByte> operator-(RValue<SByte> lhs, RValue<SByte> rhs)
{
	return RValue<SByte>(Nucleus::createSub(lhs.value(), rhs.value()));
}

RValue<SByte> operator*(RValue<SByte> lhs, RValue<SByte> rhs)
{
	return RValue<SByte>(Nucleus::createMul(lhs.value(), rhs.value()));
}

RValue<SByte> operator/(RValue<SByte> lhs, RValue<SByte> rhs)
{
	return RValue<SByte>(Nucleus::createSDiv(lhs.value(), rhs.value()));
}

RValue<SByte> operator%(RValue<SByte> lhs, RValue<SByte> rhs)
{
	return RValue<SByte>(Nucleus::createSRem(lhs.value(), rhs.value()));
}

RValue<SByte> operator&(RValue<SByte> lhs, RValue<SByte> rhs)
{
	return RValue<SByte>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<SByte> operator|(RValue<SByte> lhs, RValue<SByte> rhs)
{
	return RValue<SByte>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<SByte> operator^(RValue<SByte> lhs, RValue<SByte> rhs)
{
	return RValue<SByte>(Nucleus::createXor(lhs.value(), rhs.value()));
}

RValue<SByte> operator<<(RValue<SByte> lhs, RValue<SByte> rhs)
{
	return RValue<SByte>(Nucleus::createShl(lhs.value(), rhs.value()));
}

RValue<SByte> operator>>(RValue<SByte> lhs, RValue<SByte> rhs)
{
	return RValue<SByte>(Nucleus::createAShr(lhs.value(), rhs.value()));
}

RValue<SByte> operator+=(SByte &lhs, RValue<SByte> rhs)
{
	return lhs = lhs + rhs;
}

RValue<SByte> operator-=(SByte &lhs, RValue<SByte> rhs)
{
	return lhs = lhs - rhs;
}

RValue<SByte> operator*=(SByte &lhs, RValue<SByte> rhs)
{
	return lhs = lhs * rhs;
}

RValue<SByte> operator/=(SByte &lhs, RValue<SByte> rhs)
{
	return lhs = lhs / rhs;
}

RValue<SByte> operator%=(SByte &lhs, RValue<SByte> rhs)
{
	return lhs = lhs % rhs;
}

RValue<SByte> operator&=(SByte &lhs, RValue<SByte> rhs)
{
	return lhs = lhs & rhs;
}

RValue<SByte> operator|=(SByte &lhs, RValue<SByte> rhs)
{
	return lhs = lhs | rhs;
}

RValue<SByte> operator^=(SByte &lhs, RValue<SByte> rhs)
{
	return lhs = lhs ^ rhs;
}

RValue<SByte> operator<<=(SByte &lhs, RValue<SByte> rhs)
{
	return lhs = lhs << rhs;
}

RValue<SByte> operator>>=(SByte &lhs, RValue<SByte> rhs)
{
	return lhs = lhs >> rhs;
}

RValue<SByte> operator+(RValue<SByte> val)
{
	return val;
}

RValue<SByte> operator-(RValue<SByte> val)
{
	return RValue<SByte>(Nucleus::createNeg(val.value()));
}

RValue<SByte> operator~(RValue<SByte> val)
{
	return RValue<SByte>(Nucleus::createNot(val.value()));
}

RValue<SByte> operator++(SByte &val, int)  // Post-increment
{
	RValue<SByte> res = val;

	Value *inc = Nucleus::createAdd(res.value(), Nucleus::createConstantByte((signed char)1));
	val.storeValue(inc);

	return res;
}

const SByte &operator++(SByte &val)  // Pre-increment
{
	Value *inc = Nucleus::createAdd(val.loadValue(), Nucleus::createConstantByte((signed char)1));
	val.storeValue(inc);

	return val;
}

RValue<SByte> operator--(SByte &val, int)  // Post-decrement
{
	RValue<SByte> res = val;

	Value *inc = Nucleus::createSub(res.value(), Nucleus::createConstantByte((signed char)1));
	val.storeValue(inc);

	return res;
}

const SByte &operator--(SByte &val)  // Pre-decrement
{
	Value *inc = Nucleus::createSub(val.loadValue(), Nucleus::createConstantByte((signed char)1));
	val.storeValue(inc);

	return val;
}

RValue<Bool> operator<(RValue<SByte> lhs, RValue<SByte> rhs)
{
	return RValue<Bool>(Nucleus::createICmpSLT(lhs.value(), rhs.value()));
}

RValue<Bool> operator<=(RValue<SByte> lhs, RValue<SByte> rhs)
{
	return RValue<Bool>(Nucleus::createICmpSLE(lhs.value(), rhs.value()));
}

RValue<Bool> operator>(RValue<SByte> lhs, RValue<SByte> rhs)
{
	return RValue<Bool>(Nucleus::createICmpSGT(lhs.value(), rhs.value()));
}

RValue<Bool> operator>=(RValue<SByte> lhs, RValue<SByte> rhs)
{
	return RValue<Bool>(Nucleus::createICmpSGE(lhs.value(), rhs.value()));
}

RValue<Bool> operator!=(RValue<SByte> lhs, RValue<SByte> rhs)
{
	return RValue<Bool>(Nucleus::createICmpNE(lhs.value(), rhs.value()));
}

RValue<Bool> operator==(RValue<SByte> lhs, RValue<SByte> rhs)
{
	return RValue<Bool>(Nucleus::createICmpEQ(lhs.value(), rhs.value()));
}

Short::Short(Argument<Short> argument)
{
	store(argument.rvalue());
}

Short::Short(RValue<Int> cast)
{
	Value *integer = Nucleus::createTrunc(cast.value(), Short::type());

	storeValue(integer);
}

Short::Short(short x)
{
	storeValue(Nucleus::createConstantShort(x));
}

Short::Short(RValue<Short> rhs)
{
	store(rhs);
}

Short::Short(const Short &rhs)
{
	store(rhs.load());
}

Short::Short(const Reference<Short> &rhs)
{
	store(rhs.load());
}

RValue<Short> Short::operator=(RValue<Short> rhs)
{
	return store(rhs);
}

RValue<Short> Short::operator=(const Short &rhs)
{
	return store(rhs.load());
}

RValue<Short> Short::operator=(const Reference<Short> &rhs)
{
	return store(rhs.load());
}

RValue<Short> operator+(RValue<Short> lhs, RValue<Short> rhs)
{
	return RValue<Short>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<Short> operator-(RValue<Short> lhs, RValue<Short> rhs)
{
	return RValue<Short>(Nucleus::createSub(lhs.value(), rhs.value()));
}

RValue<Short> operator*(RValue<Short> lhs, RValue<Short> rhs)
{
	return RValue<Short>(Nucleus::createMul(lhs.value(), rhs.value()));
}

RValue<Short> operator/(RValue<Short> lhs, RValue<Short> rhs)
{
	return RValue<Short>(Nucleus::createSDiv(lhs.value(), rhs.value()));
}

RValue<Short> operator%(RValue<Short> lhs, RValue<Short> rhs)
{
	return RValue<Short>(Nucleus::createSRem(lhs.value(), rhs.value()));
}

RValue<Short> operator&(RValue<Short> lhs, RValue<Short> rhs)
{
	return RValue<Short>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<Short> operator|(RValue<Short> lhs, RValue<Short> rhs)
{
	return RValue<Short>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<Short> operator^(RValue<Short> lhs, RValue<Short> rhs)
{
	return RValue<Short>(Nucleus::createXor(lhs.value(), rhs.value()));
}

RValue<Short> operator<<(RValue<Short> lhs, RValue<Short> rhs)
{
	return RValue<Short>(Nucleus::createShl(lhs.value(), rhs.value()));
}

RValue<Short> operator>>(RValue<Short> lhs, RValue<Short> rhs)
{
	return RValue<Short>(Nucleus::createAShr(lhs.value(), rhs.value()));
}

RValue<Short> operator+=(Short &lhs, RValue<Short> rhs)
{
	return lhs = lhs + rhs;
}

RValue<Short> operator-=(Short &lhs, RValue<Short> rhs)
{
	return lhs = lhs - rhs;
}

RValue<Short> operator*=(Short &lhs, RValue<Short> rhs)
{
	return lhs = lhs * rhs;
}

RValue<Short> operator/=(Short &lhs, RValue<Short> rhs)
{
	return lhs = lhs / rhs;
}

RValue<Short> operator%=(Short &lhs, RValue<Short> rhs)
{
	return lhs = lhs % rhs;
}

RValue<Short> operator&=(Short &lhs, RValue<Short> rhs)
{
	return lhs = lhs & rhs;
}

RValue<Short> operator|=(Short &lhs, RValue<Short> rhs)
{
	return lhs = lhs | rhs;
}

RValue<Short> operator^=(Short &lhs, RValue<Short> rhs)
{
	return lhs = lhs ^ rhs;
}

RValue<Short> operator<<=(Short &lhs, RValue<Short> rhs)
{
	return lhs = lhs << rhs;
}

RValue<Short> operator>>=(Short &lhs, RValue<Short> rhs)
{
	return lhs = lhs >> rhs;
}

RValue<Short> operator+(RValue<Short> val)
{
	return val;
}

RValue<Short> operator-(RValue<Short> val)
{
	return RValue<Short>(Nucleus::createNeg(val.value()));
}

RValue<Short> operator~(RValue<Short> val)
{
	return RValue<Short>(Nucleus::createNot(val.value()));
}

RValue<Short> operator++(Short &val, int)  // Post-increment
{
	RValue<Short> res = val;

	Value *inc = Nucleus::createAdd(res.value(), Nucleus::createConstantShort((short)1));
	val.storeValue(inc);

	return res;
}

const Short &operator++(Short &val)  // Pre-increment
{
	Value *inc = Nucleus::createAdd(val.loadValue(), Nucleus::createConstantShort((short)1));
	val.storeValue(inc);

	return val;
}

RValue<Short> operator--(Short &val, int)  // Post-decrement
{
	RValue<Short> res = val;

	Value *inc = Nucleus::createSub(res.value(), Nucleus::createConstantShort((short)1));
	val.storeValue(inc);

	return res;
}

const Short &operator--(Short &val)  // Pre-decrement
{
	Value *inc = Nucleus::createSub(val.loadValue(), Nucleus::createConstantShort((short)1));
	val.storeValue(inc);

	return val;
}

RValue<Bool> operator<(RValue<Short> lhs, RValue<Short> rhs)
{
	return RValue<Bool>(Nucleus::createICmpSLT(lhs.value(), rhs.value()));
}

RValue<Bool> operator<=(RValue<Short> lhs, RValue<Short> rhs)
{
	return RValue<Bool>(Nucleus::createICmpSLE(lhs.value(), rhs.value()));
}

RValue<Bool> operator>(RValue<Short> lhs, RValue<Short> rhs)
{
	return RValue<Bool>(Nucleus::createICmpSGT(lhs.value(), rhs.value()));
}

RValue<Bool> operator>=(RValue<Short> lhs, RValue<Short> rhs)
{
	return RValue<Bool>(Nucleus::createICmpSGE(lhs.value(), rhs.value()));
}

RValue<Bool> operator!=(RValue<Short> lhs, RValue<Short> rhs)
{
	return RValue<Bool>(Nucleus::createICmpNE(lhs.value(), rhs.value()));
}

RValue<Bool> operator==(RValue<Short> lhs, RValue<Short> rhs)
{
	return RValue<Bool>(Nucleus::createICmpEQ(lhs.value(), rhs.value()));
}

UShort::UShort(Argument<UShort> argument)
{
	store(argument.rvalue());
}

UShort::UShort(RValue<UInt> cast)
{
	Value *integer = Nucleus::createTrunc(cast.value(), UShort::type());

	storeValue(integer);
}

UShort::UShort(RValue<Int> cast)
{
	Value *integer = Nucleus::createTrunc(cast.value(), UShort::type());

	storeValue(integer);
}

UShort::UShort(RValue<Byte> cast)
{
	Value *integer = Nucleus::createZExt(cast.value(), UShort::type());

	storeValue(integer);
}

UShort::UShort(unsigned short x)
{
	storeValue(Nucleus::createConstantShort(x));
}

UShort::UShort(RValue<UShort> rhs)
{
	store(rhs);
}

UShort::UShort(const UShort &rhs)
{
	store(rhs.load());
}

UShort::UShort(const Reference<UShort> &rhs)
{
	store(rhs.load());
}

RValue<UShort> UShort::operator=(RValue<UShort> rhs)
{
	return store(rhs);
}

RValue<UShort> UShort::operator=(const UShort &rhs)
{
	return store(rhs.load());
}

RValue<UShort> UShort::operator=(const Reference<UShort> &rhs)
{
	return store(rhs.load());
}

RValue<UShort> operator+(RValue<UShort> lhs, RValue<UShort> rhs)
{
	return RValue<UShort>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<UShort> operator-(RValue<UShort> lhs, RValue<UShort> rhs)
{
	return RValue<UShort>(Nucleus::createSub(lhs.value(), rhs.value()));
}

RValue<UShort> operator*(RValue<UShort> lhs, RValue<UShort> rhs)
{
	return RValue<UShort>(Nucleus::createMul(lhs.value(), rhs.value()));
}

RValue<UShort> operator/(RValue<UShort> lhs, RValue<UShort> rhs)
{
	return RValue<UShort>(Nucleus::createUDiv(lhs.value(), rhs.value()));
}

RValue<UShort> operator%(RValue<UShort> lhs, RValue<UShort> rhs)
{
	return RValue<UShort>(Nucleus::createURem(lhs.value(), rhs.value()));
}

RValue<UShort> operator&(RValue<UShort> lhs, RValue<UShort> rhs)
{
	return RValue<UShort>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<UShort> operator|(RValue<UShort> lhs, RValue<UShort> rhs)
{
	return RValue<UShort>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<UShort> operator^(RValue<UShort> lhs, RValue<UShort> rhs)
{
	return RValue<UShort>(Nucleus::createXor(lhs.value(), rhs.value()));
}

RValue<UShort> operator<<(RValue<UShort> lhs, RValue<UShort> rhs)
{
	return RValue<UShort>(Nucleus::createShl(lhs.value(), rhs.value()));
}

RValue<UShort> operator>>(RValue<UShort> lhs, RValue<UShort> rhs)
{
	return RValue<UShort>(Nucleus::createLShr(lhs.value(), rhs.value()));
}

RValue<UShort> operator+=(UShort &lhs, RValue<UShort> rhs)
{
	return lhs = lhs + rhs;
}

RValue<UShort> operator-=(UShort &lhs, RValue<UShort> rhs)
{
	return lhs = lhs - rhs;
}

RValue<UShort> operator*=(UShort &lhs, RValue<UShort> rhs)
{
	return lhs = lhs * rhs;
}

RValue<UShort> operator/=(UShort &lhs, RValue<UShort> rhs)
{
	return lhs = lhs / rhs;
}

RValue<UShort> operator%=(UShort &lhs, RValue<UShort> rhs)
{
	return lhs = lhs % rhs;
}

RValue<UShort> operator&=(UShort &lhs, RValue<UShort> rhs)
{
	return lhs = lhs & rhs;
}

RValue<UShort> operator|=(UShort &lhs, RValue<UShort> rhs)
{
	return lhs = lhs | rhs;
}

RValue<UShort> operator^=(UShort &lhs, RValue<UShort> rhs)
{
	return lhs = lhs ^ rhs;
}

RValue<UShort> operator<<=(UShort &lhs, RValue<UShort> rhs)
{
	return lhs = lhs << rhs;
}

RValue<UShort> operator>>=(UShort &lhs, RValue<UShort> rhs)
{
	return lhs = lhs >> rhs;
}

RValue<UShort> operator+(RValue<UShort> val)
{
	return val;
}

RValue<UShort> operator-(RValue<UShort> val)
{
	return RValue<UShort>(Nucleus::createNeg(val.value()));
}

RValue<UShort> operator~(RValue<UShort> val)
{
	return RValue<UShort>(Nucleus::createNot(val.value()));
}

RValue<UShort> operator++(UShort &val, int)  // Post-increment
{
	RValue<UShort> res = val;

	Value *inc = Nucleus::createAdd(res.value(), Nucleus::createConstantShort((unsigned short)1));
	val.storeValue(inc);

	return res;
}

const UShort &operator++(UShort &val)  // Pre-increment
{
	Value *inc = Nucleus::createAdd(val.loadValue(), Nucleus::createConstantShort((unsigned short)1));
	val.storeValue(inc);

	return val;
}

RValue<UShort> operator--(UShort &val, int)  // Post-decrement
{
	RValue<UShort> res = val;

	Value *inc = Nucleus::createSub(res.value(), Nucleus::createConstantShort((unsigned short)1));
	val.storeValue(inc);

	return res;
}

const UShort &operator--(UShort &val)  // Pre-decrement
{
	Value *inc = Nucleus::createSub(val.loadValue(), Nucleus::createConstantShort((unsigned short)1));
	val.storeValue(inc);

	return val;
}

RValue<Bool> operator<(RValue<UShort> lhs, RValue<UShort> rhs)
{
	return RValue<Bool>(Nucleus::createICmpULT(lhs.value(), rhs.value()));
}

RValue<Bool> operator<=(RValue<UShort> lhs, RValue<UShort> rhs)
{
	return RValue<Bool>(Nucleus::createICmpULE(lhs.value(), rhs.value()));
}

RValue<Bool> operator>(RValue<UShort> lhs, RValue<UShort> rhs)
{
	return RValue<Bool>(Nucleus::createICmpUGT(lhs.value(), rhs.value()));
}

RValue<Bool> operator>=(RValue<UShort> lhs, RValue<UShort> rhs)
{
	return RValue<Bool>(Nucleus::createICmpUGE(lhs.value(), rhs.value()));
}

RValue<Bool> operator!=(RValue<UShort> lhs, RValue<UShort> rhs)
{
	return RValue<Bool>(Nucleus::createICmpNE(lhs.value(), rhs.value()));
}

RValue<Bool> operator==(RValue<UShort> lhs, RValue<UShort> rhs)
{
	return RValue<Bool>(Nucleus::createICmpEQ(lhs.value(), rhs.value()));
}

Byte4::Byte4(RValue<Byte8> cast)
{
	storeValue(Nucleus::createBitCast(cast.value(), type()));
}

Byte4::Byte4(RValue<UShort4> cast)
{
	// TODO(b/148379603): Optimize narrowing swizzle.
	*this = As<Byte4>(Swizzle(As<Byte8>(cast), 0x0246'0246));
}

Byte4::Byte4(RValue<Short4> cast)
{
	// TODO(b/148379603): Optimize narrowing swizzle.
	*this = As<Byte4>(Swizzle(As<Byte8>(cast), 0x0246'0246));
}

Byte4::Byte4(RValue<UInt4> cast)
{
	// TODO(b/148379603): Optimize narrowing swizzle.
	*this = As<Byte4>(Swizzle(As<Byte16>(cast), 0x048C'048C'048C'048C));
}

Byte4::Byte4(RValue<Int4> cast)
{
	// TODO(b/148379603): Optimize narrowing swizzle.
	*this = As<Byte4>(Swizzle(As<Byte16>(cast), 0x048C'048C'048C'048C));
}

Byte4::Byte4(RValue<Byte4> rhs)
{
	store(rhs);
}

Byte4::Byte4(const Byte4 &rhs)
{
	store(rhs.load());
}

Byte4::Byte4(const Reference<Byte4> &rhs)
{
	store(rhs.load());
}

RValue<Byte4> Byte4::operator=(RValue<Byte4> rhs)
{
	return store(rhs);
}

RValue<Byte4> Byte4::operator=(const Byte4 &rhs)
{
	return store(rhs.load());
}

RValue<Byte4> Insert(RValue<Byte4> val, RValue<Byte> element, int i)
{
	return RValue<Byte4>(Nucleus::createInsertElement(val.value(), element.value(), i));
}

Byte8::Byte8(uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4, uint8_t x5, uint8_t x6, uint8_t x7)
{
	std::vector<int64_t> constantVector = { x0, x1, x2, x3, x4, x5, x6, x7 };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

Byte8::Byte8(RValue<Byte8> rhs)
{
	store(rhs);
}

Byte8::Byte8(const Byte8 &rhs)
{
	store(rhs.load());
}

Byte8::Byte8(const Reference<Byte8> &rhs)
{
	store(rhs.load());
}

RValue<Byte8> Byte8::operator=(RValue<Byte8> rhs)
{
	return store(rhs);
}

RValue<Byte8> Byte8::operator=(const Byte8 &rhs)
{
	return store(rhs.load());
}

RValue<Byte8> Byte8::operator=(const Reference<Byte8> &rhs)
{
	return store(rhs.load());
}

RValue<Byte8> operator+(RValue<Byte8> lhs, RValue<Byte8> rhs)
{
	return RValue<Byte8>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<Byte8> operator-(RValue<Byte8> lhs, RValue<Byte8> rhs)
{
	return RValue<Byte8>(Nucleus::createSub(lhs.value(), rhs.value()));
}

//	RValue<Byte8> operator*(RValue<Byte8> lhs, RValue<Byte8> rhs)
//	{
//		return RValue<Byte8>(Nucleus::createMul(lhs.value(), rhs.value()));
//	}

//	RValue<Byte8> operator/(RValue<Byte8> lhs, RValue<Byte8> rhs)
//	{
//		return RValue<Byte8>(Nucleus::createUDiv(lhs.value(), rhs.value()));
//	}

//	RValue<Byte8> operator%(RValue<Byte8> lhs, RValue<Byte8> rhs)
//	{
//		return RValue<Byte8>(Nucleus::createURem(lhs.value(), rhs.value()));
//	}

RValue<Byte8> operator&(RValue<Byte8> lhs, RValue<Byte8> rhs)
{
	return RValue<Byte8>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<Byte8> operator|(RValue<Byte8> lhs, RValue<Byte8> rhs)
{
	return RValue<Byte8>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<Byte8> operator^(RValue<Byte8> lhs, RValue<Byte8> rhs)
{
	return RValue<Byte8>(Nucleus::createXor(lhs.value(), rhs.value()));
}

//	RValue<Byte8> operator<<(RValue<Byte8> lhs, unsigned char rhs)
//	{
//		return RValue<Byte8>(Nucleus::createShl(lhs.value(), rhs.value()));
//	}

//	RValue<Byte8> operator>>(RValue<Byte8> lhs, unsigned char rhs)
//	{
//		return RValue<Byte8>(Nucleus::createLShr(lhs.value(), rhs.value()));
//	}

RValue<Byte8> operator+=(Byte8 &lhs, RValue<Byte8> rhs)
{
	return lhs = lhs + rhs;
}

RValue<Byte8> operator-=(Byte8 &lhs, RValue<Byte8> rhs)
{
	return lhs = lhs - rhs;
}

//	RValue<Byte8> operator*=(Byte8 &lhs, RValue<Byte8> rhs)
//	{
//		return lhs = lhs * rhs;
//	}

//	RValue<Byte8> operator/=(Byte8 &lhs, RValue<Byte8> rhs)
//	{
//		return lhs = lhs / rhs;
//	}

//	RValue<Byte8> operator%=(Byte8 &lhs, RValue<Byte8> rhs)
//	{
//		return lhs = lhs % rhs;
//	}

RValue<Byte8> operator&=(Byte8 &lhs, RValue<Byte8> rhs)
{
	return lhs = lhs & rhs;
}

RValue<Byte8> operator|=(Byte8 &lhs, RValue<Byte8> rhs)
{
	return lhs = lhs | rhs;
}

RValue<Byte8> operator^=(Byte8 &lhs, RValue<Byte8> rhs)
{
	return lhs = lhs ^ rhs;
}

//	RValue<Byte8> operator<<=(Byte8 &lhs, RValue<Byte8> rhs)
//	{
//		return lhs = lhs << rhs;
//	}

//	RValue<Byte8> operator>>=(Byte8 &lhs, RValue<Byte8> rhs)
//	{
//		return lhs = lhs >> rhs;
//	}

//	RValue<Byte8> operator+(RValue<Byte8> val)
//	{
//		return val;
//	}

//	RValue<Byte8> operator-(RValue<Byte8> val)
//	{
//		return RValue<Byte8>(Nucleus::createNeg(val.value()));
//	}

RValue<Byte8> operator~(RValue<Byte8> val)
{
	return RValue<Byte8>(Nucleus::createNot(val.value()));
}

RValue<Byte8> Swizzle(RValue<Byte8> x, uint32_t select)
{
	// Real type is v16i8
	// TODO(b/148379603): Optimize narrowing swizzle.
	std::vector<int> shuffle = {
		static_cast<int>((select >> 28) & 0x07),
		static_cast<int>((select >> 24) & 0x07),
		static_cast<int>((select >> 20) & 0x07),
		static_cast<int>((select >> 16) & 0x07),
		static_cast<int>((select >> 12) & 0x07),
		static_cast<int>((select >> 8) & 0x07),
		static_cast<int>((select >> 4) & 0x07),
		static_cast<int>((select >> 0) & 0x07),
		static_cast<int>((select >> 28) & 0x07),
		static_cast<int>((select >> 24) & 0x07),
		static_cast<int>((select >> 20) & 0x07),
		static_cast<int>((select >> 16) & 0x07),
		static_cast<int>((select >> 12) & 0x07),
		static_cast<int>((select >> 8) & 0x07),
		static_cast<int>((select >> 4) & 0x07),
		static_cast<int>((select >> 0) & 0x07),
	};

	return As<Byte8>(Nucleus::createShuffleVector(x.value(), x.value(), shuffle));
}

RValue<Short4> Unpack(RValue<Byte4> x)
{
	// TODO(b/148379603): Optimize narrowing swizzle.
	std::vector<int> shuffle = { 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7 };  // Real type is v16i8
	return As<Short4>(Nucleus::createShuffleVector(x.value(), x.value(), shuffle));
}

RValue<Short4> Unpack(RValue<Byte4> x, RValue<Byte4> y)
{
	return UnpackLow(As<Byte8>(x), As<Byte8>(y));
}

RValue<Short4> UnpackLow(RValue<Byte8> x, RValue<Byte8> y)
{
	// TODO(b/148379603): Optimize narrowing swizzle.
	std::vector<int> shuffle = { 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23 };  // Real type is v16i8
	return As<Short4>(Nucleus::createShuffleVector(x.value(), y.value(), shuffle));
}

RValue<Short4> UnpackHigh(RValue<Byte8> x, RValue<Byte8> y)
{
	// TODO(b/148379603): Optimize narrowing swizzle.
	std::vector<int> shuffle = { 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23 };  // Real type is v16i8
	auto lowHigh = RValue<Byte16>(Nucleus::createShuffleVector(x.value(), y.value(), shuffle));
	return As<Short4>(Swizzle(As<Int4>(lowHigh), 0x2323));
}

SByte8::SByte8(uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4, uint8_t x5, uint8_t x6, uint8_t x7)
{
	std::vector<int64_t> constantVector = { x0, x1, x2, x3, x4, x5, x6, x7 };
	Value *vector = Nucleus::createConstantVector(constantVector, type());

	storeValue(Nucleus::createBitCast(vector, type()));
}

SByte8::SByte8(RValue<SByte8> rhs)
{
	store(rhs);
}

SByte8::SByte8(const SByte8 &rhs)
{
	store(rhs.load());
}

SByte8::SByte8(const Reference<SByte8> &rhs)
{
	store(rhs.load());
}

RValue<SByte8> SByte8::operator=(RValue<SByte8> rhs)
{
	return store(rhs);
}

RValue<SByte8> SByte8::operator=(const SByte8 &rhs)
{
	return store(rhs.load());
}

RValue<SByte8> SByte8::operator=(const Reference<SByte8> &rhs)
{
	return store(rhs.load());
}

RValue<SByte8> operator+(RValue<SByte8> lhs, RValue<SByte8> rhs)
{
	return RValue<SByte8>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<SByte8> operator-(RValue<SByte8> lhs, RValue<SByte8> rhs)
{
	return RValue<SByte8>(Nucleus::createSub(lhs.value(), rhs.value()));
}

//	RValue<SByte8> operator*(RValue<SByte8> lhs, RValue<SByte8> rhs)
//	{
//		return RValue<SByte8>(Nucleus::createMul(lhs.value(), rhs.value()));
//	}

//	RValue<SByte8> operator/(RValue<SByte8> lhs, RValue<SByte8> rhs)
//	{
//		return RValue<SByte8>(Nucleus::createSDiv(lhs.value(), rhs.value()));
//	}

//	RValue<SByte8> operator%(RValue<SByte8> lhs, RValue<SByte8> rhs)
//	{
//		return RValue<SByte8>(Nucleus::createSRem(lhs.value(), rhs.value()));
//	}

RValue<SByte8> operator&(RValue<SByte8> lhs, RValue<SByte8> rhs)
{
	return RValue<SByte8>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<SByte8> operator|(RValue<SByte8> lhs, RValue<SByte8> rhs)
{
	return RValue<SByte8>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<SByte8> operator^(RValue<SByte8> lhs, RValue<SByte8> rhs)
{
	return RValue<SByte8>(Nucleus::createXor(lhs.value(), rhs.value()));
}

//	RValue<SByte8> operator<<(RValue<SByte8> lhs, unsigned char rhs)
//	{
//		return RValue<SByte8>(Nucleus::createShl(lhs.value(), rhs.value()));
//	}

//	RValue<SByte8> operator>>(RValue<SByte8> lhs, unsigned char rhs)
//	{
//		return RValue<SByte8>(Nucleus::createAShr(lhs.value(), rhs.value()));
//	}

RValue<SByte8> operator+=(SByte8 &lhs, RValue<SByte8> rhs)
{
	return lhs = lhs + rhs;
}

RValue<SByte8> operator-=(SByte8 &lhs, RValue<SByte8> rhs)
{
	return lhs = lhs - rhs;
}

//	RValue<SByte8> operator*=(SByte8 &lhs, RValue<SByte8> rhs)
//	{
//		return lhs = lhs * rhs;
//	}

//	RValue<SByte8> operator/=(SByte8 &lhs, RValue<SByte8> rhs)
//	{
//		return lhs = lhs / rhs;
//	}

//	RValue<SByte8> operator%=(SByte8 &lhs, RValue<SByte8> rhs)
//	{
//		return lhs = lhs % rhs;
//	}

RValue<SByte8> operator&=(SByte8 &lhs, RValue<SByte8> rhs)
{
	return lhs = lhs & rhs;
}

RValue<SByte8> operator|=(SByte8 &lhs, RValue<SByte8> rhs)
{
	return lhs = lhs | rhs;
}

RValue<SByte8> operator^=(SByte8 &lhs, RValue<SByte8> rhs)
{
	return lhs = lhs ^ rhs;
}

//	RValue<SByte8> operator<<=(SByte8 &lhs, RValue<SByte8> rhs)
//	{
//		return lhs = lhs << rhs;
//	}

//	RValue<SByte8> operator>>=(SByte8 &lhs, RValue<SByte8> rhs)
//	{
//		return lhs = lhs >> rhs;
//	}

//	RValue<SByte8> operator+(RValue<SByte8> val)
//	{
//		return val;
//	}

//	RValue<SByte8> operator-(RValue<SByte8> val)
//	{
//		return RValue<SByte8>(Nucleus::createNeg(val.value()));
//	}

RValue<SByte8> operator~(RValue<SByte8> val)
{
	return RValue<SByte8>(Nucleus::createNot(val.value()));
}

RValue<Short4> UnpackLow(RValue<SByte8> x, RValue<SByte8> y)
{
	// TODO(b/148379603): Optimize narrowing swizzle.
	std::vector<int> shuffle = { 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23 };  // Real type is v16i8
	return As<Short4>(Nucleus::createShuffleVector(x.value(), y.value(), shuffle));
}

RValue<Short4> UnpackHigh(RValue<SByte8> x, RValue<SByte8> y)
{
	// TODO(b/148379603): Optimize narrowing swizzle.
	std::vector<int> shuffle = { 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23 };  // Real type is v16i8
	auto lowHigh = RValue<Byte16>(Nucleus::createShuffleVector(x.value(), y.value(), shuffle));
	return As<Short4>(Swizzle(As<Int4>(lowHigh), 0x2323));
}

Byte16::Byte16(RValue<Byte16> rhs)
{
	store(rhs);
}

Byte16::Byte16(const Byte16 &rhs)
{
	store(rhs.load());
}

Byte16::Byte16(const Reference<Byte16> &rhs)
{
	store(rhs.load());
}

RValue<Byte16> Byte16::operator=(RValue<Byte16> rhs)
{
	return store(rhs);
}

RValue<Byte16> Byte16::operator=(const Byte16 &rhs)
{
	return store(rhs.load());
}

RValue<Byte16> Byte16::operator=(const Reference<Byte16> &rhs)
{
	return store(rhs.load());
}

RValue<Byte16> Swizzle(RValue<Byte16> x, uint64_t select)
{
	std::vector<int> shuffle = {
		static_cast<int>((select >> 60) & 0x0F),
		static_cast<int>((select >> 56) & 0x0F),
		static_cast<int>((select >> 52) & 0x0F),
		static_cast<int>((select >> 48) & 0x0F),
		static_cast<int>((select >> 44) & 0x0F),
		static_cast<int>((select >> 40) & 0x0F),
		static_cast<int>((select >> 36) & 0x0F),
		static_cast<int>((select >> 32) & 0x0F),
		static_cast<int>((select >> 28) & 0x0F),
		static_cast<int>((select >> 24) & 0x0F),
		static_cast<int>((select >> 20) & 0x0F),
		static_cast<int>((select >> 16) & 0x0F),
		static_cast<int>((select >> 12) & 0x0F),
		static_cast<int>((select >> 8) & 0x0F),
		static_cast<int>((select >> 4) & 0x0F),
		static_cast<int>((select >> 0) & 0x0F),
	};

	return As<Byte16>(Nucleus::createShuffleVector(x.value(), x.value(), shuffle));
}

Short2::Short2(RValue<Short4> cast)
{
	storeValue(Nucleus::createBitCast(cast.value(), type()));
}

UShort2::UShort2(RValue<UShort4> cast)
{
	storeValue(Nucleus::createBitCast(cast.value(), type()));
}

Short4::Short4(RValue<Int> cast)
{
	Value *vector = loadValue();
	Value *element = Nucleus::createTrunc(cast.value(), Short::type());
	Value *insert = Nucleus::createInsertElement(vector, element, 0);
	Value *swizzle = Swizzle(RValue<Short4>(insert), 0x0000).value();

	storeValue(swizzle);
}

Short4::Short4(RValue<UInt4> cast)
    : Short4(As<Int4>(cast))
{
}

//	Short4::Short4(RValue<Float> cast)
//	{
//	}

Short4::Short4(short xyzw)
{
	std::vector<int64_t> constantVector = { xyzw };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

Short4::Short4(short x, short y, short z, short w)
{
	std::vector<int64_t> constantVector = { x, y, z, w };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

Short4::Short4(RValue<Short4> rhs)
{
	store(rhs);
}

Short4::Short4(const Short4 &rhs)
{
	store(rhs.load());
}

Short4::Short4(const Reference<Short4> &rhs)
{
	store(rhs.load());
}

Short4::Short4(RValue<UShort4> rhs)
{
	storeValue(rhs.value());
}

Short4::Short4(const UShort4 &rhs)
{
	storeValue(rhs.loadValue());
}

Short4::Short4(const Reference<UShort4> &rhs)
{
	storeValue(rhs.loadValue());
}

RValue<Short4> Short4::operator=(RValue<Short4> rhs)
{
	return store(rhs);
}

RValue<Short4> Short4::operator=(const Short4 &rhs)
{
	return store(rhs.load());
}

RValue<Short4> Short4::operator=(const Reference<Short4> &rhs)
{
	return store(rhs.load());
}

RValue<Short4> Short4::operator=(RValue<UShort4> rhs)
{
	return RValue<Short4>(storeValue(rhs.value()));
}

RValue<Short4> Short4::operator=(const UShort4 &rhs)
{
	return RValue<Short4>(storeValue(rhs.loadValue()));
}

RValue<Short4> Short4::operator=(const Reference<UShort4> &rhs)
{
	return RValue<Short4>(storeValue(rhs.loadValue()));
}

RValue<Short4> operator+(RValue<Short4> lhs, RValue<Short4> rhs)
{
	return RValue<Short4>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<Short4> operator-(RValue<Short4> lhs, RValue<Short4> rhs)
{
	return RValue<Short4>(Nucleus::createSub(lhs.value(), rhs.value()));
}

RValue<Short4> operator*(RValue<Short4> lhs, RValue<Short4> rhs)
{
	return RValue<Short4>(Nucleus::createMul(lhs.value(), rhs.value()));
}

//	RValue<Short4> operator/(RValue<Short4> lhs, RValue<Short4> rhs)
//	{
//		return RValue<Short4>(Nucleus::createSDiv(lhs.value(), rhs.value()));
//	}

//	RValue<Short4> operator%(RValue<Short4> lhs, RValue<Short4> rhs)
//	{
//		return RValue<Short4>(Nucleus::createSRem(lhs.value(), rhs.value()));
//	}

RValue<Short4> operator&(RValue<Short4> lhs, RValue<Short4> rhs)
{
	return RValue<Short4>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<Short4> operator|(RValue<Short4> lhs, RValue<Short4> rhs)
{
	return RValue<Short4>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<Short4> operator^(RValue<Short4> lhs, RValue<Short4> rhs)
{
	return RValue<Short4>(Nucleus::createXor(lhs.value(), rhs.value()));
}

RValue<Short4> operator+=(Short4 &lhs, RValue<Short4> rhs)
{
	return lhs = lhs + rhs;
}

RValue<Short4> operator-=(Short4 &lhs, RValue<Short4> rhs)
{
	return lhs = lhs - rhs;
}

RValue<Short4> operator*=(Short4 &lhs, RValue<Short4> rhs)
{
	return lhs = lhs * rhs;
}

//	RValue<Short4> operator/=(Short4 &lhs, RValue<Short4> rhs)
//	{
//		return lhs = lhs / rhs;
//	}

//	RValue<Short4> operator%=(Short4 &lhs, RValue<Short4> rhs)
//	{
//		return lhs = lhs % rhs;
//	}

RValue<Short4> operator&=(Short4 &lhs, RValue<Short4> rhs)
{
	return lhs = lhs & rhs;
}

RValue<Short4> operator|=(Short4 &lhs, RValue<Short4> rhs)
{
	return lhs = lhs | rhs;
}

RValue<Short4> operator^=(Short4 &lhs, RValue<Short4> rhs)
{
	return lhs = lhs ^ rhs;
}

RValue<Short4> operator<<=(Short4 &lhs, unsigned char rhs)
{
	return lhs = lhs << rhs;
}

RValue<Short4> operator>>=(Short4 &lhs, unsigned char rhs)
{
	return lhs = lhs >> rhs;
}

//	RValue<Short4> operator+(RValue<Short4> val)
//	{
//		return val;
//	}

RValue<Short4> operator-(RValue<Short4> val)
{
	return RValue<Short4>(Nucleus::createNeg(val.value()));
}

RValue<Short4> operator~(RValue<Short4> val)
{
	return RValue<Short4>(Nucleus::createNot(val.value()));
}

RValue<Short4> RoundShort4(RValue<Float4> cast)
{
	RValue<Int4> int4 = RoundInt(cast);
	return As<Short4>(PackSigned(int4, int4));
}

RValue<Int2> UnpackLow(RValue<Short4> x, RValue<Short4> y)
{
	std::vector<int> shuffle = { 0, 8, 1, 9, 2, 10, 3, 11 };  // Real type is v8i16
	return As<Int2>(Nucleus::createShuffleVector(x.value(), y.value(), shuffle));
}

RValue<Int2> UnpackHigh(RValue<Short4> x, RValue<Short4> y)
{
	// TODO(b/148379603): Optimize narrowing swizzle.
	std::vector<int> shuffle = { 0, 8, 1, 9, 2, 10, 3, 11 };  // Real type is v8i16
	auto lowHigh = RValue<Short8>(Nucleus::createShuffleVector(x.value(), y.value(), shuffle));
	return As<Int2>(Swizzle(As<Int4>(lowHigh), 0x2323));
}

RValue<Short4> Swizzle(RValue<Short4> x, uint16_t select)
{
	// Real type is v8i16
	// TODO(b/148379603): Optimize narrowing swizzle.
	std::vector<int> shuffle = {
		(select >> 12) & 0x03,
		(select >> 8) & 0x03,
		(select >> 4) & 0x03,
		(select >> 0) & 0x03,
		(select >> 12) & 0x03,
		(select >> 8) & 0x03,
		(select >> 4) & 0x03,
		(select >> 0) & 0x03,
	};

	return As<Short4>(Nucleus::createShuffleVector(x.value(), x.value(), shuffle));
}

RValue<Short4> Insert(RValue<Short4> val, RValue<Short> element, int i)
{
	return RValue<Short4>(Nucleus::createInsertElement(val.value(), element.value(), i));
}

RValue<Short> Extract(RValue<Short4> val, int i)
{
	return RValue<Short>(Nucleus::createExtractElement(val.value(), Short::type(), i));
}

UShort4::UShort4(RValue<UInt4> cast)
    : UShort4(As<Int4>(cast))
{
}

UShort4::UShort4(RValue<Int4> cast)
{
	*this = Short4(cast);
}

UShort4::UShort4(unsigned short xyzw)
{
	std::vector<int64_t> constantVector = { xyzw };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

UShort4::UShort4(unsigned short x, unsigned short y, unsigned short z, unsigned short w)
{
	std::vector<int64_t> constantVector = { x, y, z, w };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

UShort4::UShort4(RValue<UShort4> rhs)
{
	store(rhs);
}

UShort4::UShort4(const UShort4 &rhs)
{
	store(rhs.load());
}

UShort4::UShort4(const Reference<UShort4> &rhs)
{
	store(rhs.load());
}

UShort4::UShort4(RValue<Short4> rhs)
{
	storeValue(rhs.value());
}

UShort4::UShort4(const Short4 &rhs)
{
	storeValue(rhs.loadValue());
}

UShort4::UShort4(const Reference<Short4> &rhs)
{
	storeValue(rhs.loadValue());
}

RValue<UShort4> UShort4::operator=(RValue<UShort4> rhs)
{
	return store(rhs);
}

RValue<UShort4> UShort4::operator=(const UShort4 &rhs)
{
	return store(rhs.load());
}

RValue<UShort4> UShort4::operator=(const Reference<UShort4> &rhs)
{
	return store(rhs.load());
}

RValue<UShort4> UShort4::operator=(RValue<Short4> rhs)
{
	return RValue<UShort4>(storeValue(rhs.value()));
}

RValue<UShort4> UShort4::operator=(const Short4 &rhs)
{
	return RValue<UShort4>(storeValue(rhs.loadValue()));
}

RValue<UShort4> UShort4::operator=(const Reference<Short4> &rhs)
{
	return RValue<UShort4>(storeValue(rhs.loadValue()));
}

RValue<UShort4> operator+(RValue<UShort4> lhs, RValue<UShort4> rhs)
{
	return RValue<UShort4>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<UShort4> operator-(RValue<UShort4> lhs, RValue<UShort4> rhs)
{
	return RValue<UShort4>(Nucleus::createSub(lhs.value(), rhs.value()));
}

RValue<UShort4> operator*(RValue<UShort4> lhs, RValue<UShort4> rhs)
{
	return RValue<UShort4>(Nucleus::createMul(lhs.value(), rhs.value()));
}

RValue<UShort4> operator&(RValue<UShort4> lhs, RValue<UShort4> rhs)
{
	return RValue<UShort4>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<UShort4> operator|(RValue<UShort4> lhs, RValue<UShort4> rhs)
{
	return RValue<UShort4>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<UShort4> operator^(RValue<UShort4> lhs, RValue<UShort4> rhs)
{
	return RValue<UShort4>(Nucleus::createXor(lhs.value(), rhs.value()));
}

RValue<UShort4> operator<<=(UShort4 &lhs, unsigned char rhs)
{
	return lhs = lhs << rhs;
}

RValue<UShort4> operator>>=(UShort4 &lhs, unsigned char rhs)
{
	return lhs = lhs >> rhs;
}

RValue<UShort4> operator~(RValue<UShort4> val)
{
	return RValue<UShort4>(Nucleus::createNot(val.value()));
}

RValue<UShort4> Insert(RValue<UShort4> val, RValue<UShort> element, int i)
{
	return RValue<UShort4>(Nucleus::createInsertElement(val.value(), element.value(), i));
}

Short8::Short8(short c)
{
	std::vector<int64_t> constantVector = { c };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

Short8::Short8(short c0, short c1, short c2, short c3, short c4, short c5, short c6, short c7)
{
	std::vector<int64_t> constantVector = { c0, c1, c2, c3, c4, c5, c6, c7 };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

Short8::Short8(RValue<Short8> rhs)
{
	store(rhs);
}

Short8::Short8(const Reference<Short8> &rhs)
{
	store(rhs.load());
}

Short8::Short8(RValue<Short4> lo, RValue<Short4> hi)
{
	std::vector<int> shuffle = { 0, 1, 2, 3, 8, 9, 10, 11 };  // Real type is v8i16
	Value *packed = Nucleus::createShuffleVector(lo.value(), hi.value(), shuffle);

	storeValue(packed);
}

RValue<Short8> Short8::operator=(RValue<Short8> rhs)
{
	return store(rhs);
}

RValue<Short8> Short8::operator=(const Short8 &rhs)
{
	return store(rhs.load());
}

RValue<Short8> Short8::operator=(const Reference<Short8> &rhs)
{
	return store(rhs.load());
}

RValue<Short8> operator+(RValue<Short8> lhs, RValue<Short8> rhs)
{
	return RValue<Short8>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<Short8> operator&(RValue<Short8> lhs, RValue<Short8> rhs)
{
	return RValue<Short8>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

UShort8::UShort8(unsigned short c)
{
	std::vector<int64_t> constantVector = { c };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

UShort8::UShort8(unsigned short c0, unsigned short c1, unsigned short c2, unsigned short c3, unsigned short c4, unsigned short c5, unsigned short c6, unsigned short c7)
{
	std::vector<int64_t> constantVector = { c0, c1, c2, c3, c4, c5, c6, c7 };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

UShort8::UShort8(RValue<UShort8> rhs)
{
	store(rhs);
}

UShort8::UShort8(const Reference<UShort8> &rhs)
{
	store(rhs.load());
}

UShort8::UShort8(RValue<UShort4> lo, RValue<UShort4> hi)
{
	std::vector<int> shuffle = { 0, 1, 2, 3, 8, 9, 10, 11 };  // Real type is v8i16
	Value *packed = Nucleus::createShuffleVector(lo.value(), hi.value(), shuffle);

	storeValue(packed);
}

RValue<UShort8> UShort8::operator=(RValue<UShort8> rhs)
{
	return store(rhs);
}

RValue<UShort8> UShort8::operator=(const UShort8 &rhs)
{
	return store(rhs.load());
}

RValue<UShort8> UShort8::operator=(const Reference<UShort8> &rhs)
{
	return store(rhs.load());
}

RValue<UShort8> operator&(RValue<UShort8> lhs, RValue<UShort8> rhs)
{
	return RValue<UShort8>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<UShort8> operator+(RValue<UShort8> lhs, RValue<UShort8> rhs)
{
	return RValue<UShort8>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<UShort8> operator*(RValue<UShort8> lhs, RValue<UShort8> rhs)
{
	return RValue<UShort8>(Nucleus::createMul(lhs.value(), rhs.value()));
}

RValue<UShort8> operator+=(UShort8 &lhs, RValue<UShort8> rhs)
{
	return lhs = lhs + rhs;
}

RValue<UShort8> operator~(RValue<UShort8> val)
{
	return RValue<UShort8>(Nucleus::createNot(val.value()));
}

RValue<UShort8> Swizzle(RValue<UShort8> x, uint32_t select)
{
	std::vector<int> swizzle = {
		static_cast<int>((select >> 28) & 0x07),
		static_cast<int>((select >> 24) & 0x07),
		static_cast<int>((select >> 20) & 0x07),
		static_cast<int>((select >> 16) & 0x07),
		static_cast<int>((select >> 12) & 0x07),
		static_cast<int>((select >> 8) & 0x07),
		static_cast<int>((select >> 4) & 0x07),
		static_cast<int>((select >> 0) & 0x07),
	};

	return RValue<UShort8>(Nucleus::createShuffleVector(x.value(), x.value(), swizzle));
}

Int::Int(Argument<Int> argument)
{
	store(argument.rvalue());
}

Int::Int(RValue<Byte> cast)
{
	Value *integer = Nucleus::createZExt(cast.value(), Int::type());

	storeValue(integer);
}

Int::Int(RValue<SByte> cast)
{
	Value *integer = Nucleus::createSExt(cast.value(), Int::type());

	storeValue(integer);
}

Int::Int(RValue<Short> cast)
{
	Value *integer = Nucleus::createSExt(cast.value(), Int::type());

	storeValue(integer);
}

Int::Int(RValue<UShort> cast)
{
	Value *integer = Nucleus::createZExt(cast.value(), Int::type());

	storeValue(integer);
}

Int::Int(RValue<Int2> cast)
{
	*this = Extract(cast, 0);
}

Int::Int(RValue<Long> cast)
{
	Value *integer = Nucleus::createTrunc(cast.value(), Int::type());

	storeValue(integer);
}

Int::Int(RValue<Float> cast)
{
	Value *integer = Nucleus::createFPToSI(cast.value(), Int::type());

	storeValue(integer);
}

Int::Int(int x)
{
	storeValue(Nucleus::createConstantInt(x));
}

Int::Int(RValue<Int> rhs)
{
	store(rhs);
}

Int::Int(RValue<UInt> rhs)
{
	storeValue(rhs.value());
}

Int::Int(const Int &rhs)
{
	store(rhs.load());
}

Int::Int(const Reference<Int> &rhs)
{
	store(rhs.load());
}

Int::Int(const UInt &rhs)
{
	storeValue(rhs.loadValue());
}

Int::Int(const Reference<UInt> &rhs)
{
	storeValue(rhs.loadValue());
}

RValue<Int> Int::operator=(int rhs)
{
	return RValue<Int>(storeValue(Nucleus::createConstantInt(rhs)));
}

RValue<Int> Int::operator=(RValue<Int> rhs)
{
	return store(rhs);
}

RValue<Int> Int::operator=(RValue<UInt> rhs)
{
	storeValue(rhs.value());

	return RValue<Int>(rhs);
}

RValue<Int> Int::operator=(const Int &rhs)
{
	return store(rhs.load());
}

RValue<Int> Int::operator=(const Reference<Int> &rhs)
{
	return store(rhs.load());
}

RValue<Int> Int::operator=(const UInt &rhs)
{
	return RValue<Int>(storeValue(rhs.loadValue()));
}

RValue<Int> Int::operator=(const Reference<UInt> &rhs)
{
	return RValue<Int>(storeValue(rhs.loadValue()));
}

RValue<Int> operator+(RValue<Int> lhs, RValue<Int> rhs)
{
	return RValue<Int>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<Int> operator-(RValue<Int> lhs, RValue<Int> rhs)
{
	return RValue<Int>(Nucleus::createSub(lhs.value(), rhs.value()));
}

RValue<Int> operator*(RValue<Int> lhs, RValue<Int> rhs)
{
	return RValue<Int>(Nucleus::createMul(lhs.value(), rhs.value()));
}

RValue<Int> operator/(RValue<Int> lhs, RValue<Int> rhs)
{
	return RValue<Int>(Nucleus::createSDiv(lhs.value(), rhs.value()));
}

RValue<Int> operator%(RValue<Int> lhs, RValue<Int> rhs)
{
	return RValue<Int>(Nucleus::createSRem(lhs.value(), rhs.value()));
}

RValue<Int> operator&(RValue<Int> lhs, RValue<Int> rhs)
{
	return RValue<Int>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<Int> operator|(RValue<Int> lhs, RValue<Int> rhs)
{
	return RValue<Int>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<Int> operator^(RValue<Int> lhs, RValue<Int> rhs)
{
	return RValue<Int>(Nucleus::createXor(lhs.value(), rhs.value()));
}

RValue<Int> operator<<(RValue<Int> lhs, RValue<Int> rhs)
{
	return RValue<Int>(Nucleus::createShl(lhs.value(), rhs.value()));
}

RValue<Int> operator>>(RValue<Int> lhs, RValue<Int> rhs)
{
	return RValue<Int>(Nucleus::createAShr(lhs.value(), rhs.value()));
}

RValue<Int> operator+=(Int &lhs, RValue<Int> rhs)
{
	return lhs = lhs + rhs;
}

RValue<Int> operator-=(Int &lhs, RValue<Int> rhs)
{
	return lhs = lhs - rhs;
}

RValue<Int> operator*=(Int &lhs, RValue<Int> rhs)
{
	return lhs = lhs * rhs;
}

RValue<Int> operator/=(Int &lhs, RValue<Int> rhs)
{
	return lhs = lhs / rhs;
}

RValue<Int> operator%=(Int &lhs, RValue<Int> rhs)
{
	return lhs = lhs % rhs;
}

RValue<Int> operator&=(Int &lhs, RValue<Int> rhs)
{
	return lhs = lhs & rhs;
}

RValue<Int> operator|=(Int &lhs, RValue<Int> rhs)
{
	return lhs = lhs | rhs;
}

RValue<Int> operator^=(Int &lhs, RValue<Int> rhs)
{
	return lhs = lhs ^ rhs;
}

RValue<Int> operator<<=(Int &lhs, RValue<Int> rhs)
{
	return lhs = lhs << rhs;
}

RValue<Int> operator>>=(Int &lhs, RValue<Int> rhs)
{
	return lhs = lhs >> rhs;
}

RValue<Int> operator+(RValue<Int> val)
{
	return val;
}

RValue<Int> operator-(RValue<Int> val)
{
	return RValue<Int>(Nucleus::createNeg(val.value()));
}

RValue<Int> operator~(RValue<Int> val)
{
	return RValue<Int>(Nucleus::createNot(val.value()));
}

RValue<Bool> operator<(RValue<Int> lhs, RValue<Int> rhs)
{
	return RValue<Bool>(Nucleus::createICmpSLT(lhs.value(), rhs.value()));
}

RValue<Bool> operator<=(RValue<Int> lhs, RValue<Int> rhs)
{
	return RValue<Bool>(Nucleus::createICmpSLE(lhs.value(), rhs.value()));
}

RValue<Bool> operator>(RValue<Int> lhs, RValue<Int> rhs)
{
	return RValue<Bool>(Nucleus::createICmpSGT(lhs.value(), rhs.value()));
}

RValue<Bool> operator>=(RValue<Int> lhs, RValue<Int> rhs)
{
	return RValue<Bool>(Nucleus::createICmpSGE(lhs.value(), rhs.value()));
}

RValue<Bool> operator!=(RValue<Int> lhs, RValue<Int> rhs)
{
	return RValue<Bool>(Nucleus::createICmpNE(lhs.value(), rhs.value()));
}

RValue<Bool> operator==(RValue<Int> lhs, RValue<Int> rhs)
{
	return RValue<Bool>(Nucleus::createICmpEQ(lhs.value(), rhs.value()));
}

RValue<Int> Max(RValue<Int> x, RValue<Int> y)
{
	return IfThenElse(x > y, x, y);
}

RValue<Int> Min(RValue<Int> x, RValue<Int> y)
{
	return IfThenElse(x < y, x, y);
}

RValue<Int> Clamp(RValue<Int> x, RValue<Int> min, RValue<Int> max)
{
	return Min(Max(x, min), max);
}

Long::Long(RValue<Int> cast)
{
	Value *integer = Nucleus::createSExt(cast.value(), Long::type());

	storeValue(integer);
}

Long::Long(RValue<UInt> cast)
{
	Value *integer = Nucleus::createZExt(cast.value(), Long::type());

	storeValue(integer);
}

Long::Long(RValue<Long> rhs)
{
	store(rhs);
}

RValue<Long> Long::operator=(int64_t rhs)
{
	return RValue<Long>(storeValue(Nucleus::createConstantLong(rhs)));
}

RValue<Long> Long::operator=(RValue<Long> rhs)
{
	return store(rhs);
}

RValue<Long> Long::operator=(const Long &rhs)
{
	return store(rhs.load());
}

RValue<Long> Long::operator=(const Reference<Long> &rhs)
{
	return store(rhs.load());
}

RValue<Long> operator+(RValue<Long> lhs, RValue<Long> rhs)
{
	return RValue<Long>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<Long> operator-(RValue<Long> lhs, RValue<Long> rhs)
{
	return RValue<Long>(Nucleus::createSub(lhs.value(), rhs.value()));
}

RValue<Long> operator*(RValue<Long> lhs, RValue<Long> rhs)
{
	return RValue<Long>(Nucleus::createMul(lhs.value(), rhs.value()));
}

RValue<Long> operator>>(RValue<Long> lhs, RValue<Long> rhs)
{
	return RValue<Long>(Nucleus::createAShr(lhs.value(), rhs.value()));
}

RValue<Long> operator+=(Long &lhs, RValue<Long> rhs)
{
	return lhs = lhs + rhs;
}

RValue<Long> operator-=(Long &lhs, RValue<Long> rhs)
{
	return lhs = lhs - rhs;
}

RValue<Long> AddAtomic(RValue<Pointer<Long>> x, RValue<Long> y)
{
	return RValue<Long>(Nucleus::createAtomicAdd(x.value(), y.value()));
}

RValue<UInt> AddAtomic(RValue<Pointer<UInt>> x, RValue<UInt> y, std::memory_order memoryOrder)
{
	return RValue<UInt>(Nucleus::createAtomicAdd(x.value(), y.value(), memoryOrder));
}

RValue<UInt> SubAtomic(RValue<Pointer<UInt>> x, RValue<UInt> y, std::memory_order memoryOrder)
{
	return RValue<UInt>(Nucleus::createAtomicSub(x.value(), y.value(), memoryOrder));
}

RValue<UInt> AndAtomic(RValue<Pointer<UInt>> x, RValue<UInt> y, std::memory_order memoryOrder)
{
	return RValue<UInt>(Nucleus::createAtomicAnd(x.value(), y.value(), memoryOrder));
}

RValue<UInt> OrAtomic(RValue<Pointer<UInt>> x, RValue<UInt> y, std::memory_order memoryOrder)
{
	return RValue<UInt>(Nucleus::createAtomicOr(x.value(), y.value(), memoryOrder));
}

RValue<UInt> XorAtomic(RValue<Pointer<UInt>> x, RValue<UInt> y, std::memory_order memoryOrder)
{
	return RValue<UInt>(Nucleus::createAtomicXor(x.value(), y.value(), memoryOrder));
}

RValue<UInt> ExchangeAtomic(RValue<Pointer<UInt>> x, RValue<UInt> y, std::memory_order memoryOrder)
{
	return RValue<UInt>(Nucleus::createAtomicExchange(x.value(), y.value(), memoryOrder));
}

RValue<UInt> CompareExchangeAtomic(RValue<Pointer<UInt>> x, RValue<UInt> y, RValue<UInt> compare, std::memory_order memoryOrderEqual, std::memory_order memoryOrderUnequal)
{
	return RValue<UInt>(Nucleus::createAtomicCompareExchange(x.value(), y.value(), compare.value(), memoryOrderEqual, memoryOrderUnequal));
}

UInt::UInt(Argument<UInt> argument)
{
	store(argument.rvalue());
}

UInt::UInt(RValue<UShort> cast)
{
	Value *integer = Nucleus::createZExt(cast.value(), UInt::type());

	storeValue(integer);
}

UInt::UInt(RValue<Long> cast)
{
	Value *integer = Nucleus::createTrunc(cast.value(), UInt::type());

	storeValue(integer);
}

UInt::UInt(int x)
{
	storeValue(Nucleus::createConstantInt(x));
}

UInt::UInt(unsigned int x)
{
	storeValue(Nucleus::createConstantInt(x));
}

UInt::UInt(RValue<UInt> rhs)
{
	store(rhs);
}

UInt::UInt(RValue<Int> rhs)
{
	storeValue(rhs.value());
}

UInt::UInt(const UInt &rhs)
{
	store(rhs.load());
}

UInt::UInt(const Reference<UInt> &rhs)
{
	store(rhs.load());
}

UInt::UInt(const Int &rhs)
{
	storeValue(rhs.loadValue());
}

UInt::UInt(const Reference<Int> &rhs)
{
	storeValue(rhs.loadValue());
}

RValue<UInt> UInt::operator=(unsigned int rhs)
{
	return RValue<UInt>(storeValue(Nucleus::createConstantInt(rhs)));
}

RValue<UInt> UInt::operator=(RValue<UInt> rhs)
{
	return store(rhs);
}

RValue<UInt> UInt::operator=(RValue<Int> rhs)
{
	storeValue(rhs.value());

	return RValue<UInt>(rhs);
}

RValue<UInt> UInt::operator=(const UInt &rhs)
{
	return store(rhs.load());
}

RValue<UInt> UInt::operator=(const Reference<UInt> &rhs)
{
	return store(rhs.load());
}

RValue<UInt> UInt::operator=(const Int &rhs)
{
	return RValue<UInt>(storeValue(rhs.loadValue()));
}

RValue<UInt> UInt::operator=(const Reference<Int> &rhs)
{
	return RValue<UInt>(storeValue(rhs.loadValue()));
}

RValue<UInt> operator+(RValue<UInt> lhs, RValue<UInt> rhs)
{
	return RValue<UInt>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<UInt> operator-(RValue<UInt> lhs, RValue<UInt> rhs)
{
	return RValue<UInt>(Nucleus::createSub(lhs.value(), rhs.value()));
}

RValue<UInt> operator*(RValue<UInt> lhs, RValue<UInt> rhs)
{
	return RValue<UInt>(Nucleus::createMul(lhs.value(), rhs.value()));
}

RValue<UInt> operator/(RValue<UInt> lhs, RValue<UInt> rhs)
{
	return RValue<UInt>(Nucleus::createUDiv(lhs.value(), rhs.value()));
}

RValue<UInt> operator%(RValue<UInt> lhs, RValue<UInt> rhs)
{
	return RValue<UInt>(Nucleus::createURem(lhs.value(), rhs.value()));
}

RValue<UInt> operator&(RValue<UInt> lhs, RValue<UInt> rhs)
{
	return RValue<UInt>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<UInt> operator|(RValue<UInt> lhs, RValue<UInt> rhs)
{
	return RValue<UInt>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<UInt> operator^(RValue<UInt> lhs, RValue<UInt> rhs)
{
	return RValue<UInt>(Nucleus::createXor(lhs.value(), rhs.value()));
}

RValue<UInt> operator<<(RValue<UInt> lhs, RValue<UInt> rhs)
{
	return RValue<UInt>(Nucleus::createShl(lhs.value(), rhs.value()));
}

RValue<UInt> operator>>(RValue<UInt> lhs, RValue<UInt> rhs)
{
	return RValue<UInt>(Nucleus::createLShr(lhs.value(), rhs.value()));
}

RValue<UInt> operator+=(UInt &lhs, RValue<UInt> rhs)
{
	return lhs = lhs + rhs;
}

RValue<UInt> operator-=(UInt &lhs, RValue<UInt> rhs)
{
	return lhs = lhs - rhs;
}

RValue<UInt> operator*=(UInt &lhs, RValue<UInt> rhs)
{
	return lhs = lhs * rhs;
}

RValue<UInt> operator/=(UInt &lhs, RValue<UInt> rhs)
{
	return lhs = lhs / rhs;
}

RValue<UInt> operator%=(UInt &lhs, RValue<UInt> rhs)
{
	return lhs = lhs % rhs;
}

RValue<UInt> operator&=(UInt &lhs, RValue<UInt> rhs)
{
	return lhs = lhs & rhs;
}

RValue<UInt> operator|=(UInt &lhs, RValue<UInt> rhs)
{
	return lhs = lhs | rhs;
}

RValue<UInt> operator^=(UInt &lhs, RValue<UInt> rhs)
{
	return lhs = lhs ^ rhs;
}

RValue<UInt> operator<<=(UInt &lhs, RValue<UInt> rhs)
{
	return lhs = lhs << rhs;
}

RValue<UInt> operator>>=(UInt &lhs, RValue<UInt> rhs)
{
	return lhs = lhs >> rhs;
}

RValue<UInt> operator+(RValue<UInt> val)
{
	return val;
}

RValue<UInt> operator-(RValue<UInt> val)
{
	return RValue<UInt>(Nucleus::createNeg(val.value()));
}

RValue<UInt> operator~(RValue<UInt> val)
{
	return RValue<UInt>(Nucleus::createNot(val.value()));
}

RValue<UInt> Max(RValue<UInt> x, RValue<UInt> y)
{
	return IfThenElse(x > y, x, y);
}

RValue<UInt> Min(RValue<UInt> x, RValue<UInt> y)
{
	return IfThenElse(x < y, x, y);
}

RValue<UInt> Clamp(RValue<UInt> x, RValue<UInt> min, RValue<UInt> max)
{
	return Min(Max(x, min), max);
}

RValue<Bool> operator<(RValue<UInt> lhs, RValue<UInt> rhs)
{
	return RValue<Bool>(Nucleus::createICmpULT(lhs.value(), rhs.value()));
}

RValue<Bool> operator<=(RValue<UInt> lhs, RValue<UInt> rhs)
{
	return RValue<Bool>(Nucleus::createICmpULE(lhs.value(), rhs.value()));
}

RValue<Bool> operator>(RValue<UInt> lhs, RValue<UInt> rhs)
{
	return RValue<Bool>(Nucleus::createICmpUGT(lhs.value(), rhs.value()));
}

RValue<Bool> operator>=(RValue<UInt> lhs, RValue<UInt> rhs)
{
	return RValue<Bool>(Nucleus::createICmpUGE(lhs.value(), rhs.value()));
}

RValue<Bool> operator!=(RValue<UInt> lhs, RValue<UInt> rhs)
{
	return RValue<Bool>(Nucleus::createICmpNE(lhs.value(), rhs.value()));
}

RValue<Bool> operator==(RValue<UInt> lhs, RValue<UInt> rhs)
{
	return RValue<Bool>(Nucleus::createICmpEQ(lhs.value(), rhs.value()));
}

Int2::Int2(RValue<Int4> cast)
{
	storeValue(Nucleus::createBitCast(cast.value(), type()));
}

Int2::Int2(int x, int y)
{
	std::vector<int64_t> constantVector = { x, y };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

Int2::Int2(RValue<Int2> rhs)
{
	store(rhs);
}

Int2::Int2(const Int2 &rhs)
{
	store(rhs.load());
}

Int2::Int2(const Reference<Int2> &rhs)
{
	store(rhs.load());
}

Int2::Int2(RValue<Int> lo, RValue<Int> hi)
{
	std::vector<int> shuffle = { 0, 4, 1, 5 };
	Value *packed = Nucleus::createShuffleVector(Int4(lo).loadValue(), Int4(hi).loadValue(), shuffle);

	storeValue(Nucleus::createBitCast(packed, Int2::type()));
}

RValue<Int2> Int2::operator=(RValue<Int2> rhs)
{
	return store(rhs);
}

RValue<Int2> Int2::operator=(const Int2 &rhs)
{
	return store(rhs.load());
}

RValue<Int2> Int2::operator=(const Reference<Int2> &rhs)
{
	return store(rhs.load());
}

RValue<Int2> operator+(RValue<Int2> lhs, RValue<Int2> rhs)
{
	return RValue<Int2>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<Int2> operator-(RValue<Int2> lhs, RValue<Int2> rhs)
{
	return RValue<Int2>(Nucleus::createSub(lhs.value(), rhs.value()));
}

//	RValue<Int2> operator*(RValue<Int2> lhs, RValue<Int2> rhs)
//	{
//		return RValue<Int2>(Nucleus::createMul(lhs.value(), rhs.value()));
//	}

//	RValue<Int2> operator/(RValue<Int2> lhs, RValue<Int2> rhs)
//	{
//		return RValue<Int2>(Nucleus::createSDiv(lhs.value(), rhs.value()));
//	}

//	RValue<Int2> operator%(RValue<Int2> lhs, RValue<Int2> rhs)
//	{
//		return RValue<Int2>(Nucleus::createSRem(lhs.value(), rhs.value()));
//	}

RValue<Int2> operator&(RValue<Int2> lhs, RValue<Int2> rhs)
{
	return RValue<Int2>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<Int2> operator|(RValue<Int2> lhs, RValue<Int2> rhs)
{
	return RValue<Int2>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<Int2> operator^(RValue<Int2> lhs, RValue<Int2> rhs)
{
	return RValue<Int2>(Nucleus::createXor(lhs.value(), rhs.value()));
}

RValue<Int2> operator+=(Int2 &lhs, RValue<Int2> rhs)
{
	return lhs = lhs + rhs;
}

RValue<Int2> operator-=(Int2 &lhs, RValue<Int2> rhs)
{
	return lhs = lhs - rhs;
}

//	RValue<Int2> operator*=(Int2 &lhs, RValue<Int2> rhs)
//	{
//		return lhs = lhs * rhs;
//	}

//	RValue<Int2> operator/=(Int2 &lhs, RValue<Int2> rhs)
//	{
//		return lhs = lhs / rhs;
//	}

//	RValue<Int2> operator%=(Int2 &lhs, RValue<Int2> rhs)
//	{
//		return lhs = lhs % rhs;
//	}

RValue<Int2> operator&=(Int2 &lhs, RValue<Int2> rhs)
{
	return lhs = lhs & rhs;
}

RValue<Int2> operator|=(Int2 &lhs, RValue<Int2> rhs)
{
	return lhs = lhs | rhs;
}

RValue<Int2> operator^=(Int2 &lhs, RValue<Int2> rhs)
{
	return lhs = lhs ^ rhs;
}

RValue<Int2> operator<<=(Int2 &lhs, unsigned char rhs)
{
	return lhs = lhs << rhs;
}

RValue<Int2> operator>>=(Int2 &lhs, unsigned char rhs)
{
	return lhs = lhs >> rhs;
}

//	RValue<Int2> operator+(RValue<Int2> val)
//	{
//		return val;
//	}

//	RValue<Int2> operator-(RValue<Int2> val)
//	{
//		return RValue<Int2>(Nucleus::createNeg(val.value()));
//	}

RValue<Int2> operator~(RValue<Int2> val)
{
	return RValue<Int2>(Nucleus::createNot(val.value()));
}

RValue<Short4> UnpackLow(RValue<Int2> x, RValue<Int2> y)
{
	// TODO(b/148379603): Optimize narrowing swizzle.
	std::vector<int> shuffle = { 0, 4, 1, 5 };  // Real type is v4i32
	return As<Short4>(Nucleus::createShuffleVector(x.value(), y.value(), shuffle));
}

RValue<Short4> UnpackHigh(RValue<Int2> x, RValue<Int2> y)
{
	// TODO(b/148379603): Optimize narrowing swizzle.
	std::vector<int> shuffle = { 0, 4, 1, 5 };  // Real type is v4i32
	auto lowHigh = RValue<Int4>(Nucleus::createShuffleVector(x.value(), y.value(), shuffle));
	return As<Short4>(Swizzle(lowHigh, 0x2323));
}

RValue<Int> Extract(RValue<Int2> val, int i)
{
	return RValue<Int>(Nucleus::createExtractElement(val.value(), Int::type(), i));
}

RValue<Int2> Insert(RValue<Int2> val, RValue<Int> element, int i)
{
	return RValue<Int2>(Nucleus::createInsertElement(val.value(), element.value(), i));
}

UInt2::UInt2(unsigned int x, unsigned int y)
{
	std::vector<int64_t> constantVector = { x, y };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

UInt2::UInt2(RValue<UInt2> rhs)
{
	store(rhs);
}

UInt2::UInt2(const UInt2 &rhs)
{
	store(rhs.load());
}

UInt2::UInt2(const Reference<UInt2> &rhs)
{
	store(rhs.load());
}

RValue<UInt2> UInt2::operator=(RValue<UInt2> rhs)
{
	return store(rhs);
}

RValue<UInt2> UInt2::operator=(const UInt2 &rhs)
{
	return store(rhs.load());
}

RValue<UInt2> UInt2::operator=(const Reference<UInt2> &rhs)
{
	return store(rhs.load());
}

RValue<UInt2> operator+(RValue<UInt2> lhs, RValue<UInt2> rhs)
{
	return RValue<UInt2>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<UInt2> operator-(RValue<UInt2> lhs, RValue<UInt2> rhs)
{
	return RValue<UInt2>(Nucleus::createSub(lhs.value(), rhs.value()));
}

//	RValue<UInt2> operator*(RValue<UInt2> lhs, RValue<UInt2> rhs)
//	{
//		return RValue<UInt2>(Nucleus::createMul(lhs.value(), rhs.value()));
//	}

//	RValue<UInt2> operator/(RValue<UInt2> lhs, RValue<UInt2> rhs)
//	{
//		return RValue<UInt2>(Nucleus::createUDiv(lhs.value(), rhs.value()));
//	}

//	RValue<UInt2> operator%(RValue<UInt2> lhs, RValue<UInt2> rhs)
//	{
//		return RValue<UInt2>(Nucleus::createURem(lhs.value(), rhs.value()));
//	}

RValue<UInt2> operator&(RValue<UInt2> lhs, RValue<UInt2> rhs)
{
	return RValue<UInt2>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<UInt2> operator|(RValue<UInt2> lhs, RValue<UInt2> rhs)
{
	return RValue<UInt2>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<UInt2> operator^(RValue<UInt2> lhs, RValue<UInt2> rhs)
{
	return RValue<UInt2>(Nucleus::createXor(lhs.value(), rhs.value()));
}

RValue<UInt2> operator+=(UInt2 &lhs, RValue<UInt2> rhs)
{
	return lhs = lhs + rhs;
}

RValue<UInt2> operator-=(UInt2 &lhs, RValue<UInt2> rhs)
{
	return lhs = lhs - rhs;
}

//	RValue<UInt2> operator*=(UInt2 &lhs, RValue<UInt2> rhs)
//	{
//		return lhs = lhs * rhs;
//	}

//	RValue<UInt2> operator/=(UInt2 &lhs, RValue<UInt2> rhs)
//	{
//		return lhs = lhs / rhs;
//	}

//	RValue<UInt2> operator%=(UInt2 &lhs, RValue<UInt2> rhs)
//	{
//		return lhs = lhs % rhs;
//	}

RValue<UInt2> operator&=(UInt2 &lhs, RValue<UInt2> rhs)
{
	return lhs = lhs & rhs;
}

RValue<UInt2> operator|=(UInt2 &lhs, RValue<UInt2> rhs)
{
	return lhs = lhs | rhs;
}

RValue<UInt2> operator^=(UInt2 &lhs, RValue<UInt2> rhs)
{
	return lhs = lhs ^ rhs;
}

RValue<UInt2> operator<<=(UInt2 &lhs, unsigned char rhs)
{
	return lhs = lhs << rhs;
}

RValue<UInt2> operator>>=(UInt2 &lhs, unsigned char rhs)
{
	return lhs = lhs >> rhs;
}

//	RValue<UInt2> operator+(RValue<UInt2> val)
//	{
//		return val;
//	}

//	RValue<UInt2> operator-(RValue<UInt2> val)
//	{
//		return RValue<UInt2>(Nucleus::createNeg(val.value()));
//	}

RValue<UInt2> operator~(RValue<UInt2> val)
{
	return RValue<UInt2>(Nucleus::createNot(val.value()));
}

RValue<UInt> Extract(RValue<UInt2> val, int i)
{
	return RValue<UInt>(Nucleus::createExtractElement(val.value(), UInt::type(), i));
}

RValue<UInt2> Insert(RValue<UInt2> val, RValue<UInt> element, int i)
{
	return RValue<UInt2>(Nucleus::createInsertElement(val.value(), element.value(), i));
}

Int4::Int4()
    : XYZW(this)
{
}

Int4::Int4(RValue<Float4> cast)
    : XYZW(this)
{
	Value *xyzw = Nucleus::createFPToSI(cast.value(), Int4::type());

	storeValue(xyzw);
}

Int4::Int4(int xyzw)
    : XYZW(this)
{
	constant(xyzw, xyzw, xyzw, xyzw);
}

Int4::Int4(int x, int yzw)
    : XYZW(this)
{
	constant(x, yzw, yzw, yzw);
}

Int4::Int4(int x, int y, int zw)
    : XYZW(this)
{
	constant(x, y, zw, zw);
}

Int4::Int4(int x, int y, int z, int w)
    : XYZW(this)
{
	constant(x, y, z, w);
}

void Int4::constant(int x, int y, int z, int w)
{
	std::vector<int64_t> constantVector = { x, y, z, w };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

Int4::Int4(RValue<Int4> rhs)
    : XYZW(this)
{
	store(rhs);
}

Int4::Int4(const Int4 &rhs)
    : XYZW(this)
{
	store(rhs.load());
}

Int4::Int4(const Reference<Int4> &rhs)
    : XYZW(this)
{
	store(rhs.load());
}

Int4::Int4(RValue<UInt4> rhs)
    : XYZW(this)
{
	storeValue(rhs.value());
}

Int4::Int4(const UInt4 &rhs)
    : XYZW(this)
{
	storeValue(rhs.loadValue());
}

Int4::Int4(const Reference<UInt4> &rhs)
    : XYZW(this)
{
	storeValue(rhs.loadValue());
}

Int4::Int4(RValue<Int2> lo, RValue<Int2> hi)
    : XYZW(this)
{
	std::vector<int> shuffle = { 0, 1, 4, 5 };  // Real type is v4i32
	Value *packed = Nucleus::createShuffleVector(lo.value(), hi.value(), shuffle);

	storeValue(packed);
}

Int4::Int4(const Int &rhs)
    : XYZW(this)
{
	*this = RValue<Int>(rhs.loadValue());
}

Int4::Int4(const Reference<Int> &rhs)
    : XYZW(this)
{
	*this = RValue<Int>(rhs.loadValue());
}

RValue<Int4> Int4::operator=(int x)
{
	return *this = Int4(x, x, x, x);
}

RValue<Int4> Int4::operator=(RValue<Int4> rhs)
{
	return store(rhs);
}

RValue<Int4> Int4::operator=(const Int4 &rhs)
{
	return store(rhs.load());
}

RValue<Int4> Int4::operator=(const Reference<Int4> &rhs)
{
	return store(rhs.load());
}

RValue<Int4> operator+(RValue<Int4> lhs, RValue<Int4> rhs)
{
	return RValue<Int4>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<Int4> operator-(RValue<Int4> lhs, RValue<Int4> rhs)
{
	return RValue<Int4>(Nucleus::createSub(lhs.value(), rhs.value()));
}

RValue<Int4> operator*(RValue<Int4> lhs, RValue<Int4> rhs)
{
	return RValue<Int4>(Nucleus::createMul(lhs.value(), rhs.value()));
}

RValue<Int4> operator/(RValue<Int4> lhs, RValue<Int4> rhs)
{
	return RValue<Int4>(Nucleus::createSDiv(lhs.value(), rhs.value()));
}

RValue<Int4> operator%(RValue<Int4> lhs, RValue<Int4> rhs)
{
	return RValue<Int4>(Nucleus::createSRem(lhs.value(), rhs.value()));
}

RValue<Int4> operator&(RValue<Int4> lhs, RValue<Int4> rhs)
{
	return RValue<Int4>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<Int4> operator|(RValue<Int4> lhs, RValue<Int4> rhs)
{
	return RValue<Int4>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<Int4> operator^(RValue<Int4> lhs, RValue<Int4> rhs)
{
	return RValue<Int4>(Nucleus::createXor(lhs.value(), rhs.value()));
}

RValue<Int4> operator<<(RValue<Int4> lhs, RValue<Int4> rhs)
{
	return RValue<Int4>(Nucleus::createShl(lhs.value(), rhs.value()));
}

RValue<Int4> operator>>(RValue<Int4> lhs, RValue<Int4> rhs)
{
	return RValue<Int4>(Nucleus::createAShr(lhs.value(), rhs.value()));
}

RValue<Int4> operator+=(Int4 &lhs, RValue<Int4> rhs)
{
	return lhs = lhs + rhs;
}

RValue<Int4> operator-=(Int4 &lhs, RValue<Int4> rhs)
{
	return lhs = lhs - rhs;
}

RValue<Int4> operator*=(Int4 &lhs, RValue<Int4> rhs)
{
	return lhs = lhs * rhs;
}

//	RValue<Int4> operator/=(Int4 &lhs, RValue<Int4> rhs)
//	{
//		return lhs = lhs / rhs;
//	}

//	RValue<Int4> operator%=(Int4 &lhs, RValue<Int4> rhs)
//	{
//		return lhs = lhs % rhs;
//	}

RValue<Int4> operator&=(Int4 &lhs, RValue<Int4> rhs)
{
	return lhs = lhs & rhs;
}

RValue<Int4> operator|=(Int4 &lhs, RValue<Int4> rhs)
{
	return lhs = lhs | rhs;
}

RValue<Int4> operator^=(Int4 &lhs, RValue<Int4> rhs)
{
	return lhs = lhs ^ rhs;
}

RValue<Int4> operator<<=(Int4 &lhs, unsigned char rhs)
{
	return lhs = lhs << rhs;
}

RValue<Int4> operator>>=(Int4 &lhs, unsigned char rhs)
{
	return lhs = lhs >> rhs;
}

RValue<Int4> operator+(RValue<Int4> val)
{
	return val;
}

RValue<Int4> operator-(RValue<Int4> val)
{
	return RValue<Int4>(Nucleus::createNeg(val.value()));
}

RValue<Int4> operator~(RValue<Int4> val)
{
	return RValue<Int4>(Nucleus::createNot(val.value()));
}

RValue<Int> Extract(RValue<Int4> x, int i)
{
	return RValue<Int>(Nucleus::createExtractElement(x.value(), Int::type(), i));
}

RValue<Int4> Insert(RValue<Int4> x, RValue<Int> element, int i)
{
	return RValue<Int4>(Nucleus::createInsertElement(x.value(), element.value(), i));
}

RValue<Int4> Swizzle(RValue<Int4> x, uint16_t select)
{
	return RValue<Int4>(createSwizzle4(x.value(), select));
}

RValue<Int4> Shuffle(RValue<Int4> x, RValue<Int4> y, unsigned short select)
{
	return RValue<Int4>(createShuffle4(x.value(), y.value(), select));
}

UInt4::UInt4()
    : XYZW(this)
{
}

UInt4::UInt4(int xyzw)
    : XYZW(this)
{
	constant(xyzw, xyzw, xyzw, xyzw);
}

UInt4::UInt4(int x, int yzw)
    : XYZW(this)
{
	constant(x, yzw, yzw, yzw);
}

UInt4::UInt4(int x, int y, int zw)
    : XYZW(this)
{
	constant(x, y, zw, zw);
}

UInt4::UInt4(int x, int y, int z, int w)
    : XYZW(this)
{
	constant(x, y, z, w);
}

void UInt4::constant(int x, int y, int z, int w)
{
	std::vector<int64_t> constantVector = { x, y, z, w };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

UInt4::UInt4(RValue<UInt4> rhs)
    : XYZW(this)
{
	store(rhs);
}

UInt4::UInt4(const UInt4 &rhs)
    : XYZW(this)
{
	store(rhs.load());
}

UInt4::UInt4(const Reference<UInt4> &rhs)
    : XYZW(this)
{
	store(rhs.load());
}

UInt4::UInt4(RValue<Int4> rhs)
    : XYZW(this)
{
	storeValue(rhs.value());
}

UInt4::UInt4(const Int4 &rhs)
    : XYZW(this)
{
	storeValue(rhs.loadValue());
}

UInt4::UInt4(const Reference<Int4> &rhs)
    : XYZW(this)
{
	storeValue(rhs.loadValue());
}

UInt4::UInt4(RValue<UInt2> lo, RValue<UInt2> hi)
    : XYZW(this)
{
	std::vector<int> shuffle = { 0, 1, 4, 5 };  // Real type is v4i32
	Value *packed = Nucleus::createShuffleVector(lo.value(), hi.value(), shuffle);

	storeValue(packed);
}

UInt4::UInt4(const UInt &rhs)
    : XYZW(this)
{
	*this = RValue<UInt>(rhs.loadValue());
}

UInt4::UInt4(const Reference<UInt> &rhs)
    : XYZW(this)
{
	*this = RValue<UInt>(rhs.loadValue());
}

RValue<UInt4> UInt4::operator=(RValue<UInt4> rhs)
{
	return store(rhs);
}

RValue<UInt4> UInt4::operator=(const UInt4 &rhs)
{
	return store(rhs.load());
}

RValue<UInt4> UInt4::operator=(const Reference<UInt4> &rhs)
{
	return store(rhs.load());
}

RValue<UInt4> operator+(RValue<UInt4> lhs, RValue<UInt4> rhs)
{
	return RValue<UInt4>(Nucleus::createAdd(lhs.value(), rhs.value()));
}

RValue<UInt4> operator-(RValue<UInt4> lhs, RValue<UInt4> rhs)
{
	return RValue<UInt4>(Nucleus::createSub(lhs.value(), rhs.value()));
}

RValue<UInt4> operator*(RValue<UInt4> lhs, RValue<UInt4> rhs)
{
	return RValue<UInt4>(Nucleus::createMul(lhs.value(), rhs.value()));
}

RValue<UInt4> operator/(RValue<UInt4> lhs, RValue<UInt4> rhs)
{
	return RValue<UInt4>(Nucleus::createUDiv(lhs.value(), rhs.value()));
}

RValue<UInt4> operator%(RValue<UInt4> lhs, RValue<UInt4> rhs)
{
	return RValue<UInt4>(Nucleus::createURem(lhs.value(), rhs.value()));
}

RValue<UInt4> operator&(RValue<UInt4> lhs, RValue<UInt4> rhs)
{
	return RValue<UInt4>(Nucleus::createAnd(lhs.value(), rhs.value()));
}

RValue<UInt4> operator|(RValue<UInt4> lhs, RValue<UInt4> rhs)
{
	return RValue<UInt4>(Nucleus::createOr(lhs.value(), rhs.value()));
}

RValue<UInt4> operator^(RValue<UInt4> lhs, RValue<UInt4> rhs)
{
	return RValue<UInt4>(Nucleus::createXor(lhs.value(), rhs.value()));
}

RValue<UInt4> operator<<(RValue<UInt4> lhs, RValue<UInt4> rhs)
{
	return RValue<UInt4>(Nucleus::createShl(lhs.value(), rhs.value()));
}

RValue<UInt4> operator>>(RValue<UInt4> lhs, RValue<UInt4> rhs)
{
	return RValue<UInt4>(Nucleus::createLShr(lhs.value(), rhs.value()));
}

RValue<UInt4> operator+=(UInt4 &lhs, RValue<UInt4> rhs)
{
	return lhs = lhs + rhs;
}

RValue<UInt4> operator-=(UInt4 &lhs, RValue<UInt4> rhs)
{
	return lhs = lhs - rhs;
}

RValue<UInt4> operator*=(UInt4 &lhs, RValue<UInt4> rhs)
{
	return lhs = lhs * rhs;
}

//	RValue<UInt4> operator/=(UInt4 &lhs, RValue<UInt4> rhs)
//	{
//		return lhs = lhs / rhs;
//	}

//	RValue<UInt4> operator%=(UInt4 &lhs, RValue<UInt4> rhs)
//	{
//		return lhs = lhs % rhs;
//	}

RValue<UInt4> operator&=(UInt4 &lhs, RValue<UInt4> rhs)
{
	return lhs = lhs & rhs;
}

RValue<UInt4> operator|=(UInt4 &lhs, RValue<UInt4> rhs)
{
	return lhs = lhs | rhs;
}

RValue<UInt4> operator^=(UInt4 &lhs, RValue<UInt4> rhs)
{
	return lhs = lhs ^ rhs;
}

RValue<UInt4> operator<<=(UInt4 &lhs, unsigned char rhs)
{
	return lhs = lhs << rhs;
}

RValue<UInt4> operator>>=(UInt4 &lhs, unsigned char rhs)
{
	return lhs = lhs >> rhs;
}

RValue<UInt4> operator+(RValue<UInt4> val)
{
	return val;
}

RValue<UInt4> operator-(RValue<UInt4> val)
{
	return RValue<UInt4>(Nucleus::createNeg(val.value()));
}

RValue<UInt4> operator~(RValue<UInt4> val)
{
	return RValue<UInt4>(Nucleus::createNot(val.value()));
}

RValue<UInt> Extract(RValue<UInt4> x, int i)
{
	return RValue<UInt>(Nucleus::createExtractElement(x.value(), Int::type(), i));
}

RValue<UInt4> Insert(RValue<UInt4> x, RValue<UInt> element, int i)
{
	return RValue<UInt4>(Nucleus::createInsertElement(x.value(), element.value(), i));
}

RValue<UInt4> Swizzle(RValue<UInt4> x, uint16_t select)
{
	return RValue<UInt4>(createSwizzle4(x.value(), select));
}

RValue<UInt4> Shuffle(RValue<UInt4> x, RValue<UInt4> y, unsigned short select)
{
	return RValue<UInt4>(createShuffle4(x.value(), y.value(), select));
}

Half::Half(RValue<Float> cast)
{
	UInt fp32i = As<UInt>(cast);
	UInt abs = fp32i & 0x7FFFFFFF;
	UShort fp16i((fp32i & 0x80000000) >> 16);  // sign

	If(abs > 0x47FFEFFF)  // Infinity
	{
		fp16i |= UShort(0x7FFF);
	}
	Else
	{
		If(abs < 0x38800000)  // Denormal
		{
			Int mantissa = (abs & 0x007FFFFF) | 0x00800000;
			Int e = 113 - (abs >> 23);
			abs = IfThenElse(e < 24, (mantissa >> e), Int(0));
			fp16i |= UShort((abs + 0x00000FFF + ((abs >> 13) & 1)) >> 13);
		}
		Else
		{
			fp16i |= UShort((abs + 0xC8000000 + 0x00000FFF + ((abs >> 13) & 1)) >> 13);
		}
	}

	storeValue(fp16i.loadValue());
}

Float::Float(RValue<Int> cast)
{
	Value *integer = Nucleus::createSIToFP(cast.value(), Float::type());

	storeValue(integer);
}

Float::Float(RValue<UInt> cast)
{
	RValue<Float> result = Float(Int(cast & UInt(0x7FFFFFFF))) +
	                       As<Float>((As<Int>(cast) >> 31) & As<Int>(Float(0x80000000u)));

	storeValue(result.value());
}

Float::Float(RValue<Half> cast)
{
	Int fp16i(As<UShort>(cast));

	Int s = (fp16i >> 15) & 0x00000001;
	Int e = (fp16i >> 10) & 0x0000001F;
	Int m = fp16i & 0x000003FF;

	UInt fp32i(s << 31);
	If(e == 0)
	{
		If(m != 0)
		{
			While((m & 0x00000400) == 0)
			{
				m <<= 1;
				e -= 1;
			}

			fp32i |= As<UInt>(((e + (127 - 15) + 1) << 23) | ((m & ~0x00000400) << 13));
		}
	}
	Else
	{
		fp32i |= As<UInt>(((e + (127 - 15)) << 23) | (m << 13));
	}

	storeValue(As<Float>(fp32i).value());
}

Float::Float(float x)
{
	// C++ does not have a way to write an infinite or NaN literal,
	// nor does it allow division by zero as a constant expression.
	// Thus we should not accept inf or NaN as a Reactor Float constant,
	// as this would typically idicate a bug, and avoids undefined
	// behavior.
	//
	// This also prevents the issue of the LLVM JIT only taking double
	// values for constructing floating-point constants. During the
	// conversion from single-precision to double, a signaling NaN can
	// become a quiet NaN, thus altering its bit pattern. Hence this
	// assert is also helpful for detecting cases where integers are
	// being reinterpreted as float and then bitcast to integer again,
	// which does not guarantee preserving the integer value.
	//
	// The inifinity() method can be used to obtain positive infinity.
	// Should NaN constants be required, methods like quiet_NaN() and
	// signaling_NaN() should be added (matching std::numeric_limits).
	ASSERT(std::isfinite(x));

	storeValue(Nucleus::createConstantFloat(x));
}

// TODO(b/140302841): Negative infinity can be obtained by using '-infinity()'.
// This comes at a minor run-time JIT cost, and the backend may or may not
// perform constant folding. This can be optimized by having Reactor perform
// the folding, which would still be cheaper than having a capable backend do it.
Float Float::infinity()
{
	Float result;

	constexpr double inf = std::numeric_limits<double>::infinity();
	result.storeValue(Nucleus::createConstantFloat(inf));

	return result;
}

Float::Float(RValue<Float> rhs)
{
	store(rhs);
}

Float::Float(const Float &rhs)
{
	store(rhs.load());
}

Float::Float(const Reference<Float> &rhs)
{
	store(rhs.load());
}

Float::Float(Argument<Float> argument)
{
	store(argument.rvalue());
}

RValue<Float> Float::operator=(float rhs)
{
	return RValue<Float>(storeValue(Nucleus::createConstantFloat(rhs)));
}

RValue<Float> Float::operator=(RValue<Float> rhs)
{
	return store(rhs);
}

RValue<Float> Float::operator=(const Float &rhs)
{
	return store(rhs.load());
}

RValue<Float> Float::operator=(const Reference<Float> &rhs)
{
	return store(rhs.load());
}

RValue<Float> operator+(RValue<Float> lhs, RValue<Float> rhs)
{
	return RValue<Float>(Nucleus::createFAdd(lhs.value(), rhs.value()));
}

RValue<Float> operator-(RValue<Float> lhs, RValue<Float> rhs)
{
	return RValue<Float>(Nucleus::createFSub(lhs.value(), rhs.value()));
}

RValue<Float> operator*(RValue<Float> lhs, RValue<Float> rhs)
{
	return RValue<Float>(Nucleus::createFMul(lhs.value(), rhs.value()));
}

RValue<Float> operator/(RValue<Float> lhs, RValue<Float> rhs)
{
	return RValue<Float>(Nucleus::createFDiv(lhs.value(), rhs.value()));
}

RValue<Float> operator+=(Float &lhs, RValue<Float> rhs)
{
	return lhs = lhs + rhs;
}

RValue<Float> operator-=(Float &lhs, RValue<Float> rhs)
{
	return lhs = lhs - rhs;
}

RValue<Float> operator*=(Float &lhs, RValue<Float> rhs)
{
	return lhs = lhs * rhs;
}

RValue<Float> operator/=(Float &lhs, RValue<Float> rhs)
{
	return lhs = lhs / rhs;
}

RValue<Float> operator+(RValue<Float> val)
{
	return val;
}

RValue<Float> operator-(RValue<Float> val)
{
	return RValue<Float>(Nucleus::createFNeg(val.value()));
}

RValue<Bool> operator<(RValue<Float> lhs, RValue<Float> rhs)
{
	return RValue<Bool>(Nucleus::createFCmpOLT(lhs.value(), rhs.value()));
}

RValue<Bool> operator<=(RValue<Float> lhs, RValue<Float> rhs)
{
	return RValue<Bool>(Nucleus::createFCmpOLE(lhs.value(), rhs.value()));
}

RValue<Bool> operator>(RValue<Float> lhs, RValue<Float> rhs)
{
	return RValue<Bool>(Nucleus::createFCmpOGT(lhs.value(), rhs.value()));
}

RValue<Bool> operator>=(RValue<Float> lhs, RValue<Float> rhs)
{
	return RValue<Bool>(Nucleus::createFCmpOGE(lhs.value(), rhs.value()));
}

RValue<Bool> operator!=(RValue<Float> lhs, RValue<Float> rhs)
{
	return RValue<Bool>(Nucleus::createFCmpONE(lhs.value(), rhs.value()));
}

RValue<Bool> operator==(RValue<Float> lhs, RValue<Float> rhs)
{
	return RValue<Bool>(Nucleus::createFCmpOEQ(lhs.value(), rhs.value()));
}

RValue<Float> Abs(RValue<Float> x)
{
	return IfThenElse(x > 0.0f, x, -x);
}

RValue<Float> Max(RValue<Float> x, RValue<Float> y)
{
	return IfThenElse(x > y, x, y);
}

RValue<Float> Min(RValue<Float> x, RValue<Float> y)
{
	return IfThenElse(x < y, x, y);
}

Float2::Float2(RValue<Float4> cast)
{
	storeValue(Nucleus::createBitCast(cast.value(), type()));
}

Float4::Float4(RValue<Byte4> cast)
    : XYZW(this)
{
	Value *a = Int4(cast).loadValue();
	Value *xyzw = Nucleus::createSIToFP(a, Float4::type());

	storeValue(xyzw);
}

Float4::Float4(RValue<SByte4> cast)
    : XYZW(this)
{
	Value *a = Int4(cast).loadValue();
	Value *xyzw = Nucleus::createSIToFP(a, Float4::type());

	storeValue(xyzw);
}

Float4::Float4(RValue<Short4> cast)
    : XYZW(this)
{
	Int4 c(cast);
	storeValue(Nucleus::createSIToFP(RValue<Int4>(c).value(), Float4::type()));
}

Float4::Float4(RValue<UShort4> cast)
    : XYZW(this)
{
	Int4 c(cast);
	storeValue(Nucleus::createSIToFP(RValue<Int4>(c).value(), Float4::type()));
}

Float4::Float4(RValue<Int4> cast)
    : XYZW(this)
{
	Value *xyzw = Nucleus::createSIToFP(cast.value(), Float4::type());

	storeValue(xyzw);
}

Float4::Float4(RValue<UInt4> cast)
    : XYZW(this)
{
	RValue<Float4> result = Float4(Int4(cast & UInt4(0x7FFFFFFF))) +
	                        As<Float4>((As<Int4>(cast) >> 31) & As<Int4>(Float4(0x80000000u)));

	storeValue(result.value());
}

Float4::Float4()
    : XYZW(this)
{
}

Float4::Float4(float xyzw)
    : XYZW(this)
{
	constant(xyzw, xyzw, xyzw, xyzw);
}

Float4::Float4(float x, float yzw)
    : XYZW(this)
{
	constant(x, yzw, yzw, yzw);
}

Float4::Float4(float x, float y, float zw)
    : XYZW(this)
{
	constant(x, y, zw, zw);
}

Float4::Float4(float x, float y, float z, float w)
    : XYZW(this)
{
	constant(x, y, z, w);
}

Float4 Float4::infinity()
{
	Float4 result;

	constexpr double inf = std::numeric_limits<double>::infinity();
	std::vector<double> constantVector = { inf };
	result.storeValue(Nucleus::createConstantVector(constantVector, type()));

	return result;
}

void Float4::constant(float x, float y, float z, float w)
{
	// See Float(float) constructor for the rationale behind this assert.
	ASSERT(std::isfinite(x) && std::isfinite(y) && std::isfinite(z) && std::isfinite(w));

	std::vector<double> constantVector = { x, y, z, w };
	storeValue(Nucleus::createConstantVector(constantVector, type()));
}

Float4::Float4(RValue<Float4> rhs)
    : XYZW(this)
{
	store(rhs);
}

Float4::Float4(const Float4 &rhs)
    : XYZW(this)
{
	store(rhs.load());
}

Float4::Float4(const Reference<Float4> &rhs)
    : XYZW(this)
{
	store(rhs.load());
}

Float4::Float4(const Float &rhs)
    : XYZW(this)
{
	*this = RValue<Float>(rhs.loadValue());
}

Float4::Float4(const Reference<Float> &rhs)
    : XYZW(this)
{
	*this = RValue<Float>(rhs.loadValue());
}

Float4::Float4(RValue<Float2> lo, RValue<Float2> hi)
    : XYZW(this)
{
	std::vector<int> shuffle = { 0, 1, 4, 5 };  // Real type is v4i32
	Value *packed = Nucleus::createShuffleVector(lo.value(), hi.value(), shuffle);

	storeValue(packed);
}

RValue<Float4> Float4::operator=(float x)
{
	return *this = Float4(x, x, x, x);
}

RValue<Float4> Float4::operator=(RValue<Float4> rhs)
{
	return store(rhs);
}

RValue<Float4> Float4::operator=(const Float4 &rhs)
{
	return store(rhs.load());
}

RValue<Float4> Float4::operator=(const Reference<Float4> &rhs)
{
	return store(rhs.load());
}

RValue<Float4> Float4::operator=(RValue<Float> rhs)
{
	return *this = Float4(rhs);
}

RValue<Float4> Float4::operator=(const Float &rhs)
{
	return *this = Float4(rhs);
}

RValue<Float4> Float4::operator=(const Reference<Float> &rhs)
{
	return *this = Float4(rhs);
}

RValue<Float4> operator+(RValue<Float4> lhs, RValue<Float4> rhs)
{
	return RValue<Float4>(Nucleus::createFAdd(lhs.value(), rhs.value()));
}

RValue<Float4> operator-(RValue<Float4> lhs, RValue<Float4> rhs)
{
	return RValue<Float4>(Nucleus::createFSub(lhs.value(), rhs.value()));
}

RValue<Float4> operator*(RValue<Float4> lhs, RValue<Float4> rhs)
{
	return RValue<Float4>(Nucleus::createFMul(lhs.value(), rhs.value()));
}

RValue<Float4> operator/(RValue<Float4> lhs, RValue<Float4> rhs)
{
	return RValue<Float4>(Nucleus::createFDiv(lhs.value(), rhs.value()));
}

RValue<Float4> operator+=(Float4 &lhs, RValue<Float4> rhs)
{
	return lhs = lhs + rhs;
}

RValue<Float4> operator-=(Float4 &lhs, RValue<Float4> rhs)
{
	return lhs = lhs - rhs;
}

RValue<Float4> operator*=(Float4 &lhs, RValue<Float4> rhs)
{
	return lhs = lhs * rhs;
}

RValue<Float4> operator/=(Float4 &lhs, RValue<Float4> rhs)
{
	return lhs = lhs / rhs;
}

RValue<Float4> operator%=(Float4 &lhs, RValue<Float4> rhs)
{
	return lhs = lhs % rhs;
}

RValue<Float4> operator+(RValue<Float4> val)
{
	return val;
}

RValue<Float4> operator-(RValue<Float4> val)
{
	return RValue<Float4>(Nucleus::createFNeg(val.value()));
}

RValue<Float4> Insert(RValue<Float4> x, RValue<Float> element, int i)
{
	return RValue<Float4>(Nucleus::createInsertElement(x.value(), element.value(), i));
}

RValue<Float> Extract(RValue<Float4> x, int i)
{
	return RValue<Float>(Nucleus::createExtractElement(x.value(), Float::type(), i));
}

RValue<Float4> Swizzle(RValue<Float4> x, uint16_t select)
{
	return RValue<Float4>(createSwizzle4(x.value(), select));
}

RValue<Float4> Shuffle(RValue<Float4> x, RValue<Float4> y, uint16_t select)
{
	return RValue<Float4>(createShuffle4(x.value(), y.value(), select));
}

RValue<Float4> ShuffleLowHigh(RValue<Float4> x, RValue<Float4> y, uint16_t imm)
{
	std::vector<int> shuffle = {
		((imm >> 12) & 0x03) + 0,
		((imm >> 8) & 0x03) + 0,
		((imm >> 4) & 0x03) + 4,
		((imm >> 0) & 0x03) + 4,
	};

	return RValue<Float4>(Nucleus::createShuffleVector(x.value(), y.value(), shuffle));
}

RValue<Float4> UnpackLow(RValue<Float4> x, RValue<Float4> y)
{
	std::vector<int> shuffle = { 0, 4, 1, 5 };
	return RValue<Float4>(Nucleus::createShuffleVector(x.value(), y.value(), shuffle));
}

RValue<Float4> UnpackHigh(RValue<Float4> x, RValue<Float4> y)
{
	std::vector<int> shuffle = { 2, 6, 3, 7 };
	return RValue<Float4>(Nucleus::createShuffleVector(x.value(), y.value(), shuffle));
}

RValue<Float4> Mask(Float4 &lhs, RValue<Float4> rhs, uint16_t select)
{
	Value *vector = lhs.loadValue();
	Value *result = createMask4(vector, rhs.value(), select);
	lhs.storeValue(result);

	return RValue<Float4>(result);
}

RValue<Int4> IsInf(RValue<Float4> x)
{
	return CmpEQ(As<Int4>(x) & Int4(0x7FFFFFFF), Int4(0x7F800000));
}

RValue<Int4> IsNan(RValue<Float4> x)
{
	return ~CmpEQ(x, x);
}

RValue<Float> Exp2(RValue<Float> x)
{
	return Call(exp2f, x);
}

RValue<Float> Log2(RValue<Float> x)
{
	return Call(log2f, x);
}

RValue<Float4> Sin(RValue<Float4> x)
{
	return ScalarizeCall(sinf, x);
}

RValue<Float4> Cos(RValue<Float4> x)
{
	return ScalarizeCall(cosf, x);
}

RValue<Float4> Tan(RValue<Float4> x)
{
	return ScalarizeCall(tanf, x);
}

RValue<Float4> Asin(RValue<Float4> x)
{
	return ScalarizeCall(asinf, x);
}

RValue<Float4> Acos(RValue<Float4> x)
{
	return ScalarizeCall(acosf, x);
}

RValue<Float4> Atan(RValue<Float4> x)
{
	return ScalarizeCall(atanf, x);
}

RValue<Float4> Sinh(RValue<Float4> x)
{
	return ScalarizeCall(sinhf, x);
}

RValue<Float4> Cosh(RValue<Float4> x)
{
	return ScalarizeCall(coshf, x);
}

RValue<Float4> Tanh(RValue<Float4> x)
{
	return ScalarizeCall(tanhf, x);
}

RValue<Float4> Asinh(RValue<Float4> x)
{
	return ScalarizeCall(asinhf, x);
}

RValue<Float4> Acosh(RValue<Float4> x)
{
	return ScalarizeCall(acoshf, x);
}

RValue<Float4> Atanh(RValue<Float4> x)
{
	return ScalarizeCall(atanhf, x);
}

RValue<Float4> Atan2(RValue<Float4> x, RValue<Float4> y)
{
	return ScalarizeCall(atan2f, x, y);
}

RValue<Float4> Pow(RValue<Float4> x, RValue<Float4> y)
{
	return ScalarizeCall(powf, x, y);
}

RValue<Float4> Exp(RValue<Float4> x)
{
	return ScalarizeCall(expf, x);
}

RValue<Float4> Log(RValue<Float4> x)
{
	return ScalarizeCall(logf, x);
}

RValue<Float4> Exp2(RValue<Float4> x)
{
	return ScalarizeCall(exp2f, x);
}

RValue<Float4> Log2(RValue<Float4> x)
{
	return ScalarizeCall(log2f, x);
}

RValue<Pointer<Byte>> operator+(RValue<Pointer<Byte>> lhs, int offset)
{
	return lhs + RValue<Int>(Nucleus::createConstantInt(offset));
}

RValue<Pointer<Byte>> operator+(RValue<Pointer<Byte>> lhs, RValue<Int> offset)
{
	return RValue<Pointer<Byte>>(Nucleus::createGEP(lhs.value(), Byte::type(), offset.value(), false));
}

RValue<Pointer<Byte>> operator+(RValue<Pointer<Byte>> lhs, RValue<UInt> offset)
{
	return RValue<Pointer<Byte>>(Nucleus::createGEP(lhs.value(), Byte::type(), offset.value(), true));
}

RValue<Pointer<Byte>> operator+=(Pointer<Byte> &lhs, int offset)
{
	return lhs = lhs + offset;
}

RValue<Pointer<Byte>> operator+=(Pointer<Byte> &lhs, RValue<Int> offset)
{
	return lhs = lhs + offset;
}

RValue<Pointer<Byte>> operator+=(Pointer<Byte> &lhs, RValue<UInt> offset)
{
	return lhs = lhs + offset;
}

RValue<Pointer<Byte>> operator-(RValue<Pointer<Byte>> lhs, int offset)
{
	return lhs + -offset;
}

RValue<Pointer<Byte>> operator-(RValue<Pointer<Byte>> lhs, RValue<Int> offset)
{
	return lhs + -offset;
}

RValue<Pointer<Byte>> operator-(RValue<Pointer<Byte>> lhs, RValue<UInt> offset)
{
	return lhs + -offset;
}

RValue<Pointer<Byte>> operator-=(Pointer<Byte> &lhs, int offset)
{
	return lhs = lhs - offset;
}

RValue<Pointer<Byte>> operator-=(Pointer<Byte> &lhs, RValue<Int> offset)
{
	return lhs = lhs - offset;
}

RValue<Pointer<Byte>> operator-=(Pointer<Byte> &lhs, RValue<UInt> offset)
{
	return lhs = lhs - offset;
}

RValue<Bool> AnyTrue(const RValue<Int4> &bools)
{
	return SignMask(bools) != 0;
}

RValue<Bool> AnyFalse(const RValue<Int4> &bools)
{
	return SignMask(~bools) != 0;  // TODO(b/214588983): Compare against mask of 4 1's to avoid bitwise NOT.
}

RValue<Bool> AllTrue(const RValue<Int4> &bools)
{
	return SignMask(~bools) == 0;  // TODO(b/214588983): Compare against mask of 4 1's to avoid bitwise NOT.
}

RValue<Bool> AllFalse(const RValue<Int4> &bools)
{
	return SignMask(bools) == 0;
}

RValue<Bool> Divergent(const RValue<Int4> &ints)
{
	auto broadcastFirst = Int4(Extract(ints, 0));
	return AnyTrue(CmpNEQ(broadcastFirst, ints));
}

RValue<Bool> Divergent(const RValue<Float4> &floats)
{
	auto broadcastFirst = Float4(Extract(floats, 0));
	return AnyTrue(CmpNEQ(broadcastFirst, floats));
}

RValue<Bool> Uniform(const RValue<Int4> &ints)
{
	auto broadcastFirst = Int4(Extract(ints, 0));
	return AllFalse(CmpNEQ(broadcastFirst, ints));
}

RValue<Bool> Uniform(const RValue<Float4> &floats)
{
	auto broadcastFirst = Float4(Extract(floats, 0));
	return AllFalse(CmpNEQ(broadcastFirst, floats));
}

void Return()
{
	Nucleus::createRetVoid();
	// Place any unreachable instructions in an unreferenced block.
	Nucleus::setInsertBlock(Nucleus::createBasicBlock());
}

void branch(RValue<Bool> cmp, BasicBlock *bodyBB, BasicBlock *endBB)
{
	Nucleus::createCondBr(cmp.value(), bodyBB, endBB);
	Nucleus::setInsertBlock(bodyBB);
}

RValue<Float4> MaskedLoad(RValue<Pointer<Float4>> base, RValue<Int4> mask, unsigned int alignment, bool zeroMaskedLanes /* = false */)
{
	return RValue<Float4>(Nucleus::createMaskedLoad(base.value(), Float::type(), mask.value(), alignment, zeroMaskedLanes));
}

RValue<Int4> MaskedLoad(RValue<Pointer<Int4>> base, RValue<Int4> mask, unsigned int alignment, bool zeroMaskedLanes /* = false */)
{
	return RValue<Int4>(Nucleus::createMaskedLoad(base.value(), Int::type(), mask.value(), alignment, zeroMaskedLanes));
}

void MaskedStore(RValue<Pointer<Float4>> base, RValue<Float4> val, RValue<Int4> mask, unsigned int alignment)
{
	Nucleus::createMaskedStore(base.value(), val.value(), mask.value(), alignment);
}

void MaskedStore(RValue<Pointer<Int4>> base, RValue<Int4> val, RValue<Int4> mask, unsigned int alignment)
{
	Nucleus::createMaskedStore(base.value(), val.value(), mask.value(), alignment);
}

void Fence(std::memory_order memoryOrder)
{
	ASSERT_MSG(memoryOrder == std::memory_order_acquire ||
	               memoryOrder == std::memory_order_release ||
	               memoryOrder == std::memory_order_acq_rel ||
	               memoryOrder == std::memory_order_seq_cst,
	           "Unsupported memoryOrder: %d", int(memoryOrder));
	Nucleus::createFence(memoryOrder);
}

Bool CToReactor<bool>::cast(bool v)
{
	return type(v);
}
Byte CToReactor<uint8_t>::cast(uint8_t v)
{
	return type(v);
}
SByte CToReactor<int8_t>::cast(int8_t v)
{
	return type(v);
}
Short CToReactor<int16_t>::cast(int16_t v)
{
	return type(v);
}
UShort CToReactor<uint16_t>::cast(uint16_t v)
{
	return type(v);
}
Int CToReactor<int32_t>::cast(int32_t v)
{
	return type(v);
}
UInt CToReactor<uint32_t>::cast(uint32_t v)
{
	return type(v);
}
Float CToReactor<float>::cast(float v)
{
	return type(v);
}
Float4 CToReactor<float[4]>::cast(float v[4])
{
	return type(v[0], v[1], v[2], v[3]);
}

// TODO: Long has no constructor that takes a uint64_t
// Long          CToReactor<uint64_t>::cast(uint64_t v)       { return type(v); }

#ifdef ENABLE_RR_PRINT
static std::string replaceAll(std::string str, const std::string &substr, const std::string &replacement)
{
	size_t pos = 0;
	while((pos = str.find(substr, pos)) != std::string::npos)
	{
		str.replace(pos, substr.length(), replacement);
		pos += replacement.length();
	}
	return str;
}

// extractAll returns a vector containing the extracted n scalar values of the vector vec.
static std::vector<Value *> extractAll(Value *vec, int n)
{
	Type *elemTy = Nucleus::getContainedType(Nucleus::getType(vec));
	std::vector<Value *> elements;
	elements.reserve(n);
	for(int i = 0; i < n; i++)
	{
		auto el = Nucleus::createExtractElement(vec, elemTy, i);
		elements.push_back(el);
	}
	return elements;
}

// toInt returns all the integer values in vals extended to a printf-required storage value
static std::vector<Value *> toInt(const std::vector<Value *> &vals, bool isSigned)
{
	auto storageTy = Nucleus::getPrintfStorageType(Int::type());
	std::vector<Value *> elements;
	elements.reserve(vals.size());
	for(auto v : vals)
	{
		if(isSigned)
		{
			elements.push_back(Nucleus::createSExt(v, storageTy));
		}
		else
		{
			elements.push_back(Nucleus::createZExt(v, storageTy));
		}
	}
	return elements;
}

// toFloat returns all the float values in vals extended to extended to a printf-required storage value
static std::vector<Value *> toFloat(const std::vector<Value *> &vals)
{
	auto storageTy = Nucleus::getPrintfStorageType(Float::type());
	std::vector<Value *> elements;
	elements.reserve(vals.size());
	for(auto v : vals)
	{
		elements.push_back(Nucleus::createFPExt(v, storageTy));
	}
	return elements;
}

std::vector<Value *> PrintValue::Ty<Bool>::val(const RValue<Bool> &v)
{
	auto t = Nucleus::createConstantString("true");
	auto f = Nucleus::createConstantString("false");
	return { Nucleus::createSelect(v.value(), t, f) };
}

std::vector<Value *> PrintValue::Ty<Byte>::val(const RValue<Byte> &v)
{
	return toInt({ v.value() }, false);
}

std::vector<Value *> PrintValue::Ty<Byte4>::val(const RValue<Byte4> &v)
{
	return toInt(extractAll(v.value(), 4), false);
}

std::vector<Value *> PrintValue::Ty<Int>::val(const RValue<Int> &v)
{
	return toInt({ v.value() }, true);
}

std::vector<Value *> PrintValue::Ty<Int2>::val(const RValue<Int2> &v)
{
	return toInt(extractAll(v.value(), 2), true);
}

std::vector<Value *> PrintValue::Ty<Int4>::val(const RValue<Int4> &v)
{
	return toInt(extractAll(v.value(), 4), true);
}

std::vector<Value *> PrintValue::Ty<UInt>::val(const RValue<UInt> &v)
{
	return toInt({ v.value() }, false);
}

std::vector<Value *> PrintValue::Ty<UInt2>::val(const RValue<UInt2> &v)
{
	return toInt(extractAll(v.value(), 2), false);
}

std::vector<Value *> PrintValue::Ty<UInt4>::val(const RValue<UInt4> &v)
{
	return toInt(extractAll(v.value(), 4), false);
}

std::vector<Value *> PrintValue::Ty<Short>::val(const RValue<Short> &v)
{
	return toInt({ v.value() }, true);
}

std::vector<Value *> PrintValue::Ty<Short4>::val(const RValue<Short4> &v)
{
	return toInt(extractAll(v.value(), 4), true);
}

std::vector<Value *> PrintValue::Ty<UShort>::val(const RValue<UShort> &v)
{
	return toInt({ v.value() }, false);
}

std::vector<Value *> PrintValue::Ty<UShort4>::val(const RValue<UShort4> &v)
{
	return toInt(extractAll(v.value(), 4), false);
}

std::vector<Value *> PrintValue::Ty<Float>::val(const RValue<Float> &v)
{
	return toFloat({ v.value() });
}

std::vector<Value *> PrintValue::Ty<Float4>::val(const RValue<Float4> &v)
{
	return toFloat(extractAll(v.value(), 4));
}

std::vector<Value *> PrintValue::Ty<SIMD::Int>::val(const RValue<SIMD::Int> &v)
{
	return toInt(extractAll(v.value(), SIMD::Width), true);
}

std::vector<Value *> PrintValue::Ty<SIMD::UInt>::val(const RValue<SIMD::UInt> &v)
{
	return toInt(extractAll(v.value(), SIMD::Width), false);
}

std::vector<Value *> PrintValue::Ty<SIMD::Float>::val(const RValue<SIMD::Float> &v)
{
	return toFloat(extractAll(v.value(), SIMD::Width));
}

std::vector<Value *> PrintValue::Ty<const char *>::val(const char *v)
{
	return { Nucleus::createConstantString(v) };
}

void Printv(const char *function, const char *file, int line, const char *fmt, std::initializer_list<PrintValue> args)
{
	// Build the printf format message string.
	std::string str;
	if(file != nullptr) { str += (line > 0) ? "%s:%d " : "%s "; }
	if(function != nullptr) { str += "%s "; }
	str += fmt;

	// Perform substitution on all '{n}' bracketed indices in the format
	// message.
	int i = 0;
	for(const PrintValue &arg : args)
	{
		str = replaceAll(str, "{" + std::to_string(i++) + "}", arg.format);
	}

	std::vector<Value *> vals;
	vals.reserve(8);

	// The format message is always the first argument.
	vals.push_back(Nucleus::createConstantString(str));

	// Add optional file, line and function info if provided.
	if(file != nullptr)
	{
		vals.push_back(Nucleus::createConstantString(file));
		if(line > 0)
		{
			vals.push_back(Nucleus::createConstantInt(line));
		}
	}
	if(function != nullptr)
	{
		vals.push_back(Nucleus::createConstantString(function));
	}

	// Add all format arguments.
	for(const PrintValue &arg : args)
	{
		for(auto val : arg.values)
		{
			vals.push_back(val);
		}
	}

	// This call is implemented by each backend
	VPrintf(vals);
}

// This is the function that is called by VPrintf from the backends
int DebugPrintf(const char *format, ...)
{
	// Uncomment this to make it so that we do not print, but the call to this function is emitted.
	// Useful when debugging emitted code to see the Reactor source location.
	// #	define RR_PRINT_OUTPUT_TYPE_STUB

#	if defined(RR_PRINT_OUTPUT_TYPE_STUB)
	return 0;
#	else

	int result;
	va_list args;

	va_start(args, format);
	char buffer[2048];
	result = vsprintf(buffer, format, args);
	va_end(args);

	std::fputs(buffer, stdout);
#		if defined(_WIN32)
	OutputDebugString(buffer);
#		endif

	return result;
#	endif
}

#endif  // ENABLE_RR_PRINT

// Functions implemented by backends
bool HasRcpApprox();
RValue<Float4> RcpApprox(RValue<Float4> x, bool exactAtPow2 = false);
RValue<Float> RcpApprox(RValue<Float> x, bool exactAtPow2 = false);

template<typename T>
static RValue<T> DoRcp(RValue<T> x, bool relaxedPrecision, bool exactAtPow2)
{
#if defined(__i386__) || defined(__x86_64__)  // On x86, 1/x is fast enough, except for lower precision
	bool approx = HasRcpApprox() && relaxedPrecision;
#else
	bool approx = HasRcpApprox();
#endif

	T rcp;

	if(approx)
	{
		rcp = RcpApprox(x, exactAtPow2);

		if(!relaxedPrecision)
		{
			// Perform one more iteration of Newton-Rhapson division to increase precision
			rcp = (rcp + rcp) - (x * rcp * rcp);
		}
	}
	else
	{
		rcp = T(1.0f) / x;
	}

	return rcp;
}

RValue<Float4> Rcp(RValue<Float4> x, bool relaxedPrecision, bool exactAtPow2)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return DoRcp(x, relaxedPrecision, exactAtPow2);
}

RValue<Float> Rcp(RValue<Float> x, bool relaxedPrecision, bool exactAtPow2)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return DoRcp(x, relaxedPrecision, exactAtPow2);
}

// Functions implemented by backends
bool HasRcpSqrtApprox();
RValue<Float4> RcpSqrtApprox(RValue<Float4> x);
RValue<Float> RcpSqrtApprox(RValue<Float> x);

template<typename T>
struct CastToIntType;

template<>
struct CastToIntType<Float4>
{
	using type = Int4;
};

template<>
struct CastToIntType<Float>
{
	using type = Int;
};

// TODO: move to Reactor.hpp?
RValue<Int> CmpNEQ(RValue<Int> x, RValue<Int> y)
{
	return IfThenElse(x != y, Int(~0), Int(0));
}

template<typename T>
static RValue<T> DoRcpSqrt(RValue<T> x, bool relaxedPrecision)
{
#if defined(__i386__) || defined(__x86_64__)  // On x86, 1/x is fast enough, except for lower precision
	bool approx = HasRcpApprox() && relaxedPrecision;
#else
	bool approx = HasRcpApprox();
#endif

	if(approx)
	{
		using IntType = typename CastToIntType<T>::type;

		T rsq = RcpSqrtApprox(x);

		if(!relaxedPrecision)
		{
			rsq = rsq * (T(3.0f) - rsq * rsq * x) * T(0.5f);
			rsq = As<T>(CmpNEQ(As<IntType>(x), IntType(0x7F800000)) & As<IntType>(rsq));
		}

		return rsq;
	}
	else
	{
		return T(1.0f) / Sqrt(x);
	}
}

RValue<Float4> RcpSqrt(RValue<Float4> x, bool relaxedPrecision)
{
	return DoRcpSqrt(x, relaxedPrecision);
}

RValue<Float> RcpSqrt(RValue<Float> x, bool relaxedPrecision)
{
	return DoRcpSqrt(x, relaxedPrecision);
}

}  // namespace rr
