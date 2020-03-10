#pragma clang diagnostic ignored "-Wmissing-prototypes"

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

static inline __attribute__((always_inline))
void set_position(thread float4& gl_Position, thread float2& gl_TessCoord, thread float2& gl_TessLevelInner, thread float4& gl_TessLevelOuter)
{
    gl_Position = float4(((gl_TessCoord.x * gl_TessLevelInner.x) * gl_TessLevelOuter.x) + (((1.0 - gl_TessCoord.x) * gl_TessLevelInner.x) * gl_TessLevelOuter.z), ((gl_TessCoord.y * gl_TessLevelInner.y) * gl_TessLevelOuter.y) + (((1.0 - gl_TessCoord.y) * gl_TessLevelInner.y) * gl_TessLevelOuter.w), 0.0, 1.0);
}

[[ patch(quad, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], float2 gl_TessCoord [[position_in_patch]])
{
    main0_out out = {};
    set_position(out.gl_Position, gl_TessCoord, patchIn.gl_TessLevelInner, patchIn.gl_TessLevelOuter);
    return out;
}

