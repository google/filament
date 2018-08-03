#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Structy
{
    float4 c;
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

void foo2(thread Structy& f)
{
    f.c = float4(10.0);
}

Structy foo()
{
    Structy param;
    foo2(param);
    Structy f = param;
    return f;
}

fragment main0_out main0()
{
    main0_out out = {};
    Structy s = foo();
    out.FragColor = s.c;
    return out;
}

