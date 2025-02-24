#version 320 es

const int GaussianKernelSize = 19;
const int NumGaussianWeightsAndOffsets = (GaussianKernelSize - 1) / 2 + 1;
const int HalfSetGaussianWeightsAndOffsets = NumGaussianWeightsAndOffsets / 2;

layout(std140, set = 0, binding = 0) uniform UboBlurConfig
{
	mediump vec2 config; // (windowWidth, inverseImageHeight)
};

const mediump float Offsets[] = float[](-8.052631578947368, -6.157894736842105, -4.263157894736842, -2.368421052631579, -0.642857142857143, 0.642857142857143, 2.368421052631579,
	4.263157894736842, 6.157894736842105, 8.052631578947368);

layout(location = 0) out mediump vec2 vTexCoords[NumGaussianWeightsAndOffsets + 1];

void main()
{
	mediump vec2 texcoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(texcoord * 2.0 + -1.0, 0.0, 1.0);

	texcoord = vec2(texcoord.x, 1.0 - texcoord.y);

	mediump float inverseImageHeight = config.y;

	vTexCoords[0] = texcoord - Offsets[0] * vec2(0.0, inverseImageHeight);
	vTexCoords[1] = texcoord - Offsets[1] * vec2(0.0, inverseImageHeight);
	vTexCoords[2] = texcoord - Offsets[2] * vec2(0.0, inverseImageHeight);
	vTexCoords[3] = texcoord - Offsets[3] * vec2(0.0, inverseImageHeight);
	vTexCoords[4] = texcoord - Offsets[4] * vec2(0.0, inverseImageHeight);
	vTexCoords[5] = texcoord + Offsets[5] * vec2(0.0, inverseImageHeight);
	vTexCoords[6] = texcoord + Offsets[6] * vec2(0.0, inverseImageHeight);
	vTexCoords[7] = texcoord + Offsets[7] * vec2(0.0, inverseImageHeight);
	vTexCoords[8] = texcoord + Offsets[8] * vec2(0.0, inverseImageHeight);
	vTexCoords[9] = texcoord + Offsets[9] * vec2(0.0, inverseImageHeight);

	vTexCoords[NumGaussianWeightsAndOffsets] = texcoord;
}