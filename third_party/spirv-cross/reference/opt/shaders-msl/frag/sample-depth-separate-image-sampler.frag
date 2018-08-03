#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float FragColor [[color(0)]];
};

fragment main0_out main0(depth2d<float> uDepth [[texture(0)]], texture2d<float> uColor [[texture(1)]], sampler uSampler [[sampler(2)]], sampler uSamplerShadow [[sampler(3)]])
{
    main0_out out = {};
    out.FragColor = uDepth.sample_compare(uSamplerShadow, float3(0.5).xy, 0.5) + uColor.sample(uSampler, float2(0.5)).x;
    return out;
}

