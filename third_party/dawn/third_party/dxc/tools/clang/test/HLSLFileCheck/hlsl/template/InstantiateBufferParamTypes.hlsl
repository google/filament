// RUN: %dxc -E main -T cs_6_0 -ast-dump -HV 2021 %s | FileCheck %s
struct Foo { float f;};
template <typename T> struct Wrap { T value; };
ConstantBuffer<Wrap<Foo> >CB;

[numthreads(1,1,1)]
void main() {
}

// CHECK: ClassTemplateSpecializationDecl 0x{{[0-9a-zA-Z]+}} <col:1, col:46> col:30 struct Wrap definition
// CHECK-NEXT: TemplateArgument type 'Foo'
// CHECK-NEXT: CXXRecordDecl 0x{{[0-9a-zA-Z]+}} prev 0x{{[0-9a-zA-Z]+}} <col:23, col:30> col:30 implicit struct Wrap
// CHECK-NEXT: `-FieldDecl 0x{{[0-9a-zA-Z]+}} <col:37, col:39> col:39 value 'Foo':'Foo'
