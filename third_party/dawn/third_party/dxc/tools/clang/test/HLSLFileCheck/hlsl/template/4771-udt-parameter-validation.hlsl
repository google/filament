// RUN: %dxc -T lib_6_3 -HV 2021 -ast-dump %s | FileCheck %s
template<typename T>
struct Leg {
    static T zero() {
        return (T)0;
    }
};

template<class Animal>
typename Animal::LegType getLegs(Animal A) {
  return A.Legs + Leg<typename Animal::LegType>::zero();
}

// CHECK:      FunctionTemplateDecl {{0x[0-9a-fA-F]+}} <line:9:1, line:12:1> line:10:26 getLegs
// CHECK:      CXXDependentScopeMemberExpr {{0x[0-9a-fA-F]+}} <col:10, col:12> '<dependent type>' lvalue
// CHECK: DeclRefExpr {{0x[0-9a-fA-F]+}} <col:10> 'Animal' lvalue ParmVar {{0x[0-9a-fA-F]+}} 'A' 'Animal'
// CHECK-NEXT: CallExpr {{0x[0-9a-fA-F]+}} <col:19, col:55> '<dependent type>'
// CHECK-NEXT: DependentScopeDeclRefExpr {{0x[0-9a-fA-F]+}} <col:19, col:50> '<dependent type>' lvalue

// CHECK: FunctionDecl {{0x[0-9a-fA-F]+}} <line:10:1, line:12:1> line:10:26 used getLegs 'typename Pup::LegType (Pup)'
// CHECK-NEXT: TemplateArgument type 'Pup'
// CHECK:      DeclRefExpr {{0x[0-9a-fA-F]+}} <col:10> 'Pup':'Pup' lvalue ParmVar {{0x[0-9a-fA-F]+}} 'A' 'Pup':'Pup'
// CHECK-NEXT: CallExpr {{0x[0-9a-fA-F]+}} <col:19, col:55> 'int':'int'
// CHECK-NEXT: ImplicitCastExpr {{0x[0-9a-fA-F]+}} <col:19, col:50> 'int (*)()' <FunctionToPointerDecay>
// CHECK-NEXT: DeclRefExpr {{0x[0-9a-fA-F]+}} <col:19, col:50> 'int ()' lvalue CXXMethod {{0x[0-9a-fA-F]+}} 'zero' 'int ()'

struct Pup {
  using LegType = int;
  LegType Legs;
};

int Fn() {
  Pup P = {0};
  return getLegs<Pup>(P);
}
