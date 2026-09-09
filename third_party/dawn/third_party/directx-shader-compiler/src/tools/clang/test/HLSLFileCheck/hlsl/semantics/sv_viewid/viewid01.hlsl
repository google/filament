// RUN: %dxilver 1.1 | %dxc -E main -T ps_6_1 %s | FileCheck %s

// CHECK: Number of inputs: 4, outputs: 4
// CHECK: Outputs dependent on ViewId: { 1 }
// CHECK: Inputs contributing to computation of Outputs:
// CHECK:   output 0 depends on inputs: { 0 }
// CHECK:   output 1 depends on inputs: { 1, 2 }
// CHECK:   output 2 depends on inputs: { 2 }
// CHECK:   output 3 depends on inputs: { 3 }

float4 main(float4 a : AAA, uint viewid : SV_ViewId) : SV_Target
{
  float4 ret = a;
  ret.y += a.z + viewid;
  return ret;
}
