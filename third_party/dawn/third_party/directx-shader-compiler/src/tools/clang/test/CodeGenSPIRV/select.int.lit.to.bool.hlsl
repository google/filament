// RUN: %dxc -T ps_6_0 -HV 2021 -E main -fcgl  %s -spirv | FileCheck %s

// In this case, the types for the literals in the select cannot be deduced
// because the cast to a bool does not force the literals to be any particular
// bitwidth. So the types should fall back to a 32-bit signed interger.
void foo(uint x) {
// CHECK: %temp_var_ternary = OpVariable %_ptr_Function_int Function
// CHECK: [[x:%[0-9]+]] = OpLoad %uint %x
// CHECK: [[cond:%[0-9]+]] = OpULessThan %bool [[x]] %uint_64
// CHECK: OpSelectionMerge %ternary_merge None
// CHECK: OpBranchConditional [[cond]] %ternary_lhs %ternary_rhs
// CHECK: %ternary_lhs = OpLabel
// CHECK: OpStore %temp_var_ternary %int_1
// CHECK: OpBranch %ternary_merge
// CHECK: %ternary_rhs = OpLabel
// CHECK: OpStore %temp_var_ternary %int_0
// CHECK: OpBranch %ternary_merge
// CHECK: %ternary_merge = OpLabel
// CHECK: [[ld:%[0-9]+]] = OpLoad %int %temp_var_ternary
// CHECK: [[res:%[0-9]+]] = OpINotEqual %bool [[ld]] %int_0
// CHECK: OpStore %value [[res]]
  bool value = x < 64 ? 1 : 0;
}

void main() {
  foo(2);
}
