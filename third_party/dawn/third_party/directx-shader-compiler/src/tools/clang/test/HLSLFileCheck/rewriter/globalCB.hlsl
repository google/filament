// RUN: %dxr -decl-global-cb  -line-directive   %s | FileCheck %s

// Make sure created GlobalCB and the dependent type is before GlobalCB.
// CHECK:struct S0
// CHECK:namespace NN
// CHECK:namespace N
// CHECK:struct C
// CHECK:struct S1
// CHECK:NN::N::C
// CHECK:cbuffer GlobalCB
// CHECK:float4 a;
// CHECK:S1 s;
// CHECK:float foo(S0 s0)
// CHECK:float main()

#include "inc/globalCB.h"


struct S0 {
  float4 b;
};

float foo(S0 s0) {
  return s0.b.y;
}

struct S1 : S0 {
  float4 c;
  float getX() { return c.x; }
  NN::N::C c0;
};

S1 s;

//C c;

float main() : SV_Target {
  return s.getX();
}