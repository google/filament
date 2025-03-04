// RUN: %dxc -T ps_6_0 -Od -Zi %s | FileCheck %s

// CHECK-NOT: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"ctx" !DIExpression(DW_OP_bit_piece, 64, 32) func:"foo"
// CHECK-NOT: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"ctx" !DIExpression(DW_OP_bit_piece, 64, 32) func:"bar"

// CHECK: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"ctx" !DIExpression(DW_OP_bit_piece, 64, 32) func:"main"

// CHECK-NOT: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"ctx" !DIExpression(DW_OP_bit_piece, 64, 32) func:"foo"
// CHECK-NOT: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"ctx" !DIExpression(DW_OP_bit_piece, 64, 32) func:"bar"

struct Context {
   float a, b, c;
};
void bar(inout Context ctx) {
  ctx.b = ctx.a * 2;
}
void foo(inout Context ctx) {
  ctx.a = 10;
  bar(ctx);
}

float main() : SV_Target {
  Context ctx = (Context)0;
  foo(ctx);
  return ctx.a + ctx.b + ctx.c;
}


