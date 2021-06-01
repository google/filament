#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_patchOut
{
    float3 vFoo;
};

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], constant uint* spvIndirectParams [[buffer(29)]], device main0_patchOut* spvPatchOut [[buffer(27)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]])
{
    device main0_patchOut& patchOut = spvPatchOut[gl_GlobalInvocationID.x / 1];
    uint gl_PrimitiveID = min(gl_GlobalInvocationID.x / 1, spvIndirectParams[1] - 1);
    spvTessLevel[gl_PrimitiveID].insideTessellationFactor[0] = half(8.8999996185302734375);
    spvTessLevel[gl_PrimitiveID].insideTessellationFactor[1] = half(6.900000095367431640625);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0] = half(8.8999996185302734375);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1] = half(6.900000095367431640625);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2] = half(3.900000095367431640625);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[3] = half(4.900000095367431640625);
    patchOut.vFoo = float3(1.0);
}

