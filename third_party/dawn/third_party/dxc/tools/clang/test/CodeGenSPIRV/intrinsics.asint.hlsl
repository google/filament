// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// According to HLSL reference:
// The 'asint' function can only operate on uint, float,
// vector of these scalars, and matrix of these scalars.

void main() {
    int result;
    int4 result4;

    // CHECK:      [[b:%[0-9]+]] = OpLoad %uint %b
    // CHECK-NEXT: [[b_as_int:%[0-9]+]] = OpBitcast %int [[b]]
    // CHECK-NEXT: OpStore %result [[b_as_int]]
    uint b;
    result = asint(b);

    // CHECK-NEXT: [[c:%[0-9]+]] = OpLoad %float %c
    // CHECK-NEXT: [[c_as_int:%[0-9]+]] = OpBitcast %int [[c]]
    // CHECK-NEXT: OpStore %result [[c_as_int]]
    float c;
    result = asint(c);

    // CHECK-NEXT: [[e:%[0-9]+]] = OpLoad %uint %e
    // CHECK-NEXT: [[e_as_int:%[0-9]+]] = OpBitcast %int [[e]]
    // CHECK-NEXT: OpStore %result [[e_as_int]]
    uint1 e;
    result = asint(e);

    // CHECK-NEXT: [[f:%[0-9]+]] = OpLoad %float %f
    // CHECK-NEXT: [[f_as_int:%[0-9]+]] = OpBitcast %int [[f]]
    // CHECK-NEXT: OpStore %result [[f_as_int]]
    float1 f;
    result = asint(f);

    // CHECK-NEXT: [[h:%[0-9]+]] = OpLoad %v4uint %h
    // CHECK-NEXT: [[h_as_int:%[0-9]+]] = OpBitcast %v4int [[h]]
    // CHECK-NEXT: OpStore %result4 [[h_as_int]]
    uint4 h;
    result4 = asint(h);

    // CHECK-NEXT: [[i:%[0-9]+]] = OpLoad %v4float %i
    // CHECK-NEXT: [[i_as_int:%[0-9]+]] = OpBitcast %v4int [[i]]
    // CHECK-NEXT: OpStore %result4 [[i_as_int]]
    float4 i;
    result4 = asint(i);

    float2x3 floatMat;
    uint2x3 uintMat;

// CHECK:       [[floatMat:%[0-9]+]] = OpLoad %mat2v3float %floatMat
// CHECK-NEXT: [[floatMat0:%[0-9]+]] = OpCompositeExtract %v3float [[floatMat]] 0
// CHECK-NEXT:      [[row0:%[0-9]+]] = OpBitcast %v3int [[floatMat0]]
// CHECK-NEXT: [[floatMat1:%[0-9]+]] = OpCompositeExtract %v3float [[floatMat]] 1
// CHECK-NEXT:      [[row1:%[0-9]+]] = OpBitcast %v3int [[floatMat1]]
// CHECK-NEXT:         [[j:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[row0]] [[row1]]
// CHECK-NEXT:                      OpStore %j [[j]]
    int2x3 j = asint(floatMat);
// CHECK:       [[uintMat:%[0-9]+]] = OpLoad %_arr_v3uint_uint_2 %uintMat
// CHECK-NEXT: [[uintMat0:%[0-9]+]] = OpCompositeExtract %v3uint [[uintMat]] 0
// CHECK-NEXT:     [[row0_0:%[0-9]+]] = OpBitcast %v3int [[uintMat0]]
// CHECK-NEXT: [[uintMat1:%[0-9]+]] = OpCompositeExtract %v3uint [[uintMat]] 1
// CHECK-NEXT:     [[row1_0:%[0-9]+]] = OpBitcast %v3int [[uintMat1]]
// CHECK-NEXT:        [[k:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[row0_0]] [[row1_0]]
// CHECK-NEXT:                     OpStore %k [[k]]
    int2x3 k = asint(uintMat);
}
