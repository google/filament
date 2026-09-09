// RUN: %dxc -T lib_6_3 -auto-binding-space 11 %s | FileCheck %s

// CHECK: call void @dx.op.acceptHitAndEndSearch(i32 156)
// CHECK: unreachable
// CHECK: call void @dx.op.ignoreHit(i32 155)
// CHECK: unreachable

// store at end should be unaffected by writes to result in branches
// store float 0x4004CCCCC0000000, float*

struct Payload {
  float foo;
};
struct Attr {
  float2 bary;
};

[shader("anyhit")]
void AnyHit(inout Payload p, Attr a)  {
  float4 result = 2.6;
  if (p.foo < 2) {
    result.x = 1;             // unused
    AcceptHitAndEndSearch();  // does not return
    result.y = 3;             // dead code
  }
  if (a.bary.x < 9) {
    result.x = 2;             // unused
    IgnoreHit();              // does not return
    result.y = 4;             // dead code
  }
   p.foo = result;
}
