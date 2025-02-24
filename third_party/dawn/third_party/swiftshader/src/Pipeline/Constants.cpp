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

#include "Constants.hpp"

#include "System/Half.hpp"
#include "System/Math.hpp"

#include <cstring>

namespace sw {

Constants::Constants()
{
	static const unsigned int transposeBit0[16] = {
		0x00000000,
		0x00000001,
		0x00000010,
		0x00000011,
		0x00000100,
		0x00000101,
		0x00000110,
		0x00000111,
		0x00001000,
		0x00001001,
		0x00001010,
		0x00001011,
		0x00001100,
		0x00001101,
		0x00001110,
		0x00001111
	};

	static const unsigned int transposeBit1[16] = {
		0x00000000,
		0x00000002,
		0x00000020,
		0x00000022,
		0x00000200,
		0x00000202,
		0x00000220,
		0x00000222,
		0x00002000,
		0x00002002,
		0x00002020,
		0x00002022,
		0x00002200,
		0x00002202,
		0x00002220,
		0x00002222
	};

	static const unsigned int transposeBit2[16] = {
		0x00000000,
		0x00000004,
		0x00000040,
		0x00000044,
		0x00000400,
		0x00000404,
		0x00000440,
		0x00000444,
		0x00004000,
		0x00004004,
		0x00004040,
		0x00004044,
		0x00004400,
		0x00004404,
		0x00004440,
		0x00004444
	};

	memcpy(&this->transposeBit0, transposeBit0, sizeof(transposeBit0));
	memcpy(&this->transposeBit1, transposeBit1, sizeof(transposeBit1));
	memcpy(&this->transposeBit2, transposeBit2, sizeof(transposeBit2));

	static const ushort4 cWeight[17] = {
		{ 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF },  // 0xFFFF / 1  = 0xFFFF
		{ 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF },  // 0xFFFF / 1  = 0xFFFF
		{ 0x8000, 0x8000, 0x8000, 0x8000 },  // 0xFFFF / 2  = 0x8000
		{ 0x5555, 0x5555, 0x5555, 0x5555 },  // 0xFFFF / 3  = 0x5555
		{ 0x4000, 0x4000, 0x4000, 0x4000 },  // 0xFFFF / 4  = 0x4000
		{ 0x3333, 0x3333, 0x3333, 0x3333 },  // 0xFFFF / 5  = 0x3333
		{ 0x2AAA, 0x2AAA, 0x2AAA, 0x2AAA },  // 0xFFFF / 6  = 0x2AAA
		{ 0x2492, 0x2492, 0x2492, 0x2492 },  // 0xFFFF / 7  = 0x2492
		{ 0x2000, 0x2000, 0x2000, 0x2000 },  // 0xFFFF / 8  = 0x2000
		{ 0x1C71, 0x1C71, 0x1C71, 0x1C71 },  // 0xFFFF / 9  = 0x1C71
		{ 0x1999, 0x1999, 0x1999, 0x1999 },  // 0xFFFF / 10 = 0x1999
		{ 0x1745, 0x1745, 0x1745, 0x1745 },  // 0xFFFF / 11 = 0x1745
		{ 0x1555, 0x1555, 0x1555, 0x1555 },  // 0xFFFF / 12 = 0x1555
		{ 0x13B1, 0x13B1, 0x13B1, 0x13B1 },  // 0xFFFF / 13 = 0x13B1
		{ 0x1249, 0x1249, 0x1249, 0x1249 },  // 0xFFFF / 14 = 0x1249
		{ 0x1111, 0x1111, 0x1111, 0x1111 },  // 0xFFFF / 15 = 0x1111
		{ 0x1000, 0x1000, 0x1000, 0x1000 },  // 0xFFFF / 16 = 0x1000
	};

	static const float4 uvWeight[17] = {
		{ 1.0f / 1.0f, 1.0f / 1.0f, 1.0f / 1.0f, 1.0f / 1.0f },
		{ 1.0f / 1.0f, 1.0f / 1.0f, 1.0f / 1.0f, 1.0f / 1.0f },
		{ 1.0f / 2.0f, 1.0f / 2.0f, 1.0f / 2.0f, 1.0f / 2.0f },
		{ 1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f, 1.0f / 3.0f },
		{ 1.0f / 4.0f, 1.0f / 4.0f, 1.0f / 4.0f, 1.0f / 4.0f },
		{ 1.0f / 5.0f, 1.0f / 5.0f, 1.0f / 5.0f, 1.0f / 5.0f },
		{ 1.0f / 6.0f, 1.0f / 6.0f, 1.0f / 6.0f, 1.0f / 6.0f },
		{ 1.0f / 7.0f, 1.0f / 7.0f, 1.0f / 7.0f, 1.0f / 7.0f },
		{ 1.0f / 8.0f, 1.0f / 8.0f, 1.0f / 8.0f, 1.0f / 8.0f },
		{ 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f },
		{ 1.0f / 10.0f, 1.0f / 10.0f, 1.0f / 10.0f, 1.0f / 10.0f },
		{ 1.0f / 11.0f, 1.0f / 11.0f, 1.0f / 11.0f, 1.0f / 11.0f },
		{ 1.0f / 12.0f, 1.0f / 12.0f, 1.0f / 12.0f, 1.0f / 12.0f },
		{ 1.0f / 13.0f, 1.0f / 13.0f, 1.0f / 13.0f, 1.0f / 13.0f },
		{ 1.0f / 14.0f, 1.0f / 14.0f, 1.0f / 14.0f, 1.0f / 14.0f },
		{ 1.0f / 15.0f, 1.0f / 15.0f, 1.0f / 15.0f, 1.0f / 15.0f },
		{ 1.0f / 16.0f, 1.0f / 16.0f, 1.0f / 16.0f, 1.0f / 16.0f },
	};

	static const float4 uvStart[17] = {
		{ -0.0f / 2.0f, -0.0f / 2.0f, -0.0f / 2.0f, -0.0f / 2.0f },
		{ -0.0f / 2.0f, -0.0f / 2.0f, -0.0f / 2.0f, -0.0f / 2.0f },
		{ -1.0f / 4.0f, -1.0f / 4.0f, -1.0f / 4.0f, -1.0f / 4.0f },
		{ -2.0f / 6.0f, -2.0f / 6.0f, -2.0f / 6.0f, -2.0f / 6.0f },
		{ -3.0f / 8.0f, -3.0f / 8.0f, -3.0f / 8.0f, -3.0f / 8.0f },
		{ -4.0f / 10.0f, -4.0f / 10.0f, -4.0f / 10.0f, -4.0f / 10.0f },
		{ -5.0f / 12.0f, -5.0f / 12.0f, -5.0f / 12.0f, -5.0f / 12.0f },
		{ -6.0f / 14.0f, -6.0f / 14.0f, -6.0f / 14.0f, -6.0f / 14.0f },
		{ -7.0f / 16.0f, -7.0f / 16.0f, -7.0f / 16.0f, -7.0f / 16.0f },
		{ -8.0f / 18.0f, -8.0f / 18.0f, -8.0f / 18.0f, -8.0f / 18.0f },
		{ -9.0f / 20.0f, -9.0f / 20.0f, -9.0f / 20.0f, -9.0f / 20.0f },
		{ -10.0f / 22.0f, -10.0f / 22.0f, -10.0f / 22.0f, -10.0f / 22.0f },
		{ -11.0f / 24.0f, -11.0f / 24.0f, -11.0f / 24.0f, -11.0f / 24.0f },
		{ -12.0f / 26.0f, -12.0f / 26.0f, -12.0f / 26.0f, -12.0f / 26.0f },
		{ -13.0f / 28.0f, -13.0f / 28.0f, -13.0f / 28.0f, -13.0f / 28.0f },
		{ -14.0f / 30.0f, -14.0f / 30.0f, -14.0f / 30.0f, -14.0f / 30.0f },
		{ -15.0f / 32.0f, -15.0f / 32.0f, -15.0f / 32.0f, -15.0f / 32.0f },
	};

	memcpy(&this->cWeight, cWeight, sizeof(cWeight));
	memcpy(&this->uvWeight, uvWeight, sizeof(uvWeight));
	memcpy(&this->uvStart, uvStart, sizeof(uvStart));

	static const unsigned int occlusionCount[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

	memcpy(&this->occlusionCount, &occlusionCount, sizeof(occlusionCount));

	for(int i = 0; i < 16; i++)
	{
		maskB4Q[i][0] = -(i >> 0 & 1);
		maskB4Q[i][1] = -(i >> 1 & 1);
		maskB4Q[i][2] = -(i >> 2 & 1);
		maskB4Q[i][3] = -(i >> 3 & 1);
		maskB4Q[i][4] = -(i >> 0 & 1);
		maskB4Q[i][5] = -(i >> 1 & 1);
		maskB4Q[i][6] = -(i >> 2 & 1);
		maskB4Q[i][7] = -(i >> 3 & 1);

		invMaskB4Q[i][0] = ~maskB4Q[i][0];
		invMaskB4Q[i][1] = ~maskB4Q[i][1];
		invMaskB4Q[i][2] = ~maskB4Q[i][2];
		invMaskB4Q[i][3] = ~maskB4Q[i][3];
		invMaskB4Q[i][4] = ~maskB4Q[i][4];
		invMaskB4Q[i][5] = ~maskB4Q[i][5];
		invMaskB4Q[i][6] = ~maskB4Q[i][6];
		invMaskB4Q[i][7] = ~maskB4Q[i][7];

		maskW4Q[i][0] = -(i >> 0 & 1);
		maskW4Q[i][1] = -(i >> 1 & 1);
		maskW4Q[i][2] = -(i >> 2 & 1);
		maskW4Q[i][3] = -(i >> 3 & 1);

		invMaskW4Q[i][0] = ~maskW4Q[i][0];
		invMaskW4Q[i][1] = ~maskW4Q[i][1];
		invMaskW4Q[i][2] = ~maskW4Q[i][2];
		invMaskW4Q[i][3] = ~maskW4Q[i][3];

		maskD4X[i][0] = -(i >> 0 & 1);
		maskD4X[i][1] = -(i >> 1 & 1);
		maskD4X[i][2] = -(i >> 2 & 1);
		maskD4X[i][3] = -(i >> 3 & 1);

		invMaskD4X[i][0] = ~maskD4X[i][0];
		invMaskD4X[i][1] = ~maskD4X[i][1];
		invMaskD4X[i][2] = ~maskD4X[i][2];
		invMaskD4X[i][3] = ~maskD4X[i][3];

		maskQ0Q[i] = -(i >> 0 & 1);
		maskQ1Q[i] = -(i >> 1 & 1);
		maskQ2Q[i] = -(i >> 2 & 1);
		maskQ3Q[i] = -(i >> 3 & 1);

		invMaskQ0Q[i] = ~maskQ0Q[i];
		invMaskQ1Q[i] = ~maskQ1Q[i];
		invMaskQ2Q[i] = ~maskQ2Q[i];
		invMaskQ3Q[i] = ~maskQ3Q[i];

		maskX0X[i][0] = maskX0X[i][1] = maskX0X[i][2] = maskX0X[i][3] = -(i >> 0 & 1);
		maskX1X[i][0] = maskX1X[i][1] = maskX1X[i][2] = maskX1X[i][3] = -(i >> 1 & 1);
		maskX2X[i][0] = maskX2X[i][1] = maskX2X[i][2] = maskX2X[i][3] = -(i >> 2 & 1);
		maskX3X[i][0] = maskX3X[i][1] = maskX3X[i][2] = maskX3X[i][3] = -(i >> 3 & 1);

		invMaskX0X[i][0] = invMaskX0X[i][1] = invMaskX0X[i][2] = invMaskX0X[i][3] = ~maskX0X[i][0];
		invMaskX1X[i][0] = invMaskX1X[i][1] = invMaskX1X[i][2] = invMaskX1X[i][3] = ~maskX1X[i][0];
		invMaskX2X[i][0] = invMaskX2X[i][1] = invMaskX2X[i][2] = invMaskX2X[i][3] = ~maskX2X[i][0];
		invMaskX3X[i][0] = invMaskX3X[i][1] = invMaskX3X[i][2] = invMaskX3X[i][3] = ~maskX3X[i][0];

		maskD01Q[i][0] = -(i >> 0 & 1);
		maskD01Q[i][1] = -(i >> 1 & 1);
		maskD23Q[i][0] = -(i >> 2 & 1);
		maskD23Q[i][1] = -(i >> 3 & 1);

		invMaskD01Q[i][0] = ~maskD01Q[i][0];
		invMaskD01Q[i][1] = ~maskD01Q[i][1];
		invMaskD23Q[i][0] = ~maskD23Q[i][0];
		invMaskD23Q[i][1] = ~maskD23Q[i][1];

		maskQ01X[i][0] = -(i >> 0 & 1);
		maskQ01X[i][1] = -(i >> 1 & 1);
		maskQ23X[i][0] = -(i >> 2 & 1);
		maskQ23X[i][1] = -(i >> 3 & 1);

		invMaskQ01X[i][0] = ~maskQ01X[i][0];
		invMaskQ01X[i][1] = ~maskQ01X[i][1];
		invMaskQ23X[i][0] = ~maskQ23X[i][0];
		invMaskQ23X[i][1] = ~maskQ23X[i][1];
	}

	for(int i = 0; i < 8; i++)
	{
		mask565Q[i] = word4((i & 0x1 ? 0x001F : 0) | (i & 0x2 ? 0x07E0 : 0) | (i & 0x4 ? 0xF800 : 0));
		mask11X[i] = dword4((i & 0x1 ? 0x000007FFu : 0) | (i & 0x2 ? 0x003FF800u : 0) | (i & 0x4 ? 0xFFC00000u : 0));
	}

	for(int i = 0; i < 16; i++)
	{
		mask5551Q[i] = word4((i & 0x1 ? 0x001F : 0) | (i & 0x2 ? 0x03E0 : 0) | (i & 0x4 ? 0x7C00 : 0) | (i & 8 ? 0x8000 : 0));
		maskr5g5b5a1Q[i] = word4((i & 0x1 ? 0xF800 : 0) | (i & 0x2 ? 0x07C0 : 0) | (i & 0x4 ? 0x003E : 0) | (i & 8 ? 0x0001 : 0));
		maskb5g5r5a1Q[i] = word4((i & 0x1 ? 0x003E : 0) | (i & 0x2 ? 0x07C0 : 0) | (i & 0x4 ? 0xF800 : 0) | (i & 8 ? 0x0001 : 0));
		mask4argbQ[i] = word4((i & 0x1 ? 0x00F0 : 0) | (i & 0x2 ? 0x0F00 : 0) | (i & 0x4 ? 0xF000 : 0) | (i & 8 ? 0x000F : 0));
		mask4rgbaQ[i] = word4((i & 0x1 ? 0x000F : 0) | (i & 0x2 ? 0x00F0 : 0) | (i & 0x4 ? 0x0F00 : 0) | (i & 8 ? 0xF000 : 0));
	}

	for(int i = 0; i < 4; i++)
	{
		maskW01Q[i][0] = -(i >> 0 & 1);
		maskW01Q[i][1] = -(i >> 1 & 1);
		maskW01Q[i][2] = -(i >> 0 & 1);
		maskW01Q[i][3] = -(i >> 1 & 1);

		maskD01X[i][0] = -(i >> 0 & 1);
		maskD01X[i][1] = -(i >> 1 & 1);
		maskD01X[i][2] = -(i >> 0 & 1);
		maskD01X[i][3] = -(i >> 1 & 1);
	}

	for(int i = 0; i < 16; i++)
	{
		mask10Q[i][0] = mask10Q[i][1] =
		    (i & 0x1 ? 0x3FF : 0) |
		    (i & 0x2 ? 0xFFC00 : 0) |
		    (i & 0x4 ? 0x3FF00000 : 0) |
		    (i & 0x8 ? 0xC0000000 : 0);
	}

	for(int i = 0; i < 256; i++)
	{
		sRGBtoLinearFF_FF00[i] = (unsigned short)(sRGBtoLinear((float)i / 0xFF) * 0xFF00 + 0.5f);
	}

	for(int q = 0; q < 4; q++)
	{
		for(int c = 0; c < 16; c++)
		{
			for(int i = 0; i < 4; i++)
			{
				sampleX[q][c][i] = c & (1 << i) ? SampleLocationsX[q] : 0.0f;
				sampleY[q][c][i] = c & (1 << i) ? SampleLocationsY[q] : 0.0f;
				weight[c][i] = c & (1 << i) ? 1.0f : 0.0f;
			}
		}
	}

	constexpr auto subPixB = vk::SUBPIXEL_PRECISION_BITS;

	const int Xf[4] = { toFixedPoint(SampleLocationsX[0], subPixB), toFixedPoint(SampleLocationsX[1], subPixB), toFixedPoint(SampleLocationsX[2], subPixB), toFixedPoint(SampleLocationsX[3], subPixB) };
	const int Yf[4] = { toFixedPoint(SampleLocationsY[0], subPixB), toFixedPoint(SampleLocationsY[1], subPixB), toFixedPoint(SampleLocationsY[2], subPixB), toFixedPoint(SampleLocationsY[3], subPixB) };

	memcpy(&this->Xf, &Xf, sizeof(Xf));
	memcpy(&this->Yf, &Yf, sizeof(Yf));

	for(int i = 0; i <= 0xFFFF; i++)
	{
		half2float[i] = static_cast<float>(bit_cast<half>(i));
	}
}

}  // namespace sw
