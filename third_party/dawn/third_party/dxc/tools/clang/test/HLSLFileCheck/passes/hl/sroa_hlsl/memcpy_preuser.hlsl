// RUN: %dxc -T ps_6_0 %s | FileCheck %s

// A test that has the only user of a memcpy before it
// Meant to test the differnce between dominator trees

// this could theoretically cause a big problem by using postdom instead of dom
// to determine if a memcpy src can replace all the dests.
// The loop after if postdoms the if, so the src of that memcpy
// replaces the ostruct.fval2 in the if
// however, the way dxilgen handles cbuffers, it fixes it all up anyway.
// Nevertheless, the weirdo backward dependency this creates results
// in more complicated code when RAUW is incorrectly used rather than ld/str
// Including placing a lot of the cbuffer loading in the inner if block

// CHECK: @main
// CHECK-NOT: memcpy

// broken case will have no select and cbuffer load will precede fadd
// CHECK: fadd
// CHECK: select i1
// CHECK: cbufferLoadLegacy
// CHECK: add nuw nsw
// CHECK: icmp
// CHECK: br i1
struct OuterStruct
{
  float fval;
  float fval2;
};

cbuffer cbuf : register(b1)
{
 OuterStruct g_oStruct[1];
};

float main(int doit : A) : SV_Target
{
  float res = 0.0;
  OuterStruct oStruct;
  // Need a loop so the dest user can come before the memcpy
  for (int i = 0; i < doit; i++) {
    // This should be expressable as a select unless a bunch of mem stuff gets crammed in
    if(i%2 == 0) {
      res += oStruct.fval2;
    }
    // This block post dominates the if block
    oStruct = g_oStruct[doit];
  }

  return res;
}
