// RUN: %dxc -T ps_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void main() {
  int a, b;
  bool cond = true;

// CHECK:      OpBranch %while_check
// CHECK-NEXT: %while_check = OpLabel
// CHECK-NEXT: [[cond:%[0-9]+]] = OpLoad %bool %cond
// CHECK-NEXT: OpLoopMerge %while_merge %while_continue None
// CHECK-NEXT: OpBranchConditional [[cond]] %while_body %while_merge
  while(cond) {
// CHECK-NEXT: %while_body = OpLabel
// CHECK-NEXT: [[b:%[0-9]+]] = OpLoad %int %b
// CHECK-NEXT: OpSelectionMerge %switch_merge None
// CHECK-NEXT: OpSwitch [[b]] %switch_default 1 %switch_1 2 %switch_2 5 %switch_5
    switch(b) {
// CHECK-NEXT: %switch_1 = OpLabel
// CHECK-NEXT: OpStore %a %int_1
// CHECK-NEXT: OpBranch %switch_merge
      case 1:
        a = 1;
        break;    // Break from the case statement.
// CHECK-NEXT: %switch_2 = OpLabel
// CHECK-NEXT: OpStore %a %int_3
// CHECK-NEXT: OpBranch %while_continue
      case 2: {
        a = 3;
        {continue;} // continue for the while loop.
        a = 4;      // No SPIR-V should be emitted for this statement.
        break;      // No SPIR-V should be emitted for this statement.
      }
// CHECK-NEXT: %switch_5 = OpLabel
// CHECK-NEXT: OpStore %a %int_5
// CHECK-NEXT: OpBranch %switch_merge
      case 5 : {
        a = 5;
        {{break;}} // Break from the case statement.
        a = 6;     // No SPIR-V should be emitted for this statement.
      }
// CHECK-NEXT: %switch_default = OpLabel
// CHECK-NEXT: OpStore %i %int_0
// CHECK-NEXT: OpBranch %for_check
      default:
      // CHECK-NEXT: %for_check = OpLabel
      // CHECK:      [[i_lt_10:%[0-9]+]] = OpSLessThan %bool {{%[0-9]+}} %int_10
      // CHECK-NEXT: OpLoopMerge %for_merge %for_continue None
      // CHECK-NEXT: OpBranchConditional [[i_lt_10]] %for_body %for_merge
        for (int i=0; i<10; ++i) {
        // CHECK-NEXT: %for_body = OpLabel
        // CHECK-NEXT: [[cond1:%[0-9]+]] = OpLoad %bool %cond
        // CHECK-NEXT: OpSelectionMerge %if_merge None
        // CHECK-NEXT: OpBranchConditional [[cond1]] %if_true %if_false
          if (cond) {
          // CHECK-NEXT: %if_true = OpLabel
          // CHECK-NEXT: OpBranch %for_merge
            break;    // Break the for loop.
            break;    // No SPIR-V should be emitted for this statement.
            continue; // No SPIR-V should be emitted for this statement.
            ++a;      // No SPIR-V should be emitted for this statement.
          } else {
          // CHECK-NEXT: %if_false = OpLabel
          // CHECK-NEXT: OpBranch %for_continue
            continue; // continue for the for loop.
            continue; // No SPIR-V should be emitted for this statement.
            break;    // No SPIR-V should be emitted for this statement.
            ++a;      // No SPIR-V should be emitted for this statement.
          }
        // CHECK-NEXT: %if_merge = OpLabel
        // CHECK-NEXT: OpBranch %for_continue
        // CHECK-NEXT: %for_continue = OpLabel
        // CHECK:      OpBranch %for_check
        }
        // CHECK-NEXT: %for_merge = OpLabel
        // CHECK-NEXT: OpBranch %switch_merge
        break;        // Break from the default statement.
    }
    // CHECK-NEXT: %switch_merge = OpLabel

    // CHECK-NEXT: OpBranch %while_merge
    break; // Break the while loop.

  // CHECK-NEXT: %while_continue = OpLabel
  // CHECK-NEXT: OpBranch %while_check
  }
  // CHECK-NEXT: %while_merge = OpLabel
}
