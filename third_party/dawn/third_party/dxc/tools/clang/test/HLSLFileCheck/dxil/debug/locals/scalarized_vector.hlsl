// RUN: %dxc -E main -T vs_6_0 -Zi %s | FileCheck %s

// Test that the vector scalarizer preserves debug information.

// CHECK: %[[x:.*]] = add i32
// CHECK: %[[y:.*]] = add i32
// CHECK-DAG: call void @llvm.dbg.value(metadata i32 %[[x]], i64 0, metadata ![[vec:.*]], metadata ![[xexp:.*]]), !dbg
// CHECK-DAG: call void @llvm.dbg.value(metadata i32 %[[y]], i64 0, metadata ![[vec]], metadata ![[yexp:.*]]), !dbg

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

// CHECK-DAG: ![[vec]] = !DILocalVariable(tag: DW_TAG_auto_variable, name: "vec"
// CHECK-DAG: ![[xexp]] = !DIExpression(DW_OP_bit_piece, 0, 32)
// CHECK-DAG: ![[yexp]] = !DIExpression(DW_OP_bit_piece, 32, 32)

int2 main(int2 a : A, int2 b : B) : OUT
{
    int2 vec = a + b;
    return vec;
}
