// RUN: %dxc -Zi -Od -E main -T ps_6_0 %s | FileCheck %s

// Regression test for structs with just a single 32-bit member.

// CHECK-DAG: call void @llvm.dbg.value(metadata float %{{.*}}, i64 0, metadata !{{[0-9]+}}, metadata !{{[0-9]+}}), !dbg !{{[0-9]+}} ; var:"my_s" !DIExpression()

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

struct MyStruct {
  float1 foo;
};

[RootSignature("")]
float main(float a : A) : SV_Target {
  MyStruct my_s;
  my_s.foo.x = a;
  return my_s.foo.x;
}

