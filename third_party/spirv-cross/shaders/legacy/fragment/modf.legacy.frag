#version 450

layout(location=0) in float scalar;
layout(location=1) in vec2 vector;

layout(location=0) out vec4 result;

void main() {
	float sipart;
	float sfpart = modf(scalar, sipart);
	result.xy = vec2(sipart, sfpart);

	vec2 vipart;
	vec2 vfpart = modf(vector, vipart);
	result.zw = vfpart;
}
