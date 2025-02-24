// RUN: %dxc -T ps_6_0 -E main -HV 2021 -fcgl  %s -spirv | FileCheck %s

void main() {
  // CHECK-LABEL: %bb_entry = OpLabel

  bool b;
  int m, n, o;
  // CHECK:      %temp_var_ternary = OpVariable %_ptr_Function_int Function
  // CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %bool %b
  // CHECK-NEXT: OpSelectionMerge %ternary_merge None
  // CHECK-NEXT: OpBranchConditional [[b]] %ternary_lhs %ternary_rhs
  // CHECK-NEXT: %ternary_lhs = OpLabel
  // CHECK-NEXT: [[m:%[0-9]+]] = OpLoad %int %m
  // CHECK-NEXT: OpStore %temp_var_ternary [[m]]
  // CHECK-NEXT: OpBranch %ternary_merge
  // CHECK-NEXT: %ternary_rhs = OpLabel
  // CHECK-NEXT: [[n:%[0-9]+]] = OpLoad %int %n
  // CHECK-NEXT: OpStore %temp_var_ternary [[n]]
  // CHECK-NEXT: OpBranch %ternary_merge
  // CHECK-NEXT: %ternary_merge = OpLabel
  // CHECK-NEXT: [[r:%[0-9]+]] = OpLoad %int %temp_var_ternary
  // CHECK-NEXT: OpStore %o [[r]]
  o = b ? m : n;
}
