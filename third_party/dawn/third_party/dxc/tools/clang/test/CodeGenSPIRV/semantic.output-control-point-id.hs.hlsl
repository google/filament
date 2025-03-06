// RUN: %dxc -T hs_6_0 -E SubDToBezierHS -fcgl  %s -spirv | FileCheck %s

// CHECK: OpEntryPoint TessellationControl %SubDToBezierHS "SubDToBezierHS"
// CHECK-SAME: %gl_InvocationID

// CHECK: OpDecorate %gl_InvocationID BuiltIn InvocationId

// CHECK: %gl_InvocationID = OpVariable %_ptr_Input_uint Input

#include "bezier_common_hull.hlsli"

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("line")]
[outputcontrolpoints(16)]
[patchconstantfunc("SubDToBezierConstantsHS")]
BEZIER_CONTROL_POINT SubDToBezierHS(InputPatch<VS_CONTROL_POINT_OUTPUT, MAX_POINTS> ip, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID) {
  VS_CONTROL_POINT_OUTPUT vsOutput;
  BEZIER_CONTROL_POINT result;
  result.vPosition = vsOutput.vPosition;
  return result;
}
