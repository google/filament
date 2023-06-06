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
    spvUnsafeArray<float4, 2> Foo;
    float4 gl_Position;
};

struct main0_patchOut
{
    spvUnsafeArray<float4, 2> pFoo;
};

struct main0_in
{
    float4 iFoo_0 [[attribute(0)]];
    float4 iFoo_1 [[attribute(1)]];
    float4 ipFoo [[attribute(2)]];
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
    gl_out[gl_InvocationID].gl_Position = float4(1.0);
    spvUnsafeArray<float4, 2> _38 = spvUnsafeArray<float4, 2>({ gl_in[gl_InvocationID].iFoo_0, gl_in[gl_InvocationID].iFoo_1 });
    gl_out[gl_InvocationID].Foo = _38;
    if (gl_InvocationID == 0)
    {
        spvUnsafeArray<float4, 2> _56 = spvUnsafeArray<float4, 2>({ gl_in[0].ipFoo, gl_in[1].ipFoo });
        patchOut.pFoo = _56;
    }
}

