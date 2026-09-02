// RUN: %dxc -E main -T vs_6_0 -Zi -Od %s | FileCheck %s

// Check that debug info is preserved with stride information
// for arrays of structs getting SROA'd down into arrays of struct elements,
// when compiling without optimizations.

// CHECK-DAG: %[[intalloca:.*]] = alloca [2 x i32]
// CHECK-DAG: %[[floatalloca:.*]] = alloca [2 x float]

// CHECK-DAG: call void @llvm.dbg.declare(metadata [2 x i32]* %[[intalloca]], metadata !{{.*}}, metadata ![[intdiexpr:.*]]), !dbg !{{.*}}, !dx.dbg.varlayout ![[intlayout:.*]]
// CHECK-DAG: call void @llvm.dbg.declare(metadata [2 x float]* %[[floatalloca]], metadata !{{.*}}, metadata ![[floatdiexpr:.*]]), !dbg !{{.*}}, !dx.dbg.varlayout ![[floatlayout:.*]]

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

// CHECK-DAG: !DILocalVariable(tag: DW_TAG_auto_variable, name: "var"

// CHECK-DAG: ![[intdiexpr]] = !DIExpression(DW_OP_bit_piece, 0, 32)
// CHECK-DAG: ![[intlayout]] = !{i32 0, i32 64, i32 2}
// CHECK-DAG: ![[floatdiexpr]] = !DIExpression(DW_OP_bit_piece, 32, 32)
// CHECK-DAG: ![[floatlayout]] = !{i32 32, i32 64, i32 2}

struct intfloat { int i; float f; };
float4 main(int i : IN) : OUT
{
  intfloat var[2] = (intfloat[2])i;
  return float4(var[0].i, var[0].f, var[1].i, var[1].f);
}