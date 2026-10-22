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
    float3 vVertex;
};

struct main0_patchOut
{
    spvUnsafeArray<float3, 2> vPatch;
};

struct main0_in
{
    float3 vInput [[attribute(0)]];
};

kernel void main0(main0_in in [[stage_in]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], threadgroup main0_in* gl_in [[threadgroup(0)]])
{
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 4];
    device main0_patchOut& patchOut = spvPatchOut[gl_PrimitiveID];
    if (gl_InvocationID < spvIndirectParams[0])
        gl_in[gl_InvocationID] = in;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (gl_InvocationID >= 4)
        return;
    gl_out[gl_InvocationID].vVertex = gl_in[gl_InvocationID].vInput + gl_in[gl_InvocationID ^ 1].vInput;
    threadgroup_barrier(mem_flags::mem_device | mem_flags::mem_threadgroup);
    if (gl_InvocationID == 0)
    {
        patchOut.vPatch[0] = float3(10.0);
        patchOut.vPatch[1] = float3(20.0);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0] = half(1.0);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1] = half(2.0);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2] = half(3.0);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[3] = half(4.0);
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor[0] = half(1.0);
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor[1] = half(2.0);
    }
}

