#version 450

layout(binding = 1) uniform samplerCube samplerColor;

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in mat4 inInvModelView;
layout(location = 6) in float inLodBias;
layout(location = 0) out vec4 outFragColor;

void main()
{
	vec3 cI = normalize(inPos);
	vec3 cR = reflect(cI, normalize(inNormal));
	cR = vec3((inInvModelView * vec4(cR, 0.0)).xyz);
	cR.x *= (-1.0);
	outFragColor = texture(samplerColor, cR, inLodBias);
}

