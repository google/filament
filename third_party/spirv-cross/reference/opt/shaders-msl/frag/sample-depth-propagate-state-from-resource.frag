#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float FragColor [[color(0)]];
};

struct main0_in
{
    float3 vUV [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], depth2d<float> uTexture [[texture(0)]], sampler uSampler [[sampler(0)]], sampler uSamplerShadow [[sampler(1)]])
{
    main0_out out = {};
    out.FragColor = float4(uTexture.sample(uSampler, in.vUV.xy)).x;
    out.FragColor += uTexture.sample_compare(uSamplerShadow, in.vUV.xy, in.vUV.z);
    return out;
}

