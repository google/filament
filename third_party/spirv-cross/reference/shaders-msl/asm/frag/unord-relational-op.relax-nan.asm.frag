#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant float a_tmp [[function_constant(1)]];
constant float a = is_function_constant_defined(a_tmp) ? a_tmp : 1.0;
constant float b_tmp [[function_constant(2)]];
constant float b = is_function_constant_defined(b_tmp) ? b_tmp : 2.0;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float2 c [[user(locn2)]];
    float2 d [[user(locn3)]];
    float3 e [[user(locn4)]];
    float3 f [[user(locn5)]];
    float4 g [[user(locn6)]];
    float4 h [[user(locn7)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    float t0 = a;
    float t1 = b;
    bool c1 = a == b;
    c1 = a != b;
    bool c2 = a != b;
    bool c3 = a < b;
    bool c4 = a > b;
    bool c5 = a <= b;
    bool c6 = a >= b;
    bool2 c7 = in.c == in.d;
    bool2 c8 = in.c != in.d;
    bool2 c9 = in.c < in.d;
    bool2 c10 = in.c > in.d;
    bool2 c11 = in.c <= in.d;
    bool2 c12 = in.c >= in.d;
    bool3 c13 = in.e == in.f;
    bool3 c14 = in.e != in.f;
    bool3 c15 = in.e < in.f;
    bool3 c16 = in.e > in.f;
    bool3 c17 = in.e <= in.f;
    bool3 c18 = in.e >= in.f;
    bool4 c19 = in.g == in.h;
    bool4 c20 = in.g != in.h;
    bool4 c21 = in.g < in.h;
    bool4 c22 = in.g > in.h;
    bool4 c23 = in.g <= in.h;
    bool4 c24 = in.g >= in.h;
    out.FragColor = float4(t0 + t1);
    return out;
}

