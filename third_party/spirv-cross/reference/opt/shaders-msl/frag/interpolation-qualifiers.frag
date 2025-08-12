#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float2 v0 [[user(locn0)]];
    float2 v1 [[user(locn1), center_no_perspective]];
    float3 v2 [[user(locn2), centroid_perspective]];
    float4 v3 [[user(locn3), centroid_no_perspective]];
    float v4 [[user(locn4), sample_perspective]];
    float v5 [[user(locn5), sample_no_perspective]];
    float v6 [[user(locn6), flat]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.FragColor = float4(in.v0.x + in.v1.y, in.v2.xy, fma(in.v3.w, in.v4, in.v5) - in.v6);
    return out;
}

