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

#ifndef rr_Traits_hpp
#define rr_Traits_hpp

#include <stdint.h>
#include <type_traits>

namespace rr {

// Forward declarations
class Value;

class Void;
class Bool;
class Byte;
class SByte;
class Short;
class UShort;
class Int;
class UInt;
class Long;
class Half;
class Float;
class Float4;

template<class T>
class Pointer;
template<class T>
class LValue;
template<class T>
class RValue;

// IsDefined<T>::value is true if T is a valid type, otherwise false.
template<typename T, typename Enable = void>
struct IsDefined
{
	static constexpr bool value = false;
};

template<typename T>
struct IsDefined<T, std::enable_if_t<(sizeof(T) > 0)>>
{
	static constexpr bool value = true;
};

template<>
struct IsDefined<void>
{
	static constexpr bool value = true;
};

// CToReactorT<T> resolves to the corresponding Reactor type for the given C
// template type T.
template<typename T, typename ENABLE = void>
struct CToReactor;
template<typename T>
using CToReactorT = typename CToReactor<T>::type;

// CToReactor specializations for POD types.
template<>
struct CToReactor<void>
{
	using type = Void;
};
template<>
struct CToReactor<bool>
{
	using type = Bool;
	static Bool cast(bool);
};
template<>
struct CToReactor<uint8_t>
{
	using type = Byte;
	static Byte cast(uint8_t);
};
template<>
struct CToReactor<int8_t>
{
	using type = SByte;
	static SByte cast(int8_t);
};
template<>
struct CToReactor<int16_t>
{
	using type = Short;
	static Short cast(int16_t);
};
template<>
struct CToReactor<uint16_t>
{
	using type = UShort;
	static UShort cast(uint16_t);
};
template<>
struct CToReactor<int32_t>
{
	using type = Int;
	static Int cast(int32_t);
};
template<>
struct CToReactor<uint32_t>
{
	using type = UInt;
	static UInt cast(uint32_t);
};
template<>
struct CToReactor<float>
{
	using type = Float;
	static Float cast(float);
};
template<>
struct CToReactor<float[4]>
{
	using type = Float4;
	static Float4 cast(float[4]);
};

// TODO: Long has no constructor that takes a uint64_t
template<>
struct CToReactor<uint64_t>
{
	using type = Long; /* static Long   cast(uint64_t); */
};

// HasReactorType<T>::value resolves to true iff there exists a
// CToReactorT specialization for type T.
template<typename T>
using HasReactorType = IsDefined<CToReactorT<T>>;

// CToReactorPtr<T>::type resolves to the corresponding Reactor Pointer<>
// type for T*.
// For T types that have a CToReactorT<> specialization,
// CToReactorPtr<T>::type resolves to Pointer< CToReactorT<T> >, otherwise
// CToReactorPtr<T>::type resolves to Pointer<Byte>.
template<typename T, typename ENABLE = void>
struct CToReactorPtr
{
	using type = Pointer<Byte>;
	static inline type cast(const T *v);  // implemented in Traits.inl
};

// CToReactorPtr specialization for T types that have a CToReactorT<>
// specialization.
template<typename T>
struct CToReactorPtr<T, std::enable_if_t<HasReactorType<T>::value>>
{
	using type = Pointer<CToReactorT<T>>;
	static inline type cast(const T *v);  // implemented in Traits.inl
};

// CToReactorPtr specialization for void*.
// Maps to Pointer<Byte> instead of Pointer<Void>.
template<>
struct CToReactorPtr<void, void>
{
	using type = Pointer<Byte>;
	static inline type cast(const void *v);  // implemented in Traits.inl
};

// CToReactorPtr specialization for function pointer types.
// Maps to Pointer<Byte>.
// Drops the 'const' qualifier from the cast() method to avoid warnings
// about const having no meaning for function types.
template<typename T>
struct CToReactorPtr<T, std::enable_if_t<std::is_function<T>::value>>
{
	using type = Pointer<Byte>;
	static inline type cast(T *v);  // implemented in Traits.inl
};

template<typename T>
using CToReactorPtrT = typename CToReactorPtr<T>::type;

// CToReactor specialization for pointer types.
// For T types that have a CToReactorT<> specialization,
// CToReactorT<T*>::type resolves to Pointer< CToReactorT<T> >, otherwise
// CToReactorT<T*>::type resolves to Pointer<Byte>.
template<typename T>
struct CToReactor<T, std::enable_if_t<std::is_pointer<T>::value>>
{
	using elem = typename std::remove_pointer<T>::type;
	using type = CToReactorPtrT<elem>;
	static inline type cast(T v);  // implemented in Traits.inl
};

// CToReactor specialization for enum types.
template<typename T>
struct CToReactor<T, std::enable_if_t<std::is_enum<T>::value>>
{
	using underlying = typename std::underlying_type<T>::type;
	using type = CToReactorT<underlying>;
	static type cast(T v);  // implemented in Traits.inl
};

// IsRValue::value is true if T is of type RValue<X>, where X is any type.
template<typename T, typename Enable = void>
struct IsRValue
{
	static constexpr bool value = false;
};
template<typename T>
struct IsRValue<T, std::enable_if_t<IsDefined<typename T::rvalue_underlying_type>::value>>
{
	static constexpr bool value = true;
};

// IsLValue::value is true if T is of, or derives from type LValue<T>.
template<typename T>
struct IsLValue
{
	static constexpr bool value = std::is_base_of<LValue<T>, T>::value;
};

// IsReference::value is true if T is of type Reference<X>, where X is any type.
template<typename T, typename Enable = void>
struct IsReference
{
	static constexpr bool value = false;
};
template<typename T>
struct IsReference<T, std::enable_if_t<IsDefined<typename T::reference_underlying_type>::value>>
{
	static constexpr bool value = true;
};

// ReactorTypeT<T> returns the LValue Reactor type for T.
// T can be a C-type, RValue or LValue.
template<typename T, typename ENABLE = void>
struct ReactorType;
template<typename T>
using ReactorTypeT = typename ReactorType<T>::type;
template<typename T>
struct ReactorType<T, std::enable_if_t<IsDefined<CToReactorT<T>>::value>>
{
	using type = CToReactorT<T>;
	static type cast(T v) { return CToReactor<T>::cast(v); }
};
template<typename T>
struct ReactorType<T, std::enable_if_t<IsRValue<T>::value>>
{
	using type = typename T::rvalue_underlying_type;
	static type cast(T v) { return type(v); }
};
template<typename T>
struct ReactorType<T, std::enable_if_t<IsLValue<T>::value>>
{
	using type = T;
	static type cast(T v) { return type(v); }
};
template<typename T>
struct ReactorType<T, std::enable_if_t<IsReference<T>::value>>
{
	using type = T;
	static type cast(T v) { return type(v); }
};

// Reactor types that can be used as a return type for a function.
template<typename T>
struct CanBeUsedAsReturn
{
	static constexpr bool value = false;
};
template<>
struct CanBeUsedAsReturn<Void>
{
	static constexpr bool value = true;
};
template<>
struct CanBeUsedAsReturn<Int>
{
	static constexpr bool value = true;
};
template<>
struct CanBeUsedAsReturn<UInt>
{
	static constexpr bool value = true;
};
template<>
struct CanBeUsedAsReturn<Float>
{
	static constexpr bool value = true;
};
template<typename T>
struct CanBeUsedAsReturn<Pointer<T>>
{
	static constexpr bool value = true;
};

// Reactor types that can be used as a parameter types for a function.
template<typename T>
struct CanBeUsedAsParameter
{
	static constexpr bool value = false;
};
template<>
struct CanBeUsedAsParameter<Int>
{
	static constexpr bool value = true;
};
template<>
struct CanBeUsedAsParameter<UInt>
{
	static constexpr bool value = true;
};
template<>
struct CanBeUsedAsParameter<Float>
{
	static constexpr bool value = true;
};
template<typename T>
struct CanBeUsedAsParameter<Pointer<T>>
{
	static constexpr bool value = true;
};

// AssertParameterTypeIsValid statically asserts that all template parameter
// types can be used as a Reactor function parameter.
template<typename T, typename... other>
struct AssertParameterTypeIsValid : AssertParameterTypeIsValid<other...>
{
	static_assert(CanBeUsedAsParameter<T>::value, "Invalid parameter type");
};
template<typename T>
struct AssertParameterTypeIsValid<T>
{
	static_assert(CanBeUsedAsParameter<T>::value, "Invalid parameter type");
};

// AssertFunctionSignatureIsValid statically asserts that the Reactor
// function signature is valid.
template<typename Return, typename... Arguments>
class AssertFunctionSignatureIsValid;
template<typename Return>
class AssertFunctionSignatureIsValid<Return(Void)>
{};
template<typename Return, typename... Arguments>
class AssertFunctionSignatureIsValid<Return(Arguments...)>
{
	static_assert(CanBeUsedAsReturn<Return>::value, "Invalid return type");
	static_assert(sizeof(AssertParameterTypeIsValid<Arguments...>) >= 0, "");
};

}  // namespace rr

#endif  // rr_Traits_hpp
