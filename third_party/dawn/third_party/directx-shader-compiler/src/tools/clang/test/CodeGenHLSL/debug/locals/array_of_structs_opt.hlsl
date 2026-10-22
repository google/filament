// RUN: %dxc -E main -T vs_6_0 -Zi %s | FileCheck %s

// Check that debug info is preserved for arrays of structs
// getting SROA'd down into arrays of struct elements,
// then SROA'd into individual allocas and promoted to registers,
// when compiling with optimizations.

// CHECK-DAG: %[[i1:.*]] = extractvalue %dx.types.CBufRet.i32 %{{.*}}, 0
// CHECK-DAG: %[[f1:.*]] = extractvalue %dx.types.CBufRet.f32 %{{.*}}, 1
// CHECK-DAG: %[[i2:.*]] = extractvalue %dx.types.CBufRet.i32 %{{.*}}, 2
// CHECK-DAG: %[[f2:.*]] = extractvalue %dx.types.CBufRet.f32 %{{.*}}, 3

// CHECK-DAG: call void @llvm.dbg.value(metadata i32 %[[i1]], i64 0, metadata !{{.*}}, metadata ![[i1expr:.*]])
// CHECK-DAG: call void @llvm.dbg.value(metadata float %[[f1]], i64 0, metadata !{{.*}}, metadata ![[f1expr:.*]])
// CHECK-DAG: call void @llvm.dbg.value(metadata i32 %[[i2]], i64 0, metadata !{{.*}}, metadata ![[i2expr:.*]])
// CHECK-DAG: call void @llvm.dbg.value(metadata float %[[f2]], i64 0, metadata !{{.*}}, metadata ![[f2expr:.*]])

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

// CHECK-DAG: !DILocalVariable(tag: DW_TAG_auto_variable, name: "var"

// CHECK-DAG: ![[i1expr]] = !DIExpression(DW_OP_bit_piece, 0, 32)
// CHECK-DAG: ![[f1expr]] = !DIExpression(DW_OP_bit_piece, 32, 32)
// CHECK-DAG: ![[i2expr]] = !DIExpression(DW_OP_bit_piece, 64, 32)
// CHECK-DAG: ![[f2expr]] = !DIExpression(DW_OP_bit_piece, 96, 32)

struct intfloat { int i; float f; };

int cb_i1; float cb_f1;
int cb_i2; float cb_f2;

void main(
    out int o_i1 : I1, out float o_f1 : F1,
    out int o_i2 : I2, out float o_f2 : F2)
{
  intfloat var[2] = { cb_i1, cb_f1, cb_i2, cb_f2 };
  
  o_i1 = var[0].i; o_f1 = var[0].f;
  o_i2 = var[1].i; o_f2 = var[1].f;
}