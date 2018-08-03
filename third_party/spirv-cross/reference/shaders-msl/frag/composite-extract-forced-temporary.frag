#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float2 vTexCoord [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> Texture [[texture(0)]], sampler TextureSmplr [[sampler(0)]])
{
    main0_out out = {};
    float f = Texture.sample(TextureSmplr, in.vTexCoord).x;
    out.FragColor = float4(f * f);
    return out;
}

