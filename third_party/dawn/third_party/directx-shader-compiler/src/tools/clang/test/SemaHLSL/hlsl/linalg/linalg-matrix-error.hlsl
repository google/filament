// REQUIRES: dxil-1-10
// RUN: %dxc -T lib_6_10 -verify %s

#include <dx/linalg.h>
using namespace dx::linalg;

// CHECK:: foo
void f() {
    // expected-error@+1{{too few template arguments for class template 'Matrix'}}
    Matrix<ComponentType::I32, 4, 5> mat1;

    // expected-error@+1{{non-type template argument of type 'literal string' must have an integral or enumeration type}}
    Matrix<ComponentType::F32, 4, 5, "B", MatrixScope::ThreadGroup> mat2;

    Matrix<ComponentType::I32, 4, 5, MatrixUse::A, MatrixScope::ThreadGroup> mat3;

    Matrix<ComponentType::F32, 4, 5, MatrixUse::A, MatrixScope::Wave> mat4;

    // expected-error@+1{{cannot convert from 'Matrix<ComponentType::F32, 4, 5, MatrixUse::A, MatrixScope::Wave>' to 'Matrix<ComponentType::I32, 4, 5, MatrixUse::A, MatrixScope::ThreadGroup>'}}
    mat3 = mat4;

    __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(ComponentType::I32, 4, 5, MatrixUse::B, MatrixScope::ThreadGroup)]] mat8;

    // expected-error@+1 {{cannot initialize a variable of type '__builtin_LinAlgMatrix' with an lvalue of type '__builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(ComponentType::I32, 4, 5, MatrixUse::B, MatrixScope::ThreadGroup)]]'}}
    __builtin_LinAlgMatrix naked_mat = mat8;

    // ok
    Matrix<ComponentType::F32, 4, 5, MatrixUse::A, MatrixScope::Wave> same_as_mat4 = mat4;

    // expected-error@+1{{cannot initialize a variable of type 'Matrix<ComponentType::I32 aka 4, 4, 5, MatrixUse::B aka 1, MatrixScope::ThreadGroup aka 2>' with an lvalue of type 'Matrix<ComponentType::F32 aka 9, 4, 5, MatrixUse::A aka 0, MatrixScope::Wave aka 1>'}}
    Matrix<ComponentType::I32, 4, 5, MatrixUse::B, MatrixScope::ThreadGroup> different_mat = mat4;
}

// expected-note@dx/linalg.h:*{{template is declared here}}
// expected-note@dx/linalg.h:*{{template parameter is declared here}}
