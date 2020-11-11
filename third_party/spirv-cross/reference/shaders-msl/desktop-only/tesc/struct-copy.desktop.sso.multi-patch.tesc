#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Boo
{
    float3 a;
    uint3 b;
};

struct main0_out
{
    Boo vVertex;
};

struct main0_in
{
    float3 Boo_a;
    uint3 Boo_b;
};

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], device main0_out* spvOut [[buffer(28)]], constant uint* spvIndirectParams [[buffer(29)]], device MTLQuadTessellationFactorsHalf* spvTessLevel [[buffer(26)]], device main0_in* spvIn [[buffer(22)]])
{
    device main0_out* gl_out = &spvOut[gl_GlobalInvocationID.x - gl_GlobalInvocationID.x % 4];
    device main0_in* gl_in = &spvIn[min(gl_GlobalInvocationID.x / 4, spvIndirectParams[1] - 1) * spvIndirectParams[0]];
    uint gl_InvocationID = gl_GlobalInvocationID.x % 4;
    uint gl_PrimitiveID = min(gl_GlobalInvocationID.x / 4, spvIndirectParams[1]);
    Boo _26 = Boo{ gl_in[gl_InvocationID].Boo_a, gl_in[gl_InvocationID].Boo_b };
    gl_out[gl_InvocationID].vVertex = _26;
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[0] = half(1.0);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[1] = half(2.0);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[2] = half(3.0);
    spvTessLevel[gl_PrimitiveID].edgeTessellationFactor[3] = half(4.0);
    spvTessLevel[gl_PrimitiveID].insideTessellationFactor[0] = half(1.0);
    spvTessLevel[gl_PrimitiveID].insideTessellationFactor[1] = half(2.0);
}

