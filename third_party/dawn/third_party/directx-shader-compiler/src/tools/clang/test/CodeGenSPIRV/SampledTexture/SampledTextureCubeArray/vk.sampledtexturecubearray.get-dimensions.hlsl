// RUN: %dxc -T ps_6_8 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpCapability ImageQuery

// CHECK: [[type_cube_array_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float Cube 0 1 0 1 Unknown
// CHECK: [[type_cube_array_sampled:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_cube_array_image]]

vk::SampledTextureCUBEArray<float4> tex;

void main() {
  uint mip = 1;
  uint w, h, e, levels;
  float fw, fh, fe, flevels;

// CHECK: [[tex0:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_array_sampled]] %tex
// CHECK: [[img0:%[a-zA-Z0-9_]+]] = OpImage [[type_cube_array_image]] [[tex0]]
// CHECK: [[q0:%[a-zA-Z0-9_]+]] = OpImageQuerySizeLod %v3uint [[img0]] %int_0
  tex.GetDimensions(w, h, e);

// CHECK: [[tex1:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_array_sampled]] %tex
// CHECK: [[img1:%[a-zA-Z0-9_]+]] = OpImage [[type_cube_array_image]] [[tex1]]
// CHECK: [[mip_load:%[a-zA-Z0-9_]+]] = OpLoad %uint %mip
// CHECK: [[q1:%[a-zA-Z0-9_]+]] = OpImageQuerySizeLod %v3uint [[img1]] [[mip_load]]
// CHECK: [[q1_levels:%[a-zA-Z0-9_]+]] = OpImageQueryLevels %uint [[img1]]
  tex.GetDimensions(mip, w, h, e, levels);

// CHECK: [[tex2:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_array_sampled]] %tex
// CHECK: [[img2:%[a-zA-Z0-9_]+]] = OpImage [[type_cube_array_image]] [[tex2]]
// CHECK: [[q2:%[a-zA-Z0-9_]+]] = OpImageQuerySizeLod %v3uint [[img2]] %int_0
// CHECK: OpConvertUToF %float
  tex.GetDimensions(fw, fh, fe);

// CHECK: [[tex3:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_array_sampled]] %tex
// CHECK: [[img3:%[a-zA-Z0-9_]+]] = OpImage [[type_cube_array_image]] [[tex3]]
// CHECK: [[mip_load_1:%[a-zA-Z0-9_]+]] = OpLoad %uint %mip
// CHECK: [[q3:%[a-zA-Z0-9_]+]] = OpImageQuerySizeLod %v3uint [[img3]] [[mip_load_1]]
// CHECK: [[q3_levels:%[a-zA-Z0-9_]+]] = OpImageQueryLevels %uint [[img3]]
// CHECK: OpConvertUToF %float
  tex.GetDimensions(mip, fw, fh, fe, flevels);
}
