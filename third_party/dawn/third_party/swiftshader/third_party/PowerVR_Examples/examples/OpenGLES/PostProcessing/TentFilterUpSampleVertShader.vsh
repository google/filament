#version 310 es

uniform mediump vec2 upSampleConfigs[8];

layout(location = 0) out mediump vec2 vTexCoords[9];

void main()
{
	mediump vec2 texcoord = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2);
	gl_Position = vec4(texcoord * 2.0 + -1.0, 0.0, 1.0);

	vTexCoords[0] = texcoord;
	vTexCoords[1] = texcoord + upSampleConfigs[0];
	vTexCoords[2] = texcoord + upSampleConfigs[1];
	vTexCoords[3] = texcoord + upSampleConfigs[2];
	vTexCoords[4] = texcoord + upSampleConfigs[3];
	vTexCoords[5] = texcoord + upSampleConfigs[4];
	vTexCoords[6] = texcoord + upSampleConfigs[5];
	vTexCoords[7] = texcoord + upSampleConfigs[6];
	vTexCoords[8] = texcoord + upSampleConfigs[7];
}