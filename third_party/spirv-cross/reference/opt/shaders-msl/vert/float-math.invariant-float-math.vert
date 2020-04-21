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

struct Matrices
{
    float4x4 vpMatrix;
    float4x4 wMatrix;
    float4x3 wMatrix4x3;
    float3x4 wMatrix3x4;
};

struct main0_out
{
    float3 OutNormal [[user(locn0)]];
    float4 OutWorldPos_0 [[user(locn1)]];
    float4 OutWorldPos_1 [[user(locn2)]];
    float4 OutWorldPos_2 [[user(locn3)]];
    float4 OutWorldPos_3 [[user(locn4)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float3 InPos [[attribute(0)]];
    float3 InNormal [[attribute(1)]];
};

template<typename T>
T spvFMul(T l, T r)
{
    return fma(l, r, T(0));
}

template<typename T, int Cols, int Rows>
vec<T, Cols> spvFMulVectorMatrix(vec<T, Rows> v, matrix<T, Cols, Rows> m)
{
    vec<T, Cols> res = vec<T, Cols>(0);
    for (uint i = Rows; i > 0; --i)
    {
        vec<T, Cols> tmp(0);
        for (uint j = 0; j < Cols; ++j)
        {
            tmp[j] = m[j][i - 1];
        }
        res = fma(tmp, vec<T, Cols>(v[i - 1]), res);
    }
    return res;
}

template<typename T, int Cols, int Rows>
vec<T, Rows> spvFMulMatrixVector(matrix<T, Cols, Rows> m, vec<T, Cols> v)
{
    vec<T, Rows> res = vec<T, Rows>(0);
    for (uint i = Cols; i > 0; --i)
    {
        res = fma(m[i - 1], vec<T, Rows>(v[i - 1]), res);
    }
    return res;
}

template<typename T, int LCols, int LRows, int RCols, int RRows>
matrix<T, RCols, LRows> spvFMulMatrixMatrix(matrix<T, LCols, LRows> l, matrix<T, RCols, RRows> r)
{
    matrix<T, RCols, LRows> res;
    for (uint i = 0; i < RCols; i++)
    {
        vec<T, RCols> tmp(0);
        for (uint j = 0; j < LCols; j++)
        {
            tmp = fma(vec<T, RCols>(r[i][j]), l[j], tmp);
        }
        res[i] = tmp;
    }
    return res;
}

vertex main0_out main0(main0_in in [[stage_in]], constant Matrices& _22 [[buffer(0)]])
{
    main0_out out = {};
    spvUnsafeArray<float4, 4> OutWorldPos = {};
    float4 _37 = float4(in.InPos, 1.0);
    out.gl_Position = spvFMulMatrixVector(spvFMulMatrixMatrix(_22.vpMatrix, _22.wMatrix), _37);
    OutWorldPos[0] = spvFMulMatrixVector(_22.wMatrix, _37);
    OutWorldPos[1] = spvFMulVectorMatrix(_37, _22.wMatrix);
    OutWorldPos[2] = spvFMulMatrixVector(_22.wMatrix3x4, in.InPos);
    OutWorldPos[3] = spvFMulVectorMatrix(in.InPos, _22.wMatrix4x3);
    out.OutNormal = spvFMulMatrixVector(_22.wMatrix, float4(in.InNormal, 0.0)).xyz;
    out.OutWorldPos_0 = OutWorldPos[0];
    out.OutWorldPos_1 = OutWorldPos[1];
    out.OutWorldPos_2 = OutWorldPos[2];
    out.OutWorldPos_3 = OutWorldPos[3];
    return out;
}

