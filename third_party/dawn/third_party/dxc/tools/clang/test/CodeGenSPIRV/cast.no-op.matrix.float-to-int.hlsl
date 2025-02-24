// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
  float2x2 a;
  int4 b;

// CHECK:        [[a:%[0-9]+]] = OpLoad %mat2v2float %a
// CHECK-NEXT: [[a_0:%[0-9]+]] = OpCompositeExtract %v2float [[a]] 0
// CHECK-NEXT: [[a_1:%[0-9]+]] = OpConvertFToS %v2int [[a_0]]
// CHECK-NEXT: [[a_2:%[0-9]+]] = OpCompositeExtract %v2float [[a]] 1
// CHECK-NEXT: [[a_3:%[0-9]+]] = OpConvertFToS %v2int [[a_2]]
// CHECK-NEXT:   [[a:%[0-9]+]] = OpCompositeConstruct %_arr_v2int_uint_2 [[a_1]] [[a_3]]
  b.zw = mul(int2x2(a), b.yx);
}
