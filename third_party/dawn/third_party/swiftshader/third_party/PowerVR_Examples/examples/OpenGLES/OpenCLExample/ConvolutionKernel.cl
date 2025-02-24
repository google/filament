/*
 *   Caching sampling pattern:
 *
 *   |---|---|---|---|---|---|---|---|---|---|
 *   | o | o | o | o | o | o | o | o | o | o |
 *   |---|---|---|---|---|---|---|---|---|---|
 *   | x | x | x |   |   |   |   |   |   | o |  this example shows two filter masks applied at the same time (denoted as X and Y)
 *   |---|---|---|---|---|---|---|---|---|---|
 *   | x | X |x/y| y | y |   |   |   |   | o |  the pre-fetch allows the sharing of samples, effectively reducing the bandwidth requirements
 *   |---|---|---|---|---|---|---|---|---|---|
 *   | x | x |x/y| Y | y |   |   |   |   | o |
 *   |---|---|---|---|---|---|---|---|---|---|
 *   | o |   | y | y | y |   |   |   |   | o |
 *   |---|---|---|---|---|---|---|---|---|---|
 *   | o | o | o | o | o | o | o | o | o | o |
 *   |---|---|---|---|---|---|---|---|---|---|
 *
 *   The 3x3 kernels (larger ones are similar) for work group sizes of 32 use the same sampling pattern:
 *    - the area that is processed per work group is 8x4 pixels
 *    - it is required to fetch a 1x1 pixel border around the area that is to be processed,
 *      effectively increasing the area to 10x6 (equals 60 samples)
 */


/*
 *   At the beginning of each kernel the 60 samples are collected in a flat array, distributing the
 *   sampling workload among as many work items as possible.
 *
 *   The numbers (x/y) indicate the individual work items that issue the texture fetch for the specified location:
 *
 *   |---|---|---|---|---|---|---|---|---|---|
 *   |0/0|0/0|0/0|0/0|0/0|0/1|0/1|0/1|0/1|0/1|
 *   |---|---|---|---|---|---|---|---|---|---|
 *   |1/0|1/0|1/0|1/0|1/0|1/1|1/1|1/1|1/1|1/1|
 *   |---|---|---|---|---|---|---|---|---|---|
 *   |2/0|2/0|2/0|2/0|2/0|2/1|2/1|2/1|2/1|2/1|
 *   |---|---|---|---|---|---|---|---|---|---|
 *   |3/0|3/0|3/0|3/0|3/0|3/1|3/1|3/1|3/1|3/1|
 *   |---|---|---|---|---|---|---|---|---|---|
 *   |4/0|4/0|4/0|4/0|4/0|4/1|4/1|4/1|4/1|4/1|
 *   |---|---|---|---|---|---|---|---|---|---|
 *   |5/0|5/0|5/0|5/0|5/0|5/1|5/1|5/1|5/1|5/1|
 *   |---|---|---|---|---|---|---|---|---|---|
 *
 */
inline void prefetch_texture_samples_8x4(__read_only image2d_t srcImage, sampler_t sampler, __local float3* colors)
{
	const int2 grid = (int2)(get_group_id(0) * 8, get_group_id(1) * 4);
	const int2 lid = (int2)(get_local_id(0), get_local_id(1));

	if (lid.x < 6 && lid.y < 2)
	{
		// Pre-fetch required colour samples
		const int2 lid_offset = lid * (int2)(10, 5);
		colors[lid_offset.x + lid_offset.y] =     read_imagef(srcImage, sampler, grid + (int2)(lid_offset.y - 1, lid.x - 1)).xyz;
		colors[lid_offset.x + lid_offset.y + 1] = read_imagef(srcImage, sampler, grid + (int2)(lid_offset.y, lid.x - 1)).xyz;
		colors[lid_offset.x + lid_offset.y + 2] = read_imagef(srcImage, sampler, grid + (int2)(lid_offset.y + 1, lid.x - 1)).xyz;
		colors[lid_offset.x + lid_offset.y + 3] = read_imagef(srcImage, sampler, grid + (int2)(lid_offset.y + 2, lid.x - 1)).xyz;
		colors[lid_offset.x + lid_offset.y + 4] = read_imagef(srcImage, sampler, grid + (int2)(lid_offset.y + 3, lid.x - 1)).xyz;
	}

	// wait for all work-items to finish
	barrier(CLK_LOCAL_MEM_FENCE);
}

/*
 * Picks the highest luminance value in a 3x3 surrounding and
 * writes the corresponding colour as result.
 */
__attribute__((reqd_work_group_size(8, 4, 1)))
__kernel void erode_3x3(__read_only image2d_t srcImage, __write_only image2d_t dstImage, sampler_t sampler)
{
	const int2 lid = (int2)(get_local_id(0), get_local_id(1));

	__local float3 colors[60];
	prefetch_texture_samples_8x4(srcImage, sampler, colors);

	// Convert all color values to luminance in parallel
	const float3 rgb_to_lum = (float3)(0.3f, 0.59f, 0.11f);

	__local float luminance[60];
	if (lid.x < 6 && lid.y < 2)
	{
		const int2 lid_offset = lid * (int2)(10, 5);
		int cache_id = lid_offset.x + lid_offset.y;
		luminance[cache_id] = dot(rgb_to_lum, colors[cache_id]);
		luminance[cache_id + 1] = dot(rgb_to_lum, colors[cache_id + 1]);
		luminance[cache_id + 2] = dot(rgb_to_lum, colors[cache_id + 2]);
		luminance[cache_id + 3] = dot(rgb_to_lum, colors[cache_id + 3]);
		luminance[cache_id + 4] = dot(rgb_to_lum, colors[cache_id + 4]);
	}

	// wait for all work-items to finish
	barrier(CLK_LOCAL_MEM_FENCE);

	//Find which pixel has min brightness from the nine neighbouring pixels ([0,0] -> [2,2])

	// Our cache area is 8x4. ++idx moves to the next pixel. idx+=8 is a simplification that moves
	// from the last item of a line our 3x3 neighbourhood (say, 0,2) to the first item of the same
	//line, since +10 moves one row down, and -2 moves to columns back.

	short min_idx = lid.y * 10 + lid.x;  //Assume 0,0
	short idx = min_idx;			//test vs 0,1
	if (luminance[idx + 1] > luminance[min_idx]) { min_idx = idx + 1; }
	if (luminance[idx + 2] > luminance[min_idx]) { min_idx = idx + 2; } //test vs 0,2
	if (luminance[idx + 10] > luminance[min_idx]) { min_idx = idx + 10; } //test vs 1,0
	if (luminance[idx + 11] > luminance[min_idx]) { min_idx = idx + 11; }//test vs 1,1
	if (luminance[idx + 12] > luminance[min_idx]) { min_idx = idx + 12; }//test vs 1,2
	if (luminance[idx + 20] > luminance[min_idx]) { min_idx = idx + 20; }//test vs 2,0
	if (luminance[idx + 21] > luminance[min_idx]) { min_idx = idx + 21; }//test vs 2,1
	if (luminance[idx + 22] > luminance[min_idx]) { min_idx = idx + 22; }//test vs 2,2

	const int2 gid = (int2)(get_group_id(0) * 8, get_group_id(1) * 4);
	write_imagef(dstImage, gid + lid, (float4)(colors[min_idx], 1.0f));
}

/*
 * Picks the average of the neighbouring pixels
 */
__attribute__((reqd_work_group_size(8, 4, 1)))
__kernel void box_3x3(__read_only image2d_t srcImage, __write_only image2d_t dstImage, sampler_t sampler)
{
	const int2 gid = (int2)(get_group_id(0) * 8, get_group_id(1) * 4);
	const int2 lid = (int2)(get_local_id(0), get_local_id(1));

	__local float3 colors[60];
	prefetch_texture_samples_8x4(srcImage, sampler, colors);

	int offset = lid.y * 10 + lid.x;
	float3 color = 0.f;
	color +=  colors[offset + 00]*1.f + colors[offset + 01]*1.f + colors[offset + 02]*1.f;
	color +=  colors[offset + 10]*1.f + colors[offset + 11]*1.f + colors[offset + 12]*1.f;
	color +=  colors[offset + 20]*1.f + colors[offset + 21]*1.f + colors[offset + 22]*1.f;
	color /= 9.0f;

	write_imagef(dstImage, gid + lid, (float4)(color, 1.0f));
}

__attribute__((reqd_work_group_size(8, 4, 1)))
__kernel void copy(__read_only image2d_t srcImage, __write_only image2d_t dstImage, sampler_t sampler)
{
	const int2 globid = (int2)(get_global_id(0), get_global_id(1));
	write_imagef(dstImage, globid, read_imagef(srcImage, sampler, globid));
}



/*
 * Picks the lowest luminance value in a 3x3 surrounding and
 * writes the corresponding colour as result.
 */
__attribute__((reqd_work_group_size(8, 4, 1)))
__kernel void dilate_3x3(__read_only image2d_t srcImage, __write_only image2d_t dstImage, sampler_t sampler)
{
	const int2 lid = (int2)(get_local_id(0), get_local_id(1));

	__local float3 colors[60];
	prefetch_texture_samples_8x4(srcImage, sampler, colors);

	// Convert all color values to luminance in parallel
	const float3 rgb_to_lum = (float3)(0.3f, 0.59f, 0.11f);

	__local float luminance[60];
	if (lid.x < 6 && lid.y < 2)
	{
		const int2 lid_offset = lid * (int2)(10, 5);
		luminance[lid_offset.x + lid_offset.y] = dot(rgb_to_lum, colors[lid_offset.x + lid_offset.y]);
		luminance[lid_offset.x + lid_offset.y + 1] = dot(rgb_to_lum, colors[lid_offset.x + lid_offset.y + 1]);
		luminance[lid_offset.x + lid_offset.y + 2] = dot(rgb_to_lum, colors[lid_offset.x + lid_offset.y + 2]);
		luminance[lid_offset.x + lid_offset.y + 3] = dot(rgb_to_lum, colors[lid_offset.x + lid_offset.y + 3]);
		luminance[lid_offset.x + lid_offset.y + 4] = dot(rgb_to_lum, colors[lid_offset.x + lid_offset.y + 4]);
	}

	// wait for all work-items to finish
	barrier(CLK_LOCAL_MEM_FENCE);

	short max_idx = lid.y * 10 + lid.x;  //Assume 0,0
	short idx = max_idx;			//test vs 0,1
	//PREFER THIS FORMAT OF INDEXING ( array[CONSTANT + OFFSET]. Additions commonly free while indexing.)
	if (luminance[idx + 1] < luminance[max_idx]) { max_idx = idx + 1; } 
	if (luminance[idx + 2] < luminance[max_idx]) { max_idx = idx + 2; } //test vs 0,2
	if (luminance[idx + 10] < luminance[max_idx]) { max_idx = idx + 10; } //test vs 1,0
	if (luminance[idx + 11] < luminance[max_idx]) { max_idx = idx + 11; }//test vs 1,1
	if (luminance[idx + 12] < luminance[max_idx]) { max_idx = idx + 12; }//test vs 1,2
	if (luminance[idx + 20] < luminance[max_idx]) { max_idx = idx + 20; }//test vs 2,0
	if (luminance[idx + 21] < luminance[max_idx]) { max_idx = idx + 21; }//test vs 2,1
	if (luminance[idx + 22] < luminance[max_idx]) { max_idx = idx + 22; }//test vs 2,2

	const int2 gid = (int2)(get_group_id(0) * 8, get_group_id(1) * 4);
	write_imagef(dstImage, gid + lid, (float4)(colors[max_idx], 1.0f));
}


/*
 *  Simple edge detection
 */
__attribute__((reqd_work_group_size(8, 4, 1)))
__kernel void edgedetect_3x3(__read_only image2d_t srcImage, __write_only image2d_t dstImage, sampler_t sampler)
{
	const int2 gid = (int2)(get_group_id(0) * 8, get_group_id(1) * 4);
	const int2 lid = (int2)(get_local_id(0), get_local_id(1));

	__local float3 colors[60];
	prefetch_texture_samples_8x4(srcImage, sampler, colors);

	int offset = lid.y * 10 + lid.x;
	float3 color = colors[offset + 00] * -1.0f + colors[offset + 01] * -1.0f + colors[offset + 02] * -1.0f;
	color +=       colors[offset + 10] * -1.0f + colors[offset + 11] *  8.0f + colors[offset + 12] * -1.0f;
	color +=       colors[offset + 20] * -1.0f + colors[offset + 21] * -1.0f + colors[offset + 22] * -1.0f;

	write_imagef(dstImage, gid + lid, (float4)(color, 1.0f));
}

/*
 *  Sobel edge-detection
 */
__attribute__((reqd_work_group_size(8, 4, 1)))
__kernel void sobel_3x3(__read_only image2d_t srcImage,
                        __write_only image2d_t dstImage,
                        sampler_t sampler)
{
	const int2 gid = (int2)(get_group_id(0) * 8, get_group_id(1) * 4);
	const int2 lid = (int2)(get_local_id(0), get_local_id(1));

	// Convert all color values to luminance in parallel
	const float3 rgb_to_lum = (float3)(0.3f, 0.59f, 0.11f);

	__local float luminance[60];
	if (lid.x < 6 && lid.y < 2)
	{
		const int2 lid_offset = lid * (int2)(10, 5);
		int cache_id = lid_offset.x + lid_offset.y;
		luminance[cache_id]     = dot(rgb_to_lum, read_imagef(srcImage, sampler, gid + (int2)(lid_offset.y - 1, lid.x - 1)).xyz);
		luminance[cache_id + 1] = dot(rgb_to_lum, read_imagef(srcImage, sampler, gid + (int2)(lid_offset.y, lid.x - 1)).xyz);
		luminance[cache_id + 2] = dot(rgb_to_lum, read_imagef(srcImage, sampler, gid + (int2)(lid_offset.y + 1, lid.x - 1)).xyz);
		luminance[cache_id + 3] = dot(rgb_to_lum, read_imagef(srcImage, sampler, gid + (int2)(lid_offset.y + 2, lid.x - 1)).xyz);
		luminance[cache_id + 4] = dot(rgb_to_lum, read_imagef(srcImage, sampler, gid + (int2)(lid_offset.y + 3, lid.x - 1)).xyz);
	}

	barrier(CLK_LOCAL_MEM_FENCE);
	int offset = lid.y * 10 + lid.x;

	// horizontal filter
	float lumx = luminance[offset] * -1.0f + luminance[offset + 1] * -2.0f + luminance[offset + 2] * -1.0f;
	lumx      += luminance[offset + 20] + luminance[offset + 21] * 2.0f + luminance[offset + 22];

	
	// vertical filter
	float lumy = luminance[offset] * -1.0f + luminance[offset + 2];
	lumy      += luminance[offset + 10] * -2.0f + luminance[offset + 12] * 2.0f;
	lumy      += luminance[offset + 20] * -1.0f + luminance[offset + 22] * 1.0f;

	float lum = lumy*lumy + lumx*lumx; // This should normaly be sqrt'd but a square has a bit more contrast.
	write_imagef(dstImage, gid + lid, (float4)((float3)(lum), 1.0f));
}


/*
 *  Gaussian image smoothing
 */
__attribute__((reqd_work_group_size(8, 4, 1)))
__kernel void gaussian_3x3(__read_only image2d_t srcImage, __write_only image2d_t dstImage, sampler_t sampler)
{
	const int2 gid = (int2)(get_group_id(0) * 8, get_group_id(1) * 4);
	const int2 lid = (int2)(get_local_id(0), get_local_id(1));

	__local float3 colors[60];
	prefetch_texture_samples_8x4(srcImage, sampler, colors);

	int offset = lid.y * 10 + lid.x;
	float3 color;
	color = colors[offset] * 0.0625f + colors[offset + 1] * 0.125f + colors[offset + 2] * 0.0625f;
	color += colors[offset + 10] * 0.125f + colors[offset + 11] * 0.25f + colors[offset + 12] * 0.125f;
	color += colors[offset + 20] * 0.0625f + colors[offset + 21] * 0.125f + colors[offset + 22] * 0.0625f;

	write_imagef(dstImage, gid + lid, (float4)(color, 1.0f));
}


/*
 *  Emboss effect filter
 */
__attribute__((reqd_work_group_size(8, 4, 1)))
__kernel void emboss_3x3(__read_only image2d_t srcImage, __write_only image2d_t dstImage, sampler_t sampler)
{
	const int2 gid = (int2)(get_group_id(0) * 8, get_group_id(1) * 4);
	const int2 lid = (int2)(get_local_id(0), get_local_id(1));

	__local float3 colors[60];
	prefetch_texture_samples_8x4(srcImage, sampler, colors);

	int offset = lid.y * 10 + lid.x;
	float3 color;
	color = colors[offset] * -2.0f + colors[offset + 1] * -1.0f;
	color += colors[offset + 10] * -1.0f + colors[offset + 11] * 1.0f + colors[offset + 12] * 1.0f;
	color += colors[offset + 21] * 1.0f + colors[offset + 22] * 2.0f;

	write_imagef(dstImage, gid + lid, (float4)(color, 1.0f));
}

/*
 *  Image sharpening filter
 */
__kernel void sharpen_3x3(__read_only image2d_t srcImage, __write_only image2d_t dstImage, sampler_t sampler)
{
	const int2 gid = (int2)(get_group_id(0) * 8, get_group_id(1) * 4);
	const int2 lid = (int2)(get_local_id(0), get_local_id(1));

	__local float3 colors[60];
	prefetch_texture_samples_8x4(srcImage, sampler, colors);

	int offset = lid.y * 10 + lid.x;
	float3 color;
	color = colors[offset + 11] * -1.0f;
	color += colors[offset + 10] * -1.0f + colors[offset + 11] * 5.0f + colors[offset + 12] * -1.0f;
	color += colors[offset + 21] * -1.0f;

	write_imagef(dstImage, gid + lid, (float4)(color, 1.0f));
}
