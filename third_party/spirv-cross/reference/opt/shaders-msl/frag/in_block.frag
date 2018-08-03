#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float4 VertexOut_color [[user(locn2)]];
    float4 VertexOut_color2 [[user(locn3)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    out.FragColor = in.VertexOut_color + in.VertexOut_color2;
    return out;
}

