// RUN: %dxc -T ps_6_7 -E main -fcgl  %s -spirv | FileCheck %s
// RUN: not %dxc -T ps_6_7 -E main -fcgl  %s -spirv -DERROR 2>&1 | FileCheck %s --check-prefix=CHECK-ERROR

// CHECK: %type_1d_image = OpTypeImage %float 1D 0 0 0 1 Unknown
// CHECK: %type_sampled_image = OpTypeSampledImage %type_1d_image

vk::SampledTexture1D<float4> tex1d;

struct S { int a; };

void main() {
// CHECK: OpStore %pos1 %uint_1
  uint pos1 = 1;

// CHECK: [[pos1:%[a-zA-Z0-9_]+]] = OpLoad %uint %pos1
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1d
// CHECK: [[tex_image:%[a-zA-Z0-9_]+]] = OpImage %type_1d_image [[tex1_load]]
// CHECK: [[fetch_result:%[a-zA-Z0-9_]+]] = OpImageFetch %v4float [[tex_image]] [[pos1]] Lod %uint_0
// CHECK: OpStore %a1 [[fetch_result]]
  float4 a1 = tex1d[pos1];

#ifdef ERROR
  S s = { 1 };
// CHECK-ERROR: error: no viable overloaded operator[]
// CHECK-ERROR: note: candidate function {{.*}} no known conversion from 'S' to 'unsigned int' for 1st argument
  float4 val2 = tex1d[s];

  int2 i2 = int2(1, 2);
// CHECK-ERROR: error: no viable overloaded operator[]
// CHECK-ERROR: note: candidate function {{.*}} no known conversion from '{{int2|vector<int, 2>}}' to 'unsigned int' for 1st argument
  float4 val3 = tex1d[i2];

  int3 i3 = int3(1, 2, 3);
// CHECK-ERROR: error: no viable overloaded operator[]
// CHECK-ERROR: note: candidate function {{.*}} no known conversion from '{{int3|vector<int, 3>}}' to 'unsigned int' for 1st argument
  float4 val4 = tex1d[i3];
#endif
}
