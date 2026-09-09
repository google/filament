// RUN: %dxc -T hs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

#include "bezier_common_hull.hlsli"

// Test: PCF takes the PrimitiveID
// Note that in this test, the main entry point has also taken the PrimitiveID as input.


// CHECK: OpEntryPoint TessellationControl %main "main"
// CHECK-SAME: %gl_PrimitiveID

// CHECK: OpDecorate %gl_PrimitiveID BuiltIn PrimitiveId

// CHECK:     [[fType:%[0-9]+]] = OpTypeFunction %HS_CONSTANT_DATA_OUTPUT %_ptr_Function_uint
// CHECK:    %gl_PrimitiveID = OpVariable %_ptr_Input_uint Input

// CHECK:              %main = OpFunction %void None {{%[0-9]+}}
// CHECK: %param_var_PatchID = OpVariable %_ptr_Function_uint Function

// CHECK: [[pid:%[0-9]+]] = OpLoad %uint %gl_PrimitiveID
// CHECK:                OpStore %param_var_PatchID [[pid]]

// CHECK: {{%[0-9]+}} = OpFunctionCall %HS_CONSTANT_DATA_OUTPUT %PCF %param_var_PatchID

// CHECK:     %PCF = OpFunction %HS_CONSTANT_DATA_OUTPUT None [[fType]]
// CHECK: %PatchID = OpFunctionParameter %_ptr_Function_uint

HS_CONSTANT_DATA_OUTPUT PCF(uint PatchID : SV_PrimitiveID) {
  HS_CONSTANT_DATA_OUTPUT Output;
  // Must initialize Edges and Inside; otherwise HLSL validation will fail.
  Output.Edges[0]  = 1.0;
  Output.Edges[1]  = 2.0;
  Output.Edges[2]  = 3.0;
  Output.Edges[3]  = 4.0;
  Output.Inside[0] = 5.0;
  Output.Inside[1] = 6.0;
  return Output;
}

[domain("isoline")]
[partitioning("fractional_odd")]
[outputtopology("line")]
[outputcontrolpoints(16)]
[patchconstantfunc("PCF")]
BEZIER_CONTROL_POINT main(InputPatch<VS_CONTROL_POINT_OUTPUT, MAX_POINTS> ip, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID) {
  VS_CONTROL_POINT_OUTPUT vsOutput;
  BEZIER_CONTROL_POINT result;
  result.vPosition = vsOutput.vPosition + PatchID;
  return result;
}
