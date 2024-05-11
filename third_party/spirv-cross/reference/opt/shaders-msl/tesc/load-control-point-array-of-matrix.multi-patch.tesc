#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4x4 vOutputs;
};

struct main0_in
{
    float4x4 vInputs;
};

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], device main0_in* spvIn [[buffer(22)]])
{
    device main0_out* gl_out = &spvOut[gl_GlobalInvocationID.x - gl_GlobalInvocationID.x % 4];
    device main0_in* gl_in = &spvIn[min(gl_GlobalInvocationID.x / 4, spvIndirectParams[1] - 1) * spvIndirectParams[0]];
    uint gl_InvocationID = gl_GlobalInvocationID.x % 4;
    uint gl_PrimitiveID = min(gl_GlobalInvocationID.x / 4, spvIndirectParams[1] - 1);
    gl_out[gl_InvocationID].vOutputs = gl_in[gl_InvocationID].vInputs;
}

