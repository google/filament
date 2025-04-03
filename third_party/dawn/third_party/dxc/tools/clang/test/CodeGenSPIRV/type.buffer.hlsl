// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability SampledBuffer
// CHECK: OpCapability StorageImageExtendedFormats

// CHECK: %type_buffer_image = OpTypeImage %int Buffer 2 0 0 1 R32i
// CHECK: %_ptr_UniformConstant_type_buffer_image = OpTypePointer UniformConstant %type_buffer_image
Buffer<int> intbuf;
// CHECK: %type_buffer_image_0 = OpTypeImage %uint Buffer 2 0 0 1 R32ui
// CHECK: %_ptr_UniformConstant_type_buffer_image_0 = OpTypePointer UniformConstant %type_buffer_image_0
Buffer<uint> uintbuf;
// CHECK: %type_buffer_image_1 = OpTypeImage %float Buffer 2 0 0 1 R32f
// CHECK: %_ptr_UniformConstant_type_buffer_image_1 = OpTypePointer UniformConstant %type_buffer_image_1
Buffer<float> floatbuf;

// CHECK: %type_buffer_image_2 = OpTypeImage %int Buffer 2 0 0 2 R32i
// CHECK: %_ptr_UniformConstant_type_buffer_image_2 = OpTypePointer UniformConstant %type_buffer_image_2
RWBuffer<int> intrwbuf;
// CHECK: %type_buffer_image_3 = OpTypeImage %uint Buffer 2 0 0 2 R32ui
// CHECK: %_ptr_UniformConstant_type_buffer_image_3 = OpTypePointer UniformConstant %type_buffer_image_3
RWBuffer<uint> uintrwbuf;
// CHECK: %type_buffer_image_4 = OpTypeImage %float Buffer 2 0 0 2 R32f
// CHECK: %_ptr_UniformConstant_type_buffer_image_4 = OpTypePointer UniformConstant %type_buffer_image_4
RWBuffer<float> floatrwbuf;

// CHECK: %type_buffer_image_5 = OpTypeImage %int Buffer 2 0 0 1 Rg32i
// CHECK: %_ptr_UniformConstant_type_buffer_image_5 = OpTypePointer UniformConstant %type_buffer_image_5
Buffer<int2> int2buf;
// CHECK: %type_buffer_image_6 = OpTypeImage %uint Buffer 2 0 0 1 Rg32ui
// CHECK: %_ptr_UniformConstant_type_buffer_image_6 = OpTypePointer UniformConstant %type_buffer_image_6
Buffer<uint2> uint2buf;
// CHECK: %type_buffer_image_7 = OpTypeImage %float Buffer 2 0 0 1 Rg32f
// CHECK: %_ptr_UniformConstant_type_buffer_image_7 = OpTypePointer UniformConstant %type_buffer_image_7
Buffer<float2> float2buf;

// CHECK: %type_buffer_image_8 = OpTypeImage %int Buffer 2 0 0 2 Rg32i
// CHECK: %_ptr_UniformConstant_type_buffer_image_8 = OpTypePointer UniformConstant %type_buffer_image_8
RWBuffer<int2> int2rwbuf;
// CHECK: %type_buffer_image_9 = OpTypeImage %uint Buffer 2 0 0 2 Rg32ui
// CHECK: %_ptr_UniformConstant_type_buffer_image_9 = OpTypePointer UniformConstant %type_buffer_image_9
RWBuffer<uint2> uint2rwbuf;
// CHECK: %type_buffer_image_10 = OpTypeImage %float Buffer 2 0 0 2 Rg32f
// CHECK: %_ptr_UniformConstant_type_buffer_image_10 = OpTypePointer UniformConstant %type_buffer_image_10
RWBuffer<float2> float2rwbuf;

// CHECK: %type_buffer_image_11 = OpTypeImage %int Buffer 2 0 0 1 Unknown
// CHECK: %_ptr_UniformConstant_type_buffer_image_11 = OpTypePointer UniformConstant %type_buffer_image_11
// CHECK: %type_buffer_image_12 = OpTypeImage %int Buffer 2 0 0 1 Rgba32i
// CHECK: %_ptr_UniformConstant_type_buffer_image_12 = OpTypePointer UniformConstant %type_buffer_image_12
Buffer<int3> int3buf;
Buffer<int4> int4buf;
// CHECK: %type_buffer_image_13 = OpTypeImage %uint Buffer 2 0 0 1 Unknown
// CHECK: %_ptr_UniformConstant_type_buffer_image_13 = OpTypePointer UniformConstant %type_buffer_image_13
// CHECK: %type_buffer_image_14 = OpTypeImage %uint Buffer 2 0 0 1 Rgba32ui
// CHECK: %_ptr_UniformConstant_type_buffer_image_14 = OpTypePointer UniformConstant %type_buffer_image_14
Buffer<uint3> uint3buf;
Buffer<uint4> uint4buf;
// CHECK: %type_buffer_image_15 = OpTypeImage %float Buffer 2 0 0 1 Unknown
// CHECK: %_ptr_UniformConstant_type_buffer_image_15 = OpTypePointer UniformConstant %type_buffer_image_15
// CHECK: %type_buffer_image_16 = OpTypeImage %float Buffer 2 0 0 1 Rgba32f
// CHECK: %_ptr_UniformConstant_type_buffer_image_16 = OpTypePointer UniformConstant %type_buffer_image_16
Buffer<float3> float3buf;
Buffer<float4> float4buf;

// CHECK: %type_buffer_image_17 = OpTypeImage %int Buffer 2 0 0 2 Unknown
// CHECK: %_ptr_UniformConstant_type_buffer_image_17 = OpTypePointer UniformConstant %type_buffer_image_17
// CHECK: %type_buffer_image_18 = OpTypeImage %int Buffer 2 0 0 2 Rgba32i
// CHECK: %_ptr_UniformConstant_type_buffer_image_18 = OpTypePointer UniformConstant %type_buffer_image_18
RWBuffer<int3> int3rwbuf;
RWBuffer<int4> int4rwbuf;
// CHECK: %type_buffer_image_19 = OpTypeImage %uint Buffer 2 0 0 2 Unknown
// CHECK: %_ptr_UniformConstant_type_buffer_image_19 = OpTypePointer UniformConstant %type_buffer_image_19
// CHECK: %type_buffer_image_20 = OpTypeImage %uint Buffer 2 0 0 2 Rgba32ui
// CHECK: %_ptr_UniformConstant_type_buffer_image_20 = OpTypePointer UniformConstant %type_buffer_image_20
RWBuffer<uint3> uint3rwbuf;
RWBuffer<uint4> uint4rwbuf;
// CHECK: %type_buffer_image_21 = OpTypeImage %float Buffer 2 0 0 2 Unknown
// CHECK: %_ptr_UniformConstant_type_buffer_image_21 = OpTypePointer UniformConstant %type_buffer_image_21
// CHECK: %type_buffer_image_22 = OpTypeImage %float Buffer 2 0 0 2 Rgba32f
// CHECK: %_ptr_UniformConstant_type_buffer_image_22 = OpTypePointer UniformConstant %type_buffer_image_22
RWBuffer<float3> float3rwbuf;
RWBuffer<float4> float4rwbuf;

// CHECK: %intbuf = OpVariable %_ptr_UniformConstant_type_buffer_image UniformConstant
// CHECK: %uintbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_0 UniformConstant
// CHECK: %floatbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_1 UniformConstant
// CHECK: %intrwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_2 UniformConstant
// CHECK: %uintrwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_3 UniformConstant
// CHECK: %floatrwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_4 UniformConstant
// CHECK: %int2buf = OpVariable %_ptr_UniformConstant_type_buffer_image_5 UniformConstant
// CHECK: %uint2buf = OpVariable %_ptr_UniformConstant_type_buffer_image_6 UniformConstant
// CHECK: %float2buf = OpVariable %_ptr_UniformConstant_type_buffer_image_7 UniformConstant
// CHECK: %int2rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_8 UniformConstant
// CHECK: %uint2rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_9 UniformConstant
// CHECK: %float2rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_10 UniformConstant
// CHECK: %int3buf = OpVariable %_ptr_UniformConstant_type_buffer_image_11 UniformConstant
// CHECK: %int4buf = OpVariable %_ptr_UniformConstant_type_buffer_image_12 UniformConstant
// CHECK: %uint3buf = OpVariable %_ptr_UniformConstant_type_buffer_image_13 UniformConstant
// CHECK: %uint4buf = OpVariable %_ptr_UniformConstant_type_buffer_image_14 UniformConstant
// CHECK: %float3buf = OpVariable %_ptr_UniformConstant_type_buffer_image_15 UniformConstant
// CHECK: %float4buf = OpVariable %_ptr_UniformConstant_type_buffer_image_16 UniformConstant
// CHECK: %int3rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_17 UniformConstant
// CHECK: %int4rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_18 UniformConstant
// CHECK: %uint3rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_19 UniformConstant
// CHECK: %uint4rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_20 UniformConstant
// CHECK: %float3rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_21 UniformConstant
// CHECK: %float4rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_22 UniformConstant

void main() {}
