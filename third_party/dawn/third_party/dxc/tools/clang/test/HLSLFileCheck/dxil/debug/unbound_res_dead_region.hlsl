// RUN: %dxc -T ps_6_6 -Od %s | FileCheck %s

// Regression check for when resources are used by
// arithmetic that can be trivially replaced by constants.

// Notes: This test relies on root signature validation.

// CHECK: @main
// CHECK-NOT: createHandle

Texture2D t0 : register(t0); // Unbound resource

[RootSignature("")]
float main(int index : INDEX, float foo : FOO) : SV_Target {
  float ret = 0;
  float non_contributing = 0;
  if (t0.Load(index).x) {
    non_contributing += 1;
  }
  else {
    non_contributing += 2;
  }
  return ret;
}

