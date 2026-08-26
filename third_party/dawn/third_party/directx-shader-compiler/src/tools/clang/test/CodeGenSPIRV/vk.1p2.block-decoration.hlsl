// RUN: %dxc -T cs_6_0 -E main -fspv-target-env=vulkan1.2 -fcgl  %s -spirv | FileCheck %s

// We cannot use BufferBlock decoration for SPIR-V 1.4 or above.
// Instead, we must use Block decorated StorageBuffer Storage Class.

// CHECK: ; Version: 1.5

// CHECK: OpDecorate %type_ByteAddressBuffer Block
// CHECK: OpDecorate %type_RWByteAddressBuffer Block
// CHECK: OpDecorate %type_TextureBuffer_S Block
// CHECK: OpDecorate %type_StructuredBuffer_v3uint Block

// CHECK: %_ptr_StorageBuffer_type_ByteAddressBuffer = OpTypePointer StorageBuffer %type_ByteAddressBuffer
// CHECK: %_ptr_StorageBuffer_type_RWByteAddressBuffer = OpTypePointer StorageBuffer %type_RWByteAddressBuffer
// CHECK: %_ptr_StorageBuffer_type_TextureBuffer_S = OpTypePointer StorageBuffer %type_TextureBuffer_S
// CHECK: %_ptr_StorageBuffer_type_StructuredBuffer_v3uint = OpTypePointer StorageBuffer %type_StructuredBuffer_v3uint
// CHECK: %counter_var_rwsb = OpVariable %_ptr_StorageBuffer_type_ACSBuffer_counter StorageBuffer
// CHECK: %rwsb = OpVariable %_ptr_StorageBuffer_type_RWStructuredBuffer_S StorageBuffer
struct S {
  float4 f[5];
};

ByteAddressBuffer bab;
RWByteAddressBuffer rwbab;
TextureBuffer<S> tb;
StructuredBuffer<uint3> sb;
RWStructuredBuffer<S> rwsb;

[numthreads(1, 1, 1)]
void main() {
// CHECK:   [[base:%[0-9]+]] = OpAccessChain %_ptr_StorageBuffer__arr_v4float_uint_5 %rwsb %int_0 %uint_2 %int_0
// CHECK:   [[vec:%[0-9]+]] = OpAccessChain %_ptr_StorageBuffer_v4float [[base]] %int_1
// CHECK:       {{%[0-9]+}} = OpAccessChain %_ptr_StorageBuffer_float [[vec]] %int_0
  float a = rwsb[2].f[1].x;

// CHECK: [[counterPtr:%[0-9]+]] = OpAccessChain %_ptr_StorageBuffer_int %counter_var_rwsb %uint_0
// CHECK:            {{%[0-9]+}} = OpAtomicIAdd %int [[counterPtr]] %uint_1 %uint_0 %int_1
  rwsb.IncrementCounter();
}
