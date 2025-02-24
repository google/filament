// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

// CHECK:       [[ext:%[0-9]+]] = OpExtInstImport "GLSL.std.450"

struct S1 {
  float value;
};

struct S2 {
  float1 value;
};

struct S3 {
  int value;
};

struct S4 {
  int1 value;
};

struct S5 {
  bool value;
};

struct S6 {
  bool1 value;
};

[numthreads(1,1,1)]
void main() {
  int1 vi;
  float1 vf;
  bool1 vb;

// CHECK:      [[vi:%[0-9]+]] = OpLoad %int %vi
// CHECK-NEXT:  [[x:%[0-9]+]] = OpConvertSToF %float [[vi]]
// CHECK-NEXT:  [[s:%[0-9]+]] = OpCompositeConstruct %S1 [[x]]
// CHECK-NEXT:               OpStore %a1 [[s]]
  S1 a1 = { vi };

// CHECK-NEXT: [[vf:%[0-9]+]] = OpLoad %float %vf
// CHECK-NEXT:  [[s_0:%[0-9]+]] = OpCompositeConstruct %S1 [[vf]]
// CHECK-NEXT:               OpStore %b1 [[s_0]]
  S1 b1 = { vf };

// CHECK-NEXT: [[vb:%[0-9]+]] = OpLoad %bool %vb
// CHECK-NEXT:  [[x_0:%[0-9]+]] = OpSelect %float [[vb]] %float_1 %float_0
// CHECK-NEXT:  [[s_1:%[0-9]+]] = OpCompositeConstruct %S1 [[x_0]]
// CHECK-NEXT:               OpStore %c1 [[s_1]]
  S1 c1 = { vb };

// CHECK-NEXT:   [[vi_0:%[0-9]+]] = OpLoad %int %vi
// CHECK-NEXT:    [[u:%[0-9]+]] = OpBitcast %uint [[vi_0]]
// CHECK-NEXT: [[half:%[0-9]+]] = OpExtInst %v2float [[ext]] UnpackHalf2x16 [[u]]
// CHECK-NEXT:    [[x_1:%[0-9]+]] = OpCompositeExtract %float [[half]] 0
// CHECK-NEXT:    [[s_2:%[0-9]+]] = OpCompositeConstruct %S1 [[x_1]]
// CHECK-NEXT:                 OpStore %d1 [[s_2]]
  S1 d1 = { half1(f16tof32(vi)) };

// CHECK-NEXT: [[vi_1:%[0-9]+]] = OpLoad %int %vi
// CHECK-NEXT:  [[x_2:%[0-9]+]] = OpConvertSToF %float [[vi_1]]
// CHECK-NEXT:  [[s_3:%[0-9]+]] = OpCompositeConstruct %S2 [[x_2]]
// CHECK-NEXT:               OpStore %a2 [[s_3]]
  S2 a2 = { vi };

// CHECK-NEXT: [[vf_0:%[0-9]+]] = OpLoad %float %vf
// CHECK-NEXT:  [[s_4:%[0-9]+]] = OpCompositeConstruct %S2 [[vf_0]]
// CHECK-NEXT:               OpStore %b2 [[s_4]]
  S2 b2 = { vf };

// CHECK-NEXT: [[vb_0:%[0-9]+]] = OpLoad %bool %vb
// CHECK-NEXT:  [[x_3:%[0-9]+]] = OpSelect %float [[vb_0]] %float_1 %float_0
// CHECK-NEXT:  [[s_5:%[0-9]+]] = OpCompositeConstruct %S2 [[x_3]]
// CHECK-NEXT:               OpStore %c2 [[s_5]]
  S2 c2 = { vb };

// CHECK-NEXT:   [[vi_2:%[0-9]+]] = OpLoad %int %vi
// CHECK-NEXT:    [[u_0:%[0-9]+]] = OpBitcast %uint [[vi_2]]
// CHECK-NEXT: [[half_0:%[0-9]+]] = OpExtInst %v2float [[ext]] UnpackHalf2x16 [[u_0]]
// CHECK-NEXT:    [[x_4:%[0-9]+]] = OpCompositeExtract %float [[half_0]] 0
// CHECK-NEXT:    [[s_6:%[0-9]+]] = OpCompositeConstruct %S2 [[x_4]]
// CHECK-NEXT:                 OpStore %d2 [[s_6]]
  S2 d2 = { half1(f16tof32(vi)) };

// CHECK-NEXT: [[vi_3:%[0-9]+]] = OpLoad %int %vi
// CHECK-NEXT:  [[s_7:%[0-9]+]] = OpCompositeConstruct %S3 [[vi_3]]
// CHECK-NEXT:               OpStore %a3 [[s_7]]
  S3 a3 = { vi };

// CHECK-NEXT: [[vf_1:%[0-9]+]] = OpLoad %float %vf
// CHECK-NEXT:  [[x_5:%[0-9]+]] = OpConvertFToS %int [[vf_1]]
// CHECK-NEXT:  [[s_8:%[0-9]+]] = OpCompositeConstruct %S3 [[x_5]]
// CHECK-NEXT:               OpStore %b3 [[s_8]]
  S3 b3 = { vf };

// CHECK-NEXT: [[vb_1:%[0-9]+]] = OpLoad %bool %vb
// CHECK-NEXT:  [[x_6:%[0-9]+]] = OpSelect %int [[vb_1]] %int_1 %int_0
// CHECK-NEXT:  [[s_9:%[0-9]+]] = OpCompositeConstruct %S3 [[x_6]]
// CHECK-NEXT:               OpStore %c3 [[s_9]]
  S3 c3 = { vb };

// CHECK-NEXT:   [[vi_4:%[0-9]+]] = OpLoad %int %vi
// CHECK-NEXT:    [[u_1:%[0-9]+]] = OpBitcast %uint [[vi_4]]
// CHECK-NEXT: [[half_1:%[0-9]+]] = OpExtInst %v2float [[ext]] UnpackHalf2x16 [[u_1]]
// CHECK-NEXT:    [[f:%[0-9]+]] = OpCompositeExtract %float [[half_1]] 0
// CHECK-NEXT:    [[x_7:%[0-9]+]] = OpConvertFToS %int [[f]]
// CHECK-NEXT:    [[s_10:%[0-9]+]] = OpCompositeConstruct %S3 [[x_7]]
// CHECK-NEXT:                 OpStore %d3 [[s_10]]
  S3 d3 = { half1(f16tof32(vi)) };

// CHECK-NEXT: [[vi_5:%[0-9]+]] = OpLoad %int %vi
// CHECK-NEXT:  [[s_11:%[0-9]+]] = OpCompositeConstruct %S4 [[vi_5]]
// CHECK-NEXT:               OpStore %a4 [[s_11]]
  S4 a4 = { vi };

// CHECK-NEXT: [[vf_2:%[0-9]+]] = OpLoad %float %vf
// CHECK-NEXT:  [[x_8:%[0-9]+]] = OpConvertFToS %int [[vf_2]]
// CHECK-NEXT:  [[s_12:%[0-9]+]] = OpCompositeConstruct %S4 [[x_8]]
// CHECK-NEXT:               OpStore %b4 [[s_12]]
  S4 b4 = { vf };

// CHECK-NEXT: [[vb_2:%[0-9]+]] = OpLoad %bool %vb
// CHECK-NEXT:  [[x_9:%[0-9]+]] = OpSelect %int [[vb_2]] %int_1 %int_0
// CHECK-NEXT:  [[s_13:%[0-9]+]] = OpCompositeConstruct %S4 [[x_9]]
// CHECK-NEXT:               OpStore %c4 [[s_13]]
  S4 c4 = { vb };

// CHECK-NEXT:   [[vi_6:%[0-9]+]] = OpLoad %int %vi
// CHECK-NEXT:    [[u_2:%[0-9]+]] = OpBitcast %uint [[vi_6]]
// CHECK-NEXT: [[half_2:%[0-9]+]] = OpExtInst %v2float [[ext]] UnpackHalf2x16 [[u_2]]
// CHECK-NEXT:    [[f_0:%[0-9]+]] = OpCompositeExtract %float [[half_2]] 0
// CHECK-NEXT:    [[x_10:%[0-9]+]] = OpConvertFToS %int [[f_0]]
// CHECK-NEXT:    [[s_14:%[0-9]+]] = OpCompositeConstruct %S4 [[x_10]]
// CHECK-NEXT:                 OpStore %d4 [[s_14]]
  S4 d4 = { half1(f16tof32(vi)) };

// CHECK-NEXT: [[vi_7:%[0-9]+]] = OpLoad %int %vi
// CHECK-NEXT:  [[x_11:%[0-9]+]] = OpINotEqual %bool [[vi_7]] %int_0
// CHECK-NEXT:  [[s_15:%[0-9]+]] = OpCompositeConstruct %S5 [[x_11]]
// CHECK-NEXT:               OpStore %a5 [[s_15]]
  S5 a5 = { vi };

// CHECK-NEXT: [[vf_3:%[0-9]+]] = OpLoad %float %vf
// CHECK-NEXT:  [[x_12:%[0-9]+]] = OpFOrdNotEqual %bool [[vf_3]] %float_0
// CHECK-NEXT:  [[s_16:%[0-9]+]] = OpCompositeConstruct %S5 [[x_12]]
// CHECK-NEXT:               OpStore %b5 [[s_16]]
  S5 b5 = { vf };

// CHECK-NEXT: [[vb_3:%[0-9]+]] = OpLoad %bool %vb
// CHECK-NEXT:  [[s_17:%[0-9]+]] = OpCompositeConstruct %S5 [[vb_3]]
// CHECK-NEXT:               OpStore %c5 [[s_17]]
  S5 c5 = { vb };

// CHECK-NEXT:   [[vi_8:%[0-9]+]] = OpLoad %int %vi
// CHECK-NEXT:    [[u_3:%[0-9]+]] = OpBitcast %uint [[vi_8]]
// CHECK-NEXT: [[half_3:%[0-9]+]] = OpExtInst %v2float [[ext]] UnpackHalf2x16 [[u_3]]
// CHECK-NEXT:    [[f_1:%[0-9]+]] = OpCompositeExtract %float [[half_3]] 0
// CHECK-NEXT:    [[x_13:%[0-9]+]] = OpFOrdNotEqual %bool [[f_1]] %float_0
// CHECK-NEXT:    [[s_18:%[0-9]+]] = OpCompositeConstruct %S5 [[x_13]]
// CHECK-NEXT:                 OpStore %d5 [[s_18]]
  S5 d5 = { half1(f16tof32(vi)) };

// CHECK-NEXT: [[vi_9:%[0-9]+]] = OpLoad %int %vi
// CHECK-NEXT:  [[x_14:%[0-9]+]] = OpINotEqual %bool [[vi_9]] %int_0
// CHECK-NEXT:  [[s_19:%[0-9]+]] = OpCompositeConstruct %S6 [[x_14]]
// CHECK-NEXT:               OpStore %a6 [[s_19]]
  S6 a6 = { vi };

// CHECK-NEXT: [[vf_4:%[0-9]+]] = OpLoad %float %vf
// CHECK-NEXT:  [[x_15:%[0-9]+]] = OpFOrdNotEqual %bool [[vf_4]] %float_0
// CHECK-NEXT:  [[s_20:%[0-9]+]] = OpCompositeConstruct %S6 [[x_15]]
// CHECK-NEXT:               OpStore %b6 [[s_20]]
  S6 b6 = { vf };

// CHECK-NEXT: [[vb_4:%[0-9]+]] = OpLoad %bool %vb
// CHECK-NEXT:  [[s_21:%[0-9]+]] = OpCompositeConstruct %S6 [[vb_4]]
// CHECK-NEXT:               OpStore %c6 [[s_21]]
  S6 c6 = { vb };

// CHECK-NEXT:   [[vi_10:%[0-9]+]] = OpLoad %int %vi
// CHECK-NEXT:    [[u_4:%[0-9]+]] = OpBitcast %uint [[vi_10]]
// CHECK-NEXT: [[half_4:%[0-9]+]] = OpExtInst %v2float [[ext]] UnpackHalf2x16 [[u_4]]
// CHECK-NEXT:    [[f_2:%[0-9]+]] = OpCompositeExtract %float [[half_4]] 0
// CHECK-NEXT:    [[x_16:%[0-9]+]] = OpFOrdNotEqual %bool [[f_2]] %float_0
// CHECK-NEXT:    [[s_22:%[0-9]+]] = OpCompositeConstruct %S6 [[x_16]]
// CHECK-NEXT:                 OpStore %d6 [[s_22]]
  S6 d6 = { half1(f16tof32(vi)) };
}
