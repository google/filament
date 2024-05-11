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

fragment main0_out main0(main0_in in [[stage_in]], depth2d<float> g_depthTexture [[texture(0)]], sampler g_sampler [[sampler(0)]], sampler g_comp [[sampler(1)]])
{
    main0_out out = {};
    out.out_var_SV_Target0 = g_depthTexture.gather_compare(g_comp, in.in_var_TEXCOORD0, 0.5) * g_depthTexture.gather(g_sampler, in.in_var_TEXCOORD0, int2(0));
    return out;
}

