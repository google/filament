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
    float4 m_location_0 [[color(0)]];
    float4 m_location_1 [[color(1)]];
    float4 m_location_2 [[color(2)]];
};

struct main0_in
{
    float InC_0 [[user(locn0_1), flat]];
    float InA_0 [[user(locn1), flat]];
    float InC_1 [[user(locn1_1), flat]];
    float2 InB_0 [[user(locn1_2), flat]];
    float InA_1 [[user(locn2), flat]];
    float InC_2 [[user(locn2_1), flat]];
    float2 InB_1 [[user(locn2_2), flat]];
    float InD [[user(locn3_1), sample_perspective]];
    float InE [[user(locn4_2), center_no_perspective]];
    float InF [[user(locn5_3), centroid_perspective]];
};

fragment main0_out main0(main0_in in [[stage_in]])
{
    main0_out out = {};
    spvUnsafeArray<float, 2> A = {};
    spvUnsafeArray<float2, 2> B = {};
    spvUnsafeArray<float, 3> C = {};
    float D = {};
    spvUnsafeArray<float, 2> InA = {};
    spvUnsafeArray<float2, 2> InB = {};
    spvUnsafeArray<float, 3> InC = {};
    InA[0] = in.InA_0;
    InA[1] = in.InA_1;
    InB[0] = in.InB_0;
    InB[1] = in.InB_1;
    InC[0] = in.InC_0;
    InC[1] = in.InC_1;
    InC[2] = in.InC_2;
    A = InA;
    B = InB;
    C = InC;
    D = (in.InD + in.InE) + in.InF;
    out.m_location_1.x = A[0];
    out.m_location_2.x = A[1];
    out.m_location_1.zw = B[0];
    out.m_location_2.zw = B[1];
    out.m_location_0.y = C[0];
    out.m_location_1.y = C[1];
    out.m_location_2.y = C[2];
    out.m_location_0.w = D;
    return out;
}

