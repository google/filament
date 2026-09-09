// RUN: %dxc -T cs_6_6 -spirv %s | FileCheck %s

// CHECK-DAG:                                        OpCapability RuntimeDescriptorArray
// CHECK-DAG:                                        OpExtension "SPV_EXT_descriptor_indexing"

// CHECK-DAG:   [[heap_bytebuffer:%[_a-zA-Z0-9]+]] = OpVariable %_ptr_Uniform__runtimearr_type_ByteAddressBuffer Uniform
// CHECK-DAG:                                        OpDecorate [[heap_bytebuffer]] DescriptorSet 0
// CHECK-DAG:                                        OpDecorate [[heap_bytebuffer]] Binding 0

// CHECK-DAG: [[heap_rwbytebuffer:%[_a-zA-Z0-9]+]] = OpVariable %_ptr_Uniform__runtimearr_type_RWByteAddressBuffer Uniform
// CHECK-DAG:                                        OpDecorate [[heap_rwbytebuffer]] DescriptorSet 0
// CHECK-DAG:                                        OpDecorate [[heap_rwbytebuffer]] Binding 0

[numthreads(1, 1, 1)]
void main() {
  RWByteAddressBuffer output = ResourceDescriptorHeap[0];
  ByteAddressBuffer input = ResourceDescriptorHeap[1];

// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[heap_bytebuffer]] %uint_1 %uint_0 %uint_2
// CHECK: [[tmp:%[0-9]+]] = OpLoad %uint [[ptr]]
// CHECK: [[ptr:%[0-9]+]] = OpAccessChain %_ptr_Uniform_uint [[heap_rwbytebuffer]] %uint_0 %uint_0 %uint_2
// CHECK:                   OpStore [[ptr]] [[tmp]]
  output.Store(11, input.Load(10));
}
