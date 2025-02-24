// RUN: %dxc -T lib_6_3 -auto-binding-space 11 -default-linkage external %s | FileCheck %s

// Make sure static const field works.

// CHECK: float 6.000000e+00

struct X {

static const uint A = 6 ;

};


float test() {
  return X::A ;
}