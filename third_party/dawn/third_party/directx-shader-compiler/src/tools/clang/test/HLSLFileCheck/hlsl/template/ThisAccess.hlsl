// RUN: %dxc -T cs_6_6 -E main -HV 2021 -ast-dump %s | FileCheck %s


template<typename K, typename V>
struct pair {
  K First;
  V Second;

  K first() {
    return this.First;
  }

  V second() {
    return Second;
  }
};

[numthreads(1,1,1)]
void main() {
  pair<int, float> Vals = {1, 2.0};
  Vals.First = Vals.first();
  Vals.Second = Vals.second();
}

// CHECK:      CXXMethodDecl 0x{{[0-9a-zA-Z]+}} <line:9:3, line:11:3> line:9:5 first 'K ()'
// CHECK-NEXT: `-CompoundStmt 0x{{[0-9a-zA-Z]+}} <col:13, line:11:3>
// CHECK-NEXT: `-ReturnStmt 0x{{[0-9a-zA-Z]+}} <line:10:5, col:17>
// CHECK-NEXT: `-CXXDependentScopeMemberExpr 0x{{[0-9a-zA-Z]+}} <col:12, col:17> '<dependent type>' lvalue
// CHECK-NEXT: `-CXXThisExpr 0x{{[0-9a-zA-Z]+}} <col:12> 'pair<K, V>' lvalue this
// CHECK-NEXT: CXXMethodDecl 0x{{[0-9a-zA-Z]+}} <line:13:3, line:15:3> line:13:5 second 'V ()'
// CHECK-NEXT: `-CompoundStmt 0x{{[0-9a-zA-Z]+}} <col:14, line:15:3>
// CHECK-NEXT: `-ReturnStmt 0x{{[0-9a-zA-Z]+}} <line:14:5, col:12>
// CHECK-NEXT: `-MemberExpr 0x{{[0-9a-zA-Z]+}} <col:12> 'V' lvalue .Second 0x{{[0-9a-zA-Z]+}}
// CHECK-NEXT: `-CXXThisExpr 0x{{[0-9a-zA-Z]+}} <col:12> 'pair<K, V>' lvalue this


// CHECK:      CXXMethodDecl 0x{{[0-9a-zA-Z]+}} <line:9:3, line:11:3> line:9:5 used first 'int ()'
// CHECK-NEXT: `-CompoundStmt 0x{{[0-9a-zA-Z]+}} <col:13, line:11:3>
// CHECK-NEXT: `-ReturnStmt 0x{{[0-9a-zA-Z]+}} <line:10:5, col:17>
// CHECK-NEXT: `-ImplicitCastExpr 0x{{[0-9a-zA-Z]+}} <col:12, col:17> 'int':'int' <LValueToRValue>
// CHECK-NEXT: `-MemberExpr 0x{{[0-9a-zA-Z]+}} <col:12, col:17> 'int':'int' lvalue .First 0x{{[0-9a-zA-Z]+}}
// CHECK-NEXT: `-CXXThisExpr 0x{{[0-9a-zA-Z]+}} <col:12> 'pair<int, float>' lvalue this
// CHECK-NEXT: `-CXXMethodDecl 0x{{[0-9a-zA-Z]+}} <line:13:3, line:15:3> line:13:5 used second 'float ()'
// CHECK-NEXT: `-CompoundStmt 0x{{[0-9a-zA-Z]+}} <col:14, line:15:3>
// CHECK-NEXT: `-ReturnStmt 0x{{[0-9a-zA-Z]+}} <line:14:5, col:12>
// CHECK-NEXT: `-ImplicitCastExpr 0x{{[0-9a-zA-Z]+}} <col:12> 'float':'float' <LValueToRValue>
// CHECK-NEXT: `-MemberExpr 0x{{[0-9a-zA-Z]+}} <col:12> 'float':'float' lvalue .Second 0x{{[0-9a-zA-Z]+}}
// CHECK-NEXT: `-CXXThisExpr 0x{{[0-9a-zA-Z]+}} <col:12> 'pair<int, float>' lvalue this
