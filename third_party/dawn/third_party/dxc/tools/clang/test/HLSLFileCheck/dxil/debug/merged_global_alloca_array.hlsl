// RUN: %dxc %s /T ps_6_0 /Zi /Od | FileCheck %s
// CHECK-LABEL: @main

// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 0, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 32, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 64, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 96, 32)

// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 896, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 928, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 960, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"global.foo" !DIExpression(DW_OP_bit_piece, 992, 32)

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

struct Foo {
  float4 a;
  float2 b[12];
  float4 c;
};

static Foo foo;

[RootSignature("")]
float4 main(float4 a : A, float2 b : B, float4 c : C, uint i : I) : SV_Target {
  foo.a = a * 2;
  foo.b[0] = b;
  foo.b[1] = b*2;
  foo.b[2] = b*3;
  foo.b[3] = b*5;
  foo.b[4] = b*6;
  foo.b[5] = b*7;
  foo.b[6] = b*8;
  foo.b[7] = b*9;
  foo.b[8] = b*10;
  foo.b[9] = b*11;
  foo.b[10] = b*12;
  foo.b[11] = b*13;
  foo.c = c * 2;
  return foo.a + float4(foo.b[i%4+2], foo.b[i%4]) + foo.c;
}

