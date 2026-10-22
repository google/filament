#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4x4 vOutputs;
};

struct main0_in
{
    float4 vInputs_0 [[attribute(0)]];
    float4 vInputs_1 [[attribute(1)]];
    float4 vInputs_2 [[attribute(2)]];
    float4 vInputs_3 [[attribute(3)]];
};

kernel void main0(main0_in in [[stage_in]], uint gl_InvocationID [[thread_index_in_threadgroup]], uint gl_PrimitiveID [[threadgroup_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], threadgroup main0_in* gl_in [[threadgroup(0)]])
{
    device main0_out* gl_out = &spvOut[gl_PrimitiveID * 4];
    if (gl_InvocationID < spvIndirectParams[0])
        gl_in[gl_InvocationID] = in;
    threadgroup_barrier(mem_flags::mem_threadgroup);
    if (gl_InvocationID >= 4)
        return;
    float4x4 _28 = float4x4(gl_in[gl_InvocationID].vInputs_0, gl_in[gl_InvocationID].vInputs_1, gl_in[gl_InvocationID].vInputs_2, gl_in[gl_InvocationID].vInputs_3);
    gl_out[gl_InvocationID].vOutputs = _28;
}

