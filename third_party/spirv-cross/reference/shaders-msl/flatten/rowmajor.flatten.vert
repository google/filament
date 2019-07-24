#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO
{
    float4x4 uMVPR;
    float4x4 uMVPC;
    float4x4 uMVP;
};

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 aVertex [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant UBO& _18 [[buffer(0)]])
{
    main0_out out = {};
    float2 v = float4x2(_18.uMVP[0].xy, _18.uMVP[1].xy, _18.uMVP[2].xy, _18.uMVP[3].xy) * in.aVertex;
    out.gl_Position = (_18.uMVPR * in.aVertex) + (in.aVertex * _18.uMVPC);
    return out;
}

