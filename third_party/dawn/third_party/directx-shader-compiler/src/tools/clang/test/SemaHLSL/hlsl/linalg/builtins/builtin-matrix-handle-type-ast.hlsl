// REQUIRES: dxil-1-10
// RUN: %dxc -T lib_6_10 -ast-dump %s | FileCheck %s

// CHECK: CXXRecordDecl {{.*}} struct S definition
// CHECK: FieldDecl {{.*}} handle '__builtin_LinAlgMatrix':'__builtin_LinAlgMatrix'
struct S {
  __builtin_LinAlgMatrix handle;
};

// CHECK: VarDecl {{.*}} global_handle '__builtin_LinAlgMatrix':'__builtin_LinAlgMatrix'
__builtin_LinAlgMatrix global_handle;

// CHECK: FunctionDecl {{.*}} f1 'void (__builtin_LinAlgMatrix)'
// CHECK: ParmVarDecl {{.*}} m '__builtin_LinAlgMatrix':'__builtin_LinAlgMatrix'
void f1(__builtin_LinAlgMatrix m);

// CHECK: FunctionDecl {{.*}} f2 '__builtin_LinAlgMatrix ()'
__builtin_LinAlgMatrix f2();

// CHECK: FunctionDecl {{.*}} main 'void ()'
// CHECK-NEXT: CompoundStmt
// CHECK-NEXT: DeclStmt
// CHECK-NEXT: VarDecl {{.*}} m '__builtin_LinAlgMatrix':'__builtin_LinAlgMatrix'
[shader("compute")]
[numthreads(4,1,1)]
void main() {
  __builtin_LinAlgMatrix m;
}
