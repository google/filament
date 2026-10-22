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
    float4 gl_Position;
    float gl_PointSize;
};

struct main0_patchOut
{
    float4 v1;
};

static inline __attribute__((always_inline))
void write_in_func(threadgroup spvUnsafeArray<float4, 4>& v0, thread uint& gl_InvocationID, device float4& v1, device main0_out* thread & gl_out)
{
    v0[gl_InvocationID] = float4(1.0);
    v0[gl_InvocationID].x = 2.0;
    if (gl_InvocationID == 0)
    {
        v1 = float4(2.0);
        ((device float*)&v1)[3u] = 4.0;
    }
    gl_out[gl_InvocationID].gl_Position = float4(3.0);
    gl_out[gl_InvocationID].gl_PointSize = 4.0;
    gl_out[gl_InvocationID].gl_Position.z = 5.0;
    gl_out[gl_InvocationID].gl_PointSize = 4.0;
}

kernel void main0(uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]])
{
    threadgroup spvUnsafeArray<float4, 4> v0;
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 4];
    device main0_patchOut& patchOut = spvPatchOut[gl_PrimitiveID];
    write_in_func(v0, gl_InvocationID, patchOut.v1, gl_out);
}

