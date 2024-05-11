#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Foo
{
    int a;
};

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
    int uninit_function_int = {};
    int uninit_int = {};
    int4 uninit_vector = {};
    float4x4 uninit_matrix = {};
    Foo uninit_foo = {};
    if (in.vColor.x > 10.0)
    {
        uninit_function_int = 10;
    }
    else
    {
        uninit_function_int = 20;
    }
    out.FragColor = in.vColor;
    return out;
}

