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
 *	@brief	Color unquantization functions for ASTC.
 */
/*----------------------------------------------------------------------------*/

#include "astc_codec_internals.h"

#include "mathlib.h"
#include "softfloat.h"

int rgb_delta_unpack(const int input[6], int quantization_level, ushort4 * output0, ushort4 * output1)
{
	// unquantize the color endpoints
	int r0 = color_unquantization_tables[quantization_level][input[0]];
	int g0 = color_unquantization_tables[quantization_level][input[2]];
	int b0 = color_unquantization_tables[quantization_level][input[4]];

	int r1 = color_unquantization_tables[quantization_level][input[1]];
	int g1 = color_unquantization_tables[quantization_level][input[3]];
	int b1 = color_unquantization_tables[quantization_level][input[5]];

	// perform the bit-transfer procedure
	r0 |= (r1 & 0x80) << 1;
	g0 |= (g1 & 0x80) << 1;
	b0 |= (b1 & 0x80) << 1;
	r1 &= 0x7F;
	g1 &= 0x7F;
	b1 &= 0x7F;
	if (r1 & 0x40)
		r1 -= 0x80;
	if (g1 & 0x40)
		g1 -= 0x80;
	if (b1 & 0x40)
		b1 -= 0x80;

	r0 >>= 1;
	g0 >>= 1;
	b0 >>= 1;
	r1 >>= 1;
	g1 >>= 1;
	b1 >>= 1;

	int rgbsum = r1 + g1 + b1;

	r1 += r0;
	g1 += g0;
	b1 += b0;


	int retval;

	int r0e, g0e, b0e;
	int r1e, g1e, b1e;

	if (rgbsum >= 0)
	{
		r0e = r0;
		g0e = g0;
		b0e = b0;

		r1e = r1;
		g1e = g1;
		b1e = b1;

		retval = 0;
	}
	else
	{
		r0e = (r1 + b1) >> 1;
		g0e = (g1 + b1) >> 1;
		b0e = b1;

		r1e = (r0 + b0) >> 1;
		g1e = (g0 + b0) >> 1;
		b1e = b0;

		retval = 1;
	}

	if (r0e < 0)
		r0e = 0;
	else if (r0e > 255)
		r0e = 255;

	if (g0e < 0)
		g0e = 0;
	else if (g0e > 255)
		g0e = 255;

	if (b0e < 0)
		b0e = 0;
	else if (b0e > 255)
		b0e = 255;

	if (r1e < 0)
		r1e = 0;
	else if (r1e > 255)
		r1e = 255;

	if (g1e < 0)
		g1e = 0;
	else if (g1e > 255)
		g1e = 255;

	if (b1e < 0)
		b1e = 0;
	else if (b1e > 255)
		b1e = 255;

	output0->x = r0e;
	output0->y = g0e;
	output0->z = b0e;
	output0->w = 0xFF;

	output1->x = r1e;
	output1->y = g1e;
	output1->z = b1e;
	output1->w = 0xFF;

	return retval;
}


int rgb_unpack(const int input[6], int quantization_level, ushort4 * output0, ushort4 * output1)
{

	int ri0b = color_unquantization_tables[quantization_level][input[0]];
	int ri1b = color_unquantization_tables[quantization_level][input[1]];
	int gi0b = color_unquantization_tables[quantization_level][input[2]];
	int gi1b = color_unquantization_tables[quantization_level][input[3]];
	int bi0b = color_unquantization_tables[quantization_level][input[4]];
	int bi1b = color_unquantization_tables[quantization_level][input[5]];

	if (ri0b + gi0b + bi0b > ri1b + gi1b + bi1b)
	{
		// blue-contraction
		ri0b = (ri0b + bi0b) >> 1;
		gi0b = (gi0b + bi0b) >> 1;
		ri1b = (ri1b + bi1b) >> 1;
		gi1b = (gi1b + bi1b) >> 1;

		output0->x = ri1b;
		output0->y = gi1b;
		output0->z = bi1b;
		output0->w = 255;

		output1->x = ri0b;
		output1->y = gi0b;
		output1->z = bi0b;
		output1->w = 255;
		return 1;
	}
	else
	{
		output0->x = ri0b;
		output0->y = gi0b;
		output0->z = bi0b;
		output0->w = 255;

		output1->x = ri1b;
		output1->y = gi1b;
		output1->z = bi1b;
		output1->w = 255;
		return 0;
	}
}




void rgba_unpack(const int input[8], int quantization_level, ushort4 * output0, ushort4 * output1)
{
	int order = rgb_unpack(input, quantization_level, output0, output1);
	if (order == 0)
	{
		output0->w = color_unquantization_tables[quantization_level][input[6]];
		output1->w = color_unquantization_tables[quantization_level][input[7]];
	}
	else
	{
		output0->w = color_unquantization_tables[quantization_level][input[7]];
		output1->w = color_unquantization_tables[quantization_level][input[6]];
	}
}



void rgba_delta_unpack(const int input[8], int quantization_level, ushort4 * output0, ushort4 * output1)
{
	int a0 = color_unquantization_tables[quantization_level][input[6]];
	int a1 = color_unquantization_tables[quantization_level][input[7]];
	a0 |= (a1 & 0x80) << 1;
	a1 &= 0x7F;
	if (a1 & 0x40)
		a1 -= 0x80;
	a0 >>= 1;
	a1 >>= 1;
	a1 += a0;

	if (a1 < 0)
		a1 = 0;
	else if (a1 > 255)
		a1 = 255;

	int order = rgb_delta_unpack(input, quantization_level, output0, output1);
	if (order == 0)
	{
		output0->w = a0;
		output1->w = a1;
	}
	else
	{
		output0->w = a1;
		output1->w = a0;
	}
}


void rgb_scale_unpack(const int input[4], int quantization_level, ushort4 * output0, ushort4 * output1)
{
	int ir = color_unquantization_tables[quantization_level][input[0]];
	int ig = color_unquantization_tables[quantization_level][input[1]];
	int ib = color_unquantization_tables[quantization_level][input[2]];

	int iscale = color_unquantization_tables[quantization_level][input[3]];

	*output1 = ushort4(ir, ig, ib, 255);
	*output0 = ushort4((ir * iscale) >> 8, (ig * iscale) >> 8, (ib * iscale) >> 8, 255);
}



void rgb_scale_alpha_unpack(const int input[6], int quantization_level, ushort4 * output0, ushort4 * output1)
{
	rgb_scale_unpack(input, quantization_level, output0, output1);
	output0->w = color_unquantization_tables[quantization_level][input[4]];
	output1->w = color_unquantization_tables[quantization_level][input[5]];

}


void luminance_unpack(const int input[2], int quantization_level, ushort4 * output0, ushort4 * output1)
{
	int lum0 = color_unquantization_tables[quantization_level][input[0]];
	int lum1 = color_unquantization_tables[quantization_level][input[1]];
	*output0 = ushort4(lum0, lum0, lum0, 255);
	*output1 = ushort4(lum1, lum1, lum1, 255);
}


void luminance_delta_unpack(const int input[2], int quantization_level, ushort4 * output0, ushort4 * output1)
{
	int v0 = color_unquantization_tables[quantization_level][input[0]];
	int v1 = color_unquantization_tables[quantization_level][input[1]];
	int l0 = (v0 >> 2) | (v1 & 0xC0);
	int l1 = l0 + (v1 & 0x3F);

	if (l1 > 255)
		l1 = 255;

	*output0 = ushort4(l0, l0, l0, 255);
	*output1 = ushort4(l1, l1, l1, 255);
}




void luminance_alpha_unpack(const int input[4], int quantization_level, ushort4 * output0, ushort4 * output1)
{
	int lum0 = color_unquantization_tables[quantization_level][input[0]];
	int lum1 = color_unquantization_tables[quantization_level][input[1]];
	int alpha0 = color_unquantization_tables[quantization_level][input[2]];
	int alpha1 = color_unquantization_tables[quantization_level][input[3]];
	*output0 = ushort4(lum0, lum0, lum0, alpha0);
	*output1 = ushort4(lum1, lum1, lum1, alpha1);
}


void luminance_alpha_delta_unpack(const int input[4], int quantization_level, ushort4 * output0, ushort4 * output1)
{
	int lum0 = color_unquantization_tables[quantization_level][input[0]];
	int lum1 = color_unquantization_tables[quantization_level][input[1]];
	int alpha0 = color_unquantization_tables[quantization_level][input[2]];
	int alpha1 = color_unquantization_tables[quantization_level][input[3]];

	lum0 |= (lum1 & 0x80) << 1;
	alpha0 |= (alpha1 & 0x80) << 1;
	lum1 &= 0x7F;
	alpha1 &= 0x7F;
	if (lum1 & 0x40)
		lum1 -= 0x80;
	if (alpha1 & 0x40)
		alpha1 -= 0x80;

	lum0 >>= 1;
	lum1 >>= 1;
	alpha0 >>= 1;
	alpha1 >>= 1;
	lum1 += lum0;
	alpha1 += alpha0;

	if (lum1 < 0)
		lum1 = 0;
	else if (lum1 > 255)
		lum1 = 255;

	if (alpha1 < 0)
		alpha1 = 0;
	else if (alpha1 > 255)
		alpha1 = 255;

	*output0 = ushort4(lum0, lum0, lum0, alpha0);
	*output1 = ushort4(lum1, lum1, lum1, alpha1);
}




// RGB-offset format
void hdr_rgbo_unpack3(const int input[4], int quantization_level, ushort4 * output0, ushort4 * output1)
{
	int v0 = color_unquantization_tables[quantization_level][input[0]];
	int v1 = color_unquantization_tables[quantization_level][input[1]];
	int v2 = color_unquantization_tables[quantization_level][input[2]];
	int v3 = color_unquantization_tables[quantization_level][input[3]];

	int modeval = ((v0 & 0xC0) >> 6) | (((v1 & 0x80) >> 7) << 2) | (((v2 & 0x80) >> 7) << 3);

	int majcomp;
	int mode;
	if ((modeval & 0xC) != 0xC)
	{
		majcomp = modeval >> 2;
		mode = modeval & 3;
	}
	else if (modeval != 0xF)
	{
		majcomp = modeval & 3;
		mode = 4;
	}
	else
	{
		majcomp = 0;
		mode = 5;
	}

	int red = v0 & 0x3F;
	int green = v1 & 0x1F;
	int blue = v2 & 0x1F;
	int scale = v3 & 0x1F;

	int bit0 = (v1 >> 6) & 1;
	int bit1 = (v1 >> 5) & 1;
	int bit2 = (v2 >> 6) & 1;
	int bit3 = (v2 >> 5) & 1;
	int bit4 = (v3 >> 7) & 1;
	int bit5 = (v3 >> 6) & 1;
	int bit6 = (v3 >> 5) & 1;

	int ohcomp = 1 << mode;

	if (ohcomp & 0x30)
		green |= bit0 << 6;
	if (ohcomp & 0x3A)
		green |= bit1 << 5;
	if (ohcomp & 0x30)
		blue |= bit2 << 6;
	if (ohcomp & 0x3A)
		blue |= bit3 << 5;

	if (ohcomp & 0x3D)
		scale |= bit6 << 5;
	if (ohcomp & 0x2D)
		scale |= bit5 << 6;
	if (ohcomp & 0x04)
		scale |= bit4 << 7;

	if (ohcomp & 0x3B)
		red |= bit4 << 6;
	if (ohcomp & 0x04)
		red |= bit3 << 6;

	if (ohcomp & 0x10)
		red |= bit5 << 7;
	if (ohcomp & 0x0F)
		red |= bit2 << 7;

	if (ohcomp & 0x05)
		red |= bit1 << 8;
	if (ohcomp & 0x0A)
		red |= bit0 << 8;

	if (ohcomp & 0x05)
		red |= bit0 << 9;
	if (ohcomp & 0x02)
		red |= bit6 << 9;

	if (ohcomp & 0x01)
		red |= bit3 << 10;
	if (ohcomp & 0x02)
		red |= bit5 << 10;


	// expand to 12 bits.
	static const int shamts[6] = { 1, 1, 2, 3, 4, 5 };
	int shamt = shamts[mode];
	red <<= shamt;
	green <<= shamt;
	blue <<= shamt;
	scale <<= shamt;

	// on modes 0 to 4, the values stored for "green" and "blue" are differentials,
	// not absolute values.
	if (mode != 5)
	{
		green = red - green;
		blue = red - blue;
	}

	// switch around components.
	int temp;
	switch (majcomp)
	{
	case 1:
		temp = red;
		red = green;
		green = temp;
		break;
	case 2:
		temp = red;
		red = blue;
		blue = temp;
		break;
	default:
		break;
	}


	int red0 = red - scale;
	int green0 = green - scale;
	int blue0 = blue - scale;

	// clamp to [0,0xFFF].
	if (red < 0)
		red = 0;
	if (green < 0)
		green = 0;
	if (blue < 0)
		blue = 0;

	if (red0 < 0)
		red0 = 0;
	if (green0 < 0)
		green0 = 0;
	if (blue0 < 0)
		blue0 = 0;

	*output0 = ushort4(red0 << 4, green0 << 4, blue0 << 4, 0x7800);
	*output1 = ushort4(red << 4, green << 4, blue << 4, 0x7800);
}



void hdr_rgb_unpack3(const int input[6], int quantization_level, ushort4 * output0, ushort4 * output1)
{

	int v0 = color_unquantization_tables[quantization_level][input[0]];
	int v1 = color_unquantization_tables[quantization_level][input[1]];
	int v2 = color_unquantization_tables[quantization_level][input[2]];
	int v3 = color_unquantization_tables[quantization_level][input[3]];
	int v4 = color_unquantization_tables[quantization_level][input[4]];
	int v5 = color_unquantization_tables[quantization_level][input[5]];

	// extract all the fixed-placement bitfields
	int modeval = ((v1 & 0x80) >> 7) | (((v2 & 0x80) >> 7) << 1) | (((v3 & 0x80) >> 7) << 2);

	int majcomp = ((v4 & 0x80) >> 7) | (((v5 & 0x80) >> 7) << 1);

	if (majcomp == 3)
	{
		*output0 = ushort4(v0 << 8, v2 << 8, (v4 & 0x7F) << 9, 0x7800);
		*output1 = ushort4(v1 << 8, v3 << 8, (v5 & 0x7F) << 9, 0x7800);
		return;
	}

	int a = v0 | ((v1 & 0x40) << 2);
	int b0 = v2 & 0x3f;
	int b1 = v3 & 0x3f;
	int c = v1 & 0x3f;
	int d0 = v4 & 0x7f;
	int d1 = v5 & 0x7f;

	// get hold of the number of bits in 'd0' and 'd1'
	static const int dbits_tab[8] = { 7, 6, 7, 6, 5, 6, 5, 6 };
	int dbits = dbits_tab[modeval];

	// extract six variable-placement bits
	int bit0 = (v2 >> 6) & 1;
	int bit1 = (v3 >> 6) & 1;

	int bit2 = (v4 >> 6) & 1;
	int bit3 = (v5 >> 6) & 1;
	int bit4 = (v4 >> 5) & 1;
	int bit5 = (v5 >> 5) & 1;


	// and prepend the variable-placement bits depending on mode.
	int ohmod = 1 << modeval;	// one-hot-mode
	if (ohmod & 0xA4)
		a |= bit0 << 9;
	if (ohmod & 0x8)
		a |= bit2 << 9;
	if (ohmod & 0x50)
		a |= bit4 << 9;

	if (ohmod & 0x50)
		a |= bit5 << 10;
	if (ohmod & 0xA0)
		a |= bit1 << 10;

	if (ohmod & 0xC0)
		a |= bit2 << 11;

	if (ohmod & 0x4)
		c |= bit1 << 6;
	if (ohmod & 0xE8)
		c |= bit3 << 6;

	if (ohmod & 0x20)
		c |= bit2 << 7;


	if (ohmod & 0x5B)
		b0 |= bit0 << 6;
	if (ohmod & 0x5B)
		b1 |= bit1 << 6;

	if (ohmod & 0x12)
		b0 |= bit2 << 7;
	if (ohmod & 0x12)
		b1 |= bit3 << 7;

	if (ohmod & 0xAF)
		d0 |= bit4 << 5;
	if (ohmod & 0xAF)
		d1 |= bit5 << 5;
	if (ohmod & 0x5)
		d0 |= bit2 << 6;
	if (ohmod & 0x5)
		d1 |= bit3 << 6;

	// sign-extend 'd0' and 'd1'
	// note: this code assumes that signed right-shift actually sign-fills, not zero-fills.
	int32_t d0x = d0;
	int32_t d1x = d1;
	int sx_shamt = 32 - dbits;
	d0x <<= sx_shamt;
	d0x >>= sx_shamt;
	d1x <<= sx_shamt;
	d1x >>= sx_shamt;
	d0 = d0x;
	d1 = d1x;

	// expand all values to 12 bits, with left-shift as needed.
	int val_shamt = (modeval >> 1) ^ 3;
	a <<= val_shamt;
	b0 <<= val_shamt;
	b1 <<= val_shamt;
	c <<= val_shamt;
	d0 <<= val_shamt;
	d1 <<= val_shamt;

	// then compute the actual color values.
	int red1 = a;
	int green1 = a - b0;
	int blue1 = a - b1;
	int red0 = a - c;
	int green0 = a - b0 - c - d0;
	int blue0 = a - b1 - c - d1;

	// clamp the color components to [0,2^12 - 1]
	if (red0 < 0)
		red0 = 0;
	else if (red0 > 0xFFF)
		red0 = 0xFFF;

	if (green0 < 0)
		green0 = 0;
	else if (green0 > 0xFFF)
		green0 = 0xFFF;

	if (blue0 < 0)
		blue0 = 0;
	else if (blue0 > 0xFFF)
		blue0 = 0xFFF;

	if (red1 < 0)
		red1 = 0;
	else if (red1 > 0xFFF)
		red1 = 0xFFF;

	if (green1 < 0)
		green1 = 0;
	else if (green1 > 0xFFF)
		green1 = 0xFFF;

	if (blue1 < 0)
		blue1 = 0;
	else if (blue1 > 0xFFF)
		blue1 = 0xFFF;


	// switch around the color components
	int temp0, temp1;
	switch (majcomp)
	{
	case 1:					// switch around red and green
		temp0 = red0;
		temp1 = red1;
		red0 = green0;
		red1 = green1;
		green0 = temp0;
		green1 = temp1;
		break;
	case 2:					// switch around red and blue
		temp0 = red0;
		temp1 = red1;
		red0 = blue0;
		red1 = blue1;
		blue0 = temp0;
		blue1 = temp1;
		break;
	case 0:					// no switch
		break;
	}

	*output0 = ushort4(red0 << 4, green0 << 4, blue0 << 4, 0x7800);
	*output1 = ushort4(red1 << 4, green1 << 4, blue1 << 4, 0x7800);
}




void hdr_rgb_ldr_alpha_unpack3(const int input[8], int quantization_level, ushort4 * output0, ushort4 * output1)
{
	hdr_rgb_unpack3(input, quantization_level, output0, output1);

	int v6 = color_unquantization_tables[quantization_level][input[6]];
	int v7 = color_unquantization_tables[quantization_level][input[7]];
	output0->w = v6;
	output1->w = v7;
}



void hdr_luminance_small_range_unpack(const int input[2], int quantization_level, ushort4 * output0, ushort4 * output1)
{
	int v0 = color_unquantization_tables[quantization_level][input[0]];
	int v1 = color_unquantization_tables[quantization_level][input[1]];

	int y0, y1;
	if (v0 & 0x80)
	{
		y0 = ((v1 & 0xE0) << 4) | ((v0 & 0x7F) << 2);
		y1 = (v1 & 0x1F) << 2;
	}
	else
	{
		y0 = ((v1 & 0xF0) << 4) | ((v0 & 0x7F) << 1);
		y1 = (v1 & 0xF) << 1;
	}

	y1 += y0;
	if (y1 > 0xFFF)
		y1 = 0xFFF;

	*output0 = ushort4(y0 << 4, y0 << 4, y0 << 4, 0x7800);
	*output1 = ushort4(y1 << 4, y1 << 4, y1 << 4, 0x7800);
}


void hdr_luminance_large_range_unpack(const int input[2], int quantization_level, ushort4 * output0, ushort4 * output1)
{
	int v0 = color_unquantization_tables[quantization_level][input[0]];
	int v1 = color_unquantization_tables[quantization_level][input[1]];

	int y0, y1;
	if (v1 >= v0)
	{
		y0 = v0 << 4;
		y1 = v1 << 4;
	}
	else
	{
		y0 = (v1 << 4) + 8;
		y1 = (v0 << 4) - 8;
	}
	*output0 = ushort4(y0 << 4, y0 << 4, y0 << 4, 0x7800);
	*output1 = ushort4(y1 << 4, y1 << 4, y1 << 4, 0x7800);
}



void hdr_alpha_unpack(const int input[2], int quantization_level, int *a0, int *a1)
{

	int v6 = color_unquantization_tables[quantization_level][input[0]];
	int v7 = color_unquantization_tables[quantization_level][input[1]];

	int selector = ((v6 >> 7) & 1) | ((v7 >> 6) & 2);
	v6 &= 0x7F;
	v7 &= 0x7F;
	if (selector == 3)
	{
		*a0 = v6 << 5;
		*a1 = v7 << 5;
	}
	else
	{
		v6 |= (v7 << (selector + 1)) & 0x780;
		v7 &= (0x3f >> selector);
		v7 ^= 32 >> selector;
		v7 -= 32 >> selector;
		v6 <<= (4 - selector);
		v7 <<= (4 - selector);
		v7 += v6;

		if (v7 < 0)
			v7 = 0;
		else if (v7 > 0xFFF)
			v7 = 0xFFF;

		*a0 = v6;
		*a1 = v7;
	}

	*a0 <<= 4;
	*a1 <<= 4;
}



void hdr_rgb_hdr_alpha_unpack3(const int input[8], int quantization_level, ushort4 * output0, ushort4 * output1)
{
	hdr_rgb_unpack3(input, quantization_level, output0, output1);

	int alpha0, alpha1;
	hdr_alpha_unpack(input + 6, quantization_level, &alpha0, &alpha1);

	output0->w = alpha0;
	output1->w = alpha1;
}






void unpack_color_endpoints(astc_decode_mode decode_mode, int format, int quantization_level, const int *input, int *rgb_hdr, int *alpha_hdr, int *nan_endpoint, ushort4 * output0, ushort4 * output1)
{
	*nan_endpoint = 0;

	switch (format)
	{
	case FMT_LUMINANCE:
		*rgb_hdr = 0;
		*alpha_hdr = 0;
		luminance_unpack(input, quantization_level, output0, output1);
		break;

	case FMT_LUMINANCE_DELTA:
		*rgb_hdr = 0;
		*alpha_hdr = 0;
		luminance_delta_unpack(input, quantization_level, output0, output1);
		break;

	case FMT_HDR_LUMINANCE_SMALL_RANGE:
		*rgb_hdr = 1;
		*alpha_hdr = -1;
		hdr_luminance_small_range_unpack(input, quantization_level, output0, output1);
		break;

	case FMT_HDR_LUMINANCE_LARGE_RANGE:
		*rgb_hdr = 1;
		*alpha_hdr = -1;
		hdr_luminance_large_range_unpack(input, quantization_level, output0, output1);
		break;

	case FMT_LUMINANCE_ALPHA:
		*rgb_hdr = 0;
		*alpha_hdr = 0;
		luminance_alpha_unpack(input, quantization_level, output0, output1);
		break;

	case FMT_LUMINANCE_ALPHA_DELTA:
		*rgb_hdr = 0;
		*alpha_hdr = 0;
		luminance_alpha_delta_unpack(input, quantization_level, output0, output1);
		break;

	case FMT_RGB_SCALE:
		*rgb_hdr = 0;
		*alpha_hdr = 0;
		rgb_scale_unpack(input, quantization_level, output0, output1);
		break;

	case FMT_RGB_SCALE_ALPHA:
		*rgb_hdr = 0;
		*alpha_hdr = 0;
		rgb_scale_alpha_unpack(input, quantization_level, output0, output1);
		break;

	case FMT_HDR_RGB_SCALE:
		*rgb_hdr = 1;
		*alpha_hdr = -1;
		hdr_rgbo_unpack3(input, quantization_level, output0, output1);
		break;

	case FMT_RGB:
		*rgb_hdr = 0;
		*alpha_hdr = 0;
		rgb_unpack(input, quantization_level, output0, output1);
		break;

	case FMT_RGB_DELTA:
		*rgb_hdr = 0;
		*alpha_hdr = 0;
		rgb_delta_unpack(input, quantization_level, output0, output1);
		break;

	case FMT_HDR_RGB:
		*rgb_hdr = 1;
		*alpha_hdr = -1;
		hdr_rgb_unpack3(input, quantization_level, output0, output1);
		break;

	case FMT_RGBA:
		*rgb_hdr = 0;
		*alpha_hdr = 0;
		rgba_unpack(input, quantization_level, output0, output1);
		break;

	case FMT_RGBA_DELTA:
		*rgb_hdr = 0;
		*alpha_hdr = 0;
		rgba_delta_unpack(input, quantization_level, output0, output1);
		break;

	case FMT_HDR_RGB_LDR_ALPHA:
		*rgb_hdr = 1;
		*alpha_hdr = 0;
		hdr_rgb_ldr_alpha_unpack3(input, quantization_level, output0, output1);
		break;

	case FMT_HDR_RGBA:
		*rgb_hdr = 1;
		*alpha_hdr = 1;
		hdr_rgb_hdr_alpha_unpack3(input, quantization_level, output0, output1);
		break;

	default:
		ASTC_CODEC_INTERNAL_ERROR;
	}



	if (*alpha_hdr == -1)
	{
		if (alpha_force_use_of_hdr)
		{
			output0->w = 0x7800;
			output1->w = 0x7800;
			*alpha_hdr = 1;
		}
		else
		{
			output0->w = 0x00FF;
			output1->w = 0x00FF;
			*alpha_hdr = 0;
		}
	}



	switch (decode_mode)
	{
	case DECODE_LDR_SRGB:
		if (*rgb_hdr == 1)
		{
			output0->x = 0xFF00;
			output0->y = 0x0000;
			output0->z = 0xFF00;
			output0->w = 0xFF00;
			output1->x = 0xFF00;
			output1->y = 0x0000;
			output1->z = 0xFF00;
			output1->w = 0xFF00;
		}
		else
		{
			output0->x *= 257;
			output0->y *= 257;
			output0->z *= 257;
			output0->w *= 257;
			output1->x *= 257;
			output1->y *= 257;
			output1->z *= 257;
			output1->w *= 257;
		}
		*rgb_hdr = 0;
		*alpha_hdr = 0;
		break;

	case DECODE_LDR:
		if (*rgb_hdr == 1)
		{
			output0->x = 0xFFFF;
			output0->y = 0xFFFF;
			output0->z = 0xFFFF;
			output0->w = 0xFFFF;
			output1->x = 0xFFFF;
			output1->y = 0xFFFF;
			output1->z = 0xFFFF;
			output1->w = 0xFFFF;
			*nan_endpoint = 1;
		}
		else
		{
			output0->x *= 257;
			output0->y *= 257;
			output0->z *= 257;
			output0->w *= 257;
			output1->x *= 257;
			output1->y *= 257;
			output1->z *= 257;
			output1->w *= 257;
		}
		*rgb_hdr = 0;
		*alpha_hdr = 0;
		break;

	case DECODE_HDR:

		if (*rgb_hdr == 0)
		{
			output0->x *= 257;
			output0->y *= 257;
			output0->z *= 257;
			output1->x *= 257;
			output1->y *= 257;
			output1->z *= 257;
		}
		if (*alpha_hdr == 0)
		{
			output0->w *= 257;
			output1->w *= 257;
		}
		break;
	}
}
