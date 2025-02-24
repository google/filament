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

#ifndef sw_Math_hpp
#define sw_Math_hpp

#include "Debug.hpp"
#include "Types.hpp"

#include <cmath>
#if defined(_MSC_VER)
#	include <intrin.h>
#endif

namespace sw {

using std::abs;

#undef min
#undef max

template<class T>
inline T constexpr max(T a, T b)
{
	return a > b ? a : b;
}

template<class T>
inline constexpr T min(T a, T b)
{
	return a < b ? a : b;
}

template<class T>
inline constexpr T max(T a, T b, T c)
{
	return max(max(a, b), c);
}

template<class T>
inline constexpr T min(T a, T b, T c)
{
	return min(min(a, b), c);
}

template<class T>
inline constexpr T max(T a, T b, T c, T d)
{
	return max(max(a, b), max(c, d));
}

template<class T>
inline constexpr T min(T a, T b, T c, T d)
{
	return min(min(a, b), min(c, d));
}

template<typename destType, typename sourceType>
destType bit_cast(const sourceType &source)
{
	union
	{
		sourceType s;
		destType d;
	} sd;
	sd.s = source;
	return sd.d;
}

inline int iround(float x)
{
	return (int)floor(x + 0.5f);
	//	return _mm_cvtss_si32(_mm_load_ss(&x));   // FIXME: Demands SSE support
}

inline int ifloor(float x)
{
	return (int)floor(x);
}

inline int ceilFix4(int x)
{
	return (x + 0xF) & 0xFFFFFFF0;
}

inline int ceilInt4(int x)
{
	return (x + 0xF) >> 4;
}

#define BITS(x) (        \
	!!((x)&0x80000000) + \
	!!((x)&0xC0000000) + \
	!!((x)&0xE0000000) + \
	!!((x)&0xF0000000) + \
	!!((x)&0xF8000000) + \
	!!((x)&0xFC000000) + \
	!!((x)&0xFE000000) + \
	!!((x)&0xFF000000) + \
	!!((x)&0xFF800000) + \
	!!((x)&0xFFC00000) + \
	!!((x)&0xFFE00000) + \
	!!((x)&0xFFF00000) + \
	!!((x)&0xFFF80000) + \
	!!((x)&0xFFFC0000) + \
	!!((x)&0xFFFE0000) + \
	!!((x)&0xFFFF0000) + \
	!!((x)&0xFFFF8000) + \
	!!((x)&0xFFFFC000) + \
	!!((x)&0xFFFFE000) + \
	!!((x)&0xFFFFF000) + \
	!!((x)&0xFFFFF800) + \
	!!((x)&0xFFFFFC00) + \
	!!((x)&0xFFFFFE00) + \
	!!((x)&0xFFFFFF00) + \
	!!((x)&0xFFFFFF80) + \
	!!((x)&0xFFFFFFC0) + \
	!!((x)&0xFFFFFFE0) + \
	!!((x)&0xFFFFFFF0) + \
	!!((x)&0xFFFFFFF8) + \
	!!((x)&0xFFFFFFFC) + \
	!!((x)&0xFFFFFFFE) + \
	!!((x)&0xFFFFFFFF))

inline unsigned long log2i(int x)
{
#if defined(_MSC_VER)
	unsigned long y;
	_BitScanReverse(&y, x);
	return y;
#else
	return 31 - __builtin_clz(x);
#endif
}

inline bool isPow2(int x)
{
	return (x & -x) == x;
}

template<class T>
inline T clamp(T x, T a, T b)
{
	ASSERT(a <= b);
	if(x < a) x = a;
	if(x > b) x = b;

	return x;
}

inline float clamp01(float x)
{
	return clamp(x, 0.0f, 1.0f);
}

// Bit-cast of a floating-point value into a two's complement integer representation.
// This makes floating-point values comparable as integers.
inline int32_t float_as_twos_complement(float f)
{
	// IEEE-754 floating-point numbers are sorted by magnitude in the same way as integers,
	// except negative values are like one's complement integers. Convert them to two's complement.
	int32_t i = bit_cast<int32_t>(f);
	return (i < 0) ? (0x7FFFFFFFu - i) : i;
}

// 'Safe' clamping operation which always returns a value between min and max (inclusive).
inline float clamp_s(float x, float min, float max)
{
	// NaN values can't be compared directly
	if(float_as_twos_complement(x) < float_as_twos_complement(min)) x = min;
	if(float_as_twos_complement(x) > float_as_twos_complement(max)) x = max;

	return x;
}

inline int ceilPow2(int x)
{
	int i = 1;

	while(i < x)
	{
		i <<= 1;
	}

	return i;
}

inline int floorDiv(int a, int b)
{
	return a / b + ((a % b) >> 31);
}

inline int floorMod(int a, int b)
{
	int r = a % b;
	return r + ((r >> 31) & b);
}

inline int ceilDiv(int a, int b)
{
	return a / b - (-(a % b) >> 31);
}

inline int ceilMod(int a, int b)
{
	int r = a % b;
	return r - ((-r >> 31) & b);
}

template<const int n>
inline unsigned int unorm(float x)
{
	static const unsigned int max = 0xFFFFFFFF >> (32 - n);
	static const float maxf = static_cast<float>(max);

	if(x >= 1.0f)
	{
		return max;
	}
	else if(x <= 0.0f)
	{
		return 0;
	}
	else
	{
		return static_cast<unsigned int>(maxf * x + 0.5f);
	}
}

template<const int n>
inline int snorm(float x)
{
	static const unsigned int min = 0x80000000 >> (32 - n);
	static const unsigned int max = 0xFFFFFFFF >> (32 - n + 1);
	static const float maxf = static_cast<float>(max);
	static const unsigned int range = 0xFFFFFFFF >> (32 - n);

	if(x >= 0.0f)
	{
		if(x >= 1.0f)
		{
			return max;
		}
		else
		{
			return static_cast<int>(maxf * x + 0.5f);
		}
	}
	else
	{
		if(x <= -1.0f)
		{
			return min;
		}
		else
		{
			return static_cast<int>(maxf * x - 0.5f) & range;
		}
	}
}

template<const int n>
inline unsigned int ucast(float x)
{
	static const unsigned int max = 0xFFFFFFFF >> (32 - n);
	static const float maxf = static_cast<float>(max);

	if(x >= maxf)
	{
		return max;
	}
	else if(x <= 0.0f)
	{
		return 0;
	}
	else
	{
		return static_cast<unsigned int>(x + 0.5f);
	}
}

template<const int n>
inline int scast(float x)
{
	static const unsigned int min = 0x80000000 >> (32 - n);
	static const unsigned int max = 0xFFFFFFFF >> (32 - n + 1);
	static const float maxf = static_cast<float>(max);
	static const float minf = static_cast<float>(min);
	static const unsigned int range = 0xFFFFFFFF >> (32 - n);

	if(x > 0.0f)
	{
		if(x >= maxf)
		{
			return max;
		}
		else
		{
			return static_cast<int>(x + 0.5f);
		}
	}
	else
	{
		if(x <= -minf)
		{
			return min;
		}
		else
		{
			return static_cast<int>(x - 0.5f) & range;
		}
	}
}

inline float sRGBtoLinear(float c)
{
	if(c <= 0.04045f)
	{
		return c / 12.92f;
	}
	else
	{
		return powf((c + 0.055f) / 1.055f, 2.4f);
	}
}

inline float linearToSRGB(float c)
{
	if(c <= 0.0031308f)
	{
		return c * 12.92f;
	}
	else
	{
		return 1.055f * powf(c, 1.0f / 2.4f) - 0.055f;
	}
}

unsigned char sRGB8toLinear8(unsigned char value);

uint64_t FNV_1a(const unsigned char *data, int size);  // Fowler-Noll-Vo hash function

// Round up to the next multiple of alignment
template<typename T>
inline T align(T value, unsigned int alignment)
{
	return ((value + alignment - 1) / alignment) * alignment;
}

template<unsigned int alignment, typename T>
inline T align(T value)
{
	return ((value + alignment - 1) / alignment) * alignment;
}

inline int clampToSignedInt(unsigned int x)
{
	return static_cast<int>(min(x, 0x7FFFFFFFu));
}

// Convert floating value v to fixed point with p digits after the decimal point
inline constexpr int toFixedPoint(float v, int p)
{
	return static_cast<int>(v * (1 << p));
}

// Returns the next floating-point number which is not treated identical to the input.
// Note that std::nextafter() does not skip representations flushed to zero.
[[nodiscard]] inline float inc(float x)
{
	int x1 = bit_cast<int>(x);

	while(bit_cast<float>(x1) == x)
	{
		// Since IEEE 754 uses ones' complement and integers are two's complement,
		// we need to explicitly hop from negative zero to positive zero.
		if(x1 == (int)0x80000000)  // -0.0f
		{
			// Note that while the comparison -0.0f == +0.0f returns true, this
			// function returns the next value which can be treated differently.
			return +0.0f;
		}

		// Negative ones' complement value are made less negative by subtracting 1
		// in two's complement representation.
		x1 += (x1 >= 0) ? 1 : -1;
	}

	float y = bit_cast<float>(x1);

	// If we have a value which compares equal to 0.0, return 0.0. This ensures
	// subnormal values get flushed to zero when denormals-are-zero is enabled.
	return (y == 0.0f) ? +0.0f : y;
}

inline uint16_t compactEvenBits(uint32_t x)
{
	x &= 0x55555555;                  // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
	x = (x ^ (x >> 1)) & 0x33333333;  // x = --fe --dc --ba --98 --76 --54 --32 --10
	x = (x ^ (x >> 2)) & 0x0F0F0F0F;  // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
	x = (x ^ (x >> 4)) & 0x00FF00FF;  // x = ---- ---- fedc ba98 ---- ---- 7654 3210
	x = (x ^ (x >> 8)) & 0x0000FFFF;  // x = ---- ---- ---- ---- fedc ba98 7654 3210

	return static_cast<uint16_t>(x);
}

}  // namespace sw

#endif  // sw_Math_hpp
