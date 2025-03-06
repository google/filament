// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel
    bool c;
    int val;

    // Both then and else
// CHECK:      [[c0:%[0-9]+]] = OpLoad %bool %c
// CHECK-NEXT: OpSelectionMerge %if_merge Flatten
// CHECK-NEXT: OpBranchConditional [[c0]] %if_true %if_false
    [flatten] if (c) {
// CHECK-LABEL: %if_true = OpLabel
// CHECK-NEXT: [[val0:%[0-9]+]] = OpLoad %int %val
// CHECK-NEXT: [[val1:%[0-9]+]] = OpIAdd %int [[val0]] %int_1
// CHECK-NEXT: OpStore %val [[val1]]
// CHECK-NEXT: OpBranch %if_merge
        val = val + 1;
    } else {
// CHECK-LABEL: %if_false = OpLabel
// CHECK-NEXT: [[val2:%[0-9]+]] = OpLoad %int %val
// CHECK-NEXT: [[val3:%[0-9]+]] = OpIAdd %int [[val2]] %int_2
// CHECK-NEXT: OpStore %val [[val3]]
// CHECK-NEXT: OpBranch %if_merge
        val = val + 2;
    }
// CHECK-LABEL: %if_merge = OpLabel

    // No else
// CHECK-NEXT: [[c1:%[0-9]+]] = OpLoad %bool %c
// CHECK-NEXT: OpSelectionMerge %if_merge_0 None
// CHECK-NEXT: OpBranchConditional [[c1]] %if_true_0 %if_merge_0
    if (c)
// CHECK-LABEL: %if_true_0 = OpLabel
// CHECK-NEXT: OpStore %val %int_1
// CHECK-NEXT: OpBranch %if_merge_0
        val = 1;
// CHECK-LABEL: %if_merge_0 = OpLabel

    // Empty then
// CHECK-NEXT: [[c2:%[0-9]+]] = OpLoad %bool %c
// CHECK-NEXT: OpSelectionMerge %if_merge_1 None
// CHECK-NEXT: OpBranchConditional [[c2]] %if_true_1 %if_false_0
    if (c) {
// CHECK-LABEL: %if_true_1 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_1
    } else {
// CHECK-LABEL: %if_false_0 = OpLabel
// CHECK-NEXT: OpStore %val %int_2
// CHECK-NEXT: OpBranch %if_merge_1
        val = 2;
    }
// CHECK-LABEL: %if_merge_1 = OpLabel

    // Null body
// CHECK-NEXT: [[c3:%[0-9]+]] = OpLoad %bool %c
// CHECK-NEXT: OpSelectionMerge %if_merge_2 None
// CHECK-NEXT: OpBranchConditional [[c3]] %if_true_2 %if_merge_2
    if (c)
// CHECK-LABEL: %if_true_2 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_2
        ;

// CHECK-LABEL: %if_merge_2 = OpLabel

// CHECK-NEXT: [[val4:%[0-9]+]] = OpLoad %int %val
// CHECK-NEXT: OpStore %d [[val4]]
// CHECK-NEXT: [[d:%[0-9]+]] = OpLoad %int %d
// CHECK-NEXT: [[cmp:%[0-9]+]] = OpINotEqual %bool [[d]] %int_0
// CHECK-NEXT: OpSelectionMerge %if_merge_3 DontFlatten
// CHECK-NEXT: OpBranchConditional [[cmp]] %if_true_3 %if_merge_3
    [branch] if (int d = val) {
// CHECK-LABEL: %if_true_3 = OpLabel
// CHECK-NEXT: OpStore %c %true
        c = true;
// CHECK-NEXT: OpBranch %if_merge_3
// CHECK-LABEL:%if_merge_3 = OpLabel
    }
// CHECK-NEXT: OpReturn
}
