#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float out_var_SV_Target [[color(0)]];
};

struct main0_in
{
    float2 in_var_TEXCOORD0 [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], depth2d<float> g_Texture [[texture(0)]], sampler g_Sampler [[sampler(0)]], sampler g_CompareSampler [[sampler(1)]])
{
    main0_out out = {};
    out.out_var_SV_Target = float4(g_Texture.sample(g_Sampler, in.in_var_TEXCOORD0)).x + g_Texture.sample_compare(g_CompareSampler, in.in_var_TEXCOORD0, 0.5, level(0.0));
    return out;
}

