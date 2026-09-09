// RUN: %dxc -E main -T ps_6_0 -Od -Zi %s | FileCheck %s

// Check that the debug info for a global variable is present for every inlined scope

// CHECK: @main

// CHECK-DAG: ![[main_scope:[0-9]+]] = !DISubprogram(name: "main"
// CHECK-DAG: ![[main_var:[0-9]+]] = !DILocalVariable(tag: DW_TAG_arg_variable, name: "global.g_cond", arg: {{[0-9]+}}, scope: ![[main_scope]],
// CHECK-DAG: call void @llvm.dbg.value(metadata i32 %{{.+}}, metadata ![[main_var]],

// CHECK-DAG: ![[foo_scope:[0-9]+]] = !DISubprogram(name: "foo"
// CHECK-DAG: ![[foo_var:[0-9]+]] = !DILocalVariable(tag: DW_TAG_arg_variable, name: "global.g_cond", arg: {{[0-9]+}}, scope: ![[foo_scope]],
// CHECK-DAG: call void @llvm.dbg.value(metadata i32 %{{.+}}, metadata ![[foo_var]],

// CHECK-DAG: ![[bar_scope:[0-9]+]] = !DISubprogram(name: "bar"
// CHECK-DAG: ![[bar_var:[0-9]+]] = !DILocalVariable(tag: DW_TAG_arg_variable, name: "global.g_cond", arg: {{[0-9]+}}, scope: ![[bar_scope]],
// CHECK-DAG: call void @llvm.dbg.value(metadata i32 %{{.+}}, metadata ![[bar_var]],

static bool g_cond;

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);

float4 bar() {
  if (g_cond)
    return tex0.Load(1);
  return 0;
}

float4 foo() {
  float4 ret = g_cond ? tex0.Load(0) : tex1.Load(0);
  ret += bar();
  return ret;
}

[RootSignature("DescriptorTable(SRV(t0, numDescriptors=2))")]
float4 main(uint a : A) : sv_target {
  g_cond = a != 0;
  return foo();
};



