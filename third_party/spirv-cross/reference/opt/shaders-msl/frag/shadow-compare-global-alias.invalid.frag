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

fragment main0_out main0(main0_in in [[stage_in]], depth2d<float> uTex [[texture(0)]], depth2d<float> uSampler [[texture(1)]], sampler uSamp [[sampler(0)]], sampler uSamplerSmplr [[sampler(1)]])
{
    main0_out out = {};
    out.FragColor = uSampler.sample_compare(uSamplerSmplr, in.vUV.xy, in.vUV.z);
    out.FragColor += uTex.sample_compare(uSamp, in.vUV.xy, in.vUV.z);
    out.FragColor += uTex.sample_compare(uSamp, in.vUV.xy, in.vUV.z);
    out.FragColor += uSampler.sample_compare(uSamplerSmplr, in.vUV.xy, in.vUV.z);
    out.FragColor += uTex.sample_compare(uSamp, in.vUV.xy, in.vUV.z);
    out.FragColor += uSampler.sample_compare(uSamplerSmplr, in.vUV.xy, in.vUV.z);
    return out;
}

