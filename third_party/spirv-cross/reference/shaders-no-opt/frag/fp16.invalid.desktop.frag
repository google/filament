#version 450
#if defined(GL_AMD_gpu_shader_half_float)
#extension GL_AMD_gpu_shader_half_float : require
#elif defined(GL_NV_gpu_shader5)
#extension GL_NV_gpu_shader5 : require
#else
#error No extension available for FP16.
#endif

struct ResType
{
    f16vec4 _m0;
    ivec4 _m1;
};

layout(location = 3) in f16vec4 v4;
layout(location = 2) in f16vec3 v3;
layout(location = 0) in float16_t v1;
layout(location = 1) in f16vec2 v2;

f16mat2 test_mat2(f16vec2 a, f16vec2 b, f16vec2 c, f16vec2 d)
{
    return f16mat2(f16vec2(a), f16vec2(b)) * f16mat2(f16vec2(c), f16vec2(d));
}

f16mat3 test_mat3(f16vec3 a, f16vec3 b, f16vec3 c, f16vec3 d, f16vec3 e, f16vec3 f)
{
    return f16mat3(f16vec3(a), f16vec3(b), f16vec3(c)) * f16mat3(f16vec3(d), f16vec3(e), f16vec3(f));
}

void test_constants()
{
    float16_t a = float16_t(1.0);
    float16_t b = float16_t(1.5);
    float16_t c = float16_t(-1.5);
    float16_t d = float16_t(0.0 / 0.0);
    float16_t e = float16_t(1.0 / 0.0);
    float16_t f = float16_t(-1.0 / 0.0);
    float16_t g = float16_t(1014.0);
    float16_t h = float16_t(9.5367431640625e-07);
}

float16_t test_result()
{
    return float16_t(1.0);
}

void test_conversions()
{
    float16_t one = test_result();
    int a = int(one);
    uint b = uint(one);
    bool c = one != float16_t(0.0);
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
    f16vec4 res = radians(v4);
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
    res = asinh(v4);
    res = acosh(v4);
    res = atanh(v4);
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
    res = roundEven(v4);
    res = ceil(v4);
    res = fract(v4);
    res = mod(v4, v4);
    f16vec4 tmp;
    f16vec4 _231 = modf(v4, tmp);
    res = _231;
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
    ResType _275;
    _275._m0 = frexp(v4, _275._m1);
    ivec4 itmp = _275._m1;
    res = _275._m0;
    res = ldexp(res, itmp);
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
}

void main()
{
    f16vec2 param = v2;
    f16vec2 param_1 = v2;
    f16vec2 param_2 = v3.xy;
    f16vec2 param_3 = v3.xy;
    f16mat2 m0 = test_mat2(param, param_1, param_2, param_3);
    f16vec3 param_4 = v3;
    f16vec3 param_5 = v3;
    f16vec3 param_6 = v3;
    f16vec3 param_7 = v4.xyz;
    f16vec3 param_8 = v4.xyz;
    f16vec3 param_9 = v4.yzw;
    f16mat3 m1 = test_mat3(param_4, param_5, param_6, param_7, param_8, param_9);
    test_constants();
    test_conversions();
    test_builtins();
}

