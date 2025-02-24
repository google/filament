// RUN: %dxc -T ps_6_0 -HV 2018 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    bool a, b, c;
    // Plain assign (scalar)
// CHECK:      [[a0:%[0-9]+]] = OpLoad %bool %a
// CHECK-NEXT: [[b0:%[0-9]+]] = OpLoad %bool %b
// CHECK-NEXT: [[or0:%[0-9]+]] = OpLogicalOr %bool [[a0]] [[b0]]
// CHECK-NEXT: OpStore %c [[or0]]
    c = a || b;

    bool1 i, j, k;
    bool3 o, p, q;
    // Plain assign (vector)
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %bool %i
// CHECK-NEXT: [[j0:%[0-9]+]] = OpLoad %bool %j
// CHECK-NEXT: [[or1:%[0-9]+]] = OpLogicalOr %bool [[i0]] [[j0]]
// CHECK-NEXT: OpStore %k [[or1]]
// CHECK-NEXT: [[o0:%[0-9]+]] = OpLoad %v3bool %o
// CHECK-NEXT: [[p0:%[0-9]+]] = OpLoad %v3bool %p
// CHECK-NEXT: [[or2:%[0-9]+]] = OpLogicalOr %v3bool [[o0]] [[p0]]
// CHECK-NEXT: OpStore %q [[or2]]
    k = i || j;
    q = o || p;
}
