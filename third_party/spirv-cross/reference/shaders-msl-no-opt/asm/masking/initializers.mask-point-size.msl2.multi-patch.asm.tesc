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

struct gl_PerVertex
{
    float4 gl_Position;
    float gl_PointSize;
    spvUnsafeArray<float, 1> gl_ClipDistance;
    spvUnsafeArray<float, 1> gl_CullDistance;
};

constant spvUnsafeArray<float4, 4> _15 = spvUnsafeArray<float4, 4>({ float4(0.0), float4(0.0), float4(0.0), float4(0.0) });
constant spvUnsafeArray<float, 1> _45 = spvUnsafeArray<float, 1>({ 0.0 });
constant spvUnsafeArray<float, 1> _46 = spvUnsafeArray<float, 1>({ 0.0 });

struct main0_out
{
    float4 foo;
    float4 gl_Position;
    spvUnsafeArray<float, 1> gl_ClipDistance;
    spvUnsafeArray<float, 1> gl_CullDistance;
};

struct main0_patchOut
{
    float4 foo_patch;
};

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]])
{
    spvUnsafeArray<gl_PerVertex, 4> _29 = spvUnsafeArray<gl_PerVertex, 4>({ gl_PerVertex{ float4(0.0), 0.0, spvUnsafeArray<float, 1>({ 0.0 }), spvUnsafeArray<float, 1>({ 0.0 }) }, gl_PerVertex{ float4(0.0), 0.0, spvUnsafeArray<float, 1>({ 0.0 }), spvUnsafeArray<float, 1>({ 0.0 }) }, gl_PerVertex{ float4(0.0), 0.0, spvUnsafeArray<float, 1>({ 0.0 }), spvUnsafeArray<float, 1>({ 0.0 }) }, gl_PerVertex{ float4(0.0), 0.0, spvUnsafeArray<float, 1>({ 0.0 }), spvUnsafeArray<float, 1>({ 0.0 }) } });
    
    device main0_out* gl_out = &spvOut[gl_GlobalInvocationID.x - gl_GlobalInvocationID.x % 4];
    gl_out[gl_GlobalInvocationID.x % 4].foo = _15[gl_GlobalInvocationID.x % 4];
    gl_out[gl_GlobalInvocationID.x % 4].gl_Position = _29[gl_GlobalInvocationID.x % 4].gl_Position;
    gl_out[gl_GlobalInvocationID.x % 4].gl_ClipDistance = _29[gl_GlobalInvocationID.x % 4].gl_ClipDistance;
    gl_out[gl_GlobalInvocationID.x % 4].gl_CullDistance = _29[gl_GlobalInvocationID.x % 4].gl_CullDistance;
    threadgroup gl_PerVertex spvStoragegl_out_masked[8][4];
    threadgroup gl_PerVertex (&gl_out_masked)[4] = spvStoragegl_out_masked[(gl_GlobalInvocationID.x / 4) % 8];
    gl_out_masked[gl_GlobalInvocationID.x % 4] = _29[gl_GlobalInvocationID.x % 4];
    device main0_patchOut& patchOut = spvPatchOut[gl_GlobalInvocationID.x / 4];
    patchOut.foo_patch = float4(0.0);
    uint gl_InvocationID = gl_GlobalInvocationID.x % 4;
    uint gl_PrimitiveID = min(gl_GlobalInvocationID.x / 4, spvIndirectParams[1] - 1);
    gl_out[gl_InvocationID].foo = float4(1.0);
    patchOut.foo_patch = float4(2.0);
    gl_out[gl_InvocationID].gl_Position = float4(3.0);
    gl_out_masked[gl_InvocationID].gl_PointSize = 4.0;
}

