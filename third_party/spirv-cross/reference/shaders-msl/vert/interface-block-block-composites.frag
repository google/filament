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

struct Vert
{
    float3x3 wMatrix;
    float4 wTmp;
    spvUnsafeArray<float, 4> arr;
};

struct main0_out
{
    float4 FragColor [[color(0)]];
};

struct main0_in
{
    float3 vMatrix_0 [[user(locn0)]];
    float3 vMatrix_1 [[user(locn1)]];
    float3 vMatrix_2 [[user(locn2)]];
    float3 m_17_wMatrix_0 [[user(locn4)]];
    float3 m_17_wMatrix_1 [[user(locn5)]];
    float3 m_17_wMatrix_2 [[user(locn6)]];
    float4 m_17_wTmp [[user(locn7)]];
    float m_17_arr_0 [[user(locn8)]];
    float m_17_arr_1 [[user(locn9)]];
    float m_17_arr_2 [[user(locn10)]];
    float m_17_arr_3 [[user(locn11)]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    Vert _17 = {};
    float3x3 vMatrix = {};
    _17.wMatrix[0] = in.m_17_wMatrix_0;
    _17.wMatrix[1] = in.m_17_wMatrix_1;
    _17.wMatrix[2] = in.m_17_wMatrix_2;
    _17.wTmp = in.m_17_wTmp;
    _17.arr[0] = in.m_17_arr_0;
    _17.arr[1] = in.m_17_arr_1;
    _17.arr[2] = in.m_17_arr_2;
    _17.arr[3] = in.m_17_arr_3;
    vMatrix[0] = in.vMatrix_0;
    vMatrix[1] = in.vMatrix_1;
    vMatrix[2] = in.vMatrix_2;
    out.FragColor = (_17.wMatrix[0].xxyy + _17.wTmp) + vMatrix[1].yyzz;
    for (int i = 0; i < 4; i++)
    {
        out.FragColor += float4(_17.arr[i]);
    }
    return out;
}

