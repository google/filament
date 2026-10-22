#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 gl_Position [[attribute(0)]];
};

struct main0_patchIn
{
    patch_control_point<main0_in> gl_in;
};

[[ patch(triangle, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], float3 gl_TessCoord [[position_in_patch]])
{
    main0_out out = {};
    out.gl_Position = ((patchIn.gl_in[0].gl_Position * gl_TessCoord.x) + (patchIn.gl_in[1].gl_Position * gl_TessCoord.y)) + (patchIn.gl_in[2].gl_Position * gl_TessCoord.z);
    return out;
}

