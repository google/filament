#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO
{
    float4x4 uMVP;
};

struct main0_out
{
    float3 vNormal [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 aVertex [[attribute(0)]];
    float3 aNormal [[attribute(1)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant UBO& _16 [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = _16.uMVP * in.aVertex;
    out.vNormal = in.aNormal;
    return out;
}

