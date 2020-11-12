#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct _RESERVED_IDENTIFIER_FIXUP_10_12
{
    uint4 _RESERVED_IDENTIFIER_FIXUP_m0[1024];
};

struct main0_in
{
    uint4 _RESERVED_IDENTIFIER_FIXUP_19 [[attribute(0)]];
};

vertex void main0(main0_in in [[stage_in]], device _RESERVED_IDENTIFIER_FIXUP_10_12& _RESERVED_IDENTIFIER_FIXUP_12 [[buffer(0)]], uint gl_VertexIndex [[vertex_id]])
{
    _RESERVED_IDENTIFIER_FIXUP_12._RESERVED_IDENTIFIER_FIXUP_m0[int(gl_VertexIndex)] = in._RESERVED_IDENTIFIER_FIXUP_19;
}

