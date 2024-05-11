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

struct VertexData
{
    float4x4 a;
    spvUnsafeArray<float4, 2> b;
    float4 c;
};

struct main0_out
{
    float4 vOutputs;
};

struct main0_in
{
    float4 vInputs_a_0 [[attribute(0)]];
    float4 vInputs_a_1 [[attribute(1)]];
    float4 vInputs_a_2 [[attribute(2)]];
    float4 vInputs_a_3 [[attribute(3)]];
    float4 vInputs_b_0 [[attribute(4)]];
    float4 vInputs_b_1 [[attribute(5)]];
    float4 vInputs_c [[attribute(6)]];
};

kernel void main0(main0_in in [[stage_in]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], threadgroup main0_in* gl_in [[threadgroup(0)]])
{
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 4];
    if (gl_InvocationID < spvIndirectParams[0])
        gl_in[gl_InvocationID] = in;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (gl_InvocationID >= 4)
        return;
    int _27 = gl_InvocationID ^ 1;
    gl_out[gl_InvocationID].vOutputs = ((gl_in[gl_InvocationID].vInputs_a_1 + gl_in[gl_InvocationID].vInputs_b_1) + gl_in[gl_InvocationID].vInputs_c) + gl_in[_27].vInputs_c;
}

