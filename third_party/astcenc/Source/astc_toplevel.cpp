/*----------------------------------------------------------------------------*/
/**
 *	This confidential and proprietary software may be used only as
 *	authorised by a licensing agreement from ARM Limited
 *	(C) COPYRIGHT 2011-2013 ARM Limited
 *	ALL RIGHTS RESERVED
 *
 *	The entire notice above must be reproduced on all authorised
 *	copies and copies may only be made to the extent permitted
 *	by a licensing agreement from ARM Limited.
 *
 *	@brief	Top level functions - parsing command line, managing conversions,
 *			etc.
 *
 *			This is also where main() lives.
 */
/*----------------------------------------------------------------------------*/

#include "astc_codec_internals.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef WIN32
	#include <sys/time.h>
	#include <pthread.h>
	#include <unistd.h>

	double get_time()
	{
		timeval tv;
		gettimeofday(&tv, 0);

		return (double)tv.tv_sec + (double)tv.tv_usec * 1.0e-6;
	}


	int astc_codec_unlink(const char *filename)
	{
		return unlink(filename);
	}

#else
	// Windows.h defines IGNORE, so we must #undef our own version.
	#undef IGNORE

	// Define pthread-like functions in terms of Windows threading API
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>

	typedef HANDLE pthread_t;
	typedef int pthread_attr_t;

	int pthread_create(pthread_t * thread, const pthread_attr_t * attribs, void *(*threadfunc) (void *), void *thread_arg)
	{
		*thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE) threadfunc, thread_arg, 0, NULL);
		return 0;
	}

	int pthread_join(pthread_t thread, void **value)
	{
		WaitForSingleObject(thread, INFINITE);
		return 0;
	}

	double get_time()
	{
		FILETIME tv;
		GetSystemTimeAsFileTime(&tv);

		unsigned __int64 ticks = tv.dwHighDateTime;
		ticks = (ticks << 32) | tv.dwLowDateTime;

		return ((double)ticks) / 1.0e7;
	}

	// Define an unlink() function in terms of the Win32 DeleteFile function.
	int astc_codec_unlink(const char *filename)
	{
		BOOL res = DeleteFileA(filename);
		return (res ? 0 : -1);
	}
#endif

#ifdef DEBUG_CAPTURE_NAN
	#ifndef _GNU_SOURCE
		#define _GNU_SOURCE
	#endif

	#include <fenv.h>
#endif

// Define this to be 1 to allow "illegal" block sizes
#define DEBUG_ALLOW_ILLEGAL_BLOCK_SIZES 0

extern int block_mode_histogram[2048];

#ifdef DEBUG_PRINT_DIAGNOSTICS
	int print_diagnostics = 0;
	int diagnostics_tile = -1;
#endif

int print_tile_errors = 0;

int print_statistics = 0;

int progress_counter_divider = 1;

int rgb_force_use_of_hdr = 0;
int alpha_force_use_of_hdr = 0;


static double start_time;
static double end_time;
static double start_coding_time;
static double end_coding_time;


// code to discover the number of logical CPUs available.

#if defined(__APPLE__)
	#define _DARWIN_C_SOURCE
	#include <sys/types.h>
	#include <sys/sysctl.h>
#endif

#if defined(_WIN32) || defined(__CYGWIN__)
	#include <windows.h>
#else
	#include <unistd.h>
#endif



unsigned get_number_of_cpus(void)
{
	unsigned n_cpus = 1;

	#ifdef __linux__
		cpu_set_t mask;
		CPU_ZERO(&mask);
		sched_getaffinity(getpid(), sizeof(mask), &mask);
		n_cpus = 0;
		for (unsigned i = 0; i < CPU_SETSIZE; ++i)
		{
			if (CPU_ISSET(i, &mask))
				n_cpus++;
		}
		if (n_cpus == 0)
			n_cpus = 1;

	#elif defined (_WIN32) || defined(__CYGWIN__)
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		n_cpus = sysinfo.dwNumberOfProcessors;

	#elif defined(__APPLE__)
		int mib[4];
		size_t length = 100;
		mib[0] = CTL_HW;
		mib[1] = HW_AVAILCPU;
		sysctl(mib, 2, &n_cpus, &length, NULL, 0);
	#endif

	return n_cpus;
}

void astc_codec_internal_error(const char *filename, int linenum)
{
	printf("Internal error: File=%s Line=%d\n", filename, linenum);
	exit(1);
}

#define MAGIC_FILE_CONSTANT 0x5CA1AB13

struct astc_header
{
	uint8_t magic[4];
	uint8_t blockdim_x;
	uint8_t blockdim_y;
	uint8_t blockdim_z;
	uint8_t xsize[3];			// x-size = xsize[0] + xsize[1] + xsize[2]
	uint8_t ysize[3];			// x-size, y-size and z-size are given in texels;
	uint8_t zsize[3];			// block count is inferred
};


int suppress_progress_counter = 0;
int perform_srgb_transform = 0;

astc_codec_image *load_astc_file(const char *filename, int bitness, astc_decode_mode decode_mode, swizzlepattern swz_decode)
{
	int x, y, z;
	FILE *f = fopen(filename, "rb");
	if (!f)
	{
		printf("Failed to open file %s\n", filename);
		exit(1);
	}
	astc_header hdr;
	size_t hdr_bytes_read = fread(&hdr, 1, sizeof(astc_header), f);
	if (hdr_bytes_read != sizeof(astc_header))
	{
		fclose(f);
		printf("Failed to read file %s\n", filename);
		exit(1);
	}

	uint32_t magicval = hdr.magic[0] + 256 * (uint32_t) (hdr.magic[1]) + 65536 * (uint32_t) (hdr.magic[2]) + 16777216 * (uint32_t) (hdr.magic[3]);

	if (magicval != MAGIC_FILE_CONSTANT)
	{
		fclose(f);
		printf("File %s not recognized\n", filename);
		exit(1);
	}

	int xdim = hdr.blockdim_x;
	int ydim = hdr.blockdim_y;
	int zdim = hdr.blockdim_z;

	if ( (xdim < 3 || xdim > 6 || ydim < 3 || ydim > 6 || zdim < 3 || zdim > 6) &&
	     (xdim < 4 || xdim == 7 || xdim == 9 || xdim == 11 || xdim > 12 ||
	      ydim < 4 || ydim == 7 || ydim == 9 || ydim == 11 || ydim > 12 || zdim != 1) )
	{
		fclose(f);
		printf("File %s not recognized %d %d %d\n", filename, xdim, ydim, zdim);
		exit(1);
	}


	int xsize = hdr.xsize[0] + 256 * hdr.xsize[1] + 65536 * hdr.xsize[2];
	int ysize = hdr.ysize[0] + 256 * hdr.ysize[1] + 65536 * hdr.ysize[2];
	int zsize = hdr.zsize[0] + 256 * hdr.zsize[1] + 65536 * hdr.zsize[2];

	if (xsize == 0 || ysize == 0 || zsize == 0)
	{
		fclose(f);
		printf("File %s has zero dimension %d %d %d\n", filename, xsize, ysize, zsize);
		exit(1);
	}


	int xblocks = (xsize + xdim - 1) / xdim;
	int yblocks = (ysize + ydim - 1) / ydim;
	int zblocks = (zsize + zdim - 1) / zdim;

	uint8_t *buffer = (uint8_t *) malloc(xblocks * yblocks * zblocks * 16);
	if (!buffer)
	{
		fclose(f);
		printf("Ran out of memory\n");
		exit(1);
	}
	size_t bytes_to_read = xblocks * yblocks * zblocks * 16;
	size_t bytes_read = fread(buffer, 1, bytes_to_read, f);
	fclose(f);
	if (bytes_read != bytes_to_read)
	{
		printf("Failed to read file %s\n", filename);
		exit(1);
	}


	astc_codec_image *img = allocate_image(bitness, xsize, ysize, zsize, 0);
	initialize_image(img);

	imageblock pb;
	for (z = 0; z < zblocks; z++)
		for (y = 0; y < yblocks; y++)
			for (x = 0; x < xblocks; x++)
			{
				int offset = (((z * yblocks + y) * xblocks) + x) * 16;
				uint8_t *bp = buffer + offset;
				physical_compressed_block pcb = *(physical_compressed_block *) bp;
				symbolic_compressed_block scb;
				physical_to_symbolic(xdim, ydim, zdim, pcb, &scb);
				decompress_symbolic_block(decode_mode, xdim, ydim, zdim, x * xdim, y * ydim, z * zdim, &scb, &pb);
				write_imageblock(img, &pb, xdim, ydim, zdim, x * xdim, y * ydim, z * zdim, swz_decode);
			}

	free(buffer);

	return img;
}



struct encode_astc_image_info
{
	int xdim;
	int ydim;
	int zdim;
	const error_weighting_params *ewp;
	uint8_t *buffer;
	int *counters;
	int pack_and_unpack;
	int thread_id;
	int threadcount;
	astc_decode_mode decode_mode;
	swizzlepattern swz_encode;
	swizzlepattern swz_decode;
	int *threads_completed;
	const astc_codec_image *input_image;
	astc_codec_image *output_image;
};



void *encode_astc_image_threadfunc(void *vblk)
{
	const encode_astc_image_info *blk = (const encode_astc_image_info *)vblk;
	int xdim = blk->xdim;
	int ydim = blk->ydim;
	int zdim = blk->zdim;
	uint8_t *buffer = blk->buffer;
	const error_weighting_params *ewp = blk->ewp;
	int thread_id = blk->thread_id;
	int threadcount = blk->threadcount;
	int *counters = blk->counters;
	int pack_and_unpack = blk->pack_and_unpack;
	astc_decode_mode decode_mode = blk->decode_mode;
	swizzlepattern swz_encode = blk->swz_encode;
	swizzlepattern swz_decode = blk->swz_decode;
	int *threads_completed = blk->threads_completed;
	const astc_codec_image *input_image = blk->input_image;
	astc_codec_image *output_image = blk->output_image;

	imageblock pb;
	int ctr = thread_id;
	int pctr = 0;

	int x, y, z, i;
	int xsize = input_image->xsize;
	int ysize = input_image->ysize;
	int zsize = input_image->zsize;
	int xblocks = (xsize + xdim - 1) / xdim;
	int yblocks = (ysize + ydim - 1) / ydim;
	int zblocks = (zsize + zdim - 1) / zdim;

	int owns_progress_counter = 0;

	//allocate memory for temporary buffers
	compress_symbolic_block_buffers temp_buffers;
	temp_buffers.ewb = new error_weight_block;
	temp_buffers.ewbo = new error_weight_block_orig;
	temp_buffers.tempblocks = new symbolic_compressed_block[4];
	temp_buffers.temp = new imageblock;
	temp_buffers.planes2 = new compress_fixed_partition_buffers;
	temp_buffers.planes2->ei1 = new endpoints_and_weights;
	temp_buffers.planes2->ei2 = new endpoints_and_weights;
	temp_buffers.planes2->eix1 = new endpoints_and_weights[MAX_DECIMATION_MODES];
	temp_buffers.planes2->eix2 = new endpoints_and_weights[MAX_DECIMATION_MODES];
	temp_buffers.planes2->decimated_quantized_weights = new float[2 * MAX_DECIMATION_MODES * MAX_WEIGHTS_PER_BLOCK];
	temp_buffers.planes2->decimated_weights = new float[2 * MAX_DECIMATION_MODES * MAX_WEIGHTS_PER_BLOCK];
	temp_buffers.planes2->flt_quantized_decimated_quantized_weights = new float[2 * MAX_WEIGHT_MODES * MAX_WEIGHTS_PER_BLOCK];
	temp_buffers.planes2->u8_quantized_decimated_quantized_weights = new uint8_t[2 * MAX_WEIGHT_MODES * MAX_WEIGHTS_PER_BLOCK];
	temp_buffers.plane1 = temp_buffers.planes2;

	for (z = 0; z < zblocks; z++)
		for (y = 0; y < yblocks; y++)
			for (x = 0; x < xblocks; x++)
			{
				if (ctr == 0)
				{
					int offset = ((z * yblocks + y) * xblocks + x) * 16;
					uint8_t *bp = buffer + offset;
				#ifdef DEBUG_PRINT_DIAGNOSTICS
					if (diagnostics_tile < 0 || diagnostics_tile == pctr)
					{
						print_diagnostics = (diagnostics_tile == pctr) ? 1 : 0;
				#endif
						fetch_imageblock(input_image, &pb, xdim, ydim, zdim, x * xdim, y * ydim, z * zdim, swz_encode);
						symbolic_compressed_block scb;
						compress_symbolic_block(input_image, decode_mode, xdim, ydim, zdim, ewp, &pb, &scb, &temp_buffers);
						if (pack_and_unpack)
						{
							decompress_symbolic_block(decode_mode, xdim, ydim, zdim, x * xdim, y * ydim, z * zdim, &scb, &pb);
							write_imageblock(output_image, &pb, xdim, ydim, zdim, x * xdim, y * ydim, z * zdim, swz_decode);
						}
						else
						{
							physical_compressed_block pcb;
							pcb = symbolic_to_physical(xdim, ydim, zdim, &scb);
							*(physical_compressed_block *) bp = pcb;
						}
				#ifdef DEBUG_PRINT_DIAGNOSTICS
					}
				#endif

					counters[thread_id]++;
					ctr = threadcount - 1;

					pctr++;

					// routine to print the progress counter.
					if (suppress_progress_counter == 0 && (pctr % progress_counter_divider) == 0 && print_tile_errors == 0 && print_statistics == 0)
					{
						int do_print = 0;
						// the current thread has the responsibility for printing the progress counter
						// if every previous thread has completed. Also, if we have ever received the
						// responsibility to print the progress counter, we are going to keep it
						// until the thread is completed.
						if (!owns_progress_counter)
						{
							for (i = thread_id - 1; i >= 0; i--)
							{
								if (threads_completed[i] == 0)
								{
									do_print = 0;
									break;
								}
							}
						}
						if (do_print)
						{
							owns_progress_counter = 1;
							int summa = 0;
							for (i = 0; i < threadcount; i++)
								summa += counters[i];
							printf("\r%d", summa);
							fflush(stdout);
						}
					}
				}
				else
					ctr--;
			}

	delete[] temp_buffers.planes2->decimated_quantized_weights;
	delete[] temp_buffers.planes2->decimated_weights;
	delete[] temp_buffers.planes2->flt_quantized_decimated_quantized_weights;
	delete[] temp_buffers.planes2->u8_quantized_decimated_quantized_weights;
	delete[] temp_buffers.planes2->eix1;
	delete[] temp_buffers.planes2->eix2;
	delete   temp_buffers.planes2->ei1;
	delete   temp_buffers.planes2->ei2;
	delete   temp_buffers.planes2;
	delete[] temp_buffers.tempblocks;
	delete   temp_buffers.temp;
	delete   temp_buffers.ewbo;
	delete   temp_buffers.ewb;

	threads_completed[thread_id] = 1;
	return NULL;
}


void encode_astc_image(const astc_codec_image * input_image,
					   astc_codec_image * output_image,
					   int xdim,
					   int ydim,
					   int zdim,
					   const error_weighting_params * ewp, astc_decode_mode decode_mode, swizzlepattern swz_encode, swizzlepattern swz_decode, uint8_t * buffer, int pack_and_unpack, int threadcount)
{
	int i;
	int *counters = new int[threadcount];
	int *threads_completed = new int[threadcount];

	// before entering into the multi-threaded routine, ensure that the block size descriptors
	// and the partition table descriptors needed actually exist.
	get_block_size_descriptor(xdim, ydim, zdim);
	get_partition_table(xdim, ydim, zdim, 0);

	encode_astc_image_info *ai = new encode_astc_image_info[threadcount];
	for (i = 0; i < threadcount; i++)
	{
		ai[i].xdim = xdim;
		ai[i].ydim = ydim;
		ai[i].zdim = zdim;
		ai[i].buffer = buffer;
		ai[i].ewp = ewp;
		ai[i].counters = counters;
		ai[i].pack_and_unpack = pack_and_unpack;
		ai[i].thread_id = i;
		ai[i].threadcount = threadcount;
		ai[i].decode_mode = decode_mode;
		ai[i].swz_encode = swz_encode;
		ai[i].swz_decode = swz_decode;
		ai[i].threads_completed = threads_completed;
		ai[i].input_image = input_image;
		ai[i].output_image = output_image;
		counters[i] = 0;
		threads_completed[i] = 0;
	}

	if (threadcount == 1)
		encode_astc_image_threadfunc(&ai[0]);
	else
	{
		pthread_t *threads = new pthread_t[threadcount];
		for (i = 0; i < threadcount; i++)
			pthread_create(&(threads[i]), NULL, encode_astc_image_threadfunc, (void *)(&(ai[i])));

		for (i = 0; i < threadcount; i++)
			pthread_join(threads[i], NULL);
		delete[]threads;
	}

	delete[]ai;
	delete[]counters;
	delete[]threads_completed;
}


void store_astc_file(const astc_codec_image * input_image,
					 const char *filename, int xdim, int ydim, int zdim, const error_weighting_params * ewp, astc_decode_mode decode_mode, swizzlepattern swz_encode, int threadcount)
{
	int xsize = input_image->xsize;
	int ysize = input_image->ysize;
	int zsize = input_image->zsize;

	int xblocks = (xsize + xdim - 1) / xdim;
	int yblocks = (ysize + ydim - 1) / ydim;
	int zblocks = (zsize + zdim - 1) / zdim;

	uint8_t *buffer = (uint8_t *) malloc(xblocks * yblocks * zblocks * 16);
	if (!buffer)
	{
		printf("Ran out of memory\n");
		exit(1);
	}

	if (!suppress_progress_counter)
		printf("%d blocks to process ..\n", xblocks * yblocks * zblocks);

	encode_astc_image(input_image, NULL, xdim, ydim, zdim, ewp, decode_mode, swz_encode, swz_encode, buffer, 0, threadcount);

	end_coding_time = get_time();

	astc_header hdr;
	hdr.magic[0] = MAGIC_FILE_CONSTANT & 0xFF;
	hdr.magic[1] = (MAGIC_FILE_CONSTANT >> 8) & 0xFF;
	hdr.magic[2] = (MAGIC_FILE_CONSTANT >> 16) & 0xFF;
	hdr.magic[3] = (MAGIC_FILE_CONSTANT >> 24) & 0xFF;
	hdr.blockdim_x = xdim;
	hdr.blockdim_y = ydim;
	hdr.blockdim_z = zdim;
	hdr.xsize[0] = xsize & 0xFF;
	hdr.xsize[1] = (xsize >> 8) & 0xFF;
	hdr.xsize[2] = (xsize >> 16) & 0xFF;
	hdr.ysize[0] = ysize & 0xFF;
	hdr.ysize[1] = (ysize >> 8) & 0xFF;
	hdr.ysize[2] = (ysize >> 16) & 0xFF;
	hdr.zsize[0] = zsize & 0xFF;
	hdr.zsize[1] = (zsize >> 8) & 0xFF;
	hdr.zsize[2] = (zsize >> 16) & 0xFF;

	FILE *wf = fopen(filename, "wb");
	fwrite(&hdr, 1, sizeof(astc_header), wf);
	fwrite(buffer, 1, xblocks * yblocks * zblocks * 16, wf);
	fclose(wf);
	free(buffer);
}



astc_codec_image *pack_and_unpack_astc_image(const astc_codec_image * input_image,
											 int xdim,
											 int ydim,
											 int zdim,
											 const error_weighting_params * ewp, astc_decode_mode decode_mode, swizzlepattern swz_encode, swizzlepattern swz_decode, int bitness, int threadcount)
{
	int xsize = input_image->xsize;
	int ysize = input_image->ysize;
	int zsize = input_image->zsize;

	astc_codec_image *img = allocate_image(bitness, xsize, ysize, zsize, 0);

	/*
	   allocate_output_image_space( bitness, xsize, ysize, zsize ); */

	int xblocks = (xsize + xdim - 1) / xdim;
	int yblocks = (ysize + ydim - 1) / ydim;
	int zblocks = (zsize + zdim - 1) / zdim;

	if (!suppress_progress_counter)
		printf("%d blocks to process...\n", xblocks * yblocks * zblocks);

	encode_astc_image(input_image, img, xdim, ydim, zdim, ewp, decode_mode, swz_encode, swz_decode, NULL, 1, threadcount);

	if (!suppress_progress_counter)
		printf("\n");

	return img;
}


void find_closest_blockdim_2d(float target_bitrate, int *x, int *y, int consider_illegal)
{
	int blockdims[6] = { 4, 5, 6, 8, 10, 12 };

	float best_error = 1000;
	float aspect_of_best = 1;
	int i, j;

	// Y dimension
	for (i = 0; i < 6; i++)
	{
		// X dimension
		for (j = i; j < 6; j++)
		{
			//              NxN       MxN         8x5               10x5              10x6
			int is_legal = (j==i) || (j==i+1) || (j==3 && i==1) || (j==4 && i==1) || (j==4 && i==2);

			if(consider_illegal || is_legal)
			{
				float bitrate = 128.0f / (blockdims[i] * blockdims[j]);
				float bitrate_error = fabs(bitrate - target_bitrate);
				float aspect = (float)blockdims[j] / blockdims[i];
				if (bitrate_error < best_error || (bitrate_error == best_error && aspect < aspect_of_best))
				{
					*x = blockdims[j];
					*y = blockdims[i];
					best_error = bitrate_error;
					aspect_of_best = aspect;
				}
			}
		}
	}
}



void find_closest_blockdim_3d(float target_bitrate, int *x, int *y, int *z, int consider_illegal)
{
	int blockdims[4] = { 3, 4, 5, 6 };

	float best_error = 1000;
	float aspect_of_best = 1;
	int i, j, k;

	for (i = 0; i < 4; i++)	// Z
		for (j = i; j < 4; j++) // Y
			for (k = j; k < 4; k++) // X
			{
				//              NxNxN              MxNxN                  MxMxN
				int is_legal = ((k==j)&&(j==i)) || ((k==j+1)&&(j==i)) || ((k==j)&&(j==i+1));

				if(consider_illegal || is_legal)
				{
					float bitrate = 128.0f / (blockdims[i] * blockdims[j] * blockdims[k]);
					float bitrate_error = fabs(bitrate - target_bitrate);
					float aspect = (float)blockdims[k] / blockdims[j] + (float)blockdims[j] / blockdims[i] + (float)blockdims[k] / blockdims[i];

					if (bitrate_error < best_error || (bitrate_error == best_error && aspect < aspect_of_best))
					{
						*x = blockdims[k];
						*y = blockdims[j];
						*z = blockdims[i];
						best_error = bitrate_error;
						aspect_of_best = aspect;
					}
				}
			}
}


void compare_two_files(const char *filename1, const char *filename2, int low_fstop, int high_fstop, int psnrmode)
{
	int load_result1;
	int load_result2;
	astc_codec_image *img1 = astc_codec_load_image(filename1, 0, &load_result1);
	if (load_result1 < 0)
	{
		printf("Failed to load file %s.\n", filename1);
		exit(1);
	}
	astc_codec_image *img2 = astc_codec_load_image(filename2, 0, &load_result2);
	if (load_result2 < 0)
	{
		printf("Failed to load file %s.\n", filename2);
		exit(1);
	}

	int file1_components = load_result1 & 0x7;
	int file2_components = load_result2 & 0x7;
	int comparison_components = MAX(file1_components, file2_components);

	int compare_hdr = 0;
	if (load_result1 & 0x80)
		compare_hdr = 1;
	if (load_result2 & 0x80)
		compare_hdr = 1;

	compute_error_metrics(compare_hdr, comparison_components, img1, img2, low_fstop, high_fstop, psnrmode);
}


union if32
{
	float f;
	int32_t s;
	uint32_t u;
};


// The ASTC codec is written with the assumption that a float threaded through
// the "if32" union will in fact be stored and reloaded as a 32-bit IEEE-754 single-precision
// float, stored with round-to-nearest rounding. This is always the case in an
// IEEE-754 compliant system, however not every system is actually IEEE-754 compliant
// in the first place. As such, we run a quick test to check that this is actually the case
// (e.g. gcc on 32-bit x86 will typically fail unless -msse2 -mfpmath=sse2 is specified).

volatile float xprec_testval = 2.51f;
void test_inappropriate_extended_precision(void)
{
	if32 p;
	p.f = xprec_testval + 12582912.0f;
	float q = p.f - 12582912.0f;
	if (q != 3.0f)
	{
		printf("Single-precision test failed; please recompile with proper IEEE-754 support.\n");
		exit(1);
	}
}

// Debug routine to dump the entire image if requested.
void dump_image(astc_codec_image * img)
{
	int x, y, z, xdim, ydim, zdim;

	printf("\n\nDumping image ( %d x %d x %d + %d)...\n\n", img->xsize, img->ysize, img->zsize, img->padding);

	if (img->zsize != 1)
		zdim = img->zsize + 2 * img->padding;
	else
		zdim = img->zsize;

	ydim = img->ysize + 2 * img->padding;
	xdim = img->xsize + 2 * img->padding;

	for (z = 0; z < zdim; z++)
	{
		if (z != 0)
			printf("\n\n");
		for (y = 0; y < ydim; y++)
		{
			if (y != 0)
				printf("\n");
			for (x = 0; x < xdim; x++)
			{
				printf("  0x%08X", *(int unsigned *)&img->imagedata8[z][y][x]);
			}
		}
	}
	printf("\n\n");
}


int standalone_main(int argc, char **argv)
{
	int i;

	test_inappropriate_extended_precision();
	// initialization routines
	prepare_angular_tables();
	build_quantization_mode_table();

	start_time = get_time();

	#ifdef DEBUG_CAPTURE_NAN
		feenableexcept(FE_DIVBYZERO | FE_INVALID);
	#endif

	if (argc < 4)
	{

		printf(	"ASTC codec version 1.3\n"
				"Copyright (C) 2011-2013 ARM Limited\n"
				"All rights reserved. Use of this software is subject to terms of its license.\n\n"
				"Usage:\n"
				"Compress to texture file:\n"
				"   %s -c <inputfile> <outputfile> <rate> [options]\n"
				"Decompress from texture file:\n"
				"   %s -d <inputfile> <outputfile> [options]\n"
				"Compress, then immediately decompress to image:\n"
				"   %s -t <inputfile> <outputfile> <rate> [options]\n"
				"Compare two files (no compression or decompression):\n"
				"   %s -compare <file1> <file2> [options]\n"
				"\n"
				"When encoding/decoding a texture for use with the LDR-SRGB submode,\n"
				"use -cs, -ds, -ts instead of -c, -d, -t.\n"
				"When encoding/decoding a texture for use with the LDR-linear submode,\n"
				"use -cl, -dl, -tl instead of -c, -d, -t.\n"
				"\n"
				"For compression, the input file formats supported are\n"
				" * PNG (*.png)\n"
				" * Targa (*.tga)\n"
				" * JPEG (*.jpg)\n"
				" * GIF (*.gif) (non-animated only)\n"
				" * BMP (*.bmp)\n"
				" * Radiance HDR (*.hdr)\n"
				" * Khronos Texture KTX (*.ktx)\n"
				" * DirectDraw Surface DDS (*.dds)\n"
				" * Half-Float-TGA (*.htga)\n"
				" * OpenEXR (*.exr; only if 'exr_to_htga' is present in the path)\n"
				"\n"
				"For the KTX and DDS formats, the following subset of the format\n"
				"features are supported; the subset is:\n"
				" * 2D and 3D textures supported\n"
				" * Uncompressed only, with unorm8, unorm16, float16 or float32 components\n"
				" * R, RG, RGB, BGR, RGBA, BGRA, Luminance and Luminance-Alpha texel formats\n"
				" * In case of multiple image in one file (mipmap, cube-faces, texture-arrays)\n"
				"   the codec will read the first one and ignore the other ones.\n"
				"\n"
				"When using HDR or 3D textures, it is recommended to use the KTX or DDS formats.\n"
				"Separate 2D image slices can be assembled into a 3D image using the -array option.\n"
				"\n"
				"The output file will be an ASTC compressed texture file (recommended filename\n"
				"ending .astc)\n"
				"\n"
				"For decompression, the input file must be an ASTC compressed texture file;\n"
				"the following formats are supported for output:\n"
				" * Targa (*.tga)\n"
				" * KTX (*.ktx)\n"
				" * DDS (*.dds)\n"
				" * Half-Float-TGA (*.htga)\n"
				" * OpenEXR (*.exr; only if t'exr_to_htga' is present in the path)\n"
				"\n"
				"Targa is suitable only for 2D LDR images; for HDR and/or 3D images,\n"
				"please use KTX or DDS.\n"
				"\n"
				"For compression, the <rate> argument specifies the bitrate or block\n"
				"dimension to use. This argument can be specified in one of two ways:\n"
				" * A decimal number (at least one actual decimal needed). This will cause \n"
				"   the codec to interpret the number as a desired bitrate, and pick a block\n"
				"   size to match that bitrate as closely as possible. For example, if you want a\n"
				"   bitrate of 2.0 bits per texel, then specifiy the <rate> argument as 2.0\n"
				" * A block size. This specifies the block dimensions to use along the\n"
				"   X, Y (and for 3D textures) Z axes. The dimensions are separated with\n"
				"   the character x, with no spaces. For 2D textures, the supported\n"
				"   dimensions along each axis are picked from the set {4,5,6,8,10,12};\n"
				"   for 3D textures, the supported dimensions are picked from the\n"
				"   set {3,4,5,6}. For example, if you wish to encode a 2D texture using the\n"
				"   10x6 block size (10 texels per block along the X axis, 6 texels per block\n"
				"   along the Y axis, then specify the <rate> argument as 10x6 .\n"
				"Some examples of supported 2D block sizes are:\n"
				"  4x4 -> 8.0 bpp\n"
				"  5x5 -> 5.12 bpp\n"
				"  6x6 -> 3.56 bpp\n"
				"  8x6 -> 2.67 bpp\n"
				"  8x8 -> 2.0 bpp\n"
				" 10x8 -> 1.6 bpp\n"
				" 10x10 -> 1.28 bpp\n"
				" 10x12 -> 1.07 bpp\n"
				" 12x12 -> 0.89 bpp\n"
				"If you try to specify a bitrate that can potentially map to multiple different\n"
				"block sizes, the codec will choose the block size with the least lopsided\n"
				"aspect ratio (e.g. if you specify 2.67, then the codec will choose the\n"
				"8x6 block size, not 12x4)\n"
				"\n"
				"Below is a description of all the available options. Most of them make sense\n"
				"for encoding only, however there are some that affect decoding as well\n"
				"(such as -dsw and the normal-presets)\n"
				"\n"
				"\n"
				"Built-in error-weighting Presets:\n"
				"---------------------------------\n"
				"The presets provide easy-to-use combinations of encoding options that\n"
				"are designed for use with certain commonly-occurring kinds of\n"
				"textures.\n"
				"\n"
				" -normal_psnr\n"
				"      For encoding, assume that the input texture is a normal map with the\n"
				"      X and Y components of the actual normals in the Red and Green\n"
				"      color channels. The codec will then move the 2nd component to Alpha,\n"
				"      and apply an error-weighting function based on angular error.\n"
				"\n"
				"      It is possible to use this preset with texture decoding as well,\n"
				"      in which case it will expand the normal map from 2 to 3 components\n"
				"      after the actual decoding.\n"
				"\n"
				"      The -normal_psnr preset as a whole is equivalent to the options\n"
				"      \"-rn -esw rrrg -dsw raz1 -ch 1 0 0 1 -oplimit 1000 -mincorrel 0.99\" .\n"
				"\n"
				" -normal_percep\n"
				"      Similar to -normal_psnr, except that it tries to optimize the normal\n"
				"      map for best possible perceptual results instead of just maximizing\n"
				"      angular PSNR.\n"
				"      The -normal_percep preset as a whole is equivalent to the options\n"
				"      \"-normal_psnr -b 2.5 -v 3 1 1 0 50 0 -va 1 1 0 50 -dblimit 60\" .\n"
				"\n"
				" -mask\n"
				"      Assume that the input texture is a texture that contains\n"
				"      unrelated content in its various color channels, and where\n"
				"      it is undesirable for errors in one channel to affect\n"
				"      the other channels.\n"
				"      Equivalent to \"-v 3 1 1 0 25 0.03 -va 0 25\" .\n"
				"\n"
				" -alphablend\n"
				"      Assume that the input texture is an RGB-alpha texture where\n"
				"      the alpha component is used to represent opacity.\n"
				"      (0=fully transparent, 1=fully opaque)\n"
				"      Equivalent to \"-a 1\" .\n"
				"\n"
				" -hdr\n"
				"      Assume that the input texture is an HDR texture. If an alpha channel is\n"
				"      present, it is treated as an LDR channel (e.g. opacity)\n"
				"      Optimize for 4th-root error for the color and linear error for the alpha.\n"
				"      Equivalent to\n"
				"          \"-forcehdr_rgb -v 0 0.75 0 1 0 0 -va 0.02 1 0 0 -dblimit 999\"\n"
				"\n"
				" -hdra\n"
				"      Assume that the input texture is an HDR texture, and optimize\n"
				"      for 4th-root error. If an alpha channel is present, it is\n"
				"      assumed to be HDR and optimized for 4th-root error as well.\n"
				"      Equivalent to\n"
				"          \"-forcehdr_rgba -v 0 0.75 0 1 0 0 -va 0.75 0 1 0 -dblimit 999\"\n"
				"\n"
				" -hdr_log\n"
				" -hdra_log\n"
				"      Assume that the input texture is an HDR texture, and optimize\n"
				"      for logarithmic error. This should give better results than -hdr\n"
				"      on metrics like \"logRMSE\" and \"mPSNR\", but the subjective\n"
				"      quality (in particular block artifacts) is generally significantly worse\n"
				"      than -hdr.\n"
				"      \"-hdr_log\" is equivalent to\n"
				"          \"-forcehdr_rgb -v 0 1 0 1 0 0 -va 0.02 1 0 0 -dblimit 999\"\n"
				"      \"-hdra_log\" is equivalent to\n"
				"          \"-forcehdr_rgba -v 0 1 0 1 0 0 -va 1 0 1 0 -dblimit 999\"\n"
				"\n"
				"\n"
				"\n"
				"Performance-quality tradeoff presets:\n"
				"-------------------------------------\n"
				"These are presets that provide different tradeoffs between encoding\n"
				"performance and quality. Exactly one of these presets has to be specified\n"
				"for encoding; if this is not done, the codec reports an error message.\n"
				"\n"
				" -veryfast\n"
				"      Run codec in very-fast-mode; this generally results in substantial\n"
				"      quality loss.\n"
				"\n"
				" -fast\n"
				"      Run codec in fast-mode. This generally results in mild quality loss.\n"
				"\n"
				" -medium\n"
				"      Run codec in medium-speed-mode.\n"
				"\n"
				" -thorough\n"
				"     Run codec in thorough-mode. This should be sufficient to fix most\n"
				"     cases where \"-medium\" provides inadequate quality.\n"
				"\n"
				" -exhaustive\n"
				"      Run codec in exhaustive-mode. This usually produces only\n"
				"      marginally better quality than \"-thorough\" while considerably\n"
				"      increasing encode time.\n"
				"\n"
				"\n"
				"Low-level error weighting options:\n"
				"----------------------------------\n"
				"These options provide low-level control of the error-weighting options\n"
				"that the codec provides.\n"
				"\n"
				" -v <radius> <power> <baseweight> <avgscale> <stdevscale> <mixing-factor>\n"
				"      Compute the per-texel relative error weighting for the RGB color\n"
				"      channels as follows:\n"
				"\n"
				"       weight = 1 / (<baseweight> + <avgscale>\n"
				"            * average^2 + <stdevscale> * stdev^2)\n"
				"\n"
				"      The average and stdev are computed as the average-value and the\n"
				"      standard deviation across a neighborhood of each texel; the <radius>\n"
				"      argument specifies how wide this neighborhood should be.\n"
				"      If this option is given without -va, it affects the weighting of RGB\n"
				"      color components only, while alpha is assigned the weight 1.0 .\n"
				"\n"
				"      The <mixing-factor> parameter is used to control the degree of mixing\n"
				"      between color channels. Setting this parameter to 0 causes the average\n"
				"      and stdev computation to be done completely separately for each color\n"
				"      channel; setting it to 1 causes the results from the red, green and\n"
				"      blue color channel to be combined into a single result that is applied\n"
				"      to all three channels. It is possible to set the mixing factor\n"
				"      to a value between 0 and 1 in order to obtain a result in-between.\n"
				"\n"
				"      The <power> argument is a power used to raise the values of the input\n"
				"      pixels before computing average and stdev; e.g. a power of 0.5 causes\n"
				"      the codec to take the square root of every input pixel value before\n"
				"      computing the averages and standard deviations.\n"
				"\n"
				" -va <baseweight> <power> <avgscale> <stdevscale>\n"
				"      Used together with -v; it computes a relative per-texel\n"
				"      weighting for the alpha component based on average and standard\n"
				"      deviation in the same manner as described for -v, but with its own\n"
				"      <baseweight>, <power>, <avgscale> and <stdevscale> parameters.\n"
				"\n"
				" -a <radius>\n"
				"      For textures with alpha channel, scale per-texel weights by\n"
				"      alpha. The alpha value chosen for scaling of any particular texel\n"
				"      is taken as an average across a neighborhood of the texel.\n"
				"      The <radius> argument gives the radius of this neighborhood;\n"
				"      a radius of 0 causes the texel's own alpha value to be used with\n"
				"      no contribution from neighboring texels.\n"
				"\n"
				" -ch <red_weight> <green_weight> <blue_weight> <alpha_weight>\n"
				"      Assign relative weight to each color channel.\n"
				"      If this option is combined with any of the other options above,\n"
				"      the other options are used to compute a weighting, then the \n"
				"      weigthing is multiplied by the weighting provided by this argument.\n"
				"\n"
				" -rn\n"
				"      Assume that the red and alpha color channels (after swizzle)\n"
				"      represent the X and Y components for a normal map,\n"
				"      and scale the error weighting so as to match angular error as closely\n"
				"      as possible. The reconstruction function for the Z component\n"
				"      is assumed to be Z=sqrt(1 - X^2 - X^2).\n"
				"\n"
				" -b <weighting>\n"
				"      Increase error weight for texels at compression-block edges\n"
				"      and corners; the parameter specifies how much the weights are to be\n"
				"      modified, with 0 giving no modification. Higher values should reduce\n"
				"      block-artifacts, at the cost of worsening other artifacts.\n"
				"\n"
				"\n"
				"Low-level performance-quality tradeoff options:\n"
				"-----------------------------------------------\n"
				"These options provide low-level control of the performance-quality tradeoffs\n"
				"that the codec provides.\n"
				"\n"
				" -plimit <number>\n"
				"      Test only <number> different partitionings. Higher numbers give better\n"
				"      quality at the expense of longer encode time; however large values tend\n"
				"      to give diminishing returns. This parameter can be set to a\n"
				"      number from 1 to %d. By default, this limit is set based on the active\n"
				"      preset, as follows:\n"
				"        -veryfast :  2\n"
				"        -fast     :  4\n"
				"        -medium   :  25\n"
				"        -thorough :  100\n"
				"        -exhaustive  : %d\n"
				"\n"
				" -dblimit <number>\n"
				"      Stop compression work on a block as soon as the PSNR of the block,\n"
				"      as measured in dB, exceeds this limit. Higher numbers give better\n"
				"      quality at the expense of longer encode times. If not set explicitly,\n"
				"      it is set based on the currently-active block size and preset, as listed\n"
				"      below (where N is the number of texels per block):\n"
				"\n"
				"        -veryfast : dblimit = MAX( 53-19*log10(N), 70-35*log10(N) )\n"
				"        -fast     : dblimit = MAX( 63-19*log10(N), 85-35*log10(N) )\n"
				"        -medium   : dblimit = MAX( 70-19*log10(N), 95-35*log10(N) )\n"
				"        -thorough   : dblimit = MAX( 77-19*log10(N), 105-35*log10(N) )\n"
				"        -exhaustive : dblimit = 999\n"
				"\n"
				"      Note that the compressor is not actually guaranteed to reach these PSNR\n"
				"      numbers for any given block; also, at the point where the compressor\n"
				"      discovers that it has exceeded the dblimit, it may have exceeded it by\n"
				"      a large amount, so it is still possible to get a PSNR value that is\n"
				"      substantially higher than the dblimit would suggest.\n"
				"\n"
				"      This option is ineffective for HDR textures.\n"
				"\n"
				" -oplimit <factor>\n"
				"      If the error term from encoding with 2 partitions is greater than the\n"
				"      error term from encoding with 1 partition by more than the specified\n"
				"      factor, then cut compression work short.\n"
				"      By default, this factor is set based on the active preset, as follows:\n"
				"        -veryfast : 1.0\n"
				"        -fast     : 1.0\n"
				"        -medium   : 1.2\n"
				"        -thorough : 2.5\n"
				"        -exhaustive  : 1000\n"
				"      The codec will not apply this factor if the input texture is a normal\n"
				"      map (content resembles a normal-map, or one of the -normal_* presets\n"
				"      is used).\n"
				"\n"
				" -mincorrel <value>\n"
				"      For each block, the codec will compute the correlation coefficients\n"
				"      between any two color components; if no pair of colors have a\n"
				"      correlation coefficient below the cutoff specified by this switch,\n"
				"      the codec will abstain from trying the dual-weight-planes.\n"
				"      By default, this factor is set based on the active preset, as follows:\n"
				"        -veryfast : 0.5\n"
				"        -fast     : 0.5\n"
				"        -medium   : 0.75\n"
				"        -thorough : 0.95\n"
				"        -exhaustive  : 0.99\n"
				"      If the input texture is a normal-map (content resembles a normal-map\n"
				"      or one of the -normal_* presets are used) the codec will use a value\n"
				"      of 0.99.\n"
				"\n"
				" -bmc <value>\n"
				"      Cutoff on the set of block modes to use; the cutoff is a percentile\n"
				"      of the block modes that are most commonly used. The value takes a value\n"
				"      from 0 to 100, where 0 offers the highest speed and lowest quality,\n"
				"      and 100 offers the highest quality and lowest speed.\n"
				"      By default, this factor is set based on the active preset, as follows:\n"
				"       -veryfast  : 25\n"
				"       -fast      : 50\n"
				"       -medium    : 75\n"
				"       -thorough  : 95\n"
				"       -exhaustive : 100\n"
				"      This option is ineffective for 3D textures.\n"
				"\n"
				" -maxiters <value>\n"
				"      Maximum number of refinement iterations to apply to colors and weights.\n"
				"      Minimum value is 1; larger values give slight quality increase\n"
				"      at expense of mild performance loss. By default, the iteration count is\n"
				"      picked based on the active preset, as follows:\n"
				"       -veryfast  : 1\n"
				"       -fast      : 1\n"
				"       -medium    : 2\n"
				"       -thorough  : 4\n"
				"       -exhaustive : 4\n"
				"\n"
				"\n"
				"\n"
				"Other options:\n"
				"--------------\n"
				"\n"
				" -array <size>\n"
				"      Loads a an array of 2D image slices as a 3D image. The filename given\n"
				"      is used as a base, and decorated with _0, _1, up to _<size-1> prior\n"
				"      to loading each slice. So -array 3 input.png would load input_0.png,\n"
				"      input_1.png and input_2.png as slices at z=0,1,2 respectively.\n"
				"\n"
				" -forcehdr_rgb\n"
				"      Force the use of HDR endpoint modes. By default, only LDR endpoint\n"
				"      modes are used. If alpha is present, alpha is kept as LDR.\n"
				" -forcehdr_rgba\n"
				"      Force the use of HDR endpoint modes. By default, only LDR endpoint\n"
				"      modes are used. If alpha is present, alpha is forced into HDR as well.\n"
				"\n"
				" -esw <swizzlepattern>\n"
				"      Swizzle the color components before encoding. The swizzle pattern\n"
				"      is specified as a 4-character string, where the characters specify\n"
				"      which color component will end up in the Red, Green, Blue and Alpha\n"
				"      channels before encoding takes place. The characters may be taken\n"
				"      from the set (r,g,b,a,0,1), where r,g,b,a use color components from\n"
				"      the input texture and 0,1 use the constant values 0 and 1.\n"
				"\n"
				"      As an example, if you have an input RGBA texture where you wish to\n"
				"      switch around the R and G channels, as well as replacing the\n"
				"      alpha channel with the constant value 1, a suitable swizzle\n"
				"      option would be:\n"
				"        -esw grb1\n"
				"      Note that if -esw is used together with any of the\n"
				"      error weighting functions, the swizzle is considered to be\n"
				"      applied before the error weighting function.\n"
				"\n"
				" -dsw <swizzlepattern>\n"
				"      Swizzle pattern to apply after decoding a texture. This pattern is\n"
				"      specified in the samw way as the pre-encoding swizzle pattern\n"
				"      for the -sw switch. However, one additional character is supported,\n"
				"      namely 'z' for constructing the third component of a normal map.\n"
				"\n"
				" -srgb\n"
				"      Convert input image from sRGB to linear-RGB before encode; convert\n"
				"      output image from linear-RGB to sRGB after decode. For encode, the\n"
				"      transform is applied after swizzle; for decode, the transform\n"
				"      is applied before swizzle.\n"
				"\n"
				" -j <numthreads>\n"
				"      Run encoding with multithreading, using the specified number\n"
				"      of threads. If not specified, the codec will autodetect the\n"
				"      number of available logical CPUs and spawn one thread for each.\n"
				"      Use \"-j 1\" if you wish to run the codec in single-thread mode.\n"
				"\n"
				" -silentmode\n"
				"      Suppresses all output from the codec, except in case of errors.\n"
				"      If this switch is not provided, the codec will display the encoding\n"
				"      settings it uses and show a progress counter during encode.\n"
				"\n"
				" -time\n"
				"      Displays time taken for entire run, together with time taken for\n"
				"      coding step only. If requested, this is output even in -silentmode.\n"
				"\n"
				" -showpsnr\n"
				"      In test mode (-t), displays PSNR difference between input and output\n"
				"      images, in dB, even if -silentmode is specified. Works for LDR images\n"
				"      only.\n"
				"\n"
				" -mpsnr <low> <high>\n"
				"     Set the low and high f-stop values to use for the mPSNR error metric.\n"
				"     Default is low=-10, high=10.\n"
				"     The mPSNR error metric only applies to HDR textures.\n"
				"     This option can be used together with -compare .\n"
				"\n"
				"\n"
				"\n"
				"Tips & tricks:\n"
				"--------------"
				"\n"
				"ASTC, being a block-based format, is moderately prone to block artifacts.\n"
				"If block artifacts are a problem when compressing a given texture,\n"
				"adding some or all of following command-line options may help:\n"
				" -b 1.8\n"
				" -v 2 1 1 0 25 0.1\n"
				" -va 1 1 0 25\n"
				" -dblimit 60\n"
				"The -b option is a general-purpose block-artifact reduction option. The\n"
				"-v and -va options concentrate effort where smooth regions lie next to regions\n"
				"with high detail (such regions are particularly prone to block artifacts\n"
				"otherwise). The -dblimit option is sometimes also needed to reduce\n"
				"block artifacts in regions with very smooth gradients.\n"
				"\n"
				"If a texture exhibits severe block artifacts in only some, but not all, of\n"
				"the color channels (common problem with mask textures), then it may help\n"
				"to use the -ch option to raise the weighting of the affected color channel(s).\n"
				"For example, if the green color channel in particular suffers from block\n"
				"artifacts, then using the commandline option\n"
				" -ch 1 6 1 1\n"
				"should improve the result significantly.\n"

			   , argv[0], argv[0], argv[0], argv[0], PARTITION_COUNT, PARTITION_COUNT);

		exit(1);
	}


	astc_decode_mode decode_mode = DECODE_HDR;
	int opmode;					// 0=compress, 1=decompress, 2=do both, 4=compare
	if (!strcmp(argv[1], "-c"))
	{
		opmode = 0;
		decode_mode = DECODE_HDR;
	}
	else if (!strcmp(argv[1], "-d"))
	{
		opmode = 1;
		decode_mode = DECODE_HDR;
	}
	else if (!strcmp(argv[1], "-t"))
	{
		opmode = 2;
		decode_mode = DECODE_HDR;
	}
	else if (!strcmp(argv[1], "-cs"))
	{
		opmode = 0;
		decode_mode = DECODE_LDR_SRGB;
	}
	else if (!strcmp(argv[1], "-ds"))
	{
		opmode = 1;
		decode_mode = DECODE_LDR_SRGB;
	}
	else if (!strcmp(argv[1], "-ts"))
	{
		opmode = 2;
		decode_mode = DECODE_LDR_SRGB;
	}
	else if (!strcmp(argv[1], "-cl"))
	{
		opmode = 0;
		decode_mode = DECODE_LDR;
	}
	else if (!strcmp(argv[1], "-dl"))
	{
		opmode = 1;
		decode_mode = DECODE_LDR;
	}
	else if (!strcmp(argv[1], "-tl"))
	{
		opmode = 2;
		decode_mode = DECODE_LDR;
	}
	else if (!strcmp(argv[1], "-compare"))
	{
		opmode = 4;
		decode_mode = DECODE_HDR;
	}
	else
	{
		printf("Unrecognized operation\n");
		exit(1);
	}

	int array_size = 1;

	const char *input_filename = argv[2];
	const char *output_filename = argv[3];

	int silentmode = 0;
	int timemode = 0;
	int psnrmode = 0;

	error_weighting_params ewp;

	ewp.rgb_power = 1.0f;
	ewp.alpha_power = 1.0f;
	ewp.rgb_base_weight = 1.0f;
	ewp.alpha_base_weight = 1.0f;
	ewp.rgb_mean_weight = 0.0f;
	ewp.rgb_stdev_weight = 0.0f;
	ewp.alpha_mean_weight = 0.0f;
	ewp.alpha_stdev_weight = 0.0f;

	ewp.rgb_mean_and_stdev_mixing = 0.0f;
	ewp.mean_stdev_radius = 0;
	ewp.enable_rgb_scale_with_alpha = 0;
	ewp.alpha_radius = 0;

	ewp.block_artifact_suppression = 0.0f;
	ewp.rgba_weights[0] = 1.0f;
	ewp.rgba_weights[1] = 1.0f;
	ewp.rgba_weights[2] = 1.0f;
	ewp.rgba_weights[3] = 1.0f;
	ewp.ra_normal_angular_scale = 0;

	swizzlepattern swz_encode = { 0, 1, 2, 3 };
	swizzlepattern swz_decode = { 0, 1, 2, 3 };


	int thread_count = 0;		// default value
	int thread_count_autodetected = 0;

	int preset_has_been_set = 0;

	int plimit_autoset = -1;
	int plimit_user_specified = -1;
	int plimit_set_by_user = 0;

	float dblimit_autoset_2d = 0.0;
	float dblimit_autoset_3d = 0.0;
	float dblimit_user_specified = 0.0;
	int dblimit_set_by_user = 0;

	float oplimit_autoset = 0.0;
	float oplimit_user_specified = 0.0;
	int oplimit_set_by_user = 0;

	float mincorrel_autoset = 0.0;
	float mincorrel_user_specified = 0.0;
	int mincorrel_set_by_user = 0;

	float bmc_user_specified = 0.0;
	float bmc_autoset = 0.0;
	int bmc_set_by_user = 0;

	int maxiters_user_specified = 0;
	int maxiters_autoset = 0;
	int maxiters_set_by_user = 0;

	int pcdiv = 1;

	int xdim_2d = 0;
	int ydim_2d = 0;
	int xdim_3d = 0;
	int ydim_3d = 0;
	int zdim_3d = 0;

	int target_bitrate_set = 0;
	float target_bitrate = 0;

	int print_block_mode_histogram = 0;

	float log10_texels_2d = 0.0f;
	float log10_texels_3d = 0.0f;

	int low_fstop = -10;
	int high_fstop = 10;


	// parse the command line's encoding options.
	int argidx;
	if (opmode == 0 || opmode == 2)
	{
		if (argc < 5)
		{
			printf("Cannot encode without specifying blocksize\n");
			exit(1);
		}

		if (strchr(argv[4], '.') != NULL)
		{
			target_bitrate = static_cast < float >(atof(argv[4]));
			target_bitrate_set = 1;
			find_closest_blockdim_2d(target_bitrate, &xdim_2d, &ydim_2d, DEBUG_ALLOW_ILLEGAL_BLOCK_SIZES);
			find_closest_blockdim_3d(target_bitrate, &xdim_3d, &ydim_3d, &zdim_3d, DEBUG_ALLOW_ILLEGAL_BLOCK_SIZES);
		}

		else
		{
			int dimensions = sscanf(argv[4], "%dx%dx%d", &xdim_3d, &ydim_3d, &zdim_3d);
			switch (dimensions)
			{
			case 0:
			case 1:
				// failed to parse the blocksize argument at all.
				printf("Blocksize not specified\n");
				exit(1);
			case 2:
				{
					zdim_3d = 1;

					// Check 2D constraints
					if(!(xdim_3d ==4 || xdim_3d == 5 || xdim_3d == 6 || xdim_3d == 8 || xdim_3d == 10 || xdim_3d == 12) ||
					   !(ydim_3d ==4 || ydim_3d == 5 || ydim_3d == 6 || ydim_3d == 8 || ydim_3d == 10 || ydim_3d == 12) )
					{
						printf("Block dimensions %d x %d unsupported\n", xdim_3d, ydim_3d);
						exit(1);
					}

					int is_legal_2d = (xdim_3d==ydim_3d) || (xdim_3d==ydim_3d+1) || ((xdim_3d==ydim_3d+2) && !(xdim_3d==6 && ydim_3d==4)) ||
									  (xdim_3d==8 && ydim_3d==5) || (xdim_3d==10 && ydim_3d==5) || (xdim_3d==10 && ydim_3d==6);

					if(!DEBUG_ALLOW_ILLEGAL_BLOCK_SIZES && !is_legal_2d)
					{
						printf("Block dimensions %d x %d disallowed\n", xdim_3d, ydim_3d);
						exit(1);
					}
				}
				break;

			default:
				{
					// Check 3D constraints
					if(xdim_3d < 3 || xdim_3d > 6 || ydim_3d < 3 || ydim_3d > 6 || zdim_3d < 3 || zdim_3d > 6)
					{
						printf("Block dimensions %d x %d x %d unsupported\n", xdim_3d, ydim_3d, zdim_3d);
						exit(1);
					}

					int is_legal_3d = ((xdim_3d==ydim_3d)&&(ydim_3d==zdim_3d)) || ((xdim_3d==ydim_3d+1)&&(ydim_3d==zdim_3d)) || ((xdim_3d==ydim_3d)&&(ydim_3d==zdim_3d+1));

					if(!DEBUG_ALLOW_ILLEGAL_BLOCK_SIZES && !is_legal_3d)
					{
						printf("Block dimensions %d x %d x %d disallowed\n", xdim_3d, ydim_3d, zdim_3d);
						exit(1);
					}
				}
				break;
			}

			xdim_2d = xdim_3d;
			ydim_2d = ydim_3d;
		}

		log10_texels_2d = log((float)(xdim_2d * ydim_2d)) / log(10.0f);
		log10_texels_3d = log((float)(xdim_3d * ydim_3d * zdim_3d)) / log(10.0f);
		argidx = 5;
	}
	else
	{
		// for decode and comparison, block size is not needed.
		argidx = 4;
	}


	while (argidx < argc)
	{
		if (!strcmp(argv[argidx], "-silentmode"))
		{
			argidx++;
			silentmode = 1;
			suppress_progress_counter = 1;
		}
		else if (!strcmp(argv[argidx], "-time"))
		{
			argidx++;
			timemode = 1;
		}
		else if (!strcmp(argv[argidx], "-showpsnr"))
		{
			argidx++;
			psnrmode = 1;
		}
		else if (!strcmp(argv[argidx], "-v"))
		{
			argidx += 7;
			if (argidx > argc)
			{
				printf("-v switch with less than 6 arguments, quitting\n");
				exit(1);
			}
			ewp.mean_stdev_radius = atoi(argv[argidx - 6]);
			ewp.rgb_power = static_cast < float >(atof(argv[argidx - 5]));
			ewp.rgb_base_weight = static_cast < float >(atof(argv[argidx - 4]));
			ewp.rgb_mean_weight = static_cast < float >(atof(argv[argidx - 3]));
			ewp.rgb_stdev_weight = static_cast < float >(atof(argv[argidx - 2]));
			ewp.rgb_mean_and_stdev_mixing = static_cast < float >(atof(argv[argidx - 1]));
		}
		else if (!strcmp(argv[argidx], "-va"))
		{
			argidx += 5;
			if (argidx > argc)
			{
				printf("-va switch with less than 4 arguments, quitting\n");
				exit(1);
			}
			ewp.alpha_power = static_cast < float >(atof(argv[argidx - 5]));
			ewp.alpha_base_weight = static_cast < float >(atof(argv[argidx - 3]));
			ewp.alpha_mean_weight = static_cast < float >(atof(argv[argidx - 2]));
			ewp.alpha_stdev_weight = static_cast < float >(atof(argv[argidx - 1]));
		}
		else if (!strcmp(argv[argidx], "-ch"))
		{
			argidx += 5;
			if (argidx > argc)
			{
				printf("-ch switch with less than 4 arguments\n");
				exit(1);
			}
			ewp.rgba_weights[0] = static_cast < float >(atof(argv[argidx - 4]));
			ewp.rgba_weights[1] = static_cast < float >(atof(argv[argidx - 3]));
			ewp.rgba_weights[2] = static_cast < float >(atof(argv[argidx - 2]));
			ewp.rgba_weights[3] = static_cast < float >(atof(argv[argidx - 1]));
		}
		else if (!strcmp(argv[argidx], "-a"))
		{
			argidx += 2;
			if (argidx > argc)
			{
				printf("-a switch with no argument\n");
				exit(1);
			}
			ewp.enable_rgb_scale_with_alpha = 1;
			ewp.alpha_radius = atoi(argv[argidx - 1]);
		}
		else if (!strcmp(argv[argidx], "-rn"))
		{
			argidx++;
			ewp.ra_normal_angular_scale = 1;
		}

		else if (!strcmp(argv[argidx], "-b"))
		{
			argidx += 2;
			if (argidx > argc)
			{
				printf("-b switch with no argument\n");
				exit(1);
			}
			ewp.block_artifact_suppression = static_cast < float >(atof(argv[argidx - 1]));
		}

		else if (!strcmp(argv[argidx], "-esw"))
		{
			argidx += 2;
			if (argidx > argc)
			{
				printf("-esw switch with no argument\n");
				exit(1);
			}
			if (strlen(argv[argidx - 1]) != 4)
			{
				printf("Swizzle pattern for the -esw switch must have exactly 4 characters\n");
				exit(1);
			}
			int swizzle_components[4];
			for (i = 0; i < 4; i++)
				switch (argv[argidx - 1][i])
				{
				case 'r':
					swizzle_components[i] = 0;
					break;
				case 'g':
					swizzle_components[i] = 1;
					break;
				case 'b':
					swizzle_components[i] = 2;
					break;
				case 'a':
					swizzle_components[i] = 3;
					break;
				case '0':
					swizzle_components[i] = 4;
					break;
				case '1':
					swizzle_components[i] = 5;
					break;
				default:
					printf("Character '%c' is not a valid swizzle-character\n", argv[argidx - 1][i]);
					exit(1);
				}
			swz_encode.r = swizzle_components[0];
			swz_encode.g = swizzle_components[1];
			swz_encode.b = swizzle_components[2];
			swz_encode.a = swizzle_components[3];
		}

		else if (!strcmp(argv[argidx], "-dsw"))
		{
			argidx += 2;
			if (argidx > argc)
			{
				printf("-dsw switch with no argument\n");
				exit(1);
			}
			if (strlen(argv[argidx - 1]) != 4)
			{
				printf("Swizzle pattern for the -dsw switch must have exactly 4 characters\n");
				exit(1);
			}
			int swizzle_components[4];
			for (i = 0; i < 4; i++)
				switch (argv[argidx - 1][i])
				{
				case 'r':
					swizzle_components[i] = 0;
					break;
				case 'g':
					swizzle_components[i] = 1;
					break;
				case 'b':
					swizzle_components[i] = 2;
					break;
				case 'a':
					swizzle_components[i] = 3;
					break;
				case '0':
					swizzle_components[i] = 4;
					break;
				case '1':
					swizzle_components[i] = 5;
					break;
				case 'z':
					swizzle_components[i] = 6;
					break;
				default:
					printf("Character '%c' is not a valid swizzle-character\n", argv[argidx - 1][i]);
					exit(1);
				}
			swz_decode.r = swizzle_components[0];
			swz_decode.g = swizzle_components[1];
			swz_decode.b = swizzle_components[2];
			swz_decode.a = swizzle_components[3];
		}


		// presets begin here
		else if (!strcmp(argv[argidx], "-normal_psnr"))
		{
			argidx++;
			ewp.rgba_weights[0] = 1.0f;
			ewp.rgba_weights[1] = 0.0f;
			ewp.rgba_weights[2] = 0.0f;
			ewp.rgba_weights[3] = 1.0f;
			ewp.ra_normal_angular_scale = 1;
			swz_encode.r = 0;	// r <- red
			swz_encode.g = 0;	// g <- red
			swz_encode.b = 0;	// b <- red
			swz_encode.a = 1;	// a <- green
			swz_decode.r = 0;	// r <- red
			swz_decode.g = 3;	// g <- alpha
			swz_decode.b = 6;	// b <- reconstruct
			swz_decode.a = 5;	// 1.0

			oplimit_user_specified = 1000.0f;
			oplimit_set_by_user = 1;
			mincorrel_user_specified = 0.99f;
			mincorrel_set_by_user = 1;
		}

		else if (!strcmp(argv[argidx], "-normal_percep"))
		{
			argidx++;
			ewp.rgba_weights[0] = 1.0f;
			ewp.rgba_weights[1] = 0.0f;
			ewp.rgba_weights[2] = 0.0f;
			ewp.rgba_weights[3] = 1.0f;
			ewp.ra_normal_angular_scale = 1;
			swz_encode.r = 0;	// r <- red
			swz_encode.g = 0;	// g <- red
			swz_encode.b = 0;	// b <- red
			swz_encode.a = 1;	// a <- green
			swz_decode.r = 0;	// r <- red
			swz_decode.g = 3;	// g <- alpha
			swz_decode.b = 6;	// b <- reconstruct
			swz_decode.a = 5;	// 1.0

			oplimit_user_specified = 1000.0f;
			oplimit_set_by_user = 1;
			mincorrel_user_specified = 0.99f;
			mincorrel_set_by_user = 1;

			dblimit_user_specified = 999;
			dblimit_set_by_user = 1;

			ewp.block_artifact_suppression = 1.8f;
			ewp.mean_stdev_radius = 3;
			ewp.rgb_mean_weight = 0;
			ewp.rgb_stdev_weight = 50;
			ewp.rgb_mean_and_stdev_mixing = 0.0;
			ewp.alpha_mean_weight = 0;
			ewp.alpha_stdev_weight = 50;
		}


		else if (!strcmp(argv[argidx], "-mask"))
		{
			argidx++;
			ewp.mean_stdev_radius = 3;
			ewp.rgb_mean_weight = 0.0f;
			ewp.rgb_stdev_weight = 25.0f;
			ewp.rgb_mean_and_stdev_mixing = 0.03f;
			ewp.alpha_mean_weight = 0.0f;
			ewp.alpha_stdev_weight = 25.0f;
		}

		else if (!strcmp(argv[argidx], "-alphablend"))
		{
			argidx++;
			ewp.enable_rgb_scale_with_alpha = 1;
			ewp.alpha_radius = 1;
		}

		else if (!strcmp(argv[argidx], "-hdra"))
		{
			if(decode_mode != DECODE_HDR)
			{
				printf("The option -hdra is only available in HDR mode\n");
				exit(1);
			}

			argidx++;
			ewp.mean_stdev_radius = 0;
			ewp.rgb_power = 0.75;
			ewp.rgb_base_weight = 0;
			ewp.rgb_mean_weight = 1;
			ewp.alpha_power = 0.75;
			ewp.alpha_base_weight = 0;
			ewp.alpha_mean_weight = 1;
			rgb_force_use_of_hdr = 1;
			alpha_force_use_of_hdr = 1;
			dblimit_user_specified = 999;
			dblimit_set_by_user = 1;
		}

		else if (!strcmp(argv[argidx], "-hdr"))
		{
			if(decode_mode != DECODE_HDR)
			{
				printf("The option -hdr is only available in HDR mode\n");
				exit(1);
			}

			argidx++;
			ewp.mean_stdev_radius = 0;
			ewp.rgb_power = 0.75;
			ewp.rgb_base_weight = 0;
			ewp.rgb_mean_weight = 1;
			ewp.alpha_base_weight = 0.05f;
			rgb_force_use_of_hdr = 1;
			alpha_force_use_of_hdr = 0;
			dblimit_user_specified = 999;
			dblimit_set_by_user = 1;
		}

		else if (!strcmp(argv[argidx], "-hdra_log"))
		{
			if(decode_mode != DECODE_HDR)
			{
				printf("The option -hdra_log is only available in HDR mode\n");
				exit(1);
			}

			argidx++;
			ewp.mean_stdev_radius = 0;
			ewp.rgb_power = 1;
			ewp.rgb_base_weight = 0;
			ewp.rgb_mean_weight = 1;
			ewp.alpha_power = 1;
			ewp.alpha_base_weight = 0;
			ewp.alpha_mean_weight = 1;
			rgb_force_use_of_hdr = 1;
			alpha_force_use_of_hdr = 1;
			dblimit_user_specified = 999;
			dblimit_set_by_user = 1;
		}

		else if (!strcmp(argv[argidx], "-hdr_log"))
		{
			argidx++;
			ewp.mean_stdev_radius = 0;
			ewp.rgb_power = 1;
			ewp.rgb_base_weight = 0;
			ewp.rgb_mean_weight = 1;
			ewp.alpha_base_weight = 0.05f;
			rgb_force_use_of_hdr = 1;
			alpha_force_use_of_hdr = 0;
			dblimit_user_specified = 999;
			dblimit_set_by_user = 1;
		}


		// presets end here

		else if (!strcmp(argv[argidx], "-forcehdr_rgb"))
		{
			if(decode_mode != DECODE_HDR)
			{
				printf("The option -forcehdr_rgb is only available in HDR mode\n");
				exit(1);
			}

			argidx++;
			rgb_force_use_of_hdr = 1;
		}

		else if (!strcmp(argv[argidx], "-forcehdr_rgba"))
		{
			if(decode_mode != DECODE_HDR)
			{
				printf("The option -forcehdr_rgbs is only available in HDR mode\n");
				exit(1);
			}

			argidx++;
			rgb_force_use_of_hdr = 1;
			alpha_force_use_of_hdr = 1;
		}

		else if (!strcmp(argv[argidx], "-bmc"))
		{
			argidx += 2;
			if (argidx > argc)
			{
				printf("-bmc switch with no argument\n");
				exit(1);
			}
			float cutoff = (float)atof(argv[argidx - 1]);
			if (cutoff > 100 || !(cutoff >= 0))
				cutoff = 100;
			bmc_user_specified = cutoff;
			bmc_set_by_user = 1;
		}

		else if (!strcmp(argv[argidx], "-plimit"))
		{
			argidx += 2;
			if (argidx > argc)
			{
				printf("-plimit switch with no argument\n");
				exit(1);
			}
			plimit_user_specified = atoi(argv[argidx - 1]);
			plimit_set_by_user = 1;
		}
		else if (!strcmp(argv[argidx], "-dblimit"))
		{
			argidx += 2;
			if (argidx > argc)
			{
				printf("-dblimit switch with no argument\n");
				exit(1);
			}
			dblimit_user_specified = static_cast < float >(atof(argv[argidx - 1]));
			dblimit_set_by_user = 1;
		}
		else if (!strcmp(argv[argidx], "-oplimit"))
		{
			argidx += 2;
			if (argidx > argc)
			{
				printf("-oplimit switch with no argument\n");
				exit(1);
			}
			oplimit_user_specified = static_cast < float >(atof(argv[argidx - 1]));
			oplimit_set_by_user = 1;
		}
		else if (!strcmp(argv[argidx], "-mincorrel"))
		{
			argidx += 2;
			if (argidx > argc)
			{
				printf("-mincorrel switch with no argument\n");
				exit(1);
			}
			mincorrel_user_specified = static_cast < float >(atof(argv[argidx - 1]));
			mincorrel_set_by_user = 1;
		}
		else if (!strcmp(argv[argidx], "-maxiters"))
		{
			argidx += 2;
			if (argidx > argc)
			{
				printf("-maxiters switch with no argument\n");
				exit(1);
			}
			maxiters_user_specified = atoi(argv[argidx - 1]);
			maxiters_set_by_user = 1;
		}


		else if (!strcmp(argv[argidx], "-veryfast"))
		{
			argidx++;
			plimit_autoset = 2;
			oplimit_autoset = 1.0;
			dblimit_autoset_2d = MAX(70 - 35 * log10_texels_2d, 53 - 19 * log10_texels_2d);
			dblimit_autoset_3d = MAX(70 - 35 * log10_texels_3d, 53 - 19 * log10_texels_3d);
			bmc_autoset = 25;
			mincorrel_autoset = 0.5;
			maxiters_autoset = 1;

			switch (ydim_2d)
			{
			case 4:
				pcdiv = 240;
				break;
			case 5:
				pcdiv = 56;
				break;
			case 6:
				pcdiv = 64;
				break;
			case 8:
				pcdiv = 47;
				break;
			case 10:
				pcdiv = 36;
				break;
			case 12:
				pcdiv = 30;
				break;
			default:
				pcdiv = 30;
				break;
			}
			preset_has_been_set++;
		}

		else if (!strcmp(argv[argidx], "-fast"))
		{
			argidx++;
			plimit_autoset = 4;
			oplimit_autoset = 1.0;
			mincorrel_autoset = 0.5;
			dblimit_autoset_2d = MAX(85 - 35 * log10_texels_2d, 63 - 19 * log10_texels_2d);
			dblimit_autoset_3d = MAX(85 - 35 * log10_texels_3d, 63 - 19 * log10_texels_3d);
			bmc_autoset = 50;
			maxiters_autoset = 1;


			switch (ydim_2d)
			{
			case 4:
				pcdiv = 60;
				break;
			case 5:
				pcdiv = 27;
				break;
			case 6:
				pcdiv = 30;
				break;
			case 8:
				pcdiv = 24;
				break;
			case 10:
				pcdiv = 16;
				break;
			case 12:
				pcdiv = 20;
				break;
			default:
				pcdiv = 20;
				break;
			};
			preset_has_been_set++;
		}
		else if (!strcmp(argv[argidx], "-medium"))
		{
			argidx++;
			plimit_autoset = 25;
			oplimit_autoset = 1.2f;
			mincorrel_autoset = 0.75f;
			dblimit_autoset_2d = MAX(95 - 35 * log10_texels_2d, 70 - 19 * log10_texels_2d);
			dblimit_autoset_3d = MAX(95 - 35 * log10_texels_3d, 70 - 19 * log10_texels_3d);
			bmc_autoset = 75;
			maxiters_autoset = 2;

			switch (ydim_2d)
			{
			case 4:
				pcdiv = 25;
				break;
			case 5:
				pcdiv = 15;
				break;
			case 6:
				pcdiv = 15;
				break;
			case 8:
				pcdiv = 10;
				break;
			case 10:
				pcdiv = 8;
				break;
			case 12:
				pcdiv = 6;
				break;
			default:
				pcdiv = 6;
				break;
			};
			preset_has_been_set++;
		}
		else if (!strcmp(argv[argidx], "-thorough"))
		{
			argidx++;
			plimit_autoset = 100;
			oplimit_autoset = 2.5f;
			mincorrel_autoset = 0.95f;
			dblimit_autoset_2d = MAX(105 - 35 * log10_texels_2d, 77 - 19 * log10_texels_2d);
			dblimit_autoset_3d = MAX(105 - 35 * log10_texels_3d, 77 - 19 * log10_texels_3d);
			bmc_autoset = 95;
			maxiters_autoset = 4;

			switch (ydim_2d)
			{
			case 4:
				pcdiv = 12;
				break;
			case 5:
				pcdiv = 7;
				break;
			case 6:
				pcdiv = 7;
				break;
			case 8:
				pcdiv = 5;
				break;
			case 10:
				pcdiv = 4;
				break;
			case 12:
				pcdiv = 3;
				break;
			default:
				pcdiv = 3;
				break;
			};
			preset_has_been_set++;
		}
		else if (!strcmp(argv[argidx], "-exhaustive"))
		{
			argidx++;
			plimit_autoset = PARTITION_COUNT;
			oplimit_autoset = 1000.0f;
			mincorrel_autoset = 0.99f;
			dblimit_autoset_2d = 999.0f;
			dblimit_autoset_3d = 999.0f;
			bmc_autoset = 100;
			maxiters_autoset = 4;

			preset_has_been_set++;
			switch (ydim_2d)
			{
			case 4:
				pcdiv = 3;
				break;
			case 5:
				pcdiv = 1;
				break;
			case 6:
				pcdiv = 1;
				break;
			case 8:
				pcdiv = 1;
				break;
			case 10:
				pcdiv = 1;
				break;
			case 12:
				pcdiv = 1;
				break;
			default:
				pcdiv = 1;
				break;
			}
		}
		else if (!strcmp(argv[argidx], "-j"))
		{
			argidx += 2;
			if (argidx > argc)
			{
				printf("-j switch with no argument\n");
				exit(1);
			}
			thread_count = atoi(argv[argidx - 1]);
		}

		else if (!strcmp(argv[argidx], "-srgb"))
		{
			argidx++;
			perform_srgb_transform = 1;
			dblimit_user_specified = 60;
			dblimit_set_by_user = 1;
		}

		else if (!strcmp(argv[argidx], "-mpsnr"))
		{
			argidx += 3;
			if (argidx > argc)
			{
				printf("-mpsnr switch with less than 2 arguments\n");
				exit(1);
			}
			low_fstop = atoi(argv[argidx - 2]);
			high_fstop = atoi(argv[argidx - 1]);
			if (high_fstop < low_fstop)
			{
				printf("For -mpsnr switch, the <low> argument cannot be greater than the\n" "high argument.\n");
				exit(1);
			}
		}

		else if (!strcmp(argv[argidx], "-diag"))
		{
			argidx += 2;
			if (argidx > argc)
			{
				printf("-diag switch with no argument\n");
				exit(1);
			}

			#ifdef DEBUG_PRINT_DIAGNOSTICS
				diagnostics_tile = atoi(argv[argidx - 1]);
			#else
				printf("-diag switch given, but codec has been compiled without\n" "DEBUG_PRINT_DIAGNOSTICS enabled; please recompile.\n");
				exit(1);
			#endif
		}
		else if (!strcmp(argv[argidx], "-bmstat"))
		{
			argidx++;
			print_block_mode_histogram = 1;
		}
		else if (!strcmp(argv[argidx], "-pte"))
		{
			argidx++;
			print_tile_errors = 1;
		}
		else if (!strcmp(argv[argidx], "-stats"))
		{
			argidx++;
			print_statistics = 1;
		}

		// Option: Encode a 3D image from an array of 2D images.
		else if (!strcmp(argv[argidx], "-array"))
		{
			// Only supports compressing (not decompressing or comparison).
			if (opmode != 0)
			{
				printf("-array switch given when not compressing files - decompression and comparison of arrays not supported.\n");
				exit(1);
			}

			// Image depth must be specified.
			if (argidx + 2 > argc)
			{
				printf("-array switch given, but no array size (image depth) given.\n");
				exit(1);
			}
			argidx++;

			// Read array size (image depth).
			if (!sscanf(argv[argidx], "%u", &array_size) || array_size == 0)
			{
				printf("Invalid array size (image depth) given with -array option: \"%s\".\n", argv[argidx]);
				exit(1);
			}
			argidx++;
		}

		else
		{
			printf("Commandline argument \"%s\" not recognized\n", argv[argidx]);
			exit(1);
		}
	}


	if (opmode == 4)
	{
		compare_two_files(input_filename, output_filename, low_fstop, high_fstop, psnrmode);
		exit(0);
	}


	float texel_avg_error_limit_2d = 0.0f;
	float texel_avg_error_limit_3d = 0.0f;

	if (opmode == 0 || opmode == 2)
	{
		// if encode, process the parsed command line values

		if (preset_has_been_set != 1)
		{
			printf("For encoding, need to specify exactly one performance-quality\n"
				   "trade-off preset option. The available presets are:\n" " -veryfast\n" " -fast\n" " -medium\n" " -thorough\n" " -exhaustive\n");
			exit(1);
		}

		progress_counter_divider = pcdiv;

		int partitions_to_test = plimit_set_by_user ? plimit_user_specified : plimit_autoset;
		float dblimit_2d = dblimit_set_by_user ? dblimit_user_specified : dblimit_autoset_2d;
		float dblimit_3d = dblimit_set_by_user ? dblimit_user_specified : dblimit_autoset_3d;
		float oplimit = oplimit_set_by_user ? oplimit_user_specified : oplimit_autoset;
		float mincorrel = mincorrel_set_by_user ? mincorrel_user_specified : mincorrel_autoset;

		int maxiters = maxiters_set_by_user ? maxiters_user_specified : maxiters_autoset;
		ewp.max_refinement_iters = maxiters;

		ewp.block_mode_cutoff = (bmc_set_by_user ? bmc_user_specified : bmc_autoset) / 100.0f;

		if (rgb_force_use_of_hdr == 0)
		{
			texel_avg_error_limit_2d = pow(0.1f, dblimit_2d * 0.1f) * 65535.0f * 65535.0f;
			texel_avg_error_limit_3d = pow(0.1f, dblimit_3d * 0.1f) * 65535.0f * 65535.0f;
		}
		else
		{
			texel_avg_error_limit_2d = 0.0f;
			texel_avg_error_limit_3d = 0.0f;
		}
		ewp.partition_1_to_2_limit = oplimit;
		ewp.lowest_correlation_cutoff = mincorrel;

		if (partitions_to_test < 1)
			partitions_to_test = 1;
		else if (partitions_to_test > PARTITION_COUNT)
			partitions_to_test = PARTITION_COUNT;
		ewp.partition_search_limit = partitions_to_test;

		// if diagnostics are run, force the thread count to 1.
		if (
			#ifdef DEBUG_PRINT_DIAGNOSTICS
			   diagnostics_tile >= 0 ||
			#endif
			   print_tile_errors > 0 || print_statistics > 0)
		{
			thread_count = 1;
			thread_count_autodetected = 0;
		}

		if (thread_count < 1)
		{
			thread_count = get_number_of_cpus();
			thread_count_autodetected = 1;
		}


		// Specifying the error weight of a color component as 0 is not allowed.
		// If weights are 0, then they are instead set to a small positive value.

		float max_color_component_weight = MAX(MAX(ewp.rgba_weights[0], ewp.rgba_weights[1]),
											   MAX(ewp.rgba_weights[2], ewp.rgba_weights[3]));
		ewp.rgba_weights[0] = MAX(ewp.rgba_weights[0], max_color_component_weight / 1000.0f);
		ewp.rgba_weights[1] = MAX(ewp.rgba_weights[1], max_color_component_weight / 1000.0f);
		ewp.rgba_weights[2] = MAX(ewp.rgba_weights[2], max_color_component_weight / 1000.0f);
		ewp.rgba_weights[3] = MAX(ewp.rgba_weights[3], max_color_component_weight / 1000.0f);


		// print all encoding settings unless specifically told otherwise.
		if (!silentmode)
		{
			printf("Encoding settings:\n\n");
			if (target_bitrate_set)
				printf("Target bitrate provided: %.2f bpp\n", target_bitrate);
			printf("2D Block size: %dx%d (%.2f bpp)\n", xdim_2d, ydim_2d, 128.0 / (xdim_2d * ydim_2d));
			printf("3D Block size: %dx%dx%d (%.2f bpp)\n", xdim_3d, ydim_3d, zdim_3d, 128.0 / (xdim_3d * ydim_3d * zdim_3d));
			printf("Radius for mean-and-stdev calculations: %d texels\n", ewp.mean_stdev_radius);
			printf("RGB power: %g\n", ewp.rgb_power);
			printf("RGB base-weight: %g\n", ewp.rgb_base_weight);
			printf("RGB local-mean weight: %g\n", ewp.rgb_mean_weight);
			printf("RGB local-stdev weight: %g\n", ewp.rgb_stdev_weight);
			printf("RGB mean-and-stdev mixing across color channels: %g\n", ewp.rgb_mean_and_stdev_mixing);
			printf("Alpha power: %g\n", ewp.alpha_power);
			printf("Alpha base-weight: %g\n", ewp.alpha_base_weight);
			printf("Alpha local-mean weight: %g\n", ewp.alpha_mean_weight);
			printf("Alpha local-stdev weight: %g\n", ewp.alpha_stdev_weight);
			printf("RGB weights scale with alpha: ");
			if (ewp.enable_rgb_scale_with_alpha)
				printf("enabled (radius=%d)\n", ewp.alpha_radius);
			else
				printf("disabled\n");
			printf("Color channel relative weighting: R=%g G=%g B=%g A=%g\n", ewp.rgba_weights[0], ewp.rgba_weights[1], ewp.rgba_weights[2], ewp.rgba_weights[3]);
			printf("Block-artifact suppression parameter : %g\n", ewp.block_artifact_suppression);
			printf("Number of distinct partitionings to test: %d (%s)\n", ewp.partition_search_limit, plimit_set_by_user ? "specified by user" : "preset");
			printf("PSNR decibel limit: 2D: %f 3D: %f (%s)\n", dblimit_2d, dblimit_3d, dblimit_set_by_user ? "specified by user" : "preset");
			printf("1->2 partition limit: %f\n", oplimit);
			printf("Dual-plane color-correlation cutoff: %f (%s)\n", mincorrel, mincorrel_set_by_user ? "specified by user" : "preset");
			printf("Block Mode Percentile Cutoff: %f (%s)\n", ewp.block_mode_cutoff * 100.0f, bmc_set_by_user ? "specified by user" : "preset");
			printf("Max refinement iterations: %d (%s)\n", ewp.max_refinement_iters, maxiters_set_by_user ? "specified by user" : "preset");
			printf("Thread count : %d (%s)\n", thread_count, thread_count_autodetected ? "autodetected" : "specified by user");

			printf("\n");
		}

	}


	int padding = MAX(ewp.mean_stdev_radius, ewp.alpha_radius);

	// determine encoding bitness as follows:
	// if enforced by the output format, follow the output format's result
	// else use decode_mode to pick bitness.
	int bitness = get_output_filename_enforced_bitness(output_filename);
	if (bitness == -1)
	{
		bitness = (decode_mode == DECODE_HDR) ? 16 : 8;
	}

	int xdim = -1;
	int ydim = -1;
	int zdim = -1;

	// Temporary image array (for merging multiple 2D images into one 3D image).
	int *load_results = NULL;
	astc_codec_image **input_images = NULL;

	int load_result = 0;
	astc_codec_image *input_image = NULL;
	astc_codec_image *output_image = NULL;
	int input_components = 0;

	int input_image_is_hdr = 0;

	// load image
	if (opmode == 0 || opmode == 2 || opmode == 3)
	{
		// Allocate arrays for image data and load results.
		load_results = new int[array_size];
		input_images = new astc_codec_image *[array_size];

		// Iterate over all input images.
		for (int image_index = 0; image_index < array_size; image_index++)
		{
			// 2D input data.
			if (array_size == 1)
			{
				input_images[image_index] = astc_codec_load_image(input_filename, padding, &load_results[image_index]);
			}

			// 3D input data - multiple 2D images.
			else
			{
				char new_input_filename[256];

				// Check for extension: <name>.<extension>
				if (NULL == strrchr(input_filename, '.'))
				{
					printf("Unable to determine file type from extension: %s\n", input_filename);
					exit(1);
				}

				// Construct new file name and load: <name>_N.<extension>
				strcpy(new_input_filename, input_filename);
				sprintf(strrchr(new_input_filename, '.'), "_%d%s", image_index, strrchr(input_filename, '.'));
				input_images[image_index] = astc_codec_load_image(new_input_filename, padding, &load_results[image_index]);

				// Check image is not 3D.
				if (input_images[image_index]->zsize != 1)
				{
					printf("3D source images not supported with -array option: %s\n", new_input_filename);
					exit(1);
				}

				// BCJ(DEBUG)
				// printf("\n\n Image %d \n", image_index);
				// dump_image( input_images[image_index] );
				// printf("\n\n");
			}

			// Check load result.
			if (load_results[image_index] < 0)
			{
				printf("Failed to load image %s\n", input_filename);
				exit(1);
			}

			// Check format matches other slices.
			if (load_results[image_index] != load_results[0])
			{
				printf("Mismatching image format - image 0 and %d are a different format\n", image_index);
				exit(1);
			}
		}

		load_result = load_results[0];

		// Assign input image.
		if (array_size == 1)
		{
			input_image = input_images[0];
		}

		// Merge input image data.
		else
		{
			int i, z, xsize, ysize, zsize, bitness, slice_size;

			xsize = input_images[0]->xsize;
			ysize = input_images[0]->ysize;
			zsize = array_size;
			bitness = (load_result & 0x80) ? 16 : 8;
			slice_size = (xsize + (2 * padding)) * (ysize + (2 * padding));

			// Allocate image memory.
			input_image = allocate_image(bitness, xsize, ysize, zsize, padding);

			// Combine 2D source images into one 3D image (skip padding slices as these don't exist in 2D textures).
			for (z = padding; z < zsize + padding; z++)
			{
				if (bitness == 8)
				{
					memcpy(*input_image->imagedata8[z], *input_images[z - padding]->imagedata8[0], slice_size * 4 * sizeof(uint8_t));
				}
				else
				{
					memcpy(*input_image->imagedata16[z], *input_images[z - padding]->imagedata16[0], slice_size * 4 * sizeof(uint16_t));
				}
			}

			// Clean up temporary images.
			for (i = 0; i < array_size; i++)
			{
				destroy_image(input_images[i]);
			}
			input_images = NULL;

			// Clamp texels outside the actual image area.
			fill_image_padding_area(input_image);

			// BCJ(DEBUG)
			// dump_image( input_image );
		}

		input_components = load_result & 7;
		input_image_is_hdr = (load_result & 0x80) ? 1 : 0;

		if (input_image->zsize > 1)
		{
			xdim = xdim_3d;
			ydim = ydim_3d;
			zdim = zdim_3d;
			ewp.texel_avg_error_limit = texel_avg_error_limit_3d;
		}
		else
		{
			xdim = xdim_2d;
			ydim = ydim_2d;
			zdim = 1;
			ewp.texel_avg_error_limit = texel_avg_error_limit_2d;
		}
		expand_block_artifact_suppression(xdim, ydim, zdim, &ewp);


		if (!silentmode)
		{
			printf("%s: %dD %s image, %d x %d x %d, %d components\n\n",
				   input_filename, input_image->zsize > 1 ? 3 : 2, input_image_is_hdr ? "HDR" : "LDR", input_image->xsize, input_image->ysize, input_image->zsize, load_result & 7);
		}

		if (padding > 0 || ewp.rgb_mean_weight != 0.0f || ewp.rgb_stdev_weight != 0.0f || ewp.alpha_mean_weight != 0.0f || ewp.alpha_stdev_weight != 0.0f)
		{
			if (!silentmode)
			{
				printf("Computing texel-neighborhood means and variances ... ");
				fflush(stdout);
			}
			compute_averages_and_variances(input_image, ewp.rgb_power, ewp.alpha_power, ewp.mean_stdev_radius, ewp.alpha_radius, swz_encode);
			if (!silentmode)
			{
				printf("done\n");
				fflush(stdout);
			}
		}
	}


	start_coding_time = get_time();

	if (opmode == 1)
		output_image = load_astc_file(input_filename, bitness, decode_mode, swz_decode);


	// process image, if relevant
	if (opmode == 2)
		output_image = pack_and_unpack_astc_image(input_image, xdim, ydim, zdim, &ewp, decode_mode, swz_encode, swz_decode, bitness, thread_count);


	end_coding_time = get_time();


	// print PSNR if encoding
	if (opmode == 2)
	{
		if (psnrmode == 1)
		{
			compute_error_metrics(input_image_is_hdr, input_components, input_image, output_image, low_fstop, high_fstop, psnrmode);
		}
	}


	// store image
	if (opmode == 1 || opmode == 2)
	{
		int store_result = -1;
		const char *format_string = "";

		store_result = astc_codec_store_image(output_image, output_filename, bitness, &format_string);

		if (store_result < 0)
		{
			printf("Failed to store image %s\n", output_filename);
			exit(1);
		}
		else
		{
			if (!silentmode)
			{
				printf("Stored %s image %s with %d color channels\n", format_string, output_filename, store_result);
			}
		}
	}
	if (opmode == 0)
	{
		store_astc_file(input_image, output_filename, xdim, ydim, zdim, &ewp, decode_mode, swz_encode, thread_count);
	}


	if (print_block_mode_histogram)
	{
		printf("%s ", argv[2]);
		printf("%d %d  ", xdim_2d, ydim_2d);
		for (i = 0; i < 2048; i++)
			printf(" %d", block_mode_histogram[i]);
		printf("\n");
	}


	end_time = get_time();

	if (timemode)
	{
		printf("\nElapsed time: %.2lf seconds, of which coding time: %.2lf seconds\n", end_time - start_time, end_coding_time - start_coding_time);
	}

	return 0;
}
