#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float2 v0 [[user(locn0)]];
    float2 v1 [[user(locn1)]];
    float3 v2 [[user(locn2)]];
    float4 v3 [[user(locn3)]];
    float v4 [[user(locn4)]];
    float v5 [[user(locn5)]];
    float v6 [[user(locn6)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 Position [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.v0 = in.Position.xy;
    out.v1 = in.Position.zw;
    out.v2 = float3(in.Position.x, in.Position.z * in.Position.y, in.Position.x);
    out.v3 = in.Position.xxyy;
    out.v4 = in.Position.w;
    out.v5 = in.Position.y;
    out.v6 = in.Position.x * in.Position.w;
    out.gl_Position = in.Position;
    return out;
}

