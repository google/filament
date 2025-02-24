// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

// CHECK:      OpStore %mat1 %float_1
    float1x1 mat1 = {1.};
// CHECK-NEXT: OpStore %mat2 %float_1
    float1x1 mat2 = {{{1.}}};
// CHECK-NEXT: OpStore %mat3 %float_1
    float1x1 mat3 = float1x1(1.);
// CHECK-NEXT: [[mat3:%[0-9]+]] = OpLoad %float %mat3
// CHECK-NEXT: OpStore %mat4 [[mat3]]
    float1x1 mat4 = float1x1(mat3);

    int scalar;
// CHECK-NEXT: [[scalar:%[0-9]+]] = OpLoad %int %scalar
// CHECK-NEXT: [[cv:%[0-9]+]] = OpConvertSToF %float [[scalar]]
// CHECK-NEXT: OpStore %mat5 [[cv]]
    float1x1 mat5 = {scalar};

// CHECK-NEXT: [[mat5:%[0-9]+]] = OpLoad %float %mat5
// CHECK-NEXT: OpStore %mat6 [[mat5]]
    float1x1 mat6 = {mat5};
}
