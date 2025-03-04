// RUN: %dxc -T cs_6_0 -E main -fcgl  %s -spirv | FileCheck %s

void simple() {
  uint a = 0;

// CHECK:    [[a:%[0-9]+]] = OpLoad %uint %a
// CHECK: [[cond:%[0-9]+]] = OpIEqual %bool [[a]] %uint_0
// CHECK:                    OpBranchConditional [[cond]] %if_true %if_false

// CHECK:         %if_true = OpLabel
// CHECK:                    OpStore %d1 %uint_0
// CHECK:                    OpBranch %if_merge_0
// CHECK:        %if_false = OpLabel
// CHECK:    [[a:%[0-9]+]] = OpLoad %uint %a
// CHECK: [[cond:%[0-9]+]] = OpIEqual %bool [[a]] %uint_1
// CHECK:                    OpBranchConditional [[cond]] %if_true_0 %if_false_0

// CHECK:       %if_true_0 = OpLabel
// CHECK:                    OpStore %d1_0 %uint_1
// CHECK:                    OpBranch %if_merge
// CHECK:      %if_false_0 = OpLabel
// CHECK:                    OpBranch %if_merge

// CHECK:        %if_merge = OpLabel
// CHECK:                    OpBranch %if_merge_0

// CHECK:      %if_merge_0 = OpLabel
// CHECK:                    OpReturn
  [branch]
  switch (a) {
    default:
      return;
    case 0: {
      uint d1 = 0;
      return;
    }
    case 1: {
      uint d1 = 1;
    }
  }
}

// CHECK:      [[b:%[0-9]+]] = OpLoad %uint %b
// CHECK:   [[cond:%[0-9]+]] = OpIEqual %bool [[b]] %uint_0
// CHECK:                      OpBranchConditional [[cond]] %if_true_1 %if_false_1

// CHECK:         %if_true_1 = OpLabel
// CHECK:                      OpStore %v1 %uint_0
// CHECK:                      OpStore %v1_0 %uint_2
// CHECK:                      OpStore %v2 %uint_1
// CHECK:                      OpBranch %if_merge_2
// CHECK:        %if_false_1 = OpLabel
// CHECK:      [[b:%[0-9]+]] = OpLoad %uint %b
// CHECK:   [[cond:%[0-9]+]] = OpIEqual %bool [[b]] %uint_1
// CHECK:                      OpBranchConditional [[cond]] %if_true_2 %if_false_2

// CHECK:         %if_true_2 = OpLabel
// CHECK:                      OpStore %v1_0 %uint_2
// CHECK:                      OpStore %v2 %uint_1
// CHECK:                      OpBranch %if_merge_1
// CHECK:        %if_false_2 = OpLabel
// CHECK:                      OpBranch %if_merge_1

// CHECK:        %if_merge_1 = OpLabel
// CHECK:                      OpBranch %if_merge_2

// CHECK:        %if_merge_2 = OpLabel
// CHECK:                      OpReturn
void fallthrough() {
  uint b = 0;

  [branch]
  switch (b) {
    default:
      return;
    case 0: {
      uint v1 = 0;
    }
    case 1: {
      uint v1 = 2;
      uint v2 = 1;
    }
  }
}

[numthreads(1, 1, 1)]
void main()
{
  simple();
  fallthrough();
}
