// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK: OpCapability ImageQuery

// CHECK: [[type_2d_image_ms:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 0 1 1 Unknown
// CHECK: [[type_2d_sampled_image_ms:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image_ms]]

vk::SampledTexture2DMS<float4> tex2dMS;

void main() {
  uint mipLevel = 1;
  uint width, height, numLevels, elements, numSamples;

// CHECK:          [[t1_load:%[0-9]+]] = OpLoad [[type_2d_sampled_image_ms]] %tex2dMS
// CHECK-NEXT:     [[image1:%[0-9]+]] = OpImage [[type_2d_image_ms]] [[t1_load]]
// CHECK-NEXT:     [[query1:%[0-9]+]] = OpImageQuerySize %v2uint [[image1]]
// CHECK-NEXT:   [[query1_0:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 0
// CHECK-NEXT:                        OpStore %width [[query1_0]]
// CHECK-NEXT:   [[query1_1:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 1
// CHECK-NEXT:                        OpStore %height [[query1_1]]
// CHECK-NEXT: [[query1_samples:%[0-9]+]] = OpImageQuerySamples %uint [[image1]]
// CHECK-NEXT:                        OpStore %numSamples [[query1_samples]]
  tex2dMS.GetDimensions(width, height, numSamples);

  float widthF, heightF, numSamplesF;
// CHECK:          [[t2_load:%[0-9]+]] = OpLoad [[type_2d_sampled_image_ms]] %tex2dMS
// CHECK-NEXT:     [[image2:%[0-9]+]] = OpImage [[type_2d_image_ms]] [[t2_load]]
// CHECK-NEXT:     [[query2:%[0-9]+]] = OpImageQuerySize %v2uint [[image2]]
// CHECK-NEXT:   [[query2_0:%[0-9]+]] = OpCompositeExtract %uint [[query2]] 0
// CHECK-NEXT:    [[widthF:%[0-9]+]] = OpConvertUToF %float [[query2_0]]
// CHECK-NEXT:                        OpStore %widthF [[widthF]]
// CHECK-NEXT:   [[query2_1:%[0-9]+]] = OpCompositeExtract %uint [[query2]] 1
// CHECK-NEXT:    [[heightF:%[0-9]+]] = OpConvertUToF %float [[query2_1]]
// CHECK-NEXT:                        OpStore %heightF [[heightF]]
// CHECK-NEXT: [[query2_samples:%[0-9]+]] = OpImageQuerySamples %uint [[image2]]
// CHECK-NEXT:    [[numSamplesF:%[0-9]+]] = OpConvertUToF %float [[query2_samples]]
// CHECK-NEXT:                        OpStore %numSamplesF [[numSamplesF]]
  tex2dMS.GetDimensions(widthF, heightF, numSamplesF);
}
