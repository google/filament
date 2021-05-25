#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 v0;
    float4 gl_Position;
    float gl_PointSize;
};

struct main0_patchOut
{
    float4 v3;
};

static inline __attribute__((always_inline))
void write_in_func(device main0_out* thread & gl_out, thread uint& gl_InvocationID, threadgroup float4 (&v1)[2], device float4& v3)
{
    gl_out[gl_InvocationID].v0 = float4(1.0);
    gl_out[gl_InvocationID].v0.z = 3.0;
    if (gl_InvocationID == 0)
    {
        v1[0] = float4(2.0);
        ((threadgroup float*)&v1[0])[0u] = 3.0;
        v1[1] = float4(2.0);
        ((threadgroup float*)&v1[1])[0u] = 5.0;
    }
    v3 = float4(5.0);
    gl_out[gl_InvocationID].gl_Position = float4(10.0);
    gl_out[gl_InvocationID].gl_Position.z = 20.0;
    gl_out[gl_InvocationID].gl_PointSize = 40.0;
}

kernel void main0(uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]])
{
    threadgroup float4 v1[2];
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 4];
    device main0_patchOut& patchOut = spvPatchOut[gl_PrimitiveID];
    write_in_func(gl_out, gl_InvocationID, v1, patchOut.v3);
}

