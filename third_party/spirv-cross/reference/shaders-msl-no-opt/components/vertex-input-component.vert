#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float3 Foo [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 Foo3 [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    float3 Foo3 = {};
    float Foo1 = {};
    Foo3 = in.Foo3.xyz;
    Foo1 = in.Foo3.w;
    out.gl_Position = float4(Foo3, Foo1);
    out.Foo = Foo3 + float3(Foo1);
    return out;
}

