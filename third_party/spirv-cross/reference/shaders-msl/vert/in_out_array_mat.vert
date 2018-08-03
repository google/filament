#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO
{
    float4x4 projection;
    float4x4 model;
    float lodBias;
};

struct main0_out
{
    float3 outPos [[user(locn0)]];
    float3 outNormal [[user(locn1)]];
    float4 outTransModel_0 [[user(locn2)]];
    float4 outTransModel_1 [[user(locn3)]];
    float4 outTransModel_2 [[user(locn4)]];
    float4 outTransModel_3 [[user(locn5)]];
    float outLodBias [[user(locn6)]];
    float4 color [[user(locn7)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float3 inPos [[attribute(0)]];
    float4 colors_0 [[attribute(1)]];
    float4 colors_1 [[attribute(2)]];
    float4 colors_2 [[attribute(3)]];
    float3 inNormal [[attribute(4)]];
    float4 inViewMat_0 [[attribute(5)]];
    float4 inViewMat_1 [[attribute(6)]];
    float4 inViewMat_2 [[attribute(7)]];
    float4 inViewMat_3 [[attribute(8)]];
};

void write_deeper_in_function(thread float4x4& outTransModel, constant UBO& ubo, thread float4& color, thread float4 (&colors)[3])
{
    outTransModel[1].y = ubo.lodBias;
    color = colors[2];
}

void write_in_function(thread float4x4& outTransModel, constant UBO& ubo, thread float4& color, thread float4 (&colors)[3], thread float3& inNormal)
{
    outTransModel[2] = float4(inNormal, 1.0);
    write_deeper_in_function(outTransModel, ubo, color, colors);
}

vertex main0_out main0(main0_in in [[stage_in]], constant UBO& ubo [[buffer(0)]])
{
    main0_out out = {};
    float4x4 outTransModel = {};
    float4 colors[3] = {};
    float4x4 inViewMat = {};
    colors[0] = in.colors_0;
    colors[1] = in.colors_1;
    colors[2] = in.colors_2;
    inViewMat[0] = in.inViewMat_0;
    inViewMat[1] = in.inViewMat_1;
    inViewMat[2] = in.inViewMat_2;
    inViewMat[3] = in.inViewMat_3;
    out.gl_Position = (ubo.projection * ubo.model) * float4(in.inPos, 1.0);
    out.outPos = float3((ubo.model * float4(in.inPos, 1.0)).xyz);
    out.outNormal = float3x3(float3(float3(ubo.model[0].x, ubo.model[0].y, ubo.model[0].z)), float3(float3(ubo.model[1].x, ubo.model[1].y, ubo.model[1].z)), float3(float3(ubo.model[2].x, ubo.model[2].y, ubo.model[2].z))) * in.inNormal;
    out.outLodBias = ubo.lodBias;
    outTransModel = transpose(ubo.model) * inViewMat;
    write_in_function(outTransModel, ubo, out.color, colors, in.inNormal);
    out.outTransModel_0 = outTransModel[0];
    out.outTransModel_1 = outTransModel[1];
    out.outTransModel_2 = outTransModel[2];
    out.outTransModel_3 = outTransModel[3];
    return out;
}

