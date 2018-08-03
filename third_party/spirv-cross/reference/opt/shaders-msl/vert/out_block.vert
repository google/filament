#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Transform
{
    float4x4 transform;
};

struct main0_out
{
    float4 VertexOut_color [[user(locn2)]];
    float4 VertexOut_color2 [[user(locn3)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float3 position [[attribute(0)]];
    float4 color [[attribute(1)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant Transform& block [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = block.transform * float4(in.position, 1.0);
    out.VertexOut_color = in.color;
    out.VertexOut_color2 = in.color + float4(1.0);
    return out;
}

