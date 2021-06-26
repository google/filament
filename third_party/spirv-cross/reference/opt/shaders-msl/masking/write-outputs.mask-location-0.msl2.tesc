#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position;
    float gl_PointSize;
};

struct main0_patchOut
{
    float4 v1;
};

kernel void main0(uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]])
{
    threadgroup float4 v0[4];
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 4];
    device main0_patchOut& patchOut = spvPatchOut[gl_PrimitiveID];
    v0[gl_InvocationID] = float4(1.0);
    v0[gl_InvocationID].x = 2.0;
    if (gl_InvocationID == 0)
    {
        patchOut.v1 = float4(2.0);
        ((device float*)&patchOut.v1)[3u] = 4.0;
    }
    gl_out[gl_InvocationID].gl_Position = float4(3.0);
    gl_out[gl_InvocationID].gl_PointSize = 4.0;
    gl_out[gl_InvocationID].gl_Position.z = 5.0;
    gl_out[gl_InvocationID].gl_PointSize = 4.0;
}

