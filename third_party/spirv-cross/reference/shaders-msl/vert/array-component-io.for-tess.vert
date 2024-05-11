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
    float4 m_location_0;
    float4 m_location_1;
    float4 m_location_2;
    float4 gl_Position;
};

struct main0_in
{
    float4 m_location_0 [[attribute(0)]];
    float4 m_location_1 [[attribute(1)]];
    float4 m_location_2 [[attribute(2)]];
    float4 Pos [[attribute(4)]];
};

kernel void main0(main0_in in [[stage_in]], uint3 gl_GlobalInvocationID [[thread_position_in_grid]], uint3 spvStageInputSize [[grid_size]], device main0_out* spvOut [[buffer(28)]])
{
    spvUnsafeArray<float, 2> A = {};
    spvUnsafeArray<float2, 2> B = {};
    spvUnsafeArray<float, 3> C = {};
    float D = {};
    spvUnsafeArray<float, 2> InA = {};
    spvUnsafeArray<float2, 2> InB = {};
    spvUnsafeArray<float, 3> InC = {};
    float InD = {};
    device main0_out& out = spvOut[gl_GlobalInvocationID.y * spvStageInputSize.x + gl_GlobalInvocationID.x];
    InA[0] = in.m_location_1.x;
    InA[1] = in.m_location_2.x;
    InB[0] = in.m_location_1.zw;
    InB[1] = in.m_location_2.zw;
    InC[0] = in.m_location_0.y;
    InC[1] = in.m_location_1.y;
    InC[2] = in.m_location_2.y;
    InD = in.m_location_0.w;
    if (any(gl_GlobalInvocationID >= spvStageInputSize))
        return;
    out.gl_Position = in.Pos;
    A = InA;
    B = InB;
    C = InC;
    D = InD;
    out.m_location_1.x = A[0];
    out.m_location_2.x = A[1];
    out.m_location_1.zw = B[0];
    out.m_location_2.zw = B[1];
    out.m_location_0.y = C[0];
    out.m_location_1.y = C[1];
    out.m_location_2.y = C[2];
    out.m_location_0.w = D;
}

