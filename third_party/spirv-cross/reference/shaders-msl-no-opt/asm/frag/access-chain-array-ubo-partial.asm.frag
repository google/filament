#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBOs
{
    float4 v;
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(constant UBOs* ubos_0 [[buffer(0)]], constant UBOs* ubos_1 [[buffer(1)]])
{
    constant UBOs* ubos[] =
    {
        ubos_0,
        ubos_1,
    };

    main0_out out = {};
    out.FragColor = ubos[0]->v + ubos[1]->v;
    return out;
}

