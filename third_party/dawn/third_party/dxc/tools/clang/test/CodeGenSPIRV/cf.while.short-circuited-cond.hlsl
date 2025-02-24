// RUN: %dxc -T ps_6_0 -E main -HV 2021 -fcgl  %s -spirv | FileCheck %s

void main() {
  bool a, b;
  // CHECK:      OpBranch %while_header
  // CHECK-NEXT: %while_header = OpLabel
  // CHECK-NEXT: OpLoopMerge %while_merge %while_continue None
  // CHECK-NEXT: OpBranch %while_check
  // CHECK-NEXT: %while_check = OpLabel
  // CHECK:      OpBranchConditional {{%[0-9]+}} %while_body %while_merge
  while (a && b) {
    // CHECK-NEXT: %while_body = OpLabel
    // CHECK-NEXT: OpBranch %while_continue
    // CHECK-NEXT: %while_continue = OpLabel
    // CHECK:      OpBranch %while_header
  }
  // CHECK-NEXT: %while_merge = OpLabel

  // CHECK:      OpBranch %while_header_0
  // CHECK-NEXT: %while_header_0 = OpLabel
  // CHECK-NEXT: OpLoopMerge %while_merge_0 %while_continue_0 None
  // CHECK-NEXT: OpBranch %while_check_0
  // CHECK-NEXT: %while_check_0 = OpLabel
  // CHECK:      OpBranchConditional {{%[0-9]+}} %while_body_0 %while_merge_0
  while (a || b) {
    // CHECK-NEXT: %while_body_0 = OpLabel
    // CHECK-NEXT: OpBranch %while_continue_0
    // CHECK-NEXT: %while_continue_0 = OpLabel
    // CHECK:      OpBranch %while_header_0
  }
  // CHECK-NEXT: %while_merge_0 = OpLabel

  // CHECK:      OpBranch %while_header_1
  // CHECK-NEXT: %while_header_1 = OpLabel
  // CHECK-NEXT: OpLoopMerge %while_merge_1 %while_continue_1 None
  // CHECK-NEXT: OpBranch %while_check_1
  // CHECK-NEXT: %while_check_1 = OpLabel
  // CHECK:      OpBranchConditional {{%[0-9]+}} %while_body_1 %while_merge_1
  while (a && ((a || b) && b)) {
    // CHECK-NEXT: %while_body_1 = OpLabel
    // CHECK-NEXT: OpBranch %while_continue_1
    // CHECK-NEXT: %while_continue_1 = OpLabel
    // CHECK:      OpBranch %while_header_1
  }
  // CHECK-NEXT: %while_merge_1 = OpLabel

  // CHECK:      OpBranch %while_header_2
  // CHECK-NEXT: %while_header_2 = OpLabel
  // CHECK-NEXT: OpLoopMerge %while_merge_2 %while_continue_2 None
  // CHECK-NEXT: OpBranch %while_check_2
  // CHECK-NEXT: %while_check_2 = OpLabel
  // CHECK:      OpBranchConditional {{%[0-9]+}} %while_body_2 %while_merge_2
  while (a ? a : b) {
    // CHECK-NEXT: %while_body_2 = OpLabel
    // CHECK-NEXT: OpBranch %while_continue_2
    // CHECK-NEXT: %while_continue_2 = OpLabel
    // CHECK:      OpBranch %while_header_2
  }
  // CHECK-NEXT: %while_merge_2 = OpLabel

  int x, y;
  // CHECK:      OpBranch %while_header_3
  // CHECK-NEXT: %while_header_3 = OpLabel
  // CHECK-NEXT: OpLoopMerge %while_merge_3 %while_continue_3 None
  // CHECK-NEXT: OpBranch %while_check_3
  // CHECK-NEXT: %while_check_3 = OpLabel
  // CHECK:      OpBranchConditional {{%[0-9]+}} %while_body_3 %while_merge_3
  while (x + (x && y)) {
    // CHECK-NEXT: %while_body_3 = OpLabel
    // CHECK-NEXT: OpBranch %while_continue_3
    // CHECK-NEXT: %while_continue_3 = OpLabel
    // CHECK:      OpBranch %while_header_3
  }
  // CHECK-NEXT: %while_merge_3 = OpLabel
}
