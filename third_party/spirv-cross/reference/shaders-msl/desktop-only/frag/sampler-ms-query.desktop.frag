#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(texture2d_ms<float> uSampler [[texture(0)]], texture2d_ms<float> uImage [[texture(2)]], sampler uSamplerSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = float4(float(int(uSampler.get_num_samples()) + int(uImage.get_num_samples())));
    return out;
}

