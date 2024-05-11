#version 450

layout(location=0) in float scalar;
layout(location=1) in vec4 vector;

layout(location=0) out vec4 result;

void main() {
	result = vec4(0.0);
	if (isnan(scalar)) result.r = 1.0;
	if (isinf(scalar)) result.g = 1.0;
	if (any(isnan(vector))) result.b = 1.0;
	if (any(isinf(vector))) result.a = 1.0;
}
