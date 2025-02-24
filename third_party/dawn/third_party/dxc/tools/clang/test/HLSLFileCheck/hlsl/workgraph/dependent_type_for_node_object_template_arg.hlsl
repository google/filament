// RUN: %dxc -T lib_6_8 -ast-dump %s | FileCheck %s

// CHECK: -FunctionTemplateDecl 0x[[Template:[a-f0-9]+]] {{.*}} col:28 doSomething
// CHECK: | |-TemplateTypeParmDecl {{.*}} <col:11, col:20> col:20 referenced typename T
// CHECK: | |-FunctionDecl {{.*}} <col:23, col:72> col:28 doSomething 'void (GroupNodeInputRecords<T>)'
// CHECK: | | |-ParmVarDecl {{.*}} <col:40, col:65> col:65 data 'GroupNodeInputRecords<T>'
// CHECK: | | `-CompoundStmt
// CHECK: | `-FunctionDecl 0x[[Function:[a-f0-9]+]] <col:23, col:72> col:28 used doSomething 'void (GroupNodeInputRecords<Record>)'
// CHECK: |   |-TemplateArgument type 'Record'
// CHECK: |   |-ParmVarDecl {{.*}} col:65 data 'GroupNodeInputRecords<Record>':'GroupNodeInputRecords<Record>'
// CHECK: |   `-CompoundStmt

template <typename T> void doSomething(GroupNodeInputRecords<T> data) {}

struct Record { int id; };

Texture2D<float> t;

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(4,5,6)]
void node(GroupNodeInputRecords<Record> inputData) {
// CHECK: -CallExpr {{.*}} 'void'
// CHECK: |-ImplicitCastExpr {{.*}} <col:3> 'void (*)(GroupNodeInputRecords<Record>)' <FunctionToPointerDecay>
// CHECK:  | `-DeclRefExpr {{.*}} <col:3> 'void (GroupNodeInputRecords<Record>)' lvalue Function 0x[[Function]] 'doSomething' 'void (GroupNodeInputRecords<Record>)' (FunctionTemplate 0x[[Template]] 'doSomething')
// CHECK: `-ImplicitCastExpr {{.*}} <col:15> 'GroupNodeInputRecords<Record>':'GroupNodeInputRecords<Record>' <LValueToRValue>
// CHECK:   `-DeclRefExpr {{.*}} <col:15> 'GroupNodeInputRecords<Record>':'GroupNodeInputRecords<Record>' lvalue ParmVar {{.*}} 'inputData' 'GroupNodeInputRecords<Record>':'GroupNodeInputRecords<Record>'
  doSomething(inputData);
}
