#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
    uint gl_SampleMask [[sample_mask]];
};

fragment main0_out main0(uint gl_SampleMaskIn [[sample_mask]], uint gl_SampleID [[sample_id]])
{
    main0_out out = {};
    out.FragColor = float4(1.0);
    out.gl_SampleMask = (gl_SampleMaskIn & 0x22 & (1 << gl_SampleID));
    out.gl_SampleMask &= 0x22;
    return out;
}

