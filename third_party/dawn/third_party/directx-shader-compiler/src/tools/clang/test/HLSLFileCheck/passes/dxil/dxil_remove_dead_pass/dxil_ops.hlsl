// RUN: %dxc %s -T ps_6_0 -Od | FileCheck %s

// Regression test for dxil operations not being evaluated.

// CHECK: @main

Texture2D t0 : register(t0);
Texture2D t1 : register(t1);

static const uint global = 1;
static const uint global2 = 2;

static const uint global3[3] = { 0, 1, 1 };

cbuffer cb {
  float bar, baz;
};

Texture2D foo(float x, float y, float z) {
  int i;
  i = mad(bar, 0, y); // 0
  [branch]
  if (mad(x, y, 0) == 0) { // true
    i = mad(bar, 0, x); // 1
  }

  int j = i - 1;

  if (j) {
    return t0;
  }
  else {
    return t1;
  }
}

float main(uint3 off : OFF) : SV_Target {
  return foo(1, 0, 2).Load(off).x;
}

