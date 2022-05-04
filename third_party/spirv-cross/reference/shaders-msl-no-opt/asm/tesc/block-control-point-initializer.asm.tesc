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

struct Verts
{
    float a;
    float2 b;
};

struct main0_out
{
    float verts_a;
    float2 verts_b;
    float4 gl_Position;
};

kernel void main0(uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]])
{
    spvUnsafeArray<Verts, 4> _27 = spvUnsafeArray<Verts, 4>({ Verts{ 0.0, float2(0.0) }, Verts{ 0.0, float2(0.0) }, Verts{ 0.0, float2(0.0) }, Verts{ 0.0, float2(0.0) } });
    
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 4];
    gl_out[gl_InvocationID].verts_a = _27[gl_InvocationID].a;
    gl_out[gl_InvocationID].verts_b = _27[gl_InvocationID].b;
    gl_out[gl_InvocationID].gl_Position = float4(1.0);
    gl_out[gl_InvocationID].verts_a = float(gl_InvocationID);
}

