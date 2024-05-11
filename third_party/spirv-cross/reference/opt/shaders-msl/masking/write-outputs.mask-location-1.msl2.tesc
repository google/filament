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
};
kernel void main0(uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]])
{
    threadgroup float4 v1;
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 4];
    device main0_patchOut& patchOut = spvPatchOut[gl_PrimitiveID];
    gl_out[gl_InvocationID].v0 = float4(1.0);
    gl_out[gl_InvocationID].v0.x = 2.0;
    if (gl_InvocationID == 0)
    {
        v1 = float4(2.0);
        ((threadgroup float*)&v1)[3u] = 4.0;
    }
    gl_out[gl_InvocationID].gl_Position = float4(3.0);
    gl_out[gl_InvocationID].gl_PointSize = 4.0;
    gl_out[gl_InvocationID].gl_Position.z = 5.0;
    gl_out[gl_InvocationID].gl_PointSize = 4.0;
}

