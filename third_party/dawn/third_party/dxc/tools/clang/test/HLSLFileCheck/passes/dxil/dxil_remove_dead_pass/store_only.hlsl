// RUN: %dxc %s -T vs_6_0 -Od | FileCheck %s

// CHECK: @main

// Regression test to make sure resources used by stores to unused globals
// are removed.

struct Foo {
  float4 member[8];
};

struct Bar {
  Texture2D t0;
};

struct Baz {
    Foo foo;
    Bar bar;
};

Texture2D t0 : register(t0 , space6);
cbuffer Baz_cbuffer : register(b0 , space6 ) {
  Foo cb_foo;
};

Baz CreateBaz() {
  Baz i;
  i.foo = cb_foo;
  i.bar.t0 = t0;
  return i;
}

static const Baz g_Baz = CreateBaz();

[RootSignature("")]
float4 main() : SV_Position {
  return float4(0,0,0,0);
}


