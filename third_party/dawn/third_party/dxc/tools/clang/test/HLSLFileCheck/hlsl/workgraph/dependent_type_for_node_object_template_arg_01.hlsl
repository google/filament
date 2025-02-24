// RUN: %dxc -T lib_6_8 -ast-dump %s | FileCheck %s


// CHECK: ClassTemplateDecl 0x{{.*}} col:8 Record
// CHECK: |-TemplateTypeParmDecl 0x{{.*}} col:19 referenced typename T
// CHECK: -CXXRecordDecl {{.*}} col:8 struct Record definition
// CHECK: |-CXXRecordDecl 0x{{.*}} <col:1, col:8> col:8 implicit struct Record
// CHECK: `-FieldDecl 0x{{.*}} <col:17, col:19> col:19 id 'T'
// CHECK: ClassTemplateSpecializationDecl 0x[[TemplateSpecialization:[a-f0-9]+]] {{.*}} col:8 struct Record definition
// CHECK:   |-TemplateArgument type 'float'
// CHECK:   |-CXXRecordDecl {{.*}} prev 0x[[TemplateSpecialization]] <col:1, col:8> col:8 implicit struct Record
// CHECK:   `-FieldDecl {{.*}} <col:17, col:19> col:19 id 'float':'float'

template<typename T>
struct Record { T id; };

// CHECK: FunctionTemplateDecl 0x[[Template:[a-f0-9]+]] {{.*}} col:28 doSomething
// CHECK: | |-TemplateTypeParmDecl {{.*}} <col:11, col:20> col:20 referenced typename T
// CHECK: | |-FunctionDecl {{.*}} <col:23, col:81> col:28 doSomething 'void (GroupNodeInputRecords<Record<T> >)'
// CHECK: | | |-ParmVarDecl {{.*}} <col:40, col:74> col:74 data 'GroupNodeInputRecords<Record<T> >'
// CHECK: | | `-CompoundStmt {{.*}} <col:80, col:81>
// CHECK: | `-FunctionDecl 0x[[Function:[a-f0-9]+]] <col:23, col:81> col:28 used doSomething 'void (GroupNodeInputRecords<Record<float> >)'
// CHECK: |   |-TemplateArgument type 'float'
// CHECK: |   |-ParmVarDecl {{.*}} <col:40, col:74> col:74 data 'GroupNodeInputRecords<Record<float> >':'GroupNodeInputRecords<Record<float> >'
// CHECK: |   `-CompoundStmt {{.*}} <col:80, col:81>

template <typename T> void doSomething(GroupNodeInputRecords<Record<T> > data) {}


[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(4,5,6)]
void node(GroupNodeInputRecords<Record<float> > inputData) {

// CHECK: -CallExpr {{.*}} 'void'
// CHECK:  |   |-ImplicitCastExpr {{.*}} <col:3> 'void (*)(GroupNodeInputRecords<Record<float> >)' <FunctionToPointerDecay>
// CHECK:  |   | `-DeclRefExpr {{.*}} <col:3> 'void (GroupNodeInputRecords<Record<float> >)' lvalue Function 0x[[Function]] 'doSomething' 'void (GroupNodeInputRecords<Record<float> >)' (FunctionTemplate 0x[[Template]] 'doSomething')
// CHECK:  |   `-ImplicitCastExpr {{.*}} <col:15> 'GroupNodeInputRecords<Record<float> >':'GroupNodeInputRecords<Record<float> >' <LValueToRValue>
// CHECK:  |     `-DeclRefExpr {{.*}} <col:15> 'GroupNodeInputRecords<Record<float> >':'GroupNodeInputRecords<Record<float> >' lvalue ParmVar {{.*}} 'inputData' 'GroupNodeInputRecords<Record<float> >':'GroupNodeInputRecords<Record<float> >'

  doSomething(inputData);
}
