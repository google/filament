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

#include "ETC_Decoder.hpp"

namespace {
inline unsigned char clampByte(int value)
{
	return static_cast<unsigned char>((value < 0) ? 0 : ((value > 255) ? 255 : value));
}

inline signed char clampSByte(int value)
{
	return static_cast<signed char>((value < -128) ? -128 : ((value > 127) ? 127 : value));
}

inline short clampEAC(int value, bool isSigned)
{
	short min = isSigned ? -1023 : 0;
	short max = isSigned ? 1023 : 2047;
	return static_cast<short>(((value < min) ? min : ((value > max) ? max : value)) << 5);
}

struct bgra8
{
	unsigned char b;
	unsigned char g;
	unsigned char r;
	unsigned char a;

	inline bgra8()
	{
	}

	inline void set(int red, int green, int blue)
	{
		r = clampByte(red);
		g = clampByte(green);
		b = clampByte(blue);
	}

	inline void set(int red, int green, int blue, int alpha)
	{
		r = clampByte(red);
		g = clampByte(green);
		b = clampByte(blue);
		a = clampByte(alpha);
	}

	const bgra8 &addA(unsigned char alpha)
	{
		a = alpha;
		return *this;
	}
};

inline int extend_4to8bits(int x)
{
	return (x << 4) | x;
}

inline int extend_5to8bits(int x)
{
	return (x << 3) | (x >> 2);
}

inline int extend_6to8bits(int x)
{
	return (x << 2) | (x >> 4);
}

inline int extend_7to8bits(int x)
{
	return (x << 1) | (x >> 6);
}

struct ETC2
{
	// Decodes unsigned single or dual channel block to bytes
	static void DecodeBlock(const ETC2 **sources, unsigned char *dest, int nbChannels, int x, int y, int w, int h, int pitch, bool isSigned, bool isEAC)
	{
		if(isEAC)
		{
			for(int j = 0; j < 4 && (y + j) < h; j++)
			{
				short *sDst = reinterpret_cast<short *>(dest);
				for(int i = 0; i < 4 && (x + i) < w; i++)
				{
					for(int c = nbChannels - 1; c >= 0; c--)
					{
						sDst[i * nbChannels + c] = clampEAC(sources[c]->getSingleChannel(i, j, isSigned, true), isSigned);
					}
				}
				dest += pitch;
			}
		}
		else
		{
			if(isSigned)
			{
				signed char *sDst = reinterpret_cast<signed char *>(dest);
				for(int j = 0; j < 4 && (y + j) < h; j++)
				{
					for(int i = 0; i < 4 && (x + i) < w; i++)
					{
						for(int c = nbChannels - 1; c >= 0; c--)
						{
							sDst[i * nbChannels + c] = clampSByte(sources[c]->getSingleChannel(i, j, isSigned, false));
						}
					}
					sDst += pitch;
				}
			}
			else
			{
				for(int j = 0; j < 4 && (y + j) < h; j++)
				{
					for(int i = 0; i < 4 && (x + i) < w; i++)
					{
						for(int c = nbChannels - 1; c >= 0; c--)
						{
							dest[i * nbChannels + c] = clampByte(sources[c]->getSingleChannel(i, j, isSigned, false));
						}
					}
					dest += pitch;
				}
			}
		}
	}

	// Decodes RGB block to bgra8
	void decodeBlock(unsigned char *dest, int x, int y, int w, int h, int pitch, unsigned char alphaValues[4][4], bool punchThroughAlpha) const
	{
		bool opaqueBit = diffbit;
		bool nonOpaquePunchThroughAlpha = punchThroughAlpha && !opaqueBit;

		// Select mode
		if(diffbit || punchThroughAlpha)
		{
			int r = (R + dR);
			int g = (G + dG);
			int b = (B + dB);
			if(r < 0 || r > 31)
			{
				decodeTBlock(dest, x, y, w, h, pitch, alphaValues, nonOpaquePunchThroughAlpha);
			}
			else if(g < 0 || g > 31)
			{
				decodeHBlock(dest, x, y, w, h, pitch, alphaValues, nonOpaquePunchThroughAlpha);
			}
			else if(b < 0 || b > 31)
			{
				decodePlanarBlock(dest, x, y, w, h, pitch, alphaValues);
			}
			else
			{
				decodeDifferentialBlock(dest, x, y, w, h, pitch, alphaValues, nonOpaquePunchThroughAlpha);
			}
		}
		else
		{
			decodeIndividualBlock(dest, x, y, w, h, pitch, alphaValues, nonOpaquePunchThroughAlpha);
		}
	}

private:
	struct
	{
		union
		{
			// Individual, differential, H and T modes
			struct
			{
				union
				{
					// Individual and differential modes
					struct
					{
						union
						{
							struct  // Individual colors
							{
								unsigned char R2 : 4;
								unsigned char R1 : 4;
								unsigned char G2 : 4;
								unsigned char G1 : 4;
								unsigned char B2 : 4;
								unsigned char B1 : 4;
							};

							struct  // Differential colors
							{
								signed char dR : 3;
								unsigned char R : 5;
								signed char dG : 3;
								unsigned char G : 5;
								signed char dB : 3;
								unsigned char B : 5;
							};
						};

						bool flipbit : 1;
						bool diffbit : 1;
						unsigned char cw2 : 3;
						unsigned char cw1 : 3;
					};

					// T mode
					struct
					{
						// Byte 1
						unsigned char TR1b : 2;
						unsigned char TunusedB : 1;
						unsigned char TR1a : 2;
						unsigned char TunusedA : 3;

						// Byte 2
						unsigned char TB1 : 4;
						unsigned char TG1 : 4;

						// Byte 3
						unsigned char TG2 : 4;
						unsigned char TR2 : 4;

						// Byte 4
						unsigned char Tdb : 1;
						bool Tflipbit : 1;
						unsigned char Tda : 2;
						unsigned char TB2 : 4;
					};

					// H mode
					struct
					{
						// Byte 1
						unsigned char HG1a : 3;
						unsigned char HR1 : 4;
						unsigned char HunusedA : 1;

						// Byte 2
						unsigned char HB1b : 2;
						unsigned char HunusedC : 1;
						unsigned char HB1a : 1;
						unsigned char HG1b : 1;
						unsigned char HunusedB : 3;

						// Byte 3
						unsigned char HG2a : 3;
						unsigned char HR2 : 4;
						unsigned char HB1c : 1;

						// Byte 4
						unsigned char Hdb : 1;
						bool Hflipbit : 1;
						unsigned char Hda : 1;
						unsigned char HB2 : 4;
						unsigned char HG2b : 1;
					};
				};

				unsigned char pixelIndexMSB[2];
				unsigned char pixelIndexLSB[2];
			};

			// planar mode
			struct
			{
				// Byte 1
				unsigned char GO1 : 1;
				unsigned char RO : 6;
				unsigned char PunusedA : 1;

				// Byte 2
				unsigned char BO1 : 1;
				unsigned char GO2 : 6;
				unsigned char PunusedB : 1;

				// Byte 3
				unsigned char BO3a : 2;
				unsigned char PunusedD : 1;
				unsigned char BO2 : 2;
				unsigned char PunusedC : 3;

				// Byte 4
				unsigned char RH2 : 1;
				bool Pflipbit : 1;
				unsigned char RH1 : 5;
				unsigned char BO3b : 1;

				// Byte 5
				unsigned char BHa : 1;
				unsigned char GH : 7;

				// Byte 6
				unsigned char RVa : 3;
				unsigned char BHb : 5;

				// Byte 7
				unsigned char GVa : 5;
				unsigned char RVb : 3;

				// Byte 8
				unsigned char BV : 6;
				unsigned char GVb : 2;
			};

			// Single channel block
			struct
			{
				union
				{
					unsigned char base_codeword;
					signed char signed_base_codeword;
				};

				unsigned char table_index : 4;
				unsigned char multiplier : 4;

				unsigned char mc1 : 2;
				unsigned char mb : 3;
				unsigned char ma : 3;

				unsigned char mf1 : 1;
				unsigned char me : 3;
				unsigned char md : 3;
				unsigned char mc2 : 1;

				unsigned char mh : 3;
				unsigned char mg : 3;
				unsigned char mf2 : 2;

				unsigned char mk1 : 2;
				unsigned char mj : 3;
				unsigned char mi : 3;

				unsigned char mn1 : 1;
				unsigned char mm : 3;
				unsigned char ml : 3;
				unsigned char mk2 : 1;

				unsigned char mp : 3;
				unsigned char mo : 3;
				unsigned char mn2 : 2;
			};
		};
	};

	void decodeIndividualBlock(unsigned char *dest, int x, int y, int w, int h, int pitch, unsigned char alphaValues[4][4], bool nonOpaquePunchThroughAlpha) const
	{
		int r1 = extend_4to8bits(R1);
		int g1 = extend_4to8bits(G1);
		int b1 = extend_4to8bits(B1);

		int r2 = extend_4to8bits(R2);
		int g2 = extend_4to8bits(G2);
		int b2 = extend_4to8bits(B2);

		decodeIndividualOrDifferentialBlock(dest, x, y, w, h, pitch, r1, g1, b1, r2, g2, b2, alphaValues, nonOpaquePunchThroughAlpha);
	}

	void decodeDifferentialBlock(unsigned char *dest, int x, int y, int w, int h, int pitch, unsigned char alphaValues[4][4], bool nonOpaquePunchThroughAlpha) const
	{
		int b1 = extend_5to8bits(B);
		int g1 = extend_5to8bits(G);
		int r1 = extend_5to8bits(R);

		int r2 = extend_5to8bits(R + dR);
		int g2 = extend_5to8bits(G + dG);
		int b2 = extend_5to8bits(B + dB);

		decodeIndividualOrDifferentialBlock(dest, x, y, w, h, pitch, r1, g1, b1, r2, g2, b2, alphaValues, nonOpaquePunchThroughAlpha);
	}

	void decodeIndividualOrDifferentialBlock(unsigned char *dest, int x, int y, int w, int h, int pitch, int r1, int g1, int b1, int r2, int g2, int b2, unsigned char alphaValues[4][4], bool nonOpaquePunchThroughAlpha) const
	{
		// Table 3.17.2 sorted according to table 3.17.3
		static const int intensityModifierDefault[8][4] = {
			{ 2, 8, -2, -8 },
			{ 5, 17, -5, -17 },
			{ 9, 29, -9, -29 },
			{ 13, 42, -13, -42 },
			{ 18, 60, -18, -60 },
			{ 24, 80, -24, -80 },
			{ 33, 106, -33, -106 },
			{ 47, 183, -47, -183 }
		};

		// Table C.12, intensity modifier for non opaque punchthrough alpha
		static const int intensityModifierNonOpaque[8][4] = {
			{ 0, 8, 0, -8 },
			{ 0, 17, 0, -17 },
			{ 0, 29, 0, -29 },
			{ 0, 42, 0, -42 },
			{ 0, 60, 0, -60 },
			{ 0, 80, 0, -80 },
			{ 0, 106, 0, -106 },
			{ 0, 183, 0, -183 }
		};

		const int(&intensityModifier)[8][4] = nonOpaquePunchThroughAlpha ? intensityModifierNonOpaque : intensityModifierDefault;

		bgra8 subblockColors0[4];
		bgra8 subblockColors1[4];

		const int i10 = intensityModifier[cw1][0];
		const int i11 = intensityModifier[cw1][1];
		const int i12 = intensityModifier[cw1][2];
		const int i13 = intensityModifier[cw1][3];

		subblockColors0[0].set(r1 + i10, g1 + i10, b1 + i10);
		subblockColors0[1].set(r1 + i11, g1 + i11, b1 + i11);
		subblockColors0[2].set(r1 + i12, g1 + i12, b1 + i12);
		subblockColors0[3].set(r1 + i13, g1 + i13, b1 + i13);

		const int i20 = intensityModifier[cw2][0];
		const int i21 = intensityModifier[cw2][1];
		const int i22 = intensityModifier[cw2][2];
		const int i23 = intensityModifier[cw2][3];

		subblockColors1[0].set(r2 + i20, g2 + i20, b2 + i20);
		subblockColors1[1].set(r2 + i21, g2 + i21, b2 + i21);
		subblockColors1[2].set(r2 + i22, g2 + i22, b2 + i22);
		subblockColors1[3].set(r2 + i23, g2 + i23, b2 + i23);

		unsigned char *destStart = dest;

		if(flipbit)
		{
			for(int j = 0; j < 2 && (y + j) < h; j++)
			{
				bgra8 *color = (bgra8 *)dest;
				if((x + 0) < w) color[0] = subblockColors0[getIndex(0, j)].addA(alphaValues[j][0]);
				if((x + 1) < w) color[1] = subblockColors0[getIndex(1, j)].addA(alphaValues[j][1]);
				if((x + 2) < w) color[2] = subblockColors0[getIndex(2, j)].addA(alphaValues[j][2]);
				if((x + 3) < w) color[3] = subblockColors0[getIndex(3, j)].addA(alphaValues[j][3]);
				dest += pitch;
			}

			for(int j = 2; j < 4 && (y + j) < h; j++)
			{
				bgra8 *color = (bgra8 *)dest;
				if((x + 0) < w) color[0] = subblockColors1[getIndex(0, j)].addA(alphaValues[j][0]);
				if((x + 1) < w) color[1] = subblockColors1[getIndex(1, j)].addA(alphaValues[j][1]);
				if((x + 2) < w) color[2] = subblockColors1[getIndex(2, j)].addA(alphaValues[j][2]);
				if((x + 3) < w) color[3] = subblockColors1[getIndex(3, j)].addA(alphaValues[j][3]);
				dest += pitch;
			}
		}
		else
		{
			for(int j = 0; j < 4 && (y + j) < h; j++)
			{
				bgra8 *color = (bgra8 *)dest;
				if((x + 0) < w) color[0] = subblockColors0[getIndex(0, j)].addA(alphaValues[j][0]);
				if((x + 1) < w) color[1] = subblockColors0[getIndex(1, j)].addA(alphaValues[j][1]);
				if((x + 2) < w) color[2] = subblockColors1[getIndex(2, j)].addA(alphaValues[j][2]);
				if((x + 3) < w) color[3] = subblockColors1[getIndex(3, j)].addA(alphaValues[j][3]);
				dest += pitch;
			}
		}

		if(nonOpaquePunchThroughAlpha)
		{
			decodePunchThroughAlphaBlock(destStart, x, y, w, h, pitch);
		}
	}

	void decodeTBlock(unsigned char *dest, int x, int y, int w, int h, int pitch, unsigned char alphaValues[4][4], bool nonOpaquePunchThroughAlpha) const
	{
		// Table C.8, distance index fot T and H modes
		static const int distance[8] = { 3, 6, 11, 16, 23, 32, 41, 64 };

		bgra8 paintColors[4];

		int r1 = extend_4to8bits(TR1a << 2 | TR1b);
		int g1 = extend_4to8bits(TG1);
		int b1 = extend_4to8bits(TB1);

		int r2 = extend_4to8bits(TR2);
		int g2 = extend_4to8bits(TG2);
		int b2 = extend_4to8bits(TB2);

		const int d = distance[Tda << 1 | Tdb];

		paintColors[0].set(r1, g1, b1);
		paintColors[1].set(r2 + d, g2 + d, b2 + d);
		paintColors[2].set(r2, g2, b2);
		paintColors[3].set(r2 - d, g2 - d, b2 - d);

		unsigned char *destStart = dest;

		for(int j = 0; j < 4 && (y + j) < h; j++)
		{
			bgra8 *color = (bgra8 *)dest;
			if((x + 0) < w) color[0] = paintColors[getIndex(0, j)].addA(alphaValues[j][0]);
			if((x + 1) < w) color[1] = paintColors[getIndex(1, j)].addA(alphaValues[j][1]);
			if((x + 2) < w) color[2] = paintColors[getIndex(2, j)].addA(alphaValues[j][2]);
			if((x + 3) < w) color[3] = paintColors[getIndex(3, j)].addA(alphaValues[j][3]);
			dest += pitch;
		}

		if(nonOpaquePunchThroughAlpha)
		{
			decodePunchThroughAlphaBlock(destStart, x, y, w, h, pitch);
		}
	}

	void decodeHBlock(unsigned char *dest, int x, int y, int w, int h, int pitch, unsigned char alphaValues[4][4], bool nonOpaquePunchThroughAlpha) const
	{
		// Table C.8, distance index fot T and H modes
		static const int distance[8] = { 3, 6, 11, 16, 23, 32, 41, 64 };

		bgra8 paintColors[4];

		int r1 = extend_4to8bits(HR1);
		int g1 = extend_4to8bits(HG1a << 1 | HG1b);
		int b1 = extend_4to8bits(HB1a << 3 | HB1b << 1 | HB1c);

		int r2 = extend_4to8bits(HR2);
		int g2 = extend_4to8bits(HG2a << 1 | HG2b);
		int b2 = extend_4to8bits(HB2);

		const int d = distance[(Hda << 2) | (Hdb << 1) | ((r1 << 16 | g1 << 8 | b1) >= (r2 << 16 | g2 << 8 | b2) ? 1 : 0)];

		paintColors[0].set(r1 + d, g1 + d, b1 + d);
		paintColors[1].set(r1 - d, g1 - d, b1 - d);
		paintColors[2].set(r2 + d, g2 + d, b2 + d);
		paintColors[3].set(r2 - d, g2 - d, b2 - d);

		unsigned char *destStart = dest;

		for(int j = 0; j < 4 && (y + j) < h; j++)
		{
			bgra8 *color = (bgra8 *)dest;
			if((x + 0) < w) color[0] = paintColors[getIndex(0, j)].addA(alphaValues[j][0]);
			if((x + 1) < w) color[1] = paintColors[getIndex(1, j)].addA(alphaValues[j][1]);
			if((x + 2) < w) color[2] = paintColors[getIndex(2, j)].addA(alphaValues[j][2]);
			if((x + 3) < w) color[3] = paintColors[getIndex(3, j)].addA(alphaValues[j][3]);
			dest += pitch;
		}

		if(nonOpaquePunchThroughAlpha)
		{
			decodePunchThroughAlphaBlock(destStart, x, y, w, h, pitch);
		}
	}

	void decodePlanarBlock(unsigned char *dest, int x, int y, int w, int h, int pitch, unsigned char alphaValues[4][4]) const
	{
		int ro = extend_6to8bits(RO);
		int go = extend_7to8bits(GO1 << 6 | GO2);
		int bo = extend_6to8bits(BO1 << 5 | BO2 << 3 | BO3a << 1 | BO3b);

		int rh = extend_6to8bits(RH1 << 1 | RH2);
		int gh = extend_7to8bits(GH);
		int bh = extend_6to8bits(BHa << 5 | BHb);

		int rv = extend_6to8bits(RVa << 3 | RVb);
		int gv = extend_7to8bits(GVa << 2 | GVb);
		int bv = extend_6to8bits(BV);

		for(int j = 0; j < 4 && (y + j) < h; j++)
		{
			int ry = j * (rv - ro) + 2;
			int gy = j * (gv - go) + 2;
			int by = j * (bv - bo) + 2;
			for(int i = 0; i < 4 && (x + i) < w; i++)
			{
				((bgra8 *)(dest))[i].set(((i * (rh - ro) + ry) >> 2) + ro,
				                         ((i * (gh - go) + gy) >> 2) + go,
				                         ((i * (bh - bo) + by) >> 2) + bo,
				                         alphaValues[j][i]);
			}
			dest += pitch;
		}
	}

	// Index for individual, differential, H and T modes
	inline int getIndex(int x, int y) const
	{
		int bitIndex = x * 4 + y;
		int bitOffset = bitIndex & 7;
		int lsb = (pixelIndexLSB[1 - (bitIndex >> 3)] >> bitOffset) & 1;
		int msb = (pixelIndexMSB[1 - (bitIndex >> 3)] >> bitOffset) & 1;

		return (msb << 1) | lsb;
	}

	void decodePunchThroughAlphaBlock(unsigned char *dest, int x, int y, int w, int h, int pitch) const
	{
		for(int j = 0; j < 4 && (y + j) < h; j++)
		{
			for(int i = 0; i < 4 && (x + i) < w; i++)
			{
				if(getIndex(i, j) == 2)  //  msb == 1 && lsb == 0
				{
					((bgra8 *)dest)[i].set(0, 0, 0, 0);
				}
			}
			dest += pitch;
		}
	}

	// Single channel utility functions
	inline int getSingleChannel(int x, int y, bool isSigned, bool isEAC) const
	{
		int codeword = isSigned ? signed_base_codeword : base_codeword;
		return isEAC ? ((multiplier == 0) ? (codeword * 8 + 4 + getSingleChannelModifier(x, y)) : (codeword * 8 + 4 + getSingleChannelModifier(x, y) * multiplier * 8)) : codeword + getSingleChannelModifier(x, y) * multiplier;
	}

	inline int getSingleChannelIndex(int x, int y) const
	{
		switch(x * 4 + y)
		{
		case 0: return ma;
		case 1: return mb;
		case 2: return mc1 << 1 | mc2;
		case 3: return md;
		case 4: return me;
		case 5: return mf1 << 2 | mf2;
		case 6: return mg;
		case 7: return mh;
		case 8: return mi;
		case 9: return mj;
		case 10: return mk1 << 1 | mk2;
		case 11: return ml;
		case 12: return mm;
		case 13: return mn1 << 2 | mn2;
		case 14: return mo;
		default: return mp;  // 15
		}
	}

	inline int getSingleChannelModifier(int x, int y) const
	{
		static const int modifierTable[16][8] = { { -3, -6, -9, -15, 2, 5, 8, 14 },
			                                      { -3, -7, -10, -13, 2, 6, 9, 12 },
			                                      { -2, -5, -8, -13, 1, 4, 7, 12 },
			                                      { -2, -4, -6, -13, 1, 3, 5, 12 },
			                                      { -3, -6, -8, -12, 2, 5, 7, 11 },
			                                      { -3, -7, -9, -11, 2, 6, 8, 10 },
			                                      { -4, -7, -8, -11, 3, 6, 7, 10 },
			                                      { -3, -5, -8, -11, 2, 4, 7, 10 },
			                                      { -2, -6, -8, -10, 1, 5, 7, 9 },
			                                      { -2, -5, -8, -10, 1, 4, 7, 9 },
			                                      { -2, -4, -8, -10, 1, 3, 7, 9 },
			                                      { -2, -5, -7, -10, 1, 4, 6, 9 },
			                                      { -3, -4, -7, -10, 2, 3, 6, 9 },
			                                      { -1, -2, -3, -10, 0, 1, 2, 9 },
			                                      { -4, -6, -8, -9, 3, 5, 7, 8 },
			                                      { -3, -5, -7, -9, 2, 4, 6, 8 } };

		return modifierTable[table_index][getSingleChannelIndex(x, y)];
	}
};
}  // namespace

// Decodes 1 to 4 channel images to 8 bit output
bool ETC_Decoder::Decode(const unsigned char *src, unsigned char *dst, int w, int h, int dstPitch, int dstBpp, InputType inputType)
{
	const ETC2 *sources[2];
	sources[0] = (const ETC2 *)src;

	unsigned char alphaValues[4][4] = { { 255, 255, 255, 255 }, { 255, 255, 255, 255 }, { 255, 255, 255, 255 }, { 255, 255, 255, 255 } };

	switch(inputType)
	{
	case ETC_R_SIGNED:
	case ETC_R_UNSIGNED:
		for(int y = 0; y < h; y += 4)
		{
			unsigned char *dstRow = dst + (y * dstPitch);
			for(int x = 0; x < w; x += 4, sources[0]++)
			{
				ETC2::DecodeBlock(sources, dstRow + (x * dstBpp), 1, x, y, w, h, dstPitch, inputType == ETC_R_SIGNED, true);
			}
		}
		break;
	case ETC_RG_SIGNED:
	case ETC_RG_UNSIGNED:
		sources[1] = sources[0] + 1;
		for(int y = 0; y < h; y += 4)
		{
			unsigned char *dstRow = dst + (y * dstPitch);
			for(int x = 0; x < w; x += 4, sources[0] += 2, sources[1] += 2)
			{
				ETC2::DecodeBlock(sources, dstRow + (x * dstBpp), 2, x, y, w, h, dstPitch, inputType == ETC_RG_SIGNED, true);
			}
		}
		break;
	case ETC_RGB:
	case ETC_RGB_PUNCHTHROUGH_ALPHA:
		for(int y = 0; y < h; y += 4)
		{
			unsigned char *dstRow = dst + (y * dstPitch);
			for(int x = 0; x < w; x += 4, sources[0]++)
			{
				sources[0]->decodeBlock(dstRow + (x * dstBpp), x, y, w, h, dstPitch, alphaValues, inputType == ETC_RGB_PUNCHTHROUGH_ALPHA);
			}
		}
		break;
	case ETC_RGBA:
		for(int y = 0; y < h; y += 4)
		{
			unsigned char *dstRow = dst + (y * dstPitch);
			for(int x = 0; x < w; x += 4)
			{
				// Decode Alpha
				ETC2::DecodeBlock(&sources[0], &(alphaValues[0][0]), 1, x, y, w, h, 4, false, false);
				sources[0]++;  // RGBA packets are 128 bits, so move on to the next 64 bit packet to decode the RGB color

				// Decode RGB
				sources[0]->decodeBlock(dstRow + (x * dstBpp), x, y, w, h, dstPitch, alphaValues, false);
				sources[0]++;
			}
		}
		break;
	default:
		return false;
	}

	return true;
}
