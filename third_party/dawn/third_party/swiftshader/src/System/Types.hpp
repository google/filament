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

#ifndef sw_Types_hpp
#define sw_Types_hpp

#include <cassert>
#include <limits>
#include <type_traits>

// GCC warns against bitfields not fitting the entire range of an enum with a fixed underlying type of unsigned int, which gets promoted to an error with -Werror and cannot be suppressed.
// However, GCC already defaults to using unsigned int as the underlying type of an unscoped enum without a fixed underlying type. So we can just omit it.
#if defined(__GNUC__) && !defined(__clang__)
namespace {
enum E
{
};
static_assert(!std::numeric_limits<std::underlying_type<E>::type>::is_signed, "expected unscoped enum whose underlying type is not fixed to be unsigned");
}  // namespace
#	define ENUM_UNDERLYING_TYPE_UNSIGNED_INT
#else
#	define ENUM_UNDERLYING_TYPE_UNSIGNED_INT : unsigned int
#endif

#if defined(_MSC_VER)
typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;
typedef signed __int64 int64_t;
typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#	define ALIGN(bytes, type) __declspec(align(bytes)) type
#else
#	include <stdint.h>
#	define ALIGN(bytes, type) type __attribute__((aligned(bytes)))
#endif

namespace sw {

// assert_cast<> is like a static_cast<> which asserts that no information was lost.
template<typename To, typename From>
To assert_cast(From x)
{
	To y = static_cast<To>(x);
	assert(static_cast<From>(y) == x);

	return y;
}

// https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
constexpr inline uint32_t bit_ceil(uint32_t i)
{
	i--;
	i |= i >> 1;
	i |= i >> 2;
	i |= i >> 4;
	i |= i >> 8;
	i |= i >> 16;
	i++;
	return i;
}

typedef ALIGN(1, uint8_t) byte;
typedef ALIGN(2, uint16_t) word;
typedef ALIGN(4, uint32_t) dword;
typedef ALIGN(8, uint64_t) qword;
typedef ALIGN(1, int8_t) sbyte;

template<typename T, int N>
struct alignas(sizeof(T) * bit_ceil(N)) vec
{
	vec() = default;

	constexpr explicit vec(T replicate)
	{
		for(int i = 0; i < N; i++)
		{
			v[i] = replicate;
		}
	}

	template<typename... ARGS>
	constexpr vec(T arg0, ARGS... args)
	    : v{ arg0, args... }
	{
	}

	// Require explicit use of replicate constructor.
	vec &operator=(T) = delete;

	T &operator[](int i)
	{
		return v[i];
	}

	const T &operator[](int i) const
	{
		return v[i];
	}

	T v[N];
};

template<typename T>
struct alignas(sizeof(T) * 4) vec<T, 4>
{
	vec() = default;

	constexpr explicit vec(T replicate)
	    : x(replicate)
	    , y(replicate)
	    , z(replicate)
	    , w(replicate)
	{
	}

	constexpr vec(T x, T y, T z, T w)
	    : x(x)
	    , y(y)
	    , z(z)
	    , w(w)
	{
	}

	// Require explicit use of replicate constructor.
	vec &operator=(T) = delete;

	T &operator[](int i)
	{
		return v[i];
	}

	const T &operator[](int i) const
	{
		return v[i];
	}

	union
	{
		T v[4];

		struct
		{
			T x;
			T y;
			T z;
			T w;
		};
	};
};

template<typename T, int N>
bool operator==(const vec<T, N> &a, const vec<T, N> &b)
{
	for(int i = 0; i < N; i++)
	{
		if(a.v[i] != b.v[i])
		{
			return false;
		}
	}

	return true;
}

template<typename T, int N>
bool operator!=(const vec<T, N> &a, const vec<T, N> &b)
{
	return !(a == b);
}

template<typename T>
using vec2 = vec<T, 2>;
template<typename T>
using vec3 = vec<T, 3>;  // aligned to 4 elements
template<typename T>
using vec4 = vec<T, 4>;
template<typename T>
using vec8 = vec<T, 8>;
template<typename T>
using vec16 = vec<T, 16>;

using int2 = vec2<int>;
using uint2 = vec2<unsigned int>;
using float2 = vec2<float>;
using dword2 = vec2<dword>;
using qword2 = vec2<qword>;

// Note: These vec3<T> types all use 4-element alignment - i.e. they have
// identical memory layout to vec4<T>, except they do not have a 4th component.
using int3 = vec3<int>;
using uint3 = vec3<unsigned int>;
using float3 = vec3<float>;
using dword3 = vec3<dword>;

using int4 = vec4<int>;
using uint4 = vec4<unsigned int>;
using float4 = vec4<float>;
using byte4 = vec4<byte>;
using sbyte4 = vec4<sbyte>;
using short4 = vec4<short>;
using ushort4 = vec4<unsigned short>;
using word4 = vec4<word>;
using dword4 = vec4<dword>;

using byte8 = vec8<byte>;
using sbyte8 = vec8<sbyte>;
using short8 = vec8<short>;
using ushort8 = vec8<unsigned short>;

using byte16 = vec16<byte>;
using sbyte16 = vec16<sbyte>;

inline constexpr float4 vector(float x, float y, float z, float w)
{
	return float4{ x, y, z, w };
}

inline constexpr float4 replicate(float f)
{
	return vector(f, f, f, f);
}

}  // namespace sw

#endif  // sw_Types_hpp
