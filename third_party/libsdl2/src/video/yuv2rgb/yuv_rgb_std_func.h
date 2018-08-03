// Copyright 2016 Adrien Descamps
// Distributed under BSD 3-Clause License

/* You need to define the following macros before including this file:
	STD_FUNCTION_NAME
	YUV_FORMAT
	RGB_FORMAT
*/

#if RGB_FORMAT == RGB_FORMAT_RGB565

#define PACK_PIXEL(rgb_ptr) \
	*(Uint16 *)rgb_ptr = \
		((((Uint16)clampU8(y_tmp+r_tmp)) << 8 ) & 0xF800) | \
		((((Uint16)clampU8(y_tmp+g_tmp)) << 3) & 0x07E0) | \
		(((Uint16)clampU8(y_tmp+b_tmp)) >> 3); \
	rgb_ptr += 2; \

#elif RGB_FORMAT == RGB_FORMAT_RGB24

#define PACK_PIXEL(rgb_ptr) \
	rgb_ptr[0] = clampU8(y_tmp+r_tmp); \
	rgb_ptr[1] = clampU8(y_tmp+g_tmp); \
	rgb_ptr[2] = clampU8(y_tmp+b_tmp); \
	rgb_ptr += 3; \

#elif RGB_FORMAT == RGB_FORMAT_RGBA

#define PACK_PIXEL(rgb_ptr) \
	*(Uint32 *)rgb_ptr = \
		(((Uint32)clampU8(y_tmp+r_tmp)) << 24) | \
		(((Uint32)clampU8(y_tmp+g_tmp)) << 16) | \
		(((Uint32)clampU8(y_tmp+b_tmp)) << 8) | \
		0x000000FF; \
	rgb_ptr += 4; \

#elif RGB_FORMAT == RGB_FORMAT_BGRA

#define PACK_PIXEL(rgb_ptr) \
	*(Uint32 *)rgb_ptr = \
		(((Uint32)clampU8(y_tmp+b_tmp)) << 24) | \
		(((Uint32)clampU8(y_tmp+g_tmp)) << 16) | \
		(((Uint32)clampU8(y_tmp+r_tmp)) << 8) | \
		0x000000FF; \
	rgb_ptr += 4; \

#elif RGB_FORMAT == RGB_FORMAT_ARGB

#define PACK_PIXEL(rgb_ptr) \
	*(Uint32 *)rgb_ptr = \
		0xFF000000 | \
		(((Uint32)clampU8(y_tmp+r_tmp)) << 16) | \
		(((Uint32)clampU8(y_tmp+g_tmp)) << 8) | \
		(((Uint32)clampU8(y_tmp+b_tmp)) << 0); \
	rgb_ptr += 4; \

#elif RGB_FORMAT == RGB_FORMAT_ABGR

#define PACK_PIXEL(rgb_ptr) \
	*(Uint32 *)rgb_ptr = \
		0xFF000000 | \
		(((Uint32)clampU8(y_tmp+b_tmp)) << 16) | \
		(((Uint32)clampU8(y_tmp+g_tmp)) << 8) | \
		(((Uint32)clampU8(y_tmp+r_tmp)) << 0); \
	rgb_ptr += 4; \

#else
#error PACK_PIXEL unimplemented
#endif


void STD_FUNCTION_NAME(
	uint32_t width, uint32_t height, 
	const uint8_t *Y, const uint8_t *U, const uint8_t *V, uint32_t Y_stride, uint32_t UV_stride, 
	uint8_t *RGB, uint32_t RGB_stride, 
	YCbCrType yuv_type)
{
	const YUV2RGBParam *const param = &(YUV2RGB[yuv_type]);
#if YUV_FORMAT == YUV_FORMAT_420
	const int y_pixel_stride = 1;
	const int uv_pixel_stride = 1;
	const int uv_x_sample_interval = 2;
	const int uv_y_sample_interval = 2;
#elif YUV_FORMAT == YUV_FORMAT_422
	const int y_pixel_stride = 2;
	const int uv_pixel_stride = 4;
	const int uv_x_sample_interval = 2;
	const int uv_y_sample_interval = 1;
#elif YUV_FORMAT == YUV_FORMAT_NV12
	const int y_pixel_stride = 1;
	const int uv_pixel_stride = 2;
	const int uv_x_sample_interval = 2;
	const int uv_y_sample_interval = 2;
#endif

	uint32_t x, y;
	for(y=0; y<(height-(uv_y_sample_interval-1)); y+=uv_y_sample_interval)
	{
		const uint8_t *y_ptr1=Y+y*Y_stride,
			*y_ptr2=Y+(y+1)*Y_stride,
			*u_ptr=U+(y/uv_y_sample_interval)*UV_stride,
			*v_ptr=V+(y/uv_y_sample_interval)*UV_stride;
		
		uint8_t *rgb_ptr1=RGB+y*RGB_stride,
			*rgb_ptr2=RGB+(y+1)*RGB_stride;
		
		for(x=0; x<(width-(uv_x_sample_interval-1)); x+=uv_x_sample_interval)
		{
			// Compute U and V contributions, common to the four pixels
			
			int32_t u_tmp = ((*u_ptr)-128);
			int32_t v_tmp = ((*v_ptr)-128);
			
			int32_t r_tmp = (v_tmp*param->v_r_factor);
			int32_t g_tmp = (u_tmp*param->u_g_factor + v_tmp*param->v_g_factor);
			int32_t b_tmp = (u_tmp*param->u_b_factor);
			
			// Compute the Y contribution for each pixel
			
			int32_t y_tmp = ((y_ptr1[0]-param->y_shift)*param->y_factor);
			PACK_PIXEL(rgb_ptr1);
			
			y_tmp = ((y_ptr1[y_pixel_stride]-param->y_shift)*param->y_factor);
			PACK_PIXEL(rgb_ptr1);
			
			if (uv_y_sample_interval > 1) {
				y_tmp = ((y_ptr2[0]-param->y_shift)*param->y_factor);
				PACK_PIXEL(rgb_ptr2);
				
				y_tmp = ((y_ptr2[y_pixel_stride]-param->y_shift)*param->y_factor);
				PACK_PIXEL(rgb_ptr2);
			}

			y_ptr1+=2*y_pixel_stride;
			y_ptr2+=2*y_pixel_stride;
			u_ptr+=2*uv_pixel_stride/uv_x_sample_interval;
			v_ptr+=2*uv_pixel_stride/uv_x_sample_interval;
		}

		/* Catch the last pixel, if needed */
		if (uv_x_sample_interval == 2 && x == (width-1))
		{
			// Compute U and V contributions, common to the four pixels
			
			int32_t u_tmp = ((*u_ptr)-128);
			int32_t v_tmp = ((*v_ptr)-128);
			
			int32_t r_tmp = (v_tmp*param->v_r_factor);
			int32_t g_tmp = (u_tmp*param->u_g_factor + v_tmp*param->v_g_factor);
			int32_t b_tmp = (u_tmp*param->u_b_factor);
			
			// Compute the Y contribution for each pixel
			
			int32_t y_tmp = ((y_ptr1[0]-param->y_shift)*param->y_factor);
			PACK_PIXEL(rgb_ptr1);
			
			if (uv_y_sample_interval > 1) {
				y_tmp = ((y_ptr2[0]-param->y_shift)*param->y_factor);
				PACK_PIXEL(rgb_ptr2);
			}
		}
	}

	/* Catch the last line, if needed */
	if (uv_y_sample_interval == 2 && y == (height-1))
	{
		const uint8_t *y_ptr1=Y+y*Y_stride,
			*u_ptr=U+(y/uv_y_sample_interval)*UV_stride,
			*v_ptr=V+(y/uv_y_sample_interval)*UV_stride;
		
		uint8_t *rgb_ptr1=RGB+y*RGB_stride;
		
		for(x=0; x<(width-(uv_x_sample_interval-1)); x+=uv_x_sample_interval)
		{
			// Compute U and V contributions, common to the four pixels
			
			int32_t u_tmp = ((*u_ptr)-128);
			int32_t v_tmp = ((*v_ptr)-128);
			
			int32_t r_tmp = (v_tmp*param->v_r_factor);
			int32_t g_tmp = (u_tmp*param->u_g_factor + v_tmp*param->v_g_factor);
			int32_t b_tmp = (u_tmp*param->u_b_factor);
			
			// Compute the Y contribution for each pixel
			
			int32_t y_tmp = ((y_ptr1[0]-param->y_shift)*param->y_factor);
			PACK_PIXEL(rgb_ptr1);
			
			y_tmp = ((y_ptr1[y_pixel_stride]-param->y_shift)*param->y_factor);
			PACK_PIXEL(rgb_ptr1);
			
			y_ptr1+=2*y_pixel_stride;
			u_ptr+=2*uv_pixel_stride/uv_x_sample_interval;
			v_ptr+=2*uv_pixel_stride/uv_x_sample_interval;
		}

		/* Catch the last pixel, if needed */
		if (uv_x_sample_interval == 2 && x == (width-1))
		{
			// Compute U and V contributions, common to the four pixels
			
			int32_t u_tmp = ((*u_ptr)-128);
			int32_t v_tmp = ((*v_ptr)-128);
			
			int32_t r_tmp = (v_tmp*param->v_r_factor);
			int32_t g_tmp = (u_tmp*param->u_g_factor + v_tmp*param->v_g_factor);
			int32_t b_tmp = (u_tmp*param->u_b_factor);
			
			// Compute the Y contribution for each pixel
			
			int32_t y_tmp = ((y_ptr1[0]-param->y_shift)*param->y_factor);
			PACK_PIXEL(rgb_ptr1);
		}
	}
}

#undef STD_FUNCTION_NAME
#undef YUV_FORMAT
#undef RGB_FORMAT
#undef PACK_PIXEL
