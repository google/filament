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
    spvUnsafeArray<float4, 2> v1;
    float4 v3;
};

static inline __attribute__((always_inline))
void write_in_func(threadgroup float4 (&v0)[4], thread uint& gl_InvocationID, device spvUnsafeArray<float4, 2>& v1, device float4& v3, device main0_out* thread & gl_out)
{
    v0[gl_InvocationID] = float4(1.0);
    v0[gl_InvocationID].z = 3.0;
    if (gl_InvocationID == 0)
    {
        v1[0] = float4(2.0);
        ((device float*)&v1[0])[0u] = 3.0;
        v1[1] = float4(2.0);
        ((device float*)&v1[1])[0u] = 5.0;
    }
    v3 = float4(5.0);
    gl_out[gl_InvocationID].gl_Position = float4(10.0);
    gl_out[gl_InvocationID].gl_Position.z = 20.0;
    gl_out[gl_InvocationID].gl_PointSize = 40.0;
}

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]])
{
    device main0_out* gl_out = &spvOut[gl_GlobalInvocationID.x - gl_GlobalInvocationID.x % 4];
    threadgroup float4 spvStoragev0[8][4];
    threadgroup float4 (&v0)[4] = spvStoragev0[(gl_GlobalInvocationID.x / 4) % 8];
    device main0_patchOut& patchOut = spvPatchOut[gl_GlobalInvocationID.x / 4];
    uint gl_InvocationID = gl_GlobalInvocationID.x % 4;
    uint gl_PrimitiveID = min(gl_GlobalInvocationID.x / 4, spvIndirectParams[1] - 1);
    write_in_func(v0, gl_InvocationID, patchOut.v1, patchOut.v3, gl_out);
}

