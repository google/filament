#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Input
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
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float2 Input_v0 [[user(locn0), centroid_no_perspective]];
    float2 Input_v1 [[user(locn1), centroid_no_perspective]];
    float3 Input_v2 [[user(locn2), centroid_no_perspective]];
    float4 Input_v3 [[user(locn3), centroid_no_perspective]];
    float Input_v4 [[user(locn4), centroid_no_perspective]];
    float Input_v5 [[user(locn5), centroid_no_perspective]];
    float Input_v6 [[user(locn6), centroid_no_perspective]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    Input inp = {};
    inp.v0 = in.Input_v0;
    inp.v1 = in.Input_v1;
    inp.v2 = in.Input_v2;
    inp.v3 = in.Input_v3;
    inp.v4 = in.Input_v4;
    inp.v5 = in.Input_v5;
    inp.v6 = in.Input_v6;
    out.FragColor = float4(inp.v0.x + inp.v1.y, inp.v2.xy, ((inp.v3.w * inp.v4) + inp.v5) - inp.v6);
    return out;
}

