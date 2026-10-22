#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

[[ patch(quad, 0) ]] vertex main0_out main0(uint gl_PrimitiveID [[patch_id]])
{
    main0_out out = {};
    uint gl_PatchVerticesIn = 0;
    out.gl_Position = float4(float(gl_PatchVerticesIn), 0.0, 0.0, 1.0);
    return out;
}

