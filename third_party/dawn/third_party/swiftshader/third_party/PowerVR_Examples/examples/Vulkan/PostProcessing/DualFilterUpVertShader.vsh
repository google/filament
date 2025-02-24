#version 320 es

layout(push_constant) uniform pushConstantsBlock{
	mediump vec2 blurConfigs[8];
	mediump float exposureBias;
};

layout(location = 0) out mediump vec2 vTexCoords[9];

void main()
{
	mediump vec2 texcoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(texcoord * 2.0 + -1.0, 0.0, 1.0);

	vTexCoords[0] = texcoord;
	vTexCoords[1] = texcoord + blurConfigs[0];
	vTexCoords[2] = texcoord + blurConfigs[1];
	vTexCoords[3] = texcoord + blurConfigs[2];
	vTexCoords[4] = texcoord + blurConfigs[3];
	vTexCoords[5] = texcoord + blurConfigs[4];
	vTexCoords[6] = texcoord + blurConfigs[5];
	vTexCoords[7] = texcoord + blurConfigs[6];
	vTexCoords[8] = texcoord + blurConfigs[7];
}