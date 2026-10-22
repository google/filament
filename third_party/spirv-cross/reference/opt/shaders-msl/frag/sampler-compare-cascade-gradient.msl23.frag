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
    float4 vUV [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d_array<float> uTex [[texture(0)]], sampler uShadow [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = spvDepthCast(uTex).sample_compare(uShadow, in.vUV.xy, uint(rint(in.vUV.z)), in.vUV.w, level(0)) + spvDepthCast(uTex).sample_compare(uShadow, in.vUV.xy, uint(rint(in.vUV.z)), in.vUV.w, gradient2d(float2(1.0), float2(1.0)));
    return out;
}

