// RUN: %dxc -T vs_6_0 -E main %s -ast-dump | FileCheck %s

// CHECK: FunctionDecl {{.*}} used CallMethod 'uint (__restrict MyClass)'
// CHECK: CXXMemberCallExpr
// CHECK-NEXT: MemberExpr {{.*}} .Method
// CHECK-NEXT: ImplicitCastExpr {{.*}} 'MyClass' lvalue <NoOp>
// CHECK-NEXT: DeclRefExpr
// CHECK-SAME: '__restrict MyClass' lvalue ParmVar 0x{{[0-9a-zA-Z]+}} 'c' '__restrict MyClass'

class MyClass {
  uint Method(uint2 u2) { return u2.y * 2; }
  uint Method(uint3 u3) { return u3.y * 3; }
  uint u;
};

uint CallMethod(inout MyClass c) {
  // `inout MyClass c` translates to: `__restrict MyClass&`
  // This doesn't exactly match MyClass type for `Method` call, so if method
  // is overloaded, ScoreCast is used, leading to a comparison of MyClass type,
  // which was broken for `class` types as opposed to `struct` types.
  return c.Method(uint2(1,2));
}

uint main() : OUT {
  MyClass c = (MyClass)1;
  return CallMethod(c);
}
