// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// TODO:
// * operands of mixed signedness (need casting)
// * operands of bool type (need casting)

void main() {
// CHECK-LABEL: %bb_entry = OpLabel
    int a, b, c;
    uint i, j, k;

// CHECK:      [[a0:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b0:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[c0:%[0-9]+]] = OpBitwiseAnd %int [[a0]] [[b0]]
// CHECK-NEXT: OpStore %c [[c0]]
    c = a & b;
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j0:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: [[k0:%[0-9]+]] = OpBitwiseAnd %uint [[i0]] [[j0]]
// CHECK-NEXT: OpStore %k [[k0]]
    k = i & j;

// CHECK-NEXT: [[a1:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b1:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[c1:%[0-9]+]] = OpBitwiseOr %int [[a1]] [[b1]]
// CHECK-NEXT: OpStore %c [[c1]]
    c = a | b;
// CHECK-NEXT: [[i1:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j1:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: [[k1:%[0-9]+]] = OpBitwiseOr %uint [[i1]] [[j1]]
// CHECK-NEXT: OpStore %k [[k1]]
    k = i | j;

// CHECK-NEXT: [[a2:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b2:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[c2:%[0-9]+]] = OpBitwiseXor %int [[a2]] [[b2]]
// CHECK-NEXT: OpStore %c [[c2]]
    c = a ^ b;
// CHECK-NEXT: [[i2:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j2:%[0-9]+]] = OpLoad %uint %j
// CHECK-NEXT: [[k2:%[0-9]+]] = OpBitwiseXor %uint [[i2]] [[j2]]
// CHECK-NEXT: OpStore %k [[k2]]
    k = i ^ j;

// CHECK-NEXT: [[a5:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b5:%[0-9]+]] = OpNot %int [[a5]]
// CHECK-NEXT: OpStore %b [[b5]]
    b = ~a;
// CHECK-NEXT: [[i5:%[0-9]+]] = OpLoad %uint %i
// CHECK-NEXT: [[j5:%[0-9]+]] = OpNot %uint [[i5]]
// CHECK-NEXT: OpStore %j [[j5]]
    j = ~i;
}
