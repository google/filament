// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// CHECK: @main

[RootSignature("")]
float2 main(float2 a : A) : SV_Target {
  precise float2 vec = a;
  if (a.x > 0)
    vec = a*2;
  return vec;
}

