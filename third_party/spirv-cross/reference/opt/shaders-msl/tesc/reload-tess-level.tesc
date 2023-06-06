#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position;
};

struct main0_in
{
    float4 gl_Position [[attribute(0)]];
};

kernel void main0(main0_in in [[stage_in]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], threadgroup main0_in* gl_in [[threadgroup(0)]])
{
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 4];
    if (gl_InvocationID < spvIndirectParams[0])
        gl_in[gl_InvocationID] = in;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (gl_InvocationID >= 4)
        return;
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

