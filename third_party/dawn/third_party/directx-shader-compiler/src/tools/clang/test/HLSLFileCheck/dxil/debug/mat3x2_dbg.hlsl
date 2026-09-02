// RUN: %dxc -E main -T vs_6_0 -Zi -Od %s | FileCheck %s

// CHECK: void @llvm.dbg.value(metadata i32 %
// CHECK: void @llvm.dbg.value(metadata i32 %

// CHECK: void @llvm.dbg.value(metadata i32 %
// CHECK: void @llvm.dbg.value(metadata i32 %
// CHECK: void @llvm.dbg.value(metadata i32 %
// CHECK: void @llvm.dbg.value(metadata i32 %
// CHECK: void @llvm.dbg.value(metadata i32 %
// CHECK: void @llvm.dbg.value(metadata i32 %{{.*}}, i64 0, metadata ![[var_md:[0-9]+]], metadata ![[expr_md:[0-9]+]]

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

// CHECK-DAG: ![[var_md]] = !DILocalVariable(tag: DW_TAG_auto_variable, name: "my_mat"
// CHECK-DAG: ![[expr_md]] = !DIExpression(DW_OP_bit_piece,

[RootSignature("")]
uint3x2 main(uint2 uv : TEXCOORD) : MY_MAT {
  uint3x2 my_mat = uint3x2(
    uv.y * 0.5, uv.x * 0.5,
    1.0 - uv.x, 1.0 - uv.x,
    1.0 - uv.x, 1.0 - uv.x
  );
  return my_mat;
}

