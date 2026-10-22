// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

int foo() { return true; }

void main() {
  int val = 0;
  int i = 0;

// CHECK:      OpBranch %do_while_header
// CHECK-NEXT: %do_while_header = OpLabel
// CHECK-NEXT: OpLoopMerge %do_while_merge %do_while_continue None
// CHECK-NEXT: OpBranch %do_while_body
  do {
// CHECK-NEXT: %do_while_body = OpLabel
    ++i;
// CHECK:      OpSelectionMerge %if_merge None
// CHECK-NEXT: OpBranchConditional {{%[0-9]+}} %if_true %if_merge
    if (i > 5) {
// CHECK-NEXT: %if_true = OpLabel
// CHECK-NEXT: OpBranch %do_while_merge
      break;
      break;       // No SPIR-V should be emitted for this statement.
      val = i;     // No SPIR-V should be emitted for this statement.
      while(true); // No SPIR-V should be emitted for this statement.
    }
// CHECK-NEXT: %if_merge = OpLabel
    val = i;
// CHECK:      OpBranch %do_while_merge
    {{break;}}
    val = val * 2; // No SPIR-V should be emitted for this statement.

// CHECK-NEXT: %do_while_continue = OpLabel
// CHECK:      OpBranchConditional {{%[0-9]+}} %do_while_header %do_while_merge
  } while (i < 10);
// CHECK-NEXT: %do_while_merge = OpLabel
  
  
  ////////////////////////////////////////////////////////////////////////////////
  // Nested do-while loops with break statements                                //
  // Each break statement should branch to the corresponding loop's break block //
  ////////////////////////////////////////////////////////////////////////////////

// CHECK-NEXT: OpBranch %do_while_header_0

// CHECK-NEXT: %do_while_header_0 = OpLabel
// CHECK-NEXT: OpLoopMerge %do_while_merge_1 %do_while_continue_1 None
// CHECK-NEXT: OpBranch %do_while_body_0
  do {
// CHECK-NEXT: %do_while_body_0 = OpLabel
    ++i;
// CHECK:      OpBranch %do_while_header_1
// CHECK-NEXT: %do_while_header_1 = OpLabel
// CHECK-NEXT: OpLoopMerge %do_while_merge_0 %do_while_continue_0 None
// CHECK-NEXT: OpBranch %do_while_body_1
    do {
// CHECK-NEXT: %do_while_body_1 = OpLabel
      ++val;
// CHECK:      OpBranch %do_while_merge_0
      break;
// CHECK-NEXT: %do_while_continue_0 = OpLabel
// CHECK:      OpBranchConditional {{%[0-9]+}} %do_while_header_1 %do_while_merge_0
    } while (i < 10);
// CHECK-NEXT: %do_while_merge_0 = OpLabel
    --i;
// CHECK:      OpBranch %do_while_merge_1
    {break;}
// CHECK-NEXT: %do_while_continue_1 = OpLabel
// CHECK:      OpBranchConditional {{%[0-9]+}} %do_while_header_0 %do_while_merge_1
  } while(val < 10);
// CHECK-NEXT: %do_while_merge_1 = OpLabel
}
