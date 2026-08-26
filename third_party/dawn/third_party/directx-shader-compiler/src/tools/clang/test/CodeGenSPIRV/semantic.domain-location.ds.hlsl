// RUN: %dxc -T ds_6_0 -E BezierEvalDS -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint TessellationEvaluation %BezierEvalDS "BezierEvalDS"
// CHECK-SAME: %gl_TessCoord

// CHECK: OpDecorate %gl_TessCoord BuiltIn TessCoord

// CHECK: %gl_TessCoord = OpVariable %_ptr_Input_v3float Input

// Note: Since SV_DomainLocation is a float2, an extra processing step must be
// performed in the wrapper function in order to extract the first 2 components
// of the TessCoord (which is a float3).

// CHECK: %BezierEvalDS = OpFunction %void None {{%[0-9]+}}
// CHECK: [[v3TessCoord:%[0-9]+]] = OpLoad %v3float %gl_TessCoord
// CHECK: [[v2TessCoord:%[0-9]+]] = OpVectorShuffle %v2float [[v3TessCoord]] [[v3TessCoord]] 0 1
// CHECK: OpStore %param_var_UV [[v2TessCoord]]
// CHECK: {{%[0-9]+}} = OpFunctionCall %DS_OUTPUT %src_BezierEvalDS %param_var_input %param_var_UV %param_var_bezpatch

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
  Output.vPosition = float4(UV, UV);
  return Output;
}
