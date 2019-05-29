#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 out_var_SV_Target0 [[color(0)]];
};

struct main0_in
{
    uint3 in_var_TEXCOORD0 [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> Tex [[texture(0)]])
{
    main0_out out = {};
    out.out_var_SV_Target0 = Tex.read(uint2(in.in_var_TEXCOORD0.xy), in.in_var_TEXCOORD0.z);
    return out;
}

