#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float4 vUV [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texturecube<float> cubeSampler [[texture(0)]], texturecube_array<float> cubeArraySampler [[texture(1)]], texture2d_array<float> texArraySampler [[texture(2)]], sampler cubeSamplerSmplr [[sampler(0)]], sampler cubeArraySamplerSmplr [[sampler(1)]], sampler texArraySamplerSmplr [[sampler(2)]])
{
    main0_out out = {};
    float4 a = cubeSampler.sample(cubeSamplerSmplr, in.vUV.xyz);
    float4 b = cubeArraySampler.sample(cubeArraySamplerSmplr, in.vUV.xyz, uint(rint(in.vUV.w)));
    float4 c = texArraySampler.sample(texArraySamplerSmplr, in.vUV.xyz.xy, uint(rint(in.vUV.xyz.z)));
    out.FragColor = (a + b) + c;
    return out;
}

