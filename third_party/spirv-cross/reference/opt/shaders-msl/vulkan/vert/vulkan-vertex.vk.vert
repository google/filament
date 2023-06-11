#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

vertex main0_out main0(uint gl_VertexIndex [[vertex_id]], uint gl_InstanceIndex [[instance_id]])
{
    main0_out out = {};
    out.gl_Position = float4(1.0, 2.0, 3.0, 4.0) * float(int(gl_VertexIndex) + int(gl_InstanceIndex));
    return out;
}

