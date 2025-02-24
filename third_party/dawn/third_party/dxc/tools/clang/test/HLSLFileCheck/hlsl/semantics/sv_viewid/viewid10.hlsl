// RUN: %dxilver 1.1 | %dxc -E main -T ps_6_1 %s | FileCheck %s

// CHECK: Name                 Index             InterpMode DynIdx
// CHECK: -------------------- ----- ---------------------- ------
// CHECK: AAA                      0                 linear      y

// CHECK: Number of inputs: 8, outputs: 4
// CHECK: Outputs dependent on ViewId: {  }
// CHECK: Inputs contributing to computation of Outputs:
// CHECK:   output 0 depends on inputs: { 0, 1, 5 }
// CHECK:   output 3 depends on inputs: { 1, 3, 5 }

uint g1;

float4 main(float4 a[2] : AAA, uint viewid : SV_ViewId) : SV_Target
{
  float4 ret = 2;
  [branch]
  switch (a[g1].y) {
  case 0:
    [branch]
    if (a[0].x) {
      ret.x += 5;
    }
    break;
  case 1:
    ret.w += a[0].w;
    break;
  }
  return ret;
}
