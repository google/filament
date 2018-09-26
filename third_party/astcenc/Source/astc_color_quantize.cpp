/*----------------------------------------------------------------------------*/
/**
 *	This confidential and proprietary software may be used only as
 *	authorised by a licensing agreement from ARM Limited
 *	(C) COPYRIGHT 2011-2012 ARM Limited
 *	ALL RIGHTS RESERVED
 *
 *	The entire notice above must be reproduced on all authorised
 *	copies and copies may only be made to the extent permitted
 *	by a licensing agreement from ARM Limited.
 *
 *	@brief	Color quantization functions for ASTC.
 */
/*----------------------------------------------------------------------------*/

#include <stdio.h>

#include "astc_codec_internals.h"
#include "softfloat.h"
#include <math.h>

#ifdef DEBUG_PRINT_DIAGNOSTICS
	#include <stdio.h>
#endif

/*
	quantize an LDR RGB color. Since this is a fall-back encoding, we cannot actually
	fail but must just go on until we can produce a sensible result.

	Due to how this encoding works, color0 cannot be larger than color1; as such,
	if color0 is actually larger than color1, then color0 is reduced and color1 is
	increased until color0 is no longer larger than color1.
*/
static inline int cqt_lookup(int quantization_level, int value)
{
	if (value < 0)
		value = 0;
	else if (value > 255)
		value = 255;
	return color_quantization_tables[quantization_level][value];
}

// clamp an input value to [0,255]; NaN is turned into 0
static inline float clamp255(float val)
{
	if (val > 255.0f)
		val = 255.0f;
	else if (val > 0.0f)
	{
		// deliberately empty
		// switching the order of calculation here will fail to handle 0.
	}
	else
		val = 0.0f;

	return val;
}



// clamp an input value to [0,1]; Nan is turned into 0.

static inline float clamp01(float val)
{
	if (val > 1.0f)
		val = 1.0f;
	else if (val > 0.0f)
	{
		// deliberately empty
		// switching the order of calculation here will fail to handle 0.
	}
	else
		val = 0.0f;

	return val;
}



void quantize_rgb(float4 color0,	// LDR: 0=lowest, 255=highest
				  float4 color1, int output[6], int quantization_level)
{
	color0.xyz = color0.xyz * (1.0f / 257.0f);
	color1.xyz = color1.xyz * (1.0f / 257.0f);


	float r0 = clamp255(color0.x);
	float g0 = clamp255(color0.y);
	float b0 = clamp255(color0.z);

	float r1 = clamp255(color1.x);
	float g1 = clamp255(color1.y);
	float b1 = clamp255(color1.z);

	int ri0, gi0, bi0, ri1, gi1, bi1;
	int ri0b, gi0b, bi0b, ri1b, gi1b, bi1b;
	float rgb0_addon = 0.5f;
	float rgb1_addon = 0.5f;
	int iters = 0;
	do
	{
		ri0 = cqt_lookup(quantization_level, (int)floor(r0 + rgb0_addon));
		gi0 = cqt_lookup(quantization_level, (int)floor(g0 + rgb0_addon));
		bi0 = cqt_lookup(quantization_level, (int)floor(b0 + rgb0_addon));
		ri1 = cqt_lookup(quantization_level, (int)floor(r1 + rgb1_addon));
		gi1 = cqt_lookup(quantization_level, (int)floor(g1 + rgb1_addon));
		bi1 = cqt_lookup(quantization_level, (int)floor(b1 + rgb1_addon));

		ri0b = color_unquantization_tables[quantization_level][ri0];
		gi0b = color_unquantization_tables[quantization_level][gi0];
		bi0b = color_unquantization_tables[quantization_level][bi0];
		ri1b = color_unquantization_tables[quantization_level][ri1];
		gi1b = color_unquantization_tables[quantization_level][gi1];
		bi1b = color_unquantization_tables[quantization_level][bi1];

		rgb0_addon -= 0.2f;
		rgb1_addon += 0.2f;
		iters++;
	}
	while (ri0b + gi0b + bi0b > ri1b + gi1b + bi1b);

	output[0] = ri0;
	output[1] = ri1;
	output[2] = gi0;
	output[3] = gi1;
	output[4] = bi0;
	output[5] = bi1;
}


/*
   quantize an RGBA color. */
void quantize_rgba(float4 color0, float4 color1, int output[8], int quantization_level)
{
	color0.w = color0.w * (1.0f / 257.0f);
	color1.w = color1.w * (1.0f / 257.0f);

	float a0 = clamp255(color0.w);
	float a1 = clamp255(color1.w);
	int ai0 = color_quantization_tables[quantization_level][(int)floor(a0 + 0.5f)];
	int ai1 = color_quantization_tables[quantization_level][(int)floor(a1 + 0.5f)];

	output[6] = ai0;
	output[7] = ai1;

	quantize_rgb(color0, color1, output, quantization_level);
}



/*
   attempt to quantize RGB endpoint values with blue-contraction. Returns 1 on failure, 0 on success. */
int try_quantize_rgb_blue_contract(float4 color0,	// assumed to be the smaller color
								   float4 color1,	// assumed to be the larger color
								   int output[6], int quantization_level)
{
	color0.xyz = color0.xyz * (1.0f / 257.0f);
	color1.xyz = color1.xyz * (1.0f / 257.0f);

	float r0 = color0.x;
	float g0 = color0.y;
	float b0 = color0.z;

	float r1 = color1.x;
	float g1 = color1.y;
	float b1 = color1.z;

	// inverse blue-contraction. This can produce an overflow;
	// just bail out immediately if this is the case.
	r0 += (r0 - b0);
	g0 += (g0 - b0);
	r1 += (r1 - b1);
	g1 += (g1 - b1);

	if (r0 < 0.0f || r0 > 255.0f || g0 < 0.0f || g0 > 255.0f || b0 < 0.0f || b0 > 255.0f ||
		r1 < 0.0f || r1 > 255.0f || g1 < 0.0f || g1 > 255.0f || b1 < 0.0f || b1 > 255.0f)
	{
		return 0;
	}

	// quantize the inverse-blue-contracted color
	int ri0 = color_quantization_tables[quantization_level][(int)floor(r0 + 0.5f)];
	int gi0 = color_quantization_tables[quantization_level][(int)floor(g0 + 0.5f)];
	int bi0 = color_quantization_tables[quantization_level][(int)floor(b0 + 0.5f)];
	int ri1 = color_quantization_tables[quantization_level][(int)floor(r1 + 0.5f)];
	int gi1 = color_quantization_tables[quantization_level][(int)floor(g1 + 0.5f)];
	int bi1 = color_quantization_tables[quantization_level][(int)floor(b1 + 0.5f)];

	// then unquantize again
	int ru0 = color_unquantization_tables[quantization_level][ri0];
	int gu0 = color_unquantization_tables[quantization_level][gi0];
	int bu0 = color_unquantization_tables[quantization_level][bi0];
	int ru1 = color_unquantization_tables[quantization_level][ri1];
	int gu1 = color_unquantization_tables[quantization_level][gi1];
	int bu1 = color_unquantization_tables[quantization_level][bi1];

	// if color #1 is not larger than color #0, then blue-contraction is not a valid approach.
	// note that blue-contraction and quantization may itself change this order, which is why
	// we must only test AFTER blue-contraction.
	if (ru1 + gu1 + bu1 <= ru0 + gu0 + bu0)
		return 0;

	output[0] = ri1;
	output[1] = ri0;
	output[2] = gi1;
	output[3] = gi0;
	output[4] = bi1;
	output[5] = bi0;

	return 1;
}




/*
   quantize an RGBA color with blue-contraction */
int try_quantize_rgba_blue_contract(float4 color0, float4 color1, int output[8], int quantization_level)
{
	color0.w = color0.w * (1.0f / 257.0f);
	color1.w = color1.w * (1.0f / 257.0f);

	float a0 = clamp255(color0.w);
	float a1 = clamp255(color1.w);

	output[7] = color_quantization_tables[quantization_level][(int)floor(a0 + 0.5f)];
	output[6] = color_quantization_tables[quantization_level][(int)floor(a1 + 0.5f)];

	return try_quantize_rgb_blue_contract(color0, color1, output, quantization_level);
}


// delta-encoding:
// at decode time, we move one bit from the offset to the base and seize another bit as a sign bit;
// we then unquantize both values as if they contain one extra bit.

// if the sum of the offsets is nonnegative, then we encode a regular delta.


/*
   attempt to quantize an RGB endpoint value with delta-encoding. */

int try_quantize_rgb_delta(float4 color0, float4 color1, int output[6], int quantization_level)
{
	color0.xyz = color0.xyz * (1.0f / 257.0f);
	color1.xyz = color1.xyz * (1.0f / 257.0f);

	float r0 = clamp255(color0.x);
	float g0 = clamp255(color0.y);
	float b0 = clamp255(color0.z);

	float r1 = clamp255(color1.x);
	float g1 = clamp255(color1.y);
	float b1 = clamp255(color1.z);

	// transform r0 to unorm9
	int r0a = (int)floor(r0 + 0.5f);
	int g0a = (int)floor(g0 + 0.5f);
	int b0a = (int)floor(b0 + 0.5f);
	r0a <<= 1;
	g0a <<= 1;
	b0a <<= 1;

	// mask off the top bit
	int r0b = r0a & 0xFF;
	int g0b = g0a & 0xFF;
	int b0b = b0a & 0xFF;

	// quantize, then unquantize in order to get a value that we take
	// differences against.
	int r0be = color_quantization_tables[quantization_level][r0b];
	int g0be = color_quantization_tables[quantization_level][g0b];
	int b0be = color_quantization_tables[quantization_level][b0b];

	r0b = color_unquantization_tables[quantization_level][r0be];
	g0b = color_unquantization_tables[quantization_level][g0be];
	b0b = color_unquantization_tables[quantization_level][b0be];
	r0b |= r0a & 0x100;			// final unquantized-values for endpoint 0.
	g0b |= g0a & 0x100;
	b0b |= b0a & 0x100;

	// then, get hold of the second value
	int r1d = (int)floor(r1 + 0.5f);
	int g1d = (int)floor(g1 + 0.5f);
	int b1d = (int)floor(b1 + 0.5f);

	r1d <<= 1;
	g1d <<= 1;
	b1d <<= 1;
	// and take differences!
	r1d -= r0b;
	g1d -= g0b;
	b1d -= b0b;

	// check if the difference is too large to be encodable.
	if (r1d > 63 || g1d > 63 || b1d > 63 || r1d < -64 || g1d < -64 || b1d < -64)
		return 0;

	// insert top bit of the base into the offset
	r1d &= 0x7F;
	g1d &= 0x7F;
	b1d &= 0x7F;

	r1d |= (r0b & 0x100) >> 1;
	g1d |= (g0b & 0x100) >> 1;
	b1d |= (b0b & 0x100) >> 1;

	// then quantize & unquantize; if this causes any of the top two bits to flip,
	// then encoding fails, since we have then corrupted either the top bit of the base
	// or the sign bit of the offset.
	int r1de = color_quantization_tables[quantization_level][r1d];
	int g1de = color_quantization_tables[quantization_level][g1d];
	int b1de = color_quantization_tables[quantization_level][b1d];

	int r1du = color_unquantization_tables[quantization_level][r1de];
	int g1du = color_unquantization_tables[quantization_level][g1de];
	int b1du = color_unquantization_tables[quantization_level][b1de];

	if (((r1d ^ r1du) | (g1d ^ g1du) | (b1d ^ b1du)) & 0xC0)
		return 0;

	// check that the sum of the encoded offsets is nonnegative, else encoding fails
	r1du &= 0x7f;
	g1du &= 0x7f;
	b1du &= 0x7f;
	if (r1du & 0x40)
		r1du -= 0x80;
	if (g1du & 0x40)
		g1du -= 0x80;
	if (b1du & 0x40)
		b1du -= 0x80;
	if (r1du + g1du + b1du < 0)
		return 0;

	// check that the offsets produce legitimate sums as well.
	r1du += r0b;
	g1du += g0b;
	b1du += b0b;
	if (r1du < 0 || r1du > 0x1FF || g1du < 0 || g1du > 0x1FF || b1du < 0 || b1du > 0x1FF)
		return 0;

	// OK, we've come this far; we can now encode legitimate values.
	output[0] = r0be;
	output[1] = r1de;
	output[2] = g0be;
	output[3] = g1de;
	output[4] = b0be;
	output[5] = b1de;

	return 1;
}


int try_quantize_rgb_delta_blue_contract(float4 color0, float4 color1, int output[6], int quantization_level)
{
	color0.xyz = color0.xyz * (1.0f / 257.0f);
	color1.xyz = color1.xyz * (1.0f / 257.0f);

	// switch around endpoint colors already at start.
	float r0 = color1.x;
	float g0 = color1.y;
	float b0 = color1.z;

	float r1 = color0.x;
	float g1 = color0.y;
	float b1 = color0.z;

	// inverse blue-contraction. This step can perform an overflow, in which case
	// we will bail out immediately.
	r0 += (r0 - b0);
	g0 += (g0 - b0);
	r1 += (r1 - b1);
	g1 += (g1 - b1);

	if (r0 < 0.0f || r0 > 255.0f || g0 < 0.0f || g0 > 255.0f || b0 < 0.0f || b0 > 255.0f || r1 < 0.0f || r1 > 255.0f || g1 < 0.0f || g1 > 255.0f || b1 < 0.0f || b1 > 255.0f)
		return 0;

	// transform r0 to unorm9
	int r0a = (int)floor(r0 + 0.5f);
	int g0a = (int)floor(g0 + 0.5f);
	int b0a = (int)floor(b0 + 0.5f);
	r0a <<= 1;
	g0a <<= 1;
	b0a <<= 1;

	// mask off the top bit
	int r0b = r0a & 0xFF;
	int g0b = g0a & 0xFF;
	int b0b = b0a & 0xFF;

	// quantize, then unquantize in order to get a value that we take
	// differences against.
	int r0be = color_quantization_tables[quantization_level][r0b];
	int g0be = color_quantization_tables[quantization_level][g0b];
	int b0be = color_quantization_tables[quantization_level][b0b];

	r0b = color_unquantization_tables[quantization_level][r0be];
	g0b = color_unquantization_tables[quantization_level][g0be];
	b0b = color_unquantization_tables[quantization_level][b0be];
	r0b |= r0a & 0x100;			// final unquantized-values for endpoint 0.
	g0b |= g0a & 0x100;
	b0b |= b0a & 0x100;

	// then, get hold of the second value
	int r1d = (int)floor(r1 + 0.5f);
	int g1d = (int)floor(g1 + 0.5f);
	int b1d = (int)floor(b1 + 0.5f);

	r1d <<= 1;
	g1d <<= 1;
	b1d <<= 1;
	// and take differences!
	r1d -= r0b;
	g1d -= g0b;
	b1d -= b0b;

	// check if the difference is too large to be encodable.
	if (r1d > 63 || g1d > 63 || b1d > 63 || r1d < -64 || g1d < -64 || b1d < -64)
		return 0;

	// insert top bit of the base into the offset
	r1d &= 0x7F;
	g1d &= 0x7F;
	b1d &= 0x7F;

	r1d |= (r0b & 0x100) >> 1;
	g1d |= (g0b & 0x100) >> 1;
	b1d |= (b0b & 0x100) >> 1;

	// then quantize & unquantize; if this causes any of the top two bits to flip,
	// then encoding fails, since we have then corrupted either the top bit of the base
	// or the sign bit of the offset.
	int r1de = color_quantization_tables[quantization_level][r1d];
	int g1de = color_quantization_tables[quantization_level][g1d];
	int b1de = color_quantization_tables[quantization_level][b1d];

	int r1du = color_unquantization_tables[quantization_level][r1de];
	int g1du = color_unquantization_tables[quantization_level][g1de];
	int b1du = color_unquantization_tables[quantization_level][b1de];

	if (((r1d ^ r1du) | (g1d ^ g1du) | (b1d ^ b1du)) & 0xC0)
		return 0;

	// check that the sum of the encoded offsets is negative, else encoding fails
	// note that this is inverse of the test for non-blue-contracted RGB.
	r1du &= 0x7f;
	g1du &= 0x7f;
	b1du &= 0x7f;
	if (r1du & 0x40)
		r1du -= 0x80;
	if (g1du & 0x40)
		g1du -= 0x80;
	if (b1du & 0x40)
		b1du -= 0x80;
	if (r1du + g1du + b1du >= 0)
		return 0;

	// check that the offsets produce legitimate sums as well.
	r1du += r0b;
	g1du += g0b;
	b1du += b0b;
	if (r1du < 0 || r1du > 0x1FF || g1du < 0 || g1du > 0x1FF || b1du < 0 || b1du > 0x1FF)
		return 0;

	// OK, we've come this far; we can now encode legitimate values.
	output[0] = r0be;
	output[1] = r1de;
	output[2] = g0be;
	output[3] = g1de;
	output[4] = b0be;
	output[5] = b1de;

	return 1;
}


int try_quantize_alpha_delta(float4 color0, float4 color1, int output[8], int quantization_level)
{
	color0.w = color0.w * (1.0f / 257.0f);
	color1.w = color1.w * (1.0f / 257.0f);

	// the calculation for alpha-delta is exactly the same as for RGB-delta; see
	// the RGB-delta function for comments.
	float a0 = clamp255(color0.w);
	float a1 = clamp255(color1.w);

	int a0a = (int)floor(a0 + 0.5f);
	a0a <<= 1;
	int a0b = a0a & 0xFF;
	int a0be = color_quantization_tables[quantization_level][a0b];
	a0b = color_unquantization_tables[quantization_level][a0be];
	a0b |= a0a & 0x100;
	int a1d = (int)floor(a1 + 0.5f);
	a1d <<= 1;
	a1d -= a0b;
	if (a1d > 63 || a1d < -64)
		return 0;
	a1d &= 0x7F;
	a1d |= (a0b & 0x100) >> 1;
	int a1de = color_quantization_tables[quantization_level][a1d];
	int a1du = color_unquantization_tables[quantization_level][a1de];
	if ((a1d ^ a1du) & 0xC0)
		return 0;
	a1du &= 0x7F;
	if (a1du & 0x40)
		a1du -= 0x80;
	a1du += a0b;
	if (a1du < 0 || a1du > 0x1FF)
		return 0;
	output[6] = a0be;
	output[7] = a1de;
	return 1;
}



int try_quantize_luminance_alpha_delta(float4 color0, float4 color1, int output[8], int quantization_level)
{
	float l0 = clamp255((color0.x + color0.y + color0.z) * ((1.0f / 3.0f) * (1.0f / 257.0f)));
	float l1 = clamp255((color1.x + color1.y + color1.z) * ((1.0f / 3.0f) * (1.0f / 257.0f)));
	float a0 = clamp255(color0.w * (1.0f / 257.0f));
	float a1 = clamp255(color1.w * (1.0f / 257.0f));

	int l0a = (int)floor(l0 + 0.5f);
	int a0a = (int)floor(a0 + 0.5f);
	l0a <<= 1;
	a0a <<= 1;
	int l0b = l0a & 0xFF;
	int a0b = a0a & 0xFF;
	int l0be = color_quantization_tables[quantization_level][l0b];
	int a0be = color_quantization_tables[quantization_level][a0b];
	l0b = color_unquantization_tables[quantization_level][l0be];
	a0b = color_unquantization_tables[quantization_level][a0be];
	l0b |= l0a & 0x100;
	a0b |= a0a & 0x100;
	int l1d = (int)floor(l1 + 0.5f);
	int a1d = (int)floor(a1 + 0.5f);
	l1d <<= 1;
	a1d <<= 1;
	l1d -= l0b;
	a1d -= a0b;
	if (l1d > 63 || l1d < -64)
		return 0;
	if (a1d > 63 || a1d < -64)
		return 0;
	l1d &= 0x7F;
	a1d &= 0x7F;
	l1d |= (l0b & 0x100) >> 1;
	a1d |= (a0b & 0x100) >> 1;

	int l1de = color_quantization_tables[quantization_level][l1d];
	int a1de = color_quantization_tables[quantization_level][a1d];
	int l1du = color_unquantization_tables[quantization_level][l1de];
	int a1du = color_unquantization_tables[quantization_level][a1de];
	if ((l1d ^ l1du) & 0xC0)
		return 0;
	if ((a1d ^ a1du) & 0xC0)
		return 0;
	l1du &= 0x7F;
	a1du &= 0x7F;
	if (l1du & 0x40)
		l1du -= 0x80;
	if (a1du & 0x40)
		a1du -= 0x80;
	l1du += l0b;
	a1du += a0b;
	if (l1du < 0 || l1du > 0x1FF)
		return 0;
	if (a1du < 0 || a1du > 0x1FF)
		return 0;
	output[0] = l0be;
	output[1] = l1de;
	output[2] = a0be;
	output[3] = a1de;

	return 1;
}




int try_quantize_rgba_delta(float4 color0, float4 color1, int output[8], int quantization_level)
{
	int alpha_delta_res = try_quantize_alpha_delta(color0, color1, output, quantization_level);

	if (alpha_delta_res == 0)
		return 0;

	return try_quantize_rgb_delta(color0, color1, output, quantization_level);
}


int try_quantize_rgba_delta_blue_contract(float4 color0, float4 color1, int output[8], int quantization_level)
{
	// notice that for the alpha encoding, we are swapping around color0 and color1;
	// this is because blue-contraction involves swapping around the two colors.
	int alpha_delta_res = try_quantize_alpha_delta(color1, color0, output, quantization_level);

	if (alpha_delta_res == 0)
		return 0;

	return try_quantize_rgb_delta_blue_contract(color0, color1, output, quantization_level);
}



void quantize_rgbs_new(float4 rgbs_color,	// W component is a desired-scale to apply, in the range 0..1
					   int output[4], int quantization_level)
{
	rgbs_color.xyz = rgbs_color.xyz * (1.0f / 257.0f);

	float r = clamp255(rgbs_color.x);
	float g = clamp255(rgbs_color.y);
	float b = clamp255(rgbs_color.z);

	int ri = color_quantization_tables[quantization_level][(int)floor(r + 0.5f)];
	int gi = color_quantization_tables[quantization_level][(int)floor(g + 0.5f)];
	int bi = color_quantization_tables[quantization_level][(int)floor(b + 0.5f)];

	int ru = color_unquantization_tables[quantization_level][ri];
	int gu = color_unquantization_tables[quantization_level][gi];
	int bu = color_unquantization_tables[quantization_level][bi];

	float oldcolorsum = rgbs_color.x + rgbs_color.y + rgbs_color.z;
	float newcolorsum = (float)(ru + gu + bu);

	float scale = clamp01(rgbs_color.w * (oldcolorsum + 1e-10f) / (newcolorsum + 1e-10f));

	int scale_idx = (int)floor(scale * 256.0f + 0.5f);

	if (scale_idx < 0)
		scale_idx = 0;
	else if (scale_idx > 255)
		scale_idx = 255;

	output[0] = ri;
	output[1] = gi;
	output[2] = bi;
	output[3] = color_quantization_tables[quantization_level][scale_idx];
}



void quantize_rgbs_alpha_new(float4 color0, float4 color1, float4 rgbs_color, int output[6], int quantization_level)
{
	color0.w = color0.w * (1.0f / 257.0f);
	color1.w = color1.w * (1.0f / 257.0f);

	float a0 = clamp255(color0.w);
	float a1 = clamp255(color1.w);

	int ai0 = color_quantization_tables[quantization_level][(int)floor(a0 + 0.5f)];
	int ai1 = color_quantization_tables[quantization_level][(int)floor(a1 + 0.5f)];

	output[4] = ai0;
	output[5] = ai1;

	quantize_rgbs_new(rgbs_color, output, quantization_level);
}



void quantize_luminance(float4 color0, float4 color1, int output[2], int quantization_level)
{
	color0.xyz = color0.xyz * (1.0f / 257.0f);
	color1.xyz = color1.xyz * (1.0f / 257.0f);

	float lum0 = clamp255((color0.x + color0.y + color0.z) * (1.0f / 3.0f));
	float lum1 = clamp255((color1.x + color1.y + color1.z) * (1.0f / 3.0f));

	if (lum0 > lum1)
	{
		float avg = (lum0 + lum1) * 0.5f;
		lum0 = avg;
		lum1 = avg;
	}

	output[0] = color_quantization_tables[quantization_level][(int)floor(lum0 + 0.5f)];
	output[1] = color_quantization_tables[quantization_level][(int)floor(lum1 + 0.5f)];
}






void quantize_luminance_alpha(float4 color0, float4 color1, int output[4], int quantization_level)
{
	color0 = color0 * (1.0f / 257.0f);
	color1 = color1 * (1.0f / 257.0f);

	float lum0 = clamp255((color0.x + color0.y + color0.z) * (1.0f / 3.0f));
	float lum1 = clamp255((color1.x + color1.y + color1.z) * (1.0f / 3.0f));
	float a0 = clamp255(color0.w);
	float a1 = clamp255(color1.w);

	// if the endpoints are *really* close, then pull them apart slightly;
	// this affords for >8 bits precision for normal maps.
	if (quantization_level > 18 && fabs(lum0 - lum1) < 3.0f)
	{
		if (lum0 < lum1)
		{
			lum0 -= 0.5f;
			lum1 += 0.5f;
		}
		else
		{
			lum0 += 0.5f;
			lum1 -= 0.5f;
		}
		lum0 = clamp255(lum0);
		lum1 = clamp255(lum1);
	}
	if (quantization_level > 18 && fabs(a0 - a1) < 3.0f)
	{
		if (a0 < a1)
		{
			a0 -= 0.5f;
			a1 += 0.5f;
		}
		else
		{
			a0 += 0.5f;
			a1 -= 0.5f;
		}
		a0 = clamp255(a0);
		a1 = clamp255(a1);
	}


	output[0] = color_quantization_tables[quantization_level][(int)floor(lum0 + 0.5f)];
	output[1] = color_quantization_tables[quantization_level][(int)floor(lum1 + 0.5f)];
	output[2] = color_quantization_tables[quantization_level][(int)floor(a0 + 0.5f)];
	output[3] = color_quantization_tables[quantization_level][(int)floor(a1 + 0.5f)];
}


void quantize0(float4 color0, float4 color1, int output[8], int quantization_level)
{
	IGNORE(color0);
	IGNORE(color1);
	IGNORE(output);
	IGNORE(quantization_level);

	int i;
	for (i = 0; i < 8; i++)
		output[i] = 0;
}


// quantize and unquantize a number, wile making sure to retain the top two bits.
static inline void quantize_and_unquantize_retain_top_two_bits(int quantization_level, int value_to_quantize,	// 0 to 255.
															   int *quantized_value, int *unquantized_value)
{

	int perform_loop;
	int quantval;
	int uquantval;

	do
	{
		quantval = color_quantization_tables[quantization_level][value_to_quantize];
		uquantval = color_unquantization_tables[quantization_level][quantval];

		// perform looping if the top two bits were modified by quant/unquant
		perform_loop = (value_to_quantize & 0xC0) != (uquantval & 0xC0);

		if ((uquantval & 0xC0) > (value_to_quantize & 0xC0))
		{
			// quant/unquant rounded UP so that the top two bits changed;
			// decrement the input value in hopes that this will avoid rounding up.
			value_to_quantize--;
		}
		else if ((uquantval & 0xC0) < (value_to_quantize & 0xC0))
		{
			// quant/unquant rounded DOWN so that the top two bits changed;
			// decrement the input value in hopes that this will avoid rounding down.
			value_to_quantize--;
		}
	}
	while (perform_loop);

	*quantized_value = quantval;
	*unquantized_value = uquantval;
}


// quantize and unquantize a number, wile making sure to retain the top four bits.
static inline void quantize_and_unquantize_retain_top_four_bits(int quantization_level, int value_to_quantize,	// 0 to 255.
																int *quantized_value, int *unquantized_value)
{

	int perform_loop;
	int quantval;
	int uquantval;

	do
	{
		quantval = color_quantization_tables[quantization_level][value_to_quantize];
		uquantval = color_unquantization_tables[quantization_level][quantval];

		// perform looping if the top two bits were modified by quant/unquant
		perform_loop = (value_to_quantize & 0xF0) != (uquantval & 0xF0);

		if ((uquantval & 0xF0) > (value_to_quantize & 0xF0))
		{
			// quant/unquant rounded UP so that the top two bits changed;
			// decrement the input value in hopes that this will avoid rounding up.
			value_to_quantize--;
		}
		else if ((uquantval & 0xF0) < (value_to_quantize & 0xF0))
		{
			// quant/unquant rounded DOWN so that the top two bits changed;
			// decrement the input value in hopes that this will avoid rounding down.
			value_to_quantize--;
		}
	}
	while (perform_loop);

	*quantized_value = quantval;
	*unquantized_value = uquantval;
}

// quantize and unquantize a number, wile making sure to retain the top two bits.
static inline void quantize_and_unquantize_retain_top_bit(int quantization_level, int value_to_quantize,	// 0 to 255.
														  int *quantized_value, int *unquantized_value)
{

	int perform_loop;
	int quantval;
	int uquantval;

	do
	{
		quantval = color_quantization_tables[quantization_level][value_to_quantize];
		uquantval = color_unquantization_tables[quantization_level][quantval];

		// perform looping if the top two bits were modified by quant/unquant
		perform_loop = (value_to_quantize & 0x80) != (uquantval & 0x80);

		if ((uquantval & 0x80) > (value_to_quantize & 0x80))
		{
			// quant/unquant rounded UP so that the top two bits changed;
			// decrement the input value in hopes that this will avoid rounding up.
			value_to_quantize--;
		}
		else if ((uquantval & 0x80) < (value_to_quantize & 0x80))
		{
			// quant/unquant rounded DOWN so that the top two bits changed;
			// decrement the input value in hopes that this will avoid rounding down.
			value_to_quantize--;
		}
	}
	while (perform_loop);

	*quantized_value = quantval;
	*unquantized_value = uquantval;
}





/*
   HDR color encoding, take #3 */


void quantize_hdr_rgbo3(float4 color, int output[4], int quantization_level)
{
	color.xyz = color.xyz + color.www;

	if (!(color.x > 0.0f))
		color.x = 0.0f;
	else if (color.x > 65535.0f)
		color.x = 65535.0f;

	if (!(color.y > 0.0f))
		color.y = 0.0f;
	else if (color.y > 65535.0f)
		color.y = 65535.0f;

	if (!(color.z > 0.0f))
		color.z = 0.0f;
	else if (color.z > 65535.0f)
		color.z = 65535.0f;

	if (!(color.w > 0.0f))
		color.w = 0.0f;
	else if (color.w > 65535.0f)
		color.w = 65535.0f;

	float4 color_bak = color;
	int majcomp;
	if (color.x > color.y && color.x > color.z)
		majcomp = 0;			// red is largest component
	else if (color.y > color.z)
		majcomp = 1;			// green is largest component
	else
		majcomp = 2;			// blue is largest component

	// swap around the red component and the largest component.
	switch (majcomp)
	{
	case 1:
		color = color.yxzw;
		break;
	case 2:
		color = color.zyxw;
		break;
	default:
		break;
	}

	static const int mode_bits[5][3] = {
		{11, 5, 7},
		{11, 6, 5},
		{10, 5, 8},
		{9, 6, 7},
		{8, 7, 6}
	};


	static const float mode_cutoffs[5][2] = {
		{1024, 4096},
		{2048, 1024},
		{2048, 16384},
		{8192, 16384},
		{32768, 16384}
	};

	static const float mode_rscales[5] = {
		32.0f,
		32.0f,
		64.0f,
		128.0f,
		256.0f,
	};

	static const float mode_scales[5] = {
		1.0f / 32.0f,
		1.0f / 32.0f,
		1.0f / 64.0f,
		1.0f / 128.0f,
		1.0f / 256.0f,
	};

	float r_base = color.x;
	float g_base = color.x - color.y;
	float b_base = color.x - color.z;
	float s_base = color.w;

	int mode;
	for (mode = 0; mode < 5; mode++)
	{
		if (g_base > mode_cutoffs[mode][0] || b_base > mode_cutoffs[mode][0] || s_base > mode_cutoffs[mode][1])
		{
			continue;
		}

		// encode the mode into a 4-bit vector.
		int mode_enc = mode < 4 ? (mode | (majcomp << 2)) : (majcomp | 0xC);

		float mode_scale = mode_scales[mode];
		float mode_rscale = mode_rscales[mode];

		int gb_intcutoff = 1 << mode_bits[mode][1];
		int s_intcutoff = 1 << mode_bits[mode][2];

		// first, quantize and unquantize R.
		int r_intval = (int)floor(r_base * mode_scale + 0.5f);

		int r_lowbits = r_intval & 0x3f;

		r_lowbits |= (mode_enc & 3) << 6;

		int r_quantval;
		int r_uquantval;
		quantize_and_unquantize_retain_top_two_bits(quantization_level, r_lowbits, &r_quantval, &r_uquantval);

		r_intval = (r_intval & ~0x3f) | (r_uquantval & 0x3f);
		float r_fval = r_intval * mode_rscale;


		// next, recompute G and B, then quantize and unquantize them.
		float g_fval = r_fval - color.y;
		float b_fval = r_fval - color.z;
		if (g_fval < 0.0f)
			g_fval = 0.0f;
		else if (g_fval > 65535.0f)
			g_fval = 65535.0f;
		if (b_fval < 0.0f)
			b_fval = 0.0f;
		else if (b_fval > 65535.0f)
			b_fval = 65535.0f;

		int g_intval = (int)floor(g_fval * mode_scale + 0.5f);
		int b_intval = (int)floor(b_fval * mode_scale + 0.5f);


		if (g_intval >= gb_intcutoff || b_intval >= gb_intcutoff)
		{
			continue;
		}

		int g_lowbits = g_intval & 0x1f;
		int b_lowbits = b_intval & 0x1f;

		int bit0 = 0;
		int bit1 = 0;
		int bit2 = 0;
		int bit3 = 0;

		switch (mode)
		{
		case 0:
		case 2:
			bit0 = (r_intval >> 9) & 1;
			break;
		case 1:
		case 3:
			bit0 = (r_intval >> 8) & 1;
			break;
		case 4:
		case 5:
			bit0 = (g_intval >> 6) & 1;
			break;
		}

		switch (mode)
		{
		case 0:
		case 1:
		case 2:
		case 3:
			bit2 = (r_intval >> 7) & 1;
			break;
		case 4:
		case 5:
			bit2 = (b_intval >> 6) & 1;
			break;
		}

		switch (mode)
		{
		case 0:
		case 2:
			bit1 = (r_intval >> 8) & 1;
			break;
		case 1:
		case 3:
		case 4:
		case 5:
			bit1 = (g_intval >> 5) & 1;
			break;
		}

		switch (mode)
		{
		case 0:
			bit3 = (r_intval >> 10) & 1;
			break;
		case 2:
			bit3 = (r_intval >> 6) & 1;
			break;
		case 1:
		case 3:
		case 4:
		case 5:
			bit3 = (b_intval >> 5) & 1;
			break;
		}

		g_lowbits |= (mode_enc & 0x4) << 5;
		b_lowbits |= (mode_enc & 0x8) << 4;

		g_lowbits |= bit0 << 6;
		g_lowbits |= bit1 << 5;
		b_lowbits |= bit2 << 6;
		b_lowbits |= bit3 << 5;

		int g_quantval;
		int b_quantval;
		int g_uquantval;
		int b_uquantval;

		quantize_and_unquantize_retain_top_four_bits(quantization_level, g_lowbits, &g_quantval, &g_uquantval);

		quantize_and_unquantize_retain_top_four_bits(quantization_level, b_lowbits, &b_quantval, &b_uquantval);

		g_intval = (g_intval & ~0x1f) | (g_uquantval & 0x1f);
		b_intval = (b_intval & ~0x1f) | (b_uquantval & 0x1f);

		g_fval = g_intval * mode_rscale;
		b_fval = b_intval * mode_rscale;


		// finally, recompute the scale value, based on the errors
		// introduced to red, green and blue.

		// If the error is positive, then the R,G,B errors combined have raised the color
		// value overall; as such, the scale value needs to be increased.
		float rgb_errorsum = (r_fval - color.x) + (r_fval - g_fval - color.y) + (r_fval - b_fval - color.z);

		float s_fval = s_base + rgb_errorsum * (1.0f / 3.0f);
		if (s_fval < 0.0f)
			s_fval = 0.0f;
		else if (s_fval > 1e9)
			s_fval = 1e9;

		int s_intval = (int)floor(s_fval * mode_scale + 0.5f);

		if (s_intval >= s_intcutoff)
		{
			continue;
		}

		int s_lowbits = s_intval & 0x1f;

		int bit4;
		int bit5;
		int bit6;
		switch (mode)
		{
		case 1:
			bit6 = (r_intval >> 9) & 1;
			break;
		default:
			bit6 = (s_intval >> 5) & 1;
			break;
		}

		switch (mode)
		{
		case 4:
			bit5 = (r_intval >> 7) & 1;
			break;
		case 1:
			bit5 = (r_intval >> 10) & 1;
			break;
		default:
			bit5 = (s_intval >> 6) & 1;
			break;
		}

		switch (mode)
		{
		case 2:
			bit4 = (s_intval >> 7) & 1;
			break;
		default:
			bit4 = (r_intval >> 6) & 1;
			break;
		}


		s_lowbits |= bit6 << 5;
		s_lowbits |= bit5 << 6;
		s_lowbits |= bit4 << 7;

		int s_quantval;
		int s_uquantval;

		quantize_and_unquantize_retain_top_four_bits(quantization_level, s_lowbits, &s_quantval, &s_uquantval);

		s_intval = (s_intval & ~0x1f) | (s_uquantval & 0x1f);
		s_fval = s_intval * mode_rscale;
		output[0] = r_quantval;
		output[1] = g_quantval;
		output[2] = b_quantval;
		output[3] = s_quantval;

		return;
	}

	// failed to encode any of the modes above? In that case,
	// encode using mode #5.
	int i;

	float vals[4];
	int ivals[4];
	vals[0] = color_bak.x;
	vals[1] = color_bak.y;
	vals[2] = color_bak.z;
	vals[3] = color_bak.w;

	float cvals[3];

	for (i = 0; i < 3; i++)
	{
		if (vals[i] < 0.0f)
			vals[i] = 0.0f;
		else if (vals[i] > 65020.0f)
			vals[i] = 65020.0f;

		ivals[i] = (int)floor(vals[i] * (1.0f / 512.0f) + 0.5f);
		cvals[i] = ivals[i] * 512.0f;
	}

	float rgb_errorsum = (cvals[0] - vals[0]) + (cvals[1] - vals[1]) + (cvals[2] - vals[2]);
	vals[3] += rgb_errorsum * (1.0f / 3.0f);

	if (vals[3] < 0.0f)
		vals[3] = 0.0f;
	else if (vals[3] > 65020.0f)
		vals[3] = 65020.0f;

	ivals[3] = (int)floor(vals[3] * (1.0f / 512.0f) + 0.5f);

	int encvals[4];

	encvals[0] = (ivals[0] & 0x3f) | 0xC0;
	encvals[1] = (ivals[1] & 0x7f) | 0x80;
	encvals[2] = (ivals[2] & 0x7f) | 0x80;
	encvals[3] = (ivals[3] & 0x7f) | ((ivals[0] & 0x40) << 1);

	for (i = 0; i < 4; i++)
	{
		int dummy;
		quantize_and_unquantize_retain_top_four_bits(quantization_level, encvals[i], &(output[i]), &dummy);
	}

	return;
}







void quantize_hdr_rgb3(float4 color0, float4 color1, int output[6], int quantization_level)
{
	if (!(color0.x > 0.0f))
		color0.x = 0.0f;
	else if (color0.x > 65535.0f)
		color0.x = 65535.0f;

	if (!(color0.y > 0.0f))
		color0.y = 0.0f;
	else if (color0.y > 65535.0f)
		color0.y = 65535.0f;

	if (!(color0.z > 0.0f))
		color0.z = 0.0f;
	else if (color0.z > 65535.0f)
		color0.z = 65535.0f;

	if (!(color1.x > 0.0f))
		color1.x = 0.0f;
	else if (color1.x > 65535.0f)
		color1.x = 65535.0f;

	if (!(color1.y > 0.0f))
		color1.y = 0.0f;
	else if (color1.y > 65535.0f)
		color1.y = 65535.0f;

	if (!(color1.z > 0.0f))
		color1.z = 0.0f;
	else if (color1.z > 65535.0f)
		color1.z = 65535.0f;

	float4 color0_bak = color0;
	float4 color1_bak = color1;

	int majcomp;
	if (color1.x > color1.y && color1.x > color1.z)
		majcomp = 0;			// red is largest
	else if (color1.y > color1.z)
		majcomp = 1;			// green is largest
	else
		majcomp = 2;			// blue is largest

	// swizzle the components
	switch (majcomp)
	{
	case 1:					// red-green swap
		color0 = color0.yxzw;
		color1 = color1.yxzw;
		break;
	case 2:					// red-blue swap
		color0 = color0.zyxw;
		color1 = color1.zyxw;
		break;
	default:
		break;
	}

	float a_base = color1.x;
	if (a_base < 0.0f)
		a_base = 0.0f;
	else if (a_base > 65535.0f)
		a_base = 65535.0f;


	float b0_base = a_base - color1.y;
	float b1_base = a_base - color1.z;
	float c_base = a_base - color0.x;
	float d0_base = a_base - b0_base - c_base - color0.y;
	float d1_base = a_base - b1_base - c_base - color0.z;



	// number of bits in the various fields in the various modes
	static const int mode_bits[8][4] = {
		{9, 7, 6, 7},
		{9, 8, 6, 6},
		{10, 6, 7, 7},
		{10, 7, 7, 6},
		{11, 8, 6, 5},
		{11, 6, 8, 6},
		{12, 7, 7, 5},
		{12, 6, 7, 6}
	};

	// cutoffs to use for the computed values of a,b,c,d, assuming the
	// range 0..65535 are LNS values corresponding to fp16.
	static const float mode_cutoffs[8][4] = {
		{16384, 8192, 8192, 8},	// mode 0: 9,7,6,7
		{32768, 8192, 4096, 8},	// mode 1: 9,8,6,6
		{4096, 8192, 4096, 4},	// mode 2: 10,6,7,7
		{8192, 8192, 2048, 4},	// mode 3: 10,7,7,6
		{8192, 2048, 512, 2},	// mode 4: 11,8,6,5
		{2048, 8192, 1024, 2},	// mode 5: 11,6,8,6
		{2048, 2048, 256, 1},	// mode 6: 12,7,7,5
		{1024, 2048, 512, 1},	// mode 7: 12,6,7,6
	};

	static const float mode_scales[8] = {
		1.0f / 128.0f,
		1.0f / 128.0f,
		1.0f / 64.0f,
		1.0f / 64.0f,
		1.0f / 32.0f,
		1.0f / 32.0f,
		1.0f / 16.0f,
		1.0f / 16.0f,
	};

	// scaling factors when going from what was encoded in the mode to 16 bits.
	static const float mode_rscales[8] = {
		128.0f,
		128.0f,
		64.0f,
		64.0f,
		32.0f,
		32.0f,
		16.0f,
		16.0f
	};


	// try modes one by one, with the highest-precision mode first.
	int mode;
	for (mode = 7; mode >= 0; mode--)
	{
		// for each mode, test if we can in fact accommodate
		// the computed b,c,d values. If we clearly can't, then we skip to the next mode.

		float b_cutoff = mode_cutoffs[mode][0];
		float c_cutoff = mode_cutoffs[mode][1];
		float d_cutoff = mode_cutoffs[mode][2];

		if (b0_base > b_cutoff || b1_base > b_cutoff || c_base > c_cutoff || fabs(d0_base) > d_cutoff || fabs(d1_base) > d_cutoff)
		{
			continue;
		}

		float mode_scale = mode_scales[mode];
		float mode_rscale = mode_rscales[mode];

		int b_intcutoff = 1 << mode_bits[mode][1];
		int c_intcutoff = 1 << mode_bits[mode][2];
		int d_intcutoff = 1 << (mode_bits[mode][3] - 1);

		// first, quantize and unquantize A, with the assumption that its high bits can be handled safely.
		int a_intval = (int)floor(a_base * mode_scale + 0.5f);
		int a_lowbits = a_intval & 0xFF;

		int a_quantval = color_quantization_tables[quantization_level][a_lowbits];
		int a_uquantval = color_unquantization_tables[quantization_level][a_quantval];
		a_intval = (a_intval & ~0xFF) | a_uquantval;
		float a_fval = a_intval * mode_rscale;

		// next, recompute C, then quantize and unquantize it
		float c_fval = a_fval - color0.x;
		if (c_fval < 0.0f)
			c_fval = 0.0f;
		else if (c_fval > 65535.0f)
			c_fval = 65535.0f;

		int c_intval = (int)floor(c_fval * mode_scale + 0.5f);

		if (c_intval >= c_intcutoff)
		{
			continue;
		}

		int c_lowbits = c_intval & 0x3f;

		c_lowbits |= (mode & 1) << 7;
		c_lowbits |= (a_intval & 0x100) >> 2;

		int c_quantval;
		int c_uquantval;
		quantize_and_unquantize_retain_top_two_bits(quantization_level, c_lowbits, &c_quantval, &c_uquantval);
		c_intval = (c_intval & ~0x3F) | (c_uquantval & 0x3F);
		c_fval = c_intval * mode_rscale;


		// next, recompute B0 and B1, then quantize and unquantize them
		float b0_fval = a_fval - color1.y;
		float b1_fval = a_fval - color1.z;
		if (b0_fval < 0.0f)
			b0_fval = 0.0f;
		else if (b0_fval > 65535.0f)
			b0_fval = 65535.0f;
		if (b1_fval < 0.0f)
			b1_fval = 0.0f;
		else if (b1_fval > 65535.0f)
			b1_fval = 65535.0f;

		int b0_intval = (int)floor(b0_fval * mode_scale + 0.5f);
		int b1_intval = (int)floor(b1_fval * mode_scale + 0.5f);

		if (b0_intval >= b_intcutoff || b1_intval >= b_intcutoff)
		{
			continue;
		}



		int b0_lowbits = b0_intval & 0x3f;
		int b1_lowbits = b1_intval & 0x3f;

		int bit0 = 0;
		int bit1 = 0;
		switch (mode)
		{
		case 0:
		case 1:
		case 3:
		case 4:
		case 6:
			bit0 = (b0_intval >> 6) & 1;
			break;
		case 2:
		case 5:
		case 7:
			bit0 = (a_intval >> 9) & 1;
			break;
		}

		switch (mode)
		{
		case 0:
		case 1:
		case 3:
		case 4:
		case 6:
			bit1 = (b1_intval >> 6) & 1;
			break;
		case 2:
			bit1 = (c_intval >> 6) & 1;
			break;
		case 5:
		case 7:
			bit1 = (a_intval >> 10) & 1;
			break;
		}

		b0_lowbits |= bit0 << 6;
		b1_lowbits |= bit1 << 6;

		b0_lowbits |= ((mode >> 1) & 1) << 7;
		b1_lowbits |= ((mode >> 2) & 1) << 7;

		int b0_quantval;
		int b1_quantval;
		int b0_uquantval;
		int b1_uquantval;

		quantize_and_unquantize_retain_top_two_bits(quantization_level, b0_lowbits, &b0_quantval, &b0_uquantval);

		quantize_and_unquantize_retain_top_two_bits(quantization_level, b1_lowbits, &b1_quantval, &b1_uquantval);

		b0_intval = (b0_intval & ~0x3f) | (b0_uquantval & 0x3f);
		b1_intval = (b1_intval & ~0x3f) | (b1_uquantval & 0x3f);
		b0_fval = b0_intval * mode_rscale;
		b1_fval = b1_intval * mode_rscale;


		// finally, recompute D0 and D1, then quantize and unquantize them
		float d0_fval = a_fval - b0_fval - c_fval - color0.y;
		float d1_fval = a_fval - b1_fval - c_fval - color0.z;

		if (d0_fval < -65535.0f)
			d0_fval = -65535.0f;
		else if (d0_fval > 65535.0f)
			d0_fval = 65535.0f;

		if (d1_fval < -65535.0f)
			d1_fval = -65535.0f;
		else if (d1_fval > 65535.0f)
			d1_fval = 65535.0f;

		int d0_intval = (int)floor(d0_fval * mode_scale + 0.5f);
		int d1_intval = (int)floor(d1_fval * mode_scale + 0.5f);

		if (abs(d0_intval) >= d_intcutoff || abs(d1_intval) >= d_intcutoff)
			continue;

		// d0_intval += mode_dbiases[mode];
		// d1_intval += mode_dbiases[mode];

		int d0_lowbits = d0_intval & 0x1f;
		int d1_lowbits = d1_intval & 0x1f;

		int bit2 = 0;
		int bit3 = 0;
		int bit4;
		int bit5;
		switch (mode)
		{
		case 0:
		case 2:
			bit2 = (d0_intval >> 6) & 1;
			break;
		case 1:
		case 4:
			bit2 = (b0_intval >> 7) & 1;
			break;
		case 3:
			bit2 = (a_intval >> 9) & 1;
			break;
		case 5:
			bit2 = (c_intval >> 7) & 1;
			break;
		case 6:
		case 7:
			bit2 = (a_intval >> 11) & 1;
			break;
		}
		switch (mode)
		{
		case 0:
		case 2:
			bit3 = (d1_intval >> 6) & 1;
			break;
		case 1:
		case 4:
			bit3 = (b1_intval >> 7) & 1;
			break;
		case 3:
		case 5:
		case 6:
		case 7:
			bit3 = (c_intval >> 6) & 1;
			break;
		}

		switch (mode)
		{
		case 4:
		case 6:
			bit4 = (a_intval >> 9) & 1;
			bit5 = (a_intval >> 10) & 1;
			break;
		default:
			bit4 = (d0_intval >> 5) & 1;
			bit5 = (d1_intval >> 5) & 1;
			break;
		}

		d0_lowbits |= bit2 << 6;
		d1_lowbits |= bit3 << 6;
		d0_lowbits |= bit4 << 5;
		d1_lowbits |= bit5 << 5;

		d0_lowbits |= (majcomp & 1) << 7;
		d1_lowbits |= ((majcomp >> 1) & 1) << 7;

		int d0_quantval;
		int d1_quantval;
		int d0_uquantval;
		int d1_uquantval;

		quantize_and_unquantize_retain_top_four_bits(quantization_level, d0_lowbits, &d0_quantval, &d0_uquantval);

		quantize_and_unquantize_retain_top_four_bits(quantization_level, d1_lowbits, &d1_quantval, &d1_uquantval);

		output[0] = a_quantval;
		output[1] = c_quantval;
		output[2] = b0_quantval;
		output[3] = b1_quantval;
		output[4] = d0_quantval;
		output[5] = d1_quantval;
		return;
	}

	// neither of the modes fit? In this case, we will use a flat representation
	// for storing data, using 8 bits for red and green, and 7 bits for blue.
	// This gives color accuracy roughly similar to LDR 4:4:3 which is not at all great
	// but usable. This representation is used if the light color is more than 4x the
	// color value of the dark color.
	int i;
	float vals[6];
	vals[0] = color0_bak.x;
	vals[1] = color1_bak.x;
	vals[2] = color0_bak.y;
	vals[3] = color1_bak.y;
	vals[4] = color0_bak.z;
	vals[5] = color1_bak.z;


	for (i = 0; i < 6; i++)
	{
		if (vals[i] < 0.0f)
			vals[i] = 0.0f;
		else if (vals[i] > 65020.0f)
			vals[i] = 65020.0f;
	}
	for (i = 0; i < 4; i++)
	{
		int idx = (int)floor(vals[i] * 1.0f / 256.0f + 0.5f);
		output[i] = color_quantization_tables[quantization_level][idx];
	}
	for (i = 4; i < 6; i++)
	{
		int dummy;
		int idx = (int)floor(vals[i] * 1.0f / 512.0f + 0.5f) + 128;
		quantize_and_unquantize_retain_top_two_bits(quantization_level, idx, &(output[i]), &dummy);
	}

	return;
}





void quantize_hdr_rgb_ldr_alpha3(float4 color0, float4 color1, int output[8], int quantization_level)
{
	color0.w *= (1.0f / 257.0f);
	color1.w *= (1.0f / 257.0f);

	quantize_hdr_rgb3(color0, color1, output, quantization_level);

	float a0 = clamp255(color0.w);
	float a1 = clamp255(color1.w);
	int ai0 = color_quantization_tables[quantization_level][(int)floor(a0 + 0.5f)];
	int ai1 = color_quantization_tables[quantization_level][(int)floor(a1 + 0.5f)];

	output[6] = ai0;
	output[7] = ai1;
}



void quantize_hdr_luminance_large_range3(float4 color0, float4 color1, int output[2], int quantization_level)
{

	float lum1 = (color1.x + color1.y + color1.z) * (1.0f / 3.0f);
	float lum0 = (color0.x + color0.y + color0.z) * (1.0f / 3.0f);

	if (lum1 < lum0)
	{
		float avg = (lum0 + lum1) * 0.5f;
		lum0 = avg;
		lum1 = avg;
	}

	int ilum1 = static_cast < int >(floor(lum1 + 0.5f));
	int ilum0 = static_cast < int >(floor(lum0 + 0.5f));

	// find the closest encodable point in the upper half of the code-point space
	int upper_v0 = (ilum0 + 128) >> 8;
	int upper_v1 = (ilum1 + 128) >> 8;

	if (upper_v0 < 0)
		upper_v0 = 0;
	else if (upper_v0 > 255)
		upper_v0 = 255;

	if (upper_v1 < 0)
		upper_v1 = 0;
	else if (upper_v1 > 255)
		upper_v1 = 255;

	// find the closest encodable point in the lower half of the code-point space
	int lower_v0 = (ilum1 + 256) >> 8;
	int lower_v1 = ilum0 >> 8;

	if (lower_v0 < 0)
		lower_v0 = 0;
	else if (lower_v0 > 255)
		lower_v0 = 255;

	if (lower_v1 < 0)
		lower_v1 = 0;
	else if (lower_v1 > 255)
		lower_v1 = 255;

	// determine the distance between the point in code-point space and the input value
	int upper0_dec = upper_v0 << 8;
	int upper1_dec = upper_v1 << 8;
	int lower0_dec = (lower_v1 << 8) + 128;
	int lower1_dec = (lower_v0 << 8) - 128;


	int upper0_diff = upper0_dec - ilum0;
	int upper1_diff = upper1_dec - ilum1;
	int lower0_diff = lower0_dec - ilum0;
	int lower1_diff = lower1_dec - ilum1;

	int upper_error = (upper0_diff * upper0_diff) + (upper1_diff * upper1_diff);
	int lower_error = (lower0_diff * lower0_diff) + (lower1_diff * lower1_diff);

	int v0, v1;
	if (upper_error < lower_error)
	{
		v0 = upper_v0;
		v1 = upper_v1;
	}
	else
	{
		v0 = lower_v0;
		v1 = lower_v1;
	}

	// OK; encode.
	output[0] = color_quantization_tables[quantization_level][v0];
	output[1] = color_quantization_tables[quantization_level][v1];
}



int try_quantize_hdr_luminance_small_range3(float4 color0, float4 color1, int output[2], int quantization_level)
{
	float lum1 = (color1.x + color1.y + color1.z) * (1.0f / 3.0f);
	float lum0 = (color0.x + color0.y + color0.z) * (1.0f / 3.0f);

	if (lum1 < lum0)
	{
		float avg = (lum0 + lum1) * 0.5f;
		lum0 = avg;
		lum1 = avg;
	}

	int ilum1 = static_cast < int >(floor(lum1 + 0.5f));
	int ilum0 = static_cast < int >(floor(lum0 + 0.5f));

	// difference of more than a factor-of-2 results in immediate failure.
	if (ilum1 - ilum0 > 2048)
		return 0;

	int lowval, highval, diffval;
	int v0, v1;
	int v0e, v1e;
	int v0d, v1d;

	// first, try to encode the high-precision submode
	lowval = (ilum0 + 16) >> 5;
	highval = (ilum1 + 16) >> 5;

	if (lowval < 0)
		lowval = 0;
	else if (lowval > 2047)
		lowval = 2047;

	if (highval < 0)
		highval = 0;
	else if (highval > 2047)
		highval = 2047;

	v0 = lowval & 0x7F;
	v0e = color_quantization_tables[quantization_level][v0];
	v0d = color_unquantization_tables[quantization_level][v0e];
	if ((v0d & 0x80) == 0x80)
		goto LOW_PRECISION_SUBMODE;

	lowval = (lowval & ~0x7F) | (v0d & 0x7F);
	diffval = highval - lowval;
	if (diffval < 0 || diffval > 15)
		goto LOW_PRECISION_SUBMODE;

	v1 = ((lowval >> 3) & 0xF0) | diffval;
	v1e = color_quantization_tables[quantization_level][v1];
	v1d = color_unquantization_tables[quantization_level][v1e];
	if ((v1d & 0xF0) != (v1 & 0xF0))
		goto LOW_PRECISION_SUBMODE;

	output[0] = v0e;
	output[1] = v1e;
	return 1;


	// failed to encode the high-precision submode; well, then try to encode the
	// low-precision submode.
  LOW_PRECISION_SUBMODE:

	lowval = (ilum0 + 32) >> 6;
	highval = (ilum1 + 32) >> 6;
	if (lowval < 0)
		lowval = 0;
	else if (lowval > 1023)
		lowval = 1023;
	if (highval < 0)
		highval = 0;
	else if (highval > 1023)
		highval = 1023;

	v0 = (lowval & 0x7F) | 0x80;
	v0e = color_quantization_tables[quantization_level][v0];
	v0d = color_unquantization_tables[quantization_level][v0e];
	if ((v0d & 0x80) == 0)
		return 0;

	lowval = (lowval & ~0x7F) | (v0d & 0x7F);
	diffval = highval - lowval;
	if (diffval < 0 || diffval > 31)
		return 0;

	v1 = ((lowval >> 2) & 0xE0) | diffval;
	v1e = color_quantization_tables[quantization_level][v1];
	v1d = color_unquantization_tables[quantization_level][v1e];
	if ((v1d & 0xE0) != (v1 & 0xE0))
		return 0;;

	output[0] = v0e;
	output[1] = v1e;
	return 1;
}


void quantize_hdr_alpha3(float alpha0, float alpha1, int output[2], int quantization_level)
{
	int i;

	if (alpha0 < 0)
		alpha0 = 0;
	else if (alpha0 > 65280)
		alpha0 = 65280;

	if (alpha1 < 0)
		alpha1 = 0;
	else if (alpha1 > 65280)
		alpha1 = 65280;

	int ialpha0 = static_cast < int >(floor(alpha0 + 0.5f));
	int ialpha1 = static_cast < int >(floor(alpha1 + 0.5f));

	int val0, val1, diffval;
	int v6, v7;
	int v6e, v7e;
	int v6d, v7d;

	// try to encode one of the delta submodes, in decreasing-precision order.
	for (i = 2; i >= 0; i--)
	{
		val0 = (ialpha0 + (128 >> i)) >> (8 - i);
		val1 = (ialpha1 + (128 >> i)) >> (8 - i);

		v6 = (val0 & 0x7F) | ((i & 1) << 7);
		v6e = color_quantization_tables[quantization_level][v6];
		v6d = color_unquantization_tables[quantization_level][v6e];

		if ((v6 ^ v6d) & 0x80)
			continue;

		val0 = (val0 & ~0x7f) | (v6d & 0x7f);
		diffval = val1 - val0;
		int cutoff = 32 >> i;
		int mask = 2 * cutoff - 1;

		if (diffval < -cutoff || diffval >= cutoff)
			continue;

		v7 = ((i & 2) << 6) | ((val0 >> 7) << (6 - i)) | (diffval & mask);
		v7e = color_quantization_tables[quantization_level][v7];
		v7d = color_unquantization_tables[quantization_level][v7e];

		static const int testbits[3] = { 0xE0, 0xF0, 0xF8 };

		if ((v7 ^ v7d) & testbits[i])
			continue;

		output[0] = v6e;
		output[1] = v7e;
		return;
	}

	// could not encode any of the delta modes; instead encode a flat value
	val0 = (ialpha0 + 256) >> 9;
	val1 = (ialpha1 + 256) >> 9;
	v6 = val0 | 0x80;
	v7 = val1 | 0x80;

	v6e = color_quantization_tables[quantization_level][v6];
	v7e = color_quantization_tables[quantization_level][v7];
	output[0] = v6e;
	output[1] = v7e;

	return;
}



void quantize_hdr_rgb_alpha3(float4 color0, float4 color1, int output[8], int quantization_level)
{
	quantize_hdr_rgb3(color0, color1, output, quantization_level);
	quantize_hdr_alpha3(color0.w, color1.w, output + 6, quantization_level);
}



/*
	Quantize a color. When quantizing an RGB or RGBA color, the quantizer may choose a
	delta-based representation; as such, it will report back the format it actually used.
*/
int pack_color_endpoints(astc_decode_mode decode_mode, float4 color0, float4 color1, float4 rgbs_color, float4 rgbo_color, float2 luminances,	// ! Unused
						 int format, int *output, int quantization_level)
{

	IGNORE(luminances);

	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("%s : format=%d  quantization_level=%d\n", __func__, format, quantization_level);
			printf("Color 0: <%g %g %g %g>\n", color0.x, color0.y, color0.z, color0.w);
			printf("Color 1: <%g %g %g %g>\n", color1.x, color1.y, color1.z, color1.w);

		}
	#endif

	// we do not support negative colors.
	color0.x = MAX(color0.x, 0.0f);
	color0.y = MAX(color0.y, 0.0f);
	color0.z = MAX(color0.z, 0.0f);
	color0.w = MAX(color0.w, 0.0f);
	color1.x = MAX(color1.x, 0.0f);
	color1.y = MAX(color1.y, 0.0f);
	color1.z = MAX(color1.z, 0.0f);
	color1.w = MAX(color1.w, 0.0f);


	int retval;

	switch (format)
	{
	case FMT_RGB:
		if (quantization_level <= 18)
		{
			if (try_quantize_rgb_delta_blue_contract(color0, color1, output, quantization_level))
			{
				retval = FMT_RGB_DELTA;
				break;
			}
			if (try_quantize_rgb_delta(color0, color1, output, quantization_level))
			{
				retval = FMT_RGB_DELTA;
				break;
			}
		}
		if (try_quantize_rgb_blue_contract(color0, color1, output, quantization_level))
		{
			retval = FMT_RGB;
			break;
		}
		quantize_rgb(color0, color1, output, quantization_level);
		retval = FMT_RGB;
		break;

	case FMT_RGBA:
		if (quantization_level <= 18)
		{
			if (try_quantize_rgba_delta_blue_contract(color0, color1, output, quantization_level))
			{
				retval = FMT_RGBA_DELTA;
				break;
			}
			if (try_quantize_rgba_delta(color0, color1, output, quantization_level))
			{
				retval = FMT_RGBA_DELTA;
				break;
			}
		}
		if (try_quantize_rgba_blue_contract(color0, color1, output, quantization_level))
		{
			retval = FMT_RGBA;
			break;
		}
		quantize_rgba(color0, color1, output, quantization_level);
		retval = FMT_RGBA;
		break;

	case FMT_RGB_SCALE:
		quantize_rgbs_new(rgbs_color, output, quantization_level);
		// quantize_rgbs( color0, color1, output, quantization_level );
		retval = FMT_RGB_SCALE;
		break;

	case FMT_HDR_RGB_SCALE:
		quantize_hdr_rgbo3(rgbo_color, output, quantization_level);

		// quantize_hdr_rgb_scale( rgbo_color, output, quantization_level );
		retval = FMT_HDR_RGB_SCALE;
		break;

	case FMT_HDR_RGB:
		quantize_hdr_rgb3(color0, color1, output, quantization_level);

		// quantize_hdr_rgb_rgba( color0, color1, 0, output, quantization_level );
		retval = FMT_HDR_RGB;
		break;

	case FMT_RGB_SCALE_ALPHA:
		quantize_rgbs_alpha_new(color0, color1, rgbs_color, output, quantization_level);
		// quantize_rgbs_alpha( color0, color1, output, quantization_level );
		retval = FMT_RGB_SCALE_ALPHA;
		break;

	case FMT_HDR_LUMINANCE_SMALL_RANGE:
	case FMT_HDR_LUMINANCE_LARGE_RANGE:
		if (try_quantize_hdr_luminance_small_range3(color0, color1, output, quantization_level))
		{
			retval = FMT_HDR_LUMINANCE_SMALL_RANGE;
			break;
		}
		quantize_hdr_luminance_large_range3(color0, color1, output, quantization_level);
		retval = FMT_HDR_LUMINANCE_LARGE_RANGE;
		break;

	case FMT_LUMINANCE:
		quantize_luminance(color0, color1, output, quantization_level);
		retval = FMT_LUMINANCE;
		break;

	case FMT_LUMINANCE_ALPHA:
		if (quantization_level <= 18)
		{
			if (try_quantize_luminance_alpha_delta(color0, color1, output, quantization_level))
			{
				retval = FMT_LUMINANCE_ALPHA_DELTA;
				break;
			}
		}
		quantize_luminance_alpha(color0, color1, output, quantization_level);
		retval = FMT_LUMINANCE_ALPHA;
		break;

	case FMT_HDR_RGB_LDR_ALPHA:
		quantize_hdr_rgb_ldr_alpha3(color0, color1, output, quantization_level);
		retval = FMT_HDR_RGB_LDR_ALPHA;
		break;

	case FMT_HDR_RGBA:
		quantize_hdr_rgb_alpha3(color0, color1, output, quantization_level);
		retval = FMT_HDR_RGBA;
		break;

	default:
		ASTC_CODEC_INTERNAL_ERROR;
		quantize0(color0, color1, output, quantization_level);
		retval = FMT_LUMINANCE;
		break;
	}

	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			int i;
			printf("Quantized to format %d\n", retval);
			printf("Quantized color:");

			for (i = 0; i < 8; i++)
				printf(" %X", output[i]);

			ushort4 res0;
			ushort4 res1;
			int rgb_hdr;
			int alpha_hdr;
			int nan_endpoint;

			unpack_color_endpoints(decode_mode, retval, quantization_level, output, &rgb_hdr, &alpha_hdr, &nan_endpoint, &res0, &res1);
			printf("rgb-hdr=%d alpha-hdr=%d nan-endpoint=%d\n", rgb_hdr, alpha_hdr, nan_endpoint);

			printf("Unquantized color 0: <%u %u %u %u>\n", res0.x, res0.y, res0.z, res0.w);
			printf("Unquantized color 1: <%u %u %u %u>\n", res1.x, res1.y, res1.z, res1.w);
			printf("\n\n");
		}
	#endif

	return retval;
}
