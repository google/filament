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

#ifndef rr_Reactor_hpp
#define rr_Reactor_hpp

#include "Nucleus.hpp"
#include "Pragma.hpp"
#include "Routine.hpp"
#include "Swizzle.hpp"
#include "Traits.hpp"

#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <limits>
#include <tuple>
#include <unordered_map>

#ifdef ENABLE_RR_DEBUG_INFO
// Functions used for generating JIT debug info.
// See docs/ReactorDebugInfo.md for more information.
namespace rr {
// Update the current source location for debug.
void EmitDebugLocation();
// Bind value to its symbolic name taken from the backtrace.
void EmitDebugVariable(class Value *value);
// Flush any pending variable bindings before the line ends.
void FlushDebug();
}  // namespace rr
#	define RR_DEBUG_INFO_UPDATE_LOC() rr::EmitDebugLocation()
#	define RR_DEBUG_INFO_EMIT_VAR(value) rr::EmitDebugVariable(value)
#	define RR_DEBUG_INFO_FLUSH() rr::FlushDebug()
#else
#	define RR_DEBUG_INFO_UPDATE_LOC()
#	define RR_DEBUG_INFO_EMIT_VAR(value)
#	define RR_DEBUG_INFO_FLUSH()
#endif  // ENABLE_RR_DEBUG_INFO

#ifdef ENABLE_RR_PRINT
namespace rr {
int DebugPrintf(const char *format, ...);
}
#endif

// A Clang extension to determine compiler features.
// We use it to detect Sanitizer builds (e.g. -fsanitize=memory).
#ifndef __has_feature
#	define __has_feature(x) 0
#endif

namespace rr {

struct Caps
{
	static std::string backendName();
	static bool coroutinesSupported();  // Support for rr::Coroutine<F>
	static bool fmaIsFast();            // rr::FMA() is faster than `x * y + z`
};

class Bool;
class Byte;
class SByte;
class Byte4;
class SByte4;
class Byte8;
class SByte8;
class Byte16;
class SByte16;
class Short;
class UShort;
class Short2;
class UShort2;
class Short4;
class UShort4;
class Short8;
class UShort8;
class Int;
class UInt;
class Int2;
class UInt2;
class Int4;
class UInt4;
class Long;
class Half;
class Float;
class Float2;
class Float4;

namespace SIMD {
class Int;
class UInt;
class Float;
}  // namespace SIMD

template<>
struct Scalar<Float4>
{
	using Type = Float;
};

template<>
struct Scalar<Int4>
{
	using Type = Int;
};

template<>
struct Scalar<UInt4>
{
	using Type = UInt;
};

template<>
struct Scalar<SIMD::Float>
{
	using Type = Float;
};

template<>
struct Scalar<SIMD::Int>
{
	using Type = Int;
};

template<>
struct Scalar<SIMD::UInt>
{
	using Type = UInt;
};

class Void
{
public:
	static Type *type();
};

template<class T>
class RValue;

template<class T>
class Pointer;

class Variable
{
	friend class Nucleus;

	Variable() = delete;
	Variable &operator=(const Variable &) = delete;

public:
	void materialize() const;

	Value *loadValue() const;
	Value *storeValue(Value *value) const;

	Value *getBaseAddress() const;
	Value *getElementPointer(Value *index, bool unsignedIndex) const;

	Type *getType() const { return type; }
	int getArraySize() const { return arraySize; }

	// This function is only public for testing purposes, as it affects performance.
	// It is not considered part of Reactor's public API.
	static void materializeAll();

protected:
	Variable(Type *type, int arraySize);
	Variable(const Variable &) = default;

	virtual ~Variable();

private:
	static void killUnmaterialized();

	// Set of variables that do not have a stack location yet.
	class UnmaterializedVariables
	{
	public:
		void add(const Variable *v);
		void remove(const Variable *v);
		void clear();
		void materializeAll();

	private:
		int counter = 0;
		std::unordered_map<const Variable *, int> variables;
	};

	// This has to be a raw pointer because glibc 2.17 doesn't support __cxa_thread_atexit_impl
	// for destructing objects at exit. See crbug.com/1074222
	static thread_local UnmaterializedVariables *unmaterializedVariables;

	Type *const type;
	const int arraySize;
	mutable Value *rvalue = nullptr;
	mutable Value *address = nullptr;
};

template<class T>
class LValue : public Variable
{
public:
	LValue(int arraySize = 0);

	RValue<Pointer<T>> operator&();

	RValue<T> load() const
	{
		return RValue<T>(this->loadValue());
	}

	RValue<T> store(RValue<T> rvalue) const
	{
		this->storeValue(rvalue.value());

		return rvalue;
	}

	// self() returns the this pointer to this LValue<T> object.
	// This function exists because operator&() is overloaded.
	inline LValue<T> *self() { return this; }
};

template<class T>
class Reference
{
public:
	using reference_underlying_type = T;

	explicit Reference(Value *pointer, int alignment = 1);
	Reference(const Reference<T> &ref) = default;

	RValue<T> operator=(RValue<T> rhs) const;
	RValue<T> operator=(const Reference<T> &ref) const;

	RValue<T> operator+=(RValue<T> rhs) const;

	RValue<Pointer<T>> operator&() const { return RValue<Pointer<T>>(address); }

	Value *loadValue() const;
	RValue<T> load() const;
	int getAlignment() const;

private:
	Value *const address;

	const int alignment;
};

template<class T>
struct BoolLiteral
{
	struct Type;
};

template<>
struct BoolLiteral<Bool>
{
	using Type = bool;
};

template<class T>
struct IntLiteral
{
	struct Type;
};

template<>
struct IntLiteral<Int>
{
	using Type = int;
};

template<>
struct IntLiteral<UInt>
{
	using Type = unsigned int;
};

template<class T>
struct LongLiteral
{
	struct Type;
};

template<>
struct LongLiteral<Long>
{
	using Type = int64_t;
};

template<class T>
struct FloatLiteral
{
	struct Type;
};

template<>
struct FloatLiteral<Float>
{
	using Type = float;
};

template<class T>
struct BroadcastLiteral
{
	struct Type;
};

template<>
struct BroadcastLiteral<Int4>
{
	using Type = int;
};

template<>
struct BroadcastLiteral<UInt4>
{
	using Type = unsigned int;
};

template<>
struct BroadcastLiteral<Float4>
{
	using Type = float;
};

template<>
struct BroadcastLiteral<SIMD::Int>
{
	using Type = int;
};

template<>
struct BroadcastLiteral<SIMD::UInt>
{
	using Type = unsigned int;
};

template<>
struct BroadcastLiteral<SIMD::Float>
{
	using Type = float;
};

template<class T>
class RValue
{
public:
	using rvalue_underlying_type = T;

	explicit RValue(Value *rvalue);

	RValue(const RValue<T> &rvalue);
	RValue(const T &lvalue);
	RValue(typename BoolLiteral<T>::Type b);
	RValue(typename IntLiteral<T>::Type i);
	RValue(typename LongLiteral<T>::Type i);
	RValue(typename FloatLiteral<T>::Type f);
	RValue(typename BroadcastLiteral<T>::Type x);
	RValue(const Reference<T> &rhs);

	// Rvalues cannot be assigned to: "(a + b) = c;"
	RValue<T> &operator=(const RValue<T> &) = delete;

	Value *value() const { return val; }

	static int element_count() { return T::element_count(); }

private:
	Value *const val;
};

template<typename T>
class Argument
{
public:
	explicit Argument(Value *val)
	    : val(val)
	{}

	RValue<T> rvalue() const { return RValue<T>(val); }

private:
	Value *const val;
};

class Bool : public LValue<Bool>
{
public:
	Bool(Argument<Bool> argument);

	Bool() = default;
	Bool(bool x);
	Bool(RValue<Bool> rhs);
	Bool(const Bool &rhs);
	Bool(const Reference<Bool> &rhs);

	//	RValue<Bool> operator=(bool rhs);   // FIXME: Implement
	RValue<Bool> operator=(RValue<Bool> rhs);
	RValue<Bool> operator=(const Bool &rhs);
	RValue<Bool> operator=(const Reference<Bool> &rhs);

	static Type *type();
};

RValue<Bool> operator!(RValue<Bool> val);
RValue<Bool> operator&&(RValue<Bool> lhs, RValue<Bool> rhs);
RValue<Bool> operator||(RValue<Bool> lhs, RValue<Bool> rhs);
RValue<Bool> operator!=(RValue<Bool> lhs, RValue<Bool> rhs);
RValue<Bool> operator==(RValue<Bool> lhs, RValue<Bool> rhs);

class Byte : public LValue<Byte>
{
public:
	Byte(Argument<Byte> argument);

	explicit Byte(RValue<Int> cast);
	explicit Byte(RValue<UInt> cast);
	explicit Byte(RValue<UShort> cast);

	Byte() = default;
	Byte(int x);
	Byte(unsigned char x);
	Byte(RValue<Byte> rhs);
	Byte(const Byte &rhs);
	Byte(const Reference<Byte> &rhs);

	//	RValue<Byte> operator=(unsigned char rhs);   // FIXME: Implement
	RValue<Byte> operator=(RValue<Byte> rhs);
	RValue<Byte> operator=(const Byte &rhs);
	RValue<Byte> operator=(const Reference<Byte> &rhs);

	static Type *type();
};

RValue<Byte> operator+(RValue<Byte> lhs, RValue<Byte> rhs);
RValue<Byte> operator-(RValue<Byte> lhs, RValue<Byte> rhs);
RValue<Byte> operator*(RValue<Byte> lhs, RValue<Byte> rhs);
RValue<Byte> operator/(RValue<Byte> lhs, RValue<Byte> rhs);
RValue<Byte> operator%(RValue<Byte> lhs, RValue<Byte> rhs);
RValue<Byte> operator&(RValue<Byte> lhs, RValue<Byte> rhs);
RValue<Byte> operator|(RValue<Byte> lhs, RValue<Byte> rhs);
RValue<Byte> operator^(RValue<Byte> lhs, RValue<Byte> rhs);
RValue<Byte> operator<<(RValue<Byte> lhs, RValue<Byte> rhs);
RValue<Byte> operator>>(RValue<Byte> lhs, RValue<Byte> rhs);
RValue<Byte> operator+=(Byte &lhs, RValue<Byte> rhs);
RValue<Byte> operator-=(Byte &lhs, RValue<Byte> rhs);
RValue<Byte> operator*=(Byte &lhs, RValue<Byte> rhs);
RValue<Byte> operator/=(Byte &lhs, RValue<Byte> rhs);
RValue<Byte> operator%=(Byte &lhs, RValue<Byte> rhs);
RValue<Byte> operator&=(Byte &lhs, RValue<Byte> rhs);
RValue<Byte> operator|=(Byte &lhs, RValue<Byte> rhs);
RValue<Byte> operator^=(Byte &lhs, RValue<Byte> rhs);
RValue<Byte> operator<<=(Byte &lhs, RValue<Byte> rhs);
RValue<Byte> operator>>=(Byte &lhs, RValue<Byte> rhs);
RValue<Byte> operator+(RValue<Byte> val);
RValue<Byte> operator-(RValue<Byte> val);
RValue<Byte> operator~(RValue<Byte> val);
RValue<Byte> operator++(Byte &val, int);  // Post-increment
const Byte &operator++(Byte &val);        // Pre-increment
RValue<Byte> operator--(Byte &val, int);  // Post-decrement
const Byte &operator--(Byte &val);        // Pre-decrement
RValue<Bool> operator<(RValue<Byte> lhs, RValue<Byte> rhs);
RValue<Bool> operator<=(RValue<Byte> lhs, RValue<Byte> rhs);
RValue<Bool> operator>(RValue<Byte> lhs, RValue<Byte> rhs);
RValue<Bool> operator>=(RValue<Byte> lhs, RValue<Byte> rhs);
RValue<Bool> operator!=(RValue<Byte> lhs, RValue<Byte> rhs);
RValue<Bool> operator==(RValue<Byte> lhs, RValue<Byte> rhs);

class SByte : public LValue<SByte>
{
public:
	SByte(Argument<SByte> argument);

	explicit SByte(RValue<Int> cast);
	explicit SByte(RValue<Short> cast);

	SByte() = default;
	SByte(signed char x);
	SByte(RValue<SByte> rhs);
	SByte(const SByte &rhs);
	SByte(const Reference<SByte> &rhs);

	//	RValue<SByte> operator=(signed char rhs);   // FIXME: Implement
	RValue<SByte> operator=(RValue<SByte> rhs);
	RValue<SByte> operator=(const SByte &rhs);
	RValue<SByte> operator=(const Reference<SByte> &rhs);

	static Type *type();
};

RValue<SByte> operator+(RValue<SByte> lhs, RValue<SByte> rhs);
RValue<SByte> operator-(RValue<SByte> lhs, RValue<SByte> rhs);
RValue<SByte> operator*(RValue<SByte> lhs, RValue<SByte> rhs);
RValue<SByte> operator/(RValue<SByte> lhs, RValue<SByte> rhs);
RValue<SByte> operator%(RValue<SByte> lhs, RValue<SByte> rhs);
RValue<SByte> operator&(RValue<SByte> lhs, RValue<SByte> rhs);
RValue<SByte> operator|(RValue<SByte> lhs, RValue<SByte> rhs);
RValue<SByte> operator^(RValue<SByte> lhs, RValue<SByte> rhs);
RValue<SByte> operator<<(RValue<SByte> lhs, RValue<SByte> rhs);
RValue<SByte> operator>>(RValue<SByte> lhs, RValue<SByte> rhs);
RValue<SByte> operator+=(SByte &lhs, RValue<SByte> rhs);
RValue<SByte> operator-=(SByte &lhs, RValue<SByte> rhs);
RValue<SByte> operator*=(SByte &lhs, RValue<SByte> rhs);
RValue<SByte> operator/=(SByte &lhs, RValue<SByte> rhs);
RValue<SByte> operator%=(SByte &lhs, RValue<SByte> rhs);
RValue<SByte> operator&=(SByte &lhs, RValue<SByte> rhs);
RValue<SByte> operator|=(SByte &lhs, RValue<SByte> rhs);
RValue<SByte> operator^=(SByte &lhs, RValue<SByte> rhs);
RValue<SByte> operator<<=(SByte &lhs, RValue<SByte> rhs);
RValue<SByte> operator>>=(SByte &lhs, RValue<SByte> rhs);
RValue<SByte> operator+(RValue<SByte> val);
RValue<SByte> operator-(RValue<SByte> val);
RValue<SByte> operator~(RValue<SByte> val);
RValue<SByte> operator++(SByte &val, int);  // Post-increment
const SByte &operator++(SByte &val);        // Pre-increment
RValue<SByte> operator--(SByte &val, int);  // Post-decrement
const SByte &operator--(SByte &val);        // Pre-decrement
RValue<Bool> operator<(RValue<SByte> lhs, RValue<SByte> rhs);
RValue<Bool> operator<=(RValue<SByte> lhs, RValue<SByte> rhs);
RValue<Bool> operator>(RValue<SByte> lhs, RValue<SByte> rhs);
RValue<Bool> operator>=(RValue<SByte> lhs, RValue<SByte> rhs);
RValue<Bool> operator!=(RValue<SByte> lhs, RValue<SByte> rhs);
RValue<Bool> operator==(RValue<SByte> lhs, RValue<SByte> rhs);

class Short : public LValue<Short>
{
public:
	Short(Argument<Short> argument);

	explicit Short(RValue<Int> cast);

	Short() = default;
	Short(short x);
	Short(RValue<Short> rhs);
	Short(const Short &rhs);
	Short(const Reference<Short> &rhs);

	//	RValue<Short> operator=(short rhs);   // FIXME: Implement
	RValue<Short> operator=(RValue<Short> rhs);
	RValue<Short> operator=(const Short &rhs);
	RValue<Short> operator=(const Reference<Short> &rhs);

	static Type *type();
};

RValue<Short> operator+(RValue<Short> lhs, RValue<Short> rhs);
RValue<Short> operator-(RValue<Short> lhs, RValue<Short> rhs);
RValue<Short> operator*(RValue<Short> lhs, RValue<Short> rhs);
RValue<Short> operator/(RValue<Short> lhs, RValue<Short> rhs);
RValue<Short> operator%(RValue<Short> lhs, RValue<Short> rhs);
RValue<Short> operator&(RValue<Short> lhs, RValue<Short> rhs);
RValue<Short> operator|(RValue<Short> lhs, RValue<Short> rhs);
RValue<Short> operator^(RValue<Short> lhs, RValue<Short> rhs);
RValue<Short> operator<<(RValue<Short> lhs, RValue<Short> rhs);
RValue<Short> operator>>(RValue<Short> lhs, RValue<Short> rhs);
RValue<Short> operator+=(Short &lhs, RValue<Short> rhs);
RValue<Short> operator-=(Short &lhs, RValue<Short> rhs);
RValue<Short> operator*=(Short &lhs, RValue<Short> rhs);
RValue<Short> operator/=(Short &lhs, RValue<Short> rhs);
RValue<Short> operator%=(Short &lhs, RValue<Short> rhs);
RValue<Short> operator&=(Short &lhs, RValue<Short> rhs);
RValue<Short> operator|=(Short &lhs, RValue<Short> rhs);
RValue<Short> operator^=(Short &lhs, RValue<Short> rhs);
RValue<Short> operator<<=(Short &lhs, RValue<Short> rhs);
RValue<Short> operator>>=(Short &lhs, RValue<Short> rhs);
RValue<Short> operator+(RValue<Short> val);
RValue<Short> operator-(RValue<Short> val);
RValue<Short> operator~(RValue<Short> val);
RValue<Short> operator++(Short &val, int);  // Post-increment
const Short &operator++(Short &val);        // Pre-increment
RValue<Short> operator--(Short &val, int);  // Post-decrement
const Short &operator--(Short &val);        // Pre-decrement
RValue<Bool> operator<(RValue<Short> lhs, RValue<Short> rhs);
RValue<Bool> operator<=(RValue<Short> lhs, RValue<Short> rhs);
RValue<Bool> operator>(RValue<Short> lhs, RValue<Short> rhs);
RValue<Bool> operator>=(RValue<Short> lhs, RValue<Short> rhs);
RValue<Bool> operator!=(RValue<Short> lhs, RValue<Short> rhs);
RValue<Bool> operator==(RValue<Short> lhs, RValue<Short> rhs);

class UShort : public LValue<UShort>
{
public:
	UShort(Argument<UShort> argument);

	explicit UShort(RValue<UInt> cast);
	explicit UShort(RValue<Int> cast);
	explicit UShort(RValue<Byte> cast);

	UShort() = default;
	UShort(unsigned short x);
	UShort(RValue<UShort> rhs);
	UShort(const UShort &rhs);
	UShort(const Reference<UShort> &rhs);

	//	RValue<UShort> operator=(unsigned short rhs);   // FIXME: Implement
	RValue<UShort> operator=(RValue<UShort> rhs);
	RValue<UShort> operator=(const UShort &rhs);
	RValue<UShort> operator=(const Reference<UShort> &rhs);

	static Type *type();
};

RValue<UShort> operator+(RValue<UShort> lhs, RValue<UShort> rhs);
RValue<UShort> operator-(RValue<UShort> lhs, RValue<UShort> rhs);
RValue<UShort> operator*(RValue<UShort> lhs, RValue<UShort> rhs);
RValue<UShort> operator/(RValue<UShort> lhs, RValue<UShort> rhs);
RValue<UShort> operator%(RValue<UShort> lhs, RValue<UShort> rhs);
RValue<UShort> operator&(RValue<UShort> lhs, RValue<UShort> rhs);
RValue<UShort> operator|(RValue<UShort> lhs, RValue<UShort> rhs);
RValue<UShort> operator^(RValue<UShort> lhs, RValue<UShort> rhs);
RValue<UShort> operator<<(RValue<UShort> lhs, RValue<UShort> rhs);
RValue<UShort> operator>>(RValue<UShort> lhs, RValue<UShort> rhs);
RValue<UShort> operator+=(UShort &lhs, RValue<UShort> rhs);
RValue<UShort> operator-=(UShort &lhs, RValue<UShort> rhs);
RValue<UShort> operator*=(UShort &lhs, RValue<UShort> rhs);
RValue<UShort> operator/=(UShort &lhs, RValue<UShort> rhs);
RValue<UShort> operator%=(UShort &lhs, RValue<UShort> rhs);
RValue<UShort> operator&=(UShort &lhs, RValue<UShort> rhs);
RValue<UShort> operator|=(UShort &lhs, RValue<UShort> rhs);
RValue<UShort> operator^=(UShort &lhs, RValue<UShort> rhs);
RValue<UShort> operator<<=(UShort &lhs, RValue<UShort> rhs);
RValue<UShort> operator>>=(UShort &lhs, RValue<UShort> rhs);
RValue<UShort> operator+(RValue<UShort> val);
RValue<UShort> operator-(RValue<UShort> val);
RValue<UShort> operator~(RValue<UShort> val);
RValue<UShort> operator++(UShort &val, int);  // Post-increment
const UShort &operator++(UShort &val);        // Pre-increment
RValue<UShort> operator--(UShort &val, int);  // Post-decrement
const UShort &operator--(UShort &val);        // Pre-decrement
RValue<Bool> operator<(RValue<UShort> lhs, RValue<UShort> rhs);
RValue<Bool> operator<=(RValue<UShort> lhs, RValue<UShort> rhs);
RValue<Bool> operator>(RValue<UShort> lhs, RValue<UShort> rhs);
RValue<Bool> operator>=(RValue<UShort> lhs, RValue<UShort> rhs);
RValue<Bool> operator!=(RValue<UShort> lhs, RValue<UShort> rhs);
RValue<Bool> operator==(RValue<UShort> lhs, RValue<UShort> rhs);

class Byte4 : public LValue<Byte4>
{
public:
	explicit Byte4(RValue<Byte8> cast);
	explicit Byte4(RValue<UShort4> cast);
	explicit Byte4(RValue<Short4> cast);
	explicit Byte4(RValue<UInt4> cast);
	explicit Byte4(RValue<Int4> cast);

	Byte4() = default;
	//	Byte4(int x, int y, int z, int w);
	Byte4(RValue<Byte4> rhs);
	Byte4(const Byte4 &rhs);
	Byte4(const Reference<Byte4> &rhs);

	RValue<Byte4> operator=(RValue<Byte4> rhs);
	RValue<Byte4> operator=(const Byte4 &rhs);
	//	RValue<Byte4> operator=(const Reference<Byte4> &rhs);

	static Type *type();
	static int element_count() { return 4; }
};

RValue<Byte4> Insert(RValue<Byte4> val, RValue<Byte> element, int i);

//	RValue<Byte4> operator+(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator-(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator*(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator/(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator%(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator&(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator|(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator^(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator<<(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator>>(RValue<Byte4> lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator+=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator-=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator*=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator/=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator%=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator&=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator|=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator^=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator<<=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator>>=(Byte4 &lhs, RValue<Byte4> rhs);
//	RValue<Byte4> operator+(RValue<Byte4> val);
//	RValue<Byte4> operator-(RValue<Byte4> val);
//	RValue<Byte4> operator~(RValue<Byte4> val);
//	RValue<Byte4> operator++(Byte4 &val, int);   // Post-increment
//	const Byte4 &operator++(Byte4 &val);   // Pre-increment
//	RValue<Byte4> operator--(Byte4 &val, int);   // Post-decrement
//	const Byte4 &operator--(Byte4 &val);   // Pre-decrement

class SByte4 : public LValue<SByte4>
{
public:
	SByte4() = default;
	//	SByte4(int x, int y, int z, int w);
	//	SByte4(RValue<SByte4> rhs);
	//	SByte4(const SByte4 &rhs);
	//	SByte4(const Reference<SByte4> &rhs);

	//	RValue<SByte4> operator=(RValue<SByte4> rhs);
	//	RValue<SByte4> operator=(const SByte4 &rhs);
	//	RValue<SByte4> operator=(const Reference<SByte4> &rhs);

	static Type *type();
	static int element_count() { return 4; }
};

//	RValue<SByte4> operator+(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator-(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator*(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator/(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator%(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator&(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator|(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator^(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator<<(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator>>(RValue<SByte4> lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator+=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator-=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator*=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator/=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator%=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator&=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator|=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator^=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator<<=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator>>=(SByte4 &lhs, RValue<SByte4> rhs);
//	RValue<SByte4> operator+(RValue<SByte4> val);
//	RValue<SByte4> operator-(RValue<SByte4> val);
//	RValue<SByte4> operator~(RValue<SByte4> val);
//	RValue<SByte4> operator++(SByte4 &val, int);   // Post-increment
//	const SByte4 &operator++(SByte4 &val);   // Pre-increment
//	RValue<SByte4> operator--(SByte4 &val, int);   // Post-decrement
//	const SByte4 &operator--(SByte4 &val);   // Pre-decrement

class Byte8 : public LValue<Byte8>
{
public:
	Byte8() = default;
	Byte8(uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4, uint8_t x5, uint8_t x6, uint8_t x7);
	Byte8(RValue<Byte8> rhs);
	Byte8(const Byte8 &rhs);
	Byte8(const Reference<Byte8> &rhs);

	RValue<Byte8> operator=(RValue<Byte8> rhs);
	RValue<Byte8> operator=(const Byte8 &rhs);
	RValue<Byte8> operator=(const Reference<Byte8> &rhs);

	static Type *type();
	static int element_count() { return 8; }
};

RValue<Byte8> operator+(RValue<Byte8> lhs, RValue<Byte8> rhs);
RValue<Byte8> operator-(RValue<Byte8> lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator*(RValue<Byte8> lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator/(RValue<Byte8> lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator%(RValue<Byte8> lhs, RValue<Byte8> rhs);
RValue<Byte8> operator&(RValue<Byte8> lhs, RValue<Byte8> rhs);
RValue<Byte8> operator|(RValue<Byte8> lhs, RValue<Byte8> rhs);
RValue<Byte8> operator^(RValue<Byte8> lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator<<(RValue<Byte8> lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator>>(RValue<Byte8> lhs, RValue<Byte8> rhs);
RValue<Byte8> operator+=(Byte8 &lhs, RValue<Byte8> rhs);
RValue<Byte8> operator-=(Byte8 &lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator*=(Byte8 &lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator/=(Byte8 &lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator%=(Byte8 &lhs, RValue<Byte8> rhs);
RValue<Byte8> operator&=(Byte8 &lhs, RValue<Byte8> rhs);
RValue<Byte8> operator|=(Byte8 &lhs, RValue<Byte8> rhs);
RValue<Byte8> operator^=(Byte8 &lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator<<=(Byte8 &lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator>>=(Byte8 &lhs, RValue<Byte8> rhs);
//	RValue<Byte8> operator+(RValue<Byte8> val);
//	RValue<Byte8> operator-(RValue<Byte8> val);
RValue<Byte8> operator~(RValue<Byte8> val);
//	RValue<Byte8> operator++(Byte8 &val, int);   // Post-increment
//	const Byte8 &operator++(Byte8 &val);   // Pre-increment
//	RValue<Byte8> operator--(Byte8 &val, int);   // Post-decrement
//	const Byte8 &operator--(Byte8 &val);   // Pre-decrement

RValue<Byte8> AddSat(RValue<Byte8> x, RValue<Byte8> y);
RValue<Byte8> SubSat(RValue<Byte8> x, RValue<Byte8> y);
RValue<Short4> Unpack(RValue<Byte4> x);
RValue<Short4> Unpack(RValue<Byte4> x, RValue<Byte4> y);
RValue<Short4> UnpackLow(RValue<Byte8> x, RValue<Byte8> y);
RValue<Short4> UnpackHigh(RValue<Byte8> x, RValue<Byte8> y);
RValue<Int> SignMask(RValue<Byte8> x);
//	RValue<Byte8> CmpGT(RValue<Byte8> x, RValue<Byte8> y);
RValue<Byte8> CmpEQ(RValue<Byte8> x, RValue<Byte8> y);
RValue<Byte8> Swizzle(RValue<Byte8> x, uint32_t select);

class SByte8 : public LValue<SByte8>
{
public:
	SByte8() = default;
	SByte8(uint8_t x0, uint8_t x1, uint8_t x2, uint8_t x3, uint8_t x4, uint8_t x5, uint8_t x6, uint8_t x7);
	SByte8(RValue<SByte8> rhs);
	SByte8(const SByte8 &rhs);
	SByte8(const Reference<SByte8> &rhs);

	RValue<SByte8> operator=(RValue<SByte8> rhs);
	RValue<SByte8> operator=(const SByte8 &rhs);
	RValue<SByte8> operator=(const Reference<SByte8> &rhs);

	static Type *type();
	static int element_count() { return 8; }
};

RValue<SByte8> operator+(RValue<SByte8> lhs, RValue<SByte8> rhs);
RValue<SByte8> operator-(RValue<SByte8> lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator*(RValue<SByte8> lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator/(RValue<SByte8> lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator%(RValue<SByte8> lhs, RValue<SByte8> rhs);
RValue<SByte8> operator&(RValue<SByte8> lhs, RValue<SByte8> rhs);
RValue<SByte8> operator|(RValue<SByte8> lhs, RValue<SByte8> rhs);
RValue<SByte8> operator^(RValue<SByte8> lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator<<(RValue<SByte8> lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator>>(RValue<SByte8> lhs, RValue<SByte8> rhs);
RValue<SByte8> operator+=(SByte8 &lhs, RValue<SByte8> rhs);
RValue<SByte8> operator-=(SByte8 &lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator*=(SByte8 &lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator/=(SByte8 &lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator%=(SByte8 &lhs, RValue<SByte8> rhs);
RValue<SByte8> operator&=(SByte8 &lhs, RValue<SByte8> rhs);
RValue<SByte8> operator|=(SByte8 &lhs, RValue<SByte8> rhs);
RValue<SByte8> operator^=(SByte8 &lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator<<=(SByte8 &lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator>>=(SByte8 &lhs, RValue<SByte8> rhs);
//	RValue<SByte8> operator+(RValue<SByte8> val);
//	RValue<SByte8> operator-(RValue<SByte8> val);
RValue<SByte8> operator~(RValue<SByte8> val);
//	RValue<SByte8> operator++(SByte8 &val, int);   // Post-increment
//	const SByte8 &operator++(SByte8 &val);   // Pre-increment
//	RValue<SByte8> operator--(SByte8 &val, int);   // Post-decrement
//	const SByte8 &operator--(SByte8 &val);   // Pre-decrement

RValue<SByte8> AddSat(RValue<SByte8> x, RValue<SByte8> y);
RValue<SByte8> SubSat(RValue<SByte8> x, RValue<SByte8> y);
RValue<Short4> UnpackLow(RValue<SByte8> x, RValue<SByte8> y);
RValue<Short4> UnpackHigh(RValue<SByte8> x, RValue<SByte8> y);
RValue<Int> SignMask(RValue<SByte8> x);
RValue<Byte8> CmpGT(RValue<SByte8> x, RValue<SByte8> y);
RValue<Byte8> CmpEQ(RValue<SByte8> x, RValue<SByte8> y);

class Byte16 : public LValue<Byte16>
{
public:
	Byte16() = default;
	Byte16(RValue<Byte16> rhs);
	Byte16(const Byte16 &rhs);
	Byte16(const Reference<Byte16> &rhs);

	RValue<Byte16> operator=(RValue<Byte16> rhs);
	RValue<Byte16> operator=(const Byte16 &rhs);
	RValue<Byte16> operator=(const Reference<Byte16> &rhs);

	static Type *type();
	static int element_count() { return 16; }
};

//	RValue<Byte16> operator+(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator-(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator*(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator/(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator%(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator&(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator|(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator^(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator<<(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator>>(RValue<Byte16> lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator+=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator-=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator*=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator/=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator%=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator&=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator|=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator^=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator<<=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator>>=(Byte16 &lhs, RValue<Byte16> rhs);
//	RValue<Byte16> operator+(RValue<Byte16> val);
//	RValue<Byte16> operator-(RValue<Byte16> val);
//	RValue<Byte16> operator~(RValue<Byte16> val);
//	RValue<Byte16> operator++(Byte16 &val, int);   // Post-increment
//	const Byte16 &operator++(Byte16 &val);   // Pre-increment
//	RValue<Byte16> operator--(Byte16 &val, int);   // Post-decrement
//	const Byte16 &operator--(Byte16 &val);   // Pre-decrement
RValue<Byte16> Swizzle(RValue<Byte16> x, uint64_t select);

class SByte16 : public LValue<SByte16>
{
public:
	SByte16() = default;
	//	SByte16(int x, int y, int z, int w);
	//	SByte16(RValue<SByte16> rhs);
	//	SByte16(const SByte16 &rhs);
	//	SByte16(const Reference<SByte16> &rhs);

	//	RValue<SByte16> operator=(RValue<SByte16> rhs);
	//	RValue<SByte16> operator=(const SByte16 &rhs);
	//	RValue<SByte16> operator=(const Reference<SByte16> &rhs);

	static Type *type();
	static int element_count() { return 16; }
};

//	RValue<SByte16> operator+(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator-(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator*(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator/(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator%(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator&(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator|(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator^(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator<<(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator>>(RValue<SByte16> lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator+=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator-=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator*=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator/=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator%=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator&=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator|=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator^=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator<<=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator>>=(SByte16 &lhs, RValue<SByte16> rhs);
//	RValue<SByte16> operator+(RValue<SByte16> val);
//	RValue<SByte16> operator-(RValue<SByte16> val);
//	RValue<SByte16> operator~(RValue<SByte16> val);
//	RValue<SByte16> operator++(SByte16 &val, int);   // Post-increment
//	const SByte16 &operator++(SByte16 &val);   // Pre-increment
//	RValue<SByte16> operator--(SByte16 &val, int);   // Post-decrement
//	const SByte16 &operator--(SByte16 &val);   // Pre-decrement

class Short2 : public LValue<Short2>
{
public:
	explicit Short2(RValue<Short4> cast);

	static Type *type();
	static int element_count() { return 2; }
};

class UShort2 : public LValue<UShort2>
{
public:
	explicit UShort2(RValue<UShort4> cast);

	static Type *type();
	static int element_count() { return 2; }
};

class Short4 : public LValue<Short4>
{
public:
	explicit Short4(RValue<Int> cast);
	explicit Short4(RValue<Int4> cast);
	explicit Short4(RValue<UInt4> cast);
	//	explicit Short4(RValue<Float> cast);
	explicit Short4(RValue<Float4> cast);

	Short4() = default;
	Short4(short xyzw);
	Short4(short x, short y, short z, short w);
	Short4(RValue<Short4> rhs);
	Short4(const Short4 &rhs);
	Short4(const Reference<Short4> &rhs);
	Short4(RValue<UShort4> rhs);
	Short4(const UShort4 &rhs);
	Short4(const Reference<UShort4> &rhs);

	RValue<Short4> operator=(RValue<Short4> rhs);
	RValue<Short4> operator=(const Short4 &rhs);
	RValue<Short4> operator=(const Reference<Short4> &rhs);
	RValue<Short4> operator=(RValue<UShort4> rhs);
	RValue<Short4> operator=(const UShort4 &rhs);
	RValue<Short4> operator=(const Reference<UShort4> &rhs);

	static Type *type();
	static int element_count() { return 4; }
};

RValue<Short4> operator+(RValue<Short4> lhs, RValue<Short4> rhs);
RValue<Short4> operator-(RValue<Short4> lhs, RValue<Short4> rhs);
RValue<Short4> operator*(RValue<Short4> lhs, RValue<Short4> rhs);
//	RValue<Short4> operator/(RValue<Short4> lhs, RValue<Short4> rhs);
//	RValue<Short4> operator%(RValue<Short4> lhs, RValue<Short4> rhs);
RValue<Short4> operator&(RValue<Short4> lhs, RValue<Short4> rhs);
RValue<Short4> operator|(RValue<Short4> lhs, RValue<Short4> rhs);
RValue<Short4> operator^(RValue<Short4> lhs, RValue<Short4> rhs);
RValue<Short4> operator<<(RValue<Short4> lhs, unsigned char rhs);
RValue<Short4> operator>>(RValue<Short4> lhs, unsigned char rhs);
RValue<Short4> operator+=(Short4 &lhs, RValue<Short4> rhs);
RValue<Short4> operator-=(Short4 &lhs, RValue<Short4> rhs);
RValue<Short4> operator*=(Short4 &lhs, RValue<Short4> rhs);
//	RValue<Short4> operator/=(Short4 &lhs, RValue<Short4> rhs);
//	RValue<Short4> operator%=(Short4 &lhs, RValue<Short4> rhs);
RValue<Short4> operator&=(Short4 &lhs, RValue<Short4> rhs);
RValue<Short4> operator|=(Short4 &lhs, RValue<Short4> rhs);
RValue<Short4> operator^=(Short4 &lhs, RValue<Short4> rhs);
RValue<Short4> operator<<=(Short4 &lhs, unsigned char rhs);
RValue<Short4> operator>>=(Short4 &lhs, unsigned char rhs);
//	RValue<Short4> operator+(RValue<Short4> val);
RValue<Short4> operator-(RValue<Short4> val);
RValue<Short4> operator~(RValue<Short4> val);
//	RValue<Short4> operator++(Short4 &val, int);   // Post-increment
//	const Short4 &operator++(Short4 &val);   // Pre-increment
//	RValue<Short4> operator--(Short4 &val, int);   // Post-decrement
//	const Short4 &operator--(Short4 &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<Short4> lhs, RValue<Short4> rhs);
//	RValue<Bool> operator<=(RValue<Short4> lhs, RValue<Short4> rhs);
//	RValue<Bool> operator>(RValue<Short4> lhs, RValue<Short4> rhs);
//	RValue<Bool> operator>=(RValue<Short4> lhs, RValue<Short4> rhs);
//	RValue<Bool> operator!=(RValue<Short4> lhs, RValue<Short4> rhs);
//	RValue<Bool> operator==(RValue<Short4> lhs, RValue<Short4> rhs);

RValue<Short4> RoundShort4(RValue<Float4> cast);
RValue<Short4> Max(RValue<Short4> x, RValue<Short4> y);
RValue<Short4> Min(RValue<Short4> x, RValue<Short4> y);
RValue<Short4> AddSat(RValue<Short4> x, RValue<Short4> y);
RValue<Short4> SubSat(RValue<Short4> x, RValue<Short4> y);
RValue<Short4> MulHigh(RValue<Short4> x, RValue<Short4> y);
RValue<Int2> MulAdd(RValue<Short4> x, RValue<Short4> y);
RValue<SByte8> PackSigned(RValue<Short4> x, RValue<Short4> y);
RValue<Byte8> PackUnsigned(RValue<Short4> x, RValue<Short4> y);
RValue<Int2> UnpackLow(RValue<Short4> x, RValue<Short4> y);
RValue<Int2> UnpackHigh(RValue<Short4> x, RValue<Short4> y);
RValue<Short4> Swizzle(RValue<Short4> x, uint16_t select);
RValue<Short4> Insert(RValue<Short4> val, RValue<Short> element, int i);
RValue<Short> Extract(RValue<Short4> val, int i);
RValue<Short4> CmpGT(RValue<Short4> x, RValue<Short4> y);
RValue<Short4> CmpEQ(RValue<Short4> x, RValue<Short4> y);

class UShort4 : public LValue<UShort4>
{
public:
	explicit UShort4(RValue<UInt4> cast);
	explicit UShort4(RValue<Int4> cast);
	explicit UShort4(RValue<Float4> cast, bool saturate = false);

	UShort4() = default;
	UShort4(unsigned short xyzw);
	UShort4(unsigned short x, unsigned short y, unsigned short z, unsigned short w);
	UShort4(RValue<UShort4> rhs);
	UShort4(const UShort4 &rhs);
	UShort4(const Reference<UShort4> &rhs);
	UShort4(RValue<Short4> rhs);
	UShort4(const Short4 &rhs);
	UShort4(const Reference<Short4> &rhs);

	RValue<UShort4> operator=(RValue<UShort4> rhs);
	RValue<UShort4> operator=(const UShort4 &rhs);
	RValue<UShort4> operator=(const Reference<UShort4> &rhs);
	RValue<UShort4> operator=(RValue<Short4> rhs);
	RValue<UShort4> operator=(const Short4 &rhs);
	RValue<UShort4> operator=(const Reference<Short4> &rhs);

	static Type *type();
	static int element_count() { return 4; }
};

RValue<UShort4> operator+(RValue<UShort4> lhs, RValue<UShort4> rhs);
RValue<UShort4> operator-(RValue<UShort4> lhs, RValue<UShort4> rhs);
RValue<UShort4> operator*(RValue<UShort4> lhs, RValue<UShort4> rhs);
//	RValue<UShort4> operator/(RValue<UShort4> lhs, RValue<UShort4> rhs);
//	RValue<UShort4> operator%(RValue<UShort4> lhs, RValue<UShort4> rhs);
RValue<UShort4> operator&(RValue<UShort4> lhs, RValue<UShort4> rhs);
RValue<UShort4> operator|(RValue<UShort4> lhs, RValue<UShort4> rhs);
RValue<UShort4> operator^(RValue<UShort4> lhs, RValue<UShort4> rhs);
RValue<UShort4> operator<<(RValue<UShort4> lhs, unsigned char rhs);
RValue<UShort4> operator>>(RValue<UShort4> lhs, unsigned char rhs);
//	RValue<UShort4> operator+=(UShort4 &lhs, RValue<UShort4> rhs);
//	RValue<UShort4> operator-=(UShort4 &lhs, RValue<UShort4> rhs);
//	RValue<UShort4> operator*=(UShort4 &lhs, RValue<UShort4> rhs);
//	RValue<UShort4> operator/=(UShort4 &lhs, RValue<UShort4> rhs);
//	RValue<UShort4> operator%=(UShort4 &lhs, RValue<UShort4> rhs);
//	RValue<UShort4> operator&=(UShort4 &lhs, RValue<UShort4> rhs);
//	RValue<UShort4> operator|=(UShort4 &lhs, RValue<UShort4> rhs);
//	RValue<UShort4> operator^=(UShort4 &lhs, RValue<UShort4> rhs);
RValue<UShort4> operator<<=(UShort4 &lhs, unsigned char rhs);
RValue<UShort4> operator>>=(UShort4 &lhs, unsigned char rhs);
//	RValue<UShort4> operator+(RValue<UShort4> val);
//	RValue<UShort4> operator-(RValue<UShort4> val);
RValue<UShort4> operator~(RValue<UShort4> val);
//	RValue<UShort4> operator++(UShort4 &val, int);   // Post-increment
//	const UShort4 &operator++(UShort4 &val);   // Pre-increment
//	RValue<UShort4> operator--(UShort4 &val, int);   // Post-decrement
//	const UShort4 &operator--(UShort4 &val);   // Pre-decrement

RValue<UShort4> Insert(RValue<UShort4> val, RValue<UShort> element, int i);
RValue<UShort4> Max(RValue<UShort4> x, RValue<UShort4> y);
RValue<UShort4> Min(RValue<UShort4> x, RValue<UShort4> y);
RValue<UShort4> AddSat(RValue<UShort4> x, RValue<UShort4> y);
RValue<UShort4> SubSat(RValue<UShort4> x, RValue<UShort4> y);
RValue<UShort4> MulHigh(RValue<UShort4> x, RValue<UShort4> y);
RValue<UShort4> Average(RValue<UShort4> x, RValue<UShort4> y);

class Short8 : public LValue<Short8>
{
public:
	Short8() = default;
	Short8(short c);
	Short8(short c0, short c1, short c2, short c3, short c4, short c5, short c6, short c7);
	Short8(RValue<Short8> rhs);
	//	Short8(const Short8 &rhs);
	Short8(const Reference<Short8> &rhs);
	Short8(RValue<Short4> lo, RValue<Short4> hi);

	RValue<Short8> operator=(RValue<Short8> rhs);
	RValue<Short8> operator=(const Short8 &rhs);
	RValue<Short8> operator=(const Reference<Short8> &rhs);

	static Type *type();
	static int element_count() { return 8; }
};

RValue<Short8> operator+(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Short8> operator-(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Short8> operator*(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Short8> operator/(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Short8> operator%(RValue<Short8> lhs, RValue<Short8> rhs);
RValue<Short8> operator&(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Short8> operator|(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Short8> operator^(RValue<Short8> lhs, RValue<Short8> rhs);
RValue<Short8> operator<<(RValue<Short8> lhs, unsigned char rhs);
RValue<Short8> operator>>(RValue<Short8> lhs, unsigned char rhs);
//	RValue<Short8> operator<<(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Short8> operator>>(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Short8> operator+=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator-=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator*=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator/=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator%=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator&=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator|=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator^=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator<<=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator>>=(Short8 &lhs, RValue<Short8> rhs);
//	RValue<Short8> operator+(RValue<Short8> val);
//	RValue<Short8> operator-(RValue<Short8> val);
//	RValue<Short8> operator~(RValue<Short8> val);
//	RValue<Short8> operator++(Short8 &val, int);   // Post-increment
//	const Short8 &operator++(Short8 &val);   // Pre-increment
//	RValue<Short8> operator--(Short8 &val, int);   // Post-decrement
//	const Short8 &operator--(Short8 &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Bool> operator<=(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Bool> operator>(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Bool> operator>=(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Bool> operator!=(RValue<Short8> lhs, RValue<Short8> rhs);
//	RValue<Bool> operator==(RValue<Short8> lhs, RValue<Short8> rhs);

RValue<Short8> MulHigh(RValue<Short8> x, RValue<Short8> y);
RValue<Int4> MulAdd(RValue<Short8> x, RValue<Short8> y);

class UShort8 : public LValue<UShort8>
{
public:
	UShort8() = default;
	UShort8(unsigned short c);
	UShort8(unsigned short c0, unsigned short c1, unsigned short c2, unsigned short c3, unsigned short c4, unsigned short c5, unsigned short c6, unsigned short c7);
	UShort8(RValue<UShort8> rhs);
	//	UShort8(const UShort8 &rhs);
	UShort8(const Reference<UShort8> &rhs);
	UShort8(RValue<UShort4> lo, RValue<UShort4> hi);

	RValue<UShort8> operator=(RValue<UShort8> rhs);
	RValue<UShort8> operator=(const UShort8 &rhs);
	RValue<UShort8> operator=(const Reference<UShort8> &rhs);

	static Type *type();
	static int element_count() { return 8; }
};

RValue<UShort8> operator+(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator-(RValue<UShort8> lhs, RValue<UShort8> rhs);
RValue<UShort8> operator*(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator/(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator%(RValue<UShort8> lhs, RValue<UShort8> rhs);
RValue<UShort8> operator&(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator|(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator^(RValue<UShort8> lhs, RValue<UShort8> rhs);
RValue<UShort8> operator<<(RValue<UShort8> lhs, unsigned char rhs);
RValue<UShort8> operator>>(RValue<UShort8> lhs, unsigned char rhs);
//	RValue<UShort8> operator<<(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator>>(RValue<UShort8> lhs, RValue<UShort8> rhs);
RValue<UShort8> operator+=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator-=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator*=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator/=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator%=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator&=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator|=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator^=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator<<=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator>>=(UShort8 &lhs, RValue<UShort8> rhs);
//	RValue<UShort8> operator+(RValue<UShort8> val);
//	RValue<UShort8> operator-(RValue<UShort8> val);
RValue<UShort8> operator~(RValue<UShort8> val);
//	RValue<UShort8> operator++(UShort8 &val, int);   // Post-increment
//	const UShort8 &operator++(UShort8 &val);   // Pre-increment
//	RValue<UShort8> operator--(UShort8 &val, int);   // Post-decrement
//	const UShort8 &operator--(UShort8 &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<Bool> operator<=(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<Bool> operator>(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<Bool> operator>=(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<Bool> operator!=(RValue<UShort8> lhs, RValue<UShort8> rhs);
//	RValue<Bool> operator==(RValue<UShort8> lhs, RValue<UShort8> rhs);

RValue<UShort8> Swizzle(RValue<UShort8> x, uint32_t select);
RValue<UShort8> MulHigh(RValue<UShort8> x, RValue<UShort8> y);

class Int : public LValue<Int>
{
public:
	Int(Argument<Int> argument);

	explicit Int(RValue<Byte> cast);
	explicit Int(RValue<SByte> cast);
	explicit Int(RValue<Short> cast);
	explicit Int(RValue<UShort> cast);
	explicit Int(RValue<Int2> cast);
	explicit Int(RValue<Long> cast);
	explicit Int(RValue<Float> cast);

	Int() = default;
	Int(int x);
	Int(RValue<Int> rhs);
	Int(RValue<UInt> rhs);
	Int(const Int &rhs);
	Int(const UInt &rhs);
	Int(const Reference<Int> &rhs);
	Int(const Reference<UInt> &rhs);

	template<int T>
	Int(const SwizzleMask1<Int4, T> &rhs);

	RValue<Int> operator=(int rhs);
	RValue<Int> operator=(RValue<Int> rhs);
	RValue<Int> operator=(RValue<UInt> rhs);
	RValue<Int> operator=(const Int &rhs);
	RValue<Int> operator=(const UInt &rhs);
	RValue<Int> operator=(const Reference<Int> &rhs);
	RValue<Int> operator=(const Reference<UInt> &rhs);

	template<int T>
	RValue<Int> operator=(const SwizzleMask1<Int4, T> &rhs);

	static Type *type();
};

RValue<Int> operator+(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator-(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator*(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator/(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator%(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator&(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator|(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator^(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator<<(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator>>(RValue<Int> lhs, RValue<Int> rhs);
RValue<Int> operator+=(Int &lhs, RValue<Int> rhs);
RValue<Int> operator-=(Int &lhs, RValue<Int> rhs);
RValue<Int> operator*=(Int &lhs, RValue<Int> rhs);
RValue<Int> operator/=(Int &lhs, RValue<Int> rhs);
RValue<Int> operator%=(Int &lhs, RValue<Int> rhs);
RValue<Int> operator&=(Int &lhs, RValue<Int> rhs);
RValue<Int> operator|=(Int &lhs, RValue<Int> rhs);
RValue<Int> operator^=(Int &lhs, RValue<Int> rhs);
RValue<Int> operator<<=(Int &lhs, RValue<Int> rhs);
RValue<Int> operator>>=(Int &lhs, RValue<Int> rhs);
RValue<Int> operator+(RValue<Int> val);
RValue<Int> operator-(RValue<Int> val);
RValue<Int> operator~(RValue<Int> val);
RValue<Int> operator++(Int &val, int);  // Post-increment
const Int &operator++(Int &val);        // Pre-increment
RValue<Int> operator--(Int &val, int);  // Post-decrement
const Int &operator--(Int &val);        // Pre-decrement
RValue<Bool> operator<(RValue<Int> lhs, RValue<Int> rhs);
RValue<Bool> operator<=(RValue<Int> lhs, RValue<Int> rhs);
RValue<Bool> operator>(RValue<Int> lhs, RValue<Int> rhs);
RValue<Bool> operator>=(RValue<Int> lhs, RValue<Int> rhs);
RValue<Bool> operator!=(RValue<Int> lhs, RValue<Int> rhs);
RValue<Bool> operator==(RValue<Int> lhs, RValue<Int> rhs);

RValue<Int> Max(RValue<Int> x, RValue<Int> y);
RValue<Int> Min(RValue<Int> x, RValue<Int> y);
RValue<Int> Clamp(RValue<Int> x, RValue<Int> min, RValue<Int> max);
RValue<Int> RoundInt(RValue<Float> cast);

class Long : public LValue<Long>
{
public:
	//	Long(Argument<Long> argument);

	//	explicit Long(RValue<Short> cast);
	//	explicit Long(RValue<UShort> cast);
	explicit Long(RValue<Int> cast);
	explicit Long(RValue<UInt> cast);
	//	explicit Long(RValue<Float> cast);

	Long() = default;
	//	Long(qword x);
	Long(RValue<Long> rhs);
	//	Long(RValue<ULong> rhs);
	//	Long(const Long &rhs);
	//	Long(const Reference<Long> &rhs);
	//	Long(const ULong &rhs);
	//	Long(const Reference<ULong> &rhs);

	RValue<Long> operator=(int64_t rhs);
	RValue<Long> operator=(RValue<Long> rhs);
	//	RValue<Long> operator=(RValue<ULong> rhs);
	RValue<Long> operator=(const Long &rhs);
	RValue<Long> operator=(const Reference<Long> &rhs);
	//	RValue<Long> operator=(const ULong &rhs);
	//	RValue<Long> operator=(const Reference<ULong> &rhs);

	static Type *type();
};

RValue<Long> operator+(RValue<Long> lhs, RValue<Long> rhs);
RValue<Long> operator-(RValue<Long> lhs, RValue<Long> rhs);
RValue<Long> operator*(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Long> operator/(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Long> operator%(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Long> operator&(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Long> operator|(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Long> operator^(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Long> operator<<(RValue<Long> lhs, RValue<Long> rhs);
RValue<Long> operator>>(RValue<Long> lhs, RValue<Long> rhs);
RValue<Long> operator+=(Long &lhs, RValue<Long> rhs);
RValue<Long> operator-=(Long &lhs, RValue<Long> rhs);
//	RValue<Long> operator*=(Long &lhs, RValue<Long> rhs);
//	RValue<Long> operator/=(Long &lhs, RValue<Long> rhs);
//	RValue<Long> operator%=(Long &lhs, RValue<Long> rhs);
//	RValue<Long> operator&=(Long &lhs, RValue<Long> rhs);
//	RValue<Long> operator|=(Long &lhs, RValue<Long> rhs);
//	RValue<Long> operator^=(Long &lhs, RValue<Long> rhs);
//	RValue<Long> operator<<=(Long &lhs, RValue<Long> rhs);
//	RValue<Long> operator>>=(Long &lhs, RValue<Long> rhs);
//	RValue<Long> operator+(RValue<Long> val);
//	RValue<Long> operator-(RValue<Long> val);
//	RValue<Long> operator~(RValue<Long> val);
//	RValue<Long> operator++(Long &val, int);   // Post-increment
//	const Long &operator++(Long &val);   // Pre-increment
//	RValue<Long> operator--(Long &val, int);   // Post-decrement
//	const Long &operator--(Long &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Bool> operator<=(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Bool> operator>(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Bool> operator>=(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Bool> operator!=(RValue<Long> lhs, RValue<Long> rhs);
//	RValue<Bool> operator==(RValue<Long> lhs, RValue<Long> rhs);

//	RValue<Long> RoundLong(RValue<Float> cast);
RValue<Long> AddAtomic(RValue<Pointer<Long>> x, RValue<Long> y);

class UInt : public LValue<UInt>
{
public:
	UInt(Argument<UInt> argument);

	explicit UInt(RValue<UShort> cast);
	explicit UInt(RValue<Long> cast);
	explicit UInt(RValue<Float> cast);

	UInt() = default;
	UInt(int x);
	UInt(unsigned int x);
	UInt(RValue<UInt> rhs);
	UInt(RValue<Int> rhs);
	UInt(const UInt &rhs);
	UInt(const Int &rhs);
	UInt(const Reference<UInt> &rhs);
	UInt(const Reference<Int> &rhs);

	RValue<UInt> operator=(unsigned int rhs);
	RValue<UInt> operator=(RValue<UInt> rhs);
	RValue<UInt> operator=(RValue<Int> rhs);
	RValue<UInt> operator=(const UInt &rhs);
	RValue<UInt> operator=(const Int &rhs);
	RValue<UInt> operator=(const Reference<UInt> &rhs);
	RValue<UInt> operator=(const Reference<Int> &rhs);

	static Type *type();
};

RValue<UInt> operator+(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator-(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator*(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator/(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator%(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator&(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator|(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator^(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator<<(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator>>(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<UInt> operator+=(UInt &lhs, RValue<UInt> rhs);
RValue<UInt> operator-=(UInt &lhs, RValue<UInt> rhs);
RValue<UInt> operator*=(UInt &lhs, RValue<UInt> rhs);
RValue<UInt> operator/=(UInt &lhs, RValue<UInt> rhs);
RValue<UInt> operator%=(UInt &lhs, RValue<UInt> rhs);
RValue<UInt> operator&=(UInt &lhs, RValue<UInt> rhs);
RValue<UInt> operator|=(UInt &lhs, RValue<UInt> rhs);
RValue<UInt> operator^=(UInt &lhs, RValue<UInt> rhs);
RValue<UInt> operator<<=(UInt &lhs, RValue<UInt> rhs);
RValue<UInt> operator>>=(UInt &lhs, RValue<UInt> rhs);
RValue<UInt> operator+(RValue<UInt> val);
RValue<UInt> operator-(RValue<UInt> val);
RValue<UInt> operator~(RValue<UInt> val);
RValue<UInt> operator++(UInt &val, int);  // Post-increment
const UInt &operator++(UInt &val);        // Pre-increment
RValue<UInt> operator--(UInt &val, int);  // Post-decrement
const UInt &operator--(UInt &val);        // Pre-decrement
RValue<Bool> operator<(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<Bool> operator<=(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<Bool> operator>(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<Bool> operator>=(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<Bool> operator!=(RValue<UInt> lhs, RValue<UInt> rhs);
RValue<Bool> operator==(RValue<UInt> lhs, RValue<UInt> rhs);

RValue<UInt> Max(RValue<UInt> x, RValue<UInt> y);
RValue<UInt> Min(RValue<UInt> x, RValue<UInt> y);
RValue<UInt> Clamp(RValue<UInt> x, RValue<UInt> min, RValue<UInt> max);

RValue<UInt> AddAtomic(RValue<Pointer<UInt>> x, RValue<UInt> y, std::memory_order memoryOrder);
RValue<UInt> SubAtomic(RValue<Pointer<UInt>> x, RValue<UInt> y, std::memory_order memoryOrder);
RValue<UInt> AndAtomic(RValue<Pointer<UInt>> x, RValue<UInt> y, std::memory_order memoryOrder);
RValue<UInt> OrAtomic(RValue<Pointer<UInt>> x, RValue<UInt> y, std::memory_order memoryOrder);
RValue<UInt> XorAtomic(RValue<Pointer<UInt>> x, RValue<UInt> y, std::memory_order memoryOrder);
RValue<Int> MinAtomic(RValue<Pointer<Int>> x, RValue<Int> y, std::memory_order memoryOrder);
RValue<Int> MaxAtomic(RValue<Pointer<Int>> x, RValue<Int> y, std::memory_order memoryOrder);
RValue<UInt> MinAtomic(RValue<Pointer<UInt>> x, RValue<UInt> y, std::memory_order memoryOrder);
RValue<UInt> MaxAtomic(RValue<Pointer<UInt>> x, RValue<UInt> y, std::memory_order memoryOrder);
RValue<UInt> ExchangeAtomic(RValue<Pointer<UInt>> x, RValue<UInt> y, std::memory_order memoryOrder);
RValue<UInt> CompareExchangeAtomic(RValue<Pointer<UInt>> x, RValue<UInt> y, RValue<UInt> compare, std::memory_order memoryOrderEqual, std::memory_order memoryOrderUnequal);

//	RValue<UInt> RoundUInt(RValue<Float> cast);

class Int2 : public LValue<Int2>
{
public:
	//	explicit Int2(RValue<Int> cast);
	explicit Int2(RValue<Int4> cast);

	Int2() = default;
	Int2(int x, int y);
	Int2(RValue<Int2> rhs);
	Int2(const Int2 &rhs);
	Int2(const Reference<Int2> &rhs);
	Int2(RValue<Int> lo, RValue<Int> hi);

	RValue<Int2> operator=(RValue<Int2> rhs);
	RValue<Int2> operator=(const Int2 &rhs);
	RValue<Int2> operator=(const Reference<Int2> &rhs);

	static Type *type();
	static int element_count() { return 2; }
};

RValue<Int2> operator+(RValue<Int2> lhs, RValue<Int2> rhs);
RValue<Int2> operator-(RValue<Int2> lhs, RValue<Int2> rhs);
//	RValue<Int2> operator*(RValue<Int2> lhs, RValue<Int2> rhs);
//	RValue<Int2> operator/(RValue<Int2> lhs, RValue<Int2> rhs);
//	RValue<Int2> operator%(RValue<Int2> lhs, RValue<Int2> rhs);
RValue<Int2> operator&(RValue<Int2> lhs, RValue<Int2> rhs);
RValue<Int2> operator|(RValue<Int2> lhs, RValue<Int2> rhs);
RValue<Int2> operator^(RValue<Int2> lhs, RValue<Int2> rhs);
RValue<Int2> operator<<(RValue<Int2> lhs, unsigned char rhs);
RValue<Int2> operator>>(RValue<Int2> lhs, unsigned char rhs);
RValue<Int2> operator+=(Int2 &lhs, RValue<Int2> rhs);
RValue<Int2> operator-=(Int2 &lhs, RValue<Int2> rhs);
//	RValue<Int2> operator*=(Int2 &lhs, RValue<Int2> rhs);
//	RValue<Int2> operator/=(Int2 &lhs, RValue<Int2> rhs);
//	RValue<Int2> operator%=(Int2 &lhs, RValue<Int2> rhs);
RValue<Int2> operator&=(Int2 &lhs, RValue<Int2> rhs);
RValue<Int2> operator|=(Int2 &lhs, RValue<Int2> rhs);
RValue<Int2> operator^=(Int2 &lhs, RValue<Int2> rhs);
RValue<Int2> operator<<=(Int2 &lhs, unsigned char rhs);
RValue<Int2> operator>>=(Int2 &lhs, unsigned char rhs);
//	RValue<Int2> operator+(RValue<Int2> val);
//	RValue<Int2> operator-(RValue<Int2> val);
RValue<Int2> operator~(RValue<Int2> val);
//	RValue<Int2> operator++(Int2 &val, int);   // Post-increment
//	const Int2 &operator++(Int2 &val);   // Pre-increment
//	RValue<Int2> operator--(Int2 &val, int);   // Post-decrement
//	const Int2 &operator--(Int2 &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<Int2> lhs, RValue<Int2> rhs);
//	RValue<Bool> operator<=(RValue<Int2> lhs, RValue<Int2> rhs);
//	RValue<Bool> operator>(RValue<Int2> lhs, RValue<Int2> rhs);
//	RValue<Bool> operator>=(RValue<Int2> lhs, RValue<Int2> rhs);
//	RValue<Bool> operator!=(RValue<Int2> lhs, RValue<Int2> rhs);
//	RValue<Bool> operator==(RValue<Int2> lhs, RValue<Int2> rhs);

//	RValue<Int2> RoundInt(RValue<Float4> cast);
RValue<Short4> UnpackLow(RValue<Int2> x, RValue<Int2> y);
RValue<Short4> UnpackHigh(RValue<Int2> x, RValue<Int2> y);
RValue<Int> Extract(RValue<Int2> val, int i);
RValue<Int2> Insert(RValue<Int2> val, RValue<Int> element, int i);

class UInt2 : public LValue<UInt2>
{
public:
	UInt2() = default;
	UInt2(unsigned int x, unsigned int y);
	UInt2(RValue<UInt2> rhs);
	UInt2(const UInt2 &rhs);
	UInt2(const Reference<UInt2> &rhs);

	RValue<UInt2> operator=(RValue<UInt2> rhs);
	RValue<UInt2> operator=(const UInt2 &rhs);
	RValue<UInt2> operator=(const Reference<UInt2> &rhs);

	static Type *type();
	static int element_count() { return 2; }
};

RValue<UInt2> operator+(RValue<UInt2> lhs, RValue<UInt2> rhs);
RValue<UInt2> operator-(RValue<UInt2> lhs, RValue<UInt2> rhs);
//	RValue<UInt2> operator*(RValue<UInt2> lhs, RValue<UInt2> rhs);
//	RValue<UInt2> operator/(RValue<UInt2> lhs, RValue<UInt2> rhs);
//	RValue<UInt2> operator%(RValue<UInt2> lhs, RValue<UInt2> rhs);
RValue<UInt2> operator&(RValue<UInt2> lhs, RValue<UInt2> rhs);
RValue<UInt2> operator|(RValue<UInt2> lhs, RValue<UInt2> rhs);
RValue<UInt2> operator^(RValue<UInt2> lhs, RValue<UInt2> rhs);
RValue<UInt2> operator<<(RValue<UInt2> lhs, unsigned char rhs);
RValue<UInt2> operator>>(RValue<UInt2> lhs, unsigned char rhs);
RValue<UInt2> operator+=(UInt2 &lhs, RValue<UInt2> rhs);
RValue<UInt2> operator-=(UInt2 &lhs, RValue<UInt2> rhs);
//	RValue<UInt2> operator*=(UInt2 &lhs, RValue<UInt2> rhs);
//	RValue<UInt2> operator/=(UInt2 &lhs, RValue<UInt2> rhs);
//	RValue<UInt2> operator%=(UInt2 &lhs, RValue<UInt2> rhs);
RValue<UInt2> operator&=(UInt2 &lhs, RValue<UInt2> rhs);
RValue<UInt2> operator|=(UInt2 &lhs, RValue<UInt2> rhs);
RValue<UInt2> operator^=(UInt2 &lhs, RValue<UInt2> rhs);
RValue<UInt2> operator<<=(UInt2 &lhs, unsigned char rhs);
RValue<UInt2> operator>>=(UInt2 &lhs, unsigned char rhs);
//	RValue<UInt2> operator+(RValue<UInt2> val);
//	RValue<UInt2> operator-(RValue<UInt2> val);
RValue<UInt2> operator~(RValue<UInt2> val);
//	RValue<UInt2> operator++(UInt2 &val, int);   // Post-increment
//	const UInt2 &operator++(UInt2 &val);   // Pre-increment
//	RValue<UInt2> operator--(UInt2 &val, int);   // Post-decrement
//	const UInt2 &operator--(UInt2 &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<UInt2> lhs, RValue<UInt2> rhs);
//	RValue<Bool> operator<=(RValue<UInt2> lhs, RValue<UInt2> rhs);
//	RValue<Bool> operator>(RValue<UInt2> lhs, RValue<UInt2> rhs);
//	RValue<Bool> operator>=(RValue<UInt2> lhs, RValue<UInt2> rhs);
//	RValue<Bool> operator!=(RValue<UInt2> lhs, RValue<UInt2> rhs);
//	RValue<Bool> operator==(RValue<UInt2> lhs, RValue<UInt2> rhs);

//	RValue<UInt2> RoundInt(RValue<Float4> cast);
RValue<UInt> Extract(RValue<UInt2> val, int i);
RValue<UInt2> Insert(RValue<UInt2> val, RValue<UInt> element, int i);

class Int4 : public LValue<Int4>, public XYZW<Int4>
{
public:
	explicit Int4(RValue<Byte4> cast);
	explicit Int4(RValue<SByte4> cast);
	explicit Int4(RValue<Float4> cast);
	explicit Int4(RValue<Short4> cast);
	explicit Int4(RValue<UShort4> cast);

	Int4();
	Int4(int xyzw);
	Int4(int x, int yzw);
	Int4(int x, int y, int zw);
	Int4(int x, int y, int z, int w);
	Int4(RValue<Int4> rhs);
	Int4(const Int4 &rhs);
	Int4(const Reference<Int4> &rhs);
	Int4(RValue<UInt4> rhs);
	Int4(const UInt4 &rhs);
	Int4(const Reference<UInt4> &rhs);
	Int4(RValue<Int2> lo, RValue<Int2> hi);
	Int4(RValue<Int> rhs);
	Int4(const Int &rhs);
	Int4(const Reference<Int> &rhs);

	template<int T>
	Int4(const SwizzleMask1<Int4, T> &rhs);

	RValue<Int4> operator=(int broadcast);
	RValue<Int4> operator=(RValue<Int4> rhs);
	RValue<Int4> operator=(const Int4 &rhs);
	RValue<Int4> operator=(const Reference<Int4> &rhs);

	static Type *type();
	static int element_count() { return 4; }

private:
	void constant(int x, int y, int z, int w);
};

RValue<Int4> operator+(RValue<Int4> lhs, RValue<Int4> rhs);
RValue<Int4> operator-(RValue<Int4> lhs, RValue<Int4> rhs);
RValue<Int4> operator*(RValue<Int4> lhs, RValue<Int4> rhs);
RValue<Int4> operator/(RValue<Int4> lhs, RValue<Int4> rhs);
RValue<Int4> operator%(RValue<Int4> lhs, RValue<Int4> rhs);
RValue<Int4> operator&(RValue<Int4> lhs, RValue<Int4> rhs);
RValue<Int4> operator|(RValue<Int4> lhs, RValue<Int4> rhs);
RValue<Int4> operator^(RValue<Int4> lhs, RValue<Int4> rhs);
RValue<Int4> operator<<(RValue<Int4> lhs, unsigned char rhs);
RValue<Int4> operator>>(RValue<Int4> lhs, unsigned char rhs);
RValue<Int4> operator<<(RValue<Int4> lhs, RValue<Int4> rhs);
RValue<Int4> operator>>(RValue<Int4> lhs, RValue<Int4> rhs);
RValue<Int4> operator+=(Int4 &lhs, RValue<Int4> rhs);
RValue<Int4> operator-=(Int4 &lhs, RValue<Int4> rhs);
RValue<Int4> operator*=(Int4 &lhs, RValue<Int4> rhs);
//	RValue<Int4> operator/=(Int4 &lhs, RValue<Int4> rhs);
//	RValue<Int4> operator%=(Int4 &lhs, RValue<Int4> rhs);
RValue<Int4> operator&=(Int4 &lhs, RValue<Int4> rhs);
RValue<Int4> operator|=(Int4 &lhs, RValue<Int4> rhs);
RValue<Int4> operator^=(Int4 &lhs, RValue<Int4> rhs);
RValue<Int4> operator<<=(Int4 &lhs, unsigned char rhs);
RValue<Int4> operator>>=(Int4 &lhs, unsigned char rhs);
RValue<Int4> operator+(RValue<Int4> val);
RValue<Int4> operator-(RValue<Int4> val);
RValue<Int4> operator~(RValue<Int4> val);
//	RValue<Int4> operator++(Int4 &val, int);   // Post-increment
//	const Int4 &operator++(Int4 &val);   // Pre-increment
//	RValue<Int4> operator--(Int4 &val, int);   // Post-decrement
//	const Int4 &operator--(Int4 &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<Int4> lhs, RValue<Int4> rhs);
//	RValue<Bool> operator<=(RValue<Int4> lhs, RValue<Int4> rhs);
//	RValue<Bool> operator>(RValue<Int4> lhs, RValue<Int4> rhs);
//	RValue<Bool> operator>=(RValue<Int4> lhs, RValue<Int4> rhs);
//	RValue<Bool> operator!=(RValue<Int4> lhs, RValue<Int4> rhs);
//	RValue<Bool> operator==(RValue<Int4> lhs, RValue<Int4> rhs);

RValue<Int4> CmpEQ(RValue<Int4> x, RValue<Int4> y);
RValue<Int4> CmpLT(RValue<Int4> x, RValue<Int4> y);
RValue<Int4> CmpLE(RValue<Int4> x, RValue<Int4> y);
RValue<Int4> CmpNEQ(RValue<Int4> x, RValue<Int4> y);
RValue<Int4> CmpNLT(RValue<Int4> x, RValue<Int4> y);
RValue<Int4> CmpNLE(RValue<Int4> x, RValue<Int4> y);
inline RValue<Int4> CmpGT(RValue<Int4> x, RValue<Int4> y)
{
	return CmpNLE(x, y);
}
inline RValue<Int4> CmpGE(RValue<Int4> x, RValue<Int4> y)
{
	return CmpNLT(x, y);
}
RValue<Int4> Abs(RValue<Int4> x);
RValue<Int4> Max(RValue<Int4> x, RValue<Int4> y);
RValue<Int4> Min(RValue<Int4> x, RValue<Int4> y);
// Convert to nearest integer. If a converted value is outside of the integer
// range, the returned result is undefined.
RValue<Int4> RoundInt(RValue<Float4> cast);
// Rounds to the nearest integer, but clamps very large values to an
// implementation-dependent range.
// Specifically, on x86, values larger than 2147483583.0 are converted to
// 2147483583 (0x7FFFFFBF) instead of producing 0x80000000.
RValue<Int4> RoundIntClamped(RValue<Float4> cast);
RValue<Short8> PackSigned(RValue<Int4> x, RValue<Int4> y);
RValue<UShort8> PackUnsigned(RValue<Int4> x, RValue<Int4> y);
RValue<Int> Extract(RValue<Int4> val, int i);
RValue<Int4> Insert(RValue<Int4> val, RValue<Int> element, int i);
RValue<Int> SignMask(RValue<Int4> x);
RValue<Int4> Swizzle(RValue<Int4> x, uint16_t select);
RValue<Int4> Shuffle(RValue<Int4> x, RValue<Int4> y, uint16_t select);
RValue<Int4> MulHigh(RValue<Int4> x, RValue<Int4> y);

class UInt4 : public LValue<UInt4>, public XYZW<UInt4>
{
public:
	explicit UInt4(RValue<Float4> cast);

	UInt4();
	UInt4(int xyzw);
	UInt4(int x, int yzw);
	UInt4(int x, int y, int zw);
	UInt4(int x, int y, int z, int w);
	UInt4(RValue<UInt4> rhs);
	UInt4(const UInt4 &rhs);
	UInt4(const Reference<UInt4> &rhs);
	UInt4(RValue<Int4> rhs);
	UInt4(const Int4 &rhs);
	UInt4(const Reference<Int4> &rhs);
	UInt4(RValue<UInt2> lo, RValue<UInt2> hi);
	UInt4(RValue<UInt> rhs);
	UInt4(const UInt &rhs);
	UInt4(const Reference<UInt> &rhs);

	RValue<UInt4> operator=(RValue<UInt4> rhs);
	RValue<UInt4> operator=(const UInt4 &rhs);
	RValue<UInt4> operator=(const Reference<UInt4> &rhs);

	static Type *type();
	static int element_count() { return 4; }

private:
	void constant(int x, int y, int z, int w);
};

RValue<UInt4> operator+(RValue<UInt4> lhs, RValue<UInt4> rhs);
RValue<UInt4> operator-(RValue<UInt4> lhs, RValue<UInt4> rhs);
RValue<UInt4> operator*(RValue<UInt4> lhs, RValue<UInt4> rhs);
RValue<UInt4> operator/(RValue<UInt4> lhs, RValue<UInt4> rhs);
RValue<UInt4> operator%(RValue<UInt4> lhs, RValue<UInt4> rhs);
RValue<UInt4> operator&(RValue<UInt4> lhs, RValue<UInt4> rhs);
RValue<UInt4> operator|(RValue<UInt4> lhs, RValue<UInt4> rhs);
RValue<UInt4> operator^(RValue<UInt4> lhs, RValue<UInt4> rhs);
RValue<UInt4> operator<<(RValue<UInt4> lhs, unsigned char rhs);
RValue<UInt4> operator>>(RValue<UInt4> lhs, unsigned char rhs);
RValue<UInt4> operator<<(RValue<UInt4> lhs, RValue<UInt4> rhs);
RValue<UInt4> operator>>(RValue<UInt4> lhs, RValue<UInt4> rhs);
RValue<UInt4> operator+=(UInt4 &lhs, RValue<UInt4> rhs);
RValue<UInt4> operator-=(UInt4 &lhs, RValue<UInt4> rhs);
RValue<UInt4> operator*=(UInt4 &lhs, RValue<UInt4> rhs);
//	RValue<UInt4> operator/=(UInt4 &lhs, RValue<UInt4> rhs);
//	RValue<UInt4> operator%=(UInt4 &lhs, RValue<UInt4> rhs);
RValue<UInt4> operator&=(UInt4 &lhs, RValue<UInt4> rhs);
RValue<UInt4> operator|=(UInt4 &lhs, RValue<UInt4> rhs);
RValue<UInt4> operator^=(UInt4 &lhs, RValue<UInt4> rhs);
RValue<UInt4> operator<<=(UInt4 &lhs, unsigned char rhs);
RValue<UInt4> operator>>=(UInt4 &lhs, unsigned char rhs);
RValue<UInt4> operator+(RValue<UInt4> val);
RValue<UInt4> operator-(RValue<UInt4> val);
RValue<UInt4> operator~(RValue<UInt4> val);
//	RValue<UInt4> operator++(UInt4 &val, int);   // Post-increment
//	const UInt4 &operator++(UInt4 &val);   // Pre-increment
//	RValue<UInt4> operator--(UInt4 &val, int);   // Post-decrement
//	const UInt4 &operator--(UInt4 &val);   // Pre-decrement
//	RValue<Bool> operator<(RValue<UInt4> lhs, RValue<UInt4> rhs);
//	RValue<Bool> operator<=(RValue<UInt4> lhs, RValue<UInt4> rhs);
//	RValue<Bool> operator>(RValue<UInt4> lhs, RValue<UInt4> rhs);
//	RValue<Bool> operator>=(RValue<UInt4> lhs, RValue<UInt4> rhs);
//	RValue<Bool> operator!=(RValue<UInt4> lhs, RValue<UInt4> rhs);
//	RValue<Bool> operator==(RValue<UInt4> lhs, RValue<UInt4> rhs);

RValue<UInt4> CmpEQ(RValue<UInt4> x, RValue<UInt4> y);
RValue<UInt4> CmpLT(RValue<UInt4> x, RValue<UInt4> y);
RValue<UInt4> CmpLE(RValue<UInt4> x, RValue<UInt4> y);
RValue<UInt4> CmpNEQ(RValue<UInt4> x, RValue<UInt4> y);
RValue<UInt4> CmpNLT(RValue<UInt4> x, RValue<UInt4> y);
RValue<UInt4> CmpNLE(RValue<UInt4> x, RValue<UInt4> y);
inline RValue<UInt4> CmpGT(RValue<UInt4> x, RValue<UInt4> y)
{
	return CmpNLE(x, y);
}
inline RValue<UInt4> CmpGE(RValue<UInt4> x, RValue<UInt4> y)
{
	return CmpNLT(x, y);
}
RValue<UInt4> Max(RValue<UInt4> x, RValue<UInt4> y);
RValue<UInt4> Min(RValue<UInt4> x, RValue<UInt4> y);
RValue<UInt4> MulHigh(RValue<UInt4> x, RValue<UInt4> y);
RValue<UInt> Extract(RValue<UInt4> val, int i);
RValue<UInt4> Insert(RValue<UInt4> val, RValue<UInt> element, int i);
//	RValue<UInt4> RoundInt(RValue<Float4> cast);
RValue<UInt4> Swizzle(RValue<UInt4> x, uint16_t select);
RValue<UInt4> Shuffle(RValue<UInt4> x, RValue<UInt4> y, uint16_t select);

class Half : public LValue<Half>
{
public:
	explicit Half(RValue<Float> cast);

	static Type *type();
};

class Float : public LValue<Float>
{
public:
	explicit Float(RValue<Int> cast);
	explicit Float(RValue<UInt> cast);
	explicit Float(RValue<Half> cast);

	Float() = default;
	Float(float x);
	Float(RValue<Float> rhs);
	Float(const Float &rhs);
	Float(const Reference<Float> &rhs);
	Float(Argument<Float> argument);

	template<int T>
	Float(const SwizzleMask1<Float4, T> &rhs);

	RValue<Float> operator=(float rhs);
	RValue<Float> operator=(RValue<Float> rhs);
	RValue<Float> operator=(const Float &rhs);
	RValue<Float> operator=(const Reference<Float> &rhs);

	template<int T>
	RValue<Float> operator=(const SwizzleMask1<Float4, T> &rhs);

	static Float infinity();

	static Type *type();
};

RValue<Float> operator+(RValue<Float> lhs, RValue<Float> rhs);
RValue<Float> operator-(RValue<Float> lhs, RValue<Float> rhs);
RValue<Float> operator*(RValue<Float> lhs, RValue<Float> rhs);
RValue<Float> operator/(RValue<Float> lhs, RValue<Float> rhs);
RValue<Float> operator+=(Float &lhs, RValue<Float> rhs);
RValue<Float> operator-=(Float &lhs, RValue<Float> rhs);
RValue<Float> operator*=(Float &lhs, RValue<Float> rhs);
RValue<Float> operator/=(Float &lhs, RValue<Float> rhs);
RValue<Float> operator+(RValue<Float> val);
RValue<Float> operator-(RValue<Float> val);
RValue<Bool> operator<(RValue<Float> lhs, RValue<Float> rhs);
RValue<Bool> operator<=(RValue<Float> lhs, RValue<Float> rhs);
RValue<Bool> operator>(RValue<Float> lhs, RValue<Float> rhs);
RValue<Bool> operator>=(RValue<Float> lhs, RValue<Float> rhs);
RValue<Bool> operator!=(RValue<Float> lhs, RValue<Float> rhs);
RValue<Bool> operator==(RValue<Float> lhs, RValue<Float> rhs);

RValue<Float> Abs(RValue<Float> x);
RValue<Float> Max(RValue<Float> x, RValue<Float> y);
RValue<Float> Min(RValue<Float> x, RValue<Float> y);
RValue<Float> Rcp(RValue<Float> x, bool relaxedPrecision, bool exactAtPow2 = false);
RValue<Float> RcpSqrt(RValue<Float> x, bool relaxedPrecision);
RValue<Float> Sqrt(RValue<Float> x);

//	RValue<Int4> IsInf(RValue<Float> x);
//	RValue<Int4> IsNan(RValue<Float> x);
RValue<Float> Round(RValue<Float> x);
RValue<Float> Trunc(RValue<Float> x);
RValue<Float> Frac(RValue<Float> x);
RValue<Float> Floor(RValue<Float> x);
RValue<Float> Ceil(RValue<Float> x);

// Trigonometric functions
// TODO: Currently unimplemented for Subzero.
//	RValue<Float> Sin(RValue<Float> x);
//	RValue<Float> Cos(RValue<Float> x);
//	RValue<Float> Tan(RValue<Float> x);
//	RValue<Float> Asin(RValue<Float> x);
//	RValue<Float> Acos(RValue<Float> x);
//	RValue<Float> Atan(RValue<Float> x);
//	RValue<Float> Sinh(RValue<Float> x);
//	RValue<Float> Cosh(RValue<Float> x);
//	RValue<Float> Tanh(RValue<Float> x);
//	RValue<Float> Asinh(RValue<Float> x);
//	RValue<Float> Acosh(RValue<Float> x);
//	RValue<Float> Atanh(RValue<Float> x);
//	RValue<Float> Atan2(RValue<Float> x, RValue<Float> y);

// Exponential functions
// TODO: Currently unimplemented for Subzero.
//	RValue<Float> Pow(RValue<Float> x, RValue<Float> y);
//	RValue<Float> Exp(RValue<Float> x);
//	RValue<Float> Log(RValue<Float> x);
RValue<Float> Exp2(RValue<Float> x);
RValue<Float> Log2(RValue<Float> x);

class Float2 : public LValue<Float2>
{
public:
	//	explicit Float2(RValue<Byte2> cast);
	//	explicit Float2(RValue<Short2> cast);
	//	explicit Float2(RValue<UShort2> cast);
	//	explicit Float2(RValue<Int2> cast);
	//	explicit Float2(RValue<UInt2> cast);
	explicit Float2(RValue<Float4> cast);

	Float2() = default;
	//	Float2(float x, float y);
	//	Float2(RValue<Float2> rhs);
	//	Float2(const Float2 &rhs);
	//	Float2(const Reference<Float2> &rhs);
	//	Float2(RValue<Float> rhs);
	//	Float2(const Float &rhs);
	//	Float2(const Reference<Float> &rhs);

	//	template<int T>
	//	Float2(const SwizzleMask1<T> &rhs);

	//	RValue<Float2> operator=(float broadcast);
	//	RValue<Float2> operator=(RValue<Float2> rhs);
	//	RValue<Float2> operator=(const Float2 &rhs);
	//	RValue<Float2> operator=(const Reference<Float2> &rhs);
	//	RValue<Float2> operator=(RValue<Float> rhs);
	//	RValue<Float2> operator=(const Float &rhs);
	//	RValue<Float2> operator=(const Reference<Float> &rhs);

	//	template<int T>
	//	RValue<Float2> operator=(const SwizzleMask1<T> &rhs);

	static Type *type();
	static int element_count() { return 2; }
};

//	RValue<Float2> operator+(RValue<Float2> lhs, RValue<Float2> rhs);
//	RValue<Float2> operator-(RValue<Float2> lhs, RValue<Float2> rhs);
//	RValue<Float2> operator*(RValue<Float2> lhs, RValue<Float2> rhs);
//	RValue<Float2> operator/(RValue<Float2> lhs, RValue<Float2> rhs);
//	RValue<Float2> operator%(RValue<Float2> lhs, RValue<Float2> rhs);
//	RValue<Float2> operator+=(Float2 &lhs, RValue<Float2> rhs);
//	RValue<Float2> operator-=(Float2 &lhs, RValue<Float2> rhs);
//	RValue<Float2> operator*=(Float2 &lhs, RValue<Float2> rhs);
//	RValue<Float2> operator/=(Float2 &lhs, RValue<Float2> rhs);
//	RValue<Float2> operator%=(Float2 &lhs, RValue<Float2> rhs);
//	RValue<Float2> operator+(RValue<Float2> val);
//	RValue<Float2> operator-(RValue<Float2> val);

//	RValue<Float2> Abs(RValue<Float2> x);
//	RValue<Float2> Max(RValue<Float2> x, RValue<Float2> y);
//	RValue<Float2> Min(RValue<Float2> x, RValue<Float2> y);
//	RValue<Float2> Swizzle(RValue<Float2> x, uint16_t select);
//	RValue<Float2> Mask(Float2 &lhs, RValue<Float2> rhs, uint16_t select);

class Float4 : public LValue<Float4>, public XYZW<Float4>
{
public:
	explicit Float4(RValue<Byte4> cast);
	explicit Float4(RValue<SByte4> cast);
	explicit Float4(RValue<Short4> cast);
	explicit Float4(RValue<UShort4> cast);
	explicit Float4(RValue<Int4> cast);
	explicit Float4(RValue<UInt4> cast);

	Float4();
	Float4(float xyzw);
	Float4(float x, float yzw);
	Float4(float x, float y, float zw);
	Float4(float x, float y, float z, float w);
	Float4(RValue<Float4> rhs);
	Float4(const Float4 &rhs);
	Float4(const Reference<Float4> &rhs);
	Float4(RValue<Float> rhs);
	Float4(const Float &rhs);
	Float4(const Reference<Float> &rhs);

	template<int T>
	Float4(const SwizzleMask1<Float4, T> &rhs);
	template<int T>
	Float4(const Swizzle4<Float4, T> &rhs);
	template<int X, int Y>
	Float4(const Swizzle2<Float4, X> &x, const Swizzle2<Float4, Y> &y);
	template<int X, int Y>
	Float4(const SwizzleMask2<Float4, X> &x, const Swizzle2<Float4, Y> &y);
	template<int X, int Y>
	Float4(const Swizzle2<Float4, X> &x, const SwizzleMask2<Float4, Y> &y);
	template<int X, int Y>
	Float4(const SwizzleMask2<Float4, X> &x, const SwizzleMask2<Float4, Y> &y);
	Float4(RValue<Float2> lo, RValue<Float2> hi);

	RValue<Float4> operator=(float broadcast);
	RValue<Float4> operator=(RValue<Float4> rhs);
	RValue<Float4> operator=(const Float4 &rhs);
	RValue<Float4> operator=(const Reference<Float4> &rhs);
	RValue<Float4> operator=(RValue<Float> rhs);
	RValue<Float4> operator=(const Float &rhs);
	RValue<Float4> operator=(const Reference<Float> &rhs);

	template<int T>
	RValue<Float4> operator=(const SwizzleMask1<Float4, T> &rhs);
	template<int T>
	RValue<Float4> operator=(const Swizzle4<Float4, T> &rhs);

	static Float4 infinity();

	static Type *type();
	static int element_count() { return 4; }

private:
	void constant(float x, float y, float z, float w);
};

RValue<Float4> operator+(RValue<Float4> lhs, RValue<Float4> rhs);
RValue<Float4> operator-(RValue<Float4> lhs, RValue<Float4> rhs);
RValue<Float4> operator*(RValue<Float4> lhs, RValue<Float4> rhs);
RValue<Float4> operator/(RValue<Float4> lhs, RValue<Float4> rhs);
RValue<Float4> operator%(RValue<Float4> lhs, RValue<Float4> rhs);
RValue<Float4> operator+=(Float4 &lhs, RValue<Float4> rhs);
RValue<Float4> operator-=(Float4 &lhs, RValue<Float4> rhs);
RValue<Float4> operator*=(Float4 &lhs, RValue<Float4> rhs);
RValue<Float4> operator/=(Float4 &lhs, RValue<Float4> rhs);
RValue<Float4> operator%=(Float4 &lhs, RValue<Float4> rhs);
RValue<Float4> operator+(RValue<Float4> val);
RValue<Float4> operator-(RValue<Float4> val);

// Computes `x * y + z`, which may be fused into one operation to produce a higher-precision result.
RValue<Float4> MulAdd(RValue<Float4> x, RValue<Float4> y, RValue<Float4> z);
// Computes a fused `x * y + z` operation. Caps::fmaIsFast indicates whether it emits an FMA instruction.
RValue<Float4> FMA(RValue<Float4> x, RValue<Float4> y, RValue<Float4> z);

RValue<Float4> Abs(RValue<Float4> x);
RValue<Float4> Max(RValue<Float4> x, RValue<Float4> y);
RValue<Float4> Min(RValue<Float4> x, RValue<Float4> y);

RValue<Float4> Rcp(RValue<Float4> x, bool relaxedPrecision, bool exactAtPow2 = false);
RValue<Float4> RcpSqrt(RValue<Float4> x, bool relaxedPrecision);
RValue<Float4> Sqrt(RValue<Float4> x);
RValue<Float4> Insert(RValue<Float4> val, RValue<Float> element, int i);
RValue<Float> Extract(RValue<Float4> x, int i);
RValue<Float4> Swizzle(RValue<Float4> x, uint16_t select);
RValue<Float4> Shuffle(RValue<Float4> x, RValue<Float4> y, uint16_t select);
RValue<Float4> ShuffleLowHigh(RValue<Float4> x, RValue<Float4> y, uint16_t imm);
RValue<Float4> UnpackLow(RValue<Float4> x, RValue<Float4> y);
RValue<Float4> UnpackHigh(RValue<Float4> x, RValue<Float4> y);
RValue<Float4> Mask(Float4 &lhs, RValue<Float4> rhs, uint16_t select);
RValue<Int> SignMask(RValue<Float4> x);

// Ordered comparison functions
RValue<Int4> CmpEQ(RValue<Float4> x, RValue<Float4> y);
RValue<Int4> CmpLT(RValue<Float4> x, RValue<Float4> y);
RValue<Int4> CmpLE(RValue<Float4> x, RValue<Float4> y);
RValue<Int4> CmpNEQ(RValue<Float4> x, RValue<Float4> y);
RValue<Int4> CmpNLT(RValue<Float4> x, RValue<Float4> y);
RValue<Int4> CmpNLE(RValue<Float4> x, RValue<Float4> y);
inline RValue<Int4> CmpGT(RValue<Float4> x, RValue<Float4> y)
{
	return CmpNLE(x, y);
}
inline RValue<Int4> CmpGE(RValue<Float4> x, RValue<Float4> y)
{
	return CmpNLT(x, y);
}

// Unordered comparison functions
RValue<Int4> CmpUEQ(RValue<Float4> x, RValue<Float4> y);
RValue<Int4> CmpULT(RValue<Float4> x, RValue<Float4> y);
RValue<Int4> CmpULE(RValue<Float4> x, RValue<Float4> y);
RValue<Int4> CmpUNEQ(RValue<Float4> x, RValue<Float4> y);
RValue<Int4> CmpUNLT(RValue<Float4> x, RValue<Float4> y);
RValue<Int4> CmpUNLE(RValue<Float4> x, RValue<Float4> y);
inline RValue<Int4> CmpUGT(RValue<Float4> x, RValue<Float4> y)
{
	return CmpUNLE(x, y);
}
inline RValue<Int4> CmpUGE(RValue<Float4> x, RValue<Float4> y)
{
	return CmpUNLT(x, y);
}

RValue<Int4> IsInf(RValue<Float4> x);
RValue<Int4> IsNan(RValue<Float4> x);
RValue<Float4> Round(RValue<Float4> x);
RValue<Float4> Trunc(RValue<Float4> x);
RValue<Float4> Frac(RValue<Float4> x);
RValue<Float4> Floor(RValue<Float4> x);
RValue<Float4> Ceil(RValue<Float4> x);
inline RValue<Float4> Mix(RValue<Float4> x, RValue<Float4> y, RValue<Float4> frac) {
	return (x * (Float4(1.0f) - frac)) + (y * frac);
}

// Trigonometric functions
RValue<Float4> Sin(RValue<Float4> x);
RValue<Float4> Cos(RValue<Float4> x);
RValue<Float4> Tan(RValue<Float4> x);
RValue<Float4> Asin(RValue<Float4> x);
RValue<Float4> Acos(RValue<Float4> x);
RValue<Float4> Atan(RValue<Float4> x);
RValue<Float4> Sinh(RValue<Float4> x);
RValue<Float4> Cosh(RValue<Float4> x);
RValue<Float4> Tanh(RValue<Float4> x);
RValue<Float4> Asinh(RValue<Float4> x);
RValue<Float4> Acosh(RValue<Float4> x);
RValue<Float4> Atanh(RValue<Float4> x);
RValue<Float4> Atan2(RValue<Float4> x, RValue<Float4> y);

// Exponential functions
RValue<Float4> Pow(RValue<Float4> x, RValue<Float4> y);
RValue<Float4> Exp(RValue<Float4> x);
RValue<Float4> Log(RValue<Float4> x);
RValue<Float4> Exp2(RValue<Float4> x);
RValue<Float4> Log2(RValue<Float4> x);

// Call a unary C function on each element of a vector type.
template<typename Func, typename T>
inline RValue<T> ScalarizeCall(Func func, const RValue<T> &x)
{
	T result;
	for(int i = 0; i < T::element_count(); i++)
	{
		result = Insert(result, Call(func, Extract(x, i)), i);
	}

	return result;
}

// Call a binary C function on each element of a vector type.
template<typename Func, typename T>
inline RValue<T> ScalarizeCall(Func func, const RValue<T> &x, const RValue<T> &y)
{
	T result;
	for(int i = 0; i < T::element_count(); i++)
	{
		result = Insert(result, Call(func, Extract(x, i), Extract(y, i)), i);
	}

	return result;
}

// Call a ternary C function on each element of a vector type.
template<typename Func, typename T>
inline RValue<T> ScalarizeCall(Func func, const RValue<T> &x, const RValue<T> &y, const RValue<T> &z)
{
	T result;
	for(int i = 0; i < T::element_count(); i++)
	{
		result = Insert(result, Call(func, Extract(x, i), Extract(y, i), Extract(z, i)), i);
	}

	return result;
}

// Invoke a unary lambda expression on each element of a vector type.
template<typename Func, typename T>
inline RValue<T> Scalarize(Func func, const RValue<T> &x)
{
	T result;
	for(int i = 0; i < T::element_count(); i++)
	{
		result = Insert(result, func(Extract(x, i)), i);
	}

	return result;
}

// Invoke a binary lambda expression on each element of a vector type.
template<typename Func, typename T>
inline RValue<T> Scalarize(Func func, const RValue<T> &x, const RValue<T> &y)
{
	T result;
	for(int i = 0; i < T::element_count(); i++)
	{
		result = Insert(result, func(Extract(x, i), Extract(y, i)), i);
	}

	return result;
}

// Invoke a ternary lambda expression on each element of a vector type.
template<typename Func, typename T>
inline RValue<T> Scalarize(Func func, const RValue<T> &x, const RValue<T> &y, const RValue<T> &z)
{
	T result;
	for(int i = 0; i < T::element_count(); i++)
	{
		result = Insert(result, func(Extract(x, i), Extract(y, i), Extract(z, i)), i);
	}

	return result;
}

// Bit Manipulation functions.
// TODO: Currently unimplemented for Subzero.

// Count leading zeros.
// Returns 32 when: !isZeroUndef && x == 0.
// Returns an undefined value when: isZeroUndef && x == 0.
RValue<UInt> Ctlz(RValue<UInt> x, bool isZeroUndef);
RValue<UInt4> Ctlz(RValue<UInt4> x, bool isZeroUndef);

// Count trailing zeros.
// Returns 32 when: !isZeroUndef && x == 0.
// Returns an undefined value when: isZeroUndef && x == 0.
RValue<UInt> Cttz(RValue<UInt> x, bool isZeroUndef);
RValue<UInt4> Cttz(RValue<UInt4> x, bool isZeroUndef);

template<class T>
class Pointer : public LValue<Pointer<T>>
{
public:
	template<class S>
	Pointer(RValue<Pointer<S>> pointerS, int alignment = 1)
	    : alignment(alignment)
	{
		Value *pointerT = Nucleus::createBitCast(pointerS.value(), Nucleus::getPointerType(T::type()));
		this->storeValue(pointerT);
	}

	template<class S>
	Pointer(const Pointer<S> &pointer, int alignment = 1)
	    : alignment(alignment)
	{
		Value *pointerS = pointer.loadValue();
		Value *pointerT = Nucleus::createBitCast(pointerS, Nucleus::getPointerType(T::type()));
		this->storeValue(pointerT);
	}

	Pointer(Argument<Pointer<T>> argument);

	Pointer();
	Pointer(RValue<Pointer<T>> rhs);
	Pointer(const Pointer<T> &rhs);
	Pointer(const Reference<Pointer<T>> &rhs);
	Pointer(std::nullptr_t);

	RValue<Pointer<T>> operator=(RValue<Pointer<T>> rhs);
	RValue<Pointer<T>> operator=(const Pointer<T> &rhs);
	RValue<Pointer<T>> operator=(const Reference<Pointer<T>> &rhs);
	RValue<Pointer<T>> operator=(std::nullptr_t);

	Reference<T> operator*() const;
	Reference<T> operator[](int index) const;
	Reference<T> operator[](unsigned int index) const;
	Reference<T> operator[](RValue<Int> index) const;
	Reference<T> operator[](RValue<UInt> index) const;

	static Type *type();

private:
	const int alignment;
};

RValue<Pointer<Byte>> operator+(RValue<Pointer<Byte>> lhs, int offset);
RValue<Pointer<Byte>> operator+(RValue<Pointer<Byte>> lhs, RValue<Int> offset);
RValue<Pointer<Byte>> operator+(RValue<Pointer<Byte>> lhs, RValue<UInt> offset);
RValue<Pointer<Byte>> operator+=(Pointer<Byte> &lhs, int offset);
RValue<Pointer<Byte>> operator+=(Pointer<Byte> &lhs, RValue<Int> offset);
RValue<Pointer<Byte>> operator+=(Pointer<Byte> &lhs, RValue<UInt> offset);

RValue<Pointer<Byte>> operator-(RValue<Pointer<Byte>> lhs, int offset);
RValue<Pointer<Byte>> operator-(RValue<Pointer<Byte>> lhs, RValue<Int> offset);
RValue<Pointer<Byte>> operator-(RValue<Pointer<Byte>> lhs, RValue<UInt> offset);
RValue<Pointer<Byte>> operator-=(Pointer<Byte> &lhs, int offset);
RValue<Pointer<Byte>> operator-=(Pointer<Byte> &lhs, RValue<Int> offset);
RValue<Pointer<Byte>> operator-=(Pointer<Byte> &lhs, RValue<UInt> offset);

template<typename T>
RValue<Bool> operator==(const Pointer<T> &lhs, const Pointer<T> &rhs)
{
	return RValue<Bool>(Nucleus::createICmpEQ(lhs.loadValue(), rhs.loadValue()));
}

template<typename T>
RValue<Bool> operator!=(const Pointer<T> &lhs, const Pointer<T> &rhs)
{
	return RValue<Bool>(Nucleus::createICmpNE(lhs.loadValue(), rhs.loadValue()));
}

template<typename T>
RValue<T> Load(RValue<Pointer<T>> pointer, unsigned int alignment, bool atomic, std::memory_order memoryOrder)
{
	return RValue<T>(Nucleus::createLoad(pointer.value(), T::type(), false, alignment, atomic, memoryOrder));
}

template<typename T>
RValue<T> Load(Pointer<T> pointer, unsigned int alignment, bool atomic, std::memory_order memoryOrder)
{
	return Load(RValue<Pointer<T>>(pointer), alignment, atomic, memoryOrder);
}

// TODO: Use SIMD to template these.
// TODO(b/155867273): These can be undeprecated if implemented for Subzero.
[[deprecated]] RValue<Float4> MaskedLoad(RValue<Pointer<Float4>> base, RValue<Int4> mask, unsigned int alignment, bool zeroMaskedLanes = false);
[[deprecated]] RValue<Int4> MaskedLoad(RValue<Pointer<Int4>> base, RValue<Int4> mask, unsigned int alignment, bool zeroMaskedLanes = false);
[[deprecated]] void MaskedStore(RValue<Pointer<Float4>> base, RValue<Float4> val, RValue<Int4> mask, unsigned int alignment);
[[deprecated]] void MaskedStore(RValue<Pointer<Int4>> base, RValue<Int4> val, RValue<Int4> mask, unsigned int alignment);

template<typename T>
void Store(RValue<T> value, RValue<Pointer<T>> pointer, unsigned int alignment, bool atomic, std::memory_order memoryOrder)
{
	Nucleus::createStore(value.value(), pointer.value(), T::type(), false, alignment, atomic, memoryOrder);
}

template<typename T>
void Store(RValue<T> value, Pointer<T> pointer, unsigned int alignment, bool atomic, std::memory_order memoryOrder)
{
	Store(value, RValue<Pointer<T>>(pointer), alignment, atomic, memoryOrder);
}

template<typename T>
void Store(T value, Pointer<T> pointer, unsigned int alignment, bool atomic, std::memory_order memoryOrder)
{
	Store(RValue<T>(value), RValue<Pointer<T>>(pointer), alignment, atomic, memoryOrder);
}

enum class OutOfBoundsBehavior
{
	Nullify,             // Loads become zero, stores are elided.
	RobustBufferAccess,  // As defined by the Vulkan spec (in short: access anywhere within bounds, or zeroing).
	UndefinedValue,      // Only for load operations. Not secure. No program termination.
	UndefinedBehavior,   // Program may terminate.
};

RValue<Bool> AnyTrue(const RValue<Int4> &bools);
RValue<Bool> AnyFalse(const RValue<Int4> &bools);
RValue<Bool> AllTrue(const RValue<Int4> &bools);
RValue<Bool> AllFalse(const RValue<Int4> &bools);

RValue<Bool> Divergent(const RValue<Int4> &ints);
RValue<Bool> Divergent(const RValue<Float4> &floats);
RValue<Bool> Uniform(const RValue<Int4> &ints);
RValue<Bool> Uniform(const RValue<Float4> &floats);

// Fence adds a memory barrier that enforces ordering constraints on memory
// operations. memoryOrder can only be one of:
// std::memory_order_acquire, std::memory_order_release,
// std::memory_order_acq_rel, or std::memory_order_seq_cst.
void Fence(std::memory_order memoryOrder);

template<class T, int S = 1>
class Array : public LValue<T>
{
public:
	Array(int size = S);

	Reference<T> operator[](int index);
	Reference<T> operator[](unsigned int index);
	Reference<T> operator[](RValue<Int> index);
	Reference<T> operator[](RValue<UInt> index);

	// self() returns the this pointer to this Array object.
	// This function exists because operator&() is overloaded by LValue<T>.
	inline Array *self() { return this; }
};

//	RValue<Array<T>> operator++(Array<T> &val, int);   // Post-increment
//	const Array<T> &operator++(Array<T> &val);   // Pre-increment
//	RValue<Array<T>> operator--(Array<T> &val, int);   // Post-decrement
//	const Array<T> &operator--(Array<T> &val);   // Pre-decrement

void branch(RValue<Bool> cmp, BasicBlock *bodyBB, BasicBlock *endBB);

// ValueOf returns a rr::Value* for the given C-type, RValue<T>, LValue<T>
// or Reference<T>.
template<typename T>
inline Value *ValueOf(const T &v)
{
	return ReactorType<T>::cast(v).loadValue();
}

void Return();

template<class T>
void Return(const T &ret)
{
	static_assert(CanBeUsedAsReturn<ReactorTypeT<T>>::value, "Unsupported type for Return()");
	Nucleus::createRet(ValueOf<T>(ret));
	// Place any unreachable instructions in an unreferenced block.
	Nucleus::setInsertBlock(Nucleus::createBasicBlock());
}

// Generic template, leave undefined!
template<typename FunctionType>
class Function;

// Specialized for function types
template<typename Return, typename... Arguments>
class Function<Return(Arguments...)>
{
	// Static assert that the function signature is valid.
	static_assert(sizeof(AssertFunctionSignatureIsValid<Return(Arguments...)>) >= 0, "Invalid function signature");

public:
	Function();

	template<int index>
	Argument<typename std::tuple_element<index, std::tuple<Arguments...>>::type> Arg() const
	{
		Value *arg = Nucleus::getArgument(index);
		return Argument<typename std::tuple_element<index, std::tuple<Arguments...>>::type>(arg);
	}

	std::shared_ptr<Routine> operator()(const char *name, ...);

protected:
	std::unique_ptr<Nucleus> core;
	std::vector<Type *> arguments;
};

template<typename Return>
class Function<Return()> : public Function<Return(Void)>
{
};

// FunctionT accepts a C-style function type template argument, allowing it to return a type-safe RoutineT wrapper
template<typename FunctionType>
class FunctionT;

template<typename Return, typename... Arguments>
class FunctionT<Return(Arguments...)> : public Function<CToReactorT<Return>(CToReactorT<Arguments>...)>
{
public:
	// Type of base class
	using BaseType = Function<CToReactorT<Return>(CToReactorT<Arguments>...)>;

	// Function type, e.g. void(int,float)
	using CFunctionType = Return(Arguments...);

	// Reactor function type, e.g. Void(Int, Float)
	using ReactorFunctionType = CToReactorT<Return>(CToReactorT<Arguments>...);

	// Returned RoutineT type
	using RoutineType = RoutineT<CFunctionType>;

	// Hide base implementations of operator()

	template<typename... VarArgs>
	RoutineType operator()(const char *name, VarArgs... varArgs)
	{
		return RoutineType(BaseType::operator()(name, std::forward<VarArgs>(varArgs)...));
	}
};

RValue<Long> Ticks();

}  // namespace rr

/* Inline implementations */

namespace rr {

template<class T>
LValue<T>::LValue(int arraySize)
    : Variable(T::type(), arraySize)
{
#ifdef ENABLE_RR_DEBUG_INFO
	materialize();
#endif  // ENABLE_RR_DEBUG_INFO
}

template<class T>
RValue<Pointer<T>> LValue<T>::operator&()
{
	return RValue<Pointer<T>>(this->getBaseAddress());
}

template<class T>
Reference<T>::Reference(Value *pointer, int alignment)
    : address(pointer)
    , alignment(alignment)
{
}

template<class T>
RValue<T> Reference<T>::operator=(RValue<T> rhs) const
{
	Nucleus::createStore(rhs.value(), address, T::type(), false, alignment);

	return rhs;
}

template<class T>
RValue<T> Reference<T>::operator=(const Reference<T> &ref) const
{
	Value *tmp = Nucleus::createLoad(ref.address, T::type(), false, ref.alignment);
	Nucleus::createStore(tmp, address, T::type(), false, alignment);

	return RValue<T>(tmp);
}

template<class T>
RValue<T> Reference<T>::operator+=(RValue<T> rhs) const
{
	return *this = *this + rhs;
}

template<class T>
Value *Reference<T>::loadValue() const
{
	return Nucleus::createLoad(address, T::type(), false, alignment);
}

template<class T>
RValue<T> Reference<T>::load() const
{
	return RValue<T>(loadValue());
}

template<class T>
int Reference<T>::getAlignment() const
{
	return alignment;
}

template<class T>
RValue<T>::RValue(const RValue<T> &rvalue)
    : val(rvalue.val)
{
	RR_DEBUG_INFO_EMIT_VAR(val);
}

template<class T>
RValue<T>::RValue(Value *value)
    : val(value)
{
	assert(Nucleus::createBitCast(value, T::type()) == value);  // Run-time type should match T, so bitcast is no-op.
	RR_DEBUG_INFO_EMIT_VAR(val);
}

template<class T>
RValue<T>::RValue(const T &lvalue)
    : val(lvalue.loadValue())
{
	RR_DEBUG_INFO_EMIT_VAR(val);
}

template<>
inline RValue<Bool>::RValue(bool b)
    : val(Nucleus::createConstantBool(b))
{
	RR_DEBUG_INFO_EMIT_VAR(val);
}

template<class T>
RValue<T>::RValue(typename IntLiteral<T>::Type i)
    : val(Nucleus::createConstantInt(i))
{
	RR_DEBUG_INFO_EMIT_VAR(val);
}

template<>
inline RValue<Long>::RValue(int64_t i)
    : val(Nucleus::createConstantLong(i))
{
	RR_DEBUG_INFO_EMIT_VAR(val);
}

template<>
inline RValue<Float>::RValue(float f)
    : val(Nucleus::createConstantFloat(f))
{
	RR_DEBUG_INFO_EMIT_VAR(val);
}

inline Value *broadcast(int i, Type *type)
{
	std::vector<int64_t> constantVector = { i };
	return Nucleus::createConstantVector(constantVector, type);
}

template<>
inline RValue<Int4>::RValue(int i)
    : val(broadcast(i, Int4::type()))
{
	RR_DEBUG_INFO_EMIT_VAR(val);
}

template<>
inline RValue<UInt4>::RValue(unsigned int i)
    : val(broadcast(int(i), UInt4::type()))
{
	RR_DEBUG_INFO_EMIT_VAR(val);
}

inline Value *broadcast(float f, Type *type)
{
	// See Float(float) constructor for the rationale behind this assert.
	assert(std::isfinite(f));

	std::vector<double> constantVector = { f };
	return Nucleus::createConstantVector(constantVector, type);
}

template<>
inline RValue<Float4>::RValue(float f)
    : val(broadcast(f, Float4::type()))
{
	RR_DEBUG_INFO_EMIT_VAR(val);
}

template<class T>
RValue<T>::RValue(const Reference<T> &ref)
    : val(ref.loadValue())
{
	RR_DEBUG_INFO_EMIT_VAR(val);
}

template<class Vector4, int T>
Swizzle2<Vector4, T>::operator RValue<Vector4>() const
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *vector = parent->loadValue();

	return Swizzle(RValue<Vector4>(vector), T);
}

template<class Vector4, int T>
Swizzle4<Vector4, T>::operator RValue<Vector4>() const
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *vector = parent->loadValue();

	return Swizzle(RValue<Vector4>(vector), T);
}

template<class Vector4, int T>
SwizzleMask4<Vector4, T>::operator RValue<Vector4>() const
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *vector = parent->loadValue();

	return Swizzle(RValue<Vector4>(vector), T);
}

template<class Vector4, int T>
RValue<Vector4> SwizzleMask4<Vector4, T>::operator=(RValue<Vector4> rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return Mask(*parent, rhs, T);
}

template<class Vector4, int T>
RValue<Vector4> SwizzleMask4<Vector4, T>::operator=(RValue<typename Scalar<Vector4>::Type> rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return Mask(*parent, Vector4(rhs), T);
}

template<class Vector4, int T>
SwizzleMask1<Vector4, T>::operator RValue<typename Scalar<Vector4>::Type>() const  // FIXME: Call a non-template function
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return Extract(*parent, T & 0x3);
}

template<class Vector4, int T>
SwizzleMask1<Vector4, T>::operator RValue<Vector4>() const
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *vector = parent->loadValue();

	return Swizzle(RValue<Vector4>(vector), T);
}

template<class Vector4, int T>
RValue<Vector4> SwizzleMask1<Vector4, T>::operator=(float x)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return *parent = Insert(*parent, Float(x), T & 0x3);
}

template<class Vector4, int T>
RValue<Vector4> SwizzleMask1<Vector4, T>::operator=(RValue<Vector4> rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return Mask(*parent, Float4(rhs), T);
}

template<class Vector4, int T>
RValue<Vector4> SwizzleMask1<Vector4, T>::operator=(RValue<typename Scalar<Vector4>::Type> rhs)  // FIXME: Call a non-template function
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return *parent = Insert(*parent, rhs, T & 0x3);
}

template<class Vector4, int T>
SwizzleMask2<Vector4, T>::operator RValue<Vector4>() const
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *vector = parent->loadValue();

	return Swizzle(RValue<Float4>(vector), T);
}

template<class Vector4, int T>
RValue<Vector4> SwizzleMask2<Vector4, T>::operator=(RValue<Vector4> rhs)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return Mask(*parent, Float4(rhs), T);
}

template<int T>
Int::Int(const SwizzleMask1<Int4, T> &rhs)
{
	*this = rhs.operator RValue<Int>();
}

template<int T>
RValue<Int> Int::operator=(const SwizzleMask1<Int4, T> &rhs)
{
	return *this = rhs.operator RValue<Int>();
}

template<int T>
Float::Float(const SwizzleMask1<Float4, T> &rhs)
{
	*this = rhs.operator RValue<Float>();
}

template<int T>
RValue<Float> Float::operator=(const SwizzleMask1<Float4, T> &rhs)
{
	return *this = rhs.operator RValue<Float>();
}

template<int T>
Int4::Int4(const SwizzleMask1<Int4, T> &rhs)
    : XYZW(this)
{
	*this = rhs.operator RValue<Int4>();
}

template<int T>
Float4::Float4(const SwizzleMask1<Float4, T> &rhs)
    : XYZW(this)
{
	*this = rhs.operator RValue<Float4>();
}

template<int T>
Float4::Float4(const Swizzle4<Float4, T> &rhs)
    : XYZW(this)
{
	*this = rhs.operator RValue<Float4>();
}

template<int X, int Y>
Float4::Float4(const Swizzle2<Float4, X> &x, const Swizzle2<Float4, Y> &y)
    : XYZW(this)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	*this = ShuffleLowHigh(*x.parent, *y.parent, (uint16_t(X) & 0xFF00u) | (uint16_t(Y >> 8) & 0x00FFu));
}

template<int X, int Y>
Float4::Float4(const SwizzleMask2<Float4, X> &x, const Swizzle2<Float4, Y> &y)
    : XYZW(this)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	*this = ShuffleLowHigh(*x.parent, *y.parent, (uint16_t(X) & 0xFF00u) | (uint16_t(Y >> 8) & 0x00FFu));
}

template<int X, int Y>
Float4::Float4(const Swizzle2<Float4, X> &x, const SwizzleMask2<Float4, Y> &y)
    : XYZW(this)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	*this = ShuffleLowHigh(*x.parent, *y.parent, (uint16_t(X) & 0xFF00u) | (uint16_t(Y >> 8) & 0x00FFu));
}

template<int X, int Y>
Float4::Float4(const SwizzleMask2<Float4, X> &x, const SwizzleMask2<Float4, Y> &y)
    : XYZW(this)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	*this = ShuffleLowHigh(*x.parent, *y.parent, (uint16_t(X) & 0xFF00u) | (uint16_t(Y >> 8) & 0x00FFu));
}

template<int T>
RValue<Float4> Float4::operator=(const SwizzleMask1<Float4, T> &rhs)
{
	return *this = rhs.operator RValue<Float4>();
}

template<int T>
RValue<Float4> Float4::operator=(const Swizzle4<Float4, T> &rhs)
{
	return *this = rhs.operator RValue<Float4>();
}

// Returns a reactor pointer to the fixed-address ptr.
RValue<Pointer<Byte>> ConstantPointer(const void *ptr);

// Returns a reactor pointer to an immutable copy of the data of size bytes.
RValue<Pointer<Byte>> ConstantData(const void *data, size_t size);

template<class T>
Pointer<T>::Pointer(Argument<Pointer<T>> argument)
    : alignment(1)
{
	this->store(argument.rvalue());
}

template<class T>
Pointer<T>::Pointer()
    : alignment(1)
{}

template<class T>
Pointer<T>::Pointer(RValue<Pointer<T>> rhs)
    : alignment(1)
{
	this->store(rhs);
}

template<class T>
Pointer<T>::Pointer(const Pointer<T> &rhs)
    : alignment(rhs.alignment)
{
	this->store(rhs.load());
}

template<class T>
Pointer<T>::Pointer(const Reference<Pointer<T>> &rhs)
    : alignment(rhs.getAlignment())
{
	this->store(rhs.load());
}

template<class T>
Pointer<T>::Pointer(std::nullptr_t)
    : alignment(1)
{
	Value *value = Nucleus::createNullPointer(T::type());
	this->storeValue(value);
}

template<class T>
RValue<Pointer<T>> Pointer<T>::operator=(RValue<Pointer<T>> rhs)
{
	return this->store(rhs);
}

template<class T>
RValue<Pointer<T>> Pointer<T>::operator=(const Pointer<T> &rhs)
{
	return this->store(rhs.load());
}

template<class T>
RValue<Pointer<T>> Pointer<T>::operator=(const Reference<Pointer<T>> &rhs)
{
	return this->store(rhs.load());
}

template<class T>
RValue<Pointer<T>> Pointer<T>::operator=(std::nullptr_t)
{
	Value *value = Nucleus::createNullPointer(T::type());
	this->storeValue(value);

	return RValue<Pointer<T>>(this);
}

template<class T>
Reference<T> Pointer<T>::operator*() const
{
	return Reference<T>(this->loadValue(), alignment);
}

template<class T>
Reference<T> Pointer<T>::operator[](int index) const
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *element = Nucleus::createGEP(this->loadValue(), T::type(), Nucleus::createConstantInt(index), false);

	return Reference<T>(element, alignment);
}

template<class T>
Reference<T> Pointer<T>::operator[](unsigned int index) const
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *element = Nucleus::createGEP(this->loadValue(), T::type(), Nucleus::createConstantInt(index), true);

	return Reference<T>(element, alignment);
}

template<class T>
Reference<T> Pointer<T>::operator[](RValue<Int> index) const
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *element = Nucleus::createGEP(this->loadValue(), T::type(), index.value(), false);

	return Reference<T>(element, alignment);
}

template<class T>
Reference<T> Pointer<T>::operator[](RValue<UInt> index) const
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *element = Nucleus::createGEP(this->loadValue(), T::type(), index.value(), true);

	return Reference<T>(element, alignment);
}

template<class T>
Type *Pointer<T>::type()
{
	return Nucleus::getPointerType(T::type());
}

template<class T, int S>
Array<T, S>::Array(int size)
    : LValue<T>(size)
{
}

template<class T, int S>
Reference<T> Array<T, S>::operator[](int index)
{
	assert(index < Variable::getArraySize());
	Value *element = this->getElementPointer(Nucleus::createConstantInt(index), false);

	return Reference<T>(element);
}

template<class T, int S>
Reference<T> Array<T, S>::operator[](unsigned int index)
{
	assert(index < static_cast<unsigned int>(Variable::getArraySize()));
	Value *element = this->getElementPointer(Nucleus::createConstantInt(index), true);

	return Reference<T>(element);
}

template<class T, int S>
Reference<T> Array<T, S>::operator[](RValue<Int> index)
{
	Value *element = this->getElementPointer(index.value(), false);

	return Reference<T>(element);
}

template<class T, int S>
Reference<T> Array<T, S>::operator[](RValue<UInt> index)
{
	Value *element = this->getElementPointer(index.value(), true);

	return Reference<T>(element);
}

//	template<class T>
//	RValue<Array<T>> operator++(Array<T> &val, int)
//	{
//		// FIXME: Requires storing the address of the array
//	}

//	template<class T>
//	const Array<T> &operator++(Array<T> &val)
//	{
//		// FIXME: Requires storing the address of the array
//	}

//	template<class T>
//	RValue<Array<T>> operator--(Array<T> &val, int)
//	{
//		// FIXME: Requires storing the address of the array
//	}

//	template<class T>
//	const Array<T> &operator--(Array<T> &val)
//	{
//		// FIXME: Requires storing the address of the array
//	}

template<class T>
RValue<T> IfThenElse(RValue<Bool> condition, RValue<T> ifTrue, RValue<T> ifFalse)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<T>(Nucleus::createSelect(condition.value(), ifTrue.value(), ifFalse.value()));
}

template<class T>
RValue<T> IfThenElse(RValue<Bool> condition, const T &ifTrue, RValue<T> ifFalse)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *trueValue = ifTrue.loadValue();

	return RValue<T>(Nucleus::createSelect(condition.value(), trueValue, ifFalse.value()));
}

template<class T>
RValue<T> IfThenElse(RValue<Bool> condition, RValue<T> ifTrue, const T &ifFalse)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *falseValue = ifFalse.loadValue();

	return RValue<T>(Nucleus::createSelect(condition.value(), ifTrue.value(), falseValue));
}

template<class T>
RValue<T> IfThenElse(RValue<Bool> condition, const T &ifTrue, const T &ifFalse)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *trueValue = ifTrue.loadValue();
	Value *falseValue = ifFalse.loadValue();

	return RValue<T>(Nucleus::createSelect(condition.value(), trueValue, falseValue));
}

template<typename Return, typename... Arguments>
Function<Return(Arguments...)>::Function()
    : core(new Nucleus())
{
	Type *types[] = { Arguments::type()... };
	for(Type *type : types)
	{
		if(type != Void::type())
		{
			arguments.push_back(type);
		}
	}

	Nucleus::createFunction(Return::type(), arguments);
}

template<typename Return, typename... Arguments>
std::shared_ptr<Routine> Function<Return(Arguments...)>::operator()(const char *name, ...)
{
	char fullName[1024 + 1];

	va_list vararg;
	va_start(vararg, name);
	vsnprintf(fullName, 1024, name, vararg);
	va_end(vararg);

	auto routine = core->acquireRoutine(fullName);
	core.reset(nullptr);

	return routine;
}

template<class T, class S>
RValue<T> ReinterpretCast(RValue<S> val)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<T>(Nucleus::createBitCast(val.value(), T::type()));
}

template<class T, class S>
RValue<T> ReinterpretCast(const LValue<S> &var)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	Value *val = var.loadValue();

	return RValue<T>(Nucleus::createBitCast(val, T::type()));
}

template<class T, class S>
RValue<T> ReinterpretCast(const Reference<S> &var)
{
	return ReinterpretCast<T>(RValue<S>(var));
}

template<class T>
RValue<T> As(Value *val)
{
	RR_DEBUG_INFO_UPDATE_LOC();
	return RValue<T>(Nucleus::createBitCast(val, T::type()));
}

template<class T, class S>
RValue<T> As(RValue<S> val)
{
	return ReinterpretCast<T>(val);
}

template<class T, class S>
RValue<T> As(const LValue<S> &var)
{
	return ReinterpretCast<T>(var);
}

template<class T, class S>
RValue<T> As(const Reference<S> &val)
{
	return ReinterpretCast<T>(val);
}

// Calls the function pointer fptr with the given arguments, return type
// and parameter types. Returns the call's return value if the function has
// a non-void return type.
Value *Call(RValue<Pointer<Byte>> fptr, Type *retTy, std::initializer_list<Value *> args, std::initializer_list<Type *> paramTys);

template<typename F>
class CallHelper
{};

template<typename Return, typename... Arguments>
class CallHelper<Return(Arguments...)>
{
public:
	using RReturn = CToReactorT<Return>;

	static inline RReturn Call(Return(fptr)(Arguments...), CToReactorT<Arguments>... args)
	{
		return RValue<RReturn>(rr::Call(
		    ConstantPointer(reinterpret_cast<void *>(fptr)),
		    RReturn::type(),
		    { ValueOf(args)... },
		    { CToReactorT<Arguments>::type()... }));
	}

	static inline RReturn Call(Pointer<Byte> fptr, CToReactorT<Arguments>... args)
	{
		return RValue<RReturn>(rr::Call(
		    fptr,
		    RReturn::type(),
		    { ValueOf(args)... },
		    { CToReactorT<Arguments>::type()... }));
	}
};

template<typename... Arguments>
class CallHelper<void(Arguments...)>
{
public:
	static inline void Call(void(fptr)(Arguments...), CToReactorT<Arguments>... args)
	{
		rr::Call(ConstantPointer(reinterpret_cast<void *>(fptr)),
		         Void::type(),
		         { ValueOf(args)... },
		         { CToReactorT<Arguments>::type()... });
	}

	static inline void Call(Pointer<Byte> fptr, CToReactorT<Arguments>... args)
	{
		rr::Call(fptr,
		         Void::type(),
		         { ValueOf(args)... },
		         { CToReactorT<Arguments>::type()... });
	}
};

template<typename T>
inline ReactorTypeT<T> CastToReactor(const T &v)
{
	return ReactorType<T>::cast(v);
}

// Calls the static function pointer fptr with the given arguments args.
template<typename Return, typename... CArgs, typename... RArgs>
inline CToReactorT<Return> Call(Return(fptr)(CArgs...), RArgs &&...args)
{
	return CallHelper<Return(CArgs...)>::Call(fptr, CastToReactor(std::forward<RArgs>(args))...);
}

// Calls the static function pointer fptr with the given arguments args.
// Overload for calling functions with void return type.
template<typename... CArgs, typename... RArgs>
inline void Call(void(fptr)(CArgs...), RArgs &&...args)
{
	CallHelper<void(CArgs...)>::Call(fptr, CastToReactor(std::forward<RArgs>(args))...);
}

// Calls the member function pointer fptr with the given arguments args.
// object can be a Class*, or a Pointer<Byte>.
template<typename Return, typename Class, typename C, typename... CArgs, typename... RArgs>
inline CToReactorT<Return> Call(Return (Class::*fptr)(CArgs...), C &&object, RArgs &&...args)
{
	using Helper = CallHelper<Return(Class *, void *, CArgs...)>;
	using fptrTy = decltype(fptr);

	struct Static
	{
		static inline Return Call(Class *object, void *fptrptr, CArgs... args)
		{
			auto fptr = *reinterpret_cast<fptrTy *>(fptrptr);
			return (object->*fptr)(std::forward<CArgs>(args)...);
		}
	};

	return Helper::Call(&Static::Call,
	                    CastToReactor(object),
	                    ConstantData(&fptr, sizeof(fptr)),
	                    CastToReactor(std::forward<RArgs>(args))...);
}

// Calls the member function pointer fptr with the given arguments args.
// Overload for calling functions with void return type.
// object can be a Class*, or a Pointer<Byte>.
template<typename Class, typename C, typename... CArgs, typename... RArgs>
inline void Call(void (Class::*fptr)(CArgs...), C &&object, RArgs &&...args)
{
	using Helper = CallHelper<void(Class *, void *, CArgs...)>;
	using fptrTy = decltype(fptr);

	struct Static
	{
		static inline void Call(Class *object, void *fptrptr, CArgs... args)
		{
			auto fptr = *reinterpret_cast<fptrTy *>(fptrptr);
			(object->*fptr)(std::forward<CArgs>(args)...);
		}
	};

	Helper::Call(&Static::Call,
	             CastToReactor(object),
	             ConstantData(&fptr, sizeof(fptr)),
	             CastToReactor(std::forward<RArgs>(args))...);
}

// NonVoidFunction<F> and VoidFunction<F> are helper classes which define ReturnType
// when F matches a non-void fuction signature or void function signature, respectively,
// as the function's return type.
template<typename F>
struct NonVoidFunction
{};

template<typename Return, typename... Arguments>
struct NonVoidFunction<Return(Arguments...)>
{
	using ReturnType = Return;
};

template<typename... Arguments>
struct NonVoidFunction<void(Arguments...)>
{
};

template<typename F>
using NonVoidFunctionReturnType = typename NonVoidFunction<F>::ReturnType;

template<typename F>
struct VoidFunction
{};

template<typename... Arguments>
struct VoidFunction<void(Arguments...)>
{
	using ReturnType = void;
};

template<typename F>
using VoidFunctionReturnType = typename VoidFunction<F>::ReturnType;

// Calls the Reactor function pointer fptr with the signature FUNCTION_SIGNATURE and arguments.
// Overload for calling functions with non-void return type.
template<typename FUNCTION_SIGNATURE, typename... RArgs>
inline CToReactorT<NonVoidFunctionReturnType<FUNCTION_SIGNATURE>> Call(Pointer<Byte> fptr, RArgs &&...args)
{
	return CallHelper<FUNCTION_SIGNATURE>::Call(fptr, CastToReactor(std::forward<RArgs>(args))...);
}

// Calls the Reactor function pointer fptr with the signature FUNCTION_SIGNATURE and arguments.
// Overload for calling functions with void return type.
template<typename FUNCTION_SIGNATURE, typename... RArgs>
inline VoidFunctionReturnType<FUNCTION_SIGNATURE> Call(Pointer<Byte> fptr, RArgs &&...args)
{
	CallHelper<FUNCTION_SIGNATURE>::Call(fptr, CastToReactor(std::forward<RArgs>(args))...);
}

// Breakpoint emits an instruction that will cause the application to trap.
// This can be used to stop an attached debugger at the given call.
void Breakpoint();

class ForData
{
public:
	ForData(bool init)
	    : loopOnce(init)
	{
	}

	operator bool()
	{
		return loopOnce;
	}

	bool operator=(bool value)
	{
		return loopOnce = value;
	}

	bool setup()
	{
		RR_DEBUG_INFO_FLUSH();
		if(Nucleus::getInsertBlock() != endBB)
		{
			testBB = Nucleus::createBasicBlock();

			Nucleus::createBr(testBB);
			Nucleus::setInsertBlock(testBB);

			return true;
		}

		return false;
	}

	bool test(RValue<Bool> cmp)
	{
		BasicBlock *bodyBB = Nucleus::createBasicBlock();
		endBB = Nucleus::createBasicBlock();

		Nucleus::createCondBr(cmp.value(), bodyBB, endBB);
		Nucleus::setInsertBlock(bodyBB);

		return true;
	}

	void end()
	{
		Nucleus::createBr(testBB);
		Nucleus::setInsertBlock(endBB);
	}

private:
	BasicBlock *testBB = nullptr;
	BasicBlock *endBB = nullptr;
	bool loopOnce = true;
};

class IfElseData
{
public:
	IfElseData(RValue<Bool> cmp)
	{
		trueBB = Nucleus::createBasicBlock();
		falseBB = Nucleus::createBasicBlock();
		endBB = falseBB;  // Until we encounter an Else statement, these are the same.

		Nucleus::createCondBr(cmp.value(), trueBB, falseBB);
		Nucleus::setInsertBlock(trueBB);
	}

	~IfElseData()
	{
		Nucleus::createBr(endBB);
		Nucleus::setInsertBlock(endBB);
	}

	operator int()
	{
		return iteration;
	}

	IfElseData &operator++()
	{
		++iteration;

		return *this;
	}

	void elseClause()
	{
		endBB = Nucleus::createBasicBlock();
		Nucleus::createBr(endBB);

		Nucleus::setInsertBlock(falseBB);
	}

private:
	BasicBlock *trueBB = nullptr;
	BasicBlock *falseBB = nullptr;
	BasicBlock *endBB = nullptr;
	int iteration = 0;
};

#define For(init, cond, inc)                        \
	for(ForData for__ = true; for__; for__ = false) \
		for(init; for__.setup() && for__.test(cond); inc, for__.end())

#define While(cond) For((void)0, cond, (void)0)

#define Do                                                \
	{                                                     \
		BasicBlock *body__ = Nucleus::createBasicBlock(); \
		Nucleus::createBr(body__);                        \
		Nucleus::setInsertBlock(body__);

#define Until(cond)                                       \
	BasicBlock *end__ = Nucleus::createBasicBlock();      \
	Nucleus::createCondBr((cond).value(), end__, body__); \
	Nucleus::setInsertBlock(end__);                       \
	}                                                     \
	do                                                    \
	{                                                     \
	} while(false)  // Require a semi-colon at the end of the Until()

enum
{
	IF_BLOCK__,
	ELSE_CLAUSE__,
	ELSE_BLOCK__,
	IFELSE_NUM__
};

#define If(cond)                                                          \
	for(IfElseData ifElse__{ cond }; ifElse__ < IFELSE_NUM__; ++ifElse__) \
		if(ifElse__ == IF_BLOCK__)

#define Else                           \
	else if(ifElse__ == ELSE_CLAUSE__) \
	{                                  \
		ifElse__.elseClause();         \
	}                                  \
	else  // ELSE_BLOCK__

// The OFFSET macro is a generalization of the offsetof() macro defined in <cstddef>.
// It allows e.g. getting the offset of array elements, even when indexed dynamically.
// We cast the address '32' and subtract it again, because null-dereference is undefined behavior.
#define OFFSET(s, m) ((int)(size_t) & reinterpret_cast<const volatile char &>((((s *)32)->m)) - 32)

}  // namespace rr

#include "Traits.inl"

#endif  // rr_Reactor_hpp
