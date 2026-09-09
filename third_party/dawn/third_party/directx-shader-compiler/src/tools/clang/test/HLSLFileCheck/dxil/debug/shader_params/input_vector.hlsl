// RUN: %dxc -E main -T vs_6_0 -Zi %s | FileCheck %s

// Test that an input vector's debug information is preserved,
// especially through its lowering to per-element loadInputs.

// CHECK: call i32 @dx.op.loadInput.i32
// CHECK: call void @llvm.dbg.value
// CHECK: call i32 @dx.op.loadInput.i32
// CHECK: call void @llvm.dbg.value
// CHECK: call void @dx.op.storeOutput.i32
// CHECK: call void @dx.op.storeOutput.i32
// CHECK: ret void

// Exclude quoted source file (see readme)
// CHECK: {{!"[^"]*\\0A[^"]*"}}

// CHECK: !DILocalVariable(tag: DW_TAG_arg_variable, name: "a", arg: 1
// CHECK: !DIExpression(DW_OP_bit_piece, 0, 32)
// CHECK: !DIExpression(DW_OP_bit_piece, 32, 32)

int2 main(int2 a : IN) : OUT { return a; }