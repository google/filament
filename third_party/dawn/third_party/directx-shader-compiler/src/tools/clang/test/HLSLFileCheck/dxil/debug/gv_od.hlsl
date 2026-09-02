// RUN: %dxc -E main -T ps_6_0 -Od -Zi %s | FileCheck %s

// Regression test for making sure that static variables
// still work with -Od.

// CHECK: @main

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

// CHECK: !DILocalVariable(tag: DW_TAG_arg_variable, name: "global.gG"

static bool gG;

Texture2D tex0 : register(t0);
Texture2D tex1 : register(t1);

Texture2D f(bool foo) {
  return foo ? tex0 : tex1;
}

[RootSignature("DescriptorTable(SRV(t0, numDescriptors=2))")]
float4 main() : sv_target {
  gG = true;
  return f(gG).Load(0);
};
