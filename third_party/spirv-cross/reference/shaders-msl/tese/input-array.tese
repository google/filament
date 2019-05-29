#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 Floats [[attribute(0)]];
    float4 Floats2 [[attribute(2)]];
};

struct main0_patchIn
{
    patch_control_point<main0_in> gl_in;
};

void set_position(thread float4& gl_Position, thread patch_control_point<main0_in>& gl_in, thread float2& gl_TessCoord)
{
    gl_Position = (gl_in[0].Floats * gl_TessCoord.x) + (gl_in[1].Floats2 * gl_TessCoord.y);
}

[[ patch(quad, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], float2 gl_TessCoord [[position_in_patch]])
{
    main0_out out = {};
    set_position(out.gl_Position, patchIn.gl_in, gl_TessCoord);
    return out;
}

