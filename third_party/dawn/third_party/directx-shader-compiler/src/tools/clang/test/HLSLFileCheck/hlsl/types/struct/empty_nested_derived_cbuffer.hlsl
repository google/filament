// RUN: %dxc -E main -T vs_6_0 %s | FileCheck %s

// Make sure nested empty struct works.  Also test related paths such as
// derived, multi-dim array in constant buffer, and argument passing.
// CHECK: main

struct KillerStruct {};

struct InnerStruct {
  KillerStruct s;
};

struct OuterStruct {
  InnerStruct s;
};

class Derived : OuterStruct {
  InnerStruct s2;
};

cbuffer Params_cbuffer : register(b0) {
  Derived constants[2][3];
};

float4 foo(Derived s) { return (float4)0; }

float4 main() : SV_POSITION {
  return foo(constants[1][2]);
}
