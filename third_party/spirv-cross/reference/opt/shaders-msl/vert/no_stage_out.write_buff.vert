#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct _RESERVED_IDENTIFIER_FIXUP_33_35
{
    uint4 _RESERVED_IDENTIFIER_FIXUP_m0[1024];
};

struct _RESERVED_IDENTIFIER_FIXUP_38_40
{
    uint4 _RESERVED_IDENTIFIER_FIXUP_m0[1024];
};

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 _RESERVED_IDENTIFIER_FIXUP_14 [[attribute(0)]];
};

vertex void main0(main0_in in [[stage_in]], device _RESERVED_IDENTIFIER_FIXUP_33_35& _RESERVED_IDENTIFIER_FIXUP_35 [[buffer(0)]], constant _RESERVED_IDENTIFIER_FIXUP_38_40& _RESERVED_IDENTIFIER_FIXUP_40 [[buffer(1)]])
{
    main0_out out = {};
    out.gl_Position = in._RESERVED_IDENTIFIER_FIXUP_14;
    for (int _52 = 0; _52 < 1024; )
    {
        _RESERVED_IDENTIFIER_FIXUP_35._RESERVED_IDENTIFIER_FIXUP_m0[_52] = _RESERVED_IDENTIFIER_FIXUP_40._RESERVED_IDENTIFIER_FIXUP_m0[_52];
        _52++;
        continue;
    }
}

