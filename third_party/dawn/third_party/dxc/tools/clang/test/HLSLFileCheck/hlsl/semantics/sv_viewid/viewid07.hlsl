// RUN: %dxilver 1.1 | %dxc -E main -T ps_6_1 %s | FileCheck %s

// CHECK: Number of inputs: 4, outputs: 4
// CHECK: Outputs dependent on ViewId: { 2 }
// CHECK: Inputs contributing to computation of Outputs:
// CHECK:   output 0 depends on inputs: { 1 }
// CHECK:   output 1 depends on inputs: { 1 }
// CHECK:   output 2 depends on inputs: { 0, 1, 2, 3 }
// CHECK:   output 3 depends on inputs: { 1 }

uint g1;

float4 main(float4 a : AAA, uint viewid : SV_ViewId) : SV_Target
{
  float4 ret = a.y;
  float x[2] = {a.x, a.z};
  x[a.w] = viewid;
  ret.z += x[g1];
  return ret;
}
