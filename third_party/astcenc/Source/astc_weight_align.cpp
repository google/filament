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
 *	@brief	Angular-sum algorithm for weight alignment.
 *
 *			This algorithm works as follows:
 *			* we compute a complex number P as (cos s*i, sin s*i) for each
 *			  weight, where i is the input value and s is a scaling factor
 *			  based on the spacing between the weights.
 *			* we then add together complex numbers for all the weights.
 *			* we then compute the length and angle of the resulting sum.
 *
 *			This should produce the following results:
 *			* perfect alignment results in a vector whose length is equal to
 *			  the sum of lengths of all inputs
 *			* even distribution results in a vector of length 0.
 *			* all samples identical results in perfect alignment for every
 *			  scaling.
 *
 *			For each scaling factor within a given set, we compute an alignment
 *			factor from 0 to 1. This should then result in some scalings standing
 *			out as having particularly good alignment factors; we can use this to
 *			produce a set of candidate scale/shift values for various quantization
 *			levels; we should then actually try them and see what happens.
 *
 *			Assuming N quantization steps, the scaling factor becomes s=2*PI*(N-1);
 *			we should probably have about 1 scaling factor for every 1/4
 *			quantization step (perhaps 1/8 for low levels of quantization)
 */
/*----------------------------------------------------------------------------*/

#include <math.h>
#include "astc_codec_internals.h"

#ifdef DEBUG_PRINT_DIAGNOSTICS
	#include <stdio.h>
#endif

static const float angular_steppings[] = {
	1.0, 1.125,
	1.25, 1.375,
	1.5, 1.625,
	1.75, 1.875,

	2.0, 2.25, 2.5, 2.75,
	3.0, 3.25, 3.5, 3.75,
	4.0, 4.25, 4.5, 4.75,
	5.0, 5.25, 5.5, 5.75,
	6.0, 6.25, 6.5, 6.75,
	7.0, 7.25, 7.5, 7.75,

	8.0, 8.5,
	9.0, 9.5,
	10.0, 10.5,
	11.0, 11.5,
	12.0, 12.5,
	13.0, 13.5,
	14.0, 14.5,
	15.0, 15.5,
	16.0, 16.5,
	17.0, 17.5,
	18.0, 18.5,
	19.0, 19.5,
	20.0, 20.5,
	21.0, 21.5,
	22.0, 22.5,
	23.0, 23.5,
	24.0, 24.5,
	25.0, 25.5,
	26.0, 26.5,
	27.0, 27.5,
	28.0, 28.5,
	29.0, 29.5,
	30.0, 30.5,
	31.0, 31.5,
	32.0, 32.5,
	33.0, 33.5,
	34.0, 34.5,
	35.0, 35.5,
};

#define ANGULAR_STEPS ((int)(sizeof(angular_steppings)/sizeof(angular_steppings[0])))

static float stepsizes[ANGULAR_STEPS];
static float stepsizes_sqr[ANGULAR_STEPS];

static int max_angular_steps_needed_for_quant_level[13];

// we store sine/cosine values for 64 possible weight values; this causes
// slight quality loss compared to using sin() and cos() directly.

#define SINCOS_STEPS 64

static float sin_table[SINCOS_STEPS][ANGULAR_STEPS];
static float cos_table[SINCOS_STEPS][ANGULAR_STEPS];

void prepare_angular_tables(void)
{
	int i, j;
	int max_angular_steps_needed_for_quant_steps[40];
	for (i = 0; i < ANGULAR_STEPS; i++)
	{
		stepsizes[i] = 1.0f / angular_steppings[i];
		stepsizes_sqr[i] = stepsizes[i] * stepsizes[i];

		for (j = 0; j < SINCOS_STEPS; j++)
		{
			sin_table[j][i] = static_cast < float >(sin((2.0f * M_PI / (SINCOS_STEPS - 1.0f)) * angular_steppings[i] * j));
			cos_table[j][i] = static_cast < float >(cos((2.0f * M_PI / (SINCOS_STEPS - 1.0f)) * angular_steppings[i] * j));
		}

		int p = static_cast < int >(floor(angular_steppings[i])) + 1;
		max_angular_steps_needed_for_quant_steps[p] = MIN(i + 1, ANGULAR_STEPS - 1);
	}


	// yes, the next-to-last entry is supposed to have the value 33. This because under
	// ASTC, the 32-weight mode leaves a double-sized hole in the middle of the
	// weight space, so we are better off matching 33 weights than 32.
	static const int steps_of_level[] = { 2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 33, 36 };

	for (i = 0; i < 13; i++)
		max_angular_steps_needed_for_quant_level[i] = max_angular_steps_needed_for_quant_steps[steps_of_level[i]];

}


union if32
{
	float f;
	int32_t s;
	uint32_t u;
};


// function to compute angular sums; then, from the
// angular sums, compute alignment factor and offset.

/* static inline */
void compute_angular_offsets(int samplecount, const float *samples, const float *sample_weights, int max_angular_steps, float *offsets)
{
	int i, j;

	float anglesum_x[ANGULAR_STEPS];
	float anglesum_y[ANGULAR_STEPS];

	for (i = 0; i < max_angular_steps; i++)
	{
		anglesum_x[i] = 0;
		anglesum_y[i] = 0;
	}


	// compute the angle-sums.
	for (i = 0; i < samplecount; i++)
	{
		float sample = samples[i];
		float sample_weight = sample_weights[i];
		if32 p;
		p.f = (sample * (SINCOS_STEPS - 1.0f)) + 12582912.0f;
		unsigned int isample = p.u & 0x3F;

		const float *sinptr = sin_table[isample];
		const float *cosptr = cos_table[isample];

		for (j = 0; j < max_angular_steps; j++)
		{
			float cp = cosptr[j];
			float sp = sinptr[j];

			anglesum_x[j] += cp * sample_weight;
			anglesum_y[j] += sp * sample_weight;
		}
	}

	// post-process the angle-sums
	for (i = 0; i < max_angular_steps; i++)
	{
		float angle = atan2(anglesum_y[i], anglesum_x[i]);	// positive angle -> positive offset
		offsets[i] = angle * (stepsizes[i] * (1.0f / (2.0f * (float)M_PI)));
	}
}



// for a given step-size and a given offset, compute the
// lowest and highest weight that results from quantizing using the stepsize & offset.
// also, compute the resulting error.


/* static inline */
void compute_lowest_and_highest_weight(int samplecount, const float *samples, const float *sample_weights,
									  int max_angular_steps, const float *offsets,
									  int8_t * lowest_weight, int8_t * highest_weight,
									  float *error, float *cut_low_weight_error, float *cut_high_weight_error)
{
	int i;

	int sp;

	float error_from_forcing_weight_down[60];
	float error_from_forcing_weight_either_way[60];
	for (i = 0; i < 60; i++)
	{
		error_from_forcing_weight_down[i] = 0;
		error_from_forcing_weight_either_way[i] = 0;
	}

	// weight + 12
	static const unsigned int idxtab[256] = {

		12, 13, 14, 15, 16, 17, 18, 19,
		20, 21, 22, 23, 24, 25, 26, 27,
		28, 29, 30, 31, 32, 33, 34, 35,
		36, 37, 38, 39, 40, 41, 42, 43,
		44, 45, 46, 47, 48, 49, 50, 51,
		52, 53, 54, 55, 55, 55, 55, 55,
		55, 55, 55, 55, 55, 55, 55, 55,
		55, 55, 55, 55, 55, 55, 55, 55,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 2, 3,
		4, 5, 6, 7, 8, 9, 10, 11,

		12, 13, 14, 15, 16, 17, 18, 19,
		20, 21, 22, 23, 24, 25, 26, 27,
		28, 29, 30, 31, 32, 33, 34, 35,
		36, 37, 38, 39, 40, 41, 42, 43,
		44, 45, 46, 47, 48, 49, 50, 51,
		52, 53, 54, 55, 55, 55, 55, 55,
		55, 55, 55, 55, 55, 55, 55, 55,
		55, 55, 55, 55, 55, 55, 55, 55,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 1, 2, 3,
		4, 5, 6, 7, 8, 9, 10, 11
	};



	for (sp = 0; sp < max_angular_steps; sp++)
	{
		unsigned int minidx_bias12 = 55;
		unsigned int maxidx_bias12 = 0;

		float errval = 0.0f;

		float rcp_stepsize = angular_steppings[sp];
		float offset = offsets[sp];

		float scaled_offset = rcp_stepsize * offset;


		for (i = 0; i < samplecount - 1; i += 2)
		{
			float wt1 = sample_weights[i];
			float wt2 = sample_weights[i + 1];
			if32 p1, p2;
			float sval1 = (samples[i] * rcp_stepsize) - scaled_offset;
			float sval2 = (samples[i + 1] * rcp_stepsize) - scaled_offset;
			p1.f = sval1 + 12582912.0f;	// FP representation abuse to avoid floor() and float->int conversion
			p2.f = sval2 + 12582912.0f;	// FP representation abuse to avoid floor() and float->int conversion
			float isval1 = p1.f - 12582912.0f;
			float isval2 = p2.f - 12582912.0f;
			float dif1 = sval1 - isval1;
			float dif2 = sval2 - isval2;

			errval += (dif1 * wt1) * dif1;
			errval += (dif2 * wt2) * dif2;

			// table lookups that really perform a minmax function.
			unsigned int idx1_bias12 = idxtab[p1.u & 0xFF];
			unsigned int idx2_bias12 = idxtab[p2.u & 0xFF];

			if (idx1_bias12 < minidx_bias12)
				minidx_bias12 = idx1_bias12;
			if (idx1_bias12 > maxidx_bias12)
				maxidx_bias12 = idx1_bias12;
			if (idx2_bias12 < minidx_bias12)
				minidx_bias12 = idx2_bias12;
			if (idx2_bias12 > maxidx_bias12)
				maxidx_bias12 = idx2_bias12;

			error_from_forcing_weight_either_way[idx1_bias12] += wt1;
			error_from_forcing_weight_down[idx1_bias12] += (dif1 * wt1);

			error_from_forcing_weight_either_way[idx2_bias12] += wt2;
			error_from_forcing_weight_down[idx2_bias12] += (dif2 * wt2);
		}

		if (samplecount & 1)
		{
			i = samplecount - 1;
			float wt = sample_weights[i];
			if32 p;
			float sval = (samples[i] * rcp_stepsize) - scaled_offset;
			p.f = sval + 12582912.0f;	// FP representation abuse to avoid floor() and float->int conversion
			float isval = p.f - 12582912.0f;
			float dif = sval - isval;

			errval += (dif * wt) * dif;

			unsigned int idx_bias12 = idxtab[p.u & 0xFF];

			if (idx_bias12 < minidx_bias12)
				minidx_bias12 = idx_bias12;
			if (idx_bias12 > maxidx_bias12)
				maxidx_bias12 = idx_bias12;

			error_from_forcing_weight_either_way[idx_bias12] += wt;
			error_from_forcing_weight_down[idx_bias12] += dif * wt;
		}


		lowest_weight[sp] = (int)minidx_bias12 - 12;
		highest_weight[sp] = (int)maxidx_bias12 - 12;
		error[sp] = errval;

		// the cut_(lowest/highest)_weight_error indicate the error that results from
		// forcing samples that should have had the (lowest/highest) weight value
		// one step (up/down).
		cut_low_weight_error[sp] = error_from_forcing_weight_either_way[minidx_bias12] - 2.0f * error_from_forcing_weight_down[minidx_bias12];
		cut_high_weight_error[sp] = error_from_forcing_weight_either_way[maxidx_bias12] + 2.0f * error_from_forcing_weight_down[maxidx_bias12];

		// clear out the error-from-forcing values we actually used in this pass
		// so that these are clean for the next pass.
		unsigned int ui;
		for (ui = minidx_bias12 & ~0x3; ui <= maxidx_bias12; ui += 4)
		{
			error_from_forcing_weight_either_way[ui] = 0;
			error_from_forcing_weight_down[ui] = 0;
			error_from_forcing_weight_either_way[ui + 1] = 0;
			error_from_forcing_weight_down[ui + 1] = 0;
			error_from_forcing_weight_either_way[ui + 2] = 0;
			error_from_forcing_weight_down[ui + 2] = 0;
			error_from_forcing_weight_either_way[ui + 3] = 0;
			error_from_forcing_weight_down[ui + 3] = 0;
		}
	}


	for (sp = 0; sp < max_angular_steps; sp++)
	{
		float errscale = stepsizes_sqr[sp];
		error[sp] *= errscale;
		cut_low_weight_error[sp] *= errscale;
		cut_high_weight_error[sp] *= errscale;
	}
}



// main function for running the angular algorithm.


void compute_angular_endpoints_for_quantization_levels(int samplecount, const float *samples, const float *sample_weights, int max_quantization_level, float low_value[12], float high_value[12])
{
	int i;


	max_quantization_level++;	// Temporarily increase level - needs refinement

	static const int quantization_steps_for_level[13] = { 2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 33, 36 };
	int max_quantization_steps = quantization_steps_for_level[max_quantization_level];

	float offsets[ANGULAR_STEPS];

	int max_angular_steps = max_angular_steps_needed_for_quant_level[max_quantization_level];

	compute_angular_offsets(samplecount, samples, sample_weights, max_angular_steps, offsets);


	// the +4 offsets are to allow for vectorization within compute_lowest_and_highest_weight().
	int8_t lowest_weight[ANGULAR_STEPS + 4];
	int8_t highest_weight[ANGULAR_STEPS + 4];
	float error[ANGULAR_STEPS + 4];

	float cut_low_weight_error[ANGULAR_STEPS + 4];
	float cut_high_weight_error[ANGULAR_STEPS + 4];

	compute_lowest_and_highest_weight(samplecount, samples, sample_weights, max_angular_steps, offsets, lowest_weight, highest_weight, error, cut_low_weight_error, cut_high_weight_error);


	#ifdef DEBUG_PRINT_DIAGNOSTICS
		if (print_diagnostics)
		{
			printf("%s : max-angular-steps=%d \n", __func__, max_angular_steps);
			printf("Samplecount=%d, max_quantization_level=%d\n", samplecount, max_quantization_level);
			for (i = 0; i < samplecount; i++)
				printf("Sample %d : %f (weight %f)\n", i, samples[i], sample_weights[i]);

			for (i = 0; i < max_angular_steps; i++)
			{
				printf("%d: offset=%f error=%f lowest=%d highest=%d cl=%f ch=%f\n", i, offsets[i], error[i], lowest_weight[i], highest_weight[i], cut_low_weight_error[i], cut_high_weight_error[i]);
			}
			printf("\n");
		}
	#endif

	// for each quantization level, find the best error terms.
	float best_errors[40];
	int best_scale[40];
	uint8_t cut_low_weight[40];

	for (i = 0; i < (max_quantization_steps + 4); i++)
	{
		best_errors[i] = 1e30f;
		best_scale[i] = -1;	// Indicates no solution found
		cut_low_weight[i] = 0;
	}



	for (i = 0; i < max_angular_steps; i++)
	{
		int samplecount = highest_weight[i] - lowest_weight[i] + 1;
		if (samplecount >= (max_quantization_steps + 4))
		{
			continue;
		}
		if (samplecount < 2)
			samplecount = 2;

		if (best_errors[samplecount] > error[i])
		{
			best_errors[samplecount] = error[i];
			best_scale[samplecount] = i;
			cut_low_weight[samplecount] = 0;
		}

		float error_cut_low = error[i] + cut_low_weight_error[i];
		float error_cut_high = error[i] + cut_high_weight_error[i];
		float error_cut_low_high = error[i] + cut_low_weight_error[i] + cut_high_weight_error[i];

		if (best_errors[samplecount - 1] > error_cut_low)
		{
			best_errors[samplecount - 1] = error_cut_low;
			best_scale[samplecount - 1] = i;
			cut_low_weight[samplecount - 1] = 1;
		}

		if (best_errors[samplecount - 1] > error_cut_high)
		{
			best_errors[samplecount - 1] = error_cut_high;
			best_scale[samplecount - 1] = i;
			cut_low_weight[samplecount - 1] = 0;
		}

		if (best_errors[samplecount - 2] > error_cut_low_high)
		{
			best_errors[samplecount - 2] = error_cut_low_high;
			best_scale[samplecount - 2] = i;
			cut_low_weight[samplecount - 2] = 1;
		}

	}

	// if we got a better error-value for a low sample count than for a high one,
	// use the low sample count error value for the higher sample count as well.
	for (i = 3; i <= max_quantization_steps; i++)
	{
		if (best_errors[i] > best_errors[i - 1])
		{
			best_errors[i] = best_errors[i - 1];
			best_scale[i] = best_scale[i - 1];
			cut_low_weight[i] = cut_low_weight[i - 1];
		}
	}


	max_quantization_level--;	// Decrease level again (see corresponding ++, above)

	static const int ql_weights[12] = { 2, 3, 4, 5, 6, 8, 10, 12, 16, 20, 24, 33 };
	for (i = 0; i <= max_quantization_level; i++)
	{
		int q = ql_weights[i];
		int bsi = best_scale[q];

		// Did we find anything?
		if(bsi < 0)
		{
			printf("ERROR: Unable to find an encoding within the specified error limits. Please revise the error limit values and try again.\n");
			exit(1);
		}

		float stepsize = stepsizes[bsi];
		int lwi = lowest_weight[bsi] + cut_low_weight[q];
		int hwi = lwi + q - 1;
		float offset = offsets[bsi];

		low_value[i] = offset + lwi * stepsize;
		high_value[i] = offset + hwi * stepsize;
	}

}


// helper functions that will compute ideal angular-endpoints
// for a given set of weights and a given block size descriptors

void compute_angular_endpoints_1plane(float mode_cutoff, const block_size_descriptor * bsd,
									  const float *decimated_quantized_weights, const float *decimated_weights,
									  float low_value[MAX_WEIGHT_MODES], float high_value[MAX_WEIGHT_MODES])
{
	int i;
	float low_values[MAX_DECIMATION_MODES][12];
	float high_values[MAX_DECIMATION_MODES][12];

	for (i = 0; i < MAX_DECIMATION_MODES; i++)
	{
		int samplecount = bsd->decimation_mode_samples[i];
		int quant_mode = bsd->decimation_mode_maxprec_1plane[i];
		float percentile = bsd->decimation_mode_percentile[i];
		int permit_encode = bsd->permit_encode[i];
		if (permit_encode == 0 || samplecount < 1 || quant_mode < 0 || percentile > mode_cutoff)
			continue;


		compute_angular_endpoints_for_quantization_levels(samplecount,
														  decimated_quantized_weights + i * MAX_WEIGHTS_PER_BLOCK,
														  decimated_weights + i * MAX_WEIGHTS_PER_BLOCK, quant_mode, low_values[i], high_values[i]);
	}

	for (i = 0; i < MAX_WEIGHT_MODES; i++)
	{
		if (bsd->block_modes[i].is_dual_plane != 0 || bsd->block_modes[i].percentile > mode_cutoff)
			continue;
		int quant_mode = bsd->block_modes[i].quantization_mode;
		int decim_mode = bsd->block_modes[i].decimation_mode;

		low_value[i] = low_values[decim_mode][quant_mode];
		high_value[i] = high_values[decim_mode][quant_mode];
	}

}



void compute_angular_endpoints_2planes(float mode_cutoff,
									   const block_size_descriptor * bsd,
									   const float *decimated_quantized_weights,
									   const float *decimated_weights,
									   float low_value1[MAX_WEIGHT_MODES], float high_value1[MAX_WEIGHT_MODES], float low_value2[MAX_WEIGHT_MODES], float high_value2[MAX_WEIGHT_MODES])
{
	int i;
	float low_values1[MAX_DECIMATION_MODES][12];
	float high_values1[MAX_DECIMATION_MODES][12];
	float low_values2[MAX_DECIMATION_MODES][12];
	float high_values2[MAX_DECIMATION_MODES][12];

	for (i = 0; i < MAX_DECIMATION_MODES; i++)
	{
		int samplecount = bsd->decimation_mode_samples[i];
		int quant_mode = bsd->decimation_mode_maxprec_2planes[i];
		float percentile = bsd->decimation_mode_percentile[i];
		int permit_encode = bsd->permit_encode[i];
		if (permit_encode == 0 || samplecount < 1 || quant_mode < 0 || percentile > mode_cutoff)
			continue;

		compute_angular_endpoints_for_quantization_levels(samplecount,
														  decimated_quantized_weights + 2 * i * MAX_WEIGHTS_PER_BLOCK,
														  decimated_weights + 2 * i * MAX_WEIGHTS_PER_BLOCK, quant_mode, low_values1[i], high_values1[i]);

		compute_angular_endpoints_for_quantization_levels(samplecount,
														  decimated_quantized_weights + (2 * i + 1) * MAX_WEIGHTS_PER_BLOCK,
														  decimated_weights + (2 * i + 1) * MAX_WEIGHTS_PER_BLOCK, quant_mode, low_values2[i], high_values2[i]);

	}

	for (i = 0; i < MAX_WEIGHT_MODES; i++)
	{
		if (bsd->block_modes[i].is_dual_plane != 1 || bsd->block_modes[i].percentile > mode_cutoff)
			continue;
		int quant_mode = bsd->block_modes[i].quantization_mode;
		int decim_mode = bsd->block_modes[i].decimation_mode;

		low_value1[i] = low_values1[decim_mode][quant_mode];
		high_value1[i] = high_values1[decim_mode][quant_mode];
		low_value2[i] = low_values2[decim_mode][quant_mode];
		high_value2[i] = high_values2[decim_mode][quant_mode];
	}
}
