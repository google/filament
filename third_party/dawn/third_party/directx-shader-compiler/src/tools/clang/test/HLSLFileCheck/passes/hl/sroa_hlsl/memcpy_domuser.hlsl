// RUN: %dxc -T ps_6_0 %s | FileCheck %s

// Test memcpy with src that dominates, but doesn't postdominate its uses

// This produces suboptimal code when postdom check is used
// When corrected, RAUW can be used

// CHECK: @main
// CHECK-NOT: memcpy
// CHECK-NOT: = br
// CHECK: icmp slt i32
// CHECK: icmp sgt i32
// CHECK: and i1
// CHECK: br i1
// CHECK: cbufferLoadLegacy
// CHECK: fadd
// CHECK: ret void
struct OuterStruct
{
  float fval;
  float fval2;
};

cbuffer cbuf : register(b1)
{
 OuterStruct g_oStruct[1];
};

float main(int doit : A, int dontit: B) : SV_Target
{
  OuterStruct oStruct;
  float addon = 0.0;
  // break up blocks so not in entry
  // RAUW is able to combine these conditionals with an AND
  // ld/str inserts loads between them at least temporarily
  if (doit < 10) {
    oStruct = g_oStruct[doit];
    if (dontit > 0)
      addon += oStruct.fval2 + oStruct.fval; // dominated, but not post dominated by memcpy. so it gets ld/str
  }

  return addon;
}
