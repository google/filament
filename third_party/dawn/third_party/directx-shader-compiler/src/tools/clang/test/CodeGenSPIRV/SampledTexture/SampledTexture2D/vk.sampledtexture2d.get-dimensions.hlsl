// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s
// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv -DERROR 2>&1 | FileCheck %s --check-prefix=ERROR

// CHECK: OpCapability ImageQuery

// CHECK: [[type_1d_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float 1D 0 0 0 1 Unknown
// CHECK: [[type_1d_sampled_image:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_1d_image]]
// CHECK: [[type_1d_image_array:%[a-zA-Z0-9_]+]] = OpTypeImage %float 1D 0 1 0 1 Unknown
// CHECK: [[type_1d_sampled_image_array:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_1d_image_array]]
// CHECK: [[type_2d_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float 2D 0 0 0 1 Unknown
// CHECK: [[type_2d_sampled_image:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_2d_image]]

vk::SampledTexture1D<float4> tex1d;
vk::SampledTexture1DArray<float4> tex1dArray;
vk::SampledTexture2D<float4> tex2d;

void main() {
  uint mipLevel = 1;
  uint width, height, numLevels, elements, numSamples;

// CHECK:             [[t1_load:%[0-9]+]] = OpLoad [[type_2d_sampled_image]] %tex2d
// CHECK-NEXT:   [[image1:%[0-9]+]] = OpImage [[type_2d_image]] [[t1_load]]
// CHECK-NEXT:   [[query1:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[image1]] %int_0
// CHECK-NEXT: [[query1_0:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 0
// CHECK-NEXT:                      OpStore %width [[query1_0]]
// CHECK-NEXT: [[query1_1:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 1
// CHECK-NEXT:                      OpStore %height [[query1_1]]
  tex2d.GetDimensions(width, height);

// CHECK:             [[t1_load:%[0-9]+]] = OpLoad [[type_2d_sampled_image]] %tex2d
// CHECK-NEXT:   [[image2:%[0-9]+]] = OpImage [[type_2d_image]] [[t1_load]]
// CHECK-NEXT:       [[mip:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:   [[query2:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[image2]] [[mip]]
// CHECK-NEXT: [[query2_0:%[0-9]+]] = OpCompositeExtract %uint [[query2]] 0
// CHECK-NEXT:                      OpStore %width [[query2_0]]
// CHECK-NEXT: [[query2_1:%[0-9]+]] = OpCompositeExtract %uint [[query2]] 1
// CHECK-NEXT:                      OpStore %height [[query2_1]]
// CHECK-NEXT:   [[query_level_2:%[0-9]+]] = OpImageQueryLevels %uint [[image2]]
// CHECK-NEXT:                      OpStore %numLevels [[query_level_2]]
  tex2d.GetDimensions(mipLevel, width, height, numLevels);

  float f_width, f_height, f_numLevels;
// CHECK:             [[t1_load:%[0-9]+]] = OpLoad [[type_2d_sampled_image]] %tex2d
// CHECK-NEXT:   [[image1:%[0-9]+]] = OpImage [[type_2d_image]] [[t1_load]]
// CHECK-NEXT:   [[query1:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[image1]] %int_0
// CHECK-NEXT: [[query1_0:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 0
// CHECK-NEXT: [[f_query1_0:%[0-9]+]] = OpConvertUToF %float [[query1_0]]
// CHECK-NEXT:                      OpStore %f_width [[f_query1_0]]
// CHECK-NEXT: [[query1_1:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 1
// CHECK-NEXT: [[f_query1_1:%[0-9]+]] = OpConvertUToF %float [[query1_1]]
// CHECK-NEXT:                      OpStore %f_height [[f_query1_1]]
  tex2d.GetDimensions(f_width, f_height);

// CHECK:             [[t1_load:%[0-9]+]] = OpLoad [[type_2d_sampled_image]] %tex2d
// CHECK-NEXT:   [[image2:%[0-9]+]] = OpImage [[type_2d_image]] [[t1_load]]
// CHECK-NEXT:       [[mip:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:   [[query2:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[image2]] [[mip]]
// CHECK-NEXT: [[query2_0:%[0-9]+]] = OpCompositeExtract %uint [[query2]] 0
// CHECK-NEXT: [[f_query2_0:%[0-9]+]] = OpConvertUToF %float [[query2_0]]
// CHECK-NEXT:                      OpStore %f_width [[f_query2_0]]
// CHECK-NEXT: [[query2_1:%[0-9]+]] = OpCompositeExtract %uint [[query2]] 1
// CHECK-NEXT: [[f_query2_1:%[0-9]+]] = OpConvertUToF %float [[query2_1]]
// CHECK-NEXT:                      OpStore %f_height [[f_query2_1]]
// CHECK-NEXT:   [[query_level_2:%[0-9]+]] = OpImageQueryLevels %uint [[image2]]
// CHECK-NEXT: [[f_query_level_2:%[0-9]+]] = OpConvertUToF %float [[query_level_2]]
// CHECK-NEXT:                      OpStore %f_numLevels [[f_query_level_2]]
  tex2d.GetDimensions(mipLevel, f_width, f_height, f_numLevels);

  int i_width, i_height, i_numLevels;
// CHECK:             [[t1_load:%[0-9]+]] = OpLoad [[type_2d_sampled_image]] %tex2d
// CHECK-NEXT:   [[image1:%[0-9]+]] = OpImage [[type_2d_image]] [[t1_load]]
// CHECK-NEXT:   [[query1:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[image1]] %int_0
// CHECK-NEXT: [[query1_0:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 0
// CHECK-NEXT:  [[query_0_int:%[0-9]+]] = OpBitcast %int [[query1_0]]
// CHECK-NEXT:                      OpStore %i_width [[query_0_int]]
// CHECK-NEXT: [[query1_1:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 1
// CHECK-NEXT:  [[query_1_int:%[0-9]+]] = OpBitcast %int [[query1_1]]
// CHECK-NEXT:                      OpStore %i_height [[query_1_int]]
  tex2d.GetDimensions(i_width, i_height);

// CHECK:          [[t1d_load:%[0-9]+]] = OpLoad [[type_1d_sampled_image]] %tex1d
// CHECK-NEXT:     [[image1d:%[0-9]+]] = OpImage [[type_1d_image]] [[t1d_load]]
// CHECK-NEXT:     [[query1d:%[0-9]+]] = OpImageQuerySizeLod %uint [[image1d]] %int_0
// CHECK-NEXT:                        OpStore %width [[query1d]]
  tex1d.GetDimensions(width);

// CHECK:           [[t1da_load:%[0-9]+]] = OpLoad [[type_1d_sampled_image_array]] %tex1dArray
// CHECK-NEXT:     [[image1da:%[0-9]+]] = OpImage [[type_1d_image_array]] [[t1da_load]]
// CHECK-NEXT:     [[query1da:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[image1da]] %int_0
// CHECK-NEXT:   [[query1da0:%[0-9]+]] = OpCompositeExtract %uint [[query1da]] 0
// CHECK-NEXT:                        OpStore %width [[query1da0]]
// CHECK-NEXT:   [[query1da1:%[0-9]+]] = OpCompositeExtract %uint [[query1da]] 1
// CHECK-NEXT:                        OpStore %elements [[query1da1]]
  tex1dArray.GetDimensions(width, elements);

// CHECK:             [[t1_load:%[0-9]+]] = OpLoad [[type_2d_sampled_image]] %tex2d
// CHECK-NEXT:   [[image2:%[0-9]+]] = OpImage [[type_2d_image]] [[t1_load]]
// CHECK-NEXT:       [[mip:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:   [[query2:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[image2]] [[mip]]
// CHECK-NEXT: [[query2_0:%[0-9]+]] = OpCompositeExtract %uint [[query2]] 0
// CHECK-NEXT:  [[query_0_int:%[0-9]+]] = OpBitcast %int [[query2_0]]
// CHECK-NEXT:                      OpStore %i_width [[query_0_int]]
// CHECK-NEXT: [[query2_1:%[0-9]+]] = OpCompositeExtract %uint [[query2]] 1
// CHECK-NEXT:  [[query_1_int:%[0-9]+]] = OpBitcast %int [[query2_1]]
// CHECK-NEXT:                      OpStore %i_height [[query_1_int]]
// CHECK-NEXT:   [[query_level_2:%[0-9]+]] = OpImageQueryLevels %uint [[image2]]
// CHECK-NEXT:  [[query_level_2_int:%[0-9]+]] = OpBitcast %int [[query_level_2]]
// CHECK-NEXT:                      OpStore %i_numLevels [[query_level_2_int]]
  tex2d.GetDimensions(mipLevel, i_width, i_height, i_numLevels);

#ifdef ERROR
// ERROR: error: Output argument must be an l-value
  tex2d.GetDimensions(mipLevel, 0, height, numLevels);

// ERROR: error: Output argument must be an l-value
  tex2d.GetDimensions(width, 20);
#endif
}