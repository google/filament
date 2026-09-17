#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template <typename T>
static inline depth2d<T> spvDepthCast(texture2d<T> t)
{
    return reinterpret_cast<thread const depth2d<T> &>(t);
}

template <typename T>
static inline depth2d_array<T> spvDepthCast(texture2d_array<T> t)
{
    return reinterpret_cast<thread const depth2d_array<T> &>(t);
}

template <typename T>
static inline depthcube<T> spvDepthCast(texturecube<T> t)
{
    return reinterpret_cast<thread const depthcube<T> &>(t);
}

template <typename T>
static inline depthcube_array<T> spvDepthCast(texturecube_array<T> t)
{
    return reinterpret_cast<thread const depthcube_array<T> &>(t);
}

struct main0_out
{
    float FragColor [[color(0)]];
};

static inline __attribute__((always_inline))
float sample_depth_from_function(texture2d<float> uT, sampler uS)
{
    return spvDepthCast(uT).sample_compare(uS, float3(0.5).xy, 0.5);
}

static inline __attribute__((always_inline))
float sample_color_from_function(texture2d<float> uT, sampler uS)
{
    return uT.sample(uS, float2(0.5)).x;
}

fragment main0_out main0(texture2d<float> uDepth [[texture(0)]], texture2d<float> uColor [[texture(1)]], sampler uSamplerShadow [[sampler(0)]], sampler uSampler [[sampler(1)]])
{
    main0_out out = {};
    out.FragColor = sample_depth_from_function(uDepth, uSamplerShadow) + sample_color_from_function(uColor, uSampler);
    return out;
}

