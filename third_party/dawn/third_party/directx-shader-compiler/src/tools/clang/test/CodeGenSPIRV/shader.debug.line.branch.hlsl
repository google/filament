// RUN: %dxc -T ps_6_0 -HV 2018 -E main -fspv-debug=vulkan -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[file:%[0-9]+]] = OpString
// CHECK:      [[dbgsrc:%[0-9]+]] = OpExtInst %void %1 DebugSource [[file]]

static int a, b, c;

void main() {
// CHECK:       DebugLine [[dbgsrc]] %uint_11 %uint_11 %uint_3 %uint_3
// CHECK-NEXT:  OpBranch %do_while_header
  do {
// CHECK:       DebugLine [[dbgsrc]] %uint_11 %uint_11 %uint_6 %uint_6
// CHECK-NEXT:  OpLoopMerge %do_while_merge %do_while_continue None
// CHECK-NEXT:  OpBranch %do_while_body
    if (a < 27) {
      ++a;
// CHECK:       DebugLine [[dbgsrc]] %uint_19 %uint_19 %uint_7 %uint_7
// CHECK-NEXT:  OpBranch %do_while_continue
      continue;
    }
    b += a;
// CHECK:       DebugLine [[dbgsrc]] %uint_27 %uint_27 %uint_3 %uint_3
// CHECK-NEXT:  OpBranch %do_while_continue

// CHECK:       DebugLine [[dbgsrc]] %uint_27 %uint_27 %uint_17 %uint_17
// CHECK-NEXT:  OpBranchConditional {{%[0-9]+}} %do_while_header %do_while_merge
  } while (c < b);

// CHECK:       DebugLine [[dbgsrc]] %uint_33 %uint_33 %uint_3 %uint_3
// CHECK-NEXT:  OpBranch %while_check
// CHECK:       DebugLine [[dbgsrc]] %uint_33 %uint_33 %uint_10 %uint_14
// CHECK:       OpLoopMerge %while_merge %while_continue None
  while (a < c) {
// CHECK:       DebugLine [[dbgsrc]] %uint_37 %uint_37 %uint_9 %uint_13
// CHECK:       OpSelectionMerge %if_merge_1 None
// CHECK-NEXT:  OpBranchConditional {{%[0-9]+}} %if_true_0 %if_false
    if (b < 34) {
      a = 99;
// CHECK:       DebugLine [[dbgsrc]] %uint_42 %uint_42 %uint_16 %uint_20 
// CHECK:       OpSelectionMerge %if_merge_0 None
// CHECK-NEXT:  OpBranchConditional {{%[0-9]+}} %if_true_1 %if_false_0
    } else if (a > 100) {
      a -= 20;
// CHECK:       DebugLine [[dbgsrc]] %uint_46 %uint_46 %uint_7 %uint_7
// CHECK-NEXT:  OpBranch %while_merge
      break;
    } else {
      c = b;
// CHECK:       DebugLine [[dbgsrc]] %uint_51 %uint_51 %uint_5 %uint_5
// CHECK-NEXT:  OpBranch %if_merge_0
    }
// CHECK:                        DebugLine [[dbgsrc]] %uint_57 %uint_57 %uint_3 %uint_3
// CHECK-NEXT:                   OpBranch %while_continue
// CHECK-NEXT: %while_continue = OpLabel
// CHECK:                        DebugLine [[dbgsrc]] %uint_57 %uint_57 %uint_3 %uint_3
// CHECK-NEXT:                   OpBranch %while_check
  }

// CHECK:       DebugLine [[dbgsrc]] %uint_61 %uint_61 %uint_8 %uint_17
// CHECK-NEXT:  OpBranch %for_check
  for (int i = 0; i < 10 && float(a / b) < 2.7; ++i) {
// CHECK:       DebugLine [[dbgsrc]] %uint_61 %uint_61 %uint_19 %uint_44
// CHECK:       OpLoopMerge %for_merge %for_continue None
// CHECK-NEXT:  OpBranchConditional {{%[0-9]+}} %for_body %for_merge
    c = a + 2 * b + c;
// CHECK:                      DebugLine [[dbgsrc]] %uint_61 %uint_61 %uint_49 %uint_51
// CHECK-NEXT:                 OpBranch %for_continue
// CHECK-NEXT: %for_continue = OpLabel
  }
// CHECK:                      DebugLine [[dbgsrc]] %uint_61 %uint_61 %uint_49 %uint_51
// CHECK:                      OpBranch %for_check
// CHECK-NEXT:    %for_merge = OpLabel

  switch (a) {
  case 1:
    b = c;
// CHECK:      DebugLine [[dbgsrc]] %uint_79 %uint_79 %uint_5 %uint_5
// CHECK-NEXT: OpBranch %switch_merge
    break;
  case 2:
    b = 2 * c;
// CHECK:      DebugLine [[dbgsrc]] %uint_84 %uint_84 %uint_3 %uint_3
// CHECK-NEXT: OpBranch %switch_4
  case 4:
    b = b + 4;
    break;
  default:
    b = a;
// CHECK:      DebugLine [[dbgsrc]] %uint_86 %uint_86 %uint_5 %uint_5
// CHECK-NEXT: OpBranch %switch_merge
  }
}
