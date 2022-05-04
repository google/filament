#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

// Implementation of the GLSL mod() function, which is slightly different than Metal fmod()
template<typename Tx, typename Ty>
inline Tx mod(Tx x, Ty y)
{
    return x - y * floor(x / y);
}

// Implementation of the GLSL radians() function
template<typename T>
inline T radians(T d)
{
    return d * T(0.01745329251);
}

// Implementation of the GLSL degrees() function
template<typename T>
inline T degrees(T r)
{
    return r * T(57.2957795131);
}

struct ResType
{
    half4 _m0;
    int4 _m1;
};

struct main0_in
{
    half v1 [[user(locn0)]];
    half2 v2 [[user(locn1)]];
    half3 v3 [[user(locn2)]];
    half4 v4 [[user(locn3)]];
};

static inline __attribute__((always_inline))
half2x2 test_mat2(thread const half2& a, thread const half2& b, thread const half2& c, thread const half2& d)
{
    return half2x2(half2(a), half2(b)) * half2x2(half2(c), half2(d));
}

static inline __attribute__((always_inline))
half3x3 test_mat3(thread const half3& a, thread const half3& b, thread const half3& c, thread const half3& d, thread const half3& e, thread const half3& f)
{
    return half3x3(half3(a), half3(b), half3(c)) * half3x3(half3(d), half3(e), half3(f));
}

static inline __attribute__((always_inline))
void test_constants()
{
    half a = half(1.0);
    half b = half(1.5);
    half c = half(-1.5);
    half d = half(0.0 / 0.0);
    half e = half(1.0 / 0.0);
    half f = half(-1.0 / 0.0);
    half g = half(1014.0);
    half h = half(9.5367431640625e-07);
}

static inline __attribute__((always_inline))
half test_result()
{
    return half(1.0);
}

static inline __attribute__((always_inline))
void test_conversions()
{
    half one = test_result();
    int a = int(one);
    uint b = uint(one);
    bool c = one != half(0.0);
    float d = float(one);
    half a2 = half(a);
    half b2 = half(b);
    half c2 = half(c);
    half d2 = half(d);
}

static inline __attribute__((always_inline))
void test_builtins(thread half4& v4, thread half3& v3, thread half& v1)
{
    half4 res = radians(v4);
    res = degrees(v4);
    res = sin(v4);
    res = cos(v4);
    res = tan(v4);
    res = asin(v4);
    res = precise::atan2(v4, v3.xyzz);
    res = atan(v4);
    res = fast::sinh(v4);
    res = fast::cosh(v4);
    res = precise::tanh(v4);
    res = asinh(v4);
    res = acosh(v4);
    res = atanh(v4);
    res = pow(v4, v4);
    res = exp(v4);
    res = log(v4);
    res = exp2(v4);
    res = log2(v4);
    res = sqrt(v4);
    res = rsqrt(v4);
    res = abs(v4);
    res = sign(v4);
    res = floor(v4);
    res = trunc(v4);
    res = round(v4);
    res = rint(v4);
    res = ceil(v4);
    res = fract(v4);
    res = mod(v4, v4);
    half4 tmp;
    half4 _223 = modf(v4, tmp);
    res = _223;
    res = min(v4, v4);
    res = max(v4, v4);
    res = clamp(v4, v4, v4);
    res = mix(v4, v4, v4);
    res = select(v4, v4, v4 < v4);
    res = step(v4, v4);
    res = smoothstep(v4, v4, v4);
    bool4 btmp = isnan(v4);
    btmp = isinf(v4);
    res = fma(v4, v4, v4);
    ResType _267;
    _267._m0 = frexp(v4, _267._m1);
    int4 itmp = _267._m1;
    res = _267._m0;
    res = ldexp(res, itmp);
    uint pack0 = as_type<uint>(v4.xy);
    uint pack1 = as_type<uint>(v4.zw);
    res = half4(as_type<half2>(pack0), as_type<half2>(pack1));
    half t0 = length(v4);
    t0 = distance(v4, v4);
    t0 = dot(v4, v4);
    half3 res3 = cross(v3, v3);
    res = fast::normalize(v4);
    res = faceforward(v4, v4, v4);
    res = reflect(v4, v4);
    res = refract(v4, v4, v1);
    btmp = v4 < v4;
    btmp = v4 <= v4;
    btmp = v4 > v4;
    btmp = v4 >= v4;
    btmp = v4 == v4;
    btmp = v4 != v4;
    res = dfdx(v4);
    res = dfdy(v4);
    res = dfdx(v4);
    res = dfdy(v4);
    res = dfdx(v4);
    res = dfdy(v4);
    res = fwidth(v4);
    res = fwidth(v4);
    res = fwidth(v4);
}

fragment void main0(main0_in in [[stage_in]])
{
    half2 param = in.v2;
    half2 param_1 = in.v2;
    half2 param_2 = in.v3.xy;
    half2 param_3 = in.v3.xy;
    half2x2 m0 = test_mat2(param, param_1, param_2, param_3);
    half3 param_4 = in.v3;
    half3 param_5 = in.v3;
    half3 param_6 = in.v3;
    half3 param_7 = in.v4.xyz;
    half3 param_8 = in.v4.xyz;
    half3 param_9 = in.v4.yzw;
    half3x3 m1 = test_mat3(param_4, param_5, param_6, param_7, param_8, param_9);
    test_constants();
    test_conversions();
    test_builtins(in.v4, in.v3, in.v1);
}

