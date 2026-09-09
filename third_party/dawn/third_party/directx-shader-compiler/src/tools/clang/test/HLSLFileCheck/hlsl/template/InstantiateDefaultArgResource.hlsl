// RUN: %dxc -T vs_6_6 -HV 2021 %s -ast-dump | FileCheck %s -check-prefix=AST
// RUN: %dxc -T vs_6_6 -HV 2021 %s -fcgl | FileCheck %s

// AST: FunctionTemplateDecl {{.*}} foo
// AST-NEXT: TemplateTypeParmDecl {{.*}} referenced typename T
// AST-NEXT: FunctionDecl {{.*}} foo 'void (Texture2D<vector<T, 4> >)'
// AST-NEXT: ParmVarDecl {{.*}} tex 'Texture2D<vector<T, 4> >' cinit
// AST-NEXT: CXXOperatorCallExpr {{.*}} 'const .Resource'
// AST-NEXT: ImplicitCastExpr {{.*}} 'const .Resource (*)(unsigned int) const' <FunctionToPointerDecay>
// AST-NEXT: DeclRefExpr {{.*}} 'const .Resource (unsigned int) const' lvalue CXXMethod 0x{{[0-9a-fA-F]+}} 'operator[]' 'const .Resource (unsigned int) const'
// AST-NEXT: ImplicitCastExpr {{.*}} 'const .Resource' lvalue <NoOp>
// AST-NEXT: DeclRefExpr {{.*}} '.Resource' lvalue Var 0x{{[0-9a-fA-F]+}} 'ResourceDescriptorHeap' '.Resource'
// AST-NEXT: ImplicitCastExpr {{.*}} 'unsigned int' <IntegralCast>
// AST-NEXT: IntegerLiteral {{.*}} 'literal int' 0
// AST-NEXT: CompoundStmt

// AST: FunctionDecl {{.*}} used foo 'void (Texture2D<vector<float, 4> >)'
// AST-NEXT: TemplateArgument type 'float'
// AST-NEXT: ParmVarDecl {{.*}}5 tex 'Texture2D<vector<float, 4> >':'Texture2D<vector<float, 4> >' cinit
// AST-NEXT: ImplicitCastExpr {{.*}} 'Texture2D<vector<float, 4> >' <FlatConversion>
// AST-NEXT: CXXOperatorCallExpr {{.*}} 'const .Resource'
// AST-NEXT: ImplicitCastExpr {{.*}}'const .Resource (*)(unsigned int) const' <FunctionToPointerDecay>
// AST-NEXT: DeclRefExpr {{.*}} 'const .Resource (unsigned int) const' lvalue CXXMethod 0x{{[0-9a-fA-F]+}} 'operator[]' 'const .Resource (unsigned int) const'
// AST-NEXT: ImplicitCastExpr {{.*}} 'const .Resource' lvalue <NoOp>
// AST-NEXT: DeclRefExpr {{.*}} '.Resource' lvalue Var 0x{{[0-9a-fA-F]+}} 'ResourceDescriptorHeap' '.Resource'
// AST-NEXT: ImplicitCastExpr {{.*}} 'unsigned int' <IntegralCast>
// AST-NEXT: IntegerLiteral {{.*}} 'literal int' 0

template<typename T>
void foo(Texture2D<vector<T, 4> > tex = ResourceDescriptorHeap[0]) {}

void main() {
  foo<float>();
}

// CHECK: call void @"\01??$foo{{.*}}"(%"class.Texture2D<vector<float, 4> >"* {{.*}})
