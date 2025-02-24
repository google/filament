#version 310 es

layout(push_constant) uniform pushConstantsBlock{
	mediump vec4 downsampleConfigs[4];
};

layout(location = 0) out mediump vec2 vTexCoords[4];

void main()
{
	mediump vec2 texcoord = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
	gl_Position = vec4(texcoord * 2.0 + -1.0, 0.0, 1.0);

	vTexCoords[0] = texcoord + downsampleConfigs[0].xy;
	vTexCoords[1] = texcoord + downsampleConfigs[1].xy;
	vTexCoords[2] = texcoord + downsampleConfigs[2].xy;
	vTexCoords[3] = texcoord + downsampleConfigs[3].xy;
}