#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(texture2d_ms<float> uSampler [[texture(0)]], sampler uSamplerSmplr [[sampler(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    int2 _17 = int2(gl_FragCoord.xy);
    out.FragColor = ((uSampler.read(uint2(_17), 0) + uSampler.read(uint2(_17), 1)) + uSampler.read(uint2(_17), 2)) + uSampler.read(uint2(_17), 3);
    return out;
}

