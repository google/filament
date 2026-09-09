// RUN: %dxc %s /T ps_6_0 /Zi /Od | FileCheck %s

// CHECK-LABEL: @main

// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 0, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 32, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 64, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 96, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 128, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 160, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 192, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 224, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 256, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 288, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 320, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 352, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 384, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 416, 32)

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

struct Baz {
  float4 a;
};

struct Bar_ {
  float4 a;
  Baz baz;
};
typedef Bar_ Bar;

struct Foo {
  float4 a;
  Bar bar;
  float2 b;
};

static Foo foo;

[RootSignature("")]
float4 main(float4 a : A, float2 b : B) : SV_Target {
  foo.a = a * 2;
  foo.bar.a = a /2;
  foo.bar.baz.a = a + float4(2,2,2,2);
  foo.b = b * 2;
  return foo.a + float4(foo.b, foo.b) * foo.bar.a + foo.bar.baz.a;
}

