// RUN: %dxc -T hs_6_0 -E SubDToBezierHS -fcgl  %s -spirv | FileCheck %s

// Test handling of built-in size mismatch (writing out to the built-ins):
// The HLSL SV_TessFactor is a float3, but the SPIR-V equivalent is a float4.
// The HLSL SV_InsideTessFactor is a scalar float, but the SPIR-V equivalent is a float2.


// CHECK: OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
// CHECK: OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
 
// CHECK: %HS_CONSTANT_DATA_OUTPUT = OpTypeStruct %_arr_float_uint_3 %float %_arr_v3float_uint_4 %_arr_v2float_uint_4 %_arr_v3float_uint_4 %_arr_v3float_uint_4 %v4float

// CHECK: %gl_TessLevelOuter = OpVariable %_ptr_Output__arr_float_uint_4 Output
// CHECK: %gl_TessLevelInner = OpVariable %_ptr_Output__arr_float_uint_2 Output

// CHECK:                      [[pcfRet:%[0-9]+]] = OpFunctionCall %HS_CONSTANT_DATA_OUTPUT %SubDToBezierConstantsHS %param_var_ip %param_var_PatchID

// CHECK-NEXT:       [[pcfRetTessFactor:%[0-9]+]] = OpCompositeExtract %_arr_float_uint_3 [[pcfRet]] 0
// CHECK-NEXT: [[gl_TessLevelOuter_loc0:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %uint_0
// CHECK-NEXT:      [[pcfRetTessFactor0:%[0-9]+]] = OpCompositeExtract %float [[pcfRetTessFactor]] 0
// CHECK-NEXT:                                   OpStore [[gl_TessLevelOuter_loc0]] [[pcfRetTessFactor0]]

// CHECK-NEXT: [[gl_TessLevelOuter_loc1:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %uint_1
// CHECK-NEXT:      [[pcfRetTessFactor1:%[0-9]+]] = OpCompositeExtract %float [[pcfRetTessFactor]] 1
// CHECK-NEXT:                                   OpStore [[gl_TessLevelOuter_loc1]] [[pcfRetTessFactor1]]

// CHECK-NEXT: [[gl_TessLevelOuter_loc2:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_TessLevelOuter %uint_2
// CHECK-NEXT:      [[pcfRetTessFactor2:%[0-9]+]] = OpCompositeExtract %float [[pcfRetTessFactor]] 2
// CHECK-NEXT:                                   OpStore [[gl_TessLevelOuter_loc2]] [[pcfRetTessFactor2]]


// CHECK-NEXT: [[pcfRetInsideTessFactor:%[0-9]+]] = OpCompositeExtract %float [[pcfRet]] 1
// CHECK-NEXT: [[gl_TessLevelInner_loc0:%[0-9]+]] = OpAccessChain %_ptr_Output_float %gl_TessLevelInner %uint_0
// CHECK-NEXT:                                   OpStore [[gl_TessLevelInner_loc0]] [[pcfRetInsideTessFactor]]

#define MAX_POINTS 3

// Input control point
struct VS_CONTROL_POINT_OUTPUT
{
  float3 vPosition : WORLDPOS;
  float2 vUV       : TEXCOORD0;
  float3 vTangent  : TANGENT;
};

// Output control point
struct BEZIER_CONTROL_POINT
{
  float3 vPosition	: BEZIERPOS;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
  float Edges[3]        : SV_TessFactor;
  float Inside          : SV_InsideTessFactor;

  float3 vTangent[4]    : TANGENT;
  float2 vUV[4]         : TEXCOORD;
  float3 vTanUCorner[4] : TANUCORNER;
  float3 vTanVCorner[4] : TANVCORNER;
  float4 vCWts          : TANWEIGHTS;
};

// Patch Constant Function
HS_CONSTANT_DATA_OUTPUT SubDToBezierConstantsHS(InputPatch<VS_CONTROL_POINT_OUTPUT, MAX_POINTS> ip, uint PatchID : SV_PrimitiveID) {
  HS_CONSTANT_DATA_OUTPUT Output;

  // Must initialize Edges and Inside; otherwise HLSL validation will fail.
  Output.Edges[0]  = 1.0;
  Output.Edges[1]  = 2.0;
  Output.Edges[2]  = 3.0;
  Output.Inside    = 4.0;

  return Output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(MAX_POINTS)]
[patchconstantfunc("SubDToBezierConstantsHS")]
BEZIER_CONTROL_POINT SubDToBezierHS(InputPatch<VS_CONTROL_POINT_OUTPUT, MAX_POINTS> ip, uint cpid : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID) {
  VS_CONTROL_POINT_OUTPUT vsOutput;
  BEZIER_CONTROL_POINT result;
  result.vPosition = vsOutput.vPosition;
  return result;
}
