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
    float3 vNormal [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 aVertex [[attribute(0)]];
    float3 aNormal [[attribute(1)]];
};

void set_output(device float4& gl_Position, constant UBO& v_18, thread float4& aVertex, device float3& vNormal, thread float3& aNormal)
{
    gl_Position = v_18.uMVP * aVertex;
    vNormal = aNormal;
}

vertex void main0(main0_in in [[stage_in]], constant UBO& v_18 [[buffer(0)]], uint gl_VertexIndex [[vertex_id]], uint gl_BaseVertex [[base_vertex]], uint gl_InstanceIndex [[instance_id]], uint gl_BaseInstance [[base_instance]], device main0_out* spvOut [[buffer(28)]], device uint* spvIndirectParams [[buffer(29)]])
{
    device main0_out& out = spvOut[(gl_InstanceIndex - gl_BaseInstance) * spvIndirectParams[0] + gl_VertexIndex - gl_BaseVertex];
    set_output(out.gl_Position, v_18, in.aVertex, out.vNormal, in.aNormal);
}

