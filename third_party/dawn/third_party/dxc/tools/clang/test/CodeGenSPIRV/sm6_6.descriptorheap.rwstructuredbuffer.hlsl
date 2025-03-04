// RUN: %dxc -T cs_6_6 -O3 -spirv %s | FileCheck %s

// CHECK-DAG: OpCapability RuntimeDescriptorArray
// CHECK-DAG: OpExtension "SPV_EXT_descriptor_indexing"
// CHECK:     OpDecorate [[resource_heap_ro:%[_a-zA-Z0-9]+]] DescriptorSet 0
// CHECK:     OpDecorate [[resource_heap_ro]] Binding 0
// CHECK:     OpDecorate [[resource_heap_rw:%[_a-zA-Z0-9]+]] DescriptorSet 0
// CHECK:     OpDecorate [[resource_heap_rw]] Binding 0

// CHECK:                     [[ra_uint_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray %uint
// CHECK:            [[structuredbuffer_t:%[_a-zA-Z0-9]+]] = OpTypeStruct [[ra_uint_t]]
// CHECK:         [[ra_structuredbuffer_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray [[structuredbuffer_t]]
// CHECK:   [[ptr_u_ra_structuredbuffer_t:%[_a-zA-Z0-9]+]] = OpTypePointer Uniform [[ra_structuredbuffer_t]]

// CHECK:          [[rwstructuredbuffer_t:%[_a-zA-Z0-9]+]] = OpTypeStruct [[ra_uint_t]]
// CHECK:       [[ra_rwstructuredbuffer_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray [[rwstructuredbuffer_t]]
// CHECK: [[ptr_u_ra_rwstructuredbuffer_t:%[_a-zA-Z0-9]+]] = OpTypePointer Uniform [[ra_rwstructuredbuffer_t]]

// CHECK:   [[resource_heap_ro]] = OpVariable [[ptr_u_ra_structuredbuffer_t]] Uniform
// CHECK:   [[resource_heap_rw]] = OpVariable [[ptr_u_ra_rwstructuredbuffer_t]] Uniform

[numthreads(1, 1, 1)]
void main() {
  StructuredBuffer<uint> input = ResourceDescriptorHeap[0];
  RWStructuredBuffer<uint> output = ResourceDescriptorHeap[1];
  output[10] = input[11];

// CHECK-DAG: [[ptr_src:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[resource_heap_ro]] %uint_0 %int_0 %uint_11
// CHECK-DAG: [[ptr_dst:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[resource_heap_rw]] %uint_1 %int_0 %uint_10
// CHECK-DAG:     [[src:%[0-9]+]] = OpLoad %uint [[ptr_src]]
// CHECK:                           OpStore [[ptr_dst]] [[src]]
}
