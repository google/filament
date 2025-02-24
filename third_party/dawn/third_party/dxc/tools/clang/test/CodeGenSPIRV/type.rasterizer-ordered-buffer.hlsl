// RUN: %dxc -T ps_6_6 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpCapability SampledBuffer
// CHECK: OpCapability StorageImageExtendedFormats

// CHECK: %type_buffer_image = OpTypeImage %int Buffer 2 0 0 2 R32i
// CHECK: %_ptr_UniformConstant_type_buffer_image = OpTypePointer UniformConstant %type_buffer_image
RasterizerOrderedBuffer<int> introvbuf;
// CHECK: %type_buffer_image_0 = OpTypeImage %uint Buffer 2 0 0 2 R32ui
// CHECK: %_ptr_UniformConstant_type_buffer_image_0 = OpTypePointer UniformConstant %type_buffer_image_0
RasterizerOrderedBuffer<uint> uintrovbuf;
// CHECK: %type_buffer_image_1 = OpTypeImage %float Buffer 2 0 0 2 R32f
// CHECK: %_ptr_UniformConstant_type_buffer_image_1 = OpTypePointer UniformConstant %type_buffer_image_1
RasterizerOrderedBuffer<float> floatrovbuf;

// CHECK: %type_buffer_image_2 = OpTypeImage %int Buffer 2 0 0 2 Rg32i
// CHECK: %_ptr_UniformConstant_type_buffer_image_2 = OpTypePointer UniformConstant %type_buffer_image_2
RasterizerOrderedBuffer<int2> int2rovbuf;
// CHECK: %type_buffer_image_3 = OpTypeImage %uint Buffer 2 0 0 2 Rg32ui
// CHECK: %_ptr_UniformConstant_type_buffer_image_3 = OpTypePointer UniformConstant %type_buffer_image_3
RasterizerOrderedBuffer<uint2> uint2rovbuf;
// CHECK: %type_buffer_image_4 = OpTypeImage %float Buffer 2 0 0 2 Rg32f
// CHECK: %_ptr_UniformConstant_type_buffer_image_4 = OpTypePointer UniformConstant %type_buffer_image_4
RasterizerOrderedBuffer<float2> float2rovbuf;

// CHECK: %type_buffer_image_5 = OpTypeImage %int Buffer 2 0 0 2 Unknown
// CHECK: %_ptr_UniformConstant_type_buffer_image_5 = OpTypePointer UniformConstant %type_buffer_image_5
// CHECK: %type_buffer_image_6 = OpTypeImage %int Buffer 2 0 0 2 Rgba32i
// CHECK: %_ptr_UniformConstant_type_buffer_image_6 = OpTypePointer UniformConstant %type_buffer_image_6
RasterizerOrderedBuffer<int3> int3rovbuf;
RasterizerOrderedBuffer<int4> int4rovbuf;
// CHECK: %type_buffer_image_7 = OpTypeImage %uint Buffer 2 0 0 2 Unknown
// CHECK: %_ptr_UniformConstant_type_buffer_image_7 = OpTypePointer UniformConstant %type_buffer_image_7
// CHECK: %type_buffer_image_8 = OpTypeImage %uint Buffer 2 0 0 2 Rgba32ui
// CHECK: %_ptr_UniformConstant_type_buffer_image_8 = OpTypePointer UniformConstant %type_buffer_image_8
RasterizerOrderedBuffer<uint3> uint3rovbuf;
RasterizerOrderedBuffer<uint4> uint4rovbuf;
// CHECK: %type_buffer_image_9 = OpTypeImage %float Buffer 2 0 0 2 Unknown
// CHECK: %_ptr_UniformConstant_type_buffer_image_9 = OpTypePointer UniformConstant %type_buffer_image_9
// CHECK: %type_buffer_image_10 = OpTypeImage %float Buffer 2 0 0 2 Rgba32f
// CHECK: %_ptr_UniformConstant_type_buffer_image_10 = OpTypePointer UniformConstant %type_buffer_image_10
RasterizerOrderedBuffer<float3> float3rovbuf;
RasterizerOrderedBuffer<float4> float4rovbuf;

// CHECK: %introvbuf = OpVariable %_ptr_UniformConstant_type_buffer_image UniformConstant
// CHECK: %uintrovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_0 UniformConstant
// CHECK: %floatrovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_1 UniformConstant
// CHECK: %int2rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_2 UniformConstant
// CHECK: %uint2rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_3 UniformConstant
// CHECK: %float2rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_4 UniformConstant
// CHECK: %int3rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_5 UniformConstant
// CHECK: %int4rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_6 UniformConstant
// CHECK: %uint3rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_7 UniformConstant
// CHECK: %uint4rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_8 UniformConstant
// CHECK: %float3rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_9 UniformConstant
// CHECK: %float4rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_10 UniformConstant

void main() {}

