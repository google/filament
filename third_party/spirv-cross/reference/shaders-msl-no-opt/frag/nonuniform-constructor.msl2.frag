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
    int vIndex [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], array<texture2d<float>, 10> uTex [[texture(0)]], sampler Immut [[sampler(0)]])
{
    main0_out out = {};
    out.FragColor = uTex[in.vIndex].sample(Immut, in.vUV);
    return out;
}

