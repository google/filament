// REQUIRES: dxil-1-10
// RUN: %dxc -T lib_6_10 -enable-16bit-types -ast-dump %s FileCheck %s

#include <dx/linalg.h>
using namespace dx::linalg;

// CHECK: FunctionDecl {{.*}} f1 'void ()'
// CHECK: VarDecl {{.*}} mat1 '__builtin_LinAlgMatrix
// CHECK-SAME{LITERAL}: [[__LinAlgMatrix_Attributes(ComponentType::I32, 4, 5, MatrixUse::B, MatrixScope::ThreadGroup)]]'

void f1() {
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(ComponentType::I32, 4, 5, MatrixUse::B, MatrixScope::ThreadGroup)]] mat1;
}

// CHECK: FunctionDecl {{.*}} f2 'void (__builtin_LinAlgMatrix
// CHECK-SAME{LITERAL}: [[__LinAlgMatrix_Attributes(ComponentType::F32, 10, 20, MatrixUse::A, MatrixScope::Wave)]])'
// CHECK-NEXT: ParmVarDecl {{.*}} mat2 '__builtin_LinAlgMatrix
// CHECK-SAME{LITERAL}: [[__LinAlgMatrix_Attributes(ComponentType::F32, 10, 20, MatrixUse::A, MatrixScope::Wave)]]'

void f2(__builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(ComponentType::F32, 10, 20, MatrixUse::A, MatrixScope::Wave)]] mat2) {
}

// CHECK: TypedefDecl {{.*}} Mat10by20 '__builtin_LinAlgMatrix
// CHECK-SAME{LITERAL}: [[__LinAlgMatrix_Attributes(ComponentType::F32, 10, 20, MatrixUse::A, MatrixScope::Wave)]]'
typedef __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(ComponentType::F32, 10, 20, MatrixUse::A, MatrixScope::Wave)]] Mat10by20;

// CHECK: FunctionDecl {{.*}} f3 'Mat10by20 ()'
// CHECK: VarDecl {{.*}} mat3 'Mat10by20':'__builtin_LinAlgMatrix
// CHECK-SAME{LITERAL}: [[__LinAlgMatrix_Attributes(ComponentType::F32, 10, 20, MatrixUse::A, MatrixScope::Wave)]]'
// CHECK: ReturnStmt
// CHECK-NEXT: ImplicitCastExpr {{.*}} 'Mat10by20':'__builtin_LinAlgMatrix
// CHECK-SAME{LITERAL}: [[__LinAlgMatrix_Attributes(ComponentType::F32, 10, 20, MatrixUse::A, MatrixScope::Wave)]]' <LValueToRValue>
// CHECK-NEXT: DeclRefExpr {{.*}} 'Mat10by20':'__builtin_LinAlgMatrix
// CHECK-SAME{LITERAL}: [[__LinAlgMatrix_Attributes(ComponentType::F32, 10, 20, MatrixUse::A, MatrixScope::Wave)]]' lvalue Var {{.*}} 'mat3' 'Mat10by20':'__builtin_LinAlgMatrix
// CHECK-SAME{LITERAL}: [[__LinAlgMatrix_Attributes(ComponentType::F32, 10, 20, MatrixUse::A, MatrixScope::Wave)]]'

Mat10by20 f3() {
  Mat10by20 mat3;
  return mat3;
}

// CHECK: FunctionTemplateDecl {{.*}} fTemplate
// CHECK-NEXT: NonTypeTemplateParmDecl {{.*}} 'uint':'unsigned int' M
// CHECK-NEXT: NonTypeTemplateParmDecl {{.*}} 'uint':'unsigned int' N
// CHECK-NEXT: FunctionDecl {{.*}} fTemplate 'void ()'
// CHECK: VarDecl {{.*}} mat4 '__builtin_LinAlgMatrix
// CHECK-SAME{LITERAL}: [[__LinAlgMatrix_Attributes(ComponentType::I16, M, N, MatrixUse::Accumulator, MatrixScope::ThreadGroup)]]'
template <uint M, uint N>
void fTemplate() {
  __builtin_LinAlgMatrix [[__LinAlgMatrix_Attributes(ComponentType::I16, M, N, MatrixUse::Accumulator, MatrixScope::ThreadGroup)]] mat4;
}

// CHECK: FunctionDecl {{.*}} fTemplate 'void ()'
// CHECK-NEXT: TemplateArgument integral 3
// CHECK-NEXT: TemplateArgument integral 4
// CHECK: VarDecl {{.*}} mat4 '__builtin_LinAlgMatrix
// CHECK-SAME{LITERAL}: [[__LinAlgMatrix_Attributes(ComponentType::I16, 3, 4, MatrixUse::Accumulator, MatrixScope::ThreadGroup)]]'

// CHECK: FunctionDecl {{.*}} f4 'void ()'
// CHECK: CallExpr {{.*}}
// CHECK: ImplicitCastExpr {{.*}} 'void (*)()' <FunctionToPointerDecay>
// CHECK: DeclRefExpr {{.*}} 'void ()' lvalue Function {{.*}} 'fTemplate' 'void ()' (FunctionTemplate {{.*}} 'fTemplate')
void f4() {
  fTemplate<3, 4>();
}
