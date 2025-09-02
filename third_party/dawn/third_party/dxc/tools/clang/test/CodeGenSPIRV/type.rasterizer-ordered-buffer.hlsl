// RUN: %dxc -T ps_6_6 -E main -fcgl %s -spirv | FileCheck %s --check-prefixes=CHECK,INFER
// RUN: %dxc -fspv-use-unknown-image-format -T ps_6_6 -E main -fcgl %s -spirv | FileCheck %s --check-prefixes=CHECK,UNKNOWN

// Before vulkan1.3, we should be trying to infer the image type for because
// we cannot necessarily use Unknown. However in VK1.3 and later, we can use
// Unknown.

// CHECK: OpCapability SampledBuffer
// INFER: OpCapability StorageImageExtendedFormats

// INFER: %type_buffer_image = OpTypeImage %int Buffer 2 0 0 2 R32i
// UNKNOWN: %type_buffer_image = OpTypeImage %int Buffer 2 0 0 2 Unknown
// CHECK: %_ptr_UniformConstant_type_buffer_image = OpTypePointer UniformConstant %type_buffer_image
RasterizerOrderedBuffer<int> introvbuf;
// INFER: %type_buffer_image_0 = OpTypeImage %uint Buffer 2 0 0 2 R32ui
// UNKNOWN: %type_buffer_image_0 = OpTypeImage %uint Buffer 2 0 0 2 Unknown
// CHECK: %_ptr_UniformConstant_type_buffer_image_0 = OpTypePointer UniformConstant %type_buffer_image_0
RasterizerOrderedBuffer<uint> uintrovbuf;
// INFER: %type_buffer_image_1 = OpTypeImage %float Buffer 2 0 0 2 R32f
// UNKNOWN: %type_buffer_image_1 = OpTypeImage %float Buffer 2 0 0 2 Unknown
// CHECK: %_ptr_UniformConstant_type_buffer_image_1 = OpTypePointer UniformConstant %type_buffer_image_1
RasterizerOrderedBuffer<float> floatrovbuf;

// INFER: %type_buffer_image_2 = OpTypeImage %int Buffer 2 0 0 2 Rg32i
// INFER: %_ptr_UniformConstant_type_buffer_image_2 = OpTypePointer UniformConstant %type_buffer_image_2
RasterizerOrderedBuffer<int2> int2rovbuf;
// INFER: %type_buffer_image_3 = OpTypeImage %uint Buffer 2 0 0 2 Rg32ui
// INFER: %_ptr_UniformConstant_type_buffer_image_3 = OpTypePointer UniformConstant %type_buffer_image_3
RasterizerOrderedBuffer<uint2> uint2rovbuf;
// INFER: %type_buffer_image_4 = OpTypeImage %float Buffer 2 0 0 2 Rg32f
// INFER: %_ptr_UniformConstant_type_buffer_image_4 = OpTypePointer UniformConstant %type_buffer_image_4
RasterizerOrderedBuffer<float2> float2rovbuf;

// INFER: %type_buffer_image_5 = OpTypeImage %int Buffer 2 0 0 2 Unknown
// INFER: %_ptr_UniformConstant_type_buffer_image_5 = OpTypePointer UniformConstant %type_buffer_image_5
// INFER: %type_buffer_image_6 = OpTypeImage %int Buffer 2 0 0 2 Rgba32i
// INFER: %_ptr_UniformConstant_type_buffer_image_6 = OpTypePointer UniformConstant %type_buffer_image_6
RasterizerOrderedBuffer<int3> int3rovbuf;
RasterizerOrderedBuffer<int4> int4rovbuf;
// INFER: %type_buffer_image_7 = OpTypeImage %uint Buffer 2 0 0 2 Unknown
// INFER: %_ptr_UniformConstant_type_buffer_image_7 = OpTypePointer UniformConstant %type_buffer_image_7
// INFER: %type_buffer_image_8 = OpTypeImage %uint Buffer 2 0 0 2 Rgba32ui
// INFER: %_ptr_UniformConstant_type_buffer_image_8 = OpTypePointer UniformConstant %type_buffer_image_8
RasterizerOrderedBuffer<uint3> uint3rovbuf;
RasterizerOrderedBuffer<uint4> uint4rovbuf;
// INFER: %type_buffer_image_9 = OpTypeImage %float Buffer 2 0 0 2 Unknown
// INFER: %_ptr_UniformConstant_type_buffer_image_9 = OpTypePointer UniformConstant %type_buffer_image_9
// INFER: %type_buffer_image_10 = OpTypeImage %float Buffer 2 0 0 2 Rgba32f
// INFER: %_ptr_UniformConstant_type_buffer_image_10 = OpTypePointer UniformConstant %type_buffer_image_10
RasterizerOrderedBuffer<float3> float3rovbuf;
RasterizerOrderedBuffer<float4> float4rovbuf;

// INFER: %introvbuf = OpVariable %_ptr_UniformConstant_type_buffer_image UniformConstant
// INFER: %uintrovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_0 UniformConstant
// INFER: %floatrovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_1 UniformConstant
// INFER: %int2rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_2 UniformConstant
// INFER: %uint2rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_3 UniformConstant
// INFER: %float2rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_4 UniformConstant
// INFER: %int3rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_5 UniformConstant
// INFER: %int4rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_6 UniformConstant
// INFER: %uint3rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_7 UniformConstant
// INFER: %uint4rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_8 UniformConstant
// INFER: %float3rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_9 UniformConstant
// INFER: %float4rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_10 UniformConstant

// UNKNOWN: %introvbuf = OpVariable %_ptr_UniformConstant_type_buffer_image UniformConstant
// UNKNOWN: %uintrovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_0 UniformConstant
// UNKNOWN: %floatrovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_1 UniformConstant
// UNKNOWN: %int2rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image UniformConstant
// UNKNOWN: %uint2rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_0 UniformConstant
// UNKNOWN: %float2rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_1 UniformConstant
// UNKNOWN: %int3rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image UniformConstant
// UNKNOWN: %int4rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image UniformConstant
// UNKNOWN: %uint3rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_0 UniformConstant
// UNKNOWN: %uint4rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_0 UniformConstant
// UNKNOWN: %float3rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_1 UniformConstant
// UNKNOWN: %float4rovbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_1 UniformConstant

void main() {}

