// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s
// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv -DERROR 2>&1 | FileCheck %s --check-prefix=ERROR

// CHECK: OpCapability ImageQuery

// CHECK: [[type_3d_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float 3D 0 0 0 1 Unknown
// CHECK: [[type_3d_sampled_image:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_3d_image]]

vk::SampledTexture3D<float4> tex3d;

void main() {
  uint mipLevel = 1;
  uint width, height, depth, numLevels;

// CHECK:             [[t1_load:%[0-9]+]] = OpLoad [[type_3d_sampled_image]] %tex3d
// CHECK-NEXT:   [[image1:%[0-9]+]] = OpImage [[type_3d_image]] [[t1_load]]
// CHECK-NEXT:   [[query1:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[image1]] %int_0
// CHECK-NEXT: [[query1_0:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 0
// CHECK-NEXT:                      OpStore %width [[query1_0]]
// CHECK-NEXT: [[query1_1:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 1
// CHECK-NEXT:                      OpStore %height [[query1_1]]
// CHECK-NEXT: [[query1_2:%[0-9]+]] = OpCompositeExtract %uint [[query1]] 2
// CHECK-NEXT:                      OpStore %depth [[query1_2]]
  tex3d.GetDimensions(width, height, depth);

// CHECK:             [[t2_load:%[0-9]+]] = OpLoad [[type_3d_sampled_image]] %tex3d
// CHECK-NEXT:   [[image2:%[0-9]+]] = OpImage [[type_3d_image]] [[t2_load]]
// CHECK-NEXT:       [[mip:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:   [[query2:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[image2]] [[mip]]
// CHECK-NEXT: [[query2_0:%[0-9]+]] = OpCompositeExtract %uint [[query2]] 0
// CHECK-NEXT:                      OpStore %width [[query2_0]]
// CHECK-NEXT: [[query2_1:%[0-9]+]] = OpCompositeExtract %uint [[query2]] 1
// CHECK-NEXT:                      OpStore %height [[query2_1]]
// CHECK-NEXT: [[query2_2:%[0-9]+]] = OpCompositeExtract %uint [[query2]] 2
// CHECK-NEXT:                      OpStore %depth [[query2_2]]
// CHECK-NEXT: [[query_level_2:%[0-9]+]] = OpImageQueryLevels %uint [[image2]]
// CHECK-NEXT:                      OpStore %numLevels [[query_level_2]]
  tex3d.GetDimensions(mipLevel, width, height, depth, numLevels);

  float f_width, f_height, f_depth, f_numLevels;
// CHECK:             [[t3_load:%[0-9]+]] = OpLoad [[type_3d_sampled_image]] %tex3d
// CHECK-NEXT:   [[image3:%[0-9]+]] = OpImage [[type_3d_image]] [[t3_load]]
// CHECK-NEXT:   [[query3:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[image3]] %int_0
// CHECK-NEXT: [[query3_0:%[0-9]+]] = OpCompositeExtract %uint [[query3]] 0
// CHECK-NEXT: [[f_query3_0:%[0-9]+]] = OpConvertUToF %float [[query3_0]]
// CHECK-NEXT:                      OpStore %f_width [[f_query3_0]]
// CHECK-NEXT: [[query3_1:%[0-9]+]] = OpCompositeExtract %uint [[query3]] 1
// CHECK-NEXT: [[f_query3_1:%[0-9]+]] = OpConvertUToF %float [[query3_1]]
// CHECK-NEXT:                      OpStore %f_height [[f_query3_1]]
// CHECK-NEXT: [[query3_2:%[0-9]+]] = OpCompositeExtract %uint [[query3]] 2
// CHECK-NEXT: [[f_query3_2:%[0-9]+]] = OpConvertUToF %float [[query3_2]]
// CHECK-NEXT:                      OpStore %f_depth [[f_query3_2]]
  tex3d.GetDimensions(f_width, f_height, f_depth);

// CHECK:             [[t4_load:%[0-9]+]] = OpLoad [[type_3d_sampled_image]] %tex3d
// CHECK-NEXT:   [[image4:%[0-9]+]] = OpImage [[type_3d_image]] [[t4_load]]
// CHECK-NEXT:      [[mip4:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:  [[query4:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[image4]] [[mip4]]
// CHECK-NEXT: [[query4_0:%[0-9]+]] = OpCompositeExtract %uint [[query4]] 0
// CHECK-NEXT: [[f_query4_0:%[0-9]+]] = OpConvertUToF %float [[query4_0]]
// CHECK-NEXT:                      OpStore %f_width [[f_query4_0]]
// CHECK-NEXT: [[query4_1:%[0-9]+]] = OpCompositeExtract %uint [[query4]] 1
// CHECK-NEXT: [[f_query4_1:%[0-9]+]] = OpConvertUToF %float [[query4_1]]
// CHECK-NEXT:                      OpStore %f_height [[f_query4_1]]
// CHECK-NEXT: [[query4_2:%[0-9]+]] = OpCompositeExtract %uint [[query4]] 2
// CHECK-NEXT: [[f_query4_2:%[0-9]+]] = OpConvertUToF %float [[query4_2]]
// CHECK-NEXT:                      OpStore %f_depth [[f_query4_2]]
// CHECK-NEXT: [[query_level_4:%[0-9]+]] = OpImageQueryLevels %uint [[image4]]
// CHECK-NEXT: [[f_query_level_4:%[0-9]+]] = OpConvertUToF %float [[query_level_4]]
// CHECK-NEXT:                      OpStore %f_numLevels [[f_query_level_4]]
  tex3d.GetDimensions(mipLevel, f_width, f_height, f_depth, f_numLevels);

  int i_width, i_height, i_depth, i_numLevels;
// CHECK:             [[t5_load:%[0-9]+]] = OpLoad [[type_3d_sampled_image]] %tex3d
// CHECK-NEXT:   [[image5:%[0-9]+]] = OpImage [[type_3d_image]] [[t5_load]]
// CHECK-NEXT:   [[query5:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[image5]] %int_0
// CHECK-NEXT: [[query5_0:%[0-9]+]] = OpCompositeExtract %uint [[query5]] 0
// CHECK-NEXT:  [[query5_0_i:%[0-9]+]] = OpBitcast %int [[query5_0]]
// CHECK-NEXT:                      OpStore %i_width [[query5_0_i]]
// CHECK-NEXT: [[query5_1:%[0-9]+]] = OpCompositeExtract %uint [[query5]] 1
// CHECK-NEXT:  [[query5_1_i:%[0-9]+]] = OpBitcast %int [[query5_1]]
// CHECK-NEXT:                      OpStore %i_height [[query5_1_i]]
// CHECK-NEXT: [[query5_2:%[0-9]+]] = OpCompositeExtract %uint [[query5]] 2
// CHECK-NEXT:  [[query5_2_i:%[0-9]+]] = OpBitcast %int [[query5_2]]
// CHECK-NEXT:                      OpStore %i_depth [[query5_2_i]]
  tex3d.GetDimensions(i_width, i_height, i_depth);

// CHECK:             [[t6_load:%[0-9]+]] = OpLoad [[type_3d_sampled_image]] %tex3d
// CHECK-NEXT:   [[image6:%[0-9]+]] = OpImage [[type_3d_image]] [[t6_load]]
// CHECK-NEXT:      [[mip6:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:  [[query6:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[image6]] [[mip6]]
// CHECK-NEXT: [[query6_0:%[0-9]+]] = OpCompositeExtract %uint [[query6]] 0
// CHECK-NEXT:  [[query6_0_i:%[0-9]+]] = OpBitcast %int [[query6_0]]
// CHECK-NEXT:                      OpStore %i_width [[query6_0_i]]
// CHECK-NEXT: [[query6_1:%[0-9]+]] = OpCompositeExtract %uint [[query6]] 1
// CHECK-NEXT:  [[query6_1_i:%[0-9]+]] = OpBitcast %int [[query6_1]]
// CHECK-NEXT:                      OpStore %i_height [[query6_1_i]]
// CHECK-NEXT: [[query6_2:%[0-9]+]] = OpCompositeExtract %uint [[query6]] 2
// CHECK-NEXT:  [[query6_2_i:%[0-9]+]] = OpBitcast %int [[query6_2]]
// CHECK-NEXT:                      OpStore %i_depth [[query6_2_i]]
// CHECK-NEXT: [[query_level_6:%[0-9]+]] = OpImageQueryLevels %uint [[image6]]
// CHECK-NEXT: [[query_level_6_i:%[0-9]+]] = OpBitcast %int [[query_level_6]]
// CHECK-NEXT:                      OpStore %i_numLevels [[query_level_6_i]]
  tex3d.GetDimensions(mipLevel, i_width, i_height, i_depth, i_numLevels);

#ifdef ERROR
// ERROR: error: Output argument must be an l-value
  tex3d.GetDimensions(mipLevel, 0, height, depth, numLevels);

// ERROR: error: Output argument must be an l-value
  tex3d.GetDimensions(width, 20, depth);
#endif
}
