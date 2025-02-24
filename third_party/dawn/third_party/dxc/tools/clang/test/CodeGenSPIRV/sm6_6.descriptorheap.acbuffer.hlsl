// RUN: %dxc -T cs_6_6 -spirv %s | FileCheck %s

// CHECK:   OpDecorate [[resource_heap_cbuffer:%[_a-zA-Z0-9]+]] DescriptorSet 0
// CHECK:   OpDecorate [[resource_heap_cbuffer]] Binding 0
// CHECK:   OpDecorate [[resource_heap_cbuffer_counter:%[_a-zA-Z0-9]+]] DescriptorSet 0
// CHECK:   OpDecorate [[resource_heap_cbuffer_counter]] Binding 1

// CHECK:   OpDecorate [[resource_heap_abuffer:%[_a-zA-Z0-9]+]] DescriptorSet 0
// CHECK:   OpDecorate [[resource_heap_abuffer]] Binding 0
// CHECK:   OpDecorate [[resource_heap_abuffer_counter:%[_a-zA-Z0-9]+]] DescriptorSet 0
// CHECK:   OpDecorate [[resource_heap_abuffer_counter]] Binding 1
// CHECK-NOT: OpDecorate %_runtimearr_type_ACSBuffer_counter ArrayStride
// CHECK:   OpDecorate %type_ACSBuffer_counter BufferBlock
// CHECK-NOT: OpDecorate %_runtimearr_type_ACSBuffer_counter ArrayStride


// CHECK-DAG:           [[ra_uint_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray %uint
// CHECK-DAG:            [[ra_int_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray %int

// CHECK-DAG:           [[abuffer_t:%[_a-zA-Z0-9]+]] = OpTypeStruct [[ra_uint_t]]
// CHECK-DAG:           [[cbuffer_t:%[_a-zA-Z0-9]+]] = OpTypeStruct [[ra_int_t]]

// CHECK-DAG:          [[ra_abuffer_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray [[abuffer_t]]
// CHECK-DAG:    [[ptr_u_ra_abuffer_t:%[_a-zA-Z0-9]+]] = OpTypePointer Uniform [[ra_abuffer_t]]

// CHECK-DAG:          [[ra_cbuffer_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray [[cbuffer_t]]
// CHECK-DAG:    [[ptr_u_ra_cbuffer_t:%[_a-zA-Z0-9]+]] = OpTypePointer Uniform [[ra_cbuffer_t]]

// CHECK-DAG:             [[counter_t:%[_a-zA-Z0-9]+]] = OpTypeStruct %int
// CHECK-DAG:          [[ra_counter_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray [[counter_t]]
// CHECK-DAG:    [[ptr_u_ra_counter_t:%[_a-zA-Z0-9]+]] = OpTypePointer Uniform [[ra_counter_t]]

// CHECK-DAG:         [[resource_heap_cbuffer]] = OpVariable [[ptr_u_ra_cbuffer_t]] Uniform
// CHECK-DAG: [[resource_heap_cbuffer_counter]] = OpVariable [[ptr_u_ra_counter_t]] Uniform
// CHECK-DAG:         [[resource_heap_abuffer]] = OpVariable [[ptr_u_ra_abuffer_t]] Uniform
// CHECK-DAG: [[resource_heap_abuffer_counter]] = OpVariable [[ptr_u_ra_counter_t]] Uniform

[numthreads(1, 1, 1)]
void main() {
  ConsumeStructuredBuffer<int> input = ResourceDescriptorHeap[0];
  AppendStructuredBuffer<uint> output = ResourceDescriptorHeap[1];
  output.Append(input.Consume());

// CHECK:  [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int [[resource_heap_abuffer_counter]] %uint_1 %uint_0
// CHECK:  [[idx:%[0-9]+]] = OpAtomicIAdd %int [[ptr]] %uint_1 %uint_0 %int_1
// CHECK: [[dptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[resource_heap_abuffer]] %uint_1 %uint_0 [[idx]]

// CHECK:  [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int [[resource_heap_cbuffer_counter]] %uint_0 %uint_0
// CHECK:  [[tmp:%[0-9]+]] = OpAtomicISub %int [[ptr]] %uint_1 %uint_0 %int_1
// CHECK:  [[idx:%[0-9]+]] = OpISub %int [[tmp]] %int_1
// CHECK: [[sptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_int [[resource_heap_cbuffer]] %uint_0 %uint_0 [[idx]]

// CHECK: [[a:%[0-9]+]] = OpLoad %int [[sptr]]
// CHECK: [[b:%[0-9]+]] = OpBitcast %uint [[a]]
// CHECK:                 OpStore [[dptr]] [[b]]
}
