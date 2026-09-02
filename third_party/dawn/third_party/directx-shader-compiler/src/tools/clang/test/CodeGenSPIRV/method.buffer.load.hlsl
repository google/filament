// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability ImageBuffer

Buffer<int> intbuf;
Buffer<uint> uintbuf;
Buffer<float> floatbuf;
RWBuffer<int2> int2buf;
RWBuffer<uint2> uint2buf;
RWBuffer<float2> float2buf;
Buffer<int3> int3buf;
Buffer<uint3> uint3buf;
Buffer<float3> float3buf;
RWBuffer<int4> int4buf;
RWBuffer<uint4> uint4buf;
RWBuffer<float4> float4buf;

// CHECK: OpCapability SparseResidency

// CHECK: %SparseResidencyStruct = OpTypeStruct %uint %v4float

void main() {
  int address;

// CHECK:      [[img1:%[0-9]+]] = OpLoad %type_buffer_image %intbuf
// CHECK:      [[f1:%[0-9]+]] = OpImageFetch %v4int [[img1]] {{%[0-9]+}} None
// CHECK-NEXT: {{%[0-9]+}} = OpCompositeExtract %int [[f1]] 0
  int i1 = intbuf.Load(address);

// CHECK:      [[img2:%[0-9]+]] = OpLoad %type_buffer_image_0 %uintbuf
// CHECK:      [[f2:%[0-9]+]] = OpImageFetch %v4uint [[img2]] {{%[0-9]+}} None
// CHECK-NEXT: {{%[0-9]+}} = OpCompositeExtract %uint [[f2]] 0
  uint u1 = uintbuf.Load(address);

// CHECK:      [[img3:%[0-9]+]] = OpLoad %type_buffer_image_1 %floatbuf
// CHECK:      [[f3:%[0-9]+]] = OpImageFetch %v4float [[img3]] {{%[0-9]+}} None
// CHECK-NEXT: {{%[0-9]+}} = OpCompositeExtract %float [[f3]] 0
  float f1 = floatbuf.Load(address);

// CHECK:      [[img4:%[0-9]+]] = OpLoad %type_buffer_image_2 %int2buf
// CHECK:      [[f4:%[0-9]+]] = OpImageRead %v4int [[img4]] {{%[0-9]+}} None
// CHECK-NEXT: {{%[0-9]+}} = OpVectorShuffle %v2int [[f4]] [[f4]] 0 1
  int2 i2 = int2buf.Load(address);

// CHECK:      [[img5:%[0-9]+]] = OpLoad %type_buffer_image_3 %uint2buf
// CHECK:      [[f5:%[0-9]+]] = OpImageRead %v4uint [[img5]] {{%[0-9]+}} None
// CHECK-NEXT: {{%[0-9]+}} = OpVectorShuffle %v2uint [[f5]] [[f5]] 0 1
  uint2 u2 = uint2buf.Load(address);

// CHECK:      [[img6:%[0-9]+]] = OpLoad %type_buffer_image_4 %float2buf
// CHECK:      [[f6:%[0-9]+]] = OpImageRead %v4float [[img6]] {{%[0-9]+}} None
// CHECK-NEXT: {{%[0-9]+}} = OpVectorShuffle %v2float [[f6]] [[f6]] 0 1
  float2 f2 = float2buf.Load(address);

// CHECK:      [[img7:%[0-9]+]] = OpLoad %type_buffer_image_5 %int3buf
// CHECK:      [[f7:%[0-9]+]] = OpImageFetch %v4int [[img7]] {{%[0-9]+}} None
// CHECK-NEXT: {{%[0-9]+}} = OpVectorShuffle %v3int [[f7]] [[f7]] 0 1 2
  int3 i3 = int3buf.Load(address);

// CHECK:      [[img8:%[0-9]+]] = OpLoad %type_buffer_image_6 %uint3buf
// CHECK:      [[f8:%[0-9]+]] = OpImageFetch %v4uint [[img8]] {{%[0-9]+}} None
// CHECK-NEXT: {{%[0-9]+}} = OpVectorShuffle %v3uint [[f8]] [[f8]] 0 1 2
  uint3 u3 = uint3buf.Load(address);

// CHECK:      [[img9:%[0-9]+]] = OpLoad %type_buffer_image_7 %float3buf
// CHECK:      [[f9:%[0-9]+]] = OpImageFetch %v4float [[img9]] {{%[0-9]+}} None
// CHECK-NEXT: {{%[0-9]+}} = OpVectorShuffle %v3float [[f9]] [[f9]] 0 1 2
  float3 f3 = float3buf.Load(address);

// CHECK:      [[img10:%[0-9]+]] = OpLoad %type_buffer_image_8 %int4buf
// CHECK:      {{%[0-9]+}} = OpImageRead %v4int [[img10]] {{%[0-9]+}} None
// CHECK-NEXT: OpStore %i4 {{%[0-9]+}}
  int4 i4 = int4buf.Load(address);

// CHECK:      [[img11:%[0-9]+]] = OpLoad %type_buffer_image_9 %uint4buf
// CHECK:      {{%[0-9]+}} = OpImageRead %v4uint [[img11]] {{%[0-9]+}} None
// CHECK-NEXT: OpStore %u4 {{%[0-9]+}}
  uint4 u4 = uint4buf.Load(address);

// CHECK:      [[img12:%[0-9]+]] = OpLoad %type_buffer_image_10 %float4buf
// CHECK:      {{%[0-9]+}} = OpImageRead %v4float [[img12]] {{%[0-9]+}} None
// CHECK-NEXT: OpStore %f4 {{%[0-9]+}}
  float4 f4 = float4buf.Load(address);

///////////////////////////////
// Using the Status argument //  
///////////////////////////////
  uint status;

// CHECK:              [[img3_0:%[0-9]+]] = OpLoad %type_buffer_image_1 %floatbuf
// CHECK-NEXT: [[structResult:%[0-9]+]] = OpImageSparseFetch %SparseResidencyStruct [[img3_0]] {{%[0-9]+}} None
// CHECK-NEXT:       [[status:%[0-9]+]] = OpCompositeExtract %uint [[structResult]] 0
// CHECK-NEXT:                         OpStore %status [[status]]
// CHECK-NEXT:     [[v4result:%[0-9]+]] = OpCompositeExtract %v4float [[structResult]] 1
// CHECK-NEXT:       [[result:%[0-9]+]] = OpCompositeExtract %float [[v4result]] 0
// CHECK-NEXT:                         OpStore %r1 [[result]]
  float  r1 = floatbuf.Load(address, status);  // Test for Buffer

// CHECK:              [[img6_0:%[0-9]+]] = OpLoad %type_buffer_image_4 %float2buf
// CHECK-NEXT: [[structResult_0:%[0-9]+]] = OpImageSparseRead %SparseResidencyStruct [[img6_0]] {{%[0-9]+}} None
// CHECK-NEXT:       [[status_0:%[0-9]+]] = OpCompositeExtract %uint [[structResult_0]] 0
// CHECK-NEXT:                         OpStore %status [[status_0]]
// CHECK-NEXT:     [[v4result_0:%[0-9]+]] = OpCompositeExtract %v4float [[structResult_0]] 1
// CHECK-NEXT:       [[result_0:%[0-9]+]] = OpVectorShuffle %v2float [[v4result_0]] [[v4result_0]] 0 1
// CHECK-NEXT:                         OpStore %r2 [[result_0]]
  float2 r2 = float2buf.Load(address, status);  // Test for RWBuffer
}
