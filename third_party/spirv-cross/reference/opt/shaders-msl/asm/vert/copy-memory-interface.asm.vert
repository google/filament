#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 o1 [[user(locn1)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 v0 [[attribute(0)]];
    float4 v1 [[attribute(1)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.gl_Position = in.v0;
    out.o1 = in.v1;
    return out;
}

