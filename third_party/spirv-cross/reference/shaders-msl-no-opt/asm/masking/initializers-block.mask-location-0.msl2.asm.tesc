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

struct C
{
    float4 v;
};

struct P
{
    float4 v;
};

struct main0_out
{
    float4 gl_Position;
    float gl_PointSize;
};

struct main0_patchOut
{
    float4 P_v;
};

kernel void main0(uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]])
{
    spvUnsafeArray<C, 4> _18 = spvUnsafeArray<C, 4>({ C{ float4(0.0) }, C{ float4(0.0) }, C{ float4(0.0) }, C{ float4(0.0) } });
    
    threadgroup C c[4];
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 4];
    c[gl_InvocationID] = _18[gl_InvocationID];
    device main0_patchOut& patchOut = spvPatchOut[gl_PrimitiveID];
    patchOut.P_v = float4(0.0);
    c[gl_InvocationID].v = float4(1.0);
    patchOut.P_v = float4(2.0);
    gl_out[gl_InvocationID].gl_Position = float4(3.0);
    gl_out[gl_InvocationID].gl_PointSize = 4.0;
}

