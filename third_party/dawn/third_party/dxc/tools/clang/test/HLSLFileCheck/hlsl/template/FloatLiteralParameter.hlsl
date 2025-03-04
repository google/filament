// RUN: %dxc -T cs_6_5 -HV 2021 -ast-dump-implicit %s | FileCheck -check-prefix=AST %s
// RUN: %dxc -T cs_6_5 -HV 2021 %s | FileCheck %s
RWStructuredBuffer<float> Output;

template <typename T> T Fn(T input) {
  T ret = input - 1.f;

  return ret;
}

[numthreads(64, 1, 1)]
void main() {
  Output[0] = Fn(1.0);
  Output[1] = abs(-1.5);
}

// Verify the template we expect is appearing

// AST:      FunctionTemplateDecl 0x{{[0-9a-zA-Z]+}} <line:5:1, line:9:1> line:5:25 Fn
// AST-NEXT: TemplateTypeParmDecl 0x{{[0-9a-zA-Z]+}} <col:11, col:20> col:20 referenced typename T
// AST-NEXT: FunctionDecl 0x{{[0-9a-zA-Z]+}} <col:23, line:9:1> line:5:25 Fn 'T (T)'

// Make sure we don't have a 'literal float' expansion
// AST-NOT: TemplateArgument type 'literal float'

// Make sure we do have the 'float' expansion

// AST:      FunctionDecl 0x{{[0-9a-zA-Z]+}} <line:5:23, line:9:1> line:5:25 used Fn 'float (float)'
// AST-NEXT: TemplateArgument type 'float'

// Make sure a 'literal float' expansion doesn't sneak in at the end...
// AST-NOT: TemplateArgument type 'literal float'

// Unlike user-supplied templates, builtin templates we do want to support
// literal types in, because we can constant fold them as with higher precision.

// Verify that the abs call is still `literal float`
// AST: DeclRefExpr 0x{{[0-9a-zA-Z]+}} <col:15> 'literal float (literal float)' lvalue Function [[abs:0x[0-9a-zA-Z]+]] 'abs'

// AST:      FunctionDecl [[abs]] <<invalid sloc>> <invalid sloc> implicit used abs 'literal float (literal float)' extern
// AST-NEXT: ParmVarDecl 0x{{[0-9a-zA-Z]+}} <<invalid sloc>> <invalid sloc> x 'literal float'


// Since template expansion can also fail in IRGen, we should make sure we have
// some sane IR coming out of the full compile. After optimizations this should
// just amount to a constant store to the buffer.

// CHECK:       define void @main() {
// CHECK:         @dx.op.createHandle
// CHECK-NEXT:    call void @dx.op.rawBufferStore.f32
// CHECK-NEXT:    call void @dx.op.rawBufferStore.f32
// CHECK-NEXT:    ret void
