#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Foobar
{
    float a;
    float b;
};

constant float4 _37[3] = { float4(1.0), float4(2.0), float4(3.0) };
constant float4 _49[2] = { float4(1.0), float4(2.0) };
constant float4 _54[2] = { float4(8.0), float4(10.0) };
constant float4 _55[2][2] = { { float4(1.0), float4(2.0) }, { float4(8.0), float4(10.0) } };
constant Foobar _75[2] = { Foobar{ 10.0, 40.0 }, Foobar{ 90.0, 70.0 } };

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    int index [[user(locn0)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.FragColor = ((_37[in.index] + _55[in.index][in.index + 1]) + float4(30.0)) + float4(_75[in.index].a + _75[in.index].b);
    return out;
}

