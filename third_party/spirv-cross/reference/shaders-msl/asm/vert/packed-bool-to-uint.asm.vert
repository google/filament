#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Struct
{
    uint flags[1];
};

struct defaultUniformsVS
{
    Struct flags;
    float4 uquad[4];
    float4x4 umatrix;
};

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 a_position [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant defaultUniformsVS& _11 [[buffer(0)]], uint gl_VertexIndex [[vertex_id]])
{
    main0_out out = {};
    out.gl_Position = _11.umatrix * float4(_11.uquad[int(gl_VertexIndex)].x, _11.uquad[int(gl_VertexIndex)].y, in.a_position.z, in.a_position.w);
    if (_11.flags.flags[0] != 0u)
    {
        out.gl_Position.z = 0.0;
    }
    return out;
}

