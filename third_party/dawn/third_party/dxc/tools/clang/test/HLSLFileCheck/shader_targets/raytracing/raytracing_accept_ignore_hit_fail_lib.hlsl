// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// verify failure for exported library function
// CHECK: Opcode IgnoreHit not valid in shader model lib_6_3(lib)
// CHECK: Opcode AcceptHitAndEndSearch not valid in shader model lib_6_3(lib)


struct Payload {
  float foo;
};
struct Attr {
  float2 bary;
};

void libfunc(inout Payload p, Attr a)  {
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
