// RUN: %dxc -T ps_6_7 -E main -fcgl  %s -spirv | FileCheck %s
// RUN: not %dxc -T ps_6_7 -E main -fcgl  %s -spirv -DERROR 2>&1 | FileCheck %s --check-prefix=CHECK-ERROR

// CHECK: %type_1d_image_array = OpTypeImage %float 1D 0 1 0 1 Unknown
// CHECK: %type_sampled_image = OpTypeSampledImage %type_1d_image_array

vk::SampledTexture1DArray<float4> tex1dArray;

struct S { int a; };

void main() {
// CHECK: OpStore %pos1 [[pos1_init:%[a-zA-Z0-9_]+]]

  uint2 pos1 = uint2(1,2);
// CHECK: [[pos1:%[a-zA-Z0-9_]+]] = OpLoad %v2uint %pos1
// CHECK: [[tex1_load:%[a-zA-Z0-9_]+]] = OpLoad %type_sampled_image %tex1dArray
// CHECK: [[tex_image:%[a-zA-Z0-9_]+]] = OpImage %type_1d_image_array [[tex1_load]]
// CHECK: [[fetch_result:%[a-zA-Z0-9_]+]] = OpImageFetch %v4float [[tex_image]] [[pos1]] Lod %uint_0
// CHECK: OpStore %a1 [[fetch_result]]

  float4 a1 = tex1dArray[pos1];

#ifdef ERROR
  S s = { 1 };
// CHECK-ERROR: error: no viable overloaded operator[]
// CHECK-ERROR: note: candidate function {{.*}} no known conversion from 'S' to 'vector<uint, 2>' for 1st argument
  float4 val2 = tex1dArray[s];

  uint pos2 = 2;
// CHECK-ERROR: error: no viable overloaded operator[]
// CHECK-ERROR: note: candidate function {{.*}} no known conversion from '{{uint|unsigned int}}' to 'vector<uint, 2>' for 1st argument
  float4 val3 = tex1dArray[pos2];

  int3 i3 = int3(1, 2, 3);
// CHECK-ERROR: error: no viable overloaded operator[]
// CHECK-ERROR: note: candidate function {{.*}} no known conversion from '{{int3|vector<int, 3>}}' to 'vector<uint, 2>' for 1st argument
  float4 val4 = tex1dArray[i3];
#endif
}
