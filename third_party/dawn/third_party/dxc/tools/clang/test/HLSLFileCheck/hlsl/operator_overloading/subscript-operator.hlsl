// RUN: %dxc -T lib_6_4 -HV 2021 -ast-dump %s | FileCheck %s

struct MyVector3 {
    float3 V;

    float operator[](int x) {
        return V[x];
    }
};

float SomeFun(const MyVector3 V, int Idx) {
  return V[Idx];
}

// This test verifies that under HLSL 2021, overload resolution for user-defined
// subscript operators follows HLSL overload resolution rules and ignores the
// const-ness of the implicit object parameter.

// CHECK: CXXRecordDecl {{0x[0-9a-fA-F]+}} {{.*}} referenced struct MyVector3 definition
// CHECK: FieldDecl {{0x[0-9a-fA-F]+}} <line:4:5, col:12> col:12 referenced V 'float3':'vector<float, 3>'
// CHECK-NEXT: CXXMethodDecl [[Operator:0x[0-9a-fA-F]+]] <line:6:5, line:8:5> line:6:11 used operator[] 'float (int)'

// CHECK: FunctionDecl {{0x[0-9a-fA-F]+}} <line:11:1, line:13:1> line:11:7 SomeFun 'float (const MyVector3, int)'
// CHECK: CXXOperatorCallExpr {{0x[0-9a-fA-F]+}} <col:10, col:15> 'float'
// CHECK-NEXT: ImplicitCastExpr {{0x[0-9a-fA-F]+}} <col:11, col:15> 'float (*)(int)' <FunctionToPointerDecay>
// CHECK-NEXT: DeclRefExpr {{0x[0-9a-fA-F]+}} <col:11, col:15> 'float (int)' lvalue CXXMethod [[Operator]] 'operator[]' 'float (int)'
