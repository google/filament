#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position;
};

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], uint3 spvStageInputSize [[grid_size]], uint3 spvDispatchBase [[grid_origin]], device main0_out* spvOut [[buffer(28)]])
{
    device main0_out& out = spvOut[gl_GlobalInvocationID.y * spvStageInputSize.x + gl_GlobalInvocationID.x];
    if (any(gl_GlobalInvocationID >= spvStageInputSize))
        return;
    uint gl_BaseVertex = spvDispatchBase.x;
    uint gl_BaseInstance = spvDispatchBase.y;
    out.gl_Position = float4(float(int(gl_BaseVertex)), float(int(gl_BaseInstance)), 0.0, 1.0);
}

