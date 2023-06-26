#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    half4 FragColor [[color(0)]];
};

struct main0_in
{
    half2 UV [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> uTexture [[texture(0)]], sampler uTextureSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = half4(uTexture.sample(uTextureSmplr, float2(in.UV)));
    return out;
}

