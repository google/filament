// RUN: %dxc -T ps_6_8 -E main -fcgl %s -spirv | FileCheck %s

// CHECK: OpCapability ImageQuery

// CHECK: [[type_cube_image:%[a-zA-Z0-9_]+]] = OpTypeImage %float Cube 0 0 0 1 Unknown
// CHECK: [[type_cube_sampled:%[a-zA-Z0-9_]+]] = OpTypeSampledImage [[type_cube_image]]

vk::SampledTextureCUBE<float4> tex;

void main() {
  uint mip = 1;
  uint w, h, levels;
  float fw, fh, flevels;

// CHECK: [[tex0:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_sampled]] %tex
// CHECK: [[img0:%[a-zA-Z0-9_]+]] = OpImage [[type_cube_image]] [[tex0]]
// CHECK: [[q0:%[a-zA-Z0-9_]+]] = OpImageQuerySizeLod %v2uint [[img0]] %int_0
  tex.GetDimensions(w, h);

// CHECK: [[tex1:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_sampled]] %tex
// CHECK: [[img1:%[a-zA-Z0-9_]+]] = OpImage [[type_cube_image]] [[tex1]]
// CHECK: [[mip_load:%[a-zA-Z0-9_]+]] = OpLoad %uint %mip
// CHECK: [[q1:%[a-zA-Z0-9_]+]] = OpImageQuerySizeLod %v2uint [[img1]] [[mip_load]]
// CHECK: [[q1_levels:%[a-zA-Z0-9_]+]] = OpImageQueryLevels %uint [[img1]]
  tex.GetDimensions(mip, w, h, levels);

// CHECK: [[tex2:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_sampled]] %tex
// CHECK: [[img2:%[a-zA-Z0-9_]+]] = OpImage [[type_cube_image]] [[tex2]]
// CHECK: [[q2:%[a-zA-Z0-9_]+]] = OpImageQuerySizeLod %v2uint [[img2]] %int_0
// CHECK: OpConvertUToF %float
  tex.GetDimensions(fw, fh);

// CHECK: [[tex3:%[a-zA-Z0-9_]+]] = OpLoad [[type_cube_sampled]] %tex
// CHECK: [[img3:%[a-zA-Z0-9_]+]] = OpImage [[type_cube_image]] [[tex3]]
// CHECK: [[mip_load_1:%[a-zA-Z0-9_]+]] = OpLoad %uint %mip
// CHECK: [[q3:%[a-zA-Z0-9_]+]] = OpImageQuerySizeLod %v2uint [[img3]] [[mip_load_1]]
// CHECK: [[q3_levels:%[a-zA-Z0-9_]+]] = OpImageQueryLevels %uint [[img3]]
// CHECK: OpConvertUToF %float
  tex.GetDimensions(mip, fw, fh, flevels);
}
