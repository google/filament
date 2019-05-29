#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO
{
    float4 v[64];
};

struct SSBO
{
    float4 v[1];
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    int vIndex [[user(locn0)]];
    float2 vUV [[user(locn1)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant UBO* ubos_0 [[buffer(0)]], constant UBO* ubos_1 [[buffer(1)]], const device SSBO* ssbos_0 [[buffer(2)]], const device SSBO* ssbos_1 [[buffer(3)]], array<texture2d<float>, 8> uSamplers [[texture(0)]], array<texture2d<float>, 8> uCombinedSamplers [[texture(8)]], array<sampler, 7> uSamps [[sampler(1)]], array<sampler, 8> uCombinedSamplersSmplr [[sampler(8)]])
{
    constant UBO* ubos[] =
    {
        ubos_0,
        ubos_1,
    };

    const device SSBO* ssbos[] =
    {
        ssbos_0,
        ssbos_1,
    };

    main0_out out = {};
    int _24 = in.vIndex + 10;
    int _35 = in.vIndex + 40;
    out.FragColor = uSamplers[_24].sample(uSamps[_35], in.vUV);
    out.FragColor = uCombinedSamplers[_24].sample(uCombinedSamplersSmplr[_24], in.vUV);
    out.FragColor += ubos[(in.vIndex + 20)]->v[_35];
    out.FragColor += ssbos[(in.vIndex + 50)]->v[in.vIndex + 60];
    return out;
}

