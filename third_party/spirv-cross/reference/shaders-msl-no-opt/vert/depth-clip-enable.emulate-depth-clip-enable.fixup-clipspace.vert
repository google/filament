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
};

vertex main0_out main0(constant spvDepthClipState& spvDepthClipState [[buffer(17)]])
{
    main0_out out = {};
    out.gl_Position = float4(0.0, 0.0, 0.25, 1.0);
    out.gl_Position.z = (out.gl_Position.z + out.gl_Position.w) * 0.5;       // Adjust clip-space for Metal
    if (spvDepthClipState.emulateViewportZ != 0u)
    {
        float2 spvViewportDepthRange = spvDepthClipState.viewportDepthRanges[0];
        out.gl_Position.z = out.gl_Position.z * (spvViewportDepthRange.y - spvViewportDepthRange.x) + out.gl_Position.w * spvViewportDepthRange.x;    // Emulate viewport Z transform
    }
    return out;
}

