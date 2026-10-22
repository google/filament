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

struct Block
{
    spvUnsafeArray<float, 4> block0;
};

constant spvUnsafeArray<float, 4> _30 = spvUnsafeArray<float, 4>({ 1.0, 2.0, -1.0, -2.0 });

struct main0_out
{
    spvUnsafeArray<float, 4> F_array;
    spvUnsafeArray<float, 4> m_43_block0;
    float4 gl_Position;
    spvUnsafeArray<float, 4> gl_ClipDistance;
};

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], uint3 spvStageInputSize [[grid_size]], device main0_out* spvOut [[buffer(28)]])
{
    Block _43 = {};
    device main0_out& out = spvOut[gl_GlobalInvocationID.y * spvStageInputSize.x + gl_GlobalInvocationID.x];
    if (any(gl_GlobalInvocationID >= spvStageInputSize))
        return;
    out.gl_Position = float4(1.0, 2.0, 3.0, 4.0);
    out.gl_ClipDistance = _30;
    spvUnsafeArray<float, 4> _51 = out.gl_ClipDistance;
    out.gl_ClipDistance = _51;
    out.F_array = _51;
    _43.block0 = _30;
    out.m_43_block0 = _43.block0;
}

