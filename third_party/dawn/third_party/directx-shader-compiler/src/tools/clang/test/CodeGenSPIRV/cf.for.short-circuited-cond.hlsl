// RUN: %dxc -T ps_6_0 -E main -HV 2021 -fcgl  %s -spirv | FileCheck %s

void main() {
  bool a, b;
  // CHECK:      OpBranch %for_header
  // CHECK-NEXT: %for_header = OpLabel
  // CHECK-NEXT: OpLoopMerge %for_merge %for_continue None
  // CHECK-NEXT: OpBranch %for_check
  // CHECK-NEXT: %for_check = OpLabel
  // CHECK:      OpBranchConditional {{%[0-9]+}} %for_body %for_merge
  for (int i = 0; a && b; ++i) {
    // CHECK-NEXT: %for_body = OpLabel
    // CHECK-NEXT: OpBranch %for_continue
    // CHECK-NEXT: %for_continue = OpLabel
    // CHECK:      OpBranch %for_header
  }
  // CHECK-NEXT: %for_merge = OpLabel

  // CHECK:      OpBranch %for_header_0
  // CHECK-NEXT: %for_header_0 = OpLabel
  // CHECK-NEXT: OpLoopMerge %for_merge_0 %for_continue_0 None
  // CHECK-NEXT: OpBranch %for_check_0
  // CHECK-NEXT: %for_check_0 = OpLabel
  // CHECK:      OpBranchConditional {{%[0-9]+}} %for_body_0 %for_merge_0
  for (int i = 0; a || b; ++i) {
    // CHECK-NEXT: %for_body_0 = OpLabel
    // CHECK-NEXT: OpBranch %for_continue_0
    // CHECK-NEXT: %for_continue_0 = OpLabel
    // CHECK:      OpBranch %for_header_0
  }
  // CHECK-NEXT: %for_merge_0 = OpLabel

  // CHECK:      OpBranch %for_header_1
  // CHECK-NEXT: %for_header_1 = OpLabel
  // CHECK-NEXT: OpLoopMerge %for_merge_1 %for_continue_1 None
  // CHECK-NEXT: OpBranch %for_check_1
  // CHECK-NEXT: %for_check_1 = OpLabel
  // CHECK:      OpBranchConditional {{%[0-9]+}} %for_body_1 %for_merge_1
  for (int i = 0; a && ((a || b) && b); ++i) {
    // CHECK-NEXT: %for_body_1 = OpLabel
    // CHECK-NEXT: OpBranch %for_continue_1
    // CHECK-NEXT: %for_continue_1 = OpLabel
    // CHECK:      OpBranch %for_header_1
  }
  // CHECK-NEXT: %for_merge_1 = OpLabel

  // CHECK:      OpBranch %for_header_2
  // CHECK-NEXT: %for_header_2 = OpLabel
  // CHECK-NEXT: OpLoopMerge %for_merge_2 %for_continue_2 None
  // CHECK-NEXT: OpBranch %for_check_2
  // CHECK-NEXT: %for_check_2 = OpLabel
  // CHECK:      OpBranchConditional {{%[0-9]+}} %for_body_2 %for_merge_2
  for (int i = 0; a ? a : b; ++i) {
    // CHECK-NEXT: %for_body_2 = OpLabel
    // CHECK-NEXT: OpBranch %for_continue_2
    // CHECK-NEXT: %for_continue_2 = OpLabel
    // CHECK:      OpBranch %for_header_2
  }
  // CHECK-NEXT: %for_merge_2 = OpLabel

  int x, y;
  // CHECK:      OpBranch %for_header_3
  // CHECK-NEXT: %for_header_3 = OpLabel
  // CHECK-NEXT: OpLoopMerge %for_merge_3 %for_continue_3 None
  // CHECK-NEXT: OpBranch %for_check_3
  // CHECK-NEXT: %for_check_3 = OpLabel
  // CHECK:      OpBranchConditional {{%[0-9]+}} %for_body_3 %for_merge_3
  for (int i = 0; x + (x && y); ++i) {
    // CHECK-NEXT: %for_body_3 = OpLabel
    // CHECK-NEXT: OpBranch %for_continue_3
    // CHECK-NEXT: %for_continue_3 = OpLabel
    // CHECK:      OpBranch %for_header_3
  }
  // CHECK-NEXT: %for_merge_3 = OpLabel
}
