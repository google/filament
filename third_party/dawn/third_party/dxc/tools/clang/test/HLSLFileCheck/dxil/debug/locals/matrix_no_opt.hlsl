// RUN: %dxc -E main -T vs_6_0 -Zi -Od %s | FileCheck %s

// Test that local matrices preserve debug info without optimizations

// CHECK: @llvm.dbg.value(metadata i32 %{{.*}}, metadata ![[divar:.*]], metadata ![[diexpr0:[0-9]+]]
// CHECK: @llvm.dbg.value(metadata i32 %{{.*}}, metadata ![[divar]], metadata ![[diexpr1:[0-9]+]]
// CHECK: @llvm.dbg.value(metadata i32 %{{.*}}, metadata ![[divar]], metadata ![[diexpr2:[0-9]+]]
// CHECK: @llvm.dbg.value(metadata i32 %{{.*}}, metadata ![[divar]], metadata ![[diexpr3:[0-9]+]]

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

// CHECK-DAG: ![[divar]] = !DILocalVariable(tag: DW_TAG_auto_variable, name: "mat"
// CHECK-DAG: ![[diexpr0]] = !DIExpression(DW_OP_bit_piece, {{[0-9]+}}, {{[0-9]+}})
// CHECK-DAG: ![[diexpr1]] = !DIExpression(DW_OP_bit_piece, {{[0-9]+}}, {{[0-9]+}})
// CHECK-DAG: ![[diexpr2]] = !DIExpression(DW_OP_bit_piece, {{[0-9]+}}, {{[0-9]+}})
// CHECK-DAG: ![[diexpr3]] = !DIExpression(DW_OP_bit_piece, {{[0-9]+}}, {{[0-9]+}})

int2x2 cb_mat;
int main() : OUT
{
  // Initialize from CB to ensure the variable is not optimized away
  int2x2 mat = cb_mat;
  // Consume all values but return a scalar to avoid another alloca [4 x i32]
  return determinant(mat);
}
