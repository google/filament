// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability ImageBuffer
// CHECK-NOT: OpCapability StorageImageReadWithoutFormat

// CHECK: %type_buffer_image = OpTypeImage %int Buffer 2 0 0 1 R32i
// CHECK: %type_buffer_image_0 = OpTypeImage %uint Buffer 2 0 0 1 R32ui
// CHECK: %type_buffer_image_1 = OpTypeImage %float Buffer 2 0 0 1 R32f
Buffer<int> intbuf;
Buffer<uint> uintbuf;
Buffer<float> floatbuf;
// CHECK: %type_buffer_image_2 = OpTypeImage %int Buffer 2 0 0 2 Rg32i
// CHECK: %type_buffer_image_3 = OpTypeImage %uint Buffer 2 0 0 2 Rg32ui
// CHECK: %type_buffer_image_4 = OpTypeImage %float Buffer 2 0 0 2 Rg32f
RWBuffer<int2> int2buf;
RWBuffer<uint2> uint2buf;
RWBuffer<float2> float2buf;
// CHECK: %type_buffer_image_5 = OpTypeImage %int Buffer 2 0 0 1 Unknown
// CHECK: %type_buffer_image_6 = OpTypeImage %uint Buffer 2 0 0 1 Unknown
// CHECK: %type_buffer_image_7 = OpTypeImage %float Buffer 2 0 0 1 Unknown
Buffer<int3> int3buf;
Buffer<uint3> uint3buf;
Buffer<float3> float3buf;
// CHECK: %type_buffer_image_8 = OpTypeImage %int Buffer 2 0 0 2 Rgba32i
// CHECK: %type_buffer_image_9 = OpTypeImage %uint Buffer 2 0 0 2 Rgba32ui
// CHECK: %type_buffer_image_10 = OpTypeImage %float Buffer 2 0 0 2 Rgba32f
RWBuffer<int4> int4buf;
RWBuffer<uint4> uint4buf;
RWBuffer<float4> float4buf;

void main() {
  int address;

// CHECK:      [[img1:%[0-9]+]] = OpLoad %type_buffer_image %intbuf
// CHECK:      [[f1:%[0-9]+]] = OpImageFetch %v4int [[img1]] {{%[0-9]+}} None
// CHECK-NEXT: [[r1:%[0-9]+]] = OpCompositeExtract %int [[f1]] 0
// CHECK-NEXT: OpStore %int1 [[r1]]
  int int1 = intbuf[address];

// CHECK:      [[img2:%[0-9]+]] = OpLoad %type_buffer_image_0 %uintbuf
// CHECK:      [[f2:%[0-9]+]] = OpImageFetch %v4uint [[img2]] {{%[0-9]+}} None
// CHECK-NEXT: [[r2:%[0-9]+]] = OpCompositeExtract %uint [[f2]] 0
// CHECK-NEXT: OpStore %uint1 [[r2]]
  uint uint1 = uintbuf[address];

// CHECK:      [[img3:%[0-9]+]] = OpLoad %type_buffer_image_1 %floatbuf
// CHECK:      [[f3:%[0-9]+]] = OpImageFetch %v4float [[img3]] {{%[0-9]+}} None
// CHECK-NEXT: [[r3:%[0-9]+]] = OpCompositeExtract %float [[f3]] 0
// CHECK-NEXT: OpStore %float1 [[r3]]
  float float1 = floatbuf[address];

// CHECK:      [[img4:%[0-9]+]] = OpLoad %type_buffer_image_2 %int2buf
// CHECK:      [[ret4:%[0-9]+]] = OpImageRead %v4int [[img4]] {{%[0-9]+}} None
// CHECK-NEXT: [[r4:%[0-9]+]] = OpVectorShuffle %v2int [[ret4]] [[ret4]] 0 1
// CHECK-NEXT: OpStore %int2 [[r4]]
  int2 int2 = int2buf[address];

// CHECK:      [[img5:%[0-9]+]] = OpLoad %type_buffer_image_3 %uint2buf
// CHECK:      [[ret5:%[0-9]+]] = OpImageRead %v4uint [[img5]] {{%[0-9]+}} None
// CHECK-NEXT: [[r5:%[0-9]+]] = OpVectorShuffle %v2uint [[ret5]] [[ret5]] 0 1
// CHECK-NEXT: OpStore %uint2 [[r5]]
  uint2 uint2 = uint2buf[address];

// CHECK:      [[img6:%[0-9]+]] = OpLoad %type_buffer_image_4 %float2buf
// CHECK:      [[ret6:%[0-9]+]] = OpImageRead %v4float [[img6]] {{%[0-9]+}} None
// CHECK-NEXT: [[r6:%[0-9]+]] = OpVectorShuffle %v2float [[ret6]] [[ret6]] 0 1
// CHECK-NEXT: OpStore %float2 [[r6]]
  float2 float2 = float2buf[address];

// CHECK:      [[img7:%[0-9]+]] = OpLoad %type_buffer_image_5 %int3buf
// CHECK:      [[f7:%[0-9]+]] = OpImageFetch %v4int [[img7]] {{%[0-9]+}} None
// CHECK-NEXT: [[r7:%[0-9]+]] = OpVectorShuffle %v3int [[f7]] [[f7]] 0 1 2
// CHECK-NEXT: OpStore %int3 [[r7]]
  int3 int3 = int3buf[address];

// CHECK:      [[img8:%[0-9]+]] = OpLoad %type_buffer_image_6 %uint3buf
// CHECK:      [[f8:%[0-9]+]] = OpImageFetch %v4uint [[img8]] {{%[0-9]+}} None
// CHECK-NEXT: [[r8:%[0-9]+]] = OpVectorShuffle %v3uint [[f8]] [[f8]] 0 1 2
// CHECK-NEXT: OpStore %uint3 [[r8]]
  uint3 uint3 = uint3buf[address];

// CHECK:      [[img9:%[0-9]+]] = OpLoad %type_buffer_image_7 %float3buf
// CHECK:      [[f9:%[0-9]+]] = OpImageFetch %v4float [[img9]] {{%[0-9]+}} None
// CHECK-NEXT: [[r9:%[0-9]+]] = OpVectorShuffle %v3float [[f9]] [[f9]] 0 1 2
// CHECK-NEXT: OpStore %float3 [[r9]]
  float3 float3 = float3buf[address];

// CHECK:      [[img10:%[0-9]+]] = OpLoad %type_buffer_image_8 %int4buf
// CHECK:      [[r10:%[0-9]+]] = OpImageRead %v4int [[img10]] {{%[0-9]+}} None
// CHECK-NEXT: OpStore %int4 [[r10]]
  int4 int4 = int4buf[address];

// CHECK:      [[img11:%[0-9]+]] = OpLoad %type_buffer_image_9 %uint4buf
// CHECK:      [[r11:%[0-9]+]] = OpImageRead %v4uint [[img11]] {{%[0-9]+}} None
// CHECK-NEXT: OpStore %uint4 [[r11]]
  uint4 uint4 = uint4buf[address];

// CHECK:      [[img12:%[0-9]+]] = OpLoad %type_buffer_image_10 %float4buf
// CHECK:      [[r12:%[0-9]+]] = OpImageRead %v4float [[img12]] {{%[0-9]+}} None
// CHECK-NEXT: OpStore %float4 [[r12]]
  float4 float4 = float4buf[address];

// CHECK:      [[img13:%[0-9]+]] = OpLoad %type_buffer_image_5 %int3buf
// CHECK-NEXT:   [[f13:%[0-9]+]] = OpImageFetch %v4int [[img13]] %uint_0 None
// CHECK-NEXT:   [[r13:%[0-9]+]] = OpVectorShuffle %v3int [[f13]] [[f13]] 0 1 2
// CHECK-NEXT:                  OpStore %temp_var_vector [[r13]]
// CHECK-NEXT:  [[ac13:%[0-9]+]] = OpAccessChain %_ptr_Function_int %temp_var_vector %uint_1
// CHECK-NEXT:     [[a:%[0-9]+]] = OpLoad %int [[ac13]]
// CHECK-NEXT:                  OpStore %a [[a]]
  int   a = int3buf[0][1];
// CHECK:      [[img14:%[0-9]+]] = OpLoad %type_buffer_image_10 %float4buf
// CHECK-NEXT:   [[f14:%[0-9]+]] = OpImageRead %v4float [[img14]] {{%[0-9]+}} None
// CHECK-NEXT:                  OpStore %temp_var_vector_0 [[f14]]
// CHECK-NEXT:  [[ac14:%[0-9]+]] = OpAccessChain %_ptr_Function_float %temp_var_vector_0 %uint_2
// CHECK-NEXT:     [[b:%[0-9]+]] = OpLoad %float [[ac14]]
// CHECK-NEXT:                  OpStore %b [[b]]
  float b = float4buf[address][2];

}
