#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 out_var_SV_Target0 [[color(0)]];
};

struct main0_in
{
    float2 in_var_TEXCOORD0 [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], depth2d<float> ShadowMap [[texture(0)]], sampler SampleNormal [[sampler(0)]], sampler SampleShadow [[sampler(1)]])
{
    main0_out out = {};
    float _41;
    if (in.in_var_TEXCOORD0.x > 0.5)
    {
        _41 = float(float4(ShadowMap.sample(SampleNormal, in.in_var_TEXCOORD0)).x <= 0.5);
    }
    else
    {
        _41 = ShadowMap.sample_compare(SampleShadow, in.in_var_TEXCOORD0, 0.5, level(0.0));
    }
    out.out_var_SV_Target0 = float4(_41, _41, _41, 1.0);
    return out;
}

