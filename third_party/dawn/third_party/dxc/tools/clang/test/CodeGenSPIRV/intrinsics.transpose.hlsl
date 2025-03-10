// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
  float2x3 m = { {1,2,3} , {4,5,6} };

// CHECK:      [[m:%[0-9]+]] = OpLoad %mat2v3float %m
// CHECK-NEXT:   {{%[0-9]+}} = OpTranspose %mat3v2float [[m]]
  float3x2 n = transpose(m);

// CHECK:        [[p:%[0-9]+]] = OpLoad %_arr_v3int_uint_2 %p
// CHECK-NEXT: [[p00:%[0-9]+]] = OpCompositeExtract %int [[p]] 0 0
// CHECK-NEXT: [[p01:%[0-9]+]] = OpCompositeExtract %int [[p]] 0 1
// CHECK-NEXT: [[p02:%[0-9]+]] = OpCompositeExtract %int [[p]] 0 2
// CHECK-NEXT: [[p10:%[0-9]+]] = OpCompositeExtract %int [[p]] 1 0
// CHECK-NEXT: [[p11:%[0-9]+]] = OpCompositeExtract %int [[p]] 1 1
// CHECK-NEXT: [[p12:%[0-9]+]] = OpCompositeExtract %int [[p]] 1 2
// CHECK-NEXT: [[pt0:%[0-9]+]] = OpCompositeConstruct %v2int [[p00]] [[p10]]
// CHECK-NEXT: [[pt1:%[0-9]+]] = OpCompositeConstruct %v2int [[p01]] [[p11]]
// CHECK-NEXT: [[pt2:%[0-9]+]] = OpCompositeConstruct %v2int [[p02]] [[p12]]
// CHECK-NEXT:  [[pt:%[0-9]+]] = OpCompositeConstruct %_arr_v2int_uint_3 [[pt0]] [[pt1]] [[pt2]]
// CHECK-NEXT:                OpStore %pt [[pt]]
  int2x3 p;
  int3x2 pt = transpose(p);

// CHECK:        [[q:%[0-9]+]] = OpLoad %_arr_v2bool_uint_3 %q
// CHECK-NEXT: [[q00:%[0-9]+]] = OpCompositeExtract %bool [[q]] 0 0
// CHECK-NEXT: [[q01:%[0-9]+]] = OpCompositeExtract %bool [[q]] 0 1
// CHECK-NEXT: [[q10:%[0-9]+]] = OpCompositeExtract %bool [[q]] 1 0
// CHECK-NEXT: [[q11:%[0-9]+]] = OpCompositeExtract %bool [[q]] 1 1
// CHECK-NEXT: [[q20:%[0-9]+]] = OpCompositeExtract %bool [[q]] 2 0
// CHECK-NEXT: [[q21:%[0-9]+]] = OpCompositeExtract %bool [[q]] 2 1
// CHECK-NEXT: [[qt0:%[0-9]+]] = OpCompositeConstruct %v3bool [[q00]] [[q10]] [[q20]]
// CHECK-NEXT: [[qt1:%[0-9]+]] = OpCompositeConstruct %v3bool [[q01]] [[q11]] [[q21]]
// CHECK-NEXT:  [[qt:%[0-9]+]] = OpCompositeConstruct %_arr_v3bool_uint_2 [[qt0]] [[qt1]]
// CHECK-NEXT:                OpStore %qt [[qt]]
  bool3x2 q;
  bool2x3 qt = transpose(q);

// CHECK:         [[r:%[0-9]+]] = OpLoad %_arr_v4uint_uint_4 %r
// CHECK-NEXT:  [[r00:%[0-9]+]] = OpCompositeExtract %uint [[r]] 0 0
// CHECK-NEXT:  [[r01:%[0-9]+]] = OpCompositeExtract %uint [[r]] 0 1
// CHECK-NEXT:  [[r02:%[0-9]+]] = OpCompositeExtract %uint [[r]] 0 2
// CHECK-NEXT:  [[r03:%[0-9]+]] = OpCompositeExtract %uint [[r]] 0 3
// CHECK-NEXT:  [[r10:%[0-9]+]] = OpCompositeExtract %uint [[r]] 1 0
// CHECK-NEXT:  [[r11:%[0-9]+]] = OpCompositeExtract %uint [[r]] 1 1
// CHECK-NEXT:  [[r12:%[0-9]+]] = OpCompositeExtract %uint [[r]] 1 2
// CHECK-NEXT:  [[r13:%[0-9]+]] = OpCompositeExtract %uint [[r]] 1 3
// CHECK-NEXT:  [[r20:%[0-9]+]] = OpCompositeExtract %uint [[r]] 2 0
// CHECK-NEXT:  [[r21:%[0-9]+]] = OpCompositeExtract %uint [[r]] 2 1
// CHECK-NEXT:  [[r22:%[0-9]+]] = OpCompositeExtract %uint [[r]] 2 2
// CHECK-NEXT:  [[r23:%[0-9]+]] = OpCompositeExtract %uint [[r]] 2 3
// CHECK-NEXT:  [[r30:%[0-9]+]] = OpCompositeExtract %uint [[r]] 3 0
// CHECK-NEXT:  [[r31:%[0-9]+]] = OpCompositeExtract %uint [[r]] 3 1
// CHECK-NEXT:  [[r32:%[0-9]+]] = OpCompositeExtract %uint [[r]] 3 2
// CHECK-NEXT:  [[r33:%[0-9]+]] = OpCompositeExtract %uint [[r]] 3 3
// CHECK-NEXT:  [[rt0:%[0-9]+]] = OpCompositeConstruct %v4uint [[r00]] [[r10]] [[r20]] [[r30]]
// CHECK-NEXT:  [[rt1:%[0-9]+]] = OpCompositeConstruct %v4uint [[r01]] [[r11]] [[r21]] [[r31]]
// CHECK-NEXT:  [[rt2:%[0-9]+]] = OpCompositeConstruct %v4uint [[r02]] [[r12]] [[r22]] [[r32]]
// CHECK-NEXT:  [[rt3:%[0-9]+]] = OpCompositeConstruct %v4uint [[r03]] [[r13]] [[r23]] [[r33]]
// CHECK-NEXT:   [[rt:%[0-9]+]] = OpCompositeConstruct %_arr_v4uint_uint_4 [[rt0]] [[rt1]] [[rt2]] [[rt3]]
// CHECK-NEXT:                 OpStore %rt [[rt]]
  uint4x4 r;
  uint4x4 rt = transpose(r);

// A 1-D matrix is in fact a vector, and its transpose is the vector itself.
//
// CHECK:      [[s:%[0-9]+]] = OpLoad %v4float %s
// CHECK-NEXT:              OpStore %st [[s]]
  float1x4 s;
  float4x1 st = transpose(s);

// CHECK:      [[t:%[0-9]+]] = OpLoad %float %t
// CHECK-NEXT:              OpStore %tt [[t]]
  float1x1 t;
  float1x1 tt = transpose(t);
}
