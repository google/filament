// RUN: %clang_cc1 -ast-dump %s 2>&1 | FileCheck %s

// CHECK: example0
void example0() {
  double d = 2.0;
  // CHECK: VarDecl{{.*}}rd 'double &'
  // CHECK-NEXT: DeclRefExpr
  double &rd = d;
  // CHECK: VarDecl{{.*}}rcd 'const double &'
  // CHECK-NEXT: ImplicitCastExpr{{.*}}'const double' lvalue <NoOp>
  const double &rcd = d;
}

struct A { };
struct B : A { } b;

// CHECK: example1
void example1() {
  // CHECK: VarDecl{{.*}}ra 'struct A &'
  // CHECK: ImplicitCastExpr{{.*}}'struct A' lvalue <DerivedToBase (A)>
  A &ra = b;
  // CHECK: VarDecl{{.*}}rca 'const struct A &'
  // CHECK: ImplicitCastExpr{{.*}}'const struct A' lvalue <NoOp>
  // CHECK: ImplicitCastExpr{{.*}}'struct A' lvalue <DerivedToBase (A)>
  const A& rca = b;
}

extern B f();

struct X {
  operator B();
} x;

// CHECK: example2
void example2() {
  // CHECK: VarDecl{{.*}}rca 'const struct A &'
  // CHECK: ImplicitCastExpr{{.*}}'const struct A' <NoOp>
  // CHECK: ImplicitCastExpr{{.*}}'struct A' <DerivedToBase (A)>
  // CHECK: CallExpr{{.*}}B
  const A &rca = f(); 
  // CHECK: VarDecl{{.*}}r 'const struct A &'
  // CHECK: ImplicitCastExpr{{.*}}'const struct A' <NoOp>
  // CHECK: ImplicitCastExpr{{.*}}'struct A' <DerivedToBase (A)>
  // CHECK: CXXMemberCallExpr{{.*}}'struct B'
  const A& r = x;
}

// CHECK: example3
void example3() {
  // CHECK: VarDecl{{.*}}rcd2 'const double &'
  // CHECK: ImplicitCastExpr{{.*}} <IntegralToFloating>
  const double& rcd2 = 2; 
}
