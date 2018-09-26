/*----------------------------------------------------------------------------*/
/**
 *	This confidential and proprietary software may be used only as
 *	authorised by a licensing agreement from ARM Limited
 *	(C) COPYRIGHT 2011-2012, 2018 ARM Limited
 *	ALL RIGHTS RESERVED
 *
 *	The entire notice above must be reproduced on all authorised
 *	copies and copies may only be made to the extent permitted
 *	by a licensing agreement from ARM Limited.
 *
 *	@brief	Internal function and data declarations for ASTC codec.
 */
/*----------------------------------------------------------------------------*/

#ifndef ASTC_CODEC_INTERNALS_INCLUDED

#define ASTC_CODEC_INTERNALS_INCLUDED

#include <stdint.h>
#include <stdlib.h>
#include "mathlib.h"

#ifndef MIN
	#define MIN(x,y) ((x)<(y)?(x):(y))
#endif

#ifndef MAX
	#define MAX(x,y) ((x)>(y)?(x):(y))
#endif

// Macro to silence warnings on ignored parameters.
// The presence of this macro should be a signal to look at refactoring.
#define IGNORE(param) ((void)&param)

#define astc_isnan(p) ((p)!=(p))

// ASTC parameters
#define MAX_TEXELS_PER_BLOCK 216
#define MAX_WEIGHTS_PER_BLOCK 64
#define MIN_WEIGHT_BITS_PER_BLOCK 24
#define MAX_WEIGHT_BITS_PER_BLOCK 96
#define PARTITION_BITS 10
#define PARTITION_COUNT (1 << PARTITION_BITS)

// the sum of weights for one texel.
#define TEXEL_WEIGHT_SUM 16
#define MAX_DECIMATION_MODES 87
#define MAX_WEIGHT_MODES 2048

// error reporting for codec internal errors.
#define ASTC_CODEC_INTERNAL_ERROR astc_codec_internal_error(__FILE__, __LINE__)

void astc_codec_internal_error(const char *filename, int linenumber);

// uncomment this macro to enable checking for inappropriate NaNs;
// works on Linux only, and slows down encoding significantly.
// #define DEBUG_CAPTURE_NAN

// the PRINT_DIAGNOSTICS macro enables the -diag command line switch,
// which can be used to look for codec bugs
#define DEBUG_PRINT_DIAGNOSTICS

#ifdef DEBUG_PRINT_DIAGNOSTICS
	extern int print_diagnostics;
#endif

extern int print_tile_errors;
extern int print_statistics;

extern int perform_srgb_transform;
extern int rgb_force_use_of_hdr;
extern int alpha_force_use_of_hdr;

struct processed_line2
{
	float2 amod;
	float2 bs;
	float2 bis;
};
struct processed_line3
{
	float3 amod;
	float3 bs;
	float3 bis;
};
struct processed_line4
{
	float4 amod;
	float4 bs;
	float4 bis;
};

enum astc_decode_mode
{
	DECODE_LDR_SRGB,
	DECODE_LDR,
	DECODE_HDR
};


/*
	Partition table representation:
	For each block size, we have 3 tables, each with 1024 partitionings;
	these three tables correspond to 2, 3 and 4 partitions respectively.
	For each partitioning, we have:
	* a 4-entry table indicating how many texels there are in each of the 4 partitions.
	  This may be from 0 to a very large value.
	* a table indicating the partition index of each of the texels in the block.
	  Each index may be 0, 1, 2 or 3.
	* Each element in the table is an uint8_t indicating partition index (0, 1, 2 or 3)
*/

struct partition_info
{
	int partition_count;
	uint8_t texels_per_partition[4];
	uint8_t partition_of_texel[MAX_TEXELS_PER_BLOCK];
	uint8_t texels_of_partition[4][MAX_TEXELS_PER_BLOCK];

	uint64_t coverage_bitmaps[4];	// used for the purposes of k-means partition search.
};




/*
   In ASTC, we don't necessarily provide a weight for every texel.
   As such, for each block size, there are a number of patterns where some texels
   have their weights computed as a weighted average of more than 1 weight.
   As such, the codec uses a data structure that tells us: for each texel, which
   weights it is a combination of for each weight, which texels it contributes to.
   The decimation_table is this data structure.
*/
struct decimation_table
{
	int num_texels;
	int num_weights;
	uint8_t texel_num_weights[MAX_TEXELS_PER_BLOCK];	// number of indices that go into the calculation for a texel
	uint8_t texel_weights_int[MAX_TEXELS_PER_BLOCK][4];	// the weight to assign to each weight
	float texel_weights_float[MAX_TEXELS_PER_BLOCK][4];	// the weight to assign to each weight
	uint8_t texel_weights[MAX_TEXELS_PER_BLOCK][4];	// the weights that go into a texel calculation
	uint8_t weight_num_texels[MAX_WEIGHTS_PER_BLOCK];	// the number of texels that a given weight contributes to
	uint8_t weight_texel[MAX_WEIGHTS_PER_BLOCK][MAX_TEXELS_PER_BLOCK];	// the texels that the weight contributes to
	uint8_t weights_int[MAX_WEIGHTS_PER_BLOCK][MAX_TEXELS_PER_BLOCK];	// the weights that the weight contributes to a texel.
	float weights_flt[MAX_WEIGHTS_PER_BLOCK][MAX_TEXELS_PER_BLOCK];	// the weights that the weight contributes to a texel.
};




/*
   data structure describing information that pertains to a block size and its associated block modes.
*/
struct block_mode
{
	int8_t decimation_mode;
	int8_t quantization_mode;
	int8_t is_dual_plane;
	int8_t permit_encode;
	int8_t permit_decode;
	float percentile;
};


struct block_size_descriptor
{
	int decimation_mode_count;
	int decimation_mode_samples[MAX_DECIMATION_MODES];
	int decimation_mode_maxprec_1plane[MAX_DECIMATION_MODES];
	int decimation_mode_maxprec_2planes[MAX_DECIMATION_MODES];
	float decimation_mode_percentile[MAX_DECIMATION_MODES];
	int permit_encode[MAX_DECIMATION_MODES];
	const decimation_table *decimation_tables[MAX_DECIMATION_MODES + 1];
	block_mode block_modes[MAX_WEIGHT_MODES];

	// for the k-means bed bitmap partitioning algorithm, we don't
	// want to consider more than 64 texels; this array specifies
	// which 64 texels (if that many) to consider.
	int texelcount_for_bitmap_partitioning;
	int texels_for_bitmap_partitioning[64];
};

// data structure representing one block of an image.
// it is expanded to float prior to processing to save some computation time
// on conversions to/from uint8_t (this also allows us to handle HDR textures easily)
struct imageblock
{
	float orig_data[MAX_TEXELS_PER_BLOCK * 4];  // original input data
	float work_data[MAX_TEXELS_PER_BLOCK * 4];  // the data that we will compress, either linear or LNS (0..65535 in both cases)
	float deriv_data[MAX_TEXELS_PER_BLOCK * 4]; // derivative of the conversion function used, used to modify error weighting

	uint8_t rgb_lns[MAX_TEXELS_PER_BLOCK];      // 1 if RGB data are being treated as LNS
	uint8_t alpha_lns[MAX_TEXELS_PER_BLOCK];    // 1 if Alpha data are being treated as LNS
	uint8_t nan_texel[MAX_TEXELS_PER_BLOCK];    // 1 if the texel is a NaN-texel.

	float red_min, red_max;
	float green_min, green_max;
	float blue_min, blue_max;
	float alpha_min, alpha_max;
	int grayscale;				// 1 if R=G=B for every pixel, 0 otherwise

	int xpos, ypos, zpos;
};


struct error_weighting_params
{
	float rgb_power;
	float rgb_base_weight;
	float rgb_mean_weight;
	float rgb_stdev_weight;
	float alpha_power;
	float alpha_base_weight;
	float alpha_mean_weight;
	float alpha_stdev_weight;
	float rgb_mean_and_stdev_mixing;
	int mean_stdev_radius;
	int enable_rgb_scale_with_alpha;
	int alpha_radius;
	int ra_normal_angular_scale;
	float block_artifact_suppression;
	float rgba_weights[4];

	float block_artifact_suppression_expanded[MAX_TEXELS_PER_BLOCK];

	// parameters that deal with heuristic codec speedups
	int partition_search_limit;
	float block_mode_cutoff;
	float texel_avg_error_limit;
	float partition_1_to_2_limit;
	float lowest_correlation_cutoff;
	int max_refinement_iters;
};




void update_imageblock_flags(imageblock * pb, int xdim, int ydim, int zdim);


void imageblock_initialize_orig_from_work(imageblock * pb, int pixelcount);


void imageblock_initialize_work_from_orig(imageblock * pb, int pixelcount);



/*
	Data structure representing error weighting for one block of an image. this is used as
	a multiplier for the error weight to apply to each color component when computing PSNR.

	This weighting has several uses: it's usable for RA, GA, BA, A weighting, which is useful
	for alpha-textures it's usable for HDR textures, where weighting should be approximately inverse to
	luminance it's usable for perceptual weighting, where we assign higher weight to low-variability
	regions than to high-variability regions. it's usable for suppressing off-edge block content in
	case the texture doesn't actually extend to the edge of the block.

	For the default case (everything is evenly weighted), every weight is 1. For the RA,GA,BA,A case,
	we multiply the R,G,B weights with that of the alpha.

	Putting the same weight in every component should result in the default case.
	The following relations should hold:

	texel_weight_rg[i] = (texel_weight_r[i] + texel_weight_g[i]) / 2
	texel_weight_lum[i] = (texel_weight_r[i] + texel_weight_g[i] + texel_weight_b[i]) / 3
	texel_weight[i] = (texel_weight_r[i] + texel_weight_g[i] + texel_weight_b[i] + texel_weight_a[i] / 4
 */

struct error_weight_block
{
	float4 error_weights[MAX_TEXELS_PER_BLOCK];
	float texel_weight[MAX_TEXELS_PER_BLOCK];
	float texel_weight_gba[MAX_TEXELS_PER_BLOCK];
	float texel_weight_rba[MAX_TEXELS_PER_BLOCK];
	float texel_weight_rga[MAX_TEXELS_PER_BLOCK];
	float texel_weight_rgb[MAX_TEXELS_PER_BLOCK];

	float texel_weight_rg[MAX_TEXELS_PER_BLOCK];
	float texel_weight_rb[MAX_TEXELS_PER_BLOCK];
	float texel_weight_gb[MAX_TEXELS_PER_BLOCK];
	float texel_weight_ra[MAX_TEXELS_PER_BLOCK];

	float texel_weight_r[MAX_TEXELS_PER_BLOCK];
	float texel_weight_g[MAX_TEXELS_PER_BLOCK];
	float texel_weight_b[MAX_TEXELS_PER_BLOCK];
	float texel_weight_a[MAX_TEXELS_PER_BLOCK];

	int contains_zeroweight_texels;
};



struct error_weight_block_orig
{
	float4 error_weights[MAX_TEXELS_PER_BLOCK];
};


// enumeration of all the quantization methods we support under this format.
enum quantization_method
{
	QUANT_2 = 0,
	QUANT_3 = 1,
	QUANT_4 = 2,
	QUANT_5 = 3,
	QUANT_6 = 4,
	QUANT_8 = 5,
	QUANT_10 = 6,
	QUANT_12 = 7,
	QUANT_16 = 8,
	QUANT_20 = 9,
	QUANT_24 = 10,
	QUANT_32 = 11,
	QUANT_40 = 12,
	QUANT_48 = 13,
	QUANT_64 = 14,
	QUANT_80 = 15,
	QUANT_96 = 16,
	QUANT_128 = 17,
	QUANT_160 = 18,
	QUANT_192 = 19,
	QUANT_256 = 20
};


/*
	In ASTC, we support relatively many combinations of weight precisions and weight transfer functions.
	As such, for each combination we support, we have a hardwired data structure.

	This structure provides the following information: A table, used to estimate the closest quantized
	weight for a given floating-point weight. For each quantized weight, the corresponding unquantized
	and floating-point values. For each quantized weight, a previous-value and a next-value.
*/

struct quantization_and_transfer_table
{
	quantization_method method;
	uint8_t unquantized_value[32];	// 0..64
	float unquantized_value_flt[32];	// 0..1
	uint8_t prev_quantized_value[32];
	uint8_t next_quantized_value[32];
	uint8_t closest_quantized_weight[1025];
};

extern const quantization_and_transfer_table quant_and_xfer_tables[12];



enum endpoint_formats
{
	FMT_LUMINANCE = 0,
	FMT_LUMINANCE_DELTA = 1,
	FMT_HDR_LUMINANCE_LARGE_RANGE = 2,
	FMT_HDR_LUMINANCE_SMALL_RANGE = 3,
	FMT_LUMINANCE_ALPHA = 4,
	FMT_LUMINANCE_ALPHA_DELTA = 5,
	FMT_RGB_SCALE = 6,
	FMT_HDR_RGB_SCALE = 7,
	FMT_RGB = 8,
	FMT_RGB_DELTA = 9,
	FMT_RGB_SCALE_ALPHA = 10,
	FMT_HDR_RGB = 11,
	FMT_RGBA = 12,
	FMT_RGBA_DELTA = 13,
	FMT_HDR_RGB_LDR_ALPHA = 14,
	FMT_HDR_RGBA = 15,
};



struct symbolic_compressed_block
{
	int error_block;			// 1 marks error block, 0 marks non-error-block.
	int block_mode;				// 0 to 2047. Negative value marks constant-color block (-1: FP16, -2:UINT16)
	int partition_count;		// 1 to 4; Zero marks a constant-color block.
	int partition_index;		// 0 to 1023
	int color_formats[4];		// color format for each endpoint color pair.
	int color_formats_matched;	// color format for all endpoint pairs are matched.
	int color_values[4][12];	// quantized endpoint color pairs.
	int color_quantization_level;
	uint8_t plane1_weights[MAX_WEIGHTS_PER_BLOCK];	// quantized and decimated weights
	uint8_t plane2_weights[MAX_WEIGHTS_PER_BLOCK];
	int plane2_color_component;	// color component for the secondary plane of weights
	int constant_color[4];		// constant-color, as FP16 or UINT16. Used for constant-color blocks only.
};


struct physical_compressed_block
{
	uint8_t data[16];
};




const block_size_descriptor *get_block_size_descriptor(int xdim, int ydim, int zdim);


// ***********************************************************
// functions and data pertaining to quantization and encoding
// **********************************************************
extern const uint8_t color_quantization_tables[21][256];
extern const uint8_t color_unquantization_tables[21][256];

void encode_ise(int quantization_level, int elements, const uint8_t * input_data, uint8_t * output_data, int bit_offset);

void decode_ise(int quantization_level, int elements, const uint8_t * input_data, uint8_t * output_data, int bit_offset);

int compute_ise_bitcount(int items, quantization_method quant);

void build_quantization_mode_table(void);
extern int quantization_mode_table[17][128];


// **********************************************
// functions and data pertaining to partitioning
// **********************************************

// function to get a pointer to a partition table or an array thereof.
const partition_info *get_partition_table(int xdim, int ydim, int zdim, int partition_count);




// functions to compute color averages and dominant directions
// for each partition in a block


void compute_averages_and_directions_rgb(const partition_info * pt,
										 const imageblock * blk,
										 const error_weight_block * ewb,
										 const float4 * color_scalefactors, float3 * averages, float3 * directions_rgb, float2 * directions_rg, float2 * directions_rb, float2 * directions_gb);



void compute_averages_and_directions_rgba(const partition_info * pt,
										  const imageblock * blk,
										  const error_weight_block * ewb,
										  const float4 * color_scalefactors,
										  float4 * averages, float4 * directions_rgba, float3 * directions_gba, float3 * directions_rba, float3 * directions_rga, float3 * directions_rgb);


void compute_averages_and_directions_3_components(const partition_info * pt,
												  const imageblock * blk,
												  const error_weight_block * ewb,
												  const float3 * color_scalefactors, int component1, int component2, int component3, float3 * averages, float3 * directions);

void compute_averages_and_directions_2_components(const partition_info * pt,
												  const imageblock * blk,
												  const error_weight_block * ewb, const float2 * color_scalefactors, int component1, int component2, float2 * averages, float2 * directions);

// functions to compute error value across a tile given a partitioning
// (with the assumption that each partitioning has colors lying on a line where
// they are represented with infinite precision. Also return the length of the line
// segments that the partition's colors are actually projected onto.
float compute_error_squared_gba(const partition_info * pt,	// the partition that we use when computing the squared-error.
								const imageblock * blk, const error_weight_block * ewb, const processed_line3 * plines,
								// output: computed length of the partitioning's line. This is not part of the
								// error introduced by partitioning itself, but us used to estimate the error introduced by quantization
								float *length_of_lines);

float compute_error_squared_rba(const partition_info * pt,	// the partition that we use when computing the squared-error.
								const imageblock * blk, const error_weight_block * ewb, const processed_line3 * plines,
								// output: computed length of the partitioning's line. This is not part of the
								// error introduced by partitioning itself, but us used to estimate the error introduced by quantization
								float *length_of_lines);

float compute_error_squared_rga(const partition_info * pt,	// the partition that we use when computing the squared-error.
								const imageblock * blk, const error_weight_block * ewb, const processed_line3 * plines,
								// output: computed length of the partitioning's line. This is not part of the
								// error introduced by partitioning itself, but us used to estimate the error introduced by quantization
								float *length_of_lines);

float compute_error_squared_rgb(const partition_info * pt,	// the partition that we use when computing the squared-error.
								const imageblock * blk, const error_weight_block * ewb, const processed_line3 * plines,
								// output: computed length of the partitioning's line. This is not part of the
								// error introduced by partitioning itself, but us used to estimate the error introduced by quantization
								float *length_of_lines);


float compute_error_squared_rgba(const partition_info * pt,	// the partition that we use when computing the squared-error.
								 const imageblock * blk, const error_weight_block * ewb, const processed_line4 * lines,	// one line for each of the partitions. The lines are assumed to be normalized.
								 float *length_of_lines);

float compute_error_squared_rg(const partition_info * pt,	// the partition that we use when computing the squared-error.
							   const imageblock * blk, const error_weight_block * ewb, const processed_line2 * plines, float *length_of_lines);

float compute_error_squared_rb(const partition_info * pt,	// the partition that we use when computing the squared-error.
							   const imageblock * blk, const error_weight_block * ewb, const processed_line2 * plines, float *length_of_lines);

float compute_error_squared_gb(const partition_info * pt,	// the partition that we use when computing the squared-error.
							   const imageblock * blk, const error_weight_block * ewb, const processed_line2 * plines, float *length_of_lines);

float compute_error_squared_ra(const partition_info * pt,	// the partition that we use when computing the squared-error.
							   const imageblock * blk, const error_weight_block * ewb, const processed_line2 * plines, float *length_of_lines);


// functions to compute error value across a tile for a particular line function
// for a single partition.
float compute_error_squared_rgb_single_partition(int partition_to_test, int xdim, int ydim, int zdim, const partition_info * pt,	// the partition that we use when computing the squared-error.
												 const imageblock * blk, const error_weight_block * ewb, const processed_line3 * lin	// the line for the partition.
	);



// for each partition, compute its color weightings.
void compute_partition_error_color_weightings(int xdim, int ydim, int zdim, const error_weight_block * ewb, const partition_info * pi, float4 error_weightings[4], float4 color_scalefactors[4]);



// function to find the best partitioning for a given block.

void find_best_partitionings(int partition_search_limit, int xdim, int ydim, int zdim, int partition_count, const imageblock * pb, const error_weight_block * ewb, int candidates_to_return,
							 // best partitionings to use if the endpoint colors are assumed to be uncorrelated
							 int *best_partitions_uncorrellated,
							 // best partitionings to use if the endpoint colors have the same chroma
							 int *best_partitions_samechroma,
							 // best partitionings to use if dual plane of weights are present
							 int *best_partitions_dual_weight_planes);


// use k-means clustering to compute a partition ordering for a block.
void kmeans_compute_partition_ordering(int xdim, int ydim, int zdim, int partition_count, const imageblock * blk, int *ordering);




// *********************************************************
// functions and data pertaining to images and imageblocks
// *********************************************************

struct astc_codec_image
{
	uint8_t ***imagedata8;
	uint16_t ***imagedata16;
	int xsize;
	int ysize;
	int zsize;
	int padding;
};

void destroy_image(astc_codec_image * img);
astc_codec_image *allocate_image(int bitness, int xsize, int ysize, int zsize, int padding);
void initialize_image(astc_codec_image * img);
void fill_image_padding_area(astc_codec_image * img);


extern float4 ***input_averages;
extern float4 ***input_variances;
extern float ***input_alpha_averages;


// the entries here : 0=red, 1=green, 2=blue, 3=alpha, 4=0.0, 5=1.0
struct swizzlepattern
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};



int determine_image_channels(const astc_codec_image * img);

// function to compute regional averages and variances for an image
void compute_averages_and_variances(const astc_codec_image * img, float rgb_power_to_use, float alpha_power_to_use, int avg_kernel_radius, int var_kernel_radius, swizzlepattern swz);


/*
	Functions to load image from file.
	If successful, return an astc_codec_image object.
	If unsuccessful, returns NULL.

	*result is used to return a result. In case of a successfully loaded image, bits[2:0]
	of *result indicate how many components are present, and bit[7] indicate whether
	the input image was LDR or HDR (0=LDR, 1=HDR).

	In case of failure, *result is given a negative value.
*/


astc_codec_image *load_ktx_uncompressed_image(const char *filename, int padding, int *result);
astc_codec_image *load_dds_uncompressed_image(const char *filename, int padding, int *result);
astc_codec_image *load_tga_image(const char *tga_filename, int padding, int *result);
astc_codec_image *load_image_with_stb(const char *filename, int padding, int *result);

astc_codec_image *astc_codec_load_image(const char *filename, int padding, int *result);
int astc_codec_unlink(const char *filename);

// function to store image to file
// If successful, returns the number of channels in input image
// If unsuccessful, returns a negative number.
int store_ktx_uncompressed_image(const astc_codec_image * img, const char *filename, int bitness);
int store_dds_uncompressed_image(const astc_codec_image * img, const char *filename, int bitness);
int store_tga_image(const astc_codec_image * img, const char *tga_filename, int bitness);

int astc_codec_store_image(const astc_codec_image * img, const char *filename, int bitness, const char **format_string);

int get_output_filename_enforced_bitness(const char *filename);


// compute a bunch of error metrics
void compute_error_metrics(int input_image_is_hdr, int input_components, const astc_codec_image * img1, const astc_codec_image * img2, int low_fstop, int high_fstop, int psnrmode);

// fetch an image-block from the input file
void fetch_imageblock(const astc_codec_image * img, imageblock * pb,	// picture-block to initialize with image data
					  // block dimensions
					  int xdim, int ydim, int zdim,
					  // position in picture to fetch block from
					  int xpos, int ypos, int zpos, swizzlepattern swz);


// write an image block to the output file buffer.
// the data written are taken from orig_data.
void write_imageblock(astc_codec_image * img, const imageblock * pb,	// picture-block to initialize with image data
					  // block dimensions
					  int xdim, int ydim, int zdim,
					  // position in picture to write block to.
					  int xpos, int ypos, int zpos, swizzlepattern swz);


// helper function to check whether a given picture-block has alpha that is not
// just uniformly 1.
int imageblock_uses_alpha(int xdim, int ydim, int zdim, const imageblock * pb);


float compute_imageblock_difference(int xdim, int ydim, int zdim, const imageblock * p1, const imageblock * p2, const error_weight_block * ewb);





// ***********************************************************
// functions pertaining to computing texel weights for a block
// ***********************************************************


struct endpoints
{
	int partition_count;
	float4 endpt0[4];
	float4 endpt1[4];
};


struct endpoints_and_weights
{
	endpoints ep;
	float weights[MAX_TEXELS_PER_BLOCK];
	float weight_error_scale[MAX_TEXELS_PER_BLOCK];
};


void compute_endpoints_and_ideal_weights_1_plane(int xdim, int ydim, int zdim, const partition_info * pt, const imageblock * blk, const error_weight_block * ewb, endpoints_and_weights * ei);

void compute_endpoints_and_ideal_weights_2_planes(int xdim, int ydim, int zdim, const partition_info * pt, const imageblock * blk, const error_weight_block * ewb, int separate_component,
												  endpoints_and_weights * ei1,	// for the three components of the primary plane of weights
												  endpoints_and_weights * ei2	// for the remaining component.
	);

void compute_ideal_weights_for_decimation_table(const endpoints_and_weights * eai, const decimation_table * it, float *weight_set, float *weights);

void compute_ideal_quantized_weights_for_decimation_table(const endpoints_and_weights * eai,
														  const decimation_table * it,
														  float low_bound, float high_bound, const float *weight_set_in, float *weight_set_out, uint8_t * quantized_weight_set, int quantization_level);


float compute_error_of_weight_set(const endpoints_and_weights * eai, const decimation_table * it, const float *weights);


float compute_value_of_texel_flt(int texel_to_get, const decimation_table * it, const float *weights);


int compute_value_of_texel_int(int texel_to_get, const decimation_table * it, const int *weights);


void merge_endpoints(const endpoints * ep1,	// contains three of the color components
					 const endpoints * ep2,	// contains the remaining color component
					 int separate_component, endpoints * res);

// functions dealing with color endpoints

// function to pack a pair of color endpoints into a series of integers.
// the format used may or may not match the format specified;
// the return value is the format actually used.
int pack_color_endpoints(astc_decode_mode decode_mode, float4 color0, float4 color1, float4 rgbs_color, float4 rgbo_color, float2 luminances, int format, int *output, int quantization_level);


// unpack a pair of color endpoints from a series of integers.
void unpack_color_endpoints(astc_decode_mode decode_mode, int format, int quantization_level, const int *input, int *rgb_hdr, int *alpha_hdr, int *nan_endpoint, ushort4 * output0, ushort4 * output1);


struct encoding_choice_errors
{
	float rgb_scale_error;		// error of using LDR RGB-scale instead of complete endpoints.
	float rgb_luma_error;		// error of using HDR RGB-scale instead of complete endpoints.
	float luminance_error;		// error of using luminance instead of RGB
	float alpha_drop_error;		// error of discarding alpha
	float rgb_drop_error;		// error of discarding RGB
	int can_offset_encode;
	int can_blue_contract;
};

// buffers used to store intermediate data in compress_symbolic_block_fixed_partition_*()
struct compress_fixed_partition_buffers
{
	endpoints_and_weights* ei1;
	endpoints_and_weights* ei2;
	endpoints_and_weights* eix1;
	endpoints_and_weights* eix2;
	float *decimated_quantized_weights;
	float *decimated_weights;
	float *flt_quantized_decimated_quantized_weights;
	uint8_t *u8_quantized_decimated_quantized_weights;
};

struct compress_symbolic_block_buffers
{
	error_weight_block *ewb;
	error_weight_block_orig *ewbo;
	symbolic_compressed_block *tempblocks;
	imageblock *temp;
	compress_fixed_partition_buffers *plane1;
	compress_fixed_partition_buffers *planes2;
};

void compute_encoding_choice_errors(int xdim, int ydim, int zdim, const imageblock * pb, const partition_info * pi, const error_weight_block * ewb,
									int separate_component,	// component that is separated out in 2-plane mode, -1 in 1-plane mode
									encoding_choice_errors * eci);



void determine_optimal_set_of_endpoint_formats_to_use(int xdim, int ydim, int zdim, const partition_info * pt, const imageblock * blk, const error_weight_block * ewb, const endpoints * ep,
													  int separate_component,	// separate color component for 2-plane mode; -1 for single-plane mode
													  // bitcounts and errors computed for the various quantization methods
													  const int *qwt_bitcounts, const float *qwt_errors,
													  // output data
													  int partition_format_specifiers[4][4], int quantized_weight[4], int quantization_level[4], int quantization_level_mod[4]);


void recompute_ideal_colors(int xdim, int ydim, int zdim, int weight_quantization_mode, endpoints * ep,	// contains the endpoints we wish to update
							float4 * rgbs_vectors,	// used to return RGBS-vectors for endpoint mode #6
							float4 * rgbo_vectors,	// used to return RGBS-vectors for endpoint mode #7
							float2 * lum_vectors,	// used to return luminance-vectors.
							const uint8_t * weight_set,	// the current set of weight values
							const uint8_t * plane2_weight_set,	// NULL if plane 2 is not actually used.
							int plane2_color_component,	// color component for 2nd plane of weights; -1 if the 2nd plane of weights is not present
							const partition_info * pi, const decimation_table * it, const imageblock * pb,	// picture-block containing the actual data.
							const error_weight_block * ewb);



void expand_block_artifact_suppression(int xdim, int ydim, int zdim, error_weighting_params * ewp);

// Function to set error weights for each color component for each texel in a block.
// Returns the sum of all the error values set.
float prepare_error_weight_block(const astc_codec_image * input_image,
								 // dimensions of error weight block.
								 int xdim, int ydim, int zdim, const error_weighting_params * ewp, const imageblock * blk, error_weight_block * ewb, error_weight_block_orig * ewbo);


// functions pertaining to weight alignment
void prepare_angular_tables(void);

void compute_angular_endpoints_1plane(float mode_cutoff,
									  const block_size_descriptor * bsd,
									  const float *decimated_quantized_weights, const float *decimated_weights, float low_value[MAX_WEIGHT_MODES], float high_value[MAX_WEIGHT_MODES]);

void compute_angular_endpoints_2planes(float mode_cutoff,
									   const block_size_descriptor * bsd,
									   const float *decimated_quantized_weights,
									   const float *decimated_weights,
									   float low_value1[MAX_WEIGHT_MODES], float high_value1[MAX_WEIGHT_MODES], float low_value2[MAX_WEIGHT_MODES], float high_value2[MAX_WEIGHT_MODES]);




/* *********************************** high-level encode and decode functions ************************************ */

float compress_symbolic_block(const astc_codec_image * input_image,
							  astc_decode_mode decode_mode, int xdim, int ydim, int zdim, const error_weighting_params * ewp, const imageblock * blk, symbolic_compressed_block * scb,
							  compress_symbolic_block_buffers * tmpbuf);


float4 lerp_color_flt(const float4 color0, const float4 color1, float weight,	// 0..1
					  float plane2_weight,	// 0..1
					  int plane2_color_component	// 0..3; -1 if only one plane of weights is present.
	);


ushort4 lerp_color_int(astc_decode_mode decode_mode, ushort4 color0, ushort4 color1, int weight,	// 0..64
					   int plane2_weight,	// 0..64
					   int plane2_color_component	// 0..3; -1 if only one plane of weights is present.
	);


void decompress_symbolic_block(astc_decode_mode decode_mode,
							   // dimensions of block
							   int xdim, int ydim, int zdim,
							   // position of block
							   int xpos, int ypos, int zpos, const symbolic_compressed_block * scb, imageblock * blk);


physical_compressed_block symbolic_to_physical(int xdim, int ydim, int zdim, const symbolic_compressed_block * sc);

void physical_to_symbolic(int xdim, int ydim, int zdim, physical_compressed_block pb, symbolic_compressed_block * res);


uint16_t unorm16_to_sf16(uint16_t p);
uint16_t lns_to_sf16(uint16_t p);


#endif
