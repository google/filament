// RUN: %dxc -T ps_6_0 -HV 2018 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    bool a, b, c;
    // Plain assign (scalar)
// CHECK:      [[a0:%[0-9]+]] = OpLoad %bool %a
// CHECK-NEXT: [[b0:%[0-9]+]] = OpLoad %bool %b
// CHECK-NEXT: [[and0:%[0-9]+]] = OpLogicalAnd %bool [[a0]] [[b0]]
// CHECK-NEXT: OpStore %c [[and0]]
    c = a && b;

    bool1 i, j, k;
    bool3 o, p, q;
    // Plain assign (vector)
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %bool %i
// CHECK-NEXT: [[j0:%[0-9]+]] = OpLoad %bool %j
// CHECK-NEXT: [[and1:%[0-9]+]] = OpLogicalAnd %bool [[i0]] [[j0]]
// CHECK-NEXT: OpStore %k [[and1]]
// CHECK-NEXT: [[o0:%[0-9]+]] = OpLoad %v3bool %o
// CHECK-NEXT: [[p0:%[0-9]+]] = OpLoad %v3bool %p
// CHECK-NEXT: [[and2:%[0-9]+]] = OpLogicalAnd %v3bool [[o0]] [[p0]]
// CHECK-NEXT: OpStore %q [[and2]]
    k = i && j;
    q = o && p;

// The result of '&&' could be 'const bool'. In such cases, make sure
// the result type is correct.
// CHECK:        [[a1:%[0-9]+]] = OpLoad %bool %a
// CHECK-NEXT:   [[b1:%[0-9]+]] = OpLoad %bool %b
// CHECK-NEXT: [[and3:%[0-9]+]] = OpLogicalAnd %bool [[a1]] [[b1]]
// CHECK-NEXT:      {{%[0-9]+}} = OpCompositeConstruct %v2bool [[and3]] %true
    bool2 t = bool2(a&&b, true);
}
