#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(texture2d<float> uTexture [[texture(0)]], sampler uTextureSmplr [[sampler(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    out.FragColor = uTexture.read(uint2(int2(gl_FragCoord.xy)) + uint2(int2(1)), 0);
    out.FragColor += uTexture.read(uint2(int2(gl_FragCoord.xy)) + uint2(int2(-1, 1)), 0);
    return out;
}

