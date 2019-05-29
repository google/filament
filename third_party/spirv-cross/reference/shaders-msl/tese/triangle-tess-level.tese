#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_patchIn
{
    float4 gl_TessLevel [[attribute(0)]];
};

[[ patch(triangle, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], float3 gl_TessCoord [[position_in_patch]])
{
    main0_out out = {};
    float gl_TessLevelInner[2] = {};
    float gl_TessLevelOuter[4] = {};
    gl_TessLevelInner[0] = patchIn.gl_TessLevel.w;
    gl_TessLevelOuter[0] = patchIn.gl_TessLevel.x;
    gl_TessLevelOuter[1] = patchIn.gl_TessLevel.y;
    gl_TessLevelOuter[2] = patchIn.gl_TessLevel.z;
    out.gl_Position = float4((gl_TessCoord.x * gl_TessLevelInner[0]) * gl_TessLevelOuter[0], (gl_TessCoord.y * gl_TessLevelInner[0]) * gl_TessLevelOuter[1], (gl_TessCoord.z * gl_TessLevelInner[0]) * gl_TessLevelOuter[2], 1.0);
    return out;
}

