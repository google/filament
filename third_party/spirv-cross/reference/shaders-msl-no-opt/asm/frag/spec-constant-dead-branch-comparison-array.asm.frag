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

constant uint _10_tmp [[function_constant(0)]];
constant uint _10 = is_function_constant_defined(_10_tmp) ? _10_tmp : 0u;

struct main0_out
{
    float4 out_var_SV_Target0 [[color(0)]];
};

struct main0_in
{
    float3 in_var_TEXCOORD0 [[user(locn0)]];
};

static inline __attribute__((always_inline))
uint get_sampler_type()
{
    return _10;
}

fragment main0_out main0(main0_in in [[stage_in]], texture2d_array<float> ShadowMapArray [[texture(0)]], sampler Sampler [[sampler(0)]])
{
    main0_out out = {};
    if (get_sampler_type() == 3u)
    {
    }
    out.out_var_SV_Target0 = float4(ShadowMapArray.sample(Sampler, in.in_var_TEXCOORD0.xy, uint(rint(in.in_var_TEXCOORD0.z))));
    return out;
}

