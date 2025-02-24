// RUN: %dxc -T ps_6_0 -HV 2018 -E main -fcgl  %s -spirv | FileCheck %s

// The 'out' argument in the function should be handled correctly when deducing
// the literal type.
void foo(out uint value, uint x) {
  // CHECK: [[cond:%[0-9]+]] = OpULessThan %bool {{%[0-9]+}} %uint_64
  // CHECK: [[result:%[0-9]+]] = OpSelect %int [[cond]] %int_1 %int_0
  // CHECK: {{%[0-9]+}} = OpBitcast %uint [[result]]
  value = x < 64 ? 1 : 0;
}

void main() {
  uint value;
  foo(value, 2);
}
