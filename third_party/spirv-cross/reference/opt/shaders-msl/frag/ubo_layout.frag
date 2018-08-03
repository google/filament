#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Str
{
    float4x4 foo;
};

struct UBO1
{
    Str foo;
};

struct Str_1
{
    float4x4 foo;
};

struct UBO2
{
    Str_1 foo;
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

fragment main0_out main0(constant UBO1& ubo1 [[buffer(0)]], constant UBO2& ubo0 [[buffer(1)]])
{
    main0_out out = {};
    out.FragColor = transpose(ubo1.foo.foo)[0] + ubo0.foo.foo[0];
    return out;
}

