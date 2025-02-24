#version 450

/*********** INPUT DEFINES: THESE MUST BE SELECTED AND DEFINED BY THE PROGRAM ***********/
/*
IMAGE_BINDING_INPUT  : Image unit used for the texture that contains the current generation
IMAGE_BINDING_OUTPUT : Image unit used for the texture that contains the current generation
WG_WIDTH			 : Dimension X of the workgroup size
WG_HEIGHT			 : Dimension Y of the workgroup size
*/
/*********************************** END INPUT DEFINES **********************************/

// These defines determine local memory requirement ("cache" in this shader) depending on Workgroup size
#define WG_WIDTH 8
#define WG_HEIGHT 4

uniform layout(binding = 0) sampler2D imageIn;
uniform layout(r8, binding = 1) writeonly image2D imageOut;

shared int cache[WG_HEIGHT + 2][WG_WIDTH + 2]; // This should be 10 * 6

// In general, if we can use #defines that are passed to the shader at compile time we can achieve far better
// flexibility to optimize for a specific platform
layout(local_size_x = WG_WIDTH, local_size_y = WG_HEIGHT) in;
void main()
{
	vec2 sz = textureSize(imageIn, 0);

	ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
	ivec2 lid = ivec2(gl_LocalInvocationID.xy);

	if (gid.x >= sz.x || gid.y >= sz.y) { return; }

	if (lid.x < 5 && lid.y < 3) 
	{
		vec2 grpid = vec2(gl_WorkGroupID.xy * gl_WorkGroupSize.xy);
		ivec2 lid2 = lid * 2;

		vec2 texCoords = (grpid + lid2) / (sz);

		vec4 texGat = textureGather(imageIn, texCoords, 0); 

		cache[lid2.y    ][lid2.x    ] = int(texGat.w);
		cache[lid2.y    ][lid2.x + 1] = int(texGat.z);
		cache[lid2.y + 1][lid2.x    ] = int(texGat.x);
		cache[lid2.y + 1][lid2.x + 1] = int(texGat.y);
	}

	barrier();

	int lidx = lid.x + 1;
	int lidy = lid.y + 1;

	int nNeighbours = 
		(cache[lidy - 1][lidx - 1]) + (cache[lidy - 1][lidx]) + (cache[lidy - 1][lidx + 1]) + 
		(cache[lidy    ][lidx - 1]) +                           (cache[lidy    ][lidx + 1]) +
		(cache[lidy + 1][lidx - 1]) + (cache[lidy + 1][lidx]) + (cache[lidy + 1][lidx + 1]);

	 int self = cache[lidy][lidx]; 

	// OPTIMIZED Expression : the slightly faster following expression (eliminate 3 branches in favour of a
	// mix and two convert to float).
	 int nextState = int(mix(nNeighbours == 3, nNeighbours >= 2 && nNeighbours <= 3, bool(self)));

	imageStore(imageOut, gid, vec4(nextState, 0., 0., 1.));
}
