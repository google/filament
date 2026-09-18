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

struct main0_in
{
    float3 vUV [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> uShadow [[texture(0)]], texture2d<float> uTexture [[texture(1)]], sampler uShadowSmplr [[sampler(0)]], sampler uSampler [[sampler(1)]])
{
    main0_out out = {};
    out.FragColor = spvDepthCast(uShadow).sample_compare(uShadowSmplr, in.vUV.xy, in.vUV.z) + spvDepthCast(uTexture).sample_compare(uSampler, in.vUV.xy, in.vUV.z);
    return out;
}

