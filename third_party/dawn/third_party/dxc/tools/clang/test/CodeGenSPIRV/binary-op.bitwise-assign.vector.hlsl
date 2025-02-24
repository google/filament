// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    int1 a, b;
    uint2 i, j;

// CHECK:      [[a0:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b0:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[and0:%[0-9]+]] = OpBitwiseAnd %int [[b0]] [[a0]]
// CHECK-NEXT: OpStore %b [[and0]]
    b &= a;
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %v2uint %i
// CHECK-NEXT: [[j0:%[0-9]+]] = OpLoad %v2uint %j
// CHECK-NEXT: [[and1:%[0-9]+]] = OpBitwiseAnd %v2uint [[j0]] [[i0]]
// CHECK-NEXT: OpStore %j [[and1]]
    j &= i;

// CHECK-NEXT: [[a1:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b1:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[or0:%[0-9]+]] = OpBitwiseOr %int [[b1]] [[a1]]
// CHECK-NEXT: OpStore %b [[or0]]
    b |= a;
// CHECK-NEXT: [[i1:%[0-9]+]] = OpLoad %v2uint %i
// CHECK-NEXT: [[j1:%[0-9]+]] = OpLoad %v2uint %j
// CHECK-NEXT: [[or1:%[0-9]+]] = OpBitwiseOr %v2uint [[j1]] [[i1]]
// CHECK-NEXT: OpStore %j [[or1]]
    j |= i;

// CHECK-NEXT: [[a2:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT: [[b2:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[xor0:%[0-9]+]] = OpBitwiseXor %int [[b2]] [[a2]]
// CHECK-NEXT: OpStore %b [[xor0]]
    b ^= a;
// CHECK-NEXT: [[i2:%[0-9]+]] = OpLoad %v2uint %i
// CHECK-NEXT: [[j2:%[0-9]+]] = OpLoad %v2uint %j
// CHECK-NEXT: [[xor1:%[0-9]+]] = OpBitwiseXor %v2uint [[j2]] [[i2]]
// CHECK-NEXT: OpStore %j [[xor1]]
    j ^= i;
}
