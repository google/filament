// RUN: %dxc -T lib_6_4 -fspv-target-env=vulkan1.2 -fcgl  %s -spirv | FileCheck %s

// We cannot use BufferBlock decoration for SPIR-V 1.4 or above.
// Instead, we must use Block decorated StorageBuffer Storage Class.
//
// As a result, we transform the storage class of types where needed.
//
// If a resource is used as a function parameter or function return value,
// it represents a case that requires legalization. In such cases, a
// pointer-to-pointer type may be present. We must make sure that in such cases
// the inner pointer's storage class is also updated if needed.

// CHECK: ; Version: 1.5

// CHECK: OpDecorate %type_StructuredBuffer_float Block
// CHECK: OpDecorate %type_RWStructuredBuffer_float Block

// CHECK: [[fn_type:%[0-9]+]] = OpTypeFunction %_ptr_StorageBuffer_type_StructuredBuffer_float %_ptr_Function__ptr_StorageBuffer_type_StructuredBuffer_float

// CHECK:   %gSBuffer = OpVariable %_ptr_StorageBuffer_type_StructuredBuffer_float StorageBuffer
StructuredBuffer<float> gSBuffer;
// CHECK: %gRWSBuffer = OpVariable %_ptr_StorageBuffer_type_RWStructuredBuffer_float StorageBuffer
RWStructuredBuffer<float> gRWSBuffer;

StructuredBuffer<float> foo(StructuredBuffer<float> pSBuffer);

[shader("raygeneration")]
void main() {
// CHECK: %param_var_pSBuffer = OpVariable %_ptr_Function__ptr_StorageBuffer_type_StructuredBuffer_float Function
// CHECK:                       OpStore %param_var_pSBuffer %gSBuffer
// CHECK:                       OpFunctionCall %_ptr_StorageBuffer_type_StructuredBuffer_float %foo %param_var_pSBuffer
  float a = foo(gSBuffer)[0];
}

// CHECK:      %foo = OpFunction %_ptr_StorageBuffer_type_StructuredBuffer_float None [[fn_type]]
// CHECK: %pSBuffer = OpFunctionParameter %_ptr_Function__ptr_StorageBuffer_type_StructuredBuffer_float
StructuredBuffer<float> foo(StructuredBuffer<float> pSBuffer) {
// CHECK: OpLoad %_ptr_StorageBuffer_type_StructuredBuffer_float %pSBuffer
// CHECK: OpAccessChain %_ptr_StorageBuffer_float
  float x = pSBuffer[0];
// CHECK: [[buf:%[0-9]+]] = OpLoad %_ptr_StorageBuffer_type_StructuredBuffer_float %pSBuffer
// CHECK: OpReturnValue [[buf]]
  return pSBuffer;
}

