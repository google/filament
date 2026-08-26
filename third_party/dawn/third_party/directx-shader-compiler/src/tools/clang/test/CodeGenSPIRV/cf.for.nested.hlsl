// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
// CHECK-LABEL: %bb_entry = OpLabel
    int val = 0;

// CHECK:      OpStore %val %int_0
// CHECK-NEXT: OpStore %i %int_0
// CHECK-NEXT: OpBranch %for_check

// CHECK-LABEL: %for_check = OpLabel
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %int %i
// CHECK-NEXT: [[lt0:%[0-9]+]] = OpSLessThan %bool [[i0]] %int_10
// CHECK-NEXT: OpLoopMerge %for_merge_1 %for_continue_1 Unroll
// CHECK-NEXT: OpBranchConditional [[lt0]] %for_body %for_merge_1
    [unroll] for (int i = 0; i < 10; ++i) {
// CHECK-LABEL: %for_body = OpLabel
// CHECK-NEXT: [[val0:%[0-9]+]] = OpLoad %int %val
// CHECK-NEXT: [[i1:%[0-9]+]] = OpLoad %int %i
// CHECK-NEXT: [[add0:%[0-9]+]] = OpIAdd %int [[val0]] [[i1]]
// CHECK-NEXT: OpStore %val [[add0]]
        val = val + i;
// CHECK-NEXT: OpStore %j %int_0
// CHECK-NEXT: OpBranch %for_check_0

// CHECK-LABEL: %for_check_0 = OpLabel
// CHECK-NEXT: [[j0:%[0-9]+]] = OpLoad %int %j
// CHECK-NEXT: [[lt1:%[0-9]+]] = OpSLessThan %bool [[j0]] %int_10
// CHECK-NEXT: OpLoopMerge %for_merge_0 %for_continue_0 DontUnroll
// CHECK-NEXT: OpBranchConditional [[lt1]] %for_body_0 %for_merge_0
        [loop] for (int j = 0; j < 10; ++j) {
// CHECK-LABEL: %for_body_0 = OpLabel
// CHECK-NEXT: OpStore %k %int_0
// CHECK-NEXT: OpBranch %for_check_1

// CHECK-LABEL: %for_check_1 = OpLabel
// CHECK-NEXT: [[k0:%[0-9]+]] = OpLoad %int %k
// CHECK-NEXT: [[lt2:%[0-9]+]] = OpSLessThan %bool [[k0]] %int_10
// CHECK-NEXT: OpLoopMerge %for_merge %for_continue DontUnroll
// CHECK-NEXT: OpBranchConditional [[lt2]] %for_body_1 %for_merge
            [fastopt] for (int k = 0; k < 10; ++k) {
// CHECK-LABEL: %for_body_1 = OpLabel
// CHECK-NEXT: [[val1:%[0-9]+]] = OpLoad %int %val
// CHECK-NEXT: [[k1:%[0-9]+]] = OpLoad %int %k
// CHECK-NEXT: [[add1:%[0-9]+]] = OpIAdd %int [[val1]] [[k1]]
// CHECK-NEXT: OpStore %val [[add1]]
// CHECK-NEXT: OpBranch %for_continue
                val = val + k;

// CHECK-LABEL: %for_continue = OpLabel
// CHECK-NEXT: [[k2:%[0-9]+]] = OpLoad %int %k
// CHECK-NEXT: [[add2:%[0-9]+]] = OpIAdd %int [[k2]] %int_1
// CHECK-NEXT: OpStore %k [[add2]]
// CHECK-NEXT: OpBranch %for_check_1
            }

// CHECK-LABEL: %for_merge = OpLabel
// CHECK-NEXT: [[val2:%[0-9]+]] = OpLoad %int %val
// CHECK-NEXT: [[mul0:%[0-9]+]] = OpIMul %int [[val2]] %int_2
// CHECK-NEXT: OpStore %val [[mul0]]
// CHECK-NEXT: OpBranch %for_continue_0
            val = val * 2;

// CHECK-LABEL: %for_continue_0 = OpLabel
// CHECK-NEXT: [[j1:%[0-9]+]] = OpLoad %int %j
// CHECK-NEXT: [[add3:%[0-9]+]] = OpIAdd %int [[j1]] %int_1
// CHECK-NEXT: OpStore %j [[add3]]
// CHECK-NEXT: OpBranch %for_check_0
        }
// CHECK-LABEL: %for_merge_0 = OpLabel
// CHECK-NEXT: OpBranch %for_continue_1

// CHECK-LABEL: %for_continue_1 = OpLabel
// CHECK-NEXT: [[i2:%[0-9]+]] = OpLoad %int %i
// CHECK-NEXT: [[add4:%[0-9]+]] = OpIAdd %int [[i2]] %int_1
// CHECK-NEXT: OpStore %i [[add4]]
// CHECK-NEXT: OpBranch %for_check
    }

// CHECK-LABEL: %for_merge_1 = OpLabel
// CHECK-NEXT: OpReturn
}
