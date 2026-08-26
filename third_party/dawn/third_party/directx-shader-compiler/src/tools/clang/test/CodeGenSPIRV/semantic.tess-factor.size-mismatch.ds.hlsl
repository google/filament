// RUN: %dxc -T ds_6_0 -E BezierEvalDS -fcgl  %s -spirv | FileCheck %s

// Test handling of built-in size mismatch (reading in from the built-ins):
// The HLSL SV_TessFactor is a float3, but the SPIR-V equivalent is a float4.
// The HLSL SV_InsideTessFactor is a scalar float, but the SPIR-V equivalent is a float2.

// CHECK: OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
// CHECK: OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner

// CHECK: %HS_CONSTANT_DATA_OUTPUT = OpTypeStruct %_arr_float_uint_3 %float

// CHECK: %gl_TessLevelOuter = OpVariable %_ptr_Input__arr_float_uint_4 Input
// CHECK: %gl_TessLevelInner = OpVariable %_ptr_Input__arr_float_uint_2 Input

// CHECK:         [[gl_TessLevelOuter:%[0-9]+]] = OpLoad %_arr_float_uint_4 %gl_TessLevelOuter
// CHECK-NEXT:   [[gl_TessLevelOuter0:%[0-9]+]] = OpCompositeExtract %float [[gl_TessLevelOuter]] 0
// CHECK-NEXT:   [[gl_TessLevelOuter1:%[0-9]+]] = OpCompositeExtract %float [[gl_TessLevelOuter]] 1
// CHECK-NEXT:   [[gl_TessLevelOuter2:%[0-9]+]] = OpCompositeExtract %float [[gl_TessLevelOuter]] 2
// CHECK-NEXT:   [[tessLevelOuterArr3:%[0-9]+]] = OpCompositeConstruct %_arr_float_uint_3 [[gl_TessLevelOuter0]] [[gl_TessLevelOuter1]] [[gl_TessLevelOuter2]]
// CHECK-NEXT:    [[gl_TessLevelInner:%[0-9]+]] = OpLoad %_arr_float_uint_2 %gl_TessLevelInner
// CHECK-NEXT: [[tessLevelOuterScalar:%[0-9]+]] = OpCompositeExtract %float [[gl_TessLevelInner]] 0
// CHECK-NEXT:                      {{%[0-9]+}} = OpCompositeConstruct %HS_CONSTANT_DATA_OUTPUT [[tessLevelOuterArr3]] [[tessLevelOuterScalar]]


struct HS_CONSTANT_DATA_OUTPUT
{
  float Edges[3]    : SV_TessFactor;
  float Inside      : SV_InsideTessFactor;
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

[domain("tri")]
DS_OUTPUT BezierEvalDS( HS_CONSTANT_DATA_OUTPUT input,
                        float2 UV : SV_DomainLocation,
                        const OutputPatch<BEZIER_CONTROL_POINT, 3> bezpatch )
{
  DS_OUTPUT Output;
  return Output;
}
