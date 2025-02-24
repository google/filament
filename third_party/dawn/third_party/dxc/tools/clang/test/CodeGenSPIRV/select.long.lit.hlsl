// RUN: %dxc -T ps_6_0 -HV 2018 -E main -fcgl  %s -spirv | FileCheck %s

// Check that the literals get a 64-bit type, and the result of the select is
// then cast to an unsigned 64-bit value.
void foo(uint x) {
  // CHECK: %foo = OpFunction
  // CHECK: [[cond:%[0-9]+]] = OpULessThan %bool {{%[0-9]+}} %uint_64
  // CHECK: [[result:%[0-9]+]] = OpSelect %long [[cond]] %long_1 %long_0
  // CHECK: {{%[0-9]+}} = OpBitcast %ulong [[result]]
  uint64_t value = x < 64 ? 1 : 0;
}

// Check that the literals get a 64-bit type, and the result of the select is
// then cast to an signed 64-bit value. Note that the bitcast is redundant in
// this case, but we add the bitcast before the type of the literal has been
// determined, so we add it anyway.
void bar(uint x) {
  // CHECK: %bar = OpFunction
  // CHECK: [[cond_0:%[0-9]+]] = OpULessThan %bool {{%[0-9]+}} %uint_64
  // CHECK: [[result_0:%[0-9]+]] = OpSelect %long [[cond_0]] %long_1 %long_0
  // CHECK: {{%[0-9]+}} = OpBitcast %long [[result_0]]
  int64_t value = x < 64 ? 1 : 0;
}

void main() {
  uint value;
  foo(2);
  bar(2);
}
