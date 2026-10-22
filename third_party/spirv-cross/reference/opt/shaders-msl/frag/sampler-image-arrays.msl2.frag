#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float2 vTex [[user(locn0), flat]];
    int vIndex [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], array<texture2d<float>, 4> uSampler [[texture(0)]], array<texture2d<float>, 4> uTextures [[texture(4)]], array<sampler, 4> uSamplerSmplr [[sampler(0)]], array<sampler, 4> uSamplers [[sampler(4)]])
{
    main0_out out = {};
    out.FragColor = float4(0.0);
    out.FragColor += uTextures[2].sample(uSamplers[1], in.vTex);
    out.FragColor += uSampler[in.vIndex].sample(uSamplerSmplr[in.vIndex], in.vTex);
    out.FragColor += uSampler[in.vIndex].sample(uSamplerSmplr[in.vIndex], (in.vTex + float2(0.100000001490116119384765625)));
    out.FragColor += uSampler[in.vIndex].sample(uSamplerSmplr[in.vIndex], (in.vTex + float2(0.20000000298023223876953125)));
    out.FragColor += uSampler[3].sample(uSamplerSmplr[3], (in.vTex + float2(0.300000011920928955078125)));
    return out;
}

