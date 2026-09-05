// RUN: %dxc %s -T ps_6_0 -Od | FileCheck %s

// Regression test for selecting on bad resources.

// CHECK: @main

Texture2D t0 : register(t0);

struct Foo {
  Texture2D a, b;
};

float4 bar(uint x, int3 off, Foo foo) {
  return x ? foo.a.Load(off) : foo.b.Load(off);
}

float4 main(int3 off : OFF) : SV_Target {
  Foo foo;
  foo.a = t0;
  return bar(1, off, foo);
}

