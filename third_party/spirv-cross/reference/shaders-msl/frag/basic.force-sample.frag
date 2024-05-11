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

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> uTex [[texture(0)]], sampler uTexSmplr [[sampler(0)]], uint gl_SampleID [[sample_id]])
{
    main0_out out = {};
    out.FragColor = in.vColor * uTex.sample(uTexSmplr, in.vTex);
    return out;
}

