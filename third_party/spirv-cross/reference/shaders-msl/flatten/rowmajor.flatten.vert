#pragma clang diagnostic ignored "-Wmissing-prototypes"

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct UBO
{
    float4x4 uMVPR;
    float4x4 uMVPC;
    float2x4 uMVP;
};

struct main0_out
{
    float4 gl_Position [[position]];
};

struct main0_in
{
    float4 aVertex [[attribute(0)]];
};

// Implementation of a conversion of matrix content from RowMajor to ColumnMajor organization.
float2x4 spvConvertFromRowMajor2x4(float2x4 m)
{
    return float2x4(float4(m[0][0], m[0][2], m[1][0], m[1][2]), float4(m[0][1], m[0][3], m[1][1], m[1][3]));
}

vertex main0_out main0(main0_in in [[stage_in]], constant UBO& _18 [[buffer(0)]])
{
    main0_out out = {};
    float2 v = in.aVertex * spvConvertFromRowMajor2x4(_18.uMVP);
    out.gl_Position = (_18.uMVPR * in.aVertex) + (in.aVertex * _18.uMVPC);
    return out;
}

