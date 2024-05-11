#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct t35
{
    float2 m0;
    float4 m1;
};

struct t36
{
    float2 m0;
    t35 m1;
};

struct main0_out
{
    float v80 [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float2 v40_m0;
    t35 v40_m1;
};

[[ patch(triangle, 0) ]] vertex main0_out main0(float3 gl_TessCoord [[position_in_patch]], uint gl_PrimitiveID [[patch_id]], const device main0_in* spvIn [[buffer(22)]])
{
    main0_out out = {};
    const device main0_in* gl_in = &spvIn[gl_PrimitiveID * 0];
    out.gl_Position = float4((gl_TessCoord.xy * 2.0) - float2(1.0), 0.0, 1.0);
    out.v80 = ((float(abs(gl_in[0].v40_m1.m1.x - (-4.0)) < 0.001000000047497451305389404296875) * float(abs(gl_in[0].v40_m1.m1.y - (-9.0)) < 0.001000000047497451305389404296875)) * float(abs(gl_in[0].v40_m1.m1.z - 3.0) < 0.001000000047497451305389404296875)) * float(abs(gl_in[0].v40_m1.m1.w - 7.0) < 0.001000000047497451305389404296875);
    return out;
}

