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
    float4 gl_Position [[position]];
    uint gl_ViewportIndex [[viewport_array_index]];
};

struct main0_in
{
    float4 gl_Position [[attribute(0)]];
};

struct main0_patchIn
{
    patch_control_point<main0_in> gl_in;
};

[[ patch(quad, 0) ]] vertex main0_out main0(main0_patchIn patchIn [[stage_in]], constant spvDepthClipState& spvDepthClipState [[buffer(17)]])
{
    main0_out out = {};
    out.gl_Position = patchIn.gl_in[0].gl_Position;
    out.gl_ViewportIndex = uint(2);
    if (spvDepthClipState.emulateViewportZ != 0u)
    {
        float2 spvViewportDepthRange = spvDepthClipState.viewportDepthRanges[uint(out.gl_ViewportIndex)];
        out.gl_Position.z = out.gl_Position.z * (spvViewportDepthRange.y - spvViewportDepthRange.x) + out.gl_Position.w * spvViewportDepthRange.x;    // Emulate viewport Z transform
    }
    return out;
}

