// RUN: %dxr -E main -remove-unused-globals %s | FileCheck %s

// Makre sure nested struct is not removed.

// CHECK:struct A
// CHECK:struct B
// CHECK-NOT:Get(
// CHECK:StructuredBuffer<C> buf : register(t0, space6)

struct A {
  float a;
};

struct B : A {
  float b;
};

struct C {
  B b;
  float c;
  float Get() { return c + b.b + b.a; }
};

StructuredBuffer<C> buf : register(t0, space6);

float main(uint i:I) : SV_Target {
  return buf[i].c;
}

