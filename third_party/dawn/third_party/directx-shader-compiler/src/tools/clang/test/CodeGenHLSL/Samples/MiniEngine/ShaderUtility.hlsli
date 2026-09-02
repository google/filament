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

#pragma warning( disable : 3571 )

// This approximates sRGB sufficiently enough that for 8-bit encodings is indistinguishable
// from the "slow" version.  This can be a lot faster due to avoiding three pow() calls.
float3 LinearToSRGB_Fast( float3 x )
{
	return x < 0.0031308 ? 12.92 * x : 1.13005 * sqrt(x - 0.00228) - 0.13448 * x + 0.005719;
}

float3 LinearToSRGB( float3 x )
{
	return x < 0.0031308 ? 12.92 * x : 1.055 * pow(x, 1.0 / 2.4) - 0.055;
}

float3 SRGBToLinear( float3 x )
{
	return x < 0.04045 ? x / 12.92 : pow( (x + 0.055) / 1.055, 2.4 );
}

float3 LinearToREC709( float3 x )
{
	return x < 0.018 ? 4.5 * x : 1.099 * pow(x, 0.45) - 0.099;
}

float3 REC709ToLinear( float3 x )
{
	return x < 0.081 ? x / 4.5 : pow((x + 0.099) / 1.099, 1.0 / 0.45);
}

// Same as Rec.709 transfer but more precise (intended for 12-bit rather than 10-bit)
float3 LinearToREC2020(float3 x)
{
	return x < 0.0181 ? 4.5 * x : 1.0993 * pow(x, 0.45) - 0.0993;
}

float3 REC2020ToLinear(float3 x)
{
	return x < 0.08145 ? x / 4.5 : pow((x + 0.0993) / 1.0993, 1.0 / 0.45);
}

float3 LinearToREC2084( float3 L )
{
	float m1 = 2610.0 / 4096.0 / 4;
	float m2 = 2523.0 / 4096.0 * 128;
	float c1 = 3424.0 / 4096.0;
	float c2 = 2413.0 / 4096.0 * 32;
	float c3 = 2392.0 / 4096.0 * 32;
	float3 Lp = pow(L, m1);
	return pow((c1 + c2 * Lp) / (1 + c3 * Lp), m2);
}

float3 REC2084ToLinear( float3 N )
{
	float m1 = 2610.0 / 4096.0 / 4;
	float m2 = 2523.0 / 4096.0 * 128;
	float c1 = 3424.0 / 4096.0;
	float c2 = 2413.0 / 4096.0 * 32;
	float c3 = 2392.0 / 4096.0 * 32;
	float3 Np = pow(N, 1 / m2);
	return pow(max(Np - c1, 0) / (c2 - c3 * Np), 1 / m1);
}

// Encodes a smooth logarithmic gradient for even distribution of precision natural to vision
float LinearToLogLuminance( float x, float gamma = 4.0 )
{
	return log2(lerp(1, exp2(gamma), x)) / gamma;
}

// This assumes the default color gamut found in sRGB and REC709.  The color primaries determine these
// coefficients.  Note that this operates on linear values, not gamma space.
float RGBToLuminance( float3 x )
{
	return dot( x, float3(0.212671, 0.715160, 0.072169) );		// Defined by sRGB gamut
//	return dot( x, float3(0.2989164, 0.5865990, 0.1144845) );	// NTSC - don't use this
}

// Assumes the "white point" is 1.0.  Prescale your HDR values if otherwise.  'E' affects the rate
// at which colors blow out to white.
float3 ToneMap( float3 hdr, float E = 4.0 )
{
	return (1 - exp2(-E * hdr)) / (1 - exp2(-E));
}

// This variant rescales only the luminance of the color to fit in the [0, 1] range while preserving hue.
float3 ToneMap2( float3 hdr, float E = 4.0 )
{
	float luma = RGBToLuminance(hdr);
	return hdr * (1 - exp2(-E * luma)) / (1 - exp2(-E)) / (luma + 0.0001);
}

float ToneMapLuma( float Luma, float E = 4.0 )
{
	return (1 - exp2(-E * Luma)) / (1 - exp2(-E));
}

// This is the same as above, but converts the linear luminance value to a more subjective "perceived luminance",
// which could be called the Log-Luminance.
float RGBToLogLuminance( float3 x, float gamma = 4.0 )
{
	return LinearToLogLuminance( RGBToLuminance(x), gamma );
}

float3 RGBFullToLimited( float3 x )
{
	return saturate(x) * 219.0 / 255.0 + 16.0 / 255.0;
}

float3 RGBLimitedToFull( float3 x )
{
	return saturate((x - 16.0 / 255.0) * 255.0 / 219.0);
}

#define COLOR_FORMAT_LINEAR			0
#define COLOR_FORMAT_sRGB_FULL		1
#define COLOR_FORMAT_sRGB_LIMITED	2
#define COLOR_FORMAT_Rec709_FULL	3
#define COLOR_FORMAT_Rec709_LIMITED	4
#define COLOR_FORMAT_7e3_FLOAT_FULL	5
#define COLOR_FORMAT_6e4_FLOAT_FULL	6
#define COLOR_FORMAT_TV_DEFAULT		COLOR_FORMAT_Rec709_LIMITED
#define COLOR_FORMAT_PC_DEFAULT		COLOR_FORMAT_sRGB_FULL

#define HDR_COLOR_FORMAT			COLOR_FORMAT_LINEAR
#define LDR_COLOR_FORMAT			COLOR_FORMAT_LINEAR
#if _XBOX_ONE
	#define DISPLAY_PLANE_FORMAT	COLOR_FORMAT_TV_DEFAULT
	#define OVERLAY_PLANE_FORMAT	COLOR_FORMAT_sRGB_FULL
#else
	#define DISPLAY_PLANE_FORMAT	COLOR_FORMAT_PC_DEFAULT
#endif

float3 ApplyColorProfile( float3 x, int Format )
{
	switch (Format)
	{
	default:
	case COLOR_FORMAT_LINEAR:
		return x;
	case COLOR_FORMAT_sRGB_FULL:
		return LinearToSRGB(x);
	case COLOR_FORMAT_sRGB_LIMITED:
		return RGBFullToLimited(LinearToSRGB(x));
	case COLOR_FORMAT_Rec709_FULL:
		return LinearToREC709(x);
	case COLOR_FORMAT_Rec709_LIMITED:
		return RGBFullToLimited(LinearToREC709(x));

	// Xbox formats:  10-bit floats with biased exponents; range: [0, 2)
	case COLOR_FORMAT_7e3_FLOAT_FULL:
		return x * 16.0;
	case COLOR_FORMAT_6e4_FLOAT_FULL:
		return x * 256.0;
	};
}

float3 LinearizeColor( float3 x, int Format )
{
	switch (Format)
	{
	default:
	case COLOR_FORMAT_LINEAR:
		return x;
	case COLOR_FORMAT_sRGB_FULL:
		return SRGBToLinear(x);
	case COLOR_FORMAT_sRGB_LIMITED:
		return SRGBToLinear(RGBLimitedToFull(x));
	case COLOR_FORMAT_Rec709_FULL:
		return REC709ToLinear(x);
	case COLOR_FORMAT_Rec709_LIMITED:
		return REC709ToLinear(RGBLimitedToFull(x));

	// Xbox formats:  10-bit floats with biased exponents; range: [0, 2)
	case COLOR_FORMAT_7e3_FLOAT_FULL:
		return x / 16.0;
	case COLOR_FORMAT_6e4_FLOAT_FULL:
		return x / 256.0;
	};
}

float3 ConvertColor( float3 x, int FromFormat, int ToFormat )
{
	if (FromFormat == ToFormat)
		return x;

	return ApplyColorProfile(LinearizeColor(x, FromFormat), ToFormat);
}