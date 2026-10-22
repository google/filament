#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Output
{
    float2 v0;
    float2 v1;
    float3 v2;
    float4 v3;
    float v4;
    float v5;
    float v6;
};

struct main0_out
{
    float2 outp_v0 [[user(locn0)]];
    float2 outp_v1 [[user(locn1)]];
    float3 outp_v2 [[user(locn2)]];
    float4 outp_v3 [[user(locn3)]];
    float outp_v4 [[user(locn4)]];
    float outp_v5 [[user(locn5)]];
    float outp_v6 [[user(locn6)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 Position [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    Output outp = {};
    outp.v0 = in.Position.xy;
    outp.v1 = in.Position.zw;
    outp.v2 = float3(in.Position.x, in.Position.z * in.Position.y, in.Position.x);
    outp.v3 = in.Position.xxyy;
    outp.v4 = in.Position.w;
    outp.v5 = in.Position.y;
    outp.v6 = in.Position.x * in.Position.w;
    out.gl_Position = in.Position;
    out.outp_v0 = outp.v0;
    out.outp_v1 = outp.v1;
    out.outp_v2 = outp.v2;
    out.outp_v3 = outp.v3;
    out.outp_v4 = outp.v4;
    out.outp_v5 = outp.v5;
    out.outp_v6 = outp.v6;
    return out;
}

