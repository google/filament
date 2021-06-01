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

constant spvUnsafeArray<float, 2> _19 = spvUnsafeArray<float, 2>({ 1.0, 2.0 });
constant spvUnsafeArray<float, 4> _25 = spvUnsafeArray<float, 4>({ 1.0, 2.0, 3.0, 4.0 });

struct main0_out
{
    float4 gl_Position;
};

kernel void main0(uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLTriangleTessellationFactorsHalf* spvTessLevel [[buffer(26)]])
{
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 1];
    spvTessLevel[gl_PrimitiveID].insideTessellationFactor = half(_19[0]);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0] = half(_25[0]);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1] = half(_25[1]);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2] = half(_25[2]);
    spvUnsafeArray<float, 2> inner;
    inner = spvUnsafeArray<float, 2>({ float(spvTessLevel[gl_PrimitiveID].insideTessellationFactor), 0.0 });
    spvUnsafeArray<float, 4> outer;
    outer = spvUnsafeArray<float, 4>({ float(spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0]), float(spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1]), float(spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2]), 0.0 });
    gl_out[gl_InvocationID].gl_Position = float4(1.0);
}

