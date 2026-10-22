#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
    uint gl_Layer [[render_target_array_index]];
};

vertex main0_out main0(uint gl_InstanceIndex [[instance_id]], uint gl_BaseInstance [[base_instance]])
{
    main0_out out = {};
    const uint gl_ViewIndex = 0;
    out.gl_Position = float4(float(int(gl_ViewIndex)));
    return out;
}

