#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct PushMe
{
    float4x4 MVP;
    float2x2 Rot;
    float Arr[4];
};

struct main0_out
{
    float2 vRot [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float2 Rot [[attribute(0)]];
    float4 Pos [[attribute(1)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant PushMe& registers [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = registers.MVP * in.Pos;
    out.vRot = (registers.Rot * in.Rot) + float2(registers.Arr[2]);
    return out;
}

