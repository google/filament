// RUN: %dxc -E main -T ps_6_0 %s | FileCheck %s

// For cb.b
// CHECK: cbufferLoadLegacy.f32
// CHECK: i32 1)
// CHECK: extractvalue
// CHECK: , 0
// CHECK: extractvalue
// CHECK: , 1
// CHECK: extractvalue
// CHECK: , 2
// CHECK: extractvalue
// CHECK: , 3

// For cb2.a
// CHECK: cbufferLoadLegacy.f32
// CHECK: i32 2)
// CHECK: extractvalue
// CHECK: , 0
// CHECK: extractvalue
// CHECK: , 1
// CHECK: extractvalue
// CHECK: , 2
// CHECK: extractvalue
// CHECK: , 3

struct A {
  float4 a;
  void Init(A _A) {
    a = _A.a;
  }
};

struct B : A {
  float4 b;
};

struct B cb;
struct B cb2;

float4 main() :SV_TARGET {
  B b = cb;
  b.Init((A)cb2);

  return b.a + b.b;
}

