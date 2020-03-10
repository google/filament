#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float4 vUV [[user(locn0)]];
};

static inline __attribute__((always_inline))
float3 spvCubemapTo2DArrayFace(float3 P)
{
    float3 Coords = abs(P.xyz);
    float CubeFace = 0;
    float ProjectionAxis = 0;
    float u = 0;
    float v = 0;
    if (Coords.x >= Coords.y && Coords.x >= Coords.z)
    {
        CubeFace = P.x >= 0 ? 0 : 1;
        ProjectionAxis = Coords.x;
        u = P.x >= 0 ? -P.z : P.z;
        v = -P.y;
    }
    else if (Coords.y >= Coords.x && Coords.y >= Coords.z)
    {
        CubeFace = P.y >= 0 ? 2 : 3;
        ProjectionAxis = Coords.y;
        u = P.x;
        v = P.y >= 0 ? P.z : -P.z;
    }
    else
    {
        CubeFace = P.z >= 0 ? 4 : 5;
        ProjectionAxis = Coords.z;
        u = P.z >= 0 ? P.x : -P.x;
        v = -P.y;
    }
    u = 0.5 * (u/ProjectionAxis + 1);
    v = 0.5 * (v/ProjectionAxis + 1);
    return float3(u, v, CubeFace);
}

fragment main0_out main0(main0_in in [[stage_in]], texturecube<float> cubeSampler [[texture(0)]], texture2d_array<float> cubeArraySampler [[texture(1)]], texture2d_array<float> texArraySampler [[texture(2)]], sampler cubeSamplerSmplr [[sampler(0)]], sampler cubeArraySamplerSmplr [[sampler(1)]], sampler texArraySamplerSmplr [[sampler(2)]])
{
    main0_out out = {};
    float4 a = cubeSampler.sample(cubeSamplerSmplr, in.vUV.xyz);
    float4 b = cubeArraySampler.sample(cubeArraySamplerSmplr, spvCubemapTo2DArrayFace(in.vUV.xyz).xy, uint(spvCubemapTo2DArrayFace(in.vUV.xyz).z) + (uint(round(in.vUV.w)) * 6u));
    float4 c = texArraySampler.sample(texArraySamplerSmplr, in.vUV.xyz.xy, uint(round(in.vUV.xyz.z)));
    out.FragColor = (a + b) + c;
    return out;
}

