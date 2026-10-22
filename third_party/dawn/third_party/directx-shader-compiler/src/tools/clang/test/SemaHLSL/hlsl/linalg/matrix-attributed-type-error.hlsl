// REQUIRES: dxil-1-10
// RUN: %dxc -T lib_6_10 -enable-16bit-types -verify %s

#include <dx/linalg.h>
using namespace dx::linalg;

void f() {
  // expected-error@+1 {{matrix attributes can only be applied to __builtin_LinAlgMatrix}}
  int [[__LinAlgMatrix_Attributes(ComponentType::I32, 4, 5, MatrixUse::B, MatrixScope::ThreadGroup)]] mat1;

  // expected-error@+1 {{'__LinAlgMatrix_Attributes' attribute requires exactly 5 arguments}}
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(10)]] mat2;
  // expected-error@+1 {{'__LinAlgMatrix_Attributes' attribute requires exactly 5 arguments}}
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(1, 2, 3, 4, 5, 6)]] mat3;

  // ok
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(10, 4, 5, 2, 1)]] mat4;

  // expected-error@+1 {{argument is not an integer or enumeration}}
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(10.56, 4, 5, 2, 1)]] mat5;

  // expected-error@+1 {{argument is not an integer}}
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(ComponentType::I32, "str", 5, 2, 1)]] mat6;
  // expected-error@+3 {{cannot convert from '__builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(ComponentType::I32, 4, 5, MatrixUse::B, MatrixScope::ThreadGroup)]]' to '__builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(ComponentType::I32, 4, 5, MatrixUse::A, MatrixScope::ThreadGroup)]]'}}
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(ComponentType::I32, 4, 5, MatrixUse::A, MatrixScope::ThreadGroup)]] mat7;
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(ComponentType::I32, 4, 5, MatrixUse::B, MatrixScope::ThreadGroup)]] mat8;
  mat7 = mat8;

 // expected-error@+1 {{cannot initialize a variable of type '__builtin_LinAlgMatrix' with an lvalue of type '__builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(ComponentType::I32, 4, 5, MatrixUse::B, MatrixScope::ThreadGroup)]]'}}
  __builtin_LinAlgMatrix naked_mat = mat8;
}
