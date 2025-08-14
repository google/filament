// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s --check-prefixes=CHECK,INFER
// RUN: %dxc -fspv-use-unknown-image-format -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s --check-prefixes=CHECK,UNKNOWN

// CHECK: OpCapability SampledBuffer
// INFER: OpCapability StorageImageExtendedFormats

// INFER: %type_buffer_image = OpTypeImage %int Buffer 2 0 0 1 R32i
// UNKNOWN: %type_buffer_image = OpTypeImage %int Buffer 2 0 0 1 Unknown
// CHECK: %_ptr_UniformConstant_type_buffer_image = OpTypePointer UniformConstant %type_buffer_image
Buffer<int> intbuf;
// INFER: %type_buffer_image_0 = OpTypeImage %uint Buffer 2 0 0 1 R32ui
// UNKNOWN: %type_buffer_image_0 = OpTypeImage %uint Buffer 2 0 0 1 Unknown
// CHECK: %_ptr_UniformConstant_type_buffer_image_0 = OpTypePointer UniformConstant %type_buffer_image_0
Buffer<uint> uintbuf;
// INFER: %type_buffer_image_1 = OpTypeImage %float Buffer 2 0 0 1 R32f
// UNKNOWN: %type_buffer_image_1 = OpTypeImage %float Buffer 2 0 0 1 Unknown
// CHECK: %_ptr_UniformConstant_type_buffer_image_1 = OpTypePointer UniformConstant %type_buffer_image_1
Buffer<float> floatbuf;

// INFER: %type_buffer_image_2 = OpTypeImage %int Buffer 2 0 0 2 R32i
// UNKNOWN: %type_buffer_image_2 = OpTypeImage %int Buffer 2 0 0 2 Unknown
// CHECK: %_ptr_UniformConstant_type_buffer_image_2 = OpTypePointer UniformConstant %type_buffer_image_2
RWBuffer<int> intrwbuf;
// INFER: %type_buffer_image_3 = OpTypeImage %uint Buffer 2 0 0 2 R32ui
// UNKNOWN: %type_buffer_image_3 = OpTypeImage %uint Buffer 2 0 0 2 Unknown
// CHECK: %_ptr_UniformConstant_type_buffer_image_3 = OpTypePointer UniformConstant %type_buffer_image_3
RWBuffer<uint> uintrwbuf;
// INFER: %type_buffer_image_4 = OpTypeImage %float Buffer 2 0 0 2 R32f
// UNKNOWN: %type_buffer_image_4 = OpTypeImage %float Buffer 2 0 0 2 Unknown
// CHECK: %_ptr_UniformConstant_type_buffer_image_4 = OpTypePointer UniformConstant %type_buffer_image_4
RWBuffer<float> floatrwbuf;

// If the `Unkonwn image format is used, then the images below will reuse the types above.
// UNKNOWN-NOT: OpTypeImage

// INFER: %type_buffer_image_5 = OpTypeImage %int Buffer 2 0 0 1 Rg32i
// INFER: %_ptr_UniformConstant_type_buffer_image_5 = OpTypePointer UniformConstant %type_buffer_image_5
Buffer<int2> int2buf;
// INFER: %type_buffer_image_6 = OpTypeImage %uint Buffer 2 0 0 1 Rg32ui
// INFER: %_ptr_UniformConstant_type_buffer_image_6 = OpTypePointer UniformConstant %type_buffer_image_6
Buffer<uint2> uint2buf;
// INFER: %type_buffer_image_7 = OpTypeImage %float Buffer 2 0 0 1 Rg32f
// INFER: %_ptr_UniformConstant_type_buffer_image_7 = OpTypePointer UniformConstant %type_buffer_image_7
Buffer<float2> float2buf;

// INFER: %type_buffer_image_8 = OpTypeImage %int Buffer 2 0 0 2 Rg32i
// INFER: %_ptr_UniformConstant_type_buffer_image_8 = OpTypePointer UniformConstant %type_buffer_image_8
RWBuffer<int2> int2rwbuf;
// INFER: %type_buffer_image_9 = OpTypeImage %uint Buffer 2 0 0 2 Rg32ui
// INFER: %_ptr_UniformConstant_type_buffer_image_9 = OpTypePointer UniformConstant %type_buffer_image_9
RWBuffer<uint2> uint2rwbuf;
// INFER: %type_buffer_image_10 = OpTypeImage %float Buffer 2 0 0 2 Rg32f
// INFER: %_ptr_UniformConstant_type_buffer_image_10 = OpTypePointer UniformConstant %type_buffer_image_10
RWBuffer<float2> float2rwbuf;

// INFER: %type_buffer_image_11 = OpTypeImage %int Buffer 2 0 0 1 Unknown
// INFER: %_ptr_UniformConstant_type_buffer_image_11 = OpTypePointer UniformConstant %type_buffer_image_11
// INFER: %type_buffer_image_12 = OpTypeImage %int Buffer 2 0 0 1 Rgba32i
// INFER: %_ptr_UniformConstant_type_buffer_image_12 = OpTypePointer UniformConstant %type_buffer_image_12
Buffer<int3> int3buf;
Buffer<int4> int4buf;
// INFER: %type_buffer_image_13 = OpTypeImage %uint Buffer 2 0 0 1 Unknown
// INFER: %_ptr_UniformConstant_type_buffer_image_13 = OpTypePointer UniformConstant %type_buffer_image_13
// INFER: %type_buffer_image_14 = OpTypeImage %uint Buffer 2 0 0 1 Rgba32ui
// INFER: %_ptr_UniformConstant_type_buffer_image_14 = OpTypePointer UniformConstant %type_buffer_image_14
Buffer<uint3> uint3buf;
Buffer<uint4> uint4buf;
// INFER: %type_buffer_image_15 = OpTypeImage %float Buffer 2 0 0 1 Unknown
// INFER: %_ptr_UniformConstant_type_buffer_image_15 = OpTypePointer UniformConstant %type_buffer_image_15
// INFER: %type_buffer_image_16 = OpTypeImage %float Buffer 2 0 0 1 Rgba32f
// INFER: %_ptr_UniformConstant_type_buffer_image_16 = OpTypePointer UniformConstant %type_buffer_image_16
Buffer<float3> float3buf;
Buffer<float4> float4buf;

// INFER: %type_buffer_image_17 = OpTypeImage %int Buffer 2 0 0 2 Unknown
// INFER: %_ptr_UniformConstant_type_buffer_image_17 = OpTypePointer UniformConstant %type_buffer_image_17
// INFER: %type_buffer_image_18 = OpTypeImage %int Buffer 2 0 0 2 Rgba32i
// INFER: %_ptr_UniformConstant_type_buffer_image_18 = OpTypePointer UniformConstant %type_buffer_image_18
RWBuffer<int3> int3rwbuf;
RWBuffer<int4> int4rwbuf;
// INFER: %type_buffer_image_19 = OpTypeImage %uint Buffer 2 0 0 2 Unknown
// INFER: %_ptr_UniformConstant_type_buffer_image_19 = OpTypePointer UniformConstant %type_buffer_image_19
// INFER: %type_buffer_image_20 = OpTypeImage %uint Buffer 2 0 0 2 Rgba32ui
// INFER: %_ptr_UniformConstant_type_buffer_image_20 = OpTypePointer UniformConstant %type_buffer_image_20
RWBuffer<uint3> uint3rwbuf;
RWBuffer<uint4> uint4rwbuf;
// INFER: %type_buffer_image_21 = OpTypeImage %float Buffer 2 0 0 2 Unknown
// INFER: %_ptr_UniformConstant_type_buffer_image_21 = OpTypePointer UniformConstant %type_buffer_image_21
// INFER: %type_buffer_image_22 = OpTypeImage %float Buffer 2 0 0 2 Rgba32f
// INFER: %_ptr_UniformConstant_type_buffer_image_22 = OpTypePointer UniformConstant %type_buffer_image_22
RWBuffer<float3> float3rwbuf;
RWBuffer<float4> float4rwbuf;

// INFER: %intbuf = OpVariable %_ptr_UniformConstant_type_buffer_image UniformConstant
// INFER: %uintbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_0 UniformConstant
// INFER: %floatbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_1 UniformConstant
// INFER: %intrwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_2 UniformConstant
// INFER: %uintrwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_3 UniformConstant
// INFER: %floatrwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_4 UniformConstant
// INFER: %int2buf = OpVariable %_ptr_UniformConstant_type_buffer_image_5 UniformConstant
// INFER: %uint2buf = OpVariable %_ptr_UniformConstant_type_buffer_image_6 UniformConstant
// INFER: %float2buf = OpVariable %_ptr_UniformConstant_type_buffer_image_7 UniformConstant
// INFER: %int2rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_8 UniformConstant
// INFER: %uint2rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_9 UniformConstant
// INFER: %float2rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_10 UniformConstant
// INFER: %int3buf = OpVariable %_ptr_UniformConstant_type_buffer_image_11 UniformConstant
// INFER: %int4buf = OpVariable %_ptr_UniformConstant_type_buffer_image_12 UniformConstant
// INFER: %uint3buf = OpVariable %_ptr_UniformConstant_type_buffer_image_13 UniformConstant
// INFER: %uint4buf = OpVariable %_ptr_UniformConstant_type_buffer_image_14 UniformConstant
// INFER: %float3buf = OpVariable %_ptr_UniformConstant_type_buffer_image_15 UniformConstant
// INFER: %float4buf = OpVariable %_ptr_UniformConstant_type_buffer_image_16 UniformConstant
// INFER: %int3rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_17 UniformConstant
// INFER: %int4rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_18 UniformConstant
// INFER: %uint3rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_19 UniformConstant
// INFER: %uint4rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_20 UniformConstant
// INFER: %float3rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_21 UniformConstant
// INFER: %float4rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_22 UniformConstant

// UNKNOWN: %intbuf = OpVariable %_ptr_UniformConstant_type_buffer_image UniformConstant
// UNKNOWN: %uintbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_0 UniformConstant
// UNKNOWN: %floatbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_1 UniformConstant
// UNKNOWN: %intrwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_2 UniformConstant
// UNKNOWN: %uintrwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_3 UniformConstant
// UNKNOWN: %floatrwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_4 UniformConstant
// UNKNOWN: %int2buf = OpVariable %_ptr_UniformConstant_type_buffer_image UniformConstant
// UNKNOWN: %uint2buf = OpVariable %_ptr_UniformConstant_type_buffer_image_0 UniformConstant
// UNKNOWN: %float2buf = OpVariable %_ptr_UniformConstant_type_buffer_image_1 UniformConstant
// UNKNOWN: %int2rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_2 UniformConstant
// UNKNOWN: %uint2rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_3 UniformConstant
// UNKNOWN: %float2rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_4 UniformConstant
// UNKNOWN: %int3buf = OpVariable %_ptr_UniformConstant_type_buffer_image UniformConstant
// UNKNOWN: %int4buf = OpVariable %_ptr_UniformConstant_type_buffer_image UniformConstant
// UNKNOWN: %uint3buf = OpVariable %_ptr_UniformConstant_type_buffer_image_0 UniformConstant
// UNKNOWN: %uint4buf = OpVariable %_ptr_UniformConstant_type_buffer_image_0 UniformConstant
// UNKNOWN: %float3buf = OpVariable %_ptr_UniformConstant_type_buffer_image_1 UniformConstant
// UNKNOWN: %float4buf = OpVariable %_ptr_UniformConstant_type_buffer_image_1 UniformConstant
// UNKNOWN: %int3rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_2 UniformConstant
// UNKNOWN: %int4rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_2 UniformConstant
// UNKNOWN: %uint3rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_3 UniformConstant
// UNKNOWN: %uint4rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_3 UniformConstant
// UNKNOWN: %float3rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_4 UniformConstant
// UNKNOWN: %float4rwbuf = OpVariable %_ptr_UniformConstant_type_buffer_image_4 UniformConstant

void main() {}
