// RUN: %dxc -E main -T ps_6_0 %s -Od | FileCheck %s

// Check that arrays of vectors still work with -Od
// without all the inst-simplify

[RootSignature("")]
float2 main(int index : INDEX) : SV_Target {

  float2 values[4] = {
    float2(1,2),
    float2(3,4),
    float2(5,6),
    float2(7,8),
  };

  // CHECK: alloca [4 x float]
  // CHECK: alloca [4 x float]

  // CHECK: store
  // CHECK: store
  // CHECK: store
  // CHECK: store

  // CHECK: store
  // CHECK: store
  // CHECK: store
  // CHECK: store

  // CHECK: load
  // CHECK: load

  return values[index];
}

