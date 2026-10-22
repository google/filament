// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s
// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv -DERROR 2>&1 | FileCheck %s --check-prefix=ERROR

// CHECK: OpCapability ImageQuery

// CHECK: [[type_1d_image_array:%[a-zA-Z0-9_]+]] = OpTypeImage %float 1D 0 1 0 1 Unknown
// CHECK: [[type_1d_sampled_image_array:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_1d_image_array]]

vk::SampledTexture1DArray<float4> tex1dArray;

void main() {
  uint mipLevel = 1;
  uint width, height, numLevels, elements, numSamples;

// CHECK:             [[t1_load:%[0-9]+]] = OpLoad [[type_1d_sampled_image_array]] %tex1dArray
// CHECK-NEXT:   [[image1:%[0-9]+]] = OpImage [[type_1d_image_array]] [[t1_load]]
// CHECK-NEXT:   [[query1:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[image1]] %int_0
// CHECK-NEXT: [[query1_0:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 0
// CHECK-NEXT:                      OpStore %width [[query1_0]]
// CHECK-NEXT: [[query1_1:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 1
// CHECK-NEXT:                      OpStore %elements [[query1_1]]
  tex1dArray.GetDimensions(width, elements);

// CHECK:             [[t2_load:%[0-9]+]] = OpLoad [[type_1d_sampled_image_array]] %tex1dArray
// CHECK-NEXT:   [[image2:%[0-9]+]] = OpImage [[type_1d_image_array]] [[t2_load]]
// CHECK-NEXT:      [[mip2:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:  [[query2:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[image2]] [[mip2]]
// CHECK-NEXT: [[query2_0:%[0-9]+]] = OpCompositeExtract %uint [[query2]] 0
// CHECK-NEXT:                      OpStore %width [[query2_0]]
// CHECK-NEXT: [[query2_1:%[0-9]+]] = OpCompositeExtract %uint [[query2]] 1
// CHECK-NEXT:                      OpStore %elements [[query2_1]]
// CHECK-NEXT: [[query_level_2:%[0-9]+]] = OpImageQueryLevels %uint [[image2]]
// CHECK-NEXT:                      OpStore %numLevels [[query_level_2]]
  tex1dArray.GetDimensions(mipLevel, width, elements, numLevels);

  float f_width, f_elements, f_numLevels;
// CHECK:             [[t3_load:%[0-9]+]] = OpLoad [[type_1d_sampled_image_array]] %tex1dArray
// CHECK-NEXT:   [[image3:%[0-9]+]] = OpImage [[type_1d_image_array]] [[t3_load]]
// CHECK-NEXT:  [[query3:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[image3]] %int_0
// CHECK-NEXT: [[query3_0:%[0-9]+]] = OpCompositeExtract %uint [[query3]] 0
// CHECK-NEXT: [[f_query3_0:%[0-9]+]] = OpConvertUToF %float [[query3_0]]
// CHECK-NEXT:                      OpStore %f_width [[f_query3_0]]
// CHECK-NEXT: [[query3_1:%[0-9]+]] = OpCompositeExtract %uint [[query3]] 1
// CHECK-NEXT: [[f_query3_1:%[0-9]+]] = OpConvertUToF %float [[query3_1]]
// CHECK-NEXT:                      OpStore %f_elements [[f_query3_1]]
  tex1dArray.GetDimensions(f_width, f_elements);

// CHECK:             [[t4_load:%[0-9]+]] = OpLoad [[type_1d_sampled_image_array]] %tex1dArray
// CHECK-NEXT:   [[image4:%[0-9]+]] = OpImage [[type_1d_image_array]] [[t4_load]]
// CHECK-NEXT:      [[mip4:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:  [[query4:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[image4]] [[mip4]]
// CHECK-NEXT: [[query4_0:%[0-9]+]] = OpCompositeExtract %uint [[query4]] 0
// CHECK-NEXT: [[f_query4_0:%[0-9]+]] = OpConvertUToF %float [[query4_0]]
// CHECK-NEXT:                      OpStore %f_width [[f_query4_0]]
// CHECK-NEXT: [[query4_1:%[0-9]+]] = OpCompositeExtract %uint [[query4]] 1
// CHECK-NEXT: [[f_query4_1:%[0-9]+]] = OpConvertUToF %float [[query4_1]]
// CHECK-NEXT:                      OpStore %f_elements [[f_query4_1]]
// CHECK-NEXT: [[query_level_4:%[0-9]+]] = OpImageQueryLevels %uint [[image4]]
// CHECK-NEXT: [[f_query_level_4:%[0-9]+]] = OpConvertUToF %float [[query_level_4]]
// CHECK-NEXT:                      OpStore %f_numLevels [[f_query_level_4]]
  tex1dArray.GetDimensions(mipLevel, f_width, f_elements, f_numLevels);

  int i_width, i_elements, i_numLevels;
// CHECK:             [[t5_load:%[0-9]+]] = OpLoad [[type_1d_sampled_image_array]] %tex1dArray
// CHECK-NEXT:   [[image5:%[0-9]+]] = OpImage [[type_1d_image_array]] [[t5_load]]
// CHECK-NEXT:  [[query5:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[image5]] %int_0
// CHECK-NEXT: [[query5_0:%[0-9]+]] = OpCompositeExtract %uint [[query5]] 0
// CHECK-NEXT: [[query5_0_i:%[0-9]+]] = OpBitcast %int [[query5_0]]
// CHECK-NEXT:                      OpStore %i_width [[query5_0_i]]
// CHECK-NEXT: [[query5_1:%[0-9]+]] = OpCompositeExtract %uint [[query5]] 1
// CHECK-NEXT: [[query5_1_i:%[0-9]+]] = OpBitcast %int [[query5_1]]
// CHECK-NEXT:                      OpStore %i_elements [[query5_1_i]]
  tex1dArray.GetDimensions(i_width, i_elements);

// CHECK:             [[t6_load:%[0-9]+]] = OpLoad [[type_1d_sampled_image_array]] %tex1dArray
// CHECK-NEXT:   [[image6:%[0-9]+]] = OpImage [[type_1d_image_array]] [[t6_load]]
// CHECK-NEXT:      [[mip6:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:  [[query6:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[image6]] [[mip6]]
// CHECK-NEXT: [[query6_0:%[0-9]+]] = OpCompositeExtract %uint [[query6]] 0
// CHECK-NEXT: [[query6_0_i:%[0-9]+]] = OpBitcast %int [[query6_0]]
// CHECK-NEXT:                      OpStore %i_width [[query6_0_i]]
// CHECK-NEXT: [[query6_1:%[0-9]+]] = OpCompositeExtract %uint [[query6]] 1
// CHECK-NEXT: [[query6_1_i:%[0-9]+]] = OpBitcast %int [[query6_1]]
// CHECK-NEXT:                      OpStore %i_elements [[query6_1_i]]
// CHECK-NEXT: [[query_level_6:%[0-9]+]] = OpImageQueryLevels %uint [[image6]]
// CHECK-NEXT: [[query_level_6_i:%[0-9]+]] = OpBitcast %int [[query_level_6]]
// CHECK-NEXT:                      OpStore %i_numLevels [[query_level_6_i]]
  tex1dArray.GetDimensions(mipLevel, i_width, i_elements, i_numLevels);

#ifdef ERROR
// ERROR: error: Output argument must be an l-value
  tex1dArray.GetDimensions(mipLevel, 0, elements, numLevels);

// ERROR: error: Output argument must be an l-value
  tex1dArray.GetDimensions(width, 20);
#endif
}