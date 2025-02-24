// RUN: %dxc -T ps_6_0 -E main -HV 2021 -fcgl  %s -spirv | FileCheck %s
// RUN: %dxc -T ps_6_0 -E main -HV 2018 -fcgl  %s -spirv | FileCheck %s

// CHECK: [[v3_0:%[0-9]+]] = OpConstantComposite %v3int %int_0 %int_0 %int_0

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    bool a, b, c;
    // Plain assign (scalar)
// CHECK:      [[a0:%[0-9]+]] = OpLoad %bool %a
// CHECK-NEXT: [[b0:%[0-9]+]] = OpLoad %bool %b
// CHECK-NEXT: [[or0:%[0-9]+]] = OpLogicalOr %bool [[a0]] [[b0]]
// CHECK-NEXT: OpStore %c [[or0]]
    c = or(a, b);

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
    k = or(i, j);
    q = or(o, p);

    int r, s;
    bool t;
    // Plain assign (scalar)
// CHECK:      [[r0_int:%[0-9]+]] = OpLoad %int %r
// CHECK-NEXT: [[r0:%[0-9]+]] = OpINotEqual %bool [[r0_int]] %int_0
// CHECK-NEXT: [[s0_int:%[0-9]+]] = OpLoad %int %s
// CHECK-NEXT: [[s0:%[0-9]+]] = OpINotEqual %bool [[s0_int]] %int_0
// CHECK-NEXT: [[or0:%[0-9]+]] = OpLogicalOr %bool [[r0]] [[s0]]
// CHECK-NEXT: OpStore %t [[or0]]
    t = or(r, s);

    int1 u, v;
    bool1 w;
    // Plain assign (vector)
// CHECK-NEXT: [[u0_int:%[0-9]+]] = OpLoad %int %u
// CHECK-NEXT: [[u0:%[0-9]+]] = OpINotEqual %bool [[u0_int]] %int_0
// CHECK-NEXT: [[v0_int:%[0-9]+]] = OpLoad %int %v
// CHECK-NEXT: [[v0:%[0-9]+]] = OpINotEqual %bool [[v0_int]] %int_0
// CHECK-NEXT: [[or1:%[0-9]+]] = OpLogicalOr %bool [[u0]] [[v0]]
// CHECK-NEXT: OpStore %w [[or1]]
    w = or(u, v);

    int3 x, y;
    bool3 z;
// CHECK-NEXT: [[x0_int:%[0-9]+]] = OpLoad %v3int %x
// CHECK-NEXT: [[x0:%[0-9]+]] = OpINotEqual %v3bool [[x0_int]] [[v3_0]]
// CHECK-NEXT: [[y0_int:%[0-9]+]] = OpLoad %v3int %y
// CHECK-NEXT: [[y0:%[0-9]+]] = OpINotEqual %v3bool [[y0_int]] [[v3_0]]
// CHECK-NEXT: [[or2:%[0-9]+]] = OpLogicalOr %v3bool [[x0]] [[y0]]
// CHECK-NEXT: OpStore %z [[or2]]
    z = or(x, y);
}
