#version 450

layout(binding = 0, std140) uniform UBO
{
	mat4 projection;
	mat4 model;
	float lodBias;
} ubo;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec4 colors[3];
layout(location = 4) in vec3 inNormal;
layout(location = 5) in mat4 inViewMat;
layout(location = 0) out vec3 outPos;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out mat4 outTransModel;
layout(location = 6) out float outLodBias;
layout(location = 7) out vec4 color;

void write_deeper_in_function()
{
	outTransModel[1][1] = ubo.lodBias;
	color = colors[2];
}

void write_in_function()
{
	outTransModel[2] = vec4(inNormal, 1.0);
	write_deeper_in_function();
}

void main()
{
	gl_Position = (ubo.projection * ubo.model) * vec4(inPos, 1.0);
	outPos = vec3((ubo.model * vec4(inPos, 1.0)).xyz);
	outNormal = mat3(vec3(ubo.model[0].x, ubo.model[0].y, ubo.model[0].z), vec3(ubo.model[1].x, ubo.model[1].y, ubo.model[1].z), vec3(ubo.model[2].x, ubo.model[2].y, ubo.model[2].z)) * inNormal;
	outLodBias = ubo.lodBias;
	outTransModel = transpose(ubo.model) * inViewMat;
	write_in_function();
}

