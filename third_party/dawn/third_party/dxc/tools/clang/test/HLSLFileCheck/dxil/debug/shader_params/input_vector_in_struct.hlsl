// RUN: %dxc -E main -T vs_6_0 -Zi %s | FileCheck %s

// Test that debug offsets are correct after lowering a vector
// input to loadInput calls, when the vector input itself was
// offset in a struct.

// CHECK: call i32 @dx.op.loadInput.i32
// CHECK: call void @llvm.dbg.value
// CHECK: call void @dx.op.storeOutput.i32
// CHECK: ret void

// Exclude quoted source file (see readme)
// CHECK: {{!"[^"]*\\0A[^"]*"}}

// CHECK: !DILocalVariable(tag: DW_TAG_arg_variable, name: "s", arg: 1
// CHECK: !DIExpression(DW_OP_bit_piece, 96, 32)

struct S { int2 a, b; };
int main(S s : IN) : OUT { return s.b.y; }