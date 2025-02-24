// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpDecorate %_runtimearr_v3uint ArrayStride 16
// CHECK: OpDecorate %type_StructuredBuffer_v3uint BufferBlock
// CHECK: %_runtimearr_v3uint = OpTypeRuntimeArray %v3uint
// CHECK: %type_StructuredBuffer_v3uint = OpTypeStruct %_runtimearr_v3uint
// CHECK: %_ptr_Uniform_type_StructuredBuffer_v3uint = OpTypePointer Uniform %type_StructuredBuffer_v3uint
StructuredBuffer<uint3> buffer;

void main() {
}
