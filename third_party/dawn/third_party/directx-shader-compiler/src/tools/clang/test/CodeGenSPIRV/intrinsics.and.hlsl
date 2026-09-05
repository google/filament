// RUN: %dxc -T ps_6_0 -E main -HV 2021 -fcgl  %s -spirv | FileCheck %s
// RUN: %dxc -T ps_6_0 -E main -HV 2018 -fcgl  %s -spirv | FileCheck %s

// CHECK-DAG: [[v3_0:%[0-9]+]] = OpConstantComposite %v3int %int_0 %int_0 %int_0
// CHECK-DAG: [[v3_1:%[0-9]+]] = OpConstantComposite %v3int %int_1 %int_1 %int_1

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    bool a, b, c;
    // Plain assign (scalar)
// CHECK:      [[a0:%[0-9]+]] = OpLoad %bool %a
// CHECK-NEXT: [[b0:%[0-9]+]] = OpLoad %bool %b
// CHECK-NEXT: [[and0:%[0-9]+]] = OpLogicalAnd %bool [[a0]] [[b0]]
// CHECK-NEXT: OpStore %c [[and0]]
    c = and(a, b);

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
    k = and(i, j);
    q = and(o, p);

// The result of '&&' could be 'const bool'. In such cases, make sure
// the result type is correct.
// CHECK:        [[a1:%[0-9]+]] = OpLoad %bool %a
// CHECK-NEXT:   [[b1:%[0-9]+]] = OpLoad %bool %b
// CHECK-NEXT: [[and3:%[0-9]+]] = OpLogicalAnd %bool [[a1]] [[b1]]
// CHECK-NEXT:      {{%[0-9]+}} = OpCompositeConstruct %v2bool [[and3]] %true
    bool2 t = bool2(and(a, b), true);

    int a_0, b_0, c_0;
    // Plain assign (scalar)
// CHECK:      [[a0_int:%[0-9]+]] = OpLoad %int %a_0
// CHECK-NEXT: [[a0:%[0-9]+]] = OpINotEqual %bool [[a0_int]] %int_0
// CHECK-NEXT: [[b0_int:%[0-9]+]] = OpLoad %int %b_0
// CHECK-NEXT: [[b0:%[0-9]+]] = OpINotEqual %bool [[b0_int]] %int_0
// CHECK-NEXT: [[and0:%[0-9]+]] = OpLogicalAnd %bool [[a0]] [[b0]]
// CHECK-NEXT: [[sel:%[0-9]+]] = OpSelect %int [[and0]] %int_1 %int_0
// CHECK-NEXT: OpStore %c_0 [[sel]]
    c_0 = and(a_0, b_0);

    int1 i_0, j_0, k_0;
    int3 o_0, p_0, q_0;
    // Plain assign (vector)
// CHECK-NEXT: [[i0_int:%[0-9]+]] = OpLoad %int %i_0
// CHECK-NEXT: [[i0:%[0-9]+]] = OpINotEqual %bool [[i0_int]] %int_0
// CHECK-NEXT: [[j0_int:%[0-9]+]] = OpLoad %int %j_0
// CHECK-NEXT: [[j0:%[0-9]+]] = OpINotEqual %bool [[j0_int]] %int_0
// CHECK-NEXT: [[and1:%[0-9]+]] = OpLogicalAnd %bool [[i0]] [[j0]]
// CHECK-NEXT: [[sel:%[0-9]+]] = OpSelect %int [[and1]] %int_1 %int_0
// CHECK-NEXT: OpStore %k_0 [[sel]]
    k_0 = and(i_0, j_0);

// CHECK-NEXT: [[o0_int:%[0-9]+]] = OpLoad %v3int %o_0
// CHECK-NEXT: [[o0:%[0-9]+]] = OpINotEqual %v3bool [[o0_int]] [[v3_0]]
// CHECK-NEXT: [[p0_int:%[0-9]+]] = OpLoad %v3int %p_0
// CHECK-NEXT: [[p0:%[0-9]+]] = OpINotEqual %v3bool [[p0_int]] [[v3_0]]
// CHECK-NEXT: [[and2:%[0-9]+]] = OpLogicalAnd %v3bool [[o0]] [[p0]]
// CHECK-NEXT: [[sel:%[0-9]+]] = OpSelect %v3int [[and2]] [[v3_1]] [[v3_0]]
// CHECK-NEXT: OpStore %q_0 [[sel]]
    q_0 = and(o_0, p_0);

// The result of '&&' could be 'const bool'. In such cases, make sure
// the result type is correct.
// CHECK:      [[a0_int:%[0-9]+]] = OpLoad %int %a_0
// CHECK-NEXT: [[a0:%[0-9]+]] = OpINotEqual %bool [[a0_int]] %int_0
// CHECK-NEXT: [[b0_int:%[0-9]+]] = OpLoad %int %b_0
// CHECK-NEXT: [[b0:%[0-9]+]] = OpINotEqual %bool [[b0_int]] %int_0
// CHECK-NEXT: [[and0:%[0-9]+]] = OpLogicalAnd %bool [[a0]] [[b0]]
// CHECK-NEXT:      {{%[0-9]+}} = OpCompositeConstruct %v2bool [[and0]] %true
    t = bool2(and(a_0, b_0), true);
}
