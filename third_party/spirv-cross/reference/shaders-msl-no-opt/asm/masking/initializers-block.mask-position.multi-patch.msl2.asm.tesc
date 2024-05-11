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

struct gl_PerVertex
{
    float4 gl_Position;
    float gl_PointSize;
    spvUnsafeArray<float, 1> gl_ClipDistance;
    spvUnsafeArray<float, 1> gl_CullDistance;
};

constant spvUnsafeArray<float, 1> _51 = spvUnsafeArray<float, 1>({ 0.0 });
constant spvUnsafeArray<float, 1> _52 = spvUnsafeArray<float, 1>({ 0.0 });

struct main0_out
{
    float4 c_v;
    float gl_PointSize;
    spvUnsafeArray<float, 1> gl_ClipDistance;
    spvUnsafeArray<float, 1> gl_CullDistance;
};

struct main0_patchOut
{
    float4 p_v;
};

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]])
{
    spvUnsafeArray<C, 4> _18 = spvUnsafeArray<C, 4>({ C{ float4(0.0) }, C{ float4(0.0) }, C{ float4(0.0) }, C{ float4(0.0) } });
    spvUnsafeArray<gl_PerVertex, 4> _33 = spvUnsafeArray<gl_PerVertex, 4>({ gl_PerVertex{ float4(0.0), 0.0, spvUnsafeArray<float, 1>({ 0.0 }), spvUnsafeArray<float, 1>({ 0.0 }) }, gl_PerVertex{ float4(0.0), 0.0, spvUnsafeArray<float, 1>({ 0.0 }), spvUnsafeArray<float, 1>({ 0.0 }) }, gl_PerVertex{ float4(0.0), 0.0, spvUnsafeArray<float, 1>({ 0.0 }), spvUnsafeArray<float, 1>({ 0.0 }) }, gl_PerVertex{ float4(0.0), 0.0, spvUnsafeArray<float, 1>({ 0.0 }), spvUnsafeArray<float, 1>({ 0.0 }) } });
    
    device main0_out* gl_out = &spvOut[gl_GlobalInvocationID.x - gl_GlobalInvocationID.x % 4];
    gl_out[gl_GlobalInvocationID.x % 4].c_v = _18[gl_GlobalInvocationID.x % 4].v;
    gl_out[gl_GlobalInvocationID.x % 4].gl_PointSize = _33[gl_GlobalInvocationID.x % 4].gl_PointSize;
    gl_out[gl_GlobalInvocationID.x % 4].gl_ClipDistance = _33[gl_GlobalInvocationID.x % 4].gl_ClipDistance;
    gl_out[gl_GlobalInvocationID.x % 4].gl_CullDistance = _33[gl_GlobalInvocationID.x % 4].gl_CullDistance;
    threadgroup gl_PerVertex spvStoragegl_out_masked[8][4];
    threadgroup gl_PerVertex (&gl_out_masked)[4] = spvStoragegl_out_masked[(gl_GlobalInvocationID.x / 4) % 8];
    gl_out_masked[gl_GlobalInvocationID.x % 4] = _33[gl_GlobalInvocationID.x % 4];
    device main0_patchOut& patchOut = spvPatchOut[gl_GlobalInvocationID.x / 4];
    patchOut.p_v = float4(0.0);
    uint gl_InvocationID = gl_GlobalInvocationID.x % 4;
    uint gl_PrimitiveID = min(gl_GlobalInvocationID.x / 4, spvIndirectParams[1] - 1);
    gl_out[gl_InvocationID].c_v = float4(1.0);
    patchOut.p_v = float4(2.0);
    gl_out_masked[gl_InvocationID].gl_Position = float4(3.0);
    gl_out[gl_InvocationID].gl_PointSize = 4.0;
}

