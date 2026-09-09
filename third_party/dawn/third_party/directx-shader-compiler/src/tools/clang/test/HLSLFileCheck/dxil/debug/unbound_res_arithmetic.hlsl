// RUN: %dxc -T ps_6_6 -Od %s | FileCheck %s

// Regression check for when resources are used by
// arithmetic that can be trivially replaced by constants.

// CHECK: @main
// CHECK-NOT: createHandle

Texture2D t0 : register(t0); // Unbound resource

[RootSignature("")]
float main(int index : INDEX, float foo : FOO) : SV_Target {
  float ret = foo;
  ret += t0.Load(index).x; // Unbound resource
  ret *= 0;
  return ret;
}

