// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    float2 f[2];
};

void main() {
// CHECK:      [[v0102:%[0-9]+]] = OpCompositeConstruct %v2float %float_1 %float_2
// CHECK-NEXT: [[v0304:%[0-9]+]] = OpCompositeConstruct %v2float %float_3 %float_4
// CHECK-NEXT:   [[vf0:%[0-9]+]] = OpCompositeConstruct %_arr_v2float_uint_2 [[v0102]] [[v0304]]
// CHECK-NEXT:   [[vs0:%[0-9]+]] = OpCompositeConstruct %S [[vf0]]

// CHECK-NEXT: [[v0506:%[0-9]+]] = OpCompositeConstruct %v2float %float_5 %float_6
// CHECK-NEXT: [[v0708:%[0-9]+]] = OpCompositeConstruct %v2float %float_7 %float_8
// CHECK-NEXT:   [[vf1:%[0-9]+]] = OpCompositeConstruct %_arr_v2float_uint_2 [[v0506]] [[v0708]]
// CHECK-NEXT:   [[vs1:%[0-9]+]] = OpCompositeConstruct %S [[vf1]]

// CHECK-NEXT: [[v0910:%[0-9]+]] = OpCompositeConstruct %v2float %float_9 %float_10
// CHECK-NEXT: [[v1112:%[0-9]+]] = OpCompositeConstruct %v2float %float_11 %float_12
// CHECK-NEXT:   [[vf2:%[0-9]+]] = OpCompositeConstruct %_arr_v2float_uint_2 [[v0910]] [[v1112]]
// CHECK-NEXT:   [[vs2:%[0-9]+]] = OpCompositeConstruct %S [[vf2]]

// CHECK-NEXT: [[v1314:%[0-9]+]] = OpCompositeConstruct %v2float %float_13 %float_14
// CHECK-NEXT: [[v1516:%[0-9]+]] = OpCompositeConstruct %v2float %float_15 %float_16
// CHECK-NEXT:   [[vf3:%[0-9]+]] = OpCompositeConstruct %_arr_v2float_uint_2 [[v1314]] [[v1516]]
// CHECK-NEXT:   [[vs3:%[0-9]+]] = OpCompositeConstruct %S [[vf3]]

// CHECK-NEXT:   [[va4:%[0-9]+]] = OpCompositeConstruct %_arr_S_uint_4 [[vs0]] [[vs1]] [[vs2]] [[vs3]]
// CHECK-NEXT:       {{%[0-9]+}} = OpCompositeConstruct %_arr__arr_S_uint_4_uint_1 [[va4]]
    S var[1][4] = {1.0,  2.0,  3.0,  4.0,  5.0,  6.0,  7.0,  8.0,
                   9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0};
}
