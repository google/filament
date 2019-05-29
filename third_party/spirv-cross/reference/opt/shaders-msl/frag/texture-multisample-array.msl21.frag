#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    int3 vCoord [[user(locn0)]];
    int vSample [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d_ms_array<float> uTexture [[texture(0)]], sampler uTextureSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = uTexture.read(uint2(in.vCoord.xy), uint(in.vCoord.z), in.vSample);
    return out;
}

