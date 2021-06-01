#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position;
};

struct main0_in
{
    uint3 m_82;
    ushort2 m_86;
    float4 gl_Position;
};

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], device main0_in* spvIn [[buffer(22)]])
{
    device main0_out* gl_out = &spvOut[gl_GlobalInvocationID.x - gl_GlobalInvocationID.x % 4];
    device main0_in* gl_in = &spvIn[min(gl_GlobalInvocationID.x / 4, spvIndirectParams[1] - 1) * spvIndirectParams[0]];
    uint gl_InvocationID = gl_GlobalInvocationID.x % 4;
    uint gl_PrimitiveID = min(gl_GlobalInvocationID.x / 4, spvIndirectParams[1] - 1);
    if (gl_InvocationID == 0)
    {
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0] = half(2.0);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1] = half(3.0);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2] = half(4.0);
        spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[3] = half(5.0);
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor[0] = half(mix(float(spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0]), float(spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[3]), 0.5));
        spvTessLevel[gl_PrimitiveID].insideTessellationFactor[1] = half(mix(float(spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2]), float(spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1]), 0.5));
    }
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}

