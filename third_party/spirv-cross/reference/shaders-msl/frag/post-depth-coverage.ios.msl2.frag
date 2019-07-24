#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

[[ early_fragment_tests ]] fragment main0_out main0(uint gl_SampleMaskIn [[sample_mask, post_depth_coverage]])
{
    main0_out out = {};
    out.FragColor = float4(float(gl_SampleMaskIn));
    return out;
}

