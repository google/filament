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

fragment main0_out main0(main0_in in [[stage_in]], constant UBO* ubos_0 [[buffer(0)]], constant UBO* ubos_1 [[buffer(1)]], const device SSBO* ssbos_0 [[buffer(2)]], const device SSBO* ssbos_1 [[buffer(3)]], array<texture2d<float>, 8> uSamplers [[texture(0)]], array<texture2d<float>, 8> uCombinedSamplers [[texture(8)]], array<sampler, 7> uSamps [[sampler(0)]], array<sampler, 8> uCombinedSamplersSmplr [[sampler(7)]])
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
    int _25 = in.vIndex + 10;
    int _37 = in.vIndex + 40;
    out.FragColor = uSamplers[_25].sample(uSamps[_37], in.vUV);
    out.FragColor = uCombinedSamplers[_25].sample(uCombinedSamplersSmplr[_25], in.vUV);
    int _69 = in.vIndex + 20;
    out.FragColor += ubos[(_69)]->v[_37];
    int _87 = in.vIndex + 50;
    int _91 = in.vIndex + 60;
    out.FragColor += ssbos[(_87)]->v[_91];
    return out;
}

