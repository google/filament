// RUN: %dxilver 1.1 | %dxc -E main -T ps_6_1 %s | FileCheck %s

// CHECK: Name                 Index             InterpMode DynIdx
// CHECK: -------------------- ----- ---------------------- ------
// CHECK: AAA                      0                 linear      y

// CHECK: Number of inputs: 12, outputs: 4
// CHECK: Outputs dependent on ViewId: {  }
// CHECK: Inputs contributing to computation of Outputs:
// CHECK:   output 0 depends on inputs: { 1, 2, 3, 5, 8, 9 }
// CHECK:   output 1 depends on inputs: { 2 }
// CHECK:   output 2 depends on inputs: { 2 }
// CHECK:   output 3 depends on inputs: { 2 }

uint g1;

float4 main(float4 a[3] : AAA, uint viewid : SV_ViewId) : SV_Target
{
  float4 ret = a[0].z;
  if (a[g1].y) {
    if (g1) {
      if (a[0].w) {
        ret.x = a[2].x;
      }
    }
  }
  return ret;
}
