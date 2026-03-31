#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct BlockIO
{
    float v;
};

struct main0_out
{
    float outBlock_v;
};

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], uint3 spvStageInputSize [[grid_size]], device main0_out* spvOut [[buffer(28)]])
{
    BlockIO outBlock = {};
    device main0_out& out = spvOut[gl_GlobalInvocationID.y * spvStageInputSize.x + gl_GlobalInvocationID.x];
    if (any(gl_GlobalInvocationID >= spvStageInputSize))
        return;
    outBlock.v = 1.0;
    out.outBlock_v = outBlock.v;
}

