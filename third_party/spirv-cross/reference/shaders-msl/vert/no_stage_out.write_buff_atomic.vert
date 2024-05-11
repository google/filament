#pragma clang diagnostic ignored "-Wunused-variable"

#include <metal_stdlib>
#include <simd/simd.h>
#include <metal_atomic>

using namespace metal;

struct _RESERVED_IDENTIFIER_FIXUP_19_21
{
    uint _RESERVED_IDENTIFIER_FIXUP_m0;
};

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 _RESERVED_IDENTIFIER_FIXUP_14 [[attribute(0)]];
};

vertex void main0(main0_in in [[stage_in]], volatile device _RESERVED_IDENTIFIER_FIXUP_19_21& _RESERVED_IDENTIFIER_FIXUP_21 [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = in._RESERVED_IDENTIFIER_FIXUP_14;
    uint _29 = atomic_fetch_add_explicit((volatile device atomic_uint*)&_RESERVED_IDENTIFIER_FIXUP_21._RESERVED_IDENTIFIER_FIXUP_m0, 1u, memory_order_relaxed);
    uint _RESERVED_IDENTIFIER_FIXUP_26 = _29;
}

