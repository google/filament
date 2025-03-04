// RUN: %dxc -E main -T ps_6_0 -Od %s | FileCheck %s

// CHECK: @main
// CHECK-NOT: Sin(

// Make sure the loop is deleted by erase dead region.

cbuffer cb : register(b0) {
  float4 input;
}

[RootSignature("CBV(b1)")]
float main() : SV_Target {
  float4 foo = input;
  float ret = 0;
  [loop]
  for (int i = 0; i < 4; i++) {
    if (foo[i]) {
      ret += sin(foo[i]);
    }
    else {
      ret -= 1;
    }
  }

  return 0;
}

