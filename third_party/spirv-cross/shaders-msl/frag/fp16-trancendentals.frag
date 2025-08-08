#version 450
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require

layout(location = 0) in float16_t A;
layout(location = 1) in float16_t B;
layout(location = 0) out float16_t C;
layout(location = 1) out float16_t D;

void main()
{
	D = 0.0hf;	
	C = clamp(sin(A), A, B); D += C;
	C = clamp(cos(A), A, B); D += C;
	C = clamp(tan(A), A, B); D += C;
	C = clamp(asin(A), A, B); D += C;
	C = clamp(acos(A), A, B); D += C;
	C = clamp(atan(A), A, B); D += C;
	C = clamp(sinh(A), A, B); D += C;
	C = clamp(cosh(A), A, B); D += C;
	C = clamp(tanh(A), A, B); D += C;
	C = clamp(asinh(A), A, B); D += C;
	C = clamp(acosh(A), A, B); D += C;
	C = clamp(atanh(A), A, B); D += C;
	C = clamp(atan(A, B), A, B); D += C;
	C = clamp(pow(A, B), A, B); D += C;
	C = clamp(exp(A), A, B); D += C;
	C = clamp(exp2(A), A, B); D += C;
	C = clamp(log(A), A, B); D += C;
	C = clamp(log2(A), A, B); D += C;
	C = clamp(sqrt(A), A, B); D += C;
	C = clamp(inversesqrt(A), A, B); D += C;
}

