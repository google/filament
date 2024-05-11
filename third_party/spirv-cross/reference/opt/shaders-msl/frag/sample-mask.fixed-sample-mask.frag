#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
    uint gl_SampleMask [[sample_mask]];
};

fragment main0_out main0()
{
    main0_out out = {};
    out.FragColor = float4(1.0);
    out.gl_SampleMask = 0;
    out.gl_SampleMask &= 0x22;
    return out;
}

