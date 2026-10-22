// Copyright 2016 Adrien Descamps
// Distributed under BSD 3-Clause License
#include "../../SDL_internal.h"

#include "yuv_rgb.h"

#include "SDL_cpuinfo.h"
/*#include <x86intrin.h>*/

#define PRECISION 6
#define PRECISION_FACTOR (1<<PRECISION)

typedef struct
{
	uint8_t y_shift;
	int16_t matrix[3][3];
} RGB2YUVParam;
// |Y|   |y_shift|                        |matrix[0][0] matrix[0][1] matrix[0][2]|   |R|
// |U| = |  128  | + 1/PRECISION_FACTOR * |matrix[1][0] matrix[1][1] matrix[1][2]| * |G|
// |V|   |  128  |                        |matrix[2][0] matrix[2][1] matrix[2][2]|   |B|

typedef struct
{
	uint8_t y_shift;
	int16_t y_factor;
	int16_t v_r_factor;
	int16_t u_g_factor;
	int16_t v_g_factor;
	int16_t u_b_factor;
} YUV2RGBParam;
// |R|                        |y_factor      0       v_r_factor|   |Y-y_shift|
// |G| = 1/PRECISION_FACTOR * |y_factor  u_g_factor  v_g_factor| * |  U-128  |
// |B|                        |y_factor  u_b_factor      0     |   |  V-128  |

#define V(value) (int16_t)((value*PRECISION_FACTOR)+0.5)

// for ITU-T T.871, values can be found in section 7
// for ITU-R BT.601-7 values are derived from equations in sections 2.5.1-2.5.3, assuming RGB is encoded using full range ([0-1]<->[0-255])
// for ITU-R BT.709-6 values are derived from equations in sections 3.2-3.4, assuming RGB is encoded using full range ([0-1]<->[0-255])
// all values are rounded to the fourth decimal

static const YUV2RGBParam YUV2RGB[3] = {
	// ITU-T T.871 (JPEG)
	{/*.y_shift=*/ 0, /*.y_factor=*/ V(1.0), /*.v_r_factor=*/ V(1.402), /*.u_g_factor=*/ -V(0.3441), /*.v_g_factor=*/ -V(0.7141), /*.u_b_factor=*/ V(1.772)},
	// ITU-R BT.601-7
	{/*.y_shift=*/ 16, /*.y_factor=*/ V(1.1644), /*.v_r_factor=*/ V(1.596), /*.u_g_factor=*/ -V(0.3918), /*.v_g_factor=*/ -V(0.813), /*.u_b_factor=*/ V(2.0172)},
	// ITU-R BT.709-6
	{/*.y_shift=*/ 16, /*.y_factor=*/ V(1.1644), /*.v_r_factor=*/ V(1.7927), /*.u_g_factor=*/ -V(0.2132), /*.v_g_factor=*/ -V(0.5329), /*.u_b_factor=*/ V(2.1124)}
};

static const RGB2YUVParam RGB2YUV[3] = {
	// ITU-T T.871 (JPEG)
	{/*.y_shift=*/ 0, /*.matrix=*/ {{V(0.299), V(0.587), V(0.114)}, {-V(0.1687), -V(0.3313), V(0.5)}, {V(0.5), -V(0.4187), -V(0.0813)}}},
	// ITU-R BT.601-7
	{/*.y_shift=*/ 16, /*.matrix=*/ {{V(0.2568), V(0.5041), V(0.0979)}, {-V(0.1482), -V(0.291), V(0.4392)}, {V(0.4392), -V(0.3678), -V(0.0714)}}},
	// ITU-R BT.709-6
	{/*.y_shift=*/ 16, /*.matrix=*/ {{V(0.1826), V(0.6142), V(0.062)}, {-V(0.1006), -V(0.3386), V(0.4392)}, {V(0.4392), -V(0.3989), -V(0.0403)}}}
};

/* The various layouts of YUV data we support */
#define YUV_FORMAT_420	1
#define YUV_FORMAT_422	2
#define YUV_FORMAT_NV12	3

/* The various formats of RGB pixel that we support */
#define RGB_FORMAT_RGB565	1
#define RGB_FORMAT_RGB24	2
#define RGB_FORMAT_RGBA		3
#define RGB_FORMAT_BGRA		4
#define RGB_FORMAT_ARGB		5
#define RGB_FORMAT_ABGR		6

// divide by PRECISION_FACTOR and clamp to [0:255] interval
// input must be in the [-128*PRECISION_FACTOR:384*PRECISION_FACTOR] range
static uint8_t clampU8(int32_t v)
{
	static const uint8_t lut[512] = 
	{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,
	47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,
	91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,119,120,121,122,123,124,125,
	126,127,128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,
	159,160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
	192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,224,
	225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,
	255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
	};
	return lut[(v+128*PRECISION_FACTOR)>>PRECISION];
}


#define STD_FUNCTION_NAME	yuv420_rgb565_std
#define YUV_FORMAT			YUV_FORMAT_420
#define RGB_FORMAT			RGB_FORMAT_RGB565
#include "yuv_rgb_std_func.h"

#define STD_FUNCTION_NAME	yuv420_rgb24_std
#define YUV_FORMAT			YUV_FORMAT_420
#define RGB_FORMAT			RGB_FORMAT_RGB24
#include "yuv_rgb_std_func.h"

#define STD_FUNCTION_NAME	yuv420_rgba_std
#define YUV_FORMAT			YUV_FORMAT_420
#define RGB_FORMAT			RGB_FORMAT_RGBA
#include "yuv_rgb_std_func.h"

#define STD_FUNCTION_NAME	yuv420_bgra_std
#define YUV_FORMAT			YUV_FORMAT_420
#define RGB_FORMAT			RGB_FORMAT_BGRA
#include "yuv_rgb_std_func.h"

#define STD_FUNCTION_NAME	yuv420_argb_std
#define YUV_FORMAT			YUV_FORMAT_420
#define RGB_FORMAT			RGB_FORMAT_ARGB
#include "yuv_rgb_std_func.h"

#define STD_FUNCTION_NAME	yuv420_abgr_std
#define YUV_FORMAT			YUV_FORMAT_420
#define RGB_FORMAT			RGB_FORMAT_ABGR
#include "yuv_rgb_std_func.h"

#define STD_FUNCTION_NAME	yuv422_rgb565_std
#define YUV_FORMAT			YUV_FORMAT_422
#define RGB_FORMAT			RGB_FORMAT_RGB565
#include "yuv_rgb_std_func.h"

#define STD_FUNCTION_NAME	yuv422_rgb24_std
#define YUV_FORMAT			YUV_FORMAT_422
#define RGB_FORMAT			RGB_FORMAT_RGB24
#include "yuv_rgb_std_func.h"

#define STD_FUNCTION_NAME	yuv422_rgba_std
#define YUV_FORMAT			YUV_FORMAT_422
#define RGB_FORMAT			RGB_FORMAT_RGBA
#include "yuv_rgb_std_func.h"

#define STD_FUNCTION_NAME	yuv422_bgra_std
#define YUV_FORMAT			YUV_FORMAT_422
#define RGB_FORMAT			RGB_FORMAT_BGRA
#include "yuv_rgb_std_func.h"

#define STD_FUNCTION_NAME	yuv422_argb_std
#define YUV_FORMAT			YUV_FORMAT_422
#define RGB_FORMAT			RGB_FORMAT_ARGB
#include "yuv_rgb_std_func.h"

#define STD_FUNCTION_NAME	yuv422_abgr_std
#define YUV_FORMAT			YUV_FORMAT_422
#define RGB_FORMAT			RGB_FORMAT_ABGR
#include "yuv_rgb_std_func.h"

#define STD_FUNCTION_NAME	yuvnv12_rgb565_std
#define YUV_FORMAT			YUV_FORMAT_NV12
#define RGB_FORMAT			RGB_FORMAT_RGB565
#include "yuv_rgb_std_func.h"

#define STD_FUNCTION_NAME	yuvnv12_rgb24_std
#define YUV_FORMAT			YUV_FORMAT_NV12
#define RGB_FORMAT			RGB_FORMAT_RGB24
#include "yuv_rgb_std_func.h"

#define STD_FUNCTION_NAME	yuvnv12_rgba_std
#define YUV_FORMAT			YUV_FORMAT_NV12
#define RGB_FORMAT			RGB_FORMAT_RGBA
#include "yuv_rgb_std_func.h"

#define STD_FUNCTION_NAME	yuvnv12_bgra_std
#define YUV_FORMAT			YUV_FORMAT_NV12
#define RGB_FORMAT			RGB_FORMAT_BGRA
#include "yuv_rgb_std_func.h"

#define STD_FUNCTION_NAME	yuvnv12_argb_std
#define YUV_FORMAT			YUV_FORMAT_NV12
#define RGB_FORMAT			RGB_FORMAT_ARGB
#include "yuv_rgb_std_func.h"

#define STD_FUNCTION_NAME	yuvnv12_abgr_std
#define YUV_FORMAT			YUV_FORMAT_NV12
#define RGB_FORMAT			RGB_FORMAT_ABGR
#include "yuv_rgb_std_func.h"

void rgb24_yuv420_std(
	uint32_t width, uint32_t height, 
	const uint8_t *RGB, uint32_t RGB_stride, 
	uint8_t *Y, uint8_t *U, uint8_t *V, uint32_t Y_stride, uint32_t UV_stride, 
	YCbCrType yuv_type)
{
	const RGB2YUVParam *const param = &(RGB2YUV[yuv_type]);
	
	uint32_t x, y;
	for(y=0; y<(height-1); y+=2)
	{
		const uint8_t *rgb_ptr1=RGB+y*RGB_stride,
			*rgb_ptr2=RGB+(y+1)*RGB_stride;
			
		uint8_t *y_ptr1=Y+y*Y_stride,
			*y_ptr2=Y+(y+1)*Y_stride,
			*u_ptr=U+(y/2)*UV_stride,
			*v_ptr=V+(y/2)*UV_stride;
		
		for(x=0; x<(width-1); x+=2)
		{
			// compute yuv for the four pixels, u and v values are summed
			int32_t y_tmp, u_tmp, v_tmp;
			
			y_tmp = param->matrix[0][0]*rgb_ptr1[0] + param->matrix[0][1]*rgb_ptr1[1] + param->matrix[0][2]*rgb_ptr1[2];
			u_tmp = param->matrix[1][0]*rgb_ptr1[0] + param->matrix[1][1]*rgb_ptr1[1] + param->matrix[1][2]*rgb_ptr1[2];
			v_tmp = param->matrix[2][0]*rgb_ptr1[0] + param->matrix[2][1]*rgb_ptr1[1] + param->matrix[2][2]*rgb_ptr1[2];
			y_ptr1[0]=clampU8(y_tmp+((param->y_shift)<<PRECISION));
			
			y_tmp = param->matrix[0][0]*rgb_ptr1[3] + param->matrix[0][1]*rgb_ptr1[4] + param->matrix[0][2]*rgb_ptr1[5];
			u_tmp += param->matrix[1][0]*rgb_ptr1[3] + param->matrix[1][1]*rgb_ptr1[4] + param->matrix[1][2]*rgb_ptr1[5];
			v_tmp += param->matrix[2][0]*rgb_ptr1[3] + param->matrix[2][1]*rgb_ptr1[4] + param->matrix[2][2]*rgb_ptr1[5];
			y_ptr1[1]=clampU8(y_tmp+((param->y_shift)<<PRECISION));
			
			y_tmp = param->matrix[0][0]*rgb_ptr2[0] + param->matrix[0][1]*rgb_ptr2[1] + param->matrix[0][2]*rgb_ptr2[2];
			u_tmp += param->matrix[1][0]*rgb_ptr2[0] + param->matrix[1][1]*rgb_ptr2[1] + param->matrix[1][2]*rgb_ptr2[2];
			v_tmp += param->matrix[2][0]*rgb_ptr2[0] + param->matrix[2][1]*rgb_ptr2[1] + param->matrix[2][2]*rgb_ptr2[2];
			y_ptr2[0]=clampU8(y_tmp+((param->y_shift)<<PRECISION));
			
			y_tmp = param->matrix[0][0]*rgb_ptr2[3] + param->matrix[0][1]*rgb_ptr2[4] + param->matrix[0][2]*rgb_ptr2[5];
			u_tmp += param->matrix[1][0]*rgb_ptr2[3] + param->matrix[1][1]*rgb_ptr2[4] + param->matrix[1][2]*rgb_ptr2[5];
			v_tmp += param->matrix[2][0]*rgb_ptr2[3] + param->matrix[2][1]*rgb_ptr2[4] + param->matrix[2][2]*rgb_ptr2[5];
			y_ptr2[1]=clampU8(y_tmp+((param->y_shift)<<PRECISION));
			
			u_ptr[0] = clampU8(u_tmp/4+(128<<PRECISION));
			v_ptr[0] = clampU8(v_tmp/4+(128<<PRECISION));
			
			rgb_ptr1 += 6;
			rgb_ptr2 += 6;
			y_ptr1 += 2;
			y_ptr2 += 2;
			u_ptr += 1;
			v_ptr += 1;
		}
	}
}

#ifdef __SSE2__

#define SSE_FUNCTION_NAME	yuv420_rgb565_sse
#define STD_FUNCTION_NAME	yuv420_rgb565_std
#define YUV_FORMAT			YUV_FORMAT_420
#define RGB_FORMAT			RGB_FORMAT_RGB565
#define SSE_ALIGNED
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv420_rgb565_sseu
#define STD_FUNCTION_NAME	yuv420_rgb565_std
#define YUV_FORMAT			YUV_FORMAT_420
#define RGB_FORMAT			RGB_FORMAT_RGB565
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv420_rgb24_sse
#define STD_FUNCTION_NAME	yuv420_rgb24_std
#define YUV_FORMAT			YUV_FORMAT_420
#define RGB_FORMAT			RGB_FORMAT_RGB24
#define SSE_ALIGNED
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv420_rgb24_sseu
#define STD_FUNCTION_NAME	yuv420_rgb24_std
#define YUV_FORMAT			YUV_FORMAT_420
#define RGB_FORMAT			RGB_FORMAT_RGB24
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv420_rgba_sse
#define STD_FUNCTION_NAME	yuv420_rgba_std
#define YUV_FORMAT			YUV_FORMAT_420
#define RGB_FORMAT			RGB_FORMAT_RGBA
#define SSE_ALIGNED
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv420_rgba_sseu
#define STD_FUNCTION_NAME	yuv420_rgba_std
#define YUV_FORMAT			YUV_FORMAT_420
#define RGB_FORMAT			RGB_FORMAT_RGBA
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv420_bgra_sse
#define STD_FUNCTION_NAME	yuv420_bgra_std
#define YUV_FORMAT			YUV_FORMAT_420
#define RGB_FORMAT			RGB_FORMAT_BGRA
#define SSE_ALIGNED
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv420_bgra_sseu
#define STD_FUNCTION_NAME	yuv420_bgra_std
#define YUV_FORMAT			YUV_FORMAT_420
#define RGB_FORMAT			RGB_FORMAT_BGRA
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv420_argb_sse
#define STD_FUNCTION_NAME	yuv420_argb_std
#define YUV_FORMAT			YUV_FORMAT_420
#define RGB_FORMAT			RGB_FORMAT_ARGB
#define SSE_ALIGNED
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv420_argb_sseu
#define STD_FUNCTION_NAME	yuv420_argb_std
#define YUV_FORMAT			YUV_FORMAT_420
#define RGB_FORMAT			RGB_FORMAT_ARGB
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv420_abgr_sse
#define STD_FUNCTION_NAME	yuv420_abgr_std
#define YUV_FORMAT			YUV_FORMAT_420
#define RGB_FORMAT			RGB_FORMAT_ABGR
#define SSE_ALIGNED
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv420_abgr_sseu
#define STD_FUNCTION_NAME	yuv420_abgr_std
#define YUV_FORMAT			YUV_FORMAT_420
#define RGB_FORMAT			RGB_FORMAT_ABGR
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv422_rgb565_sse
#define STD_FUNCTION_NAME	yuv422_rgb565_std
#define YUV_FORMAT			YUV_FORMAT_422
#define RGB_FORMAT			RGB_FORMAT_RGB565
#define SSE_ALIGNED
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv422_rgb565_sseu
#define STD_FUNCTION_NAME	yuv422_rgb565_std
#define YUV_FORMAT			YUV_FORMAT_422
#define RGB_FORMAT			RGB_FORMAT_RGB565
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv422_rgb24_sse
#define STD_FUNCTION_NAME	yuv422_rgb24_std
#define YUV_FORMAT			YUV_FORMAT_422
#define RGB_FORMAT			RGB_FORMAT_RGB24
#define SSE_ALIGNED
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv422_rgb24_sseu
#define STD_FUNCTION_NAME	yuv422_rgb24_std
#define YUV_FORMAT			YUV_FORMAT_422
#define RGB_FORMAT			RGB_FORMAT_RGB24
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv422_rgba_sse
#define STD_FUNCTION_NAME	yuv422_rgba_std
#define YUV_FORMAT			YUV_FORMAT_422
#define RGB_FORMAT			RGB_FORMAT_RGBA
#define SSE_ALIGNED
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv422_rgba_sseu
#define STD_FUNCTION_NAME	yuv422_rgba_std
#define YUV_FORMAT			YUV_FORMAT_422
#define RGB_FORMAT			RGB_FORMAT_RGBA
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv422_bgra_sse
#define STD_FUNCTION_NAME	yuv422_bgra_std
#define YUV_FORMAT			YUV_FORMAT_422
#define RGB_FORMAT			RGB_FORMAT_BGRA
#define SSE_ALIGNED
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv422_bgra_sseu
#define STD_FUNCTION_NAME	yuv422_bgra_std
#define YUV_FORMAT			YUV_FORMAT_422
#define RGB_FORMAT			RGB_FORMAT_BGRA
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv422_argb_sse
#define STD_FUNCTION_NAME	yuv422_argb_std
#define YUV_FORMAT			YUV_FORMAT_422
#define RGB_FORMAT			RGB_FORMAT_ARGB
#define SSE_ALIGNED
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv422_argb_sseu
#define STD_FUNCTION_NAME	yuv422_argb_std
#define YUV_FORMAT			YUV_FORMAT_422
#define RGB_FORMAT			RGB_FORMAT_ARGB
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv422_abgr_sse
#define STD_FUNCTION_NAME	yuv422_abgr_std
#define YUV_FORMAT			YUV_FORMAT_422
#define RGB_FORMAT			RGB_FORMAT_ABGR
#define SSE_ALIGNED
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuv422_abgr_sseu
#define STD_FUNCTION_NAME	yuv422_abgr_std
#define YUV_FORMAT			YUV_FORMAT_422
#define RGB_FORMAT			RGB_FORMAT_ABGR
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuvnv12_rgb565_sse
#define STD_FUNCTION_NAME	yuvnv12_rgb565_std
#define YUV_FORMAT			YUV_FORMAT_NV12
#define RGB_FORMAT			RGB_FORMAT_RGB565
#define SSE_ALIGNED
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuvnv12_rgb565_sseu
#define STD_FUNCTION_NAME	yuvnv12_rgb565_std
#define YUV_FORMAT			YUV_FORMAT_NV12
#define RGB_FORMAT			RGB_FORMAT_RGB565
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuvnv12_rgb24_sse
#define STD_FUNCTION_NAME	yuvnv12_rgb24_std
#define YUV_FORMAT			YUV_FORMAT_NV12
#define RGB_FORMAT			RGB_FORMAT_RGB24
#define SSE_ALIGNED
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuvnv12_rgb24_sseu
#define STD_FUNCTION_NAME	yuvnv12_rgb24_std
#define YUV_FORMAT			YUV_FORMAT_NV12
#define RGB_FORMAT			RGB_FORMAT_RGB24
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuvnv12_rgba_sse
#define STD_FUNCTION_NAME	yuvnv12_rgba_std
#define YUV_FORMAT			YUV_FORMAT_NV12
#define RGB_FORMAT			RGB_FORMAT_RGBA
#define SSE_ALIGNED
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuvnv12_rgba_sseu
#define STD_FUNCTION_NAME	yuvnv12_rgba_std
#define YUV_FORMAT			YUV_FORMAT_NV12
#define RGB_FORMAT			RGB_FORMAT_RGBA
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuvnv12_bgra_sse
#define STD_FUNCTION_NAME	yuvnv12_bgra_std
#define YUV_FORMAT			YUV_FORMAT_NV12
#define RGB_FORMAT			RGB_FORMAT_BGRA
#define SSE_ALIGNED
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuvnv12_bgra_sseu
#define STD_FUNCTION_NAME	yuvnv12_bgra_std
#define YUV_FORMAT			YUV_FORMAT_NV12
#define RGB_FORMAT			RGB_FORMAT_BGRA
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuvnv12_argb_sse
#define STD_FUNCTION_NAME	yuvnv12_argb_std
#define YUV_FORMAT			YUV_FORMAT_NV12
#define RGB_FORMAT			RGB_FORMAT_ARGB
#define SSE_ALIGNED
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuvnv12_argb_sseu
#define STD_FUNCTION_NAME	yuvnv12_argb_std
#define YUV_FORMAT			YUV_FORMAT_NV12
#define RGB_FORMAT			RGB_FORMAT_ARGB
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuvnv12_abgr_sse
#define STD_FUNCTION_NAME	yuvnv12_abgr_std
#define YUV_FORMAT			YUV_FORMAT_NV12
#define RGB_FORMAT			RGB_FORMAT_ABGR
#define SSE_ALIGNED
#include "yuv_rgb_sse_func.h"

#define SSE_FUNCTION_NAME	yuvnv12_abgr_sseu
#define STD_FUNCTION_NAME	yuvnv12_abgr_std
#define YUV_FORMAT			YUV_FORMAT_NV12
#define RGB_FORMAT			RGB_FORMAT_ABGR
#include "yuv_rgb_sse_func.h"


#define UNPACK_RGB24_32_STEP1(RGB1, RGB2, RGB3, RGB4, RGB5, RGB6, R1, R2, G1, G2, B1, B2) \
R1 = _mm_unpacklo_epi8(RGB1, RGB4); \
R2 = _mm_unpackhi_epi8(RGB1, RGB4); \
G1 = _mm_unpacklo_epi8(RGB2, RGB5); \
G2 = _mm_unpackhi_epi8(RGB2, RGB5); \
B1 = _mm_unpacklo_epi8(RGB3, RGB6); \
B2 = _mm_unpackhi_epi8(RGB3, RGB6);

#define UNPACK_RGB24_32_STEP2(RGB1, RGB2, RGB3, RGB4, RGB5, RGB6, R1, R2, G1, G2, B1, B2) \
RGB1 = _mm_unpacklo_epi8(R1, G2); \
RGB2 = _mm_unpackhi_epi8(R1, G2); \
RGB3 = _mm_unpacklo_epi8(R2, B1); \
RGB4 = _mm_unpackhi_epi8(R2, B1); \
RGB5 = _mm_unpacklo_epi8(G1, B2); \
RGB6 = _mm_unpackhi_epi8(G1, B2); \

#define UNPACK_RGB24_32(RGB1, RGB2, RGB3, RGB4, RGB5, RGB6, R1, R2, G1, G2, B1, B2) \
UNPACK_RGB24_32_STEP1(RGB1, RGB2, RGB3, RGB4, RGB5, RGB6, R1, R2, G1, G2, B1, B2) \
UNPACK_RGB24_32_STEP2(RGB1, RGB2, RGB3, RGB4, RGB5, RGB6, R1, R2, G1, G2, B1, B2) \
UNPACK_RGB24_32_STEP1(RGB1, RGB2, RGB3, RGB4, RGB5, RGB6, R1, R2, G1, G2, B1, B2) \
UNPACK_RGB24_32_STEP2(RGB1, RGB2, RGB3, RGB4, RGB5, RGB6, R1, R2, G1, G2, B1, B2) \
UNPACK_RGB24_32_STEP1(RGB1, RGB2, RGB3, RGB4, RGB5, RGB6, R1, R2, G1, G2, B1, B2) \

#define RGB2YUV_16(R, G, B, Y, U, V) \
Y = _mm_add_epi16(_mm_mullo_epi16(R, _mm_set1_epi16(param->matrix[0][0])), \
		_mm_mullo_epi16(G, _mm_set1_epi16(param->matrix[0][1]))); \
Y = _mm_add_epi16(Y, _mm_mullo_epi16(B, _mm_set1_epi16(param->matrix[0][2]))); \
Y = _mm_add_epi16(Y, _mm_set1_epi16((param->y_shift)<<PRECISION)); \
Y = _mm_srai_epi16(Y, PRECISION); \
U = _mm_add_epi16(_mm_mullo_epi16(R, _mm_set1_epi16(param->matrix[1][0])), \
		_mm_mullo_epi16(G, _mm_set1_epi16(param->matrix[1][1]))); \
U = _mm_add_epi16(U, _mm_mullo_epi16(B, _mm_set1_epi16(param->matrix[1][2]))); \
U = _mm_add_epi16(U, _mm_set1_epi16(128<<PRECISION)); \
U = _mm_srai_epi16(U, PRECISION); \
V = _mm_add_epi16(_mm_mullo_epi16(R, _mm_set1_epi16(param->matrix[2][0])), \
		_mm_mullo_epi16(G, _mm_set1_epi16(param->matrix[2][1]))); \
V = _mm_add_epi16(V, _mm_mullo_epi16(B, _mm_set1_epi16(param->matrix[2][2]))); \
V = _mm_add_epi16(V, _mm_set1_epi16(128<<PRECISION)); \
V = _mm_srai_epi16(V, PRECISION);

#define RGB2YUV_32 \
	__m128i r1, r2, b1, b2, g1, g2; \
	__m128i r_16, g_16, b_16; \
	__m128i y1_16, y2_16, u1_16, u2_16, v1_16, v2_16, y, u1, u2, v1, v2, u1_tmp, u2_tmp, v1_tmp, v2_tmp; \
	__m128i rgb1 = LOAD_SI128((const __m128i*)(rgb_ptr1)), \
		rgb2 = LOAD_SI128((const __m128i*)(rgb_ptr1+16)), \
		rgb3 = LOAD_SI128((const __m128i*)(rgb_ptr1+32)), \
		rgb4 = LOAD_SI128((const __m128i*)(rgb_ptr2)), \
		rgb5 = LOAD_SI128((const __m128i*)(rgb_ptr2+16)), \
		rgb6 = LOAD_SI128((const __m128i*)(rgb_ptr2+32)); \
	/* unpack rgb24 data to r, g and b data in separate channels*/ \
	UNPACK_RGB24_32(rgb1, rgb2, rgb3, rgb4, rgb5, rgb6, r1, r2, g1, g2, b1, b2) \
	/* process pixels of first line */ \
	r_16 = _mm_unpacklo_epi8(r1, _mm_setzero_si128()); \
	g_16 = _mm_unpacklo_epi8(g1, _mm_setzero_si128()); \
	b_16 = _mm_unpacklo_epi8(b1, _mm_setzero_si128()); \
	RGB2YUV_16(r_16, g_16, b_16, y1_16, u1_16, v1_16) \
	r_16 = _mm_unpackhi_epi8(r1, _mm_setzero_si128()); \
	g_16 = _mm_unpackhi_epi8(g1, _mm_setzero_si128()); \
	b_16 = _mm_unpackhi_epi8(b1, _mm_setzero_si128()); \
	RGB2YUV_16(r_16, g_16, b_16, y2_16, u2_16, v2_16) \
	y = _mm_packus_epi16(y1_16, y2_16); \
	u1 = _mm_packus_epi16(u1_16, u2_16); \
	v1 = _mm_packus_epi16(v1_16, v2_16); \
	/* save Y values */ \
	SAVE_SI128((__m128i*)(y_ptr1), y); \
	/* process pixels of second line */ \
	r_16 = _mm_unpacklo_epi8(r2, _mm_setzero_si128()); \
	g_16 = _mm_unpacklo_epi8(g2, _mm_setzero_si128()); \
	b_16 = _mm_unpacklo_epi8(b2, _mm_setzero_si128()); \
	RGB2YUV_16(r_16, g_16, b_16, y1_16, u1_16, v1_16) \
	r_16 = _mm_unpackhi_epi8(r2, _mm_setzero_si128()); \
	g_16 = _mm_unpackhi_epi8(g2, _mm_setzero_si128()); \
	b_16 = _mm_unpackhi_epi8(b2, _mm_setzero_si128()); \
	RGB2YUV_16(r_16, g_16, b_16, y2_16, u2_16, v2_16) \
	y = _mm_packus_epi16(y1_16, y2_16); \
	u2 = _mm_packus_epi16(u1_16, u2_16); \
	v2 = _mm_packus_epi16(v1_16, v2_16); \
	/* save Y values */ \
	SAVE_SI128((__m128i*)(y_ptr2), y); \
	/* vertical subsampling of u/v values */ \
	u1_tmp = _mm_avg_epu8(u1, u2); \
	v1_tmp = _mm_avg_epu8(v1, v2); \
	/* do the same again with next data */ \
	rgb1 = LOAD_SI128((const __m128i*)(rgb_ptr1+48)); \
	rgb2 = LOAD_SI128((const __m128i*)(rgb_ptr1+64)); \
	rgb3 = LOAD_SI128((const __m128i*)(rgb_ptr1+80)); \
	rgb4 = LOAD_SI128((const __m128i*)(rgb_ptr2+48)); \
	rgb5 = LOAD_SI128((const __m128i*)(rgb_ptr2+64)); \
	rgb6 = LOAD_SI128((const __m128i*)(rgb_ptr2+80)); \
	/* unpack rgb24 data to r, g and b data in separate channels*/ \
	UNPACK_RGB24_32(rgb1, rgb2, rgb3, rgb4, rgb5, rgb6, r1, r2, g1, g2, b1, b2) \
	/* process pixels of first line */ \
	r_16 = _mm_unpacklo_epi8(r1, _mm_setzero_si128()); \
	g_16 = _mm_unpacklo_epi8(g1, _mm_setzero_si128()); \
	b_16 = _mm_unpacklo_epi8(b1, _mm_setzero_si128()); \
	RGB2YUV_16(r_16, g_16, b_16, y1_16, u1_16, v1_16) \
	r_16 = _mm_unpackhi_epi8(r1, _mm_setzero_si128()); \
	g_16 = _mm_unpackhi_epi8(g1, _mm_setzero_si128()); \
	b_16 = _mm_unpackhi_epi8(b1, _mm_setzero_si128()); \
	RGB2YUV_16(r_16, g_16, b_16, y2_16, u2_16, v2_16) \
	y = _mm_packus_epi16(y1_16, y2_16); \
	u1 = _mm_packus_epi16(u1_16, u2_16); \
	v1 = _mm_packus_epi16(v1_16, v2_16); \
	/* save Y values */ \
	SAVE_SI128((__m128i*)(y_ptr1+16), y); \
	/* process pixels of second line */ \
	r_16 = _mm_unpacklo_epi8(r2, _mm_setzero_si128()); \
	g_16 = _mm_unpacklo_epi8(g2, _mm_setzero_si128()); \
	b_16 = _mm_unpacklo_epi8(b2, _mm_setzero_si128()); \
	RGB2YUV_16(r_16, g_16, b_16, y1_16, u1_16, v1_16) \
	r_16 = _mm_unpackhi_epi8(r2, _mm_setzero_si128()); \
	g_16 = _mm_unpackhi_epi8(g2, _mm_setzero_si128()); \
	b_16 = _mm_unpackhi_epi8(b2, _mm_setzero_si128()); \
	RGB2YUV_16(r_16, g_16, b_16, y2_16, u2_16, v2_16) \
	y = _mm_packus_epi16(y1_16, y2_16); \
	u2 = _mm_packus_epi16(u1_16, u2_16); \
	v2 = _mm_packus_epi16(v1_16, v2_16); \
	/* save Y values */ \
	SAVE_SI128((__m128i*)(y_ptr2+16), y); \
	/* vertical subsampling of u/v values */ \
	u2_tmp = _mm_avg_epu8(u1, u2); \
	v2_tmp = _mm_avg_epu8(v1, v2); \
	/* horizontal subsampling of u/v values */ \
	u1 = _mm_packus_epi16(_mm_srl_epi16(u1_tmp, _mm_cvtsi32_si128(8)), _mm_srl_epi16(u2_tmp, _mm_cvtsi32_si128(8))); \
	v1 = _mm_packus_epi16(_mm_srl_epi16(v1_tmp, _mm_cvtsi32_si128(8)), _mm_srl_epi16(v2_tmp, _mm_cvtsi32_si128(8))); \
	u2 = _mm_packus_epi16(_mm_and_si128(u1_tmp, _mm_set1_epi16(0xFF)), _mm_and_si128(u2_tmp, _mm_set1_epi16(0xFF))); \
	v2 = _mm_packus_epi16(_mm_and_si128(v1_tmp, _mm_set1_epi16(0xFF)), _mm_and_si128(v2_tmp, _mm_set1_epi16(0xFF))); \
	u1 = _mm_avg_epu8(u1, u2); \
	v1 = _mm_avg_epu8(v1, v2); \
	SAVE_SI128((__m128i*)(u_ptr), u1); \
	SAVE_SI128((__m128i*)(v_ptr), v1);

void rgb24_yuv420_sse(uint32_t width, uint32_t height, 
	const uint8_t *RGB, uint32_t RGB_stride, 
	uint8_t *Y, uint8_t *U, uint8_t *V, uint32_t Y_stride, uint32_t UV_stride, 
	YCbCrType yuv_type)
{
	#define LOAD_SI128 _mm_load_si128
	#define SAVE_SI128 _mm_stream_si128
	const RGB2YUVParam *const param = &(RGB2YUV[yuv_type]);
	
	uint32_t xpos, ypos;
	for(ypos=0; ypos<(height-1); ypos+=2)
	{
		const uint8_t *rgb_ptr1=RGB+ypos*RGB_stride,
			*rgb_ptr2=RGB+(ypos+1)*RGB_stride;
		
		uint8_t *y_ptr1=Y+ypos*Y_stride,
			*y_ptr2=Y+(ypos+1)*Y_stride,
			*u_ptr=U+(ypos/2)*UV_stride,
			*v_ptr=V+(ypos/2)*UV_stride;
		
		for(xpos=0; xpos<(width-31); xpos+=32)
		{
			RGB2YUV_32
			
			rgb_ptr1+=96;
			rgb_ptr2+=96;
			y_ptr1+=32;
			y_ptr2+=32;
			u_ptr+=16; 
			v_ptr+=16;
		}
	}
	#undef LOAD_SI128
	#undef SAVE_SI128
}

void rgb24_yuv420_sseu(uint32_t width, uint32_t height, 
	const uint8_t *RGB, uint32_t RGB_stride, 
	uint8_t *Y, uint8_t *U, uint8_t *V, uint32_t Y_stride, uint32_t UV_stride, 
	YCbCrType yuv_type)
{
	#define LOAD_SI128 _mm_loadu_si128
	#define SAVE_SI128 _mm_storeu_si128
	const RGB2YUVParam *const param = &(RGB2YUV[yuv_type]);
	
	uint32_t xpos, ypos;
	for(ypos=0; ypos<(height-1); ypos+=2)
	{
		const uint8_t *rgb_ptr1=RGB+ypos*RGB_stride,
			*rgb_ptr2=RGB+(ypos+1)*RGB_stride;
		
		uint8_t *y_ptr1=Y+ypos*Y_stride,
			*y_ptr2=Y+(ypos+1)*Y_stride,
			*u_ptr=U+(ypos/2)*UV_stride,
			*v_ptr=V+(ypos/2)*UV_stride;
		
		for(xpos=0; xpos<(width-31); xpos+=32)
		{
			RGB2YUV_32
			
			rgb_ptr1+=96;
			rgb_ptr2+=96;
			y_ptr1+=32;
			y_ptr2+=32;
			u_ptr+=16; 
			v_ptr+=16;
		}
	}
	#undef LOAD_SI128
	#undef SAVE_SI128
}


#endif //__SSE2__

