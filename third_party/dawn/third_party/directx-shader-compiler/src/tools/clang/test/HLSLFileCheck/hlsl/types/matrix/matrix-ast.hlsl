// RUN: %dxc -T lib_6_x -ast-dump-implicit %s | FileCheck %s

// Verify the matrix template formtaion: matrix< T = float, rows = 4, cols = 4>
// CHECK: ClassTemplateDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit matrix
// CHECK-NEXT: TemplateTypeParmDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> class element
// CHECK-NEXT: TemplateArgument type 'float'
// CHECK-NEXT: NonTypeTemplateParmDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> 'int' row_count
// CHECK-NEXT: TemplateArgument expr
// CHECK-NEXT: IntegerLiteral {{0x[0-9a-fA-F]+}} <<invalid sloc>> 'int' 4
// CHECK-NEXT: NonTypeTemplateParmDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> 'int' col_count
// CHECK-NEXT: TemplateArgument expr
// CHECK-NEXT: IntegerLiteral {{0x[0-9a-fA-F]+}} <<invalid sloc>> 'int' 4

// Verify the record with the final attribute and the matrix field as an
// ext_vector array.
// CHECK-NEXT: CXXRecordDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit class matrix definition
// CHECK-NEXT: FinalAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit final
// CHECK-NEXT: HLSLMatrixAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit
// CHECK-NEXT: FieldDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit h 'element [row_count] __attribute__((ext_vector_type(col_count)))'


// Verify non-const subscript operator overload.
// CHECK-NEXT: CXXMethodDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> operator[] 'vector<element, col_count> &(unsigned int)'
// CHECK-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> index 'unsigned int'
// CHECK-NEXT: HLSLCXXOverloadAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit

// Verify const subscript operator overload.
// CHECK-NEXT: CXXMethodDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> operator[] 'vector<element, col_count> &const (unsigned int) const'
// CHECK-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> index 'unsigned int'
// CHECK-NEXT: HLSLCXXOverloadAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit
