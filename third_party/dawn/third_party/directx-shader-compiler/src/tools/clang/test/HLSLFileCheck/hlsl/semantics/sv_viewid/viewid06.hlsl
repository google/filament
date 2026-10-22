// RUN: %dxilver 1.1 | %dxc -E main -T ps_6_1 %s | FileCheck %s

// CHECK: Name                 Index             InterpMode DynIdx
// CHECK: -------------------- ----- ---------------------- ------
// CHECK: AAA                      0                 linear
// CHECK: BBB                      0        nointerpolation     yw

// CHECK: Number of inputs: 12, outputs: 4
// CHECK: Outputs dependent on ViewId: { 0 }
// CHECK: Inputs contributing to computation of Outputs:
// CHECK:   output 0 depends on inputs: { 0, 5, 7, 9, 11 }
// CHECK:   output 1 depends on inputs: { 1 }
// CHECK:   output 2 depends on inputs: { 2 }
// CHECK:   output 3 depends on inputs: { 3 }

uint g1;

float4 main(float4 a : AAA, uint4 b[2] : BBB, uint viewid : SV_ViewId) : SV_Target
{
  float4 ret = a;
  ret.x = a.x + viewid + b[g1].y;
  [branch]
  if (b[g1].w > 3) {
    ret.x = 0;
  }
  return ret;
}
