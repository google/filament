// RUN: %dxc -T ps_6_7 -E main -fcgl  %s -spirv | FileCheck %s
// RUN: not %dxc -T ps_6_7 -E main -fcgl  %s -spirv -DERROR 2>&1 | FileCheck %s --check-prefix=CHECK-ERROR

// CHECK: %type_3d_image = OpTypeImage %float 3D 0 0 0 1 Unknown
// CHECK: %type_sampled_image = OpTypeSampledImage %type_3d_image

vk::SampledTexture3D<float4> tex3d;

struct S { int a; };

void main() {
  uint3 pos1 = uint3(1, 2, 3);

// CHECK: [[pos1:%[a-zA-Z0-9_]+]] = OpLoad %v3uint %pos1
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex3d
// CHECK: [[tex_image:%[a-zA-Z0-9_]+]] = OpImage %type_3d_image [[tex1_load]]
// CHECK: [[fetch_result:%[a-zA-Z0-9_]+]] = OpImageFetch %v4float [[tex_image]] [[pos1]] Lod %uint_0
// CHECK: OpStore %a1 [[fetch_result]]
  float4 a1 = tex3d[pos1];

#ifdef ERROR
  S s = { 1 };
// CHECK-ERROR: error: no viable overloaded operator[]
  float4 val2 = tex3d[s];

  int2 i2 = int2(1, 2);
// CHECK-ERROR: error: no viable overloaded operator[]
  float4 val3 = tex3d[i2];

  int i1 = 1;
// CHECK-ERROR: error: no viable overloaded operator[]
  float4 val4 = tex3d[i1];
#endif
}
