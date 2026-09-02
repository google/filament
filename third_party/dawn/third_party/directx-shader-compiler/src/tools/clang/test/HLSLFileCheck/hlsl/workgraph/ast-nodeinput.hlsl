// RUN: %dxc -T lib_6_8 -ast-dump-implicit %s | FileCheck %s
// This test verifies the AST of the {Dispatch|Group|Thread}NodeInputRecord{s} types. The
// source doesn't matter except that it forces a use to ensure the AST is fully
// loaded by the external sema source.

struct RECORD {
  int X;
};

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1024,1,1)]
[NodeDispatchGrid(512,1,1)]
void node01(DispatchNodeInputRecord<RECORD> input) {}

//CHECK: ClassTemplateDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit DispatchNodeInputRecord
//CHECK-NEXT: TemplateTypeParmDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> class recordtype
//CHECK-NEXT: CXXRecordDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit struct DispatchNodeInputRecord definition
//CHECK-NEXT: FinalAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit final
//CHECK-NEXT: HLSLNodeObjectAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit DispatchNodeInputRecord
//CHECK-NEXT: FieldDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit h 'int'
//CHECK-NEXT: CXXMethodDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> Get 'const recordtype &() const'
//CHECK-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit "subscript" "" 0
//CHECK-NEXT: HLSLCXXOverloadAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit
//CHECK-NEXT: ClassTemplateSpecializationDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> struct DispatchNodeInputRecord definition

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1024,1,1)]
void node02(GroupNodeInputRecords<RECORD> input) {}

//CHECK: ClassTemplateDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit GroupNodeInputRecords
//CHECK-NEXT: TemplateTypeParmDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> class recordtype
//CHECK-NEXT: CXXRecordDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit struct GroupNodeInputRecords definition
//CHECK-NEXT: FinalAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit final
//CHECK-NEXT: HLSLNodeObjectAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit GroupNodeInputRecords
//CHECK-NEXT: FieldDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit h 'int'
//CHECK-NEXT: CXXMethodDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> Get 'const recordtype &(unsigned int) const'
//CHECK-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> Index 'unsigned int' cinit
//CHECK-NEXT: IntegerLiteral {{0x[0-9a-fA-F]+}} <<invalid sloc>> 'unsigned int' 0
//CHECK-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit "subscript" "" 0
//CHECK-NEXT: HLSLCXXOverloadAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit
//CHECK-NEXT: CXXMethodDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> operator[] 'const recordtype &(unsigned int) const'
//CHECK-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> Index 'unsigned int'
//CHECK-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit "subscript" "" 0
//CHECK-NEXT: HLSLCXXOverloadAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit
//CHECK-NEXT: FunctionTemplateDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> operator[]
//CHECK-NEXT: TemplateTypeParmDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> class recordtype
//CHECK-NEXT: CXXMethodDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> operator[] 'const recordtype &(unsigned int) const'
//CHECK-NEXT: ParmVarDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> index 'unsigned int'
//CHECK-NEXT: HLSLCXXOverloadAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit
//CHECK-NEXT: FunctionTemplateDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> Count
//CHECK-NEXT: TemplateTypeParmDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> class TResult
//CHECK-NEXT: CXXMethodDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit Count 'TResult () const'
//CHECK-NEXT: ClassTemplateSpecializationDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> struct GroupNodeInputRecords definition

[Shader("node")]
[NodeLaunch("thread")]
void node03(ThreadNodeInputRecord<RECORD> input) {}

//CHECK: ClassTemplateDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit ThreadNodeInputRecord
//CHECK-NEXT: TemplateTypeParmDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> class recordtype
//CHECK-NEXT: CXXRecordDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit struct ThreadNodeInputRecord definition
//CHECK-NEXT: FinalAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit final
//CHECK-NEXT: HLSLNodeObjectAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit ThreadNodeInputRecord
//CHECK-NEXT: FieldDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> implicit h 'int'
//CHECK-NEXT: CXXMethodDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> Get 'const recordtype &() const'
//CHECK-NEXT: HLSLIntrinsicAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit "subscript" "" 0
//CHECK-NEXT: HLSLCXXOverloadAttr {{0x[0-9a-fA-F]+}} <<invalid sloc>> Implicit
//CHECK-NEXT: ClassTemplateSpecializationDecl {{0x[0-9a-fA-F]+}} <<invalid sloc>> <invalid sloc> struct ThreadNodeInputRecord definition

[Shader("node")]
[NodeLaunch("coalescing")]
[NumThreads(1024,1,1)]
void node04(EmptyNodeInput input) {}
