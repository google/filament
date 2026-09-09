// RUN: %dxc -E main -T ps_6_0 -O0 %s -fcgl | FileCheck %s

// This is mostly a regression test for a bug where a bitcast
// from i32* to i1* was emitted.

// CHECK: alloca i32
// CHECK: alloca <2 x i32>
// CHECK-NOT: bitcast i32* %b to <1 x i1>*

float main() : SV_Target
{
    bool b = true;
    bool2 b2 = b.xx;
    return b2.x && b2.y ? 1 : 0;
}
