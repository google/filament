// RUN: %dxc %s -T cs_6_0 -Od | FileCheck %s

// Regression test for validation failure in O0 due to
// storing structure with uninitialized member.

// CHECK: @main

Texture2D t0 : register(t0);
Texture2D t1 : register(t1);

struct Foo {
  float a,b,c,d,e,f,g,h,i;
};

groupshared Foo foos[4];

Foo make_foo(float x, float y, float z) {
  Foo foo;
  foo.a = x;
  foo.b = y;
  // foo.c is missing
  foo.d = x;
  foo.e = y;
  foo.f = z;
  foo.g = x;
  foo.h = y;
  foo.i = z;
  return foo;
}

void foo(float x, float y, float z) {
 [unroll]
 for( int i = (4) - 1; i >= 0; --i ) {
   foos[i] = make_foo( x, y, z );
  }
}

float bar(Foo f) {
  return f.e;
}

RWStructuredBuffer<float> output;

[numthreads(1,1,1)]
void main() {
  foo(1, 2, 0);
  output[0] = bar(foos[3]);
}
