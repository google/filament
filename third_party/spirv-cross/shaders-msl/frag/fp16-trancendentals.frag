#version 450
#extension GL_EXT_shader_explicit_arithmetic_types_float16 : require

layout(location = 0) in float16_t A;
layout(location = 1) in float16_t B;
layout(location = 0) out float16_t C;
layout(location = 1) out float16_t D;

layout(location = 2) in f16vec3 v3;
layout(location = 2) out f16vec3 o3;

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

	// Vector variants since it tripped up overload resolution.
	o3 = sinh(v3);
	o3 += cosh(v3);
	o3 += tanh(v3);
	o3 += sin(v3);
	o3 += cos(v3);
	o3 += tan(v3);
	o3 += asin(v3);
	o3 += acos(v3);
	o3 += atan(v3);
	o3 += asinh(v3);
	o3 += acosh(v3);
	o3 += atanh(v3);
	o3 += atan(o3, v3);
	o3 += pow(o3, v3);
	o3 += exp(v3);
	o3 += exp2(v3);
	o3 += log(v3);
	o3 += log2(v3);
	o3 += sqrt(v3);
	o3 += inversesqrt(v3);
}

