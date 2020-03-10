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
    spvUnsafeArray<float, 3> arr;
    float3x3 wMatrix;
    float4 wTmp;
};

struct main0_out
{
    float3 vMatrix_0 [[user(locn0)]];
    float3 vMatrix_1 [[user(locn1)]];
    float3 vMatrix_2 [[user(locn2)]];
    float Vert_arr_0 [[user(locn4)]];
    float Vert_arr_1 [[user(locn5)]];
    float Vert_arr_2 [[user(locn6)]];
    float3 Vert_wMatrix_0 [[user(locn7)]];
    float3 Vert_wMatrix_1 [[user(locn8)]];
    float3 Vert_wMatrix_2 [[user(locn9)]];
    float4 Vert_wTmp [[user(locn10)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float3 Matrix_0 [[attribute(0)]];
    float3 Matrix_1 [[attribute(1)]];
    float3 Matrix_2 [[attribute(2)]];
    float4 Pos [[attribute(4)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    float3x3 vMatrix = {};
    Vert _20 = {};
    float3x3 Matrix = {};
    Matrix[0] = in.Matrix_0;
    Matrix[1] = in.Matrix_1;
    Matrix[2] = in.Matrix_2;
    vMatrix = Matrix;
    _20.wMatrix = Matrix;
    _20.arr[0] = 1.0;
    _20.arr[1] = 2.0;
    _20.arr[2] = 3.0;
    _20.wTmp = in.Pos;
    out.gl_Position = in.Pos;
    out.vMatrix_0 = vMatrix[0];
    out.vMatrix_1 = vMatrix[1];
    out.vMatrix_2 = vMatrix[2];
    out.Vert_arr_0 = _20.arr[0];
    out.Vert_arr_1 = _20.arr[1];
    out.Vert_arr_2 = _20.arr[2];
    out.Vert_wMatrix_0 = _20.wMatrix[0];
    out.Vert_wMatrix_1 = _20.wMatrix[1];
    out.Vert_wMatrix_2 = _20.wMatrix[2];
    out.Vert_wTmp = _20.wTmp;
    return out;
}

