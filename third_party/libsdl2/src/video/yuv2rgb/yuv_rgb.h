// Copyright 2016 Adrien Descamps
// Distributed under BSD 3-Clause License

// Provide optimized functions to convert images from 8bits yuv420 to rgb24 format

// There are a few slightly different variations of the YCbCr color space with different parameters that 
// change the conversion matrix.
// The three most common YCbCr color space, defined by BT.601, BT.709 and JPEG standard are implemented here.
// See the respective standards for details
// The matrix values used are derived from http://www.equasys.de/colorconversion.html

// YUV420 is stored as three separate channels, with U and V (Cb and Cr) subsampled by a 2 factor
// For conversion from yuv to rgb, no interpolation is done, and the same UV value are used for 4 rgb pixels. This 
// is suboptimal for image quality, but by far the fastest method.

// For all methods, width and height should be even, if not, the last row/column of the result image won't be affected.
// For sse methods, if the width if not divisable by 32, the last (width%32) pixels of each line won't be affected.

#include "SDL_stdinc.h"
/*#include <stdint.h>*/

typedef enum
{
	YCBCR_JPEG,
	YCBCR_601,
	YCBCR_709
} YCbCrType;

// yuv to rgb, standard c implementation
void yuv420_rgb565_std(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv420_rgb24_std(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv420_rgba_std(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv420_bgra_std(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv420_argb_std(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv420_abgr_std(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv422_rgb565_std(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv422_rgb24_std(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv422_rgba_std(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv422_bgra_std(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv422_argb_std(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv422_abgr_std(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuvnv12_rgb565_std(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuvnv12_rgb24_std(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuvnv12_rgba_std(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuvnv12_bgra_std(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuvnv12_argb_std(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuvnv12_abgr_std(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

// yuv to rgb, sse implementation
// pointers must be 16 byte aligned, and strides must be divisable by 16
void yuv420_rgb565_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv420_rgb24_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv420_rgba_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv420_bgra_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv420_argb_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv420_abgr_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv422_rgb565_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv422_rgb24_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv422_rgba_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv422_bgra_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv422_argb_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv422_abgr_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuvnv12_rgb565_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuvnv12_rgb24_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuvnv12_rgba_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuvnv12_bgra_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuvnv12_argb_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuvnv12_abgr_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

// yuv to rgb, sse implementation
// pointers do not need to be 16 byte aligned
void yuv420_rgb565_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv420_rgb24_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv420_rgba_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv420_bgra_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv420_argb_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv420_abgr_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv422_rgb565_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv422_rgb24_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv422_rgba_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv422_bgra_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv422_argb_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuv422_abgr_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuvnv12_rgb565_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuvnv12_rgb24_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuvnv12_rgba_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuvnv12_bgra_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuvnv12_argb_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);

void yuvnv12_abgr_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *y, const uint8_t *u, const uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	uint8_t *rgb, uint32_t rgb_stride, 
	YCbCrType yuv_type);


// rgb to yuv, standard c implementation
void rgb24_yuv420_std(
	uint32_t width, uint32_t height, 
	const uint8_t *rgb, uint32_t rgb_stride, 
	uint8_t *y, uint8_t *u, uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	YCbCrType yuv_type);

// rgb to yuv, sse implementation
// pointers must be 16 byte aligned, and strides must be divisible by 16
void rgb24_yuv420_sse(
	uint32_t width, uint32_t height, 
	const uint8_t *rgb, uint32_t rgb_stride, 
	uint8_t *y, uint8_t *u, uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	YCbCrType yuv_type);

// rgb to yuv, sse implementation
// pointers do not need to be 16 byte aligned
void rgb24_yuv420_sseu(
	uint32_t width, uint32_t height, 
	const uint8_t *rgb, uint32_t rgb_stride, 
	uint8_t *y, uint8_t *u, uint8_t *v, uint32_t y_stride, uint32_t uv_stride, 
	YCbCrType yuv_type);

