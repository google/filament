#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct buf
{
    float4x4 MVP;
    float4 position[36];
    float4 attr[36];
};

struct main0_out
{
    float4 texcoord [[user(locn0)]];
    float3 frag_pos [[user(locn1)]];
    float4 gl_Position [[position]];
};

vertex main0_out main0(constant buf& ubuf [[buffer(0)]], uint gl_VertexIndex [[vertex_id]])
{
    main0_out out = {};
    out.texcoord = ubuf.attr[int(gl_VertexIndex)];
    out.gl_Position = ubuf.MVP * ubuf.position[int(gl_VertexIndex)];
    out.frag_pos = out.gl_Position.xyz;
    return out;
}

