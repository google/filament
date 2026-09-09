// RUN: %dxc -T ps_6_0 -fcgl -spirv %s | FileCheck %s

float main() : SV_TARGET {
///////////////////////////////
// 32-bit int literal switch //
///////////////////////////////
// CHECK:                OpSelectionMerge %switch_merge None
// CHECK:                OpSwitch %int_0 %switch_default 0 %switch_0
  switch (0) {
// CHECK:    %switch_0 = OpLabel
  case 0:
// CHECK:                OpReturnValue %float_1
    return 1;
// CHECK: %switch_default = OpLabel
  default:
// CHECK:                OpReturnValue %float_2
    return 2;
  }
// CHECK: %switch_merge = OpLabel

///////////////////////////////
// 64-bit int literal switch //
///////////////////////////////
// CHECK:                OpSelectionMerge %switch_merge_0 None
// CHECK:                OpSwitch %long_12345678910 %switch_merge_0 12345678910 %switch_12345678910
  switch (12345678910) {
// CHECK:  %switch_12345678910 = OpLabel
  case 12345678910:
// CHECK:                OpReturnValue %float_1
    return 1;
  }
// CHECK: %switch_merge_0 = OpLabel

  return 0;
}
