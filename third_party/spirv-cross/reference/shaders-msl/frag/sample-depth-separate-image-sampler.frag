#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float FragColor [[color(0)]];
};

static inline __attribute__((always_inline))
float sample_depth_from_function(thread const depth2d<float> uT, thread const sampler uS)
{
    return uT.sample_compare(uS, float3(0.5).xy, float3(0.5).z);
}

static inline __attribute__((always_inline))
float sample_color_from_function(thread const texture2d<float> uT, thread const sampler uS)
{
    return uT.sample(uS, float2(0.5)).x;
}

fragment main0_out main0(depth2d<float> uDepth [[texture(0)]], texture2d<float> uColor [[texture(1)]], sampler uSamplerShadow [[sampler(0)]], sampler uSampler [[sampler(1)]])
{
    main0_out out = {};
    out.FragColor = sample_depth_from_function(uDepth, uSamplerShadow) + sample_color_from_function(uColor, uSampler);
    return out;
}

