#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct buf0
{
    float4 u_scale;
};

struct buf1
{
    float4 u_bias;
};

struct main0_out
{
    float4 o_color [[color(0)]];
};

struct main0_in
{
    float3 v_texCoord [[user(locn0)]];
    float v_lodBias [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], depth2d_array<float> u_sampler [[texture(0)]], sampler u_samplerSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.o_color = float4(u_sampler.sample_compare(u_samplerSmplr, float2(in.v_texCoord.x, 0.5), uint(rint(in.v_texCoord.y)), in.v_texCoord.z, gradient2d(exp2(in.v_lodBias - 0.5) / float2(u_sampler.get_width(), 1.0), exp2(in.v_lodBias - 0.5) / float2(u_sampler.get_width(), 1.0))), 0.0, 0.0, 1.0);
    return out;
}

