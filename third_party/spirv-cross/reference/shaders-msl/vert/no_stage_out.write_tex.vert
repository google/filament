#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 _RESERVED_IDENTIFIER_FIXUP_14 [[attribute(0)]];
};

vertex void main0(main0_in in [[stage_in]], texture1d<uint, access::write> _RESERVED_IDENTIFIER_FIXUP_32 [[texture(0)]], texture1d<uint> _RESERVED_IDENTIFIER_FIXUP_35 [[texture(1)]])
{
    main0_out out = {};
    out.gl_Position = in._RESERVED_IDENTIFIER_FIXUP_14;
    for (int _RESERVED_IDENTIFIER_FIXUP_19 = 0; _RESERVED_IDENTIFIER_FIXUP_19 < 128; _RESERVED_IDENTIFIER_FIXUP_19++)
    {
        _RESERVED_IDENTIFIER_FIXUP_32.write(_RESERVED_IDENTIFIER_FIXUP_35.read(uint(_RESERVED_IDENTIFIER_FIXUP_19)), uint(_RESERVED_IDENTIFIER_FIXUP_19));
    }
}

