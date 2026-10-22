#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(texture2d<float> uT [[texture(0)]], sampler uTSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = uT.gather(uTSmplr, float2(0.5), int2(0), component::w);
    return out;
}

