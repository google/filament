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

struct main0_out
{
    float C_0 [[user(locn0_1)]];
    float D [[user(locn0_3)]];
    float A_0 [[user(locn1)]];
    float C_1 [[user(locn1_1)]];
    float2 B_0 [[user(locn1_2)]];
    float A_1 [[user(locn2)]];
    float C_2 [[user(locn2_1)]];
    float2 B_1 [[user(locn2_2)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 m_location_0 [[attribute(0)]];
    float4 m_location_1 [[attribute(1)]];
    float4 m_location_2 [[attribute(2)]];
    float4 Pos [[attribute(4)]];
};

vertex main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    spvUnsafeArray<float, 2> A = {};
    spvUnsafeArray<float2, 2> B = {};
    spvUnsafeArray<float, 3> C = {};
    spvUnsafeArray<float, 2> InA = {};
    spvUnsafeArray<float2, 2> InB = {};
    spvUnsafeArray<float, 3> InC = {};
    float InD = {};
    InA[0] = in.m_location_1.x;
    InA[1] = in.m_location_2.x;
    InB[0] = in.m_location_1.zw;
    InB[1] = in.m_location_2.zw;
    InC[0] = in.m_location_0.y;
    InC[1] = in.m_location_1.y;
    InC[2] = in.m_location_2.y;
    InD = in.m_location_0.w;
    out.gl_Position = in.Pos;
    A = InA;
    B = InB;
    C = InC;
    out.D = InD;
    out.A_0 = A[0];
    out.A_1 = A[1];
    out.B_0 = B[0];
    out.B_1 = B[1];
    out.C_0 = C[0];
    out.C_1 = C[1];
    out.C_2 = C[2];
    return out;
}

