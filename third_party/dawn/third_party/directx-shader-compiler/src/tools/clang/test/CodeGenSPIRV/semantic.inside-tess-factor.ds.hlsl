// RUN: %dxc -T ds_6_0 -E BezierEvalDS -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint TessellationEvaluation %BezierEvalDS "BezierEvalDS"
// CHECK-SAME: %gl_TessLevelInner

// CHECK: OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
// CHECK: OpDecorate %gl_TessLevelInner Patch

// CHECK: %gl_TessLevelInner = OpVariable %_ptr_Input__arr_float_uint_2 Input


struct HS_CONSTANT_DATA_OUTPUT
{
  float Edges[4]        : SV_TessFactor;
  float Inside[2]       : SV_InsideTessFactor;
};

// Output control point (output of hull shader)
struct BEZIER_CONTROL_POINT
{
  float3 vPosition	: BEZIERPOS;
};

// The domain shader outputs
struct DS_OUTPUT
{
  float4 vPosition  : SV_POSITION;
};

[domain("quad")]
DS_OUTPUT BezierEvalDS( HS_CONSTANT_DATA_OUTPUT input,
                        float2 UV : SV_DomainLocation,
                        const OutputPatch<BEZIER_CONTROL_POINT, 16> bezpatch )
{
  DS_OUTPUT Output;
  return Output;
}
