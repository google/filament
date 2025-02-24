// RUN: %dxc -E main -T ps_6_0 -Zi -Od %s | FileCheck %s


// CHECK-DAG: call void @llvm.dbg.value(metadata i32 %{{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"uv" !DIExpression(DW_OP_bit_piece, 0, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata i32 %{{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"uv" !DIExpression(DW_OP_bit_piece, 32, 32)

// CHECK-DAG: call void @llvm.dbg.value(metadata i32 %{{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"my_uv" !DIExpression(DW_OP_bit_piece, 0, 32)
// CHECK-DAG: call void @llvm.dbg.value(metadata i32 %{{.+}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"my_uv" !DIExpression(DW_OP_bit_piece, 32, 32)

[RootSignature("")]
float2 main(uint2 uv : TEXCOORD) : SV_Target {
  uint2 my_uv = {
    uv.y * 0.5,
    1.0 - uv.x,
  };
  return my_uv;
}

