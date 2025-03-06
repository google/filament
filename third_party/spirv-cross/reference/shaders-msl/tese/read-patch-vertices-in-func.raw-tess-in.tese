#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

static inline __attribute__((always_inline))
float4 read_patch_vertices(thread uint& gl_PatchVerticesIn)
{
    return float4(float(gl_PatchVerticesIn), 0.0, 0.0, 1.0);
}

[[ patch(quad, 0) ]] vertex main0_out main0(uint gl_PrimitiveID [[patch_id]])
{
    main0_out out = {};
    uint gl_PatchVerticesIn = 0;
    out.gl_Position = read_patch_vertices(gl_PatchVerticesIn);
    return out;
}

