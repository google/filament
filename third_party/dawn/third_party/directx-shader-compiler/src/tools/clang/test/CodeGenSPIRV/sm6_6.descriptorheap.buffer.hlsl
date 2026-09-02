// RUN: %dxc -T cs_6_6 -spirv %s | FileCheck %s

// CHECK-DAG:                                           OpCapability RuntimeDescriptorArray
// CHECK-DAG:                                           OpExtension "SPV_EXT_descriptor_indexing"

// CHECK-DAG:           [[robuffer_t:%[_a-zA-Z0-9]+]] = OpTypeImage %uint Buffer 2 0 0 1 R32ui
// CHECK-DAG:           [[rwbuffer_t:%[_a-zA-Z0-9]+]] = OpTypeImage %uint Buffer 2 0 0 2 R32ui

// CHECK-DAG:          [[ra_robuffer_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray [[robuffer_t]]
// CHECK-DAG:       [[ptr_u_robuffer_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[robuffer_t]]
// CHECK-DAG:    [[ptr_u_ra_robuffer_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[ra_robuffer_t]]

// CHECK-DAG:          [[ra_rwbuffer_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray [[rwbuffer_t]]
// CHECK-DAG:       [[ptr_u_rwbuffer_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[rwbuffer_t]]
// CHECK-DAG:    [[ptr_u_ra_rwbuffer_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[ra_rwbuffer_t]]

// CHECK-DAG:   [[resource_heap_ro:%[_a-zA-Z0-9]+]] = OpVariable [[ptr_u_ra_robuffer_t]] UniformConstant
// CHECK-DAG:                                         OpDecorate [[resource_heap_ro]] DescriptorSet 0
// CHECK-DAG:                                         OpDecorate [[resource_heap_ro]] Binding 0
// CHECK-DAG:   [[resource_heap_rw:%[_a-zA-Z0-9]+]] = OpVariable [[ptr_u_ra_rwbuffer_t]] UniformConstant
// CHECK-DAG:                                         OpDecorate [[resource_heap_rw]] DescriptorSet 0
// CHECK-DAG:                                         OpDecorate [[resource_heap_rw]] Binding 0

[numthreads(1, 1, 1)]
void main() {
  Buffer<uint> input = ResourceDescriptorHeap[0];
// CHECK:    [[ptr:%[0-9]+]] = OpAccessChain [[ptr_u_robuffer_t]] [[resource_heap_ro]] %uint_0
// CHECK: [[input:%[0-9]+]] = OpLoad [[robuffer_t]] [[ptr]]

  RWBuffer<uint> output = ResourceDescriptorHeap[1];
// CHECK:    [[ptr:%[0-9]+]] = OpAccessChain [[ptr_u_rwbuffer_t]] [[resource_heap_rw]] %uint_1
// CHECK: [[output:%[0-9]+]] = OpLoad [[rwbuffer_t]] [[ptr]]

  output[10] = input[11];
// CHECK:    [[tmp:%[0-9]+]] = OpImageFetch %v4uint [[input]] %uint_11 None
// CHECK:    [[val:%[0-9]+]] = OpCompositeExtract %uint [[tmp]] 0
// CHECK:                      OpImageWrite [[output]] %uint_10 [[val]] None
}
