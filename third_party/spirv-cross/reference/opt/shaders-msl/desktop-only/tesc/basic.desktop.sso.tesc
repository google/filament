#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position;
};

struct main0_patchOut
{
    float3 vFoo;
};

struct main0_in
{
    float4 gl_Position [[attribute(0)]];
};

kernel void main0(main0_in in [[stage_in]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], threadgroup main0_in* gl_in [[threadgroup(0)]])
{
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 1];
    device main0_patchOut& patchOut = spvPatchOut[gl_PrimitiveID];
    if (gl_InvocationID < spvIndirectParams[0])
        gl_in[gl_InvocationID] = in;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (gl_InvocationID >= 1)
        return;
    spvTessLevel[gl_PrimitiveID].insideTessellationFactor[0] = half(8.8999996185302734375);
    spvTessLevel[gl_PrimitiveID].insideTessellationFactor[1] = half(6.900000095367431640625);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0] = half(8.8999996185302734375);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1] = half(6.900000095367431640625);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2] = half(3.900000095367431640625);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[3] = half(4.900000095367431640625);
    patchOut.vFoo = float3(1.0);
    gl_out[gl_InvocationID].gl_Position = gl_in[0].gl_Position + gl_in[1].gl_Position;
}

