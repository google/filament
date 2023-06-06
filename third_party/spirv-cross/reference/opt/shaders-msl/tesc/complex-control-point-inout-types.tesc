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

struct Meep
{
    float a;
    float b;
};

struct Block
{
    spvUnsafeArray<float, 2> a;
    float b;
    float2x2 m;
    Meep meep;
    spvUnsafeArray<Meep, 2> meeps;
};

struct Block_1
{
    spvUnsafeArray<float, 2> a;
    float b;
    float2x2 m;
};

struct main0_out
{
    spvUnsafeArray<float, 2> a;
    float b;
    float2x2 m;
    Meep meep;
    spvUnsafeArray<Meep, 2> meeps;
    spvUnsafeArray<float, 2> B_a;
    float B_b;
    float2x2 B_m;
    Meep B_meep;
    spvUnsafeArray<Meep, 2> B_meeps;
    float4 gl_Position;
};

struct main0_in
{
    float in_a_0 [[attribute(0)]];
    float in_a_1 [[attribute(1)]];
    float in_b [[attribute(2)]];
    float2 in_m_0 [[attribute(3)]];
    float2 in_m_1 [[attribute(4)]];
    float in_meep_a [[attribute(5)]];
    float in_meep_b [[attribute(6)]];
    float in_B_a_0 [[attribute(11)]];
    float in_B_a_1 [[attribute(12)]];
    float in_B_b [[attribute(13)]];
    float2 in_B_m_0 [[attribute(14)]];
    float2 in_B_m_1 [[attribute(15)]];
};

kernel void main0(main0_in in [[stage_in]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], threadgroup main0_in* gl_in [[threadgroup(0)]])
{
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 4];
    if (gl_InvocationID < spvIndirectParams[0])
        gl_in[gl_InvocationID] = in;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (gl_InvocationID >= 4)
        return;
    gl_out[gl_InvocationID].gl_Position = float4(1.0);
    gl_out[gl_InvocationID].a[0] = gl_in[gl_InvocationID].in_a_0;
    gl_out[gl_InvocationID].a[1] = gl_in[gl_InvocationID].in_a_1;
    gl_out[gl_InvocationID].b = gl_in[gl_InvocationID].in_b;
    float2x2 _178 = float2x2(gl_in[gl_InvocationID].in_m_0, gl_in[gl_InvocationID].in_m_1);
    gl_out[gl_InvocationID].m = _178;
    gl_out[gl_InvocationID].meep.a = gl_in[gl_InvocationID].in_meep_a;
    gl_out[gl_InvocationID].meep.b = gl_in[gl_InvocationID].in_meep_b;
    gl_out[gl_InvocationID].meeps[0].a = 1.0;
    gl_out[gl_InvocationID].meeps[0].b = 2.0;
    gl_out[gl_InvocationID].meeps[1].a = 3.0;
    gl_out[gl_InvocationID].meeps[1].b = 4.0;
    gl_out[gl_InvocationID].B_a[0] = gl_in[gl_InvocationID].in_B_a_0;
    gl_out[gl_InvocationID].B_a[1] = gl_in[gl_InvocationID].in_B_a_1;
    gl_out[gl_InvocationID].B_b = gl_in[gl_InvocationID].in_B_b;
    float2x2 _216 = float2x2(gl_in[gl_InvocationID].in_B_m_0, gl_in[gl_InvocationID].in_B_m_1);
    gl_out[gl_InvocationID].B_m = _216;
    gl_out[gl_InvocationID].B_meep.a = 10.0;
    gl_out[gl_InvocationID].B_meep.b = 20.0;
    gl_out[gl_InvocationID].B_meeps[0].a = 5.0;
    gl_out[gl_InvocationID].B_meeps[0].b = 6.0;
    gl_out[gl_InvocationID].B_meeps[1].a = 7.0;
    gl_out[gl_InvocationID].B_meeps[1].b = 8.0;
}

