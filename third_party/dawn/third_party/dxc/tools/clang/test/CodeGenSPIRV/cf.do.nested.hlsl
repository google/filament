// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
  int val=0, i=0, j=0, k=0;

// CHECK:      OpBranch %do_while_header
// CHECK-NEXT: %do_while_header = OpLabel
// CHECK-NEXT: OpLoopMerge %do_while_merge_1 %do_while_continue_1 DontUnroll
  [loop] do {
// CHECK-NEXT: OpBranch %do_while_body
// CHECK-NEXT: %do_while_body = OpLabel
// CHECK-NEXT: [[val0:%[0-9]+]] = OpLoad %int %val
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %int %i
// CHECK-NEXT: [[val_plus_i:%[0-9]+]] = OpIAdd %int [[val0]] [[i0]]
// CHECK-NEXT: OpStore %val [[val_plus_i]]
// CHECK-NEXT: OpBranch %do_while_header_0
    val = val + i;
// CHECK-NEXT: %do_while_header_0 = OpLabel
// CHECK-NEXT: OpLoopMerge %do_while_merge_0 %do_while_continue_0 Unroll
// CHECK-NEXT: OpBranch %do_while_body_0
    [unroll(20)] do {
// CHECK-NEXT: %do_while_body_0 = OpLabel
// CHECK-NEXT: OpBranch %do_while_header_1

// CHECK-NEXT: %do_while_header_1 = OpLabel
// CHECK-NEXT: OpLoopMerge %do_while_merge %do_while_continue DontUnroll
// CHECK-NEXT: OpBranch %do_while_body_1
      [fastopt] do {
// CHECK-NEXT: %do_while_body_1 = OpLabel
// CHECK-NEXT: [[k0:%[0-9]+]] = OpLoad %int %k
// CHECK-NEXT: [[k_plus_1:%[0-9]+]] = OpIAdd %int [[k0]] %int_1
// CHECK-NEXT: OpStore %k [[k_plus_1]]
// CHECK-NEXT: OpBranch %do_while_continue
        ++k;
// CHECK-NEXT: %do_while_continue = OpLabel
// CHECK-NEXT: [[k1:%[0-9]+]] = OpLoad %int %k
// CHECK-NEXT: [[k_lt_30:%[0-9]+]] = OpSLessThan %bool [[k1]] %int_30
// CHECK-NEXT: OpBranchConditional [[k_lt_30]] %do_while_header_1 %do_while_merge
      } while (k < 30);

// CHECK-NEXT: %do_while_merge = OpLabel
// CHECK-NEXT: [[j0:%[0-9]+]] = OpLoad %int %j
// CHECK-NEXT: [[j_plus_1:%[0-9]+]] = OpIAdd %int [[j0]] %int_1
// CHECK-NEXT: OpStore %j [[j_plus_1]]
// CHECK-NEXT: OpBranch %do_while_continue_0
      ++j;
// CHECK-NEXT: %do_while_continue_0 = OpLabel
// CHECK-NEXT: [[j1:%[0-9]+]] = OpLoad %int %j
// CHECK-NEXT: [[j_lt_20:%[0-9]+]] = OpSLessThan %bool [[j1]] %int_20
// CHECK-NEXT: OpBranchConditional [[j_lt_20]] %do_while_header_0 %do_while_merge_0
    } while (j < 20);

// CHECK-NEXT: %do_while_merge_0 = OpLabel
// CHECK-NEXT: [[i0_0:%[0-9]+]] = OpLoad %int %i
// CHECK-NEXT: [[i_plus_1:%[0-9]+]] = OpIAdd %int [[i0_0]] %int_1
// CHECK-NEXT: OpStore %i [[i_plus_1]]
// CHECK-NEXT: OpBranch %do_while_continue_1
    ++i;

// CHECK-NEXT: %do_while_continue_1 = OpLabel
// CHECK-NEXT: [[i1:%[0-9]+]] = OpLoad %int %i
// CHECK-NEXT: [[i_lt_10:%[0-9]+]] = OpSLessThan %bool [[i1]] %int_10
// CHECK-NEXT: OpBranchConditional [[i_lt_10]] %do_while_header %do_while_merge_1
  } while (i < 10);
// CHECK-NEXT: %do_while_merge_1 = OpLabel


// CHECK-NEXT: OpReturn
// CHECK-NEXT: OpFunctionEnd
}
