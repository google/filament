#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct V
{
    float4 a;
    float4 b;
    float4 c;
    float4 d;
};

struct main0_out
{
    float4 V_b;
    float4 V_c;
    float4 V_d;
    float4 gl_Position;
};

kernel void main0(uint3 gl_GlobalInvocationID [[thread_position_in_grid]], uint3 spvStageInputSize [[grid_size]], device main0_out* spvOut [[buffer(28)]])
{
    V _22 = {};
    device main0_out& out = spvOut[gl_GlobalInvocationID.y * spvStageInputSize.x + gl_GlobalInvocationID.x];
    if (any(gl_GlobalInvocationID >= spvStageInputSize))
        return;
    out.gl_Position = float4(1.0);
    _22.a = float4(2.0);
    _22.b = float4(3.0);
    out.V_b = _22.b;
    out.V_c = _22.c;
    out.V_d = _22.d;
}

