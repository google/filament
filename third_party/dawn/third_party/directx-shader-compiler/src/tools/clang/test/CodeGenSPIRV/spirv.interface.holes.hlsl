// RUN: %dxc -T vs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

float main(
  int a[1000] : A,
// CHECK:  OpDecorate %in_var_A Location 0
  int b : B,
// CHECK:  OpDecorate %in_var_B Location 1000
  int c[500] : C,
// CHECK:  OpDecorate %in_var_C Location 1001
  int D : D
// CHECK:  OpDecorate %in_var_D Location 1501
  ) : A
{
  return 1.0;
}

