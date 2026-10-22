// RUN: %dxc -T ps_6_0 -HV 2018 -E main -fcgl  %s -spirv | FileCheck %s

// TODO: write to global variable
bool fn() { return true; }
bool fn1() { return false; }
bool fn2() { return true; }

void main() {
// CHECK-LABEL: %bb_entry = OpLabel

    // Use in control flow

    bool a, b, c;
    int val = 0;
// CHECK:      [[a0:%[0-9]+]] = OpLoad %bool %a
// CHECK-NEXT: [[b0:%[0-9]+]] = OpLoad %bool %b
// CHECK-NEXT: [[c0:%[0-9]+]] = OpLoad %bool %c
// CHECK-NEXT: [[s0:%[0-9]+]] = OpSelect %bool [[a0]] [[b0]] [[c0]]
// CHECK-NEXT: OpSelectionMerge %if_merge None
// CHECK-NEXT: OpBranchConditional [[s0]] %if_true %if_merge
    if (a ? b : c) val++;

    // Operand with side effects

// CHECK-LABEL: %if_merge = OpLabel
// CHECK-NEXT: [[fn:%[0-9]+]] = OpFunctionCall %bool %fn
// CHECK-NEXT: [[fn1:%[0-9]+]] = OpFunctionCall %bool %fn1
// CHECK-NEXT: [[fn2:%[0-9]+]] = OpFunctionCall %bool %fn2
// CHECK-NEXT: [[s1:%[0-9]+]] = OpSelect %bool [[fn]] [[fn1]] [[fn2]]
// CHECK-NEXT: OpSelectionMerge %if_merge_0 None
// CHECK-NEXT: OpBranchConditional [[s1]] %if_true_0 %if_merge_0
    if (fn() ? fn1() : fn2()) val++;
}
