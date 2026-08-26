// REQUIRES: dxil-1-10
// RUN: %dxc -T lib_6_10 -ast-dump %s FileCheck %s

#include <dx/linalg.h>
using namespace dx::linalg;

void f() {
  Matrix<ComponentType::I32, 4, 5, MatrixUse::B, MatrixScope::ThreadGroup> mat1;
  Matrix<ComponentType::F32, 100, 100, MatrixUse::A, MatrixScope::Wave> mat2;
}

// CHECK: ClassTemplateDecl {{.*}} Matrix{{$}}
// CHECK-NEXT: NonTypeTemplateParmDecl {{.*}} 'ComponentEnum':'dx::linalg::ComponentType::ComponentEnum' ComponentTy
// CHECK-NEXT: NonTypeTemplateParmDecl {{.*}} 'int' M
// CHECK-NEXT: NonTypeTemplateParmDecl {{.*}} 'int' N
// CHECK-NEXT: NonTypeTemplateParmDecl {{.*}} 'MatrixUseEnum':'dx::linalg::MatrixUse::MatrixUseEnum' Use
// CHECK-NEXT: NonTypeTemplateParmDecl {{.*}} 'MatrixScopeEnum':'dx::linalg::MatrixScope::MatrixScopeEnum' Scope
// CHECK-NEXT: CXXRecordDecl {{.*}} class Matrix definition
// CHECK-NEXT: CXXRecordDecl {{.*}} Matrix
// CHECK-NEXT:TypeAliasDecl {{.*}} HandleT '__builtin_LinAlgMatrix
// CHECK-SAME{LITERAL}: [[__LinAlgMatrix_Attributes(ComponentTy, M, N, Use, Scope)]]'
// CHECK-NEXT: FieldDecl {{.*}} __handle 'HandleT':'__builtin_LinAlgMatrix
// CHECK-SAME{LITERAL}: [[__LinAlgMatrix_Attributes(ComponentTy, M, N, Use, Scope)]]'

// CHECK-NEXT: ClassTemplateSpecializationDecl {{.*}} class Matrix definition
// CHECK-NEXT: TemplateArgument integral 4
// CHECK-NEXT: TemplateArgument integral 4
// CHECK-NEXT: TemplateArgument integral 5
// CHECK-NEXT: TemplateArgument integral 1
// CHECK-NEXT: TemplateArgument integral 2
// CHECK-NEXT: CXXRecordDecl {{.*}} implicit class Matrix
// CHECK-NEXT: TypeAliasDecl {{.*}} HandleT '__builtin_LinAlgMatrix
// CHECK-SAME{LITERAL}: [[__LinAlgMatrix_Attributes(ComponentType::I32, 4, 5, MatrixUse::B, MatrixScope::ThreadGroup)]]'
// CHECK-NEXT: FieldDecl {{.*}} __handle 'HandleT':'__builtin_LinAlgMatrix
// CHECK-SAME{LITERAL}: [[__LinAlgMatrix_Attributes(ComponentType::I32, 4, 5, MatrixUse::B, MatrixScope::ThreadGroup)]]'

// CHECK-NEXT: ClassTemplateSpecializationDecl {{.*}} class Matrix definition
// CHECK-NEXT: TemplateArgument integral 17
// CHECK-NEXT: TemplateArgument integral 100
// CHECK-NEXT: TemplateArgument integral 100
// CHECK-NEXT: TemplateArgument integral 0
// CHECK-NEXT: TemplateArgument integral 1
// CHECK-NEXT: CXXRecordDecl {{.*}} implicit class Matrix
// CHECK-NEXT: TypeAliasDecl {{.*}} HandleT '__builtin_LinAlgMatrix
// CHECK-SAME{LITERAL}: [[__LinAlgMatrix_Attributes(ComponentType::F32,100, 100, MatrixUse::A, MatrixScope::Wave)]]'
// CHECK-NEXT: FieldDecl {{.*}} __handle 'HandleT':'__builtin_LinAlgMatrix
// CHECK-SAME{LITERAL}: [[__LinAlgMatrix_Attributes(ComponentType::F32, 100, 100, MatrixUse::A, MatrixScope::Wave)]]'

// CHECK: FunctionDecl {{.*}} f 'void ()'

// CHECK: VarDecl {{.*}} mat1 'Matrix<ComponentType::I32, 4, 5, MatrixUse::B, MatrixScope::ThreadGroup>':
// CHECK-SAME: 'dx::linalg::Matrix<dx::linalg::ComponentType::ComponentEnum::I32, 4, 5, dx::linalg::MatrixUse::MatrixUseEnum::B,
// CHECK-SAME: dx::linalg::MatrixScope::MatrixScopeEnum::ThreadGroup>'

// CHECK: VarDecl {{.*}} mat2 'Matrix<ComponentType::F32, 100, 100, MatrixUse::A, MatrixScope::Wave>':
// CHECK-SAME: 'dx::linalg::Matrix<dx::linalg::ComponentType::ComponentEnum::F32, 100, 100, 
// CHECK-SAME: dx::linalg::MatrixUse::MatrixUseEnum::A, dx::linalg::MatrixScope::MatrixScopeEnum::Wave>'
