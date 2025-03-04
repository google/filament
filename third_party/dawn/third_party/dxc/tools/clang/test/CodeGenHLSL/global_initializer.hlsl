// RUN: %dxc -E main -T ps_6_0 -O0 %s | FileCheck %s

// Make sure global initializer is correctly removed
// when the initial values are constants at codegen
// time.

// CHECK: @main

static float2 x[5] = {
  float2(1, 1) / 2,
  float2(2, 2) / 2,
  float2(3, 3) / 2,
  float2(4, 4) / 2,
  float2(5, 5) / 2,
};

[RootSignature("CBV(b0)")]
float2 main(int i : I) : SV_Target{ 
  return x[i];
}



