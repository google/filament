// RUN: %dxc -T ps_6_2 -HV 2021 -E main -enable-16bit-types -fcgl  %s -spirv | FileCheck %s

// Check that the literals get a 16-bit type, and the result of the select is
// then cast to an unsigned 16-bit value.
void foo(uint x) {
// CHECK:      %foo = OpFunction
// CHECK-NEXT: [[param:%[a-zA-Z0-9_]+]] = OpFunctionParameter %_ptr_Function_uint
// CHECK-NEXT: OpLabel
// CHECK-NEXT: [[value:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_ushort Function
// CHECK-NEXT: [[temp:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_short Function
// CHECK-NEXT: [[ld:%[a-zA-Z0-9_]+]] = OpLoad %uint [[param]]
// CHECK-NEXT: [[cmp:%[a-zA-Z0-9_]+]] = OpULessThan %bool [[ld]] %uint_64
// CHECK-NEXT: OpSelectionMerge [[merge_bb:%[a-zA-Z0-9_]+]] None
// CHECK-NEXT: OpBranchConditional [[cmp]] [[true_bb:%[a-zA-Z0-9_]+]] [[false_bb:%[a-zA-Z0-9_]+]]
// CHECK-NEXT: [[true_bb]] = OpLabel
// CHECK-NEXT: OpStore [[temp]] %short_1
// CHECK-NEXT: OpBranch [[merge_bb]]
// CHECK-NEXT: [[false_bb]] = OpLabel
// CHECK-NEXT: OpStore [[temp]] %short_0
// CHECK-NEXT: OpBranch [[merge_bb]]
// CHECK-NEXT: [[merge_bb]] = OpLabel
// CHECK-NEXT: [[ld2:%[a-zA-Z0-9_]+]] = OpLoad %short [[temp]]
// CHECK-NEXT: [[res:%[a-zA-Z0-9_]+]] = OpBitcast %ushort [[ld2]]
// CHECK-NEXT: OpStore [[value]] [[res_0:%[a-zA-Z0-9_]+]]
  uint16_t value = x < 64 ? 1 : 0;
}

// Check that the literals get a 16-bit type, and the result of the select is
// then cast to an signed 16-bit value. Note that the bitcast is redundant in
// this case, but we add the bitcast before the type of the literal has been
// determined, so we add it anyway.
void bar(uint x) {
// CHECK:      %bar = OpFunction
// CHECK-NEXT: [[param_0:%[a-zA-Z0-9_]+]] = OpFunctionParameter %_ptr_Function_uint
// CHECK-NEXT: OpLabel
// CHECK-NEXT: [[value_0:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_short Function
// CHECK-NEXT: [[temp_0:%[a-zA-Z0-9_]+]] = OpVariable %_ptr_Function_short Function
// CHECK-NEXT: [[ld_0:%[a-zA-Z0-9_]+]] = OpLoad %uint [[param_0]]
// CHECK-NEXT: [[cmp_0:%[a-zA-Z0-9_]+]] = OpULessThan %bool [[ld_0]] %uint_64
// CHECK-NEXT: OpSelectionMerge [[merge_bb_0:%[a-zA-Z0-9_]+]] None
// CHECK-NEXT: OpBranchConditional [[cmp_0]] [[true_bb_0:%[a-zA-Z0-9_]+]] [[false_bb_0:%[a-zA-Z0-9_]+]]
// CHECK-NEXT: [[true_bb_0]] = OpLabel
// CHECK-NEXT: OpStore [[temp_0]] %short_1
// CHECK-NEXT: OpBranch [[merge_bb_0]]
// CHECK-NEXT: [[false_bb_0]] = OpLabel
// CHECK-NEXT: OpStore [[temp_0]] %short_0
// CHECK-NEXT: OpBranch [[merge_bb_0]]
// CHECK-NEXT: [[merge_bb_0]] = OpLabel
// CHECK-NEXT: [[ld2_0:%[a-zA-Z0-9_]+]] = OpLoad %short [[temp_0]]
// CHECK-NEXT: [[res_1:%[a-zA-Z0-9_]+]] = OpBitcast %short [[ld2_0]]
// CHECK-NEXT: OpStore [[value_0]] [[res_2:%[a-zA-Z0-9_]+]]
  int16_t value = x < 64 ? 1 : 0;
}

void main() {
  uint value;
  foo(2);
  bar(2);
}
