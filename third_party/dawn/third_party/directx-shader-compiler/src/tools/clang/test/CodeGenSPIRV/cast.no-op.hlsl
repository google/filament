// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel
    int a, b;
// CHECK: [[a0:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: OpStore %b [[a0]]
    b = int(a);

    uint1 c, d;
// CHECK-NEXT: [[c0:%[0-9]+]] = OpLoad %uint %c
// CHECK-NEXT: OpStore %d [[c0]]
    d = uint1(c);

    float2 e, f;
// CHECK-NEXT: [[e0:%[0-9]+]] = OpLoad %v2float %e
// CHECK-NEXT: OpStore %f [[e0]]
    f = float2(e);
}
