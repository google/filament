// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

struct T {
  float  a;
  float3 b;
};

// CHECK: OpDecorate %myAppendStructuredBuffer DescriptorSet 0
// CHECK: OpDecorate %myAppendStructuredBuffer Binding 0
// CHECK: OpDecorate %counter_var_myAppendStructuredBuffer DescriptorSet 0
// CHECK: OpDecorate %counter_var_myAppendStructuredBuffer Binding 1

// CHECK: %counter_var_myAppendStructuredBuffer = OpVariable %_ptr_Uniform__arr_type_ACSBuffer_counter_uint_5 Uniform
// CHECK: %myAppendStructuredBuffer = OpVariable %_ptr_Uniform__arr_type_AppendStructuredBuffer_T_uint_5 Uniform
AppendStructuredBuffer<T> myAppendStructuredBuffer[5];

void main() {}

