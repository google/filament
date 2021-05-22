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

struct Meep
{
    float a;
    float b;
};

struct Block
{
    spvUnsafeArray<float, 2> a;
    float b;
    float2x2 m;
    Meep meep;
    spvUnsafeArray<Meep, 2> meeps;
};

struct main0_out
{
    float4 gl_Position;
};

struct main0_patchOut
{
    spvUnsafeArray<float, 2> a;
    float b;
    float2x2 m;
    Meep meep;
    spvUnsafeArray<Meep, 2> meeps;
    spvUnsafeArray<float, 2> Block_a;
    float Block_b;
    float2x2 Block_m;
    Meep Block_meep;
    spvUnsafeArray<Meep, 2> Block_meeps;
};

kernel void main0(uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]])
{
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 4];
    device main0_patchOut& patchOut = spvPatchOut[gl_PrimitiveID];
    gl_out[gl_InvocationID].gl_Position = float4(1.0);
    patchOut.a[0] = 1.0;
    patchOut.a[1] = 2.0;
    patchOut.b = 3.0;
    patchOut.m = float2x2(float2(2.0, 0.0), float2(0.0, 2.0));
    patchOut.meep.a = 4.0;
    patchOut.meep.b = 5.0;
    patchOut.meeps[0].a = 6.0;
    patchOut.meeps[0].b = 7.0;
    patchOut.meeps[1].a = 8.0;
    patchOut.meeps[1].b = 9.0;
    patchOut.Block_a[0] = 1.0;
    patchOut.Block_a[1] = 2.0;
    patchOut.Block_b = 3.0;
    patchOut.Block_m = float2x2(float2(4.0, 0.0), float2(0.0, 4.0));
    patchOut.Block_meep.a = 4.0;
    patchOut.Block_meep.b = 5.0;
    patchOut.Block_meeps[0].a = 6.0;
    patchOut.Block_meeps[0].b = 7.0;
    patchOut.Block_meeps[1].a = 8.0;
    patchOut.Block_meeps[1].b = 9.0;
}

