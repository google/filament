// RUN: %dxc -T ps_6_6 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpName %type_RWByteAddressBuffer "type.RWByteAddressBuffer"
// CHECK: OpDecorate %_runtimearr_uint ArrayStride 4
// CHECK: OpMemberDecorate %type_RWByteAddressBuffer 0 Offset 0
// CHECK: OpDecorate %type_RWByteAddressBuffer BufferBlock
// CHECK: %_runtimearr_uint = OpTypeRuntimeArray %uint
// CHECK: %type_RWByteAddressBuffer = OpTypeStruct %_runtimearr_uint
// CHECK: %_ptr_Uniform_type_RWByteAddressBuffer = OpTypePointer Uniform %type_RWByteAddressBuffer
// CHECK: %buf = OpVariable %_ptr_Uniform_type_RWByteAddressBuffer Uniform
RasterizerOrderedByteAddressBuffer buf;

void main() { }
