// RUN: %dxc -T ps_6_2 -E main -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

// CHECK: [[v2c31:%[0-9]+]] = OpConstantComposite %v2uint %uint_31 %uint_31
// CHECK: [[v3c63:%[0-9]+]] = OpConstantComposite %v3long %long_63 %long_63 %long_63
// CHECK: [[v4c15:%[0-9]+]] = OpConstantComposite %v4ushort %ushort_15 %ushort_15 %ushort_15 %ushort_15
void main() {
    int       a, b;
    uint2     d, e;

    int64_t3  g, h;
    uint64_t  j, k;

    int16_t   m, n;
    uint16_t4 p, q;

// CHECK:        [[b:%[0-9]+]] = OpLoad %int %b
// CHECK:      [[rhs:%[0-9]+]] = OpBitwiseAnd %int [[b]] %int_31
// CHECK-NEXT:                OpShiftRightArithmetic %int {{%[0-9]+}} [[rhs]]
    a >>= b;

// CHECK:        [[e:%[0-9]+]] = OpLoad %v2uint %e
// CHECK:      [[rhs_0:%[0-9]+]] = OpBitwiseAnd %v2uint [[e]] [[v2c31]]
// CHECK-NEXT:                OpShiftRightLogical %v2uint {{%[0-9]+}} [[rhs_0]]
    d >>= e;

// CHECK:        [[h:%[0-9]+]] = OpLoad %v3long %h
// CHECK:      [[rhs_1:%[0-9]+]] = OpBitwiseAnd %v3long [[h]] [[v3c63]]
// CHECK-NEXT:                OpShiftRightArithmetic %v3long {{%[0-9]+}} [[rhs_1]]
    g >>= h;

// CHECK:        [[k:%[0-9]+]] = OpLoad %ulong %k
// CHECK:      [[rhs_2:%[0-9]+]] = OpBitwiseAnd %ulong [[k]] %ulong_63
// CHECK-NEXT:                OpShiftRightLogical %ulong {{%[0-9]+}} [[rhs_2]]
    j >>= k;

// CHECK:        [[n:%[0-9]+]] = OpLoad %short %n
// CHECK:      [[rhs_3:%[0-9]+]] = OpBitwiseAnd %short [[n]] %short_15
// CHECK-NEXT:                OpShiftRightArithmetic %short {{%[0-9]+}} [[rhs_3]]
    m >>= n;

// CHECK:        [[q:%[0-9]+]] = OpLoad %v4ushort %q
// CHECK:      [[rhs_4:%[0-9]+]] = OpBitwiseAnd %v4ushort [[q]] [[v4c15]]
// CHECK-NEXT:                OpShiftRightLogical %v4ushort {{%[0-9]+}} [[rhs_4]]
    p >>= q;
}
