// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    float  a;
    float4 b;
};

struct T {
    static float4 M;
    static S      N;

    static const float4 U;

    int val;
};

// CHECK: [[v4fc:%[0-9]+]] = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4

// CHECK: %M = OpVariable %_ptr_Private_v4float Private
// CHECK: %N = OpVariable %_ptr_Private_S Private
// CHECK: %U = OpVariable %_ptr_Private_v4float Private

float4 T::M = float4(1., 2., 3., 4.);
S      T::N = {5.0, 1., 2., 3., 4.};

const float4 T::U = float4(1., 2., 3., 4.);

// CHECK-LABEL: %main = OpFunction
// CHECK:                       OpStore %M [[v4fc]]
// CHECK-NEXT: [[v1to4:%[0-9]+]] = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
// CHECK-NEXT: [[v1to5:%[0-9]+]] = OpCompositeConstruct %S %float_5 [[v1to4]]
// CHECK-NEXT:                  OpStore %N [[v1to5]]
// CHECK-NEXT:                  OpStore %U [[v4fc]]

// CHECK-LABEL: %src_main = OpFunction
float4 main(float4 input: A) : SV_Target {
    T t;

// CHECK: OpStore %M {{%[0-9]+}}
    T::M = input;
// CHECK: OpStore %M {{%[0-9]+}}
    t.M = input;
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Private_float %N %int_0
// CHECK-NEXT:           OpStore [[ptr]] %float_1
    T::N.a = 1.0;
// CHECK: [[ptr_0:%[0-9]+]] = OpAccessChain %_ptr_Private_v4float %N %int_1
// CHECK-NEXT:           OpStore [[ptr_0]] {{%[0-9]+}}
    t.N.b = input;

// CHECK:      {{%[0-9]+}} = OpLoad %v4float %M
// CHECK:      {{%[0-9]+}} = OpLoad %v4float %M
// CHECK:  [[ptr_1:%[0-9]+]] = OpAccessChain %_ptr_Private_v4float %N %int_1
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %v4float [[ptr_1]]
// CHECK:  [[ptr_2:%[0-9]+]] = OpAccessChain %_ptr_Private_v4float %N %int_1
// CHECK-NEXT: {{%[0-9]+}} = OpLoad %v4float [[ptr_2]]
// CHECK:      {{%[0-9]+}} = OpLoad %v4float %U
// CHECK:      {{%[0-9]+}} = OpLoad %v4float %U
    return T::M + t.M + T::N.b + t.N.b + T::U + t.U;
}
