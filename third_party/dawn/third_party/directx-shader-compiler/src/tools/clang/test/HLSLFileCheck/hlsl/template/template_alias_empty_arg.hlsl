// RUN: %dxc  -T lib_6_3 %s -ast-dump -HV 2021 | FileCheck %s
// RUN: %dxc  -T lib_6_3 %s  -HV 2021 | FileCheck %s -check-prefix=IR

// CHECK: ClassTemplateDecl 0x{{.*}} test
// CHECK-NEXT: TemplateTypeParmDecl 0x{{.*}} referenced typename T
// CHECK-NEXT: -TemplateArgument type 'float'
// CHECK-NEXT: NonTypeTemplateParmDecl 0x{{.*}} 'int' i
// CHECK-NEXT: TemplateArgument expr
// CHECK-NEXT: ImplicitCastExpr 0x{{.*}} 'int' <IntegralCast>
// CHECK-NEXT: IntegerLiteral 0x{{.*}} 'literal int' 0
// CHECK-NEXT: CXXRecordDecl 0x{{.*}} struct test definition
// CHECK-NEXT: CXXRecordDecl 0x{{.*}} implicit struct test
// CHECK-NEXT: FieldDecl 0x{{.*}} d 'T'
// CHECK-NEXT: ClassTemplateSpecializationDecl 0x[[test_CTSD0:[0-9a-f]+]] {{.*}} struct test definition
// CHECK-NEXT: TemplateArgument type 'float'
// CHECK-NEXT: TemplateArgument integral 1
// CHECK-NEXT: CXXRecordDecl 0x{{[0-9a-f]+}} prev 0x[[test_CTSD0]] {{.*}} implicit struct test
// CHECK-NEXT: FieldDecl 0x[[field_d:[0-9a-f]+]] {{.*}} referenced d 'float':'float'

template<typename T=float, int i=0>
struct test {
  T d;
};

// CHECK: TypeAliasTemplateDecl 0x{{.*}} test2
// CHECK-NEXT: TemplateTypeParmDecl 0x{{.*}} referenced typename T
// CHECK-NEXT: TemplateArgument type 'float'
// CHECK-NEXT: TypeAliasDecl 0x{{.*}} test2 'test<T, 1>'
template<typename T=float>
using test2 = test<T, 1>;

// CHECK: VarDecl 0x[[t22:[0-9a-f]+]] {{.*}} used t22 'test2':'test<float, 1>'
test2 t22;

// CHECK: ClassTemplateDecl 0x{{.*}} test3
// CHECK-NEXT: TemplateTypeParmDecl 0x{{.*}} referenced typename T
// CHECK-NEXT: CXXRecordDecl 0x{{.*}} struct test3 definition
// CHECK-NEXT: CXXRecordDecl 0x{{.*}} implicit struct test3
// CHECK-NEXT: FieldDecl 0x{{.*}} d 'T'
// CHECK-NEXT: ClassTemplateSpecializationDecl 0x[[test3_CTSD0:[0-9a-f]+]] {{.*}} struct test3 definition
// CHECK-NEXT: TemplateArgument type 'test<float, 1>'
// CHECK-NEXT: CXXRecordDecl 0x{{.*}} prev 0x[[test3_CTSD0]] {{.*}} implicit struct test3
// CHECK-NEXT: FieldDecl 0x[[test3_d:[0-9a-f]+]] {{.*}} referenced d 'test<float, 1>':'test<float, 1>'

template<typename T>
struct test3 {
  T d;
};

// CHECK: VarDecl 0x[[tt:[0-9a-f]+]] {{.*}} used tt 'test3<test2>':'test3<test<float, 1> >'
test3<test2>  tt;

// CHECK: FunctionDecl 0x{{.*}} foo 'float ()'
// CHECK-NEXT: CompoundStmt 0x{{.*}}
// CHECK-NEXT: ReturnStmt 0x{{.*}}
// CHECK-NEXT: BinaryOperator 0x{{.*}} 'float':'float' '+'
// CHECK-NEXT: ImplicitCastExpr 0x{{.*}} 'float':'float' <LValueToRValue>
// CHECK-NEXT: MemberExpr 0x{{.*}} 'float':'float' lvalue .d 0x[[field_d]]
// CHECK-NEXT: MemberExpr 0x{{.*}} <col:10, col:13> 'test<float, 1>':'test<float, 1>' lvalue .d 0x[[test3_d]]
// CHECK-NEXT: DeclRefExpr 0x{{.*}} <col:10> 'test3<test2>':'test3<test<float, 1> >' lvalue Var 0x[[tt]] 'tt' 'test3<test2>':'test3<test<float, 1> >'
// CHECK-NEXT: ImplicitCastExpr 0x{{.*}} 'float':'float' <LValueToRValue>
// CHECK-NEXT: MemberExpr 0x{{.*}} 'float':'float' lvalue .d 0x[[field_d]]
// CHECK-NEXT: DeclRefExpr 0x{{.*}} 'test2':'test<float, 1>' lvalue Var 0x[[t22]] 't22' 'test2':'test<float, 1>'
// CHECK-NEXT: HLSLExportAttr 0x{{.*}}
export
float foo() {
  return tt.d.d + t22.d; 
}

// Make sure generate cb load from index 1 and 0 then add.
// IR:%[[CB1:[0-9]+]] = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %{{.+}}, i32 1)  ; CBufferLoadLegacy(handle,regIndex)
// IR:%[[VAL1:[0-9]+]] = extractvalue %dx.types.CBufRet.f32 %[[CB1]], 0
// IR:%[[CB0:[0-9]+]] = call %dx.types.CBufRet.f32 @dx.op.cbufferLoadLegacy.f32(i32 59, %dx.types.Handle %{{.+}}, i32 0)  ; CBufferLoadLegacy(handle,regIndex)
// IR:%[[VAL0:[0-9]+]] = extractvalue %dx.types.CBufRet.f32 %[[CB0]], 0
// IR:%[[VAL:.+]] = fadd fast float %[[VAL0]], %[[VAL1]]
// IR:ret float %[[VAL]]
