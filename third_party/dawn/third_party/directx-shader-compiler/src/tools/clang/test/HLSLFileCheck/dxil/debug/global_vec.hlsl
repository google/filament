// RUN: %dxc -E main -T ps_6_0 %s -Od -Zi | FileCheck %s

// Test for dynamically index vector

static float4 MyGlobal;

// CHECK-NOT: internal global

[RootSignature("")]
float4 main(float4 vec : COLOR, int index : INDEX) : SV_Target {
  MyGlobal = vec.zyxw;
  // CHECK-NOT: alloca
  return MyGlobal;
}

// Exclude quoted source file (see readme)
// CHECK-LABEL: {{!"[^"]*\\0A[^"]*"}}

// CHECK: !DILocalVariable(tag: DW_TAG_arg_variable, name: "global.MyGlobal"

