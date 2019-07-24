#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 m_17 [[attribute(0)]];
};

vertex void main0(main0_in in [[stage_in]], texture1d<uint, access::write> _34 [[texture(0)]], texture1d<uint> _37 [[texture(1)]])
{
    main0_out out = {};
    out.gl_Position = in.m_17;
    for (int _22 = 0; _22 < 128; _22++)
    {
        _34.write(_37.read(uint(_22)), uint(_22));
    }
}

