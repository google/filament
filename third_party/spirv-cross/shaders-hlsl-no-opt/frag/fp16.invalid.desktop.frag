#version 450
#extension GL_AMD_gpu_shader_half_float : require

layout(location = 0) in float16_t v1;
layout(location = 1) in f16vec2 v2;
layout(location = 2) in f16vec3 v3;
layout(location = 3) in f16vec4 v4;

layout(location = 0) out float o1;
layout(location = 1) out vec2 o2;
layout(location = 2) out vec3 o3;
layout(location = 3) out vec4 o4;

#if 0
// Doesn't work on glslang yet.
f16mat2 test_mat2(f16vec2 a, f16vec2 b, f16vec2 c, f16vec2 d)
{
	return f16mat2(a, b) * f16mat2(c, d);
}

f16mat3 test_mat3(f16vec3 a, f16vec3 b, f16vec3 c, f16vec3 d, f16vec3 e, f16vec3 f)
{
	return f16mat3(a, b, c) * f16mat3(d, e, f);
}
#endif

void test_constants()
{
	float16_t a = 1.0hf;
	float16_t b = 1.5hf;
	float16_t c = -1.5hf; // Negatives
	float16_t d = (0.0hf / 0.0hf); // NaN
	float16_t e = (1.0hf / 0.0hf); // +Inf
	float16_t f = (-1.0hf / 0.0hf); // -Inf
	float16_t g = 1014.0hf; // Large.
	float16_t h = 0.000001hf; // Denormal
}

float16_t test_result()
{
	return 1.0hf;
}

void test_conversions()
{
	float16_t one = test_result();
	int a = int(one);
	uint b = uint(one);
	bool c = bool(one);
	float d = float(one);
	double e = double(one);
	float16_t a2 = float16_t(a);
	float16_t b2 = float16_t(b);
	float16_t c2 = float16_t(c);
	float16_t d2 = float16_t(d);
	float16_t e2 = float16_t(e);
}

void test_builtins()
{
	f16vec4 res;
	res = radians(v4);
	res = degrees(v4);
	res = sin(v4);
	res = cos(v4);
	res = tan(v4);
	res = asin(v4);
	res = atan(v4, v3.xyzz);
	res = atan(v4);
	res = sinh(v4);
	res = cosh(v4);
	res = tanh(v4);
	//res = asinh(v4);
	//res = acosh(v4);
	//res = atanh(v4);
	res = pow(v4, v4);
	res = exp(v4);
	res = log(v4);
	res = exp2(v4);
	res = log2(v4);
	res = sqrt(v4);
	res = inversesqrt(v4);
	res = abs(v4);
	res = sign(v4);
	res = floor(v4);
	res = trunc(v4);
	res = round(v4);
	//res = roundEven(v4);
	res = ceil(v4);
	res = fract(v4);
	res = mod(v4, v4);
	f16vec4 tmp;
	res = modf(v4, tmp);
	res = min(v4, v4);
	res = max(v4, v4);
	res = clamp(v4, v4, v4);
	res = mix(v4, v4, v4);
	res = mix(v4, v4, lessThan(v4, v4));
	res = step(v4, v4);
	res = smoothstep(v4, v4, v4);

	bvec4 btmp = isnan(v4);
	btmp = isinf(v4);
	res = fma(v4, v4, v4);

	//ivec4 itmp;
	//res = frexp(v4, itmp);
	//res = ldexp(res, itmp);

	uint pack0 = packFloat2x16(v4.xy);
	uint pack1 = packFloat2x16(v4.zw);
	res = f16vec4(unpackFloat2x16(pack0), unpackFloat2x16(pack1));

	float16_t t0 = length(v4);
	t0 = distance(v4, v4);
	t0 = dot(v4, v4);
	f16vec3 res3 = cross(v3, v3);
	res = normalize(v4);
	res = faceforward(v4, v4, v4);
	res = reflect(v4, v4);
	res = refract(v4, v4, v1);

	btmp = lessThan(v4, v4);
	btmp = lessThanEqual(v4, v4);
	btmp = greaterThan(v4, v4);
	btmp = greaterThanEqual(v4, v4);
	btmp = equal(v4, v4);
	btmp = notEqual(v4, v4);

	res = dFdx(v4);
	res = dFdy(v4);
	res = dFdxFine(v4);
	res = dFdyFine(v4);
	res = dFdxCoarse(v4);
	res = dFdyCoarse(v4);
	res = fwidth(v4);
	res = fwidthFine(v4);
	res = fwidthCoarse(v4);

	//res = interpolateAtCentroid(v4);
	//res = interpolateAtSample(v4, 0);
	//res = interpolateAtOffset(v4, f16vec2(0.1hf));
}

void main()
{
	// Basic matrix tests.
#if 0
	f16mat2 m0 = test_mat2(v2, v2, v3.xy, v3.xy);
	f16mat3 m1 = test_mat3(v3, v3, v3, v4.xyz, v4.xyz, v4.yzw);
#endif

	test_constants();
	test_conversions();
	test_builtins();
}
