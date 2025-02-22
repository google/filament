#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant int _13_tmp [[function_constant(201)]];
constant int _13 = is_function_constant_defined(_13_tmp) ? _13_tmp : -10;
constant int _15 = (_13 + 2);
constant uint _24_tmp [[function_constant(202)]];
constant uint _24 = is_function_constant_defined(_24_tmp) ? _24_tmp : 100u;
constant uint _26 = (_24 % 5u);
constant int _61 = _13 - (-3) * (_13 / (-3));
constant int4 _36 = int4(20, 30, _15, _61);
constant int2 _41 = int2(_36.y, _36.x);
constant int _62 = _36.y;

struct main0_out
{
    int m_58 [[user(locn0)]];
    float4 gl_Position [[position]];
};

vertex main0_out main0()
{
    main0_out out = {};
    float4 _66 = float4(0.0);
    _66.y = float(_15);
    _66.z = float(_26);
    float4 _39 = _66 + float4(_36);
    float2 _46 = _39.xy + float2(_41);
    out.gl_Position = float4(_46.x, _46.y, _39.z, _39.w);
    out.m_58 = _62;
    return out;
}

