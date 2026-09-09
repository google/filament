// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel
    int val = 0;

// CHECK: OpBranch %for_check
// CHECK-LABEL: %for_check = OpLabel
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %int %i
// CHECK-NEXT: [[lt0:%[0-9]+]] = OpSLessThan %bool [[i0]] %int_10
// CHECK-NEXT: OpLoopMerge %for_merge %for_continue None
// CHECK-NEXT: OpBranchConditional [[lt0]] %for_body %for_merge
    for (int i = 0; i < 10; ++i) {
// CHECK-LABEL: %for_body = OpLabel
// CHECK-NEXT: [[i1:%[0-9]+]] = OpLoad %int %i
// CHECK-NEXT: OpStore %val [[i1]]
// CHECK-NEXT: OpBranch %for_continue
        val = i;
// CHECK-LABEL: %for_continue = OpLabel
// CHECK-NEXT: [[i2:%[0-9]+]] = OpLoad %int %i
// CHECK-NEXT: [[add0:%[0-9]+]] = OpIAdd %int [[i2]] %int_1
// CHECK-NEXT: OpStore %i [[add0]]
// CHECK-NEXT: OpBranch %for_check
    }

// CHECK-LABEL: %for_merge = OpLabel
// CHECK-NEXT: OpBranch %for_check_0
// CHECK-LABEL: %for_check_0 = OpLabel
// CHECK-NEXT: OpLoopMerge %for_merge_0 %for_continue_0 None
// CHECK-NEXT: OpBranchConditional %true %for_body_0 %for_merge_0
    // Infinite loop
    for ( ; ; ) {
// CHECK-LABEL: %for_body_0 = OpLabel
// CHECK-NEXT: OpStore %val %int_0
// CHECK-NEXT: OpBranch %for_continue_0
        val = 0;
// CHECK-LABEL: %for_continue_0 = OpLabel
// CHECK-NEXT: OpBranch %for_check_0
    }
// CHECK-LABEL: %for_merge_0 = OpLabel
// CHECK: OpBranch %for_check_1

    // Null body
// CHECK-LABEL: %for_check_1 = OpLabel
// CHECK-NEXT: [[j0:%[0-9]+]] = OpLoad %int %j
// CHECK-NEXT: [[lt1:%[0-9]+]] = OpSLessThan %bool [[j0]] %int_10
// CHECK-NEXT: OpLoopMerge %for_merge_1 %for_continue_1 None
// CHECK-NEXT: OpBranchConditional [[lt1]] %for_body_1 %for_merge_1
    for (int j = 0; j < 10; ++j)
// CHECK-LABEL: %for_body_1 = OpLabel
// CHECK-NEXT: OpBranch %for_continue_1
        ;
// CHECK-LABEL: %for_continue_1 = OpLabel
// CHECK-NEXT: [[j1:%[0-9]+]] = OpLoad %int %j
// CHECK-NEXT: [[add1:%[0-9]+]] = OpIAdd %int [[j1]] %int_1
// CHECK-NEXT: OpStore %j [[add1]]
// CHECK-NEXT: OpBranch %for_check_1

// CHECK-LABEL: %for_merge_1 = OpLabel
// CHECK-NEXT: OpReturn
}
