// RUN: %dxc -T lib_6_4 -HV 2021 %s -ast-dump | FileCheck %s -check-prefix=AST
// RUN: %dxc -T lib_6_4 -HV 2021 %s -fcgl | FileCheck %s

// This test verifies two things. First it verifies that the AST instantiates a
// correct AST where the `CXXThisExpr` is an lvalue of type array_ext<float, 3>
// rather than a pointer (as C++ would have).

// Secondarily it verifies that the code geneariton for the `this` reference
// correctly resolves to the base pointer and indexes off the base class member.

// AST: ClassTemplateDecl {{.*}} array_ext
// AST-NEXT: TemplateTypeParmDecl {{.*}} referenced typename T
// AST-NEXT: NonTypeTemplateParmDecl {{.*}} referenced 'uint32_t':'unsigned int' N
// AST-NEXT: CXXRecordDecl {{.*}} class array_ext definition
// AST-NEXT: public 'array<T, N>'

// AST: ClassTemplateSpecializationDecl {{.*}} class array_ext definition
// AST: TemplateArgument type 'float'
// AST-NEXT: TemplateArgument integral 3
// AST-NEXT: CXXRecordDecl {{.*}} implicit class array_ext
// AST-NEXT: CXXMethodDecl {{.*}} used test 'float ()'
// AST-NEXT: CompoundStmt
// AST-NEXT: ReturnStmt
// AST-NEXT: ImplicitCastExpr {{.*}} 'float':'float' <LValueToRValue>
// AST-NEXT: ArraySubscriptExpr {{.*}} 'float':'float' lvalue

// Note: the implicit LValueToRvalue casts below are nonsensical as noted by them
// producing lvalues. This test verifies them only to ensure the correct ASTs
// around the casts. The casts themselves might be removed or changed in a
// future change.

// AST-NEXT: ImplicitCastExpr {{.*}} 'float [3]' <LValueToRValue>
// AST-NEXT: MemberExpr {{.*}} 'float [3]' lvalue .mArr
// AST-NEXT: ImplicitCastExpr {{.*}} 'array<float, 3U>':'array<float, 3>' lvalue <UncheckedDerivedToBase (array)>
// AST-NEXT: CXXThisExpr {{.* }}'array_ext<float, 3>' lvalue this
// AST-NEXT: IntegerLiteral {{.*}} 'literal int' 0

template <typename T, uint32_t N> class array { T mArr[N]; };

template <typename T, uint32_t N> class array_ext : array<T, N> {
  float test() { return array<T, N>::mArr[0]; }
};

// CHECK: define linkonce_odr float @"\01?test@?$array_ext@{{.*}}"(%"class.array_ext<float, 3>"* [[this:%.+]])
// CHECK: [[basePtr:%[0-9]+]] = bitcast %"class.array_ext<float, 3>"* [[this]] to %"class.array<float, 3>"*
// CHECK: [[mArr:%.+]] = getelementptr inbounds %"class.array<float, 3>", %"class.array<float, 3>"* [[basePtr]], i32 0, i32 0
// CHECK: [[elemPtr:%.+]] = getelementptr inbounds [3 x float], [3 x float]* [[mArr]], i32 0, i32 0
// CHECK: [[Val:%.+]] = load float, float* [[elemPtr]]
// CHECK: ret float [[Val]]

// This function only exists to force instantiation of the template.
float fn() {
  array_ext<float, 3> arr1;
  return arr1.test();
}
