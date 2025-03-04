// RUN: %dxc -T ps_6_0 -HV 2018 -E main -fcgl  %s -spirv | FileCheck %s

void foo() { int x = 1; }

void main() {
  bool cond;
  int a = 1, b = 2;
  int c = 0;

// CHECK:                          OpStore %a %int_2
// CHECK-NEXT:                     OpStore %b %int_3
// CHECK-NEXT:          {{%[0-9]+}} = OpFunctionCall %void %foo
// CHECK-NEXT:     [[cond:%[0-9]+]] = OpLoad %bool %cond
// CHECK-NEXT:        [[a:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT:        [[c:%[0-9]+]] = OpLoad %int %c
// CHECK-NEXT:      [[sel:%[0-9]+]] = OpSelect %int [[cond]] [[a]] [[c]]
// CHECK-NEXT:                     OpStore %b [[sel]]
// CHECK-NEXT:       [[a1:%[0-9]+]] = OpLoad %int %a
// CHECK-NEXT:        [[b:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: [[a_plus_b:%[0-9]+]] = OpIAdd %int [[a1]] [[b]]
// CHECK-NEXT:                     OpStore %c [[a_plus_b]]
  c = (a=2, b=3, foo(), b = cond ? a : c , a+b);
}
