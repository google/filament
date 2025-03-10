// RUN: %dxc -T ps_6_0 -HV 2021 -E main -fcgl  %s -spirv | FileCheck %s

// The 'out' argument in the function should be handled correctly when deducing
// the literal type, even in HLSL 2021 (with shortcicuiting).
void foo(out uint value, uint x) {
  // CHECK:   [[cond:%[0-9]+]] = OpULessThan %bool {{%[0-9]+}} %uint_64
  // CHECK:                   OpBranchConditional [[cond]] [[ternary_lhs:%[a-zA-Z0-9_]+]] [[ternary_rhs:%[a-zA-Z0-9_]+]]
  // CHECK: [[ternary_lhs]] = OpLabel
  // CHECK:                   OpStore [[tmp:%[a-zA-Z0-9_]+]] %int_1
  // CHECK:                   OpBranch [[merge:%[a-zA-Z0-9_]+]]
  // CHECK: [[ternary_rhs]] = OpLabel
  // CHECK:                   OpStore [[tmp_0:%[a-zA-Z0-9_]+]] %int_0
  // CHECK:                   OpBranch [[merge]]
  // CHECK:       [[merge]] = OpLabel
  // CHECK:    [[res:%[0-9]+]] = OpLoad %int [[tmp_0]]
  // CHECK:        {{%[0-9]+}} = OpBitcast %uint [[res]]
  value = x < 64 ? 1 : 0;
}

void main() {
  uint value;
  foo(value, 2);
}
