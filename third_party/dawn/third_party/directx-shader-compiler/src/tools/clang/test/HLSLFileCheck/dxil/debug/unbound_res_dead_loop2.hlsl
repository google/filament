// RUN: %dxc -T ps_6_0 -Od %s | FileCheck %s

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

  float bar = foo;
  bar *= 0;
  bool cond = !bar;

  [branch]
  if (index != 0 && cond) {
    [loop]
    while (index != 0) {
      not_ret += t0.Load(index);
      if (foo < 10) {
        break;
      }
    }
  }

  if (foo > 10) {
    not_ret++;
  }
  else {
    not_ret--;
  }

  ret++;

  return ret;
}

