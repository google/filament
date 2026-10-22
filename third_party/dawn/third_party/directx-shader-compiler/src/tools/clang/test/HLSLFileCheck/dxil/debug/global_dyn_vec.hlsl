// RUN: %dxc -E main -T ps_6_0 %s -Od | FileCheck %s

// Test for dynamically index vector

static float4 MyGlobal;

// CHECK-NOT: internal global

[RootSignature("")]
float main(float4 vec : COLOR, int index : INDEX) : SV_Target {
  MyGlobal = vec.zyxw;
  // CHECK: alloca [4 x float]

  // CHECK: store
  // CHECK: store
  // CHECK: store
  // CHECK: store

  // CHECK: load
  return MyGlobal[index];
}


