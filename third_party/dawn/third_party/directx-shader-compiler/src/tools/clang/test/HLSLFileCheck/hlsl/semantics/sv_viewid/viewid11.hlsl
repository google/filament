// RUN: %dxilver 1.1 | %dxc -E main -T ps_6_1 %s | FileCheck %s

// CHECK: Name                 Index             InterpMode DynIdx
// CHECK: -------------------- ----- ---------------------- ------
// CHECK: AAA                      0                 linear     yz

// CHECK: Number of inputs: 12, outputs: 4
// CHECK: Outputs dependent on ViewId: {  }
// CHECK: Inputs contributing to computation of Outputs:
// CHECK:   output 0 depends on inputs: { 0, 1, 5, 7, 9 }
// CHECK:   output 3 depends on inputs: { 1, 2, 5, 6, 9, 10 }

uint g1;

float4 main(float4 a[3] : AAA, uint viewid : SV_ViewId) : SV_Target
{
  float4 ret = 2;
  [branch]
  switch (a[g1].y) {
  case 0:
    [branch]
    if (a[0].x) {
      ret.x += a[1].w;
    }
    break;
  case 1:
  case 2:
    ret.w += 777;
    break;
  case 3:
  case 77:
    break;
  default:
    ret.w = a[g1].z;
    break;
  }
  return ret;
}
