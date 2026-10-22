// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability ImageQuery

// CHECK: [[type_2d_image_ms_array:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 1 1 1 Unknown
// CHECK: [[type_2d_sampled_image_ms_array:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image_ms_array]]

vk::SampledTexture2DMSArray<float4> tex2dMSArray;

void main() {
  uint mipLevel = 1;
  uint width, height, numLevels, elements, numSamples;

// CHECK:          [[t1_load:%[0-9]+]] = OpLoad [[type_2d_sampled_image_ms_array]] %tex2dMSArray
// CHECK-NEXT:     [[image1:%[0-9]+]] = OpImage [[type_2d_image_ms_array]] [[t1_load]]
// CHECK-NEXT:     [[query1:%[0-9]+]] = OpImageQuerySize %v3uint [[image1]]
// CHECK-NEXT:   [[query1_0:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 0
// CHECK-NEXT:                        OpStore %width [[query1_0]]
// CHECK-NEXT:   [[query1_1:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 1
// CHECK-NEXT:                        OpStore %height [[query1_1]]
// CHECK-NEXT:   [[query1_2:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 2
// CHECK-NEXT:                        OpStore %elements [[query1_2]]
// CHECK-NEXT: [[query1_samples:%[0-9]+]] = OpImageQuerySamples %uint [[image1]]
// CHECK-NEXT:                        OpStore %numSamples [[query1_samples]]
  tex2dMSArray.GetDimensions(width, height, elements, numSamples);

  float widthF, heightF, elementsF, numSamplesF;
// CHECK:          [[t1_load:%[0-9]+]] = OpLoad [[type_2d_sampled_image_ms_array]] %tex2dMSArray
// CHECK-NEXT:     [[image1:%[0-9]+]] = OpImage [[type_2d_image_ms_array]] [[t1_load]]
// CHECK-NEXT:     [[query1:%[0-9]+]] = OpImageQuerySize %v3uint [[image1]]
// CHECK-NEXT:   [[query1_0:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 0
// CHECK-NEXT:    [[widthF:%[0-9]+]] = OpConvertUToF %float [[query1_0]]
// CHECK-NEXT:                        OpStore %widthF [[widthF]]
// CHECK-NEXT:   [[query1_1:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 1
// CHECK-NEXT:    [[heightF:%[0-9]+]] = OpConvertUToF %float [[query1_1]]
// CHECK-NEXT:                        OpStore %heightF [[heightF]]
// CHECK-NEXT:   [[query1_2:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 2
// CHECK-NEXT:    [[elementsF:%[0-9]+]] = OpConvertUToF %float [[query1_2]]
// CHECK-NEXT:                        OpStore %elementsF [[elementsF]]
// CHECK-NEXT: [[query1_samples:%[0-9]+]] = OpImageQuerySamples %uint [[image1]]
// CHECK-NEXT:    [[numSamplesF:%[0-9]+]] = OpConvertUToF %float [[query1_samples]]
// CHECK-NEXT:                        OpStore %numSamplesF [[numSamplesF]]
  tex2dMSArray.GetDimensions(widthF, heightF, elementsF, numSamplesF);
}
