// SPDX-License-Identifier: Apache-2.0
// ----------------------------------------------------------------------------
// Copyright 2011-2020 Arm Limited
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy
// of the License at:
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations
// under the License.
// ----------------------------------------------------------------------------

/**
 * @brief Functions for loading/storing ASTC compressed images.
 */


#include "astc_codec_internals.h"

// conversion functions between the LNS representation and the FP16 representation.
float float_to_lns(float p)
{
	if (astc::isnan(p) || p <= 1.0f / 67108864.0f)
	{
		// underflow or NaN value, return 0.
		// We count underflow if the input value is smaller than 2^-26.
		return 0;
	}

	if (fabsf(p) >= 65536.0f)
	{
		// overflow, return a +INF value
		return 65535;
	}

	int expo;
	float normfrac = frexpf(p, &expo);
	float p1;
	if (expo < -13)
	{
		// input number is smaller than 2^-14. In this case, multiply by 2^25.
		p1 = p * 33554432.0f;
		expo = 0;
	}
	else
	{
		expo += 14;
		p1 = (normfrac - 0.5f) * 4096.0f;
	}

	if (p1 < 384.0f)
		p1 *= 4.0f / 3.0f;
	else if (p1 <= 1408.0f)
		p1 += 128.0f;
	else
		p1 = (p1 + 512.0f) * (4.0f / 5.0f);

	p1 += expo * 2048.0f;
	return p1 + 1.0f;
}

uint16_t lns_to_sf16(uint16_t p)
{
	uint16_t mc = p & 0x7FF;
	uint16_t ec = p >> 11;
	uint16_t mt;
	if (mc < 512)
		mt = 3 * mc;
	else if (mc < 1536)
		mt = 4 * mc - 512;
	else
		mt = 5 * mc - 2048;

	uint16_t res = (ec << 10) | (mt >> 3);
	if (res >= 0x7BFF)
		res = 0x7BFF;
	return res;
}

// conversion function from 16-bit LDR value to FP16.
// note: for LDR interpolation, it is impossible to get a denormal result;
// this simplifies the conversion.
// FALSE; we can receive a very small UNORM16 through the constant-block.
uint16_t unorm16_to_sf16(uint16_t p)
{
	if (p == 0xFFFF)
		return 0x3C00;			// value of 1.0 .
	if (p < 4)
		return p << 8;

	int lz = clz32(p) - 16;
	p <<= (lz + 1);
	p >>= 6;
	p |= (14 - lz) << 10;
	return p;
}

// helper function to initialize the work-data from the orig-data
void imageblock_initialize_work_from_orig(
	imageblock* pb,
	int pixelcount
) {
	float *fptr = pb->orig_data;

	for (int i = 0; i < pixelcount; i++)
	{
		if (pb->rgb_lns[i])
		{
			pb->data_r[i] = float_to_lns(fptr[0]);
			pb->data_g[i] = float_to_lns(fptr[1]);
			pb->data_b[i] = float_to_lns(fptr[2]);
		}
		else
		{
			pb->data_r[i] = fptr[0] * 65535.0f;
			pb->data_g[i] = fptr[1] * 65535.0f;
			pb->data_b[i] = fptr[2] * 65535.0f;
		}

		if (pb->alpha_lns[i])
		{
			pb->data_a[i] = float_to_lns(fptr[3]);
		}
		else
		{
			pb->data_a[i] = fptr[3] * 65535.0f;
		}

		fptr += 4;
	}
}

// helper function to initialize the orig-data from the work-data
void imageblock_initialize_orig_from_work(
	imageblock* pb,
	int pixelcount
) {
	float *fptr = pb->orig_data;

	for (int i = 0; i < pixelcount; i++)
	{
		if (pb->rgb_lns[i])
		{
			fptr[0] = sf16_to_float(lns_to_sf16((uint16_t)pb->data_r[i]));
			fptr[1] = sf16_to_float(lns_to_sf16((uint16_t)pb->data_g[i]));
			fptr[2] = sf16_to_float(lns_to_sf16((uint16_t)pb->data_b[i]));
		}
		else
		{
			fptr[0] = sf16_to_float(unorm16_to_sf16((uint16_t)pb->data_r[i]));
			fptr[1] = sf16_to_float(unorm16_to_sf16((uint16_t)pb->data_g[i]));
			fptr[2] = sf16_to_float(unorm16_to_sf16((uint16_t)pb->data_b[i]));
		}

		if (pb->alpha_lns[i])
		{
			fptr[3] = sf16_to_float(lns_to_sf16((uint16_t)pb->data_a[i]));
		}
		else
		{
			fptr[3] = sf16_to_float(unorm16_to_sf16((uint16_t)pb->data_a[i]));
		}

		fptr += 4;
	}
}

/*
   For an imageblock, update its flags.
   The updating is done based on data, not orig_data.
*/
void update_imageblock_flags(
	imageblock* pb,
	int xdim,
	int ydim,
	int zdim
) {
	int i;
	float red_min = 1e38f, red_max = -1e38f;
	float green_min = 1e38f, green_max = -1e38f;
	float blue_min = 1e38f, blue_max = -1e38f;
	float alpha_min = 1e38f, alpha_max = -1e38f;

	int texels_per_block = xdim * ydim * zdim;

	int grayscale = 1;

	for (i = 0; i < texels_per_block; i++)
	{
		float red = pb->data_r[i];
		float green = pb->data_g[i];
		float blue = pb->data_b[i];
		float alpha = pb->data_a[i];
		if (red < red_min)
			red_min = red;
		if (red > red_max)
			red_max = red;
		if (green < green_min)
			green_min = green;
		if (green > green_max)
			green_max = green;
		if (blue < blue_min)
			blue_min = blue;
		if (blue > blue_max)
			blue_max = blue;
		if (alpha < alpha_min)
			alpha_min = alpha;
		if (alpha > alpha_max)
			alpha_max = alpha;

		if (grayscale == 1 && (red != green || red != blue))
			grayscale = 0;
	}

	pb->red_min = red_min;
	pb->red_max = red_max;
	pb->green_min = green_min;
	pb->green_max = green_max;
	pb->blue_min = blue_min;
	pb->blue_max = blue_max;
	pb->alpha_min = alpha_min;
	pb->alpha_max = alpha_max;
	pb->grayscale = grayscale;
}

