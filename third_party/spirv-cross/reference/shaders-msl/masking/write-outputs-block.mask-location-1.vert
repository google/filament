#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct V
{
    float4 a;
    float4 b;
    float4 c;
    float4 d;
};

struct main0_out
{
    float4 V_a [[user(locn0)]];
    float4 V_c [[user(locn2)]];
    float4 V_d [[user(locn3)]];
    float4 gl_Position [[position]];
};

vertex main0_out main0()
{
    main0_out out = {};
    V _22 = {};
    out.gl_Position = float4(1.0);
    _22.a = float4(2.0);
    _22.b = float4(3.0);
    out.V_a = _22.a;
    out.V_c = _22.c;
    out.V_d = _22.d;
    return out;
}

