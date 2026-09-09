// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

int foo() { return true; }

void main() {
  int val = 0;
  int i = 0;



    /////////////////////////////
    //// Basic do-while loop ////
    /////////////////////////////

// CHECK:      OpBranch %do_while_header
// CHECK-NEXT: %do_while_header = OpLabel
// CHECK-NEXT: OpLoopMerge %do_while_merge %do_while_continue None
// CHECK-NEXT: OpBranch %do_while_body
  do {
// CHECK-NEXT: %do_while_body = OpLabel
// CHECK-NEXT: [[i0:%[0-9]+]] = OpLoad %int %i
// CHECK-NEXT: OpStore %val [[i0]]
// CHECK-NEXT: OpBranch %do_while_continue
      val = i;
// CHECK-NEXT: %do_while_continue = OpLabel
// CHECK-NEXT: [[i1:%[0-9]+]] = OpLoad %int %i
// CHECK-NEXT: [[i_lt_10:%[0-9]+]] = OpSLessThan %bool [[i1]] %int_10
// CHECK-NEXT: OpBranchConditional [[i_lt_10]] %do_while_header %do_while_merge
  } while (i < 10);
// CHECK-NEXT: %do_while_merge = OpLabel



    //////////////////////////
    ////  infinite loop   ////
    //////////////////////////

// CHECK-NEXT: OpBranch %do_while_header_0
// CHECK-NEXT: %do_while_header_0 = OpLabel
// CHECK-NEXT: OpLoopMerge %do_while_merge_0 %do_while_continue_0 None
// CHECK-NEXT: OpBranch %do_while_body_0
  do {
// CHECK-NEXT: %do_while_body_0 = OpLabel
// CHECK-NEXT: OpStore %val %int_0
// CHECK-NEXT: OpBranch %do_while_continue_0
    val = 0;
// CHECK-NEXT: %do_while_continue_0 = OpLabel
// CHECK-NEXT: OpBranchConditional %true %do_while_header_0 %do_while_merge_0
  } while (true);
// CHECK-NEXT: %do_while_merge_0 = OpLabel




    //////////////////////////
    ////    Null Body     ////
    //////////////////////////
// CHECK-NEXT: OpBranch %do_while_header_1
// CHECK-NEXT: %do_while_header_1 = OpLabel
// CHECK-NEXT: OpLoopMerge %do_while_merge_1 %do_while_continue_1 None
// CHECK-NEXT: OpBranch %do_while_body_1
  do {
// CHECK-NEXT: %do_while_body_1 = OpLabel
// CHECK-NEXT: OpBranch %do_while_continue_1

// CHECK-NEXT: %do_while_continue_1 = OpLabel
// CHECK-NEXT: [[val:%[0-9]+]] = OpLoad %int %val
// CHECK-NEXT: [[val_lt_20:%[0-9]+]] = OpSLessThan %bool [[val]] %int_20
// CHECK-NEXT: OpBranchConditional [[val_lt_20]] %do_while_header_1 %do_while_merge_1
  } while (val < 20);
// CHECK-NEXT: %do_while_merge_1 = OpLabel


// CHECK-NEXT: OpReturn
// CHECK-NEXT: OpFunctionEnd

}
