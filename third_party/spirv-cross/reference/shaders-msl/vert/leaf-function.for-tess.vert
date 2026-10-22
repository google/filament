#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO
{
    float4x4 uMVP;
};

struct main0_out
{
    float3 vNormal;
    float4 gl_Position;
};

struct main0_in
{
    float4 aVertex [[attribute(0)]];
    float3 aNormal [[attribute(1)]];
};

static inline __attribute__((always_inline))
void set_output(device float4& gl_Position, constant UBO& _18, thread float4& aVertex, device float3& vNormal, thread float3& aNormal)
{
    gl_Position = _18.uMVP * aVertex;
    vNormal = aNormal;
}

kernel void main0(main0_in in [[stage_in]], constant UBO& _18 [[buffer(0)]], uint3 gl_GlobalInvocationID [[thread_position_in_grid]], uint3 spvStageInputSize [[grid_size]], device main0_out* spvOut [[buffer(28)]])
{
    device main0_out& out = spvOut[gl_GlobalInvocationID.y * spvStageInputSize.x + gl_GlobalInvocationID.x];
    if (any(gl_GlobalInvocationID >= spvStageInputSize))
        return;
    set_output(out.gl_Position, _18, in.aVertex, out.vNormal, in.aNormal);
}

