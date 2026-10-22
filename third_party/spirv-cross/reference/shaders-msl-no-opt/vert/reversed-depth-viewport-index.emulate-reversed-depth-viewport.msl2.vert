#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
    uint gl_ViewportIndex [[viewport_array_index]];
};

vertex main0_out main0(constant uint& spvEmulatedReversedDepthViewportMask [[buffer(18)]])
{
    main0_out out = {};
    out.gl_Position = float4(0.0, 0.0, 0.25, 1.0);
    out.gl_ViewportIndex = uint(2);
    if (((spvEmulatedReversedDepthViewportMask >> uint(out.gl_ViewportIndex)) & 1u) != 0u)
    {
        out.gl_Position.z = out.gl_Position.w - out.gl_Position.z;    // Emulate reversed-depth viewport
    }
    return out;
}

