#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
    float gl_ClipDistance [[clip_distance]] [2];
    float gl_ClipDistance_0 [[user(clip0)]];
    float gl_ClipDistance_1 [[user(clip1)]];
};

struct main0_in
{
    float4 Position [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.gl_Position = in.Position;
    out.gl_ClipDistance[0] = in.Position.x;
    out.gl_ClipDistance[1] = in.Position.y;
    out.gl_ClipDistance_0 = out.gl_ClipDistance[0];
    out.gl_ClipDistance_1 = out.gl_ClipDistance[1];
    return out;
}

