#version 450

layout(location=0) in float scalar;
layout(location=1) in vec3 vector;

layout(location=0) out vec4 result;

void main() {
	result = vec4(1.0);

	result.w *= sinh(scalar);
	result.w *= cosh(scalar);
	result.w *= tanh(scalar);

	result.w *= asinh(scalar);
	result.w *= acosh(scalar);
	result.w *= atanh(scalar);

	result.xyz *= sinh(vector);
	result.xyz *= cosh(vector);
	result.xyz *= tanh(vector);

	result.xyz *= asinh(vector);
	result.xyz *= acosh(vector);
	result.xyz *= atanh(vector);
}
