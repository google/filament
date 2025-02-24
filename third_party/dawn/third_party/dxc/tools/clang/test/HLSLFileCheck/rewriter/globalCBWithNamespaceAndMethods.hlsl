// RUN: %dxr -decl-global-cb  -line-directive   %s | FileCheck %s

// Make sure namespace with method works.
// CHECK:namespace N2 {
// CHECK:struct UseA {
// CHECK:float b;
// CHECK:float foo() ;
// CHECK:};
// CHECK:}
// CHECK:cbuffer GlobalCB {
// CHECK:namespace N {
// CHECK:float4 a;
// CHECK:}
// CHECK:namespace N3 {
// CHECK:N2::UseA use;
// CHECK:}
// CHECK:}


// CHECK:namespace N2 {
// CHECK:float N2::UseA::foo() {
// CHECK:return N::a.x + this.b;
// CHECK:}
// CHECK:}

namespace N {
float4 a;
}

namespace N2 {
struct UseA {
float b;
float foo() { return N::a.x + b;}
};

}

namespace N3 {
N2::UseA use;
}

float main() : SV_Target {
  return N3::use.foo();
}