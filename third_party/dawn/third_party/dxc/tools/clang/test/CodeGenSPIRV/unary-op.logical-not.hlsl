// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel
    bool a, b;
// CHECK:      [[a0:%[0-9]+]] = OpLoad %bool %a
// CHECK-NEXT: [[b0:%[0-9]+]] = OpLogicalNot %bool [[a0]]
// CHECK-NEXT: OpStore %b [[b0]]
    b = !a;

    bool1 c, d;
// CHECK-NEXT: [[c0:%[0-9]+]] = OpLoad %bool %c
// CHECK-NEXT: [[d0:%[0-9]+]] = OpLogicalNot %bool [[c0]]
// CHECK-NEXT: OpStore %d [[d0]]
    d = !c;

    bool2 i, j;
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %v2bool %i
// CHECK-NEXT: [[j0:%[0-9]+]] = OpLogicalNot %v2bool [[i0]]
// CHECK-NEXT: OpStore %j [[j0]]
    j = !i;
}
