// RUN: %dxc -T ps_6_6 -Od %s | FileCheck %s

// Regression check for when resources are used by
// loop values that just move around in a loop but
// don't actually contribute.

// CHECK: @main
// CHECK-NOT: createHandle

Texture2D t0 : register(t0); // Unbound resource

[RootSignature("")]
float main(int foo : FOO) : SV_Target {
  int index = 0;
  float not_ret = 0;
  float ret = 0;

  [loop]
  for (int i = 0; i < foo; i++) {
    index++;
    not_ret += t0.Load(index);
    ret++;
  }

  return ret;
}

