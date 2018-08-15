#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct _10
{
    uint4 _m0[1024];
};

struct main0_in
{
    uint4 m_19 [[attribute(0)]];
};

vertex void main0(main0_in in [[stage_in]], device _10& _12 [[buffer(0)]], uint gl_VertexIndex [[vertex_id]])
{
    _12._m0[gl_VertexIndex] = in.m_19;
}

