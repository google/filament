#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float3 vUV [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], depth2d<float> uT [[texture(0)]], sampler uTSmplr [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = uT.gather_compare(uTSmplr, in.vUV.xy, in.vUV.z);
    return out;
}

