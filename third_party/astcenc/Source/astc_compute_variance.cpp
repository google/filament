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
 *	@brief	ASTC functions to calculate, for each pixel and each color component,
 *			its variance within an NxN footprint; we want N to be parametric.
 *
 *			The routine below uses summed area tables in order to perform the
 *			computation in O(1) time per pixel, independent of big N is.
 */
/*----------------------------------------------------------------------------*/

#include "astc_codec_internals.h"

#include <math.h>
#include "mathlib.h"
#include "softfloat.h"

float4 *** input_averages;
float  *** input_alpha_averages;
float4 *** input_variances;

#include <stdio.h>

// routine to compute averages and variances for a pixel region.
// The routine computes both in a single pass, using a summed-area table
// to decouple the running time from the averaging/variance kernel size.

static void compute_pixel_region_variance(const astc_codec_image * img, float rgb_power_to_use, float alpha_power_to_use, swizzlepattern swz, int use_z_axis,
										  int source_xoffset,int source_yoffset, int source_zoffset, // position of upper-left pixel in data set
										  int xsize, int ysize, int zsize, 	// the size of the region to actually compute averages and variances for.
										  int avg_var_kernel_radius, int alpha_kernel_radius,
										  int dest_xoffset, int dest_yoffset, int dest_zoffset)
{
	int x, y, z;

	int kernel_radius = MAX(avg_var_kernel_radius, alpha_kernel_radius);
	int kerneldim = 2 * kernel_radius + 1;

	// allocate memory
	int xpadsize = xsize + kerneldim;
	int ypadsize = ysize + kerneldim;
	int zpadsize = zsize + (use_z_axis ? kerneldim : 1);

	double4 ***varbuf1 = new double4 **[zpadsize];
	double4 ***varbuf2 = new double4 **[zpadsize];
	varbuf1[0] = new double4 *[ypadsize * zpadsize];
	varbuf2[0] = new double4 *[ypadsize * zpadsize];
	varbuf1[0][0] = new double4[xpadsize * ypadsize * zpadsize];
	varbuf2[0][0] = new double4[xpadsize * ypadsize * zpadsize];


	for (z = 1; z < zpadsize; z++)
	{
		varbuf1[z] = varbuf1[0] + ypadsize * z;
		varbuf2[z] = varbuf2[0] + ypadsize * z;
		varbuf1[z][0] = varbuf1[0][0] + xpadsize * ypadsize * z;
		varbuf2[z][0] = varbuf2[0][0] + xpadsize * ypadsize * z;
	}

	for (z = 0; z < zpadsize; z++)
		for (y = 1; y < ypadsize; y++)
		{
			varbuf1[z][y] = varbuf1[z][0] + xpadsize * y;
			varbuf2[z][y] = varbuf2[z][0] + xpadsize * y;
		}

	int powers_are_1 = (rgb_power_to_use == 1.0f) && (alpha_power_to_use == 1.0f);


	// load x and x^2 values into the allocated buffers
	if (img->imagedata8)
	{
		uint8_t data[6];
		data[4] = 0;
		data[5] = 255;

		for (z = 0; z < zpadsize - 1; z++)
		{
			int z_src = z + source_zoffset - (use_z_axis ? kernel_radius : 0);
			for (y = 0; y < ypadsize - 1; y++)
			{
				int y_src = y + source_yoffset - kernel_radius;
				for (x = 0; x < xpadsize - 1; x++)
				{
					int x_src = x + source_xoffset - kernel_radius;
					data[0] = img->imagedata8[z_src][y_src][4 * x_src + 0];
					data[1] = img->imagedata8[z_src][y_src][4 * x_src + 1];
					data[2] = img->imagedata8[z_src][y_src][4 * x_src + 2];
					data[3] = img->imagedata8[z_src][y_src][4 * x_src + 3];

					uint8_t r = data[swz.r];
					uint8_t g = data[swz.g];
					uint8_t b = data[swz.b];
					uint8_t a = data[swz.a];

					double4 d = double4(r * (1.0 / 255.0),
										g * (1.0 / 255.0),
										b * (1.0 / 255.0),
										a * (1.0 / 255.0));

					if (perform_srgb_transform)
					{
						d.x = (d.x <= 0.04045) ? d.x * (1.0 / 12.92) : (d.x <= 1) ? pow((d.x + 0.055) * (1.0 / 1.055), 2.4) : d.x;
						d.y = (d.y <= 0.04045) ? d.y * (1.0 / 12.92) : (d.y <= 1) ? pow((d.y + 0.055) * (1.0 / 1.055), 2.4) : d.y;
						d.z = (d.z <= 0.04045) ? d.z * (1.0 / 12.92) : (d.z <= 1) ? pow((d.z + 0.055) * (1.0 / 1.055), 2.4) : d.z;
					}

					if (!powers_are_1)
					{
						d.x = pow(MAX(d.x, 1e-6), (double)rgb_power_to_use);
						d.y = pow(MAX(d.y, 1e-6), (double)rgb_power_to_use);
						d.z = pow(MAX(d.z, 1e-6), (double)rgb_power_to_use);
						d.w = pow(MAX(d.w, 1e-6), (double)alpha_power_to_use);
					}

					varbuf1[z][y][x] = d;
					varbuf2[z][y][x] = d * d;
				}
			}
		}
	}
	else
	{
		uint16_t data[6];
		data[4] = 0;
		data[5] = 0x3C00;		// 1.0 encoded as FP16.

		for (z = 0; z < zpadsize - 1; z++)
		{
			int z_src = z + source_zoffset - (use_z_axis ? kernel_radius : 0);
			for (y = 0; y < ypadsize - 1; y++)
			{
				int y_src = y + source_yoffset - kernel_radius;
				for (x = 0; x < xpadsize - 1; x++)
				{
					int x_src = x + source_xoffset - kernel_radius;
					data[0] = img->imagedata16[z_src][y_src][4 * x_src];
					data[1] = img->imagedata16[z_src][y_src][4 * x_src + 1];
					data[2] = img->imagedata16[z_src][y_src][4 * x_src + 2];
					data[3] = img->imagedata16[z_src][y_src][4 * x_src + 3];

					uint16_t r = data[swz.r];
					uint16_t g = data[swz.g];
					uint16_t b = data[swz.b];
					uint16_t a = data[swz.a];

					double4 d = double4(sf16_to_float(r),
										sf16_to_float(g),
										sf16_to_float(b),
										sf16_to_float(a));

					if (perform_srgb_transform)
					{
						d.x = (d.x <= 0.04045) ? d.x * (1.0 / 12.92) : (d.x <= 1) ? pow((d.x + 0.055) * (1.0 / 1.055), 2.4) : d.x;
						d.y = (d.y <= 0.04045) ? d.y * (1.0 / 12.92) : (d.y <= 1) ? pow((d.y + 0.055) * (1.0 / 1.055), 2.4) : d.y;
						d.z = (d.z <= 0.04045) ? d.z * (1.0 / 12.92) : (d.z <= 1) ? pow((d.z + 0.055) * (1.0 / 1.055), 2.4) : d.z;
					}

					if (!powers_are_1)
					{
						d.x = pow(MAX(d.x, 1e-6), (double)rgb_power_to_use);
						d.y = pow(MAX(d.y, 1e-6), (double)rgb_power_to_use);
						d.z = pow(MAX(d.z, 1e-6), (double)rgb_power_to_use);
						d.w = pow(MAX(d.w, 1e-6), (double)alpha_power_to_use);
					}

					varbuf1[z][y][x] = d;
					varbuf2[z][y][x] = d * d;
				}
			}
		}
	}



	// pad out buffers with 0s
	for (z = 0; z < zpadsize; z++)
	{
		for (y = 0; y < ypadsize; y++)
		{
			varbuf1[z][y][xpadsize - 1] = double4(0.0, 0.0, 0.0, 0.0);
			varbuf2[z][y][xpadsize - 1] = double4(0.0, 0.0, 0.0, 0.0);
		}
		for (x = 0; x < xpadsize; x++)
		{
			varbuf1[z][ypadsize - 1][x] = double4(0.0, 0.0, 0.0, 0.0);
			varbuf2[z][ypadsize - 1][x] = double4(0.0, 0.0, 0.0, 0.0);
		}
	}

	if (use_z_axis)
		for (y = 0; y < ypadsize; y++)
			for (x = 0; x < xpadsize; x++)
			{
				varbuf1[zpadsize - 1][y][x] = double4(0.0, 0.0, 0.0, 0.0);
				varbuf2[zpadsize - 1][y][x] = double4(0.0, 0.0, 0.0, 0.0);
			}


	// generate summed-area tables for x and x2; this is done in-place
	for (z = 0; z < zpadsize; z++)
		for (y = 0; y < ypadsize; y++)
		{
			double4 summa1 = double4(0.0, 0.0, 0.0, 0.0);
			double4 summa2 = double4(0.0, 0.0, 0.0, 0.0);
			for (x = 0; x < xpadsize; x++)
			{
				double4 val1 = varbuf1[z][y][x];
				double4 val2 = varbuf2[z][y][x];
				varbuf1[z][y][x] = summa1;
				varbuf2[z][y][x] = summa2;
				summa1 = summa1 + val1;
				summa2 = summa2 + val2;
			}
		}

	for (z = 0; z < zpadsize; z++)
		for (x = 0; x < xpadsize; x++)
		{
			double4 summa1 = double4(0.0, 0.0, 0.0, 0.0);
			double4 summa2 = double4(0.0, 0.0, 0.0, 0.0);
			for (y = 0; y < ypadsize; y++)
			{
				double4 val1 = varbuf1[z][y][x];
				double4 val2 = varbuf2[z][y][x];
				varbuf1[z][y][x] = summa1;
				varbuf2[z][y][x] = summa2;
				summa1 = summa1 + val1;
				summa2 = summa2 + val2;
			}
		}

	if (use_z_axis)
		for (y = 0; y < ypadsize; y++)
			for (x = 0; x < xpadsize; x++)
			{
				double4 summa1 = double4(0.0, 0.0, 0.0, 0.0);
				double4 summa2 = double4(0.0, 0.0, 0.0, 0.0);
				for (z = 0; z < zpadsize; z++)
				{
					double4 val1 = varbuf1[z][y][x];
					double4 val2 = varbuf2[z][y][x];
					varbuf1[z][y][x] = summa1;
					varbuf2[z][y][x] = summa2;
					summa1 = summa1 + val1;
					summa2 = summa2 + val2;
				}
			}


	int avg_var_kerneldim = 2 * avg_var_kernel_radius + 1;
	int alpha_kerneldim = 2 * alpha_kernel_radius + 1;


	// compute a few constants used in the variance-calculation.
	double avg_var_samples;
	double alpha_rsamples;
	double mul1;

	if (use_z_axis)
	{
		avg_var_samples = avg_var_kerneldim * avg_var_kerneldim * avg_var_kerneldim;
		alpha_rsamples = 1.0 / (alpha_kerneldim * alpha_kerneldim * alpha_kerneldim);
	}
	else
	{
		avg_var_samples = avg_var_kerneldim * avg_var_kerneldim;
		alpha_rsamples = 1.0 / (alpha_kerneldim * alpha_kerneldim);
	}


	double avg_var_rsamples = 1.0 / avg_var_samples;
	if (avg_var_samples == 1)
		mul1 = 1.0;
	else
		mul1 = 1.0 / (avg_var_samples * (avg_var_samples - 1));


	double mul2 = avg_var_samples * mul1;


	// use the summed-area tables to compute variance for each sample-neighborhood
	if (use_z_axis)
	{
		for (z = 0; z < zsize; z++)
		{
			int z_src = z + kernel_radius;
			int z_dst = z + dest_zoffset;
			for (y = 0; y < ysize; y++)
			{
				int y_src = y + kernel_radius;
				int y_dst = y + dest_yoffset;

				for (x = 0; x < xsize; x++)
				{
					int x_src = x + kernel_radius;
					int x_dst = x + dest_xoffset;

					// summed-area table lookups for alpha average
					double vasum =
						(varbuf1[z_src + 1][y_src - alpha_kernel_radius][x_src - alpha_kernel_radius].w
						 - varbuf1[z_src + 1][y_src - alpha_kernel_radius][x_src + alpha_kernel_radius + 1].w
						 - varbuf1[z_src + 1][y_src + alpha_kernel_radius + 1][x_src - alpha_kernel_radius].w
						 + varbuf1[z_src + 1][y_src + alpha_kernel_radius + 1][x_src + alpha_kernel_radius + 1].w) -
						(varbuf1[z_src][y_src - alpha_kernel_radius][x_src - alpha_kernel_radius].w
						 - varbuf1[z_src][y_src - alpha_kernel_radius][x_src + alpha_kernel_radius + 1].w
						 - varbuf1[z_src][y_src + alpha_kernel_radius + 1][x_src - alpha_kernel_radius].w + varbuf1[z_src][y_src + alpha_kernel_radius + 1][x_src + alpha_kernel_radius + 1].w);
					input_alpha_averages[z_dst][y_dst][x_dst] = static_cast < float >(vasum * alpha_rsamples);


					// summed-area table lookups for RGBA average
					double4 v0sum =
						(varbuf1[z_src + 1][y_src - avg_var_kernel_radius][x_src - avg_var_kernel_radius]
						 - varbuf1[z_src + 1][y_src - avg_var_kernel_radius][x_src + avg_var_kernel_radius + 1]
						 - varbuf1[z_src + 1][y_src + avg_var_kernel_radius + 1][x_src - avg_var_kernel_radius]
						 + varbuf1[z_src + 1][y_src + avg_var_kernel_radius + 1][x_src + avg_var_kernel_radius + 1]) -
						(varbuf1[z_src][y_src - avg_var_kernel_radius][x_src - avg_var_kernel_radius]
						 - varbuf1[z_src][y_src - avg_var_kernel_radius][x_src + avg_var_kernel_radius + 1]
						 - varbuf1[z_src][y_src + avg_var_kernel_radius + 1][x_src - avg_var_kernel_radius] + varbuf1[z_src][y_src + avg_var_kernel_radius + 1][x_src + avg_var_kernel_radius + 1]);

					double4 avg = v0sum * avg_var_rsamples;

					float4 favg = float4(static_cast < float >(avg.x),
										 static_cast < float >(avg.y),
										 static_cast < float >(avg.z),
										 static_cast < float >(avg.w));
					input_averages[z_dst][y_dst][x_dst] = favg;


					// summed-area table lookups for variance
					double4 v1sum =
						(varbuf1[z_src + 1][y_src - avg_var_kernel_radius][x_src - avg_var_kernel_radius]
						 - varbuf1[z_src + 1][y_src - avg_var_kernel_radius][x_src + avg_var_kernel_radius + 1]
						 - varbuf1[z_src + 1][y_src + avg_var_kernel_radius + 1][x_src - avg_var_kernel_radius]
						 + varbuf1[z_src + 1][y_src + avg_var_kernel_radius + 1][x_src + avg_var_kernel_radius + 1]) -
						(varbuf1[z_src][y_src - avg_var_kernel_radius][x_src - avg_var_kernel_radius]
						 - varbuf1[z_src][y_src - avg_var_kernel_radius][x_src + avg_var_kernel_radius + 1]
						 - varbuf1[z_src][y_src + avg_var_kernel_radius + 1][x_src - avg_var_kernel_radius] + varbuf1[z_src][y_src + avg_var_kernel_radius + 1][x_src + avg_var_kernel_radius + 1]);
					double4 v2sum =
						(varbuf2[z_src + 1][y_src - avg_var_kernel_radius][x_src - avg_var_kernel_radius]
						 - varbuf2[z_src + 1][y_src - avg_var_kernel_radius][x_src + avg_var_kernel_radius + 1]
						 - varbuf2[z_src + 1][y_src + avg_var_kernel_radius + 1][x_src - avg_var_kernel_radius]
						 + varbuf2[z_src + 1][y_src + avg_var_kernel_radius + 1][x_src + avg_var_kernel_radius + 1]) -
						(varbuf2[z_src][y_src - avg_var_kernel_radius][x_src - avg_var_kernel_radius]
						 - varbuf2[z_src][y_src - avg_var_kernel_radius][x_src + avg_var_kernel_radius + 1]
						 - varbuf2[z_src][y_src + avg_var_kernel_radius + 1][x_src - avg_var_kernel_radius] + varbuf2[z_src][y_src + avg_var_kernel_radius + 1][x_src + avg_var_kernel_radius + 1]);

					// the actual variance
					double4 variance = mul2 * v2sum - mul1 * (v1sum * v1sum);

					float4 fvar = float4(static_cast < float >(variance.x),
										 static_cast < float >(variance.y),
										 static_cast < float >(variance.z),
										 static_cast < float >(variance.w));
					input_variances[z_dst][y_dst][x_dst] = fvar;
				}
			}
		}
	}
	else
	{
		for (z = 0; z < zsize; z++)
		{
			int z_src = z;
			int z_dst = z + dest_zoffset;
			for (y = 0; y < ysize; y++)
			{
				int y_src = y + kernel_radius;
				int y_dst = y + dest_yoffset;

				for (x = 0; x < xsize; x++)
				{
					int x_src = x + kernel_radius;
					int x_dst = x + dest_xoffset;

					// summed-area table lookups for alpha average
					double vasum =
						varbuf1[z_src][y_src - alpha_kernel_radius][x_src - alpha_kernel_radius].w
						- varbuf1[z_src][y_src - alpha_kernel_radius][x_src + alpha_kernel_radius + 1].w
						- varbuf1[z_src][y_src + alpha_kernel_radius + 1][x_src - alpha_kernel_radius].w + varbuf1[z_src][y_src + alpha_kernel_radius + 1][x_src + alpha_kernel_radius + 1].w;
					input_alpha_averages[z_dst][y_dst][x_dst] = static_cast < float >(vasum * alpha_rsamples);


					// summed-area table lookups for RGBA average
					double4 v0sum =
						varbuf1[z_src][y_src - avg_var_kernel_radius][x_src - avg_var_kernel_radius]
						- varbuf1[z_src][y_src - avg_var_kernel_radius][x_src + avg_var_kernel_radius + 1]
						- varbuf1[z_src][y_src + avg_var_kernel_radius + 1][x_src - avg_var_kernel_radius] + varbuf1[z_src][y_src + avg_var_kernel_radius + 1][x_src + avg_var_kernel_radius + 1];

					double4 avg = v0sum * avg_var_rsamples;

					float4 favg = float4(static_cast < float >(avg.x),
										 static_cast < float >(avg.y),
										 static_cast < float >(avg.z),
										 static_cast < float >(avg.w));
					input_averages[z_dst][y_dst][x_dst] = favg;


					// summed-area table lookups for variance
					double4 v1sum =
						varbuf1[z_src][y_src - avg_var_kernel_radius][x_src - avg_var_kernel_radius]
						- varbuf1[z_src][y_src - avg_var_kernel_radius][x_src + avg_var_kernel_radius + 1]
						- varbuf1[z_src][y_src + avg_var_kernel_radius + 1][x_src - avg_var_kernel_radius] + varbuf1[z_src][y_src + avg_var_kernel_radius + 1][x_src + avg_var_kernel_radius + 1];
					double4 v2sum =
						varbuf2[z_src][y_src - avg_var_kernel_radius][x_src - avg_var_kernel_radius]
						- varbuf2[z_src][y_src - avg_var_kernel_radius][x_src + avg_var_kernel_radius + 1]
						- varbuf2[z_src][y_src + avg_var_kernel_radius + 1][x_src - avg_var_kernel_radius] + varbuf2[z_src][y_src + avg_var_kernel_radius + 1][x_src + avg_var_kernel_radius + 1];

					// the actual variance
					double4 variance = mul2 * v2sum - mul1 * (v1sum * v1sum);

					float4 fvar = float4(static_cast < float >(variance.x),
										 static_cast < float >(variance.y),
										 static_cast < float >(variance.z),
										 static_cast < float >(variance.w));
					input_variances[z_dst][y_dst][x_dst] = fvar;
				}
			}
		}
	}
	delete[]varbuf2[0][0];
	delete[]varbuf1[0][0];
	delete[]varbuf2[0];
	delete[]varbuf1[0];
	delete[]varbuf2;
	delete[]varbuf1;
}


static void allocate_input_average_and_variance_buffers(int xsize, int ysize, int zsize)
{
	int y, z;
	if (input_averages)
	{
		delete[]input_averages[0][0];
		delete[]input_averages[0];
		delete[]input_averages;
	}
	if (input_variances)
	{
		delete[]input_variances[0][0];
		delete[]input_variances[0];
		delete[]input_variances;
	}
	if (input_alpha_averages)
	{
		delete[]input_alpha_averages[0][0];
		delete[]input_alpha_averages[0];
		delete[]input_alpha_averages;
	}

	input_averages = new float4 **[zsize];
	input_variances = new float4 **[zsize];
	input_alpha_averages = new float **[zsize];


	input_averages[0] = new float4 *[ysize * zsize];
	input_variances[0] = new float4 *[ysize * zsize];
	input_alpha_averages[0] = new float *[ysize * zsize];

	input_averages[0][0] = new float4[xsize * ysize * zsize];
	input_variances[0][0] = new float4[xsize * ysize * zsize];
	input_alpha_averages[0][0] = new float[xsize * ysize * zsize];

	for (z = 1; z < zsize; z++)
	{
		input_averages[z] = input_averages[0] + z * ysize;
		input_variances[z] = input_variances[0] + z * ysize;
		input_alpha_averages[z] = input_alpha_averages[0] + z * ysize;

		input_averages[z][0] = input_averages[0][0] + z * ysize * xsize;
		input_variances[z][0] = input_variances[0][0] + z * ysize * xsize;
		input_alpha_averages[z][0] = input_alpha_averages[0][0] + z * ysize * xsize;
	}

	for (z = 0; z < zsize; z++)
		for (y = 1; y < ysize; y++)
		{
			input_averages[z][y] = input_averages[z][0] + y * xsize;
			input_variances[z][y] = input_variances[z][0] + y * xsize;
			input_alpha_averages[z][y] = input_alpha_averages[z][0] + y * xsize;
		}

}


// compute averages and variances for the current input image.
void compute_averages_and_variances(const astc_codec_image * img, float rgb_power_to_use, float alpha_power_to_use, int avg_var_kernel_radius, int alpha_kernel_radius, swizzlepattern swz)
{
	int xsize = img->xsize;
	int ysize = img->ysize;
	int zsize = img->zsize;
	allocate_input_average_and_variance_buffers(xsize, ysize, zsize);


	int x, y, z;
	for (z = 0; z < zsize; z += 32)
	{
		int zblocksize = MIN(32, zsize - z);
		for (y = 0; y < ysize; y += 32)
		{
			int yblocksize = MIN(32, ysize - y);
			for (x = 0; x < xsize; x += 32)
			{
				int xblocksize = MIN(32, xsize - x);
				compute_pixel_region_variance(img,
											  rgb_power_to_use,
											  alpha_power_to_use,
											  swz,
											  (zsize > 1),
											  x + img->padding,
											  y + img->padding, z + (zsize > 1 ? img->padding : 0), xblocksize, yblocksize, zblocksize, avg_var_kernel_radius, alpha_kernel_radius, x, y, z);
			}
		}
	}
}
