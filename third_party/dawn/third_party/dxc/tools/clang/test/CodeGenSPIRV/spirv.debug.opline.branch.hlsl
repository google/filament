// RUN: %dxc -T ps_6_0 -HV 2018 -E main -Zi -fcgl  %s -spirv | FileCheck %s

// CHECK:      [[file:%[0-9]+]] = OpString
// CHECK-SAME: spirv.debug.opline.branch.hlsl

static int a, b, c;

void main() {
// CHECK:       OpLine [[file]] 11 3
// CHECK-NEXT:  OpBranch %do_while_header
  do {
// CHECK:       OpLine [[file]] 11 6
// CHECK-NEXT:  OpLoopMerge %do_while_merge %do_while_continue None
// CHECK-NEXT:  OpBranch %do_while_body
    if (a < 27) {
      ++a;
// CHECK:       OpLine [[file]] 19 7
// CHECK-NEXT:  OpBranch %do_while_continue
      continue;
    }
    b += a;
// CHECK:       OpLine [[file]] 27 3
// CHECK-NEXT:  OpBranch %do_while_continue

// CHECK:       OpLine [[file]] 27 17
// CHECK-NEXT:  OpBranchConditional {{%[0-9]+}} %do_while_header %do_while_merge
  } while (c < b);

// CHECK:       OpLine [[file]] 33 3
// CHECK-NEXT:  OpBranch %while_check
// CHECK:       OpLine [[file]] 33 3
// CHECK-NEXT:  OpLoopMerge %while_merge %while_continue None
  while (a < c) {
// CHECK:       OpLine [[file]] 37 17
// CHECK-NEXT:  OpSelectionMerge %if_merge_1 None
// CHECK-NEXT:  OpBranchConditional {{%[0-9]+}} %if_true_0 %if_false
    if (b < 34) {
      a = 99;
// CHECK:       OpLine [[file]] 42 25
// CHECK-NEXT:  OpSelectionMerge %if_merge_0 None
// CHECK-NEXT:  OpBranchConditional {{%[0-9]+}} %if_true_1 %if_false_0
    } else if (a > 100) {
      a -= 20;
// CHECK:       OpLine [[file]] 46 7
// CHECK-NEXT:  OpBranch %while_merge
      break;
    } else {
      c = b;
// CHECK:       OpLine [[file]] 51 5
// CHECK-NEXT:  OpBranch %if_merge_0
    }
// CHECK:                        OpLine [[file]] 57 3
// CHECK-NEXT:                   OpBranch %while_continue
// CHECK-NEXT: %while_continue = OpLabel
// CHECK-NEXT:                   OpLine [[file]] 57 3
// CHECK-NEXT:                   OpBranch %while_check
  }

// CHECK:       OpLine [[file]] 61 19
// CHECK-NEXT:  OpBranch %for_check
  for (int i = 0; i < 10 && float(a / b) < 2.7; ++i) {
// CHECK:       OpLine [[file]] 61 44
// CHECK-NEXT:  OpLoopMerge %for_merge %for_continue None
// CHECK-NEXT:  OpBranchConditional {{%[0-9]+}} %for_body %for_merge
    c = a + 2 * b + c;
// CHECK:                      OpLine [[file]] 69 3
// CHECK-NEXT:                 OpBranch %for_continue
// CHECK-NEXT: %for_continue = OpLabel
  }
// CHECK:                      OpLine [[file]] 69 3
// CHECK-NEXT:                 OpBranch %for_check
// CHECK-NEXT:    %for_merge = OpLabel

  switch (a) {
  case 1:
    b = c;
// CHECK:      OpLine [[file]] 79 5
// CHECK-NEXT: OpBranch %switch_merge
    break;
  case 2:
    b = 2 * c;
// CHECK:      OpLine [[file]] 84 3
// CHECK-NEXT: OpBranch %switch_4
  case 4:
    b = b + 4;
    break;
  default:
    b = a;
// CHECK:      OpLine [[file]] 91 3
// CHECK-NEXT: OpBranch %switch_merge
  }
}
