#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float4 vColor [[user(locn0)]];
    float2 vTex [[user(locn1)]];
};

static inline __attribute__((always_inline))
float4 sample_texture(thread const texture2d<float> tex, thread const sampler texSmplr, thread const float2& uv)
{
    return tex.sample(texSmplr, uv);
}

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> uTex [[texture(0)]], sampler uTexSmplr [[sampler(0)]])
{
    main0_out out = {};
    float2 param = in.vTex;
    out.FragColor = in.vColor * sample_texture(uTex, uTexSmplr, param);
    return out;
}

