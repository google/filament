// RUN: %dxc -T hs_6_0 -E SubDToBezierHS -fcgl  %s -spirv | FileCheck %s

#include "bezier_common_hull.hlsli"

[domain("tri")]
[partitioning("fractional_odd")]
// CHECK: OpExecutionMode %SubDToBezierHS PointMode
[outputtopology("point")]
[outputcontrolpoints(16)]
[patchconstantfunc("SubDToBezierConstantsHS")]
BEZIER_CONTROL_POINT SubDToBezierHS(InputPatch<VS_CONTROL_POINT_OUTPUT, MAX_POINTS> ip, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID) {
  VS_CONTROL_POINT_OUTPUT vsOutput;
  BEZIER_CONTROL_POINT result;
  result.vPosition = vsOutput.vPosition;
  return result;
}
