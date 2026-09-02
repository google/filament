// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s
// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv -DERROR 2>&1 | FileCheck %s --check-prefix=ERROR

// CHECK: OpCapability ImageQuery

// CHECK: [[type_1d_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float 1D 0 0 0 1 Unknown
// CHECK: [[type_1d_sampled_image:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_1d_image]]

vk::SampledTexture1D<float4> tex1d;

void main() {
  uint mipLevel = 1;
  uint width, height, numLevels, elements, numSamples;

// CHECK:             [[t1_load:%[0-9]+]] = OpLoad [[type_1d_sampled_image]] %tex1d
// CHECK-NEXT:   [[image1:%[0-9]+]] = OpImage [[type_1d_image]] [[t1_load]]
// CHECK-NEXT:   [[query1:%[0-9]+]] = OpImageQuerySizeLod %uint [[image1]] %int_0
// CHECK-NEXT:                      OpStore %width [[query1]]
  tex1d.GetDimensions(width);

// CHECK:             [[t2_load:%[0-9]+]] = OpLoad [[type_1d_sampled_image]] %tex1d
// CHECK-NEXT:   [[image2:%[0-9]+]] = OpImage [[type_1d_image]] [[t2_load]]
// CHECK-NEXT:       [[mip:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:   [[query2:%[0-9]+]] = OpImageQuerySizeLod %uint [[image2]] [[mip]]
// CHECK-NEXT:                      OpStore %width [[query2]]
// CHECK-NEXT: [[query_level_2:%[0-9]+]] = OpImageQueryLevels %uint [[image2]]
// CHECK-NEXT:                      OpStore %numLevels [[query_level_2]]
  tex1d.GetDimensions(mipLevel, width, numLevels);

  float f_width, f_height, f_numLevels;
// CHECK:             [[t3_load:%[0-9]+]] = OpLoad [[type_1d_sampled_image]] %tex1d
// CHECK-NEXT:   [[image3:%[0-9]+]] = OpImage [[type_1d_image]] [[t3_load]]
// CHECK-NEXT:   [[query3:%[0-9]+]] = OpImageQuerySizeLod %uint [[image3]] %int_0
// CHECK-NEXT: [[f_query3:%[0-9]+]] = OpConvertUToF %float [[query3]]
// CHECK-NEXT:                      OpStore %f_width [[f_query3]]
  tex1d.GetDimensions(f_width);

// CHECK:             [[t4_load:%[0-9]+]] = OpLoad [[type_1d_sampled_image]] %tex1d
// CHECK-NEXT:   [[image4:%[0-9]+]] = OpImage [[type_1d_image]] [[t4_load]]
// CHECK-NEXT:      [[mip4:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:  [[query4:%[0-9]+]] = OpImageQuerySizeLod %uint [[image4]] [[mip4]]
// CHECK-NEXT: [[f_query4:%[0-9]+]] = OpConvertUToF %float [[query4]]
// CHECK-NEXT:                      OpStore %f_width [[f_query4]]
// CHECK-NEXT: [[query_level_4:%[0-9]+]] = OpImageQueryLevels %uint [[image4]]
// CHECK-NEXT: [[f_query_level_4:%[0-9]+]] = OpConvertUToF %float [[query_level_4]]
// CHECK-NEXT:                      OpStore %f_numLevels [[f_query_level_4]]
  tex1d.GetDimensions(mipLevel, f_width, f_numLevels);

  int i_width, i_height, i_numLevels;
// CHECK:             [[t5_load:%[0-9]+]] = OpLoad [[type_1d_sampled_image]] %tex1d
// CHECK-NEXT:   [[image5:%[0-9]+]] = OpImage [[type_1d_image]] [[t5_load]]
// CHECK-NEXT:   [[query5:%[0-9]+]] = OpImageQuerySizeLod %uint [[image5]] %int_0
// CHECK-NEXT:   [[query5_i:%[0-9]+]] = OpBitcast %int [[query5]]
// CHECK-NEXT:                      OpStore %i_width [[query5_i]]
  tex1d.GetDimensions(i_width);

// CHECK:             [[t6_load:%[0-9]+]] = OpLoad [[type_1d_sampled_image]] %tex1d
// CHECK-NEXT:   [[image6:%[0-9]+]] = OpImage [[type_1d_image]] [[t6_load]]
// CHECK-NEXT:      [[mip6:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:  [[query6:%[0-9]+]] = OpImageQuerySizeLod %uint [[image6]] [[mip6]]
// CHECK-NEXT:   [[query6_i:%[0-9]+]] = OpBitcast %int [[query6]]
// CHECK-NEXT:                      OpStore %i_width [[query6_i]]
// CHECK-NEXT: [[query_level_6:%[0-9]+]] = OpImageQueryLevels %uint [[image6]]
// CHECK-NEXT: [[query_level_6_i:%[0-9]+]] = OpBitcast %int [[query_level_6]]
// CHECK-NEXT:                      OpStore %i_numLevels [[query_level_6_i]]
  tex1d.GetDimensions(mipLevel, i_width, i_numLevels);

#ifdef ERROR
// ERROR: error: no matching member function for call to 'GetDimensions'
  tex1d.GetDimensions(mipLevel, 0, height, numLevels);

// ERROR: error: no matching member function for call to 'GetDimensions'
  tex1d.GetDimensions(width, 20);
#endif
}