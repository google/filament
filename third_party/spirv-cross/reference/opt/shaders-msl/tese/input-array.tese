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

[[ patch(quad, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], float2 gl_TessCoord [[position_in_patch]])
{
    main0_out out = {};
    out.gl_Position = (patchIn.gl_in[0].Floats * gl_TessCoord.x) + (patchIn.gl_in[1].Floats2 * gl_TessCoord.y);
    return out;
}

