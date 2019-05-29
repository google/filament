#pragma clang diagnostic ignored "-Wmissing-prototypes"

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

float4 resolve(thread const Foobar& f)
{
    return float4(f.a + f.b);
}

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    Foobar param = Foobar{ 10.0, 20.0 };
    Foobar param_1 = _75[in.index];
    out.FragColor = ((_37[in.index] + _55[in.index][in.index + 1]) + resolve(param)) + resolve(param_1);
    return out;
}

