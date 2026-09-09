// RUN: %dxc -T hs_6_0 -E SubDToBezierHS -fcgl  %s -spirv | FileCheck %s

#define MAX_POINTS 16

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
  uint     pointID 	: ControlPointID;
};

// Output patch constant data.
struct HS_CONSTANT_DATA_OUTPUT
{
  float Edges[4]        : SV_TessFactor;
  float Inside[2]       : SV_InsideTessFactor;

  float3 vTangent[4]    : TANGENT;
  float2 vUV[4]         : TEXCOORD;
  float3 vTanUCorner[4] : TANUCORNER;
  float3 vTanVCorner[4] : TANVCORNER;
  float4 vCWts          : TANWEIGHTS;
};

HS_CONSTANT_DATA_OUTPUT PCF(OutputPatch<BEZIER_CONTROL_POINT, MAX_POINTS> op) {
// CHECK: %op = OpFunctionParameter %_ptr_Function__arr_BEZIER_CONTROL_POINT_uint_16
  HS_CONSTANT_DATA_OUTPUT Output;
  // Must initialize Edges and Inside; otherwise HLSL validation will fail.
  Output.Edges[0]  = 1.0;
  Output.Edges[1]  = 2.0;
  Output.Edges[2]  = 3.0;
  Output.Edges[3]  = 4.0;
  Output.Inside[0] = 5.0;
  Output.Inside[1] = 6.0;

  uint x = 5;

// CHECK:      [[op_1_loc:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float %op %uint_1 %int_0
// CHECK-NEXT:          {{%[0-9]+}} = OpLoad %v3float [[op_1_loc]]
  float3 out1pos = op[1].vPosition;

// CHECK:             [[x:%[0-9]+]] = OpLoad %uint %x
// CHECK-NEXT: [[op_x_loc:%[0-9]+]] = OpAccessChain %_ptr_Function_uint %op [[x]] %int_1
// CHECK-NEXT:          {{%[0-9]+}} = OpLoad %uint [[op_x_loc]]
  uint out5id = op[x].pointID;

  return Output;
}

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("line")]
[outputcontrolpoints(16)]
[patchconstantfunc("PCF")]
BEZIER_CONTROL_POINT SubDToBezierHS(InputPatch<VS_CONTROL_POINT_OUTPUT, MAX_POINTS> ip, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID) {
  BEZIER_CONTROL_POINT result;
  uint y = 5;

// CHECK:      [[ip_1_loc:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float %ip %uint_1 %int_0
// CHECK-NEXT:          {{%[0-9]+}} = OpLoad %v3float [[ip_1_loc]]
  result.vPosition = ip[1].vPosition;

// CHECK:             [[y:%[0-9]+]] = OpLoad %uint %y
// CHECK-NEXT: [[ip_y_loc:%[0-9]+]] = OpAccessChain %_ptr_Function_v3float %ip [[y]] %int_2
// CHECK-NEXT:          {{%[0-9]+}} = OpLoad %v3float [[ip_y_loc]]
  result.vPosition = ip[y].vTangent;

  return result;
}
