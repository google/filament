// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

float4 main(float color: COLOR) : SV_TARGET {
// CHECK-LABEL: %bb_entry = OpLabel

// CHECK: OpStore %val %float_0
    float val = 0.;

// CHECK-NEXT: [[color0:%[0-9]+]] = OpLoad %float %color
// CHECK-NEXT: [[lt0:%[0-9]+]] = OpFOrdLessThan %bool [[color0]] %float_0_5
// CHECK-NEXT: OpSelectionMerge %if_merge None
// CHECK-NEXT: OpBranchConditional [[lt0]] %if_true %if_merge
    if (color < 0.5) {
// CHECK-LABEL: %if_true = OpLabel
// CHECK-NEXT: OpStore %val %float_1
        val = 1.;
// CHECK-NEXT: OpBranch %if_merge
    }
// CHECK-LABEL: %if_merge = OpLabel
// CHECK-NEXT: OpStore %i %int_0
// CHECK-NEXT: OpBranch %for_check

    // for-stmt following if-stmt
// CHECK-LABEL: %for_check = OpLabel
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %int %i
// CHECK-NEXT: [[lt1:%[0-9]+]] = OpSLessThan %bool [[i0]] %int_10
// CHECK-NEXT: OpLoopMerge %for_merge_1 %for_continue_1 None
// CHECK-NEXT: OpBranchConditional [[lt1]] %for_body %for_merge_1
    for (int i = 0; i < 10; ++i) {
// CHECK-LABEL: %for_body = OpLabel
// CHECK-NEXT: [[color1:%[0-9]+]] = OpLoad %float %color
// CHECK-NEXT: [[lt2:%[0-9]+]] = OpFOrdLessThan %bool [[color1]] %float_0_5
// CHECK-NEXT: OpSelectionMerge %if_merge_0 None
// CHECK-NEXT: OpBranchConditional [[lt2]] %if_true_0 %if_merge_0
        if (color < 0.5) { // if-stmt nested in for-stmt
// CHECK-LABEL: %if_true_0 = OpLabel
// CHECK: OpStore %val
            val = val + 1.;
// CHECK-NEXT: OpStore %j %int_0
// CHECK-NEXT: OpBranch %for_check_0

// CHECK-LABEL: %for_check_0 = OpLabel
// CHECK-NEXT: [[j0:%[0-9]+]] = OpLoad %int %j
// CHECK-NEXT: [[lt3:%[0-9]+]] = OpSLessThan %bool [[j0]] %int_15
// CHECK-NEXT: OpLoopMerge %for_merge %for_continue None
// CHECK-NEXT: OpBranchConditional [[lt3]] %for_body_0 %for_merge
            for (int j = 0; j < 15; ++j) { // for-stmt deeply nested in if-then
// CHECK-LABEL: %for_body_0 = OpLabel
// CHECK: OpStore %val
                val = val * 2.;
// CHECK-NEXT: OpBranch %for_continue

// CHECK-LABEL: %for_continue = OpLabel
// CHECK-NEXT: [[j1:%[0-9]+]] = OpLoad %int %j
// CHECK-NEXT: [[incj:%[0-9]+]] = OpIAdd %int [[j1]] %int_1
// CHECK-NEXT: OpStore %j [[incj]]
// CHECK-NEXT: OpBranch %for_check_0
            } // end for (int j
// CHECK-LABEL: %for_merge = OpLabel
// CHECK: OpStore %val

            val = val + 3.;
// CHECK-NEXT: OpBranch %if_merge_0
        }
// CHECK-LABEL: %if_merge_0 = OpLabel

// CHECK-NEXT: [[color2:%[0-9]+]] = OpLoad %float %color
// CHECK-NEXT: [[lt4:%[0-9]+]] = OpFOrdLessThan %bool [[color2]] %float_0_8
// CHECK-NEXT: OpSelectionMerge %if_merge_2 None
// CHECK-NEXT: OpBranchConditional [[lt4]] %if_true_1 %if_false
        if (color < 0.8) { // if-stmt following if-stmt
// CHECK-LABEL: %if_true_1 = OpLabel
// CHECK: OpStore %val
            val = val * 4.;
// CHECK-NEXT: OpBranch %if_merge_2
        } else {
// CHECK-LABEL: %if_false = OpLabel
// CHECK-NEXT: OpStore %k %int_0
// CHECK-NEXT: OpBranch %for_check_1

// CHECK-LABEL: %for_check_1 = OpLabel
// CHECK-NEXT: [[k0:%[0-9]+]] = OpLoad %int %k
// CHECK-NEXT: [[lt5:%[0-9]+]] = OpSLessThan %bool [[k0]] %int_20
// CHECK-NEXT: OpLoopMerge %for_merge_0 %for_continue_0 None
// CHECK-NEXT: OpBranchConditional [[lt5]] %for_body_1 %for_merge_0
            for (int k = 0; k < 20; ++k) { // for-stmt deeply nested in if-else
// CHECK-LABEL: %for_body_1 = OpLabel
// CHECK: OpStore %val
                val = val - 5.;

// CHECK-NEXT: [[val5:%[0-9]+]] = OpLoad %float %val
// CHECK-NEXT: [[lt6:%[0-9]+]] = OpFOrdLessThan %bool [[val5]] %float_0
// CHECK-NEXT: OpSelectionMerge %if_merge_1 None
// CHECK-NEXT: OpBranchConditional [[lt6]] %if_true_2 %if_merge_1
                if (val < 0.) { // deeply nested if-stmt
// CHECK-LABEL: %if_true_2 = OpLabel
// CHECK: OpStore %val
                    val = val + 100.;
// CHECK-NEXT: OpBranch %if_merge_1
                }
// CHECK-LABEL: %if_merge_1 = OpLabel
// CHECK-NEXT: OpBranch %for_continue_0

// CHECK-LABEL: %for_continue_0 = OpLabel
// CHECK-NEXT: [[k1:%[0-9]+]] = OpLoad %int %k
// CHECK-NEXT: [[inck:%[0-9]+]] = OpIAdd %int [[k1]] %int_1
// CHECK-NEXT: OpStore %k [[inck]]
// CHECK-NEXT: OpBranch %for_check_1
            } // end for (int k
// CHECK-LABEL: %for_merge_0 = OpLabel
// CHECK-NEXT: OpBranch %if_merge_2
        } // end elsek
// CHECK-LABEL: %if_merge_2 = OpLabel
// CHECK-NEXT: OpBranch %for_continue_1

// CHECK-LABEL: %for_continue_1 = OpLabel
// CHECK-NEXT: [[i1:%[0-9]+]] = OpLoad %int %i
// CHECK-NEXT: [[inci:%[0-9]+]] = OpIAdd %int [[i1]] %int_1
// CHECK-NEXT: OpStore %i [[inci]]
// CHECK-NEXT: OpBranch %for_check
    } // end for (int i
// CHECK-LABEL: %for_merge_1 = OpLabel

    // if-stmt following for-stmt
// CHECK-NEXT: [[color3:%[0-9]+]] = OpLoad %float %color
// CHECK-NEXT: [[lt7:%[0-9]+]] = OpFOrdLessThan %bool [[color3]] %float_1_5
// CHECK-NEXT: OpSelectionMerge %if_merge_3 None
// CHECK-NEXT: OpBranchConditional [[lt7]] %if_true_3 %if_merge_3
    if (color < 1.5) {
// CHECK-LABEL: %if_true_3 = OpLabel
// CHECK: OpStore %val
        val = val + 6.;
// CHECK-NEXT: OpBranch %if_merge_3
    }
// CHECK-LABEL: %if_merge_3 = OpLabel

// CHECK: OpReturnValue
    return float4(val, val, val, val);
// CHECK-NEXT: OpFunctionEnd
}
