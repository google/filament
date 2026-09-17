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

fragment main0_out main0(texture2d<float> uDepth [[texture(0)]], texture2d<float> uColor [[texture(1)]], sampler uSamplerShadow [[sampler(0)]], sampler uSampler [[sampler(1)]])
{
    main0_out out = {};
    out.FragColor = spvDepthCast(uDepth).sample_compare(uSamplerShadow, float3(0.5).xy, 0.5) + uColor.sample(uSampler, float2(0.5)).x;
    return out;
}

