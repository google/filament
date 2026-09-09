// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpName %type_ByteAddressBuffer "type.ByteAddressBuffer"
// CHECK: OpName %type_RWByteAddressBuffer "type.RWByteAddressBuffer"
// CHECK: OpDecorate %_runtimearr_uint ArrayStride 4
// CHECK: OpMemberDecorate %type_ByteAddressBuffer 0 Offset 0
// CHECK: OpMemberDecorate %type_ByteAddressBuffer 0 NonWritable
// CHECK: OpDecorate %type_ByteAddressBuffer BufferBlock
// CHECK: OpMemberDecorate %type_RWByteAddressBuffer 0 Offset 0
// CHECK: OpDecorate %type_RWByteAddressBuffer BufferBlock
// CHECK: %_runtimearr_uint = OpTypeRuntimeArray %uint
// CHECK: %type_ByteAddressBuffer = OpTypeStruct %_runtimearr_uint
// CHECK: %_ptr_Uniform_type_ByteAddressBuffer = OpTypePointer Uniform %type_ByteAddressBuffer
// CHECK: %type_RWByteAddressBuffer = OpTypeStruct %_runtimearr_uint
// CHECK: %_ptr_Uniform_type_RWByteAddressBuffer = OpTypePointer Uniform %type_RWByteAddressBuffer
// CHECK: %Buffer0 = OpVariable %_ptr_Uniform_type_ByteAddressBuffer Uniform
// CHECK: %BufferArray = OpVariable %_ptr_Uniform__arr_type_ByteAddressBuffer_uint_2 Uniform
// CHECK: %BufferOut = OpVariable %_ptr_Uniform_type_RWByteAddressBuffer Uniform

ByteAddressBuffer Buffer0;
ByteAddressBuffer BufferArray[2];
RWByteAddressBuffer BufferOut;

struct MyStruct
{
	ByteAddressBuffer myBuffers[2];
};

// CHECK: %src_main = OpFunction
[numthreads(1, 1, 1)]
void main() {
// CHECK: %LocalArray = OpVariable %_ptr_Function__ptr_Uniform__arr_type_ByteAddressBuffer_uint_2 Function
// CHECK: %Local = OpVariable %_ptr_Function__ptr_Uniform_type_ByteAddressBuffer Function
// CHECK: %ms = OpVariable %_ptr_Function_MyStruct Function
  ByteAddressBuffer LocalArray[2];

// CHECK: OpStore %LocalArray %BufferArray
  LocalArray = BufferArray;

// CHECK: [[array:%[0-9]+]] = OpLoad %_ptr_Uniform__arr_type_ByteAddressBuffer_uint_2 %LocalArray
// CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_Uniform_type_ByteAddressBuffer [[array]] %int_0
// CHECK: OpStore %Local [[ac]]
  ByteAddressBuffer Local = LocalArray[0];

// CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_Function__ptr_Uniform__arr_type_ByteAddressBuffer_uint_2 %ms %int_0
// CHECK: OpStore [[ac]] %BufferArray
  MyStruct ms;
  ms.myBuffers = BufferArray;

// CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_Function__ptr_Uniform__arr_type_ByteAddressBuffer_uint_2 %ms %int_0
// CHECK: [[alias:%[0-9]+]] = OpLoad %_ptr_Uniform__arr_type_ByteAddressBuffer_uint_2 [[ac]]
// CHECK: [[ac:%[0-9]+]] = OpAccessChain %_ptr_Uniform_type_ByteAddressBuffer [[alias]] %int_0
// CHECK: OpStore %buffer [[ac]]
	ByteAddressBuffer buffer = ms.myBuffers[0];
}
