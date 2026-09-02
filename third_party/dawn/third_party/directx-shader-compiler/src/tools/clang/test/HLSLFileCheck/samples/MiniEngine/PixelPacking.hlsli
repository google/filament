//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

// RGBM is a good way to pack HDR values into R8G8B8A8_UNORM
uint PackRGBM( float3 rgb, float PeakValue = 16.0 )
{
	rgb = saturate(rgb / PeakValue);
	float maxVal = max(max(1e-6, rgb.x), max(rgb.y, rgb.z));
	maxVal = ceil(maxVal * 255.0);
	float divisor = (255 * 255.0) / maxVal;
#if _XBOX_ONE
	uint RGBM = (uint)maxVal;
	RGBM = __XB_PackF32ToU8(rgb.r * divisor + 0.5, 3, RGBM);
	RGBM = __XB_PackF32ToU8(rgb.g * divisor + 0.5, 2, RGBM);
	RGBM = __XB_PackF32ToU8(rgb.b * divisor + 0.5, 1, RGBM);
	return RGBM;
#else
	uint M = (uint)maxVal;
	uint R = (uint)(rgb.r * divisor + 0.5);
	uint G = (uint)(rgb.g * divisor + 0.5);
	uint B = (uint)(rgb.b * divisor + 0.5);
	return R << 24 | G << 16 | B << 8 | M;
#endif
}

float3 UnpackRGBM( uint p, float PeakValue = 16.0 )
{
#if _XBOX_ONE
	float R = __XB_UnpackByte3(p);
	float G = __XB_UnpackByte2(p);
	float B = __XB_UnpackByte1(p);
	float M = __XB_UnpackByte0(p);
#else
	uint R = p >> 24;
	uint G = (p >> 16) & 0xFF;
	uint B = (p >> 8) & 0xFF;
	uint M = p & 0xFF;
#endif
	return float3(R, G, B) * M * PeakValue / (255.0 * 255.0);
}

// RGBE packs 9 bits per color channel while encoding the multiplier as a perfect power of 2 (just the exponent)
// What's nice about this is that it gives you a lot more range than RGBM.  This isn't proven to be bitwise
// compatible with DXGI_FORMAT_R9B9G9E5_SHAREDEXP, but if it's not, it could be made so.
uint PackRGBE(float3 rgb)
{
	float MaxChannel = max(rgb.r, max(rgb.g, rgb.b));

	// NextPow2 has to have the biggest exponent plus 1 (and nothing in the mantissa)
	float NextPow2 = asfloat((asuint(MaxChannel) + 0x800000) & 0x7F800000);

	// By adding NextPow2, all channels have the same exponent, shifting their mantissa bits
	// to the right to accomodate it.  This also shifts in the implicit '1' bit of all channels.
	// The largest channel will always have the high bit set.
	rgb += NextPow2;

#if _XBOX_ONE
	uint R = __XB_UBFE(9, 14, asuint(rgb.r));
	uint G = __XB_UBFE(9, 14, asuint(rgb.g));
	uint B = __XB_UBFE(9, 14, asuint(rgb.b));
#else
	uint R = (asuint(rgb.r) << 9) >> 23;
	uint G = (asuint(rgb.g) << 9) >> 23;
	uint B = (asuint(rgb.b) << 9) >> 23;
#endif
	uint E = f32tof16(NextPow2) << 17;
	return R | G << 9 | B << 18 | E;
}

float3 UnpackRGBE(uint p)
{
#if _XBOX_ONE
	float Pow2 = f16tof32(__XB_UBFE(5, 27, p) << 10);
	float R = asfloat(asuint(Pow2) | __XB_UBFE(9, 0, p) << 14);
	float G = asfloat(asuint(Pow2) | __XB_UBFE(9, 9, p) << 14);
	float B = asfloat(asuint(Pow2) | __XB_UBFE(9, 18, p) << 14);
#else
	float Pow2 = f16tof32((p >> 27) << 10);
	float R = asfloat(asuint(Pow2) | (p << 14) & 0x7FC000);
	float G = asfloat(asuint(Pow2) | (p <<  5) & 0x7FC000);
	float B = asfloat(asuint(Pow2) | (p >>  4) & 0x7FC000);
#endif
	return float3(R, G, B) - Pow2;
}

// Like LogLuv "minus the log".  The intention is to store Y in 16-bit float, and UV as 8-bit
// unorm values.  This is corrected from the widely publicized LogLuv encoding which used
// the wrong color primaries for sRGB and Rec.709.  Note the correct coefficients for computing
// luminance ("Y") in the 2nd row of RGBtoXYZ.
float3 EncodeYUV(float3 RGB)
{
	/*
	// Start with the right RGBtoXYZ matrix for your color space (this one is sRGB D65)
	// http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html
	static const float3x3 RGBtoXYZ =
	{
		0.4124564, 0.3575761, 0.1804375,
		0.2126729, 0.7151522, 0.0721750,	<-- The color primaries determining luminance
		0.0193339, 0.1191920, 0.9503041
	};

	// Compute u' and v'.  These pack chrominance into two normalized channels.
	// u' = 4X / (X + 15Y + 3Z)
	// v' = 9Y / (X + 15Y + 3Z)

	// Expand visible spectrum from (0, 0.62) to (0, 1)
	// u" = u' / 0.62
	// v" = v' / 0.62

	// If we compute these two values...
	// X' = 4 / 9 * X
	// XYZ' = (X + 15 * Y + 3 * Z) * 0.62 / 9

	// ...we can derive our final Yu"v" from X', Y, and XYZ'
	// u" = X' / XYZ'
	// v" = Y  / XYZ'

	// We can compute (X', Y, XYZ') by multiplying XYZ by this matrix
	static const float3x3 FixupMatrix =
	{
		4.0 / 9.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.62 / 9.0, 15.0 * 0.62 / 9.0, 3.0 * 0.62 / 9.0
	};
	
	// But we should just concatenate the two matrices
	static const float3x3 EncodeMatrix = mul(FixupMatrix, RGBtoXYZ);
	*/

	static const float3x3 EncodeMatrix = 
	{
		0.1833140, 0.1589227, 0.0801944,
		0.2126729, 0.7151522, 0.0721750,
		0.2521713, 0.7882566, 0.2834072
	};			  

	float3 Xp_Y_XYZp = mul(EncodeMatrix, RGB);
	float Y = Xp_Y_XYZp.y;
	float2 UV = saturate(Xp_Y_XYZp.xy / max(Xp_Y_XYZp.z, 1e-6));
	return float3(Y, UV);
}

float3 DecodeYUV(float3 YUV)
{
	// Inverse of EncodeMatrix
	static const float3x3 DecodeMatrix = 
	{
		 7.6649220,  0.9555189, -2.4122494,
		-2.2120164,  1.6682305,  0.2010778,
		-0.6677212, -5.4901517,  5.1156056
	};

	// Reverse of operations
	float Y = YUV.x;
	float XYZp = YUV.x / max(YUV.z, 1e-6);
	float Xp = YUV.y * XYZp;
	return mul(DecodeMatrix, float3(Xp, Y, XYZp));
}

// If you can't write Y and UV to separate buffers (R16_FLOAT, R8G8_UNORM), then
// you can pack them into R32_UINT.
uint PackYUV(float3 YUV)
{
	uint Y = f32tof16(YUV.x);
#if _XBOX_ONE
	uint p = __XB_PackF32ToU8(YUV.y * 255.0 + 0.5, 3, Y);
	return __XB_PackF32ToU8(YUV.z * 255.0 + 0.5, 2, p);
#else
	uint U = (uint)(YUV.y * 255.0 + 0.5);
	uint V = (uint)(YUV.z * 255.0 + 0.5);
	return Y | U << 24 | V << 16;
#endif
}

float3 UnpackYUV(uint YUV)
{
	float Y = f16tof32(YUV);
#if _XBOX_ONE
	float U = __XB_UnpackByte3(YUV) / 255.0;
	float V = __XB_UnpackByte2(YUV) / 255.0;
#else
	float U = (YUV >> 24) / 255.0;
	float V = ((YUV >> 16) & 0xFF) / 255.0; 
#endif
	return float3(Y, U, V);
}

// To understand this, know that all math on YUV should really be done on
// Y, Y*U, Y*V.  You can add and blend all three of those values the same
// as RGB, but for compact encoding, you only want to store Y, U, and V.
float3 AddYUV( float3 YUV1, float3 YUV2 )
{
	// Luminance is simply added; chrominance becomes a weighted average
	float Y = YUV1.x + YUV2.x;
	float2 UV = (YUV1.yz * YUV1.x + YUV2.yz * YUV2.x) / Y;
	return float3(Y, UV);
}

float3 LerpYUV( float3 YUV1, float3 YUV2, float t )
{
	// To rescale a YUV value, you just have to rescale Y.  Chroma remains the same.
	// After scaling luminance, you can add the two colors together.  This version of
	// the math (as opposed to the possibly more readable code commented out below) is
	// more efficient.  But it's interesting to note that if you kept values as Y, Y*U, Y*V,
	// you could simply add or lerp them.
	YUV1.x *= (1 - t);
	YUV2.x *= t;
	return AddYUV(YUV1, YUV2);

	//float Y = lerp(YUV1.x, YUV2.x, t);
	//float2 UV = lerp(YUV1.yz * YUV1.x, YUV2.yz * YUV2.x, t) / Y;
	//return float3(Y, UV);
}

// The standard 32-bit HDR color format.  Each float has a 5-bit exponent and no sign bit.
uint Pack_R11G11B10_FLOAT( float3 rgb )
{
	uint r = (f32tof16(rgb.x) >>  4) & 0x000007FF;
	uint g = (f32tof16(rgb.y) <<  7) & 0x003FF800;
	uint b = (f32tof16(rgb.z) << 17) & 0xFFC00000;
	return r | g | b;
}

float3 Unpack_R11G11B10_FLOAT( uint rgb )
{
	float r = f16tof32((rgb << 4 ) & 0x7FF0);
	float g = f16tof32((rgb >> 7 ) & 0x7FF0);
	float b = f16tof32((rgb >> 17) & 0x7FE0);
	return float3(r, g, b);
}

// These next two encodings are great for LDR data.  By knowing that our values are [0.0, 1.0]
// (or [0.0, 2.0), incidentally), we can reduce how many bits we need in the exponent.  We can
// immediately eliminate all postive exponents.  By giving more bits to the mantissa, we can
// improve precision at the expense of range.  The 8E3 format goes one bit further, quadrupling
// mantissa precision but increasing smallest exponent from -14 to -6.  The smallest value of 8E3
// is 2^-14, while the smallest value of 7E4 is 2^-21.  Both are smaller than the smallest 8-bit
// sRGB value, which is close to 2^-12.

// This is like R11G11B10_FLOAT except that it moves one bit from each exponent to each mantissa.
uint Pack_R11G11B10_E4_FLOAT( float3 rgb )
{
	// Clamp to [0.0, 2.0).  The magic number is 1.FFFFF x 2^0.  (We can't represent hex floats in HLSL.)
	// This trick works because clamping your exponent to 0 reduces the number of bits needed by 1.
	rgb = clamp( rgb, 0.0, asfloat(0x3FFFFFFF) );
	uint r = (f32tof16(rgb.r) >> 3 ) & 0x000007FF;
	uint g = (f32tof16(rgb.g) << 8 ) & 0x003FF800;
	uint b = (f32tof16(rgb.b) << 18) & 0xFFC00000;
	return r | g | b;
}

float3 Unpack_R11G11B10_E4_FLOAT( uint rgb )
{
	float r = f16tof32((rgb << 3 ) & 0x3FF8);
	float g = f16tof32((rgb >> 8 ) & 0x3FF8);
	float b = f16tof32((rgb >> 18) & 0x3FF0);
	return float3(r, g, b);
}

// This is like R11G11B10_FLOAT except that it moves two bits from each exponent to each mantissa.
uint Pack_R11G11B10_E3_FLOAT( float3 rgb )
{
	// Clamp to [0.0, 2.0).  Divide by 256 to bias the exponent by -8.  This shifts it down to use one
	// fewer bit while still taking advantage of the denormalization hardware.  In half precision,
	// the exponent of 0 is 0xF.  Dividing by 256 makes the max exponent 0x7--one fewer bit.
	rgb = clamp( rgb, 0.0, asfloat(0x3FFFFFFF) ) / 256.0;
	uint r = (f32tof16(rgb.r) >> 2 ) & 0x000007FF;
	uint g = (f32tof16(rgb.g) << 9 ) & 0x003FF800;
	uint b = (f32tof16(rgb.b) << 19) & 0xFFC00000;
	return r | g | b;
}

float3 Unpack_R11G11B10_E3_FLOAT( uint rgb )
{
	float r = f16tof32((rgb << 2 ) & 0x1FFC);
	float g = f16tof32((rgb >> 9 ) & 0x1FFC);
	float b = f16tof32((rgb >> 19) & 0x1FF8);
	return float3(r, g, b) * 256.0;
}
