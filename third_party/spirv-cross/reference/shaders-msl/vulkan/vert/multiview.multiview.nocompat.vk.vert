#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct MVPs
{
    float4x4 MVP[2];
};

struct main0_out
{
    float4 gl_Position [[position]];
    uint gl_Layer [[render_target_array_index]];
};

struct main0_in
{
    float4 Position [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant uint* spvViewMask [[buffer(24)]], constant MVPs& _19 [[buffer(0)]], uint gl_InstanceIndex [[instance_id]])
{
    main0_out out = {};
    uint gl_ViewIndex = spvViewMask[0] + gl_InstanceIndex % spvViewMask[1];
    gl_InstanceIndex /= spvViewMask[1];
    out.gl_Position = _19.MVP[int(gl_ViewIndex)] * in.Position;
    out.gl_Layer = gl_ViewIndex - spvViewMask[0];
    return out;
}

