// RUN: %dxc -T cs_6_0 -ast-dump %s | FileCheck %s -check-prefixes=CHECK,NO-IMPLICIT
// RUN: %dxc -T cs_6_0 -ast-dump-implicit %s | FileCheck %s -check-prefixes=CHECK,IMPLICIT
RWBuffer<float3> In;
RWBuffer<float3> Out;

[numthreads(1,1,1)]
void main() {
  Out[0] = In[0];
}

// CHECK: TranslationUnitDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc>

// If we're not dumping implicit decls, there should be no additional decls.
// NO-IMPLICIT-NOT: Decl

// This test doesn't seek to be exhaustive, just checking a few common implicit
// decls to ensure they got generated into the AST.
// IMPLICIT: CXXRecordDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit class vector definition
// IMPLICIT: CXXRecordDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit class matrix definition

// This set of checks verifies that the implicit decls for buffers are
// incomplete by looking at the `Buffer` type which is unused in this code and
// verifying it doesn't have any methods.

// IMPLICIT-NOT: CXXRecordDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit class Buffer
// IMPLICIT: ClassTemplateDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit Buffer
// IMPLICIT-NEXT: TemplateTypeParmDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> class element
// IMPLICIT-NEXT: TemplateArgument type 'vector<float, 4>':'vector<float, 4>'
// IMPLICIT-NEXT: CXXRecordDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit class Buffer
// IMPLICIT-NEXT: FinalAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit final
// IMPLICIT-NEXT: HLSLResourceAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit 10 0
// IMPLICIT-NEXT: FieldDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit h 'element'
// IMPLICIT-NOT: CXXMethodDecl
// IMPLICIT: CXXRecordDecl


// This tests to verfy that the RWBuffer _is_ completed with method definitions.

// IMPLICIT: ClassTemplateDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit RWBuffer
// IMPLICIT-NEXT: TemplateTypeParmDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> class element
// IMPLICIT-NEXT: CXXRecordDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit class RWBuffer definition
// IMPLICIT-NEXT: FinalAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit final
// IMPLICIT-NEXT: HLSLResourceAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit 10 1
// IMPLICIT-NEXT: FieldDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit h 'element'
// IMPLICIT-NEXT: FunctionTemplateDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> operator[]

// CHECK: VarDecl {{0x[0-9a-fA-F]+}} <{{.*}}ast-dumping.hlsl:3:1, col:18> col:18 used In 'RWBuffer<float3>':'RWBuffer<vector<float, 3> >'
// CHECK-NEXT: VarDecl {{0x[0-9a-fA-F]+}} <line:4:1, col:18> col:18 used Out 'RWBuffer<float3>':'RWBuffer<vector<float, 3> >'
// CHECK-NEXT: FunctionDecl {{0x[0-9a-fA-F]+}} <line:7:1, line:9:1> line:7:6 main 'void ()'

