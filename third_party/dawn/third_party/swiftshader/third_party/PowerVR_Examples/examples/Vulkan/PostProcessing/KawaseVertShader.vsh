#version 320 es

layout(push_constant) uniform pushConstantsBlock{
	mediump vec2 blurConfigs[4];
};

layout(location = 0) out mediump vec2 vTexCoords[4];

void main()
{
	mediump vec2 texcoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(texcoord * 2.0 + -1.0, 0.0, 1.0);

	// Sample top left pixel
	vTexCoords[0] = texcoord + blurConfigs[0];

	// Sample top right pixel
	vTexCoords[1] = texcoord + blurConfigs[1];

	// Sample bottom right pixel
	vTexCoords[2] = texcoord + blurConfigs[2];

	// Sample bottom left pixel
	vTexCoords[3] = texcoord + blurConfigs[3];
}