// RUN: %dxilver 1.1 | %dxc -E main -T ps_6_1 %s | FileCheck %s

// CHECK: Number of inputs: 8, outputs: 4
// CHECK: Outputs dependent on ViewId: { 0 }
// CHECK: Inputs contributing to computation of Outputs:
// CHECK:   output 0 depends on inputs: { 0, 5, 7 }
// CHECK:   output 1 depends on inputs: { 1 }
// CHECK:   output 2 depends on inputs: { 2 }
// CHECK:   output 3 depends on inputs: { 3 }

float4 main(float4 a : AAA, uint4 b : BBB, uint viewid : SV_ViewId) : SV_Target
{
  float4 ret = a;
  ret.x = a.x + viewid + b.y;
  [branch]
  if (b.w > 3) {
    ret.x = 0;
  }
  return ret;
}
