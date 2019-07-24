#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Block
{
    float3x4 var[3][4];
};

struct main0_out
{
    float v_vtxResult [[user(locn0)]];
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 a_position [[attribute(0)]];
};

vertex main0_out main0(main0_in in [[stage_in]], constant Block& _104 [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = in.a_position;
    out.v_vtxResult = ((float(abs(float3(_104.var[0][0][0][0], _104.var[0][0][1][0], _104.var[0][0][2][0])[0] - 2.0) < 0.0500000007450580596923828125) * float(abs(float3(_104.var[0][0][0][0], _104.var[0][0][1][0], _104.var[0][0][2][0])[1] - 6.0) < 0.0500000007450580596923828125)) * float(abs(float3(_104.var[0][0][0][0], _104.var[0][0][1][0], _104.var[0][0][2][0])[2] - (-6.0)) < 0.0500000007450580596923828125)) * ((float(abs(float3(_104.var[0][0][0][1], _104.var[0][0][1][1], _104.var[0][0][2][1])[0]) < 0.0500000007450580596923828125) * float(abs(float3(_104.var[0][0][0][1], _104.var[0][0][1][1], _104.var[0][0][2][1])[1] - 5.0) < 0.0500000007450580596923828125)) * float(abs(float3(_104.var[0][0][0][1], _104.var[0][0][1][1], _104.var[0][0][2][1])[2] - 5.0) < 0.0500000007450580596923828125));
    return out;
}

