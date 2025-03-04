// RUN: %dxc -T ps_6_0 -E main %s -spirv -fcgl | FileCheck %s -check-prefix=CHECK -check-prefix=KILL
// RUN: %dxc -T ps_6_0 -E main %s -spirv -fcgl -fspv-extension=SPV_EXT_demote_to_helper_invocation | FileCheck %s -check-prefix=CHECK -check-prefix=DEMOTE
// RUN: %dxc -T ps_6_0 -E main %s -spirv -fcgl -fspv-target-env=vulkan1.3 | FileCheck %s -check-prefix=CHECK -check-prefix=DEMOTE

// According to the HLSL reference, clip operates on scalar, vector, or matrix of floats.

// CHECK: [[v4f0:%[0-9]+]] = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
// CHECK: [[v3f0:%[0-9]+]] = OpConstantComposite %v3float %float_0 %float_0 %float_0

void main() {
  float    a;
  float4   b;
  float2x3 c;

// CHECK:                      [[a:%[0-9]+]] = OpLoad %float %a
// CHECK-NEXT:          [[is_a_neg:%[0-9]+]] = OpFOrdLessThan %bool [[a]] %float_0
// CHECK-NEXT:                              OpSelectionMerge %if_merge None
// CHECK-NEXT:                              OpBranchConditional [[is_a_neg]] %if_true %if_merge
// CHECK-NEXT:                   %if_true = OpLabel
// KILL-NEXT:                               OpKill
// DEMOTE-NEXT:                             OpDemoteToHelperInvocation
// DEMOTE-NEXT:                             OpBranch %if_merge
// CHECK-NEXT:                  %if_merge = OpLabel
  clip(a);

// CHECK-NEXT:                 [[b:%[0-9]+]] = OpLoad %v4float %b
// CHECK-NEXT:      [[is_b_neg_vec:%[0-9]+]] = OpFOrdLessThan %v4bool [[b]] [[v4f0]]
// CHECK-NEXT:          [[is_b_neg:%[0-9]+]] = OpAny %bool [[is_b_neg_vec]]
// CHECK-NEXT:                              OpSelectionMerge %if_merge_0 None
// CHECK-NEXT:                              OpBranchConditional [[is_b_neg]] %if_true_0 %if_merge_0
// CHECK-NEXT:                 %if_true_0 = OpLabel
// KILL-NEXT:                               OpKill
// DEMOTE-NEXT:                             OpDemoteToHelperInvocation
// DEMOTE-NEXT:                             OpBranch %if_merge_0
// CHECK-NEXT:                %if_merge_0 = OpLabel
  clip(b);

// CHECK:                      [[c:%[0-9]+]] = OpLoad %mat2v3float %c
// CHECK-NEXT:            [[c_row0:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 0
// CHECK-NEXT: [[is_c_row0_neg_vec:%[0-9]+]] = OpFOrdLessThan %v3bool [[c_row0]] [[v3f0]]
// CHECK-NEXT:     [[is_c_row0_neg:%[0-9]+]] = OpAny %bool [[is_c_row0_neg_vec]]
// CHECK-NEXT:            [[c_row1:%[0-9]+]] = OpCompositeExtract %v3float [[c]] 1
// CHECK-NEXT: [[is_c_row1_neg_vec:%[0-9]+]] = OpFOrdLessThan %v3bool [[c_row1]] [[v3f0]]
// CHECK-NEXT:     [[is_c_row1_neg:%[0-9]+]] = OpAny %bool [[is_c_row1_neg_vec]]
// CHECK-NEXT:      [[is_c_neg_vec:%[0-9]+]] = OpCompositeConstruct %v2bool [[is_c_row0_neg]] [[is_c_row1_neg]]
// CHECK-NEXT:          [[is_c_neg:%[0-9]+]] = OpAny %bool [[is_c_neg_vec]]
// CHECK-NEXT:                              OpSelectionMerge %if_merge_1 None
// CHECK-NEXT:                              OpBranchConditional [[is_c_neg]] %if_true_1 %if_merge_1
// CHECK-NEXT:                              %if_true_1 = OpLabel
// KILL-NEXT:                               OpKill
// DEMOTE-NEXT:                             OpDemoteToHelperInvocation
// DEMOTE-NEXT:                             OpBranch %if_merge_1
  clip(c);
// CHECK-NEXT:                %if_merge_1 = OpLabel
// CHECK-NEXT:                              OpReturn
}
