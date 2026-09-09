// RUN: %dxc -T cs_6_6 -spirv %s | FileCheck %s

// CHECK-DAG:                                           OpCapability RuntimeDescriptorArray
// CHECK-DAG:                                           OpExtension "SPV_EXT_descriptor_indexing"
// CHECK-DAG:             [[buffer_t:%[_a-zA-Z0-9]+]] = OpTypeImage %uint Buffer 2 0 0 2 R32ui
// CHECK-DAG:          [[ra_buffer_t:%[_a-zA-Z0-9]+]] = OpTypeRuntimeArray [[buffer_t]]
// CHECK-DAG:       [[ptr_u_buffer_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[buffer_t]]
// CHECK-DAG:    [[ptr_u_ra_buffer_t:%[_a-zA-Z0-9]+]] = OpTypePointer UniformConstant [[ra_buffer_t]]

// CHECK-DAG:   [[resource_heap:%[_a-zA-Z0-9]+]] = OpVariable [[ptr_u_ra_buffer_t]] UniformConstant
// CHECK-DAG:                                      OpDecorate [[resource_heap]] DescriptorSet 0
// CHECK-DAG:                                      OpDecorate [[resource_heap]] Binding 0

[numthreads(1, 1, 1)]
void main() {
  RWBuffer<uint> buffer = ResourceDescriptorHeap[1];
// CHECK:    [[ptr:%[0-9]+]] = OpAccessChain [[ptr_u_buffer_t]] [[resource_heap]] %uint_1
// CHECK: [[buffer:%[0-9]+]] = OpLoad [[buffer_t]] [[ptr]]

  buffer[10] = 1;
// CHECK:                      OpImageWrite [[buffer]] %uint_10 %uint_1 None
}
