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

#ifndef sw_Half_hpp
#define sw_Half_hpp

#include "Math.hpp"

#include <algorithm>
#include <cmath>

namespace sw {

class half
{
public:
	half() = default;
	explicit half(float f);

	operator float() const;

	half &operator=(float f);

private:
	unsigned short fp16i;
};

inline half shortAsHalf(short s)
{
	union
	{
		half h;
		short s;
	} hs;

	hs.s = s;

	return hs.h;
}

class RGB9E5
{
	union
	{
		struct
		{
			unsigned int R : 9;
			unsigned int G : 9;
			unsigned int B : 9;
			unsigned int E : 5;
		};
		uint32_t packed;
	};

public:
	RGB9E5(const float rgb[3])
	    : RGB9E5(rgb[0], rgb[1], rgb[2])
	{
	}

	RGB9E5(float r, float g, float b)
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

		// Clamp components to valid range. NaN becomes 0.
		const float red_c = std::min(!(r > 0) ? 0 : r, g_sharedexp_max);
		const float green_c = std::min(!(g > 0) ? 0 : g, g_sharedexp_max);
		const float blue_c = std::min(!(b > 0) ? 0 : b, g_sharedexp_max);

		// We're reducing the mantissa to 9 bits, so we must round up if the next
		// bit is 1. In other words add 0.5 to the new mantissa's position and
		// allow overflow into the exponent so we can scale correctly.
		constexpr int half = 1 << (23 - g_sharedexp_mantissabits);
		const float red_r = bit_cast<float>(bit_cast<int>(red_c) + half);
		const float green_r = bit_cast<float>(bit_cast<int>(green_c) + half);
		const float blue_r = bit_cast<float>(bit_cast<int>(blue_c) + half);

		// The largest component determines the shared exponent. It can't be lower
		// than 0 (after bias subtraction) so also limit to the mimimum representable.
		constexpr float min_s = 0.5f / (1 << g_sharedexp_bias);
		float max_s = std::max(std::max(red_r, green_r), std::max(blue_r, min_s));

		// Obtain the reciprocal of the shared exponent by inverting the bits,
		// and scale by the new mantissa's size. Note that the IEEE-754 single-precision
		// format has an implicit leading 1, but this shared component format does not.
		float scale = bit_cast<float>((bit_cast<int>(max_s) & 0x7F800000) ^ 0x7F800000) * (1 << (g_sharedexp_mantissabits - 2));

		R = static_cast<unsigned int>(round(red_c * scale));
		G = static_cast<unsigned int>(round(green_c * scale));
		B = static_cast<unsigned int>(round(blue_c * scale));
		E = (bit_cast<unsigned int>(max_s) >> 23) - 127 + 15 + 1;
	}

	operator unsigned int() const
	{
		return packed;
	}

	void toRGB16F(half rgb[3]) const
	{
		constexpr int offset = 24;  // Exponent bias (15) + number of mantissa bits per component (9) = 24

		const float factor = (1u << E) * (1.0f / (1 << offset));
		rgb[0] = half(R * factor);
		rgb[1] = half(G * factor);
		rgb[2] = half(B * factor);
	}
};

class R11G11B10F
{
	union
	{
		struct
		{
			unsigned int R : 11;
			unsigned int G : 11;
			unsigned int B : 10;
		};
		uint32_t packed;
	};

public:
	R11G11B10F(const float rgb[3])
	{
		R = float32ToFloat11(rgb[0]);
		G = float32ToFloat11(rgb[1]);
		B = float32ToFloat10(rgb[2]);
	}

	operator unsigned int() const
	{
		return packed;
	}

	void toRGB16F(half rgb[3]) const
	{
		rgb[0] = float11ToFloat16(R);
		rgb[1] = float11ToFloat16(G);
		rgb[2] = float10ToFloat16(B);
	}

	static inline half float11ToFloat16(unsigned short fp11)
	{
		return shortAsHalf(fp11 << 4);  // Sign bit 0
	}

	static inline half float10ToFloat16(unsigned short fp10)
	{
		return shortAsHalf(fp10 << 5);  // Sign bit 0
	}

	static inline unsigned short float32ToFloat11(float fp32)
	{
		const unsigned int float32MantissaMask = 0x7FFFFF;
		const unsigned int float32ExponentMask = 0x7F800000;
		const unsigned int float32SignMask = 0x80000000;
		const unsigned int float32ValueMask = ~float32SignMask;
		const unsigned int float32ExponentFirstBit = 23;
		const unsigned int float32ExponentBias = 127;

		const unsigned short float11Max = 0x7BF;
		const unsigned short float11MantissaMask = 0x3F;
		const unsigned short float11ExponentMask = 0x7C0;
		const unsigned short float11BitMask = 0x7FF;
		const unsigned int float11ExponentBias = 14;

		const unsigned int float32Maxfloat11 = 0x477E0000;
		const unsigned int float32MinNormfloat11 = 0x38800000;
		const unsigned int float32MinDenormfloat11 = 0x35000080;

		const unsigned int float32Bits = bit_cast<unsigned int>(fp32);
		const bool float32Sign = (float32Bits & float32SignMask) == float32SignMask;

		unsigned int float32Val = float32Bits & float32ValueMask;

		if((float32Val & float32ExponentMask) == float32ExponentMask)
		{
			// INF or NAN
			if((float32Val & float32MantissaMask) != 0)
			{
				return float11ExponentMask |
				       (((float32Val >> 17) | (float32Val >> 11) | (float32Val >> 6) | (float32Val)) &
				        float11MantissaMask);
			}
			else if(float32Sign)
			{
				// -INF is clamped to 0 since float11 is positive only
				return 0;
			}
			else
			{
				return float11ExponentMask;
			}
		}
		else if(float32Sign)
		{
			// float11 is positive only, so clamp to zero
			return 0;
		}
		else if(float32Val > float32Maxfloat11)
		{
			// The number is too large to be represented as a float11, set to max
			return float11Max;
		}
		else if(float32Val < float32MinDenormfloat11)
		{
			// The number is too small to be represented as a denormalized float11, set to 0
			return 0;
		}
		else
		{
			if(float32Val < float32MinNormfloat11)
			{
				// The number is too small to be represented as a normalized float11
				// Convert it to a denormalized value.
				const unsigned int shift = (float32ExponentBias - float11ExponentBias) -
				                           (float32Val >> float32ExponentFirstBit);
				float32Val =
				    ((1 << float32ExponentFirstBit) | (float32Val & float32MantissaMask)) >> shift;
			}
			else
			{
				// Rebias the exponent to represent the value as a normalized float11
				float32Val += 0xC8000000;
			}

			return ((float32Val + 0xFFFF + ((float32Val >> 17) & 1)) >> 17) & float11BitMask;
		}
	}

	static inline unsigned short float32ToFloat10(float fp32)
	{
		const unsigned int float32MantissaMask = 0x7FFFFF;
		const unsigned int float32ExponentMask = 0x7F800000;
		const unsigned int float32SignMask = 0x80000000;
		const unsigned int float32ValueMask = ~float32SignMask;
		const unsigned int float32ExponentFirstBit = 23;
		const unsigned int float32ExponentBias = 127;

		const unsigned short float10Max = 0x3DF;
		const unsigned short float10MantissaMask = 0x1F;
		const unsigned short float10ExponentMask = 0x3E0;
		const unsigned short float10BitMask = 0x3FF;
		const unsigned int float10ExponentBias = 14;

		const unsigned int float32Maxfloat10 = 0x477C0000;
		const unsigned int float32MinNormfloat10 = 0x38800000;
		const unsigned int float32MinDenormfloat10 = 0x35800040;

		const unsigned int float32Bits = bit_cast<unsigned int>(fp32);
		const bool float32Sign = (float32Bits & float32SignMask) == float32SignMask;

		unsigned int float32Val = float32Bits & float32ValueMask;

		if((float32Val & float32ExponentMask) == float32ExponentMask)
		{
			// INF or NAN
			if((float32Val & float32MantissaMask) != 0)
			{
				return float10ExponentMask |
				       (((float32Val >> 18) | (float32Val >> 13) | (float32Val >> 3) | (float32Val)) &
				        float10MantissaMask);
			}
			else if(float32Sign)
			{
				// -INF is clamped to 0 since float10 is positive only
				return 0;
			}
			else
			{
				return float10ExponentMask;
			}
		}
		else if(float32Sign)
		{
			// float10 is positive only, so clamp to zero
			return 0;
		}
		else if(float32Val > float32Maxfloat10)
		{
			// The number is too large to be represented as a float10, set to max
			return float10Max;
		}
		else if(float32Val < float32MinDenormfloat10)
		{
			// The number is too small to be represented as a denormalized float10, set to 0
			return 0;
		}
		else
		{
			if(float32Val < float32MinNormfloat10)
			{
				// The number is too small to be represented as a normalized float10
				// Convert it to a denormalized value.
				const unsigned int shift = (float32ExponentBias - float10ExponentBias) -
				                           (float32Val >> float32ExponentFirstBit);
				float32Val =
				    ((1 << float32ExponentFirstBit) | (float32Val & float32MantissaMask)) >> shift;
			}
			else
			{
				// Rebias the exponent to represent the value as a normalized float10
				float32Val += 0xC8000000;
			}

			return ((float32Val + 0x1FFFF + ((float32Val >> 18) & 1)) >> 18) & float10BitMask;
		}
	}
};

}  // namespace sw

#endif  // sw_Half_hpp
