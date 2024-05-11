#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct P
{
    float a;
    float b;
};

struct C
{
    float a;
    float b;
};

struct main0_out
{
    float c_b;
    float4 gl_Position;
};

struct main0_patchOut
{
    float m_11_a;
    float m_11_b;
};

kernel void main0(uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]])
{
    threadgroup C c[4];
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 4];
    device main0_patchOut& patchOut = spvPatchOut[gl_PrimitiveID];
    patchOut.m_11_a = 1.0;
    patchOut.m_11_b = 2.0;
    c[gl_InvocationID].a = 3.0;
    gl_out[gl_InvocationID].c_b = 4.0;
    gl_out[gl_InvocationID].gl_Position = float4(1.0);
}

