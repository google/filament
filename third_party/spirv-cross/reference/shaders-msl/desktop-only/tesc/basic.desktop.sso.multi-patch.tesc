#pragma clang diagnostic ignored "-Wmissing-prototypes"

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
    uint3 m_78;
    ushort2 m_82;
    float4 gl_Position;
};

static inline __attribute__((always_inline))
void set_position(device main0_out* thread & gl_out, thread uint& gl_InvocationID, device main0_in* thread & gl_in)
{
    gl_out[gl_InvocationID].gl_Position = gl_in[0].gl_Position + gl_in[1].gl_Position;
}

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], device main0_in* spvIn [[buffer(22)]])
{
    device main0_out* gl_out = &spvOut[gl_GlobalInvocationID.x - gl_GlobalInvocationID.x % 1];
    device main0_patchOut& patchOut = spvPatchOut[gl_GlobalInvocationID.x / 1];
    device main0_in* gl_in = &spvIn[min(gl_GlobalInvocationID.x / 1, spvIndirectParams[1] - 1) * spvIndirectParams[0]];
    uint gl_InvocationID = gl_GlobalInvocationID.x % 1;
    uint gl_PrimitiveID = min(gl_GlobalInvocationID.x / 1, spvIndirectParams[1]);
    spvTessLevel[gl_PrimitiveID].insideTessellationFactor[0] = half(8.8999996185302734375);
    spvTessLevel[gl_PrimitiveID].insideTessellationFactor[1] = half(6.900000095367431640625);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0] = half(8.8999996185302734375);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1] = half(6.900000095367431640625);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2] = half(3.900000095367431640625);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[3] = half(4.900000095367431640625);
    patchOut.vFoo = float3(1.0);
    set_position(gl_out, gl_InvocationID, gl_in);
}

