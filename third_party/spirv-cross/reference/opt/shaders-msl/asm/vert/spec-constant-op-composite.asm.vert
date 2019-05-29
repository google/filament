#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant int _7_tmp [[function_constant(201)]];
constant int _7 = is_function_constant_defined(_7_tmp) ? _7_tmp : -10;
constant int _20 = (_7 + 2);
constant uint _8_tmp [[function_constant(202)]];
constant uint _8 = is_function_constant_defined(_8_tmp) ? _8_tmp : 100u;
constant uint _25 = (_8 % 5u);
constant int4 _30 = int4(20, 30, _20, _20);
constant int2 _32 = int2(_30.y, _30.x);
constant int _33 = _30.y;

struct main0_out
{
    int m_4 [[user(locn0)]];
    float4 gl_Position [[position]];
};

vertex main0_out main0()
{
    main0_out out = {};
    float4 _63 = float4(0.0);
    _63.y = float(_20);
    float4 _66 = _63;
    _66.z = float(_25);
    float4 _52 = _66 + float4(_30);
    float2 _56 = _52.xy + float2(_32);
    out.gl_Position = float4(_56.x, _56.y, _52.z, _52.w);
    out.m_4 = _33;
    return out;
}

