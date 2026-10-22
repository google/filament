#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VertexOut
{
    float4 color;
    float4 color2;
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float4 inputs_color [[user(locn2)]];
    float4 inputs_color2 [[user(locn3)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    VertexOut inputs = {};
    inputs.color = in.inputs_color;
    inputs.color2 = in.inputs_color2;
    out.FragColor = inputs.color + inputs.color2;
    return out;
}

