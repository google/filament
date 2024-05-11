#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Foo
{
    float4 v;
};

struct UBO
{
    Foo foo;
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(constant UBO* ubos_0 [[buffer(0)]], constant UBO* ubos_1 [[buffer(1)]])
{
    constant UBO* ubos[] =
    {
        ubos_0,
        ubos_1,
    };

    main0_out out = {};
    out.FragColor = ubos[1]->foo.v;
    return out;
}

