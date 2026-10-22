#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor0 [[color(0)]];
    float4 FragColor1 [[color(1)]];
};

struct main0_in
{
    float4 VertGeom [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> TextureBase [[texture(0)]], texture2d<float> TextureDetail [[texture(1)]], sampler TextureBaseSmplr [[sampler(0)]], sampler TextureDetailSmplr [[sampler(1)]])
{
    main0_out out = {};
    float4 texSample0 = TextureBase.sample(TextureBaseSmplr, float2(in.VertGeom.x, 0.5));
    float4 texSample1 = TextureDetail.sample(TextureDetailSmplr, float2(in.VertGeom.x, 0.5), int2(3, 0));
    int4 iResult0 = as_type<int4>(texSample0);
    int4 iResult1 = as_type<int4>(texSample1);
    out.FragColor0 = as_type<float4>(iResult0) * as_type<float4>(iResult1);
    uint4 uResult0 = as_type<uint4>(texSample0);
    uint4 uResult1 = as_type<uint4>(texSample1);
    out.FragColor1 = as_type<float4>(uResult0) * as_type<float4>(uResult1);
    return out;
}

