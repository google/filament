#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 v0 [[user(locn0)]];
    float4 gl_Position [[position]];
    float gl_PointSize [[point_size]];
    float gl_ClipDistance [[clip_distance]] [2];
    float gl_ClipDistance_0 [[user(clip0)]];
    float gl_ClipDistance_1 [[user(clip1)]];
};

vertex main0_out main0()
{
    main0_out out = {};
    float4 v1 = {};
    out.v0 = float4(1.0);
    v1 = float4(2.0);
    out.gl_Position = float4(3.0);
    out.gl_PointSize = 4.0;
    out.gl_ClipDistance[0] = 1.0;
    out.gl_ClipDistance[1] = 0.5;
    out.gl_ClipDistance_0 = out.gl_ClipDistance[0];
    out.gl_ClipDistance_1 = out.gl_ClipDistance[1];
    return out;
}

