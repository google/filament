#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct MaterialParams
{
    float4 myLod[6];
};

struct main0_out
{
    float4 fragColor [[color(0)]];
};

fragment main0_out main0(constant MaterialParams& materialParams [[buffer(0)]], texture2d<float> mySampler [[texture(0)]], sampler mySamplerSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.fragColor = mySampler.sample(mySamplerSmplr, float2(0.0), level(materialParams.myLod[0].x));
    return out;
}

