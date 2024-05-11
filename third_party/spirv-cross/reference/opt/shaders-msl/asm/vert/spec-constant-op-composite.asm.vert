#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant int _7_tmp [[function_constant(201)]];
constant int _7 = is_function_constant_defined(_7_tmp) ? _7_tmp : -10;
constant int _20 = (_7 + 2);
constant uint _8_tmp [[function_constant(202)]];
constant uint _8 = is_function_constant_defined(_8_tmp) ? _8_tmp : 100u;
constant uint _25 = (_8 % 5u);
constant int _30 = _7 - (-3) * (_7 / (-3));
constant int4 _32 = int4(20, 30, _20, _30);
constant int2 _34 = int2(_32.y, _32.x);
constant int _35 = _32.y;

struct main0_out
{
    int m_4 [[user(locn0)]];
    float4 gl_Position [[position]];
};

vertex main0_out main0()
{
    main0_out out = {};
    float4 _66 = float4(0.0);
    _66.y = float(_20);
    _66.z = float(_25);
    float4 _55 = _66 + float4(_32);
    float2 _59 = _55.xy + float2(_34);
    out.gl_Position = float4(_59.x, _59.y, _55.z, _55.w);
    out.m_4 = _35;
    return out;
}

