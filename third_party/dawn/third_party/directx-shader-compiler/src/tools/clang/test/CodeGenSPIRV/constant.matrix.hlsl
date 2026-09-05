// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// TODO: actually emit constant SPIR-V instructions for the following tests.

void main() {
// CHECK:       OpStore %a %float_1
    float1x1 a = float1x1(1.);

// CHECK-NEXT: [[b:%[0-9]+]] = OpCompositeConstruct %v2float %float_2 %float_3
// CHECK-NEXT: OpStore %b [[b]]
    float1x2 b = float1x2(2., 3.);

// CHECK-NEXT: [[c:%[0-9]+]] = OpCompositeConstruct %v2float %float_4 %float_5
// CHECK-NEXT: OpStore %c [[c]]
    float2x1 c = float2x1(4., 5.);

// CHECK-NEXT: [[d0:%[0-9]+]] = OpCompositeConstruct %v3float %float_6 %float_7 %float_8
// CHECK-NEXT: [[d1:%[0-9]+]] = OpCompositeConstruct %v3float %float_9 %float_10 %float_11
// CHECK-NEXT: [[d:%[0-9]+]] = OpCompositeConstruct %mat2v3float [[d0]] [[d1]]
// CHECK-NEXT: OpStore %d [[d]]
    float2x3 d = float2x3(6., 7., 8., 9., 10., 11.);

// CHECK-NEXT: [[e0:%[0-9]+]] = OpCompositeConstruct %v3int %int_6 %int_7 %int_8
// CHECK-NEXT: [[e1:%[0-9]+]] = OpCompositeConstruct %v3int %int_9 %int_10 %int_11
// CHECK-NEXT: [[e:%[0-9]+]] = OpCompositeConstruct %_arr_v3int_uint_2 [[e0]] [[e1]]
// CHECK-NEXT: OpStore %e [[e]]
    int2x3 e = int2x3(6, 7, 8, 9, 10, 11);
}
