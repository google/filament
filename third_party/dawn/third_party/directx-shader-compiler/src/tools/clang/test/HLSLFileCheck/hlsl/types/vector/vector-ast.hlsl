// RUN: %dxc -T lib_6_x -ast-dump-implicit %s | FileCheck %s

// Verify template and default arguments: vector<T = float, int = 4>

// CHECK: ClassTemplateDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit vector
// CHECK-NEXT: TemplateTypeParmDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> class element
// CHECK-NEXT: TemplateArgument type 'float'
// CHECK-NEXT: NonTypeTemplateParmDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> 'int' element_count
// CHECK-NEXT: TemplateArgument expr
// CHECK-NEXT: IntegerLiteral {{0x[0-9a-fA-F]+}} <<invalid sloc>> 'int' 4

// Verify the class, final attribute and ext_vector field decl.
// CHECK-NEXT: CXXRecordDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit class vector definition
// CHECK-NEXT: FinalAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit final
// CHECK-NEXT: HLSLVectorAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit
// CHECK-NEXT: FieldDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit h 'element __attribute__((ext_vector_type(element_count)))'

// Verify operator overloads for const vector subscript operators.
// CHECK-NEXT: CXXMethodDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> operator[] 'element &const (unsigned int) const'
// CHECK-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> index 'unsigned int'
// CHECK-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit "subscript" "" 7
// CHECK-NEXT: HLSLCXXOverloadAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit

// Verify operator overloads for non-const vector subscript operators.
// CHECK-NEXT: CXXMethodDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> operator[] 'element &(unsigned int)'
// CHECK-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> index 'unsigned int'
// CHECK-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit "subscript" "" 7
// CHECK-NEXT: HLSLCXXOverloadAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit

// DXC forces specializations to exist for common types. This verifies the
// specializtion for float4 which comes next in the chain.
// CHECK-NEXT: ClassTemplateSpecializationDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit class vector definition
// CHECK-NEXT: TemplateArgument type 'float'
// CHECK-NEXT: TemplateArgument integral 4

