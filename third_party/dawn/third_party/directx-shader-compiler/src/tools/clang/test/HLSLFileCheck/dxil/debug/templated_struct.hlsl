// RUN: %dxc -E main -T ps_6_0 %s -Zi -O0 | FileCheck %s
// RUN: %dxc -E main -T ps_6_0 %s -Zi -O3 | FileCheck %s

// We check that the broken down elements directly point to the value of the struct S

// CHECK: dbg.value(metadata float %{{.+}}, i64 0, metadata ![[this:.+]], metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"this" !DIExpression(DW_OP_bit_piece, 0, 32)
// CHECK: dbg.value(metadata float %{{.+}}, i64 0, metadata ![[this]], metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"this" !DIExpression(DW_OP_bit_piece, 32, 32)
// CHECK-DAG: ![[this]] = !DILocalVariable(tag: DW_TAG_arg_variable, name: "this", arg: 1, scope: !{{[0-9]+}}, type: ![[type:[0-9]+]])
// CHECK-DAG: ![[type]] = !DICompositeType(tag: DW_TAG_structure_type, name: "S<float>", file: !{{[0-9]+}}, line: {{[0-9]+}}, size: 64, align: 32, elements: !{{[0-9]+}}, templateParams: !{{[0-9]+}})

template<typename T>
struct S {
  T a, b;
  T Get() {
    return a + b;
  }
};

[RootSignature("")]
float main(float x : X, float y : Y, float z : Z, float w : W) : SV_Target {
  S<float> s;
  s.a = x;
  s.b = y;
  return s.Get();
}


