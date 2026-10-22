// RUN: %dxc -E main -T vs_6_0 -Zi %s | FileCheck %s

// Test that local matrices preserve debug info with optimizations

// CHECK: call void @llvm.dbg.value
// CHECK: call void @llvm.dbg.value
// CHECK: call void @llvm.dbg.value
// CHECK: call void @llvm.dbg.value

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

// CHECK-DAG: !DILocalVariable(tag: DW_TAG_auto_variable, name: "mat"
// CHECK-DAG: !DIExpression(DW_OP_bit_piece, 0, 32)
// CHECK-DAG: !DIExpression(DW_OP_bit_piece, 32, 32)
// CHECK-DAG: !DIExpression(DW_OP_bit_piece, 64, 32)
// CHECK-DAG: !DIExpression(DW_OP_bit_piece, 96, 32)

int2x2 cb_mat;
int main() : OUT
{
  // Initialize from CB to ensure the variable is not optimized away
  int2x2 mat = cb_mat;
  // Consume all values but return a scalar to avoid another alloca [4 x i32]
  return determinant(mat);
}