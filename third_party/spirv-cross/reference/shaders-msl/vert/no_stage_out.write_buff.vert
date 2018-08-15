#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct _35
{
    uint4 _m0[1024];
};

struct _40
{
    uint4 _m0[1024];
};

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 m_17 [[attribute(0)]];
};

vertex void main0(main0_in in [[stage_in]], constant _40& _42 [[buffer(0)]], device _35& _37 [[buffer(1)]])
{
    main0_out out = {};
    out.gl_Position = in.m_17;
    for (int _22 = 0; _22 < 1024; _22++)
    {
        _37._m0[_22] = _42._m0[_22];
    }
}

