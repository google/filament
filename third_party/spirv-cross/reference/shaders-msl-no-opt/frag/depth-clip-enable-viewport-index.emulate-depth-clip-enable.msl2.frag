#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct spvDepthClipState
{
    uint emulateViewportZ;
    uint emulateDepthClamp;
    float2 viewportDepthRanges[16];
};

struct main0_out
{
    float4 color [[color(0)]];
    float gl_FragDepth [[depth(any)]];
};

fragment main0_out main0(uint gl_ViewportIndex [[viewport_array_index]], constant spvDepthClipState& spvDepthClipState [[buffer(17)]])
{
    main0_out out = {};
    out.color = float4(float(int(gl_ViewportIndex)));
    out.gl_FragDepth = 1.25;
    if (spvDepthClipState.emulateDepthClamp != 0u)
    {
        float2 spvViewportDepthRange = spvDepthClipState.viewportDepthRanges[gl_ViewportIndex];
        out.gl_FragDepth = clamp(out.gl_FragDepth, spvViewportDepthRange.x, spvViewportDepthRange.y);
    }
    return out;
}

