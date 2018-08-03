#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct Block
{
    float2x3 var[3][4];
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

// Implementation of a conversion of matrix content from RowMajor to ColumnMajor organization.
float2x3 spvConvertFromRowMajor2x3(float2x3 m)
{
    return float2x3(float3(m[0][0], m[0][2], m[1][1]), float3(m[0][1], m[1][0], m[1][2]));
}

vertex main0_out main0(main0_in in [[stage_in]], constant Block& _104 [[buffer(0)]])
{
    main0_out out = {};
    out.gl_Position = in.a_position;
    out.v_vtxResult = ((float(abs(spvConvertFromRowMajor2x3(_104.var[0][0])[0].x - 2.0) < 0.0500000007450580596923828125) * float(abs(spvConvertFromRowMajor2x3(_104.var[0][0])[0].y - 6.0) < 0.0500000007450580596923828125)) * float(abs(spvConvertFromRowMajor2x3(_104.var[0][0])[0].z - (-6.0)) < 0.0500000007450580596923828125)) * ((float(abs(spvConvertFromRowMajor2x3(_104.var[0][0])[1].x) < 0.0500000007450580596923828125) * float(abs(spvConvertFromRowMajor2x3(_104.var[0][0])[1].y - 5.0) < 0.0500000007450580596923828125)) * float(abs(spvConvertFromRowMajor2x3(_104.var[0][0])[1].z - 5.0) < 0.0500000007450580596923828125));
    return out;
}

