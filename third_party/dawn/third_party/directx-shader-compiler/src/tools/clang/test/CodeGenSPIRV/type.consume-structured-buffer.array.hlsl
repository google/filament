// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct T {
  float  a;
  float3 b;
};

// CHECK: OpDecorate %myConsumeStructuredBuffer DescriptorSet 0
// CHECK: OpDecorate %myConsumeStructuredBuffer Binding 0
// CHECK: OpDecorate %counter_var_myConsumeStructuredBuffer DescriptorSet 0
// CHECK: OpDecorate %counter_var_myConsumeStructuredBuffer Binding 1

// CHECK: %counter_var_myConsumeStructuredBuffer = OpVariable %_ptr_Uniform__arr_type_ACSBuffer_counter_uint_2 Uniform
// CHECK: %myConsumeStructuredBuffer = OpVariable %_ptr_Uniform__arr_type_ConsumeStructuredBuffer_T_uint_2 Uniform
ConsumeStructuredBuffer<T> myConsumeStructuredBuffer[2];

void main() {}

