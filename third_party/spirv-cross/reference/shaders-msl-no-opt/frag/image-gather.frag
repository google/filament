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
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float3 vUV [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> uSamp [[texture(0)]], texture2d<float> uSampShadow [[texture(1)]], sampler uSampSmplr [[sampler(0)]], sampler uSampShadowSmplr [[sampler(1)]])
{
    main0_out out = {};
    out.FragColor = uSamp.gather(uSampSmplr, in.vUV.xy, int2(0), component::x);
    out.FragColor += uSamp.gather(uSampSmplr, in.vUV.xy, int2(0), component::y);
    out.FragColor += spvDepthCast(uSampShadow).gather_compare(uSampShadowSmplr, in.vUV.xy, in.vUV.z);
    return out;
}

