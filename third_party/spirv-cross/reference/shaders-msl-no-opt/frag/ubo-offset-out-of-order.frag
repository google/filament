#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO
{
    float4 v;
    float4x4 m;
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float4 vColor [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]], constant UBO& _13 [[buffer(0)]])
{
    main0_out out = {};
    out.FragColor = (_13.m * in.vColor) + _13.v;
    return out;
}

