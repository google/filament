// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s
// RUN: not %dxc -T ps_6_0 -E main -fcgl  %s -spirv -DERROR 2>&1 | FileCheck %s --check-prefix=ERROR

// CHECK: OpCapability ImageQuery

Texture1D        <float> t1;
Texture1DArray   <float> t2;
Texture2D        <float> t3;
Texture2DArray   <float> t4;
Texture3D        <float> t5;
Texture2DMS      <float> t6;
Texture2DMSArray <float> t7;
TextureCube      <float> t8;
TextureCubeArray <float> t9;

void main() {
  uint mipLevel = 1;
  uint width, height, depth, elements, numLevels, numSamples;

// CHECK:        [[t1_0:%[0-9]+]] = OpLoad %type_1d_image %t1
// CHECK-NEXT: [[query0:%[0-9]+]] = OpImageQuerySizeLod %uint [[t1_0]] %int_0
// CHECK-NEXT:                   OpStore %width [[query0]]
  t1.GetDimensions(width);

// CHECK:        [[t1_1:%[0-9]+]] = OpLoad %type_1d_image %t1
// CHECK-NEXT:   [[mip0:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT: [[query1:%[0-9]+]] = OpImageQuerySizeLod %uint [[t1_1]] [[mip0]]
// CHECK-NEXT:                   OpStore %width [[query1]]
// CHECK-NEXT: [[query2:%[0-9]+]] = OpImageQueryLevels %uint [[t1_1]]
// CHECK-NEXT:                   OpStore %numLevels [[query2]]
  t1.GetDimensions(mipLevel, width, numLevels);

// CHECK:          [[t2_0:%[0-9]+]] = OpLoad %type_1d_image_array %t2
// CHECK-NEXT:   [[query3:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[t2_0]] %int_0
// CHECK-NEXT: [[query3_0:%[0-9]+]] = OpCompositeExtract %uint [[query3]] 0
// CHECK-NEXT:                     OpStore %width [[query3_0]]
// CHECK-NEXT: [[query3_1:%[0-9]+]] = OpCompositeExtract %uint [[query3]] 1
// CHECK-NEXT:                     OpStore %elements [[query3_1]]
  t2.GetDimensions(width, elements);

// CHECK:          [[t2_1:%[0-9]+]] = OpLoad %type_1d_image_array %t2
// CHECK-NEXT:     [[mip1:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:   [[query4:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[t2_1]] [[mip1]]
// CHECK-NEXT: [[query4_0:%[0-9]+]] = OpCompositeExtract %uint [[query4]] 0
// CHECK-NEXT:                     OpStore %width [[query4_0]]
// CHECK-NEXT: [[query4_1:%[0-9]+]] = OpCompositeExtract %uint [[query4]] 1
// CHECK-NEXT:                     OpStore %elements [[query4_1]]
// CHECK-NEXT:   [[query5:%[0-9]+]] = OpImageQueryLevels %uint [[t2_1]]
// CHECK-NEXT:                     OpStore %numLevels [[query5]]
  t2.GetDimensions(mipLevel, width, elements, numLevels);

// CHECK:          [[t3_0:%[0-9]+]] = OpLoad %type_2d_image %t3
// CHECK-NEXT:   [[query5_0:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[t3_0]] %int_0
// CHECK-NEXT: [[query5_1:%[0-9]+]] = OpCompositeExtract %uint [[query5_0]] 0
// CHECK-NEXT:                     OpStore %width [[query5_1]]
// CHECK-NEXT: [[query5_2:%[0-9]+]] = OpCompositeExtract %uint [[query5_0]] 1
// CHECK-NEXT:                     OpStore %height [[query5_2]]
  t3.GetDimensions(width, height);
  
// CHECK:          [[t3_1:%[0-9]+]] = OpLoad %type_2d_image %t3
// CHECK-NEXT:     [[mip2:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:   [[query6:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[t3_1]] [[mip2]]
// CHECK-NEXT: [[query6_0:%[0-9]+]] = OpCompositeExtract %uint [[query6]] 0
// CHECK-NEXT:                     OpStore %width [[query6_0]]
// CHECK-NEXT: [[query6_1:%[0-9]+]] = OpCompositeExtract %uint [[query6]] 1
// CHECK-NEXT:                     OpStore %height [[query6_1]]
// CHECK-NEXT:   [[query7:%[0-9]+]] = OpImageQueryLevels %uint [[t3_1]]
// CHECK-NEXT:                     OpStore %numLevels [[query7]]
  t3.GetDimensions(mipLevel, width, height, numLevels);

// CHECK:          [[t4_0:%[0-9]+]] = OpLoad %type_2d_image_array %t4
// CHECK-NEXT:   [[query8:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[t4_0]] %int_0
// CHECK-NEXT: [[query8_0:%[0-9]+]] = OpCompositeExtract %uint [[query8]] 0
// CHECK-NEXT:                     OpStore %width [[query8_0]]
// CHECK-NEXT: [[query8_1:%[0-9]+]] = OpCompositeExtract %uint [[query8]] 1
// CHECK-NEXT:                     OpStore %height [[query8_1]]
// CHECK-NEXT: [[query8_2:%[0-9]+]] = OpCompositeExtract %uint [[query8]] 2
// CHECK-NEXT:                     OpStore %elements [[query8_2]]
  t4.GetDimensions(width, height, elements);
  
// CHECK:          [[t4_1:%[0-9]+]] = OpLoad %type_2d_image_array %t4
// CHECK-NEXT:     [[mip3:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:   [[query9:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[t4_1]] [[mip3]]
// CHECK-NEXT: [[query9_0:%[0-9]+]] = OpCompositeExtract %uint [[query9]] 0
// CHECK-NEXT:                     OpStore %width [[query9_0]]
// CHECK-NEXT: [[query9_1:%[0-9]+]] = OpCompositeExtract %uint [[query9]] 1
// CHECK-NEXT:                     OpStore %height [[query9_1]]
// CHECK-NEXT: [[query9_2:%[0-9]+]] = OpCompositeExtract %uint [[query9]] 2
// CHECK-NEXT:                     OpStore %elements [[query9_2]]
// CHECK-NEXT:  [[query10:%[0-9]+]] = OpImageQueryLevels %uint [[t4_1]]
// CHECK-NEXT:                     OpStore %numLevels [[query10]]
  t4.GetDimensions(mipLevel, width, height, elements, numLevels);

// CHECK:           [[t5_0:%[0-9]+]] = OpLoad %type_3d_image %t5
// CHECK-NEXT:   [[query11:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[t5_0]] %int_0
// CHECK-NEXT: [[query11_0:%[0-9]+]] = OpCompositeExtract %uint [[query11]] 0
// CHECK-NEXT:                      OpStore %width [[query11_0]]
// CHECK-NEXT: [[query11_1:%[0-9]+]] = OpCompositeExtract %uint [[query11]] 1
// CHECK-NEXT:                      OpStore %height [[query11_1]]
// CHECK-NEXT: [[query11_2:%[0-9]+]] = OpCompositeExtract %uint [[query11]] 2
// CHECK-NEXT:                      OpStore %depth [[query11_2]]
  t5.GetDimensions(width, height, depth);

// CHECK:           [[t5_1:%[0-9]+]] = OpLoad %type_3d_image %t5
// CHECK-NEXT:      [[mip4:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:   [[query12:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[t5_1]] [[mip4]]
// CHECK-NEXT: [[query12_0:%[0-9]+]] = OpCompositeExtract %uint [[query12]] 0
// CHECK-NEXT:                      OpStore %width [[query12_0]]
// CHECK-NEXT: [[query12_1:%[0-9]+]] = OpCompositeExtract %uint [[query12]] 1
// CHECK-NEXT:                      OpStore %height [[query12_1]]
// CHECK-NEXT: [[query12_2:%[0-9]+]] = OpCompositeExtract %uint [[query12]] 2
// CHECK-NEXT:                      OpStore %depth [[query12_2]]
// CHECK-NEXT:   [[query13:%[0-9]+]] = OpImageQueryLevels %uint [[t5_1]]
// CHECK-NEXT:                      OpStore %numLevels [[query13]]
  t5.GetDimensions(mipLevel, width, height, depth, numLevels);

// CHECK:             [[t6:%[0-9]+]] = OpLoad %type_2d_image_0 %t6
// CHECK-NEXT:   [[query14:%[0-9]+]] = OpImageQuerySize %v2uint [[t6]]
// CHECK-NEXT: [[query14_0:%[0-9]+]] = OpCompositeExtract %uint [[query14]] 0
// CHECK-NEXT:                      OpStore %width [[query14_0]]
// CHECK-NEXT: [[query14_1:%[0-9]+]] = OpCompositeExtract %uint [[query14]] 1
// CHECK-NEXT:                      OpStore %height [[query14_1]]
// CHECK-NEXT:   [[query15:%[0-9]+]] = OpImageQuerySamples %uint [[t6]]
// CHECK-NEXT:                      OpStore %numSamples [[query15]]
  t6.GetDimensions(width, height, numSamples);

// CHECK:             [[t7:%[0-9]+]] = OpLoad %type_2d_image_array_0 %t7
// CHECK-NEXT:   [[query16:%[0-9]+]] = OpImageQuerySize %v3uint [[t7]]
// CHECK-NEXT: [[query16_0:%[0-9]+]] = OpCompositeExtract %uint [[query16]] 0
// CHECK-NEXT:                      OpStore %width [[query16_0]]
// CHECK-NEXT: [[query16_1:%[0-9]+]] = OpCompositeExtract %uint [[query16]] 1
// CHECK-NEXT:                      OpStore %height [[query16_1]]
// CHECK-NEXT: [[query16_2:%[0-9]+]] = OpCompositeExtract %uint [[query16]] 2
// CHECK-NEXT:                      OpStore %elements [[query16_2]]
// CHECK-NEXT:   [[query17:%[0-9]+]] = OpImageQuerySamples %uint [[t7]]
// CHECK-NEXT:                      OpStore %numSamples [[query17]]
  t7.GetDimensions(width, height, elements, numSamples);

  // Overloads with float output arg.
  float f_width, f_height, f_depth, f_elements, f_numSamples, f_numLevels;

// CHECK:          [[t1_1:%[0-9]+]] = OpLoad %type_1d_image %t1
// CHECK-NEXT:   [[query0_0:%[0-9]+]] = OpImageQuerySizeLod %uint [[t1_1]] %int_0
// CHECK-NEXT: [[f_query0:%[0-9]+]] = OpConvertUToF %float [[query0_0]]
// CHECK-NEXT:                     OpStore %f_width [[f_query0]]
  t1.GetDimensions(f_width);

// CHECK:        [[t1_1_0:%[0-9]+]] = OpLoad %type_1d_image %t1
// CHECK-NEXT:   [[mip0_0:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT: [[query1_0:%[0-9]+]] = OpImageQuerySizeLod %uint [[t1_1_0]] [[mip0_0]]
// CHECK-NEXT: [[f_query1:%[0-9]+]] = OpConvertUToF %float [[query1_0]]
// CHECK-NEXT:                   OpStore %f_width [[f_query1]]
// CHECK-NEXT: [[query2_0:%[0-9]+]] = OpImageQueryLevels %uint [[t1_1_0]]
// CHECK-NEXT: [[f_query2:%[0-9]+]] = OpConvertUToF %float [[query2_0]]
// CHECK-NEXT:                   OpStore %f_numLevels [[f_query2]]
  t1.GetDimensions(mipLevel, f_width, f_numLevels);

// CHECK:          [[t2_1:%[0-9]+]] = OpLoad %type_1d_image_array %t2
// CHECK-NEXT:   [[query3_0:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[t2_1]] %int_0
// CHECK-NEXT: [[query3_1:%[0-9]+]] = OpCompositeExtract %uint [[query3_0]] 0
// CHECK-NEXT: [[f_query3_0:%[0-9]+]] = OpConvertUToF %float [[query3_1]]
// CHECK-NEXT:                     OpStore %f_width [[f_query3_0]]
// CHECK-NEXT: [[query3_1_0:%[0-9]+]] = OpCompositeExtract %uint [[query3_0]] 1
// CHECK-NEXT: [[f_query3_1:%[0-9]+]] = OpConvertUToF %float [[query3_1_0]]
// CHECK-NEXT:                     OpStore %f_elements [[f_query3_1]]
  t2.GetDimensions(f_width, f_elements);

// CHECK:          [[t2_1_0:%[0-9]+]] = OpLoad %type_1d_image_array %t2
// CHECK-NEXT:     [[mip1_0:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:   [[query4_0:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[t2_1_0]] [[mip1_0]]
// CHECK-NEXT: [[query4_1:%[0-9]+]] = OpCompositeExtract %uint [[query4_0]] 0
// CHECK-NEXT: [[f_query4_0:%[0-9]+]] = OpConvertUToF %float [[query4_1]]
// CHECK-NEXT:                     OpStore %f_width [[f_query4_0]]
// CHECK-NEXT: [[query4_1_0:%[0-9]+]] = OpCompositeExtract %uint [[query4_0]] 1
// CHECK-NEXT: [[f_query4_1:%[0-9]+]] = OpConvertUToF %float [[query4_1_0]]
// CHECK-NEXT:                     OpStore %f_elements [[f_query4_1]]
// CHECK-NEXT:   [[query5_1:%[0-9]+]] = OpImageQueryLevels %uint [[t2_1_0]]
// CHECK-NEXT: [[f_query5:%[0-9]+]] = OpConvertUToF %float [[query5_1]]
// CHECK-NEXT:                     OpStore %f_numLevels [[f_query5]]
  t2.GetDimensions(mipLevel, f_width, f_elements, f_numLevels);

// CHECK:          [[t3_1:%[0-9]+]] = OpLoad %type_2d_image %t3
// CHECK-NEXT:   [[query5_2:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[t3_1]] %int_0
// CHECK-NEXT: [[query5_1:%[0-9]+]] = OpCompositeExtract %uint [[query5_2]] 0
// CHECK-NEXT: [[f_query5_0:%[0-9]+]] = OpConvertUToF %float [[query5_1]]
// CHECK-NEXT:                     OpStore %f_width [[f_query5_0]]
// CHECK-NEXT: [[query5_1_0:%[0-9]+]] = OpCompositeExtract %uint [[query5_2]] 1
// CHECK-NEXT: [[f_query5_1:%[0-9]+]] = OpConvertUToF %float [[query5_1_0]]
// CHECK-NEXT:                     OpStore %f_height [[f_query5_1]]
  t3.GetDimensions(f_width, f_height);
  
// CHECK:          [[t3_1_0:%[0-9]+]] = OpLoad %type_2d_image %t3
// CHECK-NEXT:     [[mip2_0:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:   [[query6_0:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[t3_1_0]] [[mip2_0]]
// CHECK-NEXT: [[query6_1:%[0-9]+]] = OpCompositeExtract %uint [[query6_0]] 0
// CHECK-NEXT: [[f_query6_0:%[0-9]+]] = OpConvertUToF %float [[query6_1]]
// CHECK-NEXT:                     OpStore %f_width [[f_query6_0]]
// CHECK-NEXT: [[query6_1_0:%[0-9]+]] = OpCompositeExtract %uint [[query6_0]] 1
// CHECK-NEXT: [[f_query6_1:%[0-9]+]] = OpConvertUToF %float [[query6_1_0]]
// CHECK-NEXT:                     OpStore %f_height [[f_query6_1]]
// CHECK-NEXT:   [[query7_0:%[0-9]+]] = OpImageQueryLevels %uint [[t3_1_0]]
// CHECK-NEXT: [[f_query7:%[0-9]+]] = OpConvertUToF %float [[query7_0]]
// CHECK-NEXT:                     OpStore %f_numLevels [[f_query7]]
  t3.GetDimensions(mipLevel, f_width, f_height, f_numLevels);

// CHECK:          [[t4_1:%[0-9]+]] = OpLoad %type_2d_image_array %t4
// CHECK-NEXT:   [[query8_0:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[t4_1]] %int_0
// CHECK-NEXT: [[query8_1:%[0-9]+]] = OpCompositeExtract %uint [[query8_0]] 0
// CHECK-NEXT: [[f_query8_0:%[0-9]+]] = OpConvertUToF %float [[query8_1]]
// CHECK-NEXT:                     OpStore %f_width [[f_query8_0]]
// CHECK-NEXT: [[query8_1_0:%[0-9]+]] = OpCompositeExtract %uint [[query8_0]] 1
// CHECK-NEXT: [[f_query8_1:%[0-9]+]] = OpConvertUToF %float [[query8_1_0]]
// CHECK-NEXT:                     OpStore %f_height [[f_query8_1]]
// CHECK-NEXT: [[query8_2_0:%[0-9]+]] = OpCompositeExtract %uint [[query8_0]] 2
// CHECK-NEXT: [[f_query8_2:%[0-9]+]] = OpConvertUToF %float [[query8_2_0]]
// CHECK-NEXT:                     OpStore %f_elements [[f_query8_2]]
  t4.GetDimensions(f_width, f_height, f_elements);
  
// CHECK:          [[t4_1_0:%[0-9]+]] = OpLoad %type_2d_image_array %t4
// CHECK-NEXT:     [[mip3_0:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:   [[query9_0:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[t4_1_0]] [[mip3_0]]
// CHECK-NEXT: [[query9_1:%[0-9]+]] = OpCompositeExtract %uint [[query9_0]] 0
// CHECK-NEXT: [[f_query9_0:%[0-9]+]] = OpConvertUToF %float [[query9_1]]
// CHECK-NEXT:                     OpStore %f_width [[f_query9_0]]
// CHECK-NEXT: [[query9_1_0:%[0-9]+]] = OpCompositeExtract %uint [[query9_0]] 1
// CHECK-NEXT: [[f_query9_1:%[0-9]+]] = OpConvertUToF %float [[query9_1_0]]
// CHECK-NEXT:                     OpStore %f_height [[f_query9_1]]
// CHECK-NEXT: [[query9_2_0:%[0-9]+]] = OpCompositeExtract %uint [[query9_0]] 2
// CHECK-NEXT: [[f_query9_2:%[0-9]+]] = OpConvertUToF %float [[query9_2_0]]
// CHECK-NEXT:                     OpStore %f_elements [[f_query9_2]]
// CHECK-NEXT:  [[query10_0:%[0-9]+]] = OpImageQueryLevels %uint [[t4_1_0]]
// CHECK-NEXT: [[f_query10:%[0-9]+]] = OpConvertUToF %float [[query10_0]]
// CHECK-NEXT:                     OpStore %f_numLevels [[f_query10]]
  t4.GetDimensions(mipLevel, f_width, f_height, f_elements, f_numLevels);

// CHECK:           [[t5_1:%[0-9]+]] = OpLoad %type_3d_image %t5
// CHECK-NEXT:   [[query11_0:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[t5_1]] %int_0
// CHECK-NEXT: [[query11_1:%[0-9]+]] = OpCompositeExtract %uint [[query11_0]] 0
// CHECK-NEXT: [[f_query11_0:%[0-9]+]] = OpConvertUToF %float [[query11_1]]
// CHECK-NEXT:                      OpStore %f_width [[f_query11_0]]
// CHECK-NEXT: [[query11_1_0:%[0-9]+]] = OpCompositeExtract %uint [[query11_0]] 1
// CHECK-NEXT: [[f_query11_1:%[0-9]+]] = OpConvertUToF %float [[query11_1_0]]
// CHECK-NEXT:                      OpStore %f_height [[f_query11_1]]
// CHECK-NEXT: [[query11_2_0:%[0-9]+]] = OpCompositeExtract %uint [[query11_0]] 2
// CHECK-NEXT: [[f_query11_2:%[0-9]+]] = OpConvertUToF %float [[query11_2_0]]
// CHECK-NEXT:                      OpStore %f_depth [[f_query11_2]]
  t5.GetDimensions(f_width, f_height, f_depth);

// CHECK:           [[t5_1_0:%[0-9]+]] = OpLoad %type_3d_image %t5
// CHECK-NEXT:      [[mip4_0:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:   [[query12_0:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[t5_1_0]] [[mip4_0]]
// CHECK-NEXT: [[query12_1:%[0-9]+]] = OpCompositeExtract %uint [[query12_0]] 0
// CHECK-NEXT: [[f_query12_0:%[0-9]+]] = OpConvertUToF %float [[query12_1]]
// CHECK-NEXT:                      OpStore %f_width [[f_query12_0]]
// CHECK-NEXT: [[query12_1_0:%[0-9]+]] = OpCompositeExtract %uint [[query12_0]] 1
// CHECK-NEXT: [[f_query12_1:%[0-9]+]] = OpConvertUToF %float [[query12_1_0]]
// CHECK-NEXT:                      OpStore %f_height [[f_query12_1]]
// CHECK-NEXT: [[query12_2_0:%[0-9]+]] = OpCompositeExtract %uint [[query12_0]] 2
// CHECK-NEXT: [[f_query12_2:%[0-9]+]] = OpConvertUToF %float [[query12_2_0]]
// CHECK-NEXT:                      OpStore %f_depth [[f_query12_2]]
// CHECK-NEXT:   [[query13_0:%[0-9]+]] = OpImageQueryLevels %uint [[t5_1_0]]
// CHECK-NEXT: [[f_query13:%[0-9]+]] = OpConvertUToF %float [[query13_0]]
// CHECK-NEXT:                      OpStore %f_numLevels [[f_query13]]
  t5.GetDimensions(mipLevel, f_width, f_height, f_depth, f_numLevels);

// CHECK:             [[t6_0:%[0-9]+]] = OpLoad %type_2d_image_0 %t6
// CHECK-NEXT:   [[query14_0:%[0-9]+]] = OpImageQuerySize %v2uint [[t6_0]]
// CHECK-NEXT: [[query14_1:%[0-9]+]] = OpCompositeExtract %uint [[query14_0]] 0
// CHECK-NEXT: [[f_query14_0:%[0-9]+]] = OpConvertUToF %float [[query14_1]]
// CHECK-NEXT:                      OpStore %f_width [[f_query14_0]]
// CHECK-NEXT: [[query14_1_0:%[0-9]+]] = OpCompositeExtract %uint [[query14_0]] 1
// CHECK-NEXT: [[f_query14_1:%[0-9]+]] = OpConvertUToF %float [[query14_1_0]]
// CHECK-NEXT:                      OpStore %f_height [[f_query14_1]]
// CHECK-NEXT:   [[query15_0:%[0-9]+]] = OpImageQuerySamples %uint [[t6_0]]
// CHECK-NEXT: [[f_query15:%[0-9]+]] = OpConvertUToF %float [[query15_0]]
// CHECK-NEXT:                      OpStore %f_numSamples [[f_query15]]
  t6.GetDimensions(f_width, f_height, f_numSamples);

// CHECK:             [[t7_0:%[0-9]+]] = OpLoad %type_2d_image_array_0 %t7
// CHECK-NEXT:   [[query16_0:%[0-9]+]] = OpImageQuerySize %v3uint [[t7_0]]
// CHECK-NEXT: [[query16_1:%[0-9]+]] = OpCompositeExtract %uint [[query16_0]] 0
// CHECK-NEXT: [[f_query16_0:%[0-9]+]] = OpConvertUToF %float [[query16_1]]
// CHECK-NEXT:                      OpStore %f_width [[f_query16_0]]
// CHECK-NEXT: [[query16_1_0:%[0-9]+]] = OpCompositeExtract %uint [[query16_0]] 1
// CHECK-NEXT: [[f_query16_1:%[0-9]+]] = OpConvertUToF %float [[query16_1_0]]
// CHECK-NEXT:                      OpStore %f_height [[f_query16_1]]
// CHECK-NEXT: [[query16_2_0:%[0-9]+]] = OpCompositeExtract %uint [[query16_0]] 2
// CHECK-NEXT: [[f_query16_2:%[0-9]+]] = OpConvertUToF %float [[query16_2_0]]
// CHECK-NEXT:                      OpStore %f_elements [[f_query16_2]]
// CHECK-NEXT:   [[query17_0:%[0-9]+]] = OpImageQuerySamples %uint [[t7_0]]
// CHECK-NEXT: [[f_query17:%[0-9]+]] = OpConvertUToF %float [[query17_0]]
// CHECK-NEXT:                      OpStore %f_numSamples [[f_query17]]
  t7.GetDimensions(f_width, f_height, f_elements, f_numSamples);

// CHECK:             [[t8:%[0-9]+]] = OpLoad %type_cube_image %t8
// CHECK-NEXT:   [[query18:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[t8]] %int_0
// CHECK-NEXT: [[query18_0:%[0-9]+]] = OpCompositeExtract %uint [[query18]] 0
// CHECK-NEXT:                      OpStore %width [[query18_0]]
// CHECK-NEXT: [[query18_1:%[0-9]+]] = OpCompositeExtract %uint [[query18]] 1
// CHECK-NEXT:                      OpStore %height [[query18_1]]
  t8.GetDimensions(width, height);
  
// CHECK:             [[t8_0:%[0-9]+]] = OpLoad %type_cube_image %t8
// CHECK-NEXT:       [[mip:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:   [[query19:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[t8_0]] [[mip]]
// CHECK-NEXT: [[query19_0:%[0-9]+]] = OpCompositeExtract %uint [[query19]] 0
// CHECK-NEXT:                      OpStore %width [[query19_0]]
// CHECK-NEXT: [[query19_1:%[0-9]+]] = OpCompositeExtract %uint [[query19]] 1
// CHECK-NEXT:                      OpStore %height [[query19_1]]
// CHECK-NEXT:   [[query20:%[0-9]+]] = OpImageQueryLevels %uint [[t8_0]]
// CHECK-NEXT:                      OpStore %numLevels [[query20]]
  t8.GetDimensions(mipLevel, width, height, numLevels);

// CHECK:             [[t9:%[0-9]+]] = OpLoad %type_cube_image_array %t9
// CHECK-NEXT:   [[query21:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[t9]] %int_0
// CHECK-NEXT: [[query21_0:%[0-9]+]] = OpCompositeExtract %uint [[query21]] 0
// CHECK-NEXT:                      OpStore %width [[query21_0]]
// CHECK-NEXT: [[query21_1:%[0-9]+]] = OpCompositeExtract %uint [[query21]] 1
// CHECK-NEXT:                      OpStore %height [[query21_1]]
// CHECK-NEXT: [[query21_2:%[0-9]+]] = OpCompositeExtract %uint [[query21]] 2
// CHECK-NEXT:                      OpStore %elements [[query21_2]]
  t9.GetDimensions(width, height, elements);
  
// CHECK:             [[t9_0:%[0-9]+]] = OpLoad %type_cube_image_array %t9
// CHECK-NEXT:       [[mip_0:%[0-9]+]] = OpLoad %uint %mipLevel
// CHECK-NEXT:   [[query22:%[0-9]+]] = OpImageQuerySizeLod %v3uint [[t9_0]] [[mip_0]]
// CHECK-NEXT: [[query22_0:%[0-9]+]] = OpCompositeExtract %uint [[query22]] 0
// CHECK-NEXT:                      OpStore %width [[query22_0]]
// CHECK-NEXT: [[query22_1:%[0-9]+]] = OpCompositeExtract %uint [[query22]] 1
// CHECK-NEXT:                      OpStore %height [[query22_1]]
// CHECK-NEXT: [[query22_2:%[0-9]+]] = OpCompositeExtract %uint [[query22]] 2
// CHECK-NEXT:                      OpStore %elements [[query22_2]]
// CHECK-NEXT:   [[query23:%[0-9]+]] = OpImageQueryLevels %uint [[t9_0]]
// CHECK-NEXT:                      OpStore %numLevels [[query23]]
  t9.GetDimensions(mipLevel, width, height, elements, numLevels);

// Try with signed integer as argument.

// CHECK:                [[t3:%[0-9]+]] = OpLoad %type_2d_image %t3
// CHECK-NEXT:        [[query:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[t3]] %int_0
// CHECK-NEXT: [[query_0_uint:%[0-9]+]] = OpCompositeExtract %uint [[query]] 0
// CHECK-NEXT:  [[query_0_int:%[0-9]+]] = OpBitcast %int [[query_0_uint]]
// CHECK-NEXT:                         OpStore %signedWidth [[query_0_int]]
// CHECK-NEXT: [[query_1_uint:%[0-9]+]] = OpCompositeExtract %uint [[query]] 1
// CHECK-NEXT:  [[query_1_int:%[0-9]+]] = OpBitcast %int [[query_1_uint]]
// CHECK-NEXT:                         OpStore %signedHeight [[query_1_int]]
  int signedMipLevel, signedWidth, signedHeight, signedNumLevels;
  t3.GetDimensions(signedWidth, signedHeight);

// CHECK-NEXT:                [[t3_0:%[0-9]+]] = OpLoad %type_2d_image %t3
// CHECK-NEXT:    [[signedMipLevel:%[0-9]+]] = OpLoad %int %signedMipLevel
// CHECK-NEXT:  [[unsignedMipLevel:%[0-9]+]] = OpBitcast %uint [[signedMipLevel]]
// CHECK-NEXT:             [[query_0:%[0-9]+]] = OpImageQuerySizeLod %v2uint [[t3_0]] [[unsignedMipLevel]]
// CHECK-NEXT:      [[query_0_uint_0:%[0-9]+]] = OpCompositeExtract %uint [[query_0]] 0
// CHECK-NEXT:       [[query_0_int_0:%[0-9]+]] = OpBitcast %int [[query_0_uint_0]]
// CHECK-NEXT:                              OpStore %signedWidth [[query_0_int_0]]
// CHECK-NEXT:      [[query_1_uint_0:%[0-9]+]] = OpCompositeExtract %uint [[query_0]] 1
// CHECK-NEXT:       [[query_1_int_0:%[0-9]+]] = OpBitcast %int [[query_1_uint_0]]
// CHECK-NEXT:                              OpStore %signedHeight [[query_1_int_0]]
// CHECK-NEXT: [[query_levels_uint:%[0-9]+]] = OpImageQueryLevels %uint [[t3_0]]
// CHECK-NEXT:  [[query_levels_int:%[0-9]+]] = OpBitcast %int [[query_levels_uint]]
// CHECK-NEXT:                              OpStore %signedNumLevels [[query_levels_int]]
  t3.GetDimensions(signedMipLevel, signedWidth, signedHeight, signedNumLevels);

#ifdef ERROR
// ERROR: 367:30: error: Output argument must be an l-value
  t9.GetDimensions(mipLevel, 0, height, elements, numLevels);

// ERROR: 370:35: error: Output argument must be an l-value
  t9.GetDimensions(width, height, 20);
#endif
}
