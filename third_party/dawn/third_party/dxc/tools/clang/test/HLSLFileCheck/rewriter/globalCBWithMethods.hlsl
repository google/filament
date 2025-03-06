// RUN: %dxr -decl-global-cb  -line-directive   %s | FileCheck %s

// Make sure created GlobalCB and the dependent type is before GlobalCB.
// Methods define stay after globalCB.

// CHECK:struct UseA {
// CHECK:float b;
// CHECK:float foo() ;
// CHECK:};
// CHECK:cbuffer GlobalCB {
// CHECK:float4 a;
// CHECK:UseA use;
// CHECK:}

// CHECK:float UseA::foo() {

float4 a;


struct UseA {
float b;
float foo() { return a.x + b;}
};

UseA use;

float main() : SV_Target {
  return use.foo();
}