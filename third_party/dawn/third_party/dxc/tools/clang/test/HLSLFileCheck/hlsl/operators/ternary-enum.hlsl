// RUN: %dxc -ast-dump -T vs_6_0 %s | FileCheck %s
// RUN: %dxc -ast-dump -T vs_6_0 -HV 2021 %s | FileCheck %s
// RUN: %dxc -ast-dump -T vs_6_0 -DCLASS %s | FileCheck %s
// RUN: %dxc -ast-dump -T vs_6_0 -DCLASS -HV 2021 %s | FileCheck %s

// CHECK: VarDecl
// CHECK-SAME: referenced g_c 'const MyEnum' static cinit
// CHECK-NEXT: ConditionalOperator
// CHECK-SAME: 'MyEnum'
// CHECK-NEXT: CXXBoolLiteralExpr
// CHECK-SAME: 'bool' true
// CHECK-NEXT: DeclRefExpr
// CHECK-SAME: 'MyEnum' EnumConstant
// CHECK-SAME: 'One' 'MyEnum'
// CHECK-NEXT: DeclRefExpr
// CHECK-SAME: 'MyEnum' EnumConstant
// CHECK-SAME: 'Two' 'MyEnum'

enum
#ifdef CLASS
  class
#endif
MyEnum {
    One,
    Two,
};

#ifdef CLASS
static const MyEnum g_c = true ? MyEnum::One : MyEnum::Two;
#else
static const MyEnum g_c = true ? One : Two;
#endif

int main() : OUT {
  return (int)g_c;
}
