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

constant spvUnsafeArray<float, 3> _49 = spvUnsafeArray<float, 3>({ 0.0, 0.0, 0.0 });
constant spvUnsafeArray<spvUnsafeArray<float, 3>, 4> _27 = spvUnsafeArray<spvUnsafeArray<float, 3>, 4>({ spvUnsafeArray<float, 3>({ 0.0, 0.0, 0.0 }), spvUnsafeArray<float, 3>({ 0.0, 0.0, 0.0 }), spvUnsafeArray<float, 3>({ 0.0, 0.0, 0.0 }), spvUnsafeArray<float, 3>({ 0.0, 0.0, 0.0 }) });

struct main0_out
{
    spvUnsafeArray<float, 3> foo;
    float4 gl_Position;
};

kernel void main0(uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]])
{
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 4];
    gl_out[gl_InvocationID].foo = _27[gl_InvocationID];
    gl_out[gl_InvocationID].gl_Position = float4(1.0);
    gl_out[gl_InvocationID].foo[0] = float(gl_InvocationID);
    gl_out[gl_InvocationID].foo[1] = float(gl_InvocationID) + 1.0;
    gl_out[gl_InvocationID].foo[2] = float(gl_InvocationID) + 2.0;
}

