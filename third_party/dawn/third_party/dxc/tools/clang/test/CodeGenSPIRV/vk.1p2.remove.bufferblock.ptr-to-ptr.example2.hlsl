// RUN: %dxc -T cs_6_4 -E main -fspv-target-env=vulkan1.2 -fcgl  %s -spirv | FileCheck %s


// CHECK: OpDecorate %type_ByteAddressBuffer Block

ByteAddressBuffer g_byteAddressBuffer[] : register(t0, space3);
[numthreads(1,1,1)]
void main() {
// CHECK: %flat_bucket_indices = OpVariable %_ptr_Function__ptr_StorageBuffer_type_ByteAddressBuffer Function
// CHECK:             {{%[0-9]+}} = OpAccessChain %_ptr_StorageBuffer_type_ByteAddressBuffer %g_byteAddressBuffer %int_0
  ByteAddressBuffer flat_bucket_indices = g_byteAddressBuffer[0];
}
