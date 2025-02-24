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

#include "BC_Decoder.hpp"

#include "System/Debug.hpp"
#include "System/Math.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <vector>

#include <assert.h>
#include <stdint.h>

namespace {
static constexpr int BlockWidth = 4;
static constexpr int BlockHeight = 4;

struct BC_color
{
	void decode(uint8_t *dst, int x, int y, int dstW, int dstH, int dstPitch, int dstBpp, bool hasAlphaChannel, bool hasSeparateAlpha) const
	{
		Color c[4];
		c[0].extract565(c0);
		c[1].extract565(c1);
		if(hasSeparateAlpha || (c0 > c1))
		{
			c[2] = ((c[0] * 2) + c[1]) / 3;
			c[3] = ((c[1] * 2) + c[0]) / 3;
		}
		else
		{
			c[2] = (c[0] + c[1]) >> 1;
			if(hasAlphaChannel)
			{
				c[3].clearAlpha();
			}
		}

		for(int j = 0; j < BlockHeight && (y + j) < dstH; j++)
		{
			int dstOffset = j * dstPitch;
			int idxOffset = j * BlockHeight;
			for(int i = 0; i < BlockWidth && (x + i) < dstW; i++, idxOffset++, dstOffset += dstBpp)
			{
				*reinterpret_cast<unsigned int *>(dst + dstOffset) = c[getIdx(idxOffset)].pack8888();
			}
		}
	}

private:
	struct Color
	{
		Color()
		{
			c[0] = c[1] = c[2] = 0;
			c[3] = 0xFF000000;
		}

		void extract565(const unsigned int c565)
		{
			c[0] = ((c565 & 0x0000001F) << 3) | ((c565 & 0x0000001C) >> 2);
			c[1] = ((c565 & 0x000007E0) >> 3) | ((c565 & 0x00000600) >> 9);
			c[2] = ((c565 & 0x0000F800) >> 8) | ((c565 & 0x0000E000) >> 13);
		}

		unsigned int pack8888() const
		{
			return ((c[2] & 0xFF) << 16) | ((c[1] & 0xFF) << 8) | (c[0] & 0xFF) | c[3];
		}

		void clearAlpha()
		{
			c[3] = 0;
		}

		Color operator*(int factor) const
		{
			Color res;
			for(int i = 0; i < 4; ++i)
			{
				res.c[i] = c[i] * factor;
			}
			return res;
		}

		Color operator/(int factor) const
		{
			Color res;
			for(int i = 0; i < 4; ++i)
			{
				res.c[i] = c[i] / factor;
			}
			return res;
		}

		Color operator>>(int shift) const
		{
			Color res;
			for(int i = 0; i < 4; ++i)
			{
				res.c[i] = c[i] >> shift;
			}
			return res;
		}

		Color operator+(const Color &obj) const
		{
			Color res;
			for(int i = 0; i < 4; ++i)
			{
				res.c[i] = c[i] + obj.c[i];
			}
			return res;
		}

	private:
		int c[4];
	};

	unsigned int getIdx(int i) const
	{
		int offset = i << 1;  // 2 bytes per index
		return (idx & (0x3 << offset)) >> offset;
	}

	unsigned short c0;
	unsigned short c1;
	unsigned int idx;
};

struct BC_channel
{
	void decode(uint8_t *dst, int x, int y, int dstW, int dstH, int dstPitch, int dstBpp, int channel, bool isSigned) const
	{
		int c[8] = { 0 };

		if(isSigned)
		{
			c[0] = static_cast<signed char>(data & 0xFF);
			c[1] = static_cast<signed char>((data & 0xFF00) >> 8);
		}
		else
		{
			c[0] = static_cast<uint8_t>(data & 0xFF);
			c[1] = static_cast<uint8_t>((data & 0xFF00) >> 8);
		}

		if(c[0] > c[1])
		{
			for(int i = 2; i < 8; ++i)
			{
				c[i] = ((8 - i) * c[0] + (i - 1) * c[1]) / 7;
			}
		}
		else
		{
			for(int i = 2; i < 6; ++i)
			{
				c[i] = ((6 - i) * c[0] + (i - 1) * c[1]) / 5;
			}
			c[6] = isSigned ? -128 : 0;
			c[7] = isSigned ? 127 : 255;
		}

		for(int j = 0; j < BlockHeight && (y + j) < dstH; j++)
		{
			for(int i = 0; i < BlockWidth && (x + i) < dstW; i++)
			{
				dst[channel + (i * dstBpp) + (j * dstPitch)] = static_cast<uint8_t>(c[getIdx((j * BlockHeight) + i)]);
			}
		}
	}

private:
	uint8_t getIdx(int i) const
	{
		int offset = i * 3 + 16;
		return static_cast<uint8_t>((data & (0x7ull << offset)) >> offset);
	}

	uint64_t data;
};

struct BC_alpha
{
	void decode(uint8_t *dst, int x, int y, int dstW, int dstH, int dstPitch, int dstBpp) const
	{
		dst += 3;  // Write only to alpha (channel 3)
		for(int j = 0; j < BlockHeight && (y + j) < dstH; j++, dst += dstPitch)
		{
			uint8_t *dstRow = dst;
			for(int i = 0; i < BlockWidth && (x + i) < dstW; i++, dstRow += dstBpp)
			{
				*dstRow = getAlpha(j * BlockHeight + i);
			}
		}
	}

private:
	uint8_t getAlpha(int i) const
	{
		int offset = i << 2;
		int alpha = (data & (0xFull << offset)) >> offset;
		return static_cast<uint8_t>(alpha | (alpha << 4));
	}

	uint64_t data;
};

namespace BC6H {

static constexpr int MaxPartitions = 64;

static constexpr uint8_t PartitionTable2[MaxPartitions][16] = {
	{ 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1 },
	{ 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1 },
	{ 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1 },
	{ 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0 },
	{ 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 },
	{ 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0 },
	{ 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1 },
	{ 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0 },
	{ 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0 },
	{ 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0 },
	{ 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
	{ 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0 },
	{ 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0 },
	{ 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1 },
	{ 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0 },
	{ 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0 },
	{ 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0 },
	{ 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0 },
	{ 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1 },
	{ 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1 },
	{ 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0 },
	{ 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0 },
	{ 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0 },
	{ 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0 },
	{ 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 },
	{ 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1 },
	{ 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1 },
	{ 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0 },
	{ 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0 },
	{ 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0 },
	{ 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1 },
	{ 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0 },
	{ 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0 },
	{ 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1 },
	{ 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1 },
	{ 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
	{ 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
	{ 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 },
	{ 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1 },
};

static constexpr uint8_t AnchorTable2[MaxPartitions] = {
	// clang-format off
	0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
	0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
	0xf, 0x2, 0x8, 0x2, 0x2, 0x8, 0x8, 0xf,
	0x2, 0x8, 0x2, 0x2, 0x8, 0x8, 0x2, 0x2,
	0xf, 0xf, 0x6, 0x8, 0x2, 0x8, 0xf, 0xf,
	0x2, 0x8, 0x2, 0x2, 0x2, 0xf, 0xf, 0x6,
	0x6, 0x2, 0x6, 0x8, 0xf, 0xf, 0x2, 0x2,
	0xf, 0xf, 0xf, 0xf, 0xf, 0x2, 0x2, 0xf,
	// clang-format on
};

// 1.0f in half-precision floating point format
static constexpr uint16_t halfFloat1 = 0x3C00;
union Color
{
	struct RGBA
	{
		uint16_t r = 0;
		uint16_t g = 0;
		uint16_t b = 0;
		uint16_t a = halfFloat1;

		RGBA(uint16_t r, uint16_t g, uint16_t b)
		    : r(r)
		    , g(g)
		    , b(b)
		{
		}

		RGBA &operator=(const RGBA &other)
		{
			this->r = other.r;
			this->g = other.g;
			this->b = other.b;
			this->a = halfFloat1;

			return *this;
		}
	};

	Color(uint16_t r, uint16_t g, uint16_t b)
	    : rgba(r, g, b)
	{
	}

	Color(int r, int g, int b)
	    : rgba((uint16_t)r, (uint16_t)g, (uint16_t)b)
	{
	}

	Color()
	{
	}

	Color(const Color &other)
	{
		this->rgba = other.rgba;
	}

	Color &operator=(const Color &other)
	{
		this->rgba = other.rgba;

		return *this;
	}

	RGBA rgba;
	uint16_t channel[4];
};
static_assert(sizeof(Color) == 8, "BC6h::Color must be 8 bytes long");

inline int32_t extendSign(int32_t val, size_t size)
{
	// Suppose we have a 2-bit integer being stored in 4 bit variable:
	//    x = 0b00AB
	//
	// In order to sign extend x, we need to turn the 0s into A's:
	//    x_extend = 0bAAAB
	//
	// We can do that by flipping A in x then subtracting 0b0010 from x.
	// Suppose A is 1:
	//    x       = 0b001B
	//    x_flip  = 0b000B
	//    x_minus = 0b111B
	// Since A is flipped to 0, subtracting the mask sets it and all the bits above it to 1.
	// And if A is 0:
	//    x       = 0b000B
	//    x_flip  = 0b001B
	//    x_minus = 0b000B
	// We unset the bit we flipped, and touch no other bit
	uint16_t mask = 1u << (size - 1);
	return (val ^ mask) - mask;
}

static int constexpr RGBfChannels = 3;
struct RGBf
{
	uint16_t channel[RGBfChannels];
	size_t size[RGBfChannels];
	bool isSigned;

	RGBf()
	{
		static_assert(RGBfChannels == 3, "RGBf must have exactly 3 channels");
		static_assert(sizeof(channel) / sizeof(channel[0]) == RGBfChannels, "RGBf must have exactly 3 channels");
		static_assert(sizeof(channel) / sizeof(channel[0]) == sizeof(size) / sizeof(size[0]), "RGBf requires equally sized arrays for channels and channel sizes");

		for(int i = 0; i < RGBfChannels; i++)
		{
			channel[i] = 0;
			size[i] = 0;
		}

		isSigned = false;
	}

	void extendSign()
	{
		for(int i = 0; i < RGBfChannels; i++)
		{
			channel[i] = BC6H::extendSign(channel[i], size[i]);
		}
	}

	// Assuming this is the delta, take the base-endpoint and transform this into
	// a proper endpoint.
	//
	// The final computed endpoint is truncated to the base-endpoint's size;
	void resolveDelta(RGBf base)
	{
		for(int i = 0; i < RGBfChannels; i++)
		{
			size[i] = base.size[i];
			channel[i] = (base.channel[i] + channel[i]) & ((1 << base.size[i]) - 1);
		}

		// Per the spec:
		// "For signed formats, the results of the delta calculation must be sign
		// extended as well."
		if(isSigned)
		{
			extendSign();
		}
	}

	void unquantize()
	{
		if(isSigned)
		{
			unquantizeSigned();
		}
		else
		{
			unquantizeUnsigned();
		}
	}

	void unquantizeUnsigned()
	{
		for(int i = 0; i < RGBfChannels; i++)
		{
			if(size[i] >= 15 || channel[i] == 0)
			{
				continue;
			}
			else if(channel[i] == ((1u << size[i]) - 1))
			{
				channel[i] = 0xFFFFu;
			}
			else
			{
				// Need 32 bits to avoid overflow
				uint32_t tmp = channel[i];
				channel[i] = (uint16_t)(((tmp << 16) + 0x8000) >> size[i]);
			}
			size[i] = 16;
		}
	}

	void unquantizeSigned()
	{
		for(int i = 0; i < RGBfChannels; i++)
		{
			if(size[i] >= 16 || channel[i] == 0)
			{
				continue;
			}

			int16_t value = sw::bit_cast<int16_t>(channel[i]);
			int32_t result = value;
			bool signBit = value < 0;
			if(signBit)
			{
				value = -value;
			}

			if(value >= ((1 << (size[i] - 1)) - 1))
			{
				result = 0x7FFF;
			}
			else
			{
				// Need 32 bits to avoid overflow
				int32_t tmp = value;
				result = (((tmp << 15) + 0x4000) >> (size[i] - 1));
			}

			if(signBit)
			{
				result = -result;
			}

			channel[i] = (uint16_t)result;
			size[i] = 16;
		}
	}
};

struct Data
{
	uint64_t low64;
	uint64_t high64;

	Data() = default;
	Data(uint64_t low64, uint64_t high64)
	    : low64(low64)
	    , high64(high64)
	{
	}

	// Consumes the lowest N bits from from low64 and high64 where N is:
	//      abs(MSB - LSB)
	// MSB and LSB come from the block description of the BC6h spec and specify
	// the location of the bits in the returned bitstring.
	//
	// If MSB < LSB, then the bits are reversed. Otherwise, the bitstring is read and
	// shifted without further modification.
	//
	uint32_t consumeBits(uint32_t MSB, uint32_t LSB)
	{
		bool reversed = MSB < LSB;
		if(reversed)
		{
			std::swap(MSB, LSB);
		}
		ASSERT(MSB - LSB + 1 < sizeof(uint32_t) * 8);

		uint32_t numBits = MSB - LSB + 1;
		uint32_t mask = (1 << numBits) - 1;
		// Read the low N bits
		uint32_t bits = (low64 & mask);

		low64 >>= numBits;
		// Put the low N bits of high64 into the high 64-N bits of low64
		low64 |= (high64 & mask) << (sizeof(high64) * 8 - numBits);
		high64 >>= numBits;

		if(reversed)
		{
			uint32_t tmp = 0;
			for(uint32_t numSwaps = 0; numSwaps < numBits; numSwaps++)
			{
				tmp <<= 1;
				tmp |= (bits & 1);
				bits >>= 1;
			}

			bits = tmp;
		}

		return bits << LSB;
	}
};

struct IndexInfo
{
	uint64_t value;
	int numBits;
};

// Interpolates between two endpoints, then does a final unquantization step
Color interpolate(RGBf e0, RGBf e1, const IndexInfo &index, bool isSigned)
{
	static constexpr uint32_t weights3[] = { 0, 9, 18, 27, 37, 46, 55, 64 };
	static constexpr uint32_t weights4[] = { 0, 4, 9, 13, 17, 21, 26, 30,
		                                     34, 38, 43, 47, 51, 55, 60, 64 };
	static const uint32_t constexpr *weightsN[] = {
		nullptr, nullptr, nullptr, weights3, weights4
	};
	const uint32_t *weights = weightsN[index.numBits];
	ASSERT_MSG(weights != nullptr, "Unexpected number of index bits: %d", (int)index.numBits);
	Color color;
	uint32_t e0Weight = 64 - weights[index.value];
	uint32_t e1Weight = weights[index.value];

	for(int i = 0; i < RGBfChannels; i++)
	{
		int32_t e0Channel = e0.channel[i];
		int32_t e1Channel = e1.channel[i];

		if(isSigned)
		{
			e0Channel = extendSign(e0Channel, 16);
			e1Channel = extendSign(e1Channel, 16);
		}

		int32_t e0Value = e0Channel * e0Weight;
		int32_t e1Value = e1Channel * e1Weight;

		uint32_t tmp = ((e0Value + e1Value + 32) >> 6);

		// Need to unquantize value to limit it to the legal range of half-precision
		// floats. We do this by scaling by 31/32 or 31/64 depending on if the value
		// is signed or unsigned.
		if(isSigned)
		{
			tmp = ((tmp & 0x80000000) != 0) ? (((~tmp + 1) * 31) >> 5) | 0x8000 : (tmp * 31) >> 5;
			// Don't return -0.0f, just normalize it to 0.0f.
			if(tmp == 0x8000)
				tmp = 0;
		}
		else
		{
			tmp = (tmp * 31) >> 6;
		}

		color.channel[i] = (uint16_t)tmp;
	}

	return color;
}

enum DataType
{
	// Endpoints
	EP0 = 0,
	EP1 = 1,
	EP2 = 2,
	EP3 = 3,
	Mode,
	Partition,
	End,
};

enum Channel
{
	R = 0,
	G = 1,
	B = 2,
	None,
};

struct DeltaBits
{
	size_t channel[3];

	constexpr DeltaBits()
	    : channel{ 0, 0, 0 }
	{
	}

	constexpr DeltaBits(size_t r, size_t g, size_t b)
	    : channel{ r, g, b }
	{
	}
};

struct ModeDesc
{
	int number;
	bool hasDelta;
	int partitionCount;
	int endpointBits;
	DeltaBits deltaBits;

	constexpr ModeDesc()
	    : number(-1)
	    , hasDelta(false)
	    , partitionCount(0)
	    , endpointBits(0)
	{
	}

	constexpr ModeDesc(int number, bool hasDelta, int partitionCount, int endpointBits, DeltaBits deltaBits)
	    : number(number)
	    , hasDelta(hasDelta)
	    , partitionCount(partitionCount)
	    , endpointBits(endpointBits)
	    , deltaBits(deltaBits)
	{
	}
};

struct BlockDesc
{
	DataType type;
	Channel channel;
	int MSB;
	int LSB;
	ModeDesc modeDesc;

	constexpr BlockDesc()
	    : type(End)
	    , channel(None)
	    , MSB(0)
	    , LSB(0)
	    , modeDesc()
	{
	}

	constexpr BlockDesc(const DataType type, Channel channel, int MSB, int LSB, ModeDesc modeDesc)
	    : type(type)
	    , channel(channel)
	    , MSB(MSB)
	    , LSB(LSB)
	    , modeDesc(modeDesc)
	{
	}

	constexpr BlockDesc(DataType type, Channel channel, int MSB, int LSB)
	    : type(type)
	    , channel(channel)
	    , MSB(MSB)
	    , LSB(LSB)
	    , modeDesc()
	{
	}
};

// Turns a legal mode into an index into the BlockDesc table.
// Illegal or reserved modes return -1.
static int modeToIndex(uint8_t mode)
{
	if(mode <= 3)
	{
		return mode;
	}
	else if((mode & 0x2) != 0)
	{
		if(mode <= 18)
		{
			// Turns 6 into 4, 7 into 5, 10 into 6, etc.
			return (mode / 2) + 1 + (mode & 0x1);
		}
		else if(mode == 22 || mode == 26 || mode == 30)
		{
			// Turns 22 into 11, 26 into 12, etc.
			return mode / 4 + 6;
		}
	}

	return -1;
}

// Returns a description of the bitfields for each mode from the LSB
// to the MSB before the index data starts.
//
// The numbers come from the BC6h block description. Each BlockDesc in the
//   {Type, Channel, MSB, LSB}
//   * Type describes which endpoint this is, or if this is a mode, a partition
//     number, or the end of the block description.
//   * Channel describes one of the 3 color channels within an endpoint
//   * MSB and LSB specificy:
//      * The size of the bitfield being read
//      * The position of the bitfield within the variable it is being read to
//      * If the bitfield is stored in reverse bit order
//     If MSB < LSB then the bitfield is stored in reverse order. The size of
//     the bitfield is abs(MSB-LSB+1). And the position of the bitfield within
//     the variable is min(LSB, MSB).
//
// Invalid or reserved modes return an empty list.
static constexpr int NumBlocks = 14;
// The largest number of descriptions within a block.
static constexpr int MaxBlockDescIndex = 26;
static constexpr BlockDesc blockDescs[NumBlocks][MaxBlockDescIndex] = {
	// clang-format off
	// Mode 0, Index 0
	{
		{ Mode, None, 1, 0, { 0, true, 2, 10, { 5, 5, 5 } } },
		{ EP2, G, 4, 4 }, { EP2, B, 4, 4 }, { EP3, B, 4, 4 },
		{ EP0, R, 9, 0 }, { EP0, G, 9, 0 }, { EP0, B, 9, 0 },
		{ EP1, R, 4, 0 }, { EP3, G, 4, 4 }, { EP2, G, 3, 0 },
		{ EP1, G, 4, 0 }, { EP3, B, 0, 0 }, { EP3, G, 3, 0 },
		{ EP1, B, 4, 0 }, { EP3, B, 1, 1 }, { EP2, B, 3, 0 },
		{ EP2, R, 4, 0 }, { EP3, B, 2, 2 }, { EP3, R, 4, 0 },
		{ EP3, B, 3, 3 },
		{ Partition, None, 4, 0 },
		{ End, None, 0, 0},
	},
	// Mode 1, Index 1
	{
		{ Mode, None, 1, 0, { 1, true, 2, 7, { 6, 6, 6 } } },
		{ EP2, G, 5, 5 }, { EP3, G, 5, 4 }, { EP0, R, 6, 0 },
		{ EP3, B, 1, 0 }, { EP2, B, 4, 4 }, { EP0, G, 6, 0 },
		{ EP2, B, 5, 5 }, { EP3, B, 2, 2 }, { EP2, G, 4, 4 },
		{ EP0, B, 6, 0 }, { EP3, B, 3, 3 }, { EP3, B, 5, 5 },
		{ EP3, B, 4, 4 }, { EP1, R, 5, 0 }, { EP2, G, 3, 0 },
		{ EP1, G, 5, 0 }, { EP3, G, 3, 0 }, { EP1, B, 5, 0 },
		{ EP2, B, 3, 0 }, { EP2, R, 5, 0 }, { EP3, R, 5, 0 },
		{ Partition, None, 4, 0 },
		{ End, None, 0, 0},
	},
	// Mode 2, Index 2
	{
		{ Mode, None, 4, 0, { 2, true, 2, 11, { 5, 4, 4 } } },
		{ EP0, R, 9, 0 }, { EP0, G, 9, 0 }, { EP0, B, 9, 0 },
		{ EP1, R, 4, 0 }, { EP0, R, 10, 10 }, { EP2, G, 3, 0 },
		{ EP1, G, 3, 0 }, { EP0, G, 10, 10 }, { EP3, B, 0, 0 },
		{ EP3, G, 3, 0 }, { EP1, B, 3, 0 }, { EP0, B, 10, 10 },
		{ EP3, B, 1, 1 }, { EP2, B, 3, 0 }, { EP2, R, 4, 0 },
		{ EP3, B, 2, 2 }, { EP3, R, 4, 0 }, { EP3, B, 3, 3 },
		{ Partition, None, 4, 0 },
		{ End, None, 0, 0},
	},
	// Mode 3, Index 3
	{
		{ Mode, None, 4, 0, { 3, false, 1, 10, { 0, 0, 0 } } },
		{ EP0, R, 9, 0 }, { EP0, G, 9, 0 }, { EP0, B, 9, 0 },
		{ EP1, R, 9, 0 }, { EP1, G, 9, 0 }, { EP1, B, 9, 0 },
		{ End, None, 0, 0},
	},
	// Mode 6, Index 4
	{
		{ Mode, None, 4, 0, { 6, true, 2, 11, { 4, 5, 4 } } }, // 1 1
		{ EP0, R, 9, 0 }, { EP0, G, 9, 0 }, { EP0, B, 9, 0 },
		{ EP1, R, 3, 0 }, { EP0, R, 10, 10 }, { EP3, G, 4, 4 },
		{ EP2, G, 3, 0 }, { EP1, G, 4, 0 }, { EP0, G, 10, 10 },
		{ EP3, G, 3, 0 }, { EP1, B, 3, 0 }, { EP0, B, 10, 10 },
		{ EP3, B, 1, 1 }, { EP2, B, 3, 0 }, { EP2, R, 3, 0 },
		{ EP3, B, 0, 0 }, { EP3, B, 2, 2 }, { EP3, R, 3, 0 }, // 18 19
		{ EP2, G, 4, 4 }, { EP3, B, 3, 3 }, // 2 21
		{ Partition, None, 4, 0 },
		{ End, None, 0, 0},
	},
	// Mode 7, Index 5
	{
		{ Mode, None, 4, 0, { 7, true, 1, 11, { 9, 9, 9 } } },
		{ EP0, R, 9, 0 }, { EP0, G, 9, 0 }, { EP0, B, 9, 0 },
		{ EP1, R, 8, 0 }, { EP0, R, 10, 10 }, { EP1, G, 8, 0 },
		{ EP0, G, 10, 10 }, { EP1, B, 8, 0 }, { EP0, B, 10, 10 },
		{ End, None, 0, 0},
	},
	// Mode 10, Index 6
	{
		{ Mode, None, 4, 0, { 10, true, 2, 11, { 4, 4, 5 } } },
		{ EP0, R, 9, 0 }, { EP0, G, 9, 0 }, { EP0, B, 9, 0 },
		{ EP1, R, 3, 0 }, { EP0, R, 10, 10 }, { EP2, B, 4, 4 },
		{ EP2, G, 3, 0 }, { EP1, G, 3, 0 }, { EP0, G, 10, 10 },
		{ EP3, B, 0, 0 }, { EP3, G, 3, 0 }, { EP1, B, 4, 0 },
		{ EP0, B, 10, 10 }, { EP2, B, 3, 0 }, { EP2, R, 3, 0 },
		{ EP3, B, 1, 1 }, { EP3, B, 2, 2 }, { EP3, R, 3, 0 },
		{ EP3, B, 4, 4 }, { EP3, B, 3, 3 },
		{ Partition, None, 4, 0 },
		{ End, None, 0, 0},
	},
	// Mode 11, Index 7
	{
		{ Mode, None, 4, 0, { 11, true, 1, 12, { 8, 8, 8 } } },
		{ EP0, R, 9, 0 }, { EP0, G, 9, 0 }, { EP0, B, 9, 0 },
		{ EP1, R, 7, 0 }, { EP0, R, 10, 11 }, { EP1, G, 7, 0 },
		{ EP0, G, 10, 11 }, { EP1, B, 7, 0 }, { EP0, B, 10, 11 },
		{ End, None, 0, 0},
	},
	// Mode 14, Index 8
	{
		{ Mode, None, 4, 0, { 14, true, 2, 9, { 5, 5, 5 } } },
		{ EP0, R, 8, 0 }, { EP2, B, 4, 4 }, { EP0, G, 8, 0 },
		{ EP2, G, 4, 4 }, { EP0, B, 8, 0 }, { EP3, B, 4, 4 },
		{ EP1, R, 4, 0 }, { EP3, G, 4, 4 }, { EP2, G, 3, 0 },
		{ EP1, G, 4, 0 }, { EP3, B, 0, 0 }, { EP3, G, 3, 0 },
		{ EP1, B, 4, 0 }, { EP3, B, 1, 1 }, { EP2, B, 3, 0 },
		{ EP2, R, 4, 0 }, { EP3, B, 2, 2 }, { EP3, R, 4, 0 },
		{ EP3, B, 3, 3 },
		{ Partition, None, 4, 0 },
		{ End, None, 0, 0},
	},
	// Mode 15, Index 9
	{
		{ Mode, None, 4, 0, { 15, true, 1, 16, { 4, 4, 4 } } },
		{ EP0, R, 9, 0 }, { EP0, G, 9, 0 }, { EP0, B, 9, 0 },
		{ EP1, R, 3, 0 }, { EP0, R, 10, 15 }, { EP1, G, 3, 0 },
		{ EP0, G, 10, 15 }, { EP1, B, 3, 0 }, { EP0, B, 10, 15 },
		{ End, None, 0, 0},
	},
	// Mode 18, Index 10
	{
		{ Mode, None, 4, 0, { 18, true, 2, 8, { 6, 5, 5 } } },
		{ EP0, R, 7, 0 }, { EP3, G, 4, 4 }, { EP2, B, 4, 4 },
		{ EP0, G, 7, 0 }, { EP3, B, 2, 2 }, { EP2, G, 4, 4 },
		{ EP0, B, 7, 0 }, { EP3, B, 3, 3 }, { EP3, B, 4, 4 },
		{ EP1, R, 5, 0 }, { EP2, G, 3, 0 }, { EP1, G, 4, 0 },
		{ EP3, B, 0, 0 }, { EP3, G, 3, 0 }, { EP1, B, 4, 0 },
		{ EP3, B, 1, 1 }, { EP2, B, 3, 0 }, { EP2, R, 5, 0 },
		{ EP3, R, 5, 0 },
		{ Partition, None, 4, 0 },
		{ End, None, 0, 0},
	},
	// Mode 22, Index 11
	{
		{ Mode, None, 4, 0, { 22, true, 2, 8, { 5, 6, 5 } } },
		{ EP0, R, 7, 0 }, { EP3, B, 0, 0 }, { EP2, B, 4, 4 },
		{ EP0, G, 7, 0 }, { EP2, G, 5, 5 }, { EP2, G, 4, 4 },
		{ EP0, B, 7, 0 }, { EP3, G, 5, 5 }, { EP3, B, 4, 4 },
		{ EP1, R, 4, 0 }, { EP3, G, 4, 4 }, { EP2, G, 3, 0 },
		{ EP1, G, 5, 0 }, { EP3, G, 3, 0 }, { EP1, B, 4, 0 },
		{ EP3, B, 1, 1 }, { EP2, B, 3, 0 }, { EP2, R, 4, 0 },
		{ EP3, B, 2, 2 }, { EP3, R, 4, 0 }, { EP3, B, 3, 3 },
		{ Partition, None, 4, 0 },
		{ End, None, 0, 0},
	},
	// Mode 26, Index 12
	{
		{ Mode, None, 4, 0, { 26, true, 2, 8, { 5, 5, 6 } } },
		{ EP0, R, 7, 0 }, { EP3, B, 1, 1 }, { EP2, B, 4, 4 },
		{ EP0, G, 7, 0 }, { EP2, B, 5, 5 }, { EP2, G, 4, 4 },
		{ EP0, B, 7, 0 }, { EP3, B, 5, 5 }, { EP3, B, 4, 4 },
		{ EP1, R, 4, 0 }, { EP3, G, 4, 4 }, { EP2, G, 3, 0 },
		{ EP1, G, 4, 0 }, { EP3, B, 0, 0 }, { EP3, G, 3, 0 },
		{ EP1, B, 5, 0 }, { EP2, B, 3, 0 }, { EP2, R, 4, 0 },
		{ EP3, B, 2, 2 }, { EP3, R, 4, 0 }, { EP3, B, 3, 3 },
		{ Partition, None, 4, 0 },
		{ End, None, 0, 0},
	},
	// Mode 30, Index 13
	{
		{ Mode, None, 4, 0, { 30, false, 2, 6, { 0, 0, 0 } } },
		{ EP0, R, 5, 0 }, { EP3, G, 4, 4 }, { EP3, B, 0, 0 },
		{ EP3, B, 1, 1 }, { EP2, B, 4, 4 }, { EP0, G, 5, 0 },
		{ EP2, G, 5, 5 }, { EP2, B, 5, 5 }, { EP3, B, 2, 2 },
		{ EP2, G, 4, 4 }, { EP0, B, 5, 0 }, { EP3, G, 5, 5 },
		{ EP3, B, 3, 3 }, { EP3, B, 5, 5 }, { EP3, B, 4, 4 },
		{ EP1, R, 5, 0 }, { EP2, G, 3, 0 }, { EP1, G, 5, 0 },
		{ EP3, G, 3, 0 }, { EP1, B, 5, 0 }, { EP2, B, 3, 0 },
		{ EP2, R, 5, 0 }, { EP3, R, 5, 0 },
		{ Partition, None, 4, 0 },
		{ End, None, 0, 0},
	}
	// clang-format on
};

struct Block
{
	uint64_t low64;
	uint64_t high64;

	void decode(uint8_t *dst, int dstX, int dstY, int dstWidth, int dstHeight, size_t dstPitch, size_t dstBpp, bool isSigned) const
	{
		uint8_t mode = 0;
		Data data(low64, high64);
		ASSERT(dstBpp == sizeof(Color));

		if((data.low64 & 0x2) == 0)
		{
			mode = data.consumeBits(1, 0);
		}
		else
		{
			mode = data.consumeBits(4, 0);
		}

		int blockIndex = modeToIndex(mode);
		// Handle illegal or reserved mode
		if(blockIndex == -1)
		{
			for(int y = 0; y < 4 && y + dstY < dstHeight; y++)
			{
				for(int x = 0; x < 4 && x + dstX < dstWidth; x++)
				{
					auto out = reinterpret_cast<Color *>(dst + sizeof(Color) * x + dstPitch * y);
					out->rgba = { 0, 0, 0 };
				}
			}
			return;
		}
		const BlockDesc *blockDesc = blockDescs[blockIndex];

		RGBf e[4];
		e[0].isSigned = e[1].isSigned = e[2].isSigned = e[3].isSigned = isSigned;

		int partition = 0;
		ModeDesc modeDesc;
		for(int index = 0; blockDesc[index].type != End; index++)
		{
			const BlockDesc desc = blockDesc[index];

			switch(desc.type)
			{
			case Mode:
				modeDesc = desc.modeDesc;
				ASSERT(modeDesc.number == mode);

				e[0].size[0] = e[0].size[1] = e[0].size[2] = modeDesc.endpointBits;
				for(int i = 0; i < RGBfChannels; i++)
				{
					if(modeDesc.hasDelta)
					{
						e[1].size[i] = e[2].size[i] = e[3].size[i] = modeDesc.deltaBits.channel[i];
					}
					else
					{
						e[1].size[i] = e[2].size[i] = e[3].size[i] = modeDesc.endpointBits;
					}
				}
				break;
			case Partition:
				partition |= data.consumeBits(desc.MSB, desc.LSB);
				break;
			case EP0:
			case EP1:
			case EP2:
			case EP3:
				e[desc.type].channel[desc.channel] |= data.consumeBits(desc.MSB, desc.LSB);
				break;
			default:
				ASSERT_MSG(false, "Unexpected enum value: %d", (int)desc.type);
				return;
			}
		}

		// Sign extension
		if(isSigned)
		{
			for(int ep = 0; ep < modeDesc.partitionCount * 2; ep++)
			{
				e[ep].extendSign();
			}
		}
		else if(modeDesc.hasDelta)
		{
			// Don't sign-extend the base endpoint in an unsigned format.
			for(int ep = 1; ep < modeDesc.partitionCount * 2; ep++)
			{
				e[ep].extendSign();
			}
		}

		// Turn the deltas into endpoints
		if(modeDesc.hasDelta)
		{
			for(int ep = 1; ep < modeDesc.partitionCount * 2; ep++)
			{
				e[ep].resolveDelta(e[0]);
			}
		}

		for(int ep = 0; ep < modeDesc.partitionCount * 2; ep++)
		{
			e[ep].unquantize();
		}

		// Get the indices, calculate final colors, and output
		for(int y = 0; y < 4; y++)
		{
			for(int x = 0; x < 4; x++)
			{
				int pixelNum = x + y * 4;
				IndexInfo idx;
				bool isAnchor = false;
				int firstEndpoint = 0;
				// Bc6H can have either 1 or 2 petitions depending on the mode.
				// The number of petitions affects the number of indices with implicit
				// leading 0 bits and the number of bits per index.
				if(modeDesc.partitionCount == 1)
				{
					idx.numBits = 4;
					// There's an implicit leading 0 bit for the first idx
					isAnchor = (pixelNum == 0);
				}
				else
				{
					idx.numBits = 3;
					// There are 2 indices with implicit leading 0-bits.
					isAnchor = ((pixelNum == 0) || (pixelNum == AnchorTable2[partition]));
					firstEndpoint = PartitionTable2[partition][pixelNum] * 2;
				}

				idx.value = data.consumeBits(idx.numBits - isAnchor - 1, 0);

				// Don't exit the loop early, we need to consume these index bits regardless if
				// we actually output them or not.
				if((y + dstY >= dstHeight) || (x + dstX >= dstWidth))
				{
					continue;
				}

				Color color = interpolate(e[firstEndpoint], e[firstEndpoint + 1], idx, isSigned);
				auto out = reinterpret_cast<Color *>(dst + dstBpp * x + dstPitch * y);
				*out = color;
			}
		}
	}
};

}  // namespace BC6H

namespace BC7 {
// https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_texture_compression_bptc.txt
// https://docs.microsoft.com/en-us/windows/win32/direct3d11/bc7-format

struct Bitfield
{
	int offset;
	int count;
	constexpr Bitfield Then(const int bits) { return { offset + count, bits }; }
	constexpr bool operator==(const Bitfield &rhs) const
	{
		return offset == rhs.offset && count == rhs.count;
	}
};

struct Mode
{
	const int IDX;  // Mode index
	const int NS;   // Number of subsets in each partition
	const int PB;   // Partition bits
	const int RB;   // Rotation bits
	const int ISB;  // Index selection bits
	const int CB;   // Color bits
	const int AB;   // Alpha bits
	const int EPB;  // Endpoint P-bits
	const int SPB;  // Shared P-bits
	const int IB;   // Primary index bits per element
	const int IBC;  // Primary index bits total
	const int IB2;  // Secondary index bits per element

	constexpr int NumColors() const { return NS * 2; }
	constexpr Bitfield Partition() const { return { IDX + 1, PB }; }
	constexpr Bitfield Rotation() const { return Partition().Then(RB); }
	constexpr Bitfield IndexSelection() const { return Rotation().Then(ISB); }
	constexpr Bitfield Red(int idx) const
	{
		return IndexSelection().Then(CB * idx).Then(CB);
	}
	constexpr Bitfield Green(int idx) const
	{
		return Red(NumColors() - 1).Then(CB * idx).Then(CB);
	}
	constexpr Bitfield Blue(int idx) const
	{
		return Green(NumColors() - 1).Then(CB * idx).Then(CB);
	}
	constexpr Bitfield Alpha(int idx) const
	{
		return Blue(NumColors() - 1).Then(AB * idx).Then(AB);
	}
	constexpr Bitfield EndpointPBit(int idx) const
	{
		return Alpha(NumColors() - 1).Then(EPB * idx).Then(EPB);
	}
	constexpr Bitfield SharedPBit0() const
	{
		return EndpointPBit(NumColors() - 1).Then(SPB);
	}
	constexpr Bitfield SharedPBit1() const
	{
		return SharedPBit0().Then(SPB);
	}
	constexpr Bitfield PrimaryIndex(int offset, int count) const
	{
		return SharedPBit1().Then(offset).Then(count);
	}
	constexpr Bitfield SecondaryIndex(int offset, int count) const
	{
		return SharedPBit1().Then(IBC + offset).Then(count);
	}
};

static constexpr Mode Modes[] = {
	//     IDX  NS   PB   RB   ISB  CB   AB   EPB  SPB  IB   IBC, IB2
	/**/ { 0x0, 0x3, 0x4, 0x0, 0x0, 0x4, 0x0, 0x1, 0x0, 0x3, 0x2d, 0x0 },
	/**/ { 0x1, 0x2, 0x6, 0x0, 0x0, 0x6, 0x0, 0x0, 0x1, 0x3, 0x2e, 0x0 },
	/**/ { 0x2, 0x3, 0x6, 0x0, 0x0, 0x5, 0x0, 0x0, 0x0, 0x2, 0x1d, 0x0 },
	/**/ { 0x3, 0x2, 0x6, 0x0, 0x0, 0x7, 0x0, 0x1, 0x0, 0x2, 0x1e, 0x0 },
	/**/ { 0x4, 0x1, 0x0, 0x2, 0x1, 0x5, 0x6, 0x0, 0x0, 0x2, 0x1f, 0x3 },
	/**/ { 0x5, 0x1, 0x0, 0x2, 0x0, 0x7, 0x8, 0x0, 0x0, 0x2, 0x1f, 0x2 },
	/**/ { 0x6, 0x1, 0x0, 0x0, 0x0, 0x7, 0x7, 0x1, 0x0, 0x4, 0x3f, 0x0 },
	/**/ { 0x7, 0x2, 0x6, 0x0, 0x0, 0x5, 0x5, 0x1, 0x0, 0x2, 0x1e, 0x0 },
	/**/ { -1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x00, 0x0 },
};

static_assert(Modes[0].NumColors() == 6, "BC7 static assertion failed");
static_assert(Modes[0].Partition() == Bitfield{ 1, 4 }, "BC7 static assertion failed");
static_assert(Modes[0].Red(0) == Bitfield{ 5, 4 }, "BC7 static assertion failed");
static_assert(Modes[0].Red(5) == Bitfield{ 25, 4 }, "BC7 static assertion failed");
static_assert(Modes[0].Green(0) == Bitfield{ 29, 4 }, "BC7 static assertion failed");
static_assert(Modes[0].Green(5) == Bitfield{ 49, 4 }, "BC7 static assertion failed");
static_assert(Modes[0].Blue(0) == Bitfield{ 53, 4 }, "BC7 static assertion failed");
static_assert(Modes[0].Blue(5) == Bitfield{ 73, 4 }, "BC7 static assertion failed");
static_assert(Modes[0].EndpointPBit(0) == Bitfield{ 77, 1 }, "BC7 static assertion failed");
static_assert(Modes[0].EndpointPBit(5) == Bitfield{ 82, 1 }, "BC7 static assertion failed");
static_assert(Modes[0].PrimaryIndex(0, 2) == Bitfield{ 83, 2 }, "BC7 static asassertionsert failed");
static_assert(Modes[0].PrimaryIndex(43, 1) == Bitfield{ 126, 1 }, "BC7 static assertion failed");

static constexpr int MaxPartitions = 64;
static constexpr int MaxSubsets = 3;

static constexpr uint8_t PartitionTable2[MaxPartitions][16] = {
	{ 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1 },
	{ 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1 },
	{ 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1 },
	{ 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1 },
	{ 0, 1, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0 },
	{ 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0 },
	{ 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0 },
	{ 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 1 },
	{ 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0 },
	{ 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0 },
	{ 0, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0, 0 },
	{ 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
	{ 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0 },
	{ 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0 },
	{ 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1 },
	{ 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0 },
	{ 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0 },
	{ 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0 },
	{ 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0 },
	{ 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1 },
	{ 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1 },
	{ 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 1, 0 },
	{ 0, 0, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0 },
	{ 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0 },
	{ 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0 },
	{ 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 },
	{ 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1 },
	{ 0, 1, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1 },
	{ 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0 },
	{ 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0 },
	{ 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0 },
	{ 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0 },
	{ 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0 },
	{ 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1 },
	{ 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0 },
	{ 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0 },
	{ 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1 },
	{ 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1 },
	{ 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1 },
	{ 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0 },
	{ 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0 },
	{ 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1 },
};

static constexpr uint8_t PartitionTable3[MaxPartitions][16] = {
	{ 0, 0, 1, 1, 0, 0, 1, 1, 0, 2, 2, 1, 2, 2, 2, 2 },
	{ 0, 0, 0, 1, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 2, 1 },
	{ 0, 0, 0, 0, 2, 0, 0, 1, 2, 2, 1, 1, 2, 2, 1, 1 },
	{ 0, 2, 2, 2, 0, 0, 2, 2, 0, 0, 1, 1, 0, 1, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2 },
	{ 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 2, 2, 0, 0, 2, 2 },
	{ 0, 0, 2, 2, 0, 0, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1 },
	{ 0, 0, 1, 1, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2 },
	{ 0, 1, 1, 2, 0, 1, 1, 2, 0, 1, 1, 2, 0, 1, 1, 2 },
	{ 0, 1, 2, 2, 0, 1, 2, 2, 0, 1, 2, 2, 0, 1, 2, 2 },
	{ 0, 0, 1, 1, 0, 1, 1, 2, 1, 1, 2, 2, 1, 2, 2, 2 },
	{ 0, 0, 1, 1, 2, 0, 0, 1, 2, 2, 0, 0, 2, 2, 2, 0 },
	{ 0, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 2, 1, 1, 2, 2 },
	{ 0, 1, 1, 1, 0, 0, 1, 1, 2, 0, 0, 1, 2, 2, 0, 0 },
	{ 0, 0, 0, 0, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2 },
	{ 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 1, 1, 1, 1 },
	{ 0, 1, 1, 1, 0, 1, 1, 1, 0, 2, 2, 2, 0, 2, 2, 2 },
	{ 0, 0, 0, 1, 0, 0, 0, 1, 2, 2, 2, 1, 2, 2, 2, 1 },
	{ 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 2, 2, 0, 1, 2, 2 },
	{ 0, 0, 0, 0, 1, 1, 0, 0, 2, 2, 1, 0, 2, 2, 1, 0 },
	{ 0, 1, 2, 2, 0, 1, 2, 2, 0, 0, 1, 1, 0, 0, 0, 0 },
	{ 0, 0, 1, 2, 0, 0, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2 },
	{ 0, 1, 1, 0, 1, 2, 2, 1, 1, 2, 2, 1, 0, 1, 1, 0 },
	{ 0, 0, 0, 0, 0, 1, 1, 0, 1, 2, 2, 1, 1, 2, 2, 1 },
	{ 0, 0, 2, 2, 1, 1, 0, 2, 1, 1, 0, 2, 0, 0, 2, 2 },
	{ 0, 1, 1, 0, 0, 1, 1, 0, 2, 0, 0, 2, 2, 2, 2, 2 },
	{ 0, 0, 1, 1, 0, 1, 2, 2, 0, 1, 2, 2, 0, 0, 1, 1 },
	{ 0, 0, 0, 0, 2, 0, 0, 0, 2, 2, 1, 1, 2, 2, 2, 1 },
	{ 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 2, 1, 2, 2, 2 },
	{ 0, 2, 2, 2, 0, 0, 2, 2, 0, 0, 1, 2, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 0, 0, 1, 2, 0, 0, 2, 2, 0, 2, 2, 2 },
	{ 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2, 0, 0, 1, 2, 0 },
	{ 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 0 },
	{ 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0 },
	{ 0, 1, 2, 0, 2, 0, 1, 2, 1, 2, 0, 1, 0, 1, 2, 0 },
	{ 0, 0, 1, 1, 2, 2, 0, 0, 1, 1, 2, 2, 0, 0, 1, 1 },
	{ 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 0, 0, 0, 0, 1, 1 },
	{ 0, 1, 0, 1, 0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 2, 1, 2, 1, 2, 1 },
	{ 0, 0, 2, 2, 1, 1, 2, 2, 0, 0, 2, 2, 1, 1, 2, 2 },
	{ 0, 0, 2, 2, 0, 0, 1, 1, 0, 0, 2, 2, 0, 0, 1, 1 },
	{ 0, 2, 2, 0, 1, 2, 2, 1, 0, 2, 2, 0, 1, 2, 2, 1 },
	{ 0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 0, 1, 0, 1 },
	{ 0, 0, 0, 0, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1 },
	{ 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 2, 2, 2, 2 },
	{ 0, 2, 2, 2, 0, 1, 1, 1, 0, 2, 2, 2, 0, 1, 1, 1 },
	{ 0, 0, 0, 2, 1, 1, 1, 2, 0, 0, 0, 2, 1, 1, 1, 2 },
	{ 0, 0, 0, 0, 2, 1, 1, 2, 2, 1, 1, 2, 2, 1, 1, 2 },
	{ 0, 2, 2, 2, 0, 1, 1, 1, 0, 1, 1, 1, 0, 2, 2, 2 },
	{ 0, 0, 0, 2, 1, 1, 1, 2, 1, 1, 1, 2, 0, 0, 0, 2 },
	{ 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 2, 2, 2, 2 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2, 2, 1, 1, 2 },
	{ 0, 1, 1, 0, 0, 1, 1, 0, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 0, 0, 2, 2, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 2, 2 },
	{ 0, 0, 2, 2, 1, 1, 2, 2, 1, 1, 2, 2, 0, 0, 2, 2 },
	{ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 2 },
	{ 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 1 },
	{ 0, 2, 2, 2, 1, 2, 2, 2, 0, 2, 2, 2, 1, 2, 2, 2 },
	{ 0, 1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 },
	{ 0, 1, 1, 1, 2, 0, 1, 1, 2, 2, 0, 1, 2, 2, 2, 0 },
};

static constexpr uint8_t AnchorTable2[MaxPartitions] = {
	// clang-format off
	0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
	0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
	0xf, 0x2, 0x8, 0x2, 0x2, 0x8, 0x8, 0xf,
	0x2, 0x8, 0x2, 0x2, 0x8, 0x8, 0x2, 0x2,
	0xf, 0xf, 0x6, 0x8, 0x2, 0x8, 0xf, 0xf,
	0x2, 0x8, 0x2, 0x2, 0x2, 0xf, 0xf, 0x6,
	0x6, 0x2, 0x6, 0x8, 0xf, 0xf, 0x2, 0x2,
	0xf, 0xf, 0xf, 0xf, 0xf, 0x2, 0x2, 0xf,
	// clang-format on
};

static constexpr uint8_t AnchorTable3a[MaxPartitions] = {
	// clang-format off
	0x3, 0x3, 0xf, 0xf, 0x8, 0x3, 0xf, 0xf,
	0x8, 0x8, 0x6, 0x6, 0x6, 0x5, 0x3, 0x3,
	0x3, 0x3, 0x8, 0xf, 0x3, 0x3, 0x6, 0xa,
	0x5, 0x8, 0x8, 0x6, 0x8, 0x5, 0xf, 0xf,
	0x8, 0xf, 0x3, 0x5, 0x6, 0xa, 0x8, 0xf,
	0xf, 0x3, 0xf, 0x5, 0xf, 0xf, 0xf, 0xf,
	0x3, 0xf, 0x5, 0x5, 0x5, 0x8, 0x5, 0xa,
	0x5, 0xa, 0x8, 0xd, 0xf, 0xc, 0x3, 0x3,
	// clang-format on
};

static constexpr uint8_t AnchorTable3b[MaxPartitions] = {
	// clang-format off
	0xf, 0x8, 0x8, 0x3, 0xf, 0xf, 0x3, 0x8,
	0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf, 0x8,
	0xf, 0x8, 0xf, 0x3, 0xf, 0x8, 0xf, 0x8,
	0x3, 0xf, 0x6, 0xa, 0xf, 0xf, 0xa, 0x8,
	0xf, 0x3, 0xf, 0xa, 0xa, 0x8, 0x9, 0xa,
	0x6, 0xf, 0x8, 0xf, 0x3, 0x6, 0x6, 0x8,
	0xf, 0x3, 0xf, 0xf, 0xf, 0xf, 0xf, 0xf,
	0xf, 0xf, 0xf, 0xf, 0x3, 0xf, 0xf, 0x8,
	// clang-format on
};

struct Color
{
	struct RGB
	{
		RGB() = default;
		RGB(uint8_t r, uint8_t g, uint8_t b)
		    : b(b)
		    , g(g)
		    , r(r)
		{}
		RGB(int r, int g, int b)
		    : b(static_cast<uint8_t>(b))
		    , g(static_cast<uint8_t>(g))
		    , r(static_cast<uint8_t>(r))
		{}

		RGB operator<<(int shift) const { return { r << shift, g << shift, b << shift }; }
		RGB operator>>(int shift) const { return { r >> shift, g >> shift, b >> shift }; }
		RGB operator|(int bits) const { return { r | bits, g | bits, b | bits }; }
		RGB operator|(const RGB &rhs) const { return { r | rhs.r, g | rhs.g, b | rhs.b }; }
		RGB operator+(const RGB &rhs) const { return { r + rhs.r, g + rhs.g, b + rhs.b }; }

		uint8_t b;
		uint8_t g;
		uint8_t r;
	};

	RGB rgb;
	uint8_t a;
};

static_assert(sizeof(Color) == 4, "Color size must be 4 bytes");

struct Block
{
	constexpr uint64_t Get(const Bitfield &bf) const
	{
		uint64_t mask = (1ULL << bf.count) - 1;
		if(bf.offset + bf.count <= 64)
		{
			return (low >> bf.offset) & mask;
		}
		if(bf.offset >= 64)
		{
			return (high >> (bf.offset - 64)) & mask;
		}
		return ((low >> bf.offset) | (high << (64 - bf.offset))) & mask;
	}

	const Mode &mode() const
	{
		if((low & 0b00000001) != 0) { return Modes[0]; }
		if((low & 0b00000010) != 0) { return Modes[1]; }
		if((low & 0b00000100) != 0) { return Modes[2]; }
		if((low & 0b00001000) != 0) { return Modes[3]; }
		if((low & 0b00010000) != 0) { return Modes[4]; }
		if((low & 0b00100000) != 0) { return Modes[5]; }
		if((low & 0b01000000) != 0) { return Modes[6]; }
		if((low & 0b10000000) != 0) { return Modes[7]; }
		return Modes[8];  // Invalid mode
	}

	struct IndexInfo
	{
		uint64_t value;
		int numBits;
	};

	uint8_t interpolate(uint8_t e0, uint8_t e1, const IndexInfo &index) const
	{
		static constexpr uint16_t weights2[] = { 0, 21, 43, 64 };
		static constexpr uint16_t weights3[] = { 0, 9, 18, 27, 37, 46, 55, 64 };
		static constexpr uint16_t weights4[] = { 0, 4, 9, 13, 17, 21, 26, 30,
			                                     34, 38, 43, 47, 51, 55, 60, 64 };
		static const uint16_t constexpr *weightsN[] = {
			nullptr, nullptr, weights2, weights3, weights4
		};
		const uint16_t *weights = weightsN[index.numBits];
		ASSERT_MSG(weights != nullptr, "Unexpected number of index bits: %d", (int)index.numBits);
		return (uint8_t)(((64 - weights[index.value]) * uint16_t(e0) + weights[index.value] * uint16_t(e1) + 32) >> 6);
	}

	void decode(uint8_t *dst, int dstX, int dstY, int dstWidth, int dstHeight, size_t dstPitch) const
	{
		const auto &mode = this->mode();

		if(mode.IDX < 0)  // Invalid mode:
		{
			for(int y = 0; y < 4 && y + dstY < dstHeight; y++)
			{
				for(int x = 0; x < 4 && x + dstX < dstWidth; x++)
				{
					auto out = reinterpret_cast<Color *>(dst + sizeof(Color) * x + dstPitch * y);
					out->rgb = { 0, 0, 0 };
					out->a = 0;
				}
			}
			return;
		}

		using Endpoint = std::array<Color, 2>;
		std::array<Endpoint, MaxSubsets> subsets;

		for(int i = 0; i < mode.NS; i++)
		{
			auto &subset = subsets[i];
			subset[0].rgb.r = Get(mode.Red(i * 2 + 0));
			subset[0].rgb.g = Get(mode.Green(i * 2 + 0));
			subset[0].rgb.b = Get(mode.Blue(i * 2 + 0));
			subset[0].a = (mode.AB > 0) ? Get(mode.Alpha(i * 2 + 0)) : 255;

			subset[1].rgb.r = Get(mode.Red(i * 2 + 1));
			subset[1].rgb.g = Get(mode.Green(i * 2 + 1));
			subset[1].rgb.b = Get(mode.Blue(i * 2 + 1));
			subset[1].a = (mode.AB > 0) ? Get(mode.Alpha(i * 2 + 1)) : 255;
		}

		if(mode.SPB > 0)
		{
			auto pbit0 = Get(mode.SharedPBit0());
			auto pbit1 = Get(mode.SharedPBit1());
			subsets[0][0].rgb = (subsets[0][0].rgb << 1) | pbit0;
			subsets[0][1].rgb = (subsets[0][1].rgb << 1) | pbit0;
			subsets[1][0].rgb = (subsets[1][0].rgb << 1) | pbit1;
			subsets[1][1].rgb = (subsets[1][1].rgb << 1) | pbit1;
		}

		if(mode.EPB > 0)
		{
			for(int i = 0; i < mode.NS; i++)
			{
				auto &subset = subsets[i];
				auto pbit0 = Get(mode.EndpointPBit(i * 2 + 0));
				auto pbit1 = Get(mode.EndpointPBit(i * 2 + 1));
				subset[0].rgb = (subset[0].rgb << 1) | pbit0;
				subset[1].rgb = (subset[1].rgb << 1) | pbit1;
				if(mode.AB > 0)
				{
					subset[0].a = (subset[0].a << 1) | pbit0;
					subset[1].a = (subset[1].a << 1) | pbit1;
				}
			}
		}

		const auto colorBits = mode.CB + mode.SPB + mode.EPB;
		const auto alphaBits = mode.AB + mode.SPB + mode.EPB;

		for(int i = 0; i < mode.NS; i++)
		{
			auto &subset = subsets[i];
			subset[0].rgb = subset[0].rgb << (8 - colorBits);
			subset[1].rgb = subset[1].rgb << (8 - colorBits);
			subset[0].rgb = subset[0].rgb | (subset[0].rgb >> colorBits);
			subset[1].rgb = subset[1].rgb | (subset[1].rgb >> colorBits);

			if(mode.AB > 0)
			{
				subset[0].a = subset[0].a << (8 - alphaBits);
				subset[1].a = subset[1].a << (8 - alphaBits);
				subset[0].a = subset[0].a | (subset[0].a >> alphaBits);
				subset[1].a = subset[1].a | (subset[1].a >> alphaBits);
			}
		}

		int colorIndexBitOffset = 0;
		int alphaIndexBitOffset = 0;
		for(int y = 0; y < 4; y++)
		{
			for(int x = 0; x < 4; x++)
			{
				auto texelIdx = y * 4 + x;
				auto partitionIdx = Get(mode.Partition());
				ASSERT(partitionIdx < MaxPartitions);
				auto subsetIdx = subsetIndex(mode, partitionIdx, texelIdx);
				ASSERT(subsetIdx < MaxSubsets);
				const auto &subset = subsets[subsetIdx];

				auto anchorIdx = anchorIndex(mode, partitionIdx, subsetIdx);
				auto isAnchor = anchorIdx == texelIdx;
				auto colorIdx = colorIndex(mode, isAnchor, colorIndexBitOffset);
				auto alphaIdx = alphaIndex(mode, isAnchor, alphaIndexBitOffset);

				if(y + dstY >= dstHeight || x + dstX >= dstWidth)
				{
					// Don't be tempted to skip early at the loops:
					// The calls to colorIndex() and alphaIndex() adjust bit
					// offsets that need to be carefully tracked.
					continue;
				}

				Color output;
				output.rgb.r = interpolate(subset[0].rgb.r, subset[1].rgb.r, colorIdx);
				output.rgb.g = interpolate(subset[0].rgb.g, subset[1].rgb.g, colorIdx);
				output.rgb.b = interpolate(subset[0].rgb.b, subset[1].rgb.b, colorIdx);
				output.a = interpolate(subset[0].a, subset[1].a, alphaIdx);

				switch(Get(mode.Rotation()))
				{
				default:
					break;
				case 1:
					std::swap(output.a, output.rgb.r);
					break;
				case 2:
					std::swap(output.a, output.rgb.g);
					break;
				case 3:
					std::swap(output.a, output.rgb.b);
					break;
				}

				auto out = reinterpret_cast<Color *>(dst + sizeof(Color) * x + dstPitch * y);
				*out = output;
			}
		}
	}

	int subsetIndex(const Mode &mode, int partitionIdx, int texelIndex) const
	{
		switch(mode.NS)
		{
		default:
			return 0;
		case 2:
			return PartitionTable2[partitionIdx][texelIndex];
		case 3:
			return PartitionTable3[partitionIdx][texelIndex];
		}
	}

	int anchorIndex(const Mode &mode, int partitionIdx, int subsetIdx) const
	{
		// ARB_texture_compression_bptc states:
		// "In partition zero, the anchor index is always index zero.
		// In other partitions, the anchor index is specified by tables
		// Table.A2 and Table.A3.""
		// Note: This is really confusing - I believe they meant subset instead
		// of partition here.
		switch(subsetIdx)
		{
		default:
			return 0;
		case 1:
			return mode.NS == 2 ? AnchorTable2[partitionIdx] : AnchorTable3a[partitionIdx];
		case 2:
			return AnchorTable3b[partitionIdx];
		}
	}

	IndexInfo colorIndex(const Mode &mode, bool isAnchor,
	                     int &indexBitOffset) const
	{
		// ARB_texture_compression_bptc states:
		// "The index value for interpolating color comes from the secondary
		// index for the texel if the format has an index selection bit and its
		// value is one and from the primary index otherwise.""
		auto idx = Get(mode.IndexSelection());
		ASSERT(idx <= 1);
		bool secondary = idx == 1;
		auto numBits = secondary ? mode.IB2 : mode.IB;
		auto numReadBits = numBits - (isAnchor ? 1 : 0);
		auto index =
		    Get(secondary ? mode.SecondaryIndex(indexBitOffset, numReadBits)
		                  : mode.PrimaryIndex(indexBitOffset, numReadBits));
		indexBitOffset += numReadBits;
		return { index, numBits };
	}

	IndexInfo alphaIndex(const Mode &mode, bool isAnchor,
	                     int &indexBitOffset) const
	{
		// ARB_texture_compression_bptc states:
		// "The alpha index comes from the secondary index if the block has a
		// secondary index and the block either doesn't have an index selection
		// bit or that bit is zero and the primary index otherwise."
		auto idx = Get(mode.IndexSelection());
		ASSERT(idx <= 1);
		bool secondary = (mode.IB2 != 0) && (idx == 0);
		auto numBits = secondary ? mode.IB2 : mode.IB;
		auto numReadBits = numBits - (isAnchor ? 1 : 0);
		auto index =
		    Get(secondary ? mode.SecondaryIndex(indexBitOffset, numReadBits)
		                  : mode.PrimaryIndex(indexBitOffset, numReadBits));
		indexBitOffset += numReadBits;
		return { index, numBits };
	}

	// Assumes little-endian
	uint64_t low;
	uint64_t high;
};

}  // namespace BC7
}  // anonymous namespace

// Decodes 1 to 4 channel images to 8 bit output
bool BC_Decoder::Decode(const uint8_t *src, uint8_t *dst, int w, int h, int dstPitch, int dstBpp, int n, bool isNoAlphaU)
{
	static_assert(sizeof(BC_color) == 8, "BC_color must be 8 bytes");
	static_assert(sizeof(BC_channel) == 8, "BC_channel must be 8 bytes");
	static_assert(sizeof(BC_alpha) == 8, "BC_alpha must be 8 bytes");

	const int dx = BlockWidth * dstBpp;
	const int dy = BlockHeight * dstPitch;
	const bool isAlpha = (n == 1) && !isNoAlphaU;
	const bool isSigned = ((n == 4) || (n == 5) || (n == 6)) && !isNoAlphaU;

	switch(n)
	{
	case 1:  // BC1
		{
			const BC_color *color = reinterpret_cast<const BC_color *>(src);
			for(int y = 0; y < h; y += BlockHeight, dst += dy)
			{
				uint8_t *dstRow = dst;
				for(int x = 0; x < w; x += BlockWidth, ++color, dstRow += dx)
				{
					color->decode(dstRow, x, y, w, h, dstPitch, dstBpp, isAlpha, false);
				}
			}
		}
		break;
	case 2:  // BC2
		{
			const BC_alpha *alpha = reinterpret_cast<const BC_alpha *>(src);
			const BC_color *color = reinterpret_cast<const BC_color *>(src + 8);
			for(int y = 0; y < h; y += BlockHeight, dst += dy)
			{
				uint8_t *dstRow = dst;
				for(int x = 0; x < w; x += BlockWidth, alpha += 2, color += 2, dstRow += dx)
				{
					color->decode(dstRow, x, y, w, h, dstPitch, dstBpp, isAlpha, true);
					alpha->decode(dstRow, x, y, w, h, dstPitch, dstBpp);
				}
			}
		}
		break;
	case 3:  // BC3
		{
			const BC_channel *alpha = reinterpret_cast<const BC_channel *>(src);
			const BC_color *color = reinterpret_cast<const BC_color *>(src + 8);
			for(int y = 0; y < h; y += BlockHeight, dst += dy)
			{
				uint8_t *dstRow = dst;
				for(int x = 0; x < w; x += BlockWidth, alpha += 2, color += 2, dstRow += dx)
				{
					color->decode(dstRow, x, y, w, h, dstPitch, dstBpp, isAlpha, true);
					alpha->decode(dstRow, x, y, w, h, dstPitch, dstBpp, 3, isSigned);
				}
			}
		}
		break;
	case 4:  // BC4
		{
			const BC_channel *red = reinterpret_cast<const BC_channel *>(src);
			for(int y = 0; y < h; y += BlockHeight, dst += dy)
			{
				uint8_t *dstRow = dst;
				for(int x = 0; x < w; x += BlockWidth, ++red, dstRow += dx)
				{
					red->decode(dstRow, x, y, w, h, dstPitch, dstBpp, 0, isSigned);
				}
			}
		}
		break;
	case 5:  // BC5
		{
			const BC_channel *red = reinterpret_cast<const BC_channel *>(src);
			const BC_channel *green = reinterpret_cast<const BC_channel *>(src + 8);
			for(int y = 0; y < h; y += BlockHeight, dst += dy)
			{
				uint8_t *dstRow = dst;
				for(int x = 0; x < w; x += BlockWidth, red += 2, green += 2, dstRow += dx)
				{
					red->decode(dstRow, x, y, w, h, dstPitch, dstBpp, 0, isSigned);
					green->decode(dstRow, x, y, w, h, dstPitch, dstBpp, 1, isSigned);
				}
			}
		}
		break;
	case 6:  // BC6H
		{
			const BC6H::Block *block = reinterpret_cast<const BC6H::Block *>(src);
			for(int y = 0; y < h; y += BlockHeight, dst += dy)
			{
				uint8_t *dstRow = dst;
				for(int x = 0; x < w; x += BlockWidth, ++block, dstRow += dx)
				{
					block->decode(dstRow, x, y, w, h, dstPitch, dstBpp, isSigned);
				}
			}
		}
		break;
	case 7:  // BC7
		{
			const BC7::Block *block = reinterpret_cast<const BC7::Block *>(src);
			for(int y = 0; y < h; y += BlockHeight, dst += dy)
			{
				uint8_t *dstRow = dst;
				for(int x = 0; x < w; x += BlockWidth, ++block, dstRow += dx)
				{
					block->decode(dstRow, x, y, w, h, dstPitch);
				}
			}
		}
		break;
	default:
		return false;
	}

	return true;
}
