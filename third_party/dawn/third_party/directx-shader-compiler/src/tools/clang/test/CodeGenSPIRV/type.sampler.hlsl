// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: %type_sampler = OpTypeSampler
// CHECK: %_ptr_UniformConstant_type_sampler = OpTypePointer UniformConstant %type_sampler

// CHECK: %s1 = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
SamplerState           s1 : register(s1);
// CHECK: %s2 = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
SamplerComparisonState s2 : register(s2);
// CHECK: %s3 = OpVariable %_ptr_UniformConstant_type_sampler UniformConstant
sampler                s3 : register(s3);

void main() {
// CHECK-LABEL: %main = OpFunction
}
