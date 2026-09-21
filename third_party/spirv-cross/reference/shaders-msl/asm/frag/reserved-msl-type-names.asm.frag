#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float2 vUV [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], texture2d<float> sampler0 [[texture(0)]], texture2d<float> depth2d0 [[texture(1)]], sampler sampler0Smplr [[sampler(0)]], sampler depth2d0Smplr [[sampler(1)]])
{
    main0_out out = {};
    out.FragColor = sampler0.sample(sampler0Smplr, in.vUV) + depth2d0.sample(depth2d0Smplr, in.vUV);
    return out;
}

