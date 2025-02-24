// RUN: %dxc -T hs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

#include "bezier_common_hull.hlsli"

// CHECK: [[fType:%[0-9]+]] = OpTypeFunction %HS_CONSTANT_DATA_OUTPUT
// CHECK:          %main = OpFunction %void None {{%[0-9]+}}
// CHECK:       {{%[0-9]+}} = OpFunctionCall %HS_CONSTANT_DATA_OUTPUT %PCF
// CHECK:           %PCF = OpFunction %HS_CONSTANT_DATA_OUTPUT None [[fType]]

// PCF does not take any args
HS_CONSTANT_DATA_OUTPUT PCF() {
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

[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(16)]
[patchconstantfunc("PCF")]
BEZIER_CONTROL_POINT main(InputPatch<VS_CONTROL_POINT_OUTPUT, MAX_POINTS> ip, uint i : SV_OutputControlPointID, uint PatchID : SV_PrimitiveID) {
  VS_CONTROL_POINT_OUTPUT vsOutput;
  BEZIER_CONTROL_POINT result;
  result.vPosition = vsOutput.vPosition;
  return result;
}
