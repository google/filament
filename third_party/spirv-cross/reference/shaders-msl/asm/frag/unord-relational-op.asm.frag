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
    bool c1 = (isunordered(a, b) || a == b);
    bool c2 = (isunordered(a, b) || a != b);
    bool c3 = (isunordered(a, b) || a < b);
    bool c4 = (isunordered(a, b) || a > b);
    bool c5 = (isunordered(a, b) || a <= b);
    bool c6 = (isunordered(a, b) || a >= b);
    bool2 c7 = (isunordered(in.c, in.d) || in.c == in.d);
    bool2 c8 = (isunordered(in.c, in.d) || in.c != in.d);
    bool2 c9 = (isunordered(in.c, in.d) || in.c < in.d);
    bool2 c10 = (isunordered(in.c, in.d) || in.c > in.d);
    bool2 c11 = (isunordered(in.c, in.d) || in.c <= in.d);
    bool2 c12 = (isunordered(in.c, in.d) || in.c >= in.d);
    bool3 c13 = (isunordered(in.e, in.f) || in.e == in.f);
    bool3 c14 = (isunordered(in.e, in.f) || in.e != in.f);
    bool3 c15 = (isunordered(in.e, in.f) || in.e < in.f);
    bool3 c16 = (isunordered(in.e, in.f) || in.e > in.f);
    bool3 c17 = (isunordered(in.e, in.f) || in.e <= in.f);
    bool3 c18 = (isunordered(in.e, in.f) || in.e >= in.f);
    bool4 c19 = (isunordered(in.g, in.h) || in.g == in.h);
    bool4 c20 = (isunordered(in.g, in.h) || in.g != in.h);
    bool4 c21 = (isunordered(in.g, in.h) || in.g < in.h);
    bool4 c22 = (isunordered(in.g, in.h) || in.g > in.h);
    bool4 c23 = (isunordered(in.g, in.h) || in.g <= in.h);
    bool4 c24 = (isunordered(in.g, in.h) || in.g >= in.h);
    out.FragColor = float4(t0 + t1);
    return out;
}

