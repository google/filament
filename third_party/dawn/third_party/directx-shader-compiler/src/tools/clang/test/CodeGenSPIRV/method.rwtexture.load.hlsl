// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

RWTexture1D<int> intbuf;
RWTexture2D<uint2> uint2buf;
RWTexture3D<float3> float3buf;
RWTexture1DArray<float4> float4buf;
RWTexture2DArray<int3> int3buf;
RWTexture2D<float> vec1buf;

// CHECK: OpCapability SparseResidency

// CHECK: %SparseResidencyStruct = OpTypeStruct %uint %v4int
// CHECK: %SparseResidencyStruct_0 = OpTypeStruct %uint %v4uint
// CHECK: %SparseResidencyStruct_1 = OpTypeStruct %uint %v4float

void main() {

// CHECK:      [[img1:%[0-9]+]] = OpLoad %type_1d_image %intbuf
// CHECK-NEXT: [[ret1:%[0-9]+]] = OpImageRead %v4int [[img1]] %int_0 None
// CHECK-NEXT: [[r1:%[0-9]+]] = OpCompositeExtract %int [[ret1]] 0
// CHECK-NEXT: OpStore %a [[r1]]
  int a    = intbuf.Load(0);

// CHECK-NEXT: [[img2:%[0-9]+]] = OpLoad %type_2d_image %uint2buf
// CHECK-NEXT: [[ret2:%[0-9]+]] = OpImageRead %v4uint [[img2]] {{%[0-9]+}} None
// CHECK-NEXT: [[r2:%[0-9]+]] = OpVectorShuffle %v2uint [[ret2]] [[ret2]] 0 1
// CHECK-NEXT: OpStore %b [[r2]]
  uint2 b  = uint2buf.Load(0);

// CHECK-NEXT: [[img3:%[0-9]+]] = OpLoad %type_3d_image %float3buf
// CHECK-NEXT: [[ret3:%[0-9]+]] = OpImageRead %v4float [[img3]] {{%[0-9]+}} None
// CHECK-NEXT: [[r3:%[0-9]+]] = OpVectorShuffle %v3float [[ret3]] [[ret3]] 0 1 2
// CHECK-NEXT: OpStore %c [[r3]]
  float3 c = float3buf.Load(0);

// CHECK-NEXT: [[img4:%[0-9]+]] = OpLoad %type_1d_image_array %float4buf
// CHECK-NEXT: [[r4:%[0-9]+]] = OpImageRead %v4float [[img4]] {{%[0-9]+}} None
// CHECK-NEXT: OpStore %d [[r4]]
  float4 d = float4buf.Load(0);

// CHECK-NEXT: [[img5:%[0-9]+]] = OpLoad %type_2d_image_array %int3buf
// CHECK-NEXT: [[ret5:%[0-9]+]] = OpImageRead %v4int [[img5]] {{%[0-9]+}} None
// CHECK-NEXT: [[r5:%[0-9]+]] = OpVectorShuffle %v3int [[ret5]] [[ret5]] 0 1 2
// CHECK-NEXT: OpStore %e [[r5]]
  int3 e   = int3buf.Load(0);

// CHECK:      [[img6:%[0-9]+]] = OpLoad %type_2d_image_0 %vec1buf
// CHECK-NEXT:      {{%[0-9]+}} = OpImageRead %v4float [[img6]] {{%[0-9]+}} None
  float f = vec1buf.Load(0);

  uint status;
// CHECK:              [[img1_0:%[0-9]+]] = OpLoad %type_1d_image %intbuf
// CHECK-NEXT: [[structResult:%[0-9]+]] = OpImageSparseRead %SparseResidencyStruct [[img1_0]] %int_0 None
// CHECK-NEXT:       [[status:%[0-9]+]] = OpCompositeExtract %uint [[structResult]] 0
// CHECK-NEXT:                         OpStore %status [[status]]
// CHECK-NEXT:     [[v4result:%[0-9]+]] = OpCompositeExtract %v4int [[structResult]] 1
// CHECK-NEXT:       [[result:%[0-9]+]] = OpCompositeExtract %int [[v4result]] 0
// CHECK-NEXT:                         OpStore %a2 [[result]]
  int    a2 = intbuf.Load(0, status);

// CHECK:              [[img2_0:%[0-9]+]] = OpLoad %type_2d_image %uint2buf
// CHECK-NEXT: [[structResult_0:%[0-9]+]] = OpImageSparseRead %SparseResidencyStruct_0 [[img2_0]] {{%[0-9]+}} None
// CHECK-NEXT:       [[status_0:%[0-9]+]] = OpCompositeExtract %uint [[structResult_0]] 0
// CHECK-NEXT:                         OpStore %status [[status_0]]
// CHECK-NEXT:     [[v4result_0:%[0-9]+]] = OpCompositeExtract %v4uint [[structResult_0]] 1
// CHECK-NEXT:       [[result_0:%[0-9]+]] = OpVectorShuffle %v2uint [[v4result_0]] [[v4result_0]] 0 1
// CHECK-NEXT:                         OpStore %b2 [[result_0]]
  uint2  b2 = uint2buf.Load(0, status);

// CHECK:              [[img3_0:%[0-9]+]] = OpLoad %type_3d_image %float3buf
// CHECK-NEXT: [[structResult_1:%[0-9]+]] = OpImageSparseRead %SparseResidencyStruct_1 [[img3_0]] {{%[0-9]+}} None
// CHECK-NEXT:       [[status_1:%[0-9]+]] = OpCompositeExtract %uint [[structResult_1]] 0
// CHECK-NEXT:                         OpStore %status [[status_1]]
// CHECK-NEXT:     [[v4result_1:%[0-9]+]] = OpCompositeExtract %v4float [[structResult_1]] 1
// CHECK-NEXT:       [[result_1:%[0-9]+]] = OpVectorShuffle %v3float [[v4result_1]] [[v4result_1]] 0 1 2
// CHECK-NEXT:                         OpStore %c2 [[result_1]]
  float3 c2 = float3buf.Load(0, status);

// CHECK:              [[img4_0:%[0-9]+]] = OpLoad %type_1d_image_array %float4buf
// CHECK-NEXT: [[structResult_2:%[0-9]+]] = OpImageSparseRead %SparseResidencyStruct_1 [[img4_0]] {{%[0-9]+}} None
// CHECK-NEXT:       [[status_2:%[0-9]+]] = OpCompositeExtract %uint [[structResult_2]] 0
// CHECK-NEXT:                         OpStore %status [[status_2]]
// CHECK-NEXT:     [[v4result_2:%[0-9]+]] = OpCompositeExtract %v4float [[structResult_2]] 1
// CHECK-NEXT:                         OpStore %d2 [[v4result_2]]
  float4 d2 = float4buf.Load(0, status);

// CHECK:              [[img5_0:%[0-9]+]] = OpLoad %type_2d_image_array %int3buf
// CHECK-NEXT: [[structResult_3:%[0-9]+]] = OpImageSparseRead %SparseResidencyStruct [[img5_0]] {{%[0-9]+}} None
// CHECK-NEXT:       [[status_3:%[0-9]+]] = OpCompositeExtract %uint [[structResult_3]] 0
// CHECK-NEXT:                         OpStore %status [[status_3]]
// CHECK-NEXT:     [[v4result_3:%[0-9]+]] = OpCompositeExtract %v4int [[structResult_3]] 1
// CHECK-NEXT:       [[result_2:%[0-9]+]] = OpVectorShuffle %v3int [[v4result_3]] [[v4result_3]] 0 1 2
// CHECK-NEXT:                         OpStore %e2 [[result_2]]
  int3   e2 = int3buf.Load(0, status);
}
