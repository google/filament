#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_patchIn
{
    float2 gl_TessLevelInner [[attribute(0)]];
    float4 gl_TessLevelOuter [[attribute(1)]];
};

[[ patch(quad, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], float2 gl_TessCoord [[position_in_patch]])
{
    main0_out out = {};
    out.gl_Position = float4(((gl_TessCoord.x * patchIn.gl_TessLevelInner.x) * patchIn.gl_TessLevelOuter.x) + (((1.0 - gl_TessCoord.x) * patchIn.gl_TessLevelInner.x) * patchIn.gl_TessLevelOuter.z), ((gl_TessCoord.y * patchIn.gl_TessLevelInner.y) * patchIn.gl_TessLevelOuter.y) + (((1.0 - gl_TessCoord.y) * patchIn.gl_TessLevelInner.y) * patchIn.gl_TessLevelOuter.w), 0.0, 1.0);
    return out;
}

