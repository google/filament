// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
    uint uVal;
    bool bVal;

    float fVal;
    int iVal;

    // No conversion of lhs
// CHECK:      [[b_bool:%[0-9]+]] = OpLoad %bool %bVal
// CHECK-NEXT: [[b_uint:%[0-9]+]] = OpSelect %uint [[b_bool]] %uint_1 %uint_0
// CHECK-NEXT: [[u_uint:%[0-9]+]] = OpLoad %uint %uVal
// CHECK-NEXT:    [[add:%[0-9]+]] = OpIAdd %uint [[u_uint]] [[b_uint]]
// CHECK-NEXT:                   OpStore %uVal [[add]]
    uVal += bVal;

    // Convert lhs to the type of rhs, do computation, and then convert back
// CHECK:        [[f_float:%[0-9]+]] = OpLoad %float %fVal
// CHECK-NEXT:     [[i_int:%[0-9]+]] = OpLoad %int %iVal
// CHECK-NEXT:   [[i_float:%[0-9]+]] = OpConvertSToF %float [[i_int]]
// CHECK-NEXT: [[mul_float:%[0-9]+]] = OpFMul %float [[i_float]] [[f_float]]
// CHECK-NEXT:   [[mul_int:%[0-9]+]] = OpConvertFToS %int [[mul_float]]
// CHECK-NEXT:                      OpStore %iVal [[mul_int]]
    iVal *= fVal;
}
