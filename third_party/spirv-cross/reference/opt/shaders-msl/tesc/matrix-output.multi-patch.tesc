#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float in_te_attr;
    float4x3 in_te_data0;
    float4x3 in_te_data1;
};

struct main0_in
{
    float3 in_tc_attr;
    ushort2 m_104;
};

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], device main0_in* spvIn [[buffer(22)]])
{
    device main0_out* gl_out = &spvOut[gl_GlobalInvocationID.x - gl_GlobalInvocationID.x % 3];
    device main0_in* gl_in = &spvIn[min(gl_GlobalInvocationID.x / 3, spvIndirectParams[1] - 1) * spvIndirectParams[0]];
    uint gl_InvocationID = gl_GlobalInvocationID.x % 3;
    uint gl_PrimitiveID = min(gl_GlobalInvocationID.x / 3, spvIndirectParams[1]);
    float _15 = float(gl_InvocationID);
    float3 _18 = float3(_15, 0.0, 0.0);
    float3 _19 = float3(0.0, _15, 0.0);
    float3 _20 = float3(0.0, 0.0, _15);
    gl_out[gl_InvocationID].in_te_data0 = float4x3(_18, _19, _20, float3(0.0));
    threadgroup_barrier(mem_flags::mem_device | mem_flags::mem_threadgroup);
    int _42 = (gl_InvocationID + 1) % 3;
    gl_out[gl_InvocationID].in_te_data1 = float4x3(_18 + gl_out[_42].in_te_data0[0], _19 + gl_out[_42].in_te_data0[1], _20 + gl_out[_42].in_te_data0[2], gl_out[_42].in_te_data0[3]);
    gl_out[gl_InvocationID].in_te_attr = gl_in[gl_InvocationID].in_tc_attr.x;
    spvTessLevel[gl_PrimitiveID].insideTessellationFactor[0] = half(1.0);
    spvTessLevel[gl_PrimitiveID].insideTessellationFactor[1] = half(1.0);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0] = half(1.0);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1] = half(1.0);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2] = half(1.0);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[3] = half(1.0);
}

