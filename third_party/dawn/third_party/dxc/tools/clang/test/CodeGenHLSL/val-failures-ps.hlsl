// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: Interpolation mode for PS input position must be linear_noperspective_centroid or 

min16float min16float_val;

struct PSResult {
  float4 color: SV_Target;
  float depth: SV_DepthGreaterEqual;
};

PSResult main(linear float4 p: SV_Position) {
  PSResult result;
  result.color = p;
  result.depth = 0.5f;
  return result;
}
