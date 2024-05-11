#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct TestStruct
{
    packed_float3 position;
    float radius;
};

struct CB0
{
    TestStruct CB0[16];
};

struct main0_out
{
    float4 _entryPointOutput [[color(0)]];
};

fragment main0_out main0(constant CB0& _RESERVED_IDENTIFIER_FIXUP_24 [[buffer(0)]])
{
    main0_out out = {};
    out._entryPointOutput = float4(_RESERVED_IDENTIFIER_FIXUP_24.CB0[1].position[0], _RESERVED_IDENTIFIER_FIXUP_24.CB0[1].position[1], _RESERVED_IDENTIFIER_FIXUP_24.CB0[1].position[2], _RESERVED_IDENTIFIER_FIXUP_24.CB0[1].radius);
    return out;
}

