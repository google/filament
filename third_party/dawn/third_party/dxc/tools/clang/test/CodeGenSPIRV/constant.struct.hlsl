// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct S {
    uint a;
    bool2 b;
    float2x2 c;
};

struct T {
    S x;
    int y;
};

void main() {
    // TODO: Okay, we are not acutally generating constants here.
    // We should optimize to use OpConstantComposite for the following.
// CHECK:      [[b:%[0-9]+]] = OpCompositeConstruct %v2bool %true %false
// CHECK-NEXT: [[c1:%[0-9]+]] = OpCompositeConstruct %v2float %float_1 %float_2
// CHECK-NEXT: [[c2:%[0-9]+]] = OpCompositeConstruct %v2float %float_3 %float_4
// CHECK-NEXT: [[c:%[0-9]+]] = OpCompositeConstruct %mat2v2float [[c1]] [[c2]]
// CHECK-NEXT: [[s:%[0-9]+]] = OpCompositeConstruct %S %uint_1 [[b]] [[c]]
// CHECK-NEXT: {{%[0-9]+}} = OpCompositeConstruct %T [[s]] %int_5
    T t = {1, true, false, 1.0, 2.0, 3.0, 4.0, 5};
}
