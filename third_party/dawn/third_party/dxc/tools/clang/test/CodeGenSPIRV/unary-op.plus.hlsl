// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel
    int a, b;
// CHECK:      [[a0:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: OpStore %b [[a0]]
    b = +a;

    uint c, d;
// CHECK-NEXT: [[c0:%[0-9]+]] = OpLoad %uint %c
// CHECK-NEXT: OpStore %d [[c0]]
    d = +c;

    float i, j;
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %float %i
// CHECK-NEXT: OpStore %j [[i0]]
    j = +i;

    float1 m, n;
// CHECK-NEXT: [[m0:%[0-9]+]] = OpLoad %float %m
// CHECK-NEXT: OpStore %n [[m0]]
    n = +m;

    int3 x, y;
// CHECK-NEXT: [[x0:%[0-9]+]] = OpLoad %v3int %x
// CHECK-NEXT: OpStore %y [[x0]]
    y = +x;
}
