#pragma clang diagnostic ignored "-Wmissing-prototypes"
#pragma clang diagnostic ignored "-Wmissing-braces"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

template<typename T, size_t Num>
struct spvUnsafeArray
{
    T elements[Num ? Num : 1];
    
    thread T& operator [] (size_t pos) thread
    {
        return elements[pos];
    }
    constexpr const thread T& operator [] (size_t pos) const thread
    {
        return elements[pos];
    }
    
    device T& operator [] (size_t pos) device
    {
        return elements[pos];
    }
    constexpr const device T& operator [] (size_t pos) const device
    {
        return elements[pos];
    }
    
    constexpr const constant T& operator [] (size_t pos) const constant
    {
        return elements[pos];
    }
    
    threadgroup T& operator [] (size_t pos) threadgroup
    {
        return elements[pos];
    }
    constexpr const threadgroup T& operator [] (size_t pos) const threadgroup
    {
        return elements[pos];
    }
};

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

vertex main0_out main0(main0_in in [[stage_in]], constant UBO& ubo [[buffer(0)]])
{
    main0_out out = {};
    float4x4 outTransModel = {};
    spvUnsafeArray<float4, 3> colors = {};
    float4x4 inViewMat = {};
    colors[0] = in.colors_0;
    colors[1] = in.colors_1;
    colors[2] = in.colors_2;
    inViewMat[0] = in.inViewMat_0;
    inViewMat[1] = in.inViewMat_1;
    inViewMat[2] = in.inViewMat_2;
    inViewMat[3] = in.inViewMat_3;
    float4 _64 = float4(in.inPos, 1.0);
    out.gl_Position = (ubo.projection * ubo.model) * _64;
    out.outPos = float3((ubo.model * _64).xyz);
    out.outNormal = float3x3(float3(ubo.model[0].x, ubo.model[0].y, ubo.model[0].z), float3(ubo.model[1].x, ubo.model[1].y, ubo.model[1].z), float3(ubo.model[2].x, ubo.model[2].y, ubo.model[2].z)) * in.inNormal;
    out.outLodBias = ubo.lodBias;
    outTransModel = transpose(ubo.model) * inViewMat;
    outTransModel[2] = float4(in.inNormal, 1.0);
    outTransModel[1].y = ubo.lodBias;
    out.color = colors[2];
    out.outTransModel_0 = outTransModel[0];
    out.outTransModel_1 = outTransModel[1];
    out.outTransModel_2 = outTransModel[2];
    out.outTransModel_3 = outTransModel[3];
    return out;
}

