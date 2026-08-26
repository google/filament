// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
  int a, b;

// CHECK:          [[a:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT:     [[b:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT:    [[eq:%[0-9]+]] = OpIEqual %bool [[a]] [[b]]
// CHECK-NEXT: [[c_int:%[0-9]+]] = OpSelect %int [[eq]] %int_1 %int_0
// CHECK-NEXT:       {{%[0-9]+}} = OpINotEqual %bool [[c_int]] %int_1
  bool c = (a == b) != 1;

// CHECK:            [[a_0:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT:       [[b_0:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT:      [[eq_0:%[0-9]+]] = OpIEqual %bool [[a_0]] [[b_0]]
// CHECK-NEXT: [[d_float:%[0-9]+]] = OpSelect %float [[eq_0]] %float_1 %float_0
// CHECK-NEXT:         {{%[0-9]+}} = OpFOrdNotEqual %bool [[d_float]] %float_1
  bool d = (a == b) != 1.0;
}
