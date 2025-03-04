// RUN: %dxc -E main -T ps_6_0 %s -Zi -O0 | FileCheck %s

// CHECK-NOT: DW_OP_deref
// CHECK-DAG: dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg0" !DIExpression(DW_OP_bit_piece, 0, 32)
// CHECK-DAG: dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg0" !DIExpression(DW_OP_bit_piece, 32, 32)
// CHECK-DAG: dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg0" !DIExpression(DW_OP_bit_piece, 64, 32)
// CHECK-DAG: dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg1" !DIExpression(DW_OP_bit_piece, 0, 32)
// CHECK-DAG: dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg1" !DIExpression(DW_OP_bit_piece, 32, 32)
// CHECK-DAG: dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"arg1" !DIExpression(DW_OP_bit_piece, 64, 32)
// CHECK-DAG: dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"output" !DIExpression(DW_OP_bit_piece, 0, 32)
// CHECK-DAG: dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"output" !DIExpression(DW_OP_bit_piece, 32, 32)
// CHECK-DAG: dbg.value(metadata float {{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"output" !DIExpression(DW_OP_bit_piece, 64, 32)

void foo(out float3 arg0) {
  arg0 = float3(1,2,3); // @BREAK
  return;
}

void bar(inout float3 arg1) {
  arg1 += float3(1,2,3);
  return;
}

[RootSignature("")]
float3 main() : SV_Target {
  float3 output;
  foo(output);
  bar(output);
  return output;
}

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

