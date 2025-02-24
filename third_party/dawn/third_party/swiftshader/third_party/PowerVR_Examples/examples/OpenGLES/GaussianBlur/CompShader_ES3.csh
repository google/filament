#version 310 es

/*********** INPUT defines: THESE MUST BE SELECTED AND DEFINED BY THE PROGRAM ***********
IMAGE_BINDING_INPUT   : Image unit used for the texture that contains the current generation
IMAGE_BINDING_OUTPUT  : Image unit used for the texture that contains the current generation
*********************************** END INPUT defines **********************************/

uniform layout(rgba8, binding = 0) readonly mediump image2D imageIn;
uniform layout(rgba8, binding = 1) writeonly mediump image2D imageOut;

const int GaussianKernelSize = 19;
const int WeightArraySize = GaussianKernelSize * 2;
const int HalfKernelIndex = int((float(GaussianKernelSize) - 1.0) * 0.5);

const mediump float Weights[] = float[]( 
	0.000003814697266, 0.000068664550781, 0.000583648681641, 0.003112792968750, 0.011672973632813, 0.032684326171875, 0.070816040039063, 0.121398925781250, 0.166923522949219, 
	0.185470581054688, 0.166923522949219, 0.121398925781250, 0.070816040039063, 0.032684326171875, 0.011672973632813, 0.003112792968750, 0.000583648681641, 0.000068664550781,
	0.000003814697266,

	0.000003814697266, 0.000068664550781, 0.000583648681641, 0.003112792968750, 0.011672973632813, 0.032684326171875, 0.070816040039063, 0.121398925781250, 0.166923522949219,
	0.185470581054688, 0.166923522949219, 0.121398925781250, 0.070816040039063, 0.032684326171875, 0.011672973632813, 0.003112792968750, 0.000583648681641, 0.000068664550781,
	0.000003814697266
);

layout(local_size_x = 32, local_size_y = 1) in;

void main()
{
	ivec2 image_bounds = ivec2(imageSize(imageIn));

	int fptr = 0; // Counter tracking the last item in our cache
	int x = 0; // Counter (Calculated from fpts) used to find which item in the weights array each colour in the cache corresponds to

	mediump vec3 f[GaussianKernelSize]; // This is our per-row cache of colours. We use the previous numbers to check what we must load or discard.

	int row = int(gl_GlobalInvocationID.x);

	f[0] = imageLoad(imageIn, ivec2(0, row)).xyz; // We load the first item multiple times so that we don't need special code in the loop.
	f[1] = f[0];
	f[2] = f[0];
	f[3] = f[0];
	f[4] = f[0];
	f[5] = f[0];
	f[6] = f[0];
	f[7] = f[0];
	f[8] = f[0];
	f[9] = f[0];
	f[10] = imageLoad(imageIn, ivec2(1, row)).xyz;
	f[11] = imageLoad(imageIn, ivec2(2, row)).xyz;
	f[12] = imageLoad(imageIn, ivec2(3, row)).xyz;
	f[13] = imageLoad(imageIn, ivec2(4, row)).xyz;
	f[14] = imageLoad(imageIn, ivec2(5, row)).xyz;
	f[15] = imageLoad(imageIn, ivec2(6, row)).xyz;
	f[16] = imageLoad(imageIn, ivec2(7, row)).xyz;
	f[17] = imageLoad(imageIn, ivec2(8, row)).xyz;
	f[18] = imageLoad(imageIn, ivec2(9, row)).xyz;

	// Scan the image left-to-right (one invocation must be launched for each row of the image)
	for (int column = 0; column < image_bounds.x; ++column)
	{
		// Average the colour. X tracks the which item in the weight array corresponds to each item in the color array
		// IMPORTANT NOTE: We do not need bounds checking or expensive modulo operations here, because we have made sure
		// the weight array is doubled.
		mediump vec3 col = f[0] * Weights[x];
		col += f[1] * Weights[1 + x];
		col += f[2] * Weights[2 + x];
		col += f[3] * Weights[3 + x];
		col += f[4] * Weights[4 + x];
		col += f[5] * Weights[5 + x];
		col += f[6] * Weights[6 + x];
		col += f[7] * Weights[7 + x];
		col += f[8] * Weights[8 + x];
		col += f[9] * Weights[9 + x];
		col += f[10] * Weights[10 + x];
		col += f[11] * Weights[11 + x];
		col += f[12] * Weights[12 + x];
		col += f[13] * Weights[13 + x];
		col += f[14] * Weights[14 + x];
		col += f[15] * Weights[15 + x];
		col += f[16] * Weights[16 + x];
		col += f[17] * Weights[17 + x];
		col += f[18] * Weights[18 + x];

		imageStore(imageOut, ivec2(column, row), vec4(col, 0.0)); // STORE INTO THE OUTPUT IMAGE.

		int column_to_sample = column + HalfKernelIndex + 1; // Select the next texel to load.
		column_to_sample = min(column_to_sample, image_bounds.x - 1);
		f[fptr++] = imageLoad(imageIn, ivec2(column_to_sample, row)).rgb; // Load the next texel replacing the oldest item in the array.
		x = GaussianKernelSize - fptr; // Calculate the X factor (used to match which item in the weight array we must match to each item in the F array)
		if (fptr == GaussianKernelSize)
		{
			fptr = 0;
		}
	}
}