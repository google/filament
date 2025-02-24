#version 310 es

uniform mediump vec2 blurConfigs[8];

layout(location = 0) out mediump vec2 vTexCoords[5];

void main()
{
	mediump vec2 texcoord = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	gl_Position = vec4(texcoord * 2.0 + -1.0, 0.0, 1.0);

	vTexCoords[0] = texcoord;
	vTexCoords[1] = texcoord + blurConfigs[0];
	vTexCoords[2] = texcoord + blurConfigs[1];
	vTexCoords[3] = texcoord + blurConfigs[2];
	vTexCoords[4] = texcoord + blurConfigs[3];
}