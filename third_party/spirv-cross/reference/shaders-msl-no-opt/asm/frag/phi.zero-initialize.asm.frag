#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

constant int uninit_int = {};
constant int4 uninit_vector = {};
constant float4x4 uninit_matrix = {};

struct Foo
{
    int a;
};

constant Foo uninit_foo = {};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float4 vColor [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    int _27 = {};
    if (in.vColor.x > 10.0)
    {
        _27 = 10;
    }
    else
    {
        _27 = 20;
    }
    out.FragColor = in.vColor;
    return out;
}

