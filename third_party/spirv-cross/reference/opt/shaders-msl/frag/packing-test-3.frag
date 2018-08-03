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

fragment main0_out main0(constant CB0& _26 [[buffer(0)]], float4 gl_FragCoord [[position]])
{
    main0_out out = {};
    out._entryPointOutput = float4(_26.CB0[1].position[0], _26.CB0[1].position[1], _26.CB0[1].position[2], _26.CB0[1].radius);
    return out;
}

