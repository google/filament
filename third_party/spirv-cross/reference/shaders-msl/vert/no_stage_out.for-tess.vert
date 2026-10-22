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

kernel void main0(main0_in in [[stage_in]], device _RESERVED_IDENTIFIER_FIXUP_10_12& _RESERVED_IDENTIFIER_FIXUP_12 [[buffer(0)]], uint3 gl_GlobalInvocationID [[thread_position_in_grid]], uint3 spvStageInputSize [[grid_size]], uint3 spvDispatchBase [[grid_origin]])
{
    if (any(gl_GlobalInvocationID >= spvStageInputSize))
        return;
    uint gl_VertexIndex = gl_GlobalInvocationID.x + spvDispatchBase.x;
    _RESERVED_IDENTIFIER_FIXUP_12._RESERVED_IDENTIFIER_FIXUP_m0[int(gl_VertexIndex)] = in._RESERVED_IDENTIFIER_FIXUP_19;
}

