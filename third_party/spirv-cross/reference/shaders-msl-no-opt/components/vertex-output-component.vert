#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float3 Foo3 [[user(locn0)]];
    float Foo1 [[user(locn0_3)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 vFoo [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.gl_Position = in.vFoo;
    out.Foo3 = in.vFoo.xyz;
    out.Foo1 = in.vFoo.w;
    return out;
}

