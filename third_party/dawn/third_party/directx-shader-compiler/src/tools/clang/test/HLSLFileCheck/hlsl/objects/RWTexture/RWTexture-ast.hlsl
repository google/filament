// RUN: %dxc -T ps_6_0 -ast-dump-implicit %s | FileCheck %s

RWTexture1D<float4> Tex;

float4 main(uint i:I) : SV_Target {
  return Tex[i];
}


// Verify the template construction, and the underlying record.
// CHECK: ClassTemplateDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit RWTexture1D
// CHECK: CXXRecordDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit class RWTexture1D definition
// CHECK-NEXT: FinalAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit final
// CHECK-NEXT: HLSLResourceAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit 1 1
// CHECK-NEXT: FieldDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit h 'element'

// Make sure we don't process into another record decl.
// CHECK-NOT: CXXRecordDecl

// Verify the FunctionTemplateDecl for the subscript operator and the method
// containing the appropriate atributes.
// CHECK: FunctionTemplateDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> operator[]
// CHECK-NEXT: TemplateTypeParmDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> class element
// CHECK-NEXT: CXXMethodDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> operator[] 'element &(unsigned int) const'
// CHECK-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> index 'unsigned int'
// CHECK-NEXT: HLSLCXXOverloadAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit
