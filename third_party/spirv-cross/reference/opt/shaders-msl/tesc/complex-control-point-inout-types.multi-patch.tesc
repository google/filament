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

struct Block_1
{
    spvUnsafeArray<float, 2> a;
    float b;
    float2x2 m;
    Meep meep;
    spvUnsafeArray<Meep, 2> meeps;
};

struct main0_out
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
    float4 gl_Position;
};

struct main0_in
{
    spvUnsafeArray<float, 2> in_a;
    float in_b;
    float2x2 in_m;
    Meep in_meep;
    spvUnsafeArray<Meep, 2> in_meeps;
    spvUnsafeArray<float, 2> Block_a;
    float Block_b;
    float2x2 Block_m;
    Meep Block_meep;
    spvUnsafeArray<Meep, 2> Block_meeps;
};

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], device main0_in* spvIn [[buffer(22)]])
{
    device main0_out* gl_out = &spvOut[gl_GlobalInvocationID.x - gl_GlobalInvocationID.x % 4];
    device main0_in* gl_in = &spvIn[min(gl_GlobalInvocationID.x / 4, spvIndirectParams[1] - 1) * spvIndirectParams[0]];
    uint gl_InvocationID = gl_GlobalInvocationID.x % 4;
    uint gl_PrimitiveID = min(gl_GlobalInvocationID.x / 4, spvIndirectParams[1] - 1);
    gl_out[gl_InvocationID].gl_Position = float4(1.0);
    gl_out[gl_InvocationID].a[0] = gl_in[gl_InvocationID].in_a[0];
    gl_out[gl_InvocationID].a[1] = gl_in[gl_InvocationID].in_a[1];
    gl_out[gl_InvocationID].b = gl_in[gl_InvocationID].in_b;
    gl_out[gl_InvocationID].m = gl_in[gl_InvocationID].in_m;
    gl_out[gl_InvocationID].meep.a = gl_in[gl_InvocationID].in_meep.a;
    gl_out[gl_InvocationID].meep.b = gl_in[gl_InvocationID].in_meep.b;
    gl_out[gl_InvocationID].meeps[0].a = gl_in[gl_InvocationID].in_meeps[0].a;
    gl_out[gl_InvocationID].meeps[0].b = gl_in[gl_InvocationID].in_meeps[0].b;
    gl_out[gl_InvocationID].meeps[1].a = gl_in[gl_InvocationID].in_meeps[1].a;
    gl_out[gl_InvocationID].meeps[1].b = gl_in[gl_InvocationID].in_meeps[1].b;
    gl_out[gl_InvocationID].Block_a[0] = gl_in[gl_InvocationID].Block_a[0];
    gl_out[gl_InvocationID].Block_a[1] = gl_in[gl_InvocationID].Block_a[1];
    gl_out[gl_InvocationID].Block_b = gl_in[gl_InvocationID].Block_b;
    gl_out[gl_InvocationID].Block_m = gl_in[gl_InvocationID].Block_m;
    gl_out[gl_InvocationID].Block_meep.a = gl_in[gl_InvocationID].Block_meep.a;
    gl_out[gl_InvocationID].Block_meep.b = gl_in[gl_InvocationID].Block_meep.b;
    gl_out[gl_InvocationID].Block_meeps[0].a = gl_in[gl_InvocationID].Block_meeps[0].a;
    gl_out[gl_InvocationID].Block_meeps[0].b = gl_in[gl_InvocationID].Block_meeps[0].b;
    gl_out[gl_InvocationID].Block_meeps[1].a = gl_in[gl_InvocationID].Block_meeps[1].a;
    gl_out[gl_InvocationID].Block_meeps[1].b = gl_in[gl_InvocationID].Block_meeps[1].b;
}

