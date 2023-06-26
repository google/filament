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

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]])
{
    device main0_out* gl_out = &spvOut[gl_GlobalInvocationID.x - gl_GlobalInvocationID.x % 4];
    device main0_patchOut& patchOut = spvPatchOut[gl_GlobalInvocationID.x / 4];
    threadgroup float4 spvStoragev1[8][2];
    threadgroup float4 (&v1)[2] = spvStoragev1[(gl_GlobalInvocationID.x / 4) % 8];
    uint gl_InvocationID = gl_GlobalInvocationID.x % 4;
    uint gl_PrimitiveID = min(gl_GlobalInvocationID.x / 4, spvIndirectParams[1] - 1);
    gl_out[gl_InvocationID].v0 = float4(1.0);
    gl_out[gl_InvocationID].v0.z = 3.0;
    if (gl_InvocationID == 0)
    {
        v1[0] = float4(2.0);
        ((threadgroup float*)&v1[0])[0u] = 3.0;
        v1[1] = float4(2.0);
        ((threadgroup float*)&v1[1])[0u] = 5.0;
    }
    patchOut.v3 = float4(5.0);
    gl_out[gl_InvocationID].gl_Position = float4(10.0);
    gl_out[gl_InvocationID].gl_Position.z = 20.0;
    gl_out[gl_InvocationID].gl_PointSize = 40.0;
}

